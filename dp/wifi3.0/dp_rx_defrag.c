/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "dp_rx_defrag.h"
#include <enet.h>	/* LLC_SNAP_HDR_LEN */
#include "dp_rx_defrag.h"

const struct dp_rx_defrag_cipher dp_f_ccmp = {
	"AES-CCM",
	IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN + IEEE80211_WEP_EXTIVLEN,
	IEEE80211_WEP_MICLEN,
	0,
};

const struct dp_rx_defrag_cipher dp_f_tkip = {
	"TKIP",
	IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN + IEEE80211_WEP_EXTIVLEN,
	IEEE80211_WEP_CRCLEN,
	IEEE80211_WEP_MICLEN,
};

const struct dp_rx_defrag_cipher dp_f_wep = {
	"WEP",
	IEEE80211_WEP_IVLEN + IEEE80211_WEP_KIDLEN,
	IEEE80211_WEP_CRCLEN,
	0,
};

/*
 * dp_rx_defrag_frames_free(): Free fragment chain
 * @frames: Fragment chain
 *
 * Iterates through the fragment chain and frees them
 * Returns: None
 */
static void dp_rx_defrag_frames_free(qdf_nbuf_t frames)
{
	qdf_nbuf_t next, frag = frames;

	while (frag) {
		next = qdf_nbuf_next(frag);
		qdf_nbuf_free(frag);
		frag = next;
	}
}

/*
 * dp_rx_clear_saved_desc_info(): Clears descriptor info
 * @peer: Pointer to the peer data structure
 * @tid: Transmit ID (TID)
 *
 * Saves MPDU descriptor info and MSDU link pointer from REO
 * ring descriptor. The cache is created per peer, per TID
 *
 * Returns: None
 */
static void dp_rx_clear_saved_desc_info(struct dp_peer *peer, unsigned tid)
{
	hal_rx_clear_mpdu_desc_info(
		&peer->rx_tid[tid].transcap_rx_mpdu_desc_info);

	hal_rx_clear_msdu_link_ptr(
		&peer->rx_tid[tid].transcap_msdu_link_ptr[0],
		HAL_RX_MAX_SAVED_RING_DESC);
}

/*
 * dp_rx_defrag_waitlist_add(): Update per-PDEV defrag wait list
 * @peer: Pointer to the peer data structure
 * @tid: Transmit ID (TID)
 *
 * Appends per-tid fragments to global fragment wait list
 *
 * Returns: None
 */
static void dp_rx_defrag_waitlist_add(struct dp_peer *peer, unsigned tid)
{
	struct dp_soc *psoc = peer->vdev->pdev->soc;
	struct dp_rx_tid *rx_reorder = &peer->rx_tid[tid];

	/* TODO: use LIST macros instead of TAIL macros */
	TAILQ_INSERT_TAIL(&psoc->rx.defrag.waitlist, rx_reorder,
				defrag_waitlist_elem);
}

/*
 * dp_rx_defrag_waitlist_remove(): Remove fragments from waitlist
 * @peer: Pointer to the peer data structure
 * @tid: Transmit ID (TID)
 *
 * Remove fragments from waitlist
 *
 * Returns: None
 */
static void dp_rx_defrag_waitlist_remove(struct dp_peer *peer, unsigned tid)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_rx_tid *rx_reorder;

	if (tid > DP_MAX_TIDS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"TID out of bounds: %d", tid);
		qdf_assert(0);
		return;
	}

	rx_reorder = &peer->rx_tid[tid];

	if (rx_reorder->defrag_waitlist_elem.tqe_next != NULL) {

		TAILQ_REMOVE(&soc->rx.defrag.waitlist, rx_reorder,
				defrag_waitlist_elem);
		rx_reorder->defrag_waitlist_elem.tqe_next = NULL;
		rx_reorder->defrag_waitlist_elem.tqe_prev = NULL;
	} else if (rx_reorder->defrag_waitlist_elem.tqe_prev == NULL) {

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"waitlist->tqe_prev is NULL");
		rx_reorder->defrag_waitlist_elem.tqe_next = NULL;
		qdf_assert(0);
	}
}

/*
 * dp_rx_defrag_fraglist_insert(): Create a per-sequence fragment list
 * @peer: Pointer to the peer data structure
 * @tid: Transmit ID (TID)
 * @head_addr: Pointer to head list
 * @tail_addr: Pointer to tail list
 * @frag: Incoming fragment
 * @all_frag_present: Flag to indicate whether all fragments are received
 *
 * Build a per-tid, per-sequence fragment list.
 *
 * Returns: None
 */
static void dp_rx_defrag_fraglist_insert(struct dp_peer *peer, unsigned tid,
	qdf_nbuf_t *head_addr, qdf_nbuf_t *tail_addr, qdf_nbuf_t frag,
	uint8_t *all_frag_present)
{
	qdf_nbuf_t next;
	qdf_nbuf_t prev = NULL;
	qdf_nbuf_t cur;
	uint16_t head_fragno, cur_fragno, next_fragno;
	uint8_t last_morefrag = 1, count = 0;
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];
	uint8_t *rx_desc_info;

	qdf_assert(frag);
	qdf_assert(head_addr);
	qdf_assert(tail_addr);

	rx_desc_info = qdf_nbuf_data(frag);
	cur_fragno = dp_rx_frag_get_mpdu_frag_number(rx_desc_info);

	/* If this is the first fragment */
	if (!(*head_addr)) {
		*head_addr = *tail_addr = frag;
		qdf_nbuf_set_next(*tail_addr, NULL);
		rx_tid->curr_frag_num = cur_fragno;

		goto end;
	}

	/* In sequence fragment */
	if (cur_fragno > rx_tid->curr_frag_num) {
		qdf_nbuf_set_next(*tail_addr, frag);
		*tail_addr = frag;
		qdf_nbuf_set_next(*tail_addr, NULL);
		rx_tid->curr_frag_num = cur_fragno;
	} else {
		/* Out of sequence fragment */
		cur = *head_addr;
		rx_desc_info = qdf_nbuf_data(cur);
		head_fragno = dp_rx_frag_get_mpdu_frag_number(rx_desc_info);

		if (cur_fragno == head_fragno) {
			qdf_nbuf_free(frag);
			*all_frag_present = 0;
		} else if (head_fragno > cur_fragno) {
			qdf_nbuf_set_next(frag, cur);
			cur = frag;
			*head_addr = frag; /* head pointer to be updated */
		} else {
			while ((cur_fragno > head_fragno) && cur != NULL) {
				prev = cur;
				cur = qdf_nbuf_next(cur);
				rx_desc_info = qdf_nbuf_data(cur);
				head_fragno =
					dp_rx_frag_get_mpdu_frag_number(
								rx_desc_info);
			}
			qdf_nbuf_set_next(prev, frag);
			qdf_nbuf_set_next(frag, cur);
		}
	}

	next = qdf_nbuf_next(*head_addr);

	rx_desc_info = qdf_nbuf_data(*tail_addr);
	last_morefrag = hal_rx_get_rx_more_frag_bit(rx_desc_info);

	/* TODO: optimize the loop */
	if (!last_morefrag) {
		/* Check if all fragments are present */
		do {
			rx_desc_info = qdf_nbuf_data(next);
			next_fragno =
				dp_rx_frag_get_mpdu_frag_number(rx_desc_info);
			count++;

			if (next_fragno != count)
				break;

			next = qdf_nbuf_next(next);
		} while (next);

		if (!next) {
			*all_frag_present = 1;
			return;
		}
	}

end:
	*all_frag_present = 0;
}


/*
 * dp_rx_defrag_tkip_decap(): decap tkip encrypted fragment
 * @msdu: Pointer to the fragment
 * @hdrlen: 802.11 header length (mostly useful in 4 addr frames)
 *
 * decap tkip encrypted fragment
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_tkip_decap(qdf_nbuf_t msdu, uint16_t hdrlen)
{
	uint8_t *ivp, *orig_hdr;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);

	/* start of 802.11 header info */
	orig_hdr = (uint8_t *)(qdf_nbuf_data(msdu) + rx_desc_len);

	/* TKIP header is located post 802.11 header */
	ivp = orig_hdr + hdrlen;
	if (!(ivp[IEEE80211_WEP_IVLEN] & IEEE80211_WEP_EXTIV)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"IEEE80211_WEP_EXTIV is missing in TKIP fragment");
		return QDF_STATUS_E_DEFRAG_ERROR;
	}

	qdf_mem_move(orig_hdr + dp_f_tkip.ic_header, orig_hdr, hdrlen);

	qdf_nbuf_pull_head(msdu, dp_f_tkip.ic_header);
	qdf_nbuf_trim_tail(msdu, dp_f_tkip.ic_trailer);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_ccmp_demic(): Remove MIC information from CCMP fragment
 * @nbuf: Pointer to the fragment buffer
 * @hdrlen: 802.11 header length (mostly useful in 4 addr frames)
 *
 * Remove MIC information from CCMP fragment
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_ccmp_demic(qdf_nbuf_t nbuf, uint16_t hdrlen)
{
	uint8_t *ivp, *orig_hdr;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);

	/* start of the 802.11 header */
	orig_hdr = (uint8_t *)(qdf_nbuf_data(nbuf) + rx_desc_len);

	/* CCMP header is located after 802.11 header */
	ivp = orig_hdr + hdrlen;
	if (!(ivp[IEEE80211_WEP_IVLEN] & IEEE80211_WEP_EXTIV))
		return QDF_STATUS_E_DEFRAG_ERROR;

	qdf_nbuf_trim_tail(nbuf, dp_f_ccmp.ic_trailer);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_ccmp_decap(): decap CCMP encrypted fragment
 * @nbuf: Pointer to the fragment
 * @hdrlen: length of the header information
 *
 * decap CCMP encrypted fragment
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_ccmp_decap(qdf_nbuf_t nbuf, uint16_t hdrlen)
{
	uint8_t *ivp, *origHdr;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);

	origHdr = (uint8_t *) (qdf_nbuf_data(nbuf) + rx_desc_len);
	ivp = origHdr + hdrlen;

	if (!(ivp[IEEE80211_WEP_IVLEN] & IEEE80211_WEP_EXTIV))
		return QDF_STATUS_E_DEFRAG_ERROR;

	qdf_mem_move(origHdr + dp_f_ccmp.ic_header, origHdr, hdrlen);
	qdf_nbuf_pull_head(nbuf, dp_f_ccmp.ic_header);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_wep_decap(): decap WEP encrypted fragment
 * @msdu: Pointer to the fragment
 * @hdrlen: length of the header information
 *
 * decap WEP encrypted fragment
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_wep_decap(qdf_nbuf_t msdu, uint16_t hdrlen)
{
	uint8_t *origHdr;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);

	origHdr = (uint8_t *) (qdf_nbuf_data(msdu) + rx_desc_len);
	qdf_mem_move(origHdr + dp_f_wep.ic_header, origHdr, hdrlen);

	qdf_nbuf_pull_head(msdu, dp_f_wep.ic_header);
	qdf_nbuf_trim_tail(msdu, dp_f_wep.ic_trailer);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_hdrsize(): Calculate the header size of the received fragment
 * @nbuf: Pointer to the fragment
 *
 * Calculate the header size of the received fragment
 *
 * Returns: header size (uint16_t)
 */
static uint16_t dp_rx_defrag_hdrsize(qdf_nbuf_t nbuf)
{
	uint8_t *rx_tlv_hdr = qdf_nbuf_data(nbuf);
	uint16_t size = sizeof(struct ieee80211_frame);
	uint16_t fc = 0;
	uint32_t to_ds, fr_ds;
	uint8_t frm_ctrl_valid;
	uint16_t frm_ctrl_field;

	to_ds = hal_rx_mpdu_get_to_ds(rx_tlv_hdr);
	fr_ds = hal_rx_mpdu_get_fr_ds(rx_tlv_hdr);
	frm_ctrl_valid = hal_rx_get_mpdu_frame_control_valid(rx_tlv_hdr);
	frm_ctrl_field = hal_rx_get_frame_ctrl_field(rx_tlv_hdr);

	if (to_ds && fr_ds)
		size += IEEE80211_ADDR_LEN;

	if (frm_ctrl_valid) {
		fc = frm_ctrl_field;

		/* use 1-st byte for validation */
		if (DP_RX_DEFRAG_IEEE80211_QOS_HAS_SEQ(fc & 0xff)) {
			size += sizeof(uint16_t);
			/* use 2-nd byte for validation */
			if (((fc & 0xff00) >> 8) & IEEE80211_FC1_ORDER)
				size += sizeof(struct ieee80211_htc);
		}
	}

	return size;
}

/*
 * dp_rx_defrag_michdr(): Calculate a psuedo MIC header
 * @wh0: Pointer to the wireless header of the fragment
 * @hdr: Array to hold the psuedo header
 *
 * Calculate a psuedo MIC header
 *
 * Returns: None
 */
static void dp_rx_defrag_michdr(const struct ieee80211_frame *wh0,
				uint8_t hdr[])
{
	const struct ieee80211_frame_addr4 *wh =
		(const struct ieee80211_frame_addr4 *)wh0;

	switch (wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) {
	case IEEE80211_FC1_DIR_NODS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr1); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + IEEE80211_ADDR_LEN,
					   wh->i_addr2);
		break;
	case IEEE80211_FC1_DIR_TODS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr3); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + IEEE80211_ADDR_LEN,
					   wh->i_addr2);
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr1); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + IEEE80211_ADDR_LEN,
					   wh->i_addr3);
		break;
	case IEEE80211_FC1_DIR_DSTODS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr3); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + IEEE80211_ADDR_LEN,
					   wh->i_addr4);
		break;
	}

	/*
	 * Bit 7 is IEEE80211_FC0_SUBTYPE_QOS for data frame, but
	 * it could also be set for deauth, disassoc, action, etc. for
	 * a mgt type frame. It comes into picture for MFP.
	 */
	if (wh->i_fc[0] & IEEE80211_FC0_SUBTYPE_QOS) {
		const struct ieee80211_qosframe *qwh =
			(const struct ieee80211_qosframe *)wh;
		hdr[12] = qwh->i_qos[0] & IEEE80211_QOS_TID;
	} else {
		hdr[12] = 0;
	}

	hdr[13] = hdr[14] = hdr[15] = 0;	/* reserved */
}

/*
 * dp_rx_defrag_mic(): Calculate MIC header
 * @key: Pointer to the key
 * @wbuf: fragment buffer
 * @off: Offset
 * @data_len: Data lengh
 * @mic: Array to hold MIC
 *
 * Calculate a psuedo MIC header
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_mic(const uint8_t *key, qdf_nbuf_t wbuf,
		uint16_t off, uint16_t data_len, uint8_t mic[])
{
	uint8_t hdr[16] = { 0, };
	uint32_t l, r;
	const uint8_t *data;
	uint32_t space;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);

	dp_rx_defrag_michdr((struct ieee80211_frame *)(qdf_nbuf_data(wbuf)
		+ rx_desc_len), hdr);
	l = dp_rx_get_le32(key);
	r = dp_rx_get_le32(key + 4);

	/* Michael MIC pseudo header: DA, SA, 3 x 0, Priority */
	l ^= dp_rx_get_le32(hdr);
	dp_rx_michael_block(l, r);
	l ^= dp_rx_get_le32(&hdr[4]);
	dp_rx_michael_block(l, r);
	l ^= dp_rx_get_le32(&hdr[8]);
	dp_rx_michael_block(l, r);
	l ^= dp_rx_get_le32(&hdr[12]);
	dp_rx_michael_block(l, r);

	/* first buffer has special handling */
	data = (uint8_t *) qdf_nbuf_data(wbuf) + rx_desc_len + off;
	space = qdf_nbuf_len(wbuf) - rx_desc_len - off;

	for (;; ) {
		if (space > data_len)
			space = data_len;

		/* collect 32-bit blocks from current buffer */
		while (space >= sizeof(uint32_t)) {
			l ^= dp_rx_get_le32(data);
			dp_rx_michael_block(l, r);
			data += sizeof(uint32_t);
			space -= sizeof(uint32_t);
			data_len -= sizeof(uint32_t);
		}
		if (data_len < sizeof(uint32_t))
			break;

		wbuf = qdf_nbuf_next(wbuf);
		if (wbuf == NULL)
			return QDF_STATUS_E_DEFRAG_ERROR;

		if (space != 0) {
			const uint8_t *data_next;
			/*
			 * Block straddles buffers, split references.
			 */
			data_next =
				(uint8_t *) qdf_nbuf_data(wbuf) + rx_desc_len;
			if ((qdf_nbuf_len(wbuf) - rx_desc_len) <
				sizeof(uint32_t) - space) {
				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			switch (space) {
			case 1:
				l ^= dp_rx_get_le32_split(data[0],
					data_next[0], data_next[1],
					data_next[2]);
				data = data_next + 3;
				space = (qdf_nbuf_len(wbuf) - rx_desc_len)
					- 3;
				break;
			case 2:
				l ^= dp_rx_get_le32_split(data[0], data[1],
						    data_next[0], data_next[1]);
				data = data_next + 2;
				space = (qdf_nbuf_len(wbuf) - rx_desc_len)
					- 2;
				break;
			case 3:
				l ^= dp_rx_get_le32_split(data[0], data[1],
					data[2], data_next[0]);
				data = data_next + 1;
				space = (qdf_nbuf_len(wbuf) - rx_desc_len)
					- 1;
				break;
			}
			dp_rx_michael_block(l, r);
			data_len -= sizeof(uint32_t);
		} else {
			/*
			 * Setup for next buffer.
			 */
			data = (uint8_t *) qdf_nbuf_data(wbuf) + rx_desc_len;
			space = qdf_nbuf_len(wbuf) - rx_desc_len;
		}
	}
	/* Last block and padding (0x5a, 4..7 x 0) */
	switch (data_len) {
	case 0:
		l ^= dp_rx_get_le32_split(0x5a, 0, 0, 0);
		break;
	case 1:
		l ^= dp_rx_get_le32_split(data[0], 0x5a, 0, 0);
		break;
	case 2:
		l ^= dp_rx_get_le32_split(data[0], data[1], 0x5a, 0);
		break;
	case 3:
		l ^= dp_rx_get_le32_split(data[0], data[1], data[2], 0x5a);
		break;
	}
	dp_rx_michael_block(l, r);
	dp_rx_michael_block(l, r);
	dp_rx_put_le32(mic, l);
	dp_rx_put_le32(mic + 4, r);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_tkip_demic(): Remove MIC header from the TKIP frame
 * @key: Pointer to the key
 * @msdu: fragment buffer
 * @hdrlen: Length of the header information
 *
 * Remove MIC information from the TKIP frame
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_tkip_demic(const uint8_t *key,
					qdf_nbuf_t msdu, uint16_t hdrlen)
{
	QDF_STATUS status;
	uint32_t pktlen;
	uint8_t mic[IEEE80211_WEP_MICLEN];
	uint8_t mic0[IEEE80211_WEP_MICLEN];
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);

	pktlen = qdf_nbuf_len(msdu) - rx_desc_len;

	status = dp_rx_defrag_mic(key, msdu, hdrlen,
				pktlen - (hdrlen + dp_f_tkip.ic_miclen), mic);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	qdf_nbuf_copy_bits(msdu, pktlen - dp_f_tkip.ic_miclen + rx_desc_len,
				dp_f_tkip.ic_miclen, (caddr_t)mic0);

	if (!qdf_mem_cmp(mic, mic0, dp_f_tkip.ic_miclen))
		return QDF_STATUS_E_DEFRAG_ERROR;

	qdf_nbuf_trim_tail(msdu, dp_f_tkip.ic_miclen);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_decap_recombine(): Recombine the fragments
 * @peer: Pointer to the peer
 * @frag_list: list of fragments
 * @tid: Transmit identifier
 * @hdrsize: Header size
 *
 * Recombine fragments
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_decap_recombine(struct dp_peer *peer,
			qdf_nbuf_t head_msdu, unsigned tid, uint16_t hdrsize)
{
	qdf_nbuf_t msdu = head_msdu;
	uint8_t i;
	uint8_t num_ring_desc_saved = peer->rx_tid[tid].curr_ring_desc_idx;
	uint8_t num_msdus;

	/* Stitch fragments together */
	for (i = 0; (i < num_ring_desc_saved) && msdu; i++) {

		struct hal_rx_msdu_link_ptr_info *msdu_link_ptr_info =
			&peer->rx_tid[tid].transcap_msdu_link_ptr[i];

		struct hal_rx_mpdu_desc_info *mpdu_desc_info =
			&peer->rx_tid[tid].transcap_rx_mpdu_desc_info;

		num_msdus = hal_rx_chain_msdu_links(msdu, msdu_link_ptr_info,
				mpdu_desc_info);

		msdu = qdf_nbuf_next(msdu);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_defrag_err() - rx err handler
 * @pdev: handle to pdev object
 * @vdev_id: vdev id
 * @peer_mac_addr: peer mac address
 * @tid: TID
 * @tsf32: TSF
 * @err_type: error type
 * @rx_frame: rx frame
 * @pn: PN Number
 * @key_id: key id
 *
 * This function handles rx error and send MIC error notification
 *
 * Return: None
 */
static void dp_rx_defrag_err(uint8_t vdev_id, uint8_t *peer_mac_addr,
	int tid, uint32_t tsf32, uint32_t err_type, qdf_nbuf_t rx_frame,
	uint64_t *pn, uint8_t key_id)
{
	/* TODO: Who needs to know about the TKIP MIC error */
}

/*
 * dp_rx_defrag_qos_decap(): Remove QOS header from the frame
 * @nbuf: Pointer to the frame buffer
 * @hdrlen: Length of the header information
 *
 * Recombine fragments
 *
 * Returns: None
 */
static void dp_rx_defrag_qos_decap(qdf_nbuf_t nbuf, uint16_t hdrlen)
{
	struct ieee80211_frame *wh;
	uint16_t qoslen;
	int pkt_tlv_size = sizeof(struct rx_pkt_tlvs); /* pkt TLV hdr size */
	uint16_t fc = 0;

	uint8_t *rx_tlv_hdr = qdf_nbuf_data(nbuf);

	/* Get the frame control field if it is valid */
	if (hal_rx_get_mpdu_frame_control_valid(rx_tlv_hdr))
		fc = hal_rx_get_frame_ctrl_field(rx_tlv_hdr);

	wh = (struct ieee80211_frame *)(qdf_nbuf_data(nbuf) + pkt_tlv_size);

	if (DP_RX_DEFRAG_IEEE80211_QOS_HAS_SEQ(fc & 0xff)) {
		qoslen = sizeof(struct ieee80211_qoscntl);

		/* Qos frame with Order bit set indicates a HTC frame */
		if (((fc & 0xff00) >> 8) & IEEE80211_FC1_ORDER)
			qoslen += sizeof(struct ieee80211_htc);

		/* remove QoS field from header */
		hdrlen -= qoslen;
		qdf_mem_move((uint8_t *)wh + qoslen, wh, hdrlen);

		wh = (struct ieee80211_frame *)qdf_nbuf_pull_head(nbuf,
							pkt_tlv_size +
							qoslen);
		/* clear QoS bit */
		if (wh)
			wh->i_fc[0] &= ~IEEE80211_FC0_SUBTYPE_QOS;
	}
}

/*
 * dp_rx_defrag_nwifi_to_8023(): Transcap 802.11 to 802.3
 * @msdu: Pointer to the fragment buffer
 *
 * Transcap the fragment from 802.11 to 802.3
 *
 * Returns: None
 */
static void dp_rx_defrag_nwifi_to_8023(qdf_nbuf_t msdu)
{
	struct ieee80211_frame wh;
	uint32_t hdrsize;
	struct llc_snap_hdr_t llchdr;
	struct ethernet_hdr_t *eth_hdr;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);
	struct ieee80211_frame *wh_ptr;

	wh_ptr = (struct ieee80211_frame *)(qdf_nbuf_data(msdu) +
		rx_desc_len);
	qdf_mem_copy(&wh, wh_ptr, sizeof(wh));
	hdrsize = sizeof(struct ieee80211_frame);
	qdf_mem_copy(&llchdr, ((uint8_t *) (qdf_nbuf_data(msdu) +
		rx_desc_len)) + hdrsize,
		sizeof(struct llc_snap_hdr_t));

	/*
	 * Now move the data pointer to the beginning of the mac header :
	 * new-header = old-hdr + (wifihdrsize + llchdrsize - ethhdrsize)
	 */
	qdf_nbuf_pull_head(msdu, (rx_desc_len + hdrsize +
		sizeof(struct llc_snap_hdr_t) -
		sizeof(struct ethernet_hdr_t)));
	eth_hdr = (struct ethernet_hdr_t *)(qdf_nbuf_data(msdu));

	switch (wh.i_fc[1] & IEEE80211_FC1_DIR_MASK) {
	case IEEE80211_FC1_DIR_NODS:
		qdf_mem_copy(eth_hdr->dest_addr, wh.i_addr1,
			IEEE80211_ADDR_LEN);
		qdf_mem_copy(eth_hdr->src_addr, wh.i_addr2,
			IEEE80211_ADDR_LEN);
		break;
	case IEEE80211_FC1_DIR_TODS:
		qdf_mem_copy(eth_hdr->dest_addr, wh.i_addr3,
			IEEE80211_ADDR_LEN);
		qdf_mem_copy(eth_hdr->src_addr, wh.i_addr2,
			IEEE80211_ADDR_LEN);
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		qdf_mem_copy(eth_hdr->dest_addr, wh.i_addr1,
			IEEE80211_ADDR_LEN);
		qdf_mem_copy(eth_hdr->src_addr, wh.i_addr3,
			IEEE80211_ADDR_LEN);
		break;
	case IEEE80211_FC1_DIR_DSTODS:
		break;
	}

	/* TODO: Is it requried to copy rx_pkt_tlvs
	 * to the start of data buffer?
	 */
	qdf_mem_copy(eth_hdr->ethertype, llchdr.ethertype,
			sizeof(llchdr.ethertype));
}

/*
 * dp_rx_defrag_reo_reinject(): Reinject the fragment chain back into REO
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 *
 * Reinject the fragment chain back into REO
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_reo_reinject(struct dp_peer *peer,
					unsigned tid)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	void *ring_desc;
	enum hal_reo_error_status error;
	struct hal_rx_mpdu_desc_info *saved_mpdu_desc_info;
	void *hal_srng = soc->reo_reinject_ring.hal_srng;
	struct hal_rx_msdu_link_ptr_info *saved_msdu_link_ptr;

	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"HAL RING Access For WBM Release SRNG Failed: %pK",
			hal_srng);
		goto done;
	}

	ring_desc = hal_srng_src_get_next(soc->hal_soc, hal_srng);

	qdf_assert(ring_desc);

	error = HAL_RX_ERROR_STATUS_GET(ring_desc);

	if (qdf_unlikely(error == HAL_REO_ERROR_DETECTED)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"HAL RING 0x%pK:error %d", hal_srng, error);

		/* Don't know how to deal with this condition -- assert */
		qdf_assert(0);
		goto done;
	}

	saved_mpdu_desc_info =
		&peer->rx_tid[tid].transcap_rx_mpdu_desc_info;

	/* first msdu link pointer */
	saved_msdu_link_ptr =
		&peer->rx_tid[tid].transcap_msdu_link_ptr[0];

	hal_rx_defrag_update_src_ring_desc(ring_desc,
		saved_mpdu_desc_info, saved_msdu_link_ptr);

	status = QDF_STATUS_SUCCESS;
done:
	hal_srng_access_end(soc->hal_soc, hal_srng);
	return status;
}

/*
 * dp_rx_defrag(): Defragment the fragment chain
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 * @frag_list: Pointer to head list
 * @frag_list_tail: Pointer to tail list
 *
 * Defragment the fragment chain
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag(struct dp_peer *peer, unsigned tid,
			qdf_nbuf_t frag_list, qdf_nbuf_t frag_list_tail)
{
	qdf_nbuf_t tmp_next;
	qdf_nbuf_t cur = frag_list, msdu;

	uint32_t index, tkip_demic = 0;
	uint16_t hdr_space;
	QDF_STATUS status;
	uint8_t key[DEFRAG_IEEE80211_KEY_LEN];
	struct dp_vdev *vdev = peer->vdev;

	cur = frag_list;
	hdr_space = dp_rx_defrag_hdrsize(cur);
	index = hal_rx_msdu_is_wlan_mcast(cur) ?
		dp_sec_mcast : dp_sec_ucast;

	switch (peer->security[index].sec_type) {
	case htt_sec_type_tkip:
		tkip_demic = 1;

	case htt_sec_type_tkip_nomic:
		while (cur) {
			tmp_next = qdf_nbuf_next(cur);
			if (dp_rx_defrag_tkip_decap(cur, hdr_space)) {

				/* TKIP decap failed, discard frags */
				dp_rx_defrag_frames_free(frag_list);

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: TKIP decap failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			cur = tmp_next;
		}
		break;

	case htt_sec_type_aes_ccmp:
		while (cur) {
			tmp_next = qdf_nbuf_next(cur);
			if (dp_rx_defrag_ccmp_demic(cur, hdr_space)) {

				/* CCMP demic failed, discard frags */
				dp_rx_defrag_frames_free(frag_list);

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: CCMP demic failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			if (dp_rx_defrag_ccmp_decap(cur, hdr_space)) {

				/* CCMP decap failed, discard frags */
				dp_rx_defrag_frames_free(frag_list);

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: CCMP decap failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			cur = tmp_next;
		}
		break;
	case htt_sec_type_wep40:
	case htt_sec_type_wep104:
	case htt_sec_type_wep128:
		while (cur) {
			tmp_next = qdf_nbuf_next(cur);
			if (dp_rx_defrag_wep_decap(cur, hdr_space)) {

				/* WEP decap failed, discard frags */
				dp_rx_defrag_frames_free(frag_list);

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: WEP decap failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			cur = tmp_next;
		}
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX,
			QDF_TRACE_LEVEL_ERROR,
			"dp_rx_defrag: Did not match any security type");
		break;
	}

	if (tkip_demic) {
		msdu = frag_list_tail; /* Only last fragment has the MIC */

		qdf_mem_copy(key,
			peer->security[index].michael_key,
			sizeof(peer->security[index].michael_key));
		if (dp_rx_defrag_tkip_demic(key, msdu, hdr_space)) {
			qdf_nbuf_free(msdu);
			dp_rx_defrag_err(vdev->vdev_id, peer->mac_addr.raw,
				tid, 0, QDF_STATUS_E_DEFRAG_ERROR, msdu,
				NULL, 0);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"dp_rx_defrag: TKIP demic failed");
			return QDF_STATUS_E_DEFRAG_ERROR;
		}
	}

	dp_rx_defrag_qos_decap(cur, hdr_space);

	/* Convert the header to 802.3 header */
	dp_rx_defrag_nwifi_to_8023(cur);

	status = dp_rx_defrag_decap_recombine(peer, cur, tid, hdr_space);

	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		"dp_rx_defrag_decap_recombine failed");

		qdf_assert(0);
	}

	return status;
}

/*
 * dp_rx_defrag_cleanup(): Clean up activities
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 * @seq: Sequence number
 *
 * Returns: None
 */
static void dp_rx_defrag_cleanup(struct dp_peer *peer, unsigned tid,
							uint16_t seq)
{
	struct dp_rx_reorder_array_elem *rx_reorder_array_elem =
				&peer->rx_tid[tid].array[seq];

	/* Free up nbufs */
	dp_rx_defrag_frames_free(rx_reorder_array_elem->head);

	/* Free up saved ring descriptors */
	dp_rx_clear_saved_desc_info(peer, tid);

	rx_reorder_array_elem->head = NULL;
	rx_reorder_array_elem->tail = NULL;
	peer->rx_tid[tid].defrag_timeout_ms = 0;
	peer->rx_tid[tid].curr_frag_num = 0;
	peer->rx_tid[tid].curr_seq_num = 0;
	peer->rx_tid[tid].curr_ring_desc_idx = 0;
}

/*
 * dp_rx_defrag_save_info_from_ring_desc(): Save info from REO ring descriptor
 * @ring_desc: Pointer to the ring descriptor
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 * @mpdu_desc_info: MPDU descriptor info
 *
 * Returns: None
 */
static void dp_rx_defrag_save_info_from_ring_desc(void *ring_desc,
	struct dp_peer *peer, unsigned tid,
	struct hal_rx_mpdu_desc_info *mpdu_desc_info)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	void *msdu_link_desc_va = NULL;
	uint8_t idx = peer->rx_tid[tid].curr_ring_desc_idx;
	uint8_t rbm;

	struct hal_rx_msdu_link_ptr_info *msdu_link_ptr_info =
		&peer->rx_tid[tid].transcap_msdu_link_ptr[++idx];
	struct hal_rx_mpdu_desc_info *tmp_mpdu_desc_info =
		&peer->rx_tid[tid].transcap_rx_mpdu_desc_info;
	struct hal_buf_info hbi;

	rbm = hal_rx_ret_buf_manager_get(ring_desc);
	if (qdf_unlikely(rbm != HAL_RX_BUF_RBM_SW3_BM)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid RBM while chaining frag MSDUs");
		return;
	}

	hal_rx_reo_buf_paddr_get(ring_desc, &hbi);

	msdu_link_desc_va =
		dp_rx_cookie_2_link_desc_va(pdev->soc, &hbi);

	hal_rx_defrag_save_info_from_ring_desc(msdu_link_desc_va,
		msdu_link_ptr_info, &hbi);

	qdf_mem_copy(tmp_mpdu_desc_info, mpdu_desc_info,
		sizeof(*tmp_mpdu_desc_info));
}

/*
 * dp_rx_defrag_store_fragment(): Store incoming fragments
 * @soc: Pointer to the SOC data structure
 * @ring_desc: Pointer to the ring descriptor
 * @mpdu_desc_info: MPDU descriptor info
 * @msdu_info: Pointer to MSDU descriptor info
 * @tid: Traffic Identifier
 * @rx_desc: Pointer to rx descriptor
  *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_store_fragment(struct dp_soc *soc,
			void *ring_desc,
			union dp_rx_desc_list_elem_t **head,
			union dp_rx_desc_list_elem_t **tail,
			struct hal_rx_mpdu_desc_info *mpdu_desc_info,
			struct hal_rx_msdu_desc_info *msdu_info,
			unsigned tid, struct dp_rx_desc *rx_desc)
{
	uint8_t idx;
	struct dp_rx_reorder_array_elem *rx_reorder_array_elem;
	struct dp_pdev *pdev;
	struct dp_peer *peer;
	uint16_t peer_id;
	uint16_t rxseq, seq;
	uint8_t fragno, more_frag, all_frag_present = 0;
	uint16_t seq_num = mpdu_desc_info->mpdu_seq;
	QDF_STATUS status;
	struct dp_rx_tid *rx_tid;
	uint8_t mpdu_sequence_control_valid;
	uint8_t mpdu_frame_control_valid;
	qdf_nbuf_t frag = rx_desc->nbuf;
	uint8_t *rx_desc_info;

	/* Check if the packet is from a valid peer */
	peer_id = DP_PEER_METADATA_PEER_ID_GET(
					mpdu_desc_info->peer_meta_data);
	peer = dp_peer_find_by_id(soc, peer_id);

	if (!peer) {
		/* We should not recieve anything from unknown peer
		 * however, that might happen while we are in the monitor mode.
		 * We don't need to handle that here
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Unknown peer, dropping the fragment");

		qdf_nbuf_free(frag);
		dp_rx_add_to_free_desc_list(head, tail, rx_desc);

		return QDF_STATUS_E_DEFRAG_ERROR;
	}

	pdev = peer->vdev->pdev;
	rx_tid = &peer->rx_tid[tid];

	seq = seq_num & (peer->rx_tid[tid].ba_win_size - 1);
	qdf_assert(seq == 0);
	rx_reorder_array_elem = &peer->rx_tid[tid].array[seq];

	rx_desc_info = qdf_nbuf_data(frag);
	mpdu_sequence_control_valid =
		hal_rx_get_mpdu_sequence_control_valid(rx_desc_info);

	/* Invalid MPDU sequence control field, MPDU is of no use */
	if (!mpdu_sequence_control_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid MPDU seq control field, dropping MPDU");
		qdf_nbuf_free(frag);
		dp_rx_add_to_free_desc_list(head, tail, rx_desc);

		qdf_assert(0);
		goto end;
	}

	mpdu_frame_control_valid =
		hal_rx_get_mpdu_frame_control_valid(rx_desc_info);

	/* Invalid frame control field */
	if (!mpdu_frame_control_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid frame control field, dropping MPDU");
		qdf_nbuf_free(frag);
		dp_rx_add_to_free_desc_list(head, tail, rx_desc);

		qdf_assert(0);
		goto end;
	}

	/* Current mpdu sequence */
	rxseq = hal_rx_get_rx_sequence(rx_desc_info);
	more_frag = hal_rx_get_rx_more_frag_bit(rx_desc_info);

	/* HW does not populate the fragment number as of now
	 * need to get from the 802.11 header
	 */
	fragno = dp_rx_frag_get_mpdu_frag_number(rx_desc_info);

	/*
	 * !more_frag: no more fragments to be delivered
	 * !frag_no: packet is not fragmented
	 * !rx_reorder_array_elem->head: no saved fragments so far
	 */
	if ((!more_frag) && (!fragno) && (!rx_reorder_array_elem->head)) {
		/* We should not get into this situation here.
		 * It means an unfragmented packet with fragment flag
		 * is delivered over the REO exception ring.
		 * Typically it follows normal rx path.
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Rcvd unfragmented pkt on REO Err srng, dropping");
		qdf_nbuf_free(frag);
		dp_rx_add_to_free_desc_list(head, tail, rx_desc);

		qdf_assert(0);
		goto end;
	}

	/* Check if the fragment is for the same sequence or a different one */
	if (rx_reorder_array_elem->head) {

		if (rxseq != rx_tid->curr_seq_num) {

			/* Drop stored fragments if out of sequence
			 * fragment is received
			 */
			dp_rx_defrag_frames_free(rx_reorder_array_elem->head);

			rx_reorder_array_elem->head = NULL;
			rx_reorder_array_elem->tail = NULL;

			/*
			 * The sequence number for this fragment becomes the
			 * new sequence number to be processed
			 */
			rx_tid->curr_seq_num = rxseq;

			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s mismatch, dropping earlier sequence ",
				(rxseq == rx_tid->curr_seq_num)
				? "address"
				: "seq number");
		}
	} else {
		/* Start of a new sequence */
		rx_tid->curr_seq_num = rxseq;
	}

	/*
	 * If the earlier sequence was dropped, this will be the fresh start.
	 * Else, continue with next fragment in a given sequence
	 */
	dp_rx_defrag_fraglist_insert(peer, tid, &rx_reorder_array_elem->head,
			&rx_reorder_array_elem->tail, frag,
			&all_frag_present);

	/*
	 * Currently, we can have only 6 MSDUs per-MPDU, if the current
	 * packet sequence has more than 6 MSDUs for some reason, we will
	 * have to use the next MSDU link descriptor and chain them together
	 * before reinjection
	 */
	if (more_frag == 0 || fragno == HAL_RX_NUM_MSDU_DESC) {
		/*
		 * Deep copy of MSDU link pointer and msdu descriptor structs
		 */
		idx = peer->rx_tid[tid].curr_ring_desc_idx;
		if (idx < HAL_RX_MAX_SAVED_RING_DESC) {
			dp_rx_defrag_save_info_from_ring_desc(ring_desc,
				peer, tid, mpdu_desc_info);

			peer->rx_tid[tid].curr_ring_desc_idx++;
		} else {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Max ring descr saved, dropping fragment");
			/*
			 * Free up saved fragments and ring descriptors if any
			 */
			goto end;
		}
	}

	/* TODO: handle fragment timeout gracefully */
	if (pdev->soc->rx.flags.defrag_timeout_check) {
		dp_rx_defrag_waitlist_remove(peer, tid);
		goto end;
	}

	/* Yet to receive more fragments for this sequence number */
	if (!all_frag_present) {
		uint32_t now_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks());

		peer->rx_tid[tid].defrag_timeout_ms =
			now_ms + pdev->soc->rx.defrag.timeout_ms;

		dp_rx_defrag_waitlist_add(peer, tid);
		goto end;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"All fragments received for sequence: %d", rxseq);

	/* Process the fragments */
	status = dp_rx_defrag(peer, tid, rx_reorder_array_elem->head,
		rx_reorder_array_elem->tail);
	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Fragment processing failed");
		goto end;
	}

	/* Re-inject the fragments back to REO for further processing */
	status = dp_rx_defrag_reo_reinject(peer, tid);
	if (QDF_IS_STATUS_SUCCESS(status))
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"Fragmented sequence successfully reinjected");
	else
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		"Fragmented sequence reinjection failed");

end:
	dp_rx_defrag_cleanup(peer, tid, seq);
	return QDF_STATUS_E_DEFRAG_ERROR;
}

/**
 * dp_rx_frag_handle() - Handles fragmented Rx frames
 *
 * @soc: core txrx main context
 * @ring_desc: opaque pointer to the REO error ring descriptor
 * @mpdu_desc_info: MPDU descriptor information from ring descriptor
 * @head: head of the local descriptor free-list
 * @tail: tail of the local descriptor free-list
 * @quota: No. of units (packets) that can be serviced in one shot.
 *
 * This function implements RX 802.11 fragmentation handling
 * The handling is mostly same as legacy fragmentation handling.
 * If required, this function can re-inject the frames back to
 * REO ring (with proper setting to by-pass fragmentation check
 * but use duplicate detection / re-ordering and routing these frames
 * to a different core.
 *
 * Return: uint32_t: No. of elements processed
 */
uint32_t dp_rx_frag_handle(struct dp_soc *soc, void *ring_desc,
		struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		union dp_rx_desc_list_elem_t **head,
		union dp_rx_desc_list_elem_t **tail,
		uint32_t quota)
{
	uint32_t rx_bufs_used = 0;
	void *link_desc_va;
	struct hal_buf_info buf_info;
	struct hal_rx_msdu_list msdu_list; /* per MPDU list of MSDUs */
	uint32_t tid;
	int idx;
	QDF_STATUS status;

	qdf_assert(soc);
	qdf_assert(mpdu_desc_info);

	/* Fragment from a valid peer */
	hal_rx_reo_buf_paddr_get(ring_desc, &buf_info);

	link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &buf_info);

	qdf_assert(link_desc_va);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"Number of MSDUs to process, num_msdus: %d",
		mpdu_desc_info->msdu_count);


	if (qdf_unlikely(mpdu_desc_info->msdu_count == 0)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Not sufficient MSDUs to process");
		return rx_bufs_used;
	}

	/* Get msdu_list for the given MPDU */
	hal_rx_msdu_list_get(link_desc_va, &msdu_list,
		&mpdu_desc_info->msdu_count);

	/* Process all MSDUs in the current MPDU */
	for (idx = 0; (idx < mpdu_desc_info->msdu_count) && quota--; idx++) {
		struct dp_rx_desc *rx_desc =
			dp_rx_cookie_2_va_rxdma_buf(soc,
				msdu_list.sw_cookie[idx]);

		qdf_assert(rx_desc);

		tid = hal_rx_mpdu_start_tid_get(rx_desc->rx_buf_start);

		/* Process fragment-by-fragment */
		status = dp_rx_defrag_store_fragment(soc, ring_desc,
				head, tail, mpdu_desc_info,
				&msdu_list.msdu_info[idx], tid,
				rx_desc);
		if (QDF_IS_STATUS_SUCCESS(status))
			rx_bufs_used++;
		else
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Rx Defragmentation error. mpdu_seq: 0x%x msdu_count: %d mpdu_flags: %d",
			mpdu_desc_info->mpdu_seq, mpdu_desc_info->msdu_count,
			mpdu_desc_info->mpdu_flags);
	}

	return rx_bufs_used;
}

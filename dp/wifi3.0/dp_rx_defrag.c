/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_rx.h"
#include "dp_peer.h"
#include "hal_api.h"
#include "qdf_trace.h"
#include "qdf_nbuf.h"
#include "dp_internal.h"
#include "dp_rx_defrag.h"
#include <enet.h>	/* LLC_SNAP_HDR_LEN */
#include "dp_rx_defrag.h"
#include "dp_ipa.h"

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
	if (peer->rx_tid[tid].dst_ring_desc)
		qdf_mem_free(peer->rx_tid[tid].dst_ring_desc);

	peer->rx_tid[tid].dst_ring_desc = NULL;
	peer->rx_tid[tid].head_frag_desc = NULL;
}

static void dp_rx_return_head_frag_desc(struct dp_peer *peer,
					unsigned int tid)
{
	struct dp_soc *soc;
	struct dp_pdev *pdev;
	struct dp_srng *dp_rxdma_srng;
	struct rx_desc_pool *rx_desc_pool;
	union dp_rx_desc_list_elem_t *head = NULL;
	union dp_rx_desc_list_elem_t *tail = NULL;

	pdev = peer->vdev->pdev;
	soc = pdev->soc;

	if (peer->rx_tid[tid].head_frag_desc) {
		dp_rxdma_srng = &pdev->rx_refill_buf_ring;
		rx_desc_pool = &soc->rx_desc_buf[pdev->pdev_id];

		dp_rx_add_to_free_desc_list(&head, &tail,
					    peer->rx_tid[tid].head_frag_desc);
		dp_rx_buffers_replenish(soc, 0, dp_rxdma_srng, rx_desc_pool,
					1, &head, &tail);
	}

	if (peer->rx_tid[tid].dst_ring_desc) {
		if (dp_rx_link_desc_return(soc,
					   peer->rx_tid[tid].dst_ring_desc,
					   HAL_BM_ACTION_PUT_IN_IDLE_LIST) !=
		    QDF_STATUS_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "%s: Failed to return link desc", __func__);
	}
}

/*
 * dp_rx_reorder_flush_frag(): Flush the frag list
 * @peer: Pointer to the peer data structure
 * @tid: Transmit ID (TID)
 *
 * Flush the per-TID frag list
 *
 * Returns: None
 */
void dp_rx_reorder_flush_frag(struct dp_peer *peer,
			 unsigned int tid)
{
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("Flushing TID %d"), tid);

	if (!peer) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"%s: NULL peer", __func__);
		return;
	}

	dp_rx_return_head_frag_desc(peer, tid);
	dp_rx_defrag_cleanup(peer, tid);
}

/*
 * dp_rx_defrag_waitlist_flush(): Flush SOC defrag wait list
 * @soc: DP SOC
 *
 * Flush fragments of all waitlisted TID's
 *
 * Returns: None
 */
void dp_rx_defrag_waitlist_flush(struct dp_soc *soc)
{
	struct dp_rx_tid *rx_reorder = NULL;
	struct dp_rx_tid *tmp;
	uint32_t now_ms = qdf_system_ticks_to_msecs(qdf_system_ticks());
	TAILQ_HEAD(, dp_rx_tid) temp_list;

	TAILQ_INIT(&temp_list);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  FL("Current time  %u"), now_ms);

	qdf_spin_lock_bh(&soc->rx.defrag.defrag_lock);
	TAILQ_FOREACH_SAFE(rx_reorder, &soc->rx.defrag.waitlist,
			   defrag_waitlist_elem, tmp) {
		uint32_t tid;

		if (rx_reorder->defrag_timeout_ms > now_ms)
			break;

		tid = rx_reorder->tid;
		if (tid >= DP_MAX_TIDS) {
			qdf_assert(0);
			continue;
		}

		TAILQ_REMOVE(&soc->rx.defrag.waitlist, rx_reorder,
			     defrag_waitlist_elem);
		DP_STATS_DEC(soc, rx.rx_frag_wait, 1);

		/* Move to temp list and clean-up later */
		TAILQ_INSERT_TAIL(&temp_list, rx_reorder,
				  defrag_waitlist_elem);
	}
	if (rx_reorder) {
		soc->rx.defrag.next_flush_ms =
			rx_reorder->defrag_timeout_ms;
	} else {
		soc->rx.defrag.next_flush_ms =
			now_ms + soc->rx.defrag.timeout_ms;
	}

	qdf_spin_unlock_bh(&soc->rx.defrag.defrag_lock);

	TAILQ_FOREACH_SAFE(rx_reorder, &temp_list,
			   defrag_waitlist_elem, tmp) {
		struct dp_peer *peer, *temp_peer = NULL;

		qdf_spin_lock_bh(&rx_reorder->tid_lock);
		TAILQ_REMOVE(&temp_list, rx_reorder,
			     defrag_waitlist_elem);
		/* get address of current peer */
		peer =
			container_of(rx_reorder, struct dp_peer,
				     rx_tid[rx_reorder->tid]);
		qdf_spin_unlock_bh(&rx_reorder->tid_lock);

		temp_peer = dp_peer_find_by_id(soc, peer->peer_ids[0]);
		if (temp_peer == peer) {
			qdf_spin_lock_bh(&rx_reorder->tid_lock);
			dp_rx_reorder_flush_frag(peer, rx_reorder->tid);
			qdf_spin_unlock_bh(&rx_reorder->tid_lock);
		}

		if (temp_peer)
			dp_peer_unref_del_find_by_id(temp_peer);

	}
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

	dp_debug("Adding TID %u to waitlist for peer %pK at MAC address %pM",
		 tid, peer, peer->mac_addr.raw);

	/* TODO: use LIST macros instead of TAIL macros */
	qdf_spin_lock_bh(&psoc->rx.defrag.defrag_lock);
	if (TAILQ_EMPTY(&psoc->rx.defrag.waitlist))
		psoc->rx.defrag.next_flush_ms = rx_reorder->defrag_timeout_ms;
	TAILQ_INSERT_TAIL(&psoc->rx.defrag.waitlist, rx_reorder,
				defrag_waitlist_elem);
	DP_STATS_INC(psoc, rx.rx_frag_wait, 1);
	qdf_spin_unlock_bh(&psoc->rx.defrag.defrag_lock);
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
void dp_rx_defrag_waitlist_remove(struct dp_peer *peer, unsigned tid)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_rx_tid *rx_reorder;
	struct dp_rx_tid *tmp;

	dp_debug("Removing TID %u to waitlist for peer %pK at MAC address %pM",
		 tid, peer, peer->mac_addr.raw);

	if (tid >= DP_MAX_TIDS) {
		dp_err("TID out of bounds: %d", tid);
		qdf_assert_always(0);
	}

	qdf_spin_lock_bh(&soc->rx.defrag.defrag_lock);
	TAILQ_FOREACH_SAFE(rx_reorder, &soc->rx.defrag.waitlist,
			   defrag_waitlist_elem, tmp) {
		struct dp_peer *peer_on_waitlist;

		/* get address of current peer */
		peer_on_waitlist =
			container_of(rx_reorder, struct dp_peer,
				     rx_tid[rx_reorder->tid]);

		/* Ensure it is TID for same peer */
		if (peer_on_waitlist == peer && rx_reorder->tid == tid) {
			TAILQ_REMOVE(&soc->rx.defrag.waitlist,
				rx_reorder, defrag_waitlist_elem);
			DP_STATS_DEC(soc, rx.rx_frag_wait, 1);
		}
	}
	qdf_spin_unlock_bh(&soc->rx.defrag.defrag_lock);
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
 * Returns: Success, if inserted
 */
static QDF_STATUS dp_rx_defrag_fraglist_insert(struct dp_peer *peer, unsigned tid,
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

	*all_frag_present = 0;
	rx_desc_info = qdf_nbuf_data(frag);
	cur_fragno = dp_rx_frag_get_mpdu_frag_number(rx_desc_info);

	/* If this is the first fragment */
	if (!(*head_addr)) {
		*head_addr = *tail_addr = frag;
		qdf_nbuf_set_next(*tail_addr, NULL);
		rx_tid->curr_frag_num = cur_fragno;

		goto insert_done;
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
			goto insert_fail;
		} else if (head_fragno > cur_fragno) {
			qdf_nbuf_set_next(frag, cur);
			cur = frag;
			*head_addr = frag; /* head pointer to be updated */
		} else {
			while ((cur_fragno > head_fragno) && cur) {
				prev = cur;
				cur = qdf_nbuf_next(cur);
				rx_desc_info = qdf_nbuf_data(cur);
				head_fragno =
					dp_rx_frag_get_mpdu_frag_number(
								rx_desc_info);
			}

			if (cur_fragno == head_fragno) {
				qdf_nbuf_free(frag);
				goto insert_fail;
			}

			qdf_nbuf_set_next(prev, frag);
			qdf_nbuf_set_next(frag, cur);
		}
	}

	next = qdf_nbuf_next(*head_addr);

	rx_desc_info = qdf_nbuf_data(*tail_addr);
	last_morefrag = dp_rx_frag_get_more_frag_bit(rx_desc_info);

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
			return QDF_STATUS_SUCCESS;
		}
	}

insert_done:
	return QDF_STATUS_SUCCESS;

insert_fail:
	return QDF_STATUS_E_FAILURE;
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

	/* Let's pull the header later */

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

	qdf_nbuf_trim_tail(msdu, dp_f_wep.ic_trailer);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_hdrsize(): Calculate the header size of the received fragment
 * @soc: soc handle
 * @nbuf: Pointer to the fragment
 *
 * Calculate the header size of the received fragment
 *
 * Returns: header size (uint16_t)
 */
static uint16_t dp_rx_defrag_hdrsize(struct dp_soc *soc, qdf_nbuf_t nbuf)
{
	uint8_t *rx_tlv_hdr = qdf_nbuf_data(nbuf);
	uint16_t size = sizeof(struct ieee80211_frame);
	uint16_t fc = 0;
	uint32_t to_ds, fr_ds;
	uint8_t frm_ctrl_valid;
	uint16_t frm_ctrl_field;

	to_ds = hal_rx_mpdu_get_to_ds(soc->hal_soc, rx_tlv_hdr);
	fr_ds = hal_rx_mpdu_get_fr_ds(soc->hal_soc, rx_tlv_hdr);
	frm_ctrl_valid =
		hal_rx_get_mpdu_frame_control_valid(soc->hal_soc,
						    rx_tlv_hdr);
	frm_ctrl_field = hal_rx_get_frame_ctrl_field(rx_tlv_hdr);

	if (to_ds && fr_ds)
		size += QDF_MAC_ADDR_SIZE;

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
 * dp_rx_defrag_michdr(): Calculate a pseudo MIC header
 * @wh0: Pointer to the wireless header of the fragment
 * @hdr: Array to hold the pseudo header
 *
 * Calculate a pseudo MIC header
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
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + QDF_MAC_ADDR_SIZE,
					   wh->i_addr2);
		break;
	case IEEE80211_FC1_DIR_TODS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr3); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + QDF_MAC_ADDR_SIZE,
					   wh->i_addr2);
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr1); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + QDF_MAC_ADDR_SIZE,
					   wh->i_addr3);
		break;
	case IEEE80211_FC1_DIR_DSTODS:
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr, wh->i_addr3); /* DA */
		DP_RX_DEFRAG_IEEE80211_ADDR_COPY(hdr + QDF_MAC_ADDR_SIZE,
					   wh->i_addr4);
		break;
	}

	/*
	 * Bit 7 is QDF_IEEE80211_FC0_SUBTYPE_QOS for data frame, but
	 * it could also be set for deauth, disassoc, action, etc. for
	 * a mgt type frame. It comes into picture for MFP.
	 */
	if (wh->i_fc[0] & QDF_IEEE80211_FC0_SUBTYPE_QOS) {
		if ((wh->i_fc[1] & IEEE80211_FC1_DIR_MASK) ==
				IEEE80211_FC1_DIR_DSTODS) {
			const struct ieee80211_qosframe_addr4 *qwh =
				(const struct ieee80211_qosframe_addr4 *)wh;
			hdr[12] = qwh->i_qos[0] & IEEE80211_QOS_TID;
		} else {
			const struct ieee80211_qosframe *qwh =
				(const struct ieee80211_qosframe *)wh;
			hdr[12] = qwh->i_qos[0] & IEEE80211_QOS_TID;
		}
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
 * @data_len: Data length
 * @mic: Array to hold MIC
 *
 * Calculate a pseudo MIC header
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
	data = (uint8_t *)qdf_nbuf_data(wbuf) + off;
	space = qdf_nbuf_len(wbuf) - off;

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
		if (!wbuf)
			return QDF_STATUS_E_DEFRAG_ERROR;

		if (space != 0) {
			const uint8_t *data_next;
			/*
			 * Block straddles buffers, split references.
			 */
			data_next =
				(uint8_t *)qdf_nbuf_data(wbuf) + off;
			if ((qdf_nbuf_len(wbuf)) <
				sizeof(uint32_t) - space) {
				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			switch (space) {
			case 1:
				l ^= dp_rx_get_le32_split(data[0],
					data_next[0], data_next[1],
					data_next[2]);
				data = data_next + 3;
				space = (qdf_nbuf_len(wbuf) - off) - 3;
				break;
			case 2:
				l ^= dp_rx_get_le32_split(data[0], data[1],
						    data_next[0], data_next[1]);
				data = data_next + 2;
				space = (qdf_nbuf_len(wbuf) - off) - 2;
				break;
			case 3:
				l ^= dp_rx_get_le32_split(data[0], data[1],
					data[2], data_next[0]);
				data = data_next + 1;
				space = (qdf_nbuf_len(wbuf) - off) - 1;
				break;
			}
			dp_rx_michael_block(l, r);
			data_len -= sizeof(uint32_t);
		} else {
			/*
			 * Setup for next buffer.
			 */
			data = (uint8_t *)qdf_nbuf_data(wbuf) + off;
			space = qdf_nbuf_len(wbuf) - off;
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
	uint32_t pktlen = 0;
	uint8_t mic[IEEE80211_WEP_MICLEN];
	uint8_t mic0[IEEE80211_WEP_MICLEN];
	qdf_nbuf_t prev = NULL, next;

	next = msdu;
	while (next) {
		pktlen += (qdf_nbuf_len(next) - hdrlen);
		prev = next;
		dp_debug("%s pktlen %u", __func__,
			 (uint32_t)(qdf_nbuf_len(next) - hdrlen));
		next = qdf_nbuf_next(next);
	}

	if (!prev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s Defrag chaining failed !\n", __func__);
		return QDF_STATUS_E_DEFRAG_ERROR;
	}

	qdf_nbuf_copy_bits(prev, qdf_nbuf_len(prev) - dp_f_tkip.ic_miclen,
			   dp_f_tkip.ic_miclen, (caddr_t)mic0);
	qdf_nbuf_trim_tail(prev, dp_f_tkip.ic_miclen);
	pktlen -= dp_f_tkip.ic_miclen;

	status = dp_rx_defrag_mic(key, msdu, hdrlen,
				pktlen, mic);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	if (qdf_mem_cmp(mic, mic0, dp_f_tkip.ic_miclen))
		return QDF_STATUS_E_DEFRAG_ERROR;

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_frag_pull_hdr(): Pulls the RXTLV & the 802.11 headers
 * @nbuf: buffer pointer
 * @hdrsize: size of the header to be pulled
 *
 * Pull the RXTLV & the 802.11 headers
 *
 * Returns: None
 */
static void dp_rx_frag_pull_hdr(qdf_nbuf_t nbuf, uint16_t hdrsize)
{
	qdf_nbuf_pull_head(nbuf,
			RX_PKT_TLVS_LEN + hdrsize);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s: final pktlen %d .11len %d",
		  __func__, (uint32_t)qdf_nbuf_len(nbuf), hdrsize);
}

/*
 * dp_rx_construct_fraglist(): Construct a nbuf fraglist
 * @peer: Pointer to the peer
 * @head: Pointer to list of fragments
 * @hdrsize: Size of the header to be pulled
 *
 * Construct a nbuf fraglist
 *
 * Returns: None
 */
static void
dp_rx_construct_fraglist(struct dp_peer *peer,
		qdf_nbuf_t head, uint16_t hdrsize)
{
	qdf_nbuf_t msdu = qdf_nbuf_next(head);
	qdf_nbuf_t rx_nbuf = msdu;
	uint32_t len = 0;

	while (msdu) {
		dp_rx_frag_pull_hdr(msdu, hdrsize);
		len += qdf_nbuf_len(msdu);
		msdu = qdf_nbuf_next(msdu);
	}

	qdf_nbuf_append_ext_list(head, rx_nbuf, len);
	qdf_nbuf_set_next(head, NULL);
	qdf_nbuf_set_is_frag(head, 1);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s: head len %d ext len %d data len %d ",
		  __func__,
		  (uint32_t)qdf_nbuf_len(head),
		  (uint32_t)qdf_nbuf_len(rx_nbuf),
		  (uint32_t)(head->data_len));
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
static void dp_rx_defrag_err(struct dp_vdev *vdev, qdf_nbuf_t nbuf)
{
	struct ol_if_ops *tops = NULL;
	struct dp_pdev *pdev = vdev->pdev;
	int rx_desc_len = sizeof(struct rx_pkt_tlvs);
	uint8_t *orig_hdr;
	struct ieee80211_frame *wh;
	struct cdp_rx_mic_err_info mic_failure_info;

	orig_hdr = (uint8_t *)(qdf_nbuf_data(nbuf) + rx_desc_len);
	wh = (struct ieee80211_frame *)orig_hdr;

	qdf_copy_macaddr((struct qdf_mac_addr *)&mic_failure_info.da_mac_addr,
			 (struct qdf_mac_addr *)&wh->i_addr1);
	qdf_copy_macaddr((struct qdf_mac_addr *)&mic_failure_info.ta_mac_addr,
			 (struct qdf_mac_addr *)&wh->i_addr2);
	mic_failure_info.key_id = 0;
	mic_failure_info.multicast =
		IEEE80211_IS_MULTICAST(wh->i_addr1);
	qdf_mem_zero(mic_failure_info.tsc, MIC_SEQ_CTR_SIZE);
	mic_failure_info.frame_type = cdp_rx_frame_type_802_11;
	mic_failure_info.data = (uint8_t *)wh;
	mic_failure_info.vdev_id = vdev->vdev_id;

	tops = pdev->soc->cdp_soc.ol_ops;
	if (tops->rx_mic_error)
		tops->rx_mic_error(pdev->soc->ctrl_psoc, pdev->pdev_id,
				   &mic_failure_info);
}


/*
 * dp_rx_defrag_nwifi_to_8023(): Transcap 802.11 to 802.3
 * @soc: dp soc handle
 * @nbuf: Pointer to the fragment buffer
 * @hdrsize: Size of headers
 *
 * Transcap the fragment from 802.11 to 802.3
 *
 * Returns: None
 */
static void
dp_rx_defrag_nwifi_to_8023(struct dp_soc *soc,
			   qdf_nbuf_t nbuf, uint16_t hdrsize)
{
	struct llc_snap_hdr_t *llchdr;
	struct ethernet_hdr_t *eth_hdr;
	uint8_t ether_type[2];
	uint16_t fc = 0;
	union dp_align_mac_addr mac_addr;
	uint8_t *rx_desc_info = qdf_mem_malloc(RX_PKT_TLVS_LEN);

	if (!rx_desc_info) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: Memory alloc failed ! ", __func__);
		QDF_ASSERT(0);
		return;
	}

	qdf_mem_copy(rx_desc_info, qdf_nbuf_data(nbuf), RX_PKT_TLVS_LEN);

	llchdr = (struct llc_snap_hdr_t *)(qdf_nbuf_data(nbuf) +
					RX_PKT_TLVS_LEN + hdrsize);
	qdf_mem_copy(ether_type, llchdr->ethertype, 2);

	qdf_nbuf_pull_head(nbuf, (RX_PKT_TLVS_LEN + hdrsize +
				  sizeof(struct llc_snap_hdr_t) -
				  sizeof(struct ethernet_hdr_t)));

	eth_hdr = (struct ethernet_hdr_t *)(qdf_nbuf_data(nbuf));

	if (hal_rx_get_mpdu_frame_control_valid(soc->hal_soc,
						rx_desc_info))
		fc = hal_rx_get_frame_ctrl_field(rx_desc_info);

	dp_debug("%s: frame control type: 0x%x", __func__, fc);

	switch (((fc & 0xff00) >> 8) & IEEE80211_FC1_DIR_MASK) {
	case IEEE80211_FC1_DIR_NODS:
		hal_rx_mpdu_get_addr1(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->dest_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		hal_rx_mpdu_get_addr2(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->src_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		break;
	case IEEE80211_FC1_DIR_TODS:
		hal_rx_mpdu_get_addr3(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->dest_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		hal_rx_mpdu_get_addr2(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->src_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		break;
	case IEEE80211_FC1_DIR_FROMDS:
		hal_rx_mpdu_get_addr1(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->dest_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		hal_rx_mpdu_get_addr3(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->src_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		break;

	case IEEE80211_FC1_DIR_DSTODS:
		hal_rx_mpdu_get_addr3(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->dest_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		hal_rx_mpdu_get_addr4(soc->hal_soc, rx_desc_info,
				      &mac_addr.raw[0]);
		qdf_mem_copy(eth_hdr->src_addr, &mac_addr.raw[0],
			QDF_MAC_ADDR_SIZE);
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"%s: Unknown frame control type: 0x%x", __func__, fc);
	}

	qdf_mem_copy(eth_hdr->ethertype, ether_type,
			sizeof(ether_type));

	qdf_nbuf_push_head(nbuf, RX_PKT_TLVS_LEN);
	qdf_mem_copy(qdf_nbuf_data(nbuf), rx_desc_info, RX_PKT_TLVS_LEN);
	qdf_mem_free(rx_desc_info);
}

/*
 * dp_rx_defrag_reo_reinject(): Reinject the fragment chain back into REO
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 * @head: Buffer to be reinjected back
 *
 * Reinject the fragment chain back into REO
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag_reo_reinject(struct dp_peer *peer,
					    unsigned int tid, qdf_nbuf_t head)
{
	struct dp_pdev *pdev = peer->vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct hal_buf_info buf_info;
	void *link_desc_va;
	void *msdu0, *msdu_desc_info;
	void *ent_ring_desc, *ent_mpdu_desc_info, *ent_qdesc_addr;
	void *dst_mpdu_desc_info, *dst_qdesc_addr;
	qdf_dma_addr_t paddr;
	uint32_t nbuf_len, seq_no, dst_ind;
	uint32_t *mpdu_wrd;
	uint32_t ret, cookie;
	hal_ring_desc_t dst_ring_desc =
		peer->rx_tid[tid].dst_ring_desc;
	hal_ring_handle_t hal_srng = soc->reo_reinject_ring.hal_srng;
	struct dp_rx_desc *rx_desc = peer->rx_tid[tid].head_frag_desc;

	head = dp_ipa_handle_rx_reo_reinject(soc, head);
	if (qdf_unlikely(!head)) {
		dp_err_rl("IPA RX REO reinject failed");
		return QDF_STATUS_E_FAILURE;
	}

	ent_ring_desc = hal_srng_src_get_next(soc->hal_soc, hal_srng);
	if (!ent_ring_desc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "HAL src ring next entry NULL");
		return QDF_STATUS_E_FAILURE;
	}

	hal_rx_reo_buf_paddr_get(dst_ring_desc, &buf_info);

	link_desc_va = dp_rx_cookie_2_link_desc_va(soc, &buf_info);

	qdf_assert(link_desc_va);

	msdu0 = hal_rx_msdu0_buffer_addr_lsb(soc->hal_soc, link_desc_va);
	nbuf_len = qdf_nbuf_len(head) - RX_PKT_TLVS_LEN;

	HAL_RX_UNIFORM_HDR_SET(link_desc_va, OWNER, UNI_DESC_OWNER_SW);
	HAL_RX_UNIFORM_HDR_SET(link_desc_va, BUFFER_TYPE,
			UNI_DESC_BUF_TYPE_RX_MSDU_LINK);

	/* msdu reconfig */
	msdu_desc_info = hal_rx_msdu_desc_info_ptr_get(soc->hal_soc, msdu0);

	dst_ind = hal_rx_msdu_reo_dst_ind_get(soc->hal_soc, link_desc_va);

	qdf_mem_zero(msdu_desc_info, sizeof(struct rx_msdu_desc_info));

	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			FIRST_MSDU_IN_MPDU_FLAG, 1);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			LAST_MSDU_IN_MPDU_FLAG, 1);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			MSDU_CONTINUATION, 0x0);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			REO_DESTINATION_INDICATION, dst_ind);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			MSDU_LENGTH, nbuf_len);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			SA_IS_VALID, 1);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
			DA_IS_VALID, 1);

	/* change RX TLV's */
	hal_rx_msdu_start_msdu_len_set(
			qdf_nbuf_data(head), nbuf_len);

	cookie = HAL_RX_BUF_COOKIE_GET(msdu0);

	/* map the nbuf before reinject it into HW */
	ret = qdf_nbuf_map_single(soc->osdev, head,
				  QDF_DMA_FROM_DEVICE);
	if (qdf_unlikely(ret == QDF_STATUS_E_FAILURE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: nbuf map failed !", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * As part of rx frag handler bufffer was unmapped and rx desc
	 * unmapped is set to 1. So again for defrag reinject frame reset
	 * it back to 0.
	 */
	rx_desc->unmapped = 0;

	dp_ipa_handle_rx_buf_smmu_mapping(soc, head, true);

	paddr = qdf_nbuf_get_frag_paddr(head, 0);

	ret = check_x86_paddr(soc, &head, &paddr, pdev);

	if (ret == QDF_STATUS_E_FAILURE) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: x86 check failed !", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	hal_rxdma_buff_addr_info_set(msdu0, paddr, cookie, DP_DEFRAG_RBM);

	/* Lets fill entrance ring now !!! */
	if (qdf_unlikely(hal_srng_access_start(soc->hal_soc, hal_srng))) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"HAL RING Access For REO entrance SRNG Failed: %pK",
		hal_srng);

		return QDF_STATUS_E_FAILURE;
	}

	paddr = (uint64_t)buf_info.paddr;
	/* buf addr */
	hal_rxdma_buff_addr_info_set(ent_ring_desc, paddr,
				     buf_info.sw_cookie,
				     HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST);
	/* mpdu desc info */
	ent_mpdu_desc_info = hal_ent_mpdu_desc_info(soc->hal_soc,
						    ent_ring_desc);
	dst_mpdu_desc_info = hal_dst_mpdu_desc_info(soc->hal_soc,
						    dst_ring_desc);

	qdf_mem_copy(ent_mpdu_desc_info, dst_mpdu_desc_info,
				sizeof(struct rx_mpdu_desc_info));
	qdf_mem_zero(ent_mpdu_desc_info, sizeof(uint32_t));

	mpdu_wrd = (uint32_t *)dst_mpdu_desc_info;
	seq_no = HAL_RX_MPDU_SEQUENCE_NUMBER_GET(mpdu_wrd);

	HAL_RX_MPDU_DESC_INFO_SET(ent_mpdu_desc_info,
			MSDU_COUNT, 0x1);
	HAL_RX_MPDU_DESC_INFO_SET(ent_mpdu_desc_info,
				  MPDU_SEQUENCE_NUMBER, seq_no);
	/* unset frag bit */
	HAL_RX_MPDU_DESC_INFO_SET(ent_mpdu_desc_info,
			FRAGMENT_FLAG, 0x0);
	/* set sa/da valid bits */
	HAL_RX_MPDU_DESC_INFO_SET(ent_mpdu_desc_info,
			SA_IS_VALID, 0x1);
	HAL_RX_MPDU_DESC_INFO_SET(ent_mpdu_desc_info,
			DA_IS_VALID, 0x1);
	HAL_RX_MPDU_DESC_INFO_SET(ent_mpdu_desc_info,
			RAW_MPDU, 0x0);

	/* qdesc addr */
	ent_qdesc_addr = (uint8_t *)ent_ring_desc +
		REO_ENTRANCE_RING_4_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET;

	dst_qdesc_addr = (uint8_t *)dst_ring_desc +
		REO_DESTINATION_RING_6_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET;

	qdf_mem_copy(ent_qdesc_addr, dst_qdesc_addr, 8);

	HAL_RX_FLD_SET(ent_ring_desc, REO_ENTRANCE_RING_5,
			REO_DESTINATION_INDICATION, dst_ind);

	hal_srng_access_end(soc->hal_soc, hal_srng);

	DP_STATS_INC(soc, rx.reo_reinject, 1);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s: reinjection done !", __func__);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag(): Defragment the fragment chain
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 * @frag_list_head: Pointer to head list
 * @frag_list_tail: Pointer to tail list
 *
 * Defragment the fragment chain
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS dp_rx_defrag(struct dp_peer *peer, unsigned tid,
			qdf_nbuf_t frag_list_head, qdf_nbuf_t frag_list_tail)
{
	qdf_nbuf_t tmp_next, prev;
	qdf_nbuf_t cur = frag_list_head, msdu;
	uint32_t index, tkip_demic = 0;
	uint16_t hdr_space;
	uint8_t key[DEFRAG_IEEE80211_KEY_LEN];
	struct dp_vdev *vdev = peer->vdev;
	struct dp_soc *soc = vdev->pdev->soc;
	uint8_t status = 0;

	hdr_space = dp_rx_defrag_hdrsize(soc, cur);
	index = hal_rx_msdu_is_wlan_mcast(cur) ?
		dp_sec_mcast : dp_sec_ucast;

	/* Remove FCS from all fragments */
	while (cur) {
		tmp_next = qdf_nbuf_next(cur);
		qdf_nbuf_set_next(cur, NULL);
		qdf_nbuf_trim_tail(cur, DEFRAG_IEEE80211_FCS_LEN);
		prev = cur;
		qdf_nbuf_set_next(cur, tmp_next);
		cur = tmp_next;
	}
	cur = frag_list_head;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: index %d Security type: %d", __func__,
		  index, peer->security[index].sec_type);

	switch (peer->security[index].sec_type) {
	case cdp_sec_type_tkip:
		tkip_demic = 1;

	case cdp_sec_type_tkip_nomic:
		while (cur) {
			tmp_next = qdf_nbuf_next(cur);
			if (dp_rx_defrag_tkip_decap(cur, hdr_space)) {

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: TKIP decap failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			cur = tmp_next;
		}

		/* If success, increment header to be stripped later */
		hdr_space += dp_f_tkip.ic_header;
		break;

	case cdp_sec_type_aes_ccmp:
		while (cur) {
			tmp_next = qdf_nbuf_next(cur);
			if (dp_rx_defrag_ccmp_demic(cur, hdr_space)) {

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: CCMP demic failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			if (dp_rx_defrag_ccmp_decap(cur, hdr_space)) {

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: CCMP decap failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			cur = tmp_next;
		}

		/* If success, increment header to be stripped later */
		hdr_space += dp_f_ccmp.ic_header;
		break;

	case cdp_sec_type_wep40:
	case cdp_sec_type_wep104:
	case cdp_sec_type_wep128:
		while (cur) {
			tmp_next = qdf_nbuf_next(cur);
			if (dp_rx_defrag_wep_decap(cur, hdr_space)) {

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"dp_rx_defrag: WEP decap failed");

				return QDF_STATUS_E_DEFRAG_ERROR;
			}
			cur = tmp_next;
		}

		/* If success, increment header to be stripped later */
		hdr_space += dp_f_wep.ic_header;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX,
			QDF_TRACE_LEVEL_ERROR,
			"dp_rx_defrag: Did not match any security type");
		break;
	}

	if (tkip_demic) {
		msdu = frag_list_head;
		qdf_mem_copy(key,
			     &peer->security[index].michael_key[0],
			     IEEE80211_WEP_MICLEN);
		status = dp_rx_defrag_tkip_demic(key, msdu,
						 RX_PKT_TLVS_LEN +
						 hdr_space);

		if (status) {
			dp_rx_defrag_err(vdev, frag_list_head);

			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: TKIP demic failed status %d",
				   __func__, status);

			return QDF_STATUS_E_DEFRAG_ERROR;
		}
	}

	/* Convert the header to 802.3 header */
	dp_rx_defrag_nwifi_to_8023(soc, frag_list_head, hdr_space);
	dp_rx_construct_fraglist(peer, frag_list_head, hdr_space);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_cleanup(): Clean up activities
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 *
 * Returns: None
 */
void dp_rx_defrag_cleanup(struct dp_peer *peer, unsigned tid)
{
	struct dp_rx_reorder_array_elem *rx_reorder_array_elem =
				peer->rx_tid[tid].array;

	if (rx_reorder_array_elem) {
		/* Free up nbufs */
		dp_rx_defrag_frames_free(rx_reorder_array_elem->head);
		rx_reorder_array_elem->head = NULL;
		rx_reorder_array_elem->tail = NULL;
	} else {
		dp_info("Cleanup self peer %pK and TID %u at MAC address %pM",
			peer, tid, peer->mac_addr.raw);
	}

	/* Free up saved ring descriptors */
	dp_rx_clear_saved_desc_info(peer, tid);

	peer->rx_tid[tid].defrag_timeout_ms = 0;
	peer->rx_tid[tid].curr_frag_num = 0;
	peer->rx_tid[tid].curr_seq_num = 0;
}

/*
 * dp_rx_defrag_save_info_from_ring_desc(): Save info from REO ring descriptor
 * @ring_desc: Pointer to the dst ring descriptor
 * @peer: Pointer to the peer
 * @tid: Transmit Identifier
 *
 * Returns: None
 */
static QDF_STATUS
dp_rx_defrag_save_info_from_ring_desc(hal_ring_desc_t ring_desc,
				      struct dp_rx_desc *rx_desc,
				      struct dp_peer *peer,
				      unsigned int tid)
{
	void *dst_ring_desc = qdf_mem_malloc(
			sizeof(struct reo_destination_ring));

	if (!dst_ring_desc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: Memory alloc failed !", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(dst_ring_desc, ring_desc,
		       sizeof(struct reo_destination_ring));

	peer->rx_tid[tid].dst_ring_desc = dst_ring_desc;
	peer->rx_tid[tid].head_frag_desc = rx_desc;

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_defrag_store_fragment(): Store incoming fragments
 * @soc: Pointer to the SOC data structure
 * @ring_desc: Pointer to the ring descriptor
 * @mpdu_desc_info: MPDU descriptor info
 * @tid: Traffic Identifier
 * @rx_desc: Pointer to rx descriptor
 * @rx_bfs: Number of bfs consumed
 *
 * Returns: QDF_STATUS
 */
static QDF_STATUS
dp_rx_defrag_store_fragment(struct dp_soc *soc,
			    hal_ring_desc_t ring_desc,
			    union dp_rx_desc_list_elem_t **head,
			    union dp_rx_desc_list_elem_t **tail,
			    struct hal_rx_mpdu_desc_info *mpdu_desc_info,
			    unsigned int tid, struct dp_rx_desc *rx_desc,
			    uint32_t *rx_bfs)
{
	struct dp_rx_reorder_array_elem *rx_reorder_array_elem;
	struct dp_pdev *pdev;
	struct dp_peer *peer = NULL;
	uint16_t peer_id;
	uint8_t fragno, more_frag, all_frag_present = 0;
	uint16_t rxseq = mpdu_desc_info->mpdu_seq;
	QDF_STATUS status;
	struct dp_rx_tid *rx_tid;
	uint8_t mpdu_sequence_control_valid;
	uint8_t mpdu_frame_control_valid;
	qdf_nbuf_t frag = rx_desc->nbuf;
	uint32_t msdu_len;

	if (qdf_nbuf_len(frag) > 0) {
		dp_info("Dropping unexpected packet with skb_len: %d,"
			"data len: %d, cookie: %d",
			(uint32_t)qdf_nbuf_len(frag), frag->data_len,
			rx_desc->cookie);
		DP_STATS_INC(soc, rx.rx_frag_err_len_error, 1);
		goto discard_frag;
	}

	msdu_len = hal_rx_msdu_start_msdu_len_get(rx_desc->rx_buf_start);

	qdf_nbuf_set_pktlen(frag, (msdu_len + RX_PKT_TLVS_LEN));
	qdf_nbuf_append_ext_list(frag, NULL, 0);

	/* Check if the packet is from a valid peer */
	peer_id = DP_PEER_METADATA_PEER_ID_GET(
					mpdu_desc_info->peer_meta_data);
	peer = dp_peer_find_by_id(soc, peer_id);

	if (!peer) {
		/* We should not receive anything from unknown peer
		 * however, that might happen while we are in the monitor mode.
		 * We don't need to handle that here
		 */
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Unknown peer, dropping the fragment");

		goto discard_frag;
	}

	if (tid >= DP_MAX_TIDS) {
		dp_info("TID out of bounds: %d", tid);
		qdf_assert_always(0);
	}

	pdev = peer->vdev->pdev;
	rx_tid = &peer->rx_tid[tid];

	mpdu_sequence_control_valid =
		hal_rx_get_mpdu_sequence_control_valid(soc->hal_soc,
						       rx_desc->rx_buf_start);

	/* Invalid MPDU sequence control field, MPDU is of no use */
	if (!mpdu_sequence_control_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid MPDU seq control field, dropping MPDU");

		qdf_assert(0);
		goto discard_frag;
	}

	mpdu_frame_control_valid =
		hal_rx_get_mpdu_frame_control_valid(soc->hal_soc,
						    rx_desc->rx_buf_start);

	/* Invalid frame control field */
	if (!mpdu_frame_control_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Invalid frame control field, dropping MPDU");

		qdf_assert(0);
		goto discard_frag;
	}

	/* Current mpdu sequence */
	more_frag = dp_rx_frag_get_more_frag_bit(rx_desc->rx_buf_start);

	/* HW does not populate the fragment number as of now
	 * need to get from the 802.11 header
	 */
	fragno = dp_rx_frag_get_mpdu_frag_number(rx_desc->rx_buf_start);

	rx_reorder_array_elem = peer->rx_tid[tid].array;
	if (!rx_reorder_array_elem) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Rcvd Fragmented pkt before peer_tid is setup");
		goto discard_frag;
	}

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

		qdf_assert(0);
		goto discard_frag;
	}

	/* Check if the fragment is for the same sequence or a different one */
	if (rx_reorder_array_elem->head) {
		if (rxseq != rx_tid->curr_seq_num) {

			/* Drop stored fragments if out of sequence
			 * fragment is received
			 */
			dp_rx_reorder_flush_frag(peer, tid);

			DP_STATS_INC(soc, rx.rx_frag_err, 1);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s mismatch, dropping earlier sequence ",
				(rxseq == rx_tid->curr_seq_num)
				? "address"
				: "seq number");

			/*
			 * The sequence number for this fragment becomes the
			 * new sequence number to be processed
			 */
			rx_tid->curr_seq_num = rxseq;
		}
	} else {
		/* Start of a new sequence */
		dp_rx_defrag_cleanup(peer, tid);
		rx_tid->curr_seq_num = rxseq;
	}

	/*
	 * If the earlier sequence was dropped, this will be the fresh start.
	 * Else, continue with next fragment in a given sequence
	 */
	status = dp_rx_defrag_fraglist_insert(peer, tid, &rx_reorder_array_elem->head,
			&rx_reorder_array_elem->tail, frag,
			&all_frag_present);

	/*
	 * Currently, we can have only 6 MSDUs per-MPDU, if the current
	 * packet sequence has more than 6 MSDUs for some reason, we will
	 * have to use the next MSDU link descriptor and chain them together
	 * before reinjection
	 */
	if ((fragno == 0) && (status == QDF_STATUS_SUCCESS) &&
			(rx_reorder_array_elem->head == frag)) {

		qdf_assert_always(ring_desc);
		status = dp_rx_defrag_save_info_from_ring_desc(ring_desc,
					rx_desc, peer, tid);

		if (status != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"%s: Unable to store ring desc !", __func__);
			goto discard_frag;
		}
	} else {
		dp_rx_add_to_free_desc_list(head, tail, rx_desc);
		*rx_bfs = 1;

		/* Return the non-head link desc */
		if (ring_desc &&
		    dp_rx_link_desc_return(soc, ring_desc,
					   HAL_BM_ACTION_PUT_IN_IDLE_LIST) !=
		    QDF_STATUS_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				  "%s: Failed to return link desc", __func__);

	}

	if (pdev->soc->rx.flags.defrag_timeout_check)
		dp_rx_defrag_waitlist_remove(peer, tid);

	/* Yet to receive more fragments for this sequence number */
	if (!all_frag_present) {
		uint32_t now_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks());

		peer->rx_tid[tid].defrag_timeout_ms =
			now_ms + pdev->soc->rx.defrag.timeout_ms;

		dp_rx_defrag_waitlist_add(peer, tid);
		dp_peer_unref_del_find_by_id(peer);

		return QDF_STATUS_SUCCESS;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "All fragments received for sequence: %d", rxseq);

	/* Process the fragments */
	status = dp_rx_defrag(peer, tid, rx_reorder_array_elem->head,
		rx_reorder_array_elem->tail);
	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Fragment processing failed");

		dp_rx_add_to_free_desc_list(head, tail,
				peer->rx_tid[tid].head_frag_desc);
		*rx_bfs = 1;

		if (dp_rx_link_desc_return(soc,
					peer->rx_tid[tid].dst_ring_desc,
					HAL_BM_ACTION_PUT_IN_IDLE_LIST) !=
				QDF_STATUS_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
					"%s: Failed to return link desc",
					__func__);
		dp_rx_defrag_cleanup(peer, tid);
		goto end;
	}

	/* Re-inject the fragments back to REO for further processing */
	status = dp_rx_defrag_reo_reinject(peer, tid,
			rx_reorder_array_elem->head);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		rx_reorder_array_elem->head = NULL;
		rx_reorder_array_elem->tail = NULL;
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "Fragmented sequence successfully reinjected");
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		"Fragmented sequence reinjection failed");
		dp_rx_return_head_frag_desc(peer, tid);
	}

	dp_rx_defrag_cleanup(peer, tid);

	dp_peer_unref_del_find_by_id(peer);

	return QDF_STATUS_SUCCESS;

discard_frag:
	qdf_nbuf_free(frag);
	dp_rx_add_to_free_desc_list(head, tail, rx_desc);
	if (dp_rx_link_desc_return(soc, ring_desc,
				   HAL_BM_ACTION_PUT_IN_IDLE_LIST) !=
	    QDF_STATUS_SUCCESS)
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to return link desc", __func__);
	*rx_bfs = 1;

end:
	if (peer)
		dp_peer_unref_del_find_by_id(peer);

	DP_STATS_INC(soc, rx.rx_frag_err, 1);
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
uint32_t dp_rx_frag_handle(struct dp_soc *soc, hal_ring_desc_t ring_desc,
			   struct hal_rx_mpdu_desc_info *mpdu_desc_info,
			   struct dp_rx_desc *rx_desc,
			   uint8_t *mac_id,
			   uint32_t quota)
{
	uint32_t rx_bufs_used = 0;
	qdf_nbuf_t msdu = NULL;
	uint32_t tid;
	int rx_bfs = 0;
	struct dp_pdev *pdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	qdf_assert(soc);
	qdf_assert(mpdu_desc_info);
	qdf_assert(rx_desc);

	dp_debug("Number of MSDUs to process, num_msdus: %d",
		 mpdu_desc_info->msdu_count);


	if (qdf_unlikely(mpdu_desc_info->msdu_count == 0)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"Not sufficient MSDUs to process");
		return rx_bufs_used;
	}

	/* all buffers in MSDU link belong to same pdev */
	pdev = soc->pdev_list[rx_desc->pool_id];
	*mac_id = rx_desc->pool_id;

	msdu = rx_desc->nbuf;

	qdf_nbuf_unmap_single(soc->osdev, msdu,	QDF_DMA_FROM_DEVICE);
	rx_desc->unmapped = 1;

	rx_desc->rx_buf_start = qdf_nbuf_data(msdu);

	tid = hal_rx_mpdu_start_tid_get(soc->hal_soc, rx_desc->rx_buf_start);

	/* Process fragment-by-fragment */
	status = dp_rx_defrag_store_fragment(soc, ring_desc,
					     &pdev->free_list_head,
					     &pdev->free_list_tail,
					     mpdu_desc_info,
					     tid, rx_desc, &rx_bfs);

	if (rx_bfs)
		rx_bufs_used++;

	if (!QDF_IS_STATUS_SUCCESS(status))
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Rx Defrag err seq#:0x%x msdu_count:%d flags:%d",
			  mpdu_desc_info->mpdu_seq,
			  mpdu_desc_info->msdu_count,
			  mpdu_desc_info->mpdu_flags);

	return rx_bufs_used;
}

QDF_STATUS dp_rx_defrag_add_last_frag(struct dp_soc *soc,
				      struct dp_peer *peer, uint16_t tid,
				      uint16_t rxseq, qdf_nbuf_t nbuf)
{
	struct dp_rx_tid *rx_tid = &peer->rx_tid[tid];
	struct dp_rx_reorder_array_elem *rx_reorder_array_elem;
	uint8_t all_frag_present;
	uint32_t msdu_len;
	QDF_STATUS status;

	rx_reorder_array_elem = peer->rx_tid[tid].array;

	/*
	 * HW may fill in unexpected peer_id in RX PKT TLV,
	 * if this peer_id related peer is valid by coincidence,
	 * but actually this peer won't do dp_peer_rx_init(like SAP vdev
	 * self peer), then invalid access to rx_reorder_array_elem happened.
	 */
	if (!rx_reorder_array_elem) {
		dp_verbose_debug(
			"peer id:%d mac:" QDF_MAC_ADDR_STR "drop rx frame!",
			peer->peer_ids[0],
			QDF_MAC_ADDR_ARRAY(peer->mac_addr.raw));
		DP_STATS_INC(soc, rx.err.defrag_peer_uninit, 1);
		qdf_nbuf_free(nbuf);
		goto fail;
	}

	if (rx_reorder_array_elem->head &&
	    rxseq != rx_tid->curr_seq_num) {
		/* Drop stored fragments if out of sequence
		 * fragment is received
		 */
		dp_rx_reorder_flush_frag(peer, tid);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: No list found for TID %d Seq# %d",
				__func__, tid, rxseq);
		qdf_nbuf_free(nbuf);
		goto fail;
	}

	msdu_len = hal_rx_msdu_start_msdu_len_get(qdf_nbuf_data(nbuf));

	qdf_nbuf_set_pktlen(nbuf, (msdu_len + RX_PKT_TLVS_LEN));

	status = dp_rx_defrag_fraglist_insert(peer, tid,
					      &rx_reorder_array_elem->head,
			&rx_reorder_array_elem->tail, nbuf,
			&all_frag_present);

	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s Fragment insert failed", __func__);

		goto fail;
	}

	if (soc->rx.flags.defrag_timeout_check)
		dp_rx_defrag_waitlist_remove(peer, tid);

	if (!all_frag_present) {
		uint32_t now_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks());

		peer->rx_tid[tid].defrag_timeout_ms =
			now_ms + soc->rx.defrag.timeout_ms;

		dp_rx_defrag_waitlist_add(peer, tid);

		return QDF_STATUS_SUCCESS;
	}

	status = dp_rx_defrag(peer, tid, rx_reorder_array_elem->head,
			      rx_reorder_array_elem->tail);

	if (QDF_IS_STATUS_ERROR(status)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s Fragment processing failed", __func__);

		dp_rx_return_head_frag_desc(peer, tid);
		dp_rx_defrag_cleanup(peer, tid);

		goto fail;
	}

	/* Re-inject the fragments back to REO for further processing */
	status = dp_rx_defrag_reo_reinject(peer, tid,
					   rx_reorder_array_elem->head);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		rx_reorder_array_elem->head = NULL;
		rx_reorder_array_elem->tail = NULL;
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
			  "%s: Frag seq successfully reinjected",
			__func__);
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Frag seq reinjection failed", __func__);
		dp_rx_return_head_frag_desc(peer, tid);
	}

	dp_rx_defrag_cleanup(peer, tid);
	return QDF_STATUS_SUCCESS;

fail:
	return QDF_STATUS_E_DEFRAG_ERROR;
}

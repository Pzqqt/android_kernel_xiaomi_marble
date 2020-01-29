/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Implement various notification handlers which are accessed
 * internally in pkt_capture component only.
 */

#include <wlan_pkt_capture_data_txrx.h>
#include <wlan_pkt_capture_main.h>
#include <enet.h>
#include <htt_internal.h>

/**
 * pkt_capture_rx_convert8023to80211() - convert 802.3 packet to 802.11
 * format from rx desc
 * @bssid: bssid
 * @msdu: netbuf
 * @desc: rx desc
 *
 * Return: none
 */
static void
pkt_capture_rx_convert8023to80211(uint8_t *bssid, qdf_nbuf_t msdu, void *desc)
{
	struct ethernet_hdr_t *eth_hdr;
	struct llc_snap_hdr_t *llc_hdr;
	struct ieee80211_frame *wh;
	uint8_t hdsize, new_hdsize;
	struct ieee80211_qoscntl *qos_cntl;
	uint16_t seq_no;
	uint8_t localbuf[sizeof(struct ieee80211_qosframe_htc_addr4) +
			sizeof(struct llc_snap_hdr_t)];
	const uint8_t ethernet_II_llc_snap_header_prefix[] = {
					0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };
	uint16_t ether_type;

	struct htt_host_rx_desc_base *rx_desc = desc;

	eth_hdr = (struct ethernet_hdr_t *)qdf_nbuf_data(msdu);
	hdsize = sizeof(struct ethernet_hdr_t);
	wh = (struct ieee80211_frame *)localbuf;

	wh->i_fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_DATA;
	*(uint16_t *)wh->i_dur = 0;

	new_hdsize = 0;

	/* DA , BSSID , SA */
	qdf_mem_copy(wh->i_addr1, eth_hdr->dest_addr,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(wh->i_addr2, bssid,
		     QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(wh->i_addr3, eth_hdr->src_addr,
		     QDF_MAC_ADDR_SIZE);

	wh->i_fc[1] = IEEE80211_FC1_DIR_FROMDS;

	if (rx_desc->attention.more_data)
		wh->i_fc[1] |= IEEE80211_FC1_MORE_DATA;

	if (rx_desc->attention.power_mgmt)
		wh->i_fc[1] |= IEEE80211_FC1_PWR_MGT;

	if (rx_desc->attention.fragment)
		wh->i_fc[1] |= IEEE80211_FC1_MORE_FRAG;

	if (rx_desc->attention.order)
		wh->i_fc[1] |= IEEE80211_FC1_ORDER;

	if (rx_desc->mpdu_start.retry)
		wh->i_fc[1] |= IEEE80211_FC1_RETRY;

	seq_no = rx_desc->mpdu_start.seq_num;
	seq_no = (seq_no << IEEE80211_SEQ_SEQ_SHIFT) & IEEE80211_SEQ_SEQ_MASK;
	qdf_mem_copy(wh->i_seq, &seq_no, sizeof(seq_no));

	new_hdsize = sizeof(struct ieee80211_frame);

	if (rx_desc->attention.non_qos == 0) {
		qos_cntl =
		(struct ieee80211_qoscntl *)(localbuf + new_hdsize);
		qos_cntl->i_qos[0] =
		(rx_desc->mpdu_start.tid & IEEE80211_QOS_TID);
		wh->i_fc[0] |= QDF_IEEE80211_FC0_SUBTYPE_QOS;

		qos_cntl->i_qos[1] = 0;
		new_hdsize += sizeof(struct ieee80211_qoscntl);
	}

	/*
	 * Prepare llc Header
	 */
	llc_hdr = (struct llc_snap_hdr_t *)(localbuf + new_hdsize);
	ether_type = (eth_hdr->ethertype[0] << 8) |
			(eth_hdr->ethertype[1]);
	if (ether_type >= ETH_P_802_3_MIN) {
		qdf_mem_copy(llc_hdr,
			     ethernet_II_llc_snap_header_prefix,
			     sizeof
			     (ethernet_II_llc_snap_header_prefix));
		if (ether_type == ETHERTYPE_AARP ||
		    ether_type == ETHERTYPE_IPX) {
			llc_hdr->org_code[2] =
				BTEP_SNAP_ORGCODE_2;
			/* 0xf8; bridge tunnel header */
		}
		llc_hdr->ethertype[0] = eth_hdr->ethertype[0];
		llc_hdr->ethertype[1] = eth_hdr->ethertype[1];
		new_hdsize += sizeof(struct llc_snap_hdr_t);
	}

	/*
	 * Remove 802.3 Header by adjusting the head
	 */
	qdf_nbuf_pull_head(msdu, hdsize);

	/*
	 * Adjust the head and prepare 802.11 Header
	 */
	qdf_nbuf_push_head(msdu, new_hdsize);
	qdf_mem_copy(qdf_nbuf_data(msdu), localbuf, new_hdsize);
}

void pkt_capture_rx_in_order_drop_offload_pkt(qdf_nbuf_t head_msdu)
{
	while (head_msdu) {
		qdf_nbuf_t msdu = head_msdu;

		head_msdu = qdf_nbuf_next(head_msdu);
		qdf_nbuf_free(msdu);
	}
}

bool pkt_capture_rx_in_order_offloaded_pkt(qdf_nbuf_t rx_ind_msg)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *)qdf_nbuf_data(rx_ind_msg);

	/* check if it is for offloaded data pkt */
	return HTT_RX_IN_ORD_PADDR_IND_PKT_CAPTURE_MODE_IS_MONITOR_SET
					(*(msg_word + 1));
}

void pkt_capture_msdu_process_pkts(
				uint8_t *bssid,
				qdf_nbuf_t head_msdu,
				uint8_t vdev_id, htt_pdev_handle pdev)
{
	qdf_nbuf_t loop_msdu, pktcapture_msdu;
	qdf_nbuf_t msdu, prev = NULL;

	pktcapture_msdu = NULL;
	loop_msdu = head_msdu;
	while (loop_msdu) {
		msdu = qdf_nbuf_copy(loop_msdu);

		if (msdu) {
			qdf_nbuf_push_head(msdu,
					   HTT_RX_STD_DESC_RESERVATION);
			qdf_nbuf_set_next(msdu, NULL);

			if (!(pktcapture_msdu)) {
				pktcapture_msdu = msdu;
				prev = msdu;
			} else {
				qdf_nbuf_set_next(prev, msdu);
				prev = msdu;
			}
		}
		loop_msdu = qdf_nbuf_next(loop_msdu);
	}

	if (!pktcapture_msdu)
		return;

	pkt_capture_datapkt_process(
			vdev_id, pktcapture_msdu,
			TXRX_PROCESS_TYPE_DATA_RX, 0, 0,
			TXRX_PKTCAPTURE_PKT_FORMAT_8023,
			bssid, pdev);
}

/**
 * pkt_capture_rx_data_cb(): callback to process data rx packets
 * for pkt capture mode. (normal rx + offloaded rx)
 * @context: objmgr vdev
 * @ppdev: device handler
 * @nbuf_list: netbuf list
 * @vdev_id: vdev id for which packet is captured
 * @tid:  tid number
 * @status: Tx status
 * @pkt_format: Frame format
 * @bssid: bssid
 *
 * Return: none
 */
static void
pkt_capture_rx_data_cb(
		void *context, void *ppdev, void *nbuf_list,
		uint8_t vdev_id, uint8_t tid,
		uint8_t status, bool pkt_format,
		uint8_t *bssid)
{
	struct pkt_capture_vdev_priv *vdev_priv;
	qdf_nbuf_t buf_list = (qdf_nbuf_t)nbuf_list;
	struct wlan_objmgr_vdev *vdev = context;
	htt_pdev_handle pdev = ppdev;
	struct pkt_capture_cb_context *cb_ctx;
	qdf_nbuf_t msdu, next_buf;
	uint8_t drop_count;
	struct htt_host_rx_desc_base *rx_desc;
	struct mon_rx_status rx_status = {0};
	uint32_t headroom;
	static uint8_t preamble_type;
	static uint32_t vht_sig_a_1;
	static uint32_t vht_sig_a_2;

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (qdf_unlikely(!vdev))
		goto free_buf;

	cb_ctx = vdev_priv->cb_ctx;
	if (!cb_ctx || !cb_ctx->mon_cb || !cb_ctx->mon_ctx)
		goto free_buf;

	msdu = buf_list;
	while (msdu) {
		struct ethernet_hdr_t *eth_hdr;

		next_buf = qdf_nbuf_queue_next(msdu);
		qdf_nbuf_set_next(msdu, NULL);   /* Add NULL terminator */

		rx_desc = htt_rx_desc(msdu);

		/*
		 * Only the first mpdu has valid preamble type, so use it
		 * till the last mpdu is reached
		 */
		if (rx_desc->attention.first_mpdu) {
			preamble_type = rx_desc->ppdu_start.preamble_type;
			if (preamble_type == 8 || preamble_type == 9 ||
			    preamble_type == 0x0c || preamble_type == 0x0d) {
				vht_sig_a_1 = VHT_SIG_A_1(rx_desc);
				vht_sig_a_2 = VHT_SIG_A_2(rx_desc);
			}
		} else {
			rx_desc->ppdu_start.preamble_type = preamble_type;
			if (preamble_type == 8 || preamble_type == 9 ||
			    preamble_type == 0x0c || preamble_type == 0x0d) {
				VHT_SIG_A_1(rx_desc) = vht_sig_a_1;
				VHT_SIG_A_2(rx_desc) = vht_sig_a_2;
			}
		}

		if (rx_desc->attention.last_mpdu) {
			preamble_type = 0;
			vht_sig_a_1 = 0;
			vht_sig_a_2 = 0;
		}

		qdf_nbuf_pull_head(msdu, HTT_RX_STD_DESC_RESERVATION);

		/*
		 * Get the channel info and update the rx status
		 */

		/* need to update this to fill rx_status*/
		htt_rx_mon_get_rx_status(pdev, rx_desc, &rx_status);
		rx_status.chan_noise_floor = NORMALIZED_TO_NOISE_FLOOR;

		/* clear IEEE80211_RADIOTAP_F_FCS flag*/
		rx_status.rtap_flags &= ~(BIT(4));
		rx_status.rtap_flags &= ~(BIT(2));

		/*
		 * convert 802.3 header format into 802.11 format
		 */
		if (vdev_id == HTT_INVALID_VDEV) {
			eth_hdr = (struct ethernet_hdr_t *)qdf_nbuf_data(msdu);
			qdf_mem_copy(bssid, eth_hdr->src_addr,
				     QDF_MAC_ADDR_SIZE);
		}

		pkt_capture_rx_convert8023to80211(bssid, msdu, rx_desc);

		/*
		 * Calculate the headroom and adjust head
		 * to prepare radiotap header.
		 */
		headroom = qdf_nbuf_headroom(msdu);
		qdf_nbuf_update_radiotap(&rx_status, msdu, headroom);

		if (QDF_STATUS_SUCCESS !=
		    cb_ctx->mon_cb(cb_ctx->mon_ctx, msdu)) {
			pkt_capture_err("Frame Rx to HDD failed");
			qdf_nbuf_free(msdu);
		}
		msdu = next_buf;
	}

	return;

free_buf:
	drop_count = pkt_capture_drop_nbuf_list(buf_list);
}

void pkt_capture_datapkt_process(
		uint8_t vdev_id,
		qdf_nbuf_t mon_buf_list,
		enum pkt_capture_data_process_type type,
		uint8_t tid, uint8_t status, bool pkt_format,
		uint8_t *bssid, htt_pdev_handle pdev)
{
	uint8_t drop_count;
	struct pkt_capture_mon_pkt *pkt;
	pkt_capture_mon_thread_cb callback = NULL;
	struct wlan_objmgr_vdev *vdev;

	vdev = pkt_capture_get_vdev();
	if (!vdev)
		goto drop_rx_buf;

	pkt = pkt_capture_alloc_mon_pkt(vdev);
	if (!pkt)
		goto drop_rx_buf;

	switch (type) {
	case TXRX_PROCESS_TYPE_DATA_RX:
		callback = pkt_capture_rx_data_cb;
		break;
	case TXRX_PROCESS_TYPE_DATA_TX:
		break;
	case TXRX_PROCESS_TYPE_DATA_TX_COMPL:
		break;
	default:
		return;
	}

	pkt->callback = callback;
	pkt->context = (void *)vdev;
	pkt->pdev = (void *)pdev;
	pkt->monpkt = (void *)mon_buf_list;
	pkt->vdev_id = vdev_id;
	pkt->tid = tid;
	pkt->status = status;
	pkt->pkt_format = pkt_format;
	qdf_mem_copy(pkt->bssid, bssid, QDF_MAC_ADDR_SIZE);
	pkt_capture_indicate_monpkt(vdev, pkt);
	return;

drop_rx_buf:
	drop_count = pkt_capture_drop_nbuf_list(mon_buf_list);
}

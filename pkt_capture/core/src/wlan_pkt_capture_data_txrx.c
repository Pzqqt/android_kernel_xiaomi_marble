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
#include <wlan_reg_services_api.h>
#include <cds_ieee80211_common.h>
#include <ol_txrx_htt_api.h>

#define RESERVE_BYTES (100)

/**
 * pkt_capture_txrx_status_map() - map Tx status for data packets
 * with packet capture Tx status
 * @status: Tx status
 *
 * Return: pkt_capture_tx_status enum
 */
static enum pkt_capture_tx_status
pkt_capture_txrx_status_map(uint8_t status)
{
	enum pkt_capture_tx_status tx_status;

	switch (status) {
	case htt_tx_status_ok:
		tx_status = pkt_capture_tx_status_ok;
		break;
	case htt_tx_status_no_ack:
		tx_status = pkt_capture_tx_status_no_ack;
		break;
	default:
		tx_status = pkt_capture_tx_status_discard;
		break;
	}

	return tx_status;
}

/**
 * pkt_capture_get_tx_rate() - get tx rate for tx packet
 * @preamble_type: preamble type
 * @rate: rate code
 * @preamble: preamble
 *
 * Return: rate
 */
static unsigned char pkt_capture_get_tx_rate(
					uint8_t preamble_type,
					uint8_t rate,
					uint8_t *preamble)
{
	char ret = 0x0;
	*preamble = LONG_PREAMBLE;

	if (preamble_type == 0) {
		switch (rate) {
		case 0x0:
			ret = 0x60;
			break;
		case 0x1:
			ret = 0x30;
			break;
		case 0x2:
			ret = 0x18;
			break;
		case 0x3:
			ret = 0x0c;
			break;
		case 0x4:
			ret = 0x6c;
			break;
		case 0x5:
			ret = 0x48;
			break;
		case 0x6:
			ret = 0x24;
			break;
		case 0x7:
			ret = 0x12;
			break;
		default:
			break;
		}
	} else if (preamble_type == 1) {
		switch (rate) {
		case 0x0:
			ret = 0x16;
			*preamble = LONG_PREAMBLE;
		case 0x1:
			ret = 0xB;
			*preamble = LONG_PREAMBLE;
			break;
		case 0x2:
			ret = 0x4;
			*preamble = LONG_PREAMBLE;
			break;
		case 0x3:
			ret = 0x2;
			*preamble = LONG_PREAMBLE;
			break;
		case 0x4:
			ret = 0x16;
			*preamble = SHORT_PREAMBLE;
			break;
		case 0x5:
			ret = 0xB;
			*preamble = SHORT_PREAMBLE;
			break;
		case 0x6:
			ret = 0x4;
			*preamble = SHORT_PREAMBLE;
			break;
		default:
			break;
		}
	} else {
		qdf_print("Invalid rate info\n");
	}
	return ret;
}

/**
 * pkt_capture_tx_get_phy_info() - get phy info for tx packets for pkt
 * capture mode(normal tx + offloaded tx) to prepare radiotap header
 * @pktcapture_hdr: tx data header
 * @tx_status: tx status to be updated with phy info
 *
 * Return: none
 */
static void pkt_capture_tx_get_phy_info(
		struct pkt_capture_tx_hdr_elem_t *pktcapture_hdr,
		struct mon_rx_status *tx_status)
{
	uint8_t preamble = 0;
	uint8_t preamble_type = pktcapture_hdr->preamble;
	uint8_t mcs = 0, bw = 0;
	uint16_t vht_flags = 0, ht_flags = 0;

	switch (preamble_type) {
	case 0x0:
	case 0x1:
	/* legacy */
		tx_status->rate = pkt_capture_get_tx_rate(
						preamble_type,
						pktcapture_hdr->rate,
						&preamble);
		break;
	case 0x2:
		ht_flags = 1;
		bw = pktcapture_hdr->bw;
		if (pktcapture_hdr->nss == 2)
			mcs = 8 + pktcapture_hdr->mcs;
		else
			mcs = pktcapture_hdr->mcs;
		break;
	case 0x3:
		vht_flags = 1;
		bw = pktcapture_hdr->bw;
		mcs = pktcapture_hdr->mcs;

		/* fallthrough */
	default:
		break;
	}

	tx_status->mcs = mcs;
	tx_status->bw = bw;
	tx_status->nr_ant = pktcapture_hdr->nss;
	tx_status->is_stbc = pktcapture_hdr->stbc;
	tx_status->sgi = pktcapture_hdr->sgi;
	tx_status->ldpc = pktcapture_hdr->ldpc;
	tx_status->beamformed = pktcapture_hdr->beamformed;
	tx_status->vht_flag_values3[0] = mcs << 0x4 | (pktcapture_hdr->nss + 1);
	tx_status->ht_flags = ht_flags;
	tx_status->vht_flags = vht_flags;
	tx_status->rtap_flags |= ((preamble == 1) ? BIT(1) : 0);
	if (bw == 0)
		tx_status->vht_flag_values2 = 0;
	else if (bw == 1)
		tx_status->vht_flag_values2 = 1;
	else if (bw == 2)
		tx_status->vht_flag_values2 = 4;
}

/**
 * pkt_capture_update_tx_status() - tx status for tx packets, for
 * pkt capture mode(normal tx + offloaded tx) to prepare radiotap header
 * @pdev: device handler
 * @tx_status: tx status to be updated
 * @mon_hdr: tx data header
 *
 * Return: none
 */
static void
pkt_capture_update_tx_status(
			htt_pdev_handle pdev,
			struct mon_rx_status *tx_status,
			struct pkt_capture_tx_hdr_elem_t *pktcapture_hdr)
{
	struct mon_channel *ch_info = &pdev->mon_ch_info;

	tx_status->tsft = (u_int64_t)(pktcapture_hdr->timestamp);
	tx_status->chan_freq = ch_info->ch_freq;
	tx_status->chan_num = ch_info->ch_num;

	pkt_capture_tx_get_phy_info(pktcapture_hdr, tx_status);

	if (pktcapture_hdr->preamble == 0)
		tx_status->ofdm_flag = 1;
	else if (pktcapture_hdr->preamble == 1)
		tx_status->cck_flag = 1;

	tx_status->ant_signal_db = pktcapture_hdr->rssi_comb;
	tx_status->rssi_comb = pktcapture_hdr->rssi_comb;
	tx_status->tx_status = pktcapture_hdr->status;
	tx_status->tx_retry_cnt = pktcapture_hdr->tx_retry_cnt;
	tx_status->add_rtap_ext = true;
}

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
			bssid, pdev, 0);
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
 * @tx_retry_cnt: tx retry count
 *
 * Return: none
 */
static void
pkt_capture_rx_data_cb(
		void *context, void *ppdev, void *nbuf_list,
		uint8_t vdev_id, uint8_t tid,
		uint16_t status, bool pkt_format,
		uint8_t *bssid, uint8_t tx_retry_cnt)
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
		rx_status.tx_status = status;
		rx_status.tx_retry_cnt = tx_retry_cnt;
		rx_status.add_rtap_ext = true;

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
		pkt_capture_mon(cb_ctx, msdu, vdev, 0);
		msdu = next_buf;
	}

	return;

free_buf:
	drop_count = pkt_capture_drop_nbuf_list(buf_list);
}

/**
 * pkt_capture_tx_data_cb() - process data tx and rx packets
 * for pkt capture mode. (normal tx/rx + offloaded tx/rx)
 * @vdev_id: vdev id for which packet is captured
 * @mon_buf_list: netbuf list
 * @type: data process type
 * @tid:  tid number
 * @status: Tx status
 * @pktformat: Frame format
 * @bssid: bssid
 * @pdev: pdev handle
 * @tx_retry_cnt: tx retry count
 *
 * Return: none
 */
static void
pkt_capture_tx_data_cb(
		void *context, void *ppdev, void *nbuf_list, uint8_t vdev_id,
		uint8_t tid, uint16_t status, bool pkt_format,
		uint8_t *bssid, uint8_t tx_retry_cnt)
{
	qdf_nbuf_t msdu, next_buf;
	struct pkt_capture_vdev_priv *vdev_priv;
	struct wlan_objmgr_vdev *vdev = context;
	htt_pdev_handle pdev = ppdev;
	struct pkt_capture_cb_context *cb_ctx;
	uint8_t drop_count;
	struct htt_tx_data_hdr_information *cmpl_desc = NULL;
	struct pkt_capture_tx_hdr_elem_t pktcapture_hdr = {0};
	struct ethernet_hdr_t *eth_hdr;
	struct llc_snap_hdr_t *llc_hdr;
	struct ieee80211_frame *wh;
	uint8_t hdsize, new_hdsize;
	struct ieee80211_qoscntl *qos_cntl;
	uint16_t ether_type;
	uint32_t headroom;
	uint16_t seq_no, fc_ctrl;
	struct mon_rx_status tx_status = {0};
	uint8_t localbuf[sizeof(struct ieee80211_qosframe_htc_addr4) +
			sizeof(struct llc_snap_hdr_t)];
	const uint8_t ethernet_II_llc_snap_header_prefix[] = {
					0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (qdf_unlikely(!vdev))
		goto free_buf;

	cb_ctx = vdev_priv->cb_ctx;
	if (!cb_ctx || !cb_ctx->mon_cb || !cb_ctx->mon_ctx)
		goto free_buf;

	msdu = nbuf_list;
	while (msdu) {
		next_buf = qdf_nbuf_queue_next(msdu);
		qdf_nbuf_set_next(msdu, NULL);   /* Add NULL terminator */

		cmpl_desc = (struct htt_tx_data_hdr_information *)
					(qdf_nbuf_data(msdu));

		pktcapture_hdr.timestamp = cmpl_desc->phy_timestamp_l32;
		pktcapture_hdr.preamble = cmpl_desc->preamble;
		pktcapture_hdr.mcs = cmpl_desc->mcs;
		pktcapture_hdr.bw = cmpl_desc->bw;
		pktcapture_hdr.nss = cmpl_desc->nss;
		pktcapture_hdr.rssi_comb = cmpl_desc->rssi;
		pktcapture_hdr.rate = cmpl_desc->rate;
		pktcapture_hdr.stbc = cmpl_desc->stbc;
		pktcapture_hdr.sgi = cmpl_desc->sgi;
		pktcapture_hdr.ldpc = cmpl_desc->ldpc;
		pktcapture_hdr.beamformed = cmpl_desc->beamformed;
		pktcapture_hdr.status = status;
		pktcapture_hdr.tx_retry_cnt = tx_retry_cnt;

		qdf_nbuf_pull_head(
			msdu,
			sizeof(struct htt_tx_data_hdr_information));

		if (pkt_format == TXRX_PKTCAPTURE_PKT_FORMAT_8023) {
			eth_hdr = (struct ethernet_hdr_t *)qdf_nbuf_data(msdu);
			hdsize = sizeof(struct ethernet_hdr_t);
			wh = (struct ieee80211_frame *)localbuf;

			*(uint16_t *)wh->i_dur = 0;

			new_hdsize = 0;

			if (vdev_id == HTT_INVALID_VDEV)
				qdf_mem_copy(bssid, eth_hdr->dest_addr,
					     QDF_MAC_ADDR_SIZE);

			/* BSSID , SA , DA */
			qdf_mem_copy(wh->i_addr1, bssid,
				     QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(wh->i_addr2, eth_hdr->src_addr,
				     QDF_MAC_ADDR_SIZE);
			qdf_mem_copy(wh->i_addr3, eth_hdr->dest_addr,
				     QDF_MAC_ADDR_SIZE);

			seq_no = cmpl_desc->seqno;
			seq_no = (seq_no << IEEE80211_SEQ_SEQ_SHIFT) &
					IEEE80211_SEQ_SEQ_MASK;
			fc_ctrl = cmpl_desc->framectrl;
			qdf_mem_copy(wh->i_fc, &fc_ctrl, sizeof(fc_ctrl));
			qdf_mem_copy(wh->i_seq, &seq_no, sizeof(seq_no));

			wh->i_fc[1] &= ~IEEE80211_FC1_WEP;

			new_hdsize = sizeof(struct ieee80211_frame);

			if (wh->i_fc[0] & QDF_IEEE80211_FC0_SUBTYPE_QOS) {
				qos_cntl = (struct ieee80211_qoscntl *)
						(localbuf + new_hdsize);
				qos_cntl->i_qos[0] =
					(tid & IEEE80211_QOS_TID);
				qos_cntl->i_qos[1] = 0;
				new_hdsize += sizeof(struct ieee80211_qoscntl);
			}
			/*
			 * Prepare llc Header
			 */
			llc_hdr = (struct llc_snap_hdr_t *)
					(localbuf + new_hdsize);
			ether_type = (eth_hdr->ethertype[0] << 8) |
					(eth_hdr->ethertype[1]);
			if (ether_type >= ETH_P_802_3_MIN) {
				qdf_mem_copy(
					llc_hdr,
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

		pkt_capture_update_tx_status(
				pdev,
				&tx_status,
				&pktcapture_hdr);
		/*
		 * Calculate the headroom and adjust head
		 * to prepare radiotap header.
		 */
		headroom = qdf_nbuf_headroom(msdu);
		qdf_nbuf_update_radiotap(&tx_status, msdu, headroom);
		pkt_capture_mon(cb_ctx, msdu, vdev, 0);
		msdu = next_buf;
	}
	return;

free_buf:
	drop_count = pkt_capture_drop_nbuf_list(nbuf_list);
}

void pkt_capture_datapkt_process(
		uint8_t vdev_id,
		qdf_nbuf_t mon_buf_list,
		enum pkt_capture_data_process_type type,
		uint8_t tid, uint8_t status, bool pkt_format,
		uint8_t *bssid, htt_pdev_handle pdev,
		uint8_t tx_retry_cnt)
{
	uint8_t drop_count;
	struct pkt_capture_mon_pkt *pkt;
	pkt_capture_mon_thread_cb callback = NULL;
	struct wlan_objmgr_vdev *vdev;

	status = pkt_capture_txrx_status_map(status);
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
	case TXRX_PROCESS_TYPE_DATA_TX_COMPL:
		callback = pkt_capture_tx_data_cb;
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
	pkt->tx_retry_cnt = tx_retry_cnt;
	pkt_capture_indicate_monpkt(vdev, pkt);
	return;

drop_rx_buf:
	drop_count = pkt_capture_drop_nbuf_list(mon_buf_list);
}

struct htt_tx_data_hdr_information *pkt_capture_tx_get_txcomplete_data_hdr(
						uint32_t *msg_word,
						int num_msdus)
{
	int offset_dwords;
	u_int32_t has_tx_tsf;
	u_int32_t has_retry;
	u_int32_t has_ack_rssi;
	u_int32_t has_tx_tsf64;
	u_int32_t has_tx_compl_payload;
	struct htt_tx_compl_ind_append_retries *retry_list = NULL;
	struct htt_tx_data_hdr_information *txcomplete_data_hrd_list = NULL;

	has_tx_compl_payload = HTT_TX_COMPL_IND_APPEND4_GET(*msg_word);
	if (num_msdus <= 0 || !has_tx_compl_payload)
		return NULL;

	offset_dwords = 1 + ((num_msdus + 1) >> 1);

	has_retry = HTT_TX_COMPL_IND_APPEND_GET(*msg_word);
	if (has_retry) {
		int retry_index = 0;
		int width_for_each_retry =
			(sizeof(struct htt_tx_compl_ind_append_retries) +
			3) >> 2;

		retry_list = (struct htt_tx_compl_ind_append_retries *)
			(msg_word + offset_dwords);
		while (retry_list) {
			if (retry_list[retry_index++].flag == 0)
				break;
		}
		offset_dwords += retry_index * width_for_each_retry;
	}
	has_tx_tsf = HTT_TX_COMPL_IND_APPEND1_GET(*msg_word);
	if (has_tx_tsf) {
		int width_for_each_tsf =
			(sizeof(struct htt_tx_compl_ind_append_tx_tstamp)) >> 2;
		offset_dwords += width_for_each_tsf * num_msdus;
	}

	has_ack_rssi = HTT_TX_COMPL_IND_APPEND2_GET(*msg_word);
	if (has_ack_rssi)
		offset_dwords += ((num_msdus + 1) >> 1);

	has_tx_tsf64 = HTT_TX_COMPL_IND_APPEND3_GET(*msg_word);
	if (has_tx_tsf64)
		offset_dwords += (num_msdus << 1);

	txcomplete_data_hrd_list = (struct htt_tx_data_hdr_information *)
					(msg_word + offset_dwords);

	return txcomplete_data_hrd_list;
}

void pkt_capture_offload_deliver_indication_handler(
					void *msg, uint8_t vdev_id,
					uint8_t *bssid, htt_pdev_handle pdev)
{
	int nbuf_len;
	qdf_nbuf_t netbuf;
	uint8_t status;
	uint8_t tid = 0;
	bool pkt_format;
	u_int32_t *msg_word = (u_int32_t *)msg;
	u_int8_t *buf = (u_int8_t *)msg;
	struct htt_tx_data_hdr_information *txhdr;
	struct htt_tx_offload_deliver_ind_hdr_t *offload_deliver_msg;

	offload_deliver_msg = (struct htt_tx_offload_deliver_ind_hdr_t *)msg;

	txhdr = (struct htt_tx_data_hdr_information *)
		(msg_word + 1);

	nbuf_len = offload_deliver_msg->tx_mpdu_bytes;

	netbuf = qdf_nbuf_alloc(NULL,
				roundup(nbuf_len + RESERVE_BYTES, 4),
				RESERVE_BYTES, 4, false);

	if (!netbuf)
		return;

	qdf_nbuf_put_tail(netbuf, nbuf_len);

	qdf_mem_copy(qdf_nbuf_data(netbuf),
		     buf + sizeof(struct htt_tx_offload_deliver_ind_hdr_t),
		     nbuf_len);

	qdf_nbuf_push_head(
			netbuf,
			sizeof(struct htt_tx_data_hdr_information));

	qdf_mem_copy(qdf_nbuf_data(netbuf), txhdr,
		     sizeof(struct htt_tx_data_hdr_information));

	status = offload_deliver_msg->status;
	pkt_format = offload_deliver_msg->format;
	tid = offload_deliver_msg->tid_num;

	pkt_capture_datapkt_process(
			vdev_id,
			netbuf, TXRX_PROCESS_TYPE_DATA_TX,
			tid, status, pkt_format, bssid, pdev,
			offload_deliver_msg->tx_retry_cnt);
}

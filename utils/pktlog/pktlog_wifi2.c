/**
 * Copyright (c) 2013-2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* WIFI2 - Refers to legacy platforms */
#include "pktlog_wifi2.h"

#ifndef REMOVE_PKT_LOG
static struct txctl_frm_hdr frm_hdr;

#ifndef HELIUMPLUS
/**
 * process_ieee_hdr(): Process ieee header from the pktlog buffer
 * @data: pktlog buffer
 *
 * Return: None
 */
static void process_ieee_hdr(void *data)
{
	uint8_t dir;
	struct ieee80211_frame *wh = (struct ieee80211_frame *)(data);

	frm_hdr.framectrl = *(uint16_t *)(wh->i_fc);
	frm_hdr.seqctrl = *(uint16_t *)(wh->i_seq);
	dir = (wh->i_fc[1] & IEEE80211_FC1_DIR_MASK);

	if (dir == IEEE80211_FC1_DIR_TODS) {
		frm_hdr.bssid_tail =
			(wh->i_addr1[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr1
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
		frm_hdr.sa_tail =
			(wh->i_addr2[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr2
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
		frm_hdr.da_tail =
			(wh->i_addr3[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr3
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
	} else if (dir == IEEE80211_FC1_DIR_FROMDS) {
		frm_hdr.bssid_tail =
			(wh->i_addr2[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr2
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
		frm_hdr.sa_tail =
			(wh->i_addr3[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr3
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
		frm_hdr.da_tail =
			(wh->i_addr1[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr1
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
	} else {
		frm_hdr.bssid_tail =
			(wh->i_addr3[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr3
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
		frm_hdr.sa_tail =
			(wh->i_addr2[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr2
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
		frm_hdr.da_tail =
			(wh->i_addr1[QDF_MAC_ADDR_SIZE - 2] << 8) | (wh->
								      i_addr1
								      [QDF_MAC_ADDR_SIZE
								       - 1]);
	}
}

/**
 * fill_ieee80211_hdr_data() - fill ieee802.11 data header
 * @txrx_pdev: txrx pdev
 * @pl_msdu_info: msdu info
 * @data: data received from event
 *
 * Return: none
 */
/* TODO: Platform specific function */
static void
fill_ieee80211_hdr_data(struct cdp_pdev *pdev,
			struct ath_pktlog_msdu_info *pl_msdu_info,
			void *data)
{
	uint32_t i;
	uint32_t *htt_tx_desc;
	struct ol_tx_desc_t *tx_desc;
	uint8_t msdu_id_offset = MSDU_ID_INFO_ID_OFFSET;
	uint16_t tx_desc_id;
	uint32_t *msdu_id_info = (uint32_t *)
				 ((void *)data + sizeof(struct ath_pktlog_hdr));
	uint32_t *msdu_id = (uint32_t *)((char *)msdu_id_info +
					  msdu_id_offset);
	uint8_t *addr, *vap_addr;
	uint8_t vdev_id;
	qdf_nbuf_t netbuf;
	uint32_t len;
	struct ol_txrx_pdev_t *txrx_pdev = (struct ol_txrx_pdev_t *)pdev;

	pl_msdu_info->num_msdu = *msdu_id_info;
	pl_msdu_info->priv_size = sizeof(uint32_t) *
				 pl_msdu_info->num_msdu + sizeof(uint32_t);

	if (pl_msdu_info->num_msdu > MAX_PKT_INFO_MSDU_ID) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid num_msdu count",
			  __func__);
		qdf_assert(0);
		return;
	}
	for (i = 0; i < pl_msdu_info->num_msdu; i++) {
		/*
		 * Handle big endianness
		 * Increment msdu_id once after retrieving
		 * lower 16 bits and uppper 16 bits
		 */
		if (!(i % 2)) {
			tx_desc_id = ((*msdu_id & TX_DESC_ID_LOW_MASK)
				      >> TX_DESC_ID_LOW_SHIFT);
		} else {
			tx_desc_id = ((*msdu_id & TX_DESC_ID_HIGH_MASK)
				      >> TX_DESC_ID_HIGH_SHIFT);
			msdu_id += 1;
		}
		if (tx_desc_id >= txrx_pdev->tx_desc.pool_size) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
				  "%s: drop due to invalid msdu id = %x",
				  __func__, tx_desc_id);
			return;
		}
		tx_desc = ol_tx_desc_find(txrx_pdev, tx_desc_id);
		qdf_assert(tx_desc);
		netbuf = tx_desc->netbuf;
		htt_tx_desc = (uint32_t *)tx_desc->htt_tx_desc;
		qdf_assert(htt_tx_desc);

		qdf_nbuf_peek_header(netbuf, &addr, &len);

		if (len < (2 * QDF_MAC_ADDR_SIZE)) {
			qdf_print("TX frame does not have a valid address");
			return;
		}
		/* Adding header information for the TX data frames */
		vdev_id = (uint8_t)(*(htt_tx_desc +
				       HTT_TX_VDEV_ID_WORD) >>
				     HTT_TX_VDEV_ID_SHIFT) &
			  HTT_TX_VDEV_ID_MASK;

		vap_addr = wma_get_vdev_address_by_vdev_id(vdev_id);

		frm_hdr.da_tail = (addr[QDF_MAC_ADDR_SIZE - 2] << 8) |
				  (addr[QDF_MAC_ADDR_SIZE - 1]);
		frm_hdr.sa_tail =
			(addr[2 * QDF_MAC_ADDR_SIZE - 2] << 8) |
			(addr[2 * QDF_MAC_ADDR_SIZE - 1]);
		if (vap_addr) {
			frm_hdr.bssid_tail =
				(vap_addr[QDF_MAC_ADDR_SIZE - 2] << 8) |
				(vap_addr[QDF_MAC_ADDR_SIZE - 1]);
		} else {
			frm_hdr.bssid_tail = 0x0000;
		}
		pl_msdu_info->priv.msdu_len[i] = *(htt_tx_desc +
						  HTT_TX_MSDU_LEN_DWORD)
						& HTT_TX_MSDU_LEN_MASK;
		/*
		 * Add more information per MSDU
		 * e.g., protocol information
		 */
	}
}
#endif /* HELIUMPLUS */

#ifdef HELIUMPLUS
A_STATUS process_tx_info(struct cdp_pdev *txrx_pdev, void *data)
{
	/*
	 * Must include to process different types
	 * TX_CTL, TX_STATUS, TX_MSDU_ID, TX_FRM_HDR
	 */
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	uint32_t *pl_tgt_hdr;
	struct ol_fw_data *fw_data;
	uint32_t len;

	if (!txrx_pdev) {
		qdf_nofl_info("Invalid pdev in %s", __func__);
		return A_ERROR;
	}

	if (!pl_dev) {
		qdf_nofl_err("Invalid pktlog handle in %s", __func__);
		qdf_assert(pl_dev);
		return A_ERROR;
	}

	qdf_assert(data);

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MAC_ID_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.flags |= PKTLOG_HDR_SIZE_16;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
		   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
		  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
		   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
		  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	pl_hdr.type_specific_data =
		*(pl_tgt_hdr + ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET);
	pl_info = pl_dev->pl_info;

	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_CTRL) {
		size_t log_size = sizeof(frm_hdr) + pl_hdr.size;
		void *txdesc_hdr_ctl = (void *)
				pktlog_getbuf(pl_dev, pl_info,
					      log_size, &pl_hdr);

		qdf_assert(txdesc_hdr_ctl);
		qdf_assert(pl_hdr.size < (370 * sizeof(u_int32_t)));

		qdf_mem_copy(txdesc_hdr_ctl, &frm_hdr, sizeof(frm_hdr));
		qdf_mem_copy((char *)txdesc_hdr_ctl + sizeof(frm_hdr),
			     ((void *)fw_data->data +
			     sizeof(struct ath_pktlog_hdr)),
			     pl_hdr.size);
		pl_hdr.size = log_size;
		cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
					       txdesc_hdr_ctl);
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_STAT) {
		struct ath_pktlog_tx_status txstat_log;
		size_t log_size = pl_hdr.size;

		txstat_log.ds_status = (void *)
				       pktlog_getbuf(pl_dev, pl_info,
						     log_size, &pl_hdr);
		qdf_assert(txstat_log.ds_status);
		qdf_mem_copy(txstat_log.ds_status,
			     ((void *)fw_data->data +
			      sizeof(struct ath_pktlog_hdr)),
			     pl_hdr.size);
		/* TODO: MCL specific API */
		cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
					       txstat_log.ds_status);
	}
	return A_OK;
}
#else
A_STATUS process_tx_info(struct cdp_pdev *txrx_pdev, void *data)
{
	/*
	 * Must include to process different types
	 * TX_CTL, TX_STATUS, TX_MSDU_ID, TX_FRM_HDR
	 */
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	uint32_t *pl_tgt_hdr;
	struct ol_fw_data *fw_data;
	uint32_t len;

	if (!txrx_pdev) {
		qdf_print("Invalid pdev in %s", __func__);
		return A_ERROR;
	}

	if (!pl_dev) {
		qdf_nofl_err("Invalid pktlog handle in %s", __func__);
		qdf_assert(pl_dev);
		return A_ERROR;
	}

	qdf_assert(data);

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);

	pktlog_hdr_set_specific_data(&pl_hdr,
				     *(pl_tgt_hdr +
				     ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET));

	pl_info = pl_dev->pl_info;

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_FRM_HDR) {
		/* Valid only for the TX CTL */
		process_ieee_hdr(fw_data->data + sizeof(pl_hdr));
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_VIRT_ADDR) {
		uint32_t desc_id = (uint32_t)*((uint32_t *)(fw_data->data +
						 sizeof(pl_hdr)));
		uint32_t vdev_id = desc_id;

		/* if the pkt log msg is for the bcn frame the vdev id
		 * is piggybacked in desc_id and the MSB of the desc ID
		 * would be set to FF
		 */
#define BCN_DESC_ID 0xFF
		if ((desc_id >> 24) == BCN_DESC_ID) {
			void *data;
			uint32_t buf_size;

			vdev_id &= 0x00FFFFFF;
			/* TODO: MCL specific API */
			data = wma_get_beacon_buffer_by_vdev_id(vdev_id,
								&buf_size);
			if (data) {
				/* TODO: platform specific API */
				process_ieee_hdr(data);
				qdf_mem_free(data);
			}
		} else {
			/*
			 * TODO: get the hdr content for mgmt frames from
			 * Tx mgmt desc pool
			 */
		}
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_CTRL) {
		struct ath_pktlog_txctl txctl_log;
		size_t log_size = sizeof(txctl_log.priv);

		txctl_log.txdesc_hdr_ctl = (void *)pktlog_getbuf(pl_dev,
								 pl_info,
								 log_size,
								 &pl_hdr);

		if (!txctl_log.txdesc_hdr_ctl) {
			qdf_nofl_info
				("failed to get txctl_log.txdesc_hdr_ctl buf");
			return A_ERROR;
		}

		/*
		 * frm hdr is currently Valid only for local frames
		 * Add capability to include the fmr hdr for remote frames
		 */
		txctl_log.priv.frm_hdr = frm_hdr;
		qdf_assert(txctl_log.priv.txdesc_ctl);
		qdf_assert(pl_hdr.size < sizeof(txctl_log.priv.txdesc_ctl));
		pl_hdr.size = (pl_hdr.size > sizeof(txctl_log.priv.txdesc_ctl))
			       ? sizeof(txctl_log.priv.txdesc_ctl) :
			       pl_hdr.size;

		if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
			qdf_assert(0);
			return A_ERROR;
		}
		qdf_mem_copy((void *)&txctl_log.priv.txdesc_ctl,
			     ((void *)fw_data->data +
			      sizeof(struct ath_pktlog_hdr)),
			     pl_hdr.size);
		qdf_assert(txctl_log.txdesc_hdr_ctl);
		qdf_mem_copy(txctl_log.txdesc_hdr_ctl, &txctl_log.priv,
			     sizeof(txctl_log.priv));
		pl_hdr.size = log_size;
		cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
					       txctl_log.txdesc_hdr_ctl);
		/* Add Protocol information and HT specific information */
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_STAT) {
		struct ath_pktlog_tx_status txstat_log;
		size_t log_size = pl_hdr.size;

		txstat_log.ds_status = (void *)
				       pktlog_getbuf(pl_dev, pl_info,
						     log_size, &pl_hdr);
		qdf_assert(txstat_log.ds_status);
		qdf_mem_copy(txstat_log.ds_status,
			     ((void *)fw_data->data +
			      sizeof(struct ath_pktlog_hdr)),
			     pl_hdr.size);

		cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
					       txstat_log.ds_status);
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_MSDU_ID) {
		struct ath_pktlog_msdu_info pl_msdu_info;
		size_t log_size;

		qdf_mem_zero(&pl_msdu_info, sizeof(pl_msdu_info));
		log_size = sizeof(pl_msdu_info.priv);

		if (pl_dev->mt_pktlog_enabled == false)
			fill_ieee80211_hdr_data(txrx_pdev,
						&pl_msdu_info, fw_data->data);

		pl_msdu_info.ath_msdu_info = pktlog_getbuf(pl_dev, pl_info,
							   log_size, &pl_hdr);
		qdf_mem_copy((void *)&pl_msdu_info.priv.msdu_id_info,
			     ((void *)fw_data->data +
			      sizeof(struct ath_pktlog_hdr)),
			     sizeof(pl_msdu_info.priv.msdu_id_info));
		qdf_mem_copy(pl_msdu_info.ath_msdu_info, &pl_msdu_info.priv,
			     sizeof(pl_msdu_info.priv));
		cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
					       pl_msdu_info.ath_msdu_info);
	}

	return A_OK;
}
#endif /* HELIUMPLUS */

A_STATUS process_rx_info_remote(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_info *pl_info;
	struct htt_host_rx_desc_base *rx_desc;
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_rx_info rxstat_log;
	size_t log_size;
	struct ol_rx_remote_data *r_data = (struct ol_rx_remote_data *)data;
	qdf_nbuf_t msdu;

	if (!pdev || !r_data || !pl_dev) {
		qdf_print("%s: Invalid handle", __func__);
		return A_ERROR;
	}

	pl_info = pl_dev->pl_info;
	msdu = r_data->msdu;

	while (msdu) {
		rx_desc =
		   (struct htt_host_rx_desc_base *)(qdf_nbuf_data(msdu)) - 1;
		log_size =
			sizeof(*rx_desc) - sizeof(struct htt_host_fw_desc_base);

		/*
		 * Construct the pktlog header pl_hdr
		 * Because desc is DMA'd to the host memory
		 */
		pl_hdr.flags = (1 << PKTLOG_FLG_FRM_TYPE_REMOTE_S);
		pl_hdr.missed_cnt = 0;
#if defined(HELIUMPLUS)
		pl_hdr.macId = r_data->mac_id;
		pl_hdr.log_type = PKTLOG_TYPE_RX_STAT;
		pl_hdr.flags |= PKTLOG_HDR_SIZE_16;
#else
		pl_hdr.log_type = PKTLOG_TYPE_RX_STAT;
#endif
		pl_hdr.size = sizeof(*rx_desc) -
			      sizeof(struct htt_host_fw_desc_base);
#if defined(HELIUMPLUS)
		pl_hdr.timestamp =
			rx_desc->ppdu_end.rx_pkt_end.phy_timestamp_1_lower_32;
		pl_hdr.type_specific_data = 0xDEADAA;
#else
		pl_hdr.timestamp = rx_desc->ppdu_end.tsf_timestamp;
#endif /* !defined(HELIUMPLUS) */

		pktlog_hdr_set_specific_data(&pl_hdr, 0xDEADAA);

		rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
							   log_size, &pl_hdr);
		qdf_mem_copy(rxstat_log.rx_desc, (void *)rx_desc +
			     sizeof(struct htt_host_fw_desc_base), pl_hdr.size);
		cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
					       rxstat_log.rx_desc);
		msdu = qdf_nbuf_next(msdu);
	}
	return A_OK;
}

#ifdef HELIUMPLUS
A_STATUS process_rx_info(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_rx_info rxstat_log;
	struct ath_pktlog_hdr pl_hdr;
	size_t log_size;
	uint32_t *pl_tgt_hdr;
	struct ol_fw_data *fw_data;
	uint32_t len;

	if (!pdev) {
		qdf_nofl_info("Invalid pdev in %s", __func__);
		return A_ERROR;
	}

	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	if (!pl_dev) {
		qdf_nofl_info("Invalid pl_dev in %s", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MAC_ID_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_info = pl_dev->pl_info;
	pl_tgt_hdr = (uint32_t *)fw_data->data;

	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
	pl_hdr.flags |= PKTLOG_HDR_SIZE_16;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}

	log_size = pl_hdr.size;
	rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);
	qdf_mem_copy(rxstat_log.rx_desc,
		     (void *)fw_data->data + sizeof(struct ath_pktlog_hdr),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rxstat_log.rx_desc);

	return A_OK;
}
#else
A_STATUS process_rx_info(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_rx_info rxstat_log;
	struct ath_pktlog_hdr pl_hdr;
	size_t log_size;
	uint32_t *pl_tgt_hdr;
	struct ol_fw_data *fw_data;
	uint32_t len;

	if (!pdev) {
		qdf_nofl_info("Invalid pdev in %s", __func__);
		return A_ERROR;
	}

	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	if (!pl_dev) {
		qdf_nofl_info("Invalid pl_dev in %s", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_info = pl_dev->pl_info;
	pl_tgt_hdr = (uint32_t *)fw_data->data;
	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
				   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}

	log_size = pl_hdr.size;
	rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);
	qdf_mem_copy(rxstat_log.rx_desc,
		     (void *)fw_data->data + sizeof(struct ath_pktlog_hdr),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rxstat_log.rx_desc);

	return A_OK;
}
#endif /* HELIUMPLUS */

#ifdef HELIUMPLUS
A_STATUS process_rate_find(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	size_t log_size;
	uint32_t len;
	struct ol_fw_data *fw_data;

	/*
	 * Will be uncommented when the rate control find
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	struct ath_pktlog_rc_find rcf_log;
	uint32_t *pl_tgt_hdr;

	if (!pdev || !data || !pl_dev) {
		qdf_print("%s: Invalid handle", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MAC_ID_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */

	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
	pl_hdr.flags |= PKTLOG_HDR_SIZE_16;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	rcf_log.rcFind = (void *)pktlog_getbuf(pl_dev, pl_info,
					       log_size, &pl_hdr);

	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}
	qdf_mem_copy(rcf_log.rcFind,
		     ((char *)fw_data->data + sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rcf_log.rcFind);

	return A_OK;
}

#else
A_STATUS process_rate_find(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	size_t log_size;
	uint32_t len;
	struct ol_fw_data *fw_data;

	/*
	 * Will be uncommented when the rate control find
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	struct ath_pktlog_rc_find rcf_log;
	uint32_t *pl_tgt_hdr;

	if (!pdev || !data || !pl_dev) {
		qdf_print("%s: Invalid handle", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */

	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	rcf_log.rcFind = (void *)pktlog_getbuf(pl_dev, pl_info,
					       log_size, &pl_hdr);

	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}
	qdf_mem_copy(rcf_log.rcFind,
		     ((char *)fw_data->data + sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rcf_log.rcFind);

	return A_OK;
}
#endif

#ifdef HELIUMPLUS
A_STATUS process_rate_update(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	size_t log_size;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_rc_update rcu_log;
	uint32_t *pl_tgt_hdr;
	struct ol_fw_data *fw_data;
	uint32_t len;

	if (!pdev || !data || !pl_dev) {
		qdf_print("%s: Invalid handle", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MAC_ID_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
	pl_hdr.flags |= PKTLOG_HDR_SIZE_16;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	log_size = pl_hdr.size;
	pl_info = pl_dev->pl_info;

	/*
	 * Will be uncommented when the rate control update
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	rcu_log.txRateCtrl = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);
	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}
	qdf_mem_copy(rcu_log.txRateCtrl,
		     ((char *)fw_data->data +
		      sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rcu_log.txRateCtrl);
	return A_OK;
}
#else
A_STATUS process_rate_update(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	size_t log_size;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_rc_update rcu_log;
	uint32_t *pl_tgt_hdr;
	struct ol_fw_data *fw_data;
	uint32_t len;

	if (!pdev || !data || !pl_dev) {
		qdf_print("%s: Invalid handle", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
				   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	log_size = pl_hdr.size;
	pl_info = pl_dev->pl_info;

	/*
	 * Will be uncommented when the rate control update
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	rcu_log.txRateCtrl = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);
	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}
	qdf_mem_copy(rcu_log.txRateCtrl,
		     ((char *)fw_data->data +
		      sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rcu_log.txRateCtrl);
	return A_OK;
}
#endif /* HELIUMPLUS */

#ifdef HELIUMPLUS
A_STATUS process_sw_event(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	size_t log_size;
	uint32_t len;
	struct ol_fw_data *fw_data;

	/*
	 * Will be uncommented when the rate control find
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	struct ath_pktlog_sw_event sw_event;
	uint32_t *pl_tgt_hdr;

	if (!pdev) {
		qdf_print("Invalid pdev in %s", __func__);
		return A_ERROR;
	}
	if (!data) {
		qdf_print("Invalid data in %s", __func__);
		return A_ERROR;
	}
	if (!pl_dev) {
		qdf_print("Invalid pl_dev in %s", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MAC_ID_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);

	pl_hdr.type_specific_data =
		*(pl_tgt_hdr + ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET);
	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	sw_event.sw_event = (void *)pktlog_getbuf(pl_dev, pl_info,
					       log_size, &pl_hdr);
	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}
	qdf_mem_copy(sw_event.sw_event,
		     ((char *)fw_data->data + sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);

	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, sw_event.sw_event);

	return A_OK;
}
#else
A_STATUS process_sw_event(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	size_t log_size;
	uint32_t len;
	struct ol_fw_data *fw_data;

	/*
	 * Will be uncommented when the rate control find
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	struct ath_pktlog_sw_event sw_event;
	uint32_t *pl_tgt_hdr;

	if (!pdev) {
		qdf_print("Invalid pdev in %s", __func__);
		return A_ERROR;
	}
	if (!data) {
		qdf_print("Invalid data in %s", __func__);
		return A_ERROR;
	}
	if (!pl_dev) {
		qdf_print("Invalid pl_dev in %s", __func__);
		return A_ERROR;
	}

	fw_data = (struct ol_fw_data *)data;
	len = fw_data->len;
	if (len < (sizeof(uint32_t) *
		   (ATH_PKTLOG_HDR_FLAGS_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_MISSED_CNT_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_LOG_TYPE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_SIZE_OFFSET + 1)) ||
		len < (sizeof(uint32_t) *
		       (ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET + 1))) {
		qdf_print("Invalid msdu len in %s", __func__);
		qdf_assert(0);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)fw_data->data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
				   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);

	pktlog_hdr_set_specific_data(&pl_hdr,
				     *(pl_tgt_hdr +
				     ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET));

	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	sw_event.sw_event = (void *)pktlog_getbuf(pl_dev, pl_info,
					       log_size, &pl_hdr);
	if (sizeof(struct ath_pktlog_hdr) + pl_hdr.size > len) {
		qdf_assert(0);
		return A_ERROR;
	}
	qdf_mem_copy(sw_event.sw_event,
		     ((char *)fw_data->data + sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);

	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, sw_event.sw_event);

	return A_OK;
}
#endif /* HELIUMPLUS */
#endif /* REMOVE_PKT_LOG */

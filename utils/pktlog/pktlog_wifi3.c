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

/* WIFI3 - Refers to platforms - 6290/6390/6490 */
#include "pktlog_wifi3.h"

#ifndef REMOVE_PKT_LOG
A_STATUS
process_offload_pktlog_wifi3(struct cdp_pdev *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_hdr pl_hdr;
	uint32_t *pl_tgt_hdr;
	void *txdesc_hdr_ctl = NULL;
	size_t log_size = 0;

	if (!pl_dev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid context in %s\n", __func__);
		return A_ERROR;
	}

	if (!data) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid data in %s\n", __func__);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *)data;

	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
				ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt =  (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
				ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
	pl_hdr.log_type =  (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.size =  (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
			ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);

	pl_hdr.type_specific_data = *(pl_tgt_hdr +
				    ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET);

	if (pl_hdr.size > MAX_PKTLOG_RECV_BUF_SIZE) {
		pl_dev->invalid_packets++;
		return A_ERROR;
	}

	/*
	 *  Must include to process different types
	 *  TX_CTL, TX_STATUS, TX_MSDU_ID, TX_FRM_HDR
	 */
	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	txdesc_hdr_ctl =
		(void *)pktlog_getbuf(pl_dev, pl_info, log_size, &pl_hdr);
	if (!txdesc_hdr_ctl) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Failed to allocate pktlog descriptor");
		return A_NO_MEMORY;
	}
	qdf_assert(txdesc_hdr_ctl);
	qdf_assert(pl_hdr->size < PKTLOG_MAX_TX_WORDS * sizeof(u_int32_t));
	qdf_mem_copy(txdesc_hdr_ctl,
		     ((void *)data + sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, txdesc_hdr_ctl);

	return A_OK;
}

int process_rx_desc_remote_wifi3(void *pdev, void *data)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_rx_info rxstat_log;
	size_t log_size;
	struct ath_pktlog_info *pl_info;
	qdf_nbuf_t log_nbuf = (qdf_nbuf_t)data;

	if (!pl_dev) {
		qdf_err("Pktlog handle is NULL");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;
	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (1 << PKTLOG_FLG_FRM_TYPE_REMOTE_S);
	pl_hdr.missed_cnt = 0;
	pl_hdr.log_type = PKTLOG_TYPE_RX_STATBUF;
	pl_hdr.size = qdf_nbuf_len(log_nbuf);
	pl_hdr.timestamp = 0;
	log_size = pl_hdr.size;
	rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
						  log_size, &pl_hdr);

	if (!rxstat_log.rx_desc) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Rx descriptor is NULL", __func__);
		return -EINVAL;
	}

	qdf_mem_copy(rxstat_log.rx_desc, qdf_nbuf_data(log_nbuf), pl_hdr.size);
	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL,
				       rxstat_log.rx_desc);
	return 0;
}

int
process_pktlog_lite_wifi3(void *context, void *log_data,
			  uint16_t log_type)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_rx_info rxstat_log;
	size_t log_size;
	qdf_nbuf_t log_nbuf = (qdf_nbuf_t)log_data;

	if (!pl_dev) {
		qdf_err("Pktlog handle is NULL");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;
	qdf_mem_zero(&pl_hdr, sizeof(pl_hdr));
	pl_hdr.flags = (1 << PKTLOG_FLG_FRM_TYPE_REMOTE_S);
	pl_hdr.missed_cnt = 0;
	pl_hdr.log_type = log_type;
	pl_hdr.size = qdf_nbuf_len(log_nbuf);
	pl_hdr.timestamp = 0;
	log_size = pl_hdr.size;
	rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);
	if (!rxstat_log.rx_desc) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Rx descriptor is NULL", __func__);
		return -EINVAL;
	}

	qdf_mem_copy(rxstat_log.rx_desc, qdf_nbuf_data(log_nbuf), pl_hdr.size);

	cds_pkt_stats_to_logger_thread(&pl_hdr, NULL, rxstat_log.rx_desc);
	return 0;
}
#endif /* REMOVE_PKT_LOG */

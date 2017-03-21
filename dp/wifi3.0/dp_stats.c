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
#include "qdf_types.h"
#include "htt_stats.h"
#include "dp_types.h"
#include "dp_internal.h"

#define DP_MAX_STRING_LEN 500

/* these values are fixed in the next gerrit */
#define DP_HTT_DATA_LEN  1
#define DP_HTT_URRN_STATS_LEN  1
#define DP_HTT_FLUSH_ERRS_LEN  1
#define DP_HTT_SIFS_STATUS_LEN  1
#define DP_HTT_PHY_ERRS_LEN  1
#define DP_HTT_HW_INTR_NAME_LEN  HTT_STATS_MAX_HW_INTR_NAME_LEN
#define DP_HTT_HW_MODULE_NAME_LEN  HTT_STATS_MAX_HW_MODULE_NAME_LEN
#define DP_HTT_COUNTER_NAME_LEN  HTT_MAX_COUNTER_NAME
#define DP_HTT_PEER_DETAILS_LEN  1
#define DP_HTT_MU_MIMO_SCH_STATS_TLV_LEN  1
#define DP_HTT_MU_MIMO_MPDU_STATS_TLV_LEN  1
#define DP_HTT_DIFS_LATENCY_HIST_LEN  1
#define DP_HTT_CMD_RESULT_LEN  1
#define DP_HTT_CMD_STALL_STATUS_LEN  1
#define DP_HTT_FES_RESULT_LEN  1
#define DP_HTT_SCHED_CMD_POSTED_LEN  1
#define DP_HTT_SCHED_CMD_REAPED_LEN  1
#define DP_HTT_GEN_MPDU_END_REASON_LEN  1
#define DP_HTT_LIST_MPDU_END_REASON_LEN  1
#define DP_HTT_LIST_MPDU_CNT_HIST_LEN  1
#define DP_HTT_LOW_WM_HIT_COUNT_LEN  HTT_STATS_LOW_WM_BINS
#define DP_HTT_HIGH_WM_HIT_COUNT_LEN  HTT_STATS_HIGH_WM_BINS
#define DP_HTT_DWORDS_USED_BY_USER_N_LEN  1
#define DP_HTT_TX_MCS_LEN  HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS
#define DP_HTT_TX_SU_MCS_LEN  HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS
#define DP_HTT_TX_MU_MCS_LEN  HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS
#define DP_HTT_TX_NSS_LEN  HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS
#define DP_HTT_TX_BW_LEN  HTT_TX_PDEV_STATS_NUM_BW_COUNTERS
#define DP_HTT_TX_STBC_LEN  HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS
#define DP_HTT_TX_PREAM_LEN  HTT_TX_PDEV_STATS_NUM_PREAMBLE_TYPES
#define DP_HTT_TX_GI_LEN  HTT_TX_PDEV_STATS_NUM_GI_COUNTERS
#define DP_HTT_TX_DCM_LEN  HTT_TX_PDEV_STATS_NUM_DCM_COUNTERS
#define DP_HTT_RX_MCS_LEN  HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS
#define DP_HTT_RX_NSS_LEN  HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS
#define DP_HTT_RX_DCM_LEN  HTT_RX_PDEV_STATS_NUM_DCM_COUNTERS
#define DP_HTT_RX_STBC_LEN  HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS
#define DP_HTT_RX_BW_LEN  HTT_RX_PDEV_STATS_NUM_BW_COUNTERS
#define DP_HTT_RX_PREAM_LEN  HTT_RX_PDEV_STATS_NUM_PREAMBLE_TYPES
#define DP_HTT_RSSI_CHAIN_LEN  HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS
#define DP_HTT_RX_GI_LEN  HTT_RX_PDEV_STATS_NUM_GI_COUNTERS
#define DP_HTT_REFILL_RING_EMPTY_CNT_LEN  1
#define DP_HTT_REFILL_RING_NUM_REFILL_LEN  1
#define DP_HTT_FW_RING_MGMT_SUBTYPE_LEN  HTT_STATS_SUBTYPE_MAX
#define DP_HTT_FW_RING_CTRL_SUBTYPE_LEN  HTT_STATS_SUBTYPE_MAX
#define DP_HTT_FW_RING_MPDU_ERR_LEN  HTT_RX_STATS_RXDMA_MAX_ERR
#define DP_HTT_FW_MPDU_DROP_LEN  1
#define DP_HTT_TID_NAME_LEN  MAX_HTT_TID_NAME

/* TODO:Below stat function is for initial testing
 * Further functions will be added after reviews on the FW side are complete
 */

/**
 * dp_print_tx_pdev_stats_cmn_tlv: display htt_tx_pdev_stats_cmn_tlv
 * @dp_htt_tag:pointer to structure htt_tx_pdev_stats_cmn_tlv
 *
 * return:void
 */
static inline void dp_print_tx_pdev_stats_cmn_tlv(uint32_t *tag_buf)
{
	htt_tx_pdev_stats_cmn_tlv *dp_htt_tag =
		(htt_tx_pdev_stats_cmn_tlv *)tag_buf;

	DP_TRACE_STATS(NONE, "Pdev Stats:\n");
	DP_TRACE_STATS(NONE, "mac_id__word = %d",
			dp_htt_tag->mac_id__word);
	DP_TRACE_STATS(NONE, "hw_queued = %d",
			dp_htt_tag->hw_queued);
	DP_TRACE_STATS(NONE, "hw_reaped = %d",
			dp_htt_tag->hw_reaped);
	DP_TRACE_STATS(NONE, "underrun = %d",
			dp_htt_tag->underrun);
	DP_TRACE_STATS(NONE, "hw_paused = %d",
			dp_htt_tag->hw_paused);
	DP_TRACE_STATS(NONE, "hw_flush = %d",
			dp_htt_tag->hw_flush);
	DP_TRACE_STATS(NONE, "hw_filt = %d",
			dp_htt_tag->hw_filt);
	DP_TRACE_STATS(NONE, "tx_abort = %d",
			dp_htt_tag->tx_abort);
	DP_TRACE_STATS(NONE, "mpdu_requeued = %d",
			dp_htt_tag->mpdu_requed);
	DP_TRACE_STATS(NONE, "tx_xretry = %d",
			dp_htt_tag->tx_xretry);
	DP_TRACE_STATS(NONE, "data_rc = %d",
			dp_htt_tag->data_rc);
	DP_TRACE_STATS(NONE, "mpdu_dropped_xretry = %d",
			dp_htt_tag->mpdu_dropped_xretry);
	DP_TRACE_STATS(NONE, "illegal_rate_phy_err = %d",
			dp_htt_tag->illgl_rate_phy_err);
	DP_TRACE_STATS(NONE, "cont_xretry = %d",
			dp_htt_tag->cont_xretry);
	DP_TRACE_STATS(NONE, "tx_timeout = %d",
			dp_htt_tag->tx_timeout);
	DP_TRACE_STATS(NONE, "pdev_resets = %d",
			dp_htt_tag->pdev_resets);
	DP_TRACE_STATS(NONE, "phy_underrun = %d",
			dp_htt_tag->phy_underrun);
	DP_TRACE_STATS(NONE, "txop_ovf = %d",
			dp_htt_tag->txop_ovf);
	DP_TRACE_STATS(NONE, "seq_posted = %d",
			dp_htt_tag->seq_posted);
	DP_TRACE_STATS(NONE, "seq_failed_queueing = %d",
			dp_htt_tag->seq_failed_queueing);
	DP_TRACE_STATS(NONE, "seq_completed = %d",
			dp_htt_tag->seq_completed);
	DP_TRACE_STATS(NONE, "seq_restarted = %d",
			dp_htt_tag->seq_restarted);
	DP_TRACE_STATS(NONE, "mu_seq_posted = %d",
			dp_htt_tag->mu_seq_posted);
	DP_TRACE_STATS(NONE, "mpdu_count_tqm = %d",
			dp_htt_tag->mpdu_count_tqm);
	DP_TRACE_STATS(NONE, "msdu_count_tqm = %d",
			dp_htt_tag->msdu_count_tqm);
	DP_TRACE_STATS(NONE, "mpdu_removed_tqm = %d",
			dp_htt_tag->mpdu_removed_tqm);
	DP_TRACE_STATS(NONE, "msdu_removed_tqm = %d",
			dp_htt_tag->msdu_removed_tqm);
	DP_TRACE_STATS(NONE, "mpdus_sw_flush = %d",
			dp_htt_tag->mpdus_sw_flush);
	DP_TRACE_STATS(NONE, "mpdus_hw_filter = %d",
			dp_htt_tag->mpdus_hw_filter);
	DP_TRACE_STATS(NONE, "mpdus_truncated = %d",
			dp_htt_tag->mpdus_truncated);
	DP_TRACE_STATS(NONE, "mpdus_ack_failed = %d",
			dp_htt_tag->mpdus_ack_failed);
	DP_TRACE_STATS(NONE, "mpdus_expired = %d",
			dp_htt_tag->mpdus_expired);
	DP_TRACE_STATS(NONE, "mpdus_seq_hw_retry = %d",
			dp_htt_tag->mpdus_seq_hw_retry);
	DP_TRACE_STATS(NONE, "ack_tlv_proc = %d",
			dp_htt_tag->ack_tlv_proc);
	DP_TRACE_STATS(NONE, "coex_abort_mpdu_cnt_valid = %d",
			dp_htt_tag->
			coex_abort_mpdu_cnt_valid);
	DP_TRACE_STATS(NONE, "coex_abort_mpdu_cnt = %d",
			dp_htt_tag->coex_abort_mpdu_cnt);
}

/**
 * dp_print_tx_pdev_stats_urrn_tlv_v: display htt_tx_pdev_stats_urrn_tlv_v
 * @tag_buf: buffer containing the tlv
 *
 * return:void
 */
static inline void dp_print_tx_pdev_stats_urrn_tlv_v(uint32_t *tag_buf)
{
	htt_tx_pdev_stats_urrn_tlv_v *dp_htt_tag =
		(htt_tx_pdev_stats_urrn_tlv_v *)tag_buf;
	uint8_t i;
	uint16_t index = 0;
	char urrn_stats[DP_MAX_STRING_LEN];
	uint32_t tag_len = (HTT_STATS_TLV_LENGTH_GET(*tag_buf) >> 2);

	tag_len = qdf_min(tag_len, (uint32_t)HTT_TX_PDEV_MAX_URRN_STATS);
	DP_TRACE_STATS(NONE, "Pdev Underun Stats:\n");
	for (i = 0; i < tag_len; i++) {
		index += qdf_snprint(&urrn_stats[index],
				DP_MAX_STRING_LEN - index, " %d,",
				dp_htt_tag->urrn_stats[i]);
	}
	DP_TRACE_STATS(NONE, "urrn_stats = %s", urrn_stats);
}

/*
 * dp_print_tx_pdev_stats_flush_tlv_v: display htt_tx_pdev_stats_flush_tlv_v
 * @tag_buf: buffer containing the tlv *
 * return:void
 */
static inline void dp_print_tx_pdev_stats_flush_tlv_v(uint32_t *tag_buf)
{
	htt_tx_pdev_stats_flush_tlv_v *dp_htt_tag =
		(htt_tx_pdev_stats_flush_tlv_v *)tag_buf;
	uint8_t i;
	uint8_t index = 0;
	char flush_errs[DP_MAX_STRING_LEN];
	uint32_t tag_len = (HTT_STATS_TLV_LENGTH_GET(*tag_buf) >> 2);

	tag_len = qdf_min(tag_len,
			(uint32_t)HTT_TX_PDEV_MAX_FLUSH_REASON_STATS);
	DP_TRACE_STATS(NONE, "Pdev Flush Stats:\n");
	for (i = 0; i <  tag_len; i++) {
		index += qdf_snprint(&flush_errs[index],
				DP_MAX_STRING_LEN - index, " %d,",
				dp_htt_tag->flush_errs[i]);
	}
	DP_TRACE_STATS(NONE, "flush_errs = %s ", flush_errs);
}

/*
 * dp_print_tx_pdev_stats_sifs_tlv_v: display htt_tx_pdev_stats_sifs_tlv_v
 * @tag_buf: buffer containing the tlv *
 * return:void
 */
static inline void dp_print_tx_pdev_stats_sifs_tlv_v(uint32_t *tag_buf)
{
	htt_tx_pdev_stats_sifs_tlv_v *dp_htt_tag =
		(htt_tx_pdev_stats_sifs_tlv_v *)tag_buf;
	uint8_t i;
	uint8_t index = 0;
	char sifs_status[DP_MAX_STRING_LEN];
	uint32_t tag_len = (HTT_STATS_TLV_LENGTH_GET(*tag_buf) >> 2);

	tag_len = qdf_min(tag_len, (uint32_t)HTT_TX_PDEV_MAX_SIFS_BURST_STATS);
	DP_TRACE_STATS(NONE, "Pdev SIFS Stats:\n");
	for (i = 0; i <  tag_len; i++) {
		index += qdf_snprint(&sifs_status[index],
				DP_MAX_STRING_LEN - index, " %d,",
				dp_htt_tag->sifs_status[i]);
	}
	DP_TRACE_STATS(NONE, "sifs_status = %s ", sifs_status);
}

/**
 * dp_htt_stats_print_tag: function to select the tag type and
 * print the corresponding tag structure
 * @tag_type: tag type that is to be printed
 * @tag_buf: pointer to the tag structure
 *
 * return: void
 */
void dp_htt_stats_print_tag(uint8_t tag_type, uint32_t *tag_buf)
{
	switch (tag_type) {
	case HTT_STATS_TX_PDEV_CMN_TAG:
		dp_print_tx_pdev_stats_cmn_tlv(tag_buf);
		break;
	case HTT_STATS_TX_PDEV_UNDERRUN_TAG:
		dp_print_tx_pdev_stats_urrn_tlv_v(tag_buf);
		break;
	case HTT_STATS_TX_PDEV_SIFS_TAG:
		dp_print_tx_pdev_stats_sifs_tlv_v(tag_buf);
		break;
	default:
		break;
	}
}

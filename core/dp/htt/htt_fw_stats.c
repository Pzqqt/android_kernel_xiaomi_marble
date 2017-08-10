/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * @file htt_fw_stats.c
 * @brief Provide functions to process FW status retrieved from FW.
 */

#include <htc_api.h>            /* HTC_PACKET */
#include <htt.h>                /* HTT_T2H_MSG_TYPE, etc. */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <qdf_mem.h>         /* qdf_mem_set */
#include <ol_fw_tx_dbg.h>       /* ol_fw_tx_dbg_ppdu_base */

#include <ol_htt_rx_api.h>
#include <ol_txrx_htt_api.h>    /* htt_tx_status */

#include <htt_internal.h>

#include <wlan_defs.h>

#define ROUND_UP_TO_4(val) (((val) + 3) & ~0x3)


static char *bw_str_arr[] = {"20MHz", "40MHz", "80MHz", "160MHz"};

/*
 * Defined the macro tx_rate_stats_print_cmn()
 * so that this could be used in both
 * htt_t2h_stats_tx_rate_stats_print() &
 * htt_t2h_stats_tx_rate_stats_print_v2().
 * Each of these functions take a different structure as argument,
 * but with common fields in the structures--so using a macro
 * to bypass the strong type-checking of a function seems a simple
 * trick to use to avoid the code duplication.
 */
#define tx_rate_stats_print_cmn(_tx_rate_info, _concise) \
	do {							 \
		qdf_debug("TX Rate Info:");			 \
		\
		/* MCS */					 \
		qdf_debug("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",\
				"MCS counts (0..9)",		 \
				_tx_rate_info->mcs[0],		 \
				_tx_rate_info->mcs[1],		 \
				_tx_rate_info->mcs[2],		 \
				_tx_rate_info->mcs[3],		 \
				_tx_rate_info->mcs[4],		 \
				_tx_rate_info->mcs[5],		 \
				_tx_rate_info->mcs[6],		 \
				_tx_rate_info->mcs[7],		 \
				_tx_rate_info->mcs[8],		 \
				_tx_rate_info->mcs[9]);		 \
		\
		/* SGI */					 \
		qdf_debug("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",\
				"SGI counts (0..9)",		 \
				_tx_rate_info->sgi[0],		 \
				_tx_rate_info->sgi[1],		 \
				_tx_rate_info->sgi[2],		 \
				_tx_rate_info->sgi[3],		 \
				_tx_rate_info->sgi[4],		 \
				_tx_rate_info->sgi[5],		 \
				_tx_rate_info->sgi[6],		 \
				_tx_rate_info->sgi[7],		 \
				_tx_rate_info->sgi[8],		 \
				_tx_rate_info->sgi[9]);		 \
		\
		/* NSS */					 \
		qdf_debug("NSS  counts: 1x1 %d, 2x2 %d, 3x3 %d", \
				_tx_rate_info->nss[0],		 \
				_tx_rate_info->nss[1], _tx_rate_info->nss[2]);\
		\
		/* BW */					 \
		if (ARRAY_SIZE(_tx_rate_info->bw) == 3) \
			qdf_debug("BW counts: %s %d, %s %d, %s %d", \
				bw_str_arr[0], _tx_rate_info->bw[0],	 \
				bw_str_arr[1], _tx_rate_info->bw[1],	 \
				bw_str_arr[2], _tx_rate_info->bw[2]);	 \
		else if (ARRAY_SIZE(_tx_rate_info->bw) == 4) \
			qdf_debug("BW counts: %s %d, %s %d, %s %d, %s %d", \
				bw_str_arr[0], _tx_rate_info->bw[0],	 \
				bw_str_arr[1], _tx_rate_info->bw[1],	 \
				bw_str_arr[2], _tx_rate_info->bw[2],     \
				bw_str_arr[3], _tx_rate_info->bw[3]);	 \
		\
		\
		/* Preamble */					 \
		qdf_debug("Preamble (O C H V) counts: %d, %d, %d, %d",\
				_tx_rate_info->pream[0],		 \
				_tx_rate_info->pream[1],		 \
				_tx_rate_info->pream[2],		 \
				_tx_rate_info->pream[3]);		 \
		\
		/* STBC rate counts */				 \
		qdf_debug("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",\
				"STBC rate counts (0..9)",	 \
				_tx_rate_info->stbc[0],		 \
				_tx_rate_info->stbc[1],		 \
				_tx_rate_info->stbc[2],		 \
				_tx_rate_info->stbc[3],		 \
				_tx_rate_info->stbc[4],		 \
				_tx_rate_info->stbc[5],		 \
				_tx_rate_info->stbc[6],		 \
				_tx_rate_info->stbc[7],		 \
				_tx_rate_info->stbc[8],		 \
				_tx_rate_info->stbc[9]);	 \
			\
		/* LDPC and TxBF counts */			 \
		qdf_debug("LDPC Counts: %d", _tx_rate_info->ldpc);\
		qdf_debug("RTS Counts: %d", _tx_rate_info->rts_cnt);\
		/* RSSI Values for last ack frames */		\
		qdf_debug("Ack RSSI: %d", _tx_rate_info->ack_rssi);\
	} while (0)

static void htt_t2h_stats_tx_rate_stats_print(wlan_dbg_tx_rate_info_t *
					      tx_rate_info, int concise)
{
	tx_rate_stats_print_cmn(tx_rate_info, concise);
}

static void htt_t2h_stats_tx_rate_stats_print_v2(wlan_dbg_tx_rate_info_v2_t *
					      tx_rate_info, int concise)
{
	tx_rate_stats_print_cmn(tx_rate_info, concise);
}

/*
 * Defined the macro rx_rate_stats_print_cmn()
 * so that this could be used in both
 * htt_t2h_stats_rx_rate_stats_print() &
 * htt_t2h_stats_rx_rate_stats_print_v2().
 * Each of these functions take a different structure as argument,
 * but with common fields in the structures -- so using a macro
 * to bypass the strong type-checking of a function seems a simple
 * trick to use to avoid the code duplication.
 */
#define rx_rate_stats_print_cmn(_rx_phy_info, _concise) \
	do {							\
		qdf_debug("RX Rate Info:");			\
		\
		/* MCS */					\
		qdf_debug("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",\
				"MCS counts (0..9)",		 \
				_rx_phy_info->mcs[0],			\
				_rx_phy_info->mcs[1],			\
				_rx_phy_info->mcs[2],			\
				_rx_phy_info->mcs[3],			\
				_rx_phy_info->mcs[4],			\
				_rx_phy_info->mcs[5],			\
				_rx_phy_info->mcs[6],			\
				_rx_phy_info->mcs[7],			\
				_rx_phy_info->mcs[8],			\
				_rx_phy_info->mcs[9]);			\
		\
		/* SGI */						\
		qdf_debug("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",\
				"SGI counts (0..9)",		 \
				_rx_phy_info->sgi[0],			\
				_rx_phy_info->sgi[1],			\
				_rx_phy_info->sgi[2],			\
				_rx_phy_info->sgi[3],			\
				_rx_phy_info->sgi[4],			\
				_rx_phy_info->sgi[5],			\
				_rx_phy_info->sgi[6],			\
				_rx_phy_info->sgi[7],			\
				_rx_phy_info->sgi[8],			\
				_rx_phy_info->sgi[9]);			\
		\
		/*
		 * NSS							       \
		 * nss[0] just holds the count of non-stbc frames that were    \
		 * sent at 1x1 rates and nsts holds the count of frames sent   \
		 * with stbc.						       \
		 * It was decided to not include PPDUs sent w/ STBC in nss[0]  \
		 * since it would be easier to change the value that needs to  \
		 * be printed (from stbc+non-stbc count to only non-stbc count)\
		 * if needed in the future. Hence the addition in the host code\
		 * at this line.
		 */							       \
		qdf_debug("NSS  counts: 1x1 %d, 2x2 %d, 3x3 %d, 4x4 %d",\
				_rx_phy_info->nss[0] + _rx_phy_info->nsts,\
				_rx_phy_info->nss[1],			\
				_rx_phy_info->nss[2],			\
				_rx_phy_info->nss[3]);		\
		\
		/* NSTS */					\
		qdf_debug("NSTS count: %d", _rx_phy_info->nsts);	\
		\
		/* BW */					\
		if (ARRAY_SIZE(_rx_phy_info->bw) == 3) \
			qdf_debug("BW counts: %s %d, %s %d, %s %d",	\
				bw_str_arr[0], _rx_phy_info->bw[0],	\
				bw_str_arr[1], _rx_phy_info->bw[1],	\
				bw_str_arr[2], _rx_phy_info->bw[2]);	\
		else if (ARRAY_SIZE(_rx_phy_info->bw) == 4) \
			qdf_debug("BW counts: %s %d, %s %d, %s %d, %s %d", \
				bw_str_arr[0], _rx_phy_info->bw[0],	\
				bw_str_arr[1], _rx_phy_info->bw[1],	\
				bw_str_arr[2], _rx_phy_info->bw[2],    \
				bw_str_arr[3], _rx_phy_info->bw[3]);	\
		\
		/* Preamble */					\
		qdf_debug("Preamble counts: %d, %d, %d, %d, %d, %d",\
				_rx_phy_info->pream[0],		\
				_rx_phy_info->pream[1],		\
				_rx_phy_info->pream[2],		\
				_rx_phy_info->pream[3],		\
				_rx_phy_info->pream[4],		\
				_rx_phy_info->pream[5]);		\
		\
		/* STBC rate counts */				\
		qdf_debug("%s: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",\
				"STBC rate counts (0..9)",	\
				_rx_phy_info->stbc[0],		\
				_rx_phy_info->stbc[1],		\
				_rx_phy_info->stbc[2],		\
				_rx_phy_info->stbc[3],		\
				_rx_phy_info->stbc[4],		\
				_rx_phy_info->stbc[5],		\
				_rx_phy_info->stbc[6],		\
				_rx_phy_info->stbc[7],		\
				_rx_phy_info->stbc[8],		\
				_rx_phy_info->stbc[9]);		\
		\
		/* LDPC and TxBF counts */			\
		qdf_debug("LDPC TXBF Counts: %d, %d",		\
				_rx_phy_info->ldpc, _rx_phy_info->txbf);\
		/* RSSI Values for last received frames */	\
		qdf_debug("RSSI (data, mgmt): %d, %d", _rx_phy_info->data_rssi,\
				_rx_phy_info->mgmt_rssi);		\
		\
		qdf_debug("RSSI Chain 0 (0x%02x 0x%02x 0x%02x 0x%02x)",\
				((_rx_phy_info->rssi_chain0 >> 24) & 0xff),\
				((_rx_phy_info->rssi_chain0 >> 16) & 0xff),\
				((_rx_phy_info->rssi_chain0 >> 8) & 0xff),\
				((_rx_phy_info->rssi_chain0 >> 0) & 0xff));\
		\
		qdf_debug("RSSI Chain 1 (0x%02x 0x%02x 0x%02x 0x%02x)",\
				((_rx_phy_info->rssi_chain1 >> 24) & 0xff),\
				((_rx_phy_info->rssi_chain1 >> 16) & 0xff),\
				((_rx_phy_info->rssi_chain1 >> 8) & 0xff),\
				((_rx_phy_info->rssi_chain1 >> 0) & 0xff));\
		\
		qdf_debug("RSSI Chain 2 (0x%02x 0x%02x 0x%02x 0x%02x)",\
				((_rx_phy_info->rssi_chain2 >> 24) & 0xff),\
				((_rx_phy_info->rssi_chain2 >> 16) & 0xff),\
				((_rx_phy_info->rssi_chain2 >> 8) & 0xff),\
				((_rx_phy_info->rssi_chain2 >> 0) & 0xff));\
	} while (0)

static void htt_t2h_stats_rx_rate_stats_print(wlan_dbg_rx_rate_info_t *
					      rx_phy_info, int concise)
{
	rx_rate_stats_print_cmn(rx_phy_info, concise);
}

static void htt_t2h_stats_rx_rate_stats_print_v2(wlan_dbg_rx_rate_info_v2_t *
					      rx_phy_info, int concise)
{
	rx_rate_stats_print_cmn(rx_phy_info, concise);
}

static void
htt_t2h_stats_pdev_stats_print(struct wlan_dbg_stats *wlan_pdev_stats,
			       int concise)
{
	struct wlan_dbg_tx_stats *tx = &wlan_pdev_stats->tx;
	struct wlan_dbg_rx_stats *rx = &wlan_pdev_stats->rx;

	qdf_debug("WAL Pdev stats:");
	qdf_debug("\n### Tx ###");

	/* Num HTT cookies queued to dispatch list */
	qdf_debug("comp_queued       :\t%d", tx->comp_queued);
	/* Num HTT cookies dispatched */
	qdf_debug("comp_delivered    :\t%d", tx->comp_delivered);
	/* Num MSDU queued to WAL */
	qdf_debug("msdu_enqued       :\t%d", tx->msdu_enqued);
	/* Num MPDU queued to WAL */
	qdf_debug("mpdu_enqued       :\t%d", tx->mpdu_enqued);
	/* Num MSDUs dropped by WMM limit */
	qdf_debug("wmm_drop          :\t%d", tx->wmm_drop);
	/* Num Local frames queued */
	qdf_debug("local_enqued      :\t%d", tx->local_enqued);
	/* Num Local frames done */
	qdf_debug("local_freed       :\t%d", tx->local_freed);
	/* Num queued to HW */
	qdf_debug("hw_queued         :\t%d", tx->hw_queued);
	/* Num PPDU reaped from HW */
	qdf_debug("hw_reaped         :\t%d", tx->hw_reaped);
	/* Num underruns */
	qdf_debug("mac underrun      :\t%d", tx->underrun);
	/* Num underruns */
	qdf_debug("phy underrun      :\t%d", tx->phy_underrun);
	/* Num PPDUs cleaned up in TX abort */
	qdf_debug("tx_abort          :\t%d", tx->tx_abort);
	/* Num MPDUs requed by SW */
	qdf_debug("mpdus_requed      :\t%d", tx->mpdus_requed);
	/* Excessive retries */
	qdf_debug("excess retries    :\t%d", tx->tx_ko);
	/* last data rate */
	qdf_debug("last rc           :\t%d", tx->data_rc);
	/* scheduler self triggers */
	qdf_debug("sched self trig   :\t%d", tx->self_triggers);
	/* SW retry failures */
	qdf_debug("ampdu retry failed:\t%d", tx->sw_retry_failure);
	/* ilegal phy rate errirs */
	qdf_debug("illegal rate errs :\t%d", tx->illgl_rate_phy_err);
	/* pdev continous excessive retries  */
	qdf_debug("pdev cont xretry  :\t%d", tx->pdev_cont_xretry);
	/* pdev continous excessive retries  */
	qdf_debug("pdev tx timeout   :\t%d", tx->pdev_tx_timeout);
	/* pdev resets  */
	qdf_debug("pdev resets       :\t%d", tx->pdev_resets);
	/* PPDU > txop duration  */
	qdf_debug("ppdu txop ovf     :\t%d", tx->txop_ovf);

	qdf_debug("\n### Rx ###\n");
	/* Cnts any change in ring routing mid-ppdu */
	qdf_debug("ppdu_route_change :\t%d", rx->mid_ppdu_route_change);
	/* Total number of statuses processed */
	qdf_debug("status_rcvd       :\t%d", rx->status_rcvd);
	/* Extra frags on rings 0-3 */
	qdf_debug("r0_frags          :\t%d", rx->r0_frags);
	qdf_debug("r1_frags          :\t%d", rx->r1_frags);
	qdf_debug("r2_frags          :\t%d", rx->r2_frags);
	qdf_debug("r3_frags          :\t%d", rx->r3_frags);
	/* MSDUs / MPDUs delivered to HTT */
	qdf_debug("htt_msdus         :\t%d", rx->htt_msdus);
	qdf_debug("htt_mpdus         :\t%d", rx->htt_mpdus);
	/* MSDUs / MPDUs delivered to local stack */
	qdf_debug("loc_msdus         :\t%d", rx->loc_msdus);
	qdf_debug("loc_mpdus         :\t%d", rx->loc_mpdus);
	/* AMSDUs that have more MSDUs than the status ring size */
	qdf_debug("oversize_amsdu    :\t%d", rx->oversize_amsdu);
	/* Number of PHY errors */
	qdf_debug("phy_errs          :\t%d", rx->phy_errs);
	/* Number of PHY errors dropped */
	qdf_debug("phy_errs dropped  :\t%d", rx->phy_err_drop);
	/* Number of mpdu errors - FCS, MIC, ENC etc. */
	qdf_debug("mpdu_errs         :\t%d", rx->mpdu_errs);

}

static void
htt_t2h_stats_rx_reorder_stats_print(struct rx_reorder_stats *stats_ptr,
				     int concise)
{
	qdf_debug("Rx reorder statistics:");
	qdf_debug("  %u non-QoS frames received", stats_ptr->deliver_non_qos);
	qdf_debug("  %u frames received in-order",
		  stats_ptr->deliver_in_order);
	qdf_debug("  %u frames flushed due to timeout",
		  stats_ptr->deliver_flush_timeout);
	qdf_debug("  %u frames flushed due to moving out of window",
		  stats_ptr->deliver_flush_oow);
	qdf_debug("  %u frames flushed due to receiving DELBA",
		  stats_ptr->deliver_flush_delba);
	qdf_debug("  %u frames discarded due to FCS error",
		  stats_ptr->fcs_error);
	qdf_debug("  %u frames discarded due to invalid peer",
		  stats_ptr->invalid_peer);
	qdf_debug
		("  %u frames discarded due to duplication (non aggregation)",
		stats_ptr->dup_non_aggr);
	qdf_debug("  %u frames discarded due to duplication in reorder queue",
		 stats_ptr->dup_in_reorder);
	qdf_debug("  %u frames discarded due to processed before",
		  stats_ptr->dup_past);
	qdf_debug("  %u times reorder timeout happened",
		  stats_ptr->reorder_timeout);
	qdf_debug("  %u times incorrect bar received",
		  stats_ptr->invalid_bar_ssn);
	qdf_debug("  %u times bar ssn reset happened",
			stats_ptr->ssn_reset);
	qdf_debug("  %u times flushed due to peer delete",
			stats_ptr->deliver_flush_delpeer);
	qdf_debug("  %u times flushed due to offload",
			stats_ptr->deliver_flush_offload);
	qdf_debug("  %u times flushed due to ouf of buffer",
			stats_ptr->deliver_flush_oob);
	qdf_debug("  %u MPDU's dropped due to PN check fail",
			stats_ptr->pn_fail);
	qdf_debug("  %u MPDU's dropped due to lack of memory",
			stats_ptr->store_fail);
	qdf_debug("  %u times tid pool alloc succeeded",
			stats_ptr->tid_pool_alloc_succ);
	qdf_debug("  %u times MPDU pool alloc succeeded",
			stats_ptr->mpdu_pool_alloc_succ);
	qdf_debug("  %u times MSDU pool alloc succeeded",
			stats_ptr->msdu_pool_alloc_succ);
	qdf_debug("  %u times tid pool alloc failed",
			stats_ptr->tid_pool_alloc_fail);
	qdf_debug("  %u times MPDU pool alloc failed",
			stats_ptr->mpdu_pool_alloc_fail);
	qdf_debug("  %u times MSDU pool alloc failed",
			stats_ptr->msdu_pool_alloc_fail);
	qdf_debug("  %u times tid pool freed",
			stats_ptr->tid_pool_free);
	qdf_debug("  %u times MPDU pool freed",
			stats_ptr->mpdu_pool_free);
	qdf_debug("  %u times MSDU pool freed",
			stats_ptr->msdu_pool_free);
	qdf_debug("  %u MSDUs undelivered to HTT, queued to Rx MSDU free list",
			stats_ptr->msdu_queued);
	qdf_debug("  %u MSDUs released from Rx MSDU list to MAC ring",
			stats_ptr->msdu_recycled);
	qdf_debug("  %u MPDUs with invalid peer but A2 found in AST",
			stats_ptr->invalid_peer_a2_in_ast);
	qdf_debug("  %u MPDUs with invalid peer but A3 found in AST",
			stats_ptr->invalid_peer_a3_in_ast);
	qdf_debug("  %u MPDUs with invalid peer, Broadcast or Mulitcast frame",
			stats_ptr->invalid_peer_bmc_mpdus);
	qdf_debug("  %u MSDUs with err attention word",
			stats_ptr->rxdesc_err_att);
	qdf_debug("  %u MSDUs with flag of peer_idx_invalid",
			stats_ptr->rxdesc_err_peer_idx_inv);
	qdf_debug("  %u MSDUs with  flag of peer_idx_timeout",
			stats_ptr->rxdesc_err_peer_idx_to);
	qdf_debug("  %u MSDUs with  flag of overflow",
			stats_ptr->rxdesc_err_ov);
	qdf_debug("  %u MSDUs with  flag of msdu_length_err",
			stats_ptr->rxdesc_err_msdu_len);
	qdf_debug("  %u MSDUs with  flag of mpdu_length_err",
			stats_ptr->rxdesc_err_mpdu_len);
	qdf_debug("  %u MSDUs with  flag of tkip_mic_err",
			stats_ptr->rxdesc_err_tkip_mic);
	qdf_debug("  %u MSDUs with  flag of decrypt_err",
			stats_ptr->rxdesc_err_decrypt);
	qdf_debug("  %u MSDUs with  flag of fcs_err",
			stats_ptr->rxdesc_err_fcs);
	qdf_debug("  %u Unicast frames with invalid peer handler",
			stats_ptr->rxdesc_uc_msdus_inv_peer);
	qdf_debug("  %u unicast frame to DUT with invalid peer handler",
			stats_ptr->rxdesc_direct_msdus_inv_peer);
	qdf_debug("  %u Broadcast/Multicast frames with invalid peer handler",
			stats_ptr->rxdesc_bmc_msdus_inv_peer);
	qdf_debug("  %u MSDUs dropped due to no first MSDU flag",
			stats_ptr->rxdesc_no_1st_msdu);
	qdf_debug("  %u MSDUs dropped due to ring overflow",
			stats_ptr->msdu_drop_ring_ov);
	qdf_debug("  %u MSDUs dropped due to FC mismatch",
			stats_ptr->msdu_drop_fc_mismatch);
	qdf_debug("  %u MSDUs dropped due to mgt frame in Remote ring",
			stats_ptr->msdu_drop_mgmt_remote_ring);
	qdf_debug("  %u MSDUs dropped due to misc non error",
			stats_ptr->msdu_drop_misc);
	qdf_debug("  %u MSDUs go to offload before reorder",
			stats_ptr->offload_msdu_wal);
	qdf_debug("  %u data frame dropped by offload after reorder",
			stats_ptr->offload_msdu_reorder);
	qdf_debug("  %u  MPDUs with SN in the past & within BA window",
			stats_ptr->dup_past_within_window);
	qdf_debug("  %u  MPDUs with SN in the past & outside BA window",
			stats_ptr->dup_past_outside_window);
}

static void
htt_t2h_stats_rx_rem_buf_stats_print(
	struct rx_remote_buffer_mgmt_stats *stats_ptr, int concise)
{
	qdf_debug("Rx Remote Buffer Statistics:");
	qdf_debug("  %u MSDU's reaped for Rx processing",
			stats_ptr->remote_reaped);
	qdf_debug("  %u MSDU's recycled within firmware",
			stats_ptr->remote_recycled);
	qdf_debug("  %u MSDU's stored by Data Rx",
			stats_ptr->data_rx_msdus_stored);
	qdf_debug("  %u HTT indications from WAL Rx MSDU",
			stats_ptr->wal_rx_ind);
	qdf_debug("  %u HTT indications unconsumed from WAL Rx MSDU",
			stats_ptr->wal_rx_ind_unconsumed);
	qdf_debug("  %u HTT indications from Data Rx MSDU",
			stats_ptr->data_rx_ind);
	qdf_debug("  %u HTT indications unconsumed from Data Rx MSDU",
			stats_ptr->data_rx_ind_unconsumed);
	qdf_debug("  %u HTT indications from ATHBUF",
			stats_ptr->athbuf_rx_ind);
	qdf_debug("  %u Remote buffers requested for refill",
			stats_ptr->refill_buf_req);
	qdf_debug("  %u Remote buffers filled by host",
			stats_ptr->refill_buf_rsp);
	qdf_debug("  %u times MAC has no buffers",
			stats_ptr->mac_no_bufs);
	qdf_debug("  %u times f/w write & read indices on MAC ring are equal",
			stats_ptr->fw_indices_equal);
	qdf_debug("  %u times f/w has no remote buffers to post to MAC",
			stats_ptr->host_no_bufs);
}

static void
htt_t2h_stats_txbf_info_buf_stats_print(
	struct wlan_dbg_txbf_data_stats *stats_ptr)
{
	qdf_debug("TXBF data Statistics:");
	qdf_debug("tx_txbf_vht (0..9): %u, %u, %u, %u, %u, %u, %u, %u, %u, %d",
		  stats_ptr->tx_txbf_vht[0],
		  stats_ptr->tx_txbf_vht[1],
		  stats_ptr->tx_txbf_vht[2],
		  stats_ptr->tx_txbf_vht[3],
		  stats_ptr->tx_txbf_vht[4],
		  stats_ptr->tx_txbf_vht[5],
		  stats_ptr->tx_txbf_vht[6],
		  stats_ptr->tx_txbf_vht[7],
		  stats_ptr->tx_txbf_vht[8],
		  stats_ptr->tx_txbf_vht[9]);
	qdf_debug("rx_txbf_vht (0..9): %u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
		  stats_ptr->rx_txbf_vht[0],
		  stats_ptr->rx_txbf_vht[1],
		  stats_ptr->rx_txbf_vht[2],
		  stats_ptr->rx_txbf_vht[3],
		  stats_ptr->rx_txbf_vht[4],
		  stats_ptr->rx_txbf_vht[5],
		  stats_ptr->rx_txbf_vht[6],
		  stats_ptr->rx_txbf_vht[7],
		  stats_ptr->rx_txbf_vht[8],
		  stats_ptr->rx_txbf_vht[9]);
	qdf_debug("tx_txbf_ht (0..7): %u, %u, %u, %u, %u, %u, %u, %u",
		  stats_ptr->tx_txbf_ht[0],
		  stats_ptr->tx_txbf_ht[1],
		  stats_ptr->tx_txbf_ht[2],
		  stats_ptr->tx_txbf_ht[3],
		  stats_ptr->tx_txbf_ht[4],
		  stats_ptr->tx_txbf_ht[5],
		  stats_ptr->tx_txbf_ht[6],
		  stats_ptr->tx_txbf_ht[7]);
	qdf_debug("tx_txbf_ofdm (0..7): %u, %u, %u, %u, %u, %u, %u, %u",
		  stats_ptr->tx_txbf_ofdm[0],
		  stats_ptr->tx_txbf_ofdm[1],
		  stats_ptr->tx_txbf_ofdm[2],
		  stats_ptr->tx_txbf_ofdm[3],
		  stats_ptr->tx_txbf_ofdm[4],
		  stats_ptr->tx_txbf_ofdm[5],
		  stats_ptr->tx_txbf_ofdm[6],
		  stats_ptr->tx_txbf_ofdm[7]);
	qdf_debug("tx_txbf_cck (0..6): %u, %u, %u, %u, %u, %u, %u",
		  stats_ptr->tx_txbf_cck[0],
		  stats_ptr->tx_txbf_cck[1],
		  stats_ptr->tx_txbf_cck[2],
		  stats_ptr->tx_txbf_cck[3],
		  stats_ptr->tx_txbf_cck[4],
		  stats_ptr->tx_txbf_cck[5],
		  stats_ptr->tx_txbf_cck[6]);
}

static void
htt_t2h_stats_txbf_snd_buf_stats_print(
	struct wlan_dbg_txbf_snd_stats *stats_ptr)
{
	qdf_debug("TXBF snd Buffer Statistics:");
	qdf_debug("cbf_20: %u, %u, %u, %u",
		  stats_ptr->cbf_20[0],
		  stats_ptr->cbf_20[1],
		  stats_ptr->cbf_20[2],
		  stats_ptr->cbf_20[3]);
	qdf_debug("cbf_40: %u, %u, %u, %u",
		  stats_ptr->cbf_40[0],
		  stats_ptr->cbf_40[1],
		  stats_ptr->cbf_40[2],
		  stats_ptr->cbf_40[3]);
	qdf_debug("cbf_80: %u, %u, %u, %u",
		  stats_ptr->cbf_80[0],
		  stats_ptr->cbf_80[1],
		  stats_ptr->cbf_80[2],
		  stats_ptr->cbf_80[3]);
	qdf_debug("sounding: %u, %u, %u, %u, %u, %u, %u, %u, %u",
		  stats_ptr->sounding[0],
		  stats_ptr->sounding[1],
		  stats_ptr->sounding[2],
		  stats_ptr->sounding[3],
		  stats_ptr->sounding[4],
		  stats_ptr->sounding[5],
		  stats_ptr->sounding[6],
		  stats_ptr->sounding[7],
		  stats_ptr->sounding[8]);
}

static void
htt_t2h_stats_tx_selfgen_buf_stats_print(
	struct wlan_dbg_tx_selfgen_stats *stats_ptr)
{
	qdf_debug("Tx selfgen Buffer Statistics:");
	qdf_debug("  %u su_ndpa",
			stats_ptr->su_ndpa);
	qdf_debug("  %u mu_ndp",
			stats_ptr->mu_ndp);
	qdf_debug("  %u mu_ndpa",
			stats_ptr->mu_ndpa);
	qdf_debug("  %u mu_ndp",
			stats_ptr->mu_ndp);
	qdf_debug("  %u mu_brpoll_1",
			stats_ptr->mu_brpoll_1);
	qdf_debug("  %u mu_brpoll_2",
			stats_ptr->mu_brpoll_2);
	qdf_debug("  %u mu_bar_1",
			stats_ptr->mu_bar_1);
	qdf_debug("  %u mu_bar_2",
			stats_ptr->mu_bar_2);
	qdf_debug("  %u cts_burst",
			stats_ptr->cts_burst);
	qdf_debug("  %u su_ndp_err",
			stats_ptr->su_ndp_err);
	qdf_debug("  %u su_ndpa_err",
			stats_ptr->su_ndpa_err);
	qdf_debug("  %u mu_ndp_err",
			stats_ptr->mu_ndp_err);
	qdf_debug("  %u mu_brp1_err",
			stats_ptr->mu_brp1_err);
	qdf_debug("  %u mu_brp2_err",
			stats_ptr->mu_brp2_err);
}

static void
htt_t2h_stats_wifi2_error_stats_print(
	struct wlan_dbg_wifi2_error_stats *stats_ptr)
{
	int i;

	qdf_debug("Scheduler error Statistics:");
	qdf_debug("urrn_stats: ");
	qdf_debug("urrn_stats: %d, %d, %d",
		  stats_ptr->urrn_stats[0],
		  stats_ptr->urrn_stats[1],
		  stats_ptr->urrn_stats[2]);
	qdf_debug("flush_errs (0..%d): ",
			WHAL_DBG_FLUSH_REASON_MAXCNT);
	for (i = 0; i < WHAL_DBG_FLUSH_REASON_MAXCNT; i++)
		qdf_debug("  %u", stats_ptr->flush_errs[i]);
	qdf_debug("\n");
	qdf_debug("schd_stall_errs (0..3): ");
	qdf_debug("%d, %d, %d, %d",
		  stats_ptr->schd_stall_errs[0],
		  stats_ptr->schd_stall_errs[1],
		  stats_ptr->schd_stall_errs[2],
		  stats_ptr->schd_stall_errs[3]);
	qdf_debug("schd_cmd_result (0..%d): ",
			WHAL_DBG_CMD_RESULT_MAXCNT);
	for (i = 0; i < WHAL_DBG_CMD_RESULT_MAXCNT; i++)
		qdf_debug("  %u", stats_ptr->schd_cmd_result[i]);
	qdf_debug("\n");
	qdf_debug("sifs_status (0..%d): ",
			WHAL_DBG_SIFS_STATUS_MAXCNT);
	for (i = 0; i < WHAL_DBG_SIFS_STATUS_MAXCNT; i++)
		qdf_debug("  %u", stats_ptr->sifs_status[i]);
	qdf_debug("\n");
	qdf_debug("phy_errs (0..%d): ",
			WHAL_DBG_PHY_ERR_MAXCNT);
	for (i = 0; i < WHAL_DBG_PHY_ERR_MAXCNT; i++)
		qdf_debug("  %u", stats_ptr->phy_errs[i]);
	qdf_debug("\n");
	qdf_debug("  %u rx_rate_inval",
			stats_ptr->rx_rate_inval);
}

static void
htt_t2h_rx_musu_ndpa_pkts_stats_print(
	struct rx_txbf_musu_ndpa_pkts_stats *stats_ptr)
{
	qdf_debug("Rx TXBF MU/SU Packets and NDPA Statistics:");
	qdf_debug("  %u Number of TXBF MU packets received",
			stats_ptr->number_mu_pkts);
	qdf_debug("  %u Number of TXBF SU packets received",
			stats_ptr->number_su_pkts);
	qdf_debug("  %u Number of TXBF directed NDPA",
			stats_ptr->txbf_directed_ndpa_count);
	qdf_debug("  %u Number of TXBF retried NDPA",
			stats_ptr->txbf_ndpa_retry_count);
	qdf_debug("  %u Total number of TXBF NDPA",
			stats_ptr->txbf_total_ndpa_count);
}

#define HTT_TICK_TO_USEC(ticks, microsec_per_tick) (ticks * microsec_per_tick)
static inline int htt_rate_flags_to_mhz(uint8_t rate_flags)
{
	if (rate_flags & 0x20)
		return 40;      /* WHAL_RC_FLAG_40MHZ */
	if (rate_flags & 0x40)
		return 80;      /* WHAL_RC_FLAG_80MHZ */
	if (rate_flags & 0x80)
		return 160;     /* WHAL_RC_FLAG_160MHZ */
	return 20;
}

#define HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW 64

static void
htt_t2h_tx_ppdu_bitmaps_pr(uint32_t *queued_ptr, uint32_t *acked_ptr)
{
	char queued_str[HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW + 1];
	char acked_str[HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW + 1];
	int i, j, word;

	qdf_mem_set(queued_str, HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW, '0');
	qdf_mem_set(acked_str, HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW, '-');
	i = 0;
	for (word = 0; word < 2; word++) {
		uint32_t queued = *(queued_ptr + word);
		uint32_t acked = *(acked_ptr + word);

		for (j = 0; j < 32; j++, i++) {
			if (queued & (1 << j)) {
				queued_str[i] = '1';
				acked_str[i] = (acked & (1 << j)) ? 'y' : 'N';
			}
		}
	}
	queued_str[HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW] = '\0';
	acked_str[HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW] = '\0';
	qdf_debug("%s\n", queued_str);
	qdf_debug("%s\n", acked_str);
}

static inline uint16_t htt_msg_read16(uint16_t *p16)
{
#ifdef BIG_ENDIAN_HOST
	/*
	 * During upload, the bytes within each uint32_t word were
	 * swapped by the HIF HW.  This results in the lower and upper bytes
	 * of each uint16_t to be in the correct big-endian order with
	 * respect to each other, but for each even-index uint16_t to
	 * have its position switched with its successor neighbor uint16_t.
	 * Undo this uint16_t position swapping.
	 */
	return (((size_t) p16) & 0x2) ? *(p16 - 1) : *(p16 + 1);
#else
	return *p16;
#endif
}

static inline uint8_t htt_msg_read8(uint8_t *p8)
{
#ifdef BIG_ENDIAN_HOST
	/*
	 * During upload, the bytes within each uint32_t word were
	 * swapped by the HIF HW.
	 * Undo this byte swapping.
	 */
	switch (((size_t) p8) & 0x3) {
	case 0:
		return *(p8 + 3);
	case 1:
		return *(p8 + 1);
	case 2:
		return *(p8 - 1);
	default /* 3 */:
		return *(p8 - 3);
	}
#else
	return *p8;
#endif
}

static void htt_make_u8_list_str(uint32_t *aligned_data,
				 char *buffer, int space, int max_elems)
{
	uint8_t *p8 = (uint8_t *) aligned_data;
	char *buf_p = buffer;

	while (max_elems-- > 0) {
		int bytes;
		uint8_t val;

		val = htt_msg_read8(p8);
		if (val == 0)
			/* not enough data to fill the reserved msg buffer*/
			break;

		bytes = qdf_snprint(buf_p, space, "%d,", val);
		space -= bytes;
		if (space > 0)
			buf_p += bytes;
		else /* not enough print buffer space for all the data */
			break;
		p8++;
	}
	if (buf_p == buffer)
		*buf_p = '\0';        /* nothing was written */
	else
		*(buf_p - 1) = '\0';  /* erase the final comma */

}

static void htt_make_u16_list_str(uint32_t *aligned_data,
				  char *buffer, int space, int max_elems)
{
	uint16_t *p16 = (uint16_t *) aligned_data;
	char *buf_p = buffer;

	while (max_elems-- > 0) {
		int bytes;
		uint16_t val;

		val = htt_msg_read16(p16);
		if (val == 0)
			/* not enough data to fill the reserved msg buffer */
			break;
		bytes = qdf_snprint(buf_p, space, "%d,", val);
		space -= bytes;
		if (space > 0)
			buf_p += bytes;
		else /* not enough print buffer space for all the data */
			break;

		p16++;
	}
	if (buf_p == buffer)
		*buf_p = '\0';  /* nothing was written */
	else
		*(buf_p - 1) = '\0';    /* erase the final comma */
}

static void
htt_t2h_tx_ppdu_log_print(struct ol_fw_tx_dbg_ppdu_msg_hdr *hdr,
			  struct ol_fw_tx_dbg_ppdu_base *record,
			  int length, int concise)
{
	int i;
	int record_size;
	int num_records;

	record_size =
		sizeof(*record) +
		hdr->mpdu_bytes_array_len * sizeof(uint16_t) +
		hdr->mpdu_msdus_array_len * sizeof(uint8_t) +
		hdr->msdu_bytes_array_len * sizeof(uint16_t);
	num_records = (length - sizeof(*hdr)) / record_size;
	qdf_debug("Tx PPDU log elements: num_records %d", num_records);

	for (i = 0; i < num_records; i++) {
		uint16_t start_seq_num;
		uint16_t start_pn_lsbs;
		uint8_t num_mpdus;
		uint16_t peer_id;
		uint8_t ext_tid;
		uint8_t rate_code;
		uint8_t rate_flags;
		uint8_t tries;
		uint8_t complete;
		uint32_t time_enqueue_us;
		uint32_t time_completion_us;
		uint32_t *msg_word = (uint32_t *) record;

		/* fields used for both concise and complete printouts */
		start_seq_num =
			((*(msg_word + OL_FW_TX_DBG_PPDU_START_SEQ_NUM_WORD)) &
			 OL_FW_TX_DBG_PPDU_START_SEQ_NUM_M) >>
			OL_FW_TX_DBG_PPDU_START_SEQ_NUM_S;
		complete =
			((*(msg_word + OL_FW_TX_DBG_PPDU_COMPLETE_WORD)) &
			 OL_FW_TX_DBG_PPDU_COMPLETE_M) >>
			OL_FW_TX_DBG_PPDU_COMPLETE_S;

		/* fields used only for complete printouts */
		if (!concise) {
#define BUF_SIZE 80
			char buf[BUF_SIZE];
			uint8_t *p8;

			time_enqueue_us =
				HTT_TICK_TO_USEC(record->timestamp_enqueue,
						 hdr->microsec_per_tick);
			time_completion_us =
				HTT_TICK_TO_USEC(record->timestamp_completion,
						 hdr->microsec_per_tick);

			start_pn_lsbs =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_START_PN_LSBS_WORD)) &
				OL_FW_TX_DBG_PPDU_START_PN_LSBS_M) >>
				OL_FW_TX_DBG_PPDU_START_PN_LSBS_S;
			num_mpdus =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_NUM_MPDUS_WORD))&
				OL_FW_TX_DBG_PPDU_NUM_MPDUS_M) >>
				OL_FW_TX_DBG_PPDU_NUM_MPDUS_S;
			peer_id =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_PEER_ID_WORD)) &
				OL_FW_TX_DBG_PPDU_PEER_ID_M) >>
				OL_FW_TX_DBG_PPDU_PEER_ID_S;
			ext_tid =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_EXT_TID_WORD)) &
				OL_FW_TX_DBG_PPDU_EXT_TID_M) >>
				OL_FW_TX_DBG_PPDU_EXT_TID_S;
			rate_code =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_RATE_CODE_WORD))&
				OL_FW_TX_DBG_PPDU_RATE_CODE_M) >>
				OL_FW_TX_DBG_PPDU_RATE_CODE_S;
			rate_flags =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_RATE_FLAGS_WORD))&
				OL_FW_TX_DBG_PPDU_RATE_FLAGS_M) >>
				OL_FW_TX_DBG_PPDU_RATE_FLAGS_S;
			tries =
				((*(msg_word +
				OL_FW_TX_DBG_PPDU_TRIES_WORD)) &
				OL_FW_TX_DBG_PPDU_TRIES_M) >>
				OL_FW_TX_DBG_PPDU_TRIES_S;

			qdf_debug(" - PPDU tx to peer %d, TID %d", peer_id,
				  ext_tid);
			qdf_debug("   start seq num= %u, start PN LSBs= %#04x",
				start_seq_num, start_pn_lsbs);
			qdf_debug("   PPDU: %d MPDUs, (?) MSDUs, %d bytes",
				num_mpdus,
				 /* num_msdus - not yet computed in target */
				record->num_bytes);
			if (complete) {
				qdf_debug("   enqueued: %u, completed: %u usec)",
				       time_enqueue_us, time_completion_us);
				qdf_debug("   %d tries, last tx used rate %d ",
					tries, rate_code);
				qdf_debug("on %d MHz chan (flags = %#x)",
					htt_rate_flags_to_mhz
					(rate_flags), rate_flags);
				qdf_debug("  enqueued and acked MPDU bitmaps:");
				htt_t2h_tx_ppdu_bitmaps_pr(msg_word +
					OL_FW_TX_DBG_PPDU_ENQUEUED_LSBS_WORD,
					msg_word +
					OL_FW_TX_DBG_PPDU_BLOCK_ACK_LSBS_WORD);
			} else {
				qdf_debug("  enqueued: %d us, not yet completed",
					time_enqueue_us);
			}
			/* skip the regular msg fields to reach the tail area */
			p8 = (uint8_t *) record;
			p8 += sizeof(struct ol_fw_tx_dbg_ppdu_base);
			if (hdr->mpdu_bytes_array_len) {
				htt_make_u16_list_str((uint32_t *) p8, buf,
						      BUF_SIZE,
						      hdr->
						      mpdu_bytes_array_len);
				qdf_debug("   MPDU bytes: %s", buf);
			}
			p8 += hdr->mpdu_bytes_array_len * sizeof(uint16_t);
			if (hdr->mpdu_msdus_array_len) {
				htt_make_u8_list_str((uint32_t *) p8, buf,
						     BUF_SIZE,
						     hdr->mpdu_msdus_array_len);
				qdf_debug("   MPDU MSDUs: %s", buf);
			}
			p8 += hdr->mpdu_msdus_array_len * sizeof(uint8_t);
			if (hdr->msdu_bytes_array_len) {
				htt_make_u16_list_str((uint32_t *) p8, buf,
						      BUF_SIZE,
						      hdr->
						      msdu_bytes_array_len);
				qdf_debug("   MSDU bytes: %s", buf);
			}
		} else {
			/* concise */
			qdf_debug("start seq num = %u ", start_seq_num);
			qdf_debug("enqueued and acked MPDU bitmaps:");
			if (complete) {
				htt_t2h_tx_ppdu_bitmaps_pr(msg_word +
					OL_FW_TX_DBG_PPDU_ENQUEUED_LSBS_WORD,
							   msg_word +
					OL_FW_TX_DBG_PPDU_BLOCK_ACK_LSBS_WORD);
			} else {
				qdf_debug("(not completed)");
			}
		}
		record = (struct ol_fw_tx_dbg_ppdu_base *)
			 (((uint8_t *) record) + record_size);
	}
}

static void htt_t2h_stats_tidq_stats_print(
	struct wlan_dbg_tidq_stats *tidq_stats, int concise)
{
	qdf_debug("TID QUEUE STATS:");
	qdf_debug("tid_txq_stats: %u", tidq_stats->wlan_dbg_tid_txq_status);
	qdf_debug("num_pkts_queued(0..9):");
	qdf_debug("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
		tidq_stats->txq_st.num_pkts_queued[0],
		tidq_stats->txq_st.num_pkts_queued[1],
		tidq_stats->txq_st.num_pkts_queued[2],
		tidq_stats->txq_st.num_pkts_queued[3],
		tidq_stats->txq_st.num_pkts_queued[4],
		tidq_stats->txq_st.num_pkts_queued[5],
		tidq_stats->txq_st.num_pkts_queued[6],
		tidq_stats->txq_st.num_pkts_queued[7],
		tidq_stats->txq_st.num_pkts_queued[8],
		tidq_stats->txq_st.num_pkts_queued[9]);
	qdf_debug("tid_hw_qdepth(0..19):");
	qdf_debug("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
		tidq_stats->txq_st.tid_hw_qdepth[0],
		tidq_stats->txq_st.tid_hw_qdepth[1],
		tidq_stats->txq_st.tid_hw_qdepth[2],
		tidq_stats->txq_st.tid_hw_qdepth[3],
		tidq_stats->txq_st.tid_hw_qdepth[4],
		tidq_stats->txq_st.tid_hw_qdepth[5],
		tidq_stats->txq_st.tid_hw_qdepth[6],
		tidq_stats->txq_st.tid_hw_qdepth[7],
		tidq_stats->txq_st.tid_hw_qdepth[8],
		tidq_stats->txq_st.tid_hw_qdepth[9]);
	qdf_debug("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
		tidq_stats->txq_st.tid_hw_qdepth[10],
		tidq_stats->txq_st.tid_hw_qdepth[11],
		tidq_stats->txq_st.tid_hw_qdepth[12],
		tidq_stats->txq_st.tid_hw_qdepth[13],
		tidq_stats->txq_st.tid_hw_qdepth[14],
		tidq_stats->txq_st.tid_hw_qdepth[15],
		tidq_stats->txq_st.tid_hw_qdepth[16],
		tidq_stats->txq_st.tid_hw_qdepth[17],
		tidq_stats->txq_st.tid_hw_qdepth[18],
		tidq_stats->txq_st.tid_hw_qdepth[19]);
	qdf_debug("tid_sw_qdepth(0..19):");
	qdf_debug("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
		tidq_stats->txq_st.tid_sw_qdepth[0],
		tidq_stats->txq_st.tid_sw_qdepth[1],
		tidq_stats->txq_st.tid_sw_qdepth[2],
		tidq_stats->txq_st.tid_sw_qdepth[3],
		tidq_stats->txq_st.tid_sw_qdepth[4],
		tidq_stats->txq_st.tid_sw_qdepth[5],
		tidq_stats->txq_st.tid_sw_qdepth[6],
		tidq_stats->txq_st.tid_sw_qdepth[7],
		tidq_stats->txq_st.tid_sw_qdepth[8],
		tidq_stats->txq_st.tid_sw_qdepth[9]);
	qdf_debug("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
		tidq_stats->txq_st.tid_sw_qdepth[10],
		tidq_stats->txq_st.tid_sw_qdepth[11],
		tidq_stats->txq_st.tid_sw_qdepth[12],
		tidq_stats->txq_st.tid_sw_qdepth[13],
		tidq_stats->txq_st.tid_sw_qdepth[14],
		tidq_stats->txq_st.tid_sw_qdepth[15],
		tidq_stats->txq_st.tid_sw_qdepth[16],
		tidq_stats->txq_st.tid_sw_qdepth[17],
		tidq_stats->txq_st.tid_sw_qdepth[18],
		tidq_stats->txq_st.tid_sw_qdepth[19]);
}

static void htt_t2h_stats_tx_mu_stats_print(
	struct wlan_dbg_tx_mu_stats *tx_mu_stats, int concise)
{
	qdf_debug("TX MU STATS:");
	qdf_debug("mu_sch_nusers_2: %u", tx_mu_stats->mu_sch_nusers_2);
	qdf_debug("mu_sch_nusers_3: %u", tx_mu_stats->mu_sch_nusers_3);
	qdf_debug("mu_mpdus_queued_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_mpdus_queued_usr[0],
		tx_mu_stats->mu_mpdus_queued_usr[1],
		tx_mu_stats->mu_mpdus_queued_usr[2],
		tx_mu_stats->mu_mpdus_queued_usr[3]);
	qdf_debug("mu_mpdus_tried_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_mpdus_tried_usr[0],
		tx_mu_stats->mu_mpdus_tried_usr[1],
		tx_mu_stats->mu_mpdus_tried_usr[2],
		tx_mu_stats->mu_mpdus_tried_usr[3]);
	qdf_debug("mu_mpdus_failed_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_mpdus_failed_usr[0],
		tx_mu_stats->mu_mpdus_failed_usr[1],
		tx_mu_stats->mu_mpdus_failed_usr[2],
		tx_mu_stats->mu_mpdus_failed_usr[3]);
	qdf_debug("mu_mpdus_requeued_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_mpdus_requeued_usr[0],
		tx_mu_stats->mu_mpdus_requeued_usr[1],
		tx_mu_stats->mu_mpdus_requeued_usr[2],
		tx_mu_stats->mu_mpdus_requeued_usr[3]);
	qdf_debug("mu_err_no_ba_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_err_no_ba_usr[0],
		tx_mu_stats->mu_err_no_ba_usr[1],
		tx_mu_stats->mu_err_no_ba_usr[2],
		tx_mu_stats->mu_err_no_ba_usr[3]);
	qdf_debug("mu_mpdu_underrun_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_mpdu_underrun_usr[0],
		tx_mu_stats->mu_mpdu_underrun_usr[1],
		tx_mu_stats->mu_mpdu_underrun_usr[2],
		tx_mu_stats->mu_mpdu_underrun_usr[3]);
	qdf_debug("mu_ampdu_underrun_usr: %u, %u, %u, %u",
		tx_mu_stats->mu_ampdu_underrun_usr[0],
		tx_mu_stats->mu_ampdu_underrun_usr[1],
		tx_mu_stats->mu_ampdu_underrun_usr[2],
		tx_mu_stats->mu_ampdu_underrun_usr[3]);

}

static void htt_t2h_stats_sifs_resp_stats_print(
	struct wlan_dbg_sifs_resp_stats *sifs_stats, int concise)
{
	qdf_debug("SIFS RESP STATS:");
	qdf_debug("num of ps-poll trigger frames: %u",
		sifs_stats->ps_poll_trigger);
	qdf_debug("num of uapsd trigger frames: %u",
		sifs_stats->uapsd_trigger);
	qdf_debug("num of data trigger frames: %u, %u",
		sifs_stats->qb_data_trigger[0],
		sifs_stats->qb_data_trigger[1]);
	qdf_debug("num of bar trigger frames: %u, %u",
		sifs_stats->qb_bar_trigger[0],
		sifs_stats->qb_bar_trigger[1]);
	qdf_debug("num of ppdu transmitted at SIFS interval: %u",
		sifs_stats->sifs_resp_data);
	qdf_debug("num of ppdu failed to meet SIFS resp timing: %u",
		sifs_stats->sifs_resp_err);
}

void htt_t2h_stats_print(uint8_t *stats_data, int concise)
{
	uint32_t *msg_word = (uint32_t *) stats_data;
	enum htt_dbg_stats_type type;
	enum htt_dbg_stats_status status;
	int length;

	type = HTT_T2H_STATS_CONF_TLV_TYPE_GET(*msg_word);
	status = HTT_T2H_STATS_CONF_TLV_STATUS_GET(*msg_word);
	length = HTT_T2H_STATS_CONF_TLV_LENGTH_GET(*msg_word);

	/* check that we've been given a valid stats type */
	if (status == HTT_DBG_STATS_STATUS_SERIES_DONE) {
		return;
	} else if (status == HTT_DBG_STATS_STATUS_INVALID) {
		qdf_debug("Target doesn't support stats type %d", type);
		return;
	} else if (status == HTT_DBG_STATS_STATUS_ERROR) {
		qdf_debug("Target couldn't upload stats type %d (no mem?)",
			  type);
		return;
	}
	/* got valid (though perhaps partial) stats - process them */
	switch (type) {
	case HTT_DBG_STATS_WAL_PDEV_TXRX:
	{
		struct wlan_dbg_stats *wlan_dbg_stats_ptr;

		wlan_dbg_stats_ptr =
			(struct wlan_dbg_stats *)(msg_word + 1);
		htt_t2h_stats_pdev_stats_print(wlan_dbg_stats_ptr,
					       concise);
		break;
	}
	case HTT_DBG_STATS_RX_REORDER:
	{
		struct rx_reorder_stats *rx_reorder_stats_ptr;

		rx_reorder_stats_ptr =
			(struct rx_reorder_stats *)(msg_word + 1);
		htt_t2h_stats_rx_reorder_stats_print
			(rx_reorder_stats_ptr, concise);
		break;
	}

	case HTT_DBG_STATS_RX_RATE_INFO:
	{
		wlan_dbg_rx_rate_info_t *rx_phy_info;

		rx_phy_info = (wlan_dbg_rx_rate_info_t *) (msg_word + 1);
		htt_t2h_stats_rx_rate_stats_print(rx_phy_info, concise);
		break;
	}
	case HTT_DBG_STATS_RX_RATE_INFO_V2:
	{
		wlan_dbg_rx_rate_info_v2_t *rx_phy_info;

		rx_phy_info = (wlan_dbg_rx_rate_info_v2_t *) (msg_word + 1);
		htt_t2h_stats_rx_rate_stats_print_v2(rx_phy_info, concise);
		break;
	}
	case HTT_DBG_STATS_TX_PPDU_LOG:
	{
		struct ol_fw_tx_dbg_ppdu_msg_hdr *hdr;
		struct ol_fw_tx_dbg_ppdu_base *record;

		if (status == HTT_DBG_STATS_STATUS_PARTIAL
		    && length == 0) {
			qdf_debug("HTT_DBG_STATS_TX_PPDU_LOG -- length = 0!");
			break;
		}
		hdr = (struct ol_fw_tx_dbg_ppdu_msg_hdr *)(msg_word + 1);
		record = (struct ol_fw_tx_dbg_ppdu_base *)(hdr + 1);
		htt_t2h_tx_ppdu_log_print(hdr, record, length, concise);
	}
	break;
	case HTT_DBG_STATS_TX_RATE_INFO:
	{
		wlan_dbg_tx_rate_info_t *tx_rate_info;

		tx_rate_info = (wlan_dbg_tx_rate_info_t *) (msg_word + 1);
		htt_t2h_stats_tx_rate_stats_print(tx_rate_info, concise);
		break;
	}
	case HTT_DBG_STATS_TX_RATE_INFO_V2:
	{
		wlan_dbg_tx_rate_info_v2_t *tx_rate_info;

		tx_rate_info = (wlan_dbg_tx_rate_info_v2_t *) (msg_word + 1);
		htt_t2h_stats_tx_rate_stats_print_v2(tx_rate_info, concise);
		break;
	}
	case HTT_DBG_STATS_RX_REMOTE_RING_BUFFER_INFO:
	{
		struct rx_remote_buffer_mgmt_stats *rx_rem_buf;

		rx_rem_buf =
			(struct rx_remote_buffer_mgmt_stats *)(msg_word + 1);
		htt_t2h_stats_rx_rem_buf_stats_print(rx_rem_buf, concise);
		break;
	}
	case HTT_DBG_STATS_TXBF_INFO:
	{
		struct wlan_dbg_txbf_data_stats *txbf_info_buf;

		txbf_info_buf =
			(struct wlan_dbg_txbf_data_stats *)(msg_word + 1);
		htt_t2h_stats_txbf_info_buf_stats_print(txbf_info_buf);
		break;
	}
	case HTT_DBG_STATS_SND_INFO:
	{
		struct wlan_dbg_txbf_snd_stats *txbf_snd_buf;

		txbf_snd_buf = (struct wlan_dbg_txbf_snd_stats *)(msg_word + 1);
		htt_t2h_stats_txbf_snd_buf_stats_print(txbf_snd_buf);
		break;
	}
	case HTT_DBG_STATS_TX_SELFGEN_INFO:
	{
		struct wlan_dbg_tx_selfgen_stats  *tx_selfgen_buf;

		tx_selfgen_buf =
			(struct wlan_dbg_tx_selfgen_stats  *)(msg_word + 1);
		htt_t2h_stats_tx_selfgen_buf_stats_print(tx_selfgen_buf);
		break;
	}
	case HTT_DBG_STATS_ERROR_INFO:
	{
		struct wlan_dbg_wifi2_error_stats  *wifi2_error_buf;

		wifi2_error_buf =
			(struct wlan_dbg_wifi2_error_stats  *)(msg_word + 1);
		htt_t2h_stats_wifi2_error_stats_print(wifi2_error_buf);
		break;
	}
	case HTT_DBG_STATS_TXBF_MUSU_NDPA_PKT:
	{
		struct rx_txbf_musu_ndpa_pkts_stats *rx_musu_ndpa_stats;

		rx_musu_ndpa_stats = (struct rx_txbf_musu_ndpa_pkts_stats *)
								(msg_word + 1);
		htt_t2h_rx_musu_ndpa_pkts_stats_print(rx_musu_ndpa_stats);
		break;
	}
	case HTT_DBG_STATS_TIDQ:
	{
		struct wlan_dbg_tidq_stats *tidq_stats;

		tidq_stats = (struct wlan_dbg_tidq_stats *)(msg_word + 1);
		htt_t2h_stats_tidq_stats_print(tidq_stats, concise);
		break;
	}
	case HTT_DBG_STATS_TX_MU_INFO:
	{
		struct wlan_dbg_tx_mu_stats *tx_mu_stats;

		tx_mu_stats = (struct wlan_dbg_tx_mu_stats *)(msg_word + 1);
		htt_t2h_stats_tx_mu_stats_print(tx_mu_stats, concise);
		break;
	}
	case HTT_DBG_STATS_SIFS_RESP_INFO:
	{
		struct wlan_dbg_sifs_resp_stats *sifs_stats;

		sifs_stats = (struct wlan_dbg_sifs_resp_stats *)(msg_word + 1);
		htt_t2h_stats_sifs_resp_stats_print(sifs_stats, concise);
		break;
	}
	default:
		break;
	}
}

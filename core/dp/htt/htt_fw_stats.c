/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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
#include <cdf_nbuf.h>           /* cdf_nbuf_t */
#include <cdf_memory.h>         /* cdf_mem_set */
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
{														\
	int i;												\
														\
	qdf_print("TX Rate Info:\n");						\
														\
	/* MCS */											\
	qdf_print("MCS counts (0..9): ");					\
	qdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
		  _tx_rate_info->mcs[0],						\
		  _tx_rate_info->mcs[1],						\
		  _tx_rate_info->mcs[2],						\
		  _tx_rate_info->mcs[3],						\
		  _tx_rate_info->mcs[4],						\
		  _tx_rate_info->mcs[5],						\
		  _tx_rate_info->mcs[6],						\
		  _tx_rate_info->mcs[7],						\
		  _tx_rate_info->mcs[8],						\
		  _tx_rate_info->mcs[9]);						\
														\
	/* SGI */											\
	qdf_print("SGI counts (0..9): ");					\
	qdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
		  _tx_rate_info->sgi[0],						\
		  _tx_rate_info->sgi[1],						\
		  _tx_rate_info->sgi[2],						\
		  _tx_rate_info->sgi[3],						\
		  _tx_rate_info->sgi[4],						\
		  _tx_rate_info->sgi[5],						\
		  _tx_rate_info->sgi[6],						\
		  _tx_rate_info->sgi[7],						\
		  _tx_rate_info->sgi[8],						\
		  _tx_rate_info->sgi[9]);						\
														\
	/* NSS */											\
	qdf_print("NSS  counts: ");							\
	qdf_print("1x1 %d, 2x2 %d, 3x3 %d\n",				\
		  _tx_rate_info->nss[0],						\
		  _tx_rate_info->nss[1], _tx_rate_info->nss[2]);\
														\
	/* BW */											\
	qdf_print("BW counts: ");							\
														\
	for (i = 0;											\
		i < sizeof(_tx_rate_info->bw) / sizeof(_tx_rate_info->bw[0]);\
		i++) {											\
			qdf_print("%s %d ", bw_str_arr[i], _tx_rate_info->bw[i]);\
	}													\
	qdf_print("\n");									\
														\
	/* Preamble */										\
	qdf_print("Preamble (O C H V) counts: ");			\
	qdf_print("%d, %d, %d, %d\n",						\
		  _tx_rate_info->pream[0],						\
		  _tx_rate_info->pream[1],						\
		  _tx_rate_info->pream[2],						\
		  _tx_rate_info->pream[3]);						\
														\
	/* STBC rate counts */								\
	qdf_print("STBC rate counts (0..9): ");				\
	qdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
		  _tx_rate_info->stbc[0],						\
		  _tx_rate_info->stbc[1],						\
		  _tx_rate_info->stbc[2],						\
		  _tx_rate_info->stbc[3],						\
		  _tx_rate_info->stbc[4],						\
		  _tx_rate_info->stbc[5],						\
		  _tx_rate_info->stbc[6],						\
		  _tx_rate_info->stbc[7],						\
		  _tx_rate_info->stbc[8],						\
		  _tx_rate_info->stbc[9]);						\
														\
	/* LDPC and TxBF counts */							\
	qdf_print("LDPC Counts: ");							\
	qdf_print("%d\n", _tx_rate_info->ldpc);				\
	qdf_print("RTS Counts: ");							\
	qdf_print("%d\n", _tx_rate_info->rts_cnt);			\
	/* RSSI Values for last ack frames */				\
	qdf_print("Ack RSSI: %d\n", _tx_rate_info->ack_rssi);\
}

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
{														\
	int i;												\
														\
	qdf_print("RX Rate Info:\n");						\
														\
	/* MCS */											\
	qdf_print("MCS counts (0..9): ");					\
	qdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
		  _rx_phy_info->mcs[0],							\
		  _rx_phy_info->mcs[1],							\
		  _rx_phy_info->mcs[2],							\
		  _rx_phy_info->mcs[3],							\
		  _rx_phy_info->mcs[4],							\
		  _rx_phy_info->mcs[5],							\
		  _rx_phy_info->mcs[6],							\
		  _rx_phy_info->mcs[7],							\
		  _rx_phy_info->mcs[8],							\
		  _rx_phy_info->mcs[9]);						\
														\
	/* SGI */											\
	qdf_print("SGI counts (0..9): ");					\
	qdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
		  _rx_phy_info->sgi[0],							\
		  _rx_phy_info->sgi[1],							\
		  _rx_phy_info->sgi[2],							\
		  _rx_phy_info->sgi[3],							\
		  _rx_phy_info->sgi[4],							\
		  _rx_phy_info->sgi[5],							\
		  _rx_phy_info->sgi[6],							\
		  _rx_phy_info->sgi[7],							\
		  _rx_phy_info->sgi[8],							\
		  _rx_phy_info->sgi[9]);						\
														\
	/* NSS */											\
	qdf_print("NSS  counts: ");							\
	/* nss[0] just holds the count of non-stbc frames that were sent at 1x1 \
	 * rates and nsts holds the count of frames sent with stbc.	\
	 * It was decided to not include PPDUs sent w/ STBC in nss[0]\
	 * since it would be easier to change the value that needs to be\
	 * printed (from "stbc+non-stbc count to only non-stbc count")\
	 * if needed in the future. Hence the addition in the host code\
	 * at this line. */									\
	qdf_print("1x1 %d, 2x2 %d, 3x3 %d, 4x4 %d\n",		\
		  _rx_phy_info->nss[0] + _rx_phy_info->nsts,	\
		  _rx_phy_info->nss[1],							\
		  _rx_phy_info->nss[2],							\
		  _rx_phy_info->nss[3]);						\
														\
	/* NSTS */											\
	qdf_print("NSTS count: ");							\
	qdf_print("%d\n", _rx_phy_info->nsts);				\
														\
	/* BW */											\
	qdf_print("BW counts: ");							\
	for (i = 0;											\
		i < sizeof(_rx_phy_info->bw) / sizeof(_rx_phy_info->bw[0]);	\
		i++) {											\
			qdf_print("%s %d ", bw_str_arr[i], _rx_phy_info->bw[i]);\
	}													\
	qdf_print("\n");									\
														\
	/* Preamble */										\
	qdf_print("Preamble counts: ");						\
	qdf_print("%d, %d, %d, %d, %d, %d\n",				\
		  _rx_phy_info->pream[0],						\
		  _rx_phy_info->pream[1],						\
		  _rx_phy_info->pream[2],						\
		  _rx_phy_info->pream[3],						\
		  _rx_phy_info->pream[4],						\
		  _rx_phy_info->pream[5]);						\
														\
	/* STBC rate counts */								\
	qdf_print("STBC rate counts (0..9): ");				\
	qdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
		  _rx_phy_info->stbc[0],						\
		  _rx_phy_info->stbc[1],						\
		  _rx_phy_info->stbc[2],						\
		  _rx_phy_info->stbc[3],						\
		  _rx_phy_info->stbc[4],						\
		  _rx_phy_info->stbc[5],						\
		  _rx_phy_info->stbc[6],						\
		  _rx_phy_info->stbc[7],						\
		  _rx_phy_info->stbc[8],						\
		  _rx_phy_info->stbc[9]);						\
														\
	/* LDPC and TxBF counts */							\
	qdf_print("LDPC TXBF Counts: ");					\
	qdf_print("%d, %d\n", _rx_phy_info->ldpc, _rx_phy_info->txbf);\
	/* RSSI Values for last received frames */			\
	qdf_print("RSSI (data, mgmt): %d, %d\n", _rx_phy_info->data_rssi,\
		  _rx_phy_info->mgmt_rssi);						\
														\
	qdf_print("RSSI Chain 0 (0x%02x 0x%02x 0x%02x 0x%02x)\n",\
		  ((_rx_phy_info->rssi_chain0 >> 24) & 0xff),	\
		  ((_rx_phy_info->rssi_chain0 >> 16) & 0xff),	\
		  ((_rx_phy_info->rssi_chain0 >> 8) & 0xff),	\
		  ((_rx_phy_info->rssi_chain0 >> 0) & 0xff));	\
														\
	qdf_print("RSSI Chain 1 (0x%02x 0x%02x 0x%02x 0x%02x)\n",\
		  ((_rx_phy_info->rssi_chain1 >> 24) & 0xff),	\
		  ((_rx_phy_info->rssi_chain1 >> 16) & 0xff),	\
		  ((_rx_phy_info->rssi_chain1 >> 8) & 0xff),	\
		  ((_rx_phy_info->rssi_chain1 >> 0) & 0xff));	\
														\
	qdf_print("RSSI Chain 2 (0x%02x 0x%02x 0x%02x 0x%02x)\n",\
		  ((_rx_phy_info->rssi_chain2 >> 24) & 0xff),	\
		  ((_rx_phy_info->rssi_chain2 >> 16) & 0xff),	\
		  ((_rx_phy_info->rssi_chain2 >> 8) & 0xff),	\
		  ((_rx_phy_info->rssi_chain2 >> 0) & 0xff));	\
}

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

	qdf_print("WAL Pdev stats:\n");
	qdf_print("\n### Tx ###\n");

	/* Num HTT cookies queued to dispatch list */
	qdf_print("comp_queued       :\t%d\n", tx->comp_queued);
	/* Num HTT cookies dispatched */
	qdf_print("comp_delivered    :\t%d\n", tx->comp_delivered);
	/* Num MSDU queued to WAL */
	qdf_print("msdu_enqued       :\t%d\n", tx->msdu_enqued);
	/* Num MPDU queued to WAL */
	qdf_print("mpdu_enqued       :\t%d\n", tx->mpdu_enqued);
	/* Num MSDUs dropped by WMM limit */
	qdf_print("wmm_drop          :\t%d\n", tx->wmm_drop);
	/* Num Local frames queued */
	qdf_print("local_enqued      :\t%d\n", tx->local_enqued);
	/* Num Local frames done */
	qdf_print("local_freed       :\t%d\n", tx->local_freed);
	/* Num queued to HW */
	qdf_print("hw_queued         :\t%d\n", tx->hw_queued);
	/* Num PPDU reaped from HW */
	qdf_print("hw_reaped         :\t%d\n", tx->hw_reaped);
	/* Num underruns */
	qdf_print("mac underrun      :\t%d\n", tx->underrun);
	/* Num underruns */
	qdf_print("phy underrun      :\t%d\n", tx->phy_underrun);
	/* Num PPDUs cleaned up in TX abort */
	qdf_print("tx_abort          :\t%d\n", tx->tx_abort);
	/* Num MPDUs requed by SW */
	qdf_print("mpdus_requed      :\t%d\n", tx->mpdus_requed);
	/* Excessive retries */
	qdf_print("excess retries    :\t%d\n", tx->tx_ko);
	/* last data rate */
	qdf_print("last rc           :\t%d\n", tx->data_rc);
	/* scheduler self triggers */
	qdf_print("sched self trig   :\t%d\n", tx->self_triggers);
	/* SW retry failures */
	qdf_print("ampdu retry failed:\t%d\n", tx->sw_retry_failure);
	/* ilegal phy rate errirs */
	qdf_print("illegal rate errs :\t%d\n", tx->illgl_rate_phy_err);
	/* pdev continous excessive retries  */
	qdf_print("pdev cont xretry  :\t%d\n", tx->pdev_cont_xretry);
	/* pdev continous excessive retries  */
	qdf_print("pdev tx timeout   :\t%d\n", tx->pdev_tx_timeout);
	/* pdev resets  */
	qdf_print("pdev resets       :\t%d\n", tx->pdev_resets);
	/* PPDU > txop duration  */
	qdf_print("ppdu txop ovf     :\t%d\n", tx->txop_ovf);

	qdf_print("\n### Rx ###\n");
	/* Cnts any change in ring routing mid-ppdu */
	qdf_print("ppdu_route_change :\t%d\n", rx->mid_ppdu_route_change);
	/* Total number of statuses processed */
	qdf_print("status_rcvd       :\t%d\n", rx->status_rcvd);
	/* Extra frags on rings 0-3 */
	qdf_print("r0_frags          :\t%d\n", rx->r0_frags);
	qdf_print("r1_frags          :\t%d\n", rx->r1_frags);
	qdf_print("r2_frags          :\t%d\n", rx->r2_frags);
	qdf_print("r3_frags          :\t%d\n", rx->r3_frags);
	/* MSDUs / MPDUs delivered to HTT */
	qdf_print("htt_msdus         :\t%d\n", rx->htt_msdus);
	qdf_print("htt_mpdus         :\t%d\n", rx->htt_mpdus);
	/* MSDUs / MPDUs delivered to local stack */
	qdf_print("loc_msdus         :\t%d\n", rx->loc_msdus);
	qdf_print("loc_mpdus         :\t%d\n", rx->loc_mpdus);
	/* AMSDUs that have more MSDUs than the status ring size */
	qdf_print("oversize_amsdu    :\t%d\n", rx->oversize_amsdu);
	/* Number of PHY errors */
	qdf_print("phy_errs          :\t%d\n", rx->phy_errs);
	/* Number of PHY errors dropped */
	qdf_print("phy_errs dropped  :\t%d\n", rx->phy_err_drop);
	/* Number of mpdu errors - FCS, MIC, ENC etc. */
	qdf_print("mpdu_errs         :\t%d\n", rx->mpdu_errs);

}

static void
htt_t2h_stats_rx_reorder_stats_print(struct rx_reorder_stats *stats_ptr,
				     int concise)
{
	qdf_print("Rx reorder statistics:\n");
	qdf_print("  %u non-QoS frames received\n", stats_ptr->deliver_non_qos);
	qdf_print("  %u frames received in-order\n",
		  stats_ptr->deliver_in_order);
	qdf_print("  %u frames flushed due to timeout\n",
		  stats_ptr->deliver_flush_timeout);
	qdf_print("  %u frames flushed due to moving out of window\n",
		  stats_ptr->deliver_flush_oow);
	qdf_print("  %u frames flushed due to receiving DELBA\n",
		  stats_ptr->deliver_flush_delba);
	qdf_print("  %u frames discarded due to FCS error\n",
		  stats_ptr->fcs_error);
	qdf_print("  %u frames discarded due to invalid peer\n",
		  stats_ptr->invalid_peer);
	qdf_print
		("  %u frames discarded due to duplication (non aggregation)\n",
		stats_ptr->dup_non_aggr);
	qdf_print("  %u frames discarded due to duplication in reorder queue\n",
		 stats_ptr->dup_in_reorder);
	qdf_print("  %u frames discarded due to processed before\n",
		  stats_ptr->dup_past);
	qdf_print("  %u times reorder timeout happened\n",
		  stats_ptr->reorder_timeout);
	qdf_print("  %u times incorrect bar received\n",
		  stats_ptr->invalid_bar_ssn);
	qdf_print("  %u times bar ssn reset happened\n",
			stats_ptr->ssn_reset);
	qdf_print("  %u times flushed due to peer delete\n",
			stats_ptr->deliver_flush_delpeer);
	qdf_print("  %u times flushed due to offload\n",
			stats_ptr->deliver_flush_offload);
	qdf_print("  %u times flushed due to ouf of buffer\n",
			stats_ptr->deliver_flush_oob);
	qdf_print("  %u MPDU's dropped due to PN check fail\n",
			stats_ptr->pn_fail);
	qdf_print("  %u MPDU's dropped due to lack of memory\n",
			stats_ptr->store_fail);
	qdf_print("  %u times tid pool alloc succeeded\n",
			stats_ptr->tid_pool_alloc_succ);
	qdf_print("  %u times MPDU pool alloc succeeded\n",
			stats_ptr->mpdu_pool_alloc_succ);
	qdf_print("  %u times MSDU pool alloc succeeded\n",
			stats_ptr->msdu_pool_alloc_succ);
	qdf_print("  %u times tid pool alloc failed\n",
			stats_ptr->tid_pool_alloc_fail);
	qdf_print("  %u times MPDU pool alloc failed\n",
			stats_ptr->mpdu_pool_alloc_fail);
	qdf_print("  %u times MSDU pool alloc failed\n",
			stats_ptr->msdu_pool_alloc_fail);
	qdf_print("  %u times tid pool freed\n",
			stats_ptr->tid_pool_free);
	qdf_print("  %u times MPDU pool freed\n",
			stats_ptr->mpdu_pool_free);
	qdf_print("  %u times MSDU pool freed\n",
			stats_ptr->msdu_pool_free);
	qdf_print("  %u MSDUs undelivered to HTT, queued to Rx MSDU free list\n",
			stats_ptr->msdu_queued);
	qdf_print("  %u MSDUs released from Rx MSDU list to MAC ring\n",
			stats_ptr->msdu_recycled);
	qdf_print("  %u MPDUs with invalid peer but A2 found in AST\n",
			stats_ptr->invalid_peer_a2_in_ast);
	qdf_print("  %u MPDUs with invalid peer but A3 found in AST\n",
			stats_ptr->invalid_peer_a3_in_ast);
	qdf_print("  %u MPDUs with invalid peer, Broadcast or Mulitcast frame\n",
			stats_ptr->invalid_peer_bmc_mpdus);
	qdf_print("  %u MSDUs with err attention word\n",
			stats_ptr->rxdesc_err_att);
	qdf_print("  %u MSDUs with flag of peer_idx_invalid\n",
			stats_ptr->rxdesc_err_peer_idx_inv);
	qdf_print("  %u MSDUs with  flag of peer_idx_timeout\n",
			stats_ptr->rxdesc_err_peer_idx_to);
	qdf_print("  %u MSDUs with  flag of overflow\n",
			stats_ptr->rxdesc_err_ov);
	qdf_print("  %u MSDUs with  flag of msdu_length_err\n",
			stats_ptr->rxdesc_err_msdu_len);
	qdf_print("  %u MSDUs with  flag of mpdu_length_err\n",
			stats_ptr->rxdesc_err_mpdu_len);
	qdf_print("  %u MSDUs with  flag of tkip_mic_err\n",
			stats_ptr->rxdesc_err_tkip_mic);
	qdf_print("  %u MSDUs with  flag of decrypt_err\n",
			stats_ptr->rxdesc_err_decrypt);
	qdf_print("  %u MSDUs with  flag of fcs_err\n",
			stats_ptr->rxdesc_err_fcs);
	qdf_print("  %u Unicast frames with invalid peer handler\n",
			stats_ptr->rxdesc_uc_msdus_inv_peer);
	qdf_print("  %u unicast frame directly to DUT with invalid peer handler\n",
			stats_ptr->rxdesc_direct_msdus_inv_peer);
	qdf_print("  %u Broadcast/Multicast frames with invalid peer handler\n",
			stats_ptr->rxdesc_bmc_msdus_inv_peer);
	qdf_print("  %u MSDUs dropped due to no first MSDU flag\n",
			stats_ptr->rxdesc_no_1st_msdu);
	qdf_print("  %u MSDUs dropped due to ring overflow\n",
			stats_ptr->msdu_drop_ring_ov);
	qdf_print("  %u MSDUs dropped due to FC mismatch\n",
			stats_ptr->msdu_drop_fc_mismatch);
	qdf_print("  %u MSDUs dropped due to mgt frame in Remote ring\n",
			stats_ptr->msdu_drop_mgmt_remote_ring);
	qdf_print("  %u MSDUs dropped due to misc non error\n",
			stats_ptr->msdu_drop_misc);
	qdf_print("  %u MSDUs go to offload before reorder\n",
			stats_ptr->offload_msdu_wal);
	qdf_print("  %u data frame dropped by offload after reorder\n",
			stats_ptr->offload_msdu_reorder);
	qdf_print("  %u  MPDUs with SN in the past & within BA window\n",
			stats_ptr->dup_past_within_window);
	qdf_print("  %u  MPDUs with SN in the past & outside BA window\n",
			stats_ptr->dup_past_outside_window);
}

static void
htt_t2h_stats_rx_rem_buf_stats_print(
    struct rx_remote_buffer_mgmt_stats *stats_ptr, int concise)
{
	qdf_print("Rx Remote Buffer Statistics:\n");
	qdf_print("  %u MSDU's reaped for Rx processing\n",
			stats_ptr->remote_reaped);
	qdf_print("  %u MSDU's recycled within firmware\n",
			stats_ptr->remote_recycled);
	qdf_print("  %u MSDU's stored by Data Rx\n",
			stats_ptr->data_rx_msdus_stored);
	qdf_print("  %u HTT indications from WAL Rx MSDU\n",
			stats_ptr->wal_rx_ind);
	qdf_print("  %u HTT indications unconsumed from WAL Rx MSDU\n",
			stats_ptr->wal_rx_ind_unconsumed);
	qdf_print("  %u HTT indications from Data Rx MSDU\n",
			stats_ptr->data_rx_ind);
	qdf_print("  %u HTT indications unconsumed from Data Rx MSDU\n",
			stats_ptr->data_rx_ind_unconsumed);
	qdf_print("  %u HTT indications from ATHBUF\n",
			stats_ptr->athbuf_rx_ind);
	qdf_print("  %u Remote buffers requested for refill\n",
			stats_ptr->refill_buf_req);
	qdf_print("  %u Remote buffers filled by host\n",
			stats_ptr->refill_buf_rsp);
	qdf_print("  %u times MAC has no buffers\n",
			stats_ptr->mac_no_bufs);
	qdf_print("  %u times f/w write & read indices on MAC ring are equal\n",
			stats_ptr->fw_indices_equal);
	qdf_print("  %u times f/w has no remote buffers to post to MAC\n",
			stats_ptr->host_no_bufs);
}

static void
htt_t2h_stats_txbf_info_buf_stats_print(
	struct wlan_dbg_txbf_data_stats *stats_ptr)
{
	qdf_print("TXBF data Statistics:\n");
	qdf_print("tx_txbf_vht (0..9): ");
	qdf_print("%u, %u, %u, %u, %u, %u, %u, %u, %u, %d\n",
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
	qdf_print("rx_txbf_vht (0..9): ");
	qdf_print("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
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
	qdf_print("tx_txbf_ht (0..7): ");
	qdf_print("%u, %u, %u, %u, %u, %u, %u, %u\n",
		  stats_ptr->tx_txbf_ht[0],
		  stats_ptr->tx_txbf_ht[1],
		  stats_ptr->tx_txbf_ht[2],
		  stats_ptr->tx_txbf_ht[3],
		  stats_ptr->tx_txbf_ht[4],
		  stats_ptr->tx_txbf_ht[5],
		  stats_ptr->tx_txbf_ht[6],
		  stats_ptr->tx_txbf_ht[7]);
	qdf_print("tx_txbf_ofdm (0..7): ");
	qdf_print("%u, %u, %u, %u, %u, %u, %u, %u\n",
		  stats_ptr->tx_txbf_ofdm[0],
		  stats_ptr->tx_txbf_ofdm[1],
		  stats_ptr->tx_txbf_ofdm[2],
		  stats_ptr->tx_txbf_ofdm[3],
		  stats_ptr->tx_txbf_ofdm[4],
		  stats_ptr->tx_txbf_ofdm[5],
		  stats_ptr->tx_txbf_ofdm[6],
		  stats_ptr->tx_txbf_ofdm[7]);
	qdf_print("tx_txbf_cck (0..6): ");
	qdf_print("%u, %u, %u, %u, %u, %u, %u\n",
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
	qdf_print("TXBF snd Buffer Statistics:\n");
	qdf_print("cbf_20: ");
	qdf_print("%u, %u, %u, %u\n",
		  stats_ptr->cbf_20[0],
		  stats_ptr->cbf_20[1],
		  stats_ptr->cbf_20[2],
		  stats_ptr->cbf_20[3]);
	qdf_print("cbf_40: ");
	qdf_print("%u, %u, %u, %u\n",
		  stats_ptr->cbf_40[0],
		  stats_ptr->cbf_40[1],
		  stats_ptr->cbf_40[2],
		  stats_ptr->cbf_40[3]);
	qdf_print("cbf_80: ");
	qdf_print("%u, %u, %u, %u\n",
		  stats_ptr->cbf_80[0],
		  stats_ptr->cbf_80[1],
		  stats_ptr->cbf_80[2],
		  stats_ptr->cbf_80[3]);
	qdf_print("sounding: ");
	qdf_print("%u, %u, %u, %u, %u, %u, %u, %u, %u\n",
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
	qdf_print("Tx selfgen Buffer Statistics:\n");
	qdf_print("  %u su_ndpa\n",
			stats_ptr->su_ndpa);
	qdf_print("  %u mu_ndp\n",
			stats_ptr->mu_ndp);
	qdf_print("  %u mu_ndpa\n",
			stats_ptr->mu_ndpa);
	qdf_print("  %u mu_ndp\n",
			stats_ptr->mu_ndp);
	qdf_print("  %u mu_brpoll_1\n",
			stats_ptr->mu_brpoll_1);
	qdf_print("  %u mu_brpoll_2\n",
			stats_ptr->mu_brpoll_2);
	qdf_print("  %u mu_bar_1\n",
			stats_ptr->mu_bar_1);
	qdf_print("  %u mu_bar_2\n",
			stats_ptr->mu_bar_2);
	qdf_print("  %u cts_burst\n",
			stats_ptr->cts_burst);
	qdf_print("  %u su_ndp_err\n",
			stats_ptr->su_ndp_err);
	qdf_print("  %u su_ndpa_err\n",
			stats_ptr->su_ndpa_err);
	qdf_print("  %u mu_ndp_err\n",
			stats_ptr->mu_ndp_err);
	qdf_print("  %u mu_brp1_err\n",
			stats_ptr->mu_brp1_err);
	qdf_print("  %u mu_brp2_err\n",
			stats_ptr->mu_brp2_err);
}

static void
htt_t2h_stats_wifi2_error_stats_print(
	struct wlan_dbg_wifi2_error_stats *stats_ptr)
{
	int i;

	qdf_print("Scheduler error Statistics:\n");
	qdf_print("urrn_stats: ");
	qdf_print("%d, %d, %d\n",
		  stats_ptr->urrn_stats[0],
		  stats_ptr->urrn_stats[1],
		  stats_ptr->urrn_stats[2]);
	qdf_print("flush_errs (0..%d): ",
			WHAL_DBG_FLUSH_REASON_MAXCNT);
	for (i = 0; i < WHAL_DBG_FLUSH_REASON_MAXCNT; i++)
		qdf_print("  %u", stats_ptr->flush_errs[i]);
	qdf_print("\n");
	qdf_print("schd_stall_errs (0..3): ");
	qdf_print("%d, %d, %d, %d\n",
		  stats_ptr->schd_stall_errs[0],
		  stats_ptr->schd_stall_errs[1],
		  stats_ptr->schd_stall_errs[2],
		  stats_ptr->schd_stall_errs[3]);
	qdf_print("schd_cmd_result (0..%d): ",
			WHAL_DBG_CMD_RESULT_MAXCNT);
	for (i = 0; i < WHAL_DBG_CMD_RESULT_MAXCNT; i++)
		qdf_print("  %u", stats_ptr->schd_cmd_result[i]);
	qdf_print("\n");
	qdf_print("sifs_status (0..%d): ",
			WHAL_DBG_SIFS_STATUS_MAXCNT);
	for (i = 0; i < WHAL_DBG_SIFS_STATUS_MAXCNT; i++)
		qdf_print("  %u", stats_ptr->sifs_status[i]);
	qdf_print("\n");
	qdf_print("phy_errs (0..%d): ",
			WHAL_DBG_PHY_ERR_MAXCNT);
	for (i = 0; i < WHAL_DBG_PHY_ERR_MAXCNT; i++)
		qdf_print("  %u", stats_ptr->phy_errs[i]);
	qdf_print("\n");
	qdf_print("  %u rx_rate_inval\n",
			stats_ptr->rx_rate_inval);
}

static void
htt_t2h_rx_musu_ndpa_pkts_stats_print(
	struct rx_txbf_musu_ndpa_pkts_stats *stats_ptr)
{
	qdf_print("Rx TXBF MU/SU Packets and NDPA Statistics:\n");
	qdf_print("  %u Number of TXBF MU packets received\n",
			stats_ptr->number_mu_pkts);
	qdf_print("  %u Number of TXBF SU packets received\n",
			stats_ptr->number_su_pkts);
	qdf_print("  %u Number of TXBF directed NDPA\n",
			stats_ptr->txbf_directed_ndpa_count);
	qdf_print("  %u Number of TXBF retried NDPA\n",
			stats_ptr->txbf_ndpa_retry_count);
	qdf_print("  %u Total number of TXBF NDPA\n",
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

	cdf_mem_set(queued_str, HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW, '0');
	cdf_mem_set(acked_str, HTT_FW_STATS_MAX_BLOCK_ACK_WINDOW, '-');
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
	qdf_print("%s\n", queued_str);
	qdf_print("%s\n", acked_str);
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

void htt_make_u8_list_str(uint32_t *aligned_data,
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

void htt_make_u16_list_str(uint32_t *aligned_data,
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

void
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
	qdf_print("Tx PPDU log elements:\n");

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
			((*(msg_word + OL_FW_TX_DBG_PPDU_START_SEQ_NUM_16)) &
			 OL_FW_TX_DBG_PPDU_START_SEQ_NUM_M) >>
			OL_FW_TX_DBG_PPDU_START_SEQ_NUM_S;
		complete =
			((*(msg_word + OL_FW_TX_DBG_PPDU_COMPLETE_16)) &
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
				((*
				  (msg_word +
				   OL_FW_TX_DBG_PPDU_START_PN_LSBS_16)) &
				 OL_FW_TX_DBG_PPDU_START_PN_LSBS_M) >>
				OL_FW_TX_DBG_PPDU_START_PN_LSBS_S;
			num_mpdus =
				((*(msg_word + OL_FW_TX_DBG_PPDU_NUM_MPDUS_16))&
				 OL_FW_TX_DBG_PPDU_NUM_MPDUS_M) >>
				OL_FW_TX_DBG_PPDU_NUM_MPDUS_S;
			peer_id =
				((*(msg_word + OL_FW_TX_DBG_PPDU_PEER_ID_16)) &
				 OL_FW_TX_DBG_PPDU_PEER_ID_M) >>
				OL_FW_TX_DBG_PPDU_PEER_ID_S;
			ext_tid =
				((*(msg_word + OL_FW_TX_DBG_PPDU_EXT_TID_16)) &
				 OL_FW_TX_DBG_PPDU_EXT_TID_M) >>
				OL_FW_TX_DBG_PPDU_EXT_TID_S;
			rate_code =
				((*(msg_word + OL_FW_TX_DBG_PPDU_RATE_CODE_16))&
				 OL_FW_TX_DBG_PPDU_RATE_CODE_M) >>
				OL_FW_TX_DBG_PPDU_RATE_CODE_S;
			rate_flags =
				((*(msg_word + OL_FW_TX_DBG_PPDU_RATEFLAGS_16))&
				 OL_FW_TX_DBG_PPDU_RATE_FLAGS_M) >>
				OL_FW_TX_DBG_PPDU_RATE_FLAGS_S;
			tries =
				((*(msg_word + OL_FW_TX_DBG_PPDU_TRIES_16)) &
				 OL_FW_TX_DBG_PPDU_TRIES_M) >>
				OL_FW_TX_DBG_PPDU_TRIES_S;

			qdf_print(" - PPDU tx to peer %d, TID %d\n", peer_id,
				  ext_tid);
			qdf_print
				("   start seq num= %u, start PN LSBs= %#04x\n",
				start_seq_num, start_pn_lsbs);
			qdf_print
				("   PPDU: %d MPDUs, (?) MSDUs, %d bytes\n",
				num_mpdus,
				 /* num_msdus - not yet computed in target */
				record->num_bytes);
			if (complete) {
				qdf_print
				      ("   enqueued: %u, completed: %u usec)\n",
				       time_enqueue_us, time_completion_us);
				qdf_print
					("   %d tries, last tx used rate %d ",
					 tries, rate_code);
				qdf_print("on %d MHz chan (flags = %#x)\n",
					  htt_rate_flags_to_mhz
					  (rate_flags), rate_flags);
				qdf_print
				      ("  enqueued and acked MPDU bitmaps:\n");
				htt_t2h_tx_ppdu_bitmaps_pr(msg_word +
					   OL_FW_TX_DBG_PPDU_ENQUEUED_LSBS_16,
							   msg_word +
					   OL_FW_TX_DBG_PPDU_BLOCK_ACK_LSBS_16);
			} else {
				qdf_print
				      ("  enqueued: %d us, not yet completed\n",
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
				qdf_print("   MPDU bytes: %s\n", buf);
			}
			p8 += hdr->mpdu_bytes_array_len * sizeof(uint16_t);
			if (hdr->mpdu_msdus_array_len) {
				htt_make_u8_list_str((uint32_t *) p8, buf,
						     BUF_SIZE,
						     hdr->mpdu_msdus_array_len);
				qdf_print("   MPDU MSDUs: %s\n", buf);
			}
			p8 += hdr->mpdu_msdus_array_len * sizeof(uint8_t);
			if (hdr->msdu_bytes_array_len) {
				htt_make_u16_list_str((uint32_t *) p8, buf,
						      BUF_SIZE,
						      hdr->
						      msdu_bytes_array_len);
				qdf_print("   MSDU bytes: %s\n", buf);
			}
		} else {
			/* concise */
			qdf_print("start seq num = %u ", start_seq_num);
			qdf_print("enqueued and acked MPDU bitmaps:\n");
			if (complete) {
				htt_t2h_tx_ppdu_bitmaps_pr(msg_word +
					OL_FW_TX_DBG_PPDU_ENQUEUED_LSBS_16,
							   msg_word +
					OL_FW_TX_DBG_PPDU_BLOCK_ACK_LSBS_16);
			} else {
				qdf_print("(not completed)\n");
			}
		}
		record = (struct ol_fw_tx_dbg_ppdu_base *)
			 (((uint8_t *) record) + record_size);
	}
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
		qdf_print("Target doesn't support stats type %d\n", type);
		return;
	} else if (status == HTT_DBG_STATS_STATUS_ERROR) {
		qdf_print("Target couldn't upload stats type %d (no mem?)\n",
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
		rx_phy_info =
			(wlan_dbg_rx_rate_info_t *) (msg_word + 1);

		htt_t2h_stats_rx_rate_stats_print(rx_phy_info, concise);

		break;
	}
	case HTT_DBG_STATS_RX_RATE_INFO_V2:
	{
		wlan_dbg_rx_rate_info_v2_t *rx_phy_info;
		rx_phy_info =
			(wlan_dbg_rx_rate_info_v2_t *) (msg_word + 1);
		htt_t2h_stats_rx_rate_stats_print_v2(rx_phy_info, concise);
		break;
	}
	case HTT_DBG_STATS_TX_PPDU_LOG:
	{
		struct ol_fw_tx_dbg_ppdu_msg_hdr *hdr;
		struct ol_fw_tx_dbg_ppdu_base *record;

		if (status == HTT_DBG_STATS_STATUS_PARTIAL
		    && length == 0) {
			qdf_print
				("HTT_DBG_STATS_TX_PPDU_LOG -- length = 0!\n");
			break;
		}
		hdr =
			(struct ol_fw_tx_dbg_ppdu_msg_hdr *)(msg_word + 1);
		record = (struct ol_fw_tx_dbg_ppdu_base *)(hdr + 1);
		htt_t2h_tx_ppdu_log_print(hdr, record, length, concise);
	}
	break;
	case HTT_DBG_STATS_TX_RATE_INFO:
	{
		wlan_dbg_tx_rate_info_t *tx_rate_info;
		tx_rate_info =
			(wlan_dbg_tx_rate_info_t *) (msg_word + 1);

		htt_t2h_stats_tx_rate_stats_print(tx_rate_info, concise);

		break;
	}
	case HTT_DBG_STATS_TX_RATE_INFO_V2:
	{
		wlan_dbg_tx_rate_info_v2_t *tx_rate_info;
		tx_rate_info =
			(wlan_dbg_tx_rate_info_v2_t *) (msg_word + 1);
		htt_t2h_stats_tx_rate_stats_print_v2(tx_rate_info, concise);
		break;
	}
	case HTT_DBG_STATS_RX_REMOTE_RING_BUFFER_INFO:
	{
		struct rx_remote_buffer_mgmt_stats *rx_rem_buf;

		rx_rem_buf = (struct rx_remote_buffer_mgmt_stats *)(msg_word + 1);
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

		txbf_snd_buf =
			(struct wlan_dbg_txbf_snd_stats *)(msg_word + 1);
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
	default:
		break;
	}
}

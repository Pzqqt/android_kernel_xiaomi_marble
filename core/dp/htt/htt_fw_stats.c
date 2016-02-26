/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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
	cdf_print("TX Rate Info:\n");						\
														\
	/* MCS */											\
	cdf_print("MCS counts (0..9): ");					\
	cdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
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
	cdf_print("SGI counts (0..9): ");					\
	cdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
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
	cdf_print("NSS  counts: ");							\
	cdf_print("1x1 %d, 2x2 %d, 3x3 %d\n",				\
		  _tx_rate_info->nss[0],						\
		  _tx_rate_info->nss[1], _tx_rate_info->nss[2]);\
														\
	/* BW */											\
	cdf_print("BW counts: ");							\
														\
	for (i = 0;											\
		i < sizeof(_tx_rate_info->bw) / sizeof(_tx_rate_info->bw[0]);\
		i++) {											\
			cdf_print("%s %d ", bw_str_arr[i], _tx_rate_info->bw[i]);\
	}													\
	cdf_print("\n");									\
														\
	/* Preamble */										\
	cdf_print("Preamble (O C H V) counts: ");			\
	cdf_print("%d, %d, %d, %d\n",						\
		  _tx_rate_info->pream[0],						\
		  _tx_rate_info->pream[1],						\
		  _tx_rate_info->pream[2],						\
		  _tx_rate_info->pream[3]);						\
														\
	/* STBC rate counts */								\
	cdf_print("STBC rate counts (0..9): ");				\
	cdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
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
	cdf_print("LDPC Counts: ");							\
	cdf_print("%d\n", _tx_rate_info->ldpc);				\
	cdf_print("RTS Counts: ");							\
	cdf_print("%d\n", _tx_rate_info->rts_cnt);			\
	/* RSSI Values for last ack frames */				\
	cdf_print("Ack RSSI: %d\n", _tx_rate_info->ack_rssi);\
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
	cdf_print("RX Rate Info:\n");						\
														\
	/* MCS */											\
	cdf_print("MCS counts (0..9): ");					\
	cdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
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
	cdf_print("SGI counts (0..9): ");					\
	cdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
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
	cdf_print("NSS  counts: ");							\
	/* nss[0] just holds the count of non-stbc frames that were sent at 1x1 \
	 * rates and nsts holds the count of frames sent with stbc.	\
	 * It was decided to not include PPDUs sent w/ STBC in nss[0]\
	 * since it would be easier to change the value that needs to be\
	 * printed (from "stbc+non-stbc count to only non-stbc count")\
	 * if needed in the future. Hence the addition in the host code\
	 * at this line. */									\
	cdf_print("1x1 %d, 2x2 %d, 3x3 %d, 4x4 %d\n",		\
		  _rx_phy_info->nss[0] + _rx_phy_info->nsts,	\
		  _rx_phy_info->nss[1],							\
		  _rx_phy_info->nss[2],							\
		  _rx_phy_info->nss[3]);						\
														\
	/* NSTS */											\
	cdf_print("NSTS count: ");							\
	cdf_print("%d\n", _rx_phy_info->nsts);				\
														\
	/* BW */											\
	cdf_print("BW counts: ");							\
	for (i = 0;											\
		i < sizeof(_rx_phy_info->bw) / sizeof(_rx_phy_info->bw[0]);	\
		i++) {											\
			cdf_print("%s %d ", bw_str_arr[i], _rx_phy_info->bw[i]);\
	}													\
	cdf_print("\n");									\
														\
	/* Preamble */										\
	cdf_print("Preamble counts: ");						\
	cdf_print("%d, %d, %d, %d, %d, %d\n",				\
		  _rx_phy_info->pream[0],						\
		  _rx_phy_info->pream[1],						\
		  _rx_phy_info->pream[2],						\
		  _rx_phy_info->pream[3],						\
		  _rx_phy_info->pream[4],						\
		  _rx_phy_info->pream[5]);						\
														\
	/* STBC rate counts */								\
	cdf_print("STBC rate counts (0..9): ");				\
	cdf_print("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",\
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
	cdf_print("LDPC TXBF Counts: ");					\
	cdf_print("%d, %d\n", _rx_phy_info->ldpc, _rx_phy_info->txbf);\
	/* RSSI Values for last received frames */			\
	cdf_print("RSSI (data, mgmt): %d, %d\n", _rx_phy_info->data_rssi,\
		  _rx_phy_info->mgmt_rssi);						\
														\
	cdf_print("RSSI Chain 0 (0x%02x 0x%02x 0x%02x 0x%02x)\n",\
		  ((_rx_phy_info->rssi_chain0 >> 24) & 0xff),	\
		  ((_rx_phy_info->rssi_chain0 >> 16) & 0xff),	\
		  ((_rx_phy_info->rssi_chain0 >> 8) & 0xff),	\
		  ((_rx_phy_info->rssi_chain0 >> 0) & 0xff));	\
														\
	cdf_print("RSSI Chain 1 (0x%02x 0x%02x 0x%02x 0x%02x)\n",\
		  ((_rx_phy_info->rssi_chain1 >> 24) & 0xff),	\
		  ((_rx_phy_info->rssi_chain1 >> 16) & 0xff),	\
		  ((_rx_phy_info->rssi_chain1 >> 8) & 0xff),	\
		  ((_rx_phy_info->rssi_chain1 >> 0) & 0xff));	\
														\
	cdf_print("RSSI Chain 2 (0x%02x 0x%02x 0x%02x 0x%02x)\n",\
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

	cdf_print("WAL Pdev stats:\n");
	cdf_print("\n### Tx ###\n");

	/* Num HTT cookies queued to dispatch list */
	cdf_print("comp_queued       :\t%d\n", tx->comp_queued);
	/* Num HTT cookies dispatched */
	cdf_print("comp_delivered    :\t%d\n", tx->comp_delivered);
	/* Num MSDU queued to WAL */
	cdf_print("msdu_enqued       :\t%d\n", tx->msdu_enqued);
	/* Num MPDU queued to WAL */
	cdf_print("mpdu_enqued       :\t%d\n", tx->mpdu_enqued);
	/* Num MSDUs dropped by WMM limit */
	cdf_print("wmm_drop          :\t%d\n", tx->wmm_drop);
	/* Num Local frames queued */
	cdf_print("local_enqued      :\t%d\n", tx->local_enqued);
	/* Num Local frames done */
	cdf_print("local_freed       :\t%d\n", tx->local_freed);
	/* Num queued to HW */
	cdf_print("hw_queued         :\t%d\n", tx->hw_queued);
	/* Num PPDU reaped from HW */
	cdf_print("hw_reaped         :\t%d\n", tx->hw_reaped);
	/* Num underruns */
	cdf_print("mac underrun      :\t%d\n", tx->underrun);
	/* Num underruns */
	cdf_print("phy underrun      :\t%d\n", tx->phy_underrun);
	/* Num PPDUs cleaned up in TX abort */
	cdf_print("tx_abort          :\t%d\n", tx->tx_abort);
	/* Num MPDUs requed by SW */
	cdf_print("mpdus_requed      :\t%d\n", tx->mpdus_requed);
	/* Excessive retries */
	cdf_print("excess retries    :\t%d\n", tx->tx_ko);
	/* last data rate */
	cdf_print("last rc           :\t%d\n", tx->data_rc);
	/* scheduler self triggers */
	cdf_print("sched self trig   :\t%d\n", tx->self_triggers);
	/* SW retry failures */
	cdf_print("ampdu retry failed:\t%d\n", tx->sw_retry_failure);
	/* ilegal phy rate errirs */
	cdf_print("illegal rate errs :\t%d\n", tx->illgl_rate_phy_err);
	/* pdev continous excessive retries  */
	cdf_print("pdev cont xretry  :\t%d\n", tx->pdev_cont_xretry);
	/* pdev continous excessive retries  */
	cdf_print("pdev tx timeout   :\t%d\n", tx->pdev_tx_timeout);
	/* pdev resets  */
	cdf_print("pdev resets       :\t%d\n", tx->pdev_resets);
	/* PPDU > txop duration  */
	cdf_print("ppdu txop ovf     :\t%d\n", tx->txop_ovf);

	cdf_print("\n### Rx ###\n");
	/* Cnts any change in ring routing mid-ppdu */
	cdf_print("ppdu_route_change :\t%d\n", rx->mid_ppdu_route_change);
	/* Total number of statuses processed */
	cdf_print("status_rcvd       :\t%d\n", rx->status_rcvd);
	/* Extra frags on rings 0-3 */
	cdf_print("r0_frags          :\t%d\n", rx->r0_frags);
	cdf_print("r1_frags          :\t%d\n", rx->r1_frags);
	cdf_print("r2_frags          :\t%d\n", rx->r2_frags);
	cdf_print("r3_frags          :\t%d\n", rx->r3_frags);
	/* MSDUs / MPDUs delivered to HTT */
	cdf_print("htt_msdus         :\t%d\n", rx->htt_msdus);
	cdf_print("htt_mpdus         :\t%d\n", rx->htt_mpdus);
	/* MSDUs / MPDUs delivered to local stack */
	cdf_print("loc_msdus         :\t%d\n", rx->loc_msdus);
	cdf_print("loc_mpdus         :\t%d\n", rx->loc_mpdus);
	/* AMSDUs that have more MSDUs than the status ring size */
	cdf_print("oversize_amsdu    :\t%d\n", rx->oversize_amsdu);
	/* Number of PHY errors */
	cdf_print("phy_errs          :\t%d\n", rx->phy_errs);
	/* Number of PHY errors dropped */
	cdf_print("phy_errs dropped  :\t%d\n", rx->phy_err_drop);
	/* Number of mpdu errors - FCS, MIC, ENC etc. */
	cdf_print("mpdu_errs         :\t%d\n", rx->mpdu_errs);

}

static void
htt_t2h_stats_rx_reorder_stats_print(struct rx_reorder_stats *stats_ptr,
				     int concise)
{
	cdf_print("Rx reorder statistics:\n");
	cdf_print("  %u non-QoS frames received\n", stats_ptr->deliver_non_qos);
	cdf_print("  %u frames received in-order\n",
		  stats_ptr->deliver_in_order);
	cdf_print("  %u frames flushed due to timeout\n",
		  stats_ptr->deliver_flush_timeout);
	cdf_print("  %u frames flushed due to moving out of window\n",
		  stats_ptr->deliver_flush_oow);
	cdf_print("  %u frames flushed due to receiving DELBA\n",
		  stats_ptr->deliver_flush_delba);
	cdf_print("  %u frames discarded due to FCS error\n",
		  stats_ptr->fcs_error);
	cdf_print("  %u frames discarded due to invalid peer\n",
		  stats_ptr->invalid_peer);
	cdf_print
		("  %u frames discarded due to duplication (non aggregation)\n",
		stats_ptr->dup_non_aggr);
	cdf_print("  %u frames discarded due to duplication in reorder queue\n",
		 stats_ptr->dup_in_reorder);
	cdf_print("  %u frames discarded due to processed before\n",
		  stats_ptr->dup_past);
	cdf_print("  %u times reorder timeout happened\n",
		  stats_ptr->reorder_timeout);
	cdf_print("  %u times incorrect bar received\n",
		  stats_ptr->invalid_bar_ssn);
	cdf_print("  %u times bar ssn reset happened\n",
			stats_ptr->ssn_reset);
	cdf_print("  %u times flushed due to peer delete\n",
			stats_ptr->deliver_flush_delpeer);
	cdf_print("  %u times flushed due to offload\n",
			stats_ptr->deliver_flush_offload);
	cdf_print("  %u times flushed due to ouf of buffer\n",
			stats_ptr->deliver_flush_oob);
	cdf_print("  %u MPDU's dropped due to PN check fail\n",
			stats_ptr->pn_fail);
	cdf_print("  %u MPDU's dropped due to lack of memory\n",
			stats_ptr->store_fail);
	cdf_print("  %u times tid pool alloc succeeded\n",
			stats_ptr->tid_pool_alloc_succ);
	cdf_print("  %u times MPDU pool alloc succeeded\n",
			stats_ptr->mpdu_pool_alloc_succ);
	cdf_print("  %u times MSDU pool alloc succeeded\n",
			stats_ptr->msdu_pool_alloc_succ);
	cdf_print("  %u times tid pool alloc failed\n",
			stats_ptr->tid_pool_alloc_fail);
	cdf_print("  %u times MPDU pool alloc failed\n",
			stats_ptr->mpdu_pool_alloc_fail);
	cdf_print("  %u times MSDU pool alloc failed\n",
			stats_ptr->msdu_pool_alloc_fail);
	cdf_print("  %u times tid pool freed\n",
			stats_ptr->tid_pool_free);
	cdf_print("  %u times MPDU pool freed\n",
			stats_ptr->mpdu_pool_free);
	cdf_print("  %u times MSDU pool freed\n",
			stats_ptr->msdu_pool_free);
	cdf_print("  %u MSDUs undelivered to HTT, queued to Rx MSDU free list\n",
			stats_ptr->msdu_queued);
	cdf_print("  %u MSDUs released from Rx MSDU list to MAC ring\n",
			stats_ptr->msdu_recycled);
	cdf_print("  %u MPDUs with invalid peer but A2 found in AST\n",
			stats_ptr->invalid_peer_a2_in_ast);
	cdf_print("  %u MPDUs with invalid peer but A3 found in AST\n",
			stats_ptr->invalid_peer_a3_in_ast);
	cdf_print("  %u MPDUs with invalid peer, Broadcast or Mulitcast frame\n",
			stats_ptr->invalid_peer_bmc_mpdus);
	cdf_print("  %u MSDUs with err attention word\n",
			stats_ptr->rxdesc_err_att);
	cdf_print("  %u MSDUs with flag of peer_idx_invalid\n",
			stats_ptr->rxdesc_err_peer_idx_inv);
	cdf_print("  %u MSDUs with  flag of peer_idx_timeout\n",
			stats_ptr->rxdesc_err_peer_idx_to);
	cdf_print("  %u MSDUs with  flag of overflow\n",
			stats_ptr->rxdesc_err_ov);
	cdf_print("  %u MSDUs with  flag of msdu_length_err\n",
			stats_ptr->rxdesc_err_msdu_len);
	cdf_print("  %u MSDUs with  flag of mpdu_length_err\n",
			stats_ptr->rxdesc_err_mpdu_len);
	cdf_print("  %u MSDUs with  flag of tkip_mic_err\n",
			stats_ptr->rxdesc_err_tkip_mic);
	cdf_print("  %u MSDUs with  flag of decrypt_err\n",
			stats_ptr->rxdesc_err_decrypt);
	cdf_print("  %u MSDUs with  flag of fcs_err\n",
			stats_ptr->rxdesc_err_fcs);
	cdf_print("  %u Unicast frames with invalid peer handler\n",
			stats_ptr->rxdesc_uc_msdus_inv_peer);
	cdf_print("  %u unicast frame directly to DUT with invalid peer handler\n",
			stats_ptr->rxdesc_direct_msdus_inv_peer);
	cdf_print("  %u Broadcast/Multicast frames with invalid peer handler\n",
			stats_ptr->rxdesc_bmc_msdus_inv_peer);
	cdf_print("  %u MSDUs dropped due to no first MSDU flag\n",
			stats_ptr->rxdesc_no_1st_msdu);
	cdf_print("  %u MSDUs dropped due to ring overflow\n",
			stats_ptr->msdu_drop_ring_ov);
	cdf_print("  %u MSDUs dropped due to FC mismatch\n",
			stats_ptr->msdu_drop_fc_mismatch);
	cdf_print("  %u MSDUs dropped due to mgt frame in Remote ring\n",
			stats_ptr->msdu_drop_mgmt_remote_ring);
	cdf_print("  %u MSDUs dropped due to misc non error\n",
			stats_ptr->msdu_drop_misc);
	cdf_print("  %u MSDUs go to offload before reorder\n",
			stats_ptr->offload_msdu_wal);
	cdf_print("  %u data frame dropped by offload after reorder\n",
			stats_ptr->offload_msdu_reorder);
	cdf_print("  %u  MPDUs with SN in the past & within BA window\n",
			stats_ptr->dup_past_within_window);
	cdf_print("  %u  MPDUs with SN in the past & outside BA window\n",
			stats_ptr->dup_past_outside_window);
}

static void
htt_t2h_stats_rx_rem_buf_stats_print(
    struct rx_remote_buffer_mgmt_stats *stats_ptr, int concise)
{
	cdf_print("Rx Remote Buffer Statistics:\n");
	cdf_print("  %u MSDU's reaped for Rx processing\n",
			stats_ptr->remote_reaped);
	cdf_print("  %u MSDU's recycled within firmware\n",
			stats_ptr->remote_recycled);
	cdf_print("  %u MSDU's stored by Data Rx\n",
			stats_ptr->data_rx_msdus_stored);
	cdf_print("  %u HTT indications from WAL Rx MSDU\n",
			stats_ptr->wal_rx_ind);
	cdf_print("  %u HTT indications unconsumed from WAL Rx MSDU\n",
			stats_ptr->wal_rx_ind_unconsumed);
	cdf_print("  %u HTT indications from Data Rx MSDU\n",
			stats_ptr->data_rx_ind);
	cdf_print("  %u HTT indications unconsumed from Data Rx MSDU\n",
			stats_ptr->data_rx_ind_unconsumed);
	cdf_print("  %u HTT indications from ATHBUF\n",
			stats_ptr->athbuf_rx_ind);
	cdf_print("  %u Remote buffers requested for refill\n",
			stats_ptr->refill_buf_req);
	cdf_print("  %u Remote buffers filled by host\n",
			stats_ptr->refill_buf_rsp);
	cdf_print("  %u times MAC has no buffers\n",
			stats_ptr->mac_no_bufs);
	cdf_print("  %u times f/w write & read indices on MAC ring are equal\n",
			stats_ptr->fw_indices_equal);
	cdf_print("  %u times f/w has no remote buffers to post to MAC\n",
			stats_ptr->host_no_bufs);
}

static void
htt_t2h_stats_txbf_info_buf_stats_print(
	struct wlan_dbg_txbf_data_stats *stats_ptr)
{
	cdf_print("TXBF data Statistics:\n");
	cdf_print("tx_txbf_vht (0..9): ");
	cdf_print("%u, %u, %u, %u, %u, %u, %u, %u, %u, %d\n",
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
	cdf_print("rx_txbf_vht (0..9): ");
	cdf_print("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
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
	cdf_print("tx_txbf_ht (0..7): ");
	cdf_print("%u, %u, %u, %u, %u, %u, %u, %u\n",
		  stats_ptr->tx_txbf_ht[0],
		  stats_ptr->tx_txbf_ht[1],
		  stats_ptr->tx_txbf_ht[2],
		  stats_ptr->tx_txbf_ht[3],
		  stats_ptr->tx_txbf_ht[4],
		  stats_ptr->tx_txbf_ht[5],
		  stats_ptr->tx_txbf_ht[6],
		  stats_ptr->tx_txbf_ht[7]);
	cdf_print("tx_txbf_ofdm (0..7): ");
	cdf_print("%u, %u, %u, %u, %u, %u, %u, %u\n",
		  stats_ptr->tx_txbf_ofdm[0],
		  stats_ptr->tx_txbf_ofdm[1],
		  stats_ptr->tx_txbf_ofdm[2],
		  stats_ptr->tx_txbf_ofdm[3],
		  stats_ptr->tx_txbf_ofdm[4],
		  stats_ptr->tx_txbf_ofdm[5],
		  stats_ptr->tx_txbf_ofdm[6],
		  stats_ptr->tx_txbf_ofdm[7]);
	cdf_print("tx_txbf_cck (0..6): ");
	cdf_print("%u, %u, %u, %u, %u, %u, %u\n",
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
	cdf_print("TXBF snd Buffer Statistics:\n");
	cdf_print("cbf_20: ");
	cdf_print("%u, %u, %u, %u\n",
		  stats_ptr->cbf_20[0],
		  stats_ptr->cbf_20[1],
		  stats_ptr->cbf_20[2],
		  stats_ptr->cbf_20[3]);
	cdf_print("cbf_40: ");
	cdf_print("%u, %u, %u, %u\n",
		  stats_ptr->cbf_40[0],
		  stats_ptr->cbf_40[1],
		  stats_ptr->cbf_40[2],
		  stats_ptr->cbf_40[3]);
	cdf_print("cbf_80: ");
	cdf_print("%u, %u, %u, %u\n",
		  stats_ptr->cbf_80[0],
		  stats_ptr->cbf_80[1],
		  stats_ptr->cbf_80[2],
		  stats_ptr->cbf_80[3]);
	cdf_print("sounding: ");
	cdf_print("%u, %u, %u, %u, %u, %u, %u, %u, %u\n",
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
	cdf_print("Tx selfgen Buffer Statistics:\n");
	cdf_print("  %u su_ndpa\n",
			stats_ptr->su_ndpa);
	cdf_print("  %u mu_ndp\n",
			stats_ptr->mu_ndp);
	cdf_print("  %u mu_ndpa\n",
			stats_ptr->mu_ndpa);
	cdf_print("  %u mu_ndp\n",
			stats_ptr->mu_ndp);
	cdf_print("  %u mu_brpoll_1\n",
			stats_ptr->mu_brpoll_1);
	cdf_print("  %u mu_brpoll_2\n",
			stats_ptr->mu_brpoll_2);
	cdf_print("  %u mu_bar_1\n",
			stats_ptr->mu_bar_1);
	cdf_print("  %u mu_bar_2\n",
			stats_ptr->mu_bar_2);
	cdf_print("  %u cts_burst\n",
			stats_ptr->cts_burst);
	cdf_print("  %u su_ndp_err\n",
			stats_ptr->su_ndp_err);
	cdf_print("  %u su_ndpa_err\n",
			stats_ptr->su_ndpa_err);
	cdf_print("  %u mu_ndp_err\n",
			stats_ptr->mu_ndp_err);
	cdf_print("  %u mu_brp1_err\n",
			stats_ptr->mu_brp1_err);
	cdf_print("  %u mu_brp2_err\n",
			stats_ptr->mu_brp2_err);
}

static void
htt_t2h_stats_wifi2_error_stats_print(
	struct wlan_dbg_wifi2_error_stats *stats_ptr)
{
	int i;

	cdf_print("Scheduler error Statistics:\n");
	cdf_print("urrn_stats: ");
	cdf_print("%d, %d, %d\n",
		  stats_ptr->urrn_stats[0],
		  stats_ptr->urrn_stats[1],
		  stats_ptr->urrn_stats[2]);
	cdf_print("flush_errs (0..%d): ",
			WHAL_DBG_FLUSH_REASON_MAXCNT);
	for (i = 0; i < WHAL_DBG_FLUSH_REASON_MAXCNT; i++)
		cdf_print("  %u", stats_ptr->flush_errs[i]);
	cdf_print("\n");
	cdf_print("schd_stall_errs (0..3): ");
	cdf_print("%d, %d, %d, %d\n",
		  stats_ptr->schd_stall_errs[0],
		  stats_ptr->schd_stall_errs[1],
		  stats_ptr->schd_stall_errs[2],
		  stats_ptr->schd_stall_errs[3]);
	cdf_print("schd_cmd_result (0..%d): ",
			WHAL_DBG_CMD_RESULT_MAXCNT);
	for (i = 0; i < WHAL_DBG_CMD_RESULT_MAXCNT; i++)
		cdf_print("  %u", stats_ptr->schd_cmd_result[i]);
	cdf_print("\n");
	cdf_print("sifs_status (0..%d): ",
			WHAL_DBG_SIFS_STATUS_MAXCNT);
	for (i = 0; i < WHAL_DBG_SIFS_STATUS_MAXCNT; i++)
		cdf_print("  %u", stats_ptr->sifs_status[i]);
	cdf_print("\n");
	cdf_print("phy_errs (0..%d): ",
			WHAL_DBG_PHY_ERR_MAXCNT);
	for (i = 0; i < WHAL_DBG_PHY_ERR_MAXCNT; i++)
		cdf_print("  %u", stats_ptr->phy_errs[i]);
	cdf_print("\n");
	cdf_print("  %u rx_rate_inval\n",
			stats_ptr->rx_rate_inval);
}

static void
htt_t2h_rx_musu_ndpa_pkts_stats_print(
	struct rx_txbf_musu_ndpa_pkts_stats *stats_ptr)
{
	cdf_print("Rx TXBF MU/SU Packets and NDPA Statistics:\n");
	cdf_print("  %u Number of TXBF MU packets received\n",
			stats_ptr->number_mu_pkts);
	cdf_print("  %u Number of TXBF SU packets received\n",
			stats_ptr->number_su_pkts);
	cdf_print("  %u Number of TXBF directed NDPA\n",
			stats_ptr->txbf_directed_ndpa_count);
	cdf_print("  %u Number of TXBF retried NDPA\n",
			stats_ptr->txbf_ndpa_retry_count);
	cdf_print("  %u Total number of TXBF NDPA\n",
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
	cdf_print("%s\n", queued_str);
	cdf_print("%s\n", acked_str);
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

		bytes = cdf_snprint(buf_p, space, "%d,", val);
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
		bytes = cdf_snprint(buf_p, space, "%d,", val);
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
	cdf_print("Tx PPDU log elements:\n");

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

			cdf_print(" - PPDU tx to peer %d, TID %d\n", peer_id,
				  ext_tid);
			cdf_print
				("   start seq num= %u, start PN LSBs= %#04x\n",
				start_seq_num, start_pn_lsbs);
			cdf_print
				("   PPDU: %d MPDUs, (?) MSDUs, %d bytes\n",
				num_mpdus,
				 /* num_msdus - not yet computed in target */
				record->num_bytes);
			if (complete) {
				cdf_print
				      ("   enqueued: %u, completed: %u usec)\n",
				       time_enqueue_us, time_completion_us);
				cdf_print
					("   %d tries, last tx used rate %d ",
					 tries, rate_code);
				cdf_print("on %d MHz chan (flags = %#x)\n",
					  htt_rate_flags_to_mhz
					  (rate_flags), rate_flags);
				cdf_print
				      ("  enqueued and acked MPDU bitmaps:\n");
				htt_t2h_tx_ppdu_bitmaps_pr(msg_word +
					   OL_FW_TX_DBG_PPDU_ENQUEUED_LSBS_16,
							   msg_word +
					   OL_FW_TX_DBG_PPDU_BLOCK_ACK_LSBS_16);
			} else {
				cdf_print
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
				cdf_print("   MPDU bytes: %s\n", buf);
			}
			p8 += hdr->mpdu_bytes_array_len * sizeof(uint16_t);
			if (hdr->mpdu_msdus_array_len) {
				htt_make_u8_list_str((uint32_t *) p8, buf,
						     BUF_SIZE,
						     hdr->mpdu_msdus_array_len);
				cdf_print("   MPDU MSDUs: %s\n", buf);
			}
			p8 += hdr->mpdu_msdus_array_len * sizeof(uint8_t);
			if (hdr->msdu_bytes_array_len) {
				htt_make_u16_list_str((uint32_t *) p8, buf,
						      BUF_SIZE,
						      hdr->
						      msdu_bytes_array_len);
				cdf_print("   MSDU bytes: %s\n", buf);
			}
		} else {
			/* concise */
			cdf_print("start seq num = %u ", start_seq_num);
			cdf_print("enqueued and acked MPDU bitmaps:\n");
			if (complete) {
				htt_t2h_tx_ppdu_bitmaps_pr(msg_word +
					OL_FW_TX_DBG_PPDU_ENQUEUED_LSBS_16,
							   msg_word +
					OL_FW_TX_DBG_PPDU_BLOCK_ACK_LSBS_16);
			} else {
				cdf_print("(not completed)\n");
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
		cdf_print("Target doesn't support stats type %d\n", type);
		return;
	} else if (status == HTT_DBG_STATS_STATUS_ERROR) {
		cdf_print("Target couldn't upload stats type %d (no mem?)\n",
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
			cdf_print
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

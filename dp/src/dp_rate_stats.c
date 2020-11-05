/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * @file: dp_rate_stats.c
 * @breief: Core peer rate statistics processing module
 */

#include "dp_rate_stats.h"
#include "dp_rate_stats_pub.h"

#ifdef QCA_SUPPORT_RDK_STATS

/* Calculate actual BW from BW ENUM as in
 * x = 0 for 20MHz
 * x = 1 for 40MHz
 * x = 2 for 80MHz
 * x = 3 for 160MHz
 */
#define GET_BW_FROM_BW_ENUM(x) ((20) * (1 << x))

static void
wlan_peer_read_ewma_avg_rssi(struct wlan_rx_rate_stats *rx_stats)
{
	uint8_t ant, ht, cache_idx;

	for (cache_idx = 0; cache_idx < WLANSTATS_CACHE_SIZE; cache_idx++) {
		rx_stats->avg_rssi.internal =
			qdf_ewma_rx_rssi_read(&rx_stats->avg_rssi);

		for (ant = 0; ant < SS_COUNT; ant++) {
			for (ht = 0; ht < MAX_BW; ht++) {
				rx_stats->avg_rssi_ant[ant][ht].internal =
				qdf_ewma_rx_rssi_read(
					&rx_stats->avg_rssi_ant[ant][ht]);
			}
		}
		rx_stats += 1;
	}
}

static void
wlan_peer_flush_rx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			      struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_rate_stats_intf buf;
	struct wlan_peer_rx_rate_stats *rx_stats;
	uint8_t idx;

	if (!soc_stats_ctx)
		return;

	rx_stats = &stats_ctx->rate_stats->rx;

	buf.cookie = 0;
	wlan_peer_read_ewma_avg_rssi(rx_stats->stats);
	buf.stats = (struct wlan_rx_rate_stats *)rx_stats->stats;
	buf.buf_len = WLANSTATS_CACHE_SIZE * sizeof(struct wlan_rx_rate_stats);
	buf.stats_type = DP_PEER_RX_RATE_STATS;
	/* Prepare 64 bit cookie */
	/*-------------------|-------------------|
	 *  32 bit target    | 32 bit peer cookie|
	 *-------------------|-------------------|
	 */
	buf.cookie = ((((buf.cookie | soc_stats_ctx->is_lithium)
		      << WLANSTATS_PEER_COOKIE_LSB) &
		      WLANSTATS_COOKIE_PLATFORM_OFFSET) |
		      (((buf.cookie | stats_ctx->peer_cookie) &
		      WLANSTATS_COOKIE_PEER_COOKIE_OFFSET)));
	qdf_mem_copy(buf.peer_mac, stats_ctx->mac_addr, WLAN_MAC_ADDR_LEN);
	cdp_peer_flush_rate_stats(soc_stats_ctx->soc,
				  stats_ctx->pdev_id, &buf);

	soc_stats_ctx->rxs_cache_flush++;
	dp_info("rxs_cache_flush: %d", soc_stats_ctx->rxs_cache_flush);

	qdf_mem_zero(rx_stats->stats, WLANSTATS_CACHE_SIZE *
		     sizeof(struct wlan_rx_rate_stats));
	for (idx = 0; idx < WLANSTATS_CACHE_SIZE; idx++)
		rx_stats->stats[idx].rix = INVALID_CACHE_IDX;
}

static void
wlan_peer_read_sojourn_average(struct wlan_peer_tx_rate_stats *tx_stats)
{
	uint8_t tid;

	for (tid = 0; tid < CDP_DATA_TID_MAX; tid++) {
		tx_stats->sojourn.avg_sojourn_msdu[tid].internal =
		qdf_ewma_tx_lag_read(&tx_stats->sojourn.avg_sojourn_msdu[tid]);
	}
}

static void
wlan_peer_flush_tx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			      struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_rate_stats_intf buf;
	struct wlan_peer_tx_rate_stats *tx_stats;
	uint8_t idx;
	uint8_t tid;

	if (!soc_stats_ctx)
		return;

	tx_stats = &stats_ctx->rate_stats->tx;

	buf.cookie = 0;
	buf.stats = (struct wlan_tx_rate_stats *)tx_stats->stats;
	buf.buf_len = (WLANSTATS_CACHE_SIZE * sizeof(struct wlan_tx_rate_stats)
		       + sizeof(struct wlan_tx_sojourn_stats));
	buf.stats_type = DP_PEER_TX_RATE_STATS;
	/* Prepare 64 bit cookie */
	/*-------------------|-------------------|
	 *  32 bit target    | 32 bit peer cookie|
	 *-------------------|-------------------|
	 */
	buf.cookie = ((((buf.cookie | soc_stats_ctx->is_lithium)
		      << WLANSTATS_PEER_COOKIE_LSB) &
		      WLANSTATS_COOKIE_PLATFORM_OFFSET) |
		      (((buf.cookie | stats_ctx->peer_cookie) &
		      WLANSTATS_COOKIE_PEER_COOKIE_OFFSET)));

	wlan_peer_read_sojourn_average(tx_stats);
	qdf_mem_copy(buf.peer_mac, stats_ctx->mac_addr, WLAN_MAC_ADDR_LEN);
	cdp_peer_flush_rate_stats(soc_stats_ctx->soc,
				  stats_ctx->pdev_id, &buf);

	soc_stats_ctx->txs_cache_flush++;
	dp_info("txs_cache_flush: %d", soc_stats_ctx->txs_cache_flush);

	qdf_mem_zero(tx_stats->stats, WLANSTATS_CACHE_SIZE *
		     sizeof(struct wlan_tx_rate_stats));

	for (tid = 0; tid < WLAN_DATA_TID_MAX; tid++) {
		tx_stats->sojourn.sum_sojourn_msdu[tid] = 0;
		tx_stats->sojourn.num_msdus[tid] = 0;
	}
	for (idx = 0; idx < WLANSTATS_CACHE_SIZE; idx++)
		tx_stats->stats[idx].rix = INVALID_CACHE_IDX;
}

static void
wlan_peer_flush_tx_link_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			      struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_rate_stats_intf buf;
	struct wlan_peer_tx_link_stats *tx_stats;
	uint8_t bw_max_idx;

	if (!soc_stats_ctx) {
		dp_info("soc stats context is NULL\n");
		return;
	}

	tx_stats = &stats_ctx->link_metrics->tx;

	buf.cookie = 0;
	buf.stats = (struct wlan_tx_link_stats *)&tx_stats->stats;
	buf.buf_len = sizeof(struct wlan_peer_tx_link_stats);
	buf.stats_type = DP_PEER_TX_LINK_STATS;
	/* Prepare 64 bit cookie */
	/*-------------------|-------------------|
	 *  32 bit target    | 32 bit peer cookie|
	 *-------------------|-------------------|
	 */
	buf.cookie = ((((buf.cookie | soc_stats_ctx->is_lithium)
		      << WLANSTATS_PEER_COOKIE_LSB) &
		      WLANSTATS_COOKIE_PLATFORM_OFFSET) |
		      (((buf.cookie | stats_ctx->peer_cookie) &
		      WLANSTATS_COOKIE_PEER_COOKIE_OFFSET)));

	tx_stats->stats.ack_rssi.internal =
			qdf_ewma_rx_rssi_read(&tx_stats->stats.ack_rssi);
	tx_stats->stats.phy_rate_actual_su =
			dp_ath_rate_out(tx_stats->stats.phy_rate_lpf_avg_su);
	tx_stats->stats.phy_rate_actual_mu =
			dp_ath_rate_out(tx_stats->stats.phy_rate_lpf_avg_mu);

	if (tx_stats->stats.num_ppdus) {
		tx_stats->stats.bw.usage_avg = tx_stats->stats.bw.usage_total /
					       tx_stats->stats.num_ppdus;
		bw_max_idx = tx_stats->stats.bw.usage_max;
		tx_stats->stats.bw.usage_max =
			(tx_stats->stats.bw.usage_counter[bw_max_idx] * 100) /
			 tx_stats->stats.num_ppdus;
	}

	if (tx_stats->stats.mpdu_success)
		tx_stats->stats.pkt_error_rate =
			(tx_stats->stats.mpdu_failed * 100) /
			 tx_stats->stats.mpdu_success;

	qdf_mem_copy(buf.peer_mac, stats_ctx->mac_addr, WLAN_MAC_ADDR_LEN);
	cdp_peer_flush_rate_stats(soc_stats_ctx->soc,
				  stats_ctx->pdev_id, &buf);

	qdf_mem_zero(&tx_stats->stats, sizeof(struct wlan_tx_link_stats));
}

static void
wlan_peer_flush_rx_link_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			      struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_rate_stats_intf buf;
	struct wlan_peer_rx_link_stats *rx_stats;
	uint8_t bw_max_idx;

	if (!soc_stats_ctx) {
		dp_info("soc stats context is NULL\n");
		return;
	}

	rx_stats = &stats_ctx->link_metrics->rx;

	buf.cookie = 0;
	buf.stats = (struct wlan_rx_link_stats *)&rx_stats->stats;
	buf.buf_len = sizeof(struct wlan_peer_rx_link_stats);
	buf.stats_type = DP_PEER_RX_LINK_STATS;
	/* Prepare 64 bit cookie */
	/*-------------------|-------------------|
	 *  32 bit target    | 32 bit peer cookie|
	 *-------------------|-------------------|
	 */
	buf.cookie = ((((buf.cookie | soc_stats_ctx->is_lithium)
		      << WLANSTATS_PEER_COOKIE_LSB) &
		      WLANSTATS_COOKIE_PLATFORM_OFFSET) |
		      (((buf.cookie | stats_ctx->peer_cookie) &
		      WLANSTATS_COOKIE_PEER_COOKIE_OFFSET)));

	rx_stats->stats.su_rssi.internal =
			qdf_ewma_rx_rssi_read(&rx_stats->stats.su_rssi);
	rx_stats->stats.phy_rate_actual_su =
			dp_ath_rate_out(rx_stats->stats.phy_rate_lpf_avg_su);
	rx_stats->stats.phy_rate_actual_mu =
			dp_ath_rate_out(rx_stats->stats.phy_rate_lpf_avg_mu);
	if (rx_stats->stats.num_ppdus) {
		rx_stats->stats.bw.usage_avg = rx_stats->stats.bw.usage_total /
					       rx_stats->stats.num_ppdus;
		bw_max_idx = rx_stats->stats.bw.usage_max;
		rx_stats->stats.bw.usage_max =
			(rx_stats->stats.bw.usage_counter[bw_max_idx] * 100) /
			 rx_stats->stats.num_ppdus;
	}

	if (rx_stats->stats.num_mpdus)
		rx_stats->stats.pkt_error_rate =
					(rx_stats->stats.mpdu_retries * 100) /
					 rx_stats->stats.num_mpdus;

	qdf_mem_copy(buf.peer_mac, stats_ctx->mac_addr, WLAN_MAC_ADDR_LEN);
	cdp_peer_flush_rate_stats(soc_stats_ctx->soc,
				  stats_ctx->pdev_id, &buf);

	qdf_mem_zero(&rx_stats->stats, sizeof(struct wlan_rx_link_stats));
}

static void
wlan_peer_flush_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			   struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_tx_rate_stats *tx_stats;
	struct wlan_peer_rx_rate_stats *rx_stats;
	struct wlan_peer_tx_link_stats *tx_link_stats;
	struct wlan_peer_rx_link_stats *rx_link_stats;

	if (soc_stats_ctx->stats_ver == RDK_RATE_STATS ||
	    soc_stats_ctx->stats_ver == RDK_ALL_STATS) {
		tx_stats = &stats_ctx->rate_stats->tx;
		rx_stats = &stats_ctx->rate_stats->rx;

		RATE_STATS_LOCK_ACQUIRE(&tx_stats->lock);
		wlan_peer_flush_tx_rate_stats(soc_stats_ctx, stats_ctx);
		RATE_STATS_LOCK_RELEASE(&tx_stats->lock);

		RATE_STATS_LOCK_ACQUIRE(&rx_stats->lock);
		wlan_peer_flush_rx_rate_stats(soc_stats_ctx, stats_ctx);
		RATE_STATS_LOCK_RELEASE(&rx_stats->lock);
	}

	if (soc_stats_ctx->stats_ver == RDK_LINK_STATS ||
	    soc_stats_ctx->stats_ver == RDK_ALL_STATS) {
		tx_link_stats = &stats_ctx->link_metrics->tx;
		rx_link_stats = &stats_ctx->link_metrics->rx;

		RATE_STATS_LOCK_ACQUIRE(&tx_link_stats->lock);
		wlan_peer_flush_tx_link_stats(soc_stats_ctx, stats_ctx);
		RATE_STATS_LOCK_RELEASE(&tx_link_stats->lock);

		RATE_STATS_LOCK_ACQUIRE(&rx_link_stats->lock);
		wlan_peer_flush_rx_link_stats(soc_stats_ctx, stats_ctx);
		RATE_STATS_LOCK_RELEASE(&rx_link_stats->lock);
	}
}

void wlan_peer_rate_stats_flush_req(void *ctx, enum WDI_EVENT event,
				    void *buf, uint16_t peer_id,
				    uint32_t type)
{
	if (buf)
		wlan_peer_flush_rate_stats(ctx, buf);
}

static inline void
__wlan_peer_update_rx_rate_stats(struct wlan_rx_rate_stats *__rx_stats,
				 struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	uint8_t ant, ht;

	if (cdp_rx_ppdu->rix == -1) {
		__rx_stats->rix = cdp_rx_ppdu->rix;
	} else {
		__rx_stats->rix = ASSEMBLE_STATS_CODE(cdp_rx_ppdu->rix,
						      cdp_rx_ppdu->u.nss,
						      cdp_rx_ppdu->u.mcs,
						      cdp_rx_ppdu->u.bw);
	}

	__rx_stats->rate = cdp_rx_ppdu->rx_ratekbps;
	__rx_stats->num_bytes += cdp_rx_ppdu->num_bytes;
	__rx_stats->num_msdus += cdp_rx_ppdu->num_msdu;
	__rx_stats->num_mpdus += cdp_rx_ppdu->num_mpdu;
	__rx_stats->num_retries += cdp_rx_ppdu->retries;
	__rx_stats->num_ppdus += 1;

	if (cdp_rx_ppdu->u.gi)
		__rx_stats->num_sgi++;

	qdf_ewma_rx_rssi_add(&__rx_stats->avg_rssi, cdp_rx_ppdu->rssi);

	for (ant = 0; ant < SS_COUNT; ant++) {
		for (ht = 0; ht < MAX_BW; ht++) {
			qdf_ewma_rx_rssi_add(&__rx_stats->avg_rssi_ant[ant][ht],
					     cdp_rx_ppdu->rssi_chain[ant][ht]);
		}
	}
}

static void
wlan_peer_update_tx_link_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			       struct cdp_tx_completion_ppdu *cdp_tx_ppdu)
{
	struct cdp_tx_completion_ppdu_user *ppdu_user;
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_tx_link_stats *tx_stats;
	uint8_t user_idx;

	if (soc_stats_ctx->stats_ver != RDK_LINK_STATS &&
	    soc_stats_ctx->stats_ver != RDK_ALL_STATS)
		return;

	for (user_idx = 0; user_idx < cdp_tx_ppdu->num_users; user_idx++) {
		ppdu_user = &cdp_tx_ppdu->user[user_idx];

		STATS_CTX_LOCK_ACQUIRE(&soc_stats_ctx->tx_ctx_lock);
		stats_ctx = cdp_peer_get_rdkstats_ctx(soc_stats_ctx->soc,
						      cdp_tx_ppdu->vdev_id,
						      ppdu_user->mac_addr);

		if (qdf_unlikely(!stats_ctx)) {
			qdf_warn("peer rate stats ctx is NULL, return");
			qdf_warn("peer_mac:  " QDF_MAC_ADDR_FMT,
				 QDF_MAC_ADDR_REF(ppdu_user->mac_addr));
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
			continue;
		}

		tx_stats = &stats_ctx->link_metrics->tx.stats;

		RATE_STATS_LOCK_ACQUIRE(&stats_ctx->link_metrics->tx.lock);

		tx_stats->num_ppdus += ppdu_user->long_retries + 1;
		tx_stats->bytes += ppdu_user->success_bytes;
		tx_stats->mpdu_failed += ppdu_user->mpdu_failed;
		tx_stats->mpdu_success += ppdu_user->mpdu_success;

		if (ppdu_user->ppdu_type == DP_PPDU_TYPE_SU) {
			tx_stats->phy_rate_lpf_avg_su =
				dp_ath_rate_lpf(tx_stats->phy_rate_lpf_avg_su,
						ppdu_user->tx_ratekbps);
		} else if (ppdu_user->ppdu_type == DP_PPDU_TYPE_MU_OFDMA ||
			   ppdu_user->ppdu_type == DP_PPDU_TYPE_MU_MIMO) {
			tx_stats->phy_rate_lpf_avg_mu =
				dp_ath_rate_lpf(tx_stats->phy_rate_lpf_avg_mu,
						ppdu_user->tx_ratekbps);

			if (ppdu_user->ppdu_type == DP_PPDU_TYPE_MU_OFDMA)
				tx_stats->ofdma_usage++;

			if (ppdu_user->ppdu_type == DP_PPDU_TYPE_MU_MIMO)
				tx_stats->mu_mimo_usage++;
		}

		tx_stats->bw.usage_total += GET_BW_FROM_BW_ENUM(ppdu_user->bw);

		if (ppdu_user->bw < BW_USAGE_MAX_SIZE) {
			if (tx_stats->bw.usage_max < ppdu_user->bw)
				tx_stats->bw.usage_max = ppdu_user->bw;
			tx_stats->bw.usage_counter[ppdu_user->bw]++;
		}

		if (ppdu_user->ack_rssi_valid)
			qdf_ewma_rx_rssi_add(&tx_stats->ack_rssi,
					     ppdu_user->usr_ack_rssi);

		RATE_STATS_LOCK_RELEASE(&stats_ctx->link_metrics->tx.lock);
		STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
	}
}

static void
wlan_peer_update_rx_link_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			       struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	struct cdp_rx_stats_ppdu_user *ppdu_user;
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_rx_link_stats *rx_stats;
	uint8_t user_idx;

	if (soc_stats_ctx->stats_ver != RDK_LINK_STATS &&
	    soc_stats_ctx->stats_ver != RDK_ALL_STATS)
		return;

	for (user_idx = 0;
	     user_idx < cdp_rx_ppdu->num_users && user_idx < CDP_MU_MAX_USERS;
	     user_idx++) {
		ppdu_user = &cdp_rx_ppdu->user[user_idx];

		STATS_CTX_LOCK_ACQUIRE(&soc_stats_ctx->rx_ctx_lock);
		stats_ctx = cdp_peer_get_rdkstats_ctx(soc_stats_ctx->soc,
						      ppdu_user->vdev_id,
						      ppdu_user->mac_addr);

		if (qdf_unlikely(!stats_ctx)) {
			qdf_warn("peer rate stats ctx is NULL, return");
			qdf_warn("peer_mac:  " QDF_MAC_ADDR_FMT,
				 QDF_MAC_ADDR_REF(ppdu_user->mac_addr));
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
			continue;
		}

		rx_stats = &stats_ctx->link_metrics->rx.stats;

		RATE_STATS_LOCK_ACQUIRE(&stats_ctx->link_metrics->rx.lock);

		rx_stats->num_ppdus++;
		rx_stats->bytes += cdp_rx_ppdu->num_bytes;
		rx_stats->mpdu_retries += ppdu_user->retries;
		rx_stats->num_mpdus += ppdu_user->mpdu_cnt_fcs_ok;

		if (cdp_rx_ppdu->u.ppdu_type == DP_PPDU_TYPE_SU) {
			rx_stats->phy_rate_lpf_avg_su =
				dp_ath_rate_lpf(rx_stats->phy_rate_lpf_avg_su,
						cdp_rx_ppdu->rx_ratekbps);
			qdf_ewma_rx_rssi_add(&rx_stats->su_rssi,
					     cdp_rx_ppdu->rssi);
		} else if (cdp_rx_ppdu->u.ppdu_type == DP_PPDU_TYPE_MU_OFDMA ||
			   cdp_rx_ppdu->u.ppdu_type == DP_PPDU_TYPE_MU_MIMO) {
			rx_stats->phy_rate_lpf_avg_mu =
				dp_ath_rate_lpf(rx_stats->phy_rate_lpf_avg_mu,
						cdp_rx_ppdu->rx_ratekbps);

			if (cdp_rx_ppdu->u.ppdu_type == DP_PPDU_TYPE_MU_OFDMA)
				rx_stats->ofdma_usage++;

			if (cdp_rx_ppdu->u.ppdu_type == DP_PPDU_TYPE_MU_MIMO)
				rx_stats->mu_mimo_usage++;
		}

		rx_stats->bw.usage_total +=
					GET_BW_FROM_BW_ENUM(cdp_rx_ppdu->u.bw);

		if (cdp_rx_ppdu->u.bw < BW_USAGE_MAX_SIZE) {
			if (rx_stats->bw.usage_max < cdp_rx_ppdu->u.bw)
				rx_stats->bw.usage_max = cdp_rx_ppdu->u.bw;
			rx_stats->bw.usage_counter[cdp_rx_ppdu->u.bw]++;
		}

		RATE_STATS_LOCK_RELEASE(&stats_ctx->link_metrics->rx.lock);
		STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
	}
}

static void
wlan_peer_update_rx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			       struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_peer_rx_rate_stats *rx_stats;
	struct wlan_rx_rate_stats *__rx_stats;
	struct cdp_rx_stats_ppdu_user *ppdu_user;
	uint8_t cache_idx;
	uint8_t user_idx;
	uint8_t max_users;
	bool idx_match = false;

	if (soc_stats_ctx->stats_ver != RDK_RATE_STATS &&
	    soc_stats_ctx->stats_ver != RDK_ALL_STATS)
		return;

	user_idx = 0;
	max_users = QDF_MIN(cdp_rx_ppdu->num_users, CDP_MU_MAX_USERS);

	do {
		STATS_CTX_LOCK_ACQUIRE(&soc_stats_ctx->rx_ctx_lock);
		if (cdp_rx_ppdu->u.ppdu_type != DP_PPDU_TYPE_SU) {
			ppdu_user = &cdp_rx_ppdu->user[user_idx];

			stats_ctx =
			cdp_peer_get_rdkstats_ctx(soc_stats_ctx->soc,
						  ppdu_user->vdev_id,
						  ppdu_user->mac_addr);
		} else {
			stats_ctx =
			cdp_peer_get_rdkstats_ctx(soc_stats_ctx->soc,
						  cdp_rx_ppdu->vdev_id,
						  cdp_rx_ppdu->mac_addr);
		}

		if (qdf_unlikely(!stats_ctx)) {
			qdf_warn("peer rate stats ctx is NULL, return");
			qdf_warn("peer_mac:  " QDF_MAC_ADDR_FMT,
				 QDF_MAC_ADDR_REF(cdp_rx_ppdu->mac_addr));
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
			continue;
		}

		rx_stats = &stats_ctx->rate_stats->rx;

		if (qdf_unlikely(!cdp_rx_ppdu->rx_ratekbps ||
				 cdp_rx_ppdu->rix > DP_RATE_TABLE_SIZE)) {
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
			return;
		}

		RATE_STATS_LOCK_ACQUIRE(&rx_stats->lock);
		if (qdf_likely(rx_stats->cur_rix == cdp_rx_ppdu->rix)) {
			__rx_stats = &rx_stats->stats[rx_stats->cur_cache_idx];
			__wlan_peer_update_rx_rate_stats(__rx_stats,
							 cdp_rx_ppdu);
			soc_stats_ctx->rxs_last_idx_cache_hit++;
			RATE_STATS_LOCK_RELEASE(&rx_stats->lock);
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
			continue;
		}

		/* check if cache is available */
		for (cache_idx = 0; cache_idx < WLANSTATS_CACHE_SIZE; cache_idx++) {
			__rx_stats = &rx_stats->stats[cache_idx];
			if ((__rx_stats->rix == INVALID_CACHE_IDX) ||
			    (__rx_stats->rix == cdp_rx_ppdu->rix)) {
				idx_match = true;
				break;
			}
		}
		/* if index matches or found empty index, update stats to that
		 * cache index else flush cache and update stats to cache index
		 * zero
		 */
		if (idx_match) {
			__wlan_peer_update_rx_rate_stats(__rx_stats,
							 cdp_rx_ppdu);
			rx_stats->cur_rix = cdp_rx_ppdu->rix;
			rx_stats->cur_cache_idx = cache_idx;
			soc_stats_ctx->rxs_cache_hit++;
			RATE_STATS_LOCK_RELEASE(&rx_stats->lock);
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
			continue;
		} else {
			wlan_peer_flush_rx_rate_stats(soc_stats_ctx, stats_ctx);
			__rx_stats = &rx_stats->stats[0];
			__wlan_peer_update_rx_rate_stats(__rx_stats,
							 cdp_rx_ppdu);
			rx_stats->cur_rix = cdp_rx_ppdu->rix;
			rx_stats->cur_cache_idx = 0;
			soc_stats_ctx->rxs_cache_miss++;
		}
		RATE_STATS_LOCK_RELEASE(&rx_stats->lock);
		STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
	} while (++user_idx < max_users);
}

static inline void
__wlan_peer_update_tx_rate_stats(struct wlan_tx_rate_stats *__tx_stats,
				 struct cdp_tx_completion_ppdu_user *ppdu_user)
{
	uint8_t num_ppdus;
	uint8_t mpdu_attempts;
	uint8_t mpdu_success;

	num_ppdus = ppdu_user->long_retries + 1;
	mpdu_attempts = num_ppdus * ppdu_user->mpdu_tried_ucast;
	mpdu_success = ppdu_user->mpdu_tried_ucast - ppdu_user->mpdu_failed;
	if (ppdu_user->rix == -1) {
		__tx_stats->rix = ppdu_user->rix;
	} else {
		__tx_stats->rix = ASSEMBLE_STATS_CODE(ppdu_user->rix,
						      ppdu_user->nss,
						      ppdu_user->mcs,
						      ppdu_user->bw);
	}

	__tx_stats->rate = ppdu_user->tx_ratekbps;
	__tx_stats->num_ppdus += num_ppdus;
	__tx_stats->mpdu_attempts += mpdu_attempts;
	__tx_stats->mpdu_success += mpdu_success;
	__tx_stats->num_msdus += ppdu_user->success_msdus;
	__tx_stats->num_bytes += ppdu_user->success_bytes;
	__tx_stats->num_retries += ppdu_user->long_retries + ppdu_user->short_retries;
}

static void
wlan_peer_update_tx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			       struct cdp_tx_completion_ppdu *cdp_tx_ppdu)
{
	struct cdp_tx_completion_ppdu_user *ppdu_user;
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_peer_tx_rate_stats *tx_stats;
	struct wlan_tx_rate_stats *__tx_stats;
	uint8_t cache_idx;
	uint8_t user_idx;
	bool idx_match = false;

	if (soc_stats_ctx->stats_ver != RDK_RATE_STATS &&
	    soc_stats_ctx->stats_ver != RDK_ALL_STATS)
		return;

	for (user_idx = 0; user_idx < cdp_tx_ppdu->num_users; user_idx++) {
		ppdu_user = &cdp_tx_ppdu->user[user_idx];

		STATS_CTX_LOCK_ACQUIRE(&soc_stats_ctx->tx_ctx_lock);
		stats_ctx = cdp_peer_get_rdkstats_ctx(soc_stats_ctx->soc,
						      cdp_tx_ppdu->vdev_id,
						      ppdu_user->mac_addr);

		if (qdf_unlikely(!ppdu_user->tx_ratekbps || !ppdu_user->rix ||
				 ppdu_user->rix > DP_RATE_TABLE_SIZE)) {
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
			continue;
		}

		if (qdf_unlikely(!stats_ctx)) {
			qdf_debug("peer rate stats ctx is NULL, investigate");
			qdf_debug("peer_mac: " QDF_MAC_ADDR_FMT,
				 QDF_MAC_ADDR_REF(ppdu_user->mac_addr));
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
			continue;
		}

		tx_stats = &stats_ctx->rate_stats->tx;
		RATE_STATS_LOCK_ACQUIRE(&tx_stats->lock);

		if (qdf_likely(tx_stats->cur_rix == ppdu_user->rix)) {
			__tx_stats = &tx_stats->stats[tx_stats->cur_cache_idx];
			__wlan_peer_update_tx_rate_stats(__tx_stats, ppdu_user);
			soc_stats_ctx->txs_last_idx_cache_hit++;
			RATE_STATS_LOCK_RELEASE(&tx_stats->lock);
			STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
			continue;
		}

		/* check if cache is available */
		for (cache_idx = 0; cache_idx < WLANSTATS_CACHE_SIZE;
					cache_idx++) {
			__tx_stats = &tx_stats->stats[cache_idx];

			if ((__tx_stats->rix == INVALID_CACHE_IDX) ||
			    (__tx_stats->rix == ppdu_user->rix)) {
				idx_match = true;
				break;
			}
		}
		/* if index matches or found empty index,
		 * update stats to that cache index
		 * else flush cache and update stats to cache index zero
		 */
		if (idx_match) {
			soc_stats_ctx->txs_cache_hit++;

			__wlan_peer_update_tx_rate_stats(__tx_stats,
							 ppdu_user);
			tx_stats->cur_rix = ppdu_user->rix;
			tx_stats->cur_cache_idx = cache_idx;
		} else {
			soc_stats_ctx->txs_cache_miss++;

			wlan_peer_flush_tx_rate_stats(soc_stats_ctx, stats_ctx);
			__tx_stats = &tx_stats->stats[0];
			__wlan_peer_update_tx_rate_stats(__tx_stats,
							 ppdu_user);
			tx_stats->cur_rix = ppdu_user->rix;
			tx_stats->cur_cache_idx = 0;
		}
		RATE_STATS_LOCK_RELEASE(&tx_stats->lock);
		STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
	}
}

static void
wlan_peer_update_sojourn_stats(struct wlan_soc_rate_stats_ctx  *soc_stats_ctx,
			       struct cdp_tx_sojourn_stats *sojourn_stats)
{
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_peer_tx_rate_stats *tx_stats;
	uint8_t tid;

	if (soc_stats_ctx->stats_ver != RDK_RATE_STATS &&
	    soc_stats_ctx->stats_ver != RDK_ALL_STATS)
		return;

	stats_ctx = (struct wlan_peer_rate_stats_ctx *)sojourn_stats->cookie;

	if (qdf_unlikely(!stats_ctx)) {
		qdf_warn("peer rate stats ctx is NULL, return");
		return;
	}

	tx_stats = &stats_ctx->rate_stats->tx;

	RATE_STATS_LOCK_ACQUIRE(&tx_stats->lock);
	for (tid = 0; tid < CDP_DATA_TID_MAX; tid++) {
		tx_stats->sojourn.avg_sojourn_msdu[tid].internal =
			sojourn_stats->avg_sojourn_msdu[tid].internal;

		tx_stats->sojourn.sum_sojourn_msdu[tid] +=
			sojourn_stats->sum_sojourn_msdu[tid];

		tx_stats->sojourn.num_msdus[tid] +=
			sojourn_stats->num_msdus[tid];
	}
	RATE_STATS_LOCK_RELEASE(&tx_stats->lock);
}

void wlan_peer_update_rate_stats(void *ctx,
				 enum WDI_EVENT event,
				 void *buf, uint16_t peer_id,
				 uint32_t stats_type)
{
	struct wlan_soc_rate_stats_ctx *soc_stats_ctx;
	struct cdp_tx_completion_ppdu *cdp_tx_ppdu;
	struct cdp_rx_indication_ppdu *cdp_rx_ppdu;
	struct cdp_tx_sojourn_stats *sojourn_stats;
	qdf_nbuf_t nbuf;

	if (qdf_unlikely(!buf))
		return;

	if (qdf_unlikely(!ctx))
		return;

	soc_stats_ctx = ctx;
	nbuf = buf;

	switch (event) {
	case WDI_EVENT_TX_PPDU_DESC:
		cdp_tx_ppdu = (struct cdp_tx_completion_ppdu *)
					qdf_nbuf_data(nbuf);
		wlan_peer_update_tx_rate_stats(soc_stats_ctx, cdp_tx_ppdu);
		wlan_peer_update_tx_link_stats(soc_stats_ctx, cdp_tx_ppdu);
		qdf_nbuf_free(nbuf);
		break;
	case WDI_EVENT_RX_PPDU_DESC:
		cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)
					qdf_nbuf_data(nbuf);
		wlan_peer_update_rx_rate_stats(soc_stats_ctx, cdp_rx_ppdu);
		wlan_peer_update_rx_link_stats(soc_stats_ctx, cdp_rx_ppdu);
		qdf_nbuf_free(nbuf);
		break;
	case WDI_EVENT_TX_SOJOURN_STAT:
		/* sojourn stats buffer is statically allocated buffer
		 * at pdev level, do not free it
		 */
		sojourn_stats = (struct cdp_tx_sojourn_stats *)
					qdf_nbuf_data(nbuf);
		wlan_peer_update_sojourn_stats(soc_stats_ctx, sojourn_stats);
		break;
	default:
		qdf_err("Err, Invalid type");
	}
}

void wlan_peer_create_event_handler(void *ctx, enum WDI_EVENT event,
				    void *buf, uint16_t peer_id,
				    uint32_t type)
{
	struct cdp_peer_cookie *peer_info;
	struct wlan_peer_rate_stats_ctx *stats;
	struct wlan_soc_rate_stats_ctx *soc_stats_ctx;
	uint8_t idx;

	soc_stats_ctx = ctx;

	peer_info = (struct cdp_peer_cookie *)buf;
	stats = qdf_mem_malloc(sizeof(*stats));
	if (!stats) {
		qdf_err("malloc failed, returning NULL");
		return;
	}

	if (soc_stats_ctx->stats_ver == RDK_RATE_STATS ||
	    soc_stats_ctx->stats_ver == RDK_ALL_STATS) {
		stats->rate_stats =
			qdf_mem_malloc(sizeof(struct wlan_peer_rate_stats));
		if (!stats->rate_stats) {
			qdf_err("malloc failed");
			goto peer_create_fail1;
		}
		RATE_STATS_LOCK_CREATE(&stats->rate_stats->tx.lock);
		RATE_STATS_LOCK_CREATE(&stats->rate_stats->rx.lock);
		for (idx = 0; idx < WLANSTATS_CACHE_SIZE; idx++) {
			stats->rate_stats->tx.stats[idx].rix =
							INVALID_CACHE_IDX;
			stats->rate_stats->rx.stats[idx].rix =
							INVALID_CACHE_IDX;
		}
	}

	if (soc_stats_ctx->stats_ver == RDK_LINK_STATS ||
	    soc_stats_ctx->stats_ver == RDK_ALL_STATS) {
		stats->link_metrics =
			qdf_mem_malloc(sizeof(struct wlan_peer_link_metrics));
		if (!stats->link_metrics) {
			qdf_err("malloc failed");
			goto peer_create_fail2;
		}
		RATE_STATS_LOCK_CREATE(&stats->link_metrics->tx.lock);
		RATE_STATS_LOCK_CREATE(&stats->link_metrics->rx.lock);
	}

	qdf_mem_copy(stats->mac_addr, peer_info->mac_addr, QDF_MAC_ADDR_SIZE);
	stats->peer_cookie = peer_info->cookie;
	stats->pdev_id = peer_info->pdev_id;

	peer_info->ctx = (void *)stats;
	return;

peer_create_fail2:
	if (soc_stats_ctx->stats_ver == RDK_RATE_STATS ||
	    soc_stats_ctx->stats_ver == RDK_ALL_STATS) {
		RATE_STATS_LOCK_DESTROY(&stats->rate_stats->tx.lock);
		RATE_STATS_LOCK_DESTROY(&stats->rate_stats->rx.lock);
		qdf_mem_free(stats->rate_stats);
		stats->rate_stats = NULL;
	}
peer_create_fail1:
	qdf_mem_free(stats);
}

void wlan_peer_destroy_event_handler(void *ctx, enum WDI_EVENT event,
				     void *buf, uint16_t peer_id,
				     uint32_t type)
{
	struct cdp_peer_cookie *peer_info;
	struct wlan_peer_rate_stats_ctx *stats;
	struct wlan_soc_rate_stats_ctx *soc_stats_ctx;

	soc_stats_ctx = ctx;

	peer_info = (struct cdp_peer_cookie *)buf;
	stats = (struct wlan_peer_rate_stats_ctx *)peer_info->ctx;

	STATS_CTX_LOCK_ACQUIRE(&soc_stats_ctx->tx_ctx_lock);
	STATS_CTX_LOCK_ACQUIRE(&soc_stats_ctx->rx_ctx_lock);
	if (stats) {
		wlan_peer_flush_rate_stats(ctx, stats);
		if (stats->rate_stats) {
			RATE_STATS_LOCK_DESTROY(&stats->rate_stats->tx.lock);
			RATE_STATS_LOCK_DESTROY(&stats->rate_stats->rx.lock);
			qdf_mem_free(stats->rate_stats);
		}
		if (stats->link_metrics) {
			RATE_STATS_LOCK_DESTROY(&stats->link_metrics->tx.lock);
			RATE_STATS_LOCK_DESTROY(&stats->link_metrics->rx.lock);
			qdf_mem_free(stats->link_metrics);
		}
		qdf_mem_free(stats);
		dp_info("Debug Deinitialized rate stats");
	}
	STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->rx_ctx_lock);
	STATS_CTX_LOCK_RELEASE(&soc_stats_ctx->tx_ctx_lock);
}
#endif

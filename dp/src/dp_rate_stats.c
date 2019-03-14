/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

static void
wlan_peer_flush_rx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			      struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_rate_stats_intf buf;
	struct wlan_peer_rx_rate_stats *rx_stats;

	if (!soc_stats_ctx)
		return;
	rx_stats = &stats_ctx->rx;

	buf.stats = (struct wlan_rx_rate_stats *)rx_stats->stats;
	buf.buf_len = WLANSTATS_CACHE_SIZE * sizeof(struct wlan_rx_rate_stats);
	buf.stats_type = DP_PEER_RX_RATE_STATS;
	buf.cookie = stats_ctx->peer_cookie;
	qdf_mem_copy(buf.peer_mac, stats_ctx->mac_addr, WLAN_MAC_ADDR_LEN);
	cdp_peer_flush_rate_stats(soc_stats_ctx->soc,
				  stats_ctx->pdev, &buf);

	soc_stats_ctx->rxs_cache_flush++;
	qdf_info("rxs_cache_flush: %d", soc_stats_ctx->rxs_cache_flush);

	qdf_mem_zero(rx_stats->stats, WLANSTATS_CACHE_SIZE *
		     sizeof(struct wlan_rx_rate_stats));
}

static void
wlan_peer_flush_tx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			      struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_rate_stats_intf buf;
	struct wlan_peer_tx_rate_stats *tx_stats;

	if (!soc_stats_ctx)
		return;

	tx_stats = &stats_ctx->tx;
	buf.stats = (struct wlan_tx_rate_stats *)tx_stats->stats;
	buf.buf_len = (WLANSTATS_CACHE_SIZE * sizeof(struct wlan_tx_rate_stats)
		       + sizeof(struct wlan_tx_sojourn_stats));
	buf.stats_type = DP_PEER_TX_RATE_STATS;
	buf.cookie = stats_ctx->peer_cookie;
	qdf_mem_copy(buf.peer_mac, stats_ctx->mac_addr, WLAN_MAC_ADDR_LEN);
	cdp_peer_flush_rate_stats(soc_stats_ctx->soc,
				  stats_ctx->pdev, &buf);

	soc_stats_ctx->txs_cache_flush++;
	qdf_info("txs_cache_flush: %d", soc_stats_ctx->txs_cache_flush);

	qdf_mem_zero(tx_stats->stats, WLANSTATS_CACHE_SIZE *
		     sizeof(struct wlan_tx_rate_stats) +
		     sizeof(struct wlan_tx_sojourn_stats));
}

static void
wlan_peer_flush_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			   struct wlan_peer_rate_stats_ctx *stats_ctx)
{
	struct wlan_peer_tx_rate_stats *tx_stats;
	struct wlan_peer_rx_rate_stats *rx_stats;

	tx_stats = &stats_ctx->tx;
	rx_stats = &stats_ctx->rx;

	RATE_STATS_LOCK_ACQUIRE(&tx_stats->lock);
	wlan_peer_flush_tx_rate_stats(soc_stats_ctx, stats_ctx);
	RATE_STATS_LOCK_RELEASE(&tx_stats->lock);

	RATE_STATS_LOCK_ACQUIRE(&rx_stats->lock);
	wlan_peer_flush_rx_rate_stats(soc_stats_ctx, stats_ctx);
	RATE_STATS_LOCK_RELEASE(&rx_stats->lock);
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

	__rx_stats->rix = cdp_rx_ppdu->rix;
	__rx_stats->rate = cdp_rx_ppdu->rx_ratekbps;
	__rx_stats->num_bytes += cdp_rx_ppdu->num_bytes;
	__rx_stats->num_msdus += cdp_rx_ppdu->num_msdu;
	__rx_stats->num_mpdus += cdp_rx_ppdu->num_mpdu;
	__rx_stats->num_retries += cdp_rx_ppdu->retries;
	__rx_stats->num_ppdus += 1;

	if (cdp_rx_ppdu->u.gi)
		__rx_stats->num_sgi++;

	qdf_ewma_rx_rssi_add(&__rx_stats->avg_rssi, cdp_rx_ppdu->rssi);

	for (ant = 0; ant < MAX_RSSI_ANT; ant++) {
		for (ht = 0; ht < MAX_RSSI_HT; ht++)
			qdf_ewma_rx_rssi_add(&__rx_stats->avg_rssi_ant[ant][ht],
					     cdp_rx_ppdu->rssi_chain[ant][ht]);
	}
}

static void
wlan_peer_update_rx_rate_stats(struct wlan_soc_rate_stats_ctx *soc_stats_ctx,
			       struct cdp_rx_indication_ppdu *cdp_rx_ppdu)
{
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_peer_rx_rate_stats *rx_stats;
	struct wlan_rx_rate_stats *__rx_stats;
	uint8_t cache_idx;
	bool idx_match = false;

	stats_ctx = (struct wlan_peer_rate_stats_ctx *)cdp_rx_ppdu->cookie;
	rx_stats = &stats_ctx->rx;

	if (qdf_unlikely(cdp_rx_ppdu->rix > DP_RATE_TABLE_SIZE)) {
		qdf_warn("Invalid rate index, return");
		return;
	}

	RATE_STATS_LOCK_ACQUIRE(&rx_stats->lock);
	if (qdf_likely(rx_stats->cur_rix == cdp_rx_ppdu->rix)) {
		__rx_stats = &rx_stats->stats[rx_stats->cur_cache_idx];
		__wlan_peer_update_rx_rate_stats(__rx_stats,
						 cdp_rx_ppdu);

		soc_stats_ctx->rxs_last_idx_cache_hit++;
		goto done;
	}

	/* check if cache is available */
	for (cache_idx = 0; cache_idx < WLANSTATS_CACHE_SIZE; cache_idx++) {
		__rx_stats = &rx_stats->stats[cache_idx];
		if (!__rx_stats->rix || (__rx_stats->rix == cdp_rx_ppdu->rix)) {
			idx_match = true;
			break;
		}
	}
	/* if index matches or found empty index, update stats to that
	 * cache index else flush cache and update stats to cache index zero
	 */
	if (idx_match) {
		__wlan_peer_update_rx_rate_stats(__rx_stats,
						 cdp_rx_ppdu);
		rx_stats->cur_rix = cdp_rx_ppdu->rix;
		rx_stats->cur_cache_idx = cache_idx;
		soc_stats_ctx->rxs_cache_hit++;
		goto done;
	} else {
		wlan_peer_flush_rx_rate_stats(soc_stats_ctx, stats_ctx);
		__rx_stats = &rx_stats->stats[0];
		__wlan_peer_update_rx_rate_stats(__rx_stats,
						 cdp_rx_ppdu);
		rx_stats->cur_rix = cdp_rx_ppdu->rix;
		rx_stats->cur_cache_idx = 0;
		soc_stats_ctx->rxs_cache_miss++;
	}
done:
	RATE_STATS_LOCK_RELEASE(&rx_stats->lock);
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

	__tx_stats->rix = ppdu_user->rix;
	__tx_stats->rate = ppdu_user->tx_rate;
	__tx_stats->num_ppdus += num_ppdus;
	__tx_stats->mpdu_attempts += mpdu_attempts;
	__tx_stats->mpdu_success += mpdu_success;
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

	for (user_idx = 0; user_idx < cdp_tx_ppdu->num_users; user_idx++) {
		ppdu_user = &cdp_tx_ppdu->user[user_idx];
		stats_ctx = (struct wlan_peer_rate_stats_ctx *)
				ppdu_user->cookie;
		tx_stats = &stats_ctx->tx;
		RATE_STATS_LOCK_ACQUIRE(&tx_stats->lock);

		if (qdf_unlikely(ppdu_user->rix > DP_RATE_TABLE_SIZE)) {
			qdf_warn("Invalid rate index, continue.");
			RATE_STATS_LOCK_RELEASE(&tx_stats->lock);
			continue;
		}
		if (qdf_likely(tx_stats->cur_rix == ppdu_user->rix)) {
			__tx_stats = &tx_stats->stats[tx_stats->cur_cache_idx];
			__wlan_peer_update_tx_rate_stats(__tx_stats, ppdu_user);
			soc_stats_ctx->txs_last_idx_cache_hit++;
			RATE_STATS_LOCK_RELEASE(&tx_stats->lock);
			continue;
		}

		/* check if cache is available */
		for (cache_idx = 0; cache_idx < WLANSTATS_CACHE_SIZE;
					cache_idx++) {
			__tx_stats = &tx_stats->stats[cache_idx];
			if (!__tx_stats->rix ||
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
	}
}

static void
wlan_peer_update_sojourn_stats(void *ctx,
			       struct cdp_tx_sojourn_stats *sojourn_stats)
{
	struct wlan_peer_rate_stats_ctx *stats_ctx;
	struct wlan_peer_tx_rate_stats *tx_stats;
	uint8_t tid;

	stats_ctx = (struct wlan_peer_rate_stats_ctx *)sojourn_stats->cookie;
	tx_stats = &stats_ctx->tx;

	RATE_STATS_LOCK_ACQUIRE(&tx_stats->lock);
	for (tid = 0; tid < CDP_DATA_TID_MAX; tid++) {
		qdf_ewma_tx_lag_add(&tx_stats->sojourn.avg_sojourn_msdu[tid],
				    sojourn_stats->sum_sojourn_msdu[tid]);

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
		break;
	case WDI_EVENT_RX_PPDU_DESC:
		cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)
					qdf_nbuf_data(nbuf);
		wlan_peer_update_rx_rate_stats(soc_stats_ctx, cdp_rx_ppdu);
		break;
	case WDI_EVENT_TX_SOJOURN_STAT:
		sojourn_stats = (struct cdp_tx_sojourn_stats *)
					qdf_nbuf_data(nbuf);
		wlan_peer_update_sojourn_stats(soc_stats_ctx, sojourn_stats);
		break;
	default:
		qdf_err("Err, Invalid type");
	}
}

void wlan_peer_create_event_handler(void *pdev, enum WDI_EVENT event,
				    void *buf, uint16_t peer_id,
				    uint32_t type)
{
	struct cdp_peer_cookie *peer_info;
	struct wlan_peer_rate_stats_ctx *stats;

	peer_info = (struct cdp_peer_cookie *)buf;
	stats = qdf_mem_malloc(sizeof(*stats));
	if (!stats) {
		qdf_err("malloc failed, returning NULL");
		return;
	}
	qdf_mem_zero(stats, sizeof(*stats));
	RATE_STATS_LOCK_CREATE(&stats->tx.lock);
	RATE_STATS_LOCK_CREATE(&stats->rx.lock);
	qdf_mem_copy(stats->mac_addr, peer_info->mac_addr, CDP_MAC_ADDR_LEN);
	stats->peer_cookie = peer_info->cookie;
	stats->pdev = pdev;

	peer_info->ctx = (void *)stats;
}

void wlan_peer_destroy_event_handler(void *ctx, enum WDI_EVENT event,
				     void *buf, uint16_t peer_id,
				     uint32_t type)
{
	struct cdp_peer_cookie *peer_info;
	struct wlan_peer_rate_stats_ctx *stats;

	peer_info = (struct cdp_peer_cookie *)buf;
	stats = (struct wlan_peer_rate_stats_ctx *)peer_info->ctx;

	if (stats) {
		wlan_peer_flush_rate_stats(ctx, stats);
		RATE_STATS_LOCK_DESTROY(&stats->tx.lock);
		RATE_STATS_LOCK_DESTROY(&stats->rx.lock);
		qdf_mem_free(stats);
		qdf_info("DEBUG DEiniitialized rate stats");
	}
}
#endif

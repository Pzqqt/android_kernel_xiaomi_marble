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
 * @file: dp_rate_stats.h
 * @brief: Define peer rate statistics related objects and APIs
 */

#ifndef _DP_RATE_STATS_
#define _DP_RATE_STATS_

#include <cdp_txrx_handle.h>
#include <qdf_nbuf.h>
#include <wdi_event_api.h>

#ifdef QCA_SUPPORT_RDK_STATS

#include <qdf_util.h>
#include <cdp_txrx_cmn_struct.h>
#include <cdp_txrx_cmn.h>
#include <dp_ratetable.h>
#include <dp_rate_stats_pub.h>

#define RATE_STATS_LOCK_CREATE(lock) qdf_spinlock_create(lock)
#define RATE_STATS_LOCK_DESTROY(lock) qdf_spinlock_destroy(lock)
#define RATE_STATS_LOCK_ACQUIRE(lock) qdf_spin_lock_bh(lock)
#define RATE_STATS_LOCK_RELEASE(lock) qdf_spin_unlock_bh(lock)

struct cdp_pdev;

/**
 * struct wlan_peer_tx_rate_stats - peer tx rate statistics
 * @stats: array containing tx rate stats
 * @cur_rix: rate index updated last in list
 * @lock: lock protecting list
 * @sojourn: sojourn statistics
 */
struct wlan_peer_tx_rate_stats {
	struct wlan_tx_rate_stats stats[WLANSTATS_CACHE_SIZE];
	struct wlan_tx_sojourn_stats sojourn;
	uint32_t cur_rix;
	uint8_t cur_cache_idx;
	qdf_spinlock_t lock;
};

/**
 * struct wlan_peer_rx_rate_stats - Peer Rx rate statistics
 * @stats: array containing rx rate stats
 * @cur_rix: rate index updated last in list
 * @lock: lock protecting list
 */
struct wlan_peer_rx_rate_stats {
	struct wlan_rx_rate_stats stats[WLANSTATS_CACHE_SIZE];
	uint32_t cur_rix;
	uint32_t cur_cache_idx;
	qdf_spinlock_t lock;
};

/**
 * struct wlan_peer_rate_stats - Peer rate statistics ctx
 * @tx: tx rate statistics
 * @rx: rx rate statistics
 * @mac_addr: peer MAC address
 * @peer_cookie: cookie for unique session of peer
 * @pdev: dp pdev
 */
struct wlan_peer_rate_stats_ctx {
	struct wlan_peer_tx_rate_stats tx;
	struct wlan_peer_rx_rate_stats rx;
	uint8_t mac_addr[WLAN_MAC_ADDR_LEN];
	uint64_t peer_cookie;
	struct cdp_pdev *pdev;
};

/**
 * struct wlan_soc_rate_stats_ctx - rate stats context at soc level
 * @soc: Opaque soc handle
 * @txs_cache_flush: tx stats flush count
 * @rxs_cache_flush: rx stats flush count
 * @txs_last_idx_cache_hit: cache hit for rate index received as last rate index
 * @txs_cache_hit: tx stats cache hit for rate index received from cache db
 * @rxs_last_idx_cache_hit: cache hit for rate index received as last rate index
 * @rxs_cache_hit: cache hit for rate index received from cache database
 * @txs_cache_miss: rate index recevied is not in cache database
 * @rxs_cache_miss: rate index recevied is not in cache database
 */
struct wlan_soc_rate_stats_ctx {
	struct cdp_soc_t *soc;
	uint32_t txs_cache_flush;
	uint32_t rxs_cache_flush;
	uint32_t txs_last_idx_cache_hit;
	uint32_t txs_cache_hit;
	uint32_t rxs_last_idx_cache_hit;
	uint32_t rxs_cache_hit;
	uint32_t txs_cache_miss;
	uint32_t rxs_cache_miss;
	bool is_lithium;
};

/**
 * wlan_peer_update_rate_stats ()- WDI handler to update rate stats
 * @ctx: soc level stats context
 * @event: WDI event enum
 * @stats: buffer containing cdp_peer_tx_rate_stats_intf or
 * cdp_peer_rx_rate_stats_intf
 * @peer_id: peer id
 * @type: enum wlan_rate_stats_type
 */

void wlan_peer_update_rate_stats(void *ctx,
				 enum WDI_EVENT event,
				 void *stats, uint16_t peer_id,
				 uint32_t type);

/**
 * wlan_cfg80211_flush_rate_stats ()- os_if layer handler to
 * flush rate statistics
 * @pdev: Ctrl pdev
 * @event: WDI event enum
 * @ptr: Stats buffer
 * @peer_id: peer id
 * @status: status
 */
void wlan_cfg80211_flush_rate_stats(void *ctrl_pdev, enum WDI_EVENT event,
				    void *ptr, uint16_t peer_id,
				    uint32_t status);

/**
 * wlan_peer_create_event_handler ()- peer create event handler
 * to initialize statistics objects
 * @pdev: Ctrl pdev
 * @event: WDI event enum
 * @peer_mac: Peer MAC address
 * @buf: Stats buffer
 * @peer_id: peer id
 * @status: status
 */
void wlan_peer_create_event_handler(void *pdev, enum WDI_EVENT event,
				    void *buf, uint16_t peer_id,
				    uint32_t status);

/**
 * wlan_peer_destroy_event_handler ()- peer destroy event handler
 * to deinitilize statistics objects
 * @pdev: Ctrl pdev
 * @event: WDI event enum
 * @peer_mac: Peer MAC address
 * @buf: Stats buffer
 * @peer_id: peer id
 * @status: status
 */
void wlan_peer_destroy_event_handler(void *pdev, enum WDI_EVENT event,
				     void *buf, uint16_t peer_id,
				     uint32_t status);

/**
 * wlan_peer_rate_stats_flush_req ()- stats flush request event handler
 * This flushes statistics to user space entity
 * @pdev: Ctrl pdev
 * @event: WDI event enum
 * @buf: Stats buffer
 * @peer_id: peer id
 * @status: status
 */
void wlan_peer_rate_stats_flush_req(void *pdev, enum WDI_EVENT event,
				    void *buf, uint16_t peer_id,
				    uint32_t status);

#else
static inline void
wlan_peer_update_rate_stats(void *pdev, enum WDI_EVENT event,
			    void *stats, uint16_t peer_id,
			    uint32_t type)
{
}

static inline void
wlan_cfg80211_flush_rate_stats(void *ctrl_pdev, enum WDI_EVENT event,
			       void *ptr, uint16_t peer_id,
			       uint32_t status)
{
}

static inline void
wlan_peer_create_event_handler(void *pdev, enum WDI_EVENT event,
			       void *buf, uint16_t peer_id,
			       uint32_t type)
{
}

static inline void
wlan_peer_destroy_event_handler(void *pdev, enum WDI_EVENT event,
				void *buf, uint16_t peer_id,
				uint32_t type)
{
}

static inline void
wlan_peer_rate_stats_flush_req(void *pdev,
			       enum WDI_EVENT event,
			       void *buf, uint16_t peer_id,
			       uint32_t type)
{
}
#endif /* QCA_SUPPORT_RDK_STATS */
#endif /* _DP_RATE_STATS_ */

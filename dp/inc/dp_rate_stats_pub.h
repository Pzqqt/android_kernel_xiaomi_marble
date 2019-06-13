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
 * @file: dp_rate_stats_pub.h
 * @breief: Define peer rate statistics related objects and APIs
 * accessed required for user space tools
 */

#ifndef _DP_RATE_STATS_UAPI_
#define _DP_RATE_STATS_UAPI_

#define WLANSTATS_CACHE_SIZE 10
#define WLANSTATS_MAX_CHAIN_LEGACY 4
#define WLANSTATS_MAX_BW_LEGACY 4
#define WLANSTATS_MAX_CHAIN 8
#define WLANSTATS_MAX_BW 8
#define WLAN_DATA_TID_MAX 8
#define WLAN_MAC_ADDR_LEN 6
#define WLANSTATS_RSSI_OFFSET 8
#define WLANSTATS_RSSI_MASK 0xff
#define WLANSTATS_RSSI_MAX 0x80
#define INVALID_CACHE_IDX (-1)
#define WLANSTATS_PEER_COOKIE_LSB 32
#define WLANSTATS_COOKIE_PLATFORM_OFFSET 0xFFFFFFFF00000000
#define WLANSTATS_COOKIE_PEER_COOKIE_OFFSET 0x00000000FFFFFFFF

#ifndef __KERNEL__
#define qdf_ewma_tx_lag unsigned long
#define qdf_ewma_rx_rssi unsigned long
#else
#include <cdp_txrx_cmn_struct.h>
QDF_DECLARE_EWMA(rx_rssi, 1024, 8)
#endif

/**
 * enum cdp_peer_rate_stats_cmd -
 * used by app to get specific stats
 */
enum wlan_peer_rate_stats_cmd {
	DP_PEER_RX_RATE_STATS,
	DP_PEER_TX_RATE_STATS,
	DP_PEER_SOJOURN_STATS,
};

/** struct wlan_tx_rate_stats - Tx packet rate info
 * @rix: Rate index derived from nss, mcs, preamble, ht, sgi
 * @rate: Data rate in kbps
 * @mpdu_success: success mpdus count
 * @mpdu_attempts: attempt mpdus count
 * @num_ppdus: ppdu count
 */
struct wlan_tx_rate_stats {
	uint32_t rix;
	uint32_t rate;
	uint32_t mpdu_success;
	uint32_t mpdu_attempts;
	uint32_t num_ppdus;
};

/** struct wlan_rx_rate_stats - Rx rate packet info
 * @rix: Rate index derived from nss, mcs, preamble, ht, sgi
 * @rate: Data rate in kbps
 * @num_bytes: num of bytes
 * @num_msdus: num of msdus
 * @num_mpdus: num of mpdus
 * @num_ppdus: num of ppdus
 * @num_retries: num of retries
 * @num_sgi: num of short guard interval
 */
struct wlan_rx_rate_stats {
	uint32_t rix;
	uint32_t rate;
	uint32_t num_bytes;
	uint32_t num_msdus;
	uint32_t num_mpdus;
	uint32_t num_ppdus;
	uint32_t num_retries;
	uint32_t num_sgi;
	qdf_ewma_rx_rssi avg_rssi;
	qdf_ewma_rx_rssi avg_rssi_ant[WLANSTATS_MAX_CHAIN][WLANSTATS_MAX_BW];
};

/*
 * struct wlan_tx_sojourn_stats - Tx sojourn stats
 * @ppdu_seq_id: ppdu_seq_id from tx completion
 * @avg_sojourn_msdu: average sojourn msdu time
 * @sum_sojourn_msdu: sum sojourn msdu time
 * @num_msdu: number of msdus per ppdu
 * @cookie: cookie to be used by upper layer
 */
struct wlan_tx_sojourn_stats {
	uint32_t ppdu_seq_id;
	uint32_t sum_sojourn_msdu[WLAN_DATA_TID_MAX];
	uint32_t num_msdus[WLAN_DATA_TID_MAX];
	void *cookie;
	qdf_ewma_tx_lag avg_sojourn_msdu[WLAN_DATA_TID_MAX];
};

/**
 * struct wlan_peer_rate_stats_intf - Interface structure to
 * flush stats to user spave entity
 * @stats: statistics buffer
 * @buf_len: buffer len
 * @peer_mac: peer mac address
 * @stats_type: statistics type
 * @cookie: peer cookie
 */
struct wlan_peer_rate_stats_intf {
	void *stats;
	uint32_t buf_len;
	uint8_t peer_mac[WLAN_MAC_ADDR_LEN];
	uint8_t stats_type;
	uint64_t cookie;
};
#endif /* _DP_RATE_STATS_UAPI_ */

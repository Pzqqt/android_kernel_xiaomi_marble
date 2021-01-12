/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
#define WLANSTATS_SNR_OFFSET 8
#define WLANSTATS_SNR_MASK 0xff
#define WLANSTATS_SNR_MAX 0x80
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

#define DP_PEER_STATS_RIX_MASK 0xffff
#define DP_PEER_STATS_NSS_MASK 0xf
#define DP_PEER_STATS_MCS_MASK 0xf
#define DP_PEER_STATS_BW_MASK 0xf

#define DP_PEER_STATS_RIX_OFFSET 0
#define DP_PEER_STATS_NSS_OFFSET 16
#define DP_PEER_STATS_MCS_OFFSET 20
#define DP_PEER_STATS_BW_OFFSET 24

#define ASSEMBLE_STATS_CODE(_rix, _nss, _mcs, _bw) \
(((_rix & DP_PEER_STATS_RIX_MASK) << DP_PEER_STATS_RIX_OFFSET) | \
((_nss & DP_PEER_STATS_NSS_MASK) << DP_PEER_STATS_NSS_OFFSET) | \
((_mcs & DP_PEER_STATS_MCS_MASK) << DP_PEER_STATS_MCS_OFFSET) | \
((_bw & DP_PEER_STATS_BW_MASK) << DP_PEER_STATS_BW_OFFSET))

#define GET_DP_PEER_STATS_RIX(_val) \
(((_val) >> DP_PEER_STATS_RIX_OFFSET) & DP_PEER_STATS_RIX_MASK)

#define GET_DP_PEER_STATS_NSS(_val) \
(((_val) >> DP_PEER_STATS_NSS_OFFSET) & DP_PEER_STATS_NSS_MASK)

#define GET_DP_PEER_STATS_MCS(_val) \
(((_val) >> DP_PEER_STATS_MCS_OFFSET) & DP_PEER_STATS_MCS_MASK)

#define GET_DP_PEER_STATS_BW(_val) \
(((_val) >> DP_PEER_STATS_BW_OFFSET) & DP_PEER_STATS_BW_MASK)

/**
 * enum wlan_peer_rate_stats_cmd -
 * used by app to get specific stats
 */
enum wlan_peer_rate_stats_cmd {
	DP_PEER_RX_RATE_STATS,
	DP_PEER_TX_RATE_STATS,
	DP_PEER_SOJOURN_STATS,
	DP_PEER_RX_LINK_STATS,
	DP_PEER_TX_LINK_STATS,
	DP_PEER_AVG_RATE_STATS,
};

/** struct wlan_tx_rate_stats - Tx packet rate info
 * @rix: Rate index derived from nss, mcs, preamble, ht, sgi
 * @rate: Data rate in kbps
 * @mpdu_success: success mpdus count
 * @mpdu_attempts: attempt mpdus count
 * @num_ppdus: ppdu count
 * @num_msdus: success msdu count
 * @num_bytes: success bytes count
 * @num_retries: retires count
 */
struct wlan_tx_rate_stats {
	uint32_t rix;
	uint32_t rate;
	uint32_t mpdu_success;
	uint32_t mpdu_attempts;
	uint32_t num_ppdus;
	uint32_t num_msdus;
	uint32_t num_bytes;
	uint32_t num_retries;
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

#define BW_USAGE_MAX_SIZE 4

/**
 * enum wlan_rate_ppdu_type -
 * types of communication
 */
enum wlan_rate_ppdu_type {
	WLAN_RATE_SU,
	WLAN_RATE_MU_MIMO,
	WLAN_RATE_MU_OFDMA,
	WLAN_RATE_MU_OFDMA_MIMO,
	WLAN_RATE_MAX,
};

/**
 * struct wlan_rate_avg - avg rate stats
 * @num_ppdu: number of ppdu
 * @sum_mbps: cumulative rate in mbps
 * @num_snr: number of times snr added
 * @sum_snr: sum of snr
 * @num_mpdu: number of mpdu
 * @num_retry: num of retries
 */
struct wlan_rate_avg {
	uint32_t num_ppdu;
	uint32_t sum_mbps;
	uint32_t num_snr;
	uint32_t sum_snr;
	uint64_t num_mpdu;
	uint32_t num_retry;
};

/**
 * struct wlan_avg_rate_stats - avg rate stats for tx and rx
 * @tx: avg tx rate stats
 * @rx: avg rx rate stats
 */
struct wlan_avg_rate_stats {
	struct wlan_rate_avg tx[WLAN_RATE_MAX];
	struct wlan_rate_avg rx[WLAN_RATE_MAX];
};

/**
 * struct wlan_peer_bw_stats - per link bw related stats
 * @usage_total - sum of total BW (20, 40, 80, 160)
 * @usage_avg - @usage_total / number of PPDUs (avg BW)
 * @usage_counter - each BW usage counter
 * @usage_max - number of pkts in max BW mode
 */
struct wlan_peer_bw_stats {
	uint32_t usage_total;
	uint32_t usage_counter[BW_USAGE_MAX_SIZE];
	uint8_t usage_avg;
	uint8_t usage_max;
};

/**
 * struct wlan_rx_link_stats - Peer Rx link statistics
 * @num_ppdus - number of ppdus per user
 * @bytes - number of bytes per user
 * @phy_rate_lpf_avg_su - SU packet LPF averaged rx rate
 * @phy_rate_actual_su - SU packet rounded average rx rate
 * @phy_rate_lpf_avg_mu - MU packet LPF averaged rx rate
 * @phy_rate_actual_mu - MUpacket rounded average rx rate
 * @ofdma_usage - number of packet in OFDMA
 * @mu_mimo_usage - number of pakcets in MU MIMO
 * @bw - average BW and max BW related structure
 * @su_rssi - single user RSSI
 * @mpdu_retries - number of retried MPDUs
 * @pkt_error_rate - average packet error rate
 * @num_mpdus - total number of mpdus
 */
struct wlan_rx_link_stats {
	uint64_t bytes;
	uint32_t num_ppdus;
	uint32_t phy_rate_lpf_avg_su;
	uint32_t phy_rate_actual_su;
	uint32_t phy_rate_lpf_avg_mu;
	uint32_t phy_rate_actual_mu;
	uint32_t ofdma_usage;
	uint32_t mu_mimo_usage;
	struct wlan_peer_bw_stats bw;
	qdf_ewma_rx_rssi su_rssi;
	uint32_t mpdu_retries;
	uint32_t num_mpdus;
	uint8_t pkt_error_rate;
};

/**
 * struct wlan_tx_link_stats - Peer tx link statistics
 * @num_ppdus - number of ppdus per user
 * @bytes - number of bytes per user
 * @phy_rate_lpf_avg_su - SU packet LPF averaged tx rate
 * @phy_rate_actual_su - SU packet rounded average tx rate
 * @phy_rate_lpf_avg_mu - MU packet LPF averaged tx rate
 * @phy_rate_actual_mu - MUpacket rounded average tx rate
 * @ofdma_usage - number of packet in OFDMA
 * @mu_mimo_usage - number of pakcets in MU MIMO
 * @bw - average BW and max BW related structure
 * @ack_rssi - averaged ACK rssi
 * @mpdu_failed - number of failed MPDUs
 * @mpdu_success - number of success MPDUs
 * @pkt_error_rate - average packet error rate
 */
struct wlan_tx_link_stats {
	uint64_t bytes;
	uint32_t num_ppdus;
	uint32_t phy_rate_lpf_avg_su;
	uint32_t phy_rate_actual_su;
	uint32_t phy_rate_lpf_avg_mu;
	uint32_t phy_rate_actual_mu;
	uint32_t ofdma_usage;
	uint32_t mu_mimo_usage;
	struct wlan_peer_bw_stats bw;
	qdf_ewma_rx_rssi ack_rssi;
	uint32_t mpdu_failed;
	uint32_t mpdu_success;
	uint8_t pkt_error_rate;
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

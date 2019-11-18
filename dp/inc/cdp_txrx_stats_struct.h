/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_stats_struct.h
 * @brief Define the host data path stats API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_STATS_STRUCT_H_
#define _CDP_TXRX_STATS_STRUCT_H_

#include <qdf_types.h>

#define TXRX_STATS_LEVEL_OFF   0
#define TXRX_STATS_LEVEL_BASIC 1
#define TXRX_STATS_LEVEL_FULL  2

#define BSS_CHAN_INFO_READ                        1
#define BSS_CHAN_INFO_READ_AND_CLEAR              2

#define TX_FRAME_TYPE_DATA 0
#define TX_FRAME_TYPE_MGMT 1
#define TX_FRAME_TYPE_BEACON 2

#ifndef TXRX_STATS_LEVEL
#define TXRX_STATS_LEVEL TXRX_STATS_LEVEL_BASIC
#endif

/* 1 additional MCS is for invalid values */
#define MAX_MCS (12 + 1)
#define MAX_MCS_11A 8
#define MAX_MCS_11B 7
#define MAX_MCS_11AC 12
/* 1 additional GI is for invalid values */
#define MAX_GI (4 + 1)
#define SS_COUNT 8
#define MAX_BW 7
#define MAX_RECEPTION_TYPES 4

#define MAX_TRANSMIT_TYPES	9

#define MAX_USER_POS		8
#define MAX_MU_GROUP_ID		64
#define MAX_RU_LOCATIONS	6
#define RU_26			1
#define RU_52			2
#define RU_106			4
#define RU_242			9
#define RU_484			18
#define RU_996			37

/* WME stream classes */
#define WME_AC_BE    0    /* best effort */
#define WME_AC_BK    1    /* background */
#define WME_AC_VI    2    /* video */
#define WME_AC_VO    3    /* voice */
#define WME_AC_MAX   4    /* MAX AC Value */

#define CDP_MAX_RX_RINGS 4  /* max rx rings */
#define CDP_MAX_TX_COMP_RINGS 3  /* max tx completion rings */
#define CDP_MAX_TX_TQM_STATUS 9  /* max tx tqm completion status */
#define CDP_MAX_TX_HTT_STATUS 7  /* max tx htt completion status */

/* TID level VoW stats macros
 * to add and get stats
 */
#define PFLOW_TXRX_TIDQ_STATS_ADD(_peer, _tid, _var, _val) \
	(((_peer)->tidq_stats[_tid]).stats[_var]) += _val
#define PFLOW_TXRX_TIDQ_STATS_GET(_peer, _tid, _var, _val) \
	((_peer)->tidq_stats[_tid].stats[_var])
/*
 * Video only stats
 */
#define PFLOW_CTRL_PDEV_VIDEO_STATS_SET(_pdev, _var, _val) \
	(((_pdev)->vow.vistats[_var]).value) = _val
#define PFLOW_CTRL_PDEV_VIDEO_STATS_GET(_pdev, _var) \
	((_pdev)->vow.vistats[_var].value)
#define PFLOW_CTRL_PDEV_VIDEO_STATS_ADD(_pdev, _var, _val) \
	(((_pdev)->vow.vistats[_var]).value) += _val
/*
 * video delay stats
 */
#define PFLOW_CTRL_PDEV_DELAY_VIDEO_STATS_SET(_pdev, _var, _val) \
	(((_pdev)->vow.delaystats[_var]).value) = _val
#define PFLOW_CTRL_PDEV_DELAY_VIDEO_STATS_GET(_pdev, _var) \
	((_pdev)->vow.delaystats[_var].value)
#define PFLOW_CTRL_PDEV_DELAY_VIDEO_STATS_ADD(_pdev, _var, _val) \
	(((_pdev)->vow.delaystats[_var]).value) += _val
/*
 * Number of TLVs sent by FW. Needs to reflect
 * HTT_PPDU_STATS_MAX_TAG declared in FW
 */
#define CDP_PPDU_STATS_MAX_TAG 14
#define CDP_MAX_DATA_TIDS 9

#define CDP_WDI_NUM_EVENTS WDI_NUM_EVENTS

#define CDP_FCTL_RETRY 0x0800
#define CDP_FC_IS_RETRY_SET(_fc) \
	((_fc) & qdf_cpu_to_le16(CDP_FCTL_RETRY))

#define INVALID_RSSI 255

#define CDP_RSSI_MULTIPLIER BIT(8)
#define CDP_RSSI_MUL(x, mul) ((x) * (mul))
#define CDP_RSSI_RND(x, mul) ((((x) % (mul)) >= ((mul) / 2)) ?\
	((x) + ((mul) - 1)) / (mul) : (x) / (mul))

#define CDP_RSSI_OUT(x) (CDP_RSSI_RND((x), CDP_RSSI_MULTIPLIER))
#define CDP_RSSI_IN(x)  (CDP_RSSI_MUL((x), CDP_RSSI_MULTIPLIER))
#define CDP_RSSI_AVG(x, y) ((((x) << 2) + (y) - (x)) >> 2)

#define CDP_RSSI_UPDATE_AVG(x, y) x = CDP_RSSI_AVG((x), CDP_RSSI_IN((y)))

/*Max SU EVM count */
#define DP_RX_MAX_SU_EVM_COUNT 32

#define WDI_EVENT_BASE 0x100

#define CDP_TXRX_RATECODE_MCS_MASK 0xF
#define CDP_TXRX_RATECODE_NSS_MASK 0x3
#define CDP_TXRX_RATECODE_NSS_LSB 4
#define CDP_TXRX_RATECODE_PREM_MASK 0x3
#define CDP_TXRX_RATECODE_PREM_LSB 6

/* Below BW_GAIN should be added to the SNR value of every ppdu based on the
 * bandwidth. This table is obtained from HALPHY.
 * BW         BW_Gain
 * 20          0
 * 40          3dBm
 * 80          6dBm
 * 160/80P80   9dBm
 */

#define PKT_BW_GAIN_20MHZ 0
#define PKT_BW_GAIN_40MHZ 3
#define PKT_BW_GAIN_80MHZ 6
#define PKT_BW_GAIN_160MHZ 9

/*
 * cdp_tx_transmit_type: Transmit type index
 * SU: SU Transmit type index
 * MU_MIMO: MU_MIMO Transmit type index
 * MU_OFDMA: MU_OFDMA Transmit type index
 * MU_MIMO_OFDMA: MU MIMO OFDMA Transmit type index
 */
enum cdp_tx_transmit_type {
	SU = 0,
	MU_MIMO,
	MU_OFDMA,
	MU_MIMO_OFDMA,
};

/*
 * cdp_ru_index: Different RU index
 *
 * RU_26_INDEX : 26-tone Resource Unit index
 * RU_52_INDEX : 52-tone Resource Unit index
 * RU_106_INDEX: 106-tone Resource Unit index
 * RU_242_INDEX: 242-tone Resource Unit index
 * RU_484_INDEX: 484-tone Resource Unit index
 * RU_996_INDEX: 996-tone Resource Unit index
 */
enum cdp_ru_index {
	RU_26_INDEX = 0,
	RU_52_INDEX,
	RU_106_INDEX,
	RU_242_INDEX,
	RU_484_INDEX,
	RU_996_INDEX,
};

#ifdef FEATURE_TSO_STATS
/* Number of TSO Packet Statistics captured */
#define CDP_MAX_TSO_PACKETS 5
/* Information for Number of Segments for a TSO Packet captured */
#define CDP_MAX_TSO_SEGMENTS 2
/* Information for Number of Fragments for a TSO Segment captured */
#define CDP_MAX_TSO_FRAGMENTS 6
#endif /* FEATURE_TSO_STATS */

/* Different Packet Types */
enum cdp_packet_type {
	DOT11_A = 0,
	DOT11_B = 1,
	DOT11_N = 2,
	DOT11_AC = 3,
	DOT11_AX = 4,
	DOT11_MAX = 5,
};

/*
 * cdp_mu_packet_type: MU Rx type index
 * RX_TYPE_MU_MIMO: MU MIMO Rx type index
 * RX_TYPE_MU_OFDMA: MU OFDMA Rx type index
 * MU_MIMO_OFDMA: MU Rx MAX type index
 */
enum cdp_mu_packet_type {
	RX_TYPE_MU_MIMO = 0,
	RX_TYPE_MU_OFDMA = 1,
	RX_TYPE_MU_MAX = 2,
};

enum WDI_EVENT {
	WDI_EVENT_TX_STATUS = WDI_EVENT_BASE,
	WDI_EVENT_OFFLOAD_ALL,
	WDI_EVENT_RX_DESC_REMOTE,
	WDI_EVENT_RX_PEER_INVALID,
	WDI_EVENT_DBG_PRINT, /* NEED to integrate pktlog changes*/
	WDI_EVENT_RX_CBF_REMOTE,
	WDI_EVENT_RATE_FIND,
	WDI_EVENT_RATE_UPDATE,
	WDI_EVENT_SW_EVENT,
	WDI_EVENT_RX_DESC,
	WDI_EVENT_LITE_T2H,
	WDI_EVENT_LITE_RX,
	WDI_EVENT_RX_PPDU_DESC,
	WDI_EVENT_TX_PPDU_DESC,
	WDI_EVENT_TX_MSDU_DESC,
	WDI_EVENT_TX_DATA,
	WDI_EVENT_RX_DATA,
	WDI_EVENT_TX_MGMT_CTRL,
	WDI_EVENT_HTT_STATS,
	WDI_EVENT_TX_BEACON,
	WDI_EVENT_PEER_STATS,
	WDI_EVENT_TX_SOJOURN_STAT,
	WDI_EVENT_UPDATE_DP_STATS,
	WDI_EVENT_RX_MGMT_CTRL,
	WDI_EVENT_PEER_CREATE,
	WDI_EVENT_PEER_DESTROY,
	WDI_EVENT_PEER_FLUSH_RATE_STATS,
	WDI_EVENT_FLUSH_RATE_STATS_REQ,
	WDI_EVENT_RX_MPDU,
	/* End of new event items */
	WDI_EVENT_LAST
};

#define WDI_NUM_EVENTS WDI_EVENT_LAST - WDI_EVENT_BASE

struct cdp_stats_extd {
};

/* TID level Tx/Rx stats
 *
 */
enum cdp_txrx_tidq_stats {
	/* Tx Counters */
	TX_MSDU_TOTAL_LINUX_SUBSYSTEM,
	TX_MSDU_TOTAL_FROM_OSIF,
	TX_MSDU_TX_COMP_PKT_CNT,
	/* Rx Counters */
	RX_MSDU_TOTAL_FROM_FW,
	RX_MSDU_MCAST_FROM_FW,
	RX_TID_MISMATCH_FROM_FW,
	RX_MSDU_MISC_PKTS,
	RX_MSDU_IS_ARP,
	RX_MSDU_IS_EAP,
	RX_MSDU_IS_DHCP,
	RX_AGGREGATE_10,
	RX_AGGREGATE_20,
	RX_AGGREGATE_30,
	RX_AGGREGATE_40,
	RX_AGGREGATE_50,
	RX_AGGREGATE_60,
	RX_AGGREGATE_MORE,
	RX_AMSDU_1,
	RX_AMSDU_2,
	RX_AMSDU_3,
	RX_AMSDU_4,
	RX_AMSDU_MORE,
	RX_MSDU_CHAINED_FROM_FW,
	RX_MSDU_REORDER_FAILED_FROM_FW,
	RX_MSDU_REORDER_FLUSHED_FROM_FW,
	RX_MSDU_DISCARD_FROM_FW,
	RX_MSDU_DUPLICATE_FROM_FW,
	RX_MSDU_DELIVERED_TO_STACK,
	TIDQ_STATS_MAX,
};

struct cdp_tidq_stats {
	uint32_t stats[TIDQ_STATS_MAX];
};

/*
 * struct cdp_rx_su_evm_info: Rx evm info
 * @number_of_symbols: number of symbols
 * @nss_count: number of spatial streams
 * @pilot_count: number of pilot count
 */
struct cdp_rx_su_evm_info {
	uint16_t number_of_symbols;
	uint8_t  nss_count;
	uint8_t  pilot_count;
	uint32_t pilot_evm[DP_RX_MAX_SU_EVM_COUNT];
};

/*
 * cdp_delay_stats_mode: Different types of delay statistics
 *
 * @CDP_DELAY_STATS_SW_ENQ: Stack to hw enqueue delay
 * @CDP_DELAY_STATS_TX_INTERFRAME: Interframe delay at radio entry point
 * @CDP_DELAY_STATS_FW_HW_TRANSMIT: Hw enqueue to tx completion delay
 * @CDP_DELAY_STATS_REAP_STACK: Delay in ring reap to indicating network stack
 * @CDP_DELAY_STATS_RX_INTERFRAME: Rx inteframe delay
 * @CDP_DELAY_STATS_MODE_MAX: Maximum delay mode
 */
enum cdp_delay_stats_mode {
	CDP_DELAY_STATS_SW_ENQ,
	CDP_DELAY_STATS_TX_INTERFRAME,
	CDP_DELAY_STATS_FW_HW_TRANSMIT,
	CDP_DELAY_STATS_REAP_STACK,
	CDP_DELAY_STATS_RX_INTERFRAME,
	CDP_DELAY_STATS_MODE_MAX,
};

/*
 * cdp_delay_bucket_index
 *	Index to be used for all delay stats
 */
enum cdp_delay_bucket_index {
	CDP_DELAY_BUCKET_0,
	CDP_DELAY_BUCKET_1,
	CDP_DELAY_BUCKET_2,
	CDP_DELAY_BUCKET_3,
	CDP_DELAY_BUCKET_4,
	CDP_DELAY_BUCKET_5,
	CDP_DELAY_BUCKET_6,
	CDP_DELAY_BUCKET_7,
	CDP_DELAY_BUCKET_8,
	CDP_DELAY_BUCKET_9,
	CDP_DELAY_BUCKET_10,
	CDP_DELAY_BUCKET_11,
	CDP_DELAY_BUCKET_12,
	CDP_DELAY_BUCKET_MAX,
};

/*
 * struct cdp_tx_host_drop - packet drop due to following reasons.
 */
enum cdp_tx_sw_drop {
	TX_DESC_ERR,
	TX_HAL_RING_ACCESS_ERR,
	TX_DMA_MAP_ERR,
	TX_HW_ENQUEUE,
	TX_SW_ENQUEUE,
	TX_MAX_DROP,
};

/*
 * struct cdp_rx_host_drop - packet drop due to following reasons.
 */
enum cdp_rx_sw_drop {
	INTRABSS_DROP,
	MSDU_DONE_FAILURE,
	INVALID_PEER_VDEV,
	POLICY_CHECK_DROP,
	MEC_DROP,
	NAWDS_MCAST_DROP,
	MESH_FILTER_DROP,
	ENQUEUE_DROP,
	RX_MAX_DROP,
};

/*
 * struct cdp_delay_stats
 * @delay_bucket: division of buckets as per latency
 * @min_delay: minimum delay
 * @max_delay: maximum delay
 * @avg_delay: average delay
 */
struct cdp_delay_stats {
	uint64_t delay_bucket[CDP_DELAY_BUCKET_MAX];
	uint32_t min_delay;
	uint32_t max_delay;
	uint32_t avg_delay;
};

/*
 * struct cdp_tid_tx_stats
 * @swq_delay: delay between wifi driver entry point and enqueue to HW in tx
 * @hwtx_delay: delay between wifi driver exit (enqueue to HW) and tx completion
 * @intfrm_delay: interframe delay
 * @success_cnt: total successful transmit count
 * @comp_fail_cnt: firmware drop found in tx completion path
 * @swdrop_cnt: software drop in tx path
 * @tqm_status_cnt: TQM completion status count
 * @htt_status_cnt: HTT completion status count
 */
struct cdp_tid_tx_stats {
	struct cdp_delay_stats swq_delay;
	struct cdp_delay_stats hwtx_delay;
	struct cdp_delay_stats intfrm_delay;
	uint64_t success_cnt;
	uint64_t comp_fail_cnt;
	uint64_t swdrop_cnt[TX_MAX_DROP];
	uint64_t tqm_status_cnt[CDP_MAX_TX_TQM_STATUS];
	uint64_t htt_status_cnt[CDP_MAX_TX_HTT_STATUS];
};

/*
 * struct cdp_tid_tx_stats
 * @to_stack_delay: Time taken between ring reap to indication to network stack
 * @intfrm_delay: Interframe rx delay
 * @delivered_cnt: Total packets indicated to stack
 * @intrabss_cnt: Rx total intraBSS frames
 * @msdu_cnt: number of msdu received from HW
 * @mcast_msdu_cnt: Num Mcast Msdus received from HW in Rx
 * @bcast_msdu_cnt: Num Bcast Msdus received from HW in Rx
 * @fail_cnt: Rx deliver drop counters
 */
struct cdp_tid_rx_stats {
	struct cdp_delay_stats to_stack_delay;
	struct cdp_delay_stats intfrm_delay;
	uint64_t delivered_to_stack;
	uint64_t intrabss_cnt;
	uint64_t msdu_cnt;
	uint64_t mcast_msdu_cnt;
	uint64_t bcast_msdu_cnt;
	uint64_t fail_cnt[RX_MAX_DROP];
};

/*
 * struct cdp_tid_stats
 * @ingress_stack: Total packets received from linux stack
 * @osif_drop: drops in osif layer
 * @tid_tx_stats: transmit counters per tid
 * @tid_rx_stats: receive counters per tid
 */
struct cdp_tid_stats {
	uint64_t ingress_stack;
	uint64_t osif_drop;
	struct cdp_tid_tx_stats tid_tx_stats[CDP_MAX_TX_COMP_RINGS]
					    [CDP_MAX_DATA_TIDS];
	struct cdp_tid_rx_stats tid_rx_stats[CDP_MAX_RX_RINGS]
					    [CDP_MAX_DATA_TIDS];
};

/* struct cdp_pkt_info - packet info
 * @num: no of packets
 * @bytes: total no of bytes
 */
struct cdp_pkt_info {
	uint32_t num;
	uint64_t bytes;
};

/* struct cdp_pkt_type - packet type
 * @mcs_count: Counter array for each MCS index
 */
struct cdp_pkt_type {
	uint32_t mcs_count[MAX_MCS];
};

/*
 * struct cdp_rx_mu - Rx MU Stats
 * @ppdu_nss[SS_COUNT]: Packet Count in spatial streams
 * @mpdu_cnt_fcs_ok: Rx success mpdu count
 * @mpdu_cnt_fcs_err: Rx fail mpdu count
 * @cdp_pkt_type: counter array for each MCS index
 */
struct cdp_rx_mu {
	uint32_t ppdu_nss[SS_COUNT];
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
	struct cdp_pkt_type ppdu;
};

/* struct cdp_tx_pkt_info - tx packet info
 * num_msdu - successful msdu
 * num_mpdu - successful mpdu from compltn common
 * mpdu_tried - mpdu tried
 *
 * tx packet info counter field for mpdu success/tried and msdu
 */
struct cdp_tx_pkt_info {
	uint32_t num_msdu;
	uint32_t num_mpdu;
	uint32_t mpdu_tried;
};

#ifdef FEATURE_TSO_STATS
/**
 * struct cdp_tso_seg_histogram - Segment histogram for TCP Packets
 * @segs_1: packets with single segments
 * @segs_2_5: packets with 2-5 segments
 * @segs_6_10: packets with 6-10 segments
 * @segs_11_15: packets with 11-15 segments
 * @segs_16_20: packets with 16-20 segments
 * @segs_20_plus: packets with 20 plus segments
 */
struct cdp_tso_seg_histogram {
	uint64_t segs_1;
	uint64_t segs_2_5;
	uint64_t segs_6_10;
	uint64_t segs_11_15;
	uint64_t segs_16_20;
	uint64_t segs_20_plus;
};

/**
 * struct cdp_tso_packet_info - Stats for TSO segments within a TSO packet
 * @tso_seg: TSO Segment information
 * @num_seg: Number of segments
 * @tso_packet_len: Size of the tso packet
 * @tso_seg_idx: segment number
 */
struct cdp_tso_packet_info {
	struct qdf_tso_seg_t tso_seg[CDP_MAX_TSO_SEGMENTS];
	uint8_t num_seg;
	size_t tso_packet_len;
	uint32_t tso_seg_idx;
};

/**
 * struct cdp_tso_info - stats for tso packets
 * @tso_packet_info: TSO packet information
 */
struct cdp_tso_info {
	struct cdp_tso_packet_info tso_packet_info[CDP_MAX_TSO_PACKETS];
};
#endif /* FEATURE_TSO_STATS */

/**
 * struct cdp_tso_stats -  TSO stats information
 * @num_tso_pkts: Total number of TSO Packets
 * @tso_comp: Total tso packet completions
 * @dropped_host: TSO packets dropped by host
 * @tso_no_mem_dropped: TSO packets dropped by host due to descriptor
			unavailablity
 * @dropped_target: TSO packets_dropped by target
 * @tso_info: Per TSO packet counters
 * @seg_histogram: TSO histogram stats
 */
struct cdp_tso_stats {
	struct cdp_pkt_info num_tso_pkts;
	uint32_t tso_comp;
	struct cdp_pkt_info dropped_host;
	struct cdp_pkt_info tso_no_mem_dropped;
	uint32_t dropped_target;
#ifdef FEATURE_TSO_STATS
	struct cdp_tso_info tso_info;
	struct cdp_tso_seg_histogram seg_histogram;
#endif /* FEATURE_TSO_STATS */
};

/* struct cdp_tx_stats - tx stats
 * @cdp_pkt_info comp_pkt: Pkt Info for which completions were received
 * @cdp_pkt_info ucast: Unicast Packet Count
 * @cdp_pkt_info mcast: Multicast Packet Count
 * @cdp_pkt_info bcast: Broadcast Packet Count
 * @cdp_pkt_info nawds_mcast: NAWDS  Multicast Packet Count
 * @cdp_pkt_info tx_success: Successful Tx Packets
 * @nawds_mcast_drop: NAWDS  Multicast Drop Count
 * @tx_failed: Total Tx failure
 * @ofdma: Total Packets as ofdma
 * @stbc: Packets in STBC
 * @ldpc: Packets in LDPC
 * @retries: Packet retries
 * @non_amsdu_cnt: Number of MSDUs with no MSDU level aggregation
 * @amsdu_cnt: Number of MSDUs part of AMSDU
 * @tx_rate: Tx Rate
 * @last_tx_rate: Last tx rate for unicast packets
 * @last_tx_rate_mcs: Tx rate mcs for unicast packets
 * @mcast_last_tx_rate: Last tx rate for multicast packets
 * @mcast_last_tx_rate_mcs: Last tx rate mcs for multicast
 * @last_per: Tx Per
 * @rnd_avg_tx_rate: Rounded average tx rate
 * @avg_tx_rate: Average TX rate
 * @last_ack_rssi: RSSI of last acked packet
 * @tx_bytes_success_last: last Tx success bytes
 * @tx_data_success_last: last Tx success data
 * @tx_byte_rate: Bytes Trasmitted in last one sec
 * @tx_data_rate: Data Transmitted in last one sec
 * @sgi_count[MAX_GI]: SGI count
 * @nss[SS_COUNT]: Packet count for different num_spatial_stream values
 * @bw[MAX_BW]: Packet Count for different bandwidths
 * @wme_ac_type[WME_AC_MAX]: Wireless Multimedia type Count
 * @excess_retries_per_ac[WME_AC_MAX]: Wireless Multimedia type Count
 * @dot11_tx_pkts: dot11 tx packets
 * @fw_rem: Discarded by firmware
 * @fw_rem_notx: firmware_discard_untransmitted
 * @fw_rem_tx: firmware_discard_transmitted
 * @age_out: aged out in mpdu/msdu queues
 * @fw_reason1: discarded by firmware reason 1
 * @fw_reason2: discarded by firmware reason 2
 * @fw_reason3: discarded by firmware reason 3
 * @mcs_count: MCS Count
 * @an_tx_cnt: ald tx count
 * @an_tx_rates_used: ald rx rate used
 * @an_tx_bytes: ald tx bytes
 * @ald_txcount: ald tx count
 * @ald_lastper: ald last PER
 * @ald_max4msframelen: ald max frame len
 * @an_tx_ratecount: ald tx ratecount
 * @ald_retries: ald retries
 * @ald_ac_nobufs: #buffer overflows per node per AC
 * @ald_ac_excretries: #pkts dropped after excessive retries per node per AC
 * @rssi_chain: rssi chain
 * @inactive_time: inactive time in secs
 * @tx_flags: tx flags
 * @tx_power: Tx power latest
 * @is_tx_no_ack: no ack received
 * @tx_ratecode: Tx rate code of last frame
 * @is_tx_nodefkey: tx failed 'cuz no defkey
 * @is_tx_noheadroom: tx failed 'cuz no space
 * @is_crypto_enmicfail:
 * @is_tx_nonode: tx failed for no node
 * @is_tx_unknownmgt: tx of unknown mgt frame
 * @is_tx_badcipher: tx failed 'cuz key type
 * @ampdu_cnt: completion of aggregation
 * @non_ampdu_cnt: tx completion not aggregated
 * @failed_retry_count: packets failed due to retry above 802.11 retry limit
 * @retry_count: packets successfully send after one or more retry
 * @multiple_retry_count: packets successfully sent after more than one retry
 * @transmit_type: pkt info for tx transmit type
 * @mu_group_id: mumimo mu group id
 * @ru_start: RU start index
 * @ru_tones: RU tones size
 * @ru_loc: pkt info for RU location 26/ 52/ 106/ 242/ 484 counter
 * @num_ppdu_cookie_valid : Number of comp received with valid ppdu cookie
 */
struct cdp_tx_stats {
	struct cdp_pkt_info comp_pkt;
	struct cdp_pkt_info ucast;
	struct cdp_pkt_info mcast;
	struct cdp_pkt_info bcast;
	struct cdp_pkt_info nawds_mcast;
	struct cdp_pkt_info tx_success;
	uint32_t nawds_mcast_drop;
	uint32_t tx_failed;
	uint32_t ofdma;
	uint32_t stbc;
	uint32_t ldpc;
	uint32_t retries;
	uint32_t non_amsdu_cnt;
	uint32_t amsdu_cnt;
	uint32_t tx_rate;
	uint32_t last_tx_rate;
	uint32_t last_tx_rate_mcs;
	uint32_t mcast_last_tx_rate;
	uint32_t mcast_last_tx_rate_mcs;
	uint32_t last_per;
	uint64_t rnd_avg_tx_rate;
	uint64_t avg_tx_rate;
	uint32_t last_ack_rssi;
	uint32_t tx_bytes_success_last;
	uint32_t tx_data_success_last;
	uint32_t tx_byte_rate;
	uint32_t tx_data_rate;
	uint32_t tx_data_ucast_last;
	uint32_t tx_data_ucast_rate;
	struct cdp_pkt_type pkt_type[DOT11_MAX];
	uint32_t sgi_count[MAX_GI];

	uint32_t nss[SS_COUNT];

	uint32_t bw[MAX_BW];

	uint32_t wme_ac_type[WME_AC_MAX];

	uint32_t excess_retries_per_ac[WME_AC_MAX];
	struct cdp_pkt_info dot11_tx_pkts;

	struct {
		struct cdp_pkt_info fw_rem;
		uint32_t fw_rem_notx;
		uint32_t fw_rem_tx;
		uint32_t age_out;
		uint32_t fw_reason1;
		uint32_t fw_reason2;
		uint32_t fw_reason3;
	} dropped;


	uint32_t fw_tx_cnt;
	uint32_t fw_tx_bytes;
	uint32_t fw_txcount;
	uint32_t fw_max4msframelen;
	uint32_t fw_ratecount;

	uint32_t ac_nobufs[WME_AC_MAX];
	uint32_t rssi_chain[WME_AC_MAX];
	uint32_t inactive_time;

	uint32_t tx_flags;
	uint32_t tx_power;

	/* MSDUs which the target sent but couldn't get an ack for */
	struct cdp_pkt_info is_tx_no_ack;
	uint16_t tx_ratecode;

	/*add for peer and upadted from ppdu*/
	uint32_t ampdu_cnt;
	uint32_t non_ampdu_cnt;
	uint32_t failed_retry_count;
	uint32_t retry_count;
	uint32_t multiple_retry_count;
	uint32_t last_tx_rate_used;

	struct cdp_tx_pkt_info transmit_type[MAX_TRANSMIT_TYPES];
	uint32_t mu_group_id[MAX_MU_GROUP_ID];
	uint32_t ru_start;
	uint32_t ru_tones;
	struct cdp_tx_pkt_info ru_loc[MAX_RU_LOCATIONS];

	uint32_t num_ppdu_cookie_valid;
};

/* struct cdp_rx_stats - rx Level Stats
 * @to_stack: Total packets sent up the stack
 * @rcvd_reo[CDP_MAX_RX_RINGS]:  Packets received on the reo ring
 * @unicast: Total unicast packets
 * @multicast: Total multicast packets
 * @bcast:  Broadcast Packet Count
 * @raw: Raw Pakets received
 * @nawds_mcast_drop: Total multicast packets
 * @mec_drop: Total MEC packets dropped
 * @pkts: Intra BSS packets received
 * @fail: Intra BSS packets failed
 * @mdns_no_fwd: Intra BSS MDNS packets not forwarded
 * @mic_err: Rx MIC errors CCMP
 * @decrypt_err: Rx Decryption Errors CRC
 * @fcserr: rx MIC check failed (CCMP)
 * @wme_ac_type[WME_AC_MAX]: Wireless Multimedia type Count
 * @reception_type[MAX_RECEPTION_TYPES]: Reception type os packets
 * @mcs_count[MAX_MCS]: mcs count
 * @sgi_count[MAX_GI]: sgi count
 * @nss[SS_COUNT]: packet count in spatiel Streams
 * @ppdu_nss[SS_COUNT]: PPDU packet count in spatial streams
 * @mpdu_cnt_fcs_ok: SU Rx success mpdu count
 * @mpdu_cnt_fcs_err: SU Rx fail mpdu count
 * @su_ax_ppdu_cnt: SU Rx packet count
 * @ppdu_cnt[MAX_RECEPTION_TYPES]: PPDU packet count in reception type
 * @rx_mu[RX_TYPE_MU_MAX]: Rx MU stats
 * @bw[MAX_BW]:  Packet Count in different bandwidths
 * @non_ampdu_cnt: Number of MSDUs with no MPDU level aggregation
 * @ampdu_cnt: Number of MSDUs part of AMSPU
 * @non_amsdu_cnt: Number of MSDUs with no MSDU level aggregation
 * @amsdu_cnt: Number of MSDUs part of AMSDU
 * @bar_recv_cnt: Number of bar received
 * @avg_rssi: Average rssi
 * @rx_rate: Rx rate
 * @last_rx_rate: Previous rx rate
 * @rnd_avg_rx_rate: Rounded average rx rate
 * @avg_rx_rate:  Average Rx rate
 * @dot11_rx_pkts: dot11 rx packets
 * @rx_bytes_last: last Rx success bytes
 * @rx_data_last: last rx success data
 * @rx_byte_rate: bytes received in last one sec
 * @rx_data_rate: data received in last one sec
 * @rx_retries: retries of packet in rx
 * @rx_mpdus: mpdu in rx
 * @rx_ppdus: ppdu in rx
 * @is_rx_tooshort: tooshort
 * @is_rx_decap: rx decap
 * @rx_ccmpmic: rx MIC check failed (CCMP)
 * @rx_tkipmic: rx MIC check failed (TKIP)
 * @rx_tkipicv: rx ICV check failed (TKIP)
 * @rx_wpimic: rx MIC check failed (WPI)
 * @rx_wepfail: rx wep processing failed
 * @rx_aggr: aggregation on rx
 * @rx_discard: packets discard in rx
 * @rx_ratecode: Rx rate code of last frame
 * @rx_flags: rx flags
 * @rx_rssi_measured_time: Time at which rssi is measured
 * @rssi: RSSI of received signal
 * @last_rssi: Previous rssi
 */
struct cdp_rx_stats {
	struct cdp_pkt_info to_stack;
	struct cdp_pkt_info rcvd_reo[CDP_MAX_RX_RINGS];
	struct cdp_pkt_info unicast;
	struct cdp_pkt_info multicast;
	struct cdp_pkt_info bcast;
	struct cdp_pkt_info raw;
	uint32_t nawds_mcast_drop;
	struct cdp_pkt_info mec_drop;
	struct {
		struct cdp_pkt_info pkts;
		struct cdp_pkt_info fail;
		uint32_t mdns_no_fwd;
	} intra_bss;

	struct {
		uint32_t mic_err;
		uint32_t decrypt_err;
		uint32_t fcserr;
	} err;

	uint32_t wme_ac_type[WME_AC_MAX];
	uint32_t reception_type[MAX_RECEPTION_TYPES];
	struct cdp_pkt_type pkt_type[DOT11_MAX];
	uint32_t sgi_count[MAX_GI];
	uint32_t nss[SS_COUNT];
	uint32_t ppdu_nss[SS_COUNT];
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
	struct cdp_pkt_type su_ax_ppdu_cnt;
	uint32_t ppdu_cnt[MAX_RECEPTION_TYPES];
	struct cdp_rx_mu rx_mu[RX_TYPE_MU_MAX];
	uint32_t bw[MAX_BW];
	uint32_t non_ampdu_cnt;
	uint32_t ampdu_cnt;
	uint32_t non_amsdu_cnt;
	uint32_t amsdu_cnt;
	uint32_t bar_recv_cnt;
	uint32_t avg_rssi;
	uint32_t rx_rate;
	uint32_t last_rx_rate;
	uint32_t rnd_avg_rx_rate;
	uint32_t avg_rx_rate;
	struct cdp_pkt_info  dot11_rx_pkts;

	uint32_t rx_bytes_success_last;
	uint32_t rx_data_success_last;
	uint32_t rx_byte_rate;
	uint32_t rx_data_rate;

	uint32_t rx_retries;
	uint32_t rx_mpdus;
	uint32_t rx_ppdus;

	/*add for peer updated for ppdu*/
	uint32_t rx_aggr;
	uint32_t rx_discard;
	uint32_t rx_ratecode;
	uint32_t rx_flags;
	uint32_t rx_rssi_measured_time;
	uint8_t rssi;
	uint8_t last_rssi;
};

/* struct cdp_tx_ingress_stats - Tx ingress Stats
 * @rcvd: Total packets received for transmission
 * @processed: Tx packets processed
 * @inspect_pkts: Total packets passed to inspect handler
 * @nawds_mcast: NAWDS  Multicast Packet Count
 * @bcast: Number of broadcast packets
 * @raw_pkt: Total Raw packets
 * @dma_map_error: DMA map error
 * @num_seg: No of segments in TSO packets
 * @tso_pkt:total no of TSO packets
 * @non_tso_pkts: non - TSO packets
 * @dropped_host: TSO packets dropped by host
 * @dropped_target:TSO packets dropped by target
 * @sg_pkt: Total scatter gather packets
 * @non_sg_pkts: non SG packets
 * @dropped_host: SG packets dropped by host
 * @dropped_target: SG packets dropped by target
 * @dma_map_error: Dma map error
 * @mcast_pkt: total no of multicast conversion packets
 * @dropped_map_error: packets dropped due to map error
 * @dropped_self_mac: packets dropped due to self Mac address
 * @dropped_send_fail: Packets dropped due to send fail
 * @ucast: total unicast packets transmitted
 * @fail_seg_alloc: Segment allocation failure
 * @clone_fail: NBUF clone failure
 * @dropped_pkt: Total scatter gather packets
 * @desc_na: Desc Not Available
 * @ring_full: ring full
 * @enqueue_fail: hw enqueue fail
 * @dma_error: dma fail
 * @res_full: Resource Full: Congestion Control
 * @exception_fw: packets sent to fw
 * @completion_fw: packets completions received from fw
 * @cce_classified:Number of packets classified by CCE
 * @cce_classified_raw:Number of raw packets classified by CCE
 * @sniffer_rcvd: Number of packets received with ppdu cookie
 */
struct cdp_tx_ingress_stats {
	struct cdp_pkt_info rcvd;
	struct cdp_pkt_info processed;
	struct cdp_pkt_info reinject_pkts;
	struct cdp_pkt_info inspect_pkts;
	struct cdp_pkt_info nawds_mcast;
	struct cdp_pkt_info bcast;

	struct {
		struct cdp_pkt_info raw_pkt;
		uint32_t dma_map_error;
		uint32_t invalid_raw_pkt_datatype;
	} raw;

	/* Scatter Gather packet info */
	struct {
		struct cdp_pkt_info sg_pkt;
		struct cdp_pkt_info non_sg_pkts;
		struct cdp_pkt_info  dropped_host;
		uint32_t dropped_target;
		uint32_t dma_map_error;
	} sg;

	/* Multicast Enhancement packets info */
	struct {
		struct cdp_pkt_info mcast_pkt;
		uint32_t dropped_map_error;
		uint32_t dropped_self_mac;
		uint32_t dropped_send_fail;
		uint32_t ucast;
		uint32_t fail_seg_alloc;
		uint32_t clone_fail;
	} mcast_en;

	/* Packets dropped on the Tx side */
	struct {
		struct cdp_pkt_info dropped_pkt;
		struct cdp_pkt_info  desc_na;
		uint32_t ring_full;
		uint32_t enqueue_fail;
		uint32_t dma_error;
		uint32_t res_full;
		/* headroom insufficient */
		uint32_t headroom_insufficient;
	} dropped;

	/* Mesh packets info */
	struct {
		uint32_t exception_fw;
		uint32_t completion_fw;
	} mesh;

	uint32_t cce_classified;
	uint32_t cce_classified_raw;
	struct cdp_pkt_info sniffer_rcvd;
	struct cdp_tso_stats tso_stats;
};

/* struct cdp_vdev_stats - vdev stats structure
 * @tx_i: ingress tx stats
 * @tx: cdp tx stats
 * @rx: cdp rx stats
 * @tso_stats: tso stats
 */
struct cdp_vdev_stats {
	struct cdp_tx_ingress_stats tx_i;
	struct cdp_tx_stats tx;
	struct cdp_rx_stats rx;
	struct cdp_tso_stats tso_stats;
};

/* struct cdp_peer_stats - peer stats structure
 * @tx: cdp tx stats
 * @rx: cdp rx stats
 */
struct cdp_peer_stats {
	/* CDP Tx Stats */
	struct cdp_tx_stats tx;
	/* CDP Rx Stats */
	struct cdp_rx_stats rx;
};

/* struct cdp_interface_peer_stats - interface structure for txrx peer stats
 * @peer_mac: peer mac address
 * @vdev_id : vdev_id for the peer
 * @last_peer_tx_rate: peer tx rate for last transmission
 * @peer_tx_rate: tx rate for current transmission
 * @peer_rssi: current rssi value of peer
 * @tx_packet_count: tx packet count
 * @rx_packet_count: rx packet count
 * @tx_byte_count: tx byte count
 * @rx_byte_count: rx byte count
 * @per: per error rate
 * @ack_rssi: RSSI of the last ack received
 * @rssi_changed: denotes rssi is changed
 */
struct cdp_interface_peer_stats {
	uint8_t  peer_mac[QDF_MAC_ADDR_SIZE];
	uint8_t  vdev_id;
	uint8_t  rssi_changed;
	uint32_t last_peer_tx_rate;
	uint32_t peer_tx_rate;
	uint32_t peer_rssi;
	uint32_t tx_packet_count;
	uint32_t rx_packet_count;
	uint32_t tx_byte_count;
	uint32_t rx_byte_count;
	uint32_t per;
	uint32_t ack_rssi;
};

/* Tx completions per interrupt */
struct cdp_hist_tx_comp {
	uint32_t pkts_1;
	uint32_t pkts_2_20;
	uint32_t pkts_21_40;
	uint32_t pkts_41_60;
	uint32_t pkts_61_80;
	uint32_t pkts_81_100;
	uint32_t pkts_101_200;
	uint32_t pkts_201_plus;
};

/* Rx ring descriptors reaped per interrupt */
struct cdp_hist_rx_ind {
	uint32_t pkts_1;
	uint32_t pkts_2_20;
	uint32_t pkts_21_40;
	uint32_t pkts_41_60;
	uint32_t pkts_61_80;
	uint32_t pkts_81_100;
	uint32_t pkts_101_200;
	uint32_t pkts_201_plus;
};

struct cdp_htt_tlv_hdr {
	/* BIT [11 :  0]   :- tag
	 * BIT [23 : 12]   :- length
	 * BIT [31 : 24]   :- reserved
	 */
	uint32_t tag__length;
};

#define HTT_STATS_SUBTYPE_MAX     16

struct cdp_htt_rx_pdev_fw_stats_tlv {
    struct cdp_htt_tlv_hdr tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    uint32_t mac_id__word;
    /* Num PPDU status processed from HW */
    uint32_t ppdu_recvd;
    /* Num MPDU across PPDUs with FCS ok */
    uint32_t mpdu_cnt_fcs_ok;
    /* Num MPDU across PPDUs with FCS err */
    uint32_t mpdu_cnt_fcs_err;
    /* Num MSDU across PPDUs */
    uint32_t tcp_msdu_cnt;
    /* Num MSDU across PPDUs */
    uint32_t tcp_ack_msdu_cnt;
    /* Num MSDU across PPDUs */
    uint32_t udp_msdu_cnt;
    /* Num MSDU across PPDUs */
    uint32_t other_msdu_cnt;
    /* Num MPDU on FW ring indicated */
    uint32_t fw_ring_mpdu_ind;
    /* Num MGMT MPDU given to protocol */
    uint32_t fw_ring_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
    /* Num ctrl MPDU given to protocol */
    uint32_t fw_ring_ctrl_subtype[HTT_STATS_SUBTYPE_MAX];
    /* Num mcast data packet received */
    uint32_t fw_ring_mcast_data_msdu;
    /* Num broadcast data packet received */
    uint32_t fw_ring_bcast_data_msdu;
    /* Num unicat data packet received */
    uint32_t fw_ring_ucast_data_msdu;
    /* Num null data packet received  */
    uint32_t fw_ring_null_data_msdu;
    /* Num MPDU on FW ring dropped */
    uint32_t fw_ring_mpdu_drop;

    /* Num buf indication to offload */
    uint32_t ofld_local_data_ind_cnt;
    /* Num buf recycle from offload */
    uint32_t ofld_local_data_buf_recycle_cnt;
    /* Num buf indication to data_rx */
    uint32_t drx_local_data_ind_cnt;
    /* Num buf recycle from data_rx */
    uint32_t drx_local_data_buf_recycle_cnt;
    /* Num buf indication to protocol */
    uint32_t local_nondata_ind_cnt;
    /* Num buf recycle from protocol */
    uint32_t local_nondata_buf_recycle_cnt;

    /* Num buf fed */
    uint32_t fw_status_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    uint32_t fw_status_buf_ring_empty_cnt;
    /* Num buf fed  */
    uint32_t fw_pkt_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    uint32_t fw_pkt_buf_ring_empty_cnt;
    /* Num buf fed  */
    uint32_t fw_link_buf_ring_refill_cnt;
    /* Num ring empty encountered  */
    uint32_t fw_link_buf_ring_empty_cnt;

    /* Num buf fed */
    uint32_t host_pkt_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    uint32_t host_pkt_buf_ring_empty_cnt;
    /* Num buf fed */
    uint32_t mon_pkt_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    uint32_t mon_pkt_buf_ring_empty_cnt;
    /* Num buf fed */
    uint32_t mon_status_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    uint32_t mon_status_buf_ring_empty_cnt;
    /* Num buf fed */
    uint32_t mon_desc_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    uint32_t mon_desc_buf_ring_empty_cnt;
    /* Num buf fed */
    uint32_t mon_dest_ring_update_cnt;
    /* Num ring full encountered */
    uint32_t mon_dest_ring_full_cnt;

    /* Num rx suspend is attempted */
    uint32_t rx_suspend_cnt;
    /* Num rx suspend failed */
    uint32_t rx_suspend_fail_cnt;
    /* Num rx resume attempted */
    uint32_t rx_resume_cnt;
    /* Num rx resume failed */
    uint32_t rx_resume_fail_cnt;
    /* Num rx ring switch */
    uint32_t rx_ring_switch_cnt;
    /* Num rx ring restore */
    uint32_t rx_ring_restore_cnt;
    /* Num rx flush issued */
    uint32_t rx_flush_cnt;
};

/* == TX PDEV STATS == */
struct cdp_htt_tx_pdev_stats_cmn_tlv {
    struct cdp_htt_tlv_hdr tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    uint32_t mac_id__word;
    /* Num queued to HW */
    uint32_t hw_queued;
    /* Num PPDU reaped from HW */
    uint32_t hw_reaped;
    /* Num underruns */
    uint32_t underrun;
    /* Num HW Paused counter. */
    uint32_t hw_paused;
    /* Num HW flush counter. */
    uint32_t hw_flush;
    /* Num HW filtered counter. */
    uint32_t hw_filt;
    /* Num PPDUs cleaned up in TX abort */
    uint32_t tx_abort;
    /* Num MPDUs requed by SW */
    uint32_t mpdu_requed;
    /* excessive retries */
    uint32_t tx_xretry;
    /* Last used data hw rate code */
    uint32_t data_rc;
    /* frames dropped due to excessive sw retries */
    uint32_t mpdu_dropped_xretry;
    /* illegal rate phy errors  */
    uint32_t illgl_rate_phy_err;
    /* wal pdev continuous xretry */
    uint32_t cont_xretry;
    /* wal pdev continuous xretry */
    uint32_t tx_timeout;
    /* wal pdev resets  */
    uint32_t pdev_resets;
    /* PhY/BB underrun */
    uint32_t phy_underrun;
    /* MPDU is more than txop limit */
    uint32_t txop_ovf;
    /* Number of Sequences posted */
    uint32_t seq_posted;
    /* Number of Sequences failed queueing */
    uint32_t seq_failed_queueing;
    /* Number of Sequences completed */
    uint32_t seq_completed;
    /* Number of Sequences restarted */
    uint32_t seq_restarted;
    /* Number of MU Sequences posted */
    uint32_t mu_seq_posted;
    /* Number of time HW ring is paused between seq switch within ISR */
    uint32_t seq_switch_hw_paused;
    /* Number of times seq continuation in DSR */
    uint32_t next_seq_posted_dsr;
    /* Number of times seq continuation in ISR */
    uint32_t seq_posted_isr;
    /* Number of seq_ctrl cached. */
    uint32_t seq_ctrl_cached;
    /* Number of MPDUs successfully transmitted */
    uint32_t mpdu_count_tqm;
    /* Number of MSDUs successfully transmitted */
    uint32_t msdu_count_tqm;
    /* Number of MPDUs dropped */
    uint32_t mpdu_removed_tqm;
    /* Number of MSDUs dropped */
    uint32_t msdu_removed_tqm;
    /* Num MPDUs flushed by SW, HWPAUSED, SW TXABORT (Reset,channel change) */
    uint32_t mpdus_sw_flush;
    /* Num MPDUs filtered by HW, all filter condition (TTL expired) */
    uint32_t mpdus_hw_filter;
    /* Num MPDUs truncated by PDG (TXOP, TBTT, PPDU_duration based on rate, dyn_bw) */
    uint32_t mpdus_truncated;
    /* Num MPDUs that was tried but didn't receive ACK or BA */
    uint32_t mpdus_ack_failed;
    /* Num MPDUs that was dropped due to expiry (MSDU TTL). */
    uint32_t mpdus_expired;
    /* Num MPDUs that was retried within seq_ctrl (MGMT/LEGACY) */
    uint32_t mpdus_seq_hw_retry;
    /* Num of TQM acked cmds processed */
    uint32_t ack_tlv_proc;
    /* coex_abort_mpdu_cnt valid. */
    uint32_t coex_abort_mpdu_cnt_valid;
    /* coex_abort_mpdu_cnt from TX FES stats. */
    uint32_t coex_abort_mpdu_cnt;
    /* Number of total PPDUs(DATA, MGMT, excludes selfgen) tried over the air (OTA) */
    uint32_t num_total_ppdus_tried_ota;
    /* Number of data PPDUs tried over the air (OTA) */
    uint32_t num_data_ppdus_tried_ota;
    /* Num Local control/mgmt frames (MSDUs) queued */
    uint32_t local_ctrl_mgmt_enqued;
    /* local_ctrl_mgmt_freed:
     * Num Local control/mgmt frames (MSDUs) done
     * It includes all local ctrl/mgmt completions
     * (acked, no ack, flush, TTL, etc)
     */
    uint32_t local_ctrl_mgmt_freed;
    /* Num Local data frames (MSDUs) queued */
    uint32_t local_data_enqued;
    /* local_data_freed:
     * Num Local data frames (MSDUs) done
     * It includes all local data completions
     * (acked, no ack, flush, TTL, etc)
     */
    uint32_t local_data_freed;

	/* Num MPDUs tried by SW */
	uint32_t mpdu_tried;
	/* Num of waiting seq posted in isr completion handler */
	uint32_t isr_wait_seq_posted;
	uint32_t tx_active_dur_us_low;
	uint32_t tx_active_dur_us_high;
};

struct cdp_htt_tx_pdev_stats_urrn_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    uint32_t urrn_stats[1]; /* HTT_TX_PDEV_MAX_URRN_STATS */
};

/* NOTE: Variable length TLV, use length spec to infer array size */
struct cdp_htt_tx_pdev_stats_flush_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    uint32_t flush_errs[1]; /* HTT_TX_PDEV_MAX_FLUSH_REASON_STATS */
};

/* NOTE: Variable length TLV, use length spec to infer array size */
struct cdp_htt_tx_pdev_stats_sifs_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    uint32_t sifs_status[1]; /* HTT_TX_PDEV_MAX_SIFS_BURST_STATS */
};

/* NOTE: Variable length TLV, use length spec to infer array size */
struct cdp_htt_tx_pdev_stats_phy_err_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    uint32_t  phy_errs[1]; /* HTT_TX_PDEV_MAX_PHY_ERR_STATS */
};

/* == RX PDEV/SOC STATS == */
/* HTT_STATS_RX_SOC_FW_STATS_TAG */
struct cdp_htt_rx_soc_fw_stats_tlv {
    struct cdp_htt_tlv_hdr tlv_hdr;
    /* Num Packets received on REO FW ring */
    uint32_t fw_reo_ring_data_msdu;
    /* Num bc/mc packets indicated from fw to host */
    uint32_t fw_to_host_data_msdu_bcmc;
    /* Num unicast packets indicated from fw to host */
    uint32_t fw_to_host_data_msdu_uc;
    /* Num remote buf recycle from offload  */
    uint32_t ofld_remote_data_buf_recycle_cnt;
    /* Num remote free buf given to offload */
    uint32_t ofld_remote_free_buf_indication_cnt;
};

struct cdp_htt_rx_soc_fw_refill_ring_num_refill_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    /* Num total buf refilled from refill ring */
    uint32_t refill_ring_num_refill[1]; /* HTT_RX_STATS_REFILL_MAX_RING */
};

struct cdp_htt_rx_pdev_fw_ring_mpdu_err_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    /* Num error MPDU for each RxDMA error type  */
    uint32_t fw_ring_mpdu_err[1]; /* HTT_RX_STATS_RXDMA_MAX_ERR */
};

struct cdp_htt_rx_pdev_fw_mpdu_drop_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    /* Num MPDU dropped  */
    uint32_t fw_mpdu_drop[1]; /* HTT_RX_STATS_FW_DROP_REASON_MAX */
};

#define HTT_STATS_PHY_ERR_MAX 43

struct cdp_htt_rx_pdev_fw_stats_phy_err_tlv {
    struct cdp_htt_tlv_hdr tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    uint32_t mac_id__word;
    /* Num of phy err */
    uint32_t total_phy_err_cnt;
    /* Counts of different types of phy errs
     * The mapping of PHY error types to phy_err array elements is HW dependent.
     * The only currently-supported mapping is shown below:
     *
     * 0 phyrx_err_phy_off Reception aborted due to receiving a PHY_OFF TLV
     * 1 phyrx_err_synth_off
     * 2 phyrx_err_ofdma_timing
     * 3 phyrx_err_ofdma_signal_parity
     * 4 phyrx_err_ofdma_rate_illegal
     * 5 phyrx_err_ofdma_length_illegal
     * 6 phyrx_err_ofdma_restart
     * 7 phyrx_err_ofdma_service
     * 8 phyrx_err_ppdu_ofdma_power_drop
     * 9 phyrx_err_cck_blokker
     * 10 phyrx_err_cck_timing
     * 11 phyrx_err_cck_header_crc
     * 12 phyrx_err_cck_rate_illegal
     * 13 phyrx_err_cck_length_illegal
     * 14 phyrx_err_cck_restart
     * 15 phyrx_err_cck_service
     * 16 phyrx_err_cck_power_drop
     * 17 phyrx_err_ht_crc_err
     * 18 phyrx_err_ht_length_illegal
     * 19 phyrx_err_ht_rate_illegal
     * 20 phyrx_err_ht_zlf
     * 21 phyrx_err_false_radar_ext
     * 22 phyrx_err_green_field
     * 23 phyrx_err_bw_gt_dyn_bw
     * 24 phyrx_err_leg_ht_mismatch
     * 25 phyrx_err_vht_crc_error
     * 26 phyrx_err_vht_siga_unsupported
     * 27 phyrx_err_vht_lsig_len_invalid
     * 28 phyrx_err_vht_ndp_or_zlf
     * 29 phyrx_err_vht_nsym_lt_zero
     * 30 phyrx_err_vht_rx_extra_symbol_mismatch
     * 31 phyrx_err_vht_rx_skip_group_id0
     * 32 phyrx_err_vht_rx_skip_group_id1to62
     * 33 phyrx_err_vht_rx_skip_group_id63
     * 34 phyrx_err_ofdm_ldpc_decoder_disabled
     * 35 phyrx_err_defer_nap
     * 36 phyrx_err_fdomain_timeout
     * 37 phyrx_err_lsig_rel_check
     * 38 phyrx_err_bt_collision
     * 39 phyrx_err_unsupported_mu_feedback
     * 40 phyrx_err_ppdu_tx_interrupt_rx
     * 41 phyrx_err_unsupported_cbf
     * 42 phyrx_err_other
     */
    uint32_t phy_err[HTT_STATS_PHY_ERR_MAX];
};

struct cdp_htt_rx_soc_fw_refill_ring_empty_tlv_v {
    struct cdp_htt_tlv_hdr tlv_hdr;
    /* Num ring empty encountered */
    uint32_t refill_ring_empty_cnt[1]; /* HTT_RX_STATS_REFILL_MAX_RING */
};

struct cdp_htt_tx_pdev_stats {
    struct cdp_htt_tx_pdev_stats_cmn_tlv cmn_tlv;
    struct cdp_htt_tx_pdev_stats_urrn_tlv_v underrun_tlv;
    struct cdp_htt_tx_pdev_stats_sifs_tlv_v sifs_tlv;
    struct cdp_htt_tx_pdev_stats_flush_tlv_v flush_tlv;
    struct cdp_htt_tx_pdev_stats_phy_err_tlv_v phy_err_tlv;
};

struct cdp_htt_rx_soc_stats_t {
    struct cdp_htt_rx_soc_fw_stats_tlv fw_tlv;
    struct cdp_htt_rx_soc_fw_refill_ring_empty_tlv_v fw_refill_ring_empty_tlv;
    struct cdp_htt_rx_soc_fw_refill_ring_num_refill_tlv_v fw_refill_ring_num_refill_tlv;
};

struct cdp_htt_rx_pdev_stats {
    struct cdp_htt_rx_soc_stats_t soc_stats;
    struct cdp_htt_rx_pdev_fw_stats_tlv fw_stats_tlv;
    struct cdp_htt_rx_pdev_fw_ring_mpdu_err_tlv_v fw_ring_mpdu_err_tlv;
    struct cdp_htt_rx_pdev_fw_mpdu_drop_tlv_v fw_ring_mpdu_drop;
    struct cdp_htt_rx_pdev_fw_stats_phy_err_tlv fw_stats_phy_err_tlv;
};

#ifdef WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG
/* Since protocol type enumeration value is passed as CCE metadata
 * to firmware, add a constant offset before passing it to firmware
 */
#define RX_PROTOCOL_TAG_START_OFFSET  128
/* This should align with packet type enumerations in ieee80211_ioctl.h
 * and wmi_unified_param.h files
 */
#define RX_PROTOCOL_TAG_MAX   24
/* Macro that should be used to dump the statistics counter for all
 * protocol types
 */
#define RX_PROTOCOL_TAG_ALL 0xff
#endif /* WLAN_SUPPORT_RX_PROTOCOL_TYPE_TAG */

#define OFDMA_NUM_RU_SIZE 7

#define OFDMA_NUM_USERS	37
/* struct cdp_pdev_stats - pdev stats
 * @msdu_not_done: packets dropped because msdu done bit not set
 * @mec:Multicast Echo check
 * @mesh_filter: Mesh Filtered packets
 * @mon_rx_drop: packets dropped on monitor vap
 * @wifi_parse: rxdma errors due to wifi parse error
 * @mon_radiotap_update_err: not enough space to update radiotap
 * @pkts: total packets replenished
 * @rxdma_err: rxdma errors for replenished
 * @nbuf_alloc_fail: nbuf alloc failed
 * @map_err: Mapping failure
 * @x86_fail: x86 failures
 * @low_thresh_intrs: low threshold interrupts
 * @rx_raw_pkts: Rx Raw Packets
 * @mesh_mem_alloc: Mesh Rx Stats Alloc fail
 * @tso_desc_cnt: TSO descriptors
 * @sg_desc_cnt: SG Descriptors
 * @vlan_tag_stp_cnt: Vlan tagged Stp packets in wifi parse error
 * @desc_alloc_fail: desc alloc failed errors
 * @ip_csum_err: ip checksum errors
 * @tcp_udp_csum_err: tcp/udp checksum errors
 * @buf_freelist: buffers added back in freelist
 * @tx_i: Tx Ingress stats
 * @tx:CDP Tx Stats
 * @rx: CDP Rx Stats
 * @tx_comp_histogram: Number of Tx completions per interrupt
 * @rx_ind_histogram:  Number of Rx ring descriptors reaped per interrupt
 * @ppdu_stats_counter: ppdu stats counter
 * @cdp_delayed_ba_not_recev: counter for delayed ba not received
 * @htt_tx_pdev_stats: htt pdev stats for tx
 * @htt_rx_pdev_stats: htt pdev stats for rx
 * @data_rx_ru_size: UL ofdma data ru size counter array
 * @nondata_rx_ru_size: UL ofdma non data ru size counter array
 * @data_rx_ppdu: data rx ppdu counter
 * @data_user: data user counter array
 */
struct cdp_pdev_stats {
	struct {
		uint32_t msdu_not_done;
		uint32_t mec;
		uint32_t mesh_filter;
		uint32_t wifi_parse;
		/* Monitor mode related */
		uint32_t mon_rx_drop;
		uint32_t mon_radiotap_update_err;
	} dropped;

	struct {
		struct cdp_pkt_info pkts;
		uint32_t rxdma_err;
		uint32_t nbuf_alloc_fail;
		uint32_t map_err;
		uint32_t x86_fail;
		uint32_t low_thresh_intrs;
	} replenish;

	uint32_t rx_raw_pkts;
	uint32_t mesh_mem_alloc;
	uint32_t tso_desc_cnt;
	uint32_t sg_desc_cnt;
	uint32_t vlan_tag_stp_cnt;

	/* Rx errors */
	struct {
		uint32_t desc_alloc_fail;
		uint32_t ip_csum_err;
		uint32_t tcp_udp_csum_err;
		uint32_t rxdma_error;
		uint32_t reo_error;
	} err;

	uint32_t buf_freelist;
	struct cdp_tx_ingress_stats tx_i;
	struct cdp_tx_stats tx;
	struct cdp_rx_stats rx;
	struct cdp_hist_tx_comp tx_comp_histogram;
	struct cdp_hist_rx_ind rx_ind_histogram;
	uint64_t ppdu_stats_counter[CDP_PPDU_STATS_MAX_TAG];
	uint32_t cdp_delayed_ba_not_recev;

	struct cdp_htt_tx_pdev_stats  htt_tx_pdev_stats;
	struct cdp_htt_rx_pdev_stats  htt_rx_pdev_stats;

	/* Received wdi messages from fw */
	uint32_t wdi_event[CDP_WDI_NUM_EVENTS];
	struct cdp_tid_stats tid_stats;

	/* numbers of data/nondata per RU sizes */
	struct {
		uint32_t data_rx_ru_size[OFDMA_NUM_RU_SIZE];
		uint32_t nondata_rx_ru_size[OFDMA_NUM_RU_SIZE];
		uint32_t data_rx_ppdu;
		uint32_t data_users[OFDMA_NUM_USERS];
	} ul_ofdma;

	struct cdp_tso_stats tso_stats;
};

#ifdef QCA_ENH_V3_STATS_SUPPORT
/*
 * Enumeration of PDEV Configuration parameter
 */
enum _ol_ath_param_t {
	OL_ATH_PARAM_TXCHAINMASK               = 1,
	OL_ATH_PARAM_RXCHAINMASK               = 2,
	OL_ATH_PARAM_AMPDU                     = 6,
	OL_ATH_PARAM_AMPDU_LIMIT               = 7,
	OL_ATH_PARAM_AMPDU_SUBFRAMES           = 8,
	OL_ATH_PARAM_TXPOWER_LIMIT2G           = 12,
	OL_ATH_PARAM_TXPOWER_LIMIT5G           = 13,
	OL_ATH_PARAM_LDPC                      = 32,
	OL_ATH_PARAM_VOW_EXT_STATS             = 45,
	OL_ATH_PARAM_DYN_TX_CHAINMASK          = 73,
	OL_ATH_PARAM_BURST_ENABLE              = 77,
	OL_ATH_PARAM_BURST_DUR                 = 78,
	OL_ATH_PARAM_BCN_BURST                 = 80,
	OL_ATH_PARAM_DCS                       = 82,
#if UMAC_SUPPORT_PERIODIC_PERFSTATS
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT_ENAB   = 83,
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT_WIN    = 84,
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT        = 85,
	OL_ATH_PARAM_PRDPERFSTAT_PER_ENAB      = 86,
	OL_ATH_PARAM_PRDPERFSTAT_PER_WIN       = 87,
	OL_ATH_PARAM_PRDPERFSTAT_PER           = 88,
#endif
	/* UMAC_SUPPORT_PERIODIC_PERFSTATS */
	OL_ATH_PARAM_TOTAL_PER                 = 89,
	/* set manual rate for rts frame */
	OL_ATH_PARAM_RTS_CTS_RATE              = 92,
	/* co channel interference threshold level */
	OL_ATH_PARAM_DCS_COCH_THR              = 93,
	/* transmit error threshold */
	OL_ATH_PARAM_DCS_TXERR_THR             = 94,
	/* phy error threshold */
	OL_ATH_PARAM_DCS_PHYERR_THR            = 95,
	/*
	 * The IOCTL number is 114, it is made 114, inorder to make the IOCTL
	 * number same as Direct-attach IOCTL.
	 * Please, don't change number. This IOCTL gets the Interface code path
	 * it should be either DIRECT-ATTACH or OFF-LOAD.
	 */
	OL_ATH_PARAM_GET_IF_ID                 = 114,
	/* Enable Acs back Ground Channel selection Scan timer in AP mode*/
	OL_ATH_PARAM_ACS_ENABLE_BK_SCANTIMEREN = 118,
	 /* ACS scan timer value in Seconds */
	OL_ATH_PARAM_ACS_SCANTIME              = 119,
	 /* Negligence Delta RSSI between two channel */
	OL_ATH_PARAM_ACS_RSSIVAR               = 120,
	 /* Negligence Delta Channel load between two channel*/
	OL_ATH_PARAM_ACS_CHLOADVAR             = 121,
	  /* Enable Limited OBSS check */
	OL_ATH_PARAM_ACS_LIMITEDOBSS           = 122,
	/* Acs control flag for Scan timer */
	OL_ATH_PARAM_ACS_CTRLFLAG              = 123,
	 /* Acs Run time Debug level*/
	OL_ATH_PARAM_ACS_DEBUGTRACE            = 124,
	OL_ATH_PARAM_SET_FW_HANG_ID            = 137,
	 /* Radio type 1:11ac 0:11abgn */
	OL_ATH_PARAM_RADIO_TYPE                = 138,
	OL_ATH_PARAM_IGMPMLD_OVERRIDE, /* IGMP/MLD packet override */
	OL_ATH_PARAM_IGMPMLD_TID, /* IGMP/MLD packet TID no */
	OL_ATH_PARAM_ARPDHCP_AC_OVERRIDE,
	OL_ATH_PARAM_NON_AGG_SW_RETRY_TH,
	OL_ATH_PARAM_AGG_SW_RETRY_TH,
	/* Dont change this number it as per sync with DA
	     Blocking certian channel from ic channel list */
	OL_ATH_PARAM_DISABLE_DFS   = 144,
	OL_ATH_PARAM_ENABLE_AMSDU  = 145,
	OL_ATH_PARAM_ENABLE_AMPDU  = 146,
	OL_ATH_PARAM_STA_KICKOUT_TH,
	OL_ATH_PARAM_WLAN_PROF_ENABLE,
	OL_ATH_PARAM_LTR_ENABLE,
	OL_ATH_PARAM_LTR_AC_LATENCY_BE = 150,
	OL_ATH_PARAM_LTR_AC_LATENCY_BK,
	OL_ATH_PARAM_LTR_AC_LATENCY_VI,
	OL_ATH_PARAM_LTR_AC_LATENCY_VO,
	OL_ATH_PARAM_LTR_AC_LATENCY_TIMEOUT,
	OL_ATH_PARAM_LTR_TX_ACTIVITY_TIMEOUT = 155,
	OL_ATH_PARAM_LTR_SLEEP_OVERRIDE,
	OL_ATH_PARAM_LTR_RX_OVERRIDE,
	OL_ATH_PARAM_L1SS_ENABLE,
	OL_ATH_PARAM_DSLEEP_ENABLE,
	/* radar error threshold */
	OL_ATH_PARAM_DCS_RADAR_ERR_THR = 160,
	/* Tx channel utilization due to AP's tx and rx */
	OL_ATH_PARAM_DCS_USERMAX_CU_THR,
	/* interference detection threshold */
	OL_ATH_PARAM_DCS_INTR_DETECT_THR,
	/* sampling window, default 10secs */
	OL_ATH_PARAM_DCS_SAMPLE_WINDOW,
	/* debug logs enable/disable */
	OL_ATH_PARAM_DCS_DEBUG,
	OL_ATH_PARAM_ANI_ENABLE = 165,
	OL_ATH_PARAM_ANI_POLL_PERIOD,
	OL_ATH_PARAM_ANI_LISTEN_PERIOD,
	OL_ATH_PARAM_ANI_OFDM_LEVEL,
	OL_ATH_PARAM_ANI_CCK_LEVEL,
	OL_ATH_PARAM_DSCP_TID_MAP = 170,
	OL_ATH_PARAM_TXPOWER_SCALE,
	/* Phy error penalty */
	OL_ATH_PARAM_DCS_PHYERR_PENALTY,
#if ATH_SUPPORT_DSCP_OVERRIDE
	/* set/get TID for sending HMMC packets */
	OL_ATH_PARAM_HMMC_DSCP_TID_MAP,
	/* set/get DSCP mapping override */
	OL_ATH_PARAM_DSCP_OVERRIDE,
	/* set/get HMMC-DSCP mapping override */
	OL_ATH_PARAM_HMMC_DSCP_OVERRIDE = 175,
#endif
#if ATH_RX_LOOPLIMIT_TIMER
	OL_ATH_PARAM_LOOPLIMIT_NUM,
#endif
	OL_ATH_PARAM_ANTENNA_GAIN_2G,
	OL_ATH_PARAM_ANTENNA_GAIN_5G,
	OL_ATH_PARAM_RX_FILTER,
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
	OL_ATH_PARAM_BUFF_THRESH = 180,
	OL_ATH_PARAM_BLK_REPORT_FLOOD,
	OL_ATH_PARAM_DROP_STA_QUERY,
#endif
	OL_ATH_PARAM_QBOOST,
	OL_ATH_PARAM_SIFS_FRMTYPE,
	OL_ATH_PARAM_SIFS_UAPSD = 185,
	OL_ATH_PARAM_FW_RECOVERY_ID,
	OL_ATH_PARAM_RESET_OL_STATS,
	OL_ATH_PARAM_AGGR_BURST,
	/* Number of deauth sent in consecutive rx_peer_invalid */
	OL_ATH_PARAM_DEAUTH_COUNT,
	OL_ATH_PARAM_BLOCK_INTERBSS = 190,
	/* Firmware reset control for Bmiss / timeout / reset */
	OL_ATH_PARAM_FW_DISABLE_RESET,
	OL_ATH_PARAM_MSDU_TTL,
	OL_ATH_PARAM_PPDU_DURATION,
	OL_ATH_PARAM_SET_TXBF_SND_PERIOD,
	OL_ATH_PARAM_ALLOW_PROMISC = 195,
	OL_ATH_PARAM_BURST_MODE,
	OL_ATH_PARAM_DYN_GROUPING,
	OL_ATH_PARAM_DPD_ENABLE,
	OL_ATH_PARAM_DBGLOG_RATELIM,
	/* firmware should intimate us about ps state change for node  */
	OL_ATH_PARAM_PS_STATE_CHANGE = 200,
	OL_ATH_PARAM_MCAST_BCAST_ECHO,
	/* OBSS RSSI threshold for 20/40 coexistence */
	OL_ATH_PARAM_OBSS_RSSI_THRESHOLD,
	/* Link/node RX RSSI threshold  for 20/40 coexistence */
	OL_ATH_PARAM_OBSS_RX_RSSI_THRESHOLD,
#if ATH_CHANNEL_BLOCKING
	OL_ATH_PARAM_ACS_BLOCK_MODE = 205,
#endif
	OL_ATH_PARAM_ACS_TX_POWER_OPTION,
	/*
	 * Default Antenna Polarization MSB 8 bits (24:31) specifying
	 * enable/disable ; LSB 24 bits (0:23) antenna mask value
	 */
	OL_ATH_PARAM_ANT_POLARIZATION,
	/* rate limit mute type error prints */
	OL_ATH_PARAM_PRINT_RATE_LIMIT,
	OL_ATH_PARAM_PDEV_RESET,   /* Reset FW PDEV*/
	/* Do not crash host when target assert happened*/
	OL_ATH_PARAM_FW_DUMP_NO_HOST_CRASH = 210,
	/* Consider OBSS non-erp to change to long slot*/
	OL_ATH_PARAM_CONSIDER_OBSS_NON_ERP_LONG_SLOT = 211,
	OL_ATH_PARAM_STATS_FC,
	OL_ATH_PARAM_QFLUSHINTERVAL,
	OL_ATH_PARAM_TOTAL_Q_SIZE,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE0,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE1,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE2,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE3,
	OL_ATH_PARAM_MIN_THRESHOLD,
	OL_ATH_PARAM_MAX_Q_LIMIT,
	OL_ATH_PARAM_MIN_Q_LIMIT,
	OL_ATH_PARAM_CONG_CTRL_TIMER_INTV,
	OL_ATH_PARAM_STATS_TIMER_INTV,
	OL_ATH_PARAM_ROTTING_TIMER_INTV,
	OL_ATH_PARAM_LATENCY_PROFILE,
	OL_ATH_PARAM_HOSTQ_DUMP,
	OL_ATH_PARAM_TIDQ_MAP,
	OL_ATH_PARAM_DBG_ARP_SRC_ADDR, /* ARP DEBUG source address*/
	OL_ATH_PARAM_DBG_ARP_DST_ADDR, /* ARP DEBUG destination address*/
	OL_ATH_PARAM_ARP_DBG_CONF,   /* ARP debug configuration */
	OL_ATH_PARAM_DISABLE_STA_VAP_AMSDU, /* Disable AMSDU for station vap */
#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
	OL_ATH_PARAM_STADFS_ENABLE = 300,    /* STA DFS is enabled or not  */
#endif
#if QCA_AIRTIME_FAIRNESS
	OL_ATH_PARAM_ATF_STRICT_SCHED = 301,
	OL_ATH_PARAM_ATF_GROUP_POLICY = 302,
#endif
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_PRIMARY_RADIO,
	OL_ATH_PARAM_DBDC_ENABLE,
#endif
	OL_ATH_PARAM_TXPOWER_DBSCALE,
	OL_ATH_PARAM_CTL_POWER_SCALE,
#if QCA_AIRTIME_FAIRNESS
	OL_ATH_PARAM_ATF_OBSS_SCHED = 307,
	OL_ATH_PARAM_ATF_OBSS_SCALE = 308,
#endif
	OL_ATH_PARAM_PHY_OFDM_ERR = 309,
	OL_ATH_PARAM_PHY_CCK_ERR = 310,
	OL_ATH_PARAM_FCS_ERR = 311,
	OL_ATH_PARAM_CHAN_UTIL = 312,
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_CLIENT_MCAST,
#endif
	OL_ATH_PARAM_EMIWAR_80P80 = 314,
	OL_ATH_PARAM_BATCHMODE = 315,
	OL_ATH_PARAM_PACK_AGGR_DELAY = 316,
#if UMAC_SUPPORT_ACFG
	OL_ATH_PARAM_DIAG_ENABLE = 317,
#endif
#if ATH_SUPPORT_VAP_QOS
	OL_ATH_PARAM_VAP_QOS = 318,
#endif
	OL_ATH_PARAM_CHAN_STATS_TH = 319,
	/* Passive scan is enabled or disabled  */
	OL_ATH_PARAM_PASSIVE_SCAN_ENABLE = 320,
	OL_ATH_MIN_RSSI_ENABLE = 321,
	OL_ATH_MIN_RSSI = 322,
	OL_ATH_PARAM_ACS_2G_ALLCHAN = 323,
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_DELAY_STAVAP_UP = 324,
#endif
	/* It is used to set the channel switch options */
	OL_ATH_PARAM_CHANSWITCH_OPTIONS = 327,
	OL_ATH_BTCOEX_ENABLE        = 328,
	OL_ATH_BTCOEX_WL_PRIORITY   = 329,
	OL_ATH_PARAM_TID_OVERRIDE_QUEUE_MAPPING = 330,
	OL_ATH_PARAM_CAL_VER_CHECK = 331,
	OL_ATH_PARAM_NO_VLAN       = 332,
	OL_ATH_PARAM_CCA_THRESHOLD = 333,
	OL_ATH_PARAM_ATF_LOGGING = 334,
	OL_ATH_PARAM_STRICT_DOTH = 335,
	OL_ATH_PARAM_DISCONNECTION_TIMEOUT   = 336,
	OL_ATH_PARAM_RECONFIGURATION_TIMEOUT = 337,
	OL_ATH_PARAM_CHANNEL_SWITCH_COUNT = 338,
	OL_ATH_PARAM_ALWAYS_PRIMARY = 339,
	OL_ATH_PARAM_FAST_LANE = 340,
	OL_ATH_GET_BTCOEX_DUTY_CYCLE = 341,
	OL_ATH_PARAM_SECONDARY_OFFSET_IE = 342,
	OL_ATH_PARAM_WIDE_BAND_SUB_ELEMENT = 343,
	OL_ATH_PARAM_PREFERRED_UPLINK = 344,
	OL_ATH_PARAM_PRECAC_ENABLE = 345,
	OL_ATH_PARAM_PRECAC_TIMEOUT = 346,
	OL_ATH_COEX_VER_CFG = 347,
	OL_ATH_PARAM_DUMP_TARGET = 348,
	OL_ATH_PARAM_PDEV_TO_REO_DEST = 349,
	OL_ATH_PARAM_DUMP_CHAINMASK_TABLES = 350,
	OL_ATH_PARAM_DUMP_OBJECTS = 351,
	OL_ATH_PARAM_ACS_SRLOADVAR = 352,
	OL_ATH_PARAM_MGMT_RSSI_THRESHOLD = 353,
	OL_ATH_PARAM_EXT_NSS_CAPABLE = 354,
	OL_ATH_PARAM_MGMT_PDEV_STATS_TIMER = 355,
	OL_ATH_PARAM_TXACKTIMEOUT = 356,
	OL_ATH_PARAM_ICM_ACTIVE = 357,
	OL_ATH_PARAM_NOMINAL_NOISEFLOOR = 358,
	OL_ATH_PARAM_CHAN_INFO = 359,
	OL_ATH_PARAM_ACS_RANK = 360,
	OL_ATH_PARAM_TXCHAINSOFT = 361,
	OL_ATH_PARAM_WIDE_BAND_SCAN = 362,
	OL_ATH_PARAM_CCK_TX_ENABLE = 363,
	OL_ATH_PARAM_PAPI_ENABLE = 364,
	OL_ATH_PARAM_ISOLATION = 365,
	OL_ATH_PARAM_MAX_CLIENTS_PER_RADIO = 366,
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
	OL_ATH_PARAM_DFS_HOST_WAIT_TIMEOUT = 367,
#endif
	OL_ATH_PARAM_NF_THRESH = 368,
#ifdef OL_ATH_SMART_LOGGING
	OL_ATH_PARAM_SMARTLOG_ENABLE = 369,
	OL_ATH_PARAM_SMARTLOG_FATAL_EVENT = 370,
	OL_ATH_PARAM_SMARTLOG_SKB_SZ = 371,
	OL_ATH_PARAM_SMARTLOG_P1PINGFAIL = 372,
#endif /* OL_ATH_SMART_LOGGING */
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
	OL_ATH_PARAM_PRECAC_INTER_CHANNEL = 373,
	OL_ATH_PARAM_PRECAC_CHAN_STATE = 374,
#endif
	OL_ATH_PARAM_DBR_RING_STATUS = 375,
#ifdef QCN_ESP_IE
	OL_ATH_PARAM_ESP_PERIODICITY = 376,
	OL_ATH_PARAM_ESP_AIRTIME = 377,
	OL_ATH_PARAM_ESP_PPDU_DURATION = 378,
	OL_ATH_PARAM_ESP_BA_WINDOW = 379,
#endif /* QCN_ESP_IE */

	OL_ATH_PARAM_CBS = 380,
	OL_ATH_PARAM_DCS_SIM = 381,
	OL_ATH_PARAM_CBS_DWELL_SPLIT_TIME = 382,
	OL_ATH_PARAM_CBS_DWELL_REST_TIME = 383,
	OL_ATH_PARAM_CBS_WAIT_TIME = 384,
	OL_ATH_PARAM_CBS_REST_TIME = 385,
	OL_ATH_PARAM_CBS_CSA = 386,
	OL_ATH_PARAM_TWICE_ANTENNA_GAIN = 387,
	OL_ATH_PARAM_ACTIVITY_FACTOR = 388,
	OL_ATH_PARAM_CHAN_AP_RX_UTIL = 389,
	OL_ATH_PARAM_CHAN_FREE  = 390,
	OL_ATH_PARAM_CHAN_AP_TX_UTIL = 391,
	OL_ATH_PARAM_CHAN_OBSS_RX_UTIL = 392,
	OL_ATH_PARAM_CHAN_NON_WIFI = 393,
#if PEER_FLOW_CONTROL
	OL_ATH_PARAM_VIDEO_STATS_FC = 394,
	OL_ATH_PARAM_VIDEO_DELAY_STATS_FC = 395,
#endif
	OL_ATH_PARAM_ENABLE_PEER_RETRY_STATS = 396,
	OL_ATH_PARAM_HE_UL_TRIG_INT = 397,
	OL_ATH_PARAM_DFS_NOL_SUBCHANNEL_MARKING = 398,
	/*
	 * Get the band that is tuned for low, high,
	 * full band freq range or it's 2g
	 */
	OL_ATH_PARAM_BAND_INFO = 399,
	OL_ATH_PARAM_BW_REDUCE = 400,
	/* Enable/disable Spatial Reuse */
	OL_ATH_PARAM_HE_SR = 401,
	OL_ATH_PARAM_HE_UL_PPDU_DURATION = 402,
	OL_ATH_PARAM_HE_UL_RU_ALLOCATION = 403,
	OL_ATH_PARAM_PERIODIC_CFR_CAPTURE = 404,
	OL_ATH_PARAM_FLUSH_PEER_RATE_STATS = 405,
	OL_ATH_PARAM_DCS_RE_ENABLE_TIMER = 406,
	/* Enable/disable Rx lite monitor mode */
	OL_ATH_PARAM_RX_MON_LITE = 407,
	/* wifi down indication used in MBSS feature */
	OL_ATH_PARAM_WIFI_DOWN_IND = 408,
	OL_ATH_PARAM_TX_CAPTURE = 409,
	/* Enable fw dump collectin if wmi disconnects */
	OL_ATH_PARAM_WMI_DIS_DUMP = 410,
	OL_ATH_PARAM_ACS_CHAN_GRADE_ALGO = 411,
	OL_ATH_PARAM_ACS_CHAN_EFFICIENCY_VAR = 412,
	OL_ATH_PARAM_ACS_NEAR_RANGE_WEIGHTAGE = 413,
	OL_ATH_PARAM_ACS_MID_RANGE_WEIGHTAGE = 414,
	OL_ATH_PARAM_ACS_FAR_RANGE_WEIGHTAGE = 415,
	/* Set SELF AP OBSS_PD_THRESHOLD value */
	OL_ATH_PARAM_SET_CMD_OBSS_PD_THRESHOLD = 416,
	/* Enable/Disable/Set MGMT_TTL in milliseconds. */
	OL_ATH_PARAM_MGMT_TTL = 417,
	/* Enable/Disable/Set PROBE_RESP_TTL in milliseconds */
	OL_ATH_PARAM_PROBE_RESP_TTL = 418,
	/* Set global MU PPDU duration for DL (usec units) */
	OL_ATH_PARAM_MU_PPDU_DURATION = 419,
	/* Set TBTT_CTRL_CFG */
	OL_ATH_PARAM_TBTT_CTRL = 420,
	/* Enable/disable AP OBSS_PD_THRESHOLD */
	OL_ATH_PARAM_SET_CMD_OBSS_PD_THRESHOLD_ENABLE = 421,
	/* Get baseline radio level channel width */
	OL_ATH_PARAM_RCHWIDTH = 422,
	/* Whether external ACS request is in progress */
	OL_ATH_EXT_ACS_REQUEST_IN_PROGRESS = 423,
	/* set/get hw mode */
	OL_ATH_PARAM_HW_MODE  = 424,
#if DBDC_REPEATER_SUPPORT
	/* same ssid feature global disable */
	OL_ATH_PARAM_SAME_SSID_DISABLE = 425,
#endif
	/* get MBSS enable flag */
	OL_ATH_PARAM_MBSS_EN  = 426,
	/* UNII-1 and UNII-2A channel coexistance */
	OL_ATH_PARAM_CHAN_COEX = 427,
	/* Out of Band Advertisement feature */
	OL_ATH_PARAM_OOB_ENABLE = 428,
	/* set/get opmode-notification timer for hw-mode switch */
	OL_ATH_PARAM_HW_MODE_SWITCH_OMN_TIMER = 429,
	/* enable opmode-notification when doing hw-mode switch */
	OL_ATH_PARAM_HW_MODE_SWITCH_OMN_ENABLE = 430,
	/* set primary interface for hw-mode switch */
	OL_ATH_PARAM_HW_MODE_SWITCH_PRIMARY_IF = 431,
};
#endif
/* Bitmasks for stats that can block */
#define EXT_TXRX_FW_STATS		0x0001
#endif

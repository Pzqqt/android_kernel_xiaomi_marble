/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include <cdp_txrx_hist_struct.h>

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

/*
 * Max of TxRx context
 */
#define CDP_MAX_TXRX_CTX CDP_MAX_RX_RINGS

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
	RU_INDEX_MAX,
};

struct cdp_ru_debug {
	char *ru_type;
};

static const struct cdp_ru_debug cdp_ru_string[RU_INDEX_MAX] = {
	{ "RU_26" },
	{ "RU_52" },
	{ "RU_106" },
	{ "RU_242" },
	{ "RU_484" },
	{ "RU_996" }
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

#define MCS_VALID 1
#define MCS_INVALID 0

#define CDP_MAX_MCS_STRING_LEN 34
/*
 * struct cdp_rate_debug
 *
 * @mcs_type: print string for a given mcs
 * @valid: valid mcs rate?
 */
struct cdp_rate_debug {
	char mcs_type[CDP_MAX_MCS_STRING_LEN];
	uint8_t valid;
};

static const struct cdp_rate_debug cdp_rate_string[DOT11_MAX][MAX_MCS] = {
	{
		{"OFDM 48 Mbps", MCS_VALID},
		{"OFDM 24 Mbps", MCS_VALID},
		{"OFDM 12 Mbps", MCS_VALID},
		{"OFDM 6 Mbps ", MCS_VALID},
		{"OFDM 54 Mbps", MCS_VALID},
		{"OFDM 36 Mbps", MCS_VALID},
		{"OFDM 18 Mbps", MCS_VALID},
		{"OFDM 9 Mbps ", MCS_VALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
	},
	{
		{"CCK 11 Mbps Long  ", MCS_VALID},
		{"CCK 5.5 Mbps Long ", MCS_VALID},
		{"CCK 2 Mbps Long   ", MCS_VALID},
		{"CCK 1 Mbps Long   ", MCS_VALID},
		{"CCK 11 Mbps Short ", MCS_VALID},
		{"CCK 5.5 Mbps Short", MCS_VALID},
		{"CCK 2 Mbps Short  ", MCS_VALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
	},
	{
		{"HT MCS 0 (BPSK 1/2)  ", MCS_VALID},
		{"HT MCS 1 (QPSK 1/2)  ", MCS_VALID},
		{"HT MCS 2 (QPSK 3/4)  ", MCS_VALID},
		{"HT MCS 3 (16-QAM 1/2)", MCS_VALID},
		{"HT MCS 4 (16-QAM 3/4)", MCS_VALID},
		{"HT MCS 5 (64-QAM 2/3)", MCS_VALID},
		{"HT MCS 6 (64-QAM 3/4)", MCS_VALID},
		{"HT MCS 7 (64-QAM 5/6)", MCS_VALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
		{"INVALID ", MCS_INVALID},
	},
	{
		{"VHT MCS 0 (BPSK 1/2)     ", MCS_VALID},
		{"VHT MCS 1 (QPSK 1/2)     ", MCS_VALID},
		{"VHT MCS 2 (QPSK 3/4)     ", MCS_VALID},
		{"VHT MCS 3 (16-QAM 1/2)   ", MCS_VALID},
		{"VHT MCS 4 (16-QAM 3/4)   ", MCS_VALID},
		{"VHT MCS 5 (64-QAM 2/3)   ", MCS_VALID},
		{"VHT MCS 6 (64-QAM 3/4)   ", MCS_VALID},
		{"VHT MCS 7 (64-QAM 5/6)   ", MCS_VALID},
		{"VHT MCS 8 (256-QAM 3/4)  ", MCS_VALID},
		{"VHT MCS 9 (256-QAM 5/6)  ", MCS_VALID},
		{"VHT MCS 10 (1024-QAM 3/4)", MCS_VALID},
		{"VHT MCS 11 (1024-QAM 5/6)", MCS_VALID},
		{"INVALID ", MCS_INVALID},
	},
	{
		{"HE MCS 0 (BPSK 1/2)     ", MCS_VALID},
		{"HE MCS 1 (QPSK 1/2)     ", MCS_VALID},
		{"HE MCS 2 (QPSK 3/4)     ", MCS_VALID},
		{"HE MCS 3 (16-QAM 1/2)   ", MCS_VALID},
		{"HE MCS 4 (16-QAM 3/4)   ", MCS_VALID},
		{"HE MCS 5 (64-QAM 2/3)   ", MCS_VALID},
		{"HE MCS 6 (64-QAM 3/4)   ", MCS_VALID},
		{"HE MCS 7 (64-QAM 5/6)   ", MCS_VALID},
		{"HE MCS 8 (256-QAM 3/4)  ", MCS_VALID},
		{"HE MCS 9 (256-QAM 5/6)  ", MCS_VALID},
		{"HE MCS 10 (1024-QAM 3/4)", MCS_VALID},
		{"HE MCS 11 (1024-QAM 5/6)", MCS_VALID},
		{"INVALID ", MCS_INVALID},
	}
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
	WDI_EVENT_HMWDS_AST_ADD_STATUS,
	WDI_EVENT_PEER_QOS_STATS,
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

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
/**
 * struct cdp_rx_ppdu_cfr_info - struct for storing ppdu info extracted from HW
 * TLVs, this will be used for CFR correlation
 *
 * @bb_captured_channel : Set by RXPCU when MACRX_FREEZE_CAPTURE_CHANNEL TLV is
 * sent to PHY, SW checks it to correlate current PPDU TLVs with uploaded
 * channel information.
 *
 * @bb_captured_timeout : Set by RxPCU to indicate channel capture condition is
 * met, but MACRX_FREEZE_CAPTURE_CHANNEL is not sent to PHY due to AST delay,
 * which means the rx_frame_falling edge to FREEZE TLV ready time exceeds
 * the threshold time defined by RXPCU register FREEZE_TLV_DELAY_CNT_THRESH.
 * Bb_captured_reason is still valid in this case.
 *
 * @bb_captured_reason : Copy capture_reason of MACRX_FREEZE_CAPTURE_CHANNEL
 * TLV to here for FW usage. Valid when bb_captured_channel or
 * bb_captured_timeout is set.
 * <enum 0 freeze_reason_TM>
 * <enum 1 freeze_reason_FTM>
 * <enum 2 freeze_reason_ACK_resp_to_TM_FTM>
 * <enum 3 freeze_reason_TA_RA_TYPE_FILTER>
 * <enum 4 freeze_reason_NDPA_NDP>
 * <enum 5 freeze_reason_ALL_PACKET>
 * <legal 0-5>
 *
 * @rx_location_info_valid: Indicates whether CFR DMA address in the PPDU TLV
 * is valid
 * <enum 0 rx_location_info_is_not_valid>
 * <enum 1 rx_location_info_is_valid>
 * <legal all>
 *
 * @chan_capture_status : capture status reported by ucode
 * a. CAPTURE_IDLE: FW has disabled "REPETITIVE_CHE_CAPTURE_CTRL"
 * b. CAPTURE_BUSY: previous PPDU’s channel capture upload DMA ongoing. (Note
 * that this upload is triggered after receiving freeze_channel_capture TLV
 * after last PPDU is rx)
 * c. CAPTURE_ACTIVE: channel capture is enabled and no previous channel
 * capture ongoing
 * d. CAPTURE_NO_BUFFER: next buffer in IPC ring not available
 *
 * @rtt_che_buffer_pointer_high8 : The high 8 bits of the 40 bits pointer to
 * external RTT channel information buffer
 *
 * @rtt_che_buffer_pointer_low32 : The low 32 bits of the 40 bits pointer to
 * external RTT channel information buffer
 *
 */

struct cdp_rx_ppdu_cfr_info {
	bool bb_captured_channel;
	bool bb_captured_timeout;
	uint8_t bb_captured_reason;
	bool rx_location_info_valid;
	uint8_t chan_capture_status;
	uint8_t rtt_che_buffer_pointer_high8;
	uint32_t rtt_che_buffer_pointer_low32;
};
#endif
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

/*
 * struct cdp_delay_tx_stats: Tx delay stats
 * @tx_swq_delay: software enqueue delay
 * @hwtx_delay: HW enque to completion delay
 */
struct cdp_delay_tx_stats {
	struct cdp_hist_stats    tx_swq_delay;
	struct cdp_hist_stats    hwtx_delay;
};

/*
 * struct cdp_delay_rx_stats: Rx delay stats
 * @to_stack_delay: To stack delay
 */
struct cdp_delay_rx_stats {
	struct cdp_hist_stats    to_stack_delay;
};

/*
 * struct cdp_delay_tid_stats: Delay tid stats
 * @tx_delay: Tx delay related stats
 * @rx_delay: Rx delay related stats
 */
struct cdp_delay_tid_stats {
	struct cdp_delay_tx_stats  tx_delay;
	struct cdp_delay_rx_stats  rx_delay;
};

/*
 * cdp_peer_ext_stats: Peer extended stats
 * @delay_stats: Per TID delay stats
 */
struct cdp_peer_ext_stats {
	struct cdp_delay_tid_stats delay_stats[CDP_MAX_DATA_TIDS]
						[CDP_MAX_TXRX_CTX];

	/*Customer can add MSDU level Tx/Rx stats */
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

#define CDP_PEER_STATS_START 0

enum cdp_peer_stats_type {
	cdp_peer_stats_min = CDP_PEER_STATS_START,

	/* Tx types */
	cdp_peer_tx_ucast = cdp_peer_stats_min,
	cdp_peer_tx_mcast,
	cdp_peer_tx_rate,
	cdp_peer_tx_last_tx_rate,
	cdp_peer_tx_inactive_time,
	cdp_peer_tx_ratecode,
	cdp_peer_tx_flags,
	cdp_peer_tx_power,

	/* Rx types */
	cdp_peer_rx_rate,
	cdp_peer_rx_last_rx_rate,
	cdp_peer_rx_ratecode,
	cdp_peer_rx_ucast,
	cdp_peer_rx_flags,
	cdp_peer_rx_avg_rssi,
	cdp_peer_stats_max,
};

/*
 * The max size of cdp_peer_stats_param_t is limited to 16 bytes.
 * If the buffer size is exceeding this size limit,
 * dp_txrx_get_peer_stats is to be used instead.
 */
typedef union cdp_peer_stats_buf {
	/* Tx types */
	struct cdp_pkt_info tx_ucast;
	struct cdp_pkt_info tx_mcast;
	uint32_t tx_rate;
	uint32_t last_tx_rate;
	uint32_t tx_inactive_time;
	uint32_t tx_flags;
	uint32_t tx_power;
	uint16_t tx_ratecode;

	/* Rx types */
	struct cdp_pkt_info rx_ucast;
	uint32_t rx_rate;
	uint32_t last_rx_rate;
	uint32_t rx_ratecode;
	uint32_t rx_flags;
	uint32_t rx_avg_rssi;
} cdp_peer_stats_param_t; /* Max union size 16 bytes */

/**
 * enum cdp_protocol_trace -  Protocols supported by per-peer protocol trace
 * @CDP_TRACE_ICMP: ICMP packets
 * @CDP_TRACE_EAP: EAPOL packets
 * @CDP_TRACE_ARP: ARP packets
 *
 * Enumeration of all protocols supported by per-peer protocol trace feature
 */
enum cdp_protocol_trace {
	CDP_TRACE_ICMP,
	CDP_TRACE_EAP,
	CDP_TRACE_ARP,
	CDP_TRACE_MAX
};

/**
 * struct protocol_trace_count - type of count on per-peer protocol trace
 * @egress_cnt: how many packets go out of host driver
 * @ingress_cnt: how many packets come into the host driver
 *
 * Type of count on per-peer protocol trace
 */
struct protocol_trace_count {
	uint16_t egress_cnt;
	uint16_t ingress_cnt;
};
/* struct cdp_tx_stats - tx stats
 * @cdp_pkt_info comp_pkt: Pkt Info for which completions were received
 * @cdp_pkt_info ucast: Unicast Packet Count
 * @cdp_pkt_info mcast: Multicast Packet Count
 * @cdp_pkt_info bcast: Broadcast Packet Count
 * @cdp_pkt_info nawds_mcast: NAWDS  Multicast Packet Count
 * @cdp_pkt_info tx_success: Successful Tx Packets
 * @nawds_mcast_drop: NAWDS  Multicast Drop Count
 * @protocol_trace_cnt: per-peer protocol counter
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
 * @pream_punct_cnt: Preamble Punctured count
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
 * @tx_success_twt: Successful Tx Packets in TWT session
 */
struct cdp_tx_stats {
	struct cdp_pkt_info comp_pkt;
	struct cdp_pkt_info ucast;
	struct cdp_pkt_info mcast;
	struct cdp_pkt_info bcast;
	struct cdp_pkt_info nawds_mcast;
#ifdef VDEV_PEER_PROTOCOL_COUNT
	struct protocol_trace_count protocol_trace_cnt[CDP_TRACE_MAX];
#endif
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
	uint32_t pream_punct_cnt;

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
	uint32_t no_ack_count[QDF_PROTO_SUBTYPE_MAX];
	struct cdp_pkt_info tx_success_twt;
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
 * @protocol_trace_cnt: per-peer protocol counters
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
 * @multipass_rx_pkt_drop: Dropped multipass rx pkt
 * @rx_mpdu_cnt: rx mpdu count per MCS rate
 * @to_stack_twt: Total packets sent up the stack in TWT session
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
#ifdef VDEV_PEER_PROTOCOL_COUNT
	struct protocol_trace_count protocol_trace_cnt[CDP_TRACE_MAX];
#endif

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
	uint32_t multipass_rx_pkt_drop;
	uint32_t rx_mpdu_cnt[MAX_MCS];
	struct cdp_pkt_info to_stack_twt;
};

/* struct cdp_tx_ingress_stats - Tx ingress Stats
 * @rcvd: Total packets received for transmission
 * @processed: Tx packets processed
 * @inspect_pkts: Total packets passed to inspect handler
 * @nawds_mcast: NAWDS  Multicast Packet Count
 * @bcast: Number of broadcast packets
 * @raw_pkt: Total Raw packets
 * @dma_map_error: DMA map error
 * @num_frags_overflow_err: msdu's nbuf count exceeds num of segemnts
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
 * @igmp_rcvd: igmp pkts received for conversion to ucast pkts
 * @igmp_ucast_converted: unicast pkts sent as part of VoW IGMP improvements
 * @dropped_pkt: Total scatter gather packets
 * @desc_na: Desc Not Available
 * @exc_desc_na: Exception desc Not Available
 * @ring_full: ring full
 * @enqueue_fail: hw enqueue fail
 * @dma_error: dma fail
 * @res_full: Resource Full: Congestion Control
 * @fail_per_pkt_vdev_id_check: Per pkt vdev id check
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
		uint32_t num_frags_overflow_err;
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

	/* IGMP Multicast Enhancement packets info */
	struct {
		uint32_t igmp_rcvd;
		uint32_t igmp_ucast_converted;
	} igmp_mcast_en;

	/* Packets dropped on the Tx side */
	struct {
		struct cdp_pkt_info dropped_pkt;
		struct cdp_pkt_info  desc_na;
		struct cdp_pkt_info  exc_desc_na;
		uint32_t ring_full;
		uint32_t enqueue_fail;
		uint32_t dma_error;
		uint32_t res_full;
		/* headroom insufficient */
		uint32_t headroom_insufficient;
		uint32_t fail_per_pkt_vdev_id_check;
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

/* struct cdp_peer_tid_stats - Per peer and per TID stats
 * @tx_avg_jitter: tx average jitter
 * @tx_avg_delay: tx average delay
 * @tx_avg_err: tx average error
 * @tx_total_success: tx total success
 * @tx_drop: tx drop
 */
struct cdp_peer_tid_stats {
#ifdef WLAN_PEER_JITTER
	uint32_t tx_avg_jitter;
	uint32_t tx_avg_delay;
	uint64_t tx_avg_err;
	uint64_t tx_total_success;
	uint64_t tx_drop;
#endif
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

/* struct cdp_interface_peer_qos_stats - interface structure for peer qos stats
 * @peer_mac: peer mac address
 * @frame_control: frame control field
 * @qos_control: qos control field
 * @frame_control_info_valid: frame_control valid
 * @qos_control_info_valid: qos_control valid
 * @vdev_id : vdev_id for the peer
 */
struct cdp_interface_peer_qos_stats {
	uint8_t  peer_mac[QDF_MAC_ADDR_SIZE];
	uint16_t frame_control;
	uint16_t qos_control;
	uint8_t  frame_control_info_valid;
	uint8_t  qos_control_info_valid;
	uint8_t  vdev_id;
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

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
/*
 * mac_freeze_capture_reason - capture reason counters
 * @FREEZE_REASON_TM: When m_directed_ftm is enabled, this CFR data is
 * captured for a Timing Measurement (TM) frame.
 * @FREEZE_REASON_FTM: When m_directed_ftm is enabled, this CFR data is
 * captured for a Fine Timing Measurement (FTM) frame.
 * @FREEZE_REASON_ACK_RESP_TO_TM_FTM: When m_all_ftm_ack is enabled, this CFR
 * data is captured for an ACK received for the FTM/TM frame sent to a station.
 * @FREEZE_REASON_TA_RA_TYPE_FILTER: When m_ta_ra_filter is enabled, this CFR
 * data is captured for a PPDU received,since the CFR TA_RA filter is met.
 * @FREEZE_REASON_NDPA_NDP: When m_ndpa_ndp_directed(or)m_ndpa_ndp_all is
 * enabled, this CFR data is captured for an NDP frame received.
 * @FREEZE_REASON_ALL_PACKET: When m_all_packet is enabled, this CFR data is
 * captured for an incoming PPDU.
 */
enum mac_freeze_capture_reason {
	FREEZE_REASON_TM = 0,
	FREEZE_REASON_FTM,
	FREEZE_REASON_ACK_RESP_TO_TM_FTM,
	FREEZE_REASON_TA_RA_TYPE_FILTER,
	FREEZE_REASON_NDPA_NDP,
	FREEZE_REASON_ALL_PACKET,
	FREEZE_REASON_MAX,
};

/*
 * chan_capture_status: capture status counters
 * @CAPTURE_IDLE: CFR data is not captured, since VCSR setting for CFR/RCC is
 * not enabled.
 * @CAPTURE_BUSY: CFR data is not available, since previous channel
 * upload is in progress
 * @CAPTURE_ACTIVE: CFR data is captured in HW registers
 * @CAPTURE_NO_BUFFER: CFR data is not captured, since no buffer is available
 * in IPC ring to DMA CFR data
 */
enum chan_capture_status {
	CAPTURE_IDLE = 0,
	CAPTURE_BUSY,
	CAPTURE_ACTIVE,
	CAPTURE_NO_BUFFER,
	CAPTURE_MAX,
};

/* struct cdp_cfr_rcc_stats - CFR RCC debug statistics
 * @bb_captured_channel_cnt: No. of PPDUs for which MAC sent Freeze TLV to PHY
 * @bb_captured_timeout_cnt: No. of PPDUs for which CFR filter criteria matched
 * but MAC did not send Freeze TLV to PHY as time exceeded freeze tlv delay
 * count threshold
 * @rx_loc_info_valid_cnt: No. of PPDUs for which PHY could find a valid buffer
 * in ucode IPC ring
 * @chan_capture_status[]: capture status counters
 *	[0] - No. of PPDUs with capture status CAPTURE_IDLE
 *	[1] - No. of PPDUs with capture status CAPTURE_BUSY
 *	[2] - No. of PPDUs with capture status CAPTURE_ACTIVE
 *	[3] - No. of PPDUs with capture status CAPTURE_NO_BUFFER
 * @reason_cnt[]: capture reason counters
 *	[0] - No. PPDUs filtered due to freeze_reason_TM
 *	[1] - No. PPDUs filtered due to freeze_reason_FTM
 *	[2] - No. PPDUs filtered due to freeze_reason_ACK_resp_to_TM_FTM
 *	[3] - No. PPDUs filtered due to freeze_reason_TA_RA_TYPE_FILTER
 *	[4] - No. PPDUs filtered due to freeze_reason_NDPA_NDP
 *	[5] - No. PPDUs filtered due to freeze_reason_ALL_PACKET
 */
struct cdp_cfr_rcc_stats {
	uint64_t bb_captured_channel_cnt;
	uint64_t bb_captured_timeout_cnt;
	uint64_t rx_loc_info_valid_cnt;
	uint64_t chan_capture_status[CAPTURE_MAX];
	uint64_t reason_cnt[FREEZE_REASON_MAX];
};
#else
struct cdp_cfr_rcc_stats {
};
#endif
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
 * @frag_alloc_fail: frag alloc failed
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
 * @tx_ppdu_proc: stats counter for tx ppdu processed
 * @ack_ba_comes_twice: stats counter for ack_ba_comes twice
 * @ppdu_drop: stats counter for ppdu_desc drop once threshold reached
 * @ppdu_wrap_drop: stats counter for ppdu desc drop on wrap around
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
		uint32_t frag_alloc_fail;
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
	struct cdp_cfr_rcc_stats rcc;

	uint64_t tx_ppdu_proc;
	uint64_t ack_ba_comes_twice;
	uint64_t ppdu_drop;
	uint64_t ppdu_wrap_drop;

	struct {
		uint64_t num_bufs_consumed;
		uint64_t num_pool_bufs_replenish;
		uint64_t num_bufs_alloc_success;
	} rx_buffer_pool;
};

/* struct cdp_peer_hmwds_ast_add_status - hmwds peer ast add status
 * @vdev_id: vdev id
 * @status: ast add status
 * @peer_mac: peer mac address
 * @ast_mac: ast node mac address
 */
struct cdp_peer_hmwds_ast_add_status {
	uint32_t vdev_id;
	uint32_t status;
	uint8_t  peer_mac[QDF_MAC_ADDR_SIZE];
	uint8_t  ast_mac[QDF_MAC_ADDR_SIZE];
};

enum cdp_soc_param_t {
	DP_SOC_PARAM_MSDU_EXCEPTION_DESC,
	DP_SOC_PARAM_CMEM_FSE_SUPPORT,
	DP_SOC_PARAM_MAX,
};

#ifdef QCA_ENH_V3_STATS_SUPPORT
/*
 * Enumeration of PDEV Configuration parameter
 */
enum _dp_param_t {
	DP_PARAM_MSDU_TTL,
	DP_PARAM_TOTAL_Q_SIZE_RANGE0,
	DP_PARAM_TOTAL_Q_SIZE_RANGE1,
	DP_PARAM_TOTAL_Q_SIZE_RANGE2,
	DP_PARAM_TOTAL_Q_SIZE_RANGE3,
	DP_PARAM_VIDEO_DELAY_STATS_FC,
	DP_PARAM_QFLUSHINTERVAL,
	DP_PARAM_TOTAL_Q_SIZE,
	DP_PARAM_MIN_THRESHOLD,
	DP_PARAM_MAX_Q_LIMIT,
	DP_PARAM_MIN_Q_LIMIT,
	DP_PARAM_CONG_CTRL_TIMER_INTV,
	DP_PARAM_STATS_TIMER_INTV,
	DP_PARAM_ROTTING_TIMER_INTV,
	DP_PARAM_LATENCY_PROFILE,
	DP_PARAM_HOSTQ_DUMP,
	DP_PARAM_TIDQ_MAP,
	DP_PARAM_VIDEO_STATS_FC,
	DP_PARAM_STATS_FC,

	DP_PARAM_MAX,
};
#endif
/* Bitmasks for stats that can block */
#define EXT_TXRX_FW_STATS		0x0001
#endif

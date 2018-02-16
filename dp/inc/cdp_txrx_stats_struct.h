/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_stats_struct.h
 * @brief Define the host data path stats API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_STATS_STRUCT_H_
#define _CDP_TXRX_STATS_STRUCT_H_
#ifndef CONFIG_WIN
#include <wlan_defs.h>
#endif

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

/* WME stream classes */
#define WME_AC_BE    0    /* best effort */
#define WME_AC_BK    1    /* background */
#define WME_AC_VI    2    /* video */
#define WME_AC_VO    3    /* voice */
#define WME_AC_MAX   4    /* MAX AC Value */

#define CDP_MAX_RX_RINGS 4

/*
 * Number of TLVs sent by FW. Needs to reflect
 * HTT_PPDU_STATS_MAX_TAG declared in FW
 */
#define CDP_PPDU_STATS_MAX_TAG 14

/* Different Packet Types */
enum cdp_packet_type {
	DOT11_A = 0,
	DOT11_B = 1,
	DOT11_N = 2,
	DOT11_AC = 3,
	DOT11_AX = 4,
	DOT11_MAX = 5,
};

/* packet info */
struct cdp_pkt_info {
	/*no of packets*/
	uint32_t num;
	/* total no of bytes */
	uint64_t bytes;
};

/* Tx  Stats */
struct cdp_tx_stats {
	/* Pkt Info for which completions were received */
	struct cdp_pkt_info comp_pkt;
	/* Unicast Packet Count */
	struct cdp_pkt_info ucast;
	/* Multicast Packet Count */
	struct cdp_pkt_info mcast;
	/* Broadcast Packet Count*/
	struct cdp_pkt_info bcast;
	/*NAWDS  Multicast Packet Count */
	struct cdp_pkt_info nawds_mcast;
	/*NAWDS  Multicast Drop Count */
	uint32_t nawds_mcast_drop;
	/* Successful Tx Packets */
	struct cdp_pkt_info tx_success;
	/* Total Tx failure */
	uint32_t tx_failed;
	/* Total Packets as ofdma*/
	uint32_t ofdma;
	/* Packets in STBC */
	uint32_t stbc;
	/* Packets in LDPC */
	uint32_t ldpc;
	/* Packet retries */
	uint32_t retries;
	/* Number of MSDUs with no MSDU level aggregation */
	uint32_t non_amsdu_cnt;
	/* Number of MSDUs part of AMSDU*/
	uint32_t amsdu_cnt;
	/* Tx Rate */
	uint32_t tx_rate;

	/* RSSI of last packet */
	uint32_t last_ack_rssi;

	/* Packet Type */
	struct {
		/* MCS Count */
		uint32_t mcs_count[MAX_MCS];
	} pkt_type[DOT11_MAX];

	/* SGI count */
	uint32_t sgi_count[MAX_GI];

	/* Packet count for different num_spatial_stream values */
	uint32_t nss[SS_COUNT];

	/* Packet Count for different bandwidths */
	uint32_t bw[MAX_BW];

	/* Wireless Multimedia type Count */
	uint32_t wme_ac_type[WME_AC_MAX];

	/* Wireless Multimedia type Count */
	uint32_t excess_retries_per_ac[WME_AC_MAX];

	/* Packets dropped on the Tx side */
	struct {
		/* Discarded by firmware */
		uint32_t fw_rem;
		/* firmware_discard_untransmitted */
		uint32_t fw_rem_notx;
		/* firmware_discard_transmitted */
		uint32_t fw_rem_tx;
		/* aged out in mpdu/msdu queues*/
		uint32_t age_out;
		/* discarded by firmware reason 1 */
		uint32_t fw_reason1;
		/* discarded by firmware reason 2 */
		uint32_t fw_reason2;
		/* discarded by firmware reason 3 */
		uint32_t fw_reason3;
	} dropped;
};

/* Rx Level Stats */
struct cdp_rx_stats {
	/* Total packets sent up the stack */
	struct cdp_pkt_info to_stack;
	/* Packets received on the reo ring */
	struct cdp_pkt_info rcvd_reo[CDP_MAX_RX_RINGS];
	/* Total unicast packets */
	struct cdp_pkt_info unicast;
	/* Total multicast packets */
	struct cdp_pkt_info multicast;
	/* Broadcast Packet Count*/
	struct cdp_pkt_info bcast;
	/* Raw Pakets received */
	struct cdp_pkt_info raw;
	/* Total multicast packets */
	struct cdp_pkt_info nawds_mcast_drop;

	struct {
	/* Intra BSS packets received */
	struct cdp_pkt_info pkts;
	struct cdp_pkt_info fail;
	} intra_bss;

	/* Errors */
	struct {
		/* Rx MIC errors */
		uint32_t mic_err;
		/* Rx Decryption Errors */
		uint32_t decrypt_err;
	} err;

	/* Wireless Multimedia type Count */
	uint32_t wme_ac_type[WME_AC_MAX];
	/* Reception type os packets */
	uint32_t reception_type[MAX_RECEPTION_TYPES];
	/* Packet Type */
	struct {
		/* MCS Count */
		uint32_t mcs_count[MAX_MCS];
	} pkt_type[DOT11_MAX];
	/* SGI count */
	uint32_t sgi_count[MAX_GI];
	/* Packet count in spatiel Streams */
	uint32_t nss[SS_COUNT];
	/* Packet Count in different bandwidths */
	uint32_t bw[MAX_BW];
	/*  Number of MSDUs with no MPDU level aggregation */
	uint32_t non_ampdu_cnt;
	/* Number of MSDUs part of AMSPU */
	uint32_t ampdu_cnt;
	/* Number of MSDUs with no MSDU level aggregation */
	uint32_t non_amsdu_cnt;
	/* Number of MSDUs part of AMSDU*/
	uint32_t amsdu_cnt;
	/* Number of bar received */
	uint32_t bar_recv_cnt;
	/* RSSI of received signal */
	uint32_t rssi;
	/*Rx rate */
	uint32_t rx_rate;
};

/* Tx ingress Stats */
struct cdp_tx_ingress_stats {
	/* Total packets received for transmission */
	struct cdp_pkt_info rcvd;
	/* Tx packets processed*/
	struct cdp_pkt_info processed;
	/* Total packets passed Reinject handler */
	struct cdp_pkt_info reinject_pkts;
	/*  Total packets passed to inspect handler */
	struct cdp_pkt_info inspect_pkts;
	/*NAWDS  Multicast Packet Count */
	struct cdp_pkt_info nawds_mcast;
	/* Number of broadcast packets */
	struct cdp_pkt_info bcast;

	struct {
		/* Total Raw packets */
		struct cdp_pkt_info raw_pkt;
		/* DMA map error */
		uint32_t dma_map_error;
	} raw;

	/* TSO packets info */
	struct {
		/* No of segments in TSO packets */
		uint32_t num_seg;
		/* total no of TSO packets */
		struct cdp_pkt_info tso_pkt;
		/* TSO packets dropped by host */
		uint32_t dropped_host;
		/* TSO packets dropped by target */
		uint32_t dropped_target;
	} tso;

	/* Scatter Gather packet info */
	struct {
		/* Total scatter gather packets */
		struct cdp_pkt_info sg_pkt;
		/* SG packets dropped by host */
		uint32_t dropped_host;
		/* SG packets dropped by target */
		uint32_t dropped_target;
		/* Dma map error */
		uint32_t dma_map_error;
	} sg;

	/* Multicast Enhancement packets info */
	struct {
		/* total no of multicast conversion packets */
		struct cdp_pkt_info mcast_pkt;
		/* packets dropped due to map error */
		uint32_t dropped_map_error;
		/* packets dropped due to self Mac address */
		uint32_t dropped_self_mac;
		/* Packets dropped due to send fail */
		uint32_t dropped_send_fail;
		/* total unicast packets transmitted */
		uint32_t ucast;
		/* Segment allocation failure */
		uint32_t fail_seg_alloc;
		/* NBUF clone failure */
		uint32_t clone_fail;
	} mcast_en;

	/* Packets dropped on the Tx side */
	struct {
		/* Total scatter gather packets */
		struct cdp_pkt_info dropped_pkt;
		/* Desc Not Available */
		uint32_t desc_na;
		/* Ring Full */
		uint32_t ring_full;
		/* Hwenqueue failed */
		uint32_t enqueue_fail;
		/* DMA failed */
		uint32_t dma_error;
		/* Resource Full: Congestion Control */
		uint32_t res_full;
	} dropped;

	/* Mesh packets info */
	struct {
		/* packets sent to fw */
		uint32_t exception_fw;
		/* packets completions received from fw */
		uint32_t completion_fw;
	} mesh;

	/*Number of packets classified by CCE*/
	uint32_t cce_classified;

	/*Number of raw packets classified by CCE*/
	uint32_t cce_classified_raw;
};

struct cdp_vdev_stats {
	/* Tx ingress stats */
	struct cdp_tx_ingress_stats tx_i;
	/* CDP Tx Stats */
	struct cdp_tx_stats tx;
	/* CDP Rx Stats */
	struct cdp_rx_stats rx;
};


struct cdp_peer_stats {
	/* CDP Tx Stats */
	struct cdp_tx_stats tx;
	/* CDP Rx Stats */
	struct cdp_rx_stats rx;
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

struct cdp_pdev_stats {
	/* packets dropped on rx */
	struct {
		/* packets dropped because nsdu_done bit not set */
		uint32_t msdu_not_done;
		/* Multicast Echo check */
		uint32_t mec;
		/* Mesh Filtered packets */
		uint32_t mesh_filter;
	} dropped;

	struct {
		/* total packets replnished */
		struct cdp_pkt_info pkts;
		/* rxdma errors */
		uint32_t rxdma_err;
		/* nbuf alloc failed */
		uint32_t nbuf_alloc_fail;
		/* Mapping failure */
		uint32_t map_err;
		/* x86 failures */
		uint32_t x86_fail;
		/* low threshold interrupts */
		uint32_t low_thresh_intrs;
	} replenish;

	/* Rx Raw Packets */
	uint32_t rx_raw_pkts;
	/* Mesh Rx Stats Alloc fail */
	uint32_t mesh_mem_alloc;

	/* Rx errors */
	struct {
		/* desc alloc failed errors */
		uint32_t desc_alloc_fail;
	} err;

	/* buffers added back in freelist */
	uint32_t buf_freelist;
	/* Tx Ingress stats */
	struct cdp_tx_ingress_stats tx_i;
	/* CDP Tx Stats */
	struct cdp_tx_stats tx;
	/* CDP Rx Stats */
	struct cdp_rx_stats rx;
	/* Number of Tx completions per interrupt */
	struct cdp_hist_tx_comp tx_comp_histogram;
	/* Number of Rx ring descriptors reaped per interrupt */
	struct cdp_hist_rx_ind rx_ind_histogram;
	uint64_t ppdu_stats_counter[CDP_PPDU_STATS_MAX_TAG];
};

#ifndef BIG_ENDIAN_HOST
typedef struct {
	uint64_t pkts;
	uint64_t bytes;
} ol_txrx_stats_elem;
#else
struct ol_txrx_elem_t {
	uint64_t pkts;
	uint64_t bytes;
};
typedef struct ol_txrx_elem_t ol_txrx_stats_elem;
#endif

#ifndef CONFIG_MCL
/**
 * @brief data stats published by the host txrx layer
 */
struct ol_txrx_stats {
	struct {
		/* MSDUs received from the stack */
		ol_txrx_stats_elem from_stack;
		/* MSDUs successfully sent across the WLAN */
		ol_txrx_stats_elem delivered;
		struct {
			/* MSDUs that the host did not accept */
			ol_txrx_stats_elem host_reject;
			/* MSDUs which could not be downloaded to the target */
			ol_txrx_stats_elem download_fail;
			/*
			 * MSDUs which the target discarded
			 * (lack of mem or old age)
			 */
			ol_txrx_stats_elem target_discard;
			/*
			 * MSDUs which the target sent but couldn't get
			 * an ack for
			 */
			ol_txrx_stats_elem no_ack;
			/* MSDUs dropped in NSS-FW */
			ol_txrx_stats_elem nss_ol_dropped;
		} dropped;
		u_int32_t desc_in_use;
		u_int32_t desc_alloc_fails;
		u_int32_t ce_ring_full;
		u_int32_t dma_map_error;
		/* MSDUs given to the txrx layer by the management stack */
		ol_txrx_stats_elem mgmt;
		struct {
			/* TSO applied jumbo packets received from NW Stack */
			ol_txrx_stats_elem tso_pkts;
			/* Non - TSO packets */
			ol_txrx_stats_elem non_tso_pkts;
			/* TSO packets : Dropped during TCP segmentation*/
			ol_txrx_stats_elem tso_dropped;
			/* TSO Descriptors */
			u_int32_t tso_desc_cnt;
		} tso;

		struct {
			/* TSO applied jumbo packets received from NW Stack */
			ol_txrx_stats_elem sg_pkts;
			/* Non - TSO packets */
			ol_txrx_stats_elem non_sg_pkts;
			/* TSO packets : Dropped during TCP segmentation*/
			ol_txrx_stats_elem sg_dropped;
			/* TSO Descriptors */
			u_int32_t sg_desc_cnt;
		} sg;
		struct {
			/* packets enqueued for flow control */
			u_int32_t fl_ctrl_enqueue;
			/* packets discarded for flow control is full */
			u_int32_t fl_ctrl_discard;
			/* packets sent to CE without flow control */
			u_int32_t fl_ctrl_avoid;
		} fl_ctrl;
	} tx;
	struct {
		/* MSDUs given to the OS shim */
		ol_txrx_stats_elem delivered;
		/* MSDUs forwarded from the rx path to the tx path */
		ol_txrx_stats_elem forwarded;
		/* MSDUs in which ipv4 chksum error detected by HW */
		ol_txrx_stats_elem ipv4_cksum_err;
		/* MSDUs in which tcp chksum error detected by HW */
		ol_txrx_stats_elem tcp_ipv4_cksum_err;
		/* MSDUs in which udp chksum error detected by HW */
		ol_txrx_stats_elem udp_ipv4_cksum_err;
		/* MSDUs in which tcp V6 chksum error detected by HW */
		ol_txrx_stats_elem tcp_ipv6_cksum_err;
		/* MSDUs in which UDP V6 chksum error detected by HW */
		ol_txrx_stats_elem udp_ipv6_cksum_err;
	} rx;
	struct {
		/* Number of mcast received for conversion */
		u_int32_t num_me_rcvd;
		/* Number of unicast sent as part of mcast conversion */
		u_int32_t num_me_ucast;
		/* Number of multicast frames dropped due to dma_map failure */
		u_int32_t num_me_dropped_m;
		/*
		 * Number of multicast frames dropped due to allocation
		 * failure
		 */
		u_int32_t num_me_dropped_a;
		/* Number of multicast frames dropped due to internal failure */
		u_int32_t num_me_dropped_i;
		/* Number of me buf currently in use */
		u_int32_t num_me_buf;
		/* Number of me buf frames to self mac address  */
		u_int32_t num_me_dropped_s;
		/* Number of me buf in use in non pool based allocation*/
		u_int32_t num_me_nonpool;
		/* Number of me buf allocated using non pool based allocation*/
		u_int32_t num_me_nonpool_count;
	} mcast_enhance;
};

struct ol_ath_dbg_rx_rssi {
	A_UINT8     rx_rssi_pri20;
	A_UINT8     rx_rssi_sec20;
	A_UINT8     rx_rssi_sec40;
	A_UINT8     rx_rssi_sec80;
};

struct ol_ath_radiostats {
	A_UINT64    tx_beacon;
	A_UINT32    be_nobuf;
	A_UINT32    tx_buf_count;
	A_UINT32    tx_packets;
	A_UINT32    rx_packets;
	A_INT32     tx_mgmt;
	A_UINT32    tx_num_data;
	A_UINT32    rx_num_data;
	A_INT32     rx_mgmt;
	A_UINT32    rx_num_mgmt;
	A_UINT32    rx_num_ctl;
	A_UINT32    tx_rssi;
	A_UINT32    tx_mcs[10];
	A_UINT32    rx_mcs[10];
	A_UINT32    rx_rssi_comb;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain0;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain1;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain2;
	struct      ol_ath_dbg_rx_rssi rx_rssi_chain3;
	A_UINT64    rx_bytes;
	A_UINT64    tx_bytes;
	A_UINT32    tx_compaggr;
	A_UINT32    rx_aggr;
	A_UINT32    tx_bawadv;
	A_UINT32    tx_compunaggr;
	A_UINT32    rx_overrun;
	A_UINT32    rx_badcrypt;
	A_UINT32    rx_badmic;
	A_UINT32    rx_crcerr;
	A_UINT32    rx_phyerr;
	A_UINT32    ackRcvBad;
	A_UINT32    rtsBad;
	A_UINT32    rtsGood;
	A_UINT32    fcsBad;
	A_UINT32    noBeacons;
	A_UINT32    mib_int_count;
	A_UINT32    rx_looplimit_start;
	A_UINT32    rx_looplimit_end;
	A_UINT8     ap_stats_tx_cal_enable;
	A_UINT32    tgt_asserts;
	A_INT16     chan_nf;
	A_UINT32    rx_last_msdu_unset_cnt;
	A_INT16     chan_nf_sec80;
	A_UINT64    wmi_tx_mgmt;
	A_UINT64    wmi_tx_mgmt_completions;
	A_UINT32    wmi_tx_mgmt_completion_err;
	A_UINT32    peer_delete_req;
	A_UINT32    peer_delete_resp;
	A_UINT32    rx_mgmt_rssi_drop;
	A_UINT32    tx_retries;
	A_UINT32    rx_data_bytes;
	A_UINT32    tx_frame_count;
	A_UINT32    rx_frame_count;
	A_UINT32    rx_clear_count;
	A_UINT32    cycle_count;
	A_UINT32    phy_err_count;
	A_UINT32    chan_tx_pwr;
};

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
	/* OBSS RSSI threshold for 20/40 coexistance */
	OL_ATH_PARAM_OBSS_RSSI_THRESHOLD,
	/* Link/node RX RSSI threshold  for 20/40 coexistance */
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
};

/* Enumeration of PDEV Configuration parameter */
enum _ol_hal_param_t {
	OL_HAL_CONFIG_DMA_BEACON_RESPONSE_TIME         = 0
};
#endif

/* Bitmasks for stats that can block */
#define EXT_TXRX_FW_STATS		0x0001
#endif

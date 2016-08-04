/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef WMA_H
#define WMA_H

#include "a_types.h"
#include "qdf_types.h"
#include "osapi_linux.h"
#include "htc_packet.h"
#include "i_qdf_event.h"
#include "wmi_services.h"
#include "wmi_unified.h"
#include "wmi_version.h"
#include "qdf_types.h"
#include "cfg_api.h"
#include "qdf_status.h"
#include "cds_sched.h"
#include "sir_mac_prot_def.h"
#include "wma_types.h"
#include "ol_txrx_types.h"
#include <linux/workqueue.h>
#include "utils_api.h"
#include "lim_types.h"
#include "wmi_unified_api.h"
#include "cdp_txrx_cmn.h"
#include "ol_defines.h"
#include "dbglog.h"

/* Platform specific configuration for max. no. of fragments */
#define QCA_OL_11AC_TX_MAX_FRAGS            2

/* Private */

#define WMA_READY_EVENTID_TIMEOUT          6000
#define WMA_SERVICE_READY_EXT_TIMEOUT      6000
#define WMA_TGT_SUSPEND_COMPLETE_TIMEOUT   6000
#define WMA_WAKE_LOCK_TIMEOUT              1000
#define WMA_RESUME_TIMEOUT                 6000
#define MAX_MEM_CHUNKS                     32

#define WMA_CRASH_INJECT_TIMEOUT           5000

/* MAC ID to PDEV ID mapping is as given below
 * MAC_ID           PDEV_ID
 * 0                    1
 * 1                    2
 * SOC Level            WMI_PDEV_ID_SOC
 */
#define WMA_MAC_TO_PDEV_MAP(x) ((x) + (1))
#define WMA_PDEV_TO_MAC_MAP(x) ((x) - (1))

/* In prima 12 HW stations are supported including BCAST STA(staId 0)
 * and SELF STA(staId 1) so total ASSOC stations which can connect to Prima
 * SoftAP = 12 - 1(Self STa) - 1(Bcast Sta) = 10 Stations.
 */

#ifdef WLAN_SOFTAP_VSTA_FEATURE
#define WMA_MAX_SUPPORTED_STAS    38
#else
#define WMA_MAX_SUPPORTED_STAS    12
#endif
#define WMA_MAX_SUPPORTED_BSS     5

#define FRAGMENT_SIZE 3072

#define WMA_INVALID_VDEV_ID                             0xFF
#define MAX_MEM_CHUNKS                                  32
#define WMA_MAX_VDEV_SIZE                               20
#define WMA_VDEV_TBL_ENTRY_ADD                          1
#define WMA_VDEV_TBL_ENTRY_DEL                          0

/* 11A/G channel boundary */
#define WMA_11A_CHANNEL_BEGIN           34
#define WMA_11A_CHANNEL_END             165
#define WMA_11G_CHANNEL_BEGIN           1
#define WMA_11G_CHANNEL_END             14

#define WMA_11P_CHANNEL_BEGIN           (170)
#define WMA_11P_CHANNEL_END             (184)

#define WMA_LOGD(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_DEBUG, ## args)
#define WMA_LOGI(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_INFO, ## args)
#define WMA_LOGW(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_WARN, ## args)
#define WMA_LOGE(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_ERROR, ## args)
#define WMA_LOGP(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_FATAL, ## args)

#define WMA_DEBUG_ALWAYS

#ifdef WMA_DEBUG_ALWAYS
#define WMA_LOGA(args ...) \
	QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_FATAL, ## args)
#else
#define WMA_LOGA(args ...)
#endif

#define     ALIGNED_WORD_SIZE       4
#define WLAN_HAL_MSG_TYPE_MAX_ENUM_SIZE    0x7FFF
#define WMA_WILDCARD_PDEV_ID 0x0

/* Prefix used by scan req ids generated on the host */
#define WMA_HOST_SCAN_REQID_PREFIX       0xA000
/* Prefix used by roam scan req ids generated on the host */
#define WMA_HOST_ROAM_SCAN_REQID_PREFIX  0xA800
/* Prefix used by scan requestor id on host */
#define WMA_HOST_SCAN_REQUESTOR_ID_PREFIX 0xA000

#define WMA_HW_DEF_SCAN_MAX_DURATION      30000 /* 30 secs */

/* Max offchannel duration */
#define WMA_BURST_SCAN_MAX_NUM_OFFCHANNELS  (3)
#define WMA_SCAN_NPROBES_DEFAULT            (2)
#define WMA_SCAN_IDLE_TIME_DEFAULT          (25)
#define WMA_P2P_SCAN_MAX_BURST_DURATION     (180)
#define WMA_CTS_DURATION_MS_MAX             (32)
#define WMA_GO_MIN_ACTIVE_SCAN_BURST_DURATION   (40)
#define WMA_GO_MAX_ACTIVE_SCAN_BURST_DURATION   (120)
#define WMA_DWELL_TIME_PASSIVE_DEFAULT          (110)
#define WMA_DWELL_TIME_PROBE_TIME_MAP_SIZE      (11)
#define WMA_3PORT_CONC_SCAN_MAX_BURST_DURATION  (25)

#define WMA_SEC_TO_USEC                     (1000000)

#define BEACON_TX_BUFFER_SIZE               (512)

/* WMA_ETHER_TYPE_OFFSET = sa(6) + da(6) */
#define WMA_ETHER_TYPE_OFFSET (6 + 6)
/* WMA_ICMP_V6_HEADER_OFFSET = sa(6) + da(6) + eth_type(2) + icmp_v6_hdr(6)*/
#define WMA_ICMP_V6_HEADER_OFFSET (6 + 6 + 2 + 6)
/* WMA_ICMP_V6_TYPE_OFFSET = sa(6) + da(6) + eth_type(2) + 40 */
#define WMA_ICMP_V6_TYPE_OFFSET (6 + 6 + 2 + 40)
#define WMA_ICMP_V6_HEADER_TYPE (0x3A)
#define WMA_ICMP_V6_RA_TYPE (0x86)
#define WMA_ICMP_V6_NS_TYPE (0x87)
#define WMA_ICMP_V6_NA_TYPE (0x88)
#define WMA_BCAST_MAC_ADDR (0xFF)
#define WMA_MCAST_IPV4_MAC_ADDR (0x01)
#define WMA_MCAST_IPV6_MAC_ADDR (0x33)


/* Roaming default values
 * All time and period values are in milliseconds.
 * All rssi values are in dB except for WMA_NOISE_FLOOR_DBM_DEFAULT.
 */

#define WMA_ROAM_SCAN_CHANNEL_SWITCH_TIME    (4)
#define WMA_NOISE_FLOOR_DBM_DEFAULT          (-96)
#define WMA_ROAM_RSSI_DIFF_DEFAULT           (5)
#define WMA_ROAM_DWELL_TIME_ACTIVE_DEFAULT   (100)
#define WMA_ROAM_DWELL_TIME_PASSIVE_DEFAULT  (110)
#define WMA_ROAM_MIN_REST_TIME_DEFAULT       (50)
#define WMA_ROAM_MAX_REST_TIME_DEFAULT       (500)
#define WMA_ROAM_LOW_RSSI_TRIGGER_DEFAULT    (20)
#define WMA_ROAM_LOW_RSSI_TRIGGER_VERYLOW    (10)
#define WMA_ROAM_BEACON_WEIGHT_DEFAULT       (14)
#define WMA_ROAM_OPP_SCAN_PERIOD_DEFAULT     (120000)
#define WMA_ROAM_OPP_SCAN_AGING_PERIOD_DEFAULT (WMA_ROAM_OPP_SCAN_PERIOD_DEFAULT * 5)
#define WMA_ROAM_BMISS_FIRST_BCNT_DEFAULT    (10)
#define WMA_ROAM_BMISS_FINAL_BCNT_DEFAULT    (10)
#define WMA_ROAM_BMISS_FIRST_BCNT_DEFAULT_P2P (15)
#define WMA_ROAM_BMISS_FINAL_BCNT_DEFAULT_P2P (45)

#define WMA_INVALID_KEY_IDX     0xff
#define WMA_DFS_RADAR_FOUND   1

#define WMA_MAX_RF_CHAINS(x)    ((1 << x) - 1)
#define WMA_MIN_RF_CHAINS               (1)

#ifdef FEATURE_WLAN_EXTSCAN
#define WMA_MAX_EXTSCAN_MSG_SIZE        1536
#define WMA_EXTSCAN_REST_TIME           100
#define WMA_EXTSCAN_MAX_SCAN_TIME       50000
#define WMA_EXTSCAN_BURST_DURATION      150
#endif

#define WMA_BCN_BUF_MAX_SIZE 2500
#define WMA_NOA_IE_SIZE(num_desc) (2 + (13 * (num_desc)))
#define WMA_MAX_NOA_DESCRIPTORS 4

#define WMA_TIM_SUPPORTED_PVB_LENGTH ((HAL_NUM_STA / 8) + 1)

#define WMA_WOW_PTRN_MASK_VALID     0xFF
#define WMA_NUM_BITS_IN_BYTE           8

#define WMA_AP_WOW_DEFAULT_PTRN_MAX    4
#define WMA_STA_WOW_DEFAULT_PTRN_MAX   4

#define WMA_BSS_STATUS_STARTED 0x1
#define WMA_BSS_STATUS_STOPPED 0x2

#define WMA_TARGET_REQ_TYPE_VDEV_START 0x1
#define WMA_TARGET_REQ_TYPE_VDEV_STOP  0x2
#define WMA_TARGET_REQ_TYPE_VDEV_DEL   0x3

#define WMA_PEER_ASSOC_CNF_START 0x01
#define WMA_PEER_ASSOC_TIMEOUT (3000) /* 3 seconds */

#define WMA_DELETE_STA_RSP_START 0x02
#define WMA_DELETE_STA_TIMEOUT (6000) /* 6 seconds */

#define WMA_DEL_P2P_SELF_STA_RSP_START 0x03

#define WMA_VDEV_START_REQUEST_TIMEOUT (3000)   /* 3 seconds */
#define WMA_VDEV_STOP_REQUEST_TIMEOUT  (3000)   /* 3 seconds */

#define WMA_TGT_INVALID_SNR 0x127

#define WMA_TX_Q_RECHECK_TIMER_WAIT      2      /* 2 ms */
#define WMA_TX_Q_RECHECK_TIMER_MAX_WAIT  20     /* 20 ms */
#define WMA_MAX_NUM_ARGS 8

#define WMA_SMPS_MASK_LOWER_16BITS 0xFF
#define WMA_SMPS_MASK_UPPER_3BITS 0x7
#define WMA_SMPS_PARAM_VALUE_S 29

#define WMA_MAX_SCAN_ID        0x00FF

/*
 * Setting the Tx Comp Timeout to 1 secs.
 * TODO: Need to Revist the Timing
 */
#define WMA_TX_FRAME_COMPLETE_TIMEOUT  1000
#define WMA_TX_FRAME_BUFFER_NO_FREE    0
#define WMA_TX_FRAME_BUFFER_FREE       1


/* Default InActivity Time is 200 ms */
#define POWERSAVE_DEFAULT_INACTIVITY_TIME 200

/* Default Listen Interval */
#define POWERSAVE_DEFAULT_LISTEN_INTERVAL 1

/*
 * TODO: Add WMI_CMD_ID_MAX as part of WMI_CMD_ID
 * instead of assigning it to the last valid wmi
 * cmd+1 to avoid updating this when a command is
 * added/deleted.
 */
#define WMI_CMDID_MAX (WMI_TXBF_CMDID + 1)

#define WMA_NLO_FREQ_THRESH          1000       /* in MHz */
#define WMA_SEC_TO_MSEC(sec)         (sec * 1000)       /* sec to msec */
#define WMA_MSEC_TO_USEC(msec)	     (msec * 1000) /* msec to usec */

/* Default rssi threshold defined in CFG80211 */
#define WMA_RSSI_THOLD_DEFAULT   -300

#ifdef FEATURE_WLAN_SCAN_PNO
#define WMA_PNO_MATCH_WAKE_LOCK_TIMEOUT         (5 * 1000)     /* in msec */
#define WMA_PNO_SCAN_COMPLETE_WAKE_LOCK_TIMEOUT (2 * 1000)     /* in msec */
#endif
#define WMA_AUTH_REQ_RECV_WAKE_LOCK_TIMEOUT     (5 * 1000)     /* in msec */
#define WMA_ASSOC_REQ_RECV_WAKE_LOCK_DURATION   (5 * 1000)     /* in msec */
#define WMA_DEAUTH_RECV_WAKE_LOCK_DURATION      (5 * 1000)     /* in msec */
#define WMA_DISASSOC_RECV_WAKE_LOCK_DURATION    (5 * 1000)     /* in msec */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
#define WMA_AUTO_SHUTDOWN_WAKE_LOCK_DURATION    (5 * 1000)     /* in msec */
#endif
#define WMA_BMISS_EVENT_WAKE_LOCK_DURATION      (4 * 1000)     /* in msec */
#define WMA_FW_RSP_EVENT_WAKE_LOCK_DURATION      (3 * 1000)  /* in msec */

#define WMA_TXMIC_LEN 8
#define WMA_RXMIC_LEN 8

/*
 * Length = (2 octets for Index and CTWin/Opp PS) and
 * (13 octets for each NOA Descriptors)
 */

#define WMA_P2P_NOA_IE_OPP_PS_SET (0x80)
#define WMA_P2P_NOA_IE_CTWIN_MASK (0x7F)

#define WMA_P2P_IE_ID 0xdd
#define WMA_P2P_WFA_OUI { 0x50, 0x6f, 0x9a }
#define WMA_P2P_WFA_VER 0x09    /* ver 1.0 */
#define WMA_WSC_OUI { 0x00, 0x50, 0xF2 } /* Microsoft WSC OUI byte */

/* P2P Sub element defintions (according to table 5 of Wifi's P2P spec) */
#define WMA_P2P_SUB_ELEMENT_STATUS                    0
#define WMA_P2P_SUB_ELEMENT_MINOR_REASON              1
#define WMA_P2P_SUB_ELEMENT_CAPABILITY                2
#define WMA_P2P_SUB_ELEMENT_DEVICE_ID                 3
#define WMA_P2P_SUB_ELEMENT_GO_INTENT                 4
#define WMA_P2P_SUB_ELEMENT_CONFIGURATION_TIMEOUT     5
#define WMA_P2P_SUB_ELEMENT_LISTEN_CHANNEL            6
#define WMA_P2P_SUB_ELEMENT_GROUP_BSSID               7
#define WMA_P2P_SUB_ELEMENT_EXTENDED_LISTEN_TIMING    8
#define WMA_P2P_SUB_ELEMENT_INTENDED_INTERFACE_ADDR   9
#define WMA_P2P_SUB_ELEMENT_MANAGEABILITY             10
#define WMA_P2P_SUB_ELEMENT_CHANNEL_LIST              11
#define WMA_P2P_SUB_ELEMENT_NOA                       12
#define WMA_P2P_SUB_ELEMENT_DEVICE_INFO               13
#define WMA_P2P_SUB_ELEMENT_GROUP_INFO                14
#define WMA_P2P_SUB_ELEMENT_GROUP_ID                  15
#define WMA_P2P_SUB_ELEMENT_INTERFACE                 16
#define WMA_P2P_SUB_ELEMENT_OP_CHANNEL                17
#define WMA_P2P_SUB_ELEMENT_INVITATION_FLAGS          18
#define WMA_P2P_SUB_ELEMENT_VENDOR                    221

/* Macros for handling unaligned memory accesses */
#define P2PIE_PUT_LE16(a, val)		\
	do {			\
		(a)[1] = ((u16) (val)) >> 8;	\
		(a)[0] = ((u16) (val)) & 0xff;	\
	} while (0)

#define P2PIE_PUT_LE32(a, val)			\
	do {				\
		(a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (val)) & 0xff);	    \
	} while (0)


#define WMA_DEFAULT_MAX_PSPOLL_BEFORE_WAKE 1

#define WMA_DEFAULT_QPOWER_MAX_PSPOLL_BEFORE_WAKE 1
#define WMA_DEFAULT_QPOWER_TX_WAKE_THRESHOLD 2
#define WMA_DEFAULT_SIFS_BURST_DURATION      8160

#define WMA_VHT_PPS_PAID_MATCH 1
#define WMA_VHT_PPS_GID_MATCH 2
#define WMA_VHT_PPS_DELIM_CRC_FAIL 3

#define WMA_DFS_MAX_20M_SUB_CH 8
#define WMA_80MHZ_START_CENTER_CH_DIFF 6
#define WMA_160MHZ_START_CENTER_CH_DIFF 14
#define WMA_NEXT_20MHZ_START_CH_DIFF 4

#define WMA_DEFAULT_HW_MODE_INDEX 0xFFFF

/**
 * struct probeTime_dwellTime - probe time, dwell time map
 * @dwell_time: dwell time
 * @probe_time: repeat probe time
 */
typedef struct probeTime_dwellTime {
	uint8_t dwell_time;
	uint8_t probe_time;
} t_probeTime_dwellTime;

static const t_probeTime_dwellTime
	probe_time_dwell_time_map[WMA_DWELL_TIME_PROBE_TIME_MAP_SIZE] = {
	{28, 0},                /* 0 SSID */
	{28, 20},               /* 1 SSID */
	{28, 20},               /* 2 SSID */
	{28, 20},               /* 3 SSID */
	{28, 20},               /* 4 SSID */
	{28, 20},               /* 5 SSID */
	{28, 20},               /* 6 SSID */
	{28, 11},               /* 7 SSID */
	{28, 11},               /* 8 SSID */
	{28, 11},               /* 9 SSID */
	{28, 8}                 /* 10 SSID */
};

typedef void (*txFailIndCallback)(uint8_t *peer_mac, uint8_t seqNo);

/**
 * enum t_wma_drv_type - wma driver type
 * @WMA_DRIVER_TYPE_PRODUCTION: production driver type
 * @WMA_DRIVER_TYPE_MFG: manufacture driver type
 * @WMA_DRIVER_TYPE_INVALID: invalid driver type
 */
typedef enum {
	WMA_DRIVER_TYPE_PRODUCTION = 0,
	WMA_DRIVER_TYPE_MFG = 1,
	WMA_DRIVER_TYPE_INVALID = 0x7FFFFFFF
} t_wma_drv_type;

#ifdef FEATURE_WLAN_TDLS
/**
 * enum t_wma_tdls_mode - TDLS mode
 * @WMA_TDLS_SUPPORT_NOT_ENABLED: tdls is disable
 * @WMA_TDLS_SUPPORT_DISABLED: suppress implicit trigger and not respond to peer
 * @WMA_TDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY: suppress implicit trigger,
 *                                          but respond to the peer
 * @WMA_TDLS_SUPPORT_ENABLED: implicit trigger
 * @WMA_TDLS_SUPPORT_ACTIVE_EXTERNAL_CONTROL: External control means
 *    implicit trigger but only to a peer mac configured by user space.
 */
typedef enum {
	WMA_TDLS_SUPPORT_NOT_ENABLED = 0,
	WMA_TDLS_SUPPORT_DISABLED,
	WMA_TDLS_SUPPORT_EXPLICIT_TRIGGER_ONLY,
	WMA_TDLS_SUPPORT_ENABLED,
	WMA_TDLS_SUPPORT_ACTIVE_EXTERNAL_CONTROL,
} t_wma_tdls_mode;

/**
 * enum wma_tdls_peer_notification - TDLS events
 * @WMA_TDLS_SHOULD_DISCOVER: tdls discovery recommended for peer (always based
 *                            on tx bytes per second > tx_discover threshold
 *                            NB: notification will be re-sent after
 *                            discovery_request_interval_ms
 * @WMA_TDLS_SHOULD_TEARDOWN: tdls link tear down recommended for peer
 *                            due to tx bytes per second below
 *                            tx_teardown_threshold
 *                            NB: this notification sent once
 * @WMA_TDLS_PEER_DISCONNECTED: tx peer TDLS link tear down complete
 */
enum wma_tdls_peer_notification {
	WMA_TDLS_SHOULD_DISCOVER,
	WMA_TDLS_SHOULD_TEARDOWN,
	WMA_TDLS_PEER_DISCONNECTED,
};

/**
 * enum wma_tdls_peer_reason - TDLS peer reason
 * @WMA_TDLS_TEARDOWN_REASON_TX: tdls teardown recommended due to low transmits
 * @WMA_TDLS_TEARDOWN_REASON_RATE: tdls tear down recommended due to
 *                                 packet rates < AP rates
 * @WMA_TDLS_TEARDOWN_REASON_RSSI: tdls link tear down recommended
 *                                 due to poor RSSI
 * @WMA_TDLS_TEARDOWN_REASON_SCAN: tdls link tear down recommended
 *                                 due to offchannel scan
 * @WMA_TDLS_DISCONNECTED_REASON_PEER_DELETE: tdls peer disconnected
 *                                            due to peer deletion
 */
enum wma_tdls_peer_reason {
	WMA_TDLS_TEARDOWN_REASON_TX,
	WMA_TDLS_TEARDOWN_REASON_RATE,
	WMA_TDLS_TEARDOWN_REASON_RSSI,
	WMA_TDLS_TEARDOWN_REASON_SCAN,
	WMA_TDLS_DISCONNECTED_REASON_PEER_DELETE,
};
#endif /* FEATURE_WLAN_TDLS */

/**
 * enum wma_rx_exec_ctx - wma rx execution context
 * @WMA_RX_WORK_CTX: work queue context execution
 * @WMA_RX_TASKLET_CTX: tasklet context execution
 * @WMA_RX_SERIALIZER_CTX: MC thread context execution
 *
 */
enum wma_rx_exec_ctx {
	WMA_RX_WORK_CTX,
	WMA_RX_TASKLET_CTX,
	WMA_RX_SERIALIZER_CTX
};

/**
 * enum wma_phy_idx
 * @PHY1: to notify caller that PHY1 specific param needed
 * @PHY2: to notify caller that PHY2 specific param needed
 * @PHY1_PHY2: to notify caller that both PHY's param needed
 * Note: Firmware sends phy map in terms of bitmask, so enum
 *       also needs to be defined that way.
 *
 *       For example, 0x3 = 0011 = BIT0 corresponds to one phy and
 *       BIT1 coresponds to another phy. There is no direct relation between
 *       each bit to particular PHY (ex. PHYA or PHYB).
 *
 *       In simple terms, 3 means referring both PHYs & 1 or 2 means
 *       referring to either PHYA or PHYB.
 */
enum wma_phy_idx {
	PHY1 = 0x1,  /* 0x1 */
	PHY2,        /* 0x2 */
	PHY1_PHY2,   /* 0x3 */
};

/**
 * struct wma_mem_chunk - memory chunks
 * @vaddr: virtual address
 * @paddr: physical address
 * @memctx: dma mapped memory
 * @len: length of data
 * @req_id: request id
 *
 * memory chunck allocated by Host to be managed by FW
 * used only for low latency interfaces like pcie
 */
struct wma_mem_chunk {
	uint32_t *vaddr;
	uint32_t paddr;
	qdf_dma_mem_context(memctx);
	uint32_t len;
	uint32_t req_id;
};

/**
 * struct p2p_scan_param - p2p scan listen parameters
 * @scan_id: scan id
 * @p2p_scan_type: p2p scan type
 */
struct p2p_scan_param {
	uint32_t scan_id;
	tSirP2pScanType p2p_scan_type;
};

/**
 * struct scan_param - scan parameters
 * @scan_id: scan id
 * @scan_requestor_id: scan requestor id
 * @p2p_scan_type: p2p scan type
 */
struct scan_param {
	uint32_t scan_id;
	uint32_t scan_requestor_id;
	tSirP2pScanType p2p_scan_type;
	uint32_t chan_freq;
};

/**
 * struct beacon_info - structure to store beacon template
 * @buf: skb ptr
 * @len: length
 * @dma_mapped: is it dma mapped or not
 * @tim_ie_offset: TIM IE offset
 * @dtim_count: DTIM count
 * @seq_no: sequence no
 * @noa_sub_ie: NOA sub IE
 * @noa_sub_ie_len: NOA sub IE length
 * @noa_ie: NOA IE
 * @p2p_ie_offset: p2p IE offset
 * @lock: lock
 */
struct beacon_info {
	qdf_nbuf_t buf;
	uint32_t len;
	uint8_t dma_mapped;
	uint32_t tim_ie_offset;
	uint8_t dtim_count;
	uint16_t seq_no;
	uint8_t noa_sub_ie[2 + WMA_NOA_IE_SIZE(WMA_MAX_NOA_DESCRIPTORS)];
	uint16_t noa_sub_ie_len;
	uint8_t *noa_ie;
	uint16_t p2p_ie_offset;
	qdf_spinlock_t lock;
};

/**
 * struct beacon_tim_ie - structure to store TIM IE of beacon
 * @tim_ie: tim ie
 * @tim_len: tim ie length
 * @dtim_count: dtim count
 * @dtim_period: dtim period
 * @tim_bitctl: tim bit control
 * @tim_bitmap: tim bitmap
 */
struct beacon_tim_ie {
	uint8_t tim_ie;
	uint8_t tim_len;
	uint8_t dtim_count;
	uint8_t dtim_period;
	uint8_t tim_bitctl;
	uint8_t tim_bitmap[1];
} __ATTRIB_PACK;

/**
 * struct pps - packet power save parameter
 * @paid_match_enable: paid match enable
 * @gid_match_enable: gid match enable
 * @tim_clear: time clear
 * @dtim_clear: dtim clear
 * @eof_delim: eof delim
 * @mac_match: mac match
 * @delim_fail: delim fail
 * @nsts_zero: nsts zero
 * @rssi_chk: RSSI check
 * @ebt_5g: ebt 5GHz
 */
struct pps {
	bool paid_match_enable;
	bool gid_match_enable;
	bool tim_clear;
	bool dtim_clear;
	bool eof_delim;
	bool mac_match;
	bool delim_fail;
	bool nsts_zero;
	bool rssi_chk;
	bool ebt_5g;
};

/**
 * struct qpower_params - qpower related parameters
 * @max_ps_poll_cnt: max ps poll count
 * @max_tx_before_wake: max tx before wake
 * @spec_ps_poll_wake_interval: ps poll wake interval
 * @max_spec_nodata_ps_poll: no data ps poll
 */
struct qpower_params {
	uint32_t max_ps_poll_cnt;
	uint32_t max_tx_before_wake;
	uint32_t spec_ps_poll_wake_interval;
	uint32_t max_spec_nodata_ps_poll;
};


/**
 * struct gtx_config_t - GTX config
 * @gtxRTMask: for HT and VHT rate masks
 * @gtxUsrcfg: host request for GTX mask
 * @gtxPERThreshold: PER Threshold (default: 10%)
 * @gtxPERMargin: PER margin (default: 2%)
 * @gtxTPCstep: TCP step (default: 1)
 * @gtxTPCMin: TCP min (default: 5)
 * @gtxBWMask: BW mask (20/40/80/160 Mhz)
 */
typedef struct {
	uint32_t gtxRTMask[2];
	uint32_t gtxUsrcfg;
	uint32_t gtxPERThreshold;
	uint32_t gtxPERMargin;
	uint32_t gtxTPCstep;
	uint32_t gtxTPCMin;
	uint32_t gtxBWMask;
} gtx_config_t;

/**
 * struct pdev_cli_config_t - store pdev parameters
 * @ani_enable: ANI is enabled/disable on target
 * @ani_poll_len: store ANI polling period
 * @ani_listen_len: store ANI listening period
 * @ani_ofdm_level: store ANI OFDM immunity level
 * @ani_cck_level: store ANI CCK immunity level
 * @cwmenable: Dynamic bw is enable/disable in fw
 * @txchainmask: tx chain mask
 * @rxchainmask: rx chain mask
 * @txpow2g: tx power limit for 2GHz
 * @txpow5g: tx power limit for 5GHz
 * @burst_enable: is burst enable/disable
 * @burst_dur: burst duration
 *
 * This structure stores pdev parameters.
 * Some of these parameters are set in fw and some
 * parameters are only maintained in host.
 */
typedef struct {
	uint32_t ani_enable;
	uint32_t ani_poll_len;
	uint32_t ani_listen_len;
	uint32_t ani_ofdm_level;
	uint32_t ani_cck_level;
	uint32_t cwmenable;
	uint32_t cts_cbw;
	uint32_t txchainmask;
	uint32_t rxchainmask;
	uint32_t txpow2g;
	uint32_t txpow5g;
	uint32_t burst_enable;
	uint32_t burst_dur;
} pdev_cli_config_t;

/**
 * struct vdev_cli_config_t - store vdev parameters
 * @nss: nss width
 * @ldpc: is ldpc is enable/disable
 * @tx_stbc: TX STBC is enable/disable
 * @rx_stbc: RX STBC is enable/disable
 * @shortgi: short gi is enable/disable
 * @rtscts_en: RTS/CTS is enable/disable
 * @chwidth: channel width
 * @tx_rate: tx rate
 * @ampdu: ampdu size
 * @amsdu: amsdu size
 * @erx_adjust: enable/disable early rx enable
 * @erx_bmiss_num: target bmiss number per sample
 * @erx_bmiss_cycle: sample cycle
 * @erx_slop_step: slop_step value
 * @erx_init_slop: init slop
 * @erx_adj_pause: pause adjust enable/disable
 * @erx_dri_sample: enable/disable drift sample
 * @pps_params: packet power save parameters
 * @qpower_params: qpower parameters
 * @gtx_info: GTX offload info
 *
 * This structure stores vdev parameters.
 * Some of these parameters are set in fw and some
 * parameters are only maintained in host.
 */
typedef struct {
	uint32_t nss;
	uint32_t ldpc;
	uint32_t tx_stbc;
	uint32_t rx_stbc;
	uint32_t shortgi;
	uint32_t rtscts_en;
	uint32_t chwidth;
	uint32_t tx_rate;
	uint32_t ampdu;
	uint32_t amsdu;
	uint32_t erx_adjust;
	uint32_t erx_bmiss_num;
	uint32_t erx_bmiss_cycle;
	uint32_t erx_slop_step;
	uint32_t erx_init_slop;
	uint32_t erx_adj_pause;
	uint32_t erx_dri_sample;
	struct pps pps_params;
	struct qpower_params qpower_params;
	gtx_config_t gtx_info;
} vdev_cli_config_t;

/**
 * struct wma_wow - store wow patterns
 * @magic_ptrn_enable: magic pattern enable/disable
 * @wow_enable: wow enable/disable
 * @wow_enable_cmd_sent: is wow enable command sent to fw
 * @deauth_enable: is deauth wakeup enable/disable
 * @disassoc_enable: is disassoc wakeup enable/disable
 * @bmiss_enable: is bmiss wakeup enable/disable
 * @gtk_pdev_enable: is GTK based wakeup enable/disable
 * @gtk_err_enable: is GTK error wakeup enable/disable
 * @lphb_cache: lphb cache
 *
 * This structure stores wow patterns and
 * wow related parameters in host.
 */
struct wma_wow {
	bool magic_ptrn_enable;
	bool wow_enable;
	bool wow_enable_cmd_sent;
	bool deauth_enable;
	bool disassoc_enable;
	bool bmiss_enable;
	bool gtk_err_enable[WMA_MAX_SUPPORTED_BSS];
#ifdef FEATURE_WLAN_LPHB
	/* currently supports only vdev 0.
	 * cache has two entries: one for TCP and one for UDP.
	 */
	tSirLPHBReq lphb_cache[2];
#endif
};

#ifdef WLAN_FEATURE_11W
#define CMAC_IPN_LEN         (6)
#define WMA_IGTK_KEY_INDEX_4 (4)
#define WMA_IGTK_KEY_INDEX_5 (5)

/**
 * struct wma_igtk_ipn_t - GTK IPN info
 * @ipn: IPN info
 */
typedef struct {
	uint8_t ipn[CMAC_IPN_LEN];
} wma_igtk_ipn_t;

/**
 * struct wma_igtk_key_t - GTK key
 * @key_length: key length
 * @key: key
 * @key_id: key id
 */
typedef struct {
	uint16_t key_length;
	uint8_t key[CSR_AES_KEY_LEN];

	/* IPN is maintained per iGTK keyID
	 * 0th index for iGTK keyID = 4;
	 * 1st index for iGTK KeyID = 5
	 */
	wma_igtk_ipn_t key_id[2];
} wma_igtk_key_t;
#endif

/**
 * struct vdev_restart_params_t - vdev restart parameters
 * @vdev_id: vdev id
 * @ssid: ssid
 * @flags: flags
 * @requestor_id: requestor id
 * @chan: channel
 * @hidden_ssid_restart_in_progress: hidden ssid restart flag
 * @ssidHidden: is ssid hidden or not
 */
typedef struct {
	A_UINT32 vdev_id;
	wmi_ssid ssid;
	A_UINT32 flags;
	A_UINT32 requestor_id;
	A_UINT32 disable_hw_ack;
	wmi_channel chan;
	qdf_atomic_t hidden_ssid_restart_in_progress;
	uint8_t ssidHidden;
} vdev_restart_params_t;

/**
 * struct wma_txrx_node - txrx node
 * @addr: mac address
 * @bssid: bssid
 * @handle: wma handle
 * @beacon: beacon info
 * @vdev_restart_params: vdev restart parameters
 * @config: per vdev config parameters
 * @scan_info: scan info
 * @type: type
 * @sub_type: sub type
 * @nlo_match_evt_received: is nlo match event received or not
 * @pno_in_progress: is pno in progress or not
 * @plm_in_progress: is plm in progress or not
 * @ptrn_match_enable: is pattern match is enable or not
 * @num_wow_default_patterns: number of default wow patterns configured for vdev
 * @num_wow_user_patterns: number of user wow patterns configured for vdev
 * @conn_state: connection state
 * @beaconInterval: beacon interval
 * @llbCoexist: 11b coexist
 * @shortSlotTimeSupported: is short slot time supported or not
 * @dtimPeriod: DTIM period
 * @chanmode: channel mode
 * @vht_capable: VHT capablity flag
 * @ht_capable: HT capablity flag
 * @mhz: channel frequency in KHz
 * @chan_width: channel bandwidth
 * @vdev_up: is vdev up or not
 * @tsfadjust: TSF adjust
 * @addBssStaContext: add bss context
 * @aid: association id
 * @rmfEnabled: Robust Management Frame (RMF) enabled/disabled
 * @key: GTK key
 * @uapsd_cached_val: uapsd cached value
 * @stats_rsp: stats response
 * @fw_stats_set: fw stats value
 * @del_staself_req: delete sta self request
 * @bss_status: bss status
 * @rate_flags: rate flags
 * @nss: nss value
 * @is_channel_switch: is channel switch
 * @pause_bitmap: pause bitmap
 * @tx_power: tx power in dbm
 * @max_tx_power: max tx power in dbm
 * @nwType: network type (802.11a/b/g/n/ac)
 * @staKeyParams: sta key parameters
 * @ps_enabled: is powersave enable/disable
 * @dtim_policy: DTIM policy
 * @peer_count: peer count
 * @roam_synch_in_progress: flag is in progress or not
 * @plink_status_req: link status request
 * @psnr_req: snr request
 * @delay_before_vdev_stop: delay
 * @tx_streams: number of tx streams can be used by the vdev
 * @rx_streams: number of rx streams can be used by the vdev
 * @chain_mask: chain mask can be used by the vdev
 * @mac_id: the mac on which vdev is on
 * @wep_default_key_idx: wep default index for group key
 *
 * It stores parameters per vdev in wma.
 */
struct wma_txrx_node {
	uint8_t addr[IEEE80211_ADDR_LEN];
	uint8_t bssid[IEEE80211_ADDR_LEN];
	void *handle;
	struct beacon_info *beacon;
	vdev_restart_params_t vdev_restart_params;
	vdev_cli_config_t config;
	struct scan_param scan_info;
	struct p2p_scan_param p2p_scan_info;
	uint32_t type;
	uint32_t sub_type;
#ifdef FEATURE_WLAN_SCAN_PNO
	bool nlo_match_evt_received;
	bool pno_in_progress;
#endif
#ifdef FEATURE_WLAN_ESE
	bool plm_in_progress;
#endif
	bool ptrn_match_enable;
	uint8_t num_wow_default_patterns;
	uint8_t num_wow_user_patterns;
	bool conn_state;
	tSirMacBeaconInterval beaconInterval;
	uint8_t llbCoexist;
	uint8_t shortSlotTimeSupported;
	uint8_t dtimPeriod;
	WLAN_PHY_MODE chanmode;
	uint8_t vht_capable;
	uint8_t ht_capable;
	A_UINT32 mhz;
	enum phy_ch_width chan_width;
	bool vdev_up;
	uint64_t tsfadjust;
	void *addBssStaContext;
	uint8_t aid;
	uint8_t rmfEnabled;
#ifdef WLAN_FEATURE_11W
	wma_igtk_key_t key;
#endif /* WLAN_FEATURE_11W */
	uint32_t uapsd_cached_val;
	tAniGetPEStatsRsp *stats_rsp;
	uint8_t fw_stats_set;
	void *del_staself_req;
	qdf_atomic_t bss_status;
	uint8_t rate_flags;
	uint8_t nss;
	bool is_channel_switch;
	uint16_t pause_bitmap;
	int8_t tx_power;
	int8_t max_tx_power;
	uint32_t nwType;
	void *staKeyParams;
	bool ps_enabled;
	uint32_t dtim_policy;
	uint32_t peer_count;
	bool roam_synch_in_progress;
	void *plink_status_req;
	void *psnr_req;
	uint8_t delay_before_vdev_stop;
#ifdef FEATURE_WLAN_EXTSCAN
	bool extscan_in_progress;
#endif
	uint32_t alt_modulated_dtim;
	bool alt_modulated_dtim_enabled;
	uint32_t tx_streams;
	uint32_t rx_streams;
	uint32_t chain_mask;
	uint32_t mac_id;
	bool roaming_in_progress;
	int32_t roam_synch_delay;
	uint8_t nss_2g;
	uint8_t nss_5g;
	bool p2p_lo_in_progress;
	uint8_t wep_default_key_idx;
};

#if defined(QCA_WIFI_FTM)
#define MAX_UTF_EVENT_LENGTH    2048
#define MAX_WMI_UTF_LEN         252

/**
 * struct SEG_HDR_INFO_STRUCT - header info
 * @len: length
 * @msgref: message refrence
 * @segmentInfo: segment info
 * @pad: padding
 */
typedef struct {
	A_UINT32 len;
	A_UINT32 msgref;
	A_UINT32 segmentInfo;
	A_UINT32 pad;
} SEG_HDR_INFO_STRUCT;

/**
 * struct utf_event_info - UTF event info
 * @data: data ptr
 * @length: length
 * @offset: offset
 * @currentSeq: curent squence
 * @expectedSeq: expected sequence
 */
struct utf_event_info {
	uint8_t *data;
	uint32_t length;
	qdf_size_t offset;
	uint8_t currentSeq;
	uint8_t expectedSeq;
};
#endif

/**
 * struct scan_timer_info - scan timer info
 * @vdev_id: vdev id
 * @scan_id: scan id
 */
typedef struct {
	uint8_t vdev_id;
	uint32_t scan_id;
} scan_timer_info;

/**
 * struct ibss_power_save_params - IBSS power save parameters
 * @atimWindowLength: ATIM window length
 * @isPowerSaveAllowed: is power save allowed
 * @isPowerCollapseAllowed: is power collapsed allowed
 * @isAwakeonTxRxEnabled: is awake on tx/rx enabled
 * @inactivityCount: inactivity count
 * @txSPEndInactivityTime: tx SP end inactivity time
 * @ibssPsWarmupTime: IBSS power save warm up time
 * @ibssPs1RxChainInAtimEnable: IBSS power save rx chain in ATIM enable
 */
typedef struct {
	uint32_t atimWindowLength;
	uint32_t isPowerSaveAllowed;
	uint32_t isPowerCollapseAllowed;
	uint32_t isAwakeonTxRxEnabled;
	uint32_t inactivityCount;
	uint32_t txSPEndInactivityTime;
	uint32_t ibssPsWarmupTime;
	uint32_t ibssPs1RxChainInAtimEnable;
} ibss_power_save_params;

/**
 * struct dbs_hw_mode_info - WLAN_DBS_HW_MODES_TLV Format
 * @tlv_header: TLV header, TLV tag and len; tag equals WMITLV_TAG_ARRAY_UINT32
 * @hw_mode_list: WLAN_DBS_HW_MODE_LIST entries
 */
struct dbs_hw_mode_info {
	uint32_t tlv_header;
	uint32_t *hw_mode_list;
};

/* Current HTC credit is 2, pool size of 50 is sufficient */
#define WMI_DESC_POOL_MAX 50

/**
 * struct wmi_desc_t - wmi management Tx descriptor.
 * @tx_cmpl_cb_func:	completion callback function, when DL completion and
 *			OTA done.
 * @ota_post_proc_func:	Post process callback function registered.
 * @nbuf:		Network buffer to be freed.
 * @desc_id:		WMI descriptor.
 */

struct wmi_desc_t {
	pWMATxRxCompFunc tx_cmpl_cb;
	pWMAAckFnTxComp  ota_post_proc_cb;
	qdf_nbuf_t	 nbuf;
	uint32_t	 desc_id;
};

/**
 * union wmi_desc_elem_t - linked list wmi desc pool.
 * @next: Pointer next descritor in the pool.
 * @wmi_desc: wmi descriptor element.
 */
union wmi_desc_elem_t {
	union wmi_desc_elem_t *next;
	struct wmi_desc_t wmi_desc;
};

/**
 * struct dual_mac_config - Dual MAC configurations
 * @prev_scan_config: Previous scan configuration
 * @prev_fw_mode_config: Previous FW mode configuration
 * @cur_scan_config: Current scan configuration
 * @cur_fw_mode_config: Current FW mode configuration
 * @req_scan_config: Requested scan configuration
 * @req_fw_mode_config: Requested FW mode configuration
 */
struct dual_mac_config {
	uint32_t prev_scan_config;
	uint32_t prev_fw_mode_config;
	uint32_t cur_scan_config;
	uint32_t cur_fw_mode_config;
	uint32_t req_scan_config;
	uint32_t req_fw_mode_config;

};


/**
 * struct wma_ini_config - Structure to hold wma ini configuration
 * @max_no_of_peers: Max Number of supported
 *
 * Placeholder for WMA ini parameters.
 */
struct wma_ini_config {
	uint8_t max_no_of_peers;
};

/**
 * struct wmi_valid_channels - Channel details part of WMI_SCAN_CHAN_LIST_CMDID
 * @num_channels: Number of channels
 * @channel_list: Channel list
 */
struct wma_valid_channels {
	uint8_t num_channels;
	uint8_t channel_list[MAX_NUM_CHAN];
};

/**
 * struct hw_mode_idx_to_mac_cap_idx - map between hw_mode to capabilities
 * @num_of_macs: number of macs/PHYs for given hw_mode through hw_mode_id
 * @mac_cap_idx: index of the mac/PHY for given hw_mode through hw_mode_id
 * @hw_mode_id: given hw_mode id
 */
struct hw_mode_idx_to_mac_cap_idx {
	uint8_t num_of_macs;
	uint8_t mac_cap_idx;
	uint8_t hw_mode_id;
};

/**
 * struct extended_caps - new extended caps given by firmware
 * @num_hw_modes: number of hardware modes for current SOC
 * @each_hw_mode_cap: hw mode id to phy id mapping
 * @each_phy_cap_per_hwmode: PHY's caps for each hw mode
 * @num_phy_for_hal_reg_cap: number of phy for hal reg cap
 * @hw_mode_to_mac_cap_map: map between hw_mode to capabilities
 */
struct extended_caps {
	WMI_SOC_MAC_PHY_HW_MODE_CAPS num_hw_modes;
	WMI_HW_MODE_CAPABILITIES *each_hw_mode_cap;
	WMI_MAC_PHY_CAPABILITIES *each_phy_cap_per_hwmode;
	WMI_SOC_HAL_REG_CAPABILITIES num_phy_for_hal_reg_cap;
	WMI_HAL_REG_CAPABILITIES_EXT *each_phy_hal_reg_cap;
	struct hw_mode_idx_to_mac_cap_idx *hw_mode_to_mac_cap_map;
};

/**
 * struct t_wma_handle - wma context
 * @wmi_handle: wmi handle
 * @htc_handle: htc handle
 * @cds_context: cds handle
 * @mac_context: mac context
 * @wma_ready_event: wma rx ready event
 * @wma_resume_event: wma resume event
 * @target_suspend: target suspend event
 * @recovery_event: wma FW recovery event
 * @max_station: max stations
 * @max_bssid: max bssid
 * @frame_xln_reqd: frame transmission required
 * @driver_type: driver type
 * @myaddr: current mac address
 * @hwaddr: mac address from EEPROM
 * @target_abi_vers: target firmware version
 * @final_abi_vers: The final ABI version to be used for communicating
 * @target_fw_version: Target f/w build version
 * @lpss_support: LPSS feature is supported in target or not
 * @egap_support: Enhanced Green AP support flag
 * @wmi_ready: wmi status flag
 * @wlan_init_status: wlan init status
 * @qdf_dev: qdf device
 * @phy_capability: PHY Capability from Target
 * @max_frag_entry: Max number of Fragment entry
 * @wmi_service_bitmap: wmi services bitmap received from Target
 * @wlan_resource_config: resource config
 * @frameTransRequired: frame transmission required
 * @wmaGlobalSystemRole: global system role
 * @tx_frm_download_comp_cb: Tx Frame Compl Cb registered by umac
 * @tx_frm_download_comp_event: Event to wait for tx download completion
 * @tx_queue_empty_event: wait for tx queue to get flushed
 * @umac_ota_ack_cb: Ack Complete Callback registered by umac
 * @umac_data_ota_ack_cb: ack complete callback
 * @last_umac_data_ota_timestamp: timestamp when OTA of last umac data was done
 * @last_umac_data_nbuf: cache nbuf ptr for the last umac data buf
 * @needShutdown: is shutdown needed or not
 * @num_mem_chunks: number of memory chunk
 * @mem_chunks: memory chunks
 * @tgt_cfg_update_cb: configuration update callback
 * @dfs_radar_indication_cb: Callback to indicate radar to HDD
 * @reg_cap: regulatory capablities
 * @scan_id: scan id
 * @interfaces: txrx nodes(per vdev)
 * @pdevconfig: pdev related configrations
 * @vdev_resp_queue: vdev response queue
 * @vdev_respq_lock: vdev response queue lock
 * @ht_cap_info: HT capablity info
 * @vht_cap_info: VHT capablity info
 * @vht_supp_mcs: VHT supported MCS
 * @num_rf_chains: number of RF chains
 * @utf_event_info: UTF event information
 * @is_fw_assert: is fw asserted
 * @wow: wow related patterns & parameters
 * @no_of_suspend_ind: number of suspend indications
 * @no_of_resume_ind: number of resume indications
 * @mArpInfo: arp info
 * @powersave_mode: power save mode
 * @ptrn_match_enable_all_vdev: is pattern match is enable/disable
 * @pGetRssiReq: get RSSI request
 * @thermal_mgmt_info: Thermal mitigation related info
 * @roam_offload_enabled: is roam offload enable/disable
 * @ol_ini_info: store ini status of arp offload, ns offload
 * @ssdp: ssdp flag
 * @enable_mc_list : To Check if Multicast list filtering is enabled in FW
 * @ibss_started: is IBSS started or not
 * @ibsskey_info: IBSS key info
 * @dfs_ic: DFS umac interface information
 * @hddTxFailCb: tx fail indication callback
 * @pno_wake_lock: PNO wake lock
 * @extscan_wake_lock: extscan wake lock
 * @wow_wake_lock: wow wake lock
 * @wow_nack: wow negative ack flag
 * @ap_client_cnt: ap client count
 * @is_wow_bus_suspended: is wow bus suspended flag
 * @wma_scan_comp_timer: scan completion timer
 * @dfs_phyerr_filter_offload: dfs phy error filter is offloaded or not
 * @suitable_ap_hb_failure: better ap found
 * @wma_ibss_power_save_params: IBSS Power Save config Parameters
 * @IsRArateLimitEnabled: RA rate limiti s enabled or not
 * @RArateLimitInterval: RA rate limit interval
 * @is_lpass_enabled: Flag to indicate if LPASS feature is enabled or not
 * @is_nan_enabled: Flag to indicate if NaN feature is enabled or not
 * @staMaxLIModDtim: station max listen interval
 * @staModDtim: station mode DTIM
 * @staDynamicDtim: station dynamic DTIM
 * @enable_mhf_offload: is MHF offload enable/disable
 * @last_mhf_entries_timestamp: timestamp when last entries where set
 * @dfs_pri_multiplier: DFS multiplier
 * @hw_bd_id: hardware board id
 * @hw_bd_info: hardware board info
 * @in_d0wow: D0WOW is enable/disable
 * @miracast_value: miracast value
 * @log_completion_timer: log completion timer
 * @mgmt_rx: management rx callback
 * @num_dbs_hw_modes: Number of HW modes supported by the FW
 * @dbs_mode: DBS HW mode list
 * @old_hw_mode_index: Previous configured HW mode index
 * @new_hw_mode_index: Current configured HW mode index
 * @extended_caps phy_caps: extended caps per hw mode
 * @peer_authorized_cb: peer authorized hdd callback
 * @ocb_callback: callback to OCB commands
 * @ocb_resp: response to OCB commands
 * @wow_pno_match_wake_up_count: PNO match wake up count
 * @wow_pno_complete_wake_up_count: PNO complete wake up count
 * @wow_gscan_wake_up_count: Gscan wake up count
 * @wow_low_rssi_wake_up_count: Low rssi wake up count
 * @wow_rssi_breach_wake_up_count: RSSI breach wake up count
 * @wow_ucast_wake_up_count: WoW unicast packet wake up count
 * @wow_bcast_wake_up_count: WoW brodcast packet wake up count
 * @wow_ipv4_mcast_wake_up_count: WoW IPV4 mcast packet wake up count
 * @wow_ipv6_mcast_wake_up_count: WoW IPV6 mcast packet wake up count
 * @wow_ipv6_mcast_ra_stats: WoW IPV6 mcast RA packet wake up count
 * @wow_ipv6_mcast_ns_stats: WoW IPV6 mcast NS packet wake up count
 * @wow_ipv6_mcast_na_stats: WoW IPV6 mcast NA packet wake up count
 * @dual_mac_cfg: Dual mac configuration params for scan and fw mode
 *
 * @max_scan:  maximum scan requests than can be queued
 * This structure is global wma context
 * It contains global wma module parameters and
 * handle of other modules.
 * @saved_wmi_init_cmd: Saved WMI INIT command
 * @service_ready_ext_evt: Wait event for service ready ext
 * @wmi_cmd_rsp_wake_lock: wmi command response wake lock
 * @wmi_cmd_rsp_runtime_lock: wmi command response bus lock
 * @saved_chan: saved channel list sent as part of WMI_SCAN_CHAN_LIST_CMDID
 */
typedef struct {
	void *wmi_handle;
	void *htc_handle;
	void *cds_context;
	void *mac_context;
	qdf_event_t wma_ready_event;
	qdf_event_t wma_resume_event;
	qdf_event_t target_suspend;
	qdf_event_t runtime_suspend;
	qdf_event_t recovery_event;
	uint16_t max_station;
	uint16_t max_bssid;
	uint32_t frame_xln_reqd;
	t_wma_drv_type driver_type;
	uint8_t myaddr[IEEE80211_ADDR_LEN];
	uint8_t hwaddr[IEEE80211_ADDR_LEN];
	wmi_abi_version target_abi_vers;
	wmi_abi_version final_abi_vers;
	uint32_t target_fw_version;
#ifdef WLAN_FEATURE_LPSS
	uint8_t lpss_support;
#endif
	uint8_t ap_arpns_support;
#ifdef FEATURE_GREEN_AP
	bool egap_support;
#endif
	bool wmi_ready;
	uint32_t wlan_init_status;
	qdf_device_t qdf_dev;
	uint32_t phy_capability;
	uint32_t max_frag_entry;
	uint32_t wmi_service_bitmap[WMI_SERVICE_BM_SIZE];
	wmi_resource_config wlan_resource_config;
	uint32_t frameTransRequired;
	tBssSystemRole wmaGlobalSystemRole;
	pWMATxRxCompFunc tx_frm_download_comp_cb;
	qdf_event_t tx_frm_download_comp_event;
	/*
	 * Dummy event to wait for draining MSDUs left in hardware tx
	 * queue and before requesting VDEV_STOP. Nobody will set this
	 * and wait will timeout, and code will poll the pending tx
	 * descriptors number to be zero.
	 */
	qdf_event_t tx_queue_empty_event;
	pWMAAckFnTxComp umac_ota_ack_cb[SIR_MAC_MGMT_RESERVED15];
	pWMAAckFnTxComp umac_data_ota_ack_cb;
	unsigned long last_umac_data_ota_timestamp;
	qdf_nbuf_t last_umac_data_nbuf;
	bool needShutdown;
	uint32_t num_mem_chunks;
	struct wmi_host_mem_chunk mem_chunks[MAX_MEM_CHUNKS];
	wma_tgt_cfg_cb tgt_cfg_update_cb;
	wma_dfs_radar_indication_cb dfs_radar_indication_cb;
	HAL_REG_CAPABILITIES reg_cap;
	uint32_t scan_id;
	struct wma_txrx_node *interfaces;
	pdev_cli_config_t pdevconfig;
	qdf_list_t vdev_resp_queue;
	qdf_spinlock_t vdev_respq_lock;
	qdf_list_t wma_hold_req_queue;
	qdf_spinlock_t wma_hold_req_q_lock;
	uint32_t ht_cap_info;
	uint32_t vht_cap_info;
	uint32_t vht_supp_mcs;
	uint32_t num_rf_chains;
#if defined(QCA_WIFI_FTM)
	struct utf_event_info utf_event_info;
#endif
	uint8_t is_fw_assert;
	struct wma_wow wow;
	uint8_t no_of_suspend_ind;
	uint8_t no_of_resume_ind;
	/* Have a back up of arp info to send along
	 * with ns info suppose if ns also enabled
	 */
	tSirHostOffloadReq mArpInfo;
	struct wma_tx_ack_work_ctx *ack_work_ctx;
	uint8_t powersave_mode;
	bool ptrn_match_enable_all_vdev;
	void *pGetRssiReq;
	t_thermal_mgmt thermal_mgmt_info;
	bool roam_offload_enabled;
	/* Here ol_ini_info is used to store ini
	 * status of arp offload, ns offload
	 * and others. Currently 1st bit is used
	 * for arp off load and 2nd bit for ns
	 * offload currently, rest bits are unused
	 */
	uint8_t ol_ini_info;
	bool ssdp;
	bool enable_mc_list;
	uint8_t ibss_started;
	tSetBssKeyParams ibsskey_info;
	struct ieee80211com *dfs_ic;
	txFailIndCallback hddTxFailCb;
#ifdef FEATURE_WLAN_SCAN_PNO
	qdf_wake_lock_t pno_wake_lock;
#endif
#ifdef FEATURE_WLAN_EXTSCAN
	qdf_wake_lock_t extscan_wake_lock;
#endif
	qdf_wake_lock_t wow_wake_lock;
	int wow_nack;
	qdf_atomic_t is_wow_bus_suspended;
	qdf_mc_timer_t wma_scan_comp_timer;
	uint8_t dfs_phyerr_filter_offload;
	bool suitable_ap_hb_failure;
	ibss_power_save_params wma_ibss_power_save_params;
#ifdef FEATURE_WLAN_RA_FILTERING
	bool IsRArateLimitEnabled;
	uint16_t RArateLimitInterval;
#endif
#ifdef WLAN_FEATURE_LPSS
	bool is_lpass_enabled;
#endif
#ifdef WLAN_FEATURE_NAN
	bool is_nan_enabled;
#endif
	uint8_t staMaxLIModDtim;
	uint8_t staModDtim;
	uint8_t staDynamicDtim;
	uint8_t enable_mhf_offload;
	unsigned long last_mhf_entries_timestamp;
	int32_t dfs_pri_multiplier;
	uint32_t hw_bd_id;
	uint32_t hw_bd_info[HW_BD_INFO_SIZE];
	uint32_t miracast_value;
	qdf_mc_timer_t log_completion_timer;
	wma_mgmt_frame_rx_callback mgmt_rx;
	uint32_t num_dbs_hw_modes;
	struct dbs_hw_mode_info hw_mode;
	uint32_t old_hw_mode_index;
	uint32_t new_hw_mode_index;
	struct extended_caps phy_caps;
	qdf_atomic_t scan_id_counter;
	wma_peer_authorized_fp peer_authorized_cb;
	uint32_t wow_pno_match_wake_up_count;
	uint32_t wow_pno_complete_wake_up_count;
	uint32_t wow_gscan_wake_up_count;
	uint32_t wow_low_rssi_wake_up_count;
	uint32_t wow_rssi_breach_wake_up_count;
	uint32_t wow_ucast_wake_up_count;
	uint32_t wow_bcast_wake_up_count;
	uint32_t wow_ipv4_mcast_wake_up_count;
	uint32_t wow_ipv6_mcast_wake_up_count;
	uint32_t wow_ipv6_mcast_ra_stats;
	uint32_t wow_ipv6_mcast_ns_stats;
	uint32_t wow_ipv6_mcast_na_stats;
	uint32_t wow_oem_response_wake_up_count;

	/* OCB request contexts */
	struct sir_ocb_config *ocb_config_req;
	struct dual_mac_config dual_mac_cfg;
	struct {
		uint16_t pool_size;
		uint16_t num_free;
		union wmi_desc_elem_t *array;
		union wmi_desc_elem_t *freelist;
		qdf_spinlock_t wmi_desc_pool_lock;
	} wmi_desc_pool;
	uint8_t max_scan;
	uint16_t self_gen_frm_pwr;
	bool tx_chain_mask_cck;
	/* Going with a timer instead of wait event because on receiving the
	 * service ready event, we will be waiting on the MC thread for the
	 * service extended ready event which is also processed in MC thread.
	 * This leads to MC thread being stuck. Alternative was to process
	 * these events in tasklet/workqueue context. But, this leads to
	 * race conditions when the events are processed in two different
	 * context. So, processing ready event and extended ready event in
	 * the serialized MC thread context with a timer.
	 */
	qdf_mc_timer_t service_ready_ext_timer;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	void (*csr_roam_synch_cb)(tpAniSirGlobal mac,
		roam_offload_synch_ind *roam_synch_data,
		tpSirBssDescription  bss_desc_ptr,
		enum sir_roam_op_code reason);
	QDF_STATUS (*pe_roam_synch_cb)(tpAniSirGlobal mac,
		roam_offload_synch_ind *roam_synch_data,
		tpSirBssDescription  bss_desc_ptr);
#endif
	qdf_wake_lock_t wmi_cmd_rsp_wake_lock;
	qdf_runtime_lock_t wmi_cmd_rsp_runtime_lock;
	qdf_runtime_lock_t wma_runtime_resume_lock;
	uint32_t fine_time_measurement_cap;
	bool bpf_enabled;
	struct wma_ini_config ini_config;
	struct wma_valid_channels saved_chan;
	/* NAN datapath support enabled in firmware */
	bool nan_datapath_enabled;
	QDF_STATUS (*pe_ndp_event_handler)(tpAniSirGlobal mac_ctx,
					   cds_msg_t *msg);
	bool sub_20_support;
} t_wma_handle, *tp_wma_handle;

/**
 * struct wma_target_cap - target capabality
 * @wmi_service_bitmap: wmi services bitmap
 * @wlan_resource_config: resource config
 */
struct wma_target_cap {
	/* wmi services bitmap received from Target */
	uint32_t wmi_service_bitmap[WMI_SERVICE_BM_SIZE];
	/* default resource config,the os shim can overwrite it */
	wmi_resource_config wlan_resource_config;
};

/**
 * struct t_wma_start_req - wma start request parameters
 * @pConfigBuffer: config buffer
 * @usConfigBufferLen: Length of the config buffer above
 * @driver_type: Production or FTM driver
 * @pUserData: user data
 * @pIndUserData: indication function pointer to send to UMAC
 *
 * The shared memory between WDI and HAL is 4K so maximum data can be
 * transferred from WDI to HAL is 4K
 */
typedef struct {
	void *pConfigBuffer;
	uint16_t usConfigBufferLen;
	t_wma_drv_type driver_type;
	void *pUserData;
	void *pIndUserData;
} t_wma_start_req;

/* Enumeration for Version */
typedef enum {
	WLAN_HAL_MSG_VERSION0 = 0,
	WLAN_HAL_MSG_VERSION1 = 1,
	WLAN_HAL_MSG_WCNSS_CTRL_VERSION = 0x7FFF,   /*define as 2 bytes data */
	WLAN_HAL_MSG_VERSION_MAX_FIELD = WLAN_HAL_MSG_WCNSS_CTRL_VERSION
} tHalHostMsgVersion;

/**
 * struct sHalMacStartParameter - mac start request parameters
 * @driverType: driver type (production/FTM)
 * @uConfigBufferLen: length of config buffer
 */
typedef struct qdf_packed sHalMacStartParameter {
	tDriverType driverType;
	uint32_t uConfigBufferLen;

	/* Following this there is a TLV formatted buffer of length
	 * "uConfigBufferLen" bytes containing all config values.
	 * The TLV is expected to be formatted like this:
	 * 0           15            31           31+CFG_LEN-1        length-1
	 * |   CFG_ID   |   CFG_LEN   |   CFG_BODY    |  CFG_ID  |......|
	 */
} tHalMacStartParameter, *tpHalMacStartParameter;

extern void cds_wma_complete_cback(void *p_cds_context);
extern void wma_send_regdomain_info_to_fw(uint32_t reg_dmn, uint16_t regdmn2G,
					  uint16_t regdmn5G, int8_t ctl2G,
					  int8_t ctl5G);
/**
 * enum frame_index - Frame index
 * @GENERIC_NODOWNLD_NOACK_COMP_INDEX: Frame index for no download comp no ack
 * @GENERIC_DOWNLD_COMP_NOACK_COMP_INDEX: Frame index for download comp no ack
 * @GENERIC_DOWNLD_COMP_ACK_COMP_INDEX: Frame index for download comp and ack
 * @GENERIC_NODOWLOAD_ACK_COMP_INDEX: Frame index for no download comp and ack
 * @FRAME_INDEX_MAX: maximum frame index
 */
enum frame_index {
	GENERIC_NODOWNLD_NOACK_COMP_INDEX,
	GENERIC_DOWNLD_COMP_NOACK_COMP_INDEX,
	GENERIC_DOWNLD_COMP_ACK_COMP_INDEX,
	GENERIC_NODOWLOAD_ACK_COMP_INDEX,
	FRAME_INDEX_MAX
};

/**
 * struct wma_tx_ack_work_ctx - tx ack work context
 * @wma_handle: wma handle
 * @sub_type: sub type
 * @status: status
 * @ack_cmp_work: work structure
 */
struct wma_tx_ack_work_ctx {
	tp_wma_handle wma_handle;
	uint16_t sub_type;
	int32_t status;
	qdf_work_t ack_cmp_work;
};

/**
 * struct wma_target_req - target request parameters
 * @event_timeout: event timeout
 * @node: list
 * @user_data: user data
 * @msg_type: message type
 * @vdev_id: vdev id
 * @type: type
 */
struct wma_target_req {
	qdf_mc_timer_t event_timeout;
	qdf_list_node_t node;
	void *user_data;
	uint32_t msg_type;
	uint8_t vdev_id;
	uint8_t type;
};

/**
 * struct wma_vdev_start_req - vdev start request parameters
 * @beacon_intval: beacon interval
 * @dtim_period: dtim period
 * @max_txpow: max tx power
 * @chan_offset: channel offset
 * @is_dfs: is dfs supported or not
 * @vdev_id: vdev id
 * @chan: channel
 * @oper_mode: operating mode
 * @ssid: ssid
 * @hidden_ssid: hidden ssid
 * @pmf_enabled: is pmf enabled or not
 * @vht_capable: VHT capabality
 * @ht_capable: HT capabality
 * @dfs_pri_multiplier: DFS multiplier
 * @dot11_mode: 802.11 mode
 * @is_half_rate: is the channel operating at 10MHz
 * @is_quarter_rate: is the channel operating at 5MHz
 * @preferred_tx_streams: policy manager indicates the preferred
 *			number of transmit streams
 * @preferred_rx_streams: policy manager indicates the preferred
 *			number of receive streams
 */
struct wma_vdev_start_req {
	uint32_t beacon_intval;
	uint32_t dtim_period;
	int32_t max_txpow;
	enum phy_ch_width chan_width;
	bool is_dfs;
	uint8_t vdev_id;
	uint8_t chan;
	uint8_t oper_mode;
	tSirMacSSid ssid;
	uint8_t hidden_ssid;
	uint8_t pmf_enabled;
	uint8_t vht_capable;
	uint8_t ch_center_freq_seg0;
	uint8_t ch_center_freq_seg1;
	uint8_t ht_capable;
	int32_t dfs_pri_multiplier;
	uint8_t dot11_mode;
	bool is_half_rate;
	bool is_quarter_rate;
	uint32_t preferred_tx_streams;
	uint32_t preferred_rx_streams;
};

/**
 * struct wma_set_key_params - set key parameters
 * @vdev_id: vdev id
 * @def_key_idx: used to see if we have to read the key from cfg
 * @key_len: key length
 * @peer_mac: peer mac address
 * @singl_tid_rc: 1=Single TID based Replay Count, 0=Per TID based RC
 * @key_type: key type
 * @key_idx: key index
 * @unicast: unicast flag
 * @key_data: key data
 */
struct wma_set_key_params {
	uint8_t vdev_id;
	/* def_key_idx can be used to see if we have to read the key from cfg */
	uint32_t def_key_idx;
	uint16_t key_len;
	uint8_t peer_mac[IEEE80211_ADDR_LEN];
	uint8_t singl_tid_rc;
	enum eAniEdType key_type;
	uint32_t key_idx;
	bool unicast;
	uint8_t key_data[SIR_MAC_MAX_KEY_LENGTH];
};

/**
 * struct t_thermal_cmd_params - thermal command parameters
 * @minTemp: minimum temprature
 * @maxTemp: maximum temprature
 * @thermalEnable: thermal enable
 */
typedef struct {
	uint16_t minTemp;
	uint16_t maxTemp;
	uint8_t thermalEnable;
} t_thermal_cmd_params, *tp_thermal_cmd_params;

/**
 * enum wma_cfg_cmd_id - wma cmd ids
 * @WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID: txrx firmware stats enable command
 * @WMA_VDEV_TXRX_FWSTATS_RESET_CMDID: txrx firmware stats reset command
 * @WMA_VDEV_MCC_SET_TIME_LATENCY: set MCC latency time
 * @WMA_VDEV_MCC_SET_TIME_QUOTA: set MCC time quota
 * @WMA_VDEV_IBSS_SET_ATIM_WINDOW_SIZE: set IBSS ATIM window size
 * @WMA_VDEV_IBSS_SET_POWER_SAVE_ALLOWED: set IBSS enable power save
 * @WMA_VDEV_IBSS_SET_POWER_COLLAPSE_ALLOWED: set IBSS power collapse enable
 * @WMA_VDEV_IBSS_SET_AWAKE_ON_TX_RX: awake IBSS on TX/RX
 * @WMA_VDEV_IBSS_SET_INACTIVITY_TIME: set IBSS inactivity time
 * @WMA_VDEV_IBSS_SET_TXSP_END_INACTIVITY_TIME: set IBSS TXSP
 * @WMA_VDEV_IBSS_PS_SET_WARMUP_TIME_SECS: set IBSS power save warmup time
 * @WMA_VDEV_IBSS_PS_SET_1RX_CHAIN_IN_ATIM_WINDOW: set IBSS power save ATIM
 * @WMA_VDEV_DFS_CONTROL_CMDID: DFS control command
 * @WMA_VDEV_TXRX_GET_IPA_UC_FW_STATS_CMDID: get IPA microcontroller fw stats
 *
 * wma command ids for configuration request which
 * does not involve sending a wmi command.
 */
enum wma_cfg_cmd_id {
	WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID = WMI_CMDID_MAX,
	WMA_VDEV_TXRX_FWSTATS_RESET_CMDID,
	WMA_VDEV_MCC_SET_TIME_LATENCY,
	WMA_VDEV_MCC_SET_TIME_QUOTA,
	WMA_VDEV_IBSS_SET_ATIM_WINDOW_SIZE,
	WMA_VDEV_IBSS_SET_POWER_SAVE_ALLOWED,
	WMA_VDEV_IBSS_SET_POWER_COLLAPSE_ALLOWED,
	WMA_VDEV_IBSS_SET_AWAKE_ON_TX_RX,
	WMA_VDEV_IBSS_SET_INACTIVITY_TIME,
	WMA_VDEV_IBSS_SET_TXSP_END_INACTIVITY_TIME,
	WMA_VDEV_IBSS_PS_SET_WARMUP_TIME_SECS,
	WMA_VDEV_IBSS_PS_SET_1RX_CHAIN_IN_ATIM_WINDOW,
	WMA_VDEV_DFS_CONTROL_CMDID,
	WMA_VDEV_TXRX_GET_IPA_UC_FW_STATS_CMDID,
	WMA_CMD_ID_MAX
};

/**
 * struct wma_trigger_uapsd_params - trigger uapsd parameters
 * @wmm_ac: wmm access catagory
 * @user_priority: user priority
 * @service_interval: service interval
 * @suspend_interval: suspend interval
 * @delay_interval: delay interval
 */
typedef struct wma_trigger_uapsd_params {
	uint32_t wmm_ac;
	uint32_t user_priority;
	uint32_t service_interval;
	uint32_t suspend_interval;
	uint32_t delay_interval;
} t_wma_trigger_uapsd_params, *tp_wma_trigger_uapsd_params;

/**
 * enum uapsd_peer_param_max_sp - U-APSD maximum service period of peer station
 * @UAPSD_MAX_SP_LEN_UNLIMITED: unlimited max service period
 * @UAPSD_MAX_SP_LEN_2: max service period = 2
 * @UAPSD_MAX_SP_LEN_4: max service period = 4
 * @UAPSD_MAX_SP_LEN_6: max service period = 6
 */
enum uapsd_peer_param_max_sp {
	UAPSD_MAX_SP_LEN_UNLIMITED = 0,
	UAPSD_MAX_SP_LEN_2 = 2,
	UAPSD_MAX_SP_LEN_4 = 4,
	UAPSD_MAX_SP_LEN_6 = 6
};

/**
 * enum uapsd_peer_param_enabled_ac - U-APSD Enabled AC's of peer station
 * @UAPSD_VO_ENABLED: enable uapsd for voice
 * @UAPSD_VI_ENABLED: enable uapsd for video
 * @UAPSD_BK_ENABLED: enable uapsd for background
 * @UAPSD_BE_ENABLED: enable uapsd for best effort
 */
enum uapsd_peer_param_enabled_ac {
	UAPSD_VO_ENABLED = 0x01,
	UAPSD_VI_ENABLED = 0x02,
	UAPSD_BK_ENABLED = 0x04,
	UAPSD_BE_ENABLED = 0x08
};

/**
 * enum profile_id_t - Firmware profiling index
 * @PROF_CPU_IDLE: cpu idle profile
 * @PROF_PPDU_PROC: ppdu processing profile
 * @PROF_PPDU_POST: ppdu post profile
 * @PROF_HTT_TX_INPUT: htt tx input profile
 * @PROF_MSDU_ENQ: msdu enqueue profile
 * @PROF_PPDU_POST_HAL: ppdu post profile
 * @PROF_COMPUTE_TX_TIME: tx time profile
 * @PROF_MAX_ID: max profile index
 */
enum profile_id_t {
	PROF_CPU_IDLE,
	PROF_PPDU_PROC,
	PROF_PPDU_POST,
	PROF_HTT_TX_INPUT,
	PROF_MSDU_ENQ,
	PROF_PPDU_POST_HAL,
	PROF_COMPUTE_TX_TIME,
	PROF_MAX_ID,
};

/**
 * struct p2p_ie - P2P IE structural definition.
 * @p2p_id: p2p id
 * @p2p_len: p2p length
 * @p2p_oui: p2p OUI
 * @p2p_oui_type: p2p OUI type
 */
struct p2p_ie {
	uint8_t p2p_id;
	uint8_t p2p_len;
	uint8_t p2p_oui[3];
	uint8_t p2p_oui_type;
} __packed;

/**
 * struct p2p_noa_descriptor - noa descriptor
 * @type_count: 255: continuous schedule, 0: reserved
 * @duration: Absent period duration in micro seconds
 * @interval: Absent period interval in micro seconds
 * @start_time: 32 bit tsf time when in starts
 */
struct p2p_noa_descriptor {
	uint8_t type_count;
	uint32_t duration;
	uint32_t interval;
	uint32_t start_time;
} __packed;

/**
 * struct p2p_sub_element_noa - p2p noa element
 * @p2p_sub_id: p2p sub id
 * @p2p_sub_len: p2p sub length
 * @index: identifies instance of NOA su element
 * @oppPS: oppPS state of the AP
 * @ctwindow: ctwindow in TUs
 * @num_descriptors: number of NOA descriptors
 * @noa_descriptors: noa descriptors
 */
struct p2p_sub_element_noa {
	uint8_t p2p_sub_id;
	uint8_t p2p_sub_len;
	uint8_t index;          /* identifies instance of NOA su element */
	uint8_t oppPS:1,        /* oppPS state of the AP */
		ctwindow:7;     /* ctwindow in TUs */
	uint8_t num_descriptors;        /* number of NOA descriptors */
	struct p2p_noa_descriptor noa_descriptors[WMA_MAX_NOA_DESCRIPTORS];
};

/**
 * struct wma_decap_info_t - decapsulation info
 * @hdr: header
 * @hdr_len: header length
 */
struct wma_decap_info_t {
	uint8_t hdr[sizeof(struct ieee80211_qosframe_addr4)];
	int32_t hdr_len;
};

/**
 * enum packet_power_save - packet power save params
 * @WMI_VDEV_PPS_PAID_MATCH: paid match param
 * @WMI_VDEV_PPS_GID_MATCH: gid match param
 * @WMI_VDEV_PPS_EARLY_TIM_CLEAR: early tim clear param
 * @WMI_VDEV_PPS_EARLY_DTIM_CLEAR: early dtim clear param
 * @WMI_VDEV_PPS_EOF_PAD_DELIM: eof pad delim param
 * @WMI_VDEV_PPS_MACADDR_MISMATCH: macaddr mismatch param
 * @WMI_VDEV_PPS_DELIM_CRC_FAIL: delim CRC fail param
 * @WMI_VDEV_PPS_GID_NSTS_ZERO: gid nsts zero param
 * @WMI_VDEV_PPS_RSSI_CHECK: RSSI check param
 * @WMI_VDEV_PPS_5G_EBT: 5G ebt param
 */
typedef enum {
	WMI_VDEV_PPS_PAID_MATCH = 0,
	WMI_VDEV_PPS_GID_MATCH = 1,
	WMI_VDEV_PPS_EARLY_TIM_CLEAR = 2,
	WMI_VDEV_PPS_EARLY_DTIM_CLEAR = 3,
	WMI_VDEV_PPS_EOF_PAD_DELIM = 4,
	WMI_VDEV_PPS_MACADDR_MISMATCH = 5,
	WMI_VDEV_PPS_DELIM_CRC_FAIL = 6,
	WMI_VDEV_PPS_GID_NSTS_ZERO = 7,
	WMI_VDEV_PPS_RSSI_CHECK = 8,
	WMI_VDEV_VHT_SET_GID_MGMT = 9,
	WMI_VDEV_PPS_5G_EBT = 10
} packet_power_save;

/**
 * enum green_tx_param - green tx parameters
 * @WMI_VDEV_PARAM_GTX_HT_MCS: ht mcs param
 * @WMI_VDEV_PARAM_GTX_VHT_MCS: vht mcs param
 * @WMI_VDEV_PARAM_GTX_USR_CFG: user cfg param
 * @WMI_VDEV_PARAM_GTX_THRE: thre param
 * @WMI_VDEV_PARAM_GTX_MARGIN: green tx margin param
 * @WMI_VDEV_PARAM_GTX_STEP: green tx step param
 * @WMI_VDEV_PARAM_GTX_MINTPC: mintpc param
 * @WMI_VDEV_PARAM_GTX_BW_MASK: bandwidth mask
 */
typedef enum {
	WMI_VDEV_PARAM_GTX_HT_MCS,
	WMI_VDEV_PARAM_GTX_VHT_MCS,
	WMI_VDEV_PARAM_GTX_USR_CFG,
	WMI_VDEV_PARAM_GTX_THRE,
	WMI_VDEV_PARAM_GTX_MARGIN,
	WMI_VDEV_PARAM_GTX_STEP,
	WMI_VDEV_PARAM_GTX_MINTPC,
	WMI_VDEV_PARAM_GTX_BW_MASK,
} green_tx_param;

#ifdef FEATURE_WLAN_TDLS
/**
 * struct wma_tdls_params - TDLS parameters
 * @vdev_id: vdev id
 * @tdls_state: TDLS state
 * @notification_interval_ms: notification inerval
 * @tx_discovery_threshold: tx discovery threshold
 * @tx_teardown_threshold: tx teardown threashold
 * @rssi_teardown_threshold: RSSI teardown threshold
 * @rssi_delta: RSSI delta
 * @tdls_options: TDLS options
 * @peer_traffic_ind_window: raffic indication window
 * @peer_traffic_response_timeout: traffic response timeout
 * @puapsd_mask: uapsd mask
 * @puapsd_inactivity_time: uapsd inactivity time
 * @puapsd_rx_frame_threshold: uapsd rx frame threshold
 * @teardown_notification_ms: tdls teardown notification interval
 * @tdls_peer_kickout_threshold: tdls packet threshold for
 *    peer kickout operation
 */
typedef struct wma_tdls_params {
	uint32_t vdev_id;
	uint32_t tdls_state;
	uint32_t notification_interval_ms;
	uint32_t tx_discovery_threshold;
	uint32_t tx_teardown_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
	uint32_t tdls_options;
	uint32_t peer_traffic_ind_window;
	uint32_t peer_traffic_response_timeout;
	uint32_t puapsd_mask;
	uint32_t puapsd_inactivity_time;
	uint32_t puapsd_rx_frame_threshold;
	uint32_t teardown_notification_ms;
	uint32_t tdls_peer_kickout_threshold;
} t_wma_tdls_params;

/**
 * struct wma_tdls_peer_event - TDLS peer event
 * @vdev_id: vdev id
 * @peer_macaddr: peer MAC address
 * @peer_status: TDLS peer status
 * @peer_reason: TDLS peer reason
 */
typedef struct {
	A_UINT32 vdev_id;
	wmi_mac_addr peer_macaddr;
	A_UINT32 peer_status;
	A_UINT32 peer_reason;
} wma_tdls_peer_event;

#endif /* FEATURE_WLAN_TDLS */

/**
 * struct wma_dfs_radar_channel_list - dfs radar channel list
 * @nchannels: nuber of channels
 * @channels: Channel number including bonded channels on which
 *            radar is present
 */
struct wma_dfs_radar_channel_list {
	A_UINT32 nchannels;
	uint8_t channels[WMA_DFS_MAX_20M_SUB_CH];
};

/**
 * struct wma_dfs_radar_indication - Structure to indicate RADAR
 * @vdev_id: vdev id
 * @chan_list: Channel list on which RADAR is detected
 * @dfs_radar_status: Flag to Indicate RADAR presence on the current channel
 * @use_nol: Flag to indicate use NOL
 */
struct wma_dfs_radar_indication {
	A_UINT32 vdev_id;
	struct wma_dfs_radar_channel_list chan_list;
	uint32_t dfs_radar_status;
	int use_nol;
};

/**
 * enum uapsd_ac - U-APSD Access Categories
 * @UAPSD_BE: best effort
 * @UAPSD_BK: back ground
 * @UAPSD_VI: video
 * @UAPSD_VO: voice
 */
enum uapsd_ac {
	UAPSD_BE,
	UAPSD_BK,
	UAPSD_VI,
	UAPSD_VO
};

QDF_STATUS wma_disable_uapsd_per_ac(tp_wma_handle wma_handle,
				    uint32_t vdev_id, enum uapsd_ac ac);

/**
 * enum uapsd_up - U-APSD User Priorities
 * @UAPSD_UP_BE: best effort
 * @UAPSD_UP_BK: back ground
 * @UAPSD_UP_RESV: reserve
 * @UAPSD_UP_EE: Excellent Effort
 * @UAPSD_UP_CL: Critical Applications
 * @UAPSD_UP_VI: video
 * @UAPSD_UP_VO: voice
 * @UAPSD_UP_NC: Network Control
 */
enum uapsd_up {
	UAPSD_UP_BE,
	UAPSD_UP_BK,
	UAPSD_UP_RESV,
	UAPSD_UP_EE,
	UAPSD_UP_CL,
	UAPSD_UP_VI,
	UAPSD_UP_VO,
	UAPSD_UP_NC,
	UAPSD_UP_MAX
};

/**
 * struct wma_unit_test_cmd - unit test command parameters
 * @vdev_id: vdev id
 * @module_id: module id
 * @num_args: number of arguments
 * @args: arguments
 */
typedef struct wma_unit_test_cmd {
	uint32_t vdev_id;
	WLAN_MODULE_ID module_id;
	uint32_t num_args;
	uint32_t args[WMA_MAX_NUM_ARGS];
} t_wma_unit_test_cmd;

/**
 * struct wma_roam_invoke_cmd - roam invoke command
 * @vdev_id: vdev id
 * @bssid: mac address
 * @channel: channel
 */
struct wma_roam_invoke_cmd {
	uint32_t vdev_id;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint32_t channel;
};

/**
 * struct wma_process_fw_event_params - fw event parameters
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 */
typedef struct {
	void *wmi_handle;
	void *evt_buf;
} wma_process_fw_event_params;

int wma_process_fw_event_handler(void *ctx, void *ev, uint8_t rx_ctx);

A_UINT32 e_csr_auth_type_to_rsn_authmode(eCsrAuthType authtype,
					 eCsrEncryptionType encr);
A_UINT32 e_csr_encryption_type_to_rsn_cipherset(eCsrEncryptionType encr);

/*
 * WMA-DFS Hooks
 */
int ol_if_dfs_attach(struct ieee80211com *ic, void *ptr, void *radar_info);
uint64_t ol_if_get_tsf64(struct ieee80211com *ic);
int ol_if_dfs_disable(struct ieee80211com *ic);
struct dfs_ieee80211_channel *ieee80211_find_channel(struct ieee80211com *ic,
						 int freq, uint32_t flags);
int ol_if_dfs_enable(struct ieee80211com *ic, int *is_fastclk, void *pe);
uint32_t ieee80211_ieee2mhz(uint32_t chan, uint32_t flags);
int ol_if_dfs_get_ext_busy(struct ieee80211com *ic);
int ol_if_dfs_get_mib_cycle_counts_pct(struct ieee80211com *ic,
				       uint32_t *rxc_pcnt, uint32_t *rxf_pcnt,
				       uint32_t *txf_pcnt);
uint16_t ol_if_dfs_usenol(struct ieee80211com *ic);
void ieee80211_mark_dfs(struct ieee80211com *ic,
			struct dfs_ieee80211_channel *ichan);
int wma_dfs_indicate_radar(struct ieee80211com *ic,
			   struct dfs_ieee80211_channel *ichan);
uint16_t dfs_usenol(struct ieee80211com *ic);

QDF_STATUS wma_trigger_uapsd_params(tp_wma_handle wma_handle, uint32_t vdev_id,
				    tp_wma_trigger_uapsd_params
				    trigger_uapsd_params);

/* added to get average snr for both data and beacon */
QDF_STATUS wma_send_snr_request(tp_wma_handle wma_handle, void *pGetRssiReq);


QDF_STATUS wma_update_vdev_tbl(tp_wma_handle wma_handle, uint8_t vdev_id,
			       ol_txrx_vdev_handle tx_rx_vdev_handle,
			       uint8_t *mac, uint32_t vdev_type, bool add_del);

void wma_send_flush_logs_to_fw(tp_wma_handle wma_handle);
void wma_log_completion_timeout(void *data);

QDF_STATUS wma_set_rssi_monitoring(tp_wma_handle wma,
					struct rssi_monitor_req *req);

QDF_STATUS wma_send_pdev_set_pcl_cmd(tp_wma_handle wma_handle,
		struct wmi_pcl_chan_weights *msg);

QDF_STATUS wma_send_pdev_set_hw_mode_cmd(tp_wma_handle wma_handle,
		struct sir_hw_mode *msg);
QDF_STATUS wma_get_scan_id(uint32_t *scan_id);

QDF_STATUS wma_send_pdev_set_dual_mac_config(tp_wma_handle wma_handle,
		struct sir_dual_mac_config *msg);
QDF_STATUS wma_send_pdev_set_antenna_mode(tp_wma_handle wma_handle,
		struct sir_antenna_mode_param *msg);
QDF_STATUS wma_crash_inject(tp_wma_handle wma_handle, uint32_t type,
			uint32_t delay_time_ms);

struct wma_target_req *wma_fill_vdev_req(tp_wma_handle wma,
					 uint8_t vdev_id,
					 uint32_t msg_type, uint8_t type,
					 void *params, uint32_t timeout);
struct wma_target_req *wma_fill_hold_req(tp_wma_handle wma,
				    uint8_t vdev_id, uint32_t msg_type,
				    uint8_t type, void *params,
				    uint32_t timeout);

QDF_STATUS wma_vdev_start(tp_wma_handle wma,
			  struct wma_vdev_start_req *req, bool isRestart);

void wma_remove_vdev_req(tp_wma_handle wma, uint8_t vdev_id,
				uint8_t type);

int wmi_desc_pool_init(tp_wma_handle wma_handle, uint32_t pool_size);
void wmi_desc_pool_deinit(tp_wma_handle wma_handle);
struct wmi_desc_t *wmi_desc_get(tp_wma_handle wma_handle);
void wmi_desc_put(tp_wma_handle wma_handle, struct wmi_desc_t *wmi_desc);
int wma_mgmt_tx_completion_handler(void *handle, uint8_t *cmpl_event_params,
				   uint32_t len);
int wma_mgmt_tx_bundle_completion_handler(void *handle,
	uint8_t *cmpl_event_params, uint32_t len);
void wma_set_dfs_region(tp_wma_handle wma, uint8_t dfs_region);
uint32_t wma_get_vht_ch_width(void);
QDF_STATUS
wma_config_debug_module_cmd(wmi_unified_t wmi_handle, A_UINT32 param,
		A_UINT32 val, A_UINT32 *module_id_bitmap,
		A_UINT32 bitmap_len);
#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS wma_set_gateway_params(tp_wma_handle wma,
					struct gateway_param_update_req *req);
#else
static inline QDF_STATUS wma_set_gateway_params(tp_wma_handle wma,
					struct gateway_param_update_req *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_LFR_SUBNET_DETECTION */

#if defined(FEATURE_LRO)
QDF_STATUS wma_lro_config_cmd(tp_wma_handle wma_handle,
	 struct wma_lro_config_cmd_t *wma_lro_cmd);
#else
static inline QDF_STATUS wma_lro_config_cmd(tp_wma_handle wma_handle,
	 struct wma_lro_config_cmd_t *wma_lro_cmd)
{
	return QDF_STATUS_SUCCESS;
}
#endif
void
wma_indicate_err(enum ol_rx_err_type err_type,
	 struct ol_error_info *err_info);

QDF_STATUS wma_ht40_stop_obss_scan(tp_wma_handle wma_handle,
				int32_t vdev_id);
QDF_STATUS wma_send_ht40_obss_scanind(tp_wma_handle wma,
	struct obss_ht40_scanind *req);

int wma_get_bpf_caps_event_handler(void *handle,
				u_int8_t *cmd_param_info,
				u_int32_t len);
uint32_t wma_get_num_of_setbits_from_bitmask(uint32_t mask);
QDF_STATUS wma_get_bpf_capabilities(tp_wma_handle wma);
QDF_STATUS wma_set_bpf_instructions(tp_wma_handle wma,
			struct sir_bpf_set_offload *bpf_set_offload);
void wma_process_set_pdev_ie_req(tp_wma_handle wma,
		struct set_ie_param *ie_params);
void wma_process_set_pdev_ht_ie_req(tp_wma_handle wma,
		struct set_ie_param *ie_params);
void wma_process_set_pdev_vht_ie_req(tp_wma_handle wma,
		struct set_ie_param *ie_params);
void wma_remove_peer(tp_wma_handle wma, u_int8_t *bssid,
			u_int8_t vdev_id, ol_txrx_peer_handle peer,
			bool roam_synch_in_progress);

QDF_STATUS wma_add_wow_wakeup_event(tp_wma_handle wma,
					uint32_t vdev_id,
					uint32_t bitmap,
					bool enable);
QDF_STATUS wma_create_peer(tp_wma_handle wma, ol_txrx_pdev_handle pdev,
			   ol_txrx_vdev_handle vdev, u8 peer_addr[6],
			   u_int32_t peer_type, u_int8_t vdev_id,
			   bool roam_synch_in_progress);

#endif
struct wma_ini_config *wma_get_ini_handle(tp_wma_handle wma_handle);
WLAN_PHY_MODE wma_chan_phy_mode(u8 chan, enum phy_ch_width chan_width,
	u8 dot11_mode);

#ifdef FEATURE_OEM_DATA_SUPPORT
QDF_STATUS wma_start_oem_data_req(tp_wma_handle wma_handle,
				  struct oem_data_req *oem_req);
#endif

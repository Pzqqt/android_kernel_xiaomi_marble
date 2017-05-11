/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_MAIN_H)
#define WLAN_HDD_MAIN_H
/**
 * DOC: wlan_hdd_main.h
 *
 * Linux HDD Adapter Type
 */

/*
 * Include files
 */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/cfg80211.h>
#include <qdf_list.h>
#include <qdf_types.h>
#include "sir_mac_prot_def.h"
#include "csr_api.h"
#include <wlan_hdd_assoc.h>
#include <wlan_hdd_wmm.h>
#include <wlan_hdd_cfg.h>
#include <linux/spinlock.h>
#if defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif
#include <wlan_hdd_ftm.h>
#include "wlan_hdd_tdls.h"
#include "wlan_hdd_tsf.h"
#include "wlan_hdd_cfg80211.h"
#include <qdf_defer.h>
#include "sap_api.h"
#include <wlan_hdd_lro.h>
#include "cdp_txrx_flow_ctrl_legacy.h"
#include <cdp_txrx_peer_ops.h>
#include "wlan_hdd_nan_datapath.h"
#include "wlan_tgt_def_config.h"
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include "wlan_pmo_ucfg_api.h"
#ifdef WIFI_POS_CONVERGED
#include "os_if_wifi_pos.h"
#include "wifi_pos_api.h"
#else
#include "wlan_hdd_oemdata.h"
#endif
#include "wlan_hdd_he.h"

/*
 * Preprocessor definitions and constants
 */

/** Number of Tx Queues */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#define NUM_TX_QUEUES 5
#else
#define NUM_TX_QUEUES 4
#endif

/** Length of the TX queue for the netdev */
#define HDD_NETDEV_TX_QUEUE_LEN (3000)

/** Hdd Tx Time out value */
#ifdef LIBRA_LINUX_PC
#define HDD_TX_TIMEOUT          (8000)
#else
#define HDD_TX_TIMEOUT          msecs_to_jiffies(5000)
#endif
/** Hdd Default MTU */
#define HDD_DEFAULT_MTU         (1500)

#ifdef QCA_CONFIG_SMP
#define NUM_CPUS NR_CPUS
#else
#define NUM_CPUS 1
#endif

/**event flags registered net device*/
#define NET_DEVICE_REGISTERED  (0)
#define SME_SESSION_OPENED     (1)
#define INIT_TX_RX_SUCCESS     (2)
#define WMM_INIT_DONE          (3)
#define SOFTAP_BSS_STARTED     (4)
#define DEVICE_IFACE_OPENED    (5)
#define TDLS_INIT_DONE         (6)
#define ACS_PENDING            (7)
#define SOFTAP_INIT_DONE       (8)

/* Waiting for event for vendor acs */
#define VENDOR_ACS_RESPONSE_PENDING   (8)

/* HDD global event flags */
#define ACS_IN_PROGRESS        (0)

/** Maximum time(ms)to wait for disconnect to complete **/
#ifdef QCA_WIFI_3_0_EMU
#define WLAN_WAIT_TIME_DISCONNECT  5000
#else
#define WLAN_WAIT_TIME_DISCONNECT  5000
#endif
#define WLAN_WAIT_TIME_STATS       800
#define WLAN_WAIT_TIME_POWER       800
#define WLAN_WAIT_TIME_COUNTRY     1000
#define WLAN_WAIT_TIME_LINK_STATUS 800
#define WLAN_WAIT_TIME_POWER_STATS 800
/* Amount of time to wait for sme close session callback.
 * This value should be larger than the timeout used by WDI to wait for
 * a response from WCNSS
 */
#define WLAN_WAIT_TIME_SESSIONOPENCLOSE  15000
#define WLAN_WAIT_TIME_ABORTSCAN         2000

/** Maximum time(ms) to wait for mc thread suspend **/
#define WLAN_WAIT_TIME_MCTHREAD_SUSPEND  1200

/** Maximum time(ms) to wait for target to be ready for suspend **/
#define WLAN_WAIT_TIME_READY_TO_SUSPEND  2000

/** Maximum time(ms) to wait for tdls add sta to complete **/
#define WAIT_TIME_TDLS_ADD_STA      1500

/** Maximum time(ms) to wait for tdls del sta to complete **/
#define WAIT_TIME_TDLS_DEL_STA      1500

/** Maximum time(ms) to wait for Link Establish Req to complete **/
#define WAIT_TIME_TDLS_LINK_ESTABLISH_REQ      1500

/** Maximum time(ms) to wait for tdls mgmt to complete **/
#define WAIT_TIME_TDLS_MGMT         11000

/* Scan Req Timeout */
#define WLAN_WAIT_TIME_SCAN_REQ 100

#define WLAN_WAIT_TIME_ANTENNA_MODE_REQ 3000
#define WLAN_WAIT_TIME_SET_DUAL_MAC_CFG 1500

#define WLAN_WAIT_TIME_BPF     1000

/* Maximum time(ms) to wait for RSO CMD status event */
#define WAIT_TIME_RSO_CMD_STATUS 2000

#define MAX_NUMBER_OF_ADAPTERS 4

#define MAX_CFG_STRING_LEN  255

/* Maximum time(ms) to wait for external acs response */
#define WLAN_VENDOR_ACS_WAIT_TIME 1000

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
/** Mac Address string **/
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDRESS_STR_LEN 18  /* Including null terminator */
#define MAX_GENIE_LEN 512

#define WLAN_CHIP_VERSION   "WCNSS"

#define hdd_log(level, args...) QDF_TRACE(QDF_MODULE_ID_HDD, level, ## args)
#define hdd_logfl(level, format, args...) hdd_log(level, FL(format), ## args)

#define hdd_alert(format, args...) \
		hdd_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define hdd_err(format, args...) \
		hdd_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define hdd_warn(format, args...) \
		hdd_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define hdd_notice(format, args...) \
		hdd_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define hdd_info(format, args...) \
		hdd_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define hdd_debug(format, args...) \
		hdd_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define ENTER() hdd_logfl(QDF_TRACE_LEVEL_INFO_LOW, "enter")
#define ENTER_DEV(dev) \
		hdd_logfl(QDF_TRACE_LEVEL_INFO_LOW, "enter(%s)", (dev)->name)
#define EXIT() hdd_logfl(QDF_TRACE_LEVEL_INFO_LOW, "exit")

#define WLAN_HDD_GET_PRIV_PTR(__dev__) (hdd_adapter_t *)(netdev_priv((__dev__)))

#define MAX_NO_OF_2_4_CHANNELS 14

#define WLAN_HDD_PUBLIC_ACTION_FRAME 4
#define WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET 24
#define WLAN_HDD_PUBLIC_ACTION_FRAME_BODY_OFFSET 24
#define WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET 30
#define WLAN_HDD_PUBLIC_ACTION_FRAME_CATEGORY_OFFSET 0
#define WLAN_HDD_PUBLIC_ACTION_FRAME_ACTION_OFFSET 1
#define WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_OFFSET 2
#define WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_TYPE_OFFSET 5
#define WLAN_HDD_VENDOR_SPECIFIC_ACTION 0x09
#define WLAN_HDD_WFA_OUI   0x506F9A
#define WLAN_HDD_WFA_P2P_OUI_TYPE 0x09
#define WLAN_HDD_P2P_SOCIAL_CHANNELS 3
#define WLAN_HDD_P2P_SINGLE_CHANNEL_SCAN 1
#define WLAN_HDD_PUBLIC_ACTION_FRAME_SUB_TYPE_OFFSET 6

#define WLAN_HDD_IS_SOCIAL_CHANNEL(center_freq)	\
	(((center_freq) == 2412) || ((center_freq) == 2437) || \
	((center_freq) == 2462))

#define WLAN_HDD_CHANNEL_IN_UNII_1_BAND(center_freq) \
	(((center_freq) == 5180) || ((center_freq) == 5200) \
	 || ((center_freq) == 5220) || ((center_freq) == 5240))

#ifdef WLAN_FEATURE_11W
#define WLAN_HDD_SA_QUERY_ACTION_FRAME 8
#endif

#define WLAN_HDD_PUBLIC_ACTION_TDLS_DISC_RESP 14
#define WLAN_HDD_TDLS_ACTION_FRAME 12

#define WLAN_HDD_QOS_ACTION_FRAME 1
#define WLAN_HDD_QOS_MAP_CONFIGURE 4
#define HDD_SAP_WAKE_LOCK_DURATION 10000        /* in msecs */

#if defined(CONFIG_HL_SUPPORT)
#define HDD_MOD_EXIT_SSR_MAX_RETRIES 200
#else
#define HDD_MOD_EXIT_SSR_MAX_RETRIES 75
#endif

#define MAX_USER_COMMAND_SIZE 4096

#define HDD_MIN_TX_POWER (-100) /* minimum tx power */
#define HDD_MAX_TX_POWER (+100) /* maximum tx power */

/* FW expects burst duration in 1020*ms */
#define SIFS_BURST_DUR_MULTIPLIER 1020
#define SIFS_BURST_DUR_MAX        12240

/* If IPA UC data path is enabled, target should reserve extra tx descriptors
 * for IPA data path.
 * Then host data path should allow less TX packet pumping in case
 * IPA data path enabled
 */
#define WLAN_TFC_IPAUC_TX_DESC_RESERVE   100

/*
 * NET_NAME_UNKNOWN is only introduced after Kernel 3.17, to have a macro
 * here if the Kernel version is less than 3.17 to avoid the interleave
 * conditional compilation.
 */
#if !((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)) || defined(WITH_BACKPORTS))
#define NET_NAME_UNKNOWN	0
#endif

#define PRE_CAC_SSID "pre_cac_ssid"

/* session ID invalid */
#define HDD_SESSION_ID_INVALID    0xFF

#define SCAN_REJECT_THRESHOLD_TIME 300000 /* Time is in msec, equal to 5 mins */

/* Default Psoc id */
#define DEFAULT_PSOC_ID 1

/*
 * Generic asynchronous request/response support
 *
 * Many of the APIs supported by HDD require a call to SME to
 * perform an action or to retrieve some data.  In most cases SME
 * performs the operation asynchronously, and will execute a provided
 * callback function when the request has completed.  In order to
 * synchronize this the HDD API allocates a context which is then
 * passed to SME, and which is then, in turn, passed back to the
 * callback function when the operation completes.  The callback
 * function then sets a completion variable inside the context which
 * the HDD API is waiting on.  In an ideal world the HDD API would
 * wait forever (or at least for a long time) for the response to be
 * received and for the completion variable to be set.  However in
 * most cases these HDD APIs are being invoked in the context of a
 * user space thread which has invoked either a cfg80211 API or a
 * wireless extensions ioctl and which has taken the kernel rtnl_lock.
 * Since this lock is used to synchronize many of the kernel tasks, we
 * do not want to hold it for a long time.  In addition we do not want
 * to block user space threads (such as the wpa supplicant's main
 * thread) for an extended time.  Therefore we only block for a short
 * time waiting for the response before we timeout.  This means that
 * it is possible for the HDD API to timeout, and for the callback to
 * be invoked afterwards.  In order for the callback function to
 * determine if the HDD API is still waiting, a magic value is also
 * stored in the shared context.  Only if the context has a valid
 * magic will the callback routine do any work.  In order to further
 * synchronize these activities a spinlock is used so that if any HDD
 * API timeout coincides with its callback, the operations of the two
 * threads will be serialized.
 */

extern spinlock_t hdd_context_lock;

/* MAX OS Q block time value in msec
 * Prevent from permanent stall, resume OS Q if timer expired
 */
#define WLAN_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME 1000
#define WLAN_SAP_HDD_TX_FLOW_CONTROL_OS_Q_BLOCK_TIME 100
#define WLAN_HDD_TX_FLOW_CONTROL_MAX_24BAND_CH   14

#define NUM_TX_RX_HISTOGRAM 128
#define NUM_TX_RX_HISTOGRAM_MASK (NUM_TX_RX_HISTOGRAM - 1)

/**
 * struct hdd_tx_rx_histogram - structure to keep track of tx and rx packets
 *				received over 100ms intervals
 * @interval_rx:	# of rx packets received in the last 100ms interval
 * @interval_tx:	# of tx packets received in the last 100ms interval
 * @next_vote_level:	pld_bus_width_type voting level (high or low)
 *			determined on the basis of total tx and rx packets
 *			received in the last 100ms interval
 * @next_rx_level:	pld_bus_width_type voting level (high or low)
 *			determined on the basis of rx packets received in the
 *			last 100ms interval
 * @next_tx_level:	pld_bus_width_type voting level (high or low)
 *			determined on the basis of tx packets received in the
 *			last 100ms interval
 * @qtime		timestamp when the record is added
 *
 * The structure keeps track of throughput requirements of wlan driver.
 * An entry is added if either of next_vote_level, next_rx_level or
 * next_tx_level changes. An entry is not added for every 100ms interval.
 */
struct hdd_tx_rx_histogram {
	uint64_t interval_rx;
	uint64_t interval_tx;
	uint32_t next_vote_level;
	uint32_t next_rx_level;
	uint32_t next_tx_level;
	uint64_t qtime;
};

typedef struct hdd_tx_rx_stats_s {
	/* start_xmit stats */
	__u32    txXmitCalled;
	__u32    txXmitDropped;
	__u32    txXmitOrphaned;
	__u32    txXmitClassifiedAC[NUM_TX_QUEUES];
	__u32    txXmitDroppedAC[NUM_TX_QUEUES];

	/* rx stats */
	__u32 rxPackets[NUM_CPUS];
	__u32 rxDropped[NUM_CPUS];
	__u32 rxDelivered[NUM_CPUS];
	__u32 rxRefused[NUM_CPUS];

	/* txflow stats */
	bool     is_txflow_paused;
	__u32    txflow_pause_cnt;
	__u32    txflow_unpause_cnt;
	__u32    txflow_timer_cnt;
} hdd_tx_rx_stats_t;

#ifdef WLAN_FEATURE_11W
typedef struct hdd_pmf_stats_s {
	uint8_t numUnprotDeauthRx;
	uint8_t numUnprotDisassocRx;
} hdd_pmf_stats_t;
#endif

typedef struct hdd_stats_s {
	tCsrSummaryStatsInfo summary_stat;
	tCsrGlobalClassAStatsInfo ClassA_stat;
	tCsrGlobalClassDStatsInfo ClassD_stat;
	struct csr_per_chain_rssi_stats_info  per_chain_rssi_stats;
	hdd_tx_rx_stats_t hddTxRxStats;
#ifdef WLAN_FEATURE_11W
	hdd_pmf_stats_t hddPmfStats;
#endif
} hdd_stats_t;

enum hdd_roam_state {
	HDD_ROAM_STATE_NONE,

	/* Issuing a disconnect due to transition into low power states */
	HDD_ROAM_STATE_DISCONNECTING_POWER,

	/* Move to this state when HDD sets a key with SME/CSR.  Note this is
	 * an important state to get right because we will get calls into our
	 * SME callback routine for SetKey activity that we did not initiate!
	 */
	HDD_ROAM_STATE_SETTING_KEY,
};

typedef struct roaming_info_s {
	enum hdd_roam_state roamingState;
	qdf_event_t roamingEvent;

	tSirMacAddr bssid;
	tSirMacAddr peerMac;
	uint32_t roamId;
	eRoamCmdStatus roamStatus;
	bool deferKeyComplete;

} roaming_info_t;

#ifdef FEATURE_WLAN_WAPI
/* Define WAPI macros for Length, BKID count etc*/
#define MAX_WPI_KEY_LENGTH    16
#define MAX_NUM_PN            16
#define MAC_ADDR_LEN           6
#define MAX_ADDR_INDEX        12
#define MAX_NUM_AKM_SUITES    16
#define MAX_NUM_UNI_SUITES    16
#define MAX_NUM_BKIDS         16

/** WAPI AUTH mode definition */
enum wapi_auth_mode {
	WAPI_AUTH_MODE_OPEN = 0,
	WAPI_AUTH_MODE_PSK = 1,
	WAPI_AUTH_MODE_CERT
} __packed;

/** WAPI Work mode structure definition */
#define   WZC_ORIGINAL      0
#define   WAPI_EXTENTION    1

struct wapi_function_mode {
	unsigned char wapiMode;
} __packed;

typedef struct _WAPI_BKID {
	uint8_t bkid[16];
} WAPI_BKID, *pWAPI_BKID;

/** WAPI Association information structure definition */
struct _WAPI_AssocInfo {
	uint8_t elementID;
	uint8_t length;
	uint16_t version;
	uint16_t akmSuiteCount;
	uint32_t akmSuite[MAX_NUM_AKM_SUITES];
	uint16_t unicastSuiteCount;
	uint32_t unicastSuite[MAX_NUM_UNI_SUITES];
	uint32_t multicastSuite;
	uint16_t wapiCability;
	uint16_t bkidCount;
	WAPI_BKID bkidList[MAX_NUM_BKIDS];
} __packed;

typedef struct _WAPI_AssocInfo WAPI_AssocInfo;
typedef struct _WAPI_AssocInfo *pWAPI_IEAssocInfo;

/** WAPI KEY Type definition */
enum _WAPIKeyType {
	PAIRWISE_KEY,           /* 0 */
	GROUP_KEY               /* 1 */
} __packed;

/** WAPI KEY Direction definition */
enum key_direction {
	None,
	Rx,
	Tx,
	Rx_Tx
} __packed;

/* WAPI KEY structure definition */
struct WLAN_WAPI_KEY {
	enum _WAPIKeyType keyType;
	enum key_direction keyDirection;        /*reserved for future use */
	uint8_t keyId;
	uint8_t addrIndex[MAX_ADDR_INDEX];      /*reserved for future use */
	int wpiekLen;
	uint8_t wpiek[MAX_WPI_KEY_LENGTH];
	int wpickLen;
	uint8_t wpick[MAX_WPI_KEY_LENGTH];
	uint8_t pn[MAX_NUM_PN]; /*reserved for future use */
} __packed;

typedef struct WLAN_WAPI_KEY WLAN_WAPI_KEY;
typedef struct WLAN_WAPI_KEY *pWLAN_WAPI_KEY;

#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WPA_GET_BE24(a) ((u32) ((a[0] << 16) | (a[1] << 8) | a[2]))
#define WLAN_EID_WAPI 68
#define WAPI_PSK_AKM_SUITE  0x02721400
#define WAPI_CERT_AKM_SUITE 0x01721400

/* WAPI BKID List structure definition */
struct _WLAN_BKID_LIST {
	uint32_t length;
	uint32_t BKIDCount;
	WAPI_BKID BKID[1];
} __packed;

typedef struct _WLAN_BKID_LIST WLAN_BKID_LIST;
typedef struct _WLAN_BKID_LIST *pWLAN_BKID_LIST;

/* WAPI Information structure definition */
struct hdd_wapi_info_s {
	uint32_t nWapiMode;
	bool fIsWapiSta;
	struct qdf_mac_addr cachedMacAddr;
	uint8_t wapiAuthMode;
} __packed;
typedef struct hdd_wapi_info_s hdd_wapi_info_t;
#endif /* FEATURE_WLAN_WAPI */

typedef struct beacon_data_s {
	u8 *head;
	u8 *tail;
	u8 *proberesp_ies;
	u8 *assocresp_ies;
	int head_len;
	int tail_len;
	int proberesp_ies_len;
	int assocresp_ies_len;
	int dtim_period;
} beacon_data_t;

enum rem_on_channel_request_type {
	REMAIN_ON_CHANNEL_REQUEST,
	OFF_CHANNEL_ACTION_TX,
};

typedef struct action_pkt_buffer {
	uint8_t *frame_ptr;
	uint32_t frame_length;
	uint16_t freq;
} action_pkt_buffer_t;

typedef struct hdd_remain_on_chan_ctx {
	struct net_device *dev;
	struct ieee80211_channel chan;
	enum nl80211_channel_type chan_type;
	unsigned int duration;
	u64 cookie;
	enum rem_on_channel_request_type rem_on_chan_request;
	qdf_mc_timer_t hdd_remain_on_chan_timer;
	action_pkt_buffer_t action_pkt_buff;
	bool hdd_remain_on_chan_cancel_in_progress;
	uint32_t scan_id;
} hdd_remain_on_chan_ctx_t;

/* RoC Request entry */
typedef struct hdd_roc_req {
	qdf_list_node_t node;   /* MUST be first element */
	hdd_adapter_t *pAdapter;
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
} hdd_roc_req_t;

/**
 * struct hdd_scan_req - Scan Request entry
 * @node : List entry element
 * @adapter: Adapter address
 * @scan_request: scan request holder
 * @scan_id: scan identifier used across host layers which is generated at WMI
 * @cookie: scan request identifier sent to userspace
 * @source: scan request originator (NL/Vendor scan)
 * @timestamp: scan request timestamp
 *
 * Scan request linked list element
 */
struct hdd_scan_req {
	qdf_list_node_t node;
	hdd_adapter_t *adapter;
	struct cfg80211_scan_request *scan_request;
	uint32_t scan_id;
	uint8_t source;
	uint32_t timestamp;
};

enum p2p_action_frame_state {
	HDD_IDLE,
	HDD_PD_REQ_ACK_PENDING,
	HDD_GO_NEG_REQ_ACK_PENDING,
	HDD_INVALID_STATE,
};

enum action_frm_type {
	WLAN_HDD_GO_NEG_REQ,
	WLAN_HDD_GO_NEG_RESP,
	WLAN_HDD_GO_NEG_CNF,
	WLAN_HDD_INVITATION_REQ,
	WLAN_HDD_INVITATION_RESP,
	WLAN_HDD_DEV_DIS_REQ,
	WLAN_HDD_DEV_DIS_RESP,
	WLAN_HDD_PROV_DIS_REQ,
	WLAN_HDD_PROV_DIS_RESP,
};

typedef struct hdd_cfg80211_state_s {
	uint16_t current_freq;
	u64 action_cookie;
	uint8_t *buf;
	size_t len;
	hdd_remain_on_chan_ctx_t *remain_on_chan_ctx;
	struct mutex remain_on_chan_ctx_lock;
	enum p2p_action_frame_state actionFrmState;
	/* is_go_neg_ack_received flag is set to 1 when
	 * the pending ack for GO negotiation req is
	 * received.
	 */
	bool is_go_neg_ack_received;
} hdd_cfg80211_state_t;

/**
 * struct hdd_mon_set_ch_info - Holds monitor mode channel switch params
 * @channel: Channel number.
 * @cb_mode: Channel bonding
 * @channel_width: Channel width 0/1/2 for 20/40/80MHz respectively.
 * @phy_mode: PHY mode
 */
struct hdd_mon_set_ch_info {
	uint8_t channel;
	uint8_t cb_mode;
	uint32_t channel_width;
	eCsrPhyMode phy_mode;
};

struct hdd_station_ctx {
	/* Handle to the Wireless Extension State */
	hdd_wext_state_t WextState;

#ifdef FEATURE_WLAN_TDLS
	tdlsCtx_t *pHddTdlsCtx;
#endif

	/* Connection information*/
	connection_info_t conn_info;

	roaming_info_t roam_info;

	int ft_carrier_on;

	/* Increment whenever ibss New peer joins and departs the network */
	int ibss_sta_generation;

	/* Indication of wep/wpa-none keys installation */
	bool ibss_enc_key_installed;

	/* Save the wep/wpa-none keys */
	tCsrRoamSetKey ibss_enc_key;
	tSirPeerInfoRspParams ibss_peer_info;

	bool hdd_ReassocScenario;

	/* STA ctx debug variables */
	int staDebugState;

	uint8_t broadcast_staid;

	struct hdd_mon_set_ch_info ch_info;
#if defined(WLAN_FEATURE_NAN_DATAPATH) && !defined(WLAN_FEATURE_NAN_CONVERGENCE)
	struct nan_datapath_ctx ndp_ctx;
#endif
	bool ap_supports_immediate_power_save;
};

#define BSS_STOP    0
#define BSS_START   1
typedef struct hdd_hostapd_state_s {
	int bssState;
	qdf_event_t qdf_event;
	qdf_event_t qdf_stop_bss_event;
	qdf_event_t qdf_sta_disassoc_event;
	QDF_STATUS qdf_status;
	bool bCommit;

} hdd_hostapd_state_t;

/**
 * enum bss_stop_reason - reasons why a BSS is stopped.
 * @BSS_STOP_REASON_INVALID: no reason specified explicitly.
 * @BSS_STOP_DUE_TO_MCC_SCC_SWITCH: BSS stopped due to host
 *  driver is trying to switch AP role to a different channel
 *  to maintain SCC mode with the STA role on the same card.
 *  this usually happens when STA is connected to an external
 *  AP that runs on a different channel
 */
enum bss_stop_reason {
	BSS_STOP_REASON_INVALID = 0,
	BSS_STOP_DUE_TO_MCC_SCC_SWITCH = 1,
};

/*
 * Per station structure kept in HDD for multiple station support for SoftAP
 */
typedef struct {
	/** The station entry is used or not  */
	bool isUsed;

	/* Station ID reported back from HAL (through SAP).
	 * Broadcast uses station ID zero by default
	 */
	uint8_t ucSTAId;

	/* type of station i.e. p2p client or infrastructure station */
	eStationType staType;

	/** MAC address of the station */
	struct qdf_mac_addr macAddrSTA;

	/** Current Station state so HDD knows how to deal with packet
	 *  queue. Most recent states used to change TLSHIM STA state
	 */
	enum ol_txrx_peer_state tlSTAState;

	/** Track QoS status of station */
	bool isQosEnabled;

	/** The station entry for which Deauth is in progress  */
	bool isDeauthInProgress;

	/** Number of spatial streams supported */
	uint8_t   nss;

	/** Rate Flags for this connection */
	uint32_t  rate_flags;
} hdd_station_info_t;

struct hdd_ap_ctx_s {
	hdd_hostapd_state_t HostapdState;

	/* Memory differentiation mode is enabled */
	/* uint16_t uMemoryDiffThreshold; */
	/* uint8_t uNumActiveAC; */
	/* uint8_t uActiveACMask; */

	/** Packet Count to update uNumActiveAC and uActiveACMask */
	/* uint16_t uUpdatePktCount; */

	/** Station ID assigned after BSS starts */
	uint8_t uBCStaId;

	uint8_t uPrivacy;       /* The privacy bits of configuration */

	tSirWPSPBCProbeReq WPSPBCProbeReq;

	tsap_Config_t sapConfig;

	struct semaphore semWpsPBCOverlapInd;

	bool apDisableIntraBssFwd;

	qdf_mc_timer_t hdd_ap_inactivity_timer;

	uint8_t operatingChannel;

	bool uIsAuthenticated;

	eCsrEncryptionType ucEncryptType;

	/* This will point to group key data,
	 * if it is received before start bss
	 */
	tCsrRoamSetKey groupKey;
	/* This will have WEP key data, if it is received before start bss */
	tCsrRoamSetKey wepKey[CSR_MAX_NUM_KEY];

	/* WEP default key index */
	uint8_t wep_def_key_idx;

	beacon_data_t *beacon;

	bool bApActive;

	/* SAP Context */
	void *sapContext;

	bool dfs_cac_block_tx;
	qdf_mc_timer_t vendor_acs_timer;
	bool vendor_acs_timer_initialized;

	enum bss_stop_reason bss_stop_reason;
};

typedef struct hdd_scaninfo_s {
	/* The scan pending  */
	uint32_t mScanPending;

	/* Counter for mScanPending so that the scan pending
	 * error log is not printed for more than 5 times
	 */
	uint32_t mScanPendingCounter;

	/* Additional IE for scan */
	tSirAddie scanAddIE;

	uint8_t *default_scan_ies;
	uint8_t default_scan_ies_len;
	/* Scan mode */
	tSirScanType scan_mode;

	/* completion variable for abortscan */
	struct completion abortscan_event_var;

} hdd_scaninfo_t;

#define WLAN_HDD_MAX_MC_ADDR_LIST CFG_TGT_MAX_MULTICAST_FILTER_ENTRIES

#ifdef WLAN_FEATURE_PACKET_FILTERING
typedef struct multicast_addr_list {
	uint8_t isFilterApplied;
	uint8_t mc_cnt;
	uint8_t addr[WLAN_HDD_MAX_MC_ADDR_LIST][ETH_ALEN];
} t_multicast_add_list;
#endif

#define WLAN_HDD_MAX_HISTORY_ENTRY		10

/**
 * struct hdd_netif_queue_stats - netif queue operation statistics
 * @pause_count - pause counter
 * @unpause_count - unpause counter
 */
struct hdd_netif_queue_stats {
	uint16_t pause_count;
	uint16_t unpause_count;
	qdf_time_t total_pause_time;
};

/**
 * struct hdd_netif_queue_history - netif queue operation history
 * @time: timestamp
 * @netif_action: action type
 * @netif_reason: reason type
 * @pause_map: pause map
 */
struct hdd_netif_queue_history {
	qdf_time_t time;
	uint16_t netif_action;
	uint16_t netif_reason;
	uint32_t pause_map;
};

/**
 * struct hdd_chan_change_params - channel related information
 * @chan: operating channel
 * @chan_params: channel parameters
 */
struct hdd_chan_change_params {
	uint8_t chan;
	struct ch_params chan_params;
};

/**
 * struct hdd_runtime_pm_context - context to prevent/allow runtime pm
 * @scan: scan context to prvent/allow runtime pm
 *
 * Prevent Runtime PM for scan
 */
struct hdd_runtime_pm_context {
	qdf_runtime_lock_t roc;
	qdf_runtime_lock_t dfs;
};

/**
 * struct hdd_connect_pm_context - Runtime PM connect context per adapter
 * @connect: Runtime Connect Context
 *
 * Structure to hold runtime pm connect context for each adapter.
 */
struct hdd_connect_pm_context {
	qdf_runtime_lock_t connect;
};

/*
 * WLAN_HDD_ADAPTER_MAGIC is a magic number used to identify net devices
 * belonging to this driver from net devices belonging to other devices.
 * Therefore, the magic number must be unique relative to the numbers for
 * other drivers in the system. If WLAN_HDD_ADAPTER_MAGIC is already defined
 * (e.g. by compiler argument), then use that. If it's not already defined,
 * then use the first 4 characters of MULTI_IF_NAME to construct the magic
 * number. If MULTI_IF_NAME is not defined, then use a default magic number.
 */
#ifndef WLAN_HDD_ADAPTER_MAGIC
#ifdef MULTI_IF_NAME
#define WLAN_HDD_ADAPTER_MAGIC                                          \
	(MULTI_IF_NAME[0] == 0 ? 0x574c414e :                           \
	(MULTI_IF_NAME[1] == 0 ? (MULTI_IF_NAME[0] << 24) :             \
	(MULTI_IF_NAME[2] == 0 ? (MULTI_IF_NAME[0] << 24) |             \
		(MULTI_IF_NAME[1] << 16) :                              \
	(MULTI_IF_NAME[0] << 24) | (MULTI_IF_NAME[1] << 16) |           \
	(MULTI_IF_NAME[2] << 8) | MULTI_IF_NAME[3])))
#else
#define WLAN_HDD_ADAPTER_MAGIC 0x574c414e       /* ASCII "WLAN" */
#endif
#endif

struct hdd_adapter_s {
	/* Magic cookie for adapter sanity verification.  Note that this
	 * needs to be at the beginning of the private data structure so
	 * that it will exists at the beginning of dev->priv and hence
	 * will always be in mapped memory
	 */
	uint32_t magic;

	void *pHddCtx;
	struct wlan_objmgr_vdev *hdd_vdev;

	void *txrx_vdev;

	/** Handle to the network device */
	struct net_device *dev;

	enum tQDF_ADAPTER_MODE device_mode;

	/** IPv4 notifier callback for handling ARP offload on change in IP */
	struct work_struct ipv4NotifierWorkQueue;
#ifdef WLAN_NS_OFFLOAD
	/** IPv6 notifier callback for handling NS offload on change in IP */
	struct work_struct ipv6NotifierWorkQueue;
#endif

	/* TODO Move this to sta Ctx */
	struct wireless_dev wdev;
	struct cfg80211_scan_request *request;
	uint8_t scan_source;

	/** ops checks if Opportunistic Power Save is Enable or Not
	 * ctw stores ctWindow value once we receive Opps command from
	 * wpa_supplicant then using ctWindow value we need to Enable
	 * Opportunistic Power Save
	 */
	uint8_t ops;
	uint32_t ctw;

	/** Current MAC Address for the adapter  */
	struct qdf_mac_addr macAddressCurrent;

	/**Event Flags*/
	unsigned long event_flags;

	/**Device TX/RX statistics*/
	struct net_device_stats stats;
	/** HDD statistics*/
	hdd_stats_t hdd_stats;

	/* estimated link speed */
	uint32_t estimated_linkspeed;

	uint8_t sessionId;

	/* Completion variable for session close */
	struct completion session_close_comp_var;

	/* Completion variable for session open */
	struct completion session_open_comp_var;

	/* TODO: move these to sta ctx. These may not be used in AP */
	/** completion variable for disconnect callback */
	struct completion disconnect_comp_var;

	/** Completion of change country code */
	struct completion change_country_code;

	/* completion variable for Linkup Event */
	struct completion linkup_event_var;

	/* completion variable for cancel remain on channel Event */
	struct completion cancel_rem_on_chan_var;

	/* completion variable for off channel  remain on channel Event */
	struct completion offchannel_tx_event;
	/* Completion variable for action frame */
	struct completion tx_action_cnf_event;
	/* Completion variable for remain on channel ready */
	struct completion rem_on_chan_ready_event;

	struct completion sta_authorized_event;
#ifdef FEATURE_WLAN_TDLS
	struct completion tdls_add_station_comp;
	struct completion tdls_del_station_comp;
	struct completion tdls_mgmt_comp;
	struct completion tdls_link_establish_req_comp;
	QDF_STATUS tdlsAddStaStatus;
#endif

	struct completion ibss_peer_info_comp;

	/* Track whether the linkup handling is needed  */
	bool isLinkUpSvcNeeded;

	/* Mgmt Frames TX completion status code */
	uint32_t mgmtTxCompletionStatus;

	/* WMM Status */
	struct hdd_wmm_status hddWmmStatus;
/*************************************************************
 */
/*************************************************************
 * TODO - Remove it later
 */
	/** Multiple station supports */
	/** Per-station structure */
	spinlock_t staInfo_lock;        /* To protect access to station Info */
	hdd_station_info_t aStaInfo[WLAN_MAX_STA_COUNT];
	/* uint8_t uNumActiveStation; */

/*************************************************************
 */

#ifdef FEATURE_WLAN_WAPI
	hdd_wapi_info_t wapi_info;
#endif

	int8_t rssi;
	int32_t rssi_on_disconnect;
#ifdef WLAN_FEATURE_LPSS
	bool rssi_send;
#endif

	uint8_t snr;

	struct work_struct monTxWorkQueue;
	struct sk_buff *skb_to_tx;

	union {
		hdd_station_ctx_t station;
		hdd_ap_ctx_t ap;
	} sessionCtx;

#ifdef WLAN_FEATURE_TSF
	/* tsf value received from firmware */
	uint32_t tsf_low;
	uint32_t tsf_high;
	/* TSF capture state */
	enum hdd_tsf_capture_state tsf_state;
	uint64_t tsf_sync_soc_timer;
#endif

	hdd_cfg80211_state_t cfg80211State;

#ifdef WLAN_FEATURE_PACKET_FILTERING
	t_multicast_add_list mc_addr_list;
#endif
	uint8_t addr_filter_pattern;

	bool higherDtimTransition;
	bool survey_idx;

	hdd_scaninfo_t scan_info;

	/* Flag to ensure PSB is configured through framework */
	uint8_t psbChanged;
	/* UAPSD psb value configured through framework */
	uint8_t configuredPsb;
#ifdef IPA_OFFLOAD
	void *ipa_context;
#endif
	/* Use delayed work for Sec AP ACS as Pri AP Startup need to complete
	 * since CSR (PMAC Struct) Config is same for both AP
	 */
	struct delayed_work acs_pending_work;

	struct work_struct scan_block_work;
#ifdef MSM_PLATFORM
	unsigned long prev_rx_packets;
	unsigned long prev_tx_packets;
	uint64_t prev_fwd_tx_packets;
	uint64_t prev_fwd_rx_packets;
	int connection;
#endif
	bool is_roc_inprogress;

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	qdf_mc_timer_t tx_flow_control_timer;
	bool tx_flow_timer_initialized;
	unsigned int tx_flow_low_watermark;
	unsigned int tx_flow_high_watermark_offset;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
	bool offloads_configured;

	/* DSCP to UP QoS Mapping */
	sme_QosWmmUpType hddWmmDscpToUpMap[WLAN_HDD_MAX_DSCP + 1];

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	bool isLinkLayerStatsSet;
#endif
	uint8_t linkStatus;

	/* variable for temperature in Celsius */
	int temperature;

	/* Time stamp for last completed RoC request */
	uint64_t last_roc_ts;

	/* Time stamp for start RoC request */
	uint64_t start_roc_ts;

#ifdef WLAN_FEATURE_DSRC
	/* MAC addresses used for OCB interfaces */
	struct qdf_mac_addr ocb_mac_address[QDF_MAX_CONCURRENCY_PERSONA];
	int ocb_mac_addr_count;
#endif

	/* BITMAP indicating pause reason */
	uint32_t pause_map;
	spinlock_t pause_map_lock;
	qdf_time_t start_time;
	qdf_time_t last_time;
	qdf_time_t total_pause_time;
	qdf_time_t total_unpause_time;
	uint8_t history_index;
	struct hdd_netif_queue_history
		 queue_oper_history[WLAN_HDD_MAX_HISTORY_ENTRY];
	struct hdd_netif_queue_stats queue_oper_stats[WLAN_REASON_TYPE_MAX];
	ol_txrx_tx_fp tx_fn;
	/* debugfs entry */
	struct dentry *debugfs_phy;
	/*
	 * The pre cac channel is saved here and will be used when the SAP's
	 * channel needs to be moved from the existing 2.4GHz channel.
	 */
	uint8_t pre_cac_chan;
	struct hdd_connect_pm_context connect_rpm_ctx;

	bool fast_roaming_allowed;
	/*
	 * defer disconnect is used as a flag by roaming to check
	 * if any disconnect has been deferred because of roaming
	 * and handle it. It stores the source of the disconnect.
	 * Based on the source, it will appropriately handle the
	 * disconnect.
	 */
	uint8_t defer_disconnect;
	/*
	 * cfg80211 issues a reason for disconnect. Store this reason if the
	 * disconnect is being deferred.
	 */
	uint8_t cfg80211_disconnect_reason;
	struct lfr_firmware_status lfr_fw_status;
};

/*
 * Below two definitions are useful to distinguish the
 * source of the disconnect when a disconnect is deferred
 */
#define DEFER_DISCONNECT_TRY_DISCONNECT      1
#define DEFER_DISCONNECT_CFG80211_DISCONNECT 2

#define WLAN_HDD_GET_STATION_CTX_PTR(pAdapter) (&(pAdapter)->sessionCtx.station)
#define WLAN_HDD_GET_AP_CTX_PTR(pAdapter) (&(pAdapter)->sessionCtx.ap)
#define WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter)  (&(pAdapter)->sessionCtx.station.WextState)
#define WLAN_HDD_GET_CTX(pAdapter) ((hdd_context_t *)pAdapter->pHddCtx)
#define WLAN_HDD_GET_HAL_CTX(pAdapter)  (((hdd_context_t *)(pAdapter->pHddCtx))->hHal)
#define WLAN_HDD_GET_HOSTAP_STATE_PTR(pAdapter) (&(pAdapter)->sessionCtx.ap.HostapdState)
#define WLAN_HDD_GET_CFG_STATE_PTR(pAdapter)  (&(pAdapter)->cfg80211State)
#define WLAN_HDD_GET_SAP_CTX_PTR(pAdapter) (pAdapter->sessionCtx.ap.sapContext)
#ifdef FEATURE_WLAN_TDLS
#define WLAN_HDD_IS_TDLS_SUPPORTED_ADAPTER(pAdapter) \
	(((QDF_STA_MODE != pAdapter->device_mode) && \
	  (QDF_P2P_CLIENT_MODE != pAdapter->device_mode)) ? 0 : 1)
#define WLAN_HDD_GET_TDLS_CTX_PTR(pAdapter) \
	((WLAN_HDD_IS_TDLS_SUPPORTED_ADAPTER(pAdapter)) ? \
	 (tdlsCtx_t *)(pAdapter)->sessionCtx.station.pHddTdlsCtx : NULL)
#endif

#ifdef WLAN_FEATURE_NAN_DATAPATH
#ifndef WLAN_FEATURE_NAN_CONVERGENCE
#define WLAN_HDD_GET_NDP_CTX_PTR(adapter) \
		(&(adapter)->sessionCtx.station.ndp_ctx)
#endif /* WLAN_FEATURE_NAN_CONVERGENCE */
#define WLAN_HDD_IS_NDP_ENABLED(hdd_ctx) ((hdd_ctx)->nan_datapath_enabled)
#else
/* WLAN_HDD_GET_NDP_CTX_PTR and WLAN_HDD_GET_NDP_WEXT_STATE_PTR are not defined
 * intentionally so that all references to these must be within NDP code.
 * non-NDP code can call WLAN_HDD_IS_NDP_ENABLED(), and when it is enabled,
 * invoke NDP code to do all work.
 */
#define WLAN_HDD_IS_NDP_ENABLED(hdd_ctx) (false)
#endif

/* Set mac address locally administered bit */
#define WLAN_HDD_RESET_LOCALLY_ADMINISTERED_BIT(macaddr) (macaddr[0] &= 0xFD)

#define HDD_DEFAULT_MCC_P2P_QUOTA    70
#define HDD_RESET_MCC_P2P_QUOTA      50

typedef struct hdd_adapter_list_node {
	qdf_list_node_t node;   /* MUST be first element */
	hdd_adapter_t *pAdapter;
} hdd_adapter_list_node_t;

typedef struct hdd_priv_data_s {
	uint8_t *buf;
	int used_len;
	int total_len;
} hdd_priv_data_t;

#define  MAX_MOD_LOGLEVEL 10
typedef struct {
	uint8_t enable;
	uint8_t dl_type;
	uint8_t dl_report;
	uint8_t dl_loglevel;
	uint8_t index;
	uint32_t dl_mod_loglevel[MAX_MOD_LOGLEVEL];

} fw_log_info;

/**
 * enum antenna_mode - number of TX/RX chains
 * @HDD_ANTENNA_MODE_INVALID: Invalid mode place holder
 * @HDD_ANTENNA_MODE_1X1: Number of TX/RX chains equals 1
 * @HDD_ANTENNA_MODE_2X2: Number of TX/RX chains equals 2
 * @HDD_ANTENNA_MODE_MAX: Place holder for max mode
 */
enum antenna_mode {
	HDD_ANTENNA_MODE_INVALID,
	HDD_ANTENNA_MODE_1X1,
	HDD_ANTENNA_MODE_2X2,
	HDD_ANTENNA_MODE_MAX
};

/**
 * enum smps_mode - SM power save mode
 * @HDD_SMPS_MODE_STATIC: Static power save
 * @HDD_SMPS_MODE_DYNAMIC: Dynamic power save
 * @HDD_SMPS_MODE_RESERVED: Reserved
 * @HDD_SMPS_MODE_DISABLED: Disable power save
 * @HDD_SMPS_MODE_MAX: Place holder for max mode
 */
enum smps_mode {
	HDD_SMPS_MODE_STATIC,
	HDD_SMPS_MODE_DYNAMIC,
	HDD_SMPS_MODE_RESERVED,
	HDD_SMPS_MODE_DISABLED,
	HDD_SMPS_MODE_MAX
};

#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
/**
 * struct hdd_offloaded_packets - request id to pattern id mapping
 * @request_id: request id
 * @pattern_id: pattern id
 *
 */
struct hdd_offloaded_packets {
	uint32_t request_id;
	uint8_t  pattern_id;
};

/**
 * struct hdd_offloaded_packets_ctx - offloaded packets context
 * @op_table: request id to pattern id table
 * @op_lock: mutex lock
 */
struct hdd_offloaded_packets_ctx {
	struct hdd_offloaded_packets op_table[MAXNUM_PERIODIC_TX_PTRNS];
	struct mutex op_lock;
};
#endif

/**
 * enum driver_status: Driver Modules status
 * @DRIVER_MODULES_UNINITIALIZED: Driver CDS modules uninitialized
 * @DRIVER_MODULES_OPENED: Driver CDS modules opened
 * @DRIVER_MODULES_ENABLED: Driver CDS modules opened
 * @DRIVER_MODULES_CLOSED: Driver CDS modules closed
 */
enum driver_modules_status {
	DRIVER_MODULES_UNINITIALIZED,
	DRIVER_MODULES_OPENED,
	DRIVER_MODULES_ENABLED,
	DRIVER_MODULES_CLOSED
};

/**
 * struct acs_dfs_policy - Define ACS policies
 * @acs_dfs_mode: Dfs mode enabled/disabled.
 * @acs_channel: pre defined channel to avoid ACS.
 */
struct acs_dfs_policy {
	enum dfs_mode acs_dfs_mode;
	uint8_t acs_channel;
};

/**
 * enum suspend_fail_reason: Reasons a WLAN suspend might fail
 * SUSPEND_FAIL_IPA: IPA in progress
 * SUSPEND_FAIL_RADAR: radar scan in progress
 * SUSPEND_FAIL_ROAM: roaming in progress
 * SUSPEND_FAIL_SCAN: scan in progress
 * SUSPEND_FAIL_INITIAL_WAKEUP: received initial wakeup from firmware
 * SUSPEND_FAIL_MAX_COUNT: the number of wakeup reasons, always at the end
 */
enum suspend_fail_reason {
	SUSPEND_FAIL_IPA,
	SUSPEND_FAIL_RADAR,
	SUSPEND_FAIL_ROAM,
	SUSPEND_FAIL_SCAN,
	SUSPEND_FAIL_INITIAL_WAKEUP,
	SUSPEND_FAIL_MAX_COUNT
};

/**
 * suspend_resume_stats - Collection of counters for suspend/resume events
 * @suspends: number of suspends completed
 * @resumes: number of resumes completed
 * @suspend_fail: counters for failed suspend reasons
 */
struct suspend_resume_stats {
	uint32_t suspends;
	uint32_t resumes;
	uint32_t suspend_fail[SUSPEND_FAIL_MAX_COUNT];
};

/** Adapter structure definition */
struct hdd_context_s {
	/** Global CDS context  */
	v_CONTEXT_t pcds_context;

	struct wlan_objmgr_psoc *hdd_psoc;
	struct wlan_objmgr_pdev *hdd_pdev;

	/** HAL handle...*/
	tHalHandle hHal;

	struct wiphy *wiphy;
	/* TODO Remove this from here. */

	qdf_spinlock_t hdd_adapter_lock;
	qdf_list_t hddAdapters; /* List of adapters */

	/* One per STA: 1 for BCMC_STA_ID, 1 for each SAP_SELF_STA_ID,
	 * 1 for WDS_STAID
	 */
	hdd_adapter_t *sta_to_adapter[WLAN_MAX_STA_COUNT + QDF_MAX_NO_OF_SAP_MODE + 2];

	/** Pointer for firmware image data */
	const struct firmware *fw;

	/** Pointer for configuration data */
	const struct firmware *cfg;

	/** Pointer to the parent device */
	struct device *parent_dev;

	/** Config values read from qcom_cfg.ini file */
	struct hdd_config *config;

	struct wlan_hdd_ftm_status ftm;

	/* Completion  variable to indicate Mc Thread Suspended */
	struct completion mc_sus_event_var;

	bool is_scheduler_suspended;

#ifdef QCA_CONFIG_SMP
	bool is_ol_rx_thread_suspended;
#endif

	bool hdd_wlan_suspended;
	bool suspended;

	/* Lock to avoid race condition during start/stop bss */
	struct mutex sap_lock;

#ifdef FEATURE_OEM_DATA_SUPPORT
	/* OEM App registered or not */
	bool oem_app_registered;

	/* OEM App Process ID */
	int32_t oem_pid;
#endif

	/** Concurrency Parameters*/
	uint32_t concurrency_mode;

	uint8_t no_of_open_sessions[QDF_MAX_NO_OF_MODE];
	uint8_t no_of_active_sessions[QDF_MAX_NO_OF_MODE];

	/** P2P Device MAC Address for the adapter  */
	struct qdf_mac_addr p2pDeviceAddress;

	qdf_wake_lock_t rx_wake_lock;
	qdf_wake_lock_t sap_wake_lock;

#ifdef FEATURE_WLAN_TDLS
	enum tdls_support_mode tdls_mode;
	enum tdls_support_mode tdls_mode_last;
	tdlsConnInfo_t tdlsConnInfo[HDD_MAX_NUM_TDLS_STA];
	/* maximum TDLS station number allowed upon runtime condition */
	uint16_t max_num_tdls_sta;
	/* TDLS peer connected count */
	uint16_t connected_peer_count;
	tdls_scan_context_t tdls_scan_ctxt;
	/* Lock to avoid race condition during TDLS operations */
	qdf_spinlock_t tdls_ct_spinlock;
	/* linear mac address table for counting the packets */
	struct tdls_ct_mac_table ct_peer_mac_table[TDLS_CT_MAC_MAX_TABLE_SIZE];
	/* number of valid mac entry in @ct_peer_mac_table */
	uint8_t valid_mac_entries;
	struct mutex tdls_lock;
	uint8_t tdls_off_channel;
	uint16_t tdls_channel_offset;
	int32_t tdls_fw_off_chan_mode;
	bool enable_tdls_connection_tracker;
	uint8_t tdls_external_peer_count;
	bool tdls_nss_switch_in_progress;
	bool tdls_nss_teardown_complete;
	enum tdls_nss_transition_type tdls_nss_transition_mode;
	int32_t tdls_teardown_peers_cnt;
	struct tdls_set_state_info set_state_info;
#endif

	void *hdd_ipa;

	/* Use below lock to protect access to isSchedScanUpdatePending
	 * since it will be accessed in two different contexts.
	 */
	qdf_spinlock_t sched_scan_lock;

	/* Flag keeps track of wiphy suspend/resume */
	bool isWiphySuspended;

	/* Indicates about pending sched_scan results */
	bool isSchedScanUpdatePending;

#ifdef MSM_PLATFORM
	/* DDR bus bandwidth compute timer
	 */
	qdf_timer_t bus_bw_timer;
	bool bus_bw_timer_running;
	qdf_spinlock_t bus_bw_timer_lock;
	struct work_struct bus_bw_work;
	int cur_vote_level;
	spinlock_t bus_bw_lock;
	int cur_rx_level;
	uint64_t prev_rx;
	int cur_tx_level;
	uint64_t prev_tx;
#endif

	struct completion ready_to_suspend;
	/* defining the solution type */
	uint32_t target_type;

	/* defining the firmware version */
	uint32_t target_fw_version;
	uint32_t target_fw_vers_ext;
	qdf_atomic_t dfs_radar_found;

	/* defining the chip/rom version */
	uint32_t target_hw_version;
	/* defining the chip/rom revision */
	uint32_t target_hw_revision;
	/* chip/rom name */
	const char *target_hw_name;
	struct regulatory reg;
#ifdef FEATURE_WLAN_CH_AVOID
	uint16_t unsafe_channel_count;
	uint16_t unsafe_channel_list[NUM_CHANNELS];
#endif /* FEATURE_WLAN_CH_AVOID */

	uint8_t max_intf_count;
	uint8_t current_intf_count;
#ifdef WLAN_FEATURE_LPSS
	uint8_t lpss_support;
#endif
	uint8_t ap_arpns_support;
	tSirScanType ioctl_scan_mode;

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	qdf_work_t sta_ap_intf_check_work;
#endif

	struct work_struct  sap_start_work;
	bool is_sap_restart_required;
	bool is_sta_connection_pending;
	qdf_spinlock_t sap_update_info_lock;
	qdf_spinlock_t sta_update_info_lock;

	uint8_t dev_dfs_cac_status;

	bool btCoexModeSet;
#ifdef FEATURE_GREEN_AP
	struct hdd_green_ap_ctx *green_ap_ctx;
#endif
	fw_log_info fw_log_settings;
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	qdf_mc_timer_t skip_acs_scan_timer;
	uint8_t skip_acs_scan_status;
	uint8_t *last_acs_channel_list;
	uint8_t num_of_channels;
	qdf_spinlock_t acs_skip_lock;
#endif

	qdf_wake_lock_t sap_dfs_wakelock;
	atomic_t sap_dfs_ref_cnt;

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
	bool is_extwow_app_type1_param_set;
	bool is_extwow_app_type2_param_set;
	bool ext_wow_should_suspend;
	struct completion ready_to_extwow;
#endif

	/* Time since boot up to extscan start (in micro seconds) */
	uint64_t ext_scan_start_since_boot;
	unsigned long g_event_flags;
	/* RoC request queue and work */
	struct delayed_work roc_req_work;
	qdf_spinlock_t hdd_roc_req_q_lock;
	qdf_list_t hdd_roc_req_q;
#ifndef NAPIER_SCAN
	qdf_spinlock_t hdd_scan_req_q_lock;
	qdf_list_t hdd_scan_req_q;
#endif
	uint8_t miracast_value;

#ifdef WLAN_NS_OFFLOAD
	/* IPv6 notifier callback for handling NS offload on change in IP */
	struct notifier_block ipv6_notifier;
#endif
	bool ns_offload_enable;
	/* IPv4 notifier callback for handling ARP offload on change in IP */
	struct notifier_block ipv4_notifier;

	/* number of rf chains supported by target */
	uint32_t  num_rf_chains;
	/* Is htTxSTBC supported by target */
	uint8_t   ht_tx_stbc_supported;
#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
	struct hdd_offloaded_packets_ctx op_ctx;
#endif
	bool mcc_mode;
#ifdef WLAN_FEATURE_MEMDUMP
	uint8_t *fw_dump_loc;
	uint32_t dump_loc_paddr;
	qdf_mc_timer_t memdump_cleanup_timer;
	struct mutex memdump_lock;
	bool memdump_in_progress;
	bool memdump_init_done;
#endif /* WLAN_FEATURE_MEMDUMP */
	uint16_t driver_dump_size;
	uint8_t *driver_dump_mem;

	bool connection_in_progress;
	qdf_spinlock_t connection_status_lock;

	uint16_t hdd_txrx_hist_idx;
	struct hdd_tx_rx_histogram *hdd_txrx_hist;

	/*
	 * place to store FTM capab of target. This allows changing of FTM capab
	 * at runtime and intersecting it with target capab before updating.
	 */
	uint32_t fine_time_meas_cap_target;
	uint32_t rx_high_ind_cnt;
	/* completion variable to indicate set antenna mode complete*/
	struct completion set_antenna_mode_cmpl;
	/* Current number of TX X RX chains being used */
	enum antenna_mode current_antenna_mode;
	bool bpf_enabled;

	/* the radio index assigned by cnss_logger */
	int radio_index;
	qdf_work_t sap_pre_cac_work;
	bool hbw_requested;
	uint32_t last_nil_scan_bug_report_timestamp;
#ifdef WLAN_FEATURE_NAN_DATAPATH
	bool nan_datapath_enabled;
#endif
	/* Present state of driver cds modules */
	enum driver_modules_status driver_status;
	/* MC timer interface change */
	qdf_mc_timer_t iface_change_timer;
	/* Interface change lock */
	struct mutex iface_change_lock;
	bool rps;
	bool enableRxThread;
	bool napi_enable;
	bool stop_modules_in_progress;
	bool start_modules_in_progress;
	bool update_mac_addr_to_fw;
	struct acs_dfs_policy acs_policy;
	uint16_t wmi_max_len;
	struct suspend_resume_stats suspend_resume_stats;
	struct hdd_runtime_pm_context runtime_context;
	bool roaming_in_progress;
	struct scan_chan_info *chan_info;
	struct mutex chan_info_lock;
	/* bit map to set/reset TDLS by different sources */
	unsigned long tdls_source_bitmap;
	/* tdls source timer to enable/disable TDLS on p2p listen */
	qdf_mc_timer_t tdls_source_timer;
	bool tdls_umac_comp_active;
	bool tdls_nap_active;
	uint8_t beacon_probe_rsp_cnt_per_scan;
	uint8_t last_scan_reject_session_id;
	enum scan_reject_states last_scan_reject_reason;
	unsigned long last_scan_reject_timestamp;
	bool dfs_cac_offload;
	bool reg_offload;
};

/**
 * struct hdd_vendor_acs_chan_params - vendor acs channel parameters
 * @channel_count: channel count
 * @channel_list: pointer to channel list
 * @pcl_count: pcl list count
 * @vendor_pcl_list: pointer to pcl list
 * @vendor_weight_list: pointer to pcl weight list
 */
struct hdd_vendor_acs_chan_params {
	uint32_t channel_count;
	uint8_t *channel_list;
	uint32_t pcl_count;
	uint8_t *vendor_pcl_list;
	uint8_t *vendor_weight_list;
};

/**
 * struct hdd_external_acs_timer_context - acs timer context
 * @reason: reason for acs trigger
 * @adapter: hdd adapter for acs
 */
struct hdd_external_acs_timer_context {
	int8_t reason;
	hdd_adapter_t *adapter;
};

/**
 * struct hdd_vendor_chan_info - vendor channel info
 * @pri_ch: primary channel
 * @ht_sec_ch: secondary channel
 * @vht_seg0_center_ch: segment0 for vht
 * @vht_seg1_center_ch: vht segment 1
 * @chan_width: channel width
 */
struct hdd_vendor_chan_info {
	uint8_t pri_ch;
	uint8_t ht_sec_ch;
	uint8_t vht_seg0_center_ch;
	uint8_t vht_seg1_center_ch;
	uint8_t chan_width;
};

/**
 * struct  hdd_channel_info - standard channel info
 * @freq: Freq in Mhz
 * @flags: channel info flags
 * @flagext: extended channel info flags
 * @ieee_chan_number: channel number
 * @max_reg_power: max tx power according to regulatory
 * @max_radio_power: max radio power
 * @min_radio_power: min radio power
 * @reg_class_id: regulatory class
 * @max_antenna_gain: max antenna gain allowed on channel
 * @vht_center_freq_seg0: vht center freq segment 0
 * @vht_center_freq_seg1: vht center freq segment 1
 */
struct hdd_channel_info {
	u_int16_t freq;
	u_int32_t flags;
	u_int16_t flagext;
	u_int8_t ieee_chan_number;
	int8_t max_reg_power;
	int8_t max_radio_power;
	int8_t min_radio_power;
	u_int8_t reg_class_id;
	u_int8_t max_antenna_gain;
	u_int8_t vht_center_freq_seg0;
	u_int8_t vht_center_freq_seg1;
};

/*
 * Function declarations and documentation
 */
int hdd_validate_channel_and_bandwidth(hdd_adapter_t *adapter,
				uint32_t chan_number,
				enum phy_ch_width chan_bw);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
void wlan_hdd_check_sta_ap_concurrent_ch_intf(void *sta_pAdapter);
#endif

const char *hdd_device_mode_to_string(uint8_t device_mode);

QDF_STATUS hdd_get_front_adapter(hdd_context_t *pHddCtx,
				 hdd_adapter_list_node_t **ppAdapterNode);

QDF_STATUS hdd_get_next_adapter(hdd_context_t *pHddCtx,
				hdd_adapter_list_node_t *pAdapterNode,
				hdd_adapter_list_node_t **pNextAdapterNode);

QDF_STATUS hdd_remove_adapter(hdd_context_t *pHddCtx,
			      hdd_adapter_list_node_t *pAdapterNode);

QDF_STATUS hdd_remove_front_adapter(hdd_context_t *pHddCtx,
				    hdd_adapter_list_node_t **ppAdapterNode);

QDF_STATUS hdd_add_adapter_back(hdd_context_t *pHddCtx,
				hdd_adapter_list_node_t *pAdapterNode);

QDF_STATUS hdd_add_adapter_front(hdd_context_t *pHddCtx,
				 hdd_adapter_list_node_t *pAdapterNode);

hdd_adapter_t *hdd_open_adapter(hdd_context_t *pHddCtx, uint8_t session_type,
				const char *name, tSirMacAddr macAddr,
				unsigned char name_assign_type,
				bool rtnl_held);
QDF_STATUS hdd_close_adapter(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter,
			     bool rtnl_held);
QDF_STATUS hdd_close_all_adapters(hdd_context_t *pHddCtx, bool rtnl_held);
QDF_STATUS hdd_stop_all_adapters(hdd_context_t *pHddCtx);
void hdd_deinit_all_adapters(hdd_context_t *hdd_ctx, bool rtnl_held);
QDF_STATUS hdd_reset_all_adapters(hdd_context_t *pHddCtx);
QDF_STATUS hdd_start_all_adapters(hdd_context_t *pHddCtx);
hdd_adapter_t *hdd_get_adapter_by_vdev(hdd_context_t *pHddCtx,
				       uint32_t vdev_id);
hdd_adapter_t *hdd_get_adapter_by_macaddr(hdd_context_t *pHddCtx,
					  tSirMacAddr macAddr);

int hdd_vdev_create(hdd_adapter_t *adapter);
int hdd_vdev_destroy(hdd_adapter_t *adapter);
int hdd_vdev_ready(hdd_adapter_t *adapter);

QDF_STATUS hdd_init_station_mode(hdd_adapter_t *pAdapter);
hdd_adapter_t *hdd_get_adapter(hdd_context_t *pHddCtx,
			enum tQDF_ADAPTER_MODE mode);
void hdd_deinit_adapter(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter,
			bool rtnl_held);
QDF_STATUS hdd_stop_adapter(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter,
			    const bool bCloseSession);
void hdd_set_station_ops(struct net_device *pWlanDev);
uint8_t *wlan_hdd_get_intf_addr(hdd_context_t *pHddCtx);
void wlan_hdd_release_intf_addr(hdd_context_t *pHddCtx, uint8_t *releaseAddr);
uint8_t hdd_get_operating_channel(hdd_context_t *pHddCtx,
			enum tQDF_ADAPTER_MODE mode);

void hdd_set_conparam(uint32_t con_param);
enum tQDF_GLOBAL_CON_MODE hdd_get_conparam(void);

void hdd_abort_mac_scan(hdd_context_t *pHddCtx, uint8_t sessionId,
			uint32_t scan_id, eCsrAbortReason reason);
void hdd_cleanup_actionframe(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter);

void crda_regulatory_entry_default(uint8_t *countryCode, int domain_id);
void wlan_hdd_reset_prob_rspies(hdd_adapter_t *pHostapdAdapter);
void hdd_prevent_suspend(uint32_t reason);
void hdd_allow_suspend(uint32_t reason);
void hdd_prevent_suspend_timeout(uint32_t timeout, uint32_t reason);

void wlan_hdd_cfg80211_update_wiphy_caps(struct wiphy *wiphy);
QDF_STATUS hdd_set_ibss_power_save_params(hdd_adapter_t *pAdapter);
QDF_STATUS wlan_hdd_restart_driver(hdd_context_t *pHddCtx);
void hdd_exchange_version_and_caps(hdd_context_t *pHddCtx);
int wlan_hdd_validate_context(hdd_context_t *pHddCtx);

/**
 * hdd_validate_adapter() - Validate the given adapter
 * @adapter: the adapter to validate
 *
 * This function validates the given adapter, and ensures that it is open.
 *
 * Return: Errno
 */
int hdd_validate_adapter(hdd_adapter_t *adapter);

bool hdd_is_valid_mac_address(const uint8_t *pMacAddr);
QDF_STATUS hdd_issta_p2p_clientconnected(hdd_context_t *pHddCtx);

struct qdf_mac_addr *
hdd_wlan_get_ibss_mac_addr_from_staid(hdd_adapter_t *pAdapter,
				      uint8_t staIdx);
void hdd_checkandupdate_phymode(hdd_context_t *pHddCtx);
#ifdef MSM_PLATFORM
/**
 * hdd_bus_bw_compute_timer_start() - start the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_start(hdd_context_t *hdd_ctx);

/**
 * hdd_bus_bw_compute_timer_try_start() - try to start the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * This function ensures there is at least one adapter in the associated state
 * before starting the bandwidth timer.
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_try_start(hdd_context_t *hdd_ctx);

/**
 * hdd_bus_bw_compute_timer_stop() - stop the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_stop(hdd_context_t *hdd_ctx);

/**
 * hdd_bus_bw_compute_timer_try_stop() - try to stop the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * This function ensures there are no adapters in the associated state before
 * stopping the bandwidth timer.
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_try_stop(hdd_context_t *hdd_ctx);

/**
 * hdd_bus_bandwidth_init() - Initialize bus bandwidth data structures.
 * hdd_ctx: HDD context
 *
 * Initialize bus bandwidth related data structures like spinlock and timer.
 *
 * Return: None.
 */
int hdd_bus_bandwidth_init(hdd_context_t *hdd_ctx);

/**
 * hdd_bus_bandwidth_destroy() - Destroy bus bandwidth data structures.
 * hdd_ctx: HDD context
 *
 * Destroy bus bandwidth related data structures like timer.
 *
 * Return: None.
 */
void hdd_bus_bandwidth_destroy(hdd_context_t *hdd_ctx);
#else

static inline void hdd_bus_bw_compute_timer_start(hdd_context_t *hdd_ctx)
{
}

static inline void hdd_bus_bw_compute_timer_try_start(hdd_context_t *hdd_ctx)
{
}

static inline void hdd_bus_bw_compute_timer_stop(hdd_context_t *hdd_ctx)
{
}

static inline void hdd_bus_bw_compute_timer_try_stop(hdd_context_t *hdd_ctx)
{
}

static inline void hdd_stop_bus_bw_computer_timer(hdd_adapter_t *pAdapter)
{
}

static inline int hdd_bus_bandwidth_init(hdd_context_t *hdd_ctx)
{
	return 0;
}

static inline void hdd_bus_bandwidth_destroy(hdd_context_t *hdd_ctx)
{
}
#endif

int hdd_qdf_trace_enable(QDF_MODULE_ID module_id, uint32_t bitmask);

int hdd_init(void);
void hdd_deinit(void);

int hdd_wlan_startup(struct device *dev);
void __hdd_wlan_exit(void);
int hdd_wlan_notify_modem_power_state(int state);
#ifdef QCA_HT_2040_COEX
int hdd_wlan_set_ht2040_mode(hdd_adapter_t *pAdapter, uint16_t staId,
			     struct qdf_mac_addr macAddrSTA, int width);
#endif

void wlan_hdd_send_svc_nlink_msg(int radio, int type, void *data, int len);
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
void wlan_hdd_auto_shutdown_enable(hdd_context_t *hdd_ctx, bool enable);
#endif

hdd_adapter_t *hdd_get_con_sap_adapter(hdd_adapter_t *this_sap_adapter,
							bool check_start_bss);

bool hdd_is_5g_supported(hdd_context_t *pHddCtx);

int wlan_hdd_scan_abort(hdd_adapter_t *pAdapter);

void hdd_get_fw_version(hdd_context_t *hdd_ctx,
			uint32_t *major_spid, uint32_t *minor_spid,
			uint32_t *siid, uint32_t *crmid);
/**
 * hdd_acs_response_timeout_handler() - timeout handler for acs_timer
 * @context : timeout handler context
 *
 * Return: None
 */
void hdd_acs_response_timeout_handler(void *context);

/**
 * wlan_hdd_cfg80211_start_acs(): Start ACS Procedure for SAP
 * @adapter: pointer to SAP adapter struct
 *
 * This function starts the ACS procedure if there are no
 * constraints like MBSSID DFS restrictions.
 *
 * Return: Status of ACS Start procedure
 */
int wlan_hdd_cfg80211_start_acs(hdd_adapter_t *adapter);

/**
 * hdd_cfg80211_update_acs_config() - update acs config to application
 * @adapter: hdd adapter
 * @reason: channel change reason
 *
 * Return: none
 */
void hdd_cfg80211_update_acs_config(hdd_adapter_t *adapter,
				    uint8_t reason);
/**
 * hdd_update_acs_timer_reason() - update acs timer start reason
 * @adapter: hdd adapter
 * @reason: channel change reason
 *
 * Return: 0 for success
 */
int hdd_update_acs_timer_reason(hdd_adapter_t *adapter, uint8_t reason);

/**
 * hdd_switch_sap_channel() - Move SAP to the given channel
 * @adapter: AP adapter
 * @channel: Channel
 *
 * Moves the SAP interface by invoking the function which
 * executes the callback to perform channel switch using (E)CSA.
 *
 * Return: None
 */
void hdd_switch_sap_channel(hdd_adapter_t *adapter, uint8_t channel);
#ifdef WLAN_FEATURE_MEMDUMP
/**
 * hdd_is_memdump_supported() - to check if memdump feature support
 *
 * This function is used to check if memdump feature is supported in
 * the host driver
 *
 * Return: true if supported and false otherwise
 */
static inline bool hdd_is_memdump_supported(void)
{
	return true;
}
#else
static inline bool hdd_is_memdump_supported(void)
{
	return false;
}
#endif /* WLAN_FEATURE_MEMDUMP */

void hdd_update_macaddr(struct hdd_config *config,
			struct qdf_mac_addr hw_macaddr);
void wlan_hdd_disable_roaming(hdd_adapter_t *pAdapter);
void wlan_hdd_enable_roaming(hdd_adapter_t *pAdapter);

QDF_STATUS hdd_post_cds_enable_config(hdd_context_t *pHddCtx);

QDF_STATUS hdd_abort_mac_scan_all_adapters(hdd_context_t *hdd_ctx);

QDF_STATUS wlan_hdd_check_custom_con_channel_rules(hdd_adapter_t *sta_adapter,
						  hdd_adapter_t *ap_adapter,
						  tCsrRoamProfile *roam_profile,
						  tScanResultHandle *scan_cache,
						  bool *concurrent_chnl_same);
void wlan_hdd_stop_sap(hdd_adapter_t *ap_adapter);
void wlan_hdd_start_sap(hdd_adapter_t *ap_adapter, bool reinit);

void wlan_hdd_soc_set_antenna_mode_cb(enum set_antenna_mode_status status);

#ifdef QCA_CONFIG_SMP
int wlan_hdd_get_cpu(void);
#else
static inline int wlan_hdd_get_cpu(void)
{
	return 0;
}
#endif

void wlan_hdd_sap_pre_cac_failure(void *data);
void hdd_clean_up_pre_cac_interface(hdd_context_t *hdd_ctx);

void wlan_hdd_txrx_pause_cb(uint8_t vdev_id,
	enum netif_action_type action, enum netif_reason_type reason);

int hdd_wlan_dump_stats(hdd_adapter_t *adapter, int value);
void wlan_hdd_deinit_tx_rx_histogram(hdd_context_t *hdd_ctx);
void wlan_hdd_display_tx_rx_histogram(hdd_context_t *pHddCtx);
void wlan_hdd_clear_tx_rx_histogram(hdd_context_t *pHddCtx);
void wlan_hdd_display_netif_queue_history(hdd_context_t *hdd_ctx);
void wlan_hdd_clear_netif_queue_history(hdd_context_t *hdd_ctx);
const char *hdd_get_fwpath(void);
void hdd_indicate_mgmt_frame(tSirSmeMgmtFrameInd *frame_ind);
hdd_adapter_t *hdd_get_adapter_by_sme_session_id(hdd_context_t *hdd_ctx,
						uint32_t sme_session_id);
/**
 * hdd_get_adapter_by_iface_name() - Return adapter with given interface name
 * @hdd_ctx: hdd context.
 * @iface_name: interface name
 *
 * This function is used to get the adapter with given interface name
 *
 * Return: adapter pointer if found, NULL otherwise
 *
 */
hdd_adapter_t *hdd_get_adapter_by_iface_name(hdd_context_t *hdd_ctx,
					     const char *iface_name);
enum phy_ch_width hdd_map_nl_chan_width(enum nl80211_chan_width ch_width);
uint8_t wlan_hdd_find_opclass(tHalHandle hal, uint8_t channel,
			uint8_t bw_offset);
int hdd_update_config(hdd_context_t *hdd_ctx);

/**
 * hdd_update_components_config() - Initialize driver per module ini parameters
 * @hdd_ctx: HDD Context
 *
 * API is used to initialize components configuration parameters
 * Return: 0 for success, errno for failure
 */
int hdd_update_components_config(hdd_context_t *hdd_ctx);

QDF_STATUS hdd_chan_change_notify(hdd_adapter_t *adapter,
		struct net_device *dev,
		struct hdd_chan_change_params chan_change);
int wlan_hdd_set_channel(struct wiphy *wiphy,
		struct net_device *dev,
		struct cfg80211_chan_def *chandef,
		enum nl80211_channel_type channel_type);
int wlan_hdd_cfg80211_start_bss(hdd_adapter_t *pHostapdAdapter,
		struct cfg80211_beacon_data *params,
		const u8 *ssid, size_t ssid_len,
		enum nl80211_hidden_ssid hidden_ssid,
		bool check_for_concurrency,
		bool update_beacon);

#if !defined(REMOVE_PKT_LOG)
int hdd_process_pktlog_command(hdd_context_t *hdd_ctx, uint32_t set_value,
			       int set_value2);
int hdd_pktlog_enable_disable(hdd_context_t *hdd_ctx, bool enable,
			      uint8_t user_triggered, int size);

#else
static inline int hdd_pktlog_enable_disable(hdd_context_t *hdd_ctx, bool enable,
					    uint8_t user_triggered, int size)
{
	return 0;
}
static inline int hdd_process_pktlog_command(hdd_context_t *hdd_ctx,
					     uint32_t set_value, int set_value2)
{
	return 0;
}
#endif /* REMOVE_PKT_LOG */

#ifdef FEATURE_TSO
/**
 * hdd_set_tso_flags() - enable TSO flags in the network device
 * @hdd_ctx: HDD context
 * @wlan_dev: network device structure
 *
 * This function enables the TSO related feature flags in the
 * given network device.
 *
 * Return: none
 */
static inline void hdd_set_tso_flags(hdd_context_t *hdd_ctx,
	 struct net_device *wlan_dev)
{
	if (hdd_ctx->config->tso_enable &&
	    hdd_ctx->config->enable_ip_tcp_udp_checksum_offload) {
	    /*
	     * We want to enable TSO only if IP/UDP/TCP TX checksum flag is
	     * enabled.
	     */
		hdd_info("TSO Enabled");
		wlan_dev->features |=
			 NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM |
			 NETIF_F_TSO | NETIF_F_TSO6 | NETIF_F_SG;
	}
}
#else
static inline void hdd_set_tso_flags(hdd_context_t *hdd_ctx,
	 struct net_device *wlan_dev){}
#endif /* FEATURE_TSO */

#if defined(FEATURE_WLAN_MCC_TO_SCC_SWITCH) || \
	defined(FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE)
void wlan_hdd_restart_sap(hdd_adapter_t *ap_adapter);
#else
static inline void wlan_hdd_restart_sap(hdd_adapter_t *ap_adapter)
{
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static inline bool roaming_offload_enabled(hdd_context_t *hdd_ctx)
{
	return hdd_ctx->config->isRoamOffloadEnabled;
}
#else
static inline bool roaming_offload_enabled(hdd_context_t *hdd_ctx)
{
	return false;
}
#endif

void hdd_get_ibss_peer_info_cb(void *pUserData,
				tSirPeerInfoRspParams *pPeerInfo);

#ifdef CONFIG_CNSS_LOGGER
/**
 * wlan_hdd_nl_init() - wrapper function to CNSS_LOGGER case
 * @hdd_ctx:	the hdd context pointer
 *
 * The nl_srv_init() will call to cnss_logger_device_register() and
 * expect to get a radio_index from cnss_logger module and assign to
 * hdd_ctx->radio_index, then to maintain the consistency to original
 * design, adding the radio_index check here, then return the error
 * code if radio_index is not assigned correctly, which means the nl_init
 * from cnss_logger is failed.
 *
 * Return: 0 if successfully, otherwise error code
 */
static inline int wlan_hdd_nl_init(hdd_context_t *hdd_ctx)
{
	hdd_ctx->radio_index = nl_srv_init(hdd_ctx->wiphy);

	/* radio_index is assigned from 0, so only >=0 will be valid index  */
	if (hdd_ctx->radio_index >= 0)
		return 0;
	else
		return -EINVAL;
}
#else
/**
 * wlan_hdd_nl_init() - wrapper function to non CNSS_LOGGER case
 * @hdd_ctx:	the hdd context pointer
 *
 * In case of non CNSS_LOGGER case, the nl_srv_init() will initialize
 * the netlink socket and return the success or not.
 *
 * Return: the return value from  nl_srv_init()
 */
static inline int wlan_hdd_nl_init(hdd_context_t *hdd_ctx)
{
	return nl_srv_init(hdd_ctx->wiphy);
}
#endif
QDF_STATUS hdd_sme_close_session_callback(void *pContext);

int hdd_reassoc(hdd_adapter_t *adapter, const uint8_t *bssid,
		uint8_t channel, const handoff_src src);
void hdd_svc_fw_shutdown_ind(struct device *dev);
int hdd_register_cb(hdd_context_t *hdd_ctx);
void hdd_deregister_cb(hdd_context_t *hdd_ctx);
int hdd_start_station_adapter(hdd_adapter_t *adapter);
int hdd_start_ap_adapter(hdd_adapter_t *adapter);
int hdd_configure_cds(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter);
int hdd_start_ftm_adapter(hdd_adapter_t *adapter);
int hdd_set_fw_params(hdd_adapter_t *adapter);
int hdd_wlan_start_modules(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter,
			   bool reinit);
int hdd_wlan_stop_modules(hdd_context_t *hdd_ctx, bool ftm_mode);
int hdd_start_adapter(hdd_adapter_t *adapter);

/**
 * hdd_get_bss_entry() - Get the bss entry matching the chan, bssid and ssid
 * @wiphy: wiphy
 * @channel: channel of the BSS to find
 * @bssid: bssid of the BSS to find
 * @ssid: ssid of the BSS to find
 * @ssid_len: ssid len of of the BSS to find
 *
 * The API is a wrapper to get bss from kernel matching the chan,
 * bssid and ssid
 *
 * Return: bss structure if found else NULL
 */
struct cfg80211_bss *hdd_cfg80211_get_bss(struct wiphy *wiphy,
	struct ieee80211_channel *channel,
	const u8 *bssid,
	const u8 *ssid, size_t ssid_len);

void hdd_connect_result(struct net_device *dev, const u8 *bssid,
			tCsrRoamInfo *roam_info, const u8 *req_ie,
			size_t req_ie_len, const u8 *resp_ie,
			size_t resp_ie_len, u16 status, gfp_t gfp,
			bool connect_timeout,
			tSirResultCodes timeout_reason);

#ifdef WLAN_FEATURE_FASTPATH
void hdd_enable_fastpath(struct hdd_config *hdd_cfg,
			 void *context);
#else
static inline void hdd_enable_fastpath(struct hdd_config *hdd_cfg,
				       void *context)
{
}
#endif
void hdd_wlan_update_target_info(hdd_context_t *hdd_ctx, void *context);
enum  sap_acs_dfs_mode wlan_hdd_get_dfs_mode(enum dfs_mode mode);
void hdd_ch_avoid_cb(void *hdd_context, void *indi_param);
void hdd_unsafe_channel_restart_sap(hdd_context_t *hdd_ctx);
int hdd_enable_disable_ca_event(hdd_context_t *hddctx,
				uint8_t set_value);
void wlan_hdd_undo_acs(hdd_adapter_t *adapter);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
static inline int
hdd_wlan_nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put_u64(skb, attrtype, value);
}
#else
static inline int
hdd_wlan_nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put_u64_64bit(skb, attrtype, value, NL80211_ATTR_PAD);
}
#endif

static inline int wlan_hdd_validate_session_id(u8 session_id)
{
	if (session_id != HDD_SESSION_ID_INVALID)
		return 0;

	return -EINVAL;
}

bool hdd_is_roaming_in_progress(hdd_adapter_t *adapter);
void hdd_set_roaming_in_progress(bool value);
bool hdd_is_connection_in_progress(uint8_t *session_id,
	enum scan_reject_states *reason);
void hdd_restart_sap(hdd_adapter_t *ap_adapter);
void hdd_check_and_restart_sap_with_non_dfs_acs(void);
bool hdd_set_connection_in_progress(bool value);

/**
 * wlan_hdd_sap_get_valid_channellist() - Get SAPs valid channel list
 * @ap_adapter: adapter
 * @channel_count: valid channel count
 * @channel_list: valid channel list
 * @band: frequency band
 *
 * This API returns valid channel list for SAP after removing nol and
 * channel which lies outside of configuration.
 *
 * Return: Zero on success, non-zero on failure
 */
int wlan_hdd_sap_get_valid_channellist(hdd_adapter_t *adapter,
				       uint32_t *channel_count,
				       uint8_t *channel_list,
				       eCsrBand band);
/**
 * wlan_hdd_init_chan_info() - initialize channel info variables
 * @hdd_ctx: hdd ctx
 *
 * This API initialize channel info variables
 *
 * Return: None
 */
void wlan_hdd_init_chan_info(hdd_context_t *hdd_ctx);

/**
 * wlan_hdd_deinit_chan_info() - deinitialize channel info variables
 * @hdd_ctx: hdd ctx
 *
 * This API deinitialize channel info variables
 *
 * Return: None
 */
void wlan_hdd_deinit_chan_info(hdd_context_t *hdd_ctx);
void wlan_hdd_start_sap(hdd_adapter_t *ap_adapter, bool reinit);

/**
 * hdd_check_for_opened_interfaces()- Check for interface up
 * @hdd_ctx: HDD context
 *
 * check  if there are any wlan interfaces before starting the timer
 * to close the modules
 *
 * Return: 0 if interface was opened else false
 */
bool hdd_check_for_opened_interfaces(hdd_context_t *hdd_ctx);

#ifdef WIFI_POS_CONVERGED
/**
 * hdd_send_peer_status_ind_to_app() - wrapper to call legacy or new wifi_pos
 * function to send peer status to a registered application
 * @peer_mac: MAC address of peer
 * @peer_status: ePeerConnected or ePeerDisconnected
 * @peer_timing_meas_cap: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @sessionId: SME session id, i.e. vdev_id
 * @chan_info: operating channel information
 * @dev_mode: dev mode for which indication is sent
 *
 * Return: none
 */
static inline void hdd_send_peer_status_ind_to_app(
					struct qdf_mac_addr *peer_mac,
					uint8_t peer_status,
					uint8_t peer_timing_meas_cap,
					uint8_t sessionId,
					tSirSmeChanInfo *chan_info,
					enum tQDF_ADAPTER_MODE dev_mode)
{
	struct wifi_pos_ch_info ch_info;

	if (!chan_info) {
		os_if_wifi_pos_send_peer_status(peer_mac, peer_status,
					peer_timing_meas_cap, sessionId,
					NULL, dev_mode);
		return;
	}

	ch_info.chan_id = chan_info->chan_id;
	ch_info.mhz = chan_info->mhz;
	ch_info.band_center_freq1 = chan_info->band_center_freq1;
	ch_info.band_center_freq2 = chan_info->band_center_freq2;
	ch_info.info = chan_info->info;
	ch_info.reg_info_1 = chan_info->reg_info_1;
	ch_info.reg_info_2 = chan_info->reg_info_2;
	ch_info.nss = chan_info->nss;
	ch_info.rate_flags = chan_info->rate_flags;
	ch_info.sec_ch_offset = chan_info->sec_ch_offset;
	ch_info.ch_width = chan_info->ch_width;
	os_if_wifi_pos_send_peer_status(peer_mac, peer_status,
					peer_timing_meas_cap, sessionId,
					&ch_info, dev_mode);
}
#else
static inline void hdd_send_peer_status_ind_to_app(
					struct qdf_mac_addr *peer_mac,
					uint8_t peer_status,
					uint8_t peer_timing_meas_cap,
					uint8_t sessionId,
					tSirSmeChanInfo *chan_info,
					enum tQDF_ADAPTER_MODE dev_mode)
{
	hdd_send_peer_status_ind_to_oem_app(peer_mac, peer_status,
			peer_timing_meas_cap, sessionId, chan_info, dev_mode);
}
#endif /* WIFI_POS_CONVERGENCE */

/**
 * wlan_hdd_send_p2p_quota()- Send P2P Quota value to FW
 * @adapter: Adapter data
 * @sval:    P2P quota value
 *
 * Send P2P quota value to FW
 *
 * Return: 0 sucess else failure
 */
int wlan_hdd_send_p2p_quota(hdd_adapter_t *adapter, int sval);

/**
 * wlan_hdd_send_p2p_quota()- Send MCC latency to FW
 * @adapter: Adapter data
 * @sval:    MCC latency value
 *
 * Send MCC latency value to FW
 *
 * Return: 0 sucess else failure
 */
int wlan_hdd_send_mcc_latency(hdd_adapter_t *adapter, int sval);

/**
 * wlan_hdd_get_adapter_from_vdev()- Get adapter from vdev id
 * and PSOC object data
 * @psoc: Psoc object data
 * @vdev_id: vdev id
 *
 * Get adapter from vdev id and PSOC object data
 *
 * Return: adapter pointer
 */
hdd_adapter_t *wlan_hdd_get_adapter_from_vdev(struct wlan_objmgr_psoc
					*psoc, uint8_t vdev_id);
/**
 * hdd_unregister_notifiers()- unregister kernel notifiers
 * @hdd_ctx: Hdd Context
 *
 * Unregister netdev notifiers like Netdevice,IPv4 and IPv6.
 *
 */
void hdd_unregister_notifiers(hdd_context_t *hdd_ctx);

/**
 * hdd_start_complete()- complete the start event
 * @ret: return value for complete event.
 *
 * complete the startup event and set the return in
 * global variable
 *
 * Return: void
 */

void hdd_start_complete(int ret);
#endif /* end #if !defined(WLAN_HDD_MAIN_H) */

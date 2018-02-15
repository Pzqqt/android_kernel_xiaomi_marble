 /*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * The following terms were in use in prior versions of the driver but
 * have now been replaced with terms that are aligned with the Linux
 * Coding style. Macros are defined to hopefully prevent new instances
 * from being introduced, primarily by code propagation.
 */
#define pHddCtx
#define pAdapter
#define pHostapdAdapter
#define pHddApCtx
#define pHddStaCtx
#define pHostapdState
#define pRoamInfo
#define pScanInfo
#define pBeaconIes

/*
 * Include files
 */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/cfg80211.h>
#include <linux/ieee80211.h>
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
#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)) || \
	defined(CFG80211_REMOVE_IEEE80211_BACKPORT)
#define HDD_NL80211_BAND_2GHZ   NL80211_BAND_2GHZ
#define HDD_NL80211_BAND_5GHZ   NL80211_BAND_5GHZ
#define HDD_NUM_NL80211_BANDS   NUM_NL80211_BANDS
#else
#define HDD_NL80211_BAND_2GHZ   IEEE80211_BAND_2GHZ
#define HDD_NL80211_BAND_5GHZ   IEEE80211_BAND_5GHZ
#define HDD_NUM_NL80211_BANDS   ((enum nl80211_band)IEEE80211_NUM_BANDS)
#endif

/** Length of the TX queue for the netdev */
#define HDD_NETDEV_TX_QUEUE_LEN (3000)

/** Hdd Tx Time out value */
#ifdef LIBRA_LINUX_PC
#define HDD_TX_TIMEOUT          (8000)
#else
#define HDD_TX_TIMEOUT          msecs_to_jiffies(5000)
#endif

#define HDD_TX_STALL_THRESHOLD 4

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
#define WLAN_WAIT_DISCONNECT_ALREADY_IN_PROGRESS  1000
#define WLAN_WAIT_TIME_STOP_ROAM  4000
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

/* rcpi request timeout in milli seconds */
#define WLAN_WAIT_TIME_RCPI 500

#define MAX_NUMBER_OF_ADAPTERS 4

#define MAX_CFG_STRING_LEN  255

/* Maximum time(ms) to wait for external acs response */
#define WLAN_VENDOR_ACS_WAIT_TIME 1000

#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
/** Mac Address string **/
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ADDRESS_STR_LEN 18  /* Including null terminator */
/* Max and min IEs length in bytes */
#define MAX_GENIE_LEN (512)
#define MIN_GENIE_LEN (2)

#define WLAN_CHIP_VERSION   "WCNSS"

#define hdd_log_ratelimited(rate, level, args...) \
		QDF_TRACE_RATE_LIMITED(rate, QDF_MODULE_ID_HDD, level, ## args)
#define hdd_log_ratelimited_fl(rate, level, format, args...) \
		hdd_log_ratelimited(rate, level, FL(format), ## args)
#define hdd_alert_ratelimited(rate, format, args...) \
		hdd_log_ratelimited_fl(rate, QDF_TRACE_LEVEL_FATAL,\
			format, ## args)
#define hdd_err_ratelimited(rate, format, args...) \
		hdd_log_ratelimited_fl(rate, QDF_TRACE_LEVEL_ERROR,\
			format, ## args)
#define hdd_warn_ratelimited(rate, format, args...) \
		hdd_log_ratelimited_fl(rate, QDF_TRACE_LEVEL_WARN,\
			format, ## args)
#define hdd_notice_ratelimited(rate, format, args...) \
		hdd_log_ratelimited_fl(rate, QDF_TRACE_LEVEL_INFO,\
			format, ## args)
#define hdd_info_ratelimited(rate, format, args...) \
		hdd_log_ratelimited_fl(rate, QDF_TRACE_LEVEL_INFO,\
			format, ## args)
#define hdd_debug_ratelimited(rate, format, args...) \
		hdd_log_ratelimited_fl(rate, QDF_TRACE_LEVEL_DEBUG,\
			format, ## args)

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

#define WLAN_HDD_GET_PRIV_PTR(__dev__) \
		(struct hdd_adapter *)(netdev_priv((__dev__)))

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
#define HDD_SAP_WAKE_LOCK_DURATION WAKELOCK_DURATION_RECOMMENDED

/* SAP client disconnect wake lock duration in milli seconds */
#define HDD_SAP_CLIENT_DISCONNECT_WAKE_LOCK_DURATION \
	WAKELOCK_DURATION_RECOMMENDED

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
#if !((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)) ||\
	defined(WITH_BACKPORTS))
#define NET_NAME_UNKNOWN	0
#endif

#define PRE_CAC_SSID "pre_cac_ssid"

/* session ID invalid */
#define HDD_SESSION_ID_INVALID    0xFF

#define SCAN_REJECT_THRESHOLD_TIME 300000 /* Time is in msec, equal to 5 mins */
#define SCAN_REJECT_THRESHOLD 15

/* Default Psoc id */
#define DEFAULT_PSOC_ID 1

/* wait time for nud stats in milliseconds */
#define WLAN_WAIT_TIME_NUD_STATS 800
/* nud stats skb max length */
#define WLAN_NUD_STATS_LEN 800
/* ARP packet type for NUD debug stats */
#define WLAN_NUD_STATS_ARP_PKT_TYPE 1
/* Assigned size of driver memory dump is 4096 bytes */
#define DRIVER_MEM_DUMP_SIZE    4096

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
extern struct mutex hdd_init_deinit_lock;

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

struct hdd_tx_rx_stats {
	/* start_xmit stats */
	__u32    tx_called;
	__u32    tx_dropped;
	__u32    tx_orphaned;
	__u32    tx_classified_ac[NUM_TX_QUEUES];
	__u32    tx_dropped_ac[NUM_TX_QUEUES];

	/* rx stats */
	__u32 rx_packets[NUM_CPUS];
	__u32 rx_dropped[NUM_CPUS];
	__u32 rx_delivered[NUM_CPUS];
	__u32 rx_refused[NUM_CPUS];

	/* txflow stats */
	bool     is_txflow_paused;
	__u32    txflow_pause_cnt;
	__u32    txflow_unpause_cnt;
	__u32    txflow_timer_cnt;

	/*tx timeout stats*/
	__u32 tx_timeout_cnt;
	__u32 cont_txtimeout_cnt;
	u64 jiffies_last_txtimeout;
};

#ifdef WLAN_FEATURE_11W
/**
 * struct hdd_pmf_stats - Protected Management Frame statistics
 * @num_unprot_deauth_rx: Number of unprotected deauth frames received
 * @num_unprot_disassoc_rx: Number of unprotected disassoc frames received
 */
struct hdd_pmf_stats {
	uint8_t num_unprot_deauth_rx;
	uint8_t num_unprot_disassoc_rx;
};
#endif

struct hdd_arp_stats_s {
	uint16_t tx_arp_req_count;
	uint16_t rx_arp_rsp_count;
	uint16_t tx_dropped;
	uint16_t rx_dropped;
	uint16_t rx_delivered;
	uint16_t rx_refused;
	uint16_t tx_host_fw_sent;
	uint16_t rx_host_drop_reorder;
	uint16_t rx_fw_cnt;
	uint16_t tx_ack_cnt;
};

/**
 * struct hdd_stats - per-adapter statistics
 * @summary_stat: Summary stats reported by firmware
 * @class_a_stat: "Class A" stats reported by firmware
 * @class_d_stat: "Class D" stats reported by firmware
 * @per_chain_rssi_stats: Per-chain RSSI stats
 * @tx_rx_stats: Tx & Rx stats
 * @hdd_pmf_stats: Protected Management Frame stats
 */
struct hdd_stats {
	tCsrSummaryStatsInfo summary_stat;
	tCsrGlobalClassAStatsInfo class_a_stat;
	tCsrGlobalClassDStatsInfo class_d_stat;
	struct csr_per_chain_rssi_stats_info  per_chain_rssi_stats;
	struct hdd_tx_rx_stats tx_rx_stats;
	struct hdd_arp_stats_s hdd_arp_stats;
#ifdef WLAN_FEATURE_11W
	struct hdd_pmf_stats hdd_pmf_stats;
#endif
};

/**
 * struct hdd_roaming_info - HDD Internal Roaming Information
 * @bssid: BSSID to which we are connected
 * @peer_mac: Peer MAC address for IBSS connection
 * @roam_id: Unique identifier for a roaming instance
 * @roam_status: Current roam command status
 * @defer_key_complete: Should key complete be deferred?
 *
 */
struct hdd_roaming_info {
	tSirMacAddr bssid;
	tSirMacAddr peer_mac;
	uint32_t roam_id;
	eRoamCmdStatus roam_status;
	bool defer_key_complete;

};

#ifdef FEATURE_WLAN_WAPI
/* Define WAPI macros for Length, BKID count etc*/
#define MAC_ADDR_LEN           6
#define MAX_NUM_AKM_SUITES    16

/** WAPI AUTH mode definition */
enum wapi_auth_mode {
	WAPI_AUTH_MODE_OPEN = 0,
	WAPI_AUTH_MODE_PSK = 1,
	WAPI_AUTH_MODE_CERT
} __packed;

#define WPA_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define WPA_GET_BE24(a) ((u32) ((a[0] << 16) | (a[1] << 8) | a[2]))
#define WLAN_EID_WAPI 68
#define WAPI_PSK_AKM_SUITE  0x02721400
#define WAPI_CERT_AKM_SUITE 0x01721400

/**
 * struct hdd_wapi_info - WAPI Information structure definition
 * @wapi_mode: Is WAPI enabled on this adapter?
 * @is_wapi_sta: Is the STA associated with WAPI?
 * @wapi_auth_mode: WAPI authentication mode used by this adapter
 */
struct hdd_wapi_info {
	bool wapi_mode;
	bool is_wapi_sta;
	enum wapi_auth_mode wapi_auth_mode;
};
#endif /* FEATURE_WLAN_WAPI */

struct hdd_beacon_data {
	u8 *head;
	u8 *tail;
	u8 *proberesp_ies;
	u8 *assocresp_ies;
	int head_len;
	int tail_len;
	int proberesp_ies_len;
	int assocresp_ies_len;
	int dtim_period;
};

struct action_pkt_buffer {
	uint8_t *frame_ptr;
	uint32_t frame_length;
	uint16_t freq;
};

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
	struct hdd_adapter *adapter;
	struct cfg80211_scan_request *scan_request;
	uint32_t scan_id;
	uint8_t source;
	uint32_t timestamp;
	qdf_timer_t hdd_scan_inactivity_timer;
	uint32_t scan_req_flags;
};

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

/**
 * struct hdd_station_ctx -- STA-specific information
 * @wext_state: wireless extensions state
 * @wpa_versions: bitmap of supported WPA versions
 * @requested_bssid: Specific BSSID to which to connect
 * @conn_info: current connection information
 * @roam_info: current roaming information
 * @ft_carrier_on: is carrier on
 * @ibss_sta_generation: current ibss generation. Incremented whenever
 *    ibss New peer joins and departs the network
 * @ibss_enc_key_installed: is the ibss wep/wpa-none encryptions key
 *    installed?
 * @ibss_enc_key: current ibss wep/wpa-none encryption key (if
 *    @ibss_enc_key_installed is %true)
 * @ibss_peer_info: information about the ibss peer
 * @hdd_reassoc_scenario: is station in the middle of reassociation?
 * @sta_debug_state: STA context debug variable
 * @broadcast_staid: STA ID assigned for broadcast frames
 * @ch_info: monitor mode channel information
 * @ndp_ctx: NAN data path context
 * @ap_supports_immediate_power_save: Does the current AP allow our STA
 *    to immediately go into power save?
 */
struct hdd_station_ctx {
	struct hdd_wext_state wext_state;
	enum nl80211_wpa_versions wpa_versions;
	struct qdf_mac_addr requested_bssid;
	struct hdd_connection_info conn_info;
	struct hdd_roaming_info roam_info;
	int ft_carrier_on;
	int ibss_sta_generation;
	bool ibss_enc_key_installed;
	tCsrRoamSetKey ibss_enc_key;
	tSirPeerInfoRspParams ibss_peer_info;
	bool hdd_reassoc_scenario;
	int sta_debug_state;
	uint8_t broadcast_staid;
	struct hdd_mon_set_ch_info ch_info;
	bool ap_supports_immediate_power_save;
};

/**
 * enum bss_state - current state of the BSS
 * @BSS_STOP: BSS is stopped
 * @BSS_START: BSS is started
 */
enum bss_state {
	BSS_STOP,
	BSS_START,
};

/**
 * struct hdd_hostapd_state - hostapd-related state information
 * @bss_state: Current state of the BSS
 * @qdf_event: Event to synchronize actions between hostapd thread and
 *    internal callback threads
 * @qdf_stop_bss_event: Event to synchronize Stop BSS. When Stop BSS
 *    is issued userspace thread can wait on this event. The event will
 *    be set when the Stop BSS processing in UMAC has completed.
 * @qdf_sta_disassoc_event: Event to synchronize STA Disassociation.
 *    When a STA is disassociated userspace thread can wait on this
 *    event. The event will be set when the STA Disassociation
 *    processing in UMAC has completed.
 * @qdf_status: Used to communicate state from other threads to the
 *    userspace thread.
 */
struct hdd_hostapd_state {
	enum bss_state bss_state;
	qdf_event_t qdf_event;
	qdf_event_t qdf_stop_bss_event;
	qdf_event_t qdf_sta_disassoc_event;
	QDF_STATUS qdf_status;
};

/**
 * enum bss_stop_reason - reasons why a BSS is stopped.
 * @BSS_STOP_REASON_INVALID: no reason specified explicitly.
 * @BSS_STOP_DUE_TO_MCC_SCC_SWITCH: BSS stopped due to host
 *  driver is trying to switch AP role to a different channel
 *  to maintain SCC mode with the STA role on the same card.
 *  this usually happens when STA is connected to an external
 *  AP that runs on a different channel
 * @BSS_STOP_DUE_TO_VENDOR_CONFIG_CHAN: BSS stopped due to
 *  vendor subcmd set sap config channel
 */
enum bss_stop_reason {
	BSS_STOP_REASON_INVALID = 0,
	BSS_STOP_DUE_TO_MCC_SCC_SWITCH = 1,
	BSS_STOP_DUE_TO_VENDOR_CONFIG_CHAN = 2,
};

/**
 * struct hdd_rate_info - rate_info in HDD
 * @rate: tx/rx rate (kbps)
 * @mode: 0->11abg legacy, 1->HT, 2->VHT (refer to sir_sme_phy_mode)
 * @nss: number of streams
 * @mcs: mcs index for HT/VHT mode
 * @rate_flags: rate flags for last tx/rx
 *
 * rate info in HDD
 */
struct hdd_rate_info {
	uint32_t rate;
	uint8_t mode;
	uint8_t nss;
	uint8_t mcs;
	uint8_t rate_flags;
};

/**
 * struct hdd_fw_txrx_stats - fw txrx status in HDD
 *                            (refer to station_info struct in Kernel)
 * @tx_packets: packets transmitted to this station
 * @tx_bytes: bytes transmitted to this station
 * @rx_packets: packets received from this station
 * @rx_bytes: bytes received from this station
 * @rx_retries: cumulative retry counts
 * @tx_failed: number of failed transmissions
 * @rssi: The signal strength (dbm)
 * @tx_rate: last used tx rate info
 * @rx_rate: last used rx rate info
 *
 * fw txrx status in HDD
 */
struct hdd_fw_txrx_stats {
	uint32_t tx_packets;
	uint64_t tx_bytes;
	uint32_t rx_packets;
	uint64_t rx_bytes;
	uint32_t tx_retries;
	uint32_t tx_failed;
	int8_t rssi;
	struct hdd_rate_info tx_rate;
	struct hdd_rate_info rx_rate;
};

/**
 * struct hdd_station_info - Per station structure kept in HDD for
 *                                     multiple station support for SoftAP
 * @in_use: Is the station entry in use?
 * @sta_id: Station ID reported back from HAL (through SAP).
 *           Broadcast uses station ID zero by default.
 * @sta_type: Type of station i.e. p2p client or infrastructure station
 * @sta_mac: MAC address of the station
 * @peer_state: Current Station state so HDD knows how to deal with packet
 *              queue. Most recent states used to change TLSHIM STA state.
 * @is_qos_enabled: Track QoS status of station
 * @is_deauth_in_progress: The station entry for which Deauth is in progress
 * @nss: Number of spatial streams supported
 * @rate_flags: Rate Flags for this connection
 * @ecsa_capable: Extended CSA capabilities
 * @max_phy_rate: Calcuated maximum phy rate based on mode, nss, mcs etc.
 * @tx_packets: Packets send to current station
 * @tx_bytes: Bytes send to current station
 * @rx_packets: Packets received from current station
 * @rx_bytes: Bytes received from current station
 * @last_tx_rx_ts: Last tx/rx timestamp with current station
 * @assoc_ts: Current station association timestamp
 * @tx_rate: Tx rate with current station reported from F/W
 * @rx_rate: Rx rate with current station reported from F/W
 * @ampdu: Ampdu enable or not of the station
 * @sgi_enable: Short GI enable or not of the station
 * @tx_stbc: Tx Space-time block coding enable/disable
 * @rx_stbc: Rx Space-time block coding enable/disable
 * @ch_width: Channel Width of the connection
 * @mode: Mode of the connection
 * @max_supp_idx: Max supported rate index of the station
 * @max_ext_idx: Max extended supported rate index of the station
 * @max_mcs_idx: Max supported mcs index of the station
 * @rx_mcs_map: VHT Rx mcs map
 * @tx_mcs_map: VHT Tx mcs map
 * @freq : Frequency of the current station
 * @dot11_mode: 802.11 Mode of the connection
 * @ht_present: HT caps present or not in the current station
 * @vht_present: VHT caps present or not in the current station
 * @ht_caps: HT capabilities of current station
 * @vht_caps: VHT capabilities of current station
 * @reason_code: Disconnection reason code for current station
 * @rssi: RSSI of the current station reported from F/W
 */
struct hdd_station_info {
	bool in_use;
	uint8_t sta_id;
	eStationType sta_type;
	struct qdf_mac_addr sta_mac;
	enum ol_txrx_peer_state peer_state;
	bool is_qos_enabled;
	bool is_deauth_in_progress;
	uint8_t   nss;
	uint32_t  rate_flags;
	uint8_t   ecsa_capable;
	uint32_t max_phy_rate;
	uint32_t tx_packets;
	uint64_t tx_bytes;
	uint32_t rx_packets;
	uint64_t rx_bytes;
	qdf_time_t last_tx_rx_ts;
	qdf_time_t assoc_ts;
	uint32_t tx_rate;
	uint32_t rx_rate;
	bool ampdu;
	bool sgi_enable;
	bool tx_stbc;
	bool rx_stbc;
	tSirMacHTChannelWidth ch_width;
	uint8_t mode;
	uint8_t max_supp_idx;
	uint8_t max_ext_idx;
	uint8_t max_mcs_idx;
	uint8_t rx_mcs_map;
	uint8_t tx_mcs_map;
	uint32_t freq;
	uint8_t dot11_mode;
	bool ht_present;
	bool vht_present;
	struct ieee80211_ht_cap ht_caps;
	struct ieee80211_vht_cap vht_caps;
	uint32_t reason_code;
	int8_t rssi;
};

/**
 * struct hdd_ap_ctx - SAP/P2PGO specific information
 * @hostapd_state: state control information
 * @dfs_cac_block_tx: Is data tramsmission blocked due to DFS CAC?
 * @ap_active: Are any stations active?
 * @hdd_ap_inactivity_timer: Timer used to shutdown the AP if it is
 *     inactive for an extended amount of time.
 * @disable_intrabss_fwd: Prevent forwarding between stations
 * @broadcast_sta_id: Station ID assigned after BSS starts
 * @privacy: The privacy bits of configuration
 * @encryption_type: The encryption being used
 * @group_key: Group Encryption Key
 * @wep_key: WEP key array
 * @wep_def_key_idx: WEP default key index
 * @sap_context: Pointer to context maintained by SAP (opaque to HDD)
 * @sap_config: SAP configuration
 * @operating_channel: channel upon which the SAP is operating
 * @beacon: Beacon information
 * @vendor_acs_timer: Timer for ACS
 * @vendor_acs_timer_initialized: Is @vendor_acs_timer initialized?
 * @bss_stop_reason: Reason why the BSS was stopped
 * @txrx_stats: TX RX statistics from firmware
 */
struct hdd_ap_ctx {
	struct hdd_hostapd_state hostapd_state;
	bool dfs_cac_block_tx;
	bool ap_active;
	qdf_mc_timer_t hdd_ap_inactivity_timer;
	bool disable_intrabss_fwd;
	uint8_t broadcast_sta_id;
	uint8_t privacy;
	eCsrEncryptionType encryption_type;
	tCsrRoamSetKey group_key;
	tCsrRoamSetKey wep_key[CSR_MAX_NUM_KEY];
	uint8_t wep_def_key_idx;
	struct sap_context *sap_context;
	tsap_Config_t sap_config;
	uint8_t operating_channel;
	struct hdd_beacon_data *beacon;
	qdf_mc_timer_t vendor_acs_timer;
	bool vendor_acs_timer_initialized;
	enum bss_stop_reason bss_stop_reason;
	struct hdd_fw_txrx_stats txrx_stats;
};

/**
 * struct hdd_scan_info - Per-adapter scan information
 * @scan_add_ie: Additional IE for scan
 * @default_scan_ies: Default scan IEs
 * @default_scan_ies_len: Length of @default_scan_ies
 * @scan_mode: Scan mode
 */
struct hdd_scan_info {
	tSirAddie scan_add_ie;
	uint8_t *default_scan_ies;
	uint16_t default_scan_ies_len;
	tSirScanType scan_mode;
};

#define WLAN_HDD_MAX_MC_ADDR_LIST CFG_TGT_MAX_MULTICAST_FILTER_ENTRIES

#ifdef WLAN_FEATURE_PACKET_FILTERING
struct hdd_multicast_addr_list {
	uint8_t mc_cnt;
	uint8_t addr[WLAN_HDD_MAX_MC_ADDR_LIST][ETH_ALEN];
};
#endif

#define WLAN_HDD_MAX_HISTORY_ENTRY		10

/**
 * struct hdd_netif_queue_stats - netif queue operation statistics
 * @pause_count - pause counter
 * @unpause_count - unpause counter
 */
struct hdd_netif_queue_stats {
	u32 pause_count;
	u32 unpause_count;
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
 * @dfs: dfs context to prevent/allow runtime pm
 * @connect: connect context to prevent/allow runtime pm
 *
 * Runtime PM control for underlying activities
 */
struct hdd_runtime_pm_context {
	qdf_runtime_lock_t dfs;
	qdf_runtime_lock_t connect;
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

/**
 * struct rcpi_info - rcpi info
 * @rcpi: computed value in dB
 * @mac_addr: peer mac addr for which rcpi is computed
 */
struct rcpi_info {
	int32_t rcpi;
	struct qdf_mac_addr mac_addr;
};

struct hdd_context;

struct hdd_adapter {
	/* Magic cookie for adapter sanity verification.  Note that this
	 * needs to be at the beginning of the private data structure so
	 * that it will exist at the beginning of dev->priv and hence
	 * will always be in mapped memory
	 */
	uint32_t magic;

	/* list node for membership in the adapter list */
	qdf_list_node_t node;

	struct hdd_context *hdd_ctx;
	struct wlan_objmgr_vdev *hdd_vdev;

	void *txrx_vdev;

	/** Handle to the network device */
	struct net_device *dev;

	enum QDF_OPMODE device_mode;

	/** IPv4 notifier callback for handling ARP offload on change in IP */
	struct work_struct ipv4_notifier_work;
#ifdef WLAN_NS_OFFLOAD
	/** IPv6 notifier callback for handling NS offload on change in IP */
	struct work_struct ipv6_notifier_work;
#endif

	/* TODO Move this to sta Ctx */
	struct wireless_dev wdev;
	struct cfg80211_scan_request *request;
	struct cfg80211_scan_request *vendor_request;

	/** ops checks if Opportunistic Power Save is Enable or Not
	 * ctw stores ctWindow value once we receive Opps command from
	 * wpa_supplicant then using ctWindow value we need to Enable
	 * Opportunistic Power Save
	 */
	uint8_t ops;
	uint32_t ctw;

	/** Current MAC Address for the adapter  */
	struct qdf_mac_addr mac_addr;

	/**Event Flags*/
	unsigned long event_flags;

	/**Device TX/RX statistics*/
	struct net_device_stats stats;
	/** HDD statistics*/
	struct hdd_stats hdd_stats;

	/* estimated link speed */
	uint32_t estimated_linkspeed;

	uint8_t session_id;

	/* QDF event for session close */
	qdf_event_t qdf_session_close_event;

	/* QDF event for session open */
	qdf_event_t qdf_session_open_event;

	/* TODO: move these to sta ctx. These may not be used in AP */
	/** completion variable for disconnect callback */
	struct completion disconnect_comp_var;

	struct completion roaming_comp_var;

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

	struct completion ibss_peer_info_comp;

	/* Track whether the linkup handling is needed  */
	bool is_link_up_service_needed;

	/* WMM Status */
	struct hdd_wmm_status hdd_wmm_status;

	/** Multiple station supports */
	/** Per-station structure */
	spinlock_t sta_info_lock;        /* To protect access to station Info */
	struct hdd_station_info sta_info[WLAN_MAX_STA_COUNT];
	struct hdd_station_info cache_sta_info[WLAN_MAX_STA_COUNT];


#ifdef FEATURE_WLAN_WAPI
	struct hdd_wapi_info wapi_info;
#endif

	int8_t rssi;
	int32_t rssi_on_disconnect;
#ifdef WLAN_FEATURE_LPSS
	bool rssi_send;
#endif

	uint8_t snr;

	struct work_struct  sap_stop_bss_work;

	union {
		struct hdd_station_ctx station;
		struct hdd_ap_ctx ap;
	} session;

	qdf_atomic_t dfs_radar_found;

#ifdef WLAN_FEATURE_TSF
	/* tsf value received from firmware */
	uint64_t cur_target_time;
	uint64_t tsf_sync_soc_timer;
#ifdef WLAN_FEATURE_TSF_PLUS
	/* spin lock for read/write timestamps */
	qdf_spinlock_t host_target_sync_lock;
	qdf_mc_timer_t host_target_sync_timer;
	uint64_t cur_host_time;
	uint64_t last_host_time;
	uint64_t last_target_time;
	/* to store the count of continuous invalid tstamp-pair */
	int continuous_error_count;
	/* to indicate whether tsf_sync has been initialized */
	qdf_atomic_t tsf_sync_ready_flag;
#endif /* WLAN_FEATURE_TSF_PLUS */
#endif

#ifdef WLAN_FEATURE_PACKET_FILTERING
	struct hdd_multicast_addr_list mc_addr_list;
#endif
	uint8_t addr_filter_pattern;

	bool survey_idx;

	struct hdd_scan_info scan_info;

	/* Flag to ensure PSB is configured through framework */
	uint8_t psb_changed;
	/* UAPSD psb value configured through framework */
	uint8_t configured_psb;
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
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	qdf_mc_timer_t tx_flow_control_timer;
	bool tx_flow_timer_initialized;
	unsigned int tx_flow_low_watermark;
	unsigned int tx_flow_high_watermark_offset;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
	bool offloads_configured;

	/* DSCP to UP QoS Mapping */
	enum sme_qos_wmmuptype dscp_to_up_map[WLAN_HDD_MAX_DSCP + 1];

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	bool is_link_layer_stats_set;
#endif
	uint8_t link_status;

	/* variable for temperature in Celsius */
	int temperature;

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

	/*
	 * Indicate if HO fails during disconnect so that
	 * disconnect is not initiated by HDD as its already
	 * initiated by CSR
	 */
	bool roam_ho_fail;
	struct lfr_firmware_status lfr_fw_status;
	bool con_status;
	bool dad;
	uint8_t active_ac;
	uint32_t mon_chan;
	uint32_t mon_bandwidth;

	/* rcpi information */
	struct rcpi_info rcpi;
};

#define WLAN_HDD_GET_STATION_CTX_PTR(adapter) (&(adapter)->session.station)
#define WLAN_HDD_GET_AP_CTX_PTR(adapter) (&(adapter)->session.ap)
#define WLAN_HDD_GET_WEXT_STATE_PTR(adapter) \
				(&(adapter)->session.station.wext_state)
#define WLAN_HDD_GET_CTX(adapter) ((adapter)->hdd_ctx)
#define WLAN_HDD_GET_HAL_CTX(adapter)  ((adapter)->hdd_ctx->hHal)
#define WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter) \
				(&(adapter)->session.ap.hostapd_state)
#define WLAN_HDD_GET_SAP_CTX_PTR(adapter) ((adapter)->session.ap.sap_context)

#ifdef WLAN_FEATURE_NAN_DATAPATH
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

/*
 * struct hdd_priv_data - driver ioctl private data payload
 * @buf: pointer to command buffer (may be in userspace)
 * @used_len: length of the command/data currently in @buf
 * @total_len: total length of the @buf memory allocation
 */
struct hdd_priv_data {
	uint8_t *buf;
	int used_len;
	int total_len;
};

#define  MAX_MOD_LOGLEVEL 10
struct fw_log_info {
	uint8_t enable;
	uint8_t dl_type;
	uint8_t dl_report;
	uint8_t dl_loglevel;
	uint8_t index;
	uint32_t dl_mod_loglevel[MAX_MOD_LOGLEVEL];

};

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

/**
 * hdd_sta_smps_param  - SMPS parameters to configure from hdd
 * HDD_STA_SMPS_PARAM_UPPER_RSSI_THRESH: RSSI threshold to enter Dynamic SMPS
 * mode from inactive mode
 * HDD_STA_SMPS_PARAM_STALL_RSSI_THRESH:  RSSI threshold to enter
 * Stalled-D-SMPS mode from D-SMPS mode or to enter D-SMPS mode from
 * Stalled-D-SMPS mode
 * HDD_STA_SMPS_PARAM_LOWER_RSSI_THRESH:  RSSI threshold to disable SMPS modes
 * HDD_STA_SMPS_PARAM_UPPER_BRSSI_THRESH: Upper threshold for beacon-RSSI.
 * Used to reduce RX chainmask.
 * HDD_STA_SMPS_PARAM_LOWER_BRSSI_THRESH:  Lower threshold for beacon-RSSI.
 * Used to increase RX chainmask.
 * HDD_STA_SMPS_PARAM_DTIM_1CHRX_ENABLE: Enable/Disable DTIM 1chRx feature
 */
enum hdd_sta_smps_param {
	HDD_STA_SMPS_PARAM_UPPER_RSSI_THRESH = 0,
	HDD_STA_SMPS_PARAM_STALL_RSSI_THRESH = 1,
	HDD_STA_SMPS_PARAM_LOWER_RSSI_THRESH = 2,
	HDD_STA_SMPS_PARAM_UPPER_BRSSI_THRESH = 3,
	HDD_STA_SMPS_PARAM_LOWER_BRSSI_THRESH = 4,
	HDD_STA_SMPS_PARAM_DTIM_1CHRX_ENABLE = 5
};

/**
 * struct hdd_nud_stats_context - hdd NUD stats context
 * @response_event: NUD stats request wait event
 */
struct hdd_nud_stats_context {
	struct completion response_event;
};

/**
 * tos - Type of service requested by the application
 * TOS_BK: Back ground traffic
 * TOS_BE: Best effort traffic
 * TOS_VI: Video traffic
 * TOS_VO: Voice traffic
 */
enum tos {
	TOS_BK = 0,
	TOS_BE = 1,
	TOS_VI = 2,
	TOS_VO = 3,
};

#define HDD_AC_BK_BIT                   1
#define HDD_AC_BE_BIT                   2
#define HDD_AC_VI_BIT                   4
#define HDD_AC_VO_BIT                   8

#define HDD_MAX_OFF_CHAN_TIME_FOR_VO    20
#define HDD_MAX_OFF_CHAN_TIME_FOR_VI    20
#define HDD_MAX_OFF_CHAN_TIME_FOR_BE    40
#define HDD_MAX_OFF_CHAN_TIME_FOR_BK    40

#define HDD_MAX_AC                      4
#define HDD_MAX_OFF_CHAN_ENTRIES        2

#define HDD_AC_BIT_INDX                 0
#define HDD_DWELL_TIME_INDX             1

/* One per STA: 1 for BCMC_STA_ID, 1 for each SAP_SELF_STA_ID,
 * 1 for WDS_STAID
 */
#define HDD_MAX_ADAPTERS (WLAN_MAX_STA_COUNT + QDF_MAX_NO_OF_SAP_MODE + 2)

/** Adapter structure definition */
struct hdd_context {
	struct wlan_objmgr_psoc *hdd_psoc;
	struct wlan_objmgr_pdev *hdd_pdev;

	/** HAL handle...*/
	tHalHandle hHal;

	struct wiphy *wiphy;
	/* TODO Remove this from here. */

	qdf_spinlock_t hdd_adapter_lock;
	qdf_list_t hdd_adapters; /* List of adapters */

	struct hdd_adapter *sta_to_adapter[HDD_MAX_ADAPTERS];

	/** Pointer for firmware image data */
	const struct firmware *fw;

	/** Pointer for configuration data */
	const struct firmware *cfg;

	/** Pointer to the parent device */
	struct device *parent_dev;

	/** Config values read from qcom_cfg.ini file */
	struct hdd_config *config;

	/* Completion  variable to indicate Mc Thread Suspended */
	struct completion mc_sus_event_var;

	bool is_scheduler_suspended;

#ifdef QCA_CONFIG_SMP
	bool is_ol_rx_thread_suspended;
#endif

	bool hdd_wlan_suspended;
	bool suspended;
	/* flag to start pktlog after SSR/PDR if previously enabled */
	bool is_pktlog_enabled;

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
	struct qdf_mac_addr p2p_device_address;

	qdf_wake_lock_t rx_wake_lock;
	qdf_wake_lock_t sap_wake_lock;

	void *hdd_ipa;

	/* Flag keeps track of wiphy suspend/resume */
	bool is_wiphy_suspended;

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

	qdf_atomic_t con_mode_flag;
	/* defining the firmware version */
	uint32_t target_fw_version;
	uint32_t target_fw_vers_ext;

	/* defining the chip/rom version */
	uint32_t target_hw_version;
	/* defining the chip/rom revision */
	uint32_t target_hw_revision;
	/* chip/rom name */
	char *target_hw_name;
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

	bool bt_coex_mode_set;
	struct fw_log_info fw_log_settings;
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
	struct mutex memdump_lock;
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
	/* interface idle work */
	qdf_delayed_work_t iface_idle_work;
	/* Interface change lock */
	struct mutex iface_change_lock;
	bool rps;
	bool enable_rxthread;
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
	bool tdls_umac_comp_active;
	bool tdls_nap_active;
	uint8_t beacon_probe_rsp_cnt_per_scan;
	uint8_t last_scan_reject_session_id;
	enum scan_reject_states last_scan_reject_reason;
	unsigned long last_scan_reject_timestamp;
	uint8_t scan_reject_cnt;
	bool dfs_cac_offload;
	bool reg_offload;
	bool rcpi_enabled;
#ifdef FEATURE_WLAN_CH_AVOID
	struct ch_avoid_ind_type coex_avoid_freq_list;
	struct ch_avoid_ind_type dnbs_avoid_freq_list;
	/* Lock to control access to dnbs and coex avoid freq list */
	struct mutex avoid_freq_lock;
#endif
	qdf_atomic_t is_acs_allowed;
#ifdef WLAN_FEATURE_TSF
	/* indicate whether tsf has been initialized */
	qdf_atomic_t tsf_ready_flag;
	/* indicate whether it's now capturing tsf(updating tstamp-pair) */
	qdf_atomic_t cap_tsf_flag;
	/* the context that is capturing tsf */
	struct hdd_adapter *cap_tsf_context;
#endif
	uint8_t bt_a2dp_active:1;
	uint8_t bt_vo_active:1;
	struct hdd_nud_stats_context nud_stats_context;
	enum band_info curr_band;
	bool imps_enabled;
	int user_configured_pkt_filter_rules;
	bool is_fils_roaming_supported;
	qdf_atomic_t vendor_disable_lro_flag;
	qdf_atomic_t disable_lro_in_concurrency;
	qdf_atomic_t disable_lro_in_low_tput;
	bool en_tcp_delack_no_lro;
	bool force_rsne_override;
	qdf_wake_lock_t monitor_mode_wakelock;
	bool lte_coex_ant_share;
	int sscan_pid;
	uint32_t track_arp_ip;
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
	struct hdd_adapter *adapter;
};

/**
 * struct hdd_vendor_chan_info - vendor channel info
 * @band: channel operating band
 * @pri_ch: primary channel
 * @ht_sec_ch: secondary channel
 * @vht_seg0_center_ch: segment0 for vht
 * @vht_seg1_center_ch: vht segment 1
 * @chan_width: channel width
 */
struct hdd_vendor_chan_info {
	uint8_t band;
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
 * @eHDD_DRV_OP_PROBE: Refers to .probe operation
 * @eHDD_DRV_OP_REMOVE: Refers to .remove operation
 * @eHDD_DRV_OP_SHUTDOWN: Refers to .shutdown operation
 * @eHDD_DRV_OP_REINIT: Refers to .reinit operation
 * @eHDD_DRV_OP_IFF_UP: Refers to IFF_UP operation
 */
enum {
	eHDD_DRV_OP_PROBE = 0,
	eHDD_DRV_OP_REMOVE,
	eHDD_DRV_OP_SHUTDOWN,
	eHDD_DRV_OP_REINIT,
	eHDD_DRV_OP_IFF_UP
};

/*
 * Function declarations and documentation
 */
int hdd_validate_channel_and_bandwidth(struct hdd_adapter *adapter,
				uint32_t chan_number,
				enum phy_ch_width chan_bw);

const char *hdd_device_mode_to_string(uint8_t device_mode);

QDF_STATUS hdd_get_front_adapter(struct hdd_context *hdd_ctx,
				 struct hdd_adapter **out_adapter);

QDF_STATUS hdd_get_next_adapter(struct hdd_context *hdd_ctx,
				struct hdd_adapter *current_adapter,
				struct hdd_adapter **out_adapter);

QDF_STATUS hdd_remove_adapter(struct hdd_context *hdd_ctx,
			      struct hdd_adapter *adapter);

QDF_STATUS hdd_remove_front_adapter(struct hdd_context *hdd_ctx,
				    struct hdd_adapter **out_adapter);

QDF_STATUS hdd_add_adapter_back(struct hdd_context *hdd_ctx,
				struct hdd_adapter *adapter);

QDF_STATUS hdd_add_adapter_front(struct hdd_context *hdd_ctx,
				 struct hdd_adapter *adapter);

/**
 * hdd_for_each_adapter - adapter iterator macro
 * @hdd_ctx: the global HDD context
 * @adapter: an hdd_adapter pointer to use as a cursor
 */
#define hdd_for_each_adapter(hdd_ctx, adapter) \
	for (hdd_get_front_adapter(hdd_ctx, &adapter); \
	     adapter; \
	     hdd_get_next_adapter(hdd_ctx, adapter, &adapter))

struct hdd_adapter *hdd_open_adapter(struct hdd_context *hdd_ctx,
				     uint8_t session_type,
				     const char *name, tSirMacAddr macAddr,
				     unsigned char name_assign_type,
				     bool rtnl_held);
QDF_STATUS hdd_close_adapter(struct hdd_context *hdd_ctx,
			     struct hdd_adapter *adapter,
			     bool rtnl_held);
QDF_STATUS hdd_close_all_adapters(struct hdd_context *hdd_ctx, bool rtnl_held);
QDF_STATUS hdd_stop_all_adapters(struct hdd_context *hdd_ctx);
void hdd_deinit_all_adapters(struct hdd_context *hdd_ctx, bool rtnl_held);
QDF_STATUS hdd_reset_all_adapters(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_start_all_adapters(struct hdd_context *hdd_ctx);
struct hdd_adapter *hdd_get_adapter_by_vdev(struct hdd_context *hdd_ctx,
				       uint32_t vdev_id);
struct hdd_adapter *hdd_get_adapter_by_macaddr(struct hdd_context *hdd_ctx,
					  tSirMacAddr macAddr);

int hdd_vdev_create(struct hdd_adapter *adapter,
		    csr_roam_completeCallback callback, void *ctx);
int hdd_vdev_destroy(struct hdd_adapter *adapter);
int hdd_vdev_ready(struct hdd_adapter *adapter);

QDF_STATUS hdd_init_station_mode(struct hdd_adapter *adapter);
struct hdd_adapter *hdd_get_adapter(struct hdd_context *hdd_ctx,
			enum QDF_OPMODE mode);
/*
 * hdd_get_device_mode() - Get device mode
 * @session_id: Session id
 *
 * Return: Device mode
 */
enum tQDF_ADAPTER_MODE hdd_get_device_mode(uint32_t session_id);
void hdd_deinit_adapter(struct hdd_context *hdd_ctx,
			struct hdd_adapter *adapter,
			bool rtnl_held);
QDF_STATUS hdd_stop_adapter(struct hdd_context *hdd_ctx,
			    struct hdd_adapter *adapter);
void hdd_set_station_ops(struct net_device *dev);
uint8_t *wlan_hdd_get_intf_addr(struct hdd_context *hdd_ctx);
void wlan_hdd_release_intf_addr(struct hdd_context *hdd_ctx,
				uint8_t *releaseAddr);
uint8_t hdd_get_operating_channel(struct hdd_context *hdd_ctx,
			enum QDF_OPMODE mode);

void hdd_set_conparam(uint32_t con_param);
enum QDF_GLOBAL_MODE hdd_get_conparam(void);
void crda_regulatory_entry_default(uint8_t *countryCode, int domain_id);
void wlan_hdd_reset_prob_rspies(struct hdd_adapter *adapter);
void hdd_prevent_suspend(uint32_t reason);
void hdd_allow_suspend(uint32_t reason);
void hdd_prevent_suspend_timeout(uint32_t timeout, uint32_t reason);

void wlan_hdd_cfg80211_update_wiphy_caps(struct wiphy *wiphy);
QDF_STATUS hdd_set_ibss_power_save_params(struct hdd_adapter *adapter);
int wlan_hdd_validate_context(struct hdd_context *hdd_ctx);

/**
 * hdd_validate_adapter() - Validate the given adapter
 * @adapter: the adapter to validate
 *
 * This function validates the given adapter, and ensures that it is open.
 *
 * Return: Errno
 */
int hdd_validate_adapter(struct hdd_adapter *adapter);

/**
 * wlan_hdd_validate_context_in_loading() - check the HDD context in loading
 * @hdd_ctx:	HDD context pointer
 *
 * Return: 0 if the context is valid. Error code otherwise
 */
int wlan_hdd_validate_context_in_loading(struct hdd_context *hdd_ctx);

bool hdd_is_valid_mac_address(const uint8_t *pMacAddr);
QDF_STATUS hdd_issta_p2p_clientconnected(struct hdd_context *hdd_ctx);
bool wlan_hdd_validate_modules_state(struct hdd_context *hdd_ctx);

struct qdf_mac_addr *
hdd_wlan_get_ibss_mac_addr_from_staid(struct hdd_adapter *adapter,
				      uint8_t staIdx);
#ifdef MSM_PLATFORM
/**
 * hdd_bus_bw_compute_timer_start() - start the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_start(struct hdd_context *hdd_ctx);

/**
 * hdd_bus_bw_compute_timer_try_start() - try to start the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * This function ensures there is at least one adapter in the associated state
 * before starting the bandwidth timer.
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_try_start(struct hdd_context *hdd_ctx);

/**
 * hdd_bus_bw_compute_timer_stop() - stop the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_stop(struct hdd_context *hdd_ctx);

/**
 * hdd_bus_bw_compute_timer_try_stop() - try to stop the bandwidth timer
 * @hdd_ctx: the global hdd context
 *
 * This function ensures there are no adapters in the associated state before
 * stopping the bandwidth timer.
 *
 * Return: None
 */
void hdd_bus_bw_compute_timer_try_stop(struct hdd_context *hdd_ctx);

/**
 * hdd_bus_bandwidth_init() - Initialize bus bandwidth data structures.
 * @hdd_ctx: HDD context
 *
 * Initialize bus bandwidth related data structures like spinlock and timer.
 *
 * Return: None.
 */
int hdd_bus_bandwidth_init(struct hdd_context *hdd_ctx);

/**
 * hdd_bus_bandwidth_destroy() - Destroy bus bandwidth data structures.
 * @hdd_ctx: HDD context
 *
 * Destroy bus bandwidth related data structures like timer.
 *
 * Return: None.
 */
void hdd_bus_bandwidth_destroy(struct hdd_context *hdd_ctx);

/**
 * hdd_bus_bw_cancel_work() - Cancel the bus_bw_work worker
 * @hdd_ctx: HDD context
 *
 * Cancel the bus_bw_work to stop monitor link state.
 *
 * Return: None.
 */
void hdd_bus_bw_cancel_work(struct hdd_context *hdd_ctx);

#define GET_CUR_RX_LVL(config) ((config)->cur_rx_level)
#define GET_BW_COMPUTE_INTV(config) ((config)->busBandwidthComputeInterval)

#else

static inline
void hdd_bus_bw_compute_timer_start(struct hdd_context *hdd_ctx)
{
}

static inline
void hdd_bus_bw_compute_timer_try_start(struct hdd_context *hdd_ctx)
{
}

static inline
void hdd_bus_bw_compute_timer_stop(struct hdd_context *hdd_ctx)
{
}

static inline
void hdd_bus_bw_compute_timer_try_stop(struct hdd_context *hdd_ctx)
{
}

static inline
int hdd_bus_bandwidth_init(struct hdd_context *hdd_ctx)
{
	return 0;
}

static inline
void hdd_bus_bandwidth_destroy(struct hdd_context *hdd_ctx)
{
}

static inline
void hdd_bus_bw_cancel_work(struct hdd_context *hdd_ctx)
{
}

#define GET_CUR_RX_LVL(config) 0
#define GET_BW_COMPUTE_INTV(config) 0

#endif

int hdd_qdf_trace_enable(QDF_MODULE_ID module_id, uint32_t bitmask);

int hdd_init(void);
void hdd_deinit(void);

int hdd_wlan_startup(struct device *dev);
void __hdd_wlan_exit(void);
int hdd_wlan_notify_modem_power_state(int state);
#ifdef QCA_HT_2040_COEX
/**
 * hdd_wlan_set_ht2040_mode() - notify FW with HT20/HT40 mode
 * @adapter: pointer to adapter
 * @sta_id: station id
 * @sta_mac: station MAC address
 * @channel_type: channel type
 *
 * This function notifies FW with HT20/HT40 mode
 *
 * Return: 0 if successful, error number otherwise
 */
int hdd_wlan_set_ht2040_mode(struct hdd_adapter *adapter, uint16_t sta_id,
			     struct qdf_mac_addr sta_mac, int channel_type);
#endif

void wlan_hdd_send_svc_nlink_msg(int radio, int type, void *data, int len);
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
void wlan_hdd_auto_shutdown_enable(struct hdd_context *hdd_ctx, bool enable);
#endif

struct hdd_adapter *
hdd_get_con_sap_adapter(struct hdd_adapter *this_sap_adapter,
			bool check_start_bss);

bool hdd_is_5g_supported(struct hdd_context *hdd_ctx);

int wlan_hdd_scan_abort(struct hdd_adapter *adapter);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static inline bool roaming_offload_enabled(struct hdd_context *hdd_ctx)
{
	return hdd_ctx->config->isRoamOffloadEnabled;
}
#else
static inline bool roaming_offload_enabled(struct hdd_context *hdd_ctx)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_HOST_ROAM
static inline bool hdd_driver_roaming_supported(struct hdd_context *hdd_ctx)
{
	return hdd_ctx->config->isFastRoamIniFeatureEnabled;
}
#else
static inline bool hdd_driver_roaming_supported(struct hdd_context *hdd_ctx)
{
	return false;
}
#endif

static inline bool hdd_roaming_supported(struct hdd_context *hdd_ctx)
{
	bool val;

	val = hdd_driver_roaming_supported(hdd_ctx) ||
		roaming_offload_enabled(hdd_ctx);

	return val;
}

#ifdef CFG80211_SCAN_RANDOM_MAC_ADDR
static inline bool hdd_scan_random_mac_addr_supported(void)
{
	return true;
}
#else
static inline bool hdd_scan_random_mac_addr_supported(void)
{
	return false;
}
#endif

/**
 * hdd_start_vendor_acs(): Start vendor ACS procedure
 * @adapter: pointer to SAP adapter struct
 *
 * This function sends the ACS config to the ACS daemon and
 * starts the vendor ACS timer to wait for the next command.
 *
 * Return: Status of vendor ACS procedure
 */
int hdd_start_vendor_acs(struct hdd_adapter *adapter);

void hdd_get_fw_version(struct hdd_context *hdd_ctx,
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
int wlan_hdd_cfg80211_start_acs(struct hdd_adapter *adapter);

/**
 * hdd_cfg80211_update_acs_config() - update acs config to application
 * @adapter: hdd adapter
 * @reason: channel change reason
 *
 * Return: 0 for success else error code
 */
int hdd_cfg80211_update_acs_config(struct hdd_adapter *adapter,
				   uint8_t reason);

/**
 * hdd_update_acs_timer_reason() - update acs timer start reason
 * @adapter: hdd adapter
 * @reason: channel change reason
 *
 * Return: 0 for success
 */
int hdd_update_acs_timer_reason(struct hdd_adapter *adapter, uint8_t reason);

/**
 * hdd_switch_sap_channel() - Move SAP to the given channel
 * @adapter: AP adapter
 * @channel: Channel
 * @forced: Force to switch channel, ignore SCC/MCC check
 *
 * Moves the SAP interface by invoking the function which
 * executes the callback to perform channel switch using (E)CSA.
 *
 * Return: None
 */
void hdd_switch_sap_channel(struct hdd_adapter *adapter, uint8_t channel,
			    bool forced);

#if defined(FEATURE_WLAN_CH_AVOID)
void hdd_unsafe_channel_restart_sap(struct hdd_context *hdd_ctx);

void hdd_ch_avoid_ind(struct hdd_context *hdd_ctxt,
		      struct unsafe_ch_list *unsafe_chan_list,
		      struct ch_avoid_ind_type *avoid_freq_list);
#else
static inline
void hdd_unsafe_channel_restart_sap(struct hdd_context *hdd_ctx)
{
}

static inline
void hdd_ch_avoid_ind(struct hdd_context *hdd_ctxt,
		      struct unsafe_ch_list *unsafe_chan_list,
		      struct ch_avoid_ind_type *avoid_freq_list)
{
}
#endif

void hdd_update_macaddr(struct hdd_config *config,
			struct qdf_mac_addr hw_macaddr);
void wlan_hdd_disable_roaming(struct hdd_adapter *adapter);
void wlan_hdd_enable_roaming(struct hdd_adapter *adapter);

QDF_STATUS hdd_post_cds_enable_config(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_abort_mac_scan_all_adapters(struct hdd_context *hdd_ctx);

QDF_STATUS
wlan_hdd_check_custom_con_channel_rules(struct hdd_adapter *sta_adapter,
					struct hdd_adapter *ap_adapter,
					tCsrRoamProfile *roam_profile,
					tScanResultHandle *scan_cache,
					bool *concurrent_chnl_same);

void wlan_hdd_stop_sap(struct hdd_adapter *ap_adapter);
void wlan_hdd_start_sap(struct hdd_adapter *ap_adapter, bool reinit);

/**
 * hdd_init_nud_stats_ctx() - initialize NUD stats context
 * @hdd_ctx: Pointer to hdd context
 *
 * Return: none
 */
static inline void hdd_init_nud_stats_ctx(struct hdd_context *hdd_ctx)
{
	init_completion(&hdd_ctx->nud_stats_context.response_event);
}

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
void hdd_clean_up_pre_cac_interface(struct hdd_context *hdd_ctx);

void wlan_hdd_txrx_pause_cb(uint8_t vdev_id,
	enum netif_action_type action, enum netif_reason_type reason);

int hdd_wlan_dump_stats(struct hdd_adapter *adapter, int value);
void wlan_hdd_deinit_tx_rx_histogram(struct hdd_context *hdd_ctx);
void wlan_hdd_display_tx_rx_histogram(struct hdd_context *hdd_ctx);
void wlan_hdd_clear_tx_rx_histogram(struct hdd_context *hdd_ctx);
void
wlan_hdd_display_netif_queue_history(struct hdd_context *hdd_ctx,
				     enum qdf_stats_verbosity_level verb_lvl);
void wlan_hdd_clear_netif_queue_history(struct hdd_context *hdd_ctx);
const char *hdd_get_fwpath(void);
void hdd_indicate_mgmt_frame(tSirSmeMgmtFrameInd *frame_ind);

struct hdd_adapter *
hdd_get_adapter_by_sme_session_id(struct hdd_context *hdd_ctx,
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
struct hdd_adapter *hdd_get_adapter_by_iface_name(struct hdd_context *hdd_ctx,
					     const char *iface_name);
enum phy_ch_width hdd_map_nl_chan_width(enum nl80211_chan_width ch_width);
uint8_t wlan_hdd_find_opclass(tHalHandle hal, uint8_t channel,
			uint8_t bw_offset);
int hdd_update_config(struct hdd_context *hdd_ctx);

/**
 * hdd_update_components_config() - Initialize driver per module ini parameters
 * @hdd_ctx: HDD Context
 *
 * API is used to initialize components configuration parameters
 * Return: 0 for success, errno for failure
 */
int hdd_update_components_config(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_chan_change_notify(struct hdd_adapter *adapter,
		struct net_device *dev,
		struct hdd_chan_change_params chan_change,
		bool legacy_phymode);
int wlan_hdd_set_channel(struct wiphy *wiphy,
		struct net_device *dev,
		struct cfg80211_chan_def *chandef,
		enum nl80211_channel_type channel_type);
int wlan_hdd_cfg80211_start_bss(struct hdd_adapter *adapter,
		struct cfg80211_beacon_data *params,
		const u8 *ssid, size_t ssid_len,
		enum nl80211_hidden_ssid hidden_ssid,
		bool check_for_concurrency);

#if !defined(REMOVE_PKT_LOG)
int hdd_process_pktlog_command(struct hdd_context *hdd_ctx, uint32_t set_value,
			       int set_value2);
int hdd_pktlog_enable_disable(struct hdd_context *hdd_ctx, bool enable,
			      uint8_t user_triggered, int size);

#else
static inline
int hdd_pktlog_enable_disable(struct hdd_context *hdd_ctx, bool enable,
			      uint8_t user_triggered, int size)
{
	return 0;
}

static inline
int hdd_process_pktlog_command(struct hdd_context *hdd_ctx,
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
static inline void hdd_set_tso_flags(struct hdd_context *hdd_ctx,
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
static inline void hdd_set_tso_flags(struct hdd_context *hdd_ctx,
	 struct net_device *wlan_dev){}
#endif /* FEATURE_TSO */

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
static inline int wlan_hdd_nl_init(struct hdd_context *hdd_ctx)
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
static inline int wlan_hdd_nl_init(struct hdd_context *hdd_ctx)
{
	return nl_srv_init(hdd_ctx->wiphy);
}
#endif
QDF_STATUS hdd_sme_open_session_callback(uint8_t session_id);
QDF_STATUS hdd_sme_close_session_callback(uint8_t session_id);

int hdd_reassoc(struct hdd_adapter *adapter, const uint8_t *bssid,
		uint8_t channel, const handoff_src src);
void hdd_svc_fw_shutdown_ind(struct device *dev);
int hdd_register_cb(struct hdd_context *hdd_ctx);
void hdd_deregister_cb(struct hdd_context *hdd_ctx);
int hdd_start_station_adapter(struct hdd_adapter *adapter);
int hdd_start_ap_adapter(struct hdd_adapter *adapter);
int hdd_configure_cds(struct hdd_context *hdd_ctx, struct hdd_adapter *adapter);
int hdd_set_fw_params(struct hdd_adapter *adapter);
int hdd_wlan_start_modules(struct hdd_context *hdd_ctx,
			   struct hdd_adapter *adapter,
			   bool reinit);
int hdd_wlan_stop_modules(struct hdd_context *hdd_ctx, bool ftm_mode);
int hdd_start_adapter(struct hdd_adapter *adapter);
void hdd_populate_random_mac_addr(struct hdd_context *hdd_ctx, uint32_t num);
/**
 * hdd_is_interface_up()- Checkfor interface up before ssr
 * @hdd_ctx: HDD context
 *
 * check  if there are any wlan interfaces before SSR accordingly start
 * the interface.
 *
 * Return: 0 if interface was opened else false
 */
bool hdd_is_interface_up(struct hdd_adapter *adapter);
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
			struct csr_roam_info *roam_info, const u8 *req_ie,
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
void hdd_wlan_update_target_info(struct hdd_context *hdd_ctx, void *context);

enum  sap_acs_dfs_mode wlan_hdd_get_dfs_mode(enum dfs_mode mode);
void hdd_unsafe_channel_restart_sap(struct hdd_context *hdd_ctx);
/**
 * hdd_clone_local_unsafe_chan() - clone hdd ctx unsafe chan list
 * @hdd_ctx: hdd context pointer
 * @local_unsafe_list: copied unsafe chan list array
 * @local_unsafe_list_count: channel number in returned local_unsafe_list
 *
 * The function will allocate memory and make a copy the current unsafe
 * channels from hdd ctx. The caller need to free the local_unsafe_list
 * memory after use.
 *
 * Return: 0 if successfully clone unsafe chan list.
 */
int hdd_clone_local_unsafe_chan(struct hdd_context *hdd_ctx,
	uint16_t **local_unsafe_list, uint16_t *local_unsafe_list_count);

/**
 * hdd_local_unsafe_channel_updated() - check unsafe chan list same or not
 * @hdd_ctx: hdd context pointer
 * @local_unsafe_list: unsafe chan list to be compared with hdd_ctx's list
 * @local_unsafe_list_count: channel number in local_unsafe_list
 *
 * The function checked the input channel is same as current unsafe chan
 * list in hdd_ctx.
 *
 * Return: true if input channel list is same as the list in hdd_ctx
 */
bool hdd_local_unsafe_channel_updated(struct hdd_context *hdd_ctx,
	uint16_t *local_unsafe_list, uint16_t local_unsafe_list_count);

int hdd_enable_disable_ca_event(struct hdd_context *hddctx,
				uint8_t set_value);
void wlan_hdd_undo_acs(struct hdd_adapter *adapter);

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

bool hdd_is_roaming_in_progress(struct hdd_adapter *adapter);
void hdd_set_roaming_in_progress(bool value);
bool hdd_is_connection_in_progress(uint8_t *session_id,
	enum scan_reject_states *reason);
void hdd_restart_sap(struct hdd_adapter *ap_adapter);
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
int wlan_hdd_sap_get_valid_channellist(struct hdd_adapter *adapter,
				       uint32_t *channel_count,
				       uint8_t *channel_list,
				       enum band_info band);
/**
 * wlan_hdd_init_chan_info() - initialize channel info variables
 * @hdd_ctx: hdd ctx
 *
 * This API initialize channel info variables
 *
 * Return: None
 */
void wlan_hdd_init_chan_info(struct hdd_context *hdd_ctx);

/**
 * wlan_hdd_deinit_chan_info() - deinitialize channel info variables
 * @hdd_ctx: hdd ctx
 *
 * This API deinitialize channel info variables
 *
 * Return: None
 */
void wlan_hdd_deinit_chan_info(struct hdd_context *hdd_ctx);
void wlan_hdd_start_sap(struct hdd_adapter *ap_adapter, bool reinit);

/**
 * hdd_check_for_opened_interfaces()- Check for interface up
 * @hdd_ctx: HDD context
 *
 * check  if there are any wlan interfaces before starting the timer
 * to close the modules
 *
 * Return: 0 if interface was opened else false
 */
bool hdd_check_for_opened_interfaces(struct hdd_context *hdd_ctx);

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
					enum QDF_OPMODE dev_mode)
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
					enum QDF_OPMODE dev_mode)
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
int wlan_hdd_send_p2p_quota(struct hdd_adapter *adapter, int sval);

/**
 * wlan_hdd_send_p2p_quota()- Send MCC latency to FW
 * @adapter: Adapter data
 * @sval:    MCC latency value
 *
 * Send MCC latency value to FW
 *
 * Return: 0 sucess else failure
 */
int wlan_hdd_send_mcc_latency(struct hdd_adapter *adapter, int sval);

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
struct hdd_adapter *wlan_hdd_get_adapter_from_vdev(struct wlan_objmgr_psoc
					*psoc, uint8_t vdev_id);
/**
 * hdd_unregister_notifiers()- unregister kernel notifiers
 * @hdd_ctx: Hdd Context
 *
 * Unregister netdev notifiers like Netdevice,IPv4 and IPv6.
 *
 */
void hdd_unregister_notifiers(struct hdd_context *hdd_ctx);

/**
 * hdd_dbs_scan_selection_init() - initialization for DBS scan selection config
 * @hdd_ctx: HDD context
 *
 * This function sends the DBS scan selection config configuration to the
 * firmware via WMA
 *
 * Return: 0 - success, < 0 - failure
 */
int hdd_dbs_scan_selection_init(struct hdd_context *hdd_ctx);

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

/**
 * hdd_chip_pwr_save_fail_detected_cb() - chip power save failure detected
 * callback
 * @hddctx: HDD context
 * @data: chip power save failure detected data
 *
 * This function reads the chip power save failure detected data and fill in
 * the skb with NL attributes and send up the NL event.
 * This callback execute in atomic context and must not invoke any
 * blocking calls.
 *
 * Return: none
 */

void hdd_chip_pwr_save_fail_detected_cb(void *hddctx,
				struct chip_pwr_save_fail_detected_params
				*data);

/**
 * hdd_update_ie_whitelist_attr() - Copy probe req ie whitelist attrs from cfg
 * @ie_whitelist: output parameter
 * @cfg: pointer to hdd config
 *
 * Return: None
 */
void hdd_update_ie_whitelist_attr(struct probe_req_whitelist_attr *ie_whitelist,
				  struct hdd_config *cfg);

/**
 * hdd_get_rssi_snr_by_bssid() - gets the rssi and snr by bssid from scan cache
 * @adapter: adapter handle
 * @bssid: bssid to look for in scan cache
 * @rssi: rssi value found
 * @snr: snr value found
 *
 * Return: QDF_STATUS
 */
int hdd_get_rssi_snr_by_bssid(struct hdd_adapter *adapter, const uint8_t *bssid,
			      int8_t *rssi, int8_t *snr);

/**
 * hdd_reset_limit_off_chan() - reset limit off-channel command parameters
 * @adapter - HDD adapter
 *
 * Return: 0 on success and non zero value on failure
 */
int hdd_reset_limit_off_chan(struct hdd_adapter *adapter);

#if defined(WLAN_FEATURE_FILS_SK) && \
	(defined(CFG80211_FILS_SK_OFFLOAD_SUPPORT) || \
		 (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)))
/**
 * hdd_clear_fils_connection_info: API to clear fils info from roam profile and
 * free allocated memory
 * @adapter: pointer to hdd adapter
 *
 * Return: None
 */
void hdd_clear_fils_connection_info(struct hdd_adapter *adapter);

/**
 * hdd_update_hlp_info() - Update HLP packet received in FILS (re)assoc rsp
 * @dev: net device
 * @roam_fils_params: Fils join rsp params
 *
 * This API is used to send the received HLP packet in Assoc rsp(FILS AKM)
 * to the network layer.
 *
 * Return: None
 */
void hdd_update_hlp_info(struct net_device *dev,
			 struct csr_roam_info *roam_info);
#else
static inline void hdd_clear_fils_connection_info(struct hdd_adapter *adapter)
{ }
static inline void hdd_update_hlp_info(struct net_device *dev,
				       struct csr_roam_info *roam_info)
{}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static inline void hdd_dev_setup_destructor(struct net_device *dev)
{
	dev->destructor = free_netdev;
}
#else
static inline void hdd_dev_setup_destructor(struct net_device *dev)
{
	dev->needs_free_netdev = true;
}
#endif /* KERNEL_VERSION(4, 12, 0) */

/**
 * hdd_dp_trace_init() - initialize DP Trace by calling the QDF API
 * @config: hdd config
 *
 * Return: NONE
 */
void hdd_dp_trace_init(struct hdd_config *config);
void hdd_set_rx_mode_rps(bool enable);

/**
 * hdd_set_limit_off_chan_for_tos() - set limit off-chan command parameters
 * @adapter: pointer adapter context
 * @tos: type of service
 * @status: status of the traffic (active/inactive)
 *
 * This function updates the limit off-channel command parameters to WMA
 *
 * Return: 0 on success or non zero value on failure
 */
int hdd_set_limit_off_chan_for_tos(struct hdd_adapter *adapter, enum tos tos,
		bool is_tos_active);

/**
 * hdd_drv_ops_inactivity_handler() - Timeout handler for driver ops
 * inactivity timer
 *
 * Return: None
 */
void hdd_drv_ops_inactivity_handler(void);

/**
 * hdd_start_driver_ops_timer() - Starts driver ops inactivity timer
 * @drv_op: Enum indicating driver op
 *
 * Return: none
 */
void hdd_start_driver_ops_timer(int drv_op);

/**
 * hdd_stop_driver_ops_timer() - Stops driver ops inactivity timer
 *
 * Return: none
 */
void hdd_stop_driver_ops_timer(void);

/**
 * hdd_pld_ipa_uc_shutdown_pipes() - Disconnect IPA WDI pipes during PDR
 *
 * Return: None
 */
void hdd_pld_ipa_uc_shutdown_pipes(void);

/**
 * hdd_limit_max_per_index_score() -check if per index score doesnt exceed 100%
 * (0x64). If it exceed make it 100%
 *
 * @per_index_score: per_index_score as input
 *
 * Return: per_index_score within the max limit
 */
uint32_t hdd_limit_max_per_index_score(uint32_t per_index_score);
/**
 * hdd_get_stainfo() - get stainfo for the specified peer
 * @astainfo: array of the station info in which the sta info
 * corresponding to mac_addr needs to be searched
 * @mac_addr: mac address of requested peer
 *
 * This function find the stainfo for the peer with mac_addr
 *
 * Return: stainfo if found, NULL if not found
 */
struct hdd_station_info *hdd_get_stainfo(struct hdd_station_info *astainfo,
					 struct qdf_mac_addr mac_addr);

/**
 * hdd_component_psoc_enable() - Trigger psoc enable for CLD Components
 *
 * Return: None
 */
void hdd_component_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * hdd_component_psoc_disable() - Trigger psoc disable for CLD Components
 *
 * Return: None
 */
void hdd_component_psoc_disable(struct wlan_objmgr_psoc *psoc);

int hdd_driver_memdump_init(void);
void hdd_driver_memdump_deinit(void);

/**
 * hdd_check_green_ap_enable() - to check whether to enable green ap or not
 * @hdd_ctx: hdd context
 * @is_enabled: 1 - green ap enabled, 0 - green ap not enabled
 *
 * Return: 0 - success, < 0 - failure
 */
uint8_t hdd_check_green_ap_enable(struct hdd_context *hdd_ctx,
				     bool *is_enabled);

/**
 * hdd_is_cli_iface_up() - check if there is any cli iface up
 * @hdd_ctx: HDD context
 *
 * Return: return true if there is any cli iface(STA/P2P_CLI) is up
 *         else return false
 */
bool hdd_is_cli_iface_up(struct hdd_context *hdd_ctx);

#endif /* end #if !defined(WLAN_HDD_MAIN_H) */

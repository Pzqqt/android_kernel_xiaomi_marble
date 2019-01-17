/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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

#if !defined(HDD_CONFIG_H__)
#define HDD_CONFIG_H__

/**
 *
 * DOC: wlan_hdd_config.h
 *
 * WLAN Adapter Configuration functions
 */

/* $HEADER$ */

/* Include files */
#include <wlan_hdd_includes.h>
#include <wlan_hdd_wmm.h>
#include <qdf_types.h>
#include <csr_api.h>
#include <sap_api.h>
#include "osapi_linux.h"
#include <wmi_unified.h>
#include "wlan_pmo_hw_filter_public_struct.h"
#include "wlan_action_oui_public_struct.h"
#include "hdd_config.h"

struct hdd_context;

#define CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST_LEN 30

#define FW_MODULE_LOG_LEVEL_STRING_LENGTH  (512)
#define TX_SCHED_WRR_PARAMS_NUM            (5)

#ifdef DHCP_SERVER_OFFLOAD
#define IPADDR_NUM_ENTRIES     (4)
#define IPADDR_STRING_LENGTH   (16)
#endif

/* Number of items that can be configured */
#define MAX_CFG_INI_ITEMS   1024

/* Defines for all of the things we read from the configuration (registry). */
/*
 * <ini>
 * gEnableConnectedScan - Will enable or disable scan in connected state
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable the scanning in
 * Connected state
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * <ini>
 */

#define CFG_ENABLE_CONNECTED_SCAN_NAME        "gEnableConnectedScan"
#define CFG_ENABLE_CONNECTED_SCAN_MIN         (0)
#define CFG_ENABLE_CONNECTED_SCAN_MAX         (1)
#define CFG_ENABLE_CONNECTED_SCAN_DEFAULT     (1)

/*
 * <ini>
 * enable_11d_in_world_mode - enable 11d in world mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini enables 11d in world mode, irrespective of value of
 * g11dSupportEnabled
 *
 * Usage: External
 *
 * </ini>
 */
 #define CFG_ENABLE_11D_IN_WORLD_MODE_NAME "enable_11d_in_world_mode"
 #define CFG_ENABLE_11D_IN_WORLD_MODE_MIN     (0)
 #define CFG_ENABLE_11D_IN_WORLD_MODE_MAX     (1)
 #define CFG_ENABLE_11D_IN_WORLD_MODE_DEFAULT (0)

enum hdd_dot11_mode {
	eHDD_DOT11_MODE_AUTO = 0,       /* covers all things we support */
	eHDD_DOT11_MODE_abg,    /* 11a/b/g only, no HT, no proprietary */
	eHDD_DOT11_MODE_11b,
	eHDD_DOT11_MODE_11g,
	eHDD_DOT11_MODE_11n,
	eHDD_DOT11_MODE_11g_ONLY,
	eHDD_DOT11_MODE_11n_ONLY,
	eHDD_DOT11_MODE_11b_ONLY,
	eHDD_DOT11_MODE_11ac_ONLY,
	eHDD_DOT11_MODE_11ac,
	eHDD_DOT11_MODE_11a,
	eHDD_DOT11_MODE_11ax_ONLY,
	eHDD_DOT11_MODE_11ax,
};

/*
 * <ini>
 * gScanAgingTime - Set scan aging time
 * @Min: 0
 * @Max: 200
 * @Default: 30
 *
 * This ini is used to set scan aging timeout value
 * in secs. For example after 30 secs the bss results
 * greater than 30secs age will be flushed.
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SCAN_AGING_PARAM_NAME          "gScanAgingTime"
#define CFG_SCAN_AGING_PARAM_MIN           (0)
#define CFG_SCAN_AGING_PARAM_MAX           (200)
#ifdef QCA_WIFI_NAPIER_EMULATION
#define CFG_SCAN_AGING_PARAM_DEFAULT       (90)
#else
#define CFG_SCAN_AGING_PARAM_DEFAULT       (30)
#endif

/*
 * <ini>
 * extscan_adaptive_dwell_mode - Enable adaptive dwell mode
 * during ext scan
 * @Min: 0
 * @Max: 4
 * @Default: 1
 *
 * This ini will set the algo used in dwell time optimization
 * during ext scan. see enum scan_dwelltime_adaptive_mode.
 * Acceptable values for this:
 * 0: Default (Use firmware default mode)
 * 1: Conservative optimization
 * 2: Moderate optimization
 * 3: Aggressive optimization
 * 4: Static
 *
 * Related: None
 *
 * Supported Feature: Scan
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_NAME     "extscan_adaptive_dwell_mode"
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MIN      (0)
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_MAX      (4)
#define CFG_ADAPTIVE_EXTSCAN_DWELL_MODE_DEFAULT  (1)

#ifdef FEATURE_LFR_SUBNET_DETECTION
/*
 * <ini>
 * gLFRSubnetDetectionEnable - Enable LFR3 subnet detection
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Enable IP subnet detection during legacy fast roming version 3. Legacy fast
 * roaming could roam across IP subnets without host processors' knowledge.
 * This feature enables firmware to wake up the host processor if it
 * successfully determines change in the IP subnet. Change in IP subnet could
 * potentially cause disruption in IP connnectivity if IP address is not
 * refreshed.
 *
 * Related: None
 *
 * Supported Feature: Roaming
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_LFR_SUBNET_DETECTION    "gLFRSubnetDetectionEnable"
#define CFG_ENABLE_LFR_SUBNET_MIN          (0)
#define CFG_ENABLE_LFR_SUBNET_MAX          (1)
#define CFG_ENABLE_LFR_SUBNET_DEFAULT      (1)
#endif /* FEATURE_LFR_SUBNET_DETECTION */

/*
 * <ini>
 * gDot11Mode - SAP phy mode
 * @Min: 0
 * @Max: 12 (11ax)
 * @Default: 12 (11ax)
 *
 * This ini is used to set Phy Mode (auto, b, g, n, etc/) Valid values are
 * 0-12, with 0 = Auto, 12 = 11ax.
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DOT11_MODE_NAME                    "gDot11Mode"
#define CFG_DOT11_MODE_MIN                     eHDD_DOT11_MODE_AUTO
#define CFG_DOT11_MODE_DEFAULT                 eHDD_DOT11_MODE_11ax
#define CFG_DOT11_MODE_MAX                     eHDD_DOT11_MODE_11ax

/*
 * <ini>
 * gEnableApProt - Enable/Disable AP protection
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable AP protection
 *
 * Related: None.
 *
 * Supported Feature: SAP
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_AP_ENABLE_PROTECTION_MODE_NAME            "gEnableApProt"
#define CFG_AP_ENABLE_PROTECTION_MODE_MIN             (0)
#define CFG_AP_ENABLE_PROTECTION_MODE_MAX             (1)
#define CFG_AP_ENABLE_PROTECTION_MODE_DEFAULT         (1)

/* WMM configuration */
/*
 * <ini>
 * DelayedTriggerFrmInt - UAPSD delay interval
 * @Min: 1
 * @Max: 4294967295
 * @Default: 3000
 *
 * This parameter controls the delay interval(in ms) of UAPSD auto trigger.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_TL_DELAYED_TRGR_FRM_INT_NAME                 "DelayedTriggerFrmInt"
#define CFG_TL_DELAYED_TRGR_FRM_INT_MIN                  1
#define CFG_TL_DELAYED_TRGR_FRM_INT_MAX                  (4294967295UL)
#define CFG_TL_DELAYED_TRGR_FRM_INT_DEFAULT              3000

/*
 * <ini>
 * gEnableSNRMonitoring - Enables SNR Monitoring
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default snr monitor
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_SNR_MONITORING_NAME              "gEnableSNRMonitoring"
#define CFG_ENABLE_SNR_MONITORING_MIN               (0)
#define CFG_ENABLE_SNR_MONITORING_MAX               (1)
#define CFG_ENABLE_SNR_MONITORING_DEFAULT           (0)

#ifdef DHCP_SERVER_OFFLOAD
#define CFG_DHCP_SERVER_IP_NAME     "gDHCPServerIP"
#define CFG_DHCP_SERVER_IP_DEFAULT  ""
#endif /* DHCP_SERVER_OFFLOAD */

#define CFG_ENABLE_MAC_ADDR_SPOOFING                "gEnableMacAddrSpoof"
#define CFG_ENABLE_MAC_ADDR_SPOOFING_MIN            (0)
#define CFG_ENABLE_MAC_ADDR_SPOOFING_MAX            (1)
#define CFG_ENABLE_MAC_ADDR_SPOOFING_DEFAULT        (1)

/*
 * <ini>
 * gSAP11ACOverride - Override bw to 11ac for SAP in driver even if supplicant
 *                    or hostapd configures HT.
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable 11AC override for SAP.
 * Android UI does not provide advanced configuration options
 * for SoftAP for Android O and below.
 * Default override disabled for android. Can be enabled from
 * ini for Android O and below.
 *
 *
 * Supported Feature: SAP
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_SAP_11AC_OVERRIDE_NAME             "gSAP11ACOverride"
#define CFG_SAP_11AC_OVERRIDE_MIN              (0)
#define CFG_SAP_11AC_OVERRIDE_MAX              (1)
#define CFG_SAP_11AC_OVERRIDE_DEFAULT          (0)

/*
 * <ini>
 * gGO11ACOverride - Override bw to 11ac for P2P GO
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable/disable 11AC override for GO.
 * P2P GO also follows start_bss and since P2P GO could not be
 * configured to setup VHT channel width in wpa_supplicant, driver
 * can override 11AC.
 *
 *
 * Supported Feature: P2P
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_GO_11AC_OVERRIDE_NAME             "gGO11ACOverride"
#define CFG_GO_11AC_OVERRIDE_MIN              (0)
#define CFG_GO_11AC_OVERRIDE_MAX              (1)
#define CFG_GO_11AC_OVERRIDE_DEFAULT          (1)

#ifdef CONFIG_DP_TRACE
/* Max length of gDptraceConfig string. e.g.- "1, 6, 1, 62" */
#define DP_TRACE_CONFIG_STRING_LENGTH		(20)

/* At max 4 DP Trace config parameters are allowed. Refer - gDptraceConfig */
#define DP_TRACE_CONFIG_NUM_PARAMS		(4)

/*
 * Default value of live mode in case it cannot be determined from cfg string
 * gDptraceConfig
 */
#define DP_TRACE_CONFIG_DEFAULT_LIVE_MODE	(1)

/*
 * Default value of thresh (packets/second) beyond which DP Trace is disabled.
 * Use this default in case the value cannot be determined from cfg string
 * gDptraceConfig
 */
#define DP_TRACE_CONFIG_DEFAULT_THRESH		(6)

/*
 * Number of intervals of BW timer to wait before enabling/disabling DP Trace.
 * Since throughput threshold to disable live logging for DP Trace is very low,
 * we calculate throughput based on # packets received in a second.
 * For example assuming bandwidth timer interval is 100ms, and if more than 6
 * prints are received in 10 * 100 ms interval, we want to disable DP Trace
 * live logging. DP_TRACE_CONFIG_DEFAULT_THRESH_TIME_LIMIT is the default
 * value, to be used in case the real value cannot be derived from
 * bw timer interval
 */
#define DP_TRACE_CONFIG_DEFAULT_THRESH_TIME_LIMIT (10)

/* Default proto bitmap in case its missing in gDptraceConfig string */
#define DP_TRACE_CONFIG_DEFAULT_BITMAP \
			(QDF_NBUF_PKT_TRAC_TYPE_EAPOL |\
			QDF_NBUF_PKT_TRAC_TYPE_DHCP |\
			QDF_NBUF_PKT_TRAC_TYPE_MGMT_ACTION |\
			QDF_NBUF_PKT_TRAC_TYPE_ARP |\
			QDF_NBUF_PKT_TRAC_TYPE_ICMP |\
			QDF_NBUF_PKT_TRAC_TYPE_ICMPv6)\

/* Default verbosity, in case its missing in gDptraceConfig string*/
#define DP_TRACE_CONFIG_DEFAULT_VERBOSTY QDF_DP_TRACE_VERBOSITY_LOW

#endif

/*
 * <ini>
 * gEnableUnitTestFramework - Enable/Disable unit test framework
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * Usage: Internal (only for dev and test team)
 *
 * </ini>
 */
#define CFG_ENABLE_UNIT_TEST_FRAMEWORK_NAME    "gEnableUnitTestFramework"
#define CFG_ENABLE_UNIT_TEST_FRAMEWORK_MIN     (0)
#define CFG_ENABLE_UNIT_TEST_FRAMEWORK_MAX     (1)
#define CFG_ENABLE_UINT_TEST_FRAMEWORK_DEFAULT (0)

/*
 * <ini>
 * gEnableChangeChannelBandWidth - Enable/Disable change
 * channel&bandwidth in the mission mode
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * 0 - not allow change channel&bandwidth by setMonChan
 * 1 - allow change channel&bandwidth by setMonChan
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_CHANGE_CHANNEL_BANDWIDTH_NAME    "gEnableChangeChannelBandWidth"
#define CFG_CHANGE_CHANNEL_BANDWIDTH_MIN     (0)
#define CFG_CHANGE_CHANNEL_BANDWIDTH_MAX     (1)
#ifdef FEATURE_MONITOR_MODE_SUPPORT
#define CFG_CHANGE_CHANNEL_BANDWIDTH_DEFAULT (1)
#else
#define CFG_CHANGE_CHANNEL_BANDWIDTH_DEFAULT (0)
#endif

/*
 * Type declarations
 */

struct hdd_config {
	/* Bitmap to track what is explicitly configured */
	DECLARE_BITMAP(bExplicitCfg, MAX_CFG_INI_ITEMS);

	/* Config parameters */
	bool enable_connected_scan;
	enum hdd_dot11_mode dot11Mode;
	bool apProtEnabled;

#ifdef FEATURE_WLAN_DYNAMIC_CVM
	/* Bitmap for operating voltage corner mode */
	uint32_t vc_mode_cfg_bitmap;
#endif
	uint32_t DelayedTriggerFrmInt;

#ifdef ENABLE_MTRACE_LOG
	bool enable_mtrace;
#endif
	bool prevent_link_down;
	uint8_t scanAgingTimeout;
	bool fEnableSNRMonitoring;
	bool advertiseConcurrentOperation;
#ifdef DHCP_SERVER_OFFLOAD
	uint8_t dhcpServerIP[IPADDR_STRING_LENGTH];
#endif /* DHCP_SERVER_OFFLOAD */
	bool enable_mac_spoofing;
	uint8_t sap_11ac_override;
	uint8_t go_11ac_override;
#ifdef FEATURE_LFR_SUBNET_DETECTION
	bool enable_lfr_subnet_detection;
#endif
	bool apf_enabled;
	enum scan_dwelltime_adaptive_mode extscan_adaptive_dwell_mode;
	uint16_t sap_tx_leakage_threshold;
	bool sap_internal_restart;
	bool tx_orphan_enable;

	bool enable_11d_in_world_mode;
	bool is_11k_offload_supported;
	bool action_oui_enable;
	uint8_t action_oui_str[ACTION_OUI_MAXIMUM_ID][ACTION_OUI_MAX_STR_LEN];
	bool is_unit_test_framework_enabled;
	bool enable_change_channel_bandwidth;

	/* HDD converged ini items are listed below this*/
	bool bug_on_reinit_failure;
	bool is_ramdump_enabled;
	uint32_t iface_change_wait_time;
	uint8_t multicast_host_fw_msgs;
	enum hdd_wext_control private_wext_control;
	uint32_t timer_multiplier;
	bool enablefwprint;
	uint8_t enable_fw_log;

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
	/* WLAN Logging */
	bool wlan_logging_enable;
	bool wlan_logging_to_console;
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	uint32_t wlan_auto_shutdown;
#endif

#ifndef REMOVE_PKT_LOG
	bool enable_packet_log;
#endif
	uint32_t rx_mode;
#ifdef MSM_PLATFORM
	uint32_t bus_bw_high_threshold;
	uint32_t bus_bw_medium_threshold;
	uint32_t bus_bw_low_threshold;
	uint32_t bus_bw_compute_interval;
	uint32_t enable_tcp_delack;
	bool     enable_tcp_limit_output;
	uint32_t enable_tcp_adv_win_scale;
	uint32_t tcp_delack_thres_high;
	uint32_t tcp_delack_thres_low;
	uint32_t tcp_tx_high_tput_thres;
	uint32_t tcp_delack_timer_count;
	bool     enable_tcp_param_update;
#endif /* MSM_PLATFORM */
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	uint32_t tx_flow_low_watermark;
	uint32_t tx_flow_hi_watermark_offset;
	uint32_t tx_flow_max_queue_depth;
	uint32_t tx_lbw_flow_low_watermark;
	uint32_t tx_lbw_flow_hi_watermark_offset;
	uint32_t tx_lbw_flow_max_queue_depth;
	uint32_t tx_hbw_flow_low_watermark;
	uint32_t tx_hbw_flow_hi_watermark_offset;
	uint32_t tx_hbw_flow_max_queue_depth;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
	uint32_t napi_cpu_affinity_mask;
	/* CPU affinity mask for rx_thread */
	uint32_t rx_thread_affinity_mask;
	uint8_t cpu_map_list[CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST_LEN];
	bool multicast_replay_filter;
	uint32_t rx_wakelock_timeout;
	uint8_t num_dp_rx_threads;
#ifdef CONFIG_DP_TRACE
	bool enable_dp_trace;
	uint8_t dp_trace_config[DP_TRACE_CONFIG_STRING_LENGTH];
#endif
#ifdef WLAN_NUD_TRACKING
	bool enable_nud_tracking;
#endif
	uint8_t operating_channel;
	uint8_t num_vdevs;
	uint8_t enable_concurrent_sta[CFG_CONCURRENT_IFACE_MAX_LEN];
	uint8_t dbs_scan_selection[CFG_DBS_SCAN_PARAM_LENGTH];
#ifdef FEATURE_RUNTIME_PM
	bool runtime_pm;
#endif
	uint8_t inform_bss_rssi_raw;

	bool mac_provision;
	uint32_t provisioned_intf_pool;
	uint32_t derived_intf_pool;
	uint8_t enable_rtt_support;
};

#define VAR_OFFSET(_Struct, _Var) (offsetof(_Struct, _Var))
#define VAR_SIZE(_Struct, _Var) (sizeof(((_Struct *)0)->_Var))

#define VAR_FLAGS_NONE         (0)

/* bit 0 is Required or Optional */
#define VAR_FLAGS_REQUIRED     (1 << 0)
#define VAR_FLAGS_OPTIONAL     (0 << 0)

/*
 * bit 1 tells if range checking is required.
 * If less than MIN, assume MIN.
 * If greater than MAX, assume MAX.
 */
#define VAR_FLAGS_RANGE_CHECK  (1 << 1)
#define VAR_FLAGS_RANGE_CHECK_ASSUME_MINMAX (VAR_FLAGS_RANGE_CHECK)

/*
 * bit 2 is range checking that assumes the DEFAULT value
 * If less than MIN, assume DEFAULT,
 * If greater than MAX, assume DEFAULT.
 */
#define VAR_FLAGS_RANGE_CHECK_ASSUME_DEFAULT (1 << 2)

/*
 * Bit 3 indicates that the config item can be modified dynamicially
 * on a running system
 */
#define VAR_FLAGS_DYNAMIC_CFG (1 << 3)

enum wlan_parameter_type {
	WLAN_PARAM_Integer,
	WLAN_PARAM_SignedInteger,
	WLAN_PARAM_HexInteger,
	WLAN_PARAM_String,
	WLAN_PARAM_MacAddr,
};

#define REG_VARIABLE(_Name, _Type,  _Struct, _VarName,		\
		      _Flags, _Default, _Min, _Max)		\
	{							\
		(_Name),					\
		(_Type),					\
		(_Flags),					\
		0,						\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(_Default),					\
		(_Min),						\
		(_Max),						\
		NULL						\
	}

#define REG_DYNAMIC_VARIABLE(_Name, _Type,  _Struct, _VarName,	\
			      _Flags, _Default, _Min, _Max,	\
			      _CBFunc, _CBParam)		\
	{							\
		(_Name),					\
		(_Type),					\
		(VAR_FLAGS_DYNAMIC_CFG | (_Flags)),		\
		(_CBParam),					\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(_Default),					\
		(_Min),						\
		(_Max),						\
		(_CBFunc)					\
	}

#define REG_VARIABLE_STRING(_Name, _Type,  _Struct, _VarName,	\
			     _Flags, _Default)			\
	{							\
		(_Name),					\
		(_Type),					\
		(_Flags),					\
		0,						\
		VAR_OFFSET(_Struct, _VarName),			\
		VAR_SIZE(_Struct, _VarName),			\
		(unsigned long)(_Default),			\
		0,						\
		0,						\
		NULL						\
	}

struct reg_table_entry {
	char *RegName;          /* variable name in the qcom_cfg.ini file */
	unsigned char RegType;    /* variable type in hdd_config struct */
	unsigned char Flags;    /* Specify optional parms and if RangeCheck is performed */
	unsigned char notifyId; /* Dynamic modification identifier */
	unsigned short VarOffset;       /* offset to field from the base address of the structure */
	unsigned short VarSize; /* size (in bytes) of the field */
	unsigned long VarDefault;       /* default value to use */
	unsigned long VarMin;   /* minimum value, for range checking */
	unsigned long VarMax;   /* maximum value, for range checking */
	/* Dynamic modification notifier */
	void (*pfnDynamicnotify)(struct hdd_context *hdd_ctx,
				 unsigned long notifyId);
};

/**
 * hdd_to_csr_wmm_mode() - Utility function to convert HDD to CSR WMM mode
 *
 * @uint8_t mode - hdd WMM user mode
 *
 * Return: CSR WMM mode
 */
eCsrRoamWmmUserModeType hdd_to_csr_wmm_mode(uint8_t mode);

/* Function declarations and documenation */
QDF_STATUS hdd_parse_config_ini(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_update_mac_config(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_sme_config(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_policy_mgr_user_cfg(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_set_sme_chan_list(struct hdd_context *hdd_ctx);
bool hdd_update_config_cfg(struct hdd_context *hdd_ctx);
void hdd_cfg_get_global_config(struct hdd_context *hdd_ctx, char *buf,
			       int buflen);

eCsrPhyMode hdd_cfg_xlate_to_csr_phy_mode(enum hdd_dot11_mode dot11Mode);
QDF_STATUS hdd_execute_global_config_command(struct hdd_context *hdd_ctx,
					     char *command);

QDF_STATUS hdd_set_idle_ps_config(struct hdd_context *hdd_ctx, bool val);
void hdd_get_pmkid_modes(struct hdd_context *hdd_ctx,
			 struct pmkid_mode_bits *pmkid_modes);

void hdd_update_tgt_cfg(hdd_handle_t hdd_handle, struct wma_tgt_cfg *cfg);

/**
 * hdd_string_to_u8_array() - used to convert decimal string into u8 array
 * @str: Decimal string
 * @array: Array where converted value is stored
 * @len: Length of the populated array
 * @array_max_len: Maximum length of the array
 *
 * This API is called to convert decimal string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_string_to_u8_array(char *str, uint8_t *array,
				  uint8_t *len, uint16_t array_max_len);

QDF_STATUS hdd_hex_string_to_u16_array(char *str, uint16_t *int_array,
				uint8_t *len, uint8_t int_array_max_len);

void hdd_cfg_print_global_config(struct hdd_context *hdd_ctx);

QDF_STATUS hdd_update_nss(struct hdd_adapter *adapter, uint8_t nss);

/**
 * hdd_dfs_indicate_radar() - Block tx as radar found on the channel
 * @hdd_ctxt: HDD context pointer
 *
 * This function is invoked in atomic context when a radar
 * is found on the SAP current operating channel and Data Tx
 * from netif has to be stopped to honor the DFS regulations.
 * Actions: Stop the netif Tx queues,Indicate Radar present
 * in HDD context for future usage.
 *
 * Return: true on success, else false
 */
bool hdd_dfs_indicate_radar(struct hdd_context *hdd_ctx);

/**
 * hdd_override_all_ps() - overrides to disables all the powersave features.
 * @hdd_ctx: Pointer to HDD context.
 * Overrides below powersave ini configurations.
 * gEnableImps=0
 * gEnableBmps=0
 * gRuntimePM=0
 * gWlanAutoShutdown = 0
 * gEnableSuspend=0
 * gEnablePowerSaveOffload=0
 * gEnableWoW=0
 *
 * Return: None
 */
void hdd_override_all_ps(struct hdd_context *hdd_ctx);
#endif

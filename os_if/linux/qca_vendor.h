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

/**
 * DOC: declares vendor commands interfacing with linux kernel
 */


#ifndef _WLAN_QCA_VENDOR_H_
#define _WLAN_QCA_VENDOR_H_

/* Vendor id to be used in vendor specific command and events
 * to user space.
 * NOTE: The authoritative place for definition of QCA_NL80211_VENDOR_ID,
 * vendor subcmd definitions prefixed with QCA_NL80211_VENDOR_SUBCMD, and
 * qca_wlan_vendor_attr is open source file src/common/qca-vendor.h in
 * git://w1.fi/srv/git/hostap.git; the values here are just a copy of that
 */

#define QCA_NL80211_VENDOR_ID           0x001374

/**
 * enum qca_nl80211_vendor_subcmds: NL 80211 vendor sub command
 *
 * @QCA_NL80211_VENDOR_SUBCMD_UNSPEC: Unspecified
 * @QCA_NL80211_VENDOR_SUBCMD_TEST: Test
 *	Sub commands 2 to 8 are not used
 * @QCA_NL80211_VENDOR_SUBCMD_ROAMING: Roaming
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY: Avoid frequency.
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY: DFS capability
 * @QCA_NL80211_VENDOR_SUBCMD_NAN: Nan
 * @QCA_NL80211_VENDOR_SUBCMD_STATS_EXT: Ext stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET: Link layer stats set
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET: Link layer stats get
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR: Link layer stats clear
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS: Link layer stats radio
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS: Link layer stats interface
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS: Link layer stats peer
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START: Ext scan start
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP: Ext scan stop
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS: Ext scan get valid
 *	channels
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES: Ext scan get capability
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS: Ext scan get cached
 *	results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE: Ext scan results
 *	available. Used when report_threshold is reached in scan cache.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT: Ext scan full scan
 *	result. Used to report scan results when each probe rsp. is received,
 *	if report_events enabled in wifi_scan_cmd_params.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT: Ext scan event from target.
 *	Indicates progress of scanning state-machine.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND: Ext scan hotlist
 *	ap found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST: Ext scan set hotlist
 *	bssid
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST: Ext scan reset
 *	hotlist bssid
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE: Ext scan significant
 *	change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE: Ext scan set
 *	significant change
 *	ap found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE: Ext scan reset
 *	significant change
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE: Ext tdls enable
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE: Ext tdls disable
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS: Ext tdls get status
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE: Ext tdls state
 * @QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES: Get supported features
 * @QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI: Set scanning_mac_oui
 * @QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG: No DFS flag
 * @QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX: Get Concurrency Matrix
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY: Get the key mgmt offload keys
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH: After roaming, send the
 * roaming and auth information.
 * @QCA_NL80211_VENDOR_SUBCMD_OCB_SET_SCHED: Set OCB schedule
 * @QCA_NL80211_VENDOR_SUBCMD_DO_ACS: ACS offload flag
 * @QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES: Get the supported features by the
 * driver.
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED: Indicate that driver
 *	started CAC on DFS channel
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED: Indicate that driver
 * 	completed the CAC check on DFS channel
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED: Indicate that the CAC
 * 	check was aborted by the driver
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED: Indicate that the
 * 	driver completed NOP
 * @QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED: Indicate that the
 * 	driver detected radar signal on the current operating channel
 * @QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO: get wlan driver information
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START: start wifi logger
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP: memory dump request
 * @QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET: get logger feature set
 * @QCA_NL80211_VENDOR_SUBCMD_ROAM: roam
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST: extscan set ssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST:
 *	extscan reset ssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND: hotlist ssid found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST: hotlist ssid lost
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST: set pno list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST: set passpoint list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST:
 *	reset passpoint list
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND: pno network found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND:
 *	passpoint network found
 * @QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION: set wifi config
 * @QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION: get wifi config
 * @QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET: get logging features
 * @QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES: get link properties
 * @QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG: set gateway parameters
 * @QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST: get preferred channel
	list
 * @QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL: channel hint
 * @QCA_NL80211_VENDOR_SUBCMD_SETBAND: vendor setband command
 * @QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN: venodr scan command
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE: vendor scan complete
 * @QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN: vendor abort scan
 * @QCA_NL80211_VENDOR_SUBCMD_OTA_TEST: enable OTA test
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE: set tx power by percentage
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB: reduce tx power by DB
 * @QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG: SAP configuration
 * @QCA_NL80211_VENDOR_SUBCMD_TSF: TSF operations command
 * @QCA_NL80211_VENDOR_SUBCMD_WISA: WISA mode configuration
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START: Command used to
 *	start the P2P Listen Offload function in device and pass the listen
 *	channel, period, interval, count, number of device types, device
 *	types and vendor information elements to device driver and firmware.
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP: Command/event used to
 *	indicate stop request/response of the P2P Listen Offload function in
 *	device. As an event, it indicates either the feature stopped after it
 *	was already running or feature has actually failed to start.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_STATION: send BSS Information
 * @QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH: After SAP starts
 *     beaconing, this sub command provides the driver, the frequencies on the
 *     5 GHz to check for any radar activity. Driver selects one channel from
 *     this priority list provided through
 *     @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST and starts
 *     to check for radar activity on it. If no radar activity is detected
 *     during the channel availability check period, driver internally switches
 *     to the selected frequency of operation. If the frequency is zero, driver
 *     internally selects a channel. The status of this conditional switch is
 *     indicated through an event using the same sub command through
 *     @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS. Attributes are
 *     listed in qca_wlan_vendor_attr_sap_conditional_chan_switch
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT: Command/event used to config
 *      indication period and threshold for MAC layer counters.
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS: Configure the TDLS behavior
 *	in the host driver. The different TDLS configurations are defined
 *	by the attributes in enum qca_wlan_vendor_attr_tdls_configuration.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES: Get HE related capabilities
 * @QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS:Set the Specific Absorption Rate
 *	(SAR) power limits. A critical regulation for FCC compliance, OEMs
 *	require methods to set SAR limits on TX power of WLAN/WWAN.
 *	enum qca_vendor_attr_sar_limits attributes are used with this command.
 * @QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS: Vendor command used to get/set
 *      configuration of vendor ACS.
 * @QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE: Vendor event carrying the
 *      requisite information leading to a power save failure. The information
 *      carried as part of this event is represented by the
 *      enum qca_attr_chip_power_save_failure attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET: Start/Stop the NUD statistics
 *      collection. Uses attributes defined in enum qca_attr_nud_stats_set.
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET: Get the NUD statistics. These
 *      statistics are represented by the enum qca_attr_nud_stats_get
 *      attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS: Sub-command to fetch
 *      the BSS transition status, whether accept or reject, for a list of
 *      candidate BSSIDs provided by the userspace. This uses the vendor
 *      attributes QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON and
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO. The userspace shall specify
 *      the attributes QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON and an
 *      array of QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID nested in
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO in the request. In the response
 *      the driver shall specify array of
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID and
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS pairs nested in
 *      QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO.
 * @QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL: Set the trace level for a
 *      specific QCA module. The trace levels are represented by
 *      enum qca_attr_trace_level attributes.
 * @QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT: Set the Beam Refinement
 *      Protocol antenna limit in different modes. See enum
 *      qca_wlan_vendor_attr_brp_ant_limit_mode.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START: Start spectral scan. The scan
 *      parameters are specified by enum qca_wlan_vendor_attr_spectral_scan.
 *      This returns a cookie (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE)
 *      identifying the operation in success case.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP: Stop spectral scan. This uses
 *      a cookie (%QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE) from
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START to identify the scan to
 *      be stopped.
 * @QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS: Set the active Type Of Service on the
 *     specific interface. This can be used to modify some of the low level
 *     scan parameters (off channel dwell time, home channel time) in the
 *     driver/firmware. These parameters are maintained within the host
 *     driver.
 *     This command is valid only when the interface is in the connected
 *     state.
 *     These scan parameters shall be reset by the driver/firmware once
 *     disconnected. The attributes used with this command are defined in
 *     enum qca_wlan_vendor_attr_active_tos.
 * @QCA_NL80211_VENDOR_SUBCMD_HANG: Event indicating to the user space that the
 *      driver has detected an internal failure. This event carries the
 *      information indicating the reason that triggered this detection. The
 *      attributes for this command are defined in
 *      enum qca_wlan_vendor_attr_hang.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG: Get the current values
 *     of spectral parameters used. The spectral scan parameters are specified
 *     by enum qca_wlan_vendor_attr_spectral_scan.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS: Get the debug stats
 *     for spectral scan functionality. The debug stats are specified by
 *     enum qca_wlan_vendor_attr_spectral_diag_stats.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO: Get spectral
 *     scan system capabilities. The capabilities are specified
 *     by enum qca_wlan_vendor_attr_spectral_cap.
 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS: Get the current
 *     status of spectral scan. The status values are specified
 *     by enum qca_wlan_vendor_attr_spectral_scan_status.
 * @QCA_NL80211_VENDOR_SUBCMD_HTT_STATS: Request the firmware
 *     DP stats for a particualr stats type for response evnet
 *     it carries the stats data sent from the FW
 * @QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO: Get vendor specific Representative
 *     RF Operating Parameter (RROP) information. The attributes for this
 *     information are defined in enum qca_wlan_vendor_attr_rrop_info. This is
 *     intended for use by external Auto Channel Selection applications.
 * @QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS: Get the Specific Absorption Rate
 *	(SAR) power limits. This is a companion to the command
 *	@QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS and is used to retrieve the
 *	settings currently in use. The attributes returned by this command are
 *	defined by enum qca_vendor_attr_sar_limits.
 * @QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH: Set MSDU queue depth threshold
 *	per peer per TID. Attributes for this command are define in
 *	enum qca_wlan_set_qdepth_thresh_attr
 * @QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION: Sub command to set WiFi
 *	test configuration. Attributes for this command are defined in
 *	enum qca_wlan_vendor_attr_wifi_test_config.
 *
 */

enum qca_nl80211_vendor_subcmds {
	QCA_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
	QCA_NL80211_VENDOR_SUBCMD_TEST = 1,
	QCA_NL80211_VENDOR_SUBCMD_ROAMING = 9,
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY = 10,
	QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY = 11,
	QCA_NL80211_VENDOR_SUBCMD_NAN = 12,
	QCA_NL80211_VENDOR_SUBCMD_STATS_EXT = 13,

	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET = 14,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET = 15,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR = 16,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_RADIO_RESULTS = 17,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_IFACE_RESULTS = 18,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_PEERS_RESULTS = 19,

	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START = 20,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP = 21,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS = 22,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES = 23,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS = 24,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE = 25,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT = 26,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT = 27,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND = 28,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST = 29,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST = 30,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE = 31,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE = 32,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE = 33,

	QCA_NL80211_VENDOR_SUBCMD_TDLS_ENABLE = 34,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_DISABLE = 35,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_STATUS = 36,
	QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE = 37,

	QCA_NL80211_VENDOR_SUBCMD_GET_SUPPORTED_FEATURES = 38,

	QCA_NL80211_VENDOR_SUBCMD_SCANNING_MAC_OUI = 39,
	QCA_NL80211_VENDOR_SUBCMD_NO_DFS_FLAG = 40,

	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST = 41,

	/* Get Concurrency Matrix */
	QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX = 42,

	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_SET_KEY = 50,
	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH = 51,
	QCA_NL80211_VENDOR_SUBCMD_APFIND = 52,

	/* Deprecated */
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_SCHED = 53,

	QCA_NL80211_VENDOR_SUBCMD_DO_ACS = 54,

	QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES = 55,

	/* Off loaded DFS events */
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED = 56,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED = 57,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED = 58,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED = 59,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED = 60,

	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_INFO = 61,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_START = 62,
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP = 63,
	QCA_NL80211_VENDOR_SUBCMD_ROAM = 64,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST = 65,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST = 66,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND = 67,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST = 68,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_LIST = 69,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_SET_PASSPOINT_LIST = 70,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_RESET_PASSPOINT_LIST = 71,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND = 72,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND = 73,

	/* Wi-Fi Configuration subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION = 74,
	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION = 75,
	QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET = 76,
	QCA_NL80211_VENDOR_SUBCMD_GET_RING_DATA = 77,

	QCA_NL80211_VENDOR_SUBCMD_TDLS_GET_CAPABILITIES = 78,
	QCA_NL80211_VENDOR_SUBCMD_OFFLOADED_PACKETS = 79,
	QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI = 80,
	QCA_NL80211_VENDOR_SUBCMD_NDP = 81,

	/* NS Offload enable/disable cmd */
	QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD = 82,

	QCA_NL80211_VENDOR_SUBCMD_PACKET_FILTER = 83,
	QCA_NL80211_VENDOR_SUBCMD_GET_BUS_SIZE = 84,

	QCA_NL80211_VENDOR_SUBCMD_GET_WAKE_REASON_STATS = 85,

	QCA_NL80211_VENDOR_SUBCMD_DATA_OFFLOAD = 91,
	/* OCB commands */
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_CONFIG = 92,
	QCA_NL80211_VENDOR_SUBCMD_OCB_SET_UTC_TIME = 93,
	QCA_NL80211_VENDOR_SUBCMD_OCB_START_TIMING_ADVERT = 94,
	QCA_NL80211_VENDOR_SUBCMD_OCB_STOP_TIMING_ADVERT = 95,
	QCA_NL80211_VENDOR_SUBCMD_OCB_GET_TSF_TIMER = 96,
	QCA_NL80211_VENDOR_SUBCMD_DCC_GET_STATS = 97,
	QCA_NL80211_VENDOR_SUBCMD_DCC_CLEAR_STATS = 98,
	QCA_NL80211_VENDOR_SUBCMD_DCC_UPDATE_NDL = 99,
	QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT = 100,

	/* subcommand to get link properties */
	QCA_NL80211_VENDOR_SUBCMD_LINK_PROPERTIES = 101,
	/* LFR Subnet Detection */
	QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG = 102,

	/* DBS subcommands */
	QCA_NL80211_VENDOR_SUBCMD_GET_PREFERRED_FREQ_LIST = 103,
	QCA_NL80211_VENDOR_SUBCMD_SET_PROBABLE_OPER_CHANNEL = 104,

	/* Vendor setband command */
	QCA_NL80211_VENDOR_SUBCMD_SETBAND = 105,

	/* Vendor scan commands */
	QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN = 106,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE = 107,

	/* OTA test subcommand */
	QCA_NL80211_VENDOR_SUBCMD_OTA_TEST = 108,
	/* Tx power scaling subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE = 109,
	/* Tx power scaling in db subcommands */
	QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_SCALE_DECR_DB = 115,
	QCA_NL80211_VENDOR_SUBCMD_ACS_POLICY = 116,
	QCA_NL80211_VENDOR_SUBCMD_STA_CONNECT_ROAM_POLICY = 117,
	QCA_NL80211_VENDOR_SUBCMD_SET_SAP_CONFIG  = 118,
	QCA_NL80211_VENDOR_SUBCMD_TSF = 119,
	QCA_NL80211_VENDOR_SUBCMD_WISA = 120,
	QCA_NL80211_VENDOR_SUBCMD_GET_STATION = 121,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_START = 122,
	QCA_NL80211_VENDOR_SUBCMD_P2P_LISTEN_OFFLOAD_STOP = 123,
	QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH = 124,
	QCA_NL80211_VENDOR_SUBCMD_GPIO_CONFIG_COMMAND = 125,

	QCA_NL80211_VENDOR_SUBCMD_GET_HW_CAPABILITY = 126,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT = 127,
	/* FTM/indoor location subcommands */
	QCA_NL80211_VENDOR_SUBCMD_LOC_GET_CAPA = 128,
	QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION = 129,
	QCA_NL80211_VENDOR_SUBCMD_FTM_ABORT_SESSION = 130,
	QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT = 131,
	QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE = 132,
	QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER = 133,
	QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS = 134,
	QCA_NL80211_VENDOR_SUBCMD_AOA_ABORT_MEAS = 135,
	QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT = 136,

	/* Encrypt/Decrypt command */
	QCA_NL80211_VENDOR_SUBCMD_ENCRYPTION_TEST = 137,

	QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI = 138,
	/* DMG low level RF sector operations */
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SECTOR_CFG = 139,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SECTOR_CFG = 140,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SELECTED_SECTOR = 141,
	QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SELECTED_SECTOR = 142,

	/* Configure the TDLS mode from user space */
	QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS = 143,

	QCA_NL80211_VENDOR_SUBCMD_GET_HE_CAPABILITIES = 144,

	/* Vendor abort scan command */
	QCA_NL80211_VENDOR_SUBCMD_ABORT_SCAN = 145,

	/* Set Specific Absorption Rate(SAR) Power Limits */
	QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS = 146,

	/* External Auto channel configuration setting */
	QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS = 147,

	QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE = 148,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET = 149,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET = 150,
	QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS = 151,

	/* Set the trace level for QDF */
	QCA_NL80211_VENDOR_SUBCMD_SET_TRACE_LEVEL = 152,

	QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT = 153,

	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START = 154,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP = 155,
	QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS = 156,
	QCA_NL80211_VENDOR_SUBCMD_HANG = 157,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CONFIG = 158,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS = 159,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO = 160,
	QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS = 161,
	QCA_NL80211_VENDOR_SUBCMD_HTT_STATS = 162,
	QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO = 163,
	QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS = 164,
	QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH = 166,
	/* Wi-Fi test configuration subcommand */
	QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION = 169,
};

enum qca_wlan_vendor_tos {
	QCA_WLAN_VENDOR_TOS_BK = 0,
	QCA_WLAN_VENDOR_TOS_BE = 1,
	QCA_WLAN_VENDOR_TOS_VI = 2,
	QCA_WLAN_VENDOR_TOS_VO = 3,
};

/**
 * enum qca_wlan_vendor_attr_active_tos - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_ACTIVE_TOS.
 */
enum qca_wlan_vendor_attr_active_tos {
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_INVALID = 0,
	/* Type Of Service - Represented by qca_wlan_vendor_tos */
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS = 1,
	/* Flag attribute representing the start (attribute included) or stop
	 * (attribute not included) of the respective TOS.
	 */
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_START = 2,
	QCA_WLAN_VENDOR_ATTR_ACTIVE_TOS_MAX = 3,
};

enum qca_wlan_vendor_hang_reason {
	/* Unspecified reason */
	QCA_WLAN_HANG_REASON_UNSPECIFIED = 0,
	/* No Map for the MAC entry for the received frame */
	QCA_WLAN_HANG_RX_HASH_NO_ENTRY_FOUND = 1,
	/* peer deletion timeout happened */
	QCA_WLAN_HANG_PEER_DELETION_TIMEDOUT = 2,
	/* peer unmap timeout */
	QCA_WLAN_HANG_PEER_UNMAP_TIMEDOUT = 3,
	/* Scan request timed out */
	QCA_WLAN_HANG_SCAN_REQ_EXPIRED = 4,
	/* Consecutive Scan attempt failures */
	QCA_WLAN_HANG_SCAN_ATTEMPT_FAILURES = 5,
	/* Unable to get the message buffer */
	QCA_WLAN_HANG_GET_MSG_BUFF_FAILURE = 6,
	/* Current command processing is timedout */
	QCA_WLAN_HANG_ACTIVE_LIST_TIMEOUT = 7,
	/* Timeout for an ACK from FW for suspend request */
	QCA_WLAN_HANG_SUSPEND_TIMEOUT = 8,
	/* Timeout for an ACK from FW for resume request */
	QCA_WLAN_HANG_RESUME_TIMEOUT = 9,
	/* Transmission timeout for consecutive data frames */
	QCA_WLAN_HANG_TRANSMISSIONS_TIMEOUT = 10,
	/* Timeout for the TX completion status of data frame */
	QCA_WLAN_HANG_TX_COMPLETE_TIMEOUT = 11,
	/* DXE failure for tx/Rx, DXE resource unavailability */
	QCA_WLAN_HANG_DXE_FAILURE = 12,
	/* WMI pending commands exceed the maximum count */
	QCA_WLAN_HANG_WMI_EXCEED_MAX_PENDING_CMDS = 13,
};

/**
 * enum qca_wlan_vendor_attr_hang - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_HANG.
 */
enum qca_wlan_vendor_attr_hang {
	QCA_WLAN_VENDOR_ATTR_HANG_INVALID = 0,
	/*
	 * Reason for the Hang - Represented by enum
	 * qca_wlan_vendor_hang_reason.
	 */
	QCA_WLAN_VENDOR_ATTR_HANG_REASON = 1,

	QCA_WLAN_VENDOR_ATTR_HANG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_HANG_MAX =
		QCA_WLAN_VENDOR_ATTR_HANG_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_attr_set_trace_level - Config params for QDF set trace level
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_INVALID: Invalid trace level
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM : Trace level parameters
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID : Module of which trace
    level needs to be updated.
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK : verbose mask, which need
 * to be set.
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_AFTER_LAST : after last.
 * @QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX : Max attribute.
 */
enum qca_vendor_attr_set_trace_level {
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_INVALID = 0,
	/*
	* Array of QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM
	* attributes.
	*/
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_PARAM = 1,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MODULE_ID = 2,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_TRACE_MASK = 3,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_MAX =
		QCA_WLAN_VENDOR_ATTR_SET_TRACE_LEVEL_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_get_station - Sub commands used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_STATION to get the corresponding
 * station information. The information obtained through these
 * commands signify the current info in connected state and
 * latest cached information during the connected state , if queried
 * when in disconnected state.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INVALID: Invalid attribute
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO: bss info
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_ASSOC_FAIL_REASON: assoc fail reason
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_REMOTE: remote station info
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_AFTER_LAST: After last
 */
enum qca_wlan_vendor_attr_get_station {
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_ASSOC_FAIL_REASON,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_REMOTE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_STATION_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_STATION_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_802_11_mode - dot11 mode
 * @QCA_WLAN_802_11_MODE_11B: mode B
 * @QCA_WLAN_802_11_MODE_11G: mode G
 * @QCA_WLAN_802_11_MODE_11N: mode N
 * @QCA_WLAN_802_11_MODE_11A: mode A
 * @QCA_WLAN_802_11_MODE_11AC: mode AC
 * @QCA_WLAN_802_11_MODE_INVALID: Invalid dot11 mode
 */
enum qca_wlan_802_11_mode {
	QCA_WLAN_802_11_MODE_11B,
	QCA_WLAN_802_11_MODE_11G,
	QCA_WLAN_802_11_MODE_11N,
	QCA_WLAN_802_11_MODE_11A,
	QCA_WLAN_802_11_MODE_11AC,
	QCA_WLAN_802_11_MODE_INVALID,
};

/**
 * enum qca_wlan_auth_type - Authentication key management type
 * @QCA_WLAN_AUTH_TYPE_INVALID: Invalid key management type
 * @QCA_WLAN_AUTH_TYPE_OPEN: Open key
 * @QCA_WLAN_AUTH_TYPE_SHARED: shared key
 * @QCA_WLAN_AUTH_TYPE_WPA: wpa key
 * @QCA_WLAN_AUTH_TYPE_WPA_PSK: wpa psk key
 * @QCA_WLAN_AUTH_TYPE_WPA_NONE: wpa none key
 * @QCA_WLAN_AUTH_TYPE_RSN: rsn key
 * @QCA_WLAN_AUTH_TYPE_RSN_PSK: rsn psk key
 * @QCA_WLAN_AUTH_TYPE_FT: ft key
 * @QCA_WLAN_AUTH_TYPE_FT_PSK: ft psk key
 * @QCA_WLAN_AUTH_TYPE_SHA256: shared 256 key
 * @QCA_WLAN_AUTH_TYPE_SHA256_PSK: shared 256 psk
 * @QCA_WLAN_AUTH_TYPE_WAI: wai key
 * @QCA_WLAN_AUTH_TYPE_WAI_PSK wai psk key
 * @QCA_WLAN_AUTH_TYPE_CCKM_WPA: cckm wpa key
 * @QCA_WLAN_AUTH_TYPE_CCKM_RSN: cckm rsn key
 */
enum qca_wlan_auth_type {
	QCA_WLAN_AUTH_TYPE_INVALID,
	QCA_WLAN_AUTH_TYPE_OPEN,
	QCA_WLAN_AUTH_TYPE_SHARED,
	QCA_WLAN_AUTH_TYPE_WPA,
	QCA_WLAN_AUTH_TYPE_WPA_PSK,
	QCA_WLAN_AUTH_TYPE_WPA_NONE,
	QCA_WLAN_AUTH_TYPE_RSN,
	QCA_WLAN_AUTH_TYPE_RSN_PSK,
	QCA_WLAN_AUTH_TYPE_FT,
	QCA_WLAN_AUTH_TYPE_FT_PSK,
	QCA_WLAN_AUTH_TYPE_SHA256,
	QCA_WLAN_AUTH_TYPE_SHA256_PSK,
	QCA_WLAN_AUTH_TYPE_WAI,
	QCA_WLAN_AUTH_TYPE_WAI_PSK,
	QCA_WLAN_AUTH_TYPE_CCKM_WPA,
	QCA_WLAN_AUTH_TYPE_CCKM_RSN,
	QCA_WLAN_AUTH_TYPE_AUTOSWITCH,
};

/**
 * enum qca_wlan_vendor_attr_get_station_info - Station Info queried
 * through QCA_NL80211_VENDOR_SUBCMD_GET_STATION.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_INVALID: Invalid Attribute
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_LINK_STANDARD_NL80211_ATTR:
 *  Get the standard NL attributes Nested with this attribute.
 *  Ex : Query BW , BITRATE32 , NSS , Signal , Noise of the Link -
 *  NL80211_ATTR_SSID / NL80211_ATTR_SURVEY_INFO (Connected Channel) /
 *  NL80211_ATTR_STA_INFO
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_STANDARD_NL80211_ATTR:
 *  Get the standard NL attributes Nested with this attribute.
 *  Ex : Query HT/VHT Capability advertized by the AP.
 *  NL80211_ATTR_VHT_CAPABILITY / NL80211_ATTR_HT_CAPABILITY
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ROAM_COUNT:
 *  Number of successful Roam attempts before a
 *  disconnect, Unsigned 32 bit value
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AKM:
 *  Authentication Key Management Type used for the connected session.
 *  Signified by enum qca_wlan_auth_type
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_802_11_MODE: 802.11 Mode of the
 *  connected Session, signified by enum qca_wlan_802_11_mode
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_HS20_INDICATION:
 *  HS20 Indication Element
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_FAIL_REASON:
 *  Status Code Corresponding to the Association Failure.
 *  Unsigned 32 bit value.
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_MAX_PHY_RATE:
 *  Max phy rate of remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_PACKETS:
 *  TX packets to remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_BYTES:
 *  TX bytes to remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_PACKETS:
 *  RX packets from remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_BYTES:
 *  RX bytes from remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_TX_RATE:
 *  Last TX rate with remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_RX_RATE:
 *  Last RX rate with remote station
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_WMM:
 *  Remote station enable/disable WMM
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SUPPORTED_MODE:
 *  Remote staion connection mode
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_AMPDU:
 *  Remote station AMPDU enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_STBC:
 *  Remote station TX Space-time block coding enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_STBC:
 *  Remote station RX Space-time block coding enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_CH_WIDTH:
 *  Remote station channel width
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SGI_ENABLE:
 *  Remote station short GI enable/disable
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_PAD: Attribute type for padding
 * @QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AFTER_LAST: After last
 */
enum qca_wlan_vendor_attr_get_station_info {
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_LINK_STANDARD_NL80211_ATTR,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_STANDARD_NL80211_ATTR,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ROAM_COUNT,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AKM,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_802_11_MODE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AP_HS20_INDICATION,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_HT_OPERATION,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_VHT_OPERATION,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_ASSOC_FAIL_REASON,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_MAX_PHY_RATE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_PACKETS,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_BYTES,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_PACKETS,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_BYTES,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_TX_RATE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_LAST_RX_RATE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_WMM,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SUPPORTED_MODE,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_AMPDU,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_TX_STBC,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_RX_STBC,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_CH_WIDTH,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_REMOTE_SGI_ENABLE,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_PAD,
#endif
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_STATION_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_nl80211_vendor_subcmds_index - vendor sub commands index
 *
 * @QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_INDEX: Avoid frequency
 * @QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX: Nan
 * @QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX: Ext stats
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START_INDEX: Ext scan start
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP_INDEX: Ext scan stop
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES_INDEX: Ext scan get
 *	capability
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS_INDEX: Ext scan get
 *	cached results
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE_INDEX: Ext scan
 *	results available
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT_INDEX: Ext scan full
 *	scan result
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT_INDEX: Ext scan event
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND_INDEX: Ext scan hot list
 *	AP found
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST_INDEX: Ext scan set
 *	bssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST_INDEX: Ext scan reset
 *	bssid hotlist
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE_INDEX: Ext scan
 *	significant change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE_INDEX: Ext scan
 *	set significant change
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE_INDEX: Ext scan
 *	reset significant change
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET_INDEX: Set stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET_INDEX: Get stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR_INDEX: Clear stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_RADIO_STATS_INDEX: Radio stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_IFACE_STATS_INDEX: Iface stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_PEER_INFO_STATS_INDEX: Peer info stats
 * @QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT_INDEX: MAC layer counters
 * @QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX: Ext tdls state change
 * @QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX: ACS command
 * @QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX: Pass Roam and Auth info
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST_INDEX: hotlist ap lost
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND_INDEX:
 *	pno network found index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND_INDEX:
 *	passpoint match found index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST_INDEX:
 *	set ssid hotlist index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST_INDEX:
 *	reset ssid hotlist index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND_INDEX:
 *	hotlist ssid found index
 * @QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST_INDEX:
 *	hotlist ssid lost index
 * @QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT_INDEX
 *	dcc stats event index
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX: vendor scan index
 * @QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX:
 *	vendor scan complete event  index
 * @QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG_INDEX:
 *	update gateway parameters index
 * @QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX: TSF response events index
 * @QCA_NL80211_VENDOR_SUBCMD_P2P_LO_EVENT_INDEX:
 *      P2P listen offload index
 * @QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH_INDEX: SAP
 *      conditional channel switch index
 * @QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET_INDEX: NUD DEBUG Stats index
 * @QCA_NL80211_VENDOR_SUBCMD_HANG_REASON_INDEX: hang event reason index
 */

enum qca_nl80211_vendor_subcmds_index {
	QCA_NL80211_VENDOR_SUBCMD_AVOID_FREQUENCY_INDEX = 0,

#ifdef WLAN_FEATURE_NAN
	QCA_NL80211_VENDOR_SUBCMD_NAN_INDEX,
#endif /* WLAN_FEATURE_NAN */

#ifdef WLAN_FEATURE_STATS_EXT
	QCA_NL80211_VENDOR_SUBCMD_STATS_EXT_INDEX,
#endif /* WLAN_FEATURE_STATS_EXT */

#ifdef FEATURE_WLAN_EXTSCAN
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_STOP_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CAPABILITIES_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_BSSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_BSSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SIGNIFICANT_CHANGE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SIGNIFICANT_CHANGE_INDEX,
#endif /* FEATURE_WLAN_EXTSCAN */

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_SET_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_GET_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_CLR_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_RADIO_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_IFACE_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_PEER_INFO_STATS_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_EXT_INDEX,
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

	QCA_NL80211_VENDOR_SUBCMD_TDLS_STATE_CHANGE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DO_ACS_INDEX,
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX,
#endif
	/* DFS */
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_STARTED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_FINISHED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_ABORTED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_CAC_NOP_FINISHED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_DFS_OFFLOAD_RADAR_DETECTED_INDEX,
#ifdef FEATURE_WLAN_EXTSCAN
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_LOST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_NETWORK_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_PNO_PASSPOINT_NETWORK_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_RESET_SSID_HOTLIST_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_FOUND_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_SSID_LOST_INDEX,
#endif /* FEATURE_WLAN_EXTSCAN */
	QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_MONITOR_RSSI_INDEX,
#ifdef WLAN_FEATURE_MEMDUMP
	QCA_NL80211_VENDOR_SUBCMD_WIFI_LOGGER_MEMORY_DUMP_INDEX,
#endif /* WLAN_FEATURE_MEMDUMP */
	/* OCB events */
	QCA_NL80211_VENDOR_SUBCMD_DCC_STATS_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_GW_PARAM_CONFIG_INDEX,
#ifdef WLAN_FEATURE_TSF
	QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX,
#endif
#ifdef WLAN_FEATURE_NAN_DATAPATH
	QCA_NL80211_VENDOR_SUBCMD_NDP_INDEX,
#endif /* WLAN_FEATURE_NAN_DATAPATH */
	QCA_NL80211_VENDOR_SUBCMD_P2P_LO_EVENT_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_SAP_CONDITIONAL_CHAN_SWITCH_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_UPDATE_EXTERNAL_ACS_CONFIG,
	QCA_NL80211_VENDOR_SUBCMD_PWR_SAVE_FAIL_DETECTED_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_HANG_REASON_INDEX,
	QCA_NL80211_VENDOR_SUBCMD_HTT_STATS_INDEX,
};

/**
 * enum qca_wlan_vendor_attr_tdls_enable - TDLS enable attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR: An array of 6 x Unsigned 8-bit
 *	value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL: Signed 32-bit value, but lets
 *	keep as unsigned for now
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS: operating class
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS: Enable max latency in ms
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS: Enable min bandwidth
 *	in KBPS
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_enable {
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_GLOBAL_OPERATING_CLASS,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX_LATENCY_MS,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MIN_BANDWIDTH_KBPS,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_ENABLE_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_tdls_disable: tdls disable attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR: An array of 6 x Unsigned
 *	8-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_disable {
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_DISABLE_AFTER_LAST - 1,
};

/**
 * qca_chip_power_save_failure_reason: Power save failure reason
 * @QCA_CHIP_POWER_SAVE_FAILURE_REASON_PROTOCOL: Indicates power save failure
 * due to protocol/module.
 * @QCA_CHIP_POWER_SAVE_FAILURE_REASON_HARDWARE: power save failure
 * due to hardware
 */
enum qca_chip_power_save_failure_reason {
	QCA_CHIP_POWER_SAVE_FAILURE_REASON_PROTOCOL = 0,
	QCA_CHIP_POWER_SAVE_FAILURE_REASON_HARDWARE = 1,
};

/**
 * qca_attr_chip_power_save_failure: attributes to vendor subcmd
 * @QCA_NL80211_VENDOR_SUBCMD_CHIP_PWRSAVE_FAILURE. This carry the requisite
 * information leading to the power save failure.
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_INVALID : invalid
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_REASON : power save failure reason
 * represented by enum qca_chip_power_save_failure_reason
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_LAST : Last
 * @QCA_ATTR_CHIP_POWER_SAVE_FAILURE_MAX : Max value
 */
enum qca_attr_chip_power_save_failure {
	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_INVALID = 0,

	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_REASON = 1,

	/* keep last */
	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_LAST,
	QCA_ATTR_CHIP_POWER_SAVE_FAILURE_MAX =
		QCA_ATTR_CHIP_POWER_SAVE_FAILURE_LAST - 1,
};


/**
 * enum qca_wlan_vendor_attr_tdls_get_status - tdls get status attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR: An array of 6 x Unsigned
 *	8-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE: get status state,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON: get status reason
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL: get status channel,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS: get operating
 *	class, unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_get_status {
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_STATE,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_REASON,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_GLOBAL_OPERATING_CLASS,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_GET_STATUS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_tdls_state - tdls state attribute
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_INVALID: Initial invalid value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR: An array of 6 x Unsigned
 *	8-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE: TDLS new state,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON: TDLS state reason
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL: TDLS state channel,
 *	unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS: TDLS state
 *	operating class, unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAX: Max value
 */
enum qca_wlan_vendor_attr_tdls_state {
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_TDLS_NEW_STATE,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_REASON,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_GLOBAL_OPERATING_CLASS,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_STATE_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_STATE_AFTER_LAST - 1,
};

/* enum's to provide TDLS capabilites */
enum qca_wlan_vendor_attr_get_tdls_capabilities {
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_MAX_CONC_SESSIONS = 1,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_FEATURES_SUPPORTED = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_GET_CAPS_AFTER_LAST - 1,
};

enum qca_wlan_vendor_attr {
	QCA_WLAN_VENDOR_ATTR_INVALID = 0,
	/* used by QCA_NL80211_VENDOR_SUBCMD_DFS_CAPABILITY */
	QCA_WLAN_VENDOR_ATTR_DFS     = 1,
	/* used by QCA_NL80211_VENDOR_SUBCMD_NAN */
	QCA_WLAN_VENDOR_ATTR_NAN     = 2,
	/* used by QCA_NL80211_VENDOR_SUBCMD_STATS_EXT */
	QCA_WLAN_VENDOR_ATTR_STATS_EXT     = 3,
	/* used by QCA_NL80211_VENDOR_SUBCMD_STATS_EXT */
	QCA_WLAN_VENDOR_ATTR_IFINDEX     = 4,
	/*
	 * used by QCA_NL80211_VENDOR_SUBCMD_ROAMING, u32 with values defined
	 * by enum qca_roaming_policy.
	 */
	QCA_WLAN_VENDOR_ATTR_ROAMING_POLICY = 5,
	QCA_WLAN_VENDOR_ATTR_MAC_ADDR = 6,
	/* used by QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES */
	QCA_WLAN_VENDOR_ATTR_FEATURE_FLAGS = 7,
	QCA_WLAN_VENDOR_ATTR_TEST = 8,
	/*
	 * used by QCA_NL80211_VENDOR_SUBCMD_GET_FEATURES
	 * Unsigned 32-bit value.
	 */
	QCA_WLAN_VENDOR_ATTR_CONCURRENCY_CAPA = 9,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_MAX_CONCURRENT_CHANNELS_2_4_BAND = 10,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_MAX_CONCURRENT_CHANNELS_5_0_BAND = 11,
	/* Unsigned 32-bit value from enum qca_set_band. */
	QCA_WLAN_VENDOR_ATTR_SETBAND_VALUE = 12,
	/* Dummy (NOP) attribute for 64 bit padding */
	QCA_WLAN_VENDOR_ATTR_PAD = 13,
	/*
	 * Unique FTM session cookie (Unsigned 64 bit). Specified in
	 * QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION. Reported in
	 * the session in QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT and
	 * QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_SESSION_COOKIE = 14,
	/*
	 * Indoor location capabilities, returned by
	 * QCA_NL80211_VENDOR_SUBCMD_LOC_GET_CAPA.
	 * see enum qca_wlan_vendor_attr_loc_capa.
	 */
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA = 15,
	/*
	 * Array of nested attributes containing information about each peer
	 * in FTM measurement session. See enum qca_wlan_vendor_attr_peer_info
	 * for supported attributes for each peer.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PEERS = 16,
	/*
	 * Array of nested attributes containing measurement results for
	 * one or more peers, reported by the
	 * QCA_NL80211_VENDOR_SUBCMD_FTM_MEAS_RESULT event.
	 * See enum qca_wlan_vendor_attr_peer_result for list of supported
	 * attributes.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PEER_RESULTS = 17,
	/* Flag attribute for enabling or disabling responder functionality. */
	QCA_WLAN_VENDOR_ATTR_FTM_RESPONDER_ENABLE = 18,
	/*
	 * Used in the QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER
	 * command to specify the LCI report that will be sent by
	 * the responder during a measurement exchange. The format is
	 * defined in IEEE P802.11-REVmc/D7.0, 9.4.2.22.10.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_LCI = 19,
	/*
	 * Used in the QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER
	 * command to specify the location civic report that will
	 * be sent by the responder during a measurement exchange.
	 * The format is defined in IEEE P802.11-REVmc/D7.0, 9.4.2.22.13.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_LCR = 20,
	/*
	 * Session/measurement completion status code,
	 * reported in QCA_NL80211_VENDOR_SUBCMD_FTM_SESSION_DONE and
	 * QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT
	 * see enum qca_vendor_attr_loc_session_status.
	 */
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS = 21,
	/*
	 * Initial dialog token used by responder (0 if not specified),
	 * unsigned 8 bit value.
	 */
	QCA_WLAN_VENDOR_ATTR_FTM_INITIAL_TOKEN = 22,
	/*
	 * AOA measurement type. Requested in QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS
	 * and optionally in QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION if
	 * AOA measurements are needed as part of an FTM session.
	 * Reported by QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS_RESULT. See
	 * enum qca_wlan_vendor_attr_aoa_type.
	 */
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE = 23,
	/*
	 * A bit mask (unsigned 32 bit value) of antenna arrays used
	 * by indoor location measurements. Refers to the antenna
	 * arrays described by QCA_VENDOR_ATTR_LOC_CAPA_ANTENNA_ARRAYS.
	 */
	QCA_WLAN_VENDOR_ATTR_LOC_ANTENNA_ARRAY_MASK = 24,
	/*
	 * AOA measurement data. Its contents depends on the AOA measurement
	 * type and antenna array mask:
	 * QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE: array of U16 values,
	 * phase of the strongest CIR path for each antenna in the measured
	 * array(s).
	 * QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE_AMP: array of 2 U16
	 * values, phase and amplitude of the strongest CIR path for each
	 * antenna in the measured array(s).
	 */
	QCA_WLAN_VENDOR_ATTR_AOA_MEAS_RESULT = 25,
	/*
	 *  Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to specify the chain number (unsigned 32 bit value) to inquire
	 * the corresponding antenna RSSI value */
	QCA_WLAN_VENDOR_ATTR_CHAIN_INDEX = 26,
	/*
	 *  Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to report the specific antenna RSSI value (unsigned 32 bit value) */
	QCA_WLAN_VENDOR_ATTR_CHAIN_RSSI = 27,
	/* Frequency in MHz, various uses. Unsigned 32 bit value */
	QCA_WLAN_VENDOR_ATTR_FREQ = 28,
	/*
	 *  TSF timer value, unsigned 64 bit value.
	 * May be returned by various commands.
	 */
	QCA_WLAN_VENDOR_ATTR_TSF = 29,
	/*
	 * DMG RF sector index, unsigned 16 bit number. Valid values are
	 * 0..127 for sector indices or 65535 as special value used to
	 * unlock sector selection in
	 * QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SELECTED_SECTOR.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_INDEX = 30,
	/*
	 * DMG RF sector type, unsigned 8 bit value. One of the values
	 * in enum qca_wlan_vendor_attr_dmg_rf_sector_type.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE = 31,
	/*
	 * Bitmask of DMG RF modules for which information is requested. Each
	 * bit corresponds to an RF module with the same index as the bit
	 * number. Unsigned 32 bit number but only low 8 bits can be set since
	 * all DMG chips currently have up to 8 RF modules.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_MODULE_MASK = 32,
	/*
	 * Array of nested attributes where each entry is DMG RF sector
	 * configuration for a single RF module.
	 * Attributes for each entry are taken from enum
	 * qca_wlan_vendor_attr_dmg_rf_sector_cfg.
	 * Specified in QCA_NL80211_VENDOR_SUBCMD_DMG_RF_SET_SECTOR_CFG
	 * and returned by QCA_NL80211_VENDOR_SUBCMD_DMG_RF_GET_SECTOR_CFG.
	 */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG = 33,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_STATS_EXT command
	 * to report frame aggregation statistics to userspace.
	 */
	QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_NUM = 34,
	QCA_WLAN_VENDOR_ATTR_RX_AGGREGATION_STATS_HOLES_INFO = 35,
	/*
	 * Unsigned 8-bit value representing MBO transition reason code as
	 * provided by the AP used by subcommand
	 * QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS. This is
	 * specified by the userspace in the request to the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_BTM_MBO_TRANSITION_REASON = 36,
	/*
	 * Array of nested attributes, BSSID and status code, used by subcommand
	 * QCA_NL80211_VENDOR_SUBCMD_FETCH_BSS_TRANSITION_STATUS, where each
	 * entry is taken from enum qca_wlan_vendor_attr_btm_candidate_info.
	 * The userspace space specifies the list/array of candidate BSSIDs in
	 * the order of preference in the request. The driver specifies the
	 * status code, for each BSSID in the list, in the response. The
	 * acceptable candidates are listed in the order preferred by the
	 * driver.
	 */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO = 37,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT command
	 * See enum qca_wlan_vendor_attr_brp_ant_limit_mode.
	 */
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE = 38,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_BRP_SET_ANT_LIMIT command
	 * to define the number of antennas to use for BRP.
	 * different purpose in each ANT_LIMIT_MODE:
	 * DISABLE - ignored
	 * EFFECTIVE - upper limit to number of antennas to be used
	 * FORCE - exact number of antennas to be used
	 * unsigned 8 bit value
	 */
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_NUM_LIMIT = 39,
	/*
	 * Used in QCA_NL80211_VENDOR_SUBCMD_GET_CHAIN_RSSI command
	 * to report the corresponding antenna index to the chain RSSI value
	 */
	QCA_WLAN_VENDOR_ATTR_ANTENNA_INFO = 40,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_AFTER_LAST,
		QCA_WLAN_VENDOR_ATTR_MAX = QCA_WLAN_VENDOR_ATTR_AFTER_LAST - 1
};

#ifdef FEATURE_WLAN_EXTSCAN
enum qca_wlan_vendor_attr_extscan_config_params {
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_INVALID = 0,

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_REQUEST_ID = 1,

	/*
	 * Attributes for data used by
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_VALID_CHANNELS sub command.
	 */

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_WIFI_BAND
	= 2,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_VALID_CHANNELS_CONFIG_PARAM_MAX_CHANNELS
	= 3,

	/*
	 * Attributes for input params used by
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_START sub command.
	 */

	/* Unsigned 32-bit value; channel frequency */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CHANNEL = 4,
	/* Unsigned 32-bit value; dwell time in ms. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_DWELL_TIME = 5,
	/* Unsigned 8-bit value; 0: active; 1: passive; N/A for DFS */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_PASSIVE = 6,
	/* Unsigned 8-bit value; channel class */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_CLASS = 7,

	/* Unsigned 8-bit value; bucket index, 0 based */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_INDEX = 8,
	/* Unsigned 8-bit value; band. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BAND = 9,
	/* Unsigned 32-bit value; desired period, in ms. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_PERIOD = 10,
	/* Unsigned 8-bit value; report events semantics. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_REPORT_EVENTS = 11,
	/*
	 * Unsigned 32-bit value. Followed by a nested array of
	 * EXTSCAN_CHANNEL_SPEC_* attributes.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS = 12,

	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC_* attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CHANNEL_SPEC = 13,

	/* Unsigned 32-bit value; base timer period in ms. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_BASE_PERIOD = 14,
	/*
	 * Unsigned 32-bit value; number of APs to store in each scan in the
	 * BSSID/RSSI history buffer (keep the highest RSSI APs).
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_MAX_AP_PER_SCAN = 15,
	/*
	 * Unsigned 8-bit value; in %, when scan buffer is this much full, wake
	 * up AP.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_PERCENT
	= 16,

	/*
	 * Unsigned 8-bit value; number of scan bucket specs; followed by a
	 * nested array of_EXTSCAN_BUCKET_SPEC_* attributes and values. The size
	 * of the array is determined by NUM_BUCKETS.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_NUM_BUCKETS = 17,

	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_* attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_NUM_BUCKETS
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC = 18,

	/* Unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_FLUSH
	= 19,
	/* Unsigned 32-bit value; maximum number of results to be returned. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_GET_CACHED_SCAN_RESULTS_CONFIG_PARAM_MAX
	= 20,

	/* An array of 6 x unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_BSSID = 21,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_LOW = 22,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_RSSI_HIGH = 23,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_CHANNEL = 24,

	/*
	 * Number of hotlist APs as unsigned 32-bit value, followed by a nested
	 * array of AP_THRESHOLD_PARAM attributes and values. The size of the
	 * array is determined by NUM_AP.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_NUM_AP = 25,

	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM_* attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_NUM_CHANNEL_SPECS
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_AP_THRESHOLD_PARAM = 26,

	/* Unsigned 32-bit value; number of samples for averaging RSSI. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_RSSI_SAMPLE_SIZE
	= 27,
	/* Unsigned 32-bit value; number of samples to confirm AP loss. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_LOST_AP_SAMPLE_SIZE
	= 28,
	/* Unsigned 32-bit value; number of APs breaching threshold. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_MIN_BREACHING = 29,
	/*
	 * Unsigned 32-bit value; number of APs. Followed by an array of
	 * AP_THRESHOLD_PARAM attributes. Size of the array is NUM_AP.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SIGNIFICANT_CHANGE_PARAMS_NUM_AP = 30,
	/* Unsigned 32-bit value; number of samples to confirm AP loss. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BSSID_HOTLIST_PARAMS_LOST_AP_SAMPLE_SIZE
	= 31,
	/*
	 * Unsigned 32-bit value. If max_period is non zero or different than
	 * period, then this bucket is an exponential backoff bucket.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_MAX_PERIOD = 32,
	/* Unsigned 32-bit value. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_BASE = 33,
	/*
	 * Unsigned 32-bit value. For exponential back off bucket, number of
	 * scans to perform for a given period.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_BUCKET_SPEC_STEP_COUNT = 34,
	/*
	 * Unsigned 8-bit value; in number of scans, wake up AP after these
	 * many scans.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SCAN_CMD_PARAMS_REPORT_THRESHOLD_NUM_SCANS
	= 35,

	/*
	 * Attributes for data used by
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SET_SSID_HOTLIST sub command.
	 */

	/* Unsigned 3-2bit value; number of samples to confirm SSID loss. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_LOST_SSID_SAMPLE_SIZE
	= 36,
	/*
	 * Number of hotlist SSIDs as unsigned 32-bit value, followed by a
	 * nested array of SSID_THRESHOLD_PARAM_* attributes and values. The
	 * size of the array is determined by NUM_SSID.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_NUM_SSID = 37,
	/*
	 * Array of QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_*
	 * attributes.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_HOTLIST_PARAMS_NUM_SSID
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM = 38,

	/* An array of 33 x unsigned 8-bit value; NULL terminated SSID */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_SSID = 39,
	/* Unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_BAND = 40,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_LOW = 41,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SSID_THRESHOLD_PARAM_RSSI_HIGH = 42,
	/* Unsigned 32-bit value; a bitmask with additional extscan config flag.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CONFIGURATION_FLAGS = 43,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_MAX =
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_SUBCMD_CONFIG_PARAM_AFTER_LAST - 1,
};

enum qca_wlan_vendor_attr_extscan_results {
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_INVALID = 0,

	/*
	 * Unsigned 32-bit value; must match the request Id supplied by
	 * Wi-Fi HAL in the corresponding subcmd NL msg.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_REQUEST_ID = 1,

	/*
	 * Unsigned 32-bit value; used to indicate the status response from
	 * firmware/driver for the vendor sub-command.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_STATUS = 2,

	/*
	 * EXTSCAN Valid Channels attributes */
	/* Unsigned 32bit value; followed by a nested array of CHANNELS. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_CHANNELS = 3,
	/*
	 * An array of NUM_CHANNELS x unsigned 32-bit value integers
	 * representing channel numbers.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CHANNELS = 4,

	/* EXTSCAN Capabilities attributes */

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_CACHE_SIZE = 5,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_BUCKETS = 6,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_AP_CACHE_PER_SCAN
	= 7,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_RSSI_SAMPLE_SIZE
	= 8,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SCAN_REPORTING_THRESHOLD
	= 9,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_HOTLIST_BSSIDS = 10,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_SIGNIFICANT_WIFI_CHANGE_APS
	= 11,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_BSSID_HISTORY_ENTRIES
	= 12,

	/*
	 * EXTSCAN Attributes used with
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_RESULTS_AVAILABLE sub-command.
	 */

	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE = 13,

	/*
	 * EXTSCAN attributes used with
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_FULL_SCAN_RESULT sub-command.
	 */

	/*
	 * An array of NUM_RESULTS_AVAILABLE x
	 * QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_*
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST = 14,

	/* Unsigned 64-bit value; age of sample at the time of retrieval */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_TIME_STAMP = 15,
	/* 33 x unsigned 8-bit value; NULL terminated SSID */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_SSID = 16,
	/* An array of 6 x unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BSSID = 17,
	/* Unsigned 32-bit value; channel frequency in MHz */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CHANNEL = 18,
	/* Signed 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RSSI = 19,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT = 20,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_RTT_SD = 21,
	/* Unsigned 16-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_BEACON_PERIOD = 22,
	/* Unsigned 16-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_CAPABILITY = 23,
	/* Unsigned 32-bit value; size of the IE DATA blob */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_LENGTH = 24,
	/*
	 * An array of IE_LENGTH x unsigned 8-bit value; blob of all the
	 * information elements found in the beacon; this data should be a
	 * packed list of wifi_information_element objects, one after the
	 * other.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_IE_DATA = 25,

	/*
	 * Unsigned 8-bit value; set by driver to indicate more scan results are
	 * available.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_RESULT_MORE_DATA = 26,

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SCAN_EVENT sub-command.
	 */
	/* Unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_EVENT_TYPE = 27,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SCAN_EVENT_STATUS = 28,

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_HOTLIST_AP_FOUND sub-command.
	 */
	/*
	 * Use attr QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE
	 * to indicate number of results.
	 * Also, use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the
	 * list of results.
	 */

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_SIGNIFICANT_CHANGE sub-command.
	 */
	/* An array of 6 x unsigned 8-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_BSSID = 29,
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_CHANNEL
	= 30,
	/* Unsigned 32-bit value. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_NUM_RSSI
	= 31,
	/*
	 * A nested array of signed 32-bit RSSI values. Size of the array is
	 * determined by (NUM_RSSI of SIGNIFICANT_CHANGE_RESULT_NUM_RSSI.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_SIGNIFICANT_CHANGE_RESULT_RSSI_LIST
	= 32,

	/*
	 * EXTSCAN attributes used with
	 * QCA_NL80211_VENDOR_SUBCMD_EXTSCAN_GET_CACHED_RESULTS sub-command.
	 */
	/*
	 * Use attr QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE
	 * to indicate number of extscan cached results returned.
	 * Also, use QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_LIST to indicate
	 *  the list of extscan cached results.
	 */

	/*
	 * An array of NUM_RESULTS_AVAILABLE x
	 * QCA_NL80211_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_*
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_LIST = 33,
	/* Unsigned 32-bit value; a unique identifier for the scan unit. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_SCAN_ID = 34,
	/*
	 * Unsigned 32-bit value; a bitmask w/additional information about scan.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_CACHED_RESULTS_FLAGS = 35,
	/*
	 * Use attr QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE
	 * to indicate number of wifi scan results/bssids retrieved by the scan.
	 * Also, use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the
	 * list of wifi scan results returned for each cached result block.
	 */

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_PNO_NETWORK_FOUND sub-command.
	 */
	/*
	 * Use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE for
	 * number of results.
	 * Use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the nested
	 * list of wifi scan results returned for each
	 * wifi_passpoint_match_result block.
	 * Array size: QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_NUM_RESULTS_AVAILABLE.
	 */

	/*
	 * EXTSCAN attributes for
	 * QCA_NL80211_VENDOR_SUBCMD_PNO_PASSPOINT_NETWORK_FOUND sub-command.
	 */
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_NETWORK_FOUND_NUM_MATCHES
	= 36,
	/*
	 * A nested array of
	 * QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_*
	 * attributes. Array size =
	 * *_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_NETWORK_FOUND_NUM_MATCHES.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_RESULT_LIST = 37,

	/* Unsigned 32-bit value; network block id for the matched network */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ID = 38,
	/*
	 * Use QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_LIST to indicate the nested
	 * list of wifi scan results returned for each
	 * wifi_passpoint_match_result block.
	 */
	/* Unsigned 32-bit value */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ANQP_LEN = 39,
	/*
	 * An array size of PASSPOINT_MATCH_ANQP_LEN of unsigned 8-bit values;
	 * ANQP data in the information_element format.
	 */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_PNO_RESULTS_PASSPOINT_MATCH_ANQP = 40,

	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_HOTLIST_SSIDS = 41,
	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_EPNO_NETS = 42,
	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_EPNO_NETS_BY_SSID
	= 43,
	/* Unsigned 32-bit value; a EXTSCAN Capabilities attribute. */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_CAPABILITIES_MAX_NUM_WHITELISTED_SSID
	= 44,

	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_BUCKETS_SCANNED = 45,
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_MAX_NUM_BLACKLISTED_BSSID = 46,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_MAX =
	QCA_WLAN_VENDOR_ATTR_EXTSCAN_RESULTS_AFTER_LAST - 1,
};
#endif

#ifdef WLAN_FEATURE_LINK_LAYER_STATS

/**
 * enum qca_wlan_vendor_attr_ll_stats_set - vendor attribute set stats
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD: Size threshold
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING:
 *	Aggresive stats gathering
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_set {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_MPDU_SIZE_THRESHOLD = 1,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_CONFIG_AGGRESSIVE_STATS_GATHERING,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_SET_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_get - vendor attribute get stats
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID: Unsigned 32bit value
 *	provided by the caller issuing the GET stats command. When reporting
 *	the stats results, the driver uses the same value to indicate which
 *	GET request the results correspond to.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK: Get config request mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK: Config response mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP: Config stop response
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_get {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_ID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_CONFIG_REQ_MASK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_GET_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_clr - vendor attribute clear stats
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK: Config request mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ: Config stop mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK: Config response mask
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP: Config stop response
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_clr {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_REQ_MASK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_REQ,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_RSP_MASK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_CONFIG_STOP_RSP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_CLR_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_results_type - ll stats result type
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_INVALID: Initial invalid value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_RADIO: Link layer stats type radio
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_IFACE: Link layer stats type interface
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_PEER: Link layer stats type peer
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_AFTER_LAST: Last value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_MAX: Max value
 */
enum qca_wlan_vendor_attr_ll_stats_results_type {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_RADIO = 1,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_IFACE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_PEER,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_results - vendor attribute stats results
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_* are nested
 * within the interface stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_* could be nested
 * within the interface stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_* are nested
 * within the interface stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_* could be nested
 * within the peer info stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_* could be
 * nested within the channel stats.
 *
 * Attributes of type QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ could be nested
 * within the radio stats.
 *
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_REQ_ID: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_BEACON_RX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_RX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_RX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_TX: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_MGMT: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_DATA: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_ACK: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MODE: Interface mode, e.g., STA,
 *	SOFTAP, IBSS, etc. Type = enum wifi_interface_mode
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MAC_ADDR: Interface MAC address.
 *	An array of 6 Unsigned int8_t
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_STATE:
 *	Type = enum wifi_connection_state,  e.g., DISCONNECTED, AUTHENTICATING,
 *	etc. Valid for STA, CLI only
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_ROAMING:
 *	Type = enum wifi_roam_state. Roaming state, e.g., IDLE or ACTIVE
 *	(is that valid for STA only?)
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_CAPABILITIES: Unsigned 32bit value.
 *	WIFI_CAPABILITY_XXX
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_SSID: NULL terminated SSID. An
 *	array of 33 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_BSSID: BSSID. An array of 6
 *	Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_AP_COUNTRY_STR: Country string
 *	advertised by AP. An array of 3 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_COUNTRY_STR: Country string for
 *	this association. An array of 3 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_AC: Type = enum wifi_traffic_ac e.g.
 *	V0, VI, BE and BK
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MCAST: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MCAST: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_AMPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_AMPDU: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_MPDU_LOST: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_SHORT: Unsigned int 32 value
 *	corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MIN: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MAX: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_AVG: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_NUM_SAMPLES: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_NUM_PEERS: Unsigned int 32
 *	value corresponding to respective AC
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_TYPE: Type = enum wifi_peer_type
 *	Peer type, e.g., STA, AP, P2P GO etc
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_MAC_ADDRESS: MAC addr corresponding
 *	to respective peer. An array of 6 Unsigned 8bit values
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_CAPABILITIES: Unsigned int 32bit
 *	value representing capabilities corresponding to respective peer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_NUM_RATES: Unsigned 32bit value.
 *	Number of rates
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_PREAMBLE: Unsigned int 8bit value:
 *	0: OFDM, 1:CCK, 2:HT 3:VHT 4..7 reserved
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_NSS: Unsigned int 8bit value:
 *	0:1x1, 1:2x2, 3:3x3, 4:4x4
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BW: Unsigned int 8bit value:
 *	0:20MHz, 1:40Mhz, 2:80Mhz, 3:160Mhz
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MCS_INDEX: Unsigned int 8bit value:
 *	OFDM/CCK rate code would be as per IEEE Std in the units of 0.5mbps
 *	HT/VHT it would be mcs index
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BIT_RATE: Unsigned 32bit value.
 *	Bit rate in units of 100Kbps
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_TX_MPDU: Unsigned int 32bit value.
 *	Number of successfully transmitted data pkts i.e., with ACK received
 *	corresponding to the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RX_MPDU: Unsigned int 32bit value.
 *	Number of received data pkts corresponding to the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MPDU_LOST: Unsigned int 32bit value.
 *	Number of data pkts losses, i.e., no ACK received corresponding to
 *	the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES: Unsigned int 32bit value.
 *	Total number of data pkt retries for the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_SHORT: Unsigned int 32bit value.
 *	Total number of short data pkt retries for the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_LONG: Unsigned int 32bit value.
 *	Total number of long data pkt retries for the respective rate
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ID: Radio id
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME: Unsigned 32bit value.
 *	Total number of msecs the radio is awake accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME: Unsigned 32bit value.
 *	Total number of msecs the radio is transmitting accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_RX_TIME: Unsigned 32bit value.
 *	Total number of msecs the radio is in active receive accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_SCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to all scan accruing
 *	over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_NBD: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to NAN accruing over time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_GSCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to GSCAN accruing over time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_ROAM_SCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to roam scan accruing over
 *	time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_PNO_SCAN: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to PNO scan accruing over
 *	time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_HS20: Unsigned 32bit value.
 *	Total number of msecs the radio is awake due to HS2.0 scans and GAS
 *	exchange accruing over time.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_CHANNELS: Unsigned 32bit value.
 *	Number of channels
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_WIDTH:
 *	Type = enum wifi_channel_width. Channel width, e.g., 20, 40, 80, etc.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ:
 *	Unsigned 32bit value. Primary 20MHz channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ0:
 *	Unsigned 32bit value. Center frequency (MHz) first segment.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ1:
 *	Unsigned 32bit value. Center frequency (MHz) second segment.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ON_TIME: Unsigned int 32bit value
 *	representing total number of msecs the radio is awake on that channel
 *	accruing over time, corresponding to the respective channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_CCA_BUSY_TIME:  Unsigned int 32bit
 *	value representing total number of msecs the CCA register is busy
 *	accruing  over time corresponding to the respective channel.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_NUM_RADIOS: Number of radios
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CH_INFO: Channel info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO: Peer info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_RATE_INFO: Peer rate info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_INFO: WMM info
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_MORE_DATA: Unsigned 8bit value.
 *	Used by the driver; if set to 1, it indicates that more stats, e.g.,
 *	peers or radio, are to follow in the next
 *	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_*_RESULTS event. Otherwise, it
 *	is set to 0.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_AVERAGE_TSF_OFFSET: tsf offset
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_DETECTED: leaky ap detected
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_AVG_NUM_FRAMES_LEAKED:
 *	average number of frames leaked
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_GUARD_TIME: guard time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE: Link Layer stats type
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_TX_LEVELS: LL Radio Number of
 *      Tx Levels
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME_PER_LEVEL:Number of msecs the
 *      radio spent in transmitting for each power level
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_SUCC_CNT: RTS successful count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_FAIL_CNT: RTS fail count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_SUCC_CNT: PPDU successful count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_FAIL_CNT: PPDU fail count
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_MAX: Max value
 */

enum qca_wlan_vendor_attr_ll_stats_results {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_REQ_ID = 1,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_BEACON_RX = 2,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_RX = 3,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_RX = 4,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_MGMT_ACTION_TX = 5,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_MGMT = 6,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_DATA = 7,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RSSI_ACK = 8,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MODE = 9,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_MAC_ADDR = 10,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_STATE = 11,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_ROAMING = 12,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_CAPABILITIES = 13,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_SSID = 14,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_BSSID = 15,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_AP_COUNTRY_STR = 16,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_INFO_COUNTRY_STR = 17,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_AC = 18,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MPDU = 19,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MPDU = 20,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_MCAST = 21,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_MCAST = 22,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RX_AMPDU = 23,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_TX_AMPDU = 24,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_MPDU_LOST = 25,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES = 26,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_SHORT = 27,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_RETRIES_LONG = 28,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MIN = 29,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_MAX = 30,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_TIME_AVG = 31,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_AC_CONTENTION_NUM_SAMPLES = 32,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_NUM_PEERS = 33,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_TYPE = 34,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_MAC_ADDRESS = 35,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_CAPABILITIES = 36,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_NUM_RATES = 37,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_PREAMBLE = 38,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_NSS = 39,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BW = 40,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MCS_INDEX = 41,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_BIT_RATE = 42,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_TX_MPDU = 43,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RX_MPDU = 44,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_MPDU_LOST = 45,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES = 46,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_SHORT = 47,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RATE_RETRIES_LONG = 48,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ID = 49,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME = 50,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME = 51,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_RX_TIME = 52,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_SCAN = 53,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_NBD = 54,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_GSCAN = 55,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_ROAM_SCAN = 56,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_PNO_SCAN = 57,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_ON_TIME_HS20 = 58,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_CHANNELS = 59,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_WIDTH = 60,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ = 61,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ0 = 62,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_INFO_CENTER_FREQ1 = 63,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_ON_TIME = 64,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CHANNEL_CCA_BUSY_TIME = 65,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_NUM_RADIOS = 66,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CH_INFO = 67,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO = 68,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_PEER_INFO_RATE_INFO = 69,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_WMM_INFO = 70,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RESULTS_MORE_DATA = 71,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_AVERAGE_TSF_OFFSET = 72,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_DETECTED = 73,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_AVG_NUM_FRAMES_LEAKED = 74,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_LEAKY_AP_GUARD_TIME = 75,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_TYPE = 76,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_NUM_TX_LEVELS = 77,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_RADIO_TX_TIME_PER_LEVEL = 78,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_SUCC_CNT = 79,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_RTS_FAIL_CNT = 80,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_SUCC_CNT = 81,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_IFACE_PPDU_FAIL_CNT = 82,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_AFTER_LAST - 1
};

enum qca_wlan_vendor_attr_ll_stats_type {
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_INVALID = 0,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_RADIO = 1,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_IFACE = 2,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_PEERS = 3,

	/* keep last */
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_AFTER_LAST,
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_MAX =
	QCA_NL80211_VENDOR_SUBCMD_LL_STATS_TYPE_AFTER_LAST - 1,
};

#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * enum qca_wlan_vendor_attr_get_supported_features - get supported feature
 *
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET: Unsigned 32bit value
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_FEATURE_SET_MAX: Max value
 */
enum qca_wlan_vendor_attr_get_supported_features {
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET = 1,
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FEATURE_SET_MAX =
		QCA_WLAN_VENDOR_ATTR_FEATURE_SET_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_set_scanning_mac_oui - set scanning mac oui
 *
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI: An array of 3 x Unsigned 8-bit
 *	value
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX: Max value
 */
enum qca_wlan_vendor_attr_set_scanning_mac_oui {
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI = 1,
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_MAX =
		QCA_WLAN_VENDOR_ATTR_SET_SCANNING_MAC_OUI_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_scan - Specifies vendor scan attributes
 *
 * @QCA_WLAN_VENDOR_ATTR_SCAN_IE: IEs that should be included as part of scan
 * @QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES: Nested unsigned 32-bit attributes
 *	with frequencies to be scanned (in MHz)
 * @QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS: Nested attribute with SSIDs to be scanned
 * @QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES: Nested array attribute of supported
 *	rates to be included
 * @QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE: flag used to send probe requests
 *	at non CCK rate in 2GHz band
 * @QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS: Unsigned 32-bit scan flags
 * @QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE: Unsigned 64-bit cookie provided by the
 *	driver for the specific scan request
 * @QCA_WLAN_VENDOR_ATTR_SCAN_STATUS: Unsigned 8-bit status of the scan
 *	request decoded as in enum scan_status
 * @QCA_WLAN_VENDOR_ATTR_SCAN_MAC: 6-byte MAC address to use when randomisation
 *	scan flag is set
 * @QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK: 6-byte MAC address mask to be used with
 *	randomisation
 * @QCA_WLAN_VENDOR_ATTR_SCAN_BSSID: BSSID provided to do scan for specific BSS
 */
enum qca_wlan_vendor_attr_scan {
	QCA_WLAN_VENDOR_ATTR_SCAN_INVALID_PARAM = 0,
	QCA_WLAN_VENDOR_ATTR_SCAN_IE,
	QCA_WLAN_VENDOR_ATTR_SCAN_FREQUENCIES,
	QCA_WLAN_VENDOR_ATTR_SCAN_SSIDS,
	QCA_WLAN_VENDOR_ATTR_SCAN_SUPP_RATES,
	QCA_WLAN_VENDOR_ATTR_SCAN_TX_NO_CCK_RATE,
	QCA_WLAN_VENDOR_ATTR_SCAN_FLAGS,
	QCA_WLAN_VENDOR_ATTR_SCAN_COOKIE,
	QCA_WLAN_VENDOR_ATTR_SCAN_STATUS,
	QCA_WLAN_VENDOR_ATTR_SCAN_MAC,
	QCA_WLAN_VENDOR_ATTR_SCAN_MAC_MASK,
	QCA_WLAN_VENDOR_ATTR_SCAN_BSSID,
	QCA_WLAN_VENDOR_ATTR_SCAN_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SCAN_MAX =
		QCA_WLAN_VENDOR_ATTR_SCAN_AFTER_LAST - 1
};

/**
 * enum scan_status - Specifies the valid values the vendor scan attribute
 *	QCA_WLAN_VENDOR_ATTR_SCAN_STATUS can take
 * @VENDOR_SCAN_STATUS_NEW_RESULTS: implies the vendor scan is successful with
 *	new scan results
 * @VENDOR_SCAN_STATUS_ABORTED: implies the vendor scan was aborted in-between
 */
enum scan_status {
	VENDOR_SCAN_STATUS_NEW_RESULTS,
	VENDOR_SCAN_STATUS_ABORTED,
	VENDOR_SCAN_STATUS_MAX,
};

/**
 * enum qca_wlan_vendor_attr_get_concurrency_matrix - get concurrency matrix
 *
 * NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_CONCURRENCY_MATRIX sub command.
 *
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_CONFIG_PARAM_SET_SIZE_MAX:
 *	Unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET_SIZE:
 *	Unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET: Set results. An
 *	array of SET_SIZE x Unsigned 32bit values representing concurrency
 *	combinations
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_MAX: Max value
 */
enum qca_wlan_vendor_attr_get_concurrency_matrix {
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_CONFIG_PARAM_SET_SIZE_MAX
									= 1,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET_SIZE = 2,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_RESULTS_SET = 3,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_CONCURRENCY_MATRIX_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_set_no_dfs_flag - vendor attribute set no dfs flag
 *
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG: Unsigned 32-bit value
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX: Max value
 */
enum qca_wlan_vendor_attr_set_no_dfs_flag {
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG = 1,
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_MAX =
		QCA_WLAN_VENDOR_ATTR_SET_NO_DFS_FLAG_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_attr_wisa_cmd
 * @QCA_WLAN_VENDOR_ATTR_WISA_INVALID: Invalid attr
 * @QCA_WLAN_VENDOR_ATTR_WISA_MODE: WISA mode value attr (u32)
 * @QCA_WLAN_VENDOR_ATTR_WISA_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_WISA_MAX: Max Value
 * WISA setup vendor commands
 */
enum qca_vendor_attr_wisa_cmd {
	QCA_WLAN_VENDOR_ATTR_WISA_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WISA_MODE,
	QCA_WLAN_VENDOR_ATTR_WISA_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WISA_MAX =
		QCA_WLAN_VENDOR_ATTR_WISA_AFTER_LAST - 1
};

enum qca_roaming_policy {
	QCA_ROAMING_NOT_ALLOWED,
	QCA_ROAMING_ALLOWED_WITHIN_ESS,
};

/**
 * enum qca_wlan_vendor_attr_roam_auth - vendor event for roaming
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID: BSSID of the roamed AP
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REQ_IE: Request IE
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RESP_IE: Response IE
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED: Authorization Status
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_KEY_REPLAY_CTR: Replay Counter
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KCK: KCK of the PTK
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KEK: KEK of the PTK
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_SUBNET_STATUS: subnet change status
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_STATUS:
 * Indicates the status of re-association requested by user space for
 * the BSSID specified by QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID.
 * Type u16.
 * Represents the status code from AP. Use
 * %WLAN_STATUS_UNSPECIFIED_FAILURE if the device cannot give you the
 * real status code for failures.
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RETAIN_CONNECTION:
 * This attribute indicates that the old association was maintained when
 * a re-association is requested by user space and that re-association
 * attempt fails (i.e., cannot connect to the requested BSS, but can
 * remain associated with the BSS with which the association was in
 * place when being requested to roam). Used along with
 * WLAN_VENDOR_ATTR_ROAM_AUTH_STATUS to indicate the current
 * re-association status. Type flag.
 * This attribute is applicable only for re-association failure cases.
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMK: AUTH PMK
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMKID: AUTH PMKID
 * @QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_FILS_ERP_NEXT_SEQ_NUM: FILS erp next
 * seq number
 */
enum qca_wlan_vendor_attr_roam_auth {
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REQ_IE,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RESP_IE,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_KEY_REPLAY_CTR,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KCK,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KEK,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_SUBNET_STATUS,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_STATUS,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RETAIN_CONNECTION,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMK,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMKID,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_FILS_ERP_NEXT_SEQ_NUM,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_MAX =
		QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_wifi_config - wifi config
 *
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_DYNAMIC_DTIM: dynamic DTIM
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_STATS_AVG_FACTOR: avg factor
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_GUARD_TIME: guard time
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_MAX: max value
 */
enum qca_wlan_vendor_attr_wifi_config {
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_DYNAMIC_DTIM = 1,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_STATS_AVG_FACTOR = 2,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_GUARD_TIME = 3,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_WIFI_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_epno_type - the type of request to the EPNO command
 * @QCA_WLAN_EPNO: epno type request
 * @QCA_WLAN_PNO: pno type request
 */
enum qca_wlan_epno_type {
	QCA_WLAN_EPNO,
	QCA_WLAN_PNO
};

/**
 * enum qca_wlan_vendor_attr_pno_config_params - pno config params
 *
 * @QCA_WLAN_VENDOR_ATTR_PNO_INVALID - Invalid initial value
 *
 *	NL attributes for data used by
 *	QCA_NL80211_VENDOR_SUBCMD_PNO_SET_PASSPOINT_LIST sub command.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM:
 *	Unsigned 32-bit value; pno passpoint number of networks
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NETWORK_ARRAY:
 *	Array of nested QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_*
 *	attributes. Array size =
 *	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ID:
 *	Unsigned 32-bit value; network id
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM:
 *	An array of 256 x Unsigned 8-bit value; NULL terminated UTF8 encoded
 *	realm, 0 if unspecified.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_CNSRTM_ID:
 *	An array of 16 x Unsigned 32-bit value; roaming consortium ids
 *	to match, 0 if unspecified.
 * @QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_PLMN:
 *	An array of 6 x Unsigned 8-bit value; mcc/mnc combination, 0s if
 *	unspecified.
 *
 *	NL attributes for data used by
 *	QCA_NL80211_VENDOR_SUBCMD_PNO_SET_LIST sub command.
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS:
 *	Unsigned 32-bit value; set pno number of networks
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORKS_LIST:
 *	Array of nested
 *	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_*
 *	attributes. Array size =
 *		QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID:
 *	An array of 33 x Unsigned 8-bit value; NULL terminated SSID
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD:
 *	Signed 8-bit value; threshold for considering this SSID as found,
 *	required granularity for this threshold is 4dBm to 8dBm
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS:
 *	Unsigned 8-bit value; WIFI_PNO_FLAG_XXX
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT:
 *	Unsigned 8-bit value; auth bit field for matching WPA IE
 * @QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_TYPE
 *	Unsigned 8-bit to indicate ePNO type; values from qca_wlan_epno_type
 *@QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_CHANNEL_LIST
 *	Nested attribute to send the channel list
 *@QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_SCAN_INTERVAL
 *	Unsigned 32-bit value; indicates the Interval between PNO scan
 *	cycles in msec
 *@QCA_WLAN_VENDOR_ATTR_EPNO_MIN5GHZ_RSSI
 *	Signed 32-bit value; minimum 5GHz RSSI for a BSSID to be considered
 *@QCA_WLAN_VENDOR_ATTR_EPNO_MIN24GHZ_RSSI
 *	Signed 32-bit value; minimum 2.4GHz RSSI for a BSSID to be considered
 *	This attribute is obsolete now.
 *@QCA_WLAN_VENDOR_ATTR_EPNO_INITIAL_SCORE_MAX
 *	Signed 32-bit value; the maximum score that a network
 *	can have before bonuses
 *@QCA_WLAN_VENDOR_ATTR_EPNO_CURRENT_CONNECTION_BONUS
 *	Signed 32-bit value; only report when there is a network's
 *	score this much higher han the current connection
 *@QCA_WLAN_VENDOR_ATTR_EPNO_SAME_NETWORK_BONUS
 *	Signed 32-bit value; score bonus for all networks with
 *	the same network flag
 *@QCA_WLAN_VENDOR_ATTR_EPNO_SECURE_BONUS
 *	Signed 32-bit value; score bonus for networks that are not open
 *@QCA_WLAN_VENDOR_ATTR_EPNO_BAND5GHZ_BONUS
 *	Signed 32-bit value; 5GHz RSSI score bonus applied to all
 *	5GHz networks
 *@QCA_WLAN_VENDOR_ATTR_PNO_CONFIG_REQUEST_ID
 *	Unsigned 32-bit value, representing the PNO Request ID
 * @QCA_WLAN_VENDOR_ATTR_PNO_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_PNO_MAX: max
 */
enum qca_wlan_vendor_attr_pno_config_params {
	QCA_WLAN_VENDOR_ATTR_PNO_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NUM = 1,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_LIST_PARAM_NETWORK_ARRAY = 2,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ID = 3,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_REALM = 4,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_CNSRTM_ID = 5,
	QCA_WLAN_VENDOR_ATTR_PNO_PASSPOINT_NETWORK_PARAM_ROAM_PLMN = 6,

	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_NUM_NETWORKS = 7,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORKS_LIST = 8,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_SSID = 9,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_RSSI_THRESHOLD = 10,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_FLAGS = 11,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_NETWORK_AUTH_BIT = 12,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_TYPE = 13,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_CHANNEL_LIST = 14,
	QCA_WLAN_VENDOR_ATTR_PNO_SET_LIST_PARAM_EPNO_SCAN_INTERVAL = 15,
	QCA_WLAN_VENDOR_ATTR_EPNO_MIN5GHZ_RSSI = 16,
	QCA_WLAN_VENDOR_ATTR_EPNO_MIN24GHZ_RSSI = 17,
	QCA_WLAN_VENDOR_ATTR_EPNO_INITIAL_SCORE_MAX = 18,
	QCA_WLAN_VENDOR_ATTR_EPNO_CURRENT_CONNECTION_BONUS = 19,
	QCA_WLAN_VENDOR_ATTR_EPNO_SAME_NETWORK_BONUS = 20,
	QCA_WLAN_VENDOR_ATTR_EPNO_SECURE_BONUS = 21,
	QCA_WLAN_VENDOR_ATTR_EPNO_BAND5GHZ_BONUS = 22,

	QCA_WLAN_VENDOR_ATTR_PNO_CONFIG_REQUEST_ID = 23,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PNO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PNO_MAX =
		QCA_WLAN_VENDOR_ATTR_PNO_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_roaming_config_params - roaming config params
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD: roaming sub command
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID: Request id
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS:
 *	number of whitelist networks
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_LIST:
 *	Whitelist ssid list
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID:
 *	white list ssid
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_THRESHOLD:
 *	'a' band boost threshold
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_THRESHOLD:
 *	'a' band penalty threshold
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_FACTOR:
 *	'a' band boost factor
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_FACTOR:
 *	'a' band penalty factor
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_MAX_BOOST:
 *	'a' band max boost
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_LAZY_ROAM_HISTERESYS:
 *	lazy roam histeresys
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_ALERT_ROAM_RSSI_TRIGGER:
 *	alert roam rssi trigger
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_ENABLE:
 *	set lazy roam enable
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS:
 *	set bssid preference
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_NUM_BSSID:
 *	set lazy roam number of bssid
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_BSSID:
 *	set lazy roam bssid
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_RSSI_MODIFIER:
 *	set lazy roam rssi modifier
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS:
 *	set bssid params
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID:
 *	set bssid params num bssid
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID:
 *	set bssid params bssid
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX: Max
 */
enum qca_wlan_vendor_attr_roaming_config_params {
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_ROAMING_SUBCMD = 1,
	QCA_WLAN_VENDOR_ATTR_ROAMING_REQ_ID = 2,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_NUM_NETWORKS = 3,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID_LIST = 4,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_WHITE_LIST_SSID = 5,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_THRESHOLD = 6,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_THRESHOLD = 7,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_BOOST_FACTOR = 8,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_PENALTY_FACTOR = 9,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_A_BAND_MAX_BOOST = 10,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_LAZY_ROAM_HISTERESYS = 11,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_ALERT_ROAM_RSSI_TRIGGER = 12,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_ENABLE = 13,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PREFS = 14,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_NUM_BSSID = 15,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_BSSID = 16,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_LAZY_ROAM_RSSI_MODIFIER = 17,

	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS = 18,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_NUM_BSSID = 19,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_SET_BSSID_PARAMS_BSSID = 20,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_MAX =
		QCA_WLAN_VENDOR_ATTR_ROAMING_PARAM_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_roam_subcmd - roam sub commands
 *
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SSID_WHITE_LIST: ssid white list
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_EXTSCAN_ROAM_PARAMS: roam params
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_LAZY_ROAM: set lazy roam
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BSSID_PREFS: set bssid prefs
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BSSID_PARAMS: set bssid params
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BLACKLIST_BSSID: set blacklist bssid
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_MAX: subcmd max
 */
enum qca_wlan_vendor_attr_roam_subcmd {
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SSID_WHITE_LIST = 1,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_EXTSCAN_ROAM_PARAMS = 2,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_LAZY_ROAM = 3,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BSSID_PREFS = 4,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BSSID_PARAMS = 5,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_SET_BLACKLIST_BSSID = 6,

	/* KEEP LAST */
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_MAX =
		QCA_WLAN_VENDOR_ATTR_ROAM_SUBCMD_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_get_wifi_info - wifi driver information
 *
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION: get host driver version
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION: ger firmware version
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX - get radio index
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX: subcmd max
 */
enum qca_wlan_vendor_attr_get_wifi_info {
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_DRIVER_VERSION     = 1,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_FIRMWARE_VERSION   = 2,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_RADIO_INDEX        = 3,

	/* KEEP LAST */
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_MAX  =
		QCA_WLAN_VENDOR_ATTR_WIFI_INFO_GET_AFTER_LAST - 1,
};

enum qca_wlan_vendor_attr_logger_results {
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_INVALID = 0,

	/*
	 * Unsigned 32-bit value; must match the request Id supplied by
	 * Wi-Fi HAL in the corresponding subcmd NL msg.
	 */
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_REQUEST_ID = 1,

	/*
	 * Unsigned 32-bit value; used to indicate the size of memory
	 * dump to be allocated.
	 */
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_MEMDUMP_SIZE = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_MAX =
	QCA_WLAN_VENDOR_ATTR_LOGGER_RESULTS_AFTER_LAST - 1,
};

/**
 * qca_wlan_vendor_channel_prop_flags: This represent the flags for a channel.
 * This is used by QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS.
 */
enum qca_wlan_vendor_channel_prop_flags {
	/* Bits 0, 1, 2, and 3 are reserved */

	/* Turbo channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_TURBO         = 1 << 4,
	/* CCK channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_CCK           = 1 << 5,
	/* OFDM channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_OFDM          = 1 << 6,
	/* 2.4 GHz spectrum channel. */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_2GHZ          = 1 << 7,
	/* 5 GHz spectrum channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_5GHZ          = 1 << 8,
	/* Only passive scan allowed */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_PASSIVE       = 1 << 9,
	/* Dynamic CCK-OFDM channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_DYN           = 1 << 10,
	/* GFSK channel (FHSS PHY) */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_GFSK          = 1 << 11,
	/* Radar found on channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_RADAR         = 1 << 12,
	/* 11a static turbo channel only */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_STURBO        = 1 << 13,
	/* Half rate channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HALF          = 1 << 14,
	/* Quarter rate channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_QUARTER       = 1 << 15,
	/* HT 20 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT20          = 1 << 16,
	/* HT 40 with extension channel above */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40PLUS      = 1 << 17,
	/* HT 40 with extension channel below */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40MINUS     = 1 << 18,
	/* HT 40 intolerant */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40INTOL     = 1 << 19,
	/* VHT 20 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT20         = 1 << 20,
	/* VHT 40 with extension channel above */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40PLUS     = 1 << 21,
	/* VHT 40 with extension channel below */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40MINUS    = 1 << 22,
	/* VHT 80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80         = 1 << 23,
	/* HT 40 intolerant mark bit for ACS use */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40INTOLMARK = 1 << 24,
	/* Channel temporarily blocked due to noise */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_BLOCKED       = 1 << 25,
	/* VHT 160 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT160        = 1 << 26,
	/* VHT 80+80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80_80      = 1 << 27,
	/* HE 20 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE20          = 1 << 28,
	/* HE 40 with extension channel above */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40PLUS      = 1 << 29,
	/* HE 40 with extension channel below */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40MINUS     = 1 << 30,
	/* HE 40 intolerant */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40INTOL     = 1 << 31,
};

/**
 * qca_wlan_vendor_channel_prop_flags_2: This represents the flags for a
 * channel, and is a continuation of qca_wlan_vendor_channel_prop_flags. This is
 * used by QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS_2.
 */
enum qca_wlan_vendor_channel_prop_flags_2 {
	/* HE 40 intolerant mark bit for ACS use */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40INTOLMARK = 1 << 0,
	/* HE 80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80          = 1 << 1,
	/* HE 160 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE160         = 1 << 2,
	/* HE 80+80 channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80_80       = 1 << 3,
};

/**
 * qca_wlan_vendor_channel_prop_flags_ext: This represent the extended flags for
 * each channel. This is used by
 * QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT.
 */
enum qca_wlan_vendor_channel_prop_flags_ext {
	/* Radar found on channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_RADAR_FOUND     = 1 << 0,
	/* DFS required on channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS             = 1 << 1,
	/* DFS required on channel for 2nd band of 80+80 */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS_CFREQ2      = 1 << 2,
	/* If channel has been checked for DFS */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DFS_CLEAR       = 1 << 3,
	/* Excluded in 802.11d */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_11D_EXCLUDED    = 1 << 4,
	/* Channel Switch Announcement received on this channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_CSA_RECEIVED    = 1 << 5,
	/* Ad-hoc is not allowed */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DISALLOW_ADHOC  = 1 << 6,
	/* Station only channel */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_DISALLOW_HOSTAP = 1 << 7,
	/* DFS radar history for slave device (STA mode) */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_HISTORY_RADAR   = 1 << 8,
	/* DFS CAC valid for slave device (STA mode) */
	QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_EXT_CAC_VALID       = 1 << 9,
};

/**
 * qca_wlan_vendor_attr_nud_stats_set: Attributes to vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_SET. This carries the requisite
 * information to start/stop the NUD statistics collection.
 */
enum qca_attr_nud_stats_set {
	QCA_ATTR_NUD_STATS_SET_INVALID = 0,

	/*
	 * Flag to start/stop the NUD statistics collection.
	 * Start - If included, Stop - If not included
	 */
	QCA_ATTR_NUD_STATS_SET_START = 1,
	/* IPv4 address of the default gateway (in network byte order) */
	QCA_ATTR_NUD_STATS_GW_IPV4 = 2,

	/* keep last */
	QCA_ATTR_NUD_STATS_SET_LAST,
	QCA_ATTR_NUD_STATS_SET_MAX =
		QCA_ATTR_NUD_STATS_SET_LAST - 1,
};

/**
 * qca_attr_nud_stats_get: Attributes to vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_NUD_STATS_GET. This carries the requisite
 * NUD statistics collected when queried.
 */
enum qca_attr_nud_stats_get {
	QCA_ATTR_NUD_STATS_GET_INVALID = 0,
	/* ARP Request count from netdev */
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_FROM_NETDEV = 1,
	/* ARP Request count sent to lower MAC from upper MAC */
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TO_LOWER_MAC = 2,
	/* ARP Request count received by lower MAC from upper MAC */
	QCA_ATTR_NUD_STATS_ARP_REQ_RX_COUNT_BY_LOWER_MAC = 3,
	/* ARP Request count successfully transmitted by the device */
	QCA_ATTR_NUD_STATS_ARP_REQ_COUNT_TX_SUCCESS = 4,
	/* ARP Response count received by lower MAC */
	QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_LOWER_MAC = 5,
	/* ARP Response count received by upper MAC */
	QCA_ATTR_NUD_STATS_ARP_RSP_RX_COUNT_BY_UPPER_MAC = 6,
	/* ARP Response count delivered to netdev */
	QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_TO_NETDEV = 7,
	/* ARP Response count delivered to netdev */
	QCA_ATTR_NUD_STATS_ARP_RSP_COUNT_OUT_OF_ORDER_DROP = 8,
	/*
	 * Flag indicating if the station's link to the AP is active.
	 * Active Link - If included, Inactive link - If not included
	 */
	QCA_ATTR_NUD_STATS_AP_LINK_ACTIVE = 9,
	/*
	 * Flag indicating if there is any duplicate address detected (DAD).
	 * Yes - If detected, No - If not detected.
	 */
	QCA_ATTR_NUD_STATS_IS_DAD = 10,

	/* keep last */
	QCA_ATTR_NUD_STATS_GET_LAST,
	QCA_ATTR_NUD_STATS_GET_MAX =
		QCA_ATTR_NUD_STATS_GET_LAST - 1,
};

enum qca_wlan_btm_candidate_status {
	QCA_STATUS_ACCEPT = 0,
	QCA_STATUS_REJECT_EXCESSIVE_FRAME_LOSS_EXPECTED = 1,
	QCA_STATUS_REJECT_EXCESSIVE_DELAY_EXPECTED = 2,
	QCA_STATUS_REJECT_INSUFFICIENT_QOS_CAPACITY = 3,
	QCA_STATUS_REJECT_LOW_RSSI = 4,
	QCA_STATUS_REJECT_HIGH_INTERFERENCE = 5,
	QCA_STATUS_REJECT_UNKNOWN = 6,
};

enum qca_wlan_vendor_attr_btm_candidate_info {
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_INVALID = 0,

	/* 6-byte MAC address representing the BSSID of transition candidate */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID = 1,
	/*
	 * Unsigned 32-bit value from enum qca_wlan_btm_candidate_status
	 * returned by the driver. It says whether the BSSID provided in
	 * QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_BSSID is acceptable by
	 * the driver, if not it specifies the reason for rejection.
	 * Note that the user-space can overwrite the transition reject reason
	 * codes provided by driver based on more information.
	 */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_STATUS = 2,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_MAX =
	QCA_WLAN_VENDOR_ATTR_BTM_CANDIDATE_INFO_AFTER_LAST - 1,
};

enum qca_attr_trace_level {
	QCA_ATTR_TRACE_LEVEL_INVALID = 0,
	/*
	 * Nested array of the following attributes:
	 * QCA_ATTR_TRACE_LEVEL_MODULE,
	 * QCA_ATTR_TRACE_LEVEL_MASK.
	 */
	QCA_ATTR_TRACE_LEVEL_PARAM = 1,
	/*
	 * Specific QCA host driver module. Please refer to the QCA host
	 * driver implementation to get the specific module ID.
	 */
	QCA_ATTR_TRACE_LEVEL_MODULE = 2,
	/* Different trace level masks represented in the QCA host driver. */
	QCA_ATTR_TRACE_LEVEL_MASK = 3,

	/* keep last */
	QCA_ATTR_TRACE_LEVEL_AFTER_LAST,
	QCA_ATTR_TRACE_LEVEL_MAX =
		QCA_ATTR_TRACE_LEVEL_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_get_logger_features - value for logger
 *                                                 supported features
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_INVALID - Invalid
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_SUPPORTED - Indicate the supported features
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_AFTER_LAST - To keep track of the last enum
 * @QCA_WLAN_VENDOR_ATTR_LOGGER_MAX - max value possible for this type
 *
 * enum values are used for NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_GET_LOGGER_FEATURE_SET sub command.
 */
enum qca_wlan_vendor_attr_get_logger_features {
	QCA_WLAN_VENDOR_ATTR_LOGGER_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LOGGER_SUPPORTED = 1,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LOGGER_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LOGGER_MAX =
		QCA_WLAN_VENDOR_ATTR_LOGGER_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_link_properties - link properties
 *
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_INVALID: Invalid initial value
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_NSS: Unsigned 8-bit value to
 *	specify the number of spatial streams negotiated
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_RATE_FLAGS: Unsigned 8-bit value
 *	to specify negotiated rate flags i.e. ht, vht and channel width
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_FREQ: Unsigned 32bit value to
 *	specify the operating frequency
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAX: max value
 */
enum qca_wlan_vendor_attr_link_properties {
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_NSS = 1,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_RATE_FLAGS = 2,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_FREQ = 3,

	/* KEEP LAST */
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_MAX =
		QCA_WLAN_VENDOR_ATTR_LINK_PROPERTIES_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_nd_offload - vendor NS offload support
 *
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_INVALID - Invalid
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG - Flag to set NS offload
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_AFTER_LAST - To keep track of the last enum
 * @QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX - max value possible for this type
 *
 * enum values are used for NL attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_ND_OFFLOAD sub command.
 */
enum qca_wlan_vendor_attr_nd_offload {
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_FLAG,

	/* Keep last */
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_MAX =
		QCA_WLAN_VENDOR_ATTR_ND_OFFLOAD_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_features - vendor device/driver features
 * @QCA_WLAN_VENDOR_FEATURE_KEY_MGMT_OFFLOAD: Device supports key
 * management offload, a mechanism where the station's firmware
 * does the exchange with the AP to establish the temporal keys
 * after roaming, rather than having the supplicant do it.
 * @QCA_WLAN_VENDOR_FEATURE_OFFCHANNEL_SIMULTANEOUS: Device supports
 *        simultaneous off-channel operations.
 * @QQCA_WLAN_VENDOR_FEATURE_P2P_LISTEN_OFFLOAD: Device supports P2P
 *	Listen offload; a mechanism where the station's firmware
 *	takes care of responding to incoming Probe Request frames received
 *	from other P2P devices whilst in Listen state, rather than having the
 *	user space wpa_supplicant do it. Information from received P2P
 *	Requests are forwarded from firmware to host whenever the APPS
 *	processor exits power collapse state.
 * @QCA_WLAN_VENDOR_FEATURE_OCE_STA: Device supports all OCE non-AP STA
 *	specific features
 * @QCA_WLAN_VENDOR_FEATURE_OCE_AP: Device supports all OCE AP specific
 *	features.
 * @QCA_WLAN_VENDOR_FEATURE_OCE_STA_CFON: Device supports OCE STA-CFON
 *	specific features only. If a Device sets this bit but not the
 *	QCA_WLAN_VENDOR_FEATURE_OCE_AP, the userspace shall assume that
 *	this Device may not support all OCE AP functionalities but can support
 *	only OCE STA-CFON functionalities.
 */
enum qca_wlan_vendor_features {
	QCA_WLAN_VENDOR_FEATURE_KEY_MGMT_OFFLOAD = 0,
	QCA_WLAN_VENDOR_FEATURE_SUPPORT_HW_MODE_ANY = 1,
	QCA_WLAN_VENDOR_FEATURE_OFFCHANNEL_SIMULTANEOUS = 2,
	QCA_WLAN_VENDOR_FEATURE_P2P_LISTEN_OFFLOAD	= 3,
	QCA_WLAN_VENDOR_FEATURE_OCE_STA = 4,
	QCA_WLAN_VENDOR_FEATURE_OCE_AP = 5,
	QCA_WLAN_VENDOR_FEATURE_OCE_STA_CFON = 6,
	/* Additional features need to be added above this */
	NUM_QCA_WLAN_VENDOR_FEATURES
};

/**
 * enum qca_wlan_vendor_attr_sap_conditional_chan_switch - Parameters for SAP
 *     conditional channel switch
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_INVALID: Invalid initial
 *     value
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST: Priority based
 * frequency list (an array of u32 values in host byte order)
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS: Status of the
 *     conditional switch (u32)- 0: Success, Non-zero: Failure
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_AFTER_LAST: After last
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_MAX: Subcommand max
 */
enum qca_wlan_vendor_attr_sap_conditional_chan_switch {
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_FREQ_LIST = 1,
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_STATUS = 2,

	/* Keep Last */
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_MAX =
	QCA_WLAN_VENDOR_ATTR_SAP_CONDITIONAL_CHAN_SWITCH_AFTER_LAST - 1,
};

/**
 * enum wifi_logger_supported_features - values for supported logger features
 * @WIFI_LOGGER_MEMORY_DUMP_SUPPORTED: Memory dump of FW
 * @WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED: Per packet statistics
 * @WIFI_LOGGER_CONNECT_EVENT_SUPPORTED: Logging of Connectivity events
 * @WIFI_LOGGER_POWER_EVENT_SUPPORTED: Power of driver
 * @WIFI_LOGGER_WAKE_LOCK_SUPPORTE: Wakelock of driver
 * @WIFI_LOGGER_WATCHDOG_TIMER_SUPPORTED: monitor FW health
 * @WIFI_LOGGER_DRIVER_DUMP_SUPPORTED: support driver dump
 * @WIFI_LOGGER_PACKET_FATE_SUPPORTED: tracks connection packets fate
 */
enum wifi_logger_supported_features {
	WIFI_LOGGER_MEMORY_DUMP_SUPPORTED = (1 << (0)),
	WIFI_LOGGER_PER_PACKET_TX_RX_STATUS_SUPPORTED = (1 << (1)),
	WIFI_LOGGER_CONNECT_EVENT_SUPPORTED = (1 << (2)),
	WIFI_LOGGER_POWER_EVENT_SUPPORTED = (1 << (3)),
	WIFI_LOGGER_WAKE_LOCK_SUPPORTED = (1 << (4)),
	WIFI_LOGGER_VERBOSE_SUPPORTED = (1 << (5)),
	WIFI_LOGGER_WATCHDOG_TIMER_SUPPORTED = (1 << (6)),
	WIFI_LOGGER_DRIVER_DUMP_SUPPORTED = (1 << (7)),
	WIFI_LOGGER_PACKET_FATE_SUPPORTED = (1 << (8))
};
/**
 * enum qca_wlan_vendor_attr_acs_offload
 *
 * @QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL: ACS selected primary channel
 * @QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL: ACS selected secondary channel
 * @QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE: hw_mode for ACS
 * @QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED: indicate if HT capability is enabled
 * @QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED: indicate HT capability
 */
enum qca_wlan_vendor_attr_acs_offload {
	QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ACS_PRIMARY_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_ACS_SECONDARY_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_ACS_HW_MODE,
	QCA_WLAN_VENDOR_ATTR_ACS_HT_ENABLED,
	QCA_WLAN_VENDOR_ATTR_ACS_HT40_ENABLED,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_ENABLED,
	QCA_WLAN_VENDOR_ATTR_ACS_CHWIDTH,
	QCA_WLAN_VENDOR_ATTR_ACS_CH_LIST,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG0_CENTER_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_ACS_VHT_SEG1_CENTER_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_ACS_FREQ_LIST,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ACS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ACS_MAX =
		QCA_WLAN_VENDOR_ATTR_ACS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_acs_hw_mode
 *
 * @QCA_ACS_MODE_IEEE80211B: 11b mode
 * @QCA_ACS_MODE_IEEE80211G: 11g mode
 * @QCA_ACS_MODE_IEEE80211A: 11a mode
 * @QCA_ACS_MODE_IEEE80211AD: 11ad mode
 */
enum qca_wlan_vendor_acs_hw_mode {
	QCA_ACS_MODE_IEEE80211B,
	QCA_ACS_MODE_IEEE80211G,
	QCA_ACS_MODE_IEEE80211A,
	QCA_ACS_MODE_IEEE80211AD,
	QCA_ACS_MODE_IEEE80211ANY,
};

/**
 * enum qca_access_policy - access control policy
 *
 * Access control policy is applied on the configured IE
 * (QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE).
 * To be set with QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY.
 *
 * @QCA_ACCESS_POLICY_ACCEPT_UNLESS_LISTED: Deny Wi-Fi Connections which match
 *»       with the specific configuration (IE) set, i.e. allow all the
 *»       connections which do not match the configuration.
 * @QCA_ACCESS_POLICY_DENY_UNLESS_LISTED: Accept Wi-Fi Connections which match
 *»       with the specific configuration (IE) set, i.e. deny all the
 *»       connections which do not match the configuration.
 */
enum qca_access_policy {
	QCA_ACCESS_POLICY_ACCEPT_UNLESS_LISTED,
	QCA_ACCESS_POLICY_DENY_UNLESS_LISTED,
};

/**
 * enum qca_ignore_assoc_disallowed - Ignore assoc disallowed values
 *
 * The valid values for the ignore assoc disallowed
 *
 * @QCA_IGNORE_ASSOC_DISALLOWED_DISABLE: Disable ignore assoc disallowed
 * @QCA_IGNORE_ASSOC_DISALLOWED_ENABLE: Enable ignore assoc disallowed
 *
 */
enum qca_ignore_assoc_disallowed {
	QCA_IGNORE_ASSOC_DISALLOWED_DISABLE,
	QCA_IGNORE_ASSOC_DISALLOWED_ENABLE
};

/* Attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION and
 * QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION subcommands.
 */
enum qca_wlan_vendor_attr_config {
	QCA_WLAN_VENDOR_ATTR_CONFIG_INVALID = 0,
	/*
	 * Unsigned 32-bit value to set the DTIM period.
	 * Whether the wifi chipset wakes at every dtim beacon or a multiple of
	 * the DTIM period. If DTIM is set to 3, the STA shall wake up every 3
	 * DTIM beacons.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_MODULATED_DTIM = 1,
	/*
	 * Unsigned 32-bit value to set the wifi_iface stats averaging factor
	 * used to calculate statistics like average the TSF offset or average
	 * number of frame leaked.
	 * For instance, upon Beacon frame reception:
	 * current_avg = ((beacon_TSF - TBTT) * factor + previous_avg * (0x10000 - factor) ) / 0x10000
	 * For instance, when evaluating leaky APs:
	 * current_avg = ((num frame received within guard time) * factor + previous_avg * (0x10000 - factor)) / 0x10000
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_STATS_AVG_FACTOR = 2,
	/*
	 * Unsigned 32-bit value to configure guard time, i.e., when
	 * implementing IEEE power management based on frame control PM bit, how
	 * long the driver waits before shutting down the radio and after
	 * receiving an ACK frame for a Data frame with PM bit set.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GUARD_TIME = 3,
	/* Unsigned 32-bit value to change the FTM capability dynamically */
	QCA_WLAN_VENDOR_ATTR_CONFIG_FINE_TIME_MEASUREMENT = 4,
	/* Unsigned 16-bit value to configure maximum TX rate dynamically */
	QCA_WLAN_VENDOR_ATTR_CONF_TX_RATE = 5,
	/*
	 * Unsigned 32-bit value to configure the number of continuous
	 * Beacon Miss which shall be used by the firmware to penalize
	 * the RSSI.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS = 6,
	/*
	 * Unsigned 8-bit value to configure the channel avoidance indication
	 * behavior. Firmware to send only one indication and ignore duplicate
	 * indications when set to avoid multiple Apps wakeups.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CHANNEL_AVOIDANCE_IND = 7,
	/*
	 * 8-bit unsigned value to configure the maximum TX MPDU for
	 * aggregation.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_MPDU_AGGREGATION = 8,
	/*
	 * 8-bit unsigned value to configure the maximum RX MPDU for
	 * aggregation.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_MPDU_AGGREGATION = 9,
	/*
	 * 8-bit unsigned value to configure the Non aggregrate/11g sw
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_NON_AGG_RETRY = 10,
	/*
	 * 8-bit unsigned value to configure the aggregrate sw
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_AGG_RETRY = 11,
	/*
	 * 8-bit unsigned value to configure the MGMT frame
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_MGMT_RETRY = 12,
	/*
	 * 8-bit unsigned value to configure the CTRL frame
	 * retry threshold (0 disable, 31 max).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_CTRL_RETRY = 13,
	/*
	 * 8-bit unsigned value to configure the propagation delay for
	 * 2G/5G band (0~63, units in us)
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_DELAY = 14,
	/*
	 * Unsigned 32-bit value to configure the number of unicast TX fail
	 * packet count. The peer is disconnected once this threshold is
	 * reached.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TX_FAIL_COUNT = 15,
	/*
	 * Attribute used to set scan default IEs to the driver.
	 *
	 * These IEs can be used by scan operations that will be initiated by
	 * the driver/firmware.
	 *
	 * For further scan requests coming to the driver, these IEs should be
	 * merged with the IEs received along with scan request coming to the
	 * driver. If a particular IE is present in the scan default IEs but not
	 * present in the scan request, then that IE should be added to the IEs
	 * sent in the Probe Request frames for that scan request.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_DEFAULT_IES = 16,
	/* Unsigned 32-bit attribute for generic commands */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_COMMAND = 17,
	/* Unsigned 32-bit value attribute for generic commands */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_VALUE = 18,
	/* Unsigned 32-bit data attribute for generic command response */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA = 19,
	/*
	 * Unsigned 32-bit length attribute for
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_LENGTH = 20,
	/*
	 * Unsigned 32-bit flags attribute for
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_DATA
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_GENERIC_FLAGS = 21,
	/*
	 * Unsigned 32-bit, defining the access policy.
	 * See enum qca_access_policy. Used with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY = 22,
	/*
	 * Sets the list of full set of IEs for which a specific access policy
	 * has to be applied. Used along with
	 * QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY to control the access.
	 * Zero length payload can be used to clear this access constraint.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ACCESS_POLICY_IE_LIST = 23,
	/*
	 * Unsigned 32-bit, specifies the interface index (netdev) for which the
	 * corresponding configurations are applied. If the interface index is
	 * not specified, the configurations are attributed to the respective
	 * wiphy.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_IFINDEX = 24,
	/* 8-bit unsigned value to trigger QPower: 1-Enable, 0-Disable */
	QCA_WLAN_VENDOR_ATTR_CONFIG_QPOWER = 25,
	/*
	 * 8-bit unsigned value to configure the driver and below layers to
	 * ignore the assoc disallowed set by APs while connecting
	 * 1-Ignore, 0-Don't ignore
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_IGNORE_ASSOC_DISALLOWED = 26,
	/*
	 * 32-bit unsigned value to trigger antenna diversity features:
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ENA = 27,
	/* 32-bit unsigned value to configure specific chain antenna */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_CHAIN = 28,
	/*
	 * 32-bit unsigned value to trigger cycle selftest
	 * 1-Enable, 0-Disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST = 29,
	/*
	 * 32-bit unsigned to configure the cycle time of selftest
	 * the unit is micro-second
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SELFTEST_INTVL = 30,
	/* 32-bit unsigned value to set reorder timeout for AC_VO */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VOICE = 31,
	/* 32-bit unsigned value to set reorder timeout for AC_VI */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_VIDEO = 32,
	/* 32-bit unsigned value to set reorder timeout for AC_BE */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BESTEFFORT = 33,
	/* 32-bit unsigned value to set reorder timeout for AC_BK */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_REORDER_TIMEOUT_BACKGROUND = 34,
	/* 6-byte MAC address to point out the specific peer */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_PEER_MAC = 35,
	/* 32-bit unsigned value to set window size for specific peer */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RX_BLOCKSIZE_WINLIMIT = 36,
	/* 8-bit unsigned value to set the beacon miss threshold in 2.4 GHz */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_24 = 37,
	/* 8-bit unsigned value to set the beacon miss threshold in 5 GHz */
	QCA_WLAN_VENDOR_ATTR_CONFIG_BEACON_MISS_THRESHOLD_5 = 38,
	/*
	 * 32-bit unsigned value to configure 5 or 10 MHz channel width for
	 * station device while in disconnect state. The attribute use the
	 * value of enum nl80211_chan_width: NL80211_CHAN_WIDTH_5 means 5 MHz,
	 * NL80211_CHAN_WIDTH_10 means 10 MHz. If set, the device work in 5 or
	 * 10 MHz channel width, the station will not connect to a BSS using 20
	 * MHz or higher bandwidth. Set to NL80211_CHAN_WIDTH_20_NOHT to
	 * clear this constraint.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SUB20_CHAN_WIDTH = 39,
	/*
	 * 32-bit unsigned value to configure the propagation absolute delay
	 * for 2G/5G band (units in us)
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PROPAGATION_ABS_DELAY = 40,
	/* 32-bit unsigned value to set probe period */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_PERIOD = 41,
	/* 32-bit unsigned value to set stay period */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_STAY_PERIOD = 42,
	/* 32-bit unsigned value to set snr diff */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_SNR_DIFF = 43,
	/* 32-bit unsigned value to set probe dwell time */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_PROBE_DWELL_TIME = 44,
	/* 32-bit unsigned value to set mgmt snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_MGMT_SNR_WEIGHT = 45,
	/* 32-bit unsigned value to set data snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_DATA_SNR_WEIGHT = 46,
	/* 32-bit unsigned value to set ack snr weight */
	QCA_WLAN_VENDOR_ATTR_CONFIG_ANT_DIV_ACK_SNR_WEIGHT = 47,
	/*
	 * 32-bit unsigned value to configure the listen interval.
	 * This is in units of beacon intervals. This configuration alters
	 * the negotiated listen interval with the AP during the connection.
	 * It is highly recommended to configure a value less than or equal to
	 * the one negotiated during the association. Configuring any greater
	 * value can have adverse effects (frame loss, AP disassociating STA,
	 * etc.).
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LISTEN_INTERVAL = 48,
	/*
	 * 8 bit unsigned value that is set on an AP/GO virtual interface to
	 * disable operations that would cause the AP/GO to leave its operating
	 * channel.
	 *
	 * This will restrict the scans to the AP/GO operating channel and the
	 * channels of the other band, if DBS is supported.A STA/CLI interface
	 * brought up after this setting is enabled, will be restricted to
	 * connecting to devices only on the AP/GO interface's operating channel
	 * or on the other band in DBS case. P2P supported channel list is
	 * modified, to only include AP interface's operating-channel and the
	 * channels of the other band if DBS is supported.
	 *
	 * These restrictions are only applicable as long as the AP/GO interface
	 * is alive. If the AP/GO interface is brought down then this
	 * setting/restriction is forgotten.
	 *
	 * If this variable is set on an AP/GO interface while a multi-channel
	 * concurrent session is active, it has no effect on the operation of
	 * the current interfaces, other than restricting the scan to the AP/GO
	 * operating channel and the other band channels if DBS is supported.
	 * However, if the STA is brought down and restarted then the new STA
	 * connection will either be formed on the AP/GO channel or on the
	 * other band in a DBS case. This is because of the scan being
	 * restricted on these channels as mentioned above.
	 *
	 * 1-Disable offchannel operations, 0-Enable offchannel operations.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RESTRICT_OFFCHANNEL = 49,

	/*
	 * 8 bit unsigned value to enable/disable LRO (Large Receive Offload)
	 * on an interface.
	 * 1 - Enable , 0 - Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LRO = 50,

	/*
	 * 8 bit unsigned value to globally enable/disable scan
	 * 1 - Enable, 0 - Disable.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_SCAN_ENABLE = 51,

	/* 8-bit unsigned value to set the total beacon miss count */
	QCA_WLAN_VENDOR_ATTR_CONFIG_TOTAL_BEACON_MISS_COUNT = 52,

	/*
	 * Unsigned 32-bit value to configure the number of continuous
	 * Beacon Miss which shall be used by the firmware to penalize
	 * the RSSI for BTC.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_PENALIZE_AFTER_NCONS_BEACON_MISS_BTC = 53,

	/*
	 * 8-bit unsigned value to configure the driver and below layers to
	 * enable/disable all fils features.
	 * 0-enable, 1-disable
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_DISABLE_FILS = 54,

	/* 16-bit unsigned value to configure the level of WLAN latency
	 * module. See enum qca_wlan_vendor_attr_config_latency_level.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL = 55,

	/*
	 * 8-bit unsigned value indicating the driver to use the RSNE as-is from
	 * the connect interface. Exclusively used for the scenarios where the
	 * device is used as a test bed device with special functionality and
	 * not recommended for production. This helps driver to not validate the
	 * RSNE passed from user space and thus allow arbitrary IE data to be
	 * used for testing purposes.
	 * 1-enable, 0-disable.
	 * Applications set/reset this configuration. If not reset, this
	 * parameter remains in use until the driver is unloaded.
	 */
	QCA_WLAN_VENDOR_ATTR_CONFIG_RSN_IE = 56,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_wifi_logger_start - Enum for wifi logger starting
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_INVALID: Invalid attribute
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID: Ring ID
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL: Verbose level
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS: Flag
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_AFTER_LAST: Last value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX: Max value
 */
enum qca_wlan_vendor_attr_wifi_logger_start {
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_RING_ID = 1,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_VERBOSE_LEVEL = 2,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_FLAGS = 3,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_MAX =
		QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_START_AFTER_LAST - 1,
};

/*
 * enum qca_wlan_vendor_attr_wifi_logger_get_ring_data - Get ring data
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_INVALID: Invalid attribute
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID: Ring ID
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_AFTER_LAST: Last value
 * @QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX: Max value
 */
enum qca_wlan_vendor_attr_wifi_logger_get_ring_data {
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_ID = 1,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_MAX =
		QCA_WLAN_VENDOR_ATTR_WIFI_LOGGER_GET_RING_DATA_AFTER_LAST - 1,
};

#ifdef WLAN_FEATURE_OFFLOAD_PACKETS
/**
 * enum wlan_offloaded_packets_control - control commands
 * @WLAN_START_OFFLOADED_PACKETS: start offloaded packets
 * @WLAN_STOP_OFFLOADED_PACKETS: stop offloaded packets
 *
 */
enum wlan_offloaded_packets_control {
	WLAN_START_OFFLOADED_PACKETS = 1,
	WLAN_STOP_OFFLOADED_PACKETS  = 2
};

/**
 * enum qca_wlan_vendor_attr_data_offload_ind - Vendor Data Offload Indication
 *
 * @QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_SESSION: Session corresponding to
 *      the offloaded data.
 * @QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_PROTOCOL: Protocol of the offloaded
 *      data.
 * @QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_EVENT: Event type for the data offload
 *      indication.
 */
enum qca_wlan_vendor_attr_data_offload_ind {
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_SESSION,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_PROTOCOL,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_EVENT,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_MAX =
	QCA_WLAN_VENDOR_ATTR_DATA_OFFLOAD_IND_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_offloaded_packets - offloaded packets
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_INVALID: invalid
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SENDING_CONTROL: control
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_REQUEST_ID: request id
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_IP_PACKET_DATA: ip packet data
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SRC_MAC_ADDR: src mac address
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_DST_MAC_ADDR: destination mac address
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_PERIOD: period in milli seconds
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_MAX: max
 */
enum qca_wlan_vendor_attr_offloaded_packets {
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SENDING_CONTROL,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_REQUEST_ID,

	/* Packet in hex format */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_IP_PACKET_DATA,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_SRC_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_DST_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_PERIOD,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_MAX =
		QCA_WLAN_VENDOR_ATTR_OFFLOADED_PACKETS_AFTER_LAST - 1,
};
#endif

/**
 * enum qca_wlan_rssi_monitoring_control - rssi control commands
 * @QCA_WLAN_RSSI_MONITORING_CONTROL_INVALID: invalid
 * @QCA_WLAN_RSSI_MONITORING_START: rssi monitoring start
 * @QCA_WLAN_RSSI_MONITORING_STOP: rssi monitoring stop
 */
enum qca_wlan_rssi_monitoring_control {
	QCA_WLAN_RSSI_MONITORING_CONTROL_INVALID = 0,
	QCA_WLAN_RSSI_MONITORING_START,
	QCA_WLAN_RSSI_MONITORING_STOP,
};

/**
 * enum qca_wlan_vendor_attr_rssi_monitoring - rssi monitoring
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CONTROL: control
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX_RSSI: max rssi
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MIN_RSSI: min rssi
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_BSSID: current bssid
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_RSSI: current rssi
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX: max
 */
enum qca_wlan_vendor_attr_rssi_monitoring {
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CONTROL,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_REQUEST_ID,

	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX_RSSI,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MIN_RSSI,

	/* attributes to be used/received in callback */
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_BSSID,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_CUR_RSSI,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_MAX =
		QCA_WLAN_VENDOR_ATTR_RSSI_MONITORING_AFTER_LAST - 1,
};

/**
 * qca_wlan_vendor_external_acs_event_chan_info_attr: Represents per channel
 * information. These attributes are sent as part of
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO. Each set of the following
 * attributes correspond to a single channel.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS: A bitmask (u16)
 * with flags specified in qca_wlan_vendor_channel_prop_flags_ext.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT: A bitmask (u16)
 * with flags specified in qca_wlan_vendor_channel_prop_flags_ext.
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ: frequency
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_REG_POWER: maximum
 * regulatory transmission power
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_POWER: maximum
 * transmission power
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MIN_POWER: minimum
 * transmission power
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_REG_CLASS_ID: regulatory
 * class id
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_ANTENNA_GAIN: maximum
 * antenna gain in dbm
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0: vht segment 0
 * @QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1: vht segment 1
 *
 */
enum qca_wlan_vendor_external_acs_event_chan_info_attr {
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_INVALID = 0,

	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS = 1,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAG_EXT = 2,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FREQ = 3,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_REG_POWER = 4,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX_POWER = 5,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MIN_POWER = 6,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_REG_CLASS_ID = 7,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_ANTENNA_GAIN = 8,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_0 = 9,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_VHT_SEG_1 = 10,
	/*
	 * A bitmask (u32) with flags specified in
	 * enum qca_wlan_vendor_channel_prop_flags_2.
	 */
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_FLAGS_2 = 11,

	/* keep last */
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_LAST,
	QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_MAX =
		QCA_WLAN_VENDOR_EXTERNAL_ACS_EVENT_CHAN_INFO_ATTR_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_start_acs_config: attribute to vendor sub-command
 * QCA_NL80211_VENDOR_SUBCMD_START_ACS. This will be triggered by host
 * driver.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON: This reason refers to
 * qca_wlan_vendor_acs_select_reason. This helps acs module to understand why
 * ACS need to be started
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_SPECTRAL_SUPPORTED: Does
 * driver supports spectral scanning or not
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_OFFLOAD_ENABLED: Is 11ac is
 * offloaded to firmware.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_ADD_CHAN_STATS_SUPPORT: Does driver
 * provides additional channel capability as part of scan operation.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_AP_UP:Flag attribute to indicate
 * interface status is UP
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_SAP_MODE: Operating mode of
 * interface. It takes one of nl80211_iftype values.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_WIDTH: This is the upper bound
 * of chan width. ACS logic should try to get a channel with specified width
 * if not found then look for lower values.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_BAND: nl80211_bands
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PHY_MODE: PHY/HW mode such as
 * a/b/g/n/ac.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_FREQ_LIST: Supported frequency list
 * among which ACS should choose best frequency.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL:Preferred Chan List by the
 * driver which will have <channel(u8), weight(u8)> format as array of
 * nested values.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO: Array of nested attribute
 * for each channel. It takes attr as defined in
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY:External ACS policy such as
 * PCL mandatory, PCL preferred, etc.It uses values defined in enum
 * qca_wlan_vendor_attr_external_acs_policy.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_RROPAVAIL_INFO: Reference RF
 * Operating Parameter (RROP) availability information (u16). It uses values
 * defined in enum qca_wlan_vendor_attr_rropavail_info.
 */
enum qca_wlan_vendor_attr_external_acs_event {
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON = 1,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_SPECTRAL_SUPPORTED = 2,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_IS_OFFLOAD_ENABLED = 3,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_ADD_CHAN_STATS_SUPPORT = 4,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_AP_UP = 5,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_SAP_MODE = 6,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_WIDTH = 7,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_BAND = 8,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PHY_MODE = 9,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_FREQ_LIST = 10,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL = 11,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_CHAN_INFO = 12,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY = 13,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_RROPAVAIL_INFO = 14,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_MAX =
		QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_LAST - 1,
};

enum qca_iface_type {
	QCA_IFACE_TYPE_STA,
	QCA_IFACE_TYPE_AP,
	QCA_IFACE_TYPE_P2P_CLIENT,
	QCA_IFACE_TYPE_P2P_GO,
	QCA_IFACE_TYPE_IBSS,
	QCA_IFACE_TYPE_TDLS,
};

/**
 * enum qca_wlan_vendor_attr_pcl_config: attribute to vendor sub-command
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL.
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_CHANNEL: pcl channel number
 * @QCA_WLAN_VENDOR_ATTR_PCL_CONFIG_WEIGHT: pcl channel weight
 */
enum qca_wlan_vendor_attr_pcl_config {
	QCA_WLAN_VENDOR_ATTR_PCL_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_PCL_CHANNEL = 1,
	QCA_WLAN_VENDOR_ATTR_PCL_WEIGHT = 2,
};

enum qca_set_band {
	QCA_SETBAND_AUTO,
	QCA_SETBAND_5G,
	QCA_SETBAND_2G,
};

/**
 * enum set_reset_packet_filter - set packet filter control commands
 * @QCA_WLAN_SET_PACKET_FILTER: Set Packet Filter
 * @QCA_WLAN_GET_PACKET_FILTER: Get Packet filter
 */
enum set_reset_packet_filter {
	QCA_WLAN_SET_PACKET_FILTER = 1,
	QCA_WLAN_GET_PACKET_FILTER = 2,
};

/**
 * enum qca_wlan_vendor_attr_packet_filter - BPF control commands
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_SET_RESET_PACKET_FILTER: Filter ID
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_VERSION: Filter Version
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_SIZE: Total Length
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_CURRENT_OFFSET: Current offset
 * @QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROGRAM: length of BPF instructions
 */
enum qca_wlan_vendor_attr_packet_filter {
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SET_RESET_PACKET_FILTER,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_VERSION,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_ID,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_SIZE,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_CURRENT_OFFSET,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_PROGRAM,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_MAX =
	QCA_WLAN_VENDOR_ATTR_PACKET_FILTER_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_wake_stats - wake lock stats
 * @QCA_WLAN_VENDOR_ATTR_GET_WAKE_STATS_INVALID: invalid
 * @QCA_WLAN_VENDOR_ATTR_TOTAL_CMD_EVENT_WAKE:
 * @QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_PTR:
 * @QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_SZ:
 * @QCA_WLAN_VENDOR_ATTR_TOTAL_DRIVER_FW_LOCAL_WAKE:
 * @QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_PTR:
 * @QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_SZ:
 * @QCA_WLAN_VENDOR_ATTR_TOTAL_RX_DATA_WAKE:
 * total rx wakeup count
 * @QCA_WLAN_VENDOR_ATTR_RX_UNICAST_CNT:
 * Total rx unicast packet which woke up host
 * @QCA_WLAN_VENDOR_ATTR_RX_MULTICAST_CNT:
 * Total rx multicast packet which woke up host
 * @QCA_WLAN_VENDOR_ATTR_RX_BROADCAST_CNT:
 * Total rx broadcast packet which woke up host
 * @QCA_WLAN_VENDOR_ATTR_ICMP_PKT:
 * wake icmp packet count
 * @QCA_WLAN_VENDOR_ATTR_ICMP6_PKT:
 * wake icmp6 packet count
 * @QCA_WLAN_VENDOR_ATTR_ICMP6_RA:
 * wake icmp6 RA packet count
 * @QCA_WLAN_VENDOR_ATTR_ICMP6_NA:
 * wake icmp6 NA packet count
 * @QCA_WLAN_VENDOR_ATTR_ICMP6_NS:
 * wake icmp6 NS packet count
 * @QCA_WLAN_VENDOR_ATTR_ICMP4_RX_MULTICAST_CNT:
 * Rx wake packet count due to ipv4 multicast
 * @QCA_WLAN_VENDOR_ATTR_ICMP6_RX_MULTICAST_CNT:
 * Rx wake packet count due to ipv6 multicast
 * @QCA_WLAN_VENDOR_ATTR_OTHER_RX_MULTICAST_CNT:
 * Rx wake packet count due to non-ipv4 and non-ipv6 packets
 * @QCA_WLAN_VENDOR_ATTR_RSSI_BREACH_CNT:
 * wake rssi breach packet count
 * @QCA_WLAN_VENDOR_ATTR_LOW_RSSI_CNT:
 * wake low rssi packet count
 * @QCA_WLAN_VENDOR_ATTR_GSCAN_CNT:
 * wake gscan packet count
 * @QCA_WLAN_VENDOR_ATTR_PNO_COMPLETE_CNT:
 * wake pno complete packet count
 * @QCA_WLAN_VENDOR_ATTR_PNO_MATCH_CNT:
 * wake pno match packet count
 */
enum qca_wlan_vendor_attr_wake_stats {
	QCA_WLAN_VENDOR_ATTR_GET_WAKE_STATS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TOTAL_CMD_EVENT_WAKE,
	QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_PTR,
	QCA_WLAN_VENDOR_ATTR_CMD_EVENT_WAKE_CNT_SZ,
	QCA_WLAN_VENDOR_ATTR_TOTAL_DRIVER_FW_LOCAL_WAKE,
	QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_PTR,
	QCA_WLAN_VENDOR_ATTR_DRIVER_FW_LOCAL_WAKE_CNT_SZ,
	QCA_WLAN_VENDOR_ATTR_TOTAL_RX_DATA_WAKE,
	QCA_WLAN_VENDOR_ATTR_RX_UNICAST_CNT,
	QCA_WLAN_VENDOR_ATTR_RX_MULTICAST_CNT,
	QCA_WLAN_VENDOR_ATTR_RX_BROADCAST_CNT,
	QCA_WLAN_VENDOR_ATTR_ICMP_PKT,
	QCA_WLAN_VENDOR_ATTR_ICMP6_PKT,
	QCA_WLAN_VENDOR_ATTR_ICMP6_RA,
	QCA_WLAN_VENDOR_ATTR_ICMP6_NA,
	QCA_WLAN_VENDOR_ATTR_ICMP6_NS,
	QCA_WLAN_VENDOR_ATTR_ICMP4_RX_MULTICAST_CNT,
	QCA_WLAN_VENDOR_ATTR_ICMP6_RX_MULTICAST_CNT,
	QCA_WLAN_VENDOR_ATTR_OTHER_RX_MULTICAST_CNT,
	QCA_WLAN_VENDOR_ATTR_RSSI_BREACH_CNT,
	QCA_WLAN_VENDOR_ATTR_LOW_RSSI_CNT,
	QCA_WLAN_VENDOR_ATTR_GSCAN_CNT,
	QCA_WLAN_VENDOR_ATTR_PNO_COMPLETE_CNT,
	QCA_WLAN_VENDOR_ATTR_PNO_MATCH_CNT,
	/* keep last */
	QCA_WLAN_VENDOR_GET_WAKE_STATS_AFTER_LAST,
	QCA_WLAN_VENDOR_GET_WAKE_STATS_MAX =
		QCA_WLAN_VENDOR_GET_WAKE_STATS_AFTER_LAST - 1,
};

/**
 * enum qca_vendor_element_id - QCA Vendor Specific element types
 *
 * These values are used to identify QCA Vendor Specific elements. The
 * payload of the element starts with the three octet OUI (OUI_QCA) and
 * is followed by a single octet type which is defined by this enum.
 *
 * @QCA_VENDOR_ELEM_P2P_PREF_CHAN_LIST: P2P preferred channel list.
 *      This element can be used to specify preference order for supported
 *      channels. The channels in this list are in preference order (the first
 *      one has the highest preference) and are described as a pair of
 *      (global) Operating Class and Channel Number (each one octet) fields.
 *
 *      This extends the standard P2P functionality by providing option to have
 *      more than one preferred operating channel. When this element is present,
 *      it replaces the preference indicated in the Operating Channel attribute.
 *      For supporting other implementations, the Operating Channel attribute is
 *      expected to be used with the highest preference channel. Similarly, all
 *      the channels included in this Preferred channel list element are
 *      expected to be included in the Channel List attribute.
 *
 *      This vendor element may be included in GO Negotiation Request, P2P
 *      Invitation Request, and Provision Discovery Request frames.
 *
 * @QCA_VENDOR_ELEM_HE_CAPAB: HE Capabilities element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID. The payload of this
 *      vendor specific element is defined by the latest P802.11ax draft.
 *      Please note that the draft is still work in progress and this element
 *      payload is subject to change.
 *
 * @QCA_VENDOR_ELEM_HE_OPER: HE Operation element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID. The payload of this
 *      vendor specific element is defined by the latest P802.11ax draft.
 *      Please note that the draft is still work in progress and this element
 *      payload is subject to change.
 *
 * @QCA_VENDOR_ELEM_RAPS: RAPS element (OFDMA-based Random Access Parameter Set
 *      element).
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID extension. The payload of
 *      this vendor specific element is defined by the latest P802.11ax draft
 *      (not including the Element ID Extension field). Please note that the
 *      draft is still work in progress and this element payload is subject to
 *      change.
 *
 * @QCA_VENDOR_ELEM_MU_EDCA_PARAMS: MU EDCA Parameter Set element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID extension. The payload of
 *      this vendor specific element is defined by the latest P802.11ax draft
 *      (not including the Element ID Extension field). Please note that the
 *      draft is still work in progress and this element payload is subject to
 *      change.
 *
 * @QCA_VENDOR_ELEM_BSS_COLOR_CHANGE: BSS Color Change Announcement element.
 *      This element can be used for pre-standard publication testing of HE
 *      before P802.11ax draft assigns the element ID extension. The payload of
 *      this vendor specific element is defined by the latest P802.11ax draft
 *      (not including the Element ID Extension field). Please note that the
 *      draft is still work in progress and this element payload is subject to
 *      change.
 */
enum qca_vendor_element_id {
	QCA_VENDOR_ELEM_P2P_PREF_CHAN_LIST = 0,
	QCA_VENDOR_ELEM_HE_CAPAB = 1,
	QCA_VENDOR_ELEM_HE_OPER = 2,
	QCA_VENDOR_ELEM_RAPS = 3,
	QCA_VENDOR_ELEM_MU_EDCA_PARAMS = 4,
	QCA_VENDOR_ELEM_BSS_COLOR_CHANGE = 5,
};

/**
 * enum qca_vendor_attr_get_tsf: Vendor attributes for TSF capture
 * @QCA_WLAN_VENDOR_ATTR_TSF_INVALID: Invalid attribute value
 * @QCA_WLAN_VENDOR_ATTR_TSF_CMD: enum qca_tsf_operation (u32)
 * @QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE: Unsigned 64 bit TSF timer value
 * @QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE: Unsigned 64 bit Synchronized
 *	SOC timer value at TSF capture
 * @QCA_WLAN_VENDOR_ATTR_TSF_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_TSF_MAX: Max value
 */
enum qca_vendor_attr_tsf_cmd {
	QCA_WLAN_VENDOR_ATTR_TSF_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TSF_CMD,
	QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE,
	QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE,
	QCA_WLAN_VENDOR_ATTR_TSF_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TSF_MAX =
		QCA_WLAN_VENDOR_ATTR_TSF_AFTER_LAST - 1
};

/**
 * enum qca_tsf_operation: TSF driver commands
 * @QCA_TSF_CAPTURE: Initiate TSF Capture
 * @QCA_TSF_GET: Get TSF capture value
 * @QCA_TSF_SYNC_GET: Initiate TSF capture and return with captured value
 */
enum qca_tsf_cmd {
	QCA_TSF_CAPTURE,
	QCA_TSF_GET,
	QCA_TSF_SYNC_GET,
};

/**
 * enum qca_vendor_attr_get_preferred_freq_list - get preferred channel list
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE: interface type
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST: preferred frequency list
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX: max
 */
enum qca_vendor_attr_get_preferred_freq_list {
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_INVALID,
	/* A 32-unsigned value; the interface type/mode for which the preferred
	 * frequency list is requested (see enum qca_iface_type for possible
	 * values); used in both south- and north-bound.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_IFACE_TYPE,
	/* An array of 32-unsigned values; values are frequency (MHz); used
	 * in north-bound only.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_MAX =
		QCA_WLAN_VENDOR_ATTR_GET_PREFERRED_FREQ_LIST_AFTER_LAST - 1
};

/**
 * enum qca_vendor_attr_probable_oper_channel - channel hint
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE: interface type
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ: frequency hint value
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_AFTER_LAST: last
 * @QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX: max
 */
enum qca_vendor_attr_probable_oper_channel {
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_INVALID,
	/* 32-bit unsigned value; indicates the connection/iface type likely to
	 * come on this channel (see enum qca_iface_type).
	 */
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_IFACE_TYPE,
	/* 32-bit unsigned value; the frequency (MHz) of the probable channel */
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_FREQ,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_MAX =
		QCA_WLAN_VENDOR_ATTR_PROBABLE_OPER_CHANNEL_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_gw_param_config - gateway param config
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_GW_MAC_ADDR: gateway mac addr
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV4_ADDR: ipv4 addr
 * @QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV6_ADDR: ipv6 addr
 */
enum qca_wlan_vendor_attr_gw_param_config {
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_GW_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV4_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_IPV6_ADDR,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_GW_PARAM_CONFIG_AFTER_LAST - 1,
};

/**
 * enum drv_dbs_capability - DBS capability
 * @DRV_DBS_CAPABILITY_DISABLED: DBS disabled
 * @DRV_DBS_CAPABILITY_1X1: 1x1
 * @DRV_DBS_CAPABILITY_2X2: 2x2
 */
enum drv_dbs_capability {
	DRV_DBS_CAPABILITY_DISABLED, /* not supported or disabled */
	DRV_DBS_CAPABILITY_1X1,
	DRV_DBS_CAPABILITY_2X2,
};

/**
 * enum qca_vendor_attr_txpower_decr_db - Attributes for TX power decrease
 *
 * These attributes are used with QCA_NL80211_VENDOR_SUBCMD_SET_TXPOWER_DECR_DB.
 */
enum qca_vendor_attr_txpower_decr_db {
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_INVALID,
	/*
	 * 8-bit unsigned value to indicate the reduction of TX power in dB for
	 * a virtual interface.
	 */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_MAX =
	QCA_WLAN_VENDOR_ATTR_TXPOWER_DECR_DB_AFTER_LAST - 1
};

/**
 * enum qca_vendor_attr_ota_test - Enable OTA test
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE: enable OTA test
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX: max
 */
enum qca_vendor_attr_ota_test {
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_INVALID,
	/* 8-bit unsigned value to indicate if OTA test is enabled */
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_ENABLE,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_OTA_TEST_MAX =
		QCA_WLAN_VENDOR_ATTR_OTA_TEST_AFTER_LAST - 1
};

/** enum qca_vendor_attr_txpower_scale - vendor sub commands index
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE: scaling value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX: max value
 */
enum qca_vendor_attr_txpower_scale {
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_INVALID,
	/* 8-bit unsigned value to indicate the scaling of tx power */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_MAX =
		QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_AFTER_LAST - 1
};

/**
 * enum qca_vendor_attr_txpower_scale_decr_db - vendor sub commands index
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB: scaling value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX: max value
 */
enum qca_vendor_attr_txpower_scale_decr_db {
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_INVALID,
	/* 8-bit unsigned value to indicate the scaling of tx power */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_MAX =
		QCA_WLAN_VENDOR_ATTR_TXPOWER_SCALE_DECR_DB_AFTER_LAST - 1
};

/**
 * enum dfs_mode - state of DFS mode
 * @DFS_MODE_NONE: DFS mode attribute is none
 * @DFS_MODE_ENABLE:  DFS mode is enabled
 * @DFS_MODE_DISABLE: DFS mode is disabled
 * @DFS_MODE_DEPRIORITIZE: Deprioritize DFS channels in scanning
 */
enum dfs_mode {
	DFS_MODE_NONE,
	DFS_MODE_ENABLE,
	DFS_MODE_DISABLE,
	DFS_MODE_DEPRIORITIZE
};

/**
 * enum qca_wlan_vendor_attr_acs_config - Config params for ACS
 * @QCA_WLAN_VENDOR_ATTR_ACS_MODE_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE: Dfs mode for ACS
 * QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT: channel_hint for ACS
 * QCA_WLAN_VENDOR_ATTR_ACS_DFS_AFTER_LAST: after_last
 * QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX: max attribute
 */
enum qca_wlan_vendor_attr_acs_config {
	QCA_WLAN_VENDOR_ATTR_ACS_MODE_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ACS_DFS_MODE,
	QCA_WLAN_VENDOR_ATTR_ACS_CHANNEL_HINT,

	QCA_WLAN_VENDOR_ATTR_ACS_DFS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ACS_DFS_MAX =
		QCA_WLAN_VENDOR_ATTR_ACS_DFS_AFTER_LAST - 1,

};

/**
 * enum qca_wlan_vendor_attr_get_hw_capability - Wi-Fi hardware capability
 */
enum qca_wlan_vendor_attr_get_hw_capability {
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_INVALID,
	/*
	 * Antenna isolation
	 * An attribute used in the response.
	 * The content of this attribute is encoded in a byte array. Each byte
	 * value is an antenna isolation value. The array length is the number
	 * of antennas.
	 */
	QCA_WLAN_VENDOR_ATTR_ANTENNA_ISOLATION,
	/*
	 * Request HW capability
	 * An attribute used in the request.
	 * The content of this attribute is a u32 array for one or more of
	 * hardware capabilities (attribute IDs) that are being requested. Each
	 * u32 value has a value from this
	 * enum qca_wlan_vendor_attr_get_hw_capability
	 * identifying which capabilities are requested.
	 */
	QCA_WLAN_VENDOR_ATTR_GET_HW_CAPABILITY,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_MAX =
	QCA_WLAN_VENDOR_ATTR_HW_CAPABILITY_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_sta_connect_roam_policy_config -
 *                        config params for sta roam policy
 * @QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE: If sta should skip Dfs channels
 * @QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL:
 * If sta should skip unsafe channels or not in scanning
 * @QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_LAST:
 * @QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX: max attribute
 */
enum qca_wlan_vendor_attr_sta_connect_roam_policy_config {
	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_STA_DFS_MODE,
	QCA_WLAN_VENDOR_ATTR_STA_SKIP_UNSAFE_CHANNEL,

	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_MAX =
	QCA_WLAN_VENDOR_ATTR_STA_CONNECT_ROAM_POLICY_AFTER_LAST - 1,
};

/* Attributes for FTM commands and events */

/**
 * enum qca_wlan_vendor_attr_loc_capa - Indoor location capabilities
 *
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAGS: Various flags. See
 *      enum qca_wlan_vendor_attr_loc_capa_flags.
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_SESSIONS: Maximum number
 *      of measurement sessions that can run concurrently.
 *      Default is one session (no session concurrency).
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_PEERS: The total number of unique
 *      peers that are supported in running sessions. For example,
 *      if the value is 8 and maximum number of sessions is 2, you can
 *      have one session with 8 unique peers, or 2 sessions with 4 unique
 *      peers each, and so on.
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_BURSTS_EXP: Maximum number
 *      of bursts per peer, as an exponent (2^value). Default is 0,
 *      meaning no multi-burst support.
 * @QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_MEAS_PER_BURST: Maximum number
 *      of measurement exchanges allowed in a single burst.
 * @QCA_WLAN_VENDOR_ATTR_AOA_CAPA_SUPPORTED_TYPES: Supported AOA measurement
 *      types. A bit mask (unsigned 32 bit value), each bit corresponds
 *      to an AOA type as defined by enum qca_vendor_attr_aoa_type.
 */
enum qca_wlan_vendor_attr_loc_capa {
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_INVALID,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAGS,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_SESSIONS,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_PEERS,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_NUM_BURSTS_EXP,
	QCA_WLAN_VENDOR_ATTR_FTM_CAPA_MAX_MEAS_PER_BURST,
	QCA_WLAN_VENDOR_ATTR_AOA_CAPA_SUPPORTED_TYPES,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_MAX =
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_loc_capa_flags: Indoor location capability flags
 *
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_RESPONDER: Set if driver
 *      can be configured as an FTM responder (for example, an AP that
 *      services FTM requests). QCA_NL80211_VENDOR_SUBCMD_FTM_CFG_RESPONDER
 *      will be supported if set.
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_INITIATOR: Set if driver
 *      can run FTM sessions. QCA_NL80211_VENDOR_SUBCMD_FTM_START_SESSION
 *      will be supported if set.
* @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_ASAP: Set if FTM responder
 *      supports immediate (ASAP) response.
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA: Set if driver supports standalone
 *      AOA measurement using QCA_NL80211_VENDOR_SUBCMD_AOA_MEAS.
 * @QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA_IN_FTM: Set if driver supports
 *      requesting AOA measurements as part of an FTM session.
 */
enum qca_wlan_vendor_attr_loc_capa_flags {
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_RESPONDER = 1 << 0,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_FTM_INITIATOR = 1 << 1,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_ASAP = 1 << 2,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA = 1 << 3,
	QCA_WLAN_VENDOR_ATTR_LOC_CAPA_FLAG_AOA_IN_FTM = 1 << 4,
};

/**
 * enum qca_wlan_vendor_attr_sap_config - config params for sap configuration
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_INVALID: invalid
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL: Channel on which SAP should start
 * @QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST: List of frequencies on
 *  which AP is expected to operate. This is irrespective of ACS configuration.
 *  This list is a priority based one and is looked for before the AP is created
 *  to ensure the best concurrency sessions (avoid MCC and use DBS/SCC) co-exist
 *  in the system.
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_AFTER_LAST: after last
 * @QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX: max attribute
 */
enum qca_wlan_vendor_attr_sap_config {
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_SAP_MANDATORY_FREQUENCY_LIST = 2,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_SAP_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_info: Information about
 * a single peer in a measurement session.
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MAC_ADDR: The MAC address of the peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAGS: Various flags related
 *      to measurement. See enum qca_wlan_vendor_attr_ftm_peer_meas_flags.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_PARAMS: Nested attribute of
 *      FTM measurement parameters, as specified by IEEE P802.11-REVmc/D7.0
 *      9.4.2.167. See enum qca_wlan_vendor_attr_ftm_meas_param for
 *      list of supported attributes.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_SECURE_TOKEN_ID: Initial token ID for
 *      secure measurement.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_AOA_BURST_PERIOD: Request AOA
 *      measurement every <value> bursts. If 0 or not specified,
 *      AOA measurements will be disabled for this peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_FREQ: Frequency in MHz where
 *      the measurement frames are exchanged. Optional; if not
 *      specified, try to locate the peer in the kernel scan
 *      results cache and use frequency from there.
 */
enum qca_wlan_vendor_attr_ftm_peer_info {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAGS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_PARAMS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_SECURE_TOKEN_ID,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_AOA_BURST_PERIOD,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_FREQ,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_meas_flags: Measurement request flags,
 *      per-peer
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_ASAP: If set, request
 *      immediate (ASAP) response from peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCI: If set, request
 *      LCI report from peer. The LCI report includes the absolute
 *      location of the peer in "official" coordinates (similar to GPS).
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.7 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCR: If set, request
 *      Location civic report from peer. The LCR includes the location
 *      of the peer in free-form format. See IEEE P802.11-REVmc/D7.0,
 *      11.24.6.7 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_SECURE: If set,
 *      request a secure measurement.
 *      QCA_WLAN_VENDOR_ATTR_FTM_PEER_SECURE_TOKEN_ID must also be provided.
 */
enum qca_wlan_vendor_attr_ftm_peer_meas_flags {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_ASAP    = 1 << 0,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCI     = 1 << 1,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_LCR     = 1 << 2,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_MEAS_FLAG_SECURE  = 1 << 3,
};

/**
 * enum qca_wlan_vendor_attr_ftm_meas_param: Measurement parameters
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_MEAS_PER_BURST: Number of measurements
 *      to perform in a single burst.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_NUM_BURSTS_EXP: Number of bursts to
 *      perform, specified as an exponent (2^value).
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_DURATION: Duration of burst
 *      instance, as specified in IEEE P802.11-REVmc/D7.0, 9.4.2.167.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_PERIOD: Time between bursts,
 *      as specified in IEEE P802.11-REVmc/D7.0, 9.4.2.167. Must
 *      be larger than QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_DURATION.
 */
enum qca_wlan_vendor_attr_ftm_meas_param {
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_MEAS_PER_BURST,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_NUM_BURSTS_EXP,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_DURATION,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_BURST_PERIOD,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_PARAM_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_result: Per-peer results
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MAC_ADDR: MAC address of the reported
 *       peer.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS: Status of measurement
 *      request for this peer.
 *      See enum qca_wlan_vendor_attr_ftm_peer_result_status.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAGS: Various flags related
 *      to measurement results for this peer.
 *      See enum qca_wlan_vendor_attr_ftm_peer_result_flags.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_VALUE_SECONDS: Specified when
 *      request failed and peer requested not to send an additional request
 *      for this number of seconds.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCI: LCI report when received
 *      from peer. In the format specified by IEEE P802.11-REVmc/D7.0,
 *      9.4.2.22.10.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCR: Location civic report when
 *      received from peer. In the format specified by IEEE P802.11-REVmc/D7.0,
 *      9.4.2.22.13.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS_PARAMS: Reported when peer
 *      overridden some measurement request parameters. See
 *      enum qca_wlan_vendor_attr_ftm_meas_param.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AOA_MEAS: AOA measurement
 *      for this peer. Same contents as @QCA_WLAN_VENDOR_ATTR_AOA_MEAS_RESULT.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS: Array of measurement
 *      results. Each entry is a nested attribute defined
 *      by enum qca_wlan_vendor_attr_ftm_meas.
 */
enum qca_wlan_vendor_attr_ftm_peer_result {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MAC_ADDR,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAGS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_VALUE_SECONDS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCI,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_LCR,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS_PARAMS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AOA_MEAS,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_result_status
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_OK: Request sent ok and results
 *      will be provided. Peer may have overridden some measurement parameters,
 *      in which case overridden parameters will be report by
 *      QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_MEAS_PARAM attribute.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INCAPABLE: Peer is incapable
 *      of performing the measurement request. No more results will be sent
 *      for this peer in this session.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_FAILED: Peer reported request
 *      failed, and requested not to send an additional request for number
 *      of seconds specified by QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_VALUE_SECONDS
 *      attribute.
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INVALID: Request validation
 *      failed. Request was not sent over the air.
 */
enum qca_wlan_vendor_attr_ftm_peer_result_status {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_OK,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INCAPABLE,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_FAILED,
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_STATUS_INVALID,
};

/**
 * enum qca_wlan_vendor_attr_ftm_peer_result_flags: Various flags
 *  for measurement result, per-peer
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAG_DONE: If set,
 *      measurement completed for this peer. No more results will be reported
 *      for this peer in this session.
 */
enum qca_wlan_vendor_attr_ftm_peer_result_flags {
	QCA_WLAN_VENDOR_ATTR_FTM_PEER_RES_FLAG_DONE = 1 << 0,
};

/**
 * enum qca_vendor_attr_loc_session_status: Session completion status code
 *
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_OK: Session completed
 *      successfully.
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_ABORTED: Session aborted
 *      by request.
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_INVALID: Session request
 *      was invalid and was not started.
 * @QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_FAILED: Session had an error
 *      and did not complete normally (for example out of resources).
 */
enum qca_vendor_attr_loc_session_status {
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_OK,
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_ABORTED,
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_INVALID,
	QCA_WLAN_VENDOR_ATTR_LOC_SESSION_STATUS_FAILED,
};

/**
 * enum qca_wlan_vendor_attr_ftm_meas: Single measurement data
 *
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T1: Time of departure (TOD) of FTM packet as
 *      recorded by responder, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T2: Time of arrival (TOA) of FTM packet at
 *      initiator, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T3: TOD of ACK packet as recorded by
 *      initiator, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T4: TOA of ACK packet at
 *      responder, in picoseconds.
 *      See IEEE P802.11-REVmc/D7.0, 11.24.6.4 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_RSSI: RSSI (signal level) as recorded
 *      during this measurement exchange. Optional and will be provided if
 *      the hardware can measure it.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOD_ERR: TOD error reported by
 *      responder. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOA_ERR: TOA error reported by
 *      responder. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOD_ERR: TOD error measured by
 *      initiator. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOA_ERR: TOA error measured by
 *      initiator. Not always provided.
 *      See IEEE P802.11-REVmc/D7.0, 9.6.8.33 for more information.
 * @QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PAD: Dummy attribute for padding.
 */
enum qca_wlan_vendor_attr_ftm_meas {
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INVALID,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T1,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T2,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T3,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_T4,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_RSSI,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOD_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_TOA_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOD_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_INITIATOR_TOA_ERR,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_PAD,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_MAX =
	QCA_WLAN_VENDOR_ATTR_FTM_MEAS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_p2p_listen_offload - vendor sub commands index
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INVALID: invalid value
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL:
 *     A 32-bit unsigned value; the P2P listen frequency (MHz); must be one
 *     of the social channels.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD: listen offload period
 *     A 32-bit unsigned value; the P2P listen offload period (ms).
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL:
 *     A 32-bit unsigned value; the P2P listen interval duration (ms).
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT:
 *     A 32-bit unsigned value; number of interval times the Firmware needs
 *     to run the offloaded P2P listen operation before it stops.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES: device types
 *     An array of unsigned 8-bit characters; vendor information elements.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE: vendor IEs
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG: control flag for FW
 *     A 32-bit unsigned value; a control flag to indicate whether listen
 *     results need to be flushed to wpa_supplicant.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_STOP_REASON: offload stop reason
 *     A 8-bit unsigned value; reason code for P2P listen offload stop
 *     event.
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_AFTER_LAST: last value
 * @QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX: max value
 */
enum qca_wlan_vendor_attr_p2p_listen_offload {
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CHANNEL,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_PERIOD,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_INTERVAL,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_COUNT,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_DEVICE_TYPES,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_VENDOR_IE,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_CTRL_FLAG,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_STOP_REASON,
	/* keep last */
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_MAX =
	QCA_WLAN_VENDOR_ATTR_P2P_LISTEN_OFFLOAD_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_drv_info - WLAN driver info
 * @QCA_WLAN_VENDOR_ATTR_DRV_INFO_INVALID: Invalid
 * @QCA_WLAN_VENDOR_ATTR_DRV_INFO_BUS_SIZE: Maximum Message size info
 * between Firmware & Host.
 */
enum qca_wlan_vendor_drv_info {
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_BUS_SIZE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DRV_INFO_MAX =
		QCA_WLAN_VENDOR_ATTR_DRV_INFO_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_aoa_type - AOA measurement type
 *
 * @QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE: Phase of the strongest
 *      CIR (channel impulse response) path for each antenna.
 * @QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE_AMP: Phase and amplitude
 *      of the strongest CIR path for each antenna.
 */
enum qca_wlan_vendor_attr_aoa_type {
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE,
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE_TOP_CIR_PHASE_AMP,
	QCA_WLAN_VENDOR_ATTR_AOA_TYPE_MAX
};

/**
 * enum qca_wlan_vendor_attr_encryption_test - Attributes to
 * validate encryption engine
 *
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_NEEDS_DECRYPTION: Flag attribute.
 *    This will be included if the request is for decryption; if not included,
 *    the request is treated as a request for encryption by default.
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_CIPHER: Unsigned 32-bit value
 *    indicating the key cipher suite. Takes same values as
 *    NL80211_ATTR_KEY_CIPHER.
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_KEYID: Unsigned 8-bit value
 *    Key Id to be used for encryption
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK: Array of 8-bit values.
 *    Key (TK) to be used for encryption/decryption
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN: Array of 8-bit values.
 *    Packet number to be specified for encryption/decryption
 *    6 bytes for TKIP/CCMP/GCMP.
 * @QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA: Array of 8-bit values
 *    representing the 802.11 packet (header + payload + FCS) that
 *    needs to be encrypted/decrypted.
 *    Encrypted/decrypted response from the driver will also be sent
 *    to userspace with the same attribute.
 */
enum qca_wlan_vendor_attr_encryption_test {
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_NEEDS_DECRYPTION,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_CIPHER,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_KEYID,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_TK,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_PN,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_DATA,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_MAX =
		QCA_WLAN_VENDOR_ATTR_ENCRYPTION_TEST_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_dmg_rf_sector_type - Type of
 * sector for DMG RF sector operations.
 *
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_RX: RX sector
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_TX: TX sector
 */
enum qca_wlan_vendor_attr_dmg_rf_sector_type {
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_RX,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_TX,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_TYPE_MAX
};

/**
 * BRP antenna limit mode
 *
 * @QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_DISABLE: Disable BRP force
 *      antenna limit, BRP will be performed as usual.
 * @QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_EFFECTIVE: Define maximal
 *      antennas limit. the hardware may use less antennas than the
 *      maximum limit.
 * @QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_FORCE: The hardware will
 *      use exactly the specified number of antennas for BRP.
 */
enum qca_wlan_vendor_attr_brp_ant_limit_mode {
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_DISABLE,
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_EFFECTIVE,
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_FORCE,
	QCA_WLAN_VENDOR_ATTR_BRP_ANT_LIMIT_MODE_MAX
};

/**
 * enum qca_wlan_vendor_attr_dmg_rf_sector_cfg - Attributes for
 * DMG RF sector configuration for a single RF module.
 * The values are defined in a compact way which closely matches
 * the way it is stored in HW registers.
 * The configuration provides values for 32 antennas and 8 distribution
 * amplifiers, and together describes the characteristics of the RF
 * sector - such as a beam in some direction with some gain.
 *
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_MODULE_INDEX: Index
 *      of RF module for this configuration.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE0: Bit 0 of edge
 *      amplifier gain index. Unsigned 32 bit number containing
 *      bits for all 32 antennas.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE1: Bit 1 of edge
 *      amplifier gain index. Unsigned 32 bit number containing
 *      bits for all 32 antennas.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE2: Bit 2 of edge
 *      amplifier gain index. Unsigned 32 bit number containing
 *      bits for all 32 antennas.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_HI: Phase values
 *      for first 16 antennas, 2 bits per antenna.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_LO: Phase values
 *      for last 16 antennas, 2 bits per antenna.
 * @QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_DTYPE_X16: Contains
 *      DTYPE values (3 bits) for each distribution amplifier, followed
 *      by X16 switch bits for each distribution amplifier. There are
 *      total of 8 distribution amplifiers.
 */
enum qca_wlan_vendor_attr_dmg_rf_sector_cfg {
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_MODULE_INDEX = 1,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE0 = 2,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE1 = 3,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_ETYPE2 = 4,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_HI = 5,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_PSH_LO = 6,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_DTYPE_X16 = 7,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_MAX =
	QCA_WLAN_VENDOR_ATTR_DMG_RF_SECTOR_CFG_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_tdls_configuration - Attributes for
 * @QCA_NL80211_VENDOR_SUBCMD_CONFIGURE_TDLS configuration to the host driver.
 *
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE: Configure the TDLS trigger
 *	mode in the host driver. enum qca_wlan_vendor_tdls_trigger_mode
 *	represents the different TDLS trigger modes.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD: Duration (u32) within
 *	which QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD number
 *	of packets shall meet the criteria for implicit TDLS setup.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD: Number (u32) of Tx/Rx
 *	packets within a duration.
 *	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD to initiate
 *	a TDLS setup.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_DISCOVERY_PERIOD: Time (u32) to inititate
 *	a TDLS Discovery to the Peer.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT: Max number (u32) of
 *	discovery attempts to know the TDLS capability of the peer. A peer is
 *	marked as TDLS not capable if there is no response for all the attempts.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT: Represents a duration (u32)
 *	within which QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD
 *	number of TX / RX frames meet the criteria for TDLS teardown.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD: Minumum number
 *	(u32) of Tx/Rx packets within a duration
 *	CA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT to tear down a TDLS link
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD: Threshold
 *	corresponding to the RSSI of the peer below which a TDLS
 *	setup is triggered.
 * @QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD: Threshold
 *	corresponding to the RSSI of the peer above which
 *	a TDLS teardown is triggered.
 */
enum qca_wlan_vendor_attr_tdls_configuration {
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE = 1,

	/* Attributes configuring the TDLS Implicit Trigger */
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_STATS_PERIOD = 2,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TX_THRESHOLD = 3,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_DISCOVERY_PERIOD = 4,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX_DISCOVERY_ATTEMPT = 5,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_TIMEOUT = 6,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IDLE_PACKET_THRESHOLD = 7,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_SETUP_RSSI_THRESHOLD = 8,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TEARDOWN_RSSI_THRESHOLD = 9,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_tdls_trigger_mode: Represents the TDLS trigger mode in
 *	the driver.
 *
 * The following are the different values for
 * QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_TRIGGER_MODE.
 *
 * @QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT: The trigger to
 *	initiate/teardown the TDLS connection to a respective peer comes
 *	 from the user space. wpa_supplicant provides the commands
 *	TDLS_SETUP, TDLS_TEARDOWN, TDLS_DISCOVER to do this.
 * @QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT: Host driver triggers this TDLS
 *	setup/teardown to the eligible peer once the configured criteria
 *	(such as TX/RX threshold, RSSI) is met. The attributes
 *	in QCA_WLAN_VENDOR_ATTR_TDLS_CONFIG_IMPLICIT_PARAMS correspond to
 *	the different configuration criteria for the TDLS trigger from the
 *	host driver.
 * @QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL: Enables the driver to trigger
 *	the TDLS setup / teardown through the implicit mode, only to the
 *	configured MAC addresses(wpa_supplicant, with tdls_external_control = 1,
 *	configures the MAC address through TDLS_SETUP/TDLS_TEARDOWN commands).
 *	External mode works on top of the implicit mode, thus the host Driver
 *	is expected to be configured in TDLS Implicit mode too to operate in
 *	External mode. Configuring External mode alone without Implicit
 *	mode is invalid.
 *
 * All the above implementations work as expected only when the host driver
 * advertises the capability WPA_DRIVER_FLAGS_TDLS_EXTERNAL_SETUP -
 * representing that the TDLS message exchange is not internal to the host
 * driver, but depends on wpa_supplicant to do the message exchange.
 */
enum qca_wlan_vendor_tdls_trigger_mode {
	QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT = 1 << 0,
	QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT = 1 << 1,
	QCA_WLAN_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL = 1 << 2,
};

/**
 * enum qca_vendor_attr_sar_limits_selections - Source of SAR power limits
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF0: Select SAR profile #0
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF1: Select SAR profile #1
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF2: Select SAR profile #2
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF3: Select SAR profile #3
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF4: Select SAR profile #4
 *	that is hard-coded in the Board Data File (BDF).
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_NONE: Do not select any
 *	source of SAR power limits, thereby disabling the SAR power
 *	limit feature.
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER: Select the SAR power
 *	limits configured by %QCA_NL80211_VENDOR_SUBCMD_SET_SAR.
 *
 * This enumerates the valid set of values that may be supplied for
 * attribute %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT in an instance of
 * the %QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS vendor command or in
 * the response to an instance of the
 * %QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS vendor command.
 */
enum qca_vendor_attr_sar_limits_selections {
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF0 = 0,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF1 = 1,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF2 = 2,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF3 = 3,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_BDF4 = 4,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_NONE = 5,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SELECT_USER = 6,
};

/**
 * enum qca_vendor_attr_sar_limits_spec_modulations -
 *	SAR limits specification modulation
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_CCK -
 *	CCK modulation
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_OFDM -
 *	OFDM modulation
 *
 * This enumerates the valid set of values that may be supplied for
 * attribute %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION in an
 * instance of attribute %QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC in an
 * instance of the %QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS vendor
 * command or in the response to an instance of the
 * %QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS vendor command.
 */
enum qca_vendor_attr_sar_limits_spec_modulations {
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_CCK = 0,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION_OFDM = 1,
};

/**
 * enum qca_vendor_attr_sar_limits - Attributes for SAR power limits
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE: Optional (u32) value to
 *	select which SAR power limit table should be used.  Valid
 *	values are enumerated in enum
 *	%qca_vendor_attr_sar_limits_selections.  The existing SAR
 *	power limit selection is unchanged if this attribute is not
 *	present.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS: Optional (u32) value
 *	which specifies the number of SAR power limit specifications
 *	which will follow.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC: Nested array of SAR power
 *	limit specifications. The number of specifications is
 *	specified by @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS. Each
 *	specification contains a set of
 *	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_* attributes. A
 *	specification is uniquely identified by the attributes
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND,
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN, and
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION and always
 *	contains as a payload the attribute
 *	%QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND: Optional (u32) value to
 *	indicate for which band this specification applies. Valid
 *	values are enumerated in enum %nl80211_band (although not all
 *	bands may be supported by a given device). If the attribute is
 *	not supplied then the specification will be applied to all
 *	supported bands.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN: Optional (u32) value
 *	to indicate for which antenna chain this specification
 *	applies, i.e. 1 for chain 1, 2 for chain 2, etc. If the
 *	attribute is not supplied then the specification will be
 *	applied to all chains.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION: Optional (u32)
 *	value to indicate for which modulation scheme this
 *	specification applies. Valid values are enumerated in enum
 *	%qca_vendor_attr_sar_limits_spec_modulations. If the attribute
 *	is not supplied then the specification will be applied to all
 *	modulation schemes.
 *
 * @QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT: Required (u32)
 *	value to specify the actual power limit value in steps of 0.5
 *	dbm.
 *
 * These attributes are used with %QCA_NL80211_VENDOR_SUBCMD_SET_SAR_LIMITS
 * and %QCA_NL80211_VENDOR_SUBCMD_GET_SAR_LIMITS.
 */
enum qca_vendor_attr_sar_limits {
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SAR_ENABLE = 1,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_NUM_SPECS = 2,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC = 3,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_BAND = 4,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_CHAIN = 5,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_MODULATION = 6,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_SPEC_POWER_LIMIT = 7,

	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_MAX =
		QCA_WLAN_VENDOR_ATTR_SAR_LIMITS_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_ll_stats_ext - Attributes for MAC layer monitoring
 *    offload which is an extension for LL_STATS.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD: Monitoring period. Unit in ms.
 *    If MAC counters do not exceed the threshold, FW will report monitored
 *    link layer counters periodically as this setting. The first report is
 *    always triggered by this timer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD: It is a percentage (1-99).
 *    For each MAC layer counter, FW holds two copies. One is the current value.
 *    The other is the last report. Once a current counter's increment is larger
 *    than the threshold, FW will indicate that counter to host even if the
 *    monitoring timer does not expire.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_CHG: Peer STA power state change
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TID: TID of MSDU
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NUM_MSDU: Count of MSDU with the same
 *    failure code.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_STATUS: TX failure code
 *    1: TX packet discarded
 *    2: No ACK
 *    3: Postpone
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_MAC_ADDRESS: peer MAC address
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_STATE: Peer STA current state
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL: Global threshold.
 *    Threshold for all monitored parameters. If per counter dedicated threshold
 *    is not enabled, this threshold will take effect.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_EVENT_MODE: Indicate what triggers this
 *    event, PERORID_TIMEOUT == 1, THRESH_EXCEED == 0.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_ID: interface ID
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ID: peer ID
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP: bitmap for TX counters
 *    Bit0: TX counter unit in MSDU
 *    Bit1: TX counter unit in MPDU
 *    Bit2: TX counter unit in PPDU
 *    Bit3: TX counter unit in byte
 *    Bit4: Dropped MSDUs
 *    Bit5: Dropped Bytes
 *    Bit6: MPDU retry counter
 *    Bit7: MPDU failure counter
 *    Bit8: PPDU failure counter
 *    Bit9: MPDU aggregation counter
 *    Bit10: MCS counter for ACKed MPDUs
 *    Bit11: MCS counter for Failed MPDUs
 *    Bit12: TX Delay counter
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP: bitmap for RX counters
 *    Bit0: MAC RX counter unit in MPDU
 *    Bit1: MAC RX counter unit in byte
 *    Bit2: PHY RX counter unit in PPDU
 *    Bit3: PHY RX counter unit in byte
 *    Bit4: Disorder counter
 *    Bit5: Retry counter
 *    Bit6: Duplication counter
 *    Bit7: Discard counter
 *    Bit8: MPDU aggregation size counter
 *    Bit9: MCS counter
 *    Bit10: Peer STA power state change (wake to sleep) counter
 *    Bit11: Peer STA power save counter, total time in PS mode
 *    Bit12: Probe request counter
 *    Bit13: Other management frames counter
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP: bitmap for CCA
 *    Bit0: Idle time
 *    Bit1: TX time
 *    Bit2: time RX in current bss
 *    Bit3: Out of current bss time
 *    Bit4: Wireless medium busy time
 *    Bit5: RX in bad condition time
 *    Bit6: TX in bad condition time
 *    Bit7: time wlan card not available
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP: bitmap for signal
 *    Bit0: Per channel SNR counter
 *    Bit1: Per channel noise floor counter
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_NUM: number of peers
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CHANNEL_NUM: number of channels
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_AC_RX_NUM: number of RX stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS: per channel BSS CCA stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER: container for per PEER stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU: Number of total TX MSDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU: Number of total TX MPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU: Number of total TX PPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES: bytes of TX data
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP: Number of dropped TX packets
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES: Bytes dropped
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY: waiting time without an ACK
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK: number of MPDU not-ACKed
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK: number of PPDU not-ACKed
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR_NUM:
 *    aggregation stats buffer length
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS_NUM: length of mcs stats
 *    buffer for ACKed MPDUs.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS_NUM: length of mcs stats
 *    buffer for failed MPDUs.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_DELAY_ARRAY_SIZE:
 *    length of delay stats array.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR: TX aggregation stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS: MCS stats for ACKed MPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS: MCS stats for failed MPDUs
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY: tx delay stats
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU: MPDUs received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES: bytes received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU: PPDU received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES: PPDU bytes received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST: packets lost
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY: number of RX packets
 *    flagged as retransmissions
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP: number of RX packets
 *    flagged as duplicated
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD: number of RX
 *    packets discarded
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR_NUM: length of RX aggregation
 *    stats buffer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS_NUM: length of RX mcs
 *    stats buffer.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS: RX mcs stats buffer
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR: aggregation stats buffer
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES: times STAs go to sleep
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION: STAs' total sleep time
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ: number of probe
 *    requests received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT: number of other mgmt
 *    frames received
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME: Percentage of idle time
 *    there is no TX, nor RX, nor interference.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME: percentage of time
 *    transmitting packets.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_TIME: percentage of time
 *    for receiving.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY: percentage of time
 *    interference detected.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD: percentage of time
 *    receiving packets with errors.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD: percentage of time
 *    TX no-ACK.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL: percentage of time
 *    the chip is unable to work in normal conditions.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME: percentage of time
 *    receiving packets in current BSS.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME: percentage of time
 *    receiving packets not in current BSS.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ANT_NUM: number of antennas
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_SIGNAL:
 *    This is a container for per antenna signal stats.
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR: per antenna SNR value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF: per antenna NF value
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_RSSI_BEACON: RSSI of beacon
 * @QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_SNR_BEACON: SNR of beacon
 */
enum qca_wlan_vendor_attr_ll_stats_ext {
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_INVALID = 0,

	/* Attributes for configurations */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_PERIOD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_CFG_THRESHOLD,

	/* Peer STA power state change */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_CHG,

	/* TX failure event */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NUM_MSDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_STATUS,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_STATE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_MAC_ADDRESS,

	/* MAC counters */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_GLOBAL,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_EVENT_MODE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_ID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ID,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_SIGNAL_BITMAP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CHANNEL_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_CCA_BSS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER,

	/* Sub-attributes for PEER_AC_TX */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MSDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_MPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_PPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DROP_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_RETRY,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_ACK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_NO_BACK,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_AGGR,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_SUCC_MCS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_FAIL_MCS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_DELAY_ARRAY_SIZE,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_DELAY,

	/* Sub-attributes for PEER_AC_RX */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PPDU_BYTES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_LOST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_RETRY,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DUP,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MPDU_DISCARD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MCS,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_AGGR,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_TIMES,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_PS_DURATION,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_PROBE_REQ,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_MGMT,

	/* Sub-attributes for CCA_BSS */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IDLE_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BUSY,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_RX_BAD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_TX_BAD,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_NO_AVAIL,

	/* sub-attribute for BSS_RX_TIME */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IN_BSS_TIME,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_OUT_BSS_TIME,

	/* Sub-attributes for PEER_SIGNAL */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_ANT_NUM,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_PEER_SIGNAL,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_SNR,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_ANT_NF,

	/* Sub-attributes for IFACE_BSS */
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_RSSI_BEACON,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_IFACE_SNR_BEACON,

	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_LAST,
	QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_MAX =
		QCA_WLAN_VENDOR_ATTR_LL_STATS_EXT_LAST - 1
};

/**
 * qca_wlan_vendor_attr_external_acs_channels: attribute to vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS. This carry a list of channels
 * in priority order as decided after acs operation in userspace.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_REASON: One of reason code from
 * qca_wlan_vendor_acs_select_reason.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_COUNT: Number of channels in
 * this list
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST: Array of nested values
 * for each channel with following attributes:
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1,
 *     QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY: Primary channel (u8)
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY: Secondary channel (u8)
 * required only for 160 / 80 + 80
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0: VHT seg0 channel (u8)
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1: VHT seg1 channel (u8)
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH:channel width (u8)
 */
enum qca_wlan_vendor_attr_external_acs_channels {
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_INVALID = 0,

	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_REASON = 1,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LIST = 2,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_BAND = 3,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_PRIMARY = 4,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_SECONDARY = 5,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG0 = 6,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_CENTER_SEG1 = 7,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_WIDTH = 8,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LAST,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_MAX =
		QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_CHANNEL_LAST - 1
};

/**
 * qca_wlan_vendor_acs_select_reason: This represents the different reasons why
 * the ACS has to be triggered. These parameters are used by
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_REASON and
 * QCA_NL80211_VENDOR_SUBCMD_ACS_SET_CHANNELS
 * @QCA_WLAN_VENDOR_ACS_SELECT_REASON_INIT: Represents the reason that the
 * ACS triggered during the AP start
 * @QCA_WLAN_VENDOR_ACS_SELECT_REASON_DFS: Represents the reason that
 * DFS found with current channel
 * @QCA_WLAN_VENDOR_ACS_SELECT_REASON_LTE_COEX: Represents the reason that
 * LTE CO-Exist in current band
 */
enum qca_wlan_vendor_acs_select_reason {
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_INIT,
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_DFS,
	QCA_WLAN_VENDOR_ACS_SELECT_REASON_LTE_COEX,
};

/**
 * enum qca_wlan_gpio_attr - Parameters for GPIO configuration
 */
enum qca_wlan_gpio_attr {
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INVALID = 0,
	/* Unsigned 32-bit attribute for GPIO command */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND,
	/* Unsigned 32-bit attribute for GPIO PIN number to configure */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PINNUM,
	/* Unsigned 32-bit attribute for GPIO value to configure */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_VALUE,
	/* Unsigned 32-bit attribute for GPIO pull type */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PULL_TYPE,
	/* Unsigned 32-bit attribute for GPIO interrupt mode */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTR_MODE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_LAST,
	QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MAX =
		QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_LAST - 1,
};

/**
 * qca_wlan_set_qdepth_thresh_attr - Parameters for setting
 * MSDUQ depth threshold per peer per tid in the target
 *
 * Associated Vendor Command:
 * QCA_NL80211_VENDOR_SUBCMD_SET_QDEPTH_THRESH
 */
enum qca_wlan_set_qdepth_thresh_attr {
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_INVALID = 0,
	/* 6-byte MAC address */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_MAC_ADDR,
	/* Unsigned 32-bit attribute for holding the TID */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_TID,
	/* Unsigned 32-bit attribute for holding the update mask
	 * bit 0 - Update high priority msdu qdepth threshold
	 * bit 1 - Update low priority msdu qdepth threshold
	 * bit 2 - Update UDP msdu qdepth threshold
	 * bit 3 - Update Non UDP msdu qdepth threshold
	 * rest of bits are reserved
	 */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_UPDATE_MASK,
	/* Unsigned 32-bit attribute for holding the threshold value */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_VALUE,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_LAST,
	QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_MAX =
		QCA_WLAN_VENDOR_ATTR_QDEPTH_THRESH_LAST - 1,
};

/**
 * qca_wlan_vendor_attr_external_acs_policy: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_POLICY to the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS. This represents the
 * external ACS policies to select the channels w.r.t. the PCL weights.
 * (QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_EVENT_PCL represents the channels and
 * their PCL weights.)
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_MANDATORY: Mandatory to
 * select a channel with non-zero PCL weight.
 * @QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_PREFERRED: Prefer a
 * channel with non-zero PCL weight.
 *
 */
enum qca_wlan_vendor_attr_external_acs_policy {
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_PREFERRED,
	QCA_WLAN_VENDOR_ATTR_EXTERNAL_ACS_POLICY_PCL_MANDATORY,
};

/**
 * enum qca_wlan_vendor_attr_spectral_scan - Spectral scan config parameters
 */
enum qca_wlan_vendor_attr_spectral_scan {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INVALID = 0,
	/*
	 * Number of times the chip enters spectral scan mode before
	 * deactivating spectral scans. When set to 0, chip will enter spectral
	 * scan mode continuously. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_COUNT = 1,
	/*
	 * Spectral scan period. Period increment resolution is 256*Tclk,
	 * where Tclk = 1/44 MHz (Gmode), 1/40 MHz (Amode). u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SCAN_PERIOD = 2,
	/* Spectral scan priority. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PRIORITY = 3,
	/* Number of FFT data points to compute. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_SIZE = 4,
	/*
	 * Enable targeted gain change before starting the spectral scan FFT.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_GC_ENA = 5,
	/* Restart a queued spectral scan. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RESTART_ENA = 6,
	/*
	 * Noise floor reference number for the calculation of bin power.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NOISE_FLOOR_REF = 7,
	/*
	 * Disallow spectral scan triggers after TX/RX packets by setting
	 * this delay value to roughly SIFS time period or greater.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_INIT_DELAY = 8,
	/*
	 * Number of strong bins (inclusive) per sub-channel, below
	 * which a signal is declared a narrow band tone. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_NB_TONE_THR = 9,
	/*
	 * Specify the threshold over which a bin is declared strong (for
	 * scan bandwidth analysis). u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_STR_BIN_THR = 10,
	/* Spectral scan report mode. u32 attribute. */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_WB_RPT_MODE = 11,
	/*
	 * RSSI report mode, if the ADC RSSI is below
	 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR,
	 * then FFTs will not trigger, but timestamps and summaries get
	 * reported. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_RPT_MODE = 12,
	/*
	 * ADC RSSI must be greater than or equal to this threshold (signed dB)
	 * to ensure spectral scan reporting with normal error code.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RSSI_THR = 13,
	/*
	 * Format of frequency bin magnitude for spectral scan triggered FFTs:
	 * 0: linear magnitude, 1: log magnitude (20*log10(lin_mag)).
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_PWR_FORMAT = 14,
	/*
	 * Format of FFT report to software for spectral scan triggered FFTs.
	 * 0: No FFT report (only spectral scan summary report)
	 * 1: 2-dword summary of metrics for each completed FFT + spectral scan
	 * report
	 * 2: 2-dword summary of metrics for each completed FFT + 1x-oversampled
	 * bins (in-band) per FFT + spectral scan summary report
	 * 3: 2-dword summary of metrics for each completed FFT + 2x-oversampled
	 * bins (all) per FFT + spectral scan summary report
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_RPT_MODE = 15,
	/*
	 * Number of LSBs to shift out in order to scale the FFT bins.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_BIN_SCALE = 16,
	/*
	 * Set to 1 (with spectral_scan_pwr_format=1), to report bin magnitudes
	 * in dBm power. u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DBM_ADJ = 17,
	/*
	 * Per chain enable mask to select input ADC for search FFT.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_CHN_MASK = 18,
	/*
	 * An unsigned 64-bit integer provided by host driver to identify the
	 * spectral scan request. This attribute is included in the scan
	 * response message for @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START
	 * and used as an attribute in
	 * @QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_STOP to identify the
	 * specific scan to be stopped.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_COOKIE = 19,
	/* Skip interval for FFT reports. u32 attribute */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_FFT_PERIOD = 20,
	/* Set to report only one set of FFT results.
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_SHORT_REPORT = 21,
	/* Debug level for spectral module in driver.
	 * 0 : Verbosity level 0
	 * 1 : Verbosity level 1
	 * 2 : Verbosity level 2
	 * 3 : Matched filterID display
	 * 4 : One time dump of FFT report
	 * u32 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_DEBUG_LEVEL = 22,
	/* Type of spectral scan request. u32 attribute.
	 * It uses values defined in enum
	 * qca_wlan_vendor_attr_spectral_scan_request_type.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE = 23,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_MAX =
		QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CONFIG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_spectral_diag_stats - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_DIAG_STATS.
 */
enum qca_wlan_vendor_attr_spectral_diag_stats {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_INVALID = 0,
	/* Number of spectral TLV signature mismatches.
	 * u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_SIG_MISMATCH = 1,
	/* Number of spectral phyerror events with insufficient length when
	 * parsing for secondary 80 search FFT report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_SEC80_SFFT_INSUFFLEN = 2,
	/* Number of spectral phyerror events without secondary 80
	 * search FFT report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_NOSEC80_SFFT = 3,
	/* Number of spectral phyerror events with vht operation segment 1 id
	 * mismatches in search fft report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_VHTSEG1ID_MISMATCH = 4,
	/* Number of spectral phyerror events with vht operation segment 2 id
	 * mismatches in search fft report. u64 attribute.
	 */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_VHTSEG2ID_MISMATCH = 5,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_MAX =
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_DIAG_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_spectral_cap - Used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_CAP_INFO.
 */
enum qca_wlan_vendor_attr_spectral_cap {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_INVALID = 0,
	/* Flag attribute to indicate phydiag capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_PHYDIAG = 1,
	/* Flag attribute to indicate radar detection capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_RADAR = 2,
	/* Flag attribute to indicate spectral capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_SPECTRAL = 3,
	/* Flag attribute to indicate advanced spectral capability */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_ADVANCED_SPECTRAL = 4,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_MAX =
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_CAP_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_spectral_scan_status - used by the vendor command
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_GET_STATUS.
 */
enum qca_wlan_vendor_attr_spectral_scan_status {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_INVALID = 0,
	/* Flag attribute to indicate whether spectral scan is enabled */
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_IS_ENABLED = 1,
	/* Flag attribute to indicate whether spectral scan is in progress*/
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_IS_ACTIVE = 2,

	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_MAX =
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_STATUS_AFTER_LAST - 1,
};

/**
 * enum qca_wlan_vendor_attr_config_latency_level - Level for
 * wlan latency module.
 *
 * There will be various of Wi-Fi functionality like scan/roaming/adaptive
 * power saving which would causing data exchange out of service, this
 * would be a big impact on latency. For latency sensitive applications over
 * Wi-Fi are intolerant to such operations and thus would configure them
 * to meet their respective needs. It is well understood by such applications
 * that altering the default behavior would degrade the Wi-Fi functionality
 * w.r.t the above pointed WLAN operations.
 *
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_NORMAL:
 *      Default WLAN operation level which throughput orientated.
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MODERATE:
 *      Use moderate level to improve latency by limit scan duration.
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_LOW:
 *      Use low latency level to benifit application like concurrent
 *      downloading or video streaming via constraint scan/adaptive PS.
 * @QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_ULTRALOW:
 *      Use ultra low latency level to benefit for gaming/voice
 *      application via constraint scan/roaming/adaptive PS.
 */
enum qca_wlan_vendor_attr_config_latency_level {
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_INVALID = 0,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_NORMAL = 1,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MODERATE = 2,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_LOW = 3,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_ULTRALOW = 4,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_MAX =
	QCA_WLAN_VENDOR_ATTR_CONFIG_LATENCY_LEVEL_AFTER_LAST - 1,
};

/**
 * qca_wlan_vendor_attr_spectral_scan_request_type: Attribute values for
 * QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE to the vendor subcmd
 * QCA_NL80211_VENDOR_SUBCMD_SPECTRAL_SCAN_START. This represents the
 * spectral scan request types.
 * @QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN_AND_CONFIG: Request to
 * set the spectral parameters and start scan.
 * @QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN: Request to
 * only set the spectral parameters.
 * @QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_CONFIG: Request to
 * only start the spectral scan.
 */
enum qca_wlan_vendor_attr_spectral_scan_request_type {
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN_AND_CONFIG,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_SCAN,
	QCA_WLAN_VENDOR_ATTR_SPECTRAL_SCAN_REQUEST_TYPE_CONFIG,
};

/**
 * enum qca_wlan_vendor_attr_rropavail_info - Specifies whether Representative
 * RF Operating Parameter (RROP) information is available, and if so, at which
 * point in the application-driver interaction sequence it can be retrieved by
 * the application from the driver. This point may vary by architecture and
 * other factors. This is a u16 value.
 */
enum qca_wlan_vendor_attr_rropavail_info {
	/* RROP information is unavailable. */
	QCA_WLAN_VENDOR_ATTR_RROPAVAIL_INFO_UNAVAILABLE,
	/* RROP information is available and the application can retrieve the
	 * information after receiving an QCA_NL80211_VENDOR_SUBCMD_EXTERNAL_ACS
	 * event from the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_RROPAVAIL_INFO_EXTERNAL_ACS_START,
	/* RROP information is available only after a vendor specific scan
	 * (requested using QCA_NL80211_VENDOR_SUBCMD_TRIGGER_SCAN) has
	 * successfully completed. The application can retrieve the information
	 * after receiving the QCA_NL80211_VENDOR_SUBCMD_SCAN_DONE event from
	 * the driver.
	 */
	QCA_WLAN_VENDOR_ATTR_RROPAVAIL_INFO_VSCAN_END,
};

/**
 * enum qca_wlan_vendor_attr_rrop_info - Specifies vendor specific
 * Representative RF Operating Parameter (RROP) information. It is sent for the
 * vendor command QCA_NL80211_VENDOR_SUBCMD_GET_RROP_INFO. This information is
 * intended for use by external Auto Channel Selection applications. It provides
 * guidance values for some RF parameters that are used by the system during
 * operation. These values could vary by channel, band, radio, and so on.
 */
enum qca_wlan_vendor_attr_rrop_info {
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_INVALID = 0,

	/* Representative Tx Power List (RTPL) which has an array of nested
	 * values as per attributes in enum qca_wlan_vendor_attr_rtplinst.
	 */
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_RTPL = 1,

	QCA_WLAN_VENDOR_ATTR_RROP_INFO_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_MAX =
	QCA_WLAN_VENDOR_ATTR_RROP_INFO_AFTER_LAST - 1
};

/**
 * enum qca_wlan_vendor_attr_rtplinst - Specifies attributes for individual list
 * entry instances in the Representative Tx Power List (RTPL). It provides
 * simplified power values intended for helping external Auto channel Selection
 * applications compare potential Tx power performance between channels, other
 * operating conditions remaining identical. These values are not necessarily
 * the actual Tx power values that will be used by the system. They are also not
 * necessarily the max or average values that will be used. Instead, they are
 * relative, summarized keys for algorithmic use computed by the driver or
 * underlying firmware considering a number of vendor specific factors.
 */
enum qca_wlan_vendor_attr_rtplinst {
	QCA_WLAN_VENDOR_ATTR_RTPLINST_INVALID = 0,

	/* Primary channel number (u8) */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_PRIMARY = 1,
	/* Representative Tx power in dBm (s32) with emphasis on throughput. */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_TXPOWER_THROUGHPUT = 2,
	/* Representative Tx power in dBm (s32) with emphasis on range. */
	QCA_WLAN_VENDOR_ATTR_RTPLINST_TXPOWER_RANGE = 3,

	QCA_WLAN_VENDOR_ATTR_RTPLINST_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_RTPLINST_MAX =
		QCA_WLAN_VENDOR_ATTR_RTPLINST_AFTER_LAST - 1,
};

#if !(defined (SUPPORT_WDEV_CFG80211_VENDOR_EVENT_ALLOC)) &&	\
	(LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)) && 	\
	!(defined(WITH_BACKPORTS))

static inline struct sk_buff *
backported_cfg80211_vendor_event_alloc(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					int approxlen,
					int event_idx, gfp_t gfp)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_event_alloc(wiphy, approxlen, event_idx, gfp);

	if (skb && wdev) {
		struct nlattr *attr;
		u32 ifindex = wdev->netdev->ifindex;

		nla_nest_cancel(skb, ((void **)skb->cb)[2]);
		if (nla_put_u32(skb, NL80211_ATTR_IFINDEX, ifindex))
			goto nla_fail;

		attr = nla_nest_start(skb, NL80211_ATTR_VENDOR_DATA);
		((void **)skb->cb)[2] = attr;
	}

	return skb;

nla_fail:
	kfree_skb(skb);

	return NULL;
}
#define cfg80211_vendor_event_alloc backported_cfg80211_vendor_event_alloc
#endif

/**
 * enum he_fragmentation_val - HE fragmentation support values
 * Indicates level of dynamic fragmentation that is supported by
 * a STA as a recipient.
 * HE fragmentation values are defined as per 11ax spec and are used in
 * HE capability IE to advertise the support. These values are validated
 * in the driver to check the device capability and advertised in the HE
 * capability element.
 *
 * @HE_FRAG_DISABLE: no support for dynamic fragmentation
 * @HE_FRAG_LEVEL1: support for dynamic fragments that are
 *	contained within an MPDU or S-MPDU, no support for dynamic fragments
 *	within an A-MPDU that is not an S-MPDU.
 * @HE_FRAG_LEVEL2: support for dynamic fragments that are
 *	contained within an MPDU or S-MPDU and support for up to one dynamic
 *	fragment for each MSDU, each A-MSDU if supported by the recipient, and
 *	each MMPDU within an A-MPDU or multi-TID A-MPDU that is not an
 *	MPDU or S-MPDU.
 * @HE_FRAG_LEVEL3: support for dynamic fragments that are
 *	contained within an MPDU or S-MPDU and support for multiple dynamic
 *	fragments for each MSDU and for each A-MSDU if supported by the
 *	recipient within an A-MPDU or multi-TID AMPDU and up to one dynamic
 *	fragment for each MMPDU in a multi-TID A-MPDU that is not an S-MPDU.
 */
enum he_fragmentation_val {
	HE_FRAG_DISABLE,
	HE_FRAG_LEVEL1,
	HE_FRAG_LEVEL2,
	HE_FRAG_LEVEL3,
};

/**
 * enum he_mcs_config - HE MCS support configuration
 *
 * Configures the HE Tx/Rx MCS map in HE capability IE for given bandwidth.
 * These values are used in driver to configure the HE MCS map to advertise
 * Tx/Rx MCS map in HE capability and these values are applied for all the
 * streams supported by the device. To configure MCS for different bandwidths,
 * vendor command needs to be sent using this attribute with appropriate value.
 * For example, to configure HE_80_MCS_0_7, send vendor command using HE MCS
 * attribute with QCA_WLAN_VENDOR_ATTR_HE_80_MCS0_7. And to configure HE MCS
 * for HE_160_MCS0_11 send this command using HE MCS config attribute with
 * value QCA_WLAN_VENDOR_ATTR_HE_160_MCS0_11;
 *
 * @HE_80_MCS0_7: support for HE 80/40/20MHz MCS 0 to 7
 * @HE_80_MCS0_9: support for HE 80/40/20MHz MCS 0 to 9
 * @HE_80_MCS0_11: support for HE 80/40/20MHz MCS 0 to 11
 * @HE_160_MCS0_7: support for HE 160MHz MCS 0 to 7
 * @HE_160_MCS0_9: support for HE 160MHz MCS 0 to 9
 * @HE_160_MCS0_11: support for HE 160MHz MCS 0 to 11
 * @HE_80p80_MCS0_7: support for HE 80p80MHz MCS 0 to 7
 * @HE_80p80_MCS0_9: support for HE 80p80MHz MCS 0 to 9
 * @HE_80p80_MCS0_11: support for HE 80p80MHz MCS 0 to 11
 */
enum he_mcs_config {
	HE_80_MCS0_7 = 0,
	HE_80_MCS0_9 = 1,
	HE_80_MCS0_11 = 2,
	HE_160_MCS0_7 = 4,
	HE_160_MCS0_9 = 5,
	HE_160_MCS0_11 = 6,
	HE_80p80_MCS0_7 = 8,
	HE_80p80_MCS0_9 = 9,
	HE_80p80_MCS0_11 = 10,
};

/* Attributes for data used by
 * QCA_NL80211_VENDOR_SUBCMD_WIFI_TEST_CONFIGURATION
 */
enum qca_wlan_vendor_attr_wifi_test_config {
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_INVALID = 0,
	/* 8-bit unsigned value to configure the driver to enable/disable
	 * WMM feature. This attribute is used to configure testbed device.
	 * 1-enable, 0-disable
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_WMM_ENABLE = 1,

	/* 8-bit unsigned value to configure the driver to accept/reject
	 * the addba request from peer. This attribute is used to configure
	 * the testbed device.
	 * 1-accept addba, 0-reject addba
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_ACCEPT_ADDBA_REQ = 2,

	/* 8-bit unsigned value to configure the driver to send or not to
	 * send the addba request to peer.
	 * This attribute is used to configure the testbed device.
	 * 1-send addba, 0-do not send addba
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_SEND_ADDBA_REQ = 3,

	/* 8-bit unsigned value to indicate the HE fragmentation support.
	 * Uses enum he_fragmentation_val values
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_FRAGMENTATION = 4,

	/* 8-bit unsigned value to indicate the HE MCS support.
	 * Uses he_mcs_config enum values
	 * This attribute is used to configure the testbed device.
	 */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_HE_MCS = 5,

	/* keep last */
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_AFTER_LAST,
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_MAX =
	QCA_WLAN_VENDOR_ATTR_WIFI_TEST_CONFIG_AFTER_LAST - 1,
};

#endif

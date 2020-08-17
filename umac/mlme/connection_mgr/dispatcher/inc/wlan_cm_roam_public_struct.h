/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains definitions for MLME roaming offload.
 */

#ifndef CM_ROAM_PUBLIC_STRUCT_H__
#define CM_ROAM_PUBLIC_STRUCT_H__

#include "wlan_objmgr_cmn.h"
#include "reg_services_public_struct.h"
#include "wlan_cm_bss_score_param.h"
#include "wlan_blm_public_struct.h"
#include "wmi_unified_param.h"
#include "wmi_unified_sta_param.h"

#define ROAM_SCAN_OFFLOAD_START                     1
#define ROAM_SCAN_OFFLOAD_STOP                      2
#define ROAM_SCAN_OFFLOAD_RESTART                   3
#define ROAM_SCAN_OFFLOAD_UPDATE_CFG                4
#define ROAM_SCAN_OFFLOAD_ABORT_SCAN                5

#define REASON_CONNECT                              1
#define REASON_CHANNEL_LIST_CHANGED                 2
#define REASON_LOOKUP_THRESH_CHANGED                3
#define REASON_DISCONNECTED                         4
#define REASON_RSSI_DIFF_CHANGED                    5
#define REASON_ESE_INI_CFG_CHANGED                  6
#define REASON_NEIGHBOR_SCAN_REFRESH_PERIOD_CHANGED 7
#define REASON_VALID_CHANNEL_LIST_CHANGED           8
#define REASON_FLUSH_CHANNEL_LIST                   9
#define REASON_EMPTY_SCAN_REF_PERIOD_CHANGED        10
#define REASON_PREAUTH_FAILED_FOR_ALL               11
#define REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW     12
#define REASON_NPROBES_CHANGED                      13
#define REASON_HOME_AWAY_TIME_CHANGED               14
#define REASON_OS_REQUESTED_ROAMING_NOW             15
#define REASON_SCAN_CH_TIME_CHANGED                 16
#define REASON_SCAN_HOME_TIME_CHANGED               17
#define REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED    18
#define REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED        19
#define REASON_ROAM_BMISS_FIRST_BCNT_CHANGED        20
#define REASON_ROAM_BMISS_FINAL_BCNT_CHANGED        21
#define REASON_ROAM_BEACON_RSSI_WEIGHT_CHANGED      22
#define REASON_ROAM_DFS_SCAN_MODE_CHANGED           23
#define REASON_ROAM_ABORT_ROAM_SCAN                 24
#define REASON_ROAM_EXT_SCAN_PARAMS_CHANGED         25
#define REASON_ROAM_SET_SSID_ALLOWED                26
#define REASON_ROAM_SET_FAVORED_BSSID               27
#define REASON_ROAM_GOOD_RSSI_CHANGED               28
#define REASON_ROAM_SET_BLACKLIST_BSSID             29
#define REASON_ROAM_SCAN_HI_RSSI_MAXCOUNT_CHANGED   30
#define REASON_ROAM_SCAN_HI_RSSI_DELTA_CHANGED      31
#define REASON_ROAM_SCAN_HI_RSSI_DELAY_CHANGED      32
#define REASON_ROAM_SCAN_HI_RSSI_UB_CHANGED         33
#define REASON_CONNECT_IES_CHANGED                  34
#define REASON_ROAM_SCAN_STA_ROAM_POLICY_CHANGED    35
#define REASON_ROAM_SYNCH_FAILED                    36
#define REASON_ROAM_PSK_PMK_CHANGED                 37
#define REASON_ROAM_STOP_ALL                        38
#define REASON_SUPPLICANT_DISABLED_ROAMING          39
#define REASON_CTX_INIT                             40
#define REASON_FILS_PARAMS_CHANGED                  41
#define REASON_SME_ISSUED                           42
#define REASON_DRIVER_ENABLED                       43
#define REASON_ROAM_FULL_SCAN_PERIOD_CHANGED        44
#define REASON_SCORING_CRITERIA_CHANGED             45
#define REASON_SUPPLICANT_INIT_ROAMING              46
#define REASON_SUPPLICANT_DE_INIT_ROAMING           47
#define REASON_DRIVER_DISABLED                      48
#define REASON_ROAM_CONTROL_CONFIG_CHANGED          49
#define REASON_ROAM_CONTROL_CONFIG_ENABLED          50
#define REASON_ROAM_CANDIDATE_FOUND                 51
#define REASON_ROAM_HANDOFF_DONE                    52
#define REASON_ROAM_ABORT                           53

/**
 * enum roam_offload_init_flags  - Flags sent in Roam offload initialization.
 * @WLAN_ROAM_FW_OFFLOAD_ENABLE: Init roaming module at firwmare
 * @WLAN_ROAM_BMISS_FINAL_SCAN_ENABLE: Enable partial scan after final beacon
 * miss event at firmware
 * @WLAN_ROAM_SKIP_EAPOL_4WAY_HANDSHAKE: Disable 4 Way-HS offload to firmware
 * Setting this flag will make the eapol packets reach to host every time
 * and can cause frequent APPS wake-ups.
 * @WLAN_ROAM_BMISS_FINAL_SCAN_TYPE: Set this flag to skip full scan on final
 * bmiss and use the channel map to do the partial scan alone
 */
enum roam_offload_init_flags {
	WLAN_ROAM_FW_OFFLOAD_ENABLE = BIT(1),
	WLAN_ROAM_BMISS_FINAL_SCAN_ENABLE = BIT(2),
	WLAN_ROAM_SKIP_EAPOL_4WAY_HANDSHAKE = BIT(3),
	WLAN_ROAM_BMISS_FINAL_SCAN_TYPE = BIT(4)
};

/**
 * struct wlan_roam_offload_init_params - Firmware roam module initialization
 * parameters. Used to fill
 * @vdev_id: vdev for which the roaming has to be enabled/disabled
 * @roam_offload_flag:  flag to init/deinit roam module
 */
struct wlan_roam_offload_init_params {
	uint8_t vdev_id;
	uint32_t roam_offload_flag;
};

/**
 * struct wlan_cm_roam_vendor_btm_params - vendor config roam control param
 * @scan_freq_scheme: scan frequency scheme from enum
 * qca_roam_scan_freq_scheme
 * @connected_rssi_threshold: RSSI threshold of the current
 * connected AP
 * @candidate_rssi_threshold: RSSI threshold of the
 * candidate AP
 * @user_roam_reason: Roam triggered reason code, value zero is for enable
 * and non zero value is disable
 */
struct wlan_cm_roam_vendor_btm_params {
	uint32_t scan_freq_scheme;
	uint32_t connected_rssi_threshold;
	uint32_t candidate_rssi_threshold;
	uint32_t user_roam_reason;
};

/**
 * struct wlan_roam_triggers - vendor configured roam triggers
 * @vdev_id: vdev id
 * @trigger_bitmap: vendor configured roam trigger bitmap as
 *		    defined @enum roam_control_trigger_reason
 * @roam_score_delta: Value of roam score delta
 * percentage to trigger roam
 * @control_param: roam trigger param
 */
struct wlan_roam_triggers {
	uint32_t vdev_id;
	uint32_t trigger_bitmap;
	uint32_t roam_score_delta;
	struct wlan_cm_roam_vendor_btm_params vendor_btm_param;
};

/**
 * struct ap_profile - Structure ap profile to match candidate
 * @flags: flags
 * @rssi_threshold: the value of the the candidate AP should higher by this
 *                  threshold than the rssi of the currrently associated AP
 * @ssid: ssid vlaue to be matched
 * @rsn_authmode: security params to be matched
 * @rsn_ucastcipherset: unicast cipher set
 * @rsn_mcastcipherset: mcast/group cipher set
 * @rsn_mcastmgmtcipherset: mcast/group management frames cipher set
 * @rssi_abs_thresh: the value of the candidate AP should higher than this
 *                   absolute RSSI threshold. Zero means no absolute minimum
 *                   RSSI is required. units are the offset from the noise
 *                   floor in dB
 */
struct ap_profile {
	uint32_t flags;
	uint32_t rssi_threshold;
	struct wlan_ssid  ssid;
	uint32_t rsn_authmode;
	uint32_t rsn_ucastcipherset;
	uint32_t rsn_mcastcipherset;
	uint32_t rsn_mcastmgmtcipherset;
	uint32_t rssi_abs_thresh;
};

/**
 * struct scoring_param - scoring param to sortlist selected AP
 * @disable_bitmap: Each bit will be either allow(0)/disallow(1) to
 *                 considered the roam score param.
 * @rssi_weightage: RSSI weightage out of total score in %
 * @ht_weightage: HT weightage out of total score in %.
 * @vht_weightage: VHT weightage out of total score in %.
 * @he_weightaget: 11ax weightage out of total score in %.
 * @bw_weightage: Bandwidth weightage out of total score in %.
 * @band_weightage: Band(2G/5G) weightage out of total score in %.
 * @nss_weightage: NSS(1x1 / 2x2)weightage out of total score in %.
 * @esp_qbss_weightage: ESP/QBSS weightage out of total score in %.
 * @beamforming_weightage: Beamforming weightage out of total score in %.
 * @pcl_weightage: PCL weightage out of total score in %.
 * @oce_wan_weightage OCE WAN metrics weightage out of total score in %.
 * @oce_ap_tx_pwr_weightage: OCE AP TX power score in %
 * @oce_subnet_id_weightage: OCE subnet id score in %
 * @bw_index_score: channel BW scoring percentage information.
 *                 BITS 0-7   :- It contains scoring percentage of 20MHz   BW
 *                 BITS 8-15  :- It contains scoring percentage of 40MHz   BW
 *                 BITS 16-23 :- It contains scoring percentage of 80MHz   BW
 *                 BITS 24-31 :- It contains scoring percentage of 1600MHz BW
 *                 The value of each index must be 0-100
 * @band_index_score: band scording percentage information.
 *                   BITS 0-7   :- It contains scoring percentage of 2G
 *                   BITS 8-15  :- It contains scoring percentage of 5G
 *                   BITS 16-23 :- reserved
 *                   BITS 24-31 :- reserved
 *                   The value of each index must be 0-100
 * @nss_index_score: NSS scoring percentage information.
 *                  BITS 0-7   :- It contains scoring percentage of 1x1
 *                  BITS 8-15  :- It contains scoring percentage of 2x2
 *                  BITS 16-23 :- It contains scoring percentage of 3x3
 *                  BITS 24-31 :- It contains scoring percentage of 4x4
 *                  The value of each index must be 0-100
 * @roam_score_delta: delta value expected over the roam score of the candidate
 * ap over the roam score of the current ap
 * @roam_trigger_bitmap: bitmap of roam triggers on which roam_score_delta
 * will be applied
 * @vendor_roam_score_algorithm: Preferred algorithm for roam candidate
 * selection
 * @cand_min_roam_score_delta: candidate min roam score delta value
 * @rssi_scoring: RSSI scoring information.
 * @esp_qbss_scoring: ESP/QBSS scoring percentage information
 * @oce_wan_scoring: OCE WAN metrics percentage information
 */
struct scoring_param {
	uint32_t disable_bitmap;
	int32_t rssi_weightage;
	int32_t ht_weightage;
	int32_t vht_weightage;
	int32_t he_weightage;
	int32_t bw_weightage;
	int32_t band_weightage;
	int32_t nss_weightage;
	int32_t esp_qbss_weightage;
	int32_t beamforming_weightage;
	int32_t pcl_weightage;
	int32_t oce_wan_weightage;
	uint32_t oce_ap_tx_pwr_weightage;
	uint32_t oce_subnet_id_weightage;
	uint32_t bw_index_score;
	uint32_t band_index_score;
	uint32_t nss_index_score;
	uint32_t roam_score_delta;
	uint32_t roam_trigger_bitmap;
	uint32_t vendor_roam_score_algorithm;
	uint32_t cand_min_roam_score_delta;
	struct rssi_config_score rssi_scoring;
	struct per_slot_score esp_qbss_scoring;
	struct per_slot_score oce_wan_scoring;
};

/*
 * Currently roam score delta value and min rssi values are sent
 * for 2 triggers
 */
#define NUM_OF_ROAM_TRIGGERS 2
#define IDLE_ROAM_TRIGGER 0
#define BTM_ROAM_TRIGGER  1

#define DEAUTH_MIN_RSSI 0
#define BMISS_MIN_RSSI  1

/**
 * enum roam_trigger_reason - Reason for triggering roam
 * ROAM_TRIGGER_REASON_NONE: Roam trigger reason none
 * ROAM_TRIGGER_REASON_PER:  Roam triggered due to packet error
 * ROAM_TRIGGER_REASON_BMISS: Roam triggered due to beacon miss
 * ROAM_TRIGGER_REASON_LOW_RSSI: Roam triggered due to low RSSI of current
 * connected AP.
 * ROAM_TRIGGER_REASON_HIGH_RSSI: Roam triggered because sta is connected to
 * a AP in 2.4GHz band and a better 5GHz AP is available
 * ROAM_TRIGGER_REASON_PERIODIC: Roam triggered as better AP was found during
 * periodic roam scan.
 * ROAM_TRIGGER_REASON_MAWC: Motion Aided WiFi Connectivity triggered roam.
 * ROAM_TRIGGER_REASON_DENSE: Roaming triggered due to dense environment
 * detected.
 * ROAM_TRIGGER_REASON_BACKGROUND: Roam triggered due to current AP having
 * poor rssi and scan candidate found in scan results provided by other
 * scan clients.
 * ROAM_TRIGGER_REASON_FORCED: Forced roam trigger.
 * ROAM_TRIGGER_REASON_BTM: Roam triggered due to AP sent BTM query with
 * Disassoc imminent bit set.
 * ROAM_TRIGGER_REASON_UNIT_TEST: Roam triggered due to unit test command.
 * ROAM_TRIGGER_REASON_BSS_LOAD: Roam triggered due to high channel utilization
 * in the current connected channel
 * ROAM_TRIGGER_REASON_DEAUTH: Roam triggered due to deauth received from the
 * current connected AP.
 * ROAM_TRIGGER_REASON_IDLE: Roam triggered due to inactivity of the device.
 * ROAM_TRIGGER_REASON_STA_KICKOUT: Roam triggered due to sta kickout event.
 * ROAM_TRIGGER_REASON_ESS_RSSI: Roam triggered due to ess rssi
 * ROAM_TRIGGER_REASON_WTC_BTM: Roam triggered due to WTC BTM
 * ROAM_TRIGGER_REASON_MAX: Maximum number of roam triggers
 */
enum roam_trigger_reason {
	ROAM_TRIGGER_REASON_NONE = 0,
	ROAM_TRIGGER_REASON_PER,
	ROAM_TRIGGER_REASON_BMISS,
	ROAM_TRIGGER_REASON_LOW_RSSI,
	ROAM_TRIGGER_REASON_HIGH_RSSI,
	ROAM_TRIGGER_REASON_PERIODIC,
	ROAM_TRIGGER_REASON_MAWC,
	ROAM_TRIGGER_REASON_DENSE,
	ROAM_TRIGGER_REASON_BACKGROUND,
	ROAM_TRIGGER_REASON_FORCED,
	ROAM_TRIGGER_REASON_BTM,
	ROAM_TRIGGER_REASON_UNIT_TEST,
	ROAM_TRIGGER_REASON_BSS_LOAD,
	ROAM_TRIGGER_REASON_DEAUTH,
	ROAM_TRIGGER_REASON_IDLE,
	ROAM_TRIGGER_REASON_STA_KICKOUT,
	ROAM_TRIGGER_REASON_ESS_RSSI,
	ROAM_TRIGGER_REASON_WTC_BTM,
	ROAM_TRIGGER_REASON_MAX,
};

/**
 * struct roam_trigger_min_rssi - structure to hold minimum rssi value of
 * candidate APs for each roam trigger
 * @min_rssi: minimum RSSI of candidate AP for the trigger reason specified in
 * trigger_id
 * @trigger_reason: Roam trigger reason
 */
struct roam_trigger_min_rssi {
	int32_t  min_rssi;
	enum roam_trigger_reason trigger_reason;
};

/**
 * struct roam_trigger_score_delta - structure to hold roam score delta value of
 * candidate APs for each roam trigger
 * @roam_score_delta: delta value in score of the candidate AP for the roam
 * trigger mentioned in the trigger_id.
 * @trigger_reason: Roam trigger reason
 */
struct roam_trigger_score_delta {
	uint32_t roam_score_delta;
	enum roam_trigger_reason trigger_reason;
};

/**
 * struct ap_profile_params - ap profile params
 * @vdev_id: vdev id
 * @profile: ap profile to match candidate
 * @param: scoring params to short candidate
 * @min_rssi_params: Min RSSI values for different roam triggers
 * @score_delta_params: Roam score delta values for different triggers
 */
struct ap_profile_params {
	uint8_t vdev_id;
	struct ap_profile profile;
	struct scoring_param param;
	struct roam_trigger_min_rssi min_rssi_params[NUM_OF_ROAM_TRIGGERS];
	struct roam_trigger_score_delta score_delta_param[NUM_OF_ROAM_TRIGGERS];
};

/**
 * struct wlan_roam_mawc_params - Motion Aided wireless connectivity params
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: MAWC roaming feature enable/disable
 * @traffic_load_threshold: Traffic threshold in kBps for MAWC roaming
 * @best_ap_rssi_threshold: AP RSSI Threshold for MAWC roaming
 * @rssi_stationary_high_adjust: High RSSI adjustment value to suppress scan
 * @rssi_stationary_low_adjust: Low RSSI adjustment value to suppress scan
 */
struct wlan_roam_mawc_params {
	uint8_t vdev_id;
	bool enable;
	uint32_t traffic_load_threshold;
	uint32_t best_ap_rssi_threshold;
	uint8_t rssi_stationary_high_adjust;
	uint8_t rssi_stationary_low_adjust;
};

#define MAX_SSID_ALLOWED_LIST    4
#define MAX_BSSID_AVOID_LIST     16
#define MAX_BSSID_FAVORED      16

/**
 * struct roam_scan_filter_params - Structure holding roaming scan
 *                                  parameters
 * @op_bitmap: bitmap to determine reason of roaming
 * @vdev_id: vdev id
 * @num_bssid_black_list: The number of BSSID's that we should avoid
 *                        connecting to. It is like a blacklist of BSSID's.
 * @num_ssid_white_list: The number of SSID profiles that are in the
 *                       Whitelist. When roaming, we consider the BSSID's with
 *                       this SSID also for roaming apart from the connected
 *                       one's
 * @num_bssid_preferred_list: Number of BSSID's which have a preference over
 *                            others
 * @bssid_avoid_list: Blacklist SSID's
 * @ssid_allowed_list: Whitelist SSID's
 * @bssid_favored: Favorable BSSID's
 * @bssid_favored_factor: RSSI to be added to this BSSID to prefer it
 * @lca_disallow_config_present: LCA [Last Connected AP] disallow config
 *                               present
 * @disallow_duration: How long LCA AP will be disallowed before it can be a
 *                     roaming candidate again, in seconds
 * @rssi_channel_penalization: How much RSSI will be penalized if candidate(s)
 *                             are found in the same channel as disallowed
 *                             AP's, in units of db
 * @num_disallowed_aps: How many APs the target should maintain in its LCA
 *                      list
 * @delta_rssi: (dB units) when AB in RSSI blacklist improved by at least
 *              delta_rssi,it will be removed from blacklist
 *
 * This structure holds all the key parameters related to
 * initial connection and roaming connections.
 */

struct roam_scan_filter_params {
	uint32_t op_bitmap;
	uint8_t vdev_id;
	uint32_t num_bssid_black_list;
	uint32_t num_ssid_white_list;
	uint32_t num_bssid_preferred_list;
	struct qdf_mac_addr bssid_avoid_list[MAX_BSSID_AVOID_LIST];
	struct wlan_ssid ssid_allowed_list[MAX_SSID_ALLOWED_LIST];
	struct qdf_mac_addr bssid_favored[MAX_BSSID_FAVORED];
	uint8_t bssid_favored_factor[MAX_BSSID_FAVORED];
	uint8_t lca_disallow_config_present;
	uint32_t disallow_duration;
	uint32_t rssi_channel_penalization;
	uint32_t num_disallowed_aps;
	uint32_t num_rssi_rejection_ap;
	struct reject_ap_config_params
				rssi_rejection_ap[MAX_RSSI_AVOID_BSSID_LIST];
	uint32_t delta_rssi;
};

/**
 * struct wlan_roam_scan_filter_params - structure containing parameters for
 * roam scan offload filter
 * @reason: reason for changing roam state for the requested vdev id
 * @filter_params: roam scan filter parameters
 */
struct wlan_roam_scan_filter_params {
	uint8_t reason;
	struct roam_scan_filter_params filter_params;
};

/**
 * struct wlan_roam_btm_config - BSS Transition Management offload params
 * @vdev_id: VDEV on which the parameters should be applied
 * @btm_offload_config: BTM config
 * @btm_solicited_timeout: Timeout value for waiting BTM request
 * @btm_max_attempt_cnt: Maximum attempt for sending BTM query to ESS
 * @btm_sticky_time: Stick time after roaming to new AP by BTM
 * @disassoc_timer_threshold: threshold value till which the firmware can
 * wait before triggering the roam scan after receiving the disassoc iminent
 * @btm_query_bitmask: bitmask to btm query with candidate list
 * @btm_candidate_min_score: Minimum score of the AP to consider it as a
 * candidate if the roam trigger is BTM kickout.
 */
struct wlan_roam_btm_config {
	uint8_t vdev_id;
	uint32_t btm_offload_config;
	uint32_t btm_solicited_timeout;
	uint32_t btm_max_attempt_cnt;
	uint32_t btm_sticky_time;
	uint32_t disassoc_timer_threshold;
	uint32_t btm_query_bitmask;
	uint32_t btm_candidate_min_score;
};

/**
 * struct wlan_roam_neighbor_report_params -neighbour report params
 * @time_offset: time offset after 11k offload command to trigger a neighbor
 *	report request (in seconds)
 * @low_rssi_offset: Offset from rssi threshold to trigger a neighbor
 *	report request (in dBm)
 * @bmiss_count_trigger: Number of beacon miss events to trigger neighbor
 *	report request
 * @per_threshold_offset: offset from PER threshold to trigger neighbor
 *	report request (in %)
 * @neighbor_report_cache_timeout: timeout after which new trigger can enable
 *	sending of a neighbor report request (in seconds)
 * @max_neighbor_report_req_cap: max number of neighbor report requests that
 *	can be sent to the peer in the current session
 * @ssid: Current connect SSID info
 */
struct wlan_roam_neighbor_report_params {
	uint32_t time_offset;
	uint32_t low_rssi_offset;
	uint32_t bmiss_count_trigger;
	uint32_t per_threshold_offset;
	uint32_t neighbor_report_cache_timeout;
	uint32_t max_neighbor_report_req_cap;
	struct wlan_ssid ssid;
};

/**
 * struct wlan_roam_11k_offload_params - offload 11k features to FW
 * @vdev_id: vdev id
 * @offload_11k_bitmask: bitmask to specify offloaded features
 *	B0: Neighbor Report Request offload
 *	B1-B31: Reserved
 * @neighbor_report_params: neighbor report offload params
 */
struct wlan_roam_11k_offload_params {
	uint32_t vdev_id;
	uint32_t offload_11k_bitmask;
	struct wlan_roam_neighbor_report_params neighbor_report_params;
};

/**
 * struct wlan_roam_bss_load_config - BSS load trigger parameters
 * @vdev_id: VDEV on which the parameters should be applied
 * @bss_load_threshold: BSS load threshold after which roam scan should trigger
 * @bss_load_sample_time: Time duration in milliseconds for which the bss load
 * trigger needs to be enabled
 * @rssi_threshold_5ghz: RSSI threshold of the current connected AP below which
 * roam should be triggered if bss load threshold exceeds the configured value.
 * This value is applicable only when we are connected in 5GHz band.
 * @rssi_threshold_24ghz: RSSI threshold of the current connected AP below which
 * roam should be triggered if bss load threshold exceeds the configured value.
 * This value is applicable only when we are connected in 2.4GHz band.
 */
struct wlan_roam_bss_load_config {
	uint32_t vdev_id;
	uint32_t bss_load_threshold;
	uint32_t bss_load_sample_time;
	int32_t rssi_threshold_5ghz;
	int32_t rssi_threshold_24ghz;
};

/**
 * struct roam_disable_cfg - Firmware roam module disable parameters
 * @vdev_id: vdev for which the roaming has to be enabled/disabled
 * @cfg:  Config to enable/disable FW roam module
 */
struct roam_disable_cfg {
	uint8_t vdev_id;
	uint8_t cfg;
};

/**
 * struct wlan_roam_disconnect_params - Emergency deauth/disconnect roam params
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: Enable or disable disconnect roaming.
 */
struct wlan_roam_disconnect_params {
	uint32_t vdev_id;
	bool enable;
};

/**
 * struct wlan_roam_idle_params - Idle roam trigger parameters
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: Enable/Disable Idle roaming
 * @band: Connected AP band
 * @conn_ap_rssi_delta: Rssi change of connected AP in dBm
 * @conn_ap_min_rssi: If connected AP rssi is less than min rssi trigger roam
 * @inactive_time: Connected AP idle time
 * @data_pkt_count: Data packet count allowed during idle time
 */
struct wlan_roam_idle_params {
	uint32_t vdev_id;
	bool enable;
	uint32_t band;
	uint32_t conn_ap_rssi_delta;
	int32_t conn_ap_min_rssi;
	uint32_t inactive_time;
	uint32_t data_pkt_count;
};

/**
 * struct wlan_per_roam_config - per based roaming parameters
 * @enable: if PER based roaming is enabled/disabled
 * @tx_high_rate_thresh: high rate threshold at which PER based
 *     roam will stop in tx path
 * @rx_high_rate_thresh: high rate threshold at which PER based
 *     roam will stop in rx path
 * @tx_low_rate_thresh: rate below which traffic will be considered
 *     for PER based roaming in Tx path
 * @rx_low_rate_thresh: rate below which traffic will be considered
 *     for PER based roaming in Tx path
 * @tx_rate_thresh_percnt: % above which when traffic is below low_rate_thresh
 *     will be considered for PER based scan in tx path
 * @rx_rate_thresh_percnt: % above which when traffic is below low_rate_thresh
 *     will be considered for PER based scan in rx path
 * @per_rest_time: time for which PER based roam will wait once it
 *     issues a roam scan.
 * @tx_per_mon_time: Minimum time required to be considered as valid scenario
 *     for PER based roam in tx path
 * @rx_per_mon_time: Minimum time required to be considered as valid scenario
 *     for PER based roam in rx path
 * @min_candidate_rssi: Minimum RSSI threshold for candidate AP to be used for
 *     PER based roaming
 */
struct wlan_per_roam_config {
	uint32_t enable;
	uint32_t tx_high_rate_thresh;
	uint32_t rx_high_rate_thresh;
	uint32_t tx_low_rate_thresh;
	uint32_t rx_low_rate_thresh;
	uint32_t tx_rate_thresh_percnt;
	uint32_t rx_rate_thresh_percnt;
	uint32_t per_rest_time;
	uint32_t tx_per_mon_time;
	uint32_t rx_per_mon_time;
	uint32_t min_candidate_rssi;
};

/**
 * struct wlan_per_roam_config_req: PER based roaming config request
 * @vdev_id: vdev id on which config needs to be set
 * @per_config: PER config
 */
struct wlan_per_roam_config_req {
	uint8_t vdev_id;
	struct wlan_per_roam_config per_config;
};

#ifdef ROAM_OFFLOAD_V1
#define NOISE_FLOOR_DBM_DEFAULT          (-96)
#define RSSI_MIN_VALUE                   (-128)
#define RSSI_MAX_VALUE                   (127)

/**
 * struct wlan_roam_offload_scan_rssi_params - structure containing
 *              parameters for roam offload scan based on RSSI
 * @rssi_thresh: rssi threshold
 * @rssi_thresh_diff: difference in rssi threshold
 * @hi_rssi_scan_max_count: 5G scan max count
 * @hi_rssi_scan_rssi_delta: 5G scan rssi change threshold value
 * @hi_rssi_scan_rssi_ub: 5G scan upper bound
 * @raise_rssi_thresh_5g: flag to determine penalty and boost thresholds
 * @vdev_id: vdev id
 * @penalty_threshold_5g: RSSI threshold below which 5GHz RSSI is penalized
 * @boost_threshold_5g: RSSI threshold above which 5GHz RSSI is favored
 * @raise_factor_5g: factor by which 5GHz RSSI is boosted
 * @drop_factor_5g: factor by which 5GHz RSSI is penalized
 * @max_raise_rssi_5g: maximum boost that can be applied to a 5GHz RSSI
 * @max_drop_rssi_5g: maximum penalty that can be applied to a 5GHz RSSI
 * @good_rssi_threshold: RSSI below which roam is kicked in by background
 *                       scan although rssi is still good
 * @early_stop_scan_enable: early stop scan enable
 * @roam_earlystop_thres_min: Minimum RSSI threshold value for early stop,
 *                            unit is dB above NF
 * @roam_earlystop_thres_max: Maximum RSSI threshold value for early stop,
 *                            unit is dB above NF
 * @dense_rssi_thresh_offset: dense roam RSSI threshold difference
 * @dense_min_aps_cnt: dense roam minimum APs
 * @initial_dense_status: dense status detected by host
 * @traffic_threshold: dense roam RSSI threshold
 * @bg_scan_bad_rssi_thresh: Bad RSSI threshold to perform bg scan
 * @roam_bad_rssi_thresh_offset_2g: Offset from Bad RSSI threshold for 2G
 *                                  to 5G Roam
 * @bg_scan_client_bitmap: Bitmap used to identify the client scans to snoop
 */
struct wlan_roam_offload_scan_rssi_params {
	int8_t rssi_thresh;
	uint8_t rssi_thresh_diff;
	uint32_t hi_rssi_scan_max_count;
	uint32_t hi_rssi_scan_rssi_delta;
	int32_t hi_rssi_scan_rssi_ub;
	int raise_rssi_thresh_5g;
	int drop_rssi_thresh_5g;
	uint8_t vdev_id;
	uint32_t penalty_threshold_5g;
	uint32_t boost_threshold_5g;
	uint8_t raise_factor_5g;
	uint8_t drop_factor_5g;
	int max_raise_rssi_5g;
	int max_drop_rssi_5g;
	uint32_t good_rssi_threshold;
	bool early_stop_scan_enable;
	uint32_t roam_earlystop_thres_min;
	uint32_t roam_earlystop_thres_max;
	int dense_rssi_thresh_offset;
	int dense_min_aps_cnt;
	int initial_dense_status;
	int traffic_threshold;
	int32_t rssi_thresh_offset_5g;
	int8_t bg_scan_bad_rssi_thresh;
	uint8_t roam_bad_rssi_thresh_offset_2g;
	uint32_t bg_scan_client_bitmap;
};

/**
 * struct wlan_roam_beacon_miss_cnt - roam beacon miss count
 * @vdev_id: vdev id
 * @roam_bmiss_first_bcnt: First beacon miss count
 * @roam_bmiss_final_bcnt: Final beacon miss count
 */
struct wlan_roam_beacon_miss_cnt {
	uint32_t vdev_id;
	uint8_t roam_bmiss_first_bcnt;
	uint8_t roam_bmiss_final_bcnt;
};

/**
 * struct wlan_roam_reason_vsie_enable - roam reason vsie enable parameters
 * @vdev_id: vdev id
 * @enable_roam_reason_vsie: enable/disable inclusion of roam Reason
 * in Re(association) frame
 */
struct wlan_roam_reason_vsie_enable {
	uint32_t vdev_id;
	uint8_t enable_roam_reason_vsie;
};

/**
 * struct wlan_roam_scan_period_params - Roam scan period parameters
 * @vdev_id: Vdev for which the scan period parameters are sent
 * @empty_scan_refresh_period: empty scan refresh period
 * @scan_period: Opportunistic scan runs on a timer for scan_period
 * @scan_age: Duration after which the scan entries are to be aged out
 * @roam_scan_inactivity_time: inactivity monitoring time in ms for which the
 * device is considered to be inactive
 * @roam_inactive_data_packet_count: Maximum allowed data packets count during
 * roam_scan_inactivity_time.
 * @roam_scan_period_after_inactivity: Roam scan period in ms after device is
 * in inactive state.
 * @full_scan_period: Full scan period is the idle period in seconds
 * between two successive full channel roam scans.
 */
struct wlan_roam_scan_period_params {
	uint32_t vdev_id;
	uint32_t empty_scan_refresh_period;
	uint32_t scan_period;
	uint32_t scan_age;
	uint32_t roam_scan_inactivity_time;
	uint32_t roam_inactive_data_packet_count;
	uint32_t roam_scan_period_after_inactivity;
	uint32_t full_scan_period;
};

/**
 * struct wlan_roam_start_config - structure containing parameters for
 * roam start config
 * @rssi_params: roam scan rssi threshold parameters
 * @beacon_miss_cnt: roam beacon miss count parameters
 * @reason_vsie_enable: roam reason vsie enable parameters
 * @roam_triggers: roam triggers parameters
 * @scan_period_params: roam scan period parameters
 * @profile_params: ap profile parameters
 * @mawc_params: mawc parameters
 * @scan_filter_params: roam scan filter parameters
 * @btm_config: btm configuration
 * @roam_11k_params: 11k params
 * @bss_load_config: bss load config
 * @disconnect_params: disconnect params
 * @idle_params: idle params
 */
struct wlan_roam_start_config {
	struct wlan_roam_offload_scan_rssi_params rssi_params;
	struct wlan_roam_beacon_miss_cnt beacon_miss_cnt;
	struct wlan_roam_reason_vsie_enable reason_vsie_enable;
	struct wlan_roam_triggers roam_triggers;
	struct wlan_roam_scan_period_params scan_period_params;
	struct ap_profile_params profile_params;
	struct wlan_roam_mawc_params mawc_params;
	struct wlan_roam_scan_filter_params scan_filter_params;
	struct wlan_roam_btm_config btm_config;
	struct wlan_roam_11k_offload_params roam_11k_params;
	struct wlan_roam_bss_load_config bss_load_config;
	struct wlan_roam_disconnect_params disconnect_params;
	struct wlan_roam_idle_params idle_params;
	/* other wmi cmd structures */
};

/**
 * struct wlan_roam_stop_config - structure containing parameters for
 * roam stop
 * @reason: roaming reason
 * @middle_of_roaming: in the middle of roaming
 * @roam_11k_params: 11k params
 * @btm_config: btm configuration
 * @scan_filter_params: roam scan filter parameters
 * @disconnect_params: disconnect params
 * @idle_params: idle params
 * @roam_triggers: roam triggers parameters
 * @rssi_params: roam scan rssi threshold parameters
 */
struct wlan_roam_stop_config {
	uint8_t reason;
	uint8_t middle_of_roaming;
	struct wlan_roam_11k_offload_params roam_11k_params;
	struct wlan_roam_btm_config btm_config;
	struct wlan_roam_scan_filter_params scan_filter_params;
	struct wlan_roam_disconnect_params disconnect_params;
	struct wlan_roam_idle_params idle_params;
	struct wlan_roam_triggers roam_triggers;
	struct wlan_roam_offload_scan_rssi_params rssi_params;
};

/**
 * struct wlan_roam_update_config - structure containing parameters for
 * roam update config
 * @beacon_miss_cnt: roam beacon miss count parameters
 * @scan_filter_params: roam scan filter parameters
 * @scan_period_params: roam scan period parameters
 * @profile_params: ap profile parameters
 * @rssi_params: roam scan rssi threshold parameters
 * @disconnect_params: disconnect params
 * @idle_params: idle params
 * @roam_triggers: roam triggers parameters
 */
struct wlan_roam_update_config {
	struct wlan_roam_beacon_miss_cnt beacon_miss_cnt;
	struct wlan_roam_scan_filter_params scan_filter_params;
	struct wlan_roam_scan_period_params scan_period_params;
	struct ap_profile_params profile_params;
	struct wlan_roam_offload_scan_rssi_params rssi_params;
	struct wlan_roam_disconnect_params disconnect_params;
	struct wlan_roam_idle_params idle_params;
	struct wlan_roam_triggers roam_triggers;
};

#endif

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * enum roam_offload_state - Roaming module state for each STA vdev.
 * @WLAN_ROAM_DEINIT: Roaming module is not initialized at the
 *  firmware.
 * @WLAN_ROAM_INIT: Roaming module initialized at the firmware.
 * @WLAN_ROAM_RSO_ENABLED: RSO enabled, firmware can roam to different AP.
 * @WLAN_ROAM_RSO_STOPPED: RSO stopped - roaming module is initialized at
 * firmware, but firmware cannot do roaming due to supplicant disabled
 * roaming/driver disabled roaming.
 * @WLAN_ROAMING_IN_PROG: Roaming started at firmware. This state is
 * transitioned after candidate selection is done at fw and preauth to
 * the AP is started.
 * @WLAN_ROAM_SYNCH_IN_PROG: Roaming handoff complete
 */
enum roam_offload_state {
	WLAN_ROAM_DEINIT,
	WLAN_ROAM_INIT,
	WLAN_ROAM_RSO_ENABLED,
	WLAN_ROAM_RSO_STOPPED,
	WLAN_ROAMING_IN_PROG,
	WLAN_ROAM_SYNCH_IN_PROG,
};

/**
 *  struct roam_btm_response_data - BTM response related data
 *  @present:       Flag to check if the roam btm_rsp tlv is present
 *  @btm_status:    Btm request status
 *  @target_bssid:  AP MAC address
 *  @vsie_reason:   Vsie_reason value
 *  @timestamp:     This timestamp indicates the time when btm rsp is sent
 */
struct roam_btm_response_data {
	bool present;
	uint32_t btm_status;
	struct qdf_mac_addr target_bssid;
	uint32_t vsie_reason;
	uint32_t timestamp;
};

/**
 *  struct roam_initial_data - Roam initial related data
 *  @present:                Flag to check if the roam btm_rsp tlv is present
 *  @roam_full_scan_count:   Roam full scan count
 *  @rssi_th:                RSSI threhold
 *  @cu_th:                  Channel utilization threhold
 *  @fw_cancel_timer_bitmap: FW timers, which are getting cancelled
 */
struct roam_initial_data {
	bool present;
	uint32_t roam_full_scan_count;
	uint32_t rssi_th;
	uint32_t cu_th;
	uint32_t fw_cancel_timer_bitmap;
};

/**
 * enum wlan_cm_rso_control_requestor - Driver disabled roaming requestor that
 * will request the roam module to disable roaming based on the mlme operation
 * @RSO_INVALID_REQUESTOR: invalid requestor
 * @RSO_START_BSS: disable roaming temporarily due to start bss
 * @RSO_CHANNEL_SWITCH: disable roaming due to STA channel switch
 * @RSO_CONNECT_START: disable roaming temporarily due to connect
 * @RSO_SAP_CHANNEL_CHANGE: disable roaming due to SAP channel change
 * @RSO_NDP_CON_ON_NDI: disable roaming due to NDP connection on NDI
 * @RSO_SET_PCL: Disable roaming to set pcl to firmware
 */
enum wlan_cm_rso_control_requestor {
	RSO_INVALID_REQUESTOR,
	RSO_START_BSS          = BIT(0),
	RSO_CHANNEL_SWITCH     = BIT(1),
	RSO_CONNECT_START      = BIT(2),
	RSO_SAP_CHANNEL_CHANGE = BIT(3),
	RSO_NDP_CON_ON_NDI     = BIT(4),
	RSO_SET_PCL            = BIT(5),
};
#endif

/**
 * struct set_pcl_req - Request message to set the PCL
 * @vdev_id:   Vdev id
 * @band_mask: Supported band mask
 * @clear_vdev_pcl: Clear the configured vdev pcl channels
 * @chan_weights: PCL channel weights
 */
struct set_pcl_req {
	uint8_t vdev_id;
	uint32_t band_mask;
	bool clear_vdev_pcl;
	struct wmi_pcl_chan_weights chan_weights;
};

/**
 * wlan_cm_roam_tx_ops  - structure of tx function pointers for
 * roaming related commands
 * @send_vdev_set_pcl_cmd: TX ops function pointer to send set vdev PCL
 * command
 * @send_roam_offload_init_req: TX Ops function pointer to send roam offload
 * module initialize request
 * @send_roam_start_req: TX ops function pointer to send roam start related
 * commands
 * @send_roam_abort: send roam abort
 */
struct wlan_cm_roam_tx_ops {
	QDF_STATUS (*send_vdev_set_pcl_cmd)(struct wlan_objmgr_vdev *vdev,
					    struct set_pcl_req *req);
#ifdef ROAM_OFFLOAD_V1
	QDF_STATUS (*send_roam_offload_init_req)(
			struct wlan_objmgr_vdev *vdev,
			struct wlan_roam_offload_init_params *params);

	QDF_STATUS (*send_roam_start_req)(struct wlan_objmgr_vdev *vdev,
					  struct wlan_roam_start_config *req);
	QDF_STATUS (*send_roam_stop_offload)(struct wlan_objmgr_vdev *vdev,
					     struct wlan_roam_stop_config *req);
	QDF_STATUS (*send_roam_update_config)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_roam_update_config *req);
	QDF_STATUS (*send_roam_abort)(struct wlan_objmgr_vdev *vdev,
				      uint8_t vdev_id);
	QDF_STATUS (*send_roam_per_config)(
				struct wlan_objmgr_vdev *vdev,
				struct wlan_per_roam_config_req *req);
#endif
};

/**
 * enum roam_scan_freq_scheme - Scan mode for triggering roam
 * ROAM_SCAN_FREQ_SCHEME_NO_SCAN: Indicates the fw to not scan.
 * ROAM_SCAN_FREQ_SCHEME_PARTIAL_SCAN: Indicates the firmware to
 * trigger partial frequency scans.
 * ROAM_SCAN_FREQ_SCHEME_FULL_SCAN: Indicates the firmware to
 * trigger full frequency scans.
 */
enum roam_scan_freq_scheme {
	ROAM_SCAN_FREQ_SCHEME_NO_SCAN = 0,
	ROAM_SCAN_FREQ_SCHEME_PARTIAL_SCAN = 1,
	ROAM_SCAN_FREQ_SCHEME_FULL_SCAN = 2,
};

/**
 * struct wlan_cm_roam  - Connection manager roam configs, state and roam
 * data related structure
 * @tx_ops: Roam Tx ops to send roam offload commands to firmware
 * @pcl_vdev_cmd_active:  Flag to check if vdev level pcl command needs to be
 * sent or PDEV level PCL command needs to be sent
 * @control_param: vendor configured roam control param
 */
struct wlan_cm_roam {
	struct wlan_cm_roam_tx_ops tx_ops;
	bool pcl_vdev_cmd_active;
	struct wlan_cm_roam_vendor_btm_params vendor_btm_param;
};
#endif

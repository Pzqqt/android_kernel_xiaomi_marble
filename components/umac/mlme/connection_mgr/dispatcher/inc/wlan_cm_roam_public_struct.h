/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wlan_cm_public_struct.h"
#include "wmi_unified.h"

#define WLAN_ROAM_MAX_CACHED_AUTH_FRAMES            8

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
#define REASON_ROAM_SET_PRIMARY                     54

#define FILS_MAX_KEYNAME_NAI_LENGTH WLAN_CM_FILS_MAX_KEYNAME_NAI_LENGTH
#define WLAN_FILS_MAX_REALM_LEN WLAN_CM_FILS_MAX_REALM_LEN
#define WLAN_FILS_MAX_RRK_LENGTH WLAN_CM_FILS_MAX_RRK_LENGTH

#define FILS_MAX_HLP_DATA_LEN 2048

#define WLAN_FILS_MAX_RIK_LENGTH WLAN_FILS_MAX_RRK_LENGTH
#define WLAN_FILS_FT_MAX_LEN          48

#define WLAN_MAX_PMK_DUMP_BYTES 2
#define DEFAULT_ROAM_SCAN_SCHEME_BITMAP 0
#define ROAM_MAX_CFG_VALUE 0xffffffff

#define CFG_VALID_CHANNEL_LIST_LEN 100
#define MAX_SSID_ALLOWED_LIST    8
#define MAX_BSSID_AVOID_LIST     16
#define MAX_BSSID_FAVORED      16
#define WLAN_MAX_BTM_CANDIDATES      8

/* Default value of WTC reason code */
#define DISABLE_VENDOR_BTM_CONFIG 2

#ifdef WLAN_FEATURE_HOST_ROAM
#define MAX_FTIE_SIZE CM_MAX_FTIE_SIZE
#else
#define MAX_FTIE_SIZE 384
#endif

#define ESE_MAX_TSPEC_IES 4

/*
 * To get 4 LSB of roam reason of roam_synch_data
 * received from firmware
 */
#define ROAM_REASON_MASK 0x0F

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define ROAM_SCAN_PSK_SIZE    48
#define ROAM_R0KH_ID_MAX_LEN  48
/* connected but not authenticated */
#define ROAM_AUTH_STATUS_CONNECTED      0x1
/* connected and authenticated */
#define ROAM_AUTH_STATUS_AUTHENTICATED  0x2

#define IS_ROAM_REASON_STA_KICKOUT(reason) ((reason & 0xF) == \
	WMI_ROAM_TRIGGER_REASON_STA_KICKOUT)
#define IS_ROAM_REASON_DISCONNECTION(reason) ((reason & 0xF) == \
	WMI_ROAM_TRIGGER_REASON_DEAUTH)
#endif

/*
 * Neighbor Report Params Bitmask
 */
#define NEIGHBOR_REPORT_PARAMS_TIME_OFFSET            0x01
#define NEIGHBOR_REPORT_PARAMS_LOW_RSSI_OFFSET        0x02
#define NEIGHBOR_REPORT_PARAMS_BMISS_COUNT_TRIGGER    0x04
#define NEIGHBOR_REPORT_PARAMS_PER_THRESHOLD_OFFSET   0x08
#define NEIGHBOR_REPORT_PARAMS_CACHE_TIMEOUT          0x10
#define NEIGHBOR_REPORT_PARAMS_MAX_REQ_CAP            0x20
#define NEIGHBOR_REPORT_PARAMS_ALL                    0x3F

/*
 * Neighbor report offload needs to send 0xFFFFFFFF if a particular
 * parameter is disabled from the ini
 */
#define NEIGHBOR_REPORT_PARAM_INVALID (0xFFFFFFFFU)

/*
 * Currently roam score delta value is sent for 2 triggers and min rssi
 * values are sent for 3 triggers
 */
#define NUM_OF_ROAM_TRIGGERS 2
#define IDLE_ROAM_TRIGGER 0
#define BTM_ROAM_TRIGGER  1

#define NUM_OF_ROAM_MIN_RSSI 3
#define DEAUTH_MIN_RSSI 0
#define BMISS_MIN_RSSI  1
#define MIN_RSSI_2G_TO_5G_ROAM 2
#define CM_CFG_VALID_CHANNEL_LIST_LEN 100

/**
 * enum roam_trigger_sub_reason - Roam trigger sub reasons
 * @ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER: Roam scan triggered due to
 * periodic timer expiry
 * @ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER: Roam scan triggered due to
 * inactivity detection and connected AP RSSI falls below a certain threshold
 * @ROAM_TRIGGER_SUB_REASON_BTM_DI_TIMER: Roam scan triggered due to BTM
 * Disassoc Imminent timeout
 * @ROAM_TRIGGER_SUB_REASON_FULL_SCAN: Roam scan triggered due to partial scan
 * failure
 * @ROAM_TRIGGER_SUB_REASON_LOW_RSSI_PERIODIC: Roam scan triggered due to Low
 * rssi periodic timer
 * @ROAM_TRIGGER_SUB_REASON_CU_PERIODIC: Roam scan triggered due to CU periodic
 * timer
 * @ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY: Roam scan
 * triggered due to periodic timer after device inactivity after low rssi
 * trigger
 * @ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY_CU: Roam scan
 * triggered due to first periodic timer exiry when full scan count is not 0
 * and roam scan trigger is CU load
 * @ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER_CU: Roam scan triggered due to
 * first periodic timer exiry when full scan count is 0 and roam scan trigger
 * is CU load
 */
enum roam_trigger_sub_reason {
	ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER = 1,
	ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER_LOW_RSSI,
	ROAM_TRIGGER_SUB_REASON_BTM_DI_TIMER,
	ROAM_TRIGGER_SUB_REASON_FULL_SCAN,
	ROAM_TRIGGER_SUB_REASON_LOW_RSSI_PERIODIC,
	ROAM_TRIGGER_SUB_REASON_CU_PERIODIC,
	ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY,
	ROAM_TRIGGER_SUB_REASON_PERIODIC_TIMER_AFTER_INACTIVITY_CU,
	ROAM_TRIGGER_SUB_REASON_INACTIVITY_TIMER_CU,
};

/**
 * enum wlan_roam_frame_subtype - Roam frame subtypes
 * @ROAM_FRAME_SUBTYPE_M1: EAPOL M1 Frame
 * @ROAM_FRAME_SUBTYPE_M2: EAPOL M2 Frame
 * @ROAM_FRAME_SUBTYPE_M3: EAPOL M3 Frame
 * @ROAM_FRAME_SUBTYPE_M4: EAPOL M4 Frame
 * @ROAM_FRAME_SUBTYPE_GTK_M1: GTK M1 Frame
 * @ROAM_FRAME_SUBTYPE_GTK_M2: GTK M2 Frame
 */
enum wlan_roam_frame_subtype {
	ROAM_FRAME_SUBTYPE_M1 = 1,
	ROAM_FRAME_SUBTYPE_M2,
	ROAM_FRAME_SUBTYPE_M3,
	ROAM_FRAME_SUBTYPE_M4,
	ROAM_FRAME_SUBTYPE_GTK_M1,
	ROAM_FRAME_SUBTYPE_GTK_M2,
};

/**
 * struct cm_roam_neighbor_report_offload_params - neighbor report offload
 *                                                 parameters
 * @offload_11k_enable_bitmask: neighbor report offload bitmask control
 * @params_bitmask: bitmask to specify which of the below are enabled
 * @time_offset: time offset after 11k offload command to trigger a neighbor
 *              report request (in seconds)
 * @low_rssi_offset: Offset from rssi threshold to trigger neighbor
 *      report request (in dBm)
 * @bmiss_count_trigger: Number of beacon miss events to trigger neighbor
 *              report request
 * @per_threshold_offset: offset from PER threshold to trigger neighbor
 *              report request (in %)
 * @neighbor_report_cache_timeout: timeout after which new trigger can enable
 *              sending of a neighbor report request (in seconds)
 * @max_neighbor_report_req_cap: max number of neighbor report requests that
 *              can be sent to the peer in the current session
 */
struct cm_roam_neighbor_report_offload_params {
	uint32_t offload_11k_enable_bitmask;
	uint8_t params_bitmask;
	uint32_t time_offset;
	uint32_t low_rssi_offset;
	uint32_t bmiss_count_trigger;
	uint32_t per_threshold_offset;
	uint32_t neighbor_report_cache_timeout;
	uint32_t max_neighbor_report_req_cap;
};

/**
 * struct rso_chan_info - chan info
 * @num_chan: number of channels
 * @freq_list: freq list
 */
struct rso_chan_info {
	uint8_t num_chan;
	qdf_freq_t *freq_list;
};

/**
 * struct rso_cfg_params - per vdev rso cfg
 */
struct rso_cfg_params {
	uint32_t neighbor_scan_period;
	uint32_t neighbor_scan_min_period;
	struct rso_chan_info specific_chan_info;
	uint8_t neighbor_lookup_threshold;
	int8_t rssi_thresh_offset_5g;
	uint32_t min_chan_scan_time;
	uint32_t max_chan_scan_time;
	uint16_t neighbor_results_refresh_period;
	uint16_t empty_scan_refresh_period;
	uint8_t opportunistic_threshold_diff;
	uint8_t roam_rescan_rssi_diff;
	uint8_t roam_bmiss_first_bcn_cnt;
	uint8_t roam_bmiss_final_cnt;
	uint32_t hi_rssi_scan_max_count;
	uint32_t hi_rssi_scan_rssi_delta;
	uint32_t hi_rssi_scan_delay;
	int32_t hi_rssi_scan_rssi_ub;
	struct rso_chan_info pref_chan_info;
	uint32_t full_roam_scan_period;
	bool enable_scoring_for_roam;
	uint8_t roam_rssi_diff;
	uint8_t roam_rssi_diff_6ghz;
	uint8_t bg_rssi_threshold;
	uint16_t roam_scan_home_away_time;
	uint8_t roam_scan_n_probes;
	uint32_t roam_scan_inactivity_time;
	uint32_t roam_inactive_data_packet_count;
	uint32_t roam_scan_period_after_inactivity;
};

/**
 * struct wlan_chan_list - channel list
 * @num_chan: number of channels
 * @freq_list: freq list
 */
struct wlan_chan_list {
	uint8_t num_chan;
	qdf_freq_t freq_list[CFG_VALID_CHANNEL_LIST_LEN];
};

/*
 * roam_fail_params: different types of params to set or get roam fail states
 * for the vdev
 * @ROAM_TRIGGER_REASON: Roam trigger reason(enum WMI_ROAM_TRIGGER_REASON_ID)
 * @ROAM_INVOKE_FAIL_REASON: One of WMI_ROAM_FAIL_REASON_ID for roam failure
 * in case of forced roam
 * @ROAM_FAIL_REASON: One of WMI_ROAM_FAIL_REASON_ID for roam failure
 */
enum roam_fail_params {
	ROAM_TRIGGER_REASON,
	ROAM_INVOKE_FAIL_REASON,
	ROAM_FAIL_REASON,
};

/**
 * enum wlan_roam_failure_reason_code - Roaming failure reason codes
 * @ROAM_FAIL_REASON_NO_SCAN_START: Scan start failed
 * @ROAM_FAIL_REASON_NO_AP_FOUND: No roamable AP found
 * @ROAM_FAIL_REASON_NO_CAND_AP_FOUND: No candidate AP found
 * @ROAM_FAIL_REASON_HOST: Host aborted roaming due to vdev stop from
 * host
 * @ROAM_FAIL_REASON_AUTH_SEND: Auth TX failure
 * @ROAM_FAIL_REASON_NO_AUTH_RESP: No Authentication response received
 * @ROAM_FAIL_REASON_AUTH_RECV: Authentication response received with
 * error status code
 * @ROAM_FAIL_REASON_REASSOC_SEND: Reassoc request TX failed
 * @ROAM_FAIL_REASON_REASSOC_RECV: Reassoc response frame not received
 * @ROAM_FAIL_REASON_NO_REASSOC_RESP: No reassociation response received
 * @ROAM_FAIL_REASON_EAPOL_TIMEOUT: EAPoL timedout
 * @ROAM_FAIL_REASON_MLME: MLME internal error
 * @ROAM_FAIL_REASON_INTERNAL_ABORT: Abort due to internal firmware error
 * @ROAM_FAIL_REASON_SCAN_START: Not able to start roam scan
 * @ROAM_FAIL_REASON_AUTH_NO_ACK: No ack received for Auth request frame
 * @ROAM_FAIL_REASON_AUTH_INTERNAL_DROP: Auth request dropped internally
 * @ROAM_FAIL_REASON_REASSOC_NO_ACK: No ack received for reassoc request frame
 * @ROAM_FAIL_REASON_REASSOC_INTERNAL_DROP: Reassoc frame dropped internally
 * at firmware
 * @ROAM_FAIL_REASON_EAPOL_M2_SEND: EAPoL M2 send failed
 * @ROAM_FAIL_REASON_EAPOL_M2_INTERNAL_DROP: EAPoL M2 frame dropped internally
 * at firmware
 * @ROAM_FAIL_REASON_EAPOL_M2_NO_ACK: No ack received for EAPoL M2 frame
 * @ROAM_FAIL_REASON_EAPOL_M3_TIMEOUT: EAPoL M3 not received from AP
 * @ROAM_FAIL_REASON_EAPOL_M4_SEND: EAPoL M4 frame TX failed
 * @ROAM_FAIL_REASON_EAPOL_M4_INTERNAL_DROP: EAPoL M4 frame dropped internally
 * @ROAM_FAIL_REASON_EAPOL_M4_NO_ACK: No ack received for EAPoL M4 frame
 * @ROAM_FAIL_REASON_NO_SCAN_FOR_FINAL_BMISS: Roam scan start failed for final
 * bmiss case
 * @ROAM_FAIL_REASON_DISCONNECT: Deauth/Disassoc frame received from AP during
 * roaming
 * @ROAM_FAIL_REASON_SYNC: Roam failure due to host wake-up during roaming in
 * progress
 * @ROAM_FAIL_REASON_SAE_INVALID_PMKID: Invalid PMKID during SAE roaming
 * @ROAM_FAIL_REASON_SAE_PREAUTH_TIMEOUT: SAE roaming preauthentication
 * timedout
 * @ROAM_FAIL_REASON_SAE_PREAUTH_FAIL: SAE preauthentication failure
 * @ROAM_FAIL_REASON_UNABLE_TO_START_ROAM_HO: Start handoff failed
 * @ROAM_FAIL_REASON_UNKNOWN: Default reason
 */
enum wlan_roam_failure_reason_code {
	ROAM_FAIL_REASON_NO_SCAN_START = 1,
	ROAM_FAIL_REASON_NO_AP_FOUND,
	ROAM_FAIL_REASON_NO_CAND_AP_FOUND,

	/* Failure reasons after roam scan is complete */
	ROAM_FAIL_REASON_HOST,
	ROAM_FAIL_REASON_AUTH_SEND,
	ROAM_FAIL_REASON_NO_AUTH_RESP,
	ROAM_FAIL_REASON_AUTH_RECV,
	ROAM_FAIL_REASON_REASSOC_SEND,
	ROAM_FAIL_REASON_REASSOC_RECV,
	ROAM_FAIL_REASON_NO_REASSOC_RESP,
	ROAM_FAIL_REASON_EAPOL_TIMEOUT,
	ROAM_FAIL_REASON_MLME,
	ROAM_FAIL_REASON_INTERNAL_ABORT,
	ROAM_FAIL_REASON_SCAN_START,
	ROAM_FAIL_REASON_AUTH_NO_ACK,
	ROAM_FAIL_REASON_AUTH_INTERNAL_DROP,
	ROAM_FAIL_REASON_REASSOC_NO_ACK,
	ROAM_FAIL_REASON_REASSOC_INTERNAL_DROP,
	ROAM_FAIL_REASON_EAPOL_M2_SEND,
	ROAM_FAIL_REASON_EAPOL_M2_INTERNAL_DROP,
	ROAM_FAIL_REASON_EAPOL_M2_NO_ACK,
	ROAM_FAIL_REASON_EAPOL_M3_TIMEOUT,
	ROAM_FAIL_REASON_EAPOL_M4_SEND,
	ROAM_FAIL_REASON_EAPOL_M4_INTERNAL_DROP,
	ROAM_FAIL_REASON_EAPOL_M4_NO_ACK,
	ROAM_FAIL_REASON_NO_SCAN_FOR_FINAL_BMISS,
	ROAM_FAIL_REASON_DISCONNECT,
	ROAM_FAIL_REASON_SYNC,
	ROAM_FAIL_REASON_SAE_INVALID_PMKID,
	ROAM_FAIL_REASON_SAE_PREAUTH_TIMEOUT,
	ROAM_FAIL_REASON_SAE_PREAUTH_FAIL,
	ROAM_FAIL_REASON_UNABLE_TO_START_ROAM_HO,
	ROAM_FAIL_REASON_UNKNOWN = 255,
};

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * srtuct reassoc_timer_ctx - reassoc timer context
 * @pdev: pdev object pointer
 * @vdev_id: vdev id
 * @cm_id: cm id to find cm_roam_req
 */
struct reassoc_timer_ctx {
	struct wlan_objmgr_pdev *pdev;
	uint8_t vdev_id;
	wlan_cm_id cm_id;
};
#endif

struct roam_synch_frame_ind {
	uint32_t bcn_probe_rsp_len;
	uint8_t *bcn_probe_rsp;
	uint8_t is_beacon;
	uint32_t reassoc_req_len;
	uint8_t *reassoc_req;
	uint32_t reassoc_rsp_len;
	uint8_t *reassoc_rsp;
	uint8_t vdev_id;
};

/* struct owe_transition_mode_info - structure containing owe transition mode
 * element info
 * @is_owe_transition_conn: Current connection is in owe transition mode or not
 * @ssid: ssid
 */
struct owe_transition_mode_info {
	bool is_owe_transition_conn;
	struct wlan_ssid  ssid;
};

/**
 * struct rso_config - connect config to be used to send info in
 * RSO. This is the info we dont have in VDEV or CM ctx
 * @reassoc_timer: reassoc timer
 * @ctx: reassoc timer context
 * @cm_rso_lock: RSO lock
 * @orig_sec_info: original security info coming from the connect req from
 * supplicant, without intersection of the peer capability
 * @country_code: country code from connected AP's beacon IE
 * @disable_hi_rssi: disable high rssi
 * @roam_control_enable: Flag used to cache the status of roam control
 *			 configuration. This will be set only if the
 *			 corresponding vendor command data is configured to
 *			 driver/firmware successfully. The same shall be
 *			 returned to userspace whenever queried for roam
 *			 control config status.
 * @rescan_rssi_delta: Roam scan rssi delta. Start new rssi triggered scan only
 * if it changes by rescan_rssi_delta value.
 * @beacon_rssi_weight: Number of beacons to be used to calculate the average
 * rssi of the AP.
 * @hi_rssi_scan_delay: Roam scan delay in ms for High RSSI roam trigger.
 * @roam_scan_scheme_bitmap: Bitmap of roam triggers for which partial channel
 * map scan scheme needs to be enabled. Each bit in the bitmap corresponds to
 * the bit position in the order provided by the enum roam_trigger_reason
 * Ex: roam_scan_scheme_bitmap - 0x00110 will enable partial scan for below
 * triggers:
 * ROAM_TRIGGER_REASON_PER, ROAM_TRIGGER_REASON_BMISS
 * @cfg_param: per vdev config params
 * @assoc_ie: assoc IE
 * @prev_ap_bcn_ie: last connetced AP ie
 * @occupied_chan_lst: occupied channel list
 * @roam_candidate_count: candidate count
 * @is_ese_assoc: is ese assoc
 * @krk: krk data
 * @btk: btk data
 * @psk_pmk: pmk
 * @pmk_len: length of pmk
 * @owe_info: owe ap profile info
 * @mdid: mdid info
 * @is_11r_assoc: is 11r assoc
 * @is_adaptive_11r_connection: is adaptive 11r connection
 * @hs_20_ap: Hotspot 2.0 AP
 * @mbo_oce_enabled_ap: MBO/OCE enabled network
 * @is_single_pmk: is single pmk
 * @roam_scan_freq_lst: roam freq list
 * @roam_fail_reason: One of WMI_ROAM_FAIL_REASON_ID
 * @roam_trigger_reason: Roam trigger reason(enum WMI_ROAM_TRIGGER_REASON_ID)
 * @roam_invoke_fail_reason: One of reason id from enum
 * wmi_roam_invoke_status_error in case of forced roam
 * @lost_link_rssi: lost link RSSI
 * @roam_sync_frame_ind: roam sync frame ind
 * @roam_band_bitmask: This allows the driver to roam within this band
 * @roam_invoke_source: roam invoke source
 * @roam_invoke_bssid: mac address used for roam invoke
 * @is_forced_roaming: bool value indicating if its forced roaming
 * @rso_rsn_caps: rsn caps with global user MFP which can be used for
 *                cross-AKM roaming
 */
struct rso_config {
#ifdef WLAN_FEATURE_HOST_ROAM
	qdf_mc_timer_t reassoc_timer;
	struct reassoc_timer_ctx ctx;
#endif
	qdf_mutex_t cm_rso_lock;
	struct security_info orig_sec_info;
	uint8_t country_code[REG_ALPHA2_LEN + 1];
	bool disable_hi_rssi;
	bool roam_control_enable;
	uint8_t rescan_rssi_delta;
	uint8_t beacon_rssi_weight;
	uint32_t hi_rssi_scan_delay;
	uint32_t roam_scan_scheme_bitmap;
	struct rso_cfg_params cfg_param;
	struct element_info assoc_ie;
	struct element_info prev_ap_bcn_ie;
	struct wlan_chan_list occupied_chan_lst;
	int8_t roam_candidate_count;
	uint8_t uapsd_mask;
#ifdef FEATURE_WLAN_ESE
	bool is_ese_assoc;
	uint8_t krk[WMI_KRK_KEY_LEN];
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t btk[WMI_BTK_KEY_LEN];
#endif
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t psk_pmk[ROAM_SCAN_PSK_SIZE];
	uint8_t pmk_len;
#endif
	struct owe_transition_mode_info owe_info;
	struct mobility_domain_info mdid;
	bool is_11r_assoc;
	bool is_adaptive_11r_connection;
	bool hs_20_ap;
	bool is_single_pmk;
	uint32_t mbo_oce_enabled_ap;
	struct rso_chan_info roam_scan_freq_lst;
	uint32_t roam_fail_reason;
	uint32_t roam_trigger_reason;
	uint32_t roam_invoke_fail_reason;
	int32_t lost_link_rssi;
	struct roam_synch_frame_ind roam_sync_frame_ind;
	uint32_t roam_band_bitmask;
	enum wlan_cm_source roam_invoke_source;
	struct qdf_mac_addr roam_invoke_bssid;
	bool is_forced_roaming;
	uint16_t rso_rsn_caps;
};

/**
 * enum sta_roam_policy_dfs_mode - state of DFS mode for STA ROME policy
 * @STA_ROAM_POLICY_NONE: DFS mode attribute is not valid
 * @STA_ROAM_POLICY_DFS_ENABLED:  DFS mode is enabled
 * @STA_ROAM_POLICY_DFS_DISABLED: DFS mode is disabled
 * @STA_ROAM_POLICY_DFS_DEPRIORITIZE: Deprioritize DFS channels in scanning
 */
enum sta_roam_policy_dfs_mode {
	STA_ROAM_POLICY_NONE,
	STA_ROAM_POLICY_DFS_ENABLED,
	STA_ROAM_POLICY_DFS_DISABLED,
	STA_ROAM_POLICY_DFS_DEPRIORITIZE
};

/**
 * struct rso_roam_policy_params - sta roam policy params for station
 * @dfs_mode: tell is DFS channels needs to be skipped while scanning
 * @skip_unsafe_channels: tells if unsafe channels needs to be skip in scanning
 * @sap_operating_band: Opearting band for SAP
 */
struct rso_roam_policy_params {
	enum sta_roam_policy_dfs_mode dfs_mode;
	bool skip_unsafe_channels;
	uint8_t sap_operating_band;
};

#define DEFAULT_RSSI_DB_GAP     30  /* every 30 dbm for one category */
#define ENABLE_FT_OVER_DS      1   /* enable ft_over_ds */

/**
 * struct rso_params - global RSO params
 * @num_ssid_allowed_list: The number of SSID profiles that are
 *                         in the Whitelist. When roaming, we
 *                         consider the BSSID's with this SSID
 *                         also for roaming apart from the connected one's
 * @ssid_allowed_list: Whitelist SSID's
 * @num_bssid_favored: Number of BSSID's which have a preference over others
 * @bssid_favored: Favorable BSSID's
 * @bssid_favored_factor: RSSI to be added to this BSSID to prefer it
 * @good_rssi_roam: Lazy Roam
 * @alert_rssi_threshold: Alert RSSI
 * @rssi: rssi diff
 * @raise_rssi_thresh_5g: The RSSI threshold below which the
 *                        raise_factor_5g (boost factor) should be applied.
 * @drop_rssi_thresh_5g: The RSSI threshold beyond which the
 *                       drop_factor_5g (penalty factor) should be applied
 * @raise_factor_5g: Boost factor
 * @drop_factor_5g: Penalty factor
 * @max_raise_rssi_5g: Maximum amount of Boost that can added
 * @is_fils_roaming_supported: fils roaming supported
 * @roam_scan_control: roam scan control
 * @policy_params: roam policy params
 * @neighbor_report_offload: neighbor report offload params
 */
struct rso_config_params {
	uint8_t num_ssid_allowed_list;
	struct wlan_ssid ssid_allowed_list[MAX_SSID_ALLOWED_LIST];
	uint8_t num_bssid_favored;
	struct qdf_mac_addr bssid_favored[MAX_BSSID_FAVORED];
	uint8_t bssid_favored_factor[MAX_BSSID_FAVORED];
	int good_rssi_roam;
	int alert_rssi_threshold;
	int rssi_diff;
	int raise_rssi_thresh_5g;
	int drop_rssi_thresh_5g;
	uint8_t raise_factor_5g;
	uint8_t drop_factor_5g;
	int max_raise_rssi_5g;
	uint8_t cat_rssi_offset;
	bool is_fils_roaming_supported;
	bool roam_scan_control;
	struct rso_roam_policy_params policy_params;
	struct cm_roam_neighbor_report_offload_params neighbor_report_offload;
};

/**
 * enum roam_cfg_param  - Type values for roaming parameters used as index
 * for get/set of roaming config values(pNeighborRoamInfo in legacy)
 * @RSSI_CHANGE_THRESHOLD: Rssi change threshold
 * @BEACON_RSSI_WEIGHT: Beacon Rssi weight parameter
 * @HI_RSSI_DELAY_BTW_SCANS: High Rssi delay between scans
 * @EMPTY_SCAN_REFRESH_PERIOD: empty scan refresh period
 * @FULL_ROAM_SCAN_PERIOD: Full roam scan period
 * @ENABLE_SCORING_FOR_ROAM: enable scoring
 * @SCAN_MIN_CHAN_TIME: scan min chan time
 * @SCAN_MAX_CHAN_TIME: scan max chan time
 * @NEIGHBOR_SCAN_PERIOD: neighbour scan period
 * @ROAM_CONFIG_ENABLE: Roam config enable
 * @ROAM_PREFERRED_CHAN: preferred channel list
 * @ROAM_SPECIFIC_CHAN: spedific channel list
 * @ROAM_RSSI_DIFF: rssi diff
 * @NEIGHBOUR_LOOKUP_THRESHOLD: lookup threshold
 * @SCAN_N_PROBE: scan n probe
 * @SCAN_HOME_AWAY: scan and away
 * @NEIGHBOUR_SCAN_REFRESH_PERIOD: scan refresh
 * @ROAM_CONTROL_ENABLE: roam control enable
 * @UAPSD_MASK: uapsd mask
 * @MOBILITY_DOMAIN: mobility domain
 * @IS_11R_CONNECTION: is 11r connection
 * @ADAPTIVE_11R_CONNECTION: adaptive 11r
 * @HS_20_AP: Hotspot 2.0 AP
 * @MBO_OCE_ENABLED_AP: MBO/OCE enabled network
 * @LOST_LINK_RSSI: lost link RSSI
 * @ROAM_BAND: Allowed band for roaming in FW
 * @ROAM_RSSI_DIFF_6GHZ: roam rssi diff for 6 GHz AP
 */
enum roam_cfg_param {
	RSSI_CHANGE_THRESHOLD,
	BEACON_RSSI_WEIGHT,
	HI_RSSI_DELAY_BTW_SCANS,
	EMPTY_SCAN_REFRESH_PERIOD,
	FULL_ROAM_SCAN_PERIOD,
	ENABLE_SCORING_FOR_ROAM,
	SCAN_MIN_CHAN_TIME,
	SCAN_MAX_CHAN_TIME,
	NEIGHBOR_SCAN_PERIOD,
	ROAM_CONFIG_ENABLE,
	ROAM_PREFERRED_CHAN,
	ROAM_SPECIFIC_CHAN,
	ROAM_RSSI_DIFF,
	NEIGHBOUR_LOOKUP_THRESHOLD,
	SCAN_N_PROBE,
	SCAN_HOME_AWAY,
	NEIGHBOUR_SCAN_REFRESH_PERIOD,
	ROAM_CONTROL_ENABLE,
	UAPSD_MASK,
	MOBILITY_DOMAIN,
	IS_11R_CONNECTION,
	ADAPTIVE_11R_CONNECTION,
	HS_20_AP,
	MBO_OCE_ENABLED_AP,
	IS_SINGLE_PMK,
	LOST_LINK_RSSI,
	ROAM_BAND,
	HI_RSSI_SCAN_RSSI_DELTA,
	ROAM_RSSI_DIFF_6GHZ,
};

/**
 * enum roam_offload_init_flags  - Flags sent in Roam offload initialization.
 * @WLAN_ROAM_FW_OFFLOAD_ENABLE: Init roaming module at firwmare
 * @WLAN_ROAM_BMISS_FINAL_SCAN_ENABLE: Enable partial scan after final beacon
 * miss event at firmware
 * @WLAN_ROAM_SKIP_EAPOL_4WAY_HANDSHAKE: Disable 4 Way-HS offload to firmware
 * Setting this flag will make the eapol packets reach to host every time
 * and can cause frequent APPS wake-ups. And clearing this flag will make
 * eapol offload to firmware except for SAE and OWE roam.
 * @WLAN_ROAM_BMISS_FINAL_SCAN_TYPE: Set this flag to skip full scan on final
 * bmiss and use the channel map to do the partial scan alone
 * @WLAN_ROAM_SKIP_SAE_ROAM_4WAY_HANDSHAKE: Disable 4 Way-HS offload to firmware
 * Setting this flag will make the eapol packets reach to host and clearing this
 * flag will make eapol offload to firmware including for SAE roam.
 */
enum roam_offload_init_flags {
	WLAN_ROAM_FW_OFFLOAD_ENABLE = BIT(1),
	WLAN_ROAM_BMISS_FINAL_SCAN_ENABLE = BIT(2),
	WLAN_ROAM_SKIP_EAPOL_4WAY_HANDSHAKE = BIT(3),
	WLAN_ROAM_BMISS_FINAL_SCAN_TYPE = BIT(4),
	WLAN_ROAM_SKIP_SAE_ROAM_4WAY_HANDSHAKE = BIT(5)
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
 * @candidate_rssi_threshold_2g: RSSI threshold of the
 * candidate AP in 2.4Ghz band
 * @candidate_rssi_threshold_5g: RSSI threshold of the candidate AP in 5Ghz
 * band
 * @candidate_rssi_threshold_6g: RSSI threshold of the candidate AP in 6Ghz
 * band
 * @user_roam_reason: Roam triggered reason code, value zero is for enable
 * and non zero value is disable
 */
struct wlan_cm_roam_vendor_btm_params {
	uint32_t scan_freq_scheme;
	uint32_t connected_rssi_threshold;
	uint32_t candidate_rssi_threshold_2g;
	uint32_t candidate_rssi_threshold_5g;
	uint32_t candidate_rssi_threshold_6g;
	uint32_t user_roam_reason;
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
 * @bg_rssi_threshold: Value of rssi threshold to trigger roaming
 *                     after background scan.
 * @num_allowed_authmode: Number of allowerd authmode
 * @allowed_authmode: List of allowed authmode other than connected
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
	uint8_t bg_rssi_threshold;
	uint32_t num_allowed_authmode;
	uint32_t allowed_authmode[WLAN_CRYPTO_AUTH_MAX];
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
 * @sae_pk_ap_weightage: SAE-PK AP score in %
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
 * @eht_caps_weightage: EHT caps weightage out of total score in %
 * @mlo_weightage: MLO weightage out of total score in %
 * @security_weightage: Security(WPA/WPA2/WPA3) weightage out of
 * total score in %
 * @security_index_score: Security scoring percentage information.
 *                BITS 0-7 :- It contains scoring percentage of WPA security
 *                BITS 8-15  :- It contains scoring percentage of WPA2 security
 *                BITS 16-23 :- It contains scoring percentage of WPA3 security
 *                BITS 24-31 :- reserved
 *                The value of each index must be 0-100
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
	uint32_t sae_pk_ap_weightage;
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
#ifdef WLAN_FEATURE_11BE_MLO
	uint8_t eht_caps_weightage;
	uint8_t mlo_weightage;
#endif
	int32_t security_weightage;
	uint32_t security_index_score;
};

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
 * ROAM_TRIGGER_REASON_PMK_TIMEOUT: Roam triggered due to PMK expiry
 * ROAM_TRIGGER_REASON_BTC: Roam triggered due to BT Coex
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
	ROAM_TRIGGER_REASON_PMK_TIMEOUT,
	ROAM_TRIGGER_REASON_BTC,
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
 * struct wlan_roam_triggers - vendor configured roam triggers
 * @vdev_id: vdev id
 * @trigger_bitmap: vendor configured roam trigger bitmap as
 *		    defined @enum roam_control_trigger_reason
 * @roam_score_delta: Value of roam score delta
 * percentage to trigger roam
 * @roam_scan_scheme_bitmap: Bitmap of roam triggers as defined in
 * enum roam_trigger_reason, for which the roam scan scheme should
 * be partial scan
 * @control_param: roam trigger param
 * @min_rssi_params: Min RSSI values for different roam triggers
 * @score_delta_params: Roam score delta values for different triggers
 */
struct wlan_roam_triggers {
	uint32_t vdev_id;
	uint32_t trigger_bitmap;
	uint32_t roam_score_delta;
	uint32_t roam_scan_scheme_bitmap;
	struct wlan_cm_roam_vendor_btm_params vendor_btm_param;
	struct roam_trigger_min_rssi min_rssi_params[NUM_OF_ROAM_MIN_RSSI];
	struct roam_trigger_score_delta score_delta_param[NUM_OF_ROAM_TRIGGERS];
};

/**
 * struct ap_profile_params - ap profile params
 * @vdev_id: vdev id
 * @profile: ap profile to match candidate
 * @param: scoring params to short candidate
 * @min_rssi_params: Min RSSI values for different roam triggers
 * @score_delta_params: Roam score delta values for different triggers
 * @owe_ap_profile: owe ap profile info
 */
struct ap_profile_params {
	uint8_t vdev_id;
	struct ap_profile profile;
	struct scoring_param param;
	struct roam_trigger_min_rssi min_rssi_params[NUM_OF_ROAM_MIN_RSSI];
	struct roam_trigger_score_delta score_delta_param[NUM_OF_ROAM_TRIGGERS];
	struct owe_transition_mode_info owe_ap_profile;
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

#define NOISE_FLOOR_DBM_DEFAULT          (-96)
#define RSSI_MIN_VALUE                   (-128)
#define RSSI_MAX_VALUE                   (127)

#ifdef WLAN_FEATURE_FILS_SK
#define WLAN_FILS_MAX_USERNAME_LENGTH 16

/**
 * struct wlan_roam_fils_params - Roaming FILS params
 * @next_erp_seq_num: next ERP sequence number
 * @username: username
 * @username_length: username length
 * @rrk: RRK
 * @rrk_length: length of @rrk
 * @rik: RIK
 * @rik_length: length of @rik
 * @realm: realm
 * @realm_len: length of @realm
 * @fils_ft: xx_key for FT-FILS connection
 * @fils_ft_len: length of FT-FILS
 */
struct wlan_roam_fils_params {
	uint32_t next_erp_seq_num;
	uint8_t username[WLAN_FILS_MAX_USERNAME_LENGTH];
	uint32_t username_length;
	uint8_t rrk[WLAN_FILS_MAX_RRK_LENGTH];
	uint32_t rrk_length;
	uint8_t rik[WLAN_FILS_MAX_RIK_LENGTH];
	uint32_t rik_length;
	uint8_t realm[WLAN_FILS_MAX_REALM_LEN];
	uint32_t realm_len;
	uint8_t fils_ft[WLAN_FILS_FT_MAX_LEN];
	uint8_t fils_ft_len;
};
#endif

/**
 * struct wlan_roam_scan_params  - Roaming scan parameters
 * @vdev_id: vdev id
 * @dwell_time_passive: dwell time in msec on passive channels
 * @dwell_time_active: dwell time in msec on active channels
 * @min_dwell_time_6ghz: minimum dwell time in msec for 6 GHz channel
 * @burst_duration: Burst duration time in msec
 * @min_rest_time: min time in msec on the BSS channel,only valid if atleast
 * one VDEV is active
 * @max_rest_time: max rest time in msec on the BSS channel,only valid if
 * at least one VDEV is active
 * @probe_spacing_time: time in msec between 2 consequetive probe requests with
 * in a set
 * @probe_delay: delay in msec before sending first probe request after
 * switching to a channel
 * @repeat_probe_time: time in msec between 2 consequetive probe requests within
 * a set
 * @max_scan_time: maximum time in msec allowed for scan
 * @idle_time: data inactivity time in msec on bss channel that will be used by
 * scanner for measuring the inactivity
 * @n_probes: Max number of probes to be sent
 * @scan_ctrl_flags: Scan control flags
 * @scan_ctrl_flags_ext: Scan control flags extended
 * @rso_adaptive_dwell_mode: Adaptive dwell mode
 * @num_chan: number of channels
 * @num_bssid: number of bssids in tlv bssid_list[]
 * @ie_len: number of bytes in ie data. In the TLV ie_data[]
 * @dwell_time_active_2g: dwell time in msec on active 2G channels.
 * @dwell_time_active_6ghz: dwell time in msec when 6 GHz channel
 * @dwell_time_passive_6ghz: Passive scan dwell time in msec for 6Ghz channel.
 * @scan_start_offset: Offset time is in milliseconds per channel
 */
struct wlan_roam_scan_params {
	uint32_t vdev_id;
	uint32_t dwell_time_passive;
	uint32_t dwell_time_active;
	uint32_t min_dwell_time_6ghz;
	uint32_t burst_duration;
	uint32_t min_rest_time;
	uint32_t max_rest_time;
	uint32_t probe_spacing_time;
	uint32_t probe_delay;
	uint32_t repeat_probe_time;
	uint32_t max_scan_time;
	uint32_t idle_time;
	uint32_t n_probes;
	uint32_t scan_ctrl_flags;
	uint32_t scan_ctrl_flags_ext;
	enum scan_dwelltime_adaptive_mode rso_adaptive_dwell_mode;
	uint32_t num_chan;
	uint32_t num_bssid;
	uint32_t ie_len;
	uint32_t dwell_time_active_2g;
	uint32_t dwell_time_active_6ghz;
	uint32_t dwell_time_passive_6ghz;
	uint32_t scan_start_offset;
};

/**
 * struct wlan_roam_scan_mode_params  - WMI_ROAM_SCAN_MODE command fixed_param
 * wmi_roam_scan_mode_fixed_param related params
 * @roam_scan_mode: Roam scan mode flags
 * @min_delay_btw_scans: Minimum duration allowed between two consecutive roam
 * scans in millisecs.
 * @min_delay_roam_trigger_bitmask: Roaming triggers for which the min delay
 * between roam scans is applicable(bitmask of enum WMI_ROAM_TRIGGER_REASON_ID)
 */
struct wlan_roam_scan_mode_params {
	uint32_t roam_scan_mode;
	uint32_t min_delay_btw_scans;
	uint32_t min_delay_roam_trigger_bitmask;
};

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * struct wlan_rso_lfr3_params  - LFR-3.0 roam offload params to be filled
 * in the wmi_roam_offload_tlv_param TLV of WMI_ROAM_SCAN_MODE command.
 * @roam_rssi_cat_gap: RSSI category gap
 * @prefer_5ghz: Prefer 5G candidate AP
 * @select_5gz_margin: Prefer connecting to 5G AP even if its RSSI is lower by
 * select_5g_margin dBm
 * @reassoc_failure_timeout: reassociation response failure timeout
 * @ho_delay_for_rx: Time in millisecs to delay hand-off by this duration to
 * receive pending Rx frames from current BSS
 * @roam_retry_count: maximum number of software retries for preauth and
 * reassoc req
 * @roam_preauth_no_ack_timeout: duration in millsecs to wait before another SW
 * retry made if no ack seen for previous frame
 * @diable_self_roam: Disable roaming to current connected BSS.
 * @rct_validity_timer: duration value for which the entries in
 * roam candidate table(rct) are valid
 */
struct wlan_rso_lfr3_params {
	uint8_t roam_rssi_cat_gap;
	uint8_t prefer_5ghz;
	uint8_t select_5ghz_margin;
	uint32_t reassoc_failure_timeout;
	uint32_t ho_delay_for_rx;
	uint32_t roam_retry_count;
	uint32_t roam_preauth_no_ack_timeout;
	bool disable_self_roam;
	uint32_t rct_validity_timer;
};

#define WLAN_ROAM_OFFLOAD_NUM_MCS_SET     (16)
/**
 * struct wlan_lfr3_roam_offload_param  - LFR3 Roaming offload parameters
 * @capability: RSN capabilities
 * @ht_caps_info: HT capabilities information
 * @ampdu_param: AMPDU configs
 * @ht_ext_cap: HT extended capabilities info
 * @ht_txbf: HT Tx Beamform capabilities
 * @asel_cap: Antena selection capabilities
 * @qos_enabled: QoS enabled
 * @qos_caps: QoS capabilities
 * @wmm_caps: WMM capabilities
 * @mcsset: MCS set
 */
struct wlan_rso_lfr3_caps {
	uint32_t capability;
	uint32_t ht_caps_info;
	uint32_t ampdu_param;
	uint32_t ht_ext_cap;
	uint32_t ht_txbf;
	uint32_t asel_cap;
	uint32_t qos_enabled;
	uint32_t qos_caps;
	uint32_t wmm_caps;
	/* since this is 4 byte aligned, we don't declare it as tlv array */
	uint32_t mcsset[WLAN_ROAM_OFFLOAD_NUM_MCS_SET >> 2];
};

/**
 * struct wlan_rso_11i_params  - LFR-3.0 related parameters to be filled in
 * wmi_roam_11i_offload_tlv_param TLV in the WMI_ROAM_SCAN_MODE command.
 * @roam_key_mgmt_offload_enabled: Enable 4-way HS offload to firmware
 * @fw_okc: use OKC in firmware
 * @fw_pmksa_cache: use PMKSA cache in firmware
 * @is_sae_same_pmk: Flag to indicate fw whether WLAN_SAE_SINGLE_PMK feature is
 * enable or not
 * @psk_pmk: pre shared key/pairwise master key
 * @pmk_len: length of PMK
 */
struct wlan_rso_11i_params {
	bool roam_key_mgmt_offload_enabled;
	bool fw_okc;
	bool fw_pmksa_cache;
	bool is_sae_same_pmk;
	uint8_t psk_pmk[WMI_ROAM_SCAN_PSK_SIZE];
	uint8_t pmk_len;
};

/**
 * struct wlan_rso_11r_params  - LFR-3.0 parameters to fill
 * wmi_roam_11r_offload_tlv_param TLV related info in WMI_ROAM_SCAN_MODE command
 * @enable_ft_im_roaming: Flag to enable/disable FT-IM roaming upon receiving
 * deauth
 * @rokh_id_length: r0kh id length
 * @rokh_id: r0kh id
 * @mdid: mobility domain info
 * @enable_ft_over_ds: Flag to enable/disable FT-over-DS
 */
struct wlan_rso_11r_params {
	bool is_11r_assoc;
	bool is_adaptive_11r;
	bool enable_ft_im_roaming;
	uint8_t psk_pmk[WMI_ROAM_SCAN_PSK_SIZE];
	uint8_t pmk_len;
	uint32_t r0kh_id_length;
	uint8_t r0kh_id[WMI_ROAM_R0KH_ID_MAX_LEN];
	struct mobility_domain_info mdid;
	bool enable_ft_over_ds;
};

/**
 * struct wlan_rso_ese_params  - LFR-3.0 parameters to fill the
 * wmi_roam_ese_offload_tlv_param TLV related info in WMI_ROAM_SCAN_MODE command
 * @is_ese_assoc: flag to determine ese assoc
 * @krk: KRK
 * @btk: BTK
 */
struct wlan_rso_ese_params {
	bool is_ese_assoc;
	uint8_t krk[WMI_KRK_KEY_LEN];
	uint8_t btk[WMI_BTK_KEY_LEN];
};

/**
 * struct wlan_rso_sae_offload_params - SAE authentication offload related
 * parameters.
 * @spmk_timeout: Single PMK timeout value in seconds.
 */
struct wlan_rso_sae_offload_params {
	uint32_t spmk_timeout;
};
#endif

#define ROAM_SCAN_DWELL_TIME_ACTIVE_DEFAULT   (100)
#define ROAM_SCAN_DWELL_TIME_PASSIVE_DEFAULT  (110)
#define ROAM_SCAN_MIN_REST_TIME_DEFAULT       (50)
#define ROAM_SCAN_MAX_REST_TIME_DEFAULT       (500)
#define ROAM_SCAN_HW_DEF_SCAN_MAX_DURATION    30000 /* 30 secs */
#define ROAM_SCAN_CHANNEL_SWITCH_TIME         (4)

/**
 * struct roam_offload_scan_params - structure containing roaming offload scan
 * parameters to be filled over WMI_ROAM_SCAN_MODE command.
 * @vdev_id: vdev id
 * @is_rso_stop: flag to tell whether roam req is valid or NULL
 * @rso_mode_info: Roam scan mode related parameters
 * @rso_scan_params: Roam scan offload scan start params
 * @scan_params: Roaming scan related parameters
 * @assoc_ie_length: Assoc IE length
 * @assoc_ie: Assoc IE buffer
 * @roam_offload_enabled: flag for offload enable
 * @add_fils_tlv: add FILS TLV boolean
 * @akm: authentication key management mode
 * @rso_lfr3_params: Candidate selection and other lfr-3.0 offload parameters
 * @rso_lfr3_caps: Self capabilities
 * @rso_11i_info: PMK, PMKSA, SAE single PMK related parameters
 * @rso_11r_info: FT related parameters
 * @rso_ese_info: ESE related parameters
 * @fils_roam_config: roam fils params
 * @sae_offload_params: SAE offload/single pmk related parameters
 */
struct wlan_roam_scan_offload_params {
	uint32_t vdev_id;
	uint8_t is_rso_stop;
	/* Parameters common for LFR-3.0 and LFR-2.0 */
	bool roaming_scan_policy;
	struct wlan_roam_scan_mode_params rso_mode_info;
	struct wlan_roam_scan_params rso_scan_params;
	uint32_t assoc_ie_length;
	uint8_t  assoc_ie[MAX_ASSOC_IE_LENGTH];
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	/* Parameters specific to LFR-3.0 */
	bool roam_offload_enabled;
	bool add_fils_tlv;
	int akm;
	struct wlan_rso_lfr3_params rso_lfr3_params;
	struct wlan_rso_lfr3_caps rso_lfr3_caps;
	struct wlan_rso_11i_params rso_11i_info;
	struct wlan_rso_11r_params rso_11r_info;
	struct wlan_rso_ese_params rso_ese_info;
#ifdef WLAN_FEATURE_FILS_SK
	struct wlan_roam_fils_params fils_roam_config;
#endif
	struct wlan_rso_sae_offload_params sae_offload_params;
#endif
};

/**
 * enum wlan_roam_offload_scan_rssi_flags - Flags for roam scan RSSI threshold
 * params, this enums will be used in flags param of the structure
 * wlan_roam_offload_scan_rssi_params
 * @ROAM_SCAN_RSSI_THRESHOLD_INVALID_FLAG: invalid flag
 * @ROAM_SCAN_RSSI_THRESHOLD_FLAG_ROAM_HI_RSSI_EN_ON_5G: enable high RSSI roam
 * trigger support to roam from 5 GHz to 6 GHz band
 */
enum wlan_roam_offload_scan_rssi_flags {
	ROAM_SCAN_RSSI_THRESHOLD_INVALID_FLAG,
	ROAM_SCAN_RSSI_THRESHOLD_FLAG_ROAM_HI_RSSI_EN_ON_5G = BIT(0),
};

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
 * @roam_data_rssi_threshold_triggers: triggers of bad data RSSI threshold to
 *                                  roam
 * @roam_data_rssi_threshold: Bad data RSSI threshold to roam
 * @rx_data_inactivity_time: Rx duration to check data RSSI
 * @flags: Flags for roam scan RSSI threshold params
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
	int32_t roam_earlystop_thres_min;
	int32_t roam_earlystop_thres_max;
	int dense_rssi_thresh_offset;
	int dense_min_aps_cnt;
	int initial_dense_status;
	int traffic_threshold;
	int32_t rssi_thresh_offset_5g;
	int8_t bg_scan_bad_rssi_thresh;
	uint8_t roam_bad_rssi_thresh_offset_2g;
	uint32_t bg_scan_client_bitmap;
	uint32_t roam_data_rssi_threshold_triggers;
	int32_t roam_data_rssi_threshold;
	uint32_t rx_data_inactivity_time;
	uint32_t flags;
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
 * wlan_roam_scan_channel_list  - Roam Scan channel list related
 * parameters
 * @vdev_id: Vdev id
 * @chan_count: Channel count
 * @chan_freq_list: Frequency list pointer
 * @chan_cache_type: Static or dynamic channel cache
 */
struct wlan_roam_scan_channel_list {
	uint32_t vdev_id;
	uint8_t chan_count;
	uint32_t chan_freq_list[CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t chan_cache_type;
};

/**
 * struct wlan_roam_rssi_change_params  - RSSI change parameters to be sent over
 * WMI_ROAM_SCAN_RSSI_CHANGE_THRESHOLD command
 * @vdev_id: vdev id
 * only if current RSSI changes by rssi_change_thresh value.
 * @bcn_rssi_weight: Beacon RSSI weightage
 * @hirssi_delay_btw_scans: Delay between high RSSI scans
 * @rssi_change_thresh: RSSI change threshold. Start new rssi triggered scan
 */
struct wlan_roam_rssi_change_params {
	uint32_t vdev_id;
	uint32_t bcn_rssi_weight;
	uint32_t hirssi_delay_btw_scans;
	int32_t rssi_change_thresh;
};

/**
 * struct wlan_cm_roam_rt_stats - Roam events stats update
 * @roam_stats_enabled: set 1 if roam stats feature is enabled from userspace
 * @roam_stats_wow_sent: set 1 if roam stats wow event is sent to FW
 */
struct wlan_cm_roam_rt_stats {
	uint8_t roam_stats_enabled;
	uint8_t roam_stats_wow_sent;
};

/**
 * enum roam_rt_stats_params: different types of params to set or get roam
 * events stats for the vdev
 * @ROAM_RT_STATS_ENABLE:              Roam stats feature if enable/not
 * @ROAM_RT_STATS_SUSPEND_MODE_ENABLE: Roam stats wow event if sent to FW/not
 */
enum roam_rt_stats_params {
	ROAM_RT_STATS_ENABLE,
	ROAM_RT_STATS_SUSPEND_MODE_ENABLE,
};

/**
 * struct wlan_roam_start_config - structure containing parameters for
 * roam start config
 * @rssi_params: roam scan rssi threshold parameters
 * @beacon_miss_cnt: roam beacon miss count parameters
 * @reason_vsie_enable: roam reason vsie enable parameters
 * @roam_triggers: roam triggers parameters
 * @scan_period_params: roam scan period parameters
 * @rssi_change_params: Roam offload RSSI change parameters
 * @profile_params: ap profile parameters
 * @rso_chan_info: Roam scan channel list parameters
 * @mawc_params: mawc parameters
 * @scan_filter_params: roam scan filter parameters
 * @btm_config: btm configuration
 * @roam_11k_params: 11k params
 * @bss_load_config: bss load config
 * @disconnect_params: disconnect params
 * @idle_params: idle params
 * @wlan_roam_rt_stats_config: roam events stats config
 * @wlan_roam_ho_delay_config: roam HO delay value
 * @wlan_exclude_rm_partial_scan_freq: Include/exclude the channels in roam full
 * scan that are already scanned as part of partial scan.
 * @wlan_roam_full_scan_6ghz_on_disc: Include the 6 GHz channels in roam full
 * scan only on prior discovery of any 6 GHz support in the environment.
 * @wlan_roam_rssi_diff_6ghz: This value is used as to how better the RSSI of
 * the new/roamable 6GHz AP should be for roaming.
 */
struct wlan_roam_start_config {
	struct wlan_roam_offload_scan_rssi_params rssi_params;
	struct wlan_roam_beacon_miss_cnt beacon_miss_cnt;
	struct wlan_roam_reason_vsie_enable reason_vsie_enable;
	struct wlan_roam_triggers roam_triggers;
	struct wlan_roam_scan_period_params scan_period_params;
	struct wlan_roam_scan_offload_params rso_config;
	struct wlan_roam_rssi_change_params rssi_change_params;
	struct ap_profile_params profile_params;
	struct wlan_roam_scan_channel_list rso_chan_info;
	struct wlan_roam_mawc_params mawc_params;
	struct wlan_roam_scan_filter_params scan_filter_params;
	struct wlan_roam_btm_config btm_config;
	struct wlan_roam_11k_offload_params roam_11k_params;
	struct wlan_roam_bss_load_config bss_load_config;
	struct wlan_roam_disconnect_params disconnect_params;
	struct wlan_roam_idle_params idle_params;
	uint8_t wlan_roam_rt_stats_config;
	uint16_t wlan_roam_ho_delay_config;
	uint8_t wlan_exclude_rm_partial_scan_freq;
	uint8_t wlan_roam_full_scan_6ghz_on_disc;
	uint8_t wlan_roam_rssi_diff_6ghz;
	/* other wmi cmd structures */
};

/**
 * struct wlan_roam_stop_config - structure containing parameters for
 * roam stop
 * @reason: roaming reason
 * @middle_of_roaming: in the middle of roaming
 * @rso_config: Roam scan mode config
 * @roam_11k_params: 11k params
 * @btm_config: btm configuration
 * @scan_filter_params: roam scan filter parameters
 * @disconnect_params: disconnect params
 * @idle_params: idle params
 * @roam_triggers: roam triggers parameters
 * @rssi_params: roam scan rssi threshold parameters
 * @send_rso_stop_resp: send rso stop response
 * @start_rso_stop_timer: start rso stop timer
 */
struct wlan_roam_stop_config {
	uint8_t reason;
	uint8_t middle_of_roaming;
	struct wlan_roam_scan_offload_params rso_config;
	struct wlan_roam_11k_offload_params roam_11k_params;
	struct wlan_roam_btm_config btm_config;
	struct wlan_roam_scan_filter_params scan_filter_params;
	struct wlan_roam_disconnect_params disconnect_params;
	struct wlan_roam_idle_params idle_params;
	struct wlan_roam_triggers roam_triggers;
	struct wlan_roam_offload_scan_rssi_params rssi_params;
	bool send_rso_stop_resp;
	bool start_rso_stop_timer;
};

/**
 * struct wlan_roam_update_config - structure containing parameters for
 * roam update config
 * @beacon_miss_cnt: roam beacon miss count parameters
 * @scan_filter_params: roam scan filter parameters
 * @scan_period_params: roam scan period parameters
 * @rssi_change_params: roam scan rssi change parameters
 * @rso_config: roam scan mode configurations
 * @profile_params: ap profile parameters
 * @rso_chan_info: Roam scan channel list parameters
 * @rssi_params: roam scan rssi threshold parameters
 * @disconnect_params: disconnect params
 * @idle_params: idle params
 * @roam_triggers: roam triggers parameters
 * @wlan_roam_rt_stats_config: roam events stats config
 * @wlan_roam_ho_delay_config: roam HO delay value
 * @wlan_exclude_rm_partial_scan_freq: Include/exclude the channels in roam full
 * scan that are already scanned as part of partial scan.
 * @wlan_roam_full_scan_6ghz_on_disc: Include the 6 GHz channels in roam full
 * scan only on prior discovery of any 6 GHz support in the environment.
 * @wlan_roam_rssi_diff_6ghz: This value is used as to how better the RSSI of
 * the new/roamable 6GHz AP should be for roaming.
 */
struct wlan_roam_update_config {
	struct wlan_roam_beacon_miss_cnt beacon_miss_cnt;
	struct wlan_roam_scan_filter_params scan_filter_params;
	struct wlan_roam_scan_period_params scan_period_params;
	struct wlan_roam_rssi_change_params rssi_change_params;
	struct wlan_roam_scan_offload_params rso_config;
	struct ap_profile_params profile_params;
	struct wlan_roam_scan_channel_list rso_chan_info;
	struct wlan_roam_offload_scan_rssi_params rssi_params;
	struct wlan_roam_disconnect_params disconnect_params;
	struct wlan_roam_idle_params idle_params;
	struct wlan_roam_triggers roam_triggers;
	uint8_t wlan_roam_rt_stats_config;
	uint16_t wlan_roam_ho_delay_config;
	uint8_t wlan_exclude_rm_partial_scan_freq;
	uint8_t wlan_roam_full_scan_6ghz_on_disc;
	uint8_t wlan_roam_rssi_diff_6ghz;
};

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
 * @WLAN_MLO_ROAM_SYNCH_IN_PROG: MLO Roam sync is ongoing,
 * only used for ml links.
 */
enum roam_offload_state {
	WLAN_ROAM_DEINIT,
	WLAN_ROAM_INIT,
	WLAN_ROAM_RSO_ENABLED,
	WLAN_ROAM_RSO_STOPPED,
	WLAN_ROAMING_IN_PROG,
	WLAN_ROAM_SYNCH_IN_PROG,
	WLAN_MLO_ROAM_SYNCH_IN_PROG,
};

#define WLAN_ROAM_SCAN_CANDIDATE_AP 0
#define WLAN_ROAM_SCAN_CURRENT_AP   1
#define WLAN_ROAM_SCAN_ROAMED_AP    2

/**
 *  struct roam_btm_response_data - BTM response related data
 *  @present:       Flag to check if the roam btm_rsp tlv is present
 *  @btm_status:    Btm request status
 *  @target_bssid:  AP MAC address
 *  @vsie_reason:   Vsie_reason value
 *  @timestamp:     This timestamp indicates the time when btm rsp is sent
 *  @btm_resp_dialog_token: Dialog token
 *  @btm_delay: BTM bss termination delay
 */
struct roam_btm_response_data {
	bool present;
	uint32_t btm_status;
	struct qdf_mac_addr target_bssid;
	uint32_t vsie_reason;
	uint32_t timestamp;
	uint16_t btm_resp_dialog_token;
	uint8_t btm_delay;
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
 * struct roam_msg_info - Roam message related information
 * @present:    Flag to check if the roam msg info tlv is present
 * @timestamp:  Timestamp is the absolute time w.r.t host timer which is
 * synchronized between the host and target
 * @msg_id:     Message ID from WMI_ROAM_MSG_ID
 * @msg_param1: msg_param1, values is based on the host & FW
 * understanding and depend on the msg ID
 * @msg_param2: msg_param2 value is based on the host & FW understanding
 * and depend on the msg ID
 */
struct roam_msg_info {
	bool present;
	uint32_t timestamp;
	uint32_t msg_id;
	uint32_t msg_param1;
	uint32_t msg_param2;
};

/**
 * struct roam_event_rt_info - Roam event related information
 * @roam_scan_state: roam scan state notif value
 * @roam_invoke_fail_reason: roam invoke fail reason
 */
struct roam_event_rt_info {
	uint32_t roam_scan_state;
	uint32_t roam_invoke_fail_reason;
};

/**
 * enum roam_rt_stats_type: different types of params to get roam event stats
 * for the vdev
 * @ROAM_RT_STATS_TYPE_SCAN_STATE: Roam Scan Start/End
 * @ROAM_RT_STATS_TYPE_INVOKE_FAIL_REASON: One of WMI_ROAM_FAIL_REASON_ID for
 * roam failure in case of forced roam
 * @ROAM_RT_STATS_TYPE_ROAM_SCAN_INFO: Roam Trigger/Fail/Scan/AP Stats
 */
enum roam_rt_stats_type {
	ROAM_RT_STATS_TYPE_SCAN_STATE,
	ROAM_RT_STATS_TYPE_INVOKE_FAIL_REASON,
	ROAM_RT_STATS_TYPE_ROAM_SCAN_INFO,
};

/**
 * struct roam_frame_info  - Structure to hold the mgmt frame/eapol frame
 * related info exchanged during roaming.
 * @present:     Flag to indicate if roam frame info TLV is present
 * @bssid:       BSSID of the candidate AP or roamed AP to which the
 * frame exchange happened
 * @timestamp:   Fw timestamp at which the frame was Tx/Rx'ed
 * @type:        Frame Type
 * @subtype:     Frame subtype
 * @is_rsp:      True if frame is response frame else false
 * @seq_num:     Frame sequence number from the 802.11 header
 * @status_code: Status code from 802.11 spec, section 9.4.1.9
 * @auth_algo: Authentication algorithm as defined in 802.11 spec,
 * 9.4.1.1 Authentication Algorithm Number field
 * @tx_status: Frame TX status defined by enum qdf_dp_tx_rx_status
 * applicable only for tx frames
 * @rssi: Frame rssi
 * @retry_count: Frame retry count
 */
struct roam_frame_info {
	bool present;
	struct qdf_mac_addr bssid;
	uint32_t timestamp;
	uint8_t type;
	uint8_t subtype;
	uint8_t is_rsp;
	enum qdf_dp_tx_rx_status tx_status;
	uint16_t seq_num;
	uint8_t auth_algo;
	uint16_t status_code;
	int32_t rssi;
	uint16_t retry_count;
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
 * struct roam_invoke_req - roam invoke request
 * @vdev_id: vdev for which the roaming has to be enabled/disabled
 * @target_bssid: target mac address
 * @ch_freq: channel frequency
 * @frame_len: frame length, includs mac header, fixed params and ies
 * @frame_buf: buffer contaning probe response or beacon
 * @is_same_bssid: flag to indicate if roaming is requested for same bssid
 * @forced_roaming: Roam to any bssid in any ch (here bssid & ch is not given)
 */
struct roam_invoke_req {
	uint8_t vdev_id;
	struct qdf_mac_addr target_bssid;
	uint32_t ch_freq;
	uint32_t frame_len;
	uint8_t *frame_buf;
	uint8_t is_same_bssid;
	bool forced_roaming;
};

/**
 * enum cm_roam_notif: roaming notification
 * @CM_ROAM_NOTIF_INVALID: invalid notification. Do not interpret notif field
 * @CM_ROAM_NOTIF_ROAM_START: indicate that roaming is started. sent only in
			      non WOW state
 * @CM_ROAM_NOTIF_ROAM_ABORT: indicate that roaming is aborted. sent only in
			      non WOW state
 * @CM_ROAM_NOTIF_ROAM_REASSOC: indicate that reassociation is done. sent only
				in non WOW state
 * @CM_ROAM_NOTIF_SCAN_MODE_SUCCESS: indicate that roaming scan mode is
				     successful
 * @CM_ROAM_NOTIF_SCAN_MODE_FAIL: indicate that roaming scan mode is failed due
				  to internal roaming state
 * @CM_ROAM_NOTIF_DISCONNECT: indicate that roaming not allowed due BTM req
 * @CM_ROAM_NOTIF_SUBNET_CHANGED: indicate that subnet has changed
 * @CM_ROAM_NOTIF_SCAN_START: indicate roam scan start, notif_params to be sent
			      as WMI_ROAM_TRIGGER_REASON_ID
 * @CM_ROAM_NOTIF_DEAUTH_RECV: indicate deauth received, notif_params to be sent
			       as reason code, notif_params1 to be sent as
			       frame length
 * @CM_ROAM_NOTIF_DISASSOC_RECV: indicate disassoc received, notif_params to be
				 sent as reason code, notif_params1 to be sent
				 as frame length
 * @CM_ROAM_NOTIF_HO_FAIL: indicates that roaming scan mode is successful but
			   caused disconnection and subsequent
			   WMI_ROAM_REASON_HO_FAILED is event expected
 * @CM_ROAM_NOTIF_SCAN_END: indicate roam scan end, notif_params to be sent
			    as WMI_ROAM_TRIGGER_REASON_ID
 */
enum cm_roam_notif {
	CM_ROAM_NOTIF_INVALID = 0,
	CM_ROAM_NOTIF_ROAM_START,
	CM_ROAM_NOTIF_ROAM_ABORT,
	CM_ROAM_NOTIF_ROAM_REASSOC,
	CM_ROAM_NOTIF_SCAN_MODE_SUCCESS,
	CM_ROAM_NOTIF_SCAN_MODE_FAIL,
	CM_ROAM_NOTIF_DISCONNECT,
	CM_ROAM_NOTIF_SUBNET_CHANGED,
	CM_ROAM_NOTIF_SCAN_START,
	CM_ROAM_NOTIF_DEAUTH_RECV,
	CM_ROAM_NOTIF_DISASSOC_RECV,
	CM_ROAM_NOTIF_HO_FAIL,
	CM_ROAM_NOTIF_SCAN_END,
};

/**
 * enum roam_reason: Roam reason
 * @ROAM_REASON_INVALID: invalid reason. Do not interpret reason field
 * @ROAM_REASON_BETTER_AP: found a better AP
 * @ROAM_REASON_BMISS: beacon miss detected
 * @ROAM_REASON_LOW_RSSI: connected AP's low rssi condition detected
 * @ROAM_REASON_SUITABLE_AP: found another AP that matches SSID and Security
 *  profile in WMI_ROAM_AP_PROFILE, found during scan triggered upon FINAL_BMISS
 * @ROAM_REASON_HO_FAILED: LFR3.0 roaming failed, indicate the disconnection
 *			   to host
 * @ROAM_REASON_INVOKE_ROAM_FAIL: Result code of WMI_ROAM_INVOKE_CMDID. Any
 *  roaming failure before reassociation will be indicated to host with this
 *  reason. Any roaming failure after reassociation will be indicated to host
 *  with WMI_ROAM_REASON_HO_FAILED no matter WMI_ROAM_INVOKE_CMDID is
 *  called or not.
 * @ROAM_REASON_RSO_STATUS
 * @ROAM_REASON_BTM: Roaming because of BTM request received
 * @ROAM_REASON_DEAUTH: deauth/disassoc received
 */
enum roam_reason {
	ROAM_REASON_INVALID,
	ROAM_REASON_BETTER_AP,
	ROAM_REASON_BMISS,
	ROAM_REASON_LOW_RSSI,
	ROAM_REASON_SUITABLE_AP,
	ROAM_REASON_HO_FAILED,
	ROAM_REASON_INVOKE_ROAM_FAIL,
	ROAM_REASON_RSO_STATUS,
	ROAM_REASON_BTM,
	ROAM_REASON_DEAUTH,
};

/*
 * struct roam_blacklist_timeout - BTM blacklist entry
 * @bssid: bssid that is to be blacklisted
 * @timeout: time duration for which the bssid is blacklisted
 * @received_time: boot timestamp at which the firmware event was received
 * @rssi: rssi value for which the bssid is blacklisted
 * @reject_reason: reason to add the BSSID to BLM
 * @original_timeout: original timeout sent by the AP
 * @source: Source of adding the BSSID to BLM
 */
struct roam_blacklist_timeout {
	struct qdf_mac_addr bssid;
	uint32_t timeout;
	qdf_time_t received_time;
	int32_t rssi;
	enum blm_reject_ap_reason reject_reason;
	uint32_t original_timeout;
	enum blm_reject_ap_source source;
};

/*
 * struct roam_blacklist_event - Blacklist event entries destination structure
 * @vdev_id: vdev id
 * @num_entries: total entries sent over the event
 * @roam_blacklist: blacklist details
 */
struct roam_blacklist_event {
	uint8_t vdev_id;
	uint32_t num_entries;
	struct roam_blacklist_timeout roam_blacklist[];
};

/*
 * enum cm_vdev_disconnect_reason - Roam disconnect reason
 * @CM_DISCONNECT_REASON_CSA_SA_QUERY_TIMEOUT: Disconnect due to SA query
 *  timeout after moving to new channel due to CSA in OCV enabled case.
 * @CM_DISCONNECT_REASON_MOVE_TO_CELLULAR: Disconnect from WiFi to move
 *  to cellular
 */
enum cm_vdev_disconnect_reason {
	CM_DISCONNECT_REASON_CSA_SA_QUERY_TIMEOUT = 1,
	CM_DISCONNECT_REASON_MOVE_TO_CELLULAR,
};

/*
 * struct vdev_disconnect_event_data - Roam disconnect event data
 * @vdev_id: vdev id
 * @psoc: psoc object
 * @reason: roam reason of type @enum cm_vdev_disconnect_reason
 */
struct vdev_disconnect_event_data {
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	enum cm_vdev_disconnect_reason reason;
};

/**
 * struct roam_scan_ch_resp - roam scan chan list response to userspace
 * @vdev_id: vdev id
 * @num_channels: number of roam scan channels
 * @command_resp: command response or async event
 * @chan_list: list of roam scan channels
 */
struct cm_roam_scan_ch_resp {
	uint16_t vdev_id;
	uint16_t num_channels;
	uint32_t command_resp;
	uint32_t *chan_list;
};

/**
 * enum roam_dispatcher_events - Roam events to post to scheduler thread
 * @ROAM_EVENT_INVALID: Invalid event
 * @ROAM_PMKID_REQ_EVENT: Roam pmkid request event
 * @ROAM_VDEV_DISCONNECT_EVENT: Roam disconnect event
 */
enum roam_dispatcher_events {
	ROAM_EVENT_INVALID,
	ROAM_PMKID_REQ_EVENT,
	ROAM_VDEV_DISCONNECT_EVENT,
};

/**
 * struct roam_offload_roam_event: Data carried by roam event
 * @vdev_id: vdev id
 * @psoc: psoc object
 * @reason: reason for roam event of type @enum roam_reason
 * @rssi: associated AP's rssi calculated by FW when reason code
 *	  is WMI_ROAM_REASON_LOW_RSSI
 * @notif: roam notification
 * @notif_params: Contains roam invoke fail reason from wmi_roam_invoke_error_t
 *		  if reason is WMI_ROAM_REASON_INVOKE_ROAM_FAIL.
 * @notif_params1: notif_params1 is exact frame length of deauth or disassoc if
 *		   reason is WMI_ROAM_REASON_DEAUTH.
 * @hw_mode_trans_ind: HW mode transition indication
 * @deauth_disassoc_frame: Deauth/disassoc frame received from AP
 */
struct roam_offload_roam_event {
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	enum roam_reason reason;
	uint32_t rssi;
	enum cm_roam_notif notif;
	uint32_t notif_params;
	uint32_t notif_params1;
	struct cm_hw_mode_trans_ind *hw_mode_trans_ind;
	uint8_t *deauth_disassoc_frame;
	bool rso_timer_stopped;
};

/**
 * struct roam_frame_stats  - Roam frame stats
 * @num_frame: number of frames
 * @frame_info: Roam frame info
 */
struct roam_frame_stats {
	uint8_t num_frame;
	struct roam_frame_info frame_info[WLAN_ROAM_MAX_FRAME_INFO];
};

/**
 * struct roam_stats_event - Data carried by stats event
 * @vdev_id: vdev id
 * @num_tlv: Number of roam scans triggered
 * @num_roam_msg_info: Number of roam_msg_info present in event
 * @trigger: Roam trigger related details
 * @scan: Roam scan event details
 * @result: Roam result related info
 * @frame_stats: Info on frame exchange during roaming
 * @data_11kv: Neighbor report/BTM request related data
 * @btm_rsp: BTM response related data
 * @roam_init_info: Roam initial related data
 * @roam_msg_info: Roam message related information
 * @roam_event_param: Roam event notif params
 */
struct roam_stats_event {
	uint8_t vdev_id;
	uint8_t num_tlv;
	uint8_t num_roam_msg_info;
	struct wmi_roam_trigger_info trigger[MAX_ROAM_SCAN_STATS_TLV];
	struct wmi_roam_scan_data scan[MAX_ROAM_SCAN_STATS_TLV];
	struct wmi_roam_result result[MAX_ROAM_SCAN_STATS_TLV];
	struct roam_frame_stats frame_stats[MAX_ROAM_SCAN_STATS_TLV];
	struct wmi_neighbor_report_data data_11kv[MAX_ROAM_SCAN_STATS_TLV];
	struct roam_btm_response_data btm_rsp[MAX_ROAM_SCAN_STATS_TLV];
	struct roam_initial_data roam_init_info[MAX_ROAM_SCAN_STATS_TLV];
	struct roam_msg_info *roam_msg_info;
	struct roam_event_rt_info roam_event_param;
};

/*
 * struct auth_offload_event - offload data carried by roam event
 * @vdev_id: vdev id
 * @ap_bssid: SAE authentication offload MAC Addess
 */
struct auth_offload_event {
	uint8_t vdev_id;
	struct qdf_mac_addr ap_bssid;
};

/*
 * struct roam_pmkid_req_event - Pmkid event with entries destination structure
 * @vdev_id: VDEV id
 * @psoc: psoc object
 * @num_entries: total entries sent over the event
 * @ap_bssid: bssid list
 */
struct roam_pmkid_req_event {
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	uint32_t num_entries;
	struct qdf_mac_addr ap_bssid[];
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
 * @send_roam_disable_config: send roam disable config
 * @send_roam_rt_stats_config: Send roam events vendor command param value to FW
 * @send_roam_ho_delay_config: Send roam Hand-off delay value to FW
 * @send_exclude_rm_partial_scan_freq: Include/exclude the channels in roam full
 * scan that are already scanned as part of partial scan.
 * @send_roam_full_scan_6ghz_on_disc: Include the 6 GHz channels in roam full
 * scan only on prior discovery of any 6 GHz support in the environment.
 * @send_roam_scan_offload_rssi_params: Set the RSSI parameters for roam
 * offload scan
 * @send_roam_idle_trigger: Send roam idle params to FW
 */
struct wlan_cm_roam_tx_ops {
	QDF_STATUS (*send_vdev_set_pcl_cmd)(struct wlan_objmgr_vdev *vdev,
					    struct set_pcl_req *req);
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
	QDF_STATUS (*send_roam_triggers)(struct wlan_objmgr_vdev *vdev,
					 struct wlan_roam_triggers *req);
	QDF_STATUS (*send_roam_disable_config)(struct wlan_objmgr_vdev *vdev,
				struct roam_disable_cfg *req);
	QDF_STATUS (*send_roam_invoke_cmd)(struct wlan_objmgr_vdev *vdev,
					   struct roam_invoke_req *req);
	QDF_STATUS (*send_roam_sync_complete_cmd)(struct wlan_objmgr_vdev *vdev);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	QDF_STATUS (*send_roam_rt_stats_config)(struct wlan_objmgr_vdev *vdev,
						uint8_t vdev_id, uint8_t value);
	QDF_STATUS (*send_roam_ho_delay_config)(struct wlan_objmgr_vdev *vdev,
						uint8_t vdev_id,
						uint16_t value);
	QDF_STATUS (*send_exclude_rm_partial_scan_freq)(
						struct wlan_objmgr_vdev *vdev,
						uint8_t value);
	QDF_STATUS (*send_roam_full_scan_6ghz_on_disc)(
						struct wlan_objmgr_vdev *vdev,
						uint8_t value);
	QDF_STATUS (*send_roam_scan_offload_rssi_params)(
		struct wlan_objmgr_vdev *vdev,
		struct wlan_roam_offload_scan_rssi_params *roam_rssi_params);
#endif
	QDF_STATUS (*send_roam_idle_trigger)(wmi_unified_t wmi_handle,
					     uint8_t command,
					     struct wlan_roam_idle_params *req);
};

/**
 * enum roam_scan_freq_scheme - Scan mode for triggering roam
 * ROAM_SCAN_FREQ_SCHEME_NO_SCAN: Indicates the fw to not scan.
 * ROAM_SCAN_FREQ_SCHEME_PARTIAL_SCAN: Indicates the firmware to
 * trigger partial frequency scans.
 * ROAM_SCAN_FREQ_SCHEME_FULL_SCAN: Indicates the firmware to
 * trigger full frequency scans.
 * ROAM_SCAN_FREQ_SCHEME_NONE: Invalid scan mode
 */
enum roam_scan_freq_scheme {
	ROAM_SCAN_FREQ_SCHEME_NO_SCAN = 0,
	ROAM_SCAN_FREQ_SCHEME_PARTIAL_SCAN = 1,
	ROAM_SCAN_FREQ_SCHEME_FULL_SCAN = 2,
	ROAM_SCAN_FREQ_SCHEME_NONE = 3,
};

/**
 * struct wlan_cm_roam  - Connection manager roam configs, state and roam
 * data related structure
 * @pcl_vdev_cmd_active:  Flag to check if vdev level pcl command needs to be
 * sent or PDEV level PCL command needs to be sent
 */
struct wlan_cm_roam {
	bool pcl_vdev_cmd_active;
};

/**
 * struct cm_roam_values_copy  - Structure for values copy buffer
 * @uint_value: Unsigned integer value to be copied
 * @int_value: Integer value
 * @bool_value: boolean value
 * @chan_info: chan info
 */
struct cm_roam_values_copy {
	uint32_t uint_value;
	int32_t int_value;
	bool bool_value;
	struct rso_chan_info chan_info;
};

#ifdef FEATURE_LFR_SUBNET_DETECTION
/* bit-4 and bit-5 indicate the subnet status */
#define CM_GET_SUBNET_STATUS(roam_reason) (((roam_reason) & 0x30) >> 4)
#else
#define CM_GET_SUBNET_STATUS(roam_reason) (0)
#endif

/* This should not be greater than MAX_NUMBER_OF_CONC_CONNECTIONS */
#define MAX_VDEV_SUPPORTED 4
#define MAX_PN_LEN 8
#define MAX_KEY_LEN 32

/* MAX_FREQ_RANGE_NUM shouldn't exceed as only in case od SBS there will be 3
 * frequency ranges, For DBS, it will be 2. For SMM, it will be 1
 */
#define MAX_FREQ_RANGE_NUM 3

/**
 * struct cm_ho_fail_ind - ho fail indication to CM
 * @vdev_id: vdev id
 * @psoc: psoc object
 * @bssid: bssid addr
 */
struct cm_ho_fail_ind {
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	struct qdf_mac_addr bssid;
};

/**
 * struct policy_mgr_vdev_mac_map - vdev id-mac id map
 * @vdev_id: VDEV id
 * @mac_id: MAC id
 */
struct policy_mgr_vdev_mac_map {
	uint32_t vdev_id;
	uint32_t mac_id;
};

/**
 * struct policy_mgr_pdev_mac_freq_map - vdev id-mac id map
 * @mac_id: mac_id mapped to pdev id (macros starting with WMI_PDEV_ID_)
 * @start_freq: Start Frequency in Mhz
 * @end_freq: End Frequency in Mhz
 */
struct policy_mgr_pdev_mac_freq_map {
	uint32_t mac_id;
	qdf_freq_t start_freq;
	qdf_freq_t end_freq;
};

/**
 * struct cm_hw_mode_trans_ind - HW mode transition indication
 * @old_hw_mode_index: Index of old HW mode
 * @new_hw_mode_index: Index of new HW mode
 * @num_vdev_mac_entries: Number of vdev-mac id entries
 * @vdev_mac_map: vdev id-mac id map
 */
struct cm_hw_mode_trans_ind {
	uint32_t old_hw_mode_index;
	uint32_t new_hw_mode_index;
	uint32_t num_vdev_mac_entries;
	struct policy_mgr_vdev_mac_map vdev_mac_map[MAX_VDEV_SUPPORTED];
	uint32_t num_freq_map;
	struct policy_mgr_pdev_mac_freq_map mac_freq_map[MAX_FREQ_RANGE_NUM];
};

/*
 * struct ml_setup_link_param - MLO setup link param
 * @vdev_id: vdev id of the link
 * @link_id: link id of the link
 * @channel: wmi channel
 * @flags: link flags
 * @link_addr: link mac addr
 */
struct ml_setup_link_param {
	uint32_t vdev_id;
	uint32_t link_id;
	wmi_channel channel;
	uint32_t flags;
	struct qdf_mac_addr link_addr;
};

/*
 * struct ml_key_material_param - MLO key material param
 * @link_id: key is for which link, when link_id is 0xf,
 * means the key is used for all links, like PTK
 * @key_idx: key idx
 * @key_cipher: key cipher
 * @pn: pn
 * @key_buff: key buffer
 */
struct ml_key_material_param {
	uint32_t link_id;
	uint32_t key_idx;
	uint32_t key_cipher;
	uint8_t pn[MAX_PN_LEN];
	uint8_t key_buff[MAX_KEY_LEN];
};

struct roam_offload_synch_ind {
	uint16_t beaconProbeRespOffset;
	uint16_t beaconProbeRespLength;
	uint16_t reassocRespOffset;
	uint16_t reassocRespLength;
	uint16_t reassoc_req_offset;
	uint16_t reassoc_req_length;
	uint8_t isBeacon;
	uint8_t roamed_vdev_id;
	struct qdf_mac_addr bssid;
	struct wlan_ssid ssid;
	struct qdf_mac_addr self_mac;
	int8_t txMgmtPower;
	uint32_t auth_status;
	uint8_t rssi;
	uint8_t roam_reason;
	uint32_t chan_freq;
	uint8_t kck[MAX_KCK_LEN];
	uint8_t kck_len;
	uint32_t kek_len;
	uint8_t kek[MAX_KEK_LENGTH];
	uint32_t   pmk_len;
	uint8_t    pmk[MAX_PMK_LEN];
	uint8_t    pmkid[PMKID_LEN];
	bool update_erp_next_seq_num;
	uint16_t next_erp_seq_num;
	uint8_t replay_ctr[REPLAY_CTR_LEN];
	void *add_bss_params;
	enum phy_ch_width chan_width;
	uint32_t max_rate_flags;
	uint32_t ric_data_len;
#ifdef FEATURE_WLAN_ESE
	uint32_t tspec_len;
#endif
	uint8_t *ric_tspec_data;
	uint16_t aid;
	bool hw_mode_trans_present;
	struct cm_hw_mode_trans_ind hw_mode_trans_ind;
	uint8_t nss;
	struct qdf_mac_addr dst_mac;
	struct qdf_mac_addr src_mac;
	uint16_t hlp_data_len;
	uint8_t hlp_data[FILS_MAX_HLP_DATA_LEN];
	bool is_ft_im_roam;
	uint8_t is_assoc;
	enum wlan_phymode phy_mode; /*phy mode sent by fw */
	wmi_channel chan;
#ifdef WLAN_FEATURE_11BE_MLO
	uint8_t num_setup_links;
	struct ml_setup_link_param ml_link[WLAN_UMAC_MLO_MAX_VDEVS];
	uint8_t num_ml_key_material;
	struct ml_key_material_param ml_key[WLAN_UMAC_MLO_MAX_VDEVS];
#endif
};

/*
 * struct roam_scan_candidate_frame Roam candidate scan entry
 * vdev_id : vdev id
 * frame_len : Length of the beacon/probe rsp frame
 * frame : Pointer to the frame
 */
struct roam_scan_candidate_frame {
	uint8_t vdev_id;
	uint32_t frame_length;
	uint8_t *frame;
};

/**
 * wlan_cm_roam_rx_ops  - structure of rx function pointers for
 * roaming related commands
 * @roam_sync_event: RX ops function pointer for roam sync event
 * @roam_sync_frame_event: Rx ops function pointer for roam sync frame event
 * @roam_event_rx: Rx ops function pointer for roam info event
 * @btm_blacklist_event: Rx ops function pointer for btm blacklist event
 * @vdev_disconnect_event: Rx ops function pointer for vdev disconnect event
 * @roam_scan_chan_list_event: Rx ops function pointer for roam scan ch event
 * @roam_stats_event_rx: Rx ops function pointer for roam stats event
 * @roam_auth_offload_event: Rx ops function pointer for auth offload event
 * @roam_pmkid_request_event_rx: Rx ops function pointer for roam pmkid event
 * @roam_candidate_frame_event : Rx ops function pointer for roam frame event
 */
struct wlan_cm_roam_rx_ops {
	QDF_STATUS (*roam_sync_event)(struct wlan_objmgr_psoc *psoc,
				      uint8_t *event,
				      uint32_t len,
				      struct roam_offload_synch_ind *sync_ind);
	QDF_STATUS (*roam_sync_frame_event)(struct wlan_objmgr_psoc *psoc,
					    struct roam_synch_frame_ind *frm);
	QDF_STATUS (*roam_event_rx)(struct roam_offload_roam_event *roam_event);
	QDF_STATUS (*btm_blacklist_event)(struct wlan_objmgr_psoc *psoc,
					  struct roam_blacklist_event *list);
	QDF_STATUS
	(*vdev_disconnect_event)(struct vdev_disconnect_event_data *data);
	QDF_STATUS
	(*roam_scan_chan_list_event)(struct cm_roam_scan_ch_resp *data);
	QDF_STATUS
	(*roam_stats_event_rx)(struct wlan_objmgr_psoc *psoc,
			       struct roam_stats_event *stats_info);
	QDF_STATUS
	(*roam_auth_offload_event)(struct auth_offload_event *auth_event);
	QDF_STATUS
	(*roam_pmkid_request_event_rx)(struct roam_pmkid_req_event *list);
	QDF_STATUS
	(*roam_candidate_frame_event)(struct wlan_objmgr_psoc *psoc,
				      struct roam_scan_candidate_frame *frame);
};
#endif

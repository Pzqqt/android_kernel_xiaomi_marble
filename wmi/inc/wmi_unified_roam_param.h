/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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
 * This file contains the API definitions for the ROAMING WMI APIs.
 */

#ifndef _WMI_UNIFIED_ROAM_PARAM_H_
#define _WMI_UNIFIED_ROAM_PARAM_H_

#include <wlan_blm_public_struct.h>

/**
 * struct gateway_update_req_param - gateway parameter update request
 * @request_id: request id
 * @vdev_id: vdev id
 * @max_retries: Max ARP/NS retry attempts
 * @timeout: Retry interval
 * @ipv4_addr_type: on ipv4 network
 * @ipv6_addr_type: on ipv6 network
 * @gw_mac_addr: gateway mac addr
 * @ipv4_addr: ipv4 addr
 * @ipv6_addr: ipv6 addr
 */
struct gateway_update_req_param {
	uint32_t     request_id;
	uint32_t     vdev_id;
	uint32_t     max_retries;
	uint32_t     timeout;
	uint32_t     ipv4_addr_type;
	uint32_t     ipv6_addr_type;
	struct qdf_mac_addr  gw_mac_addr;
	uint8_t      ipv4_addr[QDF_IPV4_ADDR_SIZE];
	uint8_t      ipv6_addr[QDF_IPV6_ADDR_SIZE];
};

/**
 * struct rssi_monitor_param - rssi monitoring
 * @request_id: request id
 * @vdev_id: vdev id
 * @min_rssi: minimum rssi
 * @max_rssi: maximum rssi
 * @control: flag to indicate start or stop
 */
struct rssi_monitor_param {
	uint32_t request_id;
	uint32_t vdev_id;
	int8_t   min_rssi;
	int8_t   max_rssi;
	bool     control;
};

/**
 * struct roam_offload_scan_rssi_params - structure containing
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
 * @flags: Flags for Background Roaming
 *	Bit 0 : BG roaming enabled when we connect to 2G AP only and roaming
 *	        to 5G AP only.
 *	Bit 1-31: Reserved
 */
struct roam_offload_scan_rssi_params {
	int8_t rssi_thresh;
	uint8_t rssi_thresh_diff;
	uint32_t hi_rssi_scan_max_count;
	uint32_t hi_rssi_scan_rssi_delta;
	int32_t hi_rssi_scan_rssi_ub;
	int raise_rssi_thresh_5g;
	uint8_t vdev_id;
	uint32_t penalty_threshold_5g;
	uint32_t boost_threshold_5g;
	uint8_t raise_factor_5g;
	uint8_t drop_factor_5g;
	int max_raise_rssi_5g;
	int max_drop_rssi_5g;
	uint32_t good_rssi_threshold;
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
	uint32_t flags;
};

/**
 * struct roam_scan_period_params - Roam scan period parameters
 * @vdev_id: Vdev for which the scan period parameters are sent
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
struct roam_scan_period_params {
	uint32_t vdev_id;
	uint32_t scan_period;
	uint32_t scan_age;
	uint32_t roam_scan_inactivity_time;
	uint32_t roam_inactive_data_packet_count;
	uint32_t roam_scan_period_after_inactivity;
	uint32_t full_scan_period;
};

/**
 * struct wmi_mawc_roam_params - Motion Aided wireless connectivity params
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: MAWC roaming feature enable/disable
 * @traffic_load_threshold: Traffic threshold in kBps for MAWC roaming
 * @best_ap_rssi_threshold: AP RSSI Threshold for MAWC roaming
 * @rssi_stationary_high_adjust: High RSSI adjustment value to suppress scan
 * @rssi_stationary_low_adjust: Low RSSI adjustment value to suppress scan
 */
struct wmi_mawc_roam_params {
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
	struct mac_ssid ssid_allowed_list[MAX_SSID_ALLOWED_LIST];
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

#define WMI_CFG_VALID_CHANNEL_LIST_LEN    100
/* Occupied channel list remains static */
#define WMI_CHANNEL_LIST_STATIC                   1
/* Occupied channel list can be learnt after init */
#define WMI_CHANNEL_LIST_DYNAMIC_INIT             2
/* Occupied channel list can be learnt after flush */
#define WMI_CHANNEL_LIST_DYNAMIC_FLUSH            3
/* Occupied channel list can be learnt after update */
#define WMI_CHANNEL_LIST_DYNAMIC_UPDATE           4

/**
 * struct plm_req_params - plm req parameter
 * @diag_token: Dialog token
 * @meas_token: measurement token
 * @num_bursts: total number of bursts
 * @burst_int: burst interval in seconds
 * @meas_duration:in TU's,STA goes off-ch
 * @burst_len: no of times the STA should cycle through PLM ch list
 * @desired_tx_pwr: desired tx power
 * @mac_addr: MC dest addr
 * @plm_num_ch: channel numbers
 * @plm_ch_freq_list: channel frequency list
 * @vdev_id: vdev id
 * @enable:  enable/disable
 */
struct plm_req_params {
	uint16_t diag_token;
	uint16_t meas_token;
	uint16_t num_bursts;
	uint16_t burst_int;
	uint16_t meas_duration;
	/* no of times the STA should cycle through PLM ch list */
	uint8_t burst_len;
	int8_t desired_tx_pwr;
	struct qdf_mac_addr mac_addr;
	/* no of channels */
	uint8_t plm_num_ch;
	/* channel frequency list */
	uint32_t plm_ch_freq_list[WMI_CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t vdev_id;
	bool enable;
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
	struct mac_ssid  ssid;
	uint32_t rsn_authmode;
	uint32_t rsn_ucastcipherset;
	uint32_t rsn_mcastcipherset;
	uint32_t rsn_mcastmgmtcipherset;
	uint32_t rssi_abs_thresh;
};

/**
 * struct rssi_scoring - rssi scoring param to sortlist selected AP
 * @best_rssi_threshold: Roamable AP RSSI equal or better than this threshold,
 *                      full rssi score 100. Units in dBm.
 * @good_rssi_threshold: Below threshold, scoring linear percentage between
 *                      rssi_good_pnt and 100. Units in dBm.
 * @bad_rssi_threshold: Between good and bad rssi threshold, scoring linear
 *                      % between rssi_bad_pcnt and rssi_good_pct in dBm.
 * @good_rssi_pcnt: Used to assigned scoring percentage of each slot between
 *                 best to good rssi threshold. Units in percentage.
 * @bad_rssi_pcnt: Used to assigned scoring percentage of each slot between good
 *                to bad rssi threshold. Unites in percentage.
 * @good_bucket_size : bucket size of slot in good zone
 * @bad_bucket_size : bucket size of slot in bad zone
 * @rssi_pref_5g_rssi_thresh: Below rssi threshold, 5G AP have given preference
 *                           of band percentage. Units in dBm.
 */
struct rssi_scoring {
	int32_t best_rssi_threshold;
	int32_t good_rssi_threshold;
	int32_t  bad_rssi_threshold;
	uint32_t good_rssi_pcnt;
	uint32_t bad_rssi_pcnt;
	uint32_t good_bucket_size;
	uint32_t bad_bucket_size;
	int32_t  rssi_pref_5g_rssi_thresh;
};

/**
 * struct param_slot_scoring - define % score for differents slots for a
 *                             scoring param.
 * @num_slot: number of slots in which the param will be divided.
 *           Max 15. index 0 is used for 'not_present. Num_slot will
 *           equally divide 100. e.g, if num_slot = 4 slot 0 = 0-25%, slot
 *           1 = 26-50% slot 2 = 51-75%, slot 3 = 76-100%
 * @score_pcnt3_to_0: Conatins score percentage for slot 0-3
 *             BITS 0-7   :- the scoring pcnt when not present
 *             BITS 8-15  :- SLOT_1
 *             BITS 16-23 :- SLOT_2
 *             BITS 24-31 :- SLOT_3
 * @score_pcnt7_to_4: Conatins score percentage for slot 4-7
 *             BITS 0-7   :- SLOT_4
 *             BITS 8-15  :- SLOT_5
 *             BITS 16-23 :- SLOT_6
 *             BITS 24-31 :- SLOT_7
 * @score_pcnt11_to_8: Conatins score percentage for slot 8-11
 *             BITS 0-7   :- SLOT_8
 *             BITS 8-15  :- SLOT_9
 *             BITS 16-23 :- SLOT_10
 *             BITS 24-31 :- SLOT_11
 * @score_pcnt15_to_12: Conatins score percentage for slot 12-15
 *             BITS 0-7   :- SLOT_12
 *             BITS 8-15  :- SLOT_13
 *             BITS 16-23 :- SLOT_14
 *             BITS 24-31 :- SLOT_15
 */
struct param_slot_scoring {
	uint32_t num_slot;
	uint32_t score_pcnt3_to_0;
	uint32_t score_pcnt7_to_4;
	uint32_t score_pcnt11_to_8;
	uint32_t score_pcnt15_to_12;
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
 * @vendor_roam_score_algorithm: Prefered algorithm for roam candidate selection
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
	uint32_t bw_index_score;
	uint32_t band_index_score;
	uint32_t nss_index_score;
	uint32_t roam_score_delta;
	uint32_t roam_trigger_bitmap;
	uint32_t vendor_roam_score_algorithm;
	uint32_t cand_min_roam_score_delta;
	struct rssi_scoring rssi_scoring;
	struct param_slot_scoring esp_qbss_scoring;
	struct param_slot_scoring oce_wan_scoring;
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
 * struct wmi_roam_invoke_cmd - roam invoke command
 * @vdev_id: vdev id
 * @bssid: mac address
 * @channel: channel
 * @frame_len: frame length, includs mac header, fixed params and ies
 * @frame_buf: buffer contaning probe response or beacon
 * @is_same_bssid: flag to indicate if roaming is requested for same bssid
 * @forced_roaming: Roam to any bssid in any ch (here bssid & ch is not given)
 */
struct wmi_roam_invoke_cmd {
	uint32_t vdev_id;
	uint8_t bssid[QDF_MAC_ADDR_SIZE];
	uint32_t channel;
	uint32_t frame_len;
	uint8_t *frame_buf;
	uint8_t is_same_bssid;
	bool forced_roaming;
};

/**
 * struct wmi_per_roam_config - per based roaming parameters
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
struct wmi_per_roam_config {
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
 * struct wmi_per_roam_config_req: PER based roaming config request
 * @vdev_id: vdev id on which config needs to be set
 * @per_config: PER config
 */
struct wmi_per_roam_config_req {
	uint8_t vdev_id;
	struct wmi_per_roam_config per_config;
};

/**
 * struct wmi_limit_off_chan_param - limit off channel parameters
 * @vdev_id: vdev id
 * @status: status of the command (enable/disable)
 * @max_offchan_time: max off channel time
 * @rest_time: home channel time
 * @skip_dfs_chans: skip dfs channels during scan
 */
struct wmi_limit_off_chan_param {
	uint32_t vdev_id;
	bool status;
	uint32_t max_offchan_time;
	uint32_t rest_time;
	bool skip_dfs_chans;
};

#define WMI_MAX_HLP_IE_LEN 2048
/**
 * struct hlp_params - HLP info params
 * @vdev_id: vdev id
 * @hlp_ie_len: HLP IE length
 * @hlp_ie: HLP IE
 */
struct hlp_params {
	uint8_t vdev_id;
	uint32_t  hlp_ie_len;
	uint8_t hlp_ie[WMI_MAX_HLP_IE_LEN];
};

/**
 * struct wmi_btm_config - BSS Transition Management offload params
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
struct wmi_btm_config {
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
 * struct wmi_bss_load_config - BSS load trigger parameters
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
struct wmi_bss_load_config {
	uint32_t vdev_id;
	uint32_t bss_load_threshold;
	uint32_t bss_load_sample_time;
	int32_t rssi_threshold_5ghz;
	int32_t rssi_threshold_24ghz;
};

/**
 * struct wmi_idle_roam_params - Idle roam trigger parameters
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: Enable/Disable Idle roaming
 * @band: Connected AP band
 * @conn_ap_rssi_delta: Rssi change of connected AP in dBm
 * @conn_ap_min_rssi: If connected AP rssi is less than min rssi trigger roam
 * @inactive_time: Connected AP idle time
 * @data_pkt_count: Data packet count allowed during idle time
 */
struct wmi_idle_roam_params {
	uint32_t vdev_id;
	bool enable;
	uint32_t band;
	uint32_t conn_ap_rssi_delta;
	int32_t conn_ap_min_rssi;
	uint32_t inactive_time;
	uint32_t data_pkt_count;
};

/**
 * struct wmi_disconnect_roam_params - Emergency deauth/disconnect roam params
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: Enable or disable disconnect roaming.
 */
struct wmi_disconnect_roam_params {
	uint32_t vdev_id;
	bool enable;
};

/**
 * struct wmi_roam_auth_status_params - WPA3 roam auth response status
 * parameters
 * @vdev_id: Vdev on which roam preauth is happening
 * @preauth_status: Status of the Auth response.
 *      IEEE80211_STATUS_SUCCESS(0) for success. Corresponding
 *      IEEE80211 failure status code for failure.
 *
 * @bssid: Candidate BSSID
 * @pmkid: PMKID derived for the auth
 */
struct wmi_roam_auth_status_params {
	uint32_t vdev_id;
	uint32_t preauth_status;
	struct qdf_mac_addr bssid;
	uint8_t pmkid[PMKID_LEN];
};

/**
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
struct wmi_11k_offload_neighbor_report_params {
	uint32_t time_offset;
	uint32_t low_rssi_offset;
	uint32_t bmiss_count_trigger;
	uint32_t per_threshold_offset;
	uint32_t neighbor_report_cache_timeout;
	uint32_t max_neighbor_report_req_cap;
	struct mac_ssid ssid;
};

/**
 * struct wmi_11k_offload_params - offload 11k features to FW
 * @vdev_id: vdev id
 * @offload_11k_bitmask: bitmask to specify offloaded features
 *	B0: Neighbor Report Request offload
 *	B1-B31: Reserved
 * @neighbor_report_params: neighbor report offload params
 */
struct wmi_11k_offload_params {
	uint32_t vdev_id;
	uint32_t offload_11k_bitmask;
	struct wmi_11k_offload_neighbor_report_params neighbor_report_params;
};

/**
 * struct wmi_invoke_neighbor_report_params - Invoke neighbor report request
 *	from IW to FW
 * @vdev_id: vdev id
 * @send_resp_to_host: bool to send response to host or not
 * @ssid: ssid given from the IW command
 */
struct wmi_invoke_neighbor_report_params {
	uint32_t vdev_id;
	uint32_t send_resp_to_host;
	struct mac_ssid ssid;
};

/**
 * struct roam_triggers - vendor configured roam triggers
 * @vdev_id: vdev id
 * @trigger_bitmap: vendor configured roam trigger bitmap as
 *		    defined @enum roam_control_trigger_reason
 */
struct roam_triggers {
	uint32_t vdev_id;
	uint32_t trigger_bitmap;
};

#endif /* _WMI_UNIFIED_ROAM_PARAM_H_ */

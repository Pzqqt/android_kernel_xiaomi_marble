/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan init/deinit public api
 */

#ifndef _WLAN_SCAN_MAIN_API_H_
#define _WLAN_SCAN_MAIN_API_H_

#include <qdf_atomic.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>
#include "wlan_scan_cache_db.h"
#include "wlan_scan_11d.h"
#include "cfg_scan.h"

#define scm_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_SCAN, params)
#define scm_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_SCAN, params)
#define scm_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_SCAN, params)
#define scm_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_SCAN, params)
#define scm_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_SCAN, params)
#define scm_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_SCAN, params)

/* Rate Limited Logs */
#define scm_alert_rl(params...) \
	QDF_TRACE_FATAL_RL(QDF_MODULE_ID_SCAN, params)
#define scm_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_SCAN, params)
#define scm_warn_rl(params...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_SCAN, params)
#define scm_info_rl(params...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_SCAN, params)
#define scm_debug_rl(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_SCAN, params)

#define scm_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_SCAN, params)
#define scm_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SCAN, params)
#define scm_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_SCAN, params)
#define scm_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_SCAN, params)
#define scm_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_SCAN, params)

#define scm_hex_dump(level, data, buf_len) \
		qdf_trace_hex_dump(QDF_MODULE_ID_SCAN, level, data, buf_len)

#define MAX_SCAN_EVENT_HANDLERS_PER_PDEV   100
#define WLAN_MAX_MODULE_NAME    40
#define WLAN_MAX_REQUESTORS     200
#define WLAN_SCAN_ID_MASK 0x00000FFF
#define WLAN_HOST_SCAN_REQ_ID_PREFIX 0x0000A000
#define SCAN_NPROBES_DEFAULT 2
#define WLAN_P2P_SOCIAL_CHANNELS 3

#define SCAN_BURST_SCAN_MAX_NUM_OFFCHANNELS  (3)
#define SCAN_SCAN_IDLE_TIME_DEFAULT          (25)
#define SCAN_3PORT_CONC_SCAN_MAX_BURST_DURATION  (25)
#define SCAN_CTS_DURATION_MS_MAX             (32)
#define SCAN_ROAM_SCAN_CHANNEL_SWITCH_TIME    (4)
#define SCAN_DWELL_TIME_PROBE_TIME_MAP_SIZE      (11)
#define SCAN_GO_MIN_ACTIVE_SCAN_BURST_DURATION   (40)
#define SCAN_GO_MAX_ACTIVE_SCAN_BURST_DURATION   (240)
#define SCAN_P2P_SCAN_MAX_BURST_DURATION     (240)
#define SCAN_GO_BURST_SCAN_MAX_NUM_OFFCHANNELS   (6)

/* MAX RNR entries per channel*/
#define WLAN_MAX_RNR_COUNT 15

/*
 * Maximum numbers of callback functions that may be invoked
 * for a particular scan event.
 */
#define MAX_SCAN_EVENT_LISTENERS (MAX_SCAN_EVENT_HANDLERS_PER_PDEV + 1)

/**
 * struct probe_time_dwell_time - probe time, dwell time map
 * @dwell_time: dwell time
 * @probe_time: repeat probe time
 */
struct probe_time_dwell_time {
	uint8_t dwell_time;
	uint8_t probe_time;
};

/*
 * For the requestor id:
 *     bit  0~12 is used for real requestor id.
 *     bit 13~15 is used for requestor prefix.
 *     bit 16~19 is used by specific user to aware it is issued by himself.
 *     bit 20~31 is reserved.
 */
#define WLAN_SCAN_REQUESTER_ID_PREFIX 0x0000A000
#define WLAN_SCAN_REQUESTER_ID_MASK 0x00001FFF

#define SCM_NUM_RSSI_CAT        15
#define SCAN_STA_MIRACAST_MCC_REST_TIME 400

#define SCAN_TIMEOUT_GRACE_PERIOD 10
#define SCAN_MAX_BSS_PDEV 100
#define SCAN_PRIORITY SCAN_PRIORITY_LOW

/* DBS Scan policy selection ext flags */
#define SCAN_FLAG_EXT_DBS_SCAN_POLICY_MASK  0x00000003
#define SCAN_FLAG_EXT_DBS_SCAN_POLICY_BIT   0
#define SCAN_DBS_POLICY_DEFAULT             0x0
#define SCAN_DBS_POLICY_FORCE_NONDBS        0x1
#define SCAN_DBS_POLICY_IGNORE_DUTY         0x2
#define SCAN_DBS_POLICY_MAX                 0x3
/* Minimum number of channels for enabling DBS Scan */
#define SCAN_MIN_CHAN_DBS_SCAN_THRESHOLD         8
/*
 * Enable Reception of Public Action frame with this flag
 */
#define SCAN_FLAG_EXT_FILTER_PUBLIC_ACTION_FRAME 0x4

/* Indicate to scan all PSC channel */
#define SCAN_FLAG_EXT_6GHZ_SCAN_ALL_PSC_CH 0x8

/* Indicate to scan all NON-PSC channel */
#define SCAN_FLAG_EXT_6GHZ_SCAN_ALL_NON_PSC_CH 0x10

/* Indicate to save scan result matching hint from scan client */
#define SCAN_FLAG_EXT_6GHZ_MATCH_HINT 0x20

/* Skip any channel on which RNR information is not received */
#define SCAN_FLAG_EXT_6GHZ_SKIP_NON_RNR_CH 0x40

/* Indicate client hint req is high priority than FW rnr or FILS discovery */
#define SCAN_FLAG_EXT_6GHZ_CLIENT_HIGH_PRIORITY 0x80

/* RRM scan type indication */
#define SCAN_FLAG_EXT_RRM_SCAN_IND 0x400

/* Probe request frame with unicast RA indication */
#define SCAN_FLAG_EXT_FORCE_UNICAST_RA  0x1000

/* Passive dwell time if bt_a2dp is enabled. Time in msecs*/
#define PASSIVE_DWELL_TIME_BT_A2DP_ENABLED 28

/**
 * struct cb_handler - defines scan event handler
 * call back function and arguments
 * @func: handler function pointer
 * @arg: argument to handler function
 */
struct cb_handler {
	scan_event_handler func;
	void *arg;
};

/**
 * struct pdev_scan_ev_handler - pdev scan event handlers
 * @cb_handler: array of registered scan handlers
 */
struct pdev_scan_ev_handler {
	uint32_t handler_cnt;
	struct cb_handler cb_handlers[MAX_SCAN_EVENT_HANDLERS_PER_PDEV];
};

/**
 * struct global_scan_ev_handlers - per pdev registered scan event handlers
 * @pdev_scan_ev_handler: per pdev registered scan event handlers
 */
struct global_scan_ev_handlers {
	struct pdev_scan_ev_handler pdev_ev_handlers[WLAN_UMAC_MAX_PDEVS];
};

/**
 * struct scan_requester_info - defines scan requester id
 * and event handler mapping
 * @requester:   requester ID allocated
 * @module:      module name of requester
 * @ev_handler:  event handlerto be invoked
 */
struct scan_requester_info {
	wlan_scan_requester requester;
	uint8_t module[WLAN_MAX_MODULE_NAME];
	struct cb_handler ev_handler;
};

/**
 * struct pdev_scan_info - defines per pdev scan info
 * @wide_band_scan: wide band scan capability
 * @last_scan_time: time of last scan start on this pdev
 * @custom_chan_list: scan only these channels
 * @conf_bssid: configured bssid of the hidden AP
 * @conf_ssid: configured desired ssid
 */
struct pdev_scan_info {
	bool wide_band_scan;
	qdf_time_t last_scan_time;
	struct chan_list custom_chan_list;
	uint8_t conf_bssid[QDF_MAC_ADDR_SIZE];
	struct wlan_ssid conf_ssid;
};

/**
 * struct scan_vdev_obj - scan vdev obj
 * @pno_match_evt_received: pno match received
 * @pno_in_progress: pno in progress
 * @scan_disabled: if scan is disabled for this vdev
 * @first_scan_done: Whether its the first scan or not for this particular vdev.
 */
struct scan_vdev_obj {
	bool pno_match_evt_received;
	bool pno_in_progress;
	uint32_t scan_disabled;
	bool first_scan_done;
};

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * struct pno_def_config - def configuration for PNO
 * @pno_offload_enabled: flag to check if PNO offload is enabled/disabled
 * @channel_prediction: config PNO channel prediction feature status
 * @top_k_num_of_channels: def top K number of channels are used for tanimoto
 * distance calculation.
 * @stationary_thresh: def threshold val to determine that STA is stationary.
 * @pnoscan_adaptive_dwell_mode: def adaptive dwelltime mode for pno scan
 * @channel_prediction_full_scan: def periodic timer upon which full scan needs
 * to be triggered.
 * @dfs_chnl_scan_enable: Enable dfs channel PNO scan
 * @scan_support_enabled: PNO scan support enabled/disabled
 * @scan_timer_repeat_value: PNO scan timer repeat value
 * @slow_scan_multiplier: PNO slow scan timer multiplier
 * @max_sched_scan_plan_interval: PNO scan interval
 * @max_sched_scan_plan_iterations: PNO scan number of iterations
 * @scan_backoff_multiplier: Scan banckoff multiplier
 * @pno_wake_lock: pno wake lock
 * @pno_runtime_pm_lock: pno runtime pm lock
 * @pno_cb: callback to call on PNO completion
 * @mawc_params: Configuration parameters for NLO MAWC.
 * @user_config_sched_scan_plan: if enabled set user confing sched scan plan
 */
struct pno_def_config {
	bool pno_offload_enabled;
	bool channel_prediction;
	uint8_t top_k_num_of_channels;
	uint8_t stationary_thresh;
	enum scan_dwelltime_adaptive_mode adaptive_dwell_mode;
	uint32_t channel_prediction_full_scan;
	bool dfs_chnl_scan_enabled;
	bool scan_support_enabled;
	uint32_t scan_timer_repeat_value;
	uint32_t slow_scan_multiplier;
	uint32_t max_sched_scan_plan_interval;
	uint32_t max_sched_scan_plan_iterations;
	uint8_t scan_backoff_multiplier;
	qdf_wake_lock_t pno_wake_lock;
	qdf_runtime_lock_t pno_runtime_pm_lock;
	struct cb_handler pno_cb;
	struct nlo_mawc_params mawc_params;
	bool user_config_sched_scan_plan;
};
#endif

#ifdef FEATURE_WLAN_EXTSCAN
/**
 * struct extscan_def_config - def configuration for EXTSCAN
 * @extscan_enabled: enable extscan
 * @extscan_passive_max_chn_time: max passive channel time
 * @extscan_passive_min_chn_time: min passive channel time
 * @extscan_active_max_chn_time: max active channel time
 * @extscan_active_min_chn_time: min active channel time
 */
struct extscan_def_config {
	bool     extscan_enabled;
	uint32_t extscan_passive_max_chn_time;
	uint32_t extscan_passive_min_chn_time;
	uint32_t extscan_active_max_chn_time;
	uint32_t extscan_active_min_chn_time;
};
#endif

/**
 * struct scan_default_params - default scan parameters to be used
 * @active_dwell: default active dwell time
 * @allow_dfs_chan_in_first_scan: first scan should contain dfs channels or not.
 * @allow_dfs_chan_in_scan: Scan DFS channels or not.
 * @skip_dfs_chan_in_p2p_search: Skip DFS channels in p2p search.
 * @use_wake_lock_in_user_scan: if wake lock will be acquired during user scan
 * @active_dwell_2g: default active dwell time for 2G channels, if it's not zero
 * @min_dwell_time_6g: default min dwell time for 6G channels
 * @active_dwell_6g: default active dwell time for 6G channels
 * @passive_dwell_6g: default passive dwell time for 6G channels
 * @active_dwell_time_6g_conc: default concurrent active dwell time for 6G
 * @passive_dwell_time_6g_conc: default concurrent passive dwell time for 6G
 * @passive_dwell:default passive dwell time
 * @max_rest_time: default max rest time
 * @sta_miracast_mcc_rest_time: max rest time for miracast and mcc
 * @min_rest_time: default min rest time
 * @idle_time: default idle time
 * @conc_active_dwell: default concurrent active dwell time
 * @conc_passive_dwell: default concurrent passive dwell time
 * @conc_max_rest_time: default concurrent max rest time
 * @conc_min_rest_time: default concurrent min rest time
 * @conc_idle_time: default concurrent idle time
 * @repeat_probe_time: default repeat probe time
 * @probe_spacing_time: default probe spacing time
 * @probe_delay: default probe delay
 * @burst_duration: default burst duration
 * @max_scan_time: default max scan time
 * @num_probes: default maximum number of probes to sent
 * @cache_aging_time: default scan cache aging time
 * @select_5gh_margin: Prefer connecting to 5G AP even if
 *      its RSSI is lower by select_5gh_margin dbm than 2.4G AP.
 *      applicable if prefer_5ghz is set.
 * @enable_mac_spoofing: enable mac address spoof in scan
 * @max_bss_per_pdev: maximum number of bss entries to be maintained per pdev
 * @max_active_scans_allowed: maximum number of active parallel scan allowed
 *                            per psoc
 * @scan_mode_6g: scan mode in 6Ghz
 * @duty_cycle_6ghz: Enable optimization on 6g channels for every full scan
 *                   except the duty cycle. So that every nth scan(depending on
 *                   duty cycle) is a full scan and rest are all optimized scans
 * @enable_connected_scan: enable scans after connection
 * @scan_priority: default scan priority
 * @adaptive_dwell_time_mode: adaptive dwell mode with connection
 * @adaptive_dwell_time_mode_nc: adaptive dwell mode without connection
 * @honour_nl_scan_policy_flags: honour nl80211 scan policy flags
 * @extscan_adaptive_dwell_mode: Adaptive dwell mode during ext scan
 * @scan_f_passive: passively scan all channels including active channels
 * @scan_f_bcast_probe: add wild card ssid prbreq even if ssid_list is specified
 * @scan_f_cck_rates: add cck rates to rates/xrates ie in prb req
 * @scan_f_ofdm_rates: add ofdm rates to rates/xrates ie in prb req
 * @scan_f_chan_stat_evnt: enable indication of chan load and noise floor
 * @scan_f_filter_prb_req: filter Probe request frames
 * @scan_f_bypass_dfs_chn: when set, do not scan DFS channels
 * @scan_f_continue_on_err:continue scan even if few certain erros have occurred
 * @scan_f_offchan_mgmt_tx: allow mgmt transmission during off channel scan
 * @scan_f_offchan_data_tx: allow data transmission during off channel scan
 * @scan_f_promisc_mode: scan with promiscuous mode
 * @scan_f_capture_phy_err: enable capture ppdu with phy errrors
 * @scan_f_strict_passive_pch: do passive scan on passive channels
 * @scan_f_half_rate: enable HALF (10MHz) rate support
 * @scan_f_quarter_rate: set Quarter (5MHz) rate support
 * @scan_f_force_active_dfs_chn: allow to send probe req on DFS channel
 * @scan_f_add_tpc_ie_in_probe: add TPC ie in probe req frame
 * @scan_f_add_ds_ie_in_probe: add DS ie in probe req frame
 * @scan_f_add_spoofed_mac_in_probe: use random mac address for TA in probe
 * @scan_f_add_rand_seq_in_probe: use random sequence number in probe
 * @scan_f_en_ie_whitelist_in_probe: enable ie whitelist in probe
 * @scan_f_forced: force scan even in presence of data traffic
 * @scan_f_2ghz: scan 2.4 GHz channels
 * @scan_f_5ghz: scan 5 GHz channels
 * @scan_f_wide_band: scan in 40 MHz or higher bandwidth
 * @scan_flags: variable to read and set scan_f_* flags in one shot
 *              can be used to dump all scan_f_* flags for debug
 * @scan_ev_started: notify scan started event
 * @scan_ev_completed: notify scan completed event
 * @scan_ev_bss_chan: notify bss chan event
 * @scan_ev_foreign_chan: notify foreign chan event
 * @scan_ev_dequeued: notify scan request dequed event
 * @scan_ev_preempted: notify scan preempted event
 * @scan_ev_start_failed: notify scan start failed event
 * @scan_ev_restarted: notify scan restarted event
 * @scan_ev_foreign_chn_exit: notify foreign chan exit event
 * @scan_ev_invalid: notify invalid scan request event
 * @scan_ev_gpio_timeout: notify gpio timeout event
 * @scan_ev_suspended: notify scan suspend event
 * @scan_ev_resumed: notify scan resumed event
 * @scan_events: variable to read and set scan_ev_* flags in one shot
 *               can be used to dump all scan_ev_* flags for debug
 * @roam_params: roam related params
 */
struct scan_default_params {
	uint32_t active_dwell;
	bool allow_dfs_chan_in_first_scan;
	bool allow_dfs_chan_in_scan;
	bool skip_dfs_chan_in_p2p_search;
	bool use_wake_lock_in_user_scan;
	uint32_t active_dwell_2g;
	uint32_t min_dwell_time_6g;
	uint32_t active_dwell_6g;
	uint32_t passive_dwell_6g;
	uint32_t active_dwell_time_6g_conc;
	uint32_t passive_dwell_time_6g_conc;
	uint32_t passive_dwell;
	uint32_t max_rest_time;
	uint32_t sta_miracast_mcc_rest_time;
	uint32_t min_rest_time;
	uint32_t idle_time;
	uint32_t conc_active_dwell;
	uint32_t conc_passive_dwell;
	uint32_t conc_max_rest_time;
	uint32_t conc_min_rest_time;
	uint32_t conc_idle_time;
	uint32_t repeat_probe_time;
	uint32_t probe_spacing_time;
	uint32_t probe_delay;
	uint32_t burst_duration;
	uint32_t max_scan_time;
	uint32_t num_probes;
	qdf_time_t scan_cache_aging_time;
	uint32_t select_5ghz_margin;
	bool enable_mac_spoofing;
	uint32_t usr_cfg_probe_rpt_time;
	uint32_t usr_cfg_num_probes;
	uint16_t max_bss_per_pdev;
	uint32_t max_active_scans_allowed;
	uint8_t sta_scan_burst_duration;
	uint8_t p2p_scan_burst_duration;
	uint8_t go_scan_burst_duration;
	uint8_t ap_scan_burst_duration;
	enum scan_mode_6ghz scan_mode_6g;
	uint8_t duty_cycle_6ghz;
	bool enable_connected_scan;
	enum scan_priority scan_priority;
	enum scan_dwelltime_adaptive_mode adaptive_dwell_time_mode;
	enum scan_dwelltime_adaptive_mode adaptive_dwell_time_mode_nc;
	bool honour_nl_scan_policy_flags;
	enum scan_dwelltime_adaptive_mode extscan_adaptive_dwell_mode;
	union {
		struct {
			uint32_t scan_f_passive:1,
				scan_f_bcast_probe:1,
				scan_f_cck_rates:1,
				scan_f_ofdm_rates:1,
				scan_f_chan_stat_evnt:1,
				scan_f_filter_prb_req:1,
				scan_f_bypass_dfs_chn:1,
				scan_f_continue_on_err:1,
				scan_f_offchan_mgmt_tx:1,
				scan_f_offchan_data_tx:1,
				scan_f_promisc_mode:1,
				scan_f_capture_phy_err:1,
				scan_f_strict_passive_pch:1,
				scan_f_half_rate:1,
				scan_f_quarter_rate:1,
				scan_f_force_active_dfs_chn:1,
				scan_f_add_tpc_ie_in_probe:1,
				scan_f_add_ds_ie_in_probe:1,
				scan_f_add_spoofed_mac_in_probe:1,
				scan_f_add_rand_seq_in_probe:1,
				scan_f_en_ie_whitelist_in_probe:1,
				scan_f_forced:1,
				scan_f_2ghz:1,
				scan_f_5ghz:1,
				scan_f_wide_band:1;
		};
		uint32_t scan_flags;
	};
	union {
		struct {
			uint32_t scan_ev_started:1,
				scan_ev_completed:1,
				scan_ev_bss_chan:1,
				scan_ev_foreign_chan:1,
				scan_ev_dequeued:1,
				scan_ev_preempted:1,
				scan_ev_start_failed:1,
				scan_ev_restarted:1,
				scan_ev_foreign_chn_exit:1,
				scan_ev_invalid:1,
				scan_ev_gpio_timeout:1,
				scan_ev_suspended:1,
				scan_ev_resumed:1;
		};
		uint32_t scan_events;
	};
};

/**
 * struct scan_cb - nif/sif function callbacks
 * @inform_beacon: cb to indicate frame to OS
 * @update_beacon: cb to indicate frame to MLME
 * @unlink_bss: cb to unlink bss from kernel cache
 */
struct scan_cb {
	update_beacon_cb inform_beacon;
	update_beacon_cb update_beacon;
	update_beacon_cb unlink_bss;
	/* Define nif/sif function callbacks here */
};

/**
 * struct wlan_scan_obj - scan object definition
 * @scan_disabled: if scan is disabled
 * @scan_db:    scan cache data base
 * @cc_db:      pointer of country code data base
 * @lock:       spin lock
 * @scan_def:   default scan parameters
 * @cb:         nif/sif function callbacks
 * @requesters: requester allocation pool
 * @scan_ids:   last allocated scan id
 * @global_evhandlers:  registered scan event handlers
 * @pdev_info: pointer to pdev info
 * @pno_cfg: default pno configuration
 * @extscan_cfg: default extscan configuration
 * @ie_whitelist: default ie whitelist attrs
 * @bt_a2dp_enabled: if bt a2dp is enabled
 * @miracast_enabled: miracast enabled
 * @disable_timeout: command timeout disabled
 * @drop_bcn_on_chan_mismatch: drop bcn if channel mismatch
 * @obss_scan_offload: if obss scan offload is enabled
 * @drop_bcn_on_invalid_freq: drop bcn if freq is invalid in IEs (DS/HT/HE)
 * @scan_start_request_buff: buffer used to pass
 *      scan config to event handlers
 * @rnr_channel_db: RNR channel list database
 * @duty_cycle_cnt_6ghz: Scan count to track the full scans and decide whether
 *                        to optimizate 6g channels in the scan request based
 *                        on the ini scan_mode_6ghz_duty_cycle.
 * @allow_bss_with_incomplete_ie: Continue scan entry even if any corrupted IES are
 *			    present.
 */
struct wlan_scan_obj {
	uint32_t scan_disabled;
	qdf_spinlock_t lock;
	qdf_atomic_t scan_ids;
	struct scan_dbs scan_db[WLAN_UMAC_MAX_PDEVS];
	struct scan_country_code_db *cc_db;
	struct scan_default_params scan_def;
	struct scan_cb cb;
	struct scan_requester_info requesters[WLAN_MAX_REQUESTORS];
	struct global_scan_ev_handlers global_evhandlers;
	struct pdev_scan_info pdev_info[WLAN_UMAC_MAX_PDEVS];
#ifdef FEATURE_WLAN_SCAN_PNO
	struct pno_def_config pno_cfg;
#endif
#ifdef FEATURE_WLAN_EXTSCAN
	struct extscan_def_config extscan_cfg;
#endif
	struct probe_req_whitelist_attr ie_whitelist;
	bool bt_a2dp_enabled;
	bool miracast_enabled;
	bool disable_timeout;
	bool drop_bcn_on_chan_mismatch;
	bool drop_bcn_on_invalid_freq;
	bool obss_scan_offload;
	struct scan_start_request scan_start_request_buff;
#ifdef FEATURE_6G_SCAN_CHAN_SORT_ALGO
	struct channel_list_db rnr_channel_db;
#endif
#ifdef ENABLE_SCAN_PROFILE
	uint64_t scan_listener_cb_exe_dur[MAX_SCAN_EVENT_LISTENERS];
	uint64_t scm_scan_event_duration;
	uint64_t scm_scan_to_post_scan_duration;
#endif
	uint16_t duty_cycle_cnt_6ghz;
	bool allow_bss_with_incomplete_ie;
};

#ifdef ENABLE_SCAN_PROFILE
static inline
void scm_duration_init(struct wlan_scan_obj *scan)
{
	if (!scan)
		return;

	scan->scm_scan_event_duration = 0;
	scan->scm_scan_to_post_scan_duration = 0;
}

static inline
void scm_event_duration_start(struct wlan_scan_obj *scan)
{
	if (!scan)
		return;

	scan->scm_scan_event_duration =
		qdf_ktime_to_ms(qdf_ktime_get());
}

static inline
void scm_event_duration_end(struct wlan_scan_obj *scan)
{
	if (!scan)
		return;

	scan->scm_scan_event_duration =
		(qdf_ktime_to_ms(qdf_ktime_get()) -
		 scan->scm_scan_event_duration);
}

static inline
void scm_to_post_scan_duration_set(struct wlan_scan_obj *scan)
{
	if (!scan)
		return;

	scan->scm_scan_to_post_scan_duration =
		(qdf_ktime_to_ms(qdf_ktime_get()) -
		 scan->scm_scan_event_duration);
}

static inline
void scm_listener_cb_exe_dur_start(struct wlan_scan_obj *scan, uint8_t index)
{
	if (!scan || (index >= MAX_SCAN_EVENT_LISTENERS))
		return;

	scan->scan_listener_cb_exe_dur[index] =
		qdf_ktime_to_ms(qdf_ktime_get());
}

static inline
void scm_listener_cb_exe_dur_end(struct wlan_scan_obj *scan, uint8_t index)
{
	if (!scan || (index >= MAX_SCAN_EVENT_LISTENERS))
		return;

	scan->scan_listener_cb_exe_dur[index] =
		(qdf_ktime_to_ms(qdf_ktime_get()) -
		 scan->scan_listener_cb_exe_dur[index]);
}

static inline
void scm_listener_duration_init(struct wlan_scan_obj *scan)
{
	if (!scan)
		return;

	qdf_mem_set(&scan->scan_listener_cb_exe_dur,
		    sizeof(uint64_t) * MAX_SCAN_EVENT_LISTENERS,
		    0);
}
#else
static inline
void scm_duration_init(struct wlan_scan_obj *scan)
{
}

static inline
void scm_event_duration_start(struct wlan_scan_obj *scan)
{
}

static inline
void scm_event_duration_end(struct wlan_scan_obj *scan)
{
}

static inline
void scm_to_post_scan_duration_set(struct wlan_scan_obj *scan)
{
}

static inline
void scm_listener_cb_exe_dur_start(struct wlan_scan_obj *scan, uint8_t index)
{
}

static inline
void scm_listener_cb_exe_dur_end(struct wlan_scan_obj *scan, uint8_t index)
{
}

static inline
void scm_listener_duration_init(struct wlan_scan_obj *scan)
{
}
#endif

/**
 * wlan_psoc_get_scan_obj() - private API to get scan object from psoc
 * @psoc: psoc object
 *
 * Return: scan object
 */
#define wlan_psoc_get_scan_obj(psoc) \
	wlan_psoc_get_scan_obj_fl(psoc, \
				  __func__, __LINE__)

static inline struct wlan_scan_obj *
wlan_psoc_get_scan_obj_fl(struct wlan_objmgr_psoc *psoc,
			  const char *func, uint32_t line)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = (struct wlan_scan_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_SCAN);
	if (!scan_obj) {
		scm_err("%s:%u, Failed to get scan object", func, line);
		return NULL;
	}
	return scan_obj;
}

/**
 * wlan_pdev_get_scan_obj() - private API to get scan object from pdev
 * @psoc: pdev object
 *
 * Return: scan object
 */
static inline struct wlan_scan_obj *
wlan_pdev_get_scan_obj(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);

	return wlan_psoc_get_scan_obj(psoc);
}

/**
 * wlan_vdev_get_scan_obj() - private API to get scan object from vdev
 * @psoc: vdev object
 *
 * Return: scan object
 */
static inline struct wlan_scan_obj *
wlan_vdev_get_scan_obj(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);

	return wlan_pdev_get_scan_obj(pdev);
}

/**
 * wlan_get_vdev_scan_obj() - private API to get scan object vdev
 * @vdev: vdev object
 *
 * Return: scan object
 */
static inline struct scan_vdev_obj *
wlan_get_vdev_scan_obj(struct wlan_objmgr_vdev *vdev)
{
	struct scan_vdev_obj *scan_vdev_obj;

	scan_vdev_obj = (struct scan_vdev_obj *)
		wlan_objmgr_vdev_get_comp_private_obj(vdev,
				WLAN_UMAC_COMP_SCAN);

	return scan_vdev_obj;
}

/**
 * wlan_scan_vdev_get_pdev_id() - private API to get pdev id from vdev object
 * @vdev: vdev object
 *
 * Return: parent pdev id
 */
static inline uint8_t
wlan_scan_vdev_get_pdev_id(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);

	return wlan_objmgr_pdev_get_pdev_id(pdev);
}

/**
 * wlan_pdev_get_pdev_scan_ev_handlers() - private API to get
 * pdev scan event handlers
 * @vdev: pdev object
 *
 * Return: pdev_scan_ev_handler object
 */
static inline struct pdev_scan_ev_handler*
wlan_pdev_get_pdev_scan_ev_handlers(struct wlan_objmgr_pdev *pdev)
{
	uint8_t pdevid;
	struct wlan_scan_obj *scan = NULL;

	if (!pdev)
		goto err;

	pdevid = wlan_objmgr_pdev_get_pdev_id(pdev);
	scan = wlan_pdev_get_scan_obj(pdev);
	if (!scan)
		goto err;

	return &scan->global_evhandlers.pdev_ev_handlers[pdevid];

err:
	scm_err("NULL pointer, pdev: 0x%pK, scan_obj: 0x%pK",
		pdev, scan);
	return NULL;
}

/**
 * wlan_vdev_get_pdev_scan_ev_handlers() - private API to get
 * pdev scan event handlers
 * @vdev: vdev object
 *
 * Return: pdev_scan_ev_handler object
 */
static inline struct pdev_scan_ev_handler*
wlan_vdev_get_pdev_scan_ev_handlers(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);

	return wlan_pdev_get_pdev_scan_ev_handlers(pdev);
}

/**
 * wlan_scan_psoc_get_def_params() - private API to get scan defaults
 * @psoc: psoc object
 *
 * Return: scan defaults
 */
static inline struct scan_default_params*
wlan_scan_psoc_get_def_params(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan = NULL;

	if (!psoc) {
		scm_err("null psoc");
		return NULL;
	}
	scan = wlan_psoc_get_scan_obj(psoc);

	if (!scan)
		return NULL;

	return &scan->scan_def;
}

/**
 * wlan_vdev_get_def_scan_params() - private API to get scan defaults
 * @vdev: vdev object
 *
 * Return: scan defaults
 */
static inline struct scan_default_params*
wlan_vdev_get_def_scan_params(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	if (!vdev) {
		scm_err("null vdev");
		return NULL;
	}
	psoc = wlan_vdev_get_psoc(vdev);

	return wlan_scan_psoc_get_def_params(psoc);
}

/**
 * wlan_scan_psoc_set_disable() - private API to disable scans for psoc
 * @psoc: psoc on which scans need to be disabled
 * @reason: reason for enable/disabled
 *
 * Return: QDF_STATUS.
 */
static inline QDF_STATUS
wlan_scan_psoc_set_disable(struct wlan_objmgr_psoc *psoc,
			   enum scan_disable_reason reason)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_obj->scan_disabled |= reason;

	scm_debug("Psoc scan_disabled %x", scan_obj->scan_disabled);

	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
wlan_scan_psoc_set_enable(struct wlan_objmgr_psoc *psoc,
			  enum scan_disable_reason reason)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_obj->scan_disabled &= ~reason;
	scm_debug("Psoc scan_disabled %x", scan_obj->scan_disabled);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_scan_psoc_created_notification() - scan psoc create handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_scan_psoc_created_notification(struct wlan_objmgr_psoc *psoc,
	void *arg_list);

/**
 * wlan_scan_psoc_deleted_notification() - scan psoc delete handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_scan_psoc_destroyed_notification(struct wlan_objmgr_psoc *psoc,
	void *arg_list);

/**
 * wlan_scan_vdev_created_notification() - scan psoc create handler
 * @vdev: vdev object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_scan_vdev_created_notification(struct wlan_objmgr_vdev *vdev,
	void *arg_list);

/**
 * wlan_scan_vdev_destroyed_notification() - scan psoc delete handler
 * @vdev: vdev object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_scan_vdev_destroyed_notification(struct wlan_objmgr_vdev *vdev,
	void *arg_list);

#endif

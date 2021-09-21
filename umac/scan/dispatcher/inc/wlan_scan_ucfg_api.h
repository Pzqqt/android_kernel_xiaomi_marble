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
 * DOC: contains scan north bound interface api
 */

#ifndef _WLAN_SCAN_UCFG_API_H_
#define _WLAN_SCAN_UCFG_API_H_

#include <scheduler_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>
#include "wlan_scan_api.h"

/**
 * ucfg_scan_register_requester() - Public ucfg API, assigns requester ID
 * to caller and registers scan event call back handler
 * @psoc:       psoc object
 * @module_name:name of requester module
 * @event_cb:   event callback function pointer
 * @arg:        argument to @event_cb
 *
 * API, allows other components to allocate requester id.
 * Normally used by modules at init time to register their callback
 * and get one requester id. @event_cb will be invoked for
 * all scan events whose requester id matches with @requester.
 *
 * Return: assigned non zero requester id for success
 *         zero (0) for failure
 */
static inline wlan_scan_requester
ucfg_scan_register_requester(struct wlan_objmgr_psoc *psoc,
	uint8_t *module_name, scan_event_handler event_cb, void *arg)
{
	return wlan_scan_register_requester(psoc, module_name, event_cb, arg);
}

/**
 * ucfg_scan_unregister_requester() -Public ucfg API, reclaims previously
 * allocated requester ID
 * @psoc:       psoc object
 * @requester:  requester ID to reclaim.
 *
 * API, reclaims previously allocated requester id.
 *
 * Return: void
 */
static inline
void ucfg_scan_unregister_requester(struct wlan_objmgr_psoc *psoc,
	wlan_scan_requester requester)
{
	return wlan_scan_unregister_requester(psoc, requester);
}

/**
 * ucfg_get_scan_requester_name()- returns module name of requester ID owner
 * @psoc:       psoc object
 * @requester:  requester ID
 *
 * API, returns module name of requester id owner
 *
 * Return: pointer to module name or "unknown" if requester id not found.
 */
uint8_t *ucfg_get_scan_requester_name(struct wlan_objmgr_psoc *psoc,
	wlan_scan_requester requester);

/**
 * ucfg_scan_get_scan_id() - Public ucfg API to allocate scan ID
 * @psoc: psoc object
 *
 * Public ucfg API, allocates a new scan id for caller
 *
 * Return: newly allocated scan ID
 */
static inline
wlan_scan_id ucfg_scan_get_scan_id(struct wlan_objmgr_psoc *psoc)
{
	return wlan_scan_get_scan_id(psoc);
}

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * ucfg_scan_pno_start() - Public API to start PNO
 * @vdev: vdev pointer
 * @req: pno req params
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS ucfg_scan_pno_start(struct wlan_objmgr_vdev *vdev,
struct pno_scan_req_params *req);

/**
 * ucfg_scan_pno_stop() - Public API to stop PNO
 * @vdev: vdev pointer
 * @req: pno req params
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS ucfg_scan_pno_stop(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_scan_get_pno_in_progress() - Public API to check if pno is in progress
 * @vdev: vdev pointer
 *
 * Return: true if pno in progress else false.
 */
bool ucfg_scan_get_pno_in_progress(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_scan_get_pno_match() - Public API to check if pno matched
 * @vdev: vdev pointer
 *
 * Return: true if pno matched else false.
 */
bool ucfg_scan_get_pno_match(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_scan_register_pno_cb() - register pno cb
 * @psoc: psoc object
 * @event_cb: callback function pointer
 * @arg: argument to @event_cb
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_scan_register_pno_cb(struct wlan_objmgr_psoc *psoc,
	scan_event_handler event_cb, void *arg);

/**
 * ucfg_scan_get_pno_def_params() - get the defaults pno params
 * @vdev: vdev object
 * @req: pno request object
 *
 * Return: QDF_STATUS_SUCCESS or error code
 */
QDF_STATUS
ucfg_scan_get_pno_def_params(struct wlan_objmgr_vdev *vdev,
	struct pno_scan_req_params *req);

#else

static inline bool
ucfg_scan_get_pno_in_progress(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline bool
ucfg_scan_get_pno_match(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
#endif /* FEATURE_WLAN_SCAN_PNO */
/**
 * ucfg_scm_scan_free_scan_request_mem() - Free scan request memory
 * @req: scan_start_request object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scm_scan_free_scan_request_mem(struct scan_start_request *req);

/**
 * ucfg_scan_start() - ucfg Public API to start a scan
 * @req: start scan req params
 *
 * The ucfg public API to start a scan. Post a msg to target_if queue
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS ucfg_scan_start(struct scan_start_request *req)
{
	return wlan_scan_start(req);
}

/**
 * ucfg_scan_set_psoc_enable() - Public API to enable scans for psoc
 * @psoc: psoc on which scans need to be disabled
 * @reason: reason for enable/disabled
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_psoc_set_enable(struct wlan_objmgr_psoc *psoc,
				     enum scan_disable_reason reason);

/**
 * ucfg_scan_psoc_set_disable() - Public API to disable scans for psoc
 * @psoc: psoc on which scans need to be disabled
 * @reason: reason for enable/disabled
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_psoc_set_disable(struct wlan_objmgr_psoc *psoc,
				      enum scan_disable_reason reason);

/**
 * ucfg_scan_vdev_set_enable() - Public API to enable scans for vdev
 * @psoc: psoc on which scans need to be disabled
 * @reason: reason for enable/disabled
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_vdev_set_enable(struct wlan_objmgr_vdev *vdev,
				     enum scan_disable_reason reason);

/**
 * ucfg_scan_vdev_set_disable() - Public API to disable scans for vdev
 * @psoc: psoc on which scans need to be disabled
 * @reason: reason for enable/disabled
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_vdev_set_disable(struct wlan_objmgr_vdev *vdev,
				      enum scan_disable_reason reason);



/**
 * ucfg_scan_set_miracast() - Public API to disable/enable miracast flag
 * @psoc: psoc pointer
 * @enable: enable miracast if true disable is false
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_set_miracast(
		struct wlan_objmgr_psoc *psoc, bool enable);

/**
 * ucfg_scan_set_global_config() - Public API to set global scan config
 * @psoc: psoc context
 * @config: config to set
 * @val: new config value
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
ucfg_scan_set_global_config(struct wlan_objmgr_psoc *psoc,
		enum scan_config config, uint32_t val);

/**
 * ucfg_scan_get_global_config() - Public API to get global scan config
 * @psoc: psoc context
 * @config: config to set
 * @val: uint32* to hold returned config value
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS
ucfg_scan_get_global_config(struct wlan_objmgr_psoc *psoc,
		enum scan_config config, uint32_t *val);

/**
 * ucfg_scan_set_obss_scan_offload() - Public API to set obss scan flag
 * @psoc: psoc context
 * @val: the value to be set
 *
 * Return: void.
 */
void ucfg_scan_set_obss_scan_offload(struct wlan_objmgr_psoc *psoc, bool value);

/**
 * ucfg_scan_set_wide_band_scan() - Public API to disable/enable wide band scan
 * @pdev: psoc on which scans need to be disabled
 * @enable: enable wide band scan if @enable is true, disable otherwise
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_set_wide_band_scan(
		struct wlan_objmgr_pdev *pdev, bool enable);

/**
 * ucfg_scan_get_wide_band_scan() - Public API to check if
 * wide band scan is enabled or disabled
 * @pdev: psoc on which scans status need to be checked
 *
 * Return: true if enabled else false.
 */
bool ucfg_scan_get_wide_band_scan(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_scan_set_custom_scan_chan_list() - Public API to restrict scan
 * to few pre configured channels
 * @pdev: psoc on which scans need to be disabled
 * @chan_list: list of channels to scan if set
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_set_custom_scan_chan_list(
		struct wlan_objmgr_pdev *pdev, struct chan_list *chan_list);
/**
 * ucfg_scan_set_ssid_bssid_hidden_ssid_beacon() - API to configure
 * ssid, bssid of hidden beacon
 * @pdev: psoc on which ssid bssid need to configure
 * @bssid: bssid of the hidden AP
 * @ssid: desired ssid
 *
 * Return: QDF_STATUS.
 */
#ifdef WLAN_DFS_CHAN_HIDDEN_SSID
QDF_STATUS
ucfg_scan_config_hidden_ssid_for_bssid(struct wlan_objmgr_pdev *pdev,
				       uint8_t *bssid,
				       struct wlan_ssid *ssid);
#else
static inline QDF_STATUS
ucfg_scan_config_hidden_ssid_for_bssid(struct wlan_objmgr_pdev *pdev,
				       uint8_t *bssid,
				       struct wlan_ssid *ssid)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_DFS_CHAN_HIDDEN_SSID */

/**
 * ucfg_scan_cancel() - ucfg Public API to cancel the scan
 * @req: stop scan request params
 *
 * The ucfg public API to stop a scan. Post a msg to target_if queue
 *
 * Return: QDF_STATUS.
 */
static inline QDF_STATUS ucfg_scan_cancel(struct scan_cancel_request *req)
{
	return wlan_scan_cancel(req);
}

/**
 * ucfg_scan_cancel_sync() - Public API to stop a scan and wait
 * till all scan are completed
 * @req: stop scan request params
 *
 * The Public API to stop a scan and wait
 * till all scan are completed
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
ucfg_scan_cancel_sync(struct scan_cancel_request *req);

/**
 * ucfg_scan_get_result() - The Public API to get scan results
 * @pdev: pdev info
 * @filter: Filters
 *
 * This function fetches scan result
 *
 * Return: scan list pointer
 */
qdf_list_t *ucfg_scan_get_result(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter);

/**
 * ucfg_scan_purge_results() - purge the scan list
 * @scan_list: scan list to be purged
 *
 * This function purge the temp scan list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_purge_results(qdf_list_t *scan_list);

/**
 * ucfg_scan_flush_results() - The Public API to flush scan result
 * @pdev: pdev object
 * @filter: filter to flush the scan entries
 *
 * The Public API to flush scan result.
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS ucfg_scan_flush_results(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter);

/**
 * ucfg_scan_filter_valid_channel() - The Public API to filter scan result
 * based on valid channel list
 * @pdev: pdev object
 * @chan_freq_list: valid channel frequency (in MHz) list
 * @num_chan: number of valid channels
 *
 * The Public API to to filter scan result
 * based on valid channel list.
 *
 * Return: void.
 */
void ucfg_scan_filter_valid_channel(struct wlan_objmgr_pdev *pdev,
	uint32_t *chan_freq_list, uint32_t num_chan);

/**
 * ucfg_scan_db_iterate() - function to iterate scan table
 * @pdev: pdev object
 * @func: iterator function pointer
 * @arg: argument to be passed to func()
 *
 * API, this API iterates scan table and invokes func
 * on each scan enetry by passing scan entry and arg.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_scan_db_iterate(struct wlan_objmgr_pdev *pdev,
	scan_iterator_func func, void *arg);

/**
 * ucfg_scan_update_mlme_by_bssinfo() - The Public API to update mlme
 * info in the scan entry
 * @pdev: pdev object
 * @bssid: bssid info to find the matching scan entry
 * @mlme_info: mlme info to be updated.
 *
 * The Public API to update mlme info in the scan entry.
 * Post a msg to target_if queue
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
ucfg_scan_update_mlme_by_bssinfo(struct wlan_objmgr_pdev *pdev,
	struct bss_info *bss_info,
	struct mlme_info *mlme_info);

/**
 * ucfg_scan_register_event_handler() - The Public API to register
 * an event cb handler
 * @pdev: pdev object
 * @event_cb: callback function to register
 * @arg: component specific priv argument to @event_cb callback function
 *
 * The Public API to register a event cb handler. This cb is called whenever
 * any scan event is received on @pdev.
 *
 * Return: 0 for success or error code.
 */

QDF_STATUS
ucfg_scan_register_event_handler(struct wlan_objmgr_pdev *pdev,
	scan_event_handler event_cb, void *arg);

/**
 * ucfg_scan_unregister_event_handler() - Public API to unregister
 * event cb handler
 * @pdev: pdev object
 * @event_cb: callback function to unregister
 * @arg: component specific priv argument to @event_cb callback function
 *
 * Unregister a event cb handler. cb and arg will be used to
 * find the calback.
 *
 * Return: void
 */

void
ucfg_scan_unregister_event_handler(struct wlan_objmgr_pdev *pdev,
	scan_event_handler event_cb, void *arg);

/**
 * ucfg_scan_init_default_params() - Public ucfg API to initialize scan params
 * @vdev: vdev object
 * @req: scan request object
 *
 * Public ucfg API to initialize scan start request with defaults scan params
 *
 * Return: QDF_STATUS_SUCCESS or error code
 */
static inline QDF_STATUS
ucfg_scan_init_default_params(struct wlan_objmgr_vdev *vdev,
	struct scan_start_request *req)
{
	return wlan_scan_init_default_params(vdev, req);
}

/**
 * ucfg_scan_init_ssid_params() - initialize scan request ssid list
 *
 * @scan_req: scan request object
 * @num_ssid: number of ssid's in ssid list
 * @ssid_list: ssid list
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
ucfg_scan_init_ssid_params(struct scan_start_request *scan_req,
	uint32_t num_ssid, struct wlan_ssid *ssid_list);

/**
 * ucfg_scan_init_bssid_params() - initialize scan request bssid list
 * @scan_req: scan request object
 * @num_ssid: number of bssid's in bssid list
 * @bssid_list: bssid list
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
ucfg_scan_init_bssid_params(struct scan_start_request *scan_req,
	uint32_t num_ssid, struct qdf_mac_addr *bssid_list);

/**
 * ucfg_scan_init_chanlist_params() - initialize scan request channel list
 * @scan_req: scan request object
 * @num_chans: number of channels in channel list
 * @chan_list: channel list
 * @phymode: phymode in which scan shall be done
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
ucfg_scan_init_chanlist_params(struct scan_start_request *scan_req,
	uint32_t num_chans, uint32_t *chan_list, uint32_t *phymode);

/**
 * ucfg_scan_get_vdev_status() - API to check vdev scan status
 * @vdev: vdev object
 *
 * Return: enum scm_scan_status
 */
enum scm_scan_status
ucfg_scan_get_vdev_status(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_scan_get_pdev_status() - API to check pdev scan status
 * @pdev: pdev object
 *
 * Return: enum scm_scan_status
 */
enum scm_scan_status
ucfg_scan_get_pdev_status(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_scan_register_bcn_cb() - API to register api
 * to inform/update bcn/probe as soon as they are received
 * @pdev: psoc
 * @cb: callback to be registered
 * @type: Type of callback to be registered
 *
 * Return: enum scm_scan_status
 */
QDF_STATUS ucfg_scan_register_bcn_cb(struct wlan_objmgr_psoc *psoc,
	update_beacon_cb cb, enum scan_cb_type type);

/*
 * ucfg_scan_update_user_config() - Update scan cache user config
 * @psoc: psoc
 * @scan_cfg: scan user config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_update_user_config(struct wlan_objmgr_psoc *psoc,
	struct scan_user_cfg *scan_cfg);

/*
 * ucfg_scan_init() - Scan module initialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_init(void);

/**
 * ucfg_scan_deinit() - Scan module deinitialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_deinit(void);

/**
 * ucfg_scan_psoc_enable() - Scan module enable API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_psoc_enable() - Scan module disable API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_psoc_disable(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_psoc_open() - Scan module psoc open API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_psoc_close() - Scan module psoc close API
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_max_active_scans() - API to get max active scans
 * supported on this psoc
 * @psoc: psoc object
 *
 * Return: uint32_t
 */
uint32_t ucfg_scan_get_max_active_scans(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_ie_whitelist_enabled() - Checks for IE whitelisting enable
 * @psoc: pointer to psoc object
 * @vdev: pointer to vdev
 *
 * This function is used to check whether IE whitelisting is enabled or not
 *
 * Return: If enabled returns true else returns false
 */
bool ucfg_ie_whitelist_enabled(struct wlan_objmgr_psoc *psoc,
			       struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_copy_ie_whitelist_attrs() - Populate probe req IE whitelist attrs
 * @psoc: pointer to psoc object
 * @ie_whitelist: output parameter to hold ie whitelist attrs
 *
 * If IE whitelisting is enabled then invoke this function to copy
 * IE whitelisting attrs from wlan scan object
 *
 * Return: true - successful copy
 *         false - copy failed
 */
bool ucfg_copy_ie_whitelist_attrs(struct wlan_objmgr_psoc *psoc,
				struct probe_req_whitelist_attr *ie_whitelist);

/**
 * ucfg_scan_set_bt_activity() - API to set bt activity
 * @psoc: pointer to psoc object
 * @bt_a2dp_active: bt activiy value
 *
 * Return: None
 */
void ucfg_scan_set_bt_activity(struct wlan_objmgr_psoc *psoc,
			       bool bt_a2dp_active);
/**
 * ucfg_scan_get_bt_activity() - API to get bt activity
 * @psoc: pointer to psoc object
 *
 * Return: true if enabled else false.
 */
bool ucfg_scan_get_bt_activity(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_is_mac_spoofing_enabled() - API to check if mac spoofing is enabled
 * @psoc: pointer to psoc object
 *
 * Return: true if enabled else false.
 */
bool ucfg_scan_is_mac_spoofing_enabled(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_extscan_adaptive_dwell_mode() - API to get the adaptive dwell
 * mode during ext scan
 * @psoc: pointer to psoc object
 *
 * Return: value of type enum scan_dwelltime_adaptive_mode
 */
enum scan_dwelltime_adaptive_mode
ucfg_scan_get_extscan_adaptive_dwell_mode(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_cfg_set_active_dwelltime() - API to set scan active dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active dwell time
 *
 * Return: none
 */
static inline
void ucfg_scan_cfg_set_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					uint32_t dwell_time)
{
	return wlan_scan_cfg_set_active_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_set_active_2g_dwelltime() - API to set scan active 2g dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active dwell time
 *
 * Return: none
 */
static inline
void ucfg_scan_cfg_set_active_2g_dwelltime(struct wlan_objmgr_psoc *psoc,
					   uint32_t dwell_time)
{
	return wlan_scan_cfg_set_active_2g_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_get_active_dwelltime() - API to get active dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active dwelltime
 *
 * Return: scan active dwell time
 */
static inline
void ucfg_scan_cfg_get_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_active_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_set_passive_dwelltime() - API to set scan passive dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan passive dwell time
 *
 * Return: none
 */
static inline
void ucfg_scan_cfg_set_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					 uint32_t dwell_time)
{
	return wlan_scan_cfg_set_passive_dwelltime(psoc, dwell_time);
}
/**
 * ucfg_scan_cfg_get_passive_dwelltime() - API to get passive dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan passive dwelltime
 *
 * Return: scan passive dwell time
 */
static inline
void ucfg_scan_cfg_get_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					 uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_passive_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_get_active_2g_dwelltime() - API to get active 2g dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active 2g dwelltime
 *
 * Return: scan active 2g dwelltime
 */
static inline
void ucfg_scan_cfg_get_active_2g_dwelltime(struct wlan_objmgr_psoc *psoc,
					   uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_active_2g_dwelltime(psoc, dwell_time);
}

#ifdef CONFIG_BAND_6GHZ
/**
 * ucfg_scan_cfg_set_active_6g_dwelltime() - API to set scan active 6g dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active dwell time
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS ucfg_scan_cfg_set_active_6g_dwelltime(struct wlan_objmgr_psoc *psoc,
						 uint32_t dwell_time)
{
	return wlan_scan_cfg_set_active_6g_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_get_passive_6g_dwelltime() - API to get passive 6g dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan passive 6g dwelltime
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS ucfg_scan_cfg_get_passive_6g_dwelltime(struct wlan_objmgr_psoc *psoc,
						  uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_passive_6g_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_set_passive_6g_dwelltime() - API to set scan passive 6g
 *                                            dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan passive dwell time
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS ucfg_scan_cfg_set_passive_6g_dwelltime(struct wlan_objmgr_psoc *psoc,
						  uint32_t dwell_time)
{
	return wlan_scan_cfg_set_passive_6g_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_get_active_6g_dwelltime() - API to get active 6g dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active 6g dwelltime
 *
 * Return: QDF_STATUS
 */
static inline
QDF_STATUS ucfg_scan_cfg_get_active_6g_dwelltime(struct wlan_objmgr_psoc *psoc,
						 uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_active_6g_dwelltime(psoc, dwell_time);
}
#endif

/**
 * ucfg_scan_cfg_get_conc_active_dwelltime() - Get concurrent active dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active dwelltime
 *
 * Return: scan concurrent active dwell time
 */
static inline
void ucfg_scan_cfg_get_conc_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					     uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_conc_active_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_set_conc_active_dwelltime() - Set concurrent active dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan active dwelltime
 *
 * Return: scan concurrent active dwell time
 */
static inline
void ucfg_scan_cfg_set_conc_active_dwelltime(struct wlan_objmgr_psoc *psoc,
					     uint32_t dwell_time)
{
	return wlan_scan_cfg_set_conc_active_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_get_conc_passive_dwelltime() - Get passive concurrent dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan passive dwelltime
 *
 * Return: scan concurrent passive dwell time
 */
static inline
void ucfg_scan_cfg_get_conc_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					      uint32_t *dwell_time)
{
	return wlan_scan_cfg_get_conc_passive_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_set_conc_passive_dwelltime() - Set passive concurrent dwelltime
 * @psoc: pointer to psoc object
 * @dwell_time: scan passive dwelltime
 *
 * Return: scan concurrent passive dwell time
 */
static inline
void ucfg_scan_cfg_set_conc_passive_dwelltime(struct wlan_objmgr_psoc *psoc,
					      uint32_t dwell_time)
{
	return wlan_scan_cfg_set_conc_passive_dwelltime(psoc, dwell_time);
}

/**
 * ucfg_scan_cfg_get_dfs_chan_scan_allowed() - API to get dfs scan enabled
 * @psoc: pointer to psoc object
 * @enable_dfs_scan: DFS scan enabled or not.
 *
 * Return: None
 */
static inline
void ucfg_scan_cfg_get_dfs_chan_scan_allowed(struct wlan_objmgr_psoc *psoc,
					     bool *dfs_scan_enable)
{
	return wlan_scan_cfg_get_dfs_chan_scan_allowed(psoc, dfs_scan_enable);
}

/**
 * ucfg_scan_cfg_set_dfs_channel_scan() - API to set dfs scan enabled
 * @psoc: pointer to psoc object
 * @enable_dfs_scan: Set DFS scan enabled or not.
 *
 * Return: None
 */
static inline
void ucfg_scan_cfg_set_dfs_chan_scan_allowed(struct wlan_objmgr_psoc *psoc,
					     bool dfs_scan_enable)
{
	return wlan_scan_cfg_set_dfs_chan_scan_allowed(psoc, dfs_scan_enable);
}

/**
 * ucfg_scan_wake_lock_in_user_scan() - API to determine if wake lock in user
 * scan is used.
 * @psoc: pointer to psoc object
 *
 * Return: true if wake lock in user scan is required
 */
bool ucfg_scan_wake_lock_in_user_scan(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_cfg_honour_nl_scan_policy_flags() - API to get nl scan policy
 * flags honoured.
 * @psoc: pointer to psoc object
 *
 * Return: nl scan flags is honoured or not
 */
static inline
bool ucfg_scan_cfg_honour_nl_scan_policy_flags(struct wlan_objmgr_psoc *psoc)
{
	return wlan_scan_cfg_honour_nl_scan_policy_flags(psoc);
}

/**
 * ucfg_scan_cfg_get_conc_max_resttime() - API to get max rest time
 * @psoc: pointer to psoc object
 * @rest_time: scan concurrent max resttime
 *
 * Return: scan concurrent max rest time
 */
static inline
void ucfg_scan_cfg_get_conc_max_resttime(struct wlan_objmgr_psoc *psoc,
					 uint32_t *rest_time)
{
	return wlan_scan_cfg_get_conc_max_resttime(psoc, rest_time);
}

/**
 * ucfg_scan_cfg_get_conc_min_resttime() - API to get concurrent min rest time
 * @psoc: pointer to psoc object
 * @rest_time: scan concurrent min rest time
 *
 * Return: scan concurrent min rest time
 */
static inline
void ucfg_scan_cfg_get_conc_min_resttime(struct wlan_objmgr_psoc *psoc,
					 uint32_t *rest_time)
{
	return wlan_scan_cfg_get_conc_min_resttime(psoc, rest_time);
}

#ifdef FEATURE_WLAN_SCAN_PNO
/**
 * ucfg_scan_is_pno_offload_enabled() - Check if pno offload is enabled
 * @psoc: pointer to psoc object
 *
 * Return: pno_offload_enabled flag
 */
bool ucfg_scan_is_pno_offload_enabled(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_set_pno_offload() - API to set pno offload flag based on the
 * capability received from the firmware.
 * @psoc: pointer to psoc object
 * @rest_time: scan concurrent min rest time
 *
 * Return: scan concurrent min rest time
 */
void ucfg_scan_set_pno_offload(struct wlan_objmgr_psoc *psoc, bool value);

/**
 * ucfg_scan_get_pno_scan_support() - Check if pno scan support is enabled
 * @psoc: pointer to psoc object
 *
 * Return: scan_support_enabled flag
 */
bool ucfg_scan_get_pno_scan_support(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_get_scan_backoff_multiplier() - get scan backoff multiplier value
 * @psoc: pointer to psoc object
 *
 * Return: scan_support_enabled flag
 */
uint8_t ucfg_get_scan_backoff_multiplier(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_is_dfs_chnl_scan_enabled() - Check if PNO dfs channel scan support
 * is enabled
 * @psoc: pointer to psoc object
 *
 * Return: dfs_chnl_scan_enabled flag
 */
bool ucfg_scan_is_dfs_chnl_scan_enabled(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_scan_timer_repeat_value() - API to get PNO scan timer repeat
 * value
 * @psoc: pointer to psoc object
 *
 * Return: scan_timer_repeat_value
 */
uint32_t ucfg_scan_get_scan_timer_repeat_value(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_slow_scan_multiplier() - API to get PNO slow scan multiplier
 * value
 * @psoc: pointer to psoc object
 *
 * Return: slow_scan_multiplier value
 */
uint32_t ucfg_scan_get_slow_scan_multiplier(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_max_sched_scan_plan_interval() - API to get maximum scheduled
 * scan plan interval
 * @psoc: pointer to psoc object
 *
 * Return: max_sched_scan_plan_interval value.
 */
uint32_t
ucfg_scan_get_max_sched_scan_plan_interval(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_max_sched_scan_plan_iterations() - API to get maximum scheduled
 * scan plan iterations
 * @psoc: pointer to psoc object
 *
 * Return: value.
 */
uint32_t
ucfg_scan_get_max_sched_scan_plan_iterations(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_get_user_config_sched_scan_plan() - API to get user config sched
 * scan plan configuration value
 * @psoc: pointer to psoc object
 *
 * Return: value.
 */
bool
ucfg_scan_get_user_config_sched_scan_plan(struct wlan_objmgr_psoc *psoc);

#ifdef WLAN_POLICY_MGR_ENABLE
/*
 * ucfg_scan_update_pno_dwell_time() - update active and passive dwell time
 * depending on active concurrency modes
 * @vdev: vdev object pointer
 * @req: scan request
 *
 * Return: void
 */
static inline
void ucfg_scan_update_pno_dwell_time(struct wlan_objmgr_vdev *vdev,
				     struct pno_scan_req_params *req,
				     struct scan_default_params *scan_def)
{
	wlan_scan_update_pno_dwell_time(vdev, req, scan_def);
}

#else
static inline
void ucfg_scan_update_pno_dwell_time(struct wlan_objmgr_vdev *vdev,
				     struct pno_scan_req_params *req,
				     struct scan_default_params *scan_def)
{}

#endif

#else
static inline
bool ucfg_scan_is_pno_offload_enabled(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline
void ucfg_scan_set_pno_offload(struct wlan_objmgr_psoc *psoc, bool value)
{
}

static inline
bool ucfg_scan_get_pno_scan_support(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline
uint8_t ucfg_get_scan_backoff_multiplier(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline
bool ucfg_scan_is_dfs_chnl_scan_enabled(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline
uint32_t ucfg_scan_get_scan_timer_repeat_value(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline
uint32_t ucfg_scan_get_slow_scan_multiplier(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline uint32_t
ucfg_scan_get_max_sched_scan_plan_interval(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline uint32_t
ucfg_scan_get_max_sched_scan_plan_iterations(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline bool
ucfg_scan_get_user_config_sched_scan_plan(struct wlan_objmgr_psoc *psoc)
{
	return true;
}

#endif /* FEATURE_WLAN_SCAN_PNO */

/**
 * ucfg_scan_is_connected_scan_enabled() - API to get scan enabled after connect
 * @psoc: pointer to psoc object
 *
 * Return: value.
 */
bool ucfg_scan_is_connected_scan_enabled(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_scan_is_snr_monitor_enabled() - API to get SNR monitoring enabled or not
 * @psoc: pointer to psoc object
 *
 * Return: value.
 */
static inline
bool ucfg_scan_is_snr_monitor_enabled(struct wlan_objmgr_psoc *psoc)
{
	return wlan_scan_is_snr_monitor_enabled(psoc);
}
#endif

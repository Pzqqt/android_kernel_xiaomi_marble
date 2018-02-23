/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

/**
 * ucfg_scan_register_requester() - assigns requester ID to caller and
 * registers scan event call back handler
 * @psoc:       psoc object
 * @module_name:name of requester module
 * @event_cb:   event callback function pointer
 * @arg:        argument to @event_cb
 *
 * API, allows other components to allocate requester id
 * Normally used by modules at init time to register their callback
 * and get one requester id. @event_cb will be invoked for
 * all scan events whose requester id matches with @requester.
 *
 * Return: assigned non zero requester id for success
 *         zero (0) for failure
 */
wlan_scan_requester
ucfg_scan_register_requester(struct wlan_objmgr_psoc *psoc,
	uint8_t *module_name, scan_event_handler event_cb, void *arg);

/**
 * ucfg_scan_unregister_requester() -reclaims previously allocated requester ID
 * @psoc:       psoc object
 * @requester:  requester ID to reclaim.
 *
 * API, reclaims previously allocated requester id by
 * ucfg_scan_get_req_id_reg_cb()
 *
 * Return: void
 */
void ucfg_scan_unregister_requester(struct wlan_objmgr_psoc *psoc,
	wlan_scan_requester requester);


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
 * ucfg_scan_get_scan_id() - allocates scan ID
 * @psoc: psoc object
 *
 * API, allocates a new scan id for caller
 *
 * Return: newly allocated scan ID
 */
wlan_scan_id
ucfg_scan_get_scan_id(struct wlan_objmgr_psoc *psoc);

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
 * ucfg_scan_start() - Public API to start a scan
 * @req: start scan req params
 *
 * The Public API to start a scan. Post a msg to target_if queue
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
ucfg_scan_start(struct scan_start_request *req);

/**
 * ucfg_scan_set_enable() - Public API to disable/enable scans
 * @psoc: psoc on which scans need to be disabled
 * @enable: enable scan if true disable is false
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS ucfg_scan_set_enable(struct wlan_objmgr_psoc *psoc, bool enable);

/**
 * ucfg_scan_get_enable() - Public API to get if scan is enabled or disabled
 * @psoc: psoc on which scans status need to be checked
 *
 * Return: true if enabled else false.
 */
bool ucfg_scan_get_enable(struct wlan_objmgr_psoc *psoc);

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
 * ucfg_scan_cancel() - Public API to stop a scan
 * @req: stop scan request params
 *
 * The Public API to stop a scan. Post a msg to target_if queue
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
ucfg_scan_cancel(struct scan_cancel_request *req);

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
 * @chan_list: valid channel list
 * @num_chan: number of valid channels
 *
 * The Public API to to filter scan result
 * based on valid channel list.
 *
 * Return: void.
 */
void ucfg_scan_filter_valid_channel(struct wlan_objmgr_pdev *pdev,
	uint8_t *chan_list, uint32_t num_chan);

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
 * ucfg_scan_update_mlme_info() - The Public API to update mlme
 * info in the scan entry
 * @vdev: vdev object
 * @bss: bss info to find the matching scan entry
 * @mlme_info: mlme info to be updated.
 *
 * The Public API to update mlme info in the scan entry.
 * Post a msg to target_if queue
 *
 * Return: 0 for success or error code.
 */
QDF_STATUS
ucfg_scan_update_mlme_info(struct wlan_objmgr_vdev *vdev, struct bss_info *bss,
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
 * ucfg_scan_init_default_params() - get the defaults scan params
 * @vdev: vdev object
 * @req: scan request object
 *
 * get the defaults scan params
 *
 * Return: QDF_STATUS_SUCCESS or error code
 */
QDF_STATUS
ucfg_scan_init_default_params(struct wlan_objmgr_vdev *vdev,
	struct scan_start_request *req);

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
 * @pdev: vdev object
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

/**
 * ucfg_scan_update_roam_params() - Store/Update the roam params
 * @psoc: psoc
 * @roam_params: roam params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_update_roam_params(struct wlan_objmgr_psoc *psoc,
	struct roam_filter_params *roam_params);

/*
 * ucfg_scan_init() - Scan module initialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_scan_init(void);

/**
 * ucfg_scan_deinit() - Scan module deinitilization API
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
#endif

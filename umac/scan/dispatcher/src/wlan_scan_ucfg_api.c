/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

/*
 * DOC: contains scan north bound interface definitions
 */

#include <scheduler_api.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_serialization_api.h>
#include <wlan_scan_tgt_api.h>
#include <wlan_scan_utils_api.h>
#include <wlan_reg_ucfg_api.h>
#include <wlan_reg_services_api.h>
#include <wlan_utility.h>
#include "../../core/src/wlan_scan_main.h"
#include "../../core/src/wlan_scan_manager.h"
#include "../../core/src/wlan_scan_cache_db.h"
#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
#include <wlan_pmo_obj_mgmt_api.h>
#endif
#ifdef WLAN_POLICY_MGR_ENABLE
#include <wlan_dfs_utils_api.h>
#include <wlan_policy_mgr_api.h>
#endif
#include "cfg_ucfg_api.h"
#include "wlan_extscan_api.h"

QDF_STATUS ucfg_scan_register_bcn_cb(struct wlan_objmgr_psoc *psoc,
	update_beacon_cb cb, enum scan_cb_type type)
{
	return scm_scan_register_bcn_cb(psoc, cb, type);
}

qdf_list_t *ucfg_scan_get_result(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter)
{
	return scm_get_scan_result(pdev, filter);
}

QDF_STATUS ucfg_scan_db_iterate(struct wlan_objmgr_pdev *pdev,
	scan_iterator_func func, void *arg)
{
	return scm_iterate_scan_db(pdev, func, arg);
}

QDF_STATUS ucfg_scan_purge_results(qdf_list_t *scan_list)
{
	return scm_purge_scan_results(scan_list);
}

QDF_STATUS ucfg_scan_flush_results(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter)
{
	return scm_flush_results(pdev, filter);
}

void ucfg_scan_filter_valid_channel(struct wlan_objmgr_pdev *pdev,
	uint32_t *chan_freq_list, uint32_t num_chan)
{
	scm_filter_valid_channel(pdev, chan_freq_list, num_chan);
}

QDF_STATUS ucfg_scan_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_psoc_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("Failed to register psoc create handler");
		goto fail_create_psoc;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_psoc_destroyed_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("Failed to create psoc delete handler");
		goto fail_psoc_destroy;
	}
	scm_debug("scan psoc create and delete handler registered with objmgr");

	status = wlan_objmgr_register_vdev_create_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_vdev_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("Failed to register vdev create handler");
		goto fail_pdev_create;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_vdev_destroyed_notification, NULL);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		scm_debug("scan vdev create and delete handler registered with objmgr");
		return QDF_STATUS_SUCCESS;
	}

	scm_err("Failed to destroy vdev delete handler");
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_SCAN,
				wlan_scan_vdev_created_notification, NULL);
fail_pdev_create:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_SCAN,
				wlan_scan_psoc_destroyed_notification, NULL);
fail_psoc_destroy:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_SCAN,
			wlan_scan_psoc_created_notification, NULL);
fail_create_psoc:
	return status;
}

QDF_STATUS ucfg_scan_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_psoc_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("Failed to unregister psoc create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_SCAN,
				wlan_scan_psoc_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("Failed to unregister psoc delete handler");

	status = wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_SCAN,
		wlan_scan_vdev_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_vdev_destroy_handler(
			WLAN_UMAC_COMP_SCAN,
			wlan_scan_vdev_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("Failed to unregister vdev delete handler");

	return status;
}

#ifdef FEATURE_WLAN_SCAN_PNO

QDF_STATUS ucfg_scan_pno_start(struct wlan_objmgr_vdev *vdev,
	struct pno_scan_req_params *req)
{
	struct scan_vdev_obj *scan_vdev_obj;
	QDF_STATUS status;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return QDF_STATUS_E_INVAL;
	}
	if (scan_vdev_obj->pno_in_progress) {
		scm_err("pno already in progress");
		return QDF_STATUS_E_ALREADY;
	}

	status = tgt_scan_pno_start(vdev, req);
	if (QDF_IS_STATUS_ERROR(status))
		scm_err("pno start failed");
	else
		scan_vdev_obj->pno_in_progress = true;

	return status;
}

QDF_STATUS ucfg_scan_pno_stop(struct wlan_objmgr_vdev *vdev)
{
	struct scan_vdev_obj *scan_vdev_obj;
	QDF_STATUS status;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return QDF_STATUS_E_INVAL;
	}
	if (!scan_vdev_obj->pno_in_progress) {
		scm_debug("pno already stopped");
		return QDF_STATUS_E_ALREADY;
	}

	status = tgt_scan_pno_stop(vdev, wlan_vdev_get_id(vdev));
	if (QDF_IS_STATUS_ERROR(status))
		scm_err("pno start failed");
	else
		scan_vdev_obj->pno_in_progress = false;

	return status;
}

bool ucfg_scan_get_pno_in_progress(struct wlan_objmgr_vdev *vdev)
{
	struct scan_vdev_obj *scan_vdev_obj;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return false;
	}

	return scan_vdev_obj->pno_in_progress;
}

bool ucfg_scan_get_pno_match(struct wlan_objmgr_vdev *vdev)
{
	struct scan_vdev_obj *scan_vdev_obj;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return false;
	}

	return scan_vdev_obj->pno_match_evt_received;
}

static QDF_STATUS
wlan_pno_global_init(struct wlan_objmgr_psoc *psoc,
		     struct wlan_scan_obj *scan_obj)
{
	struct nlo_mawc_params *mawc_cfg;
	struct pno_def_config *pno_def;

	pno_def = &scan_obj->pno_cfg;
	qdf_wake_lock_create(&pno_def->pno_wake_lock, "wlan_pno_wl");
	mawc_cfg = &pno_def->mawc_params;
	pno_def->channel_prediction = cfg_get(psoc, CFG_PNO_CHANNEL_PREDICTION);
	pno_def->top_k_num_of_channels =
			cfg_get(psoc, CFG_TOP_K_NUM_OF_CHANNELS);
	pno_def->stationary_thresh = cfg_get(psoc, CFG_STATIONARY_THRESHOLD);
	pno_def->channel_prediction_full_scan =
			cfg_get(psoc, CFG_CHANNEL_PREDICTION_SCAN_TIMER);
	pno_def->adaptive_dwell_mode =
			cfg_get(psoc, CFG_ADAPTIVE_PNOSCAN_DWELL_MODE);
	pno_def->dfs_chnl_scan_enabled =
			cfg_get(psoc, CFG_ENABLE_DFS_PNO_CHNL_SCAN);
	pno_def->scan_support_enabled =
			cfg_get(psoc, CFG_PNO_SCAN_SUPPORT);
	pno_def->scan_timer_repeat_value =
			cfg_get(psoc, CFG_PNO_SCAN_TIMER_REPEAT_VALUE);
	pno_def->slow_scan_multiplier =
			cfg_get(psoc, CFG_PNO_SLOW_SCAN_MULTIPLIER);
	pno_def->scan_backoff_multiplier =
			cfg_get(psoc, CFG_SCAN_BACKOFF_MULTIPLIER);
	pno_def->max_sched_scan_plan_interval =
			cfg_get(psoc, CFG_MAX_SCHED_SCAN_PLAN_INTERVAL);
	pno_def->max_sched_scan_plan_iterations =
			cfg_get(psoc, CFG_MAX_SCHED_SCAN_PLAN_ITERATIONS);

	mawc_cfg->enable = cfg_get(psoc, CFG_MAWC_NLO_ENABLED);
	mawc_cfg->exp_backoff_ratio =
			cfg_get(psoc, CFG_MAWC_NLO_EXP_BACKOFF_RATIO);
	mawc_cfg->init_scan_interval =
			cfg_get(psoc, CFG_MAWC_NLO_INIT_SCAN_INTERVAL);
	mawc_cfg->max_scan_interval =
			cfg_get(psoc, CFG_MAWC_NLO_MAX_SCAN_INTERVAL);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
wlan_pno_global_deinit(struct wlan_scan_obj *scan_obj)
{
	qdf_wake_lock_destroy(&scan_obj->pno_cfg.pno_wake_lock);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_POLICY_MGR_ENABLE
/*
 * ucfg_scan_update_pno_dwell_time() - update active and passive dwell time
 * depending on active concurrency modes
 * @vdev: vdev object pointer
 * @req: scan request
 *
 * Return: void
 */
static void ucfg_scan_update_pno_dwell_time(struct wlan_objmgr_vdev *vdev,
	struct pno_scan_req_params *req, struct scan_default_params *scan_def)
{
	bool sap_or_p2p_present;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);

	if (!psoc)
		return;

	sap_or_p2p_present = policy_mgr_mode_specific_connection_count(
				psoc, PM_SAP_MODE, NULL) ||
				policy_mgr_mode_specific_connection_count(
				psoc, PM_P2P_GO_MODE, NULL) ||
				policy_mgr_mode_specific_connection_count(
				psoc, PM_P2P_CLIENT_MODE, NULL);

	if (sap_or_p2p_present) {
		req->active_dwell_time = scan_def->conc_active_dwell;
		req->passive_dwell_time = scan_def->conc_passive_dwell;
	}

}
#else
static inline void ucfg_scan_update_pno_dwell_time(struct wlan_objmgr_vdev *vdev,
	struct pno_scan_req_params *req, struct scan_default_params *scan_def){}
#endif

QDF_STATUS
ucfg_scan_get_pno_def_params(struct wlan_objmgr_vdev *vdev,
	struct pno_scan_req_params *req)
{
	struct scan_default_params *scan_def;
	struct wlan_scan_obj *scan;
	struct pno_def_config *pno_def;

	if (!vdev || !req) {
		scm_err("vdev: 0x%pK, req: 0x%pK",
			vdev, req);
		return QDF_STATUS_E_INVAL;
	}

	scan = wlan_vdev_get_scan_obj(vdev);
	if (!scan) {
		scm_err("scan is NULL");
		return QDF_STATUS_E_INVAL;
	}
	scan_def = wlan_vdev_get_def_scan_params(vdev);
	if (!scan_def) {
		scm_err("wlan_vdev_get_def_scan_params returned NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	pno_def = &scan->pno_cfg;
	req->active_dwell_time = scan_def->active_dwell;
	req->passive_dwell_time = scan_def->passive_dwell;
	req->scan_random.randomize = scan_def->enable_mac_spoofing;

	/*
	 *  Update active and passive dwell time depending
	 *  upon the present active concurrency mode
	 */
	ucfg_scan_update_pno_dwell_time(vdev, req, scan_def);
	req->adaptive_dwell_mode = pno_def->adaptive_dwell_mode;
	req->pno_channel_prediction = pno_def->channel_prediction;
	req->top_k_num_of_channels = pno_def->top_k_num_of_channels;
	req->stationary_thresh = pno_def->stationary_thresh;
	req->channel_prediction_full_scan =
			pno_def->channel_prediction_full_scan;
	req->mawc_params.vdev_id = wlan_vdev_get_id(vdev);
	qdf_mem_copy(&req->mawc_params, &pno_def->mawc_params,
			sizeof(req->mawc_params));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_register_pno_cb(struct wlan_objmgr_psoc *psoc,
	scan_event_handler event_cb, void *arg)
{
	struct wlan_scan_obj *scan;

	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_INVAL;
	}

	scan = wlan_psoc_get_scan_obj(psoc);
	if (!scan) {
		scm_err("scan object null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&scan->lock);
	scan->pno_cfg.pno_cb.func = event_cb;
	scan->pno_cfg.pno_cb.arg = arg;
	qdf_spin_unlock_bh(&scan->lock);
	scm_debug("event_cb: 0x%pK, arg: 0x%pK", event_cb, arg);

	return QDF_STATUS_SUCCESS;
}

#else

static inline QDF_STATUS
wlan_pno_global_init(struct wlan_objmgr_psoc *psoc,
		     struct wlan_scan_obj *scan_obj)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
wlan_pno_global_deinit(struct wlan_scan_obj *scan_obj)
{
	return QDF_STATUS_SUCCESS;
}

#endif

QDF_STATUS
ucfg_scan_set_custom_scan_chan_list(struct wlan_objmgr_pdev *pdev,
				    struct chan_list *chan_list)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;

	if (!pdev || !chan_list) {
		scm_warn("pdev: 0x%pK, chan_list: 0x%pK", pdev, chan_list);
		return QDF_STATUS_E_NULL_VALUE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	scan_obj = wlan_pdev_get_scan_obj(pdev);

	qdf_mem_copy(&scan_obj->pdev_info[pdev_id].custom_chan_list,
			chan_list, sizeof(*chan_list));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_start(struct scan_start_request *req)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	if (!req || !req->vdev) {
		scm_err("req or vdev within req is NULL");
		if (req)
			scm_scan_free_scan_request_mem(req);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!scm_is_scan_allowed(req->vdev)) {
		scm_err("scan disabled, rejecting the scan req");
		scm_scan_free_scan_request_mem(req);
		return QDF_STATUS_E_AGAIN;
	}

	scm_debug("reqid: %d, scanid: %d, vdevid: %d",
		  req->scan_req.scan_req_id, req->scan_req.scan_id,
		  req->scan_req.vdev_id);

	/* Try to get vdev reference. Return if reference could
	 * not be taken. Reference will be released once scan
	 * request handling completes along with free of @req.
	 */
	status = wlan_objmgr_vdev_try_get_ref(req->vdev, WLAN_SCAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_info("unable to get reference");
		scm_scan_free_scan_request_mem(req);
		return status;
	}

	msg.bodyptr = req;
	msg.callback = scm_scan_start_req;
	msg.flush_callback = scm_scan_start_flush_callback;

	status = scheduler_post_message(QDF_MODULE_ID_OS_IF,
					QDF_MODULE_ID_SCAN,
					QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);
		scm_scan_free_scan_request_mem(req);
	}

	return status;
}

QDF_STATUS ucfg_scan_psoc_set_enable(struct wlan_objmgr_psoc *psoc,
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

QDF_STATUS ucfg_scan_psoc_set_disable(struct wlan_objmgr_psoc *psoc,
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


QDF_STATUS ucfg_scan_vdev_set_enable(struct wlan_objmgr_vdev *vdev,
				     enum scan_disable_reason reason)
{
	struct scan_vdev_obj *scan_vdev_obj;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_vdev_obj->scan_disabled &= ~reason;

	scm_debug("Vdev scan_disabled %x", scan_vdev_obj->scan_disabled);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_scan_vdev_set_disable(struct wlan_objmgr_vdev *vdev,
				      enum scan_disable_reason reason)
{
	struct scan_vdev_obj *scan_vdev_obj;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return QDF_STATUS_E_NULL_VALUE;
	}

	scan_vdev_obj->scan_disabled |= reason;

	scm_debug("Vdev scan_disabled %x", scan_vdev_obj->scan_disabled);

	return QDF_STATUS_SUCCESS;
}


QDF_STATUS ucfg_scan_set_miracast(
	struct wlan_objmgr_psoc *psoc, bool enable)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_NULL_VALUE;
	}
	scan_obj->miracast_enabled = enable;
	scm_debug("set miracast_enable to %d", scan_obj->miracast_enabled);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_set_wide_band_scan(struct wlan_objmgr_pdev *pdev, bool enable)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;

	if (!pdev) {
		scm_warn("null vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	scan_obj = wlan_pdev_get_scan_obj(pdev);
	if (!scan_obj)
		return QDF_STATUS_E_FAILURE;

	scm_debug("set wide_band_scan to %d", enable);
	scan_obj->pdev_info[pdev_id].wide_band_scan = enable;

	return QDF_STATUS_SUCCESS;
}

bool ucfg_scan_get_wide_band_scan(struct wlan_objmgr_pdev *pdev)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;

	if (!pdev) {
		scm_warn("null vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	scan_obj = wlan_pdev_get_scan_obj(pdev);
	if (!scan_obj)
		return QDF_STATUS_E_FAILURE;

	return scan_obj->pdev_info[pdev_id].wide_band_scan;
}

#ifdef WLAN_DFS_CHAN_HIDDEN_SSID
QDF_STATUS
ucfg_scan_config_hidden_ssid_for_bssid(struct wlan_objmgr_pdev *pdev,
				       uint8_t *bssid, struct wlan_ssid *ssid)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;

	if (!pdev) {
		scm_warn("null vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	scan_obj = wlan_pdev_get_scan_obj(pdev);
	if (!scan_obj)
		return QDF_STATUS_E_FAILURE;

	scm_debug("Configure bsssid:%pM ssid:%.*s",
		  bssid, ssid->length, ssid->ssid);
	qdf_mem_copy(scan_obj->pdev_info[pdev_id].conf_bssid,
		     bssid, QDF_MAC_ADDR_SIZE);
	scan_obj->pdev_info[pdev_id].conf_ssid.length = ssid->length;
	qdf_mem_copy(scan_obj->pdev_info[pdev_id].conf_ssid.ssid,
		     ssid->ssid,
		     scan_obj->pdev_info[pdev_id].conf_ssid.length);

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_DFS_CHAN_HIDDEN_SSID */

QDF_STATUS
ucfg_scan_cancel(struct scan_cancel_request *req)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	if (!req || !req->vdev) {
		scm_err("req or vdev within req is NULL");
		if (req)
			qdf_mem_free(req);
		return QDF_STATUS_E_NULL_VALUE;
	}
	scm_debug("reqid: %d, scanid: %d, vdevid: %d, type: %d",
		  req->cancel_req.requester, req->cancel_req.scan_id,
		  req->cancel_req.vdev_id, req->cancel_req.req_type);

	status = wlan_objmgr_vdev_try_get_ref(req->vdev, WLAN_SCAN_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_info("Failed to get vdev ref; status:%d", status);
		goto req_free;
	}

	msg.bodyptr = req;
	msg.callback = scm_scan_cancel_req;
	msg.flush_callback = scm_scan_cancel_flush_callback;

	status = scheduler_post_message(QDF_MODULE_ID_OS_IF,
					QDF_MODULE_ID_SCAN,
					QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status))
		goto vdev_put;

	return QDF_STATUS_SUCCESS;

vdev_put:
	wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);

req_free:
	qdf_mem_free(req);

	return status;
}

QDF_STATUS
ucfg_scan_cancel_sync(struct scan_cancel_request *req)
{
	QDF_STATUS status;
	bool cancel_vdev = false, cancel_pdev = false;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	uint32_t max_wait_iterations = SCM_CANCEL_SCAN_WAIT_ITERATION;
	qdf_event_t cancel_scan_event;

	if (!req || !req->vdev) {
		scm_err("req or vdev within req is NULL");
		if (req)
			qdf_mem_free(req);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (req->cancel_req.req_type ==
	   WLAN_SCAN_CANCEL_PDEV_ALL)
		cancel_pdev = true;
	else if (req->cancel_req.req_type ==
	   WLAN_SCAN_CANCEL_VDEV_ALL)
		cancel_vdev = true;

	vdev = req->vdev;
	status = ucfg_scan_cancel(req);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	memset(&cancel_scan_event, 0, sizeof(cancel_scan_event));
	/*
	 * If cancel req is to cancel all scan of pdev or vdev
	 * wait until all scan of pdev or vdev get cancelled
	 */
	qdf_event_create(&cancel_scan_event);
	qdf_event_reset(&cancel_scan_event);

	if (cancel_pdev) {
		pdev = wlan_vdev_get_pdev(vdev);
		while ((ucfg_scan_get_pdev_status(pdev) !=
		     SCAN_NOT_IN_PROGRESS) && max_wait_iterations) {
			scm_debug("wait for all pdev scan to get complete");
				qdf_wait_single_event(&cancel_scan_event,
					SCM_CANCEL_SCAN_WAIT_TIME);
			max_wait_iterations--;
		}
	} else if (cancel_vdev) {
		while ((ucfg_scan_get_vdev_status(vdev) !=
		     SCAN_NOT_IN_PROGRESS) && max_wait_iterations) {
			scm_debug("wait for all vdev scan to get complete");
				qdf_wait_single_event(&cancel_scan_event,
					SCM_CANCEL_SCAN_WAIT_TIME);
			max_wait_iterations--;
		}
	}

	qdf_event_destroy(&cancel_scan_event);

	if (!max_wait_iterations) {
		scm_err("Failed to wait for scans to get complete");
		return QDF_STATUS_E_TIMEOUT;
	}

	return status;
}

wlan_scan_requester
ucfg_scan_register_requester(struct wlan_objmgr_psoc *psoc,
	uint8_t *name, scan_event_handler event_cb, void *arg)
{
	int i, j;
	struct wlan_scan_obj *scan;
	struct scan_requester_info *requesters;
	wlan_scan_requester requester = {0};

	if (!psoc) {
		scm_err("null psoc");
		return 0;
	}
	scan = wlan_psoc_get_scan_obj(psoc);
	if (!scan)
		return 0;

	requesters = scan->requesters;
	qdf_spin_lock_bh(&scan->lock);
	for (i = 0; i < WLAN_MAX_REQUESTORS; ++i) {
		if (requesters[i].requester == 0) {
			requesters[i].requester =
				WLAN_SCAN_REQUESTER_ID_PREFIX | i;
			j = 0;
			while (name[j] && (j < (WLAN_MAX_MODULE_NAME - 1))) {
				requesters[i].module[j] = name[j];
				++j;
			}
			requesters[i].module[j] = 0;
			requesters[i].ev_handler.func = event_cb;
			requesters[i].ev_handler.arg = arg;
			requester = requesters[i].requester;
			break;
		}
	}
	qdf_spin_unlock_bh(&scan->lock);
	scm_debug("module: %s, event_cb: 0x%pK, arg: 0x%pK, reqid: %d",
		  name, event_cb, arg, requester);

	return requester;
}

void
ucfg_scan_unregister_requester(struct wlan_objmgr_psoc *psoc,
	wlan_scan_requester requester)
{
	int idx;
	struct wlan_scan_obj *scan;
	struct scan_requester_info *requesters;

	idx = requester & WLAN_SCAN_REQUESTER_ID_PREFIX;
	if (idx != WLAN_SCAN_REQUESTER_ID_PREFIX) {
		scm_err("prefix didn't match for requester id %d", requester);
		return;
	}

	idx = requester & WLAN_SCAN_REQUESTER_ID_MASK;
	if (idx >= WLAN_MAX_REQUESTORS) {
		scm_err("requester id %d greater than max value", requester);
		return;
	}

	if (!psoc) {
		scm_err("null psoc");
		return;
	}
	scan = wlan_psoc_get_scan_obj(psoc);
	if (!scan)
		return;
	requesters = scan->requesters;
	scm_debug("reqid: %d", requester);

	qdf_spin_lock_bh(&scan->lock);
	requesters[idx].requester = 0;
	requesters[idx].module[0] = 0;
	requesters[idx].ev_handler.func = NULL;
	requesters[idx].ev_handler.arg = NULL;
	qdf_spin_unlock_bh(&scan->lock);
}

uint8_t*
ucfg_get_scan_requester_name(struct wlan_objmgr_psoc *psoc,
	wlan_scan_requester requester)
{
	int idx = requester & WLAN_SCAN_REQUESTER_ID_MASK;
	struct wlan_scan_obj *scan;
	struct scan_requester_info *requesters;

	if (!psoc) {
		scm_err("null psoc");
		return "null";
	}
	scan = wlan_psoc_get_scan_obj(psoc);
	if (!scan)
		return "null";

	requesters = scan->requesters;

	if ((idx < WLAN_MAX_REQUESTORS) &&
		(requesters[idx].requester == requester)) {
		return requesters[idx].module;
	}

	return (uint8_t *)"unknown";
}

wlan_scan_id
ucfg_scan_get_scan_id(struct wlan_objmgr_psoc *psoc)
{
	wlan_scan_id id;
	struct wlan_scan_obj *scan;

	if (!psoc) {
		QDF_ASSERT(0);
		scm_err("null psoc");
		return 0;
	}

	scan = wlan_psoc_get_scan_obj(psoc);
	if (!scan) {
		scm_err("scan object null");
		return 0;
	}

	id = qdf_atomic_inc_return(&scan->scan_ids);
	id =  id & WLAN_SCAN_ID_MASK;
	/* Mark this scan request as triggered by host
	 * by setting WLAN_HOST_SCAN_REQ_ID_PREFIX flag.
	 */
	id =  id | WLAN_HOST_SCAN_REQ_ID_PREFIX;
	scm_debug("scan_id: 0x%x", id);

	return id;
}

static QDF_STATUS
scm_add_scan_event_handler(struct pdev_scan_ev_handler *pdev_ev_handler,
	scan_event_handler event_cb, void *arg)
{
	struct cb_handler *cb_handler;
	uint32_t handler_cnt = pdev_ev_handler->handler_cnt;

	/* Assign next available slot to this registration request */
	cb_handler = &(pdev_ev_handler->cb_handlers[handler_cnt]);
	cb_handler->func = event_cb;
	cb_handler->arg = arg;
	pdev_ev_handler->handler_cnt++;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_register_event_handler(struct wlan_objmgr_pdev *pdev,
	scan_event_handler event_cb, void *arg)
{
	uint32_t idx;
	struct wlan_scan_obj *scan;
	struct pdev_scan_ev_handler *pdev_ev_handler;
	struct cb_handler *cb_handler;

	/* scan event handler call back can't be NULL */
	if (!pdev || !event_cb) {
		scm_err("pdev: %pK, event_cb: %pK", pdev, event_cb);
		return QDF_STATUS_E_NULL_VALUE;
	}

	scm_debug("pdev: %pK, event_cb: %pK, arg: %pK\n", pdev, event_cb, arg);

	scan = wlan_pdev_get_scan_obj(pdev);
	pdev_ev_handler = wlan_pdev_get_pdev_scan_ev_handlers(pdev);
	if (!pdev_ev_handler) {
		scm_err("null pdev_ev_handler");
		return QDF_STATUS_E_NULL_VALUE;
	}
	cb_handler = &(pdev_ev_handler->cb_handlers[0]);

	qdf_spin_lock_bh(&scan->lock);
	/* Ensure its not a duplicate registration request */
	for (idx = 0; idx < MAX_SCAN_EVENT_HANDLERS_PER_PDEV;
		idx++, cb_handler++) {
		if ((cb_handler->func == event_cb) &&
			(cb_handler->arg == arg)) {
			qdf_spin_unlock_bh(&scan->lock);
			scm_debug("func: %pK, arg: %pK already exists",
				  event_cb, arg);
			return QDF_STATUS_SUCCESS;
		}
	}

	QDF_ASSERT(pdev_ev_handler->handler_cnt <
			MAX_SCAN_EVENT_HANDLERS_PER_PDEV);

	if (pdev_ev_handler->handler_cnt >= MAX_SCAN_EVENT_HANDLERS_PER_PDEV) {
		qdf_spin_unlock_bh(&scan->lock);
		scm_warn("No more registrations possible");
		return QDF_STATUS_E_NOMEM;
	}

	scm_add_scan_event_handler(pdev_ev_handler, event_cb, arg);
	qdf_spin_unlock_bh(&scan->lock);

	scm_debug("event_cb: 0x%pK, arg: 0x%pK", event_cb, arg);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
wlan_scan_global_init(struct wlan_objmgr_psoc *psoc,
		      struct wlan_scan_obj *scan_obj)
{
	scan_obj->scan_disabled = 0;
	scan_obj->drop_bcn_on_chan_mismatch =
			 cfg_get(psoc, CFG_DROP_BCN_ON_CHANNEL_MISMATCH);
	scan_obj->disable_timeout = false;
	scan_obj->scan_def.active_dwell =
			 cfg_get(psoc, CFG_ACTIVE_MAX_CHANNEL_TIME);
	/* the ini is disallow DFS channel scan if ini is 1, so negate that */
	scan_obj->scan_def.allow_dfs_chan_in_first_scan =
				!cfg_get(psoc, CFG_INITIAL_NO_DFS_SCAN);
	scan_obj->scan_def.allow_dfs_chan_in_scan =
				cfg_get(psoc, CFG_ENABLE_DFS_SCAN);
	scan_obj->scan_def.skip_dfs_chan_in_p2p_search =
				cfg_get(psoc, CFG_ENABLE_SKIP_DFS_IN_P2P_SEARCH);
	scan_obj->scan_def.use_wake_lock_in_user_scan =
				cfg_get(psoc, CFG_ENABLE_WAKE_LOCK_IN_SCAN);
	scan_obj->scan_def.active_dwell_2g =
			 cfg_get(psoc, CFG_ACTIVE_MAX_2G_CHANNEL_TIME);
	scan_obj->scan_def.active_dwell_6g =
			 cfg_get(psoc, CFG_ACTIVE_MAX_6G_CHANNEL_TIME);
	scan_obj->scan_def.passive_dwell_6g =
			 cfg_get(psoc, CFG_PASSIVE_MAX_6G_CHANNEL_TIME);
	scan_obj->scan_def.passive_dwell =
			 cfg_get(psoc, CFG_PASSIVE_MAX_CHANNEL_TIME);
	scan_obj->scan_def.max_rest_time = SCAN_MAX_REST_TIME;
	scan_obj->scan_def.sta_miracast_mcc_rest_time =
					SCAN_STA_MIRACAST_MCC_REST_TIME;
	scan_obj->scan_def.min_rest_time = SCAN_MIN_REST_TIME;
	scan_obj->scan_def.conc_active_dwell =
			cfg_get(psoc, CFG_ACTIVE_MAX_CHANNEL_TIME_CONC);
	scan_obj->scan_def.conc_passive_dwell =
			cfg_get(psoc, CFG_PASSIVE_MAX_CHANNEL_TIME_CONC);
	scan_obj->scan_def.conc_max_rest_time =
			cfg_get(psoc, CFG_MAX_REST_TIME_CONC);
	scan_obj->scan_def.conc_min_rest_time =
			cfg_get(psoc, CFG_MIN_REST_TIME_CONC);
	scan_obj->scan_def.conc_idle_time =
			cfg_get(psoc, CFG_IDLE_TIME_CONC);
	scan_obj->scan_def.repeat_probe_time =
			cfg_get(psoc, CFG_SCAN_PROBE_REPEAT_TIME);
	scan_obj->scan_def.probe_spacing_time = SCAN_PROBE_SPACING_TIME;
	scan_obj->scan_def.probe_delay = SCAN_PROBE_DELAY;
	scan_obj->scan_def.burst_duration = SCAN_BURST_DURATION;
	scan_obj->scan_def.max_scan_time = SCAN_MAX_SCAN_TIME;
	scan_obj->scan_def.num_probes = cfg_get(psoc, CFG_SCAN_NUM_PROBES);
	scan_obj->scan_def.scan_cache_aging_time =
			(cfg_get(psoc, CFG_SCAN_AGING_TIME) * 1000);
	scan_obj->scan_def.max_bss_per_pdev = SCAN_MAX_BSS_PDEV;
	scan_obj->scan_def.scan_priority = SCAN_PRIORITY;
	scan_obj->scan_def.idle_time = SCAN_NETWORK_IDLE_TIMEOUT;
	scan_obj->scan_def.adaptive_dwell_time_mode =
			cfg_get(psoc, CFG_ADAPTIVE_SCAN_DWELL_MODE);
	scan_obj->scan_def.adaptive_dwell_time_mode_nc =
			cfg_get(psoc, CFG_ADAPTIVE_SCAN_DWELL_MODE_NC);
	scan_obj->scan_def.honour_nl_scan_policy_flags =
			cfg_get(psoc, CFG_HONOUR_NL_SCAN_POLICY_FLAGS);
	scan_obj->scan_def.enable_mac_spoofing =
			cfg_get(psoc, CFG_ENABLE_MAC_ADDR_SPOOFING);
	scan_obj->scan_def.is_bssid_hint_priority =
			cfg_get(psoc, CFG_IS_BSSID_HINT_PRIORITY);
	scan_obj->scan_def.extscan_adaptive_dwell_mode =
			cfg_get(psoc, CFG_ADAPTIVE_EXTSCAN_DWELL_MODE);

	/* init burst durations */
	scan_obj->scan_def.sta_scan_burst_duration =
				cfg_get(psoc, CFG_STA_SCAN_BURST_DURATION);
	scan_obj->scan_def.p2p_scan_burst_duration =
				cfg_get(psoc, CFG_P2P_SCAN_BURST_DURATION);
	scan_obj->scan_def.go_scan_burst_duration =
				cfg_get(psoc, CFG_GO_SCAN_BURST_DURATION);
	scan_obj->scan_def.ap_scan_burst_duration =
				cfg_get(psoc, CFG_AP_SCAN_BURST_DURATION);
	/* scan contrl flags */
	scan_obj->scan_def.scan_f_passive = true;
	scan_obj->scan_def.scan_f_ofdm_rates = true;
	scan_obj->scan_def.scan_f_2ghz = true;
	scan_obj->scan_def.scan_f_5ghz = true;
	scan_obj->scan_def.scan_f_chan_stat_evnt =
				cfg_get(psoc, CFG_ENABLE_SNR_MONITORING);
	/* scan event flags */
	scan_obj->scan_def.scan_ev_started = true;
	scan_obj->scan_def.scan_ev_completed = true;
	scan_obj->scan_def.scan_ev_bss_chan = true;
	scan_obj->scan_def.scan_ev_foreign_chan = true;
	scan_obj->scan_def.scan_ev_foreign_chn_exit = true;
	scan_obj->scan_def.scan_ev_dequeued = true;
	scan_obj->scan_def.scan_ev_preempted = true;
	scan_obj->scan_def.scan_ev_start_failed = true;
	scan_obj->scan_def.scan_ev_restarted = true;
	scan_obj->scan_def.enable_connected_scan =
		cfg_get(psoc, CFG_ENABLE_CONNECTED_SCAN);
	scan_obj->scan_def.scan_mode_6g = cfg_get(psoc, CFG_6GHZ_SCAN_MODE);
	/* init scan id seed */
	qdf_atomic_init(&scan_obj->scan_ids);

	/* init extscan */
	wlan_extscan_global_init(psoc, scan_obj);

	return wlan_pno_global_init(psoc, scan_obj);
}

static void
wlan_scan_global_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	wlan_pno_global_deinit(scan_obj);
	wlan_extscan_global_deinit();
}

static QDF_STATUS
scm_remove_scan_event_handler(struct pdev_scan_ev_handler *pdev_ev_handler,
	struct cb_handler *entry)
{
	struct cb_handler *last_entry;
	uint32_t handler_cnt = pdev_ev_handler->handler_cnt;

	/* Replace event handler being deleted
	 * with the last one in the list.
	 */
	last_entry = &(pdev_ev_handler->cb_handlers[handler_cnt - 1]);
	entry->func = last_entry->func;
	entry->arg = last_entry->arg;

	/* Clear our last entry */
	last_entry->func = NULL;
	last_entry->arg = NULL;
	pdev_ev_handler->handler_cnt--;

	return QDF_STATUS_SUCCESS;
}

void
ucfg_scan_unregister_event_handler(struct wlan_objmgr_pdev *pdev,
	scan_event_handler event_cb, void *arg)
{
	uint8_t found = false;
	uint32_t idx;
	uint32_t handler_cnt;
	struct wlan_scan_obj *scan;
	struct cb_handler *cb_handler;
	struct pdev_scan_ev_handler *pdev_ev_handler;

	scm_debug("pdev: %pK, event_cb: 0x%pK, arg: 0x%pK", pdev, event_cb,
		  arg);
	if (!pdev) {
		scm_err("null pdev");
		return;
	}
	scan = wlan_pdev_get_scan_obj(pdev);
	if (!scan)
		return;

	pdev_ev_handler = wlan_pdev_get_pdev_scan_ev_handlers(pdev);
	if (!pdev_ev_handler)
		return;

	cb_handler = &(pdev_ev_handler->cb_handlers[0]);

	qdf_spin_lock_bh(&scan->lock);
	handler_cnt = pdev_ev_handler->handler_cnt;
	if (!handler_cnt) {
		qdf_spin_unlock_bh(&scan->lock);
		scm_info("No event handlers registered");
		return;
	}

	for (idx = 0; idx < MAX_SCAN_EVENT_HANDLERS_PER_PDEV;
		idx++, cb_handler++) {
		if ((cb_handler->func == event_cb) &&
			(cb_handler->arg == arg)) {
			/* Event handler found, remove it
			 * from event handler list.
			 */
			found = true;
			scm_remove_scan_event_handler(pdev_ev_handler,
				cb_handler);
			handler_cnt--;
			break;
		}
	}
	qdf_spin_unlock_bh(&scan->lock);

	scm_debug("event handler %s, remaining handlers: %d",
		  (found ? "removed" : "not found"), handler_cnt);
}

QDF_STATUS
ucfg_scan_init_default_params(struct wlan_objmgr_vdev *vdev,
	struct scan_start_request *req)
{
	struct scan_default_params *def;

	if (!vdev | !req) {
		scm_err("vdev: 0x%pK, req: 0x%pK", vdev, req);
		return QDF_STATUS_E_INVAL;
	}
	def = wlan_vdev_get_def_scan_params(vdev);
	if (!def) {
		scm_err("wlan_vdev_get_def_scan_params returned NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Zero out everything and explicitly set fields as required */
	qdf_mem_zero(req, sizeof(*req));

	req->vdev = vdev;
	req->scan_req.vdev_id = wlan_vdev_get_id(vdev);
	req->scan_req.scan_type = SCAN_TYPE_DEFAULT;
	req->scan_req.scan_priority = def->scan_priority;
	req->scan_req.dwell_time_active = def->active_dwell;
	req->scan_req.dwell_time_active_2g = def->active_dwell_2g;
	req->scan_req.dwell_time_active_6g = def->active_dwell_6g;
	req->scan_req.dwell_time_passive_6g = def->passive_dwell_6g;
	req->scan_req.dwell_time_passive = def->passive_dwell;
	req->scan_req.min_rest_time = def->min_rest_time;
	req->scan_req.max_rest_time = def->max_rest_time;
	req->scan_req.repeat_probe_time = def->repeat_probe_time;
	req->scan_req.probe_spacing_time = def->probe_spacing_time;
	req->scan_req.idle_time = def->idle_time;
	req->scan_req.max_scan_time = def->max_scan_time;
	req->scan_req.probe_delay = def->probe_delay;
	req->scan_req.burst_duration = def->burst_duration;
	req->scan_req.n_probes = def->num_probes;
	req->scan_req.adaptive_dwell_time_mode =
		def->adaptive_dwell_time_mode;
	req->scan_req.scan_flags = def->scan_flags;
	req->scan_req.scan_events = def->scan_events;
	req->scan_req.scan_random.randomize = def->enable_mac_spoofing;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_init_ssid_params(struct scan_start_request *req,
		uint32_t num_ssid, struct wlan_ssid *ssid_list)
{
	uint32_t max_ssid = sizeof(req->scan_req.ssid) /
				sizeof(req->scan_req.ssid[0]);

	if (!req) {
		scm_err("null request");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!num_ssid) {
		/* empty channel list provided */
		req->scan_req.num_ssids = 0;
		qdf_mem_zero(&req->scan_req.ssid[0],
			sizeof(req->scan_req.ssid));
		return QDF_STATUS_SUCCESS;
	}
	if (!ssid_list) {
		scm_err("null ssid_list while num_ssid: %d", num_ssid);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (num_ssid > max_ssid) {
		/* got a big list. alert and continue */
		scm_warn("overflow: received %d, max supported : %d",
			num_ssid, max_ssid);
		return QDF_STATUS_E_E2BIG;
	}

	if (max_ssid > num_ssid)
		max_ssid = num_ssid;

	req->scan_req.num_ssids = max_ssid;
	qdf_mem_copy(&req->scan_req.ssid[0], ssid_list,
		(req->scan_req.num_ssids * sizeof(req->scan_req.ssid[0])));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_init_bssid_params(struct scan_start_request *req,
		uint32_t num_bssid, struct qdf_mac_addr *bssid_list)
{
	uint32_t max_bssid = sizeof(req->scan_req.bssid_list) /
				sizeof(req->scan_req.bssid_list[0]);

	if (!req) {
		scm_err("null request");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!num_bssid) {
		/* empty channel list provided */
		req->scan_req.num_bssid = 0;
		qdf_mem_zero(&req->scan_req.bssid_list[0],
			sizeof(req->scan_req.bssid_list));
		return QDF_STATUS_SUCCESS;
	}
	if (!bssid_list) {
		scm_err("null bssid_list while num_bssid: %d", num_bssid);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (num_bssid > max_bssid) {
		/* got a big list. alert and continue */
		scm_warn("overflow: received %d, max supported : %d",
			num_bssid, max_bssid);
		return QDF_STATUS_E_E2BIG;
	}

	if (max_bssid > num_bssid)
		max_bssid = num_bssid;

	req->scan_req.num_bssid = max_bssid;
	qdf_mem_copy(&req->scan_req.bssid_list[0], bssid_list,
		req->scan_req.num_bssid * sizeof(req->scan_req.bssid_list[0]));

	return QDF_STATUS_SUCCESS;
}

/**
 * is_chan_enabled_for_scan() - helper API to check if a frequency
 * is allowed to scan.
 * @reg_chan: regulatory_channel object
 * @low_2g: lower 2.4 GHz frequency thresold
 * @high_2g: upper 2.4 GHz frequency thresold
 * @low_5g: lower 5 GHz frequency thresold
 * @high_5g: upper 5 GHz frequency thresold
 *
 * Return: true if scan is allowed. false otherwise.
 */
static bool
is_chan_enabled_for_scan(struct regulatory_channel *reg_chan,
		qdf_freq_t low_2g, qdf_freq_t high_2g, qdf_freq_t low_5g,
		qdf_freq_t high_5g)
{
	if (reg_chan->state == CHANNEL_STATE_DISABLE)
		return false;
	if (reg_chan->nol_chan)
		return false;
	/* 2 GHz channel */
	if ((util_scan_scm_chan_to_band(reg_chan->chan_num) ==
			WLAN_BAND_2_4_GHZ) &&
			((reg_chan->center_freq < low_2g) ||
			(reg_chan->center_freq > high_2g)))
		return false;
	else if ((util_scan_scm_chan_to_band(reg_chan->chan_num) ==
				WLAN_BAND_5_GHZ) &&
		 ((reg_chan->center_freq < low_5g) ||
		  (reg_chan->center_freq > high_5g)))
		return false;

	return true;
}

QDF_STATUS
ucfg_scan_init_chanlist_params(struct scan_start_request *req,
		uint32_t num_chans, uint32_t *chan_list, uint32_t *phymode)
{
	uint32_t idx;
	QDF_STATUS status;
	struct regulatory_channel *reg_chan_list = NULL;
	qdf_freq_t low_2g, high_2g, low_5g, high_5g;
	struct wlan_objmgr_pdev *pdev = NULL;
	uint32_t *scan_freqs = NULL;
	uint32_t max_chans = sizeof(req->scan_req.chan_list.chan) /
				sizeof(req->scan_req.chan_list.chan[0]);
	if (!req) {
		scm_err("null request");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (req->vdev)
		pdev = wlan_vdev_get_pdev(req->vdev);
	/*
	 * If 0 channels are provided for scan and
	 * wide band scan is enabled, scan all 20 mhz
	 * available channels. This is required as FW
	 * scans all channel/phy mode combinations
	 * provided in scan channel list if 0 chans are
	 * provided in scan request causing scan to take
	 * too much time to complete.
	 */
	if (pdev && !num_chans) {
		reg_chan_list = qdf_mem_malloc_atomic(NUM_CHANNELS *
				sizeof(struct regulatory_channel));
		if (!reg_chan_list) {
			scm_err("Couldn't allocate reg_chan_list memory");
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		scan_freqs =
			qdf_mem_malloc_atomic(sizeof(uint32_t) * max_chans);
		if (!scan_freqs) {
			scm_err("Couldn't allocate scan_freqs memory");
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
		status = ucfg_reg_get_current_chan_list(pdev, reg_chan_list);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("Couldn't get current chan list");
			goto end;
		}
		status = wlan_reg_get_freq_range(pdev, &low_2g,
				&high_2g, &low_5g, &high_5g);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("Couldn't get frequency range");
			goto end;
		}

		for (idx = 0, num_chans = 0;
			(idx < NUM_CHANNELS && num_chans < max_chans); idx++)
			if (is_chan_enabled_for_scan(&reg_chan_list[idx],
					low_2g, high_2g, low_5g, high_5g))
				scan_freqs[num_chans++] =
				reg_chan_list[idx].center_freq;

		chan_list = scan_freqs;
	}

	if (!num_chans) {
		/* empty channel list provided */
		qdf_mem_zero(&req->scan_req.chan_list,
			sizeof(req->scan_req.chan_list));
		req->scan_req.chan_list.num_chan = 0;
		status = QDF_STATUS_SUCCESS;
		goto end;
	}
	if (!chan_list) {
		scm_err("null chan_list while num_chans: %d", num_chans);
		status = QDF_STATUS_E_NULL_VALUE;
		goto end;
	}

	if (num_chans > max_chans) {
		/* got a big list. alert and fail */
		scm_warn("overflow: received %d, max supported : %d",
			num_chans, max_chans);
		status = QDF_STATUS_E_E2BIG;
		goto end;
	}

	req->scan_req.chan_list.num_chan = num_chans;
	for (idx = 0; idx < num_chans; idx++) {
		req->scan_req.chan_list.chan[idx].freq =
			(chan_list[idx] > WLAN_24_GHZ_BASE_FREQ) ?
			chan_list[idx] :
			wlan_reg_chan_to_freq(pdev, chan_list[idx]);
		if (phymode)
			req->scan_req.chan_list.chan[idx].phymode =
				phymode[idx];
		else if (req->scan_req.chan_list.chan[idx].freq <=
			WLAN_CHAN_15_FREQ)
			req->scan_req.chan_list.chan[idx].phymode =
				SCAN_PHY_MODE_11G;
		else
			req->scan_req.chan_list.chan[idx].phymode =
				SCAN_PHY_MODE_11A;

		scm_debug("chan[%d]: freq:%d, phymode:%d", idx,
			req->scan_req.chan_list.chan[idx].freq,
			req->scan_req.chan_list.chan[idx].phymode);
	}

end:
	if (scan_freqs)
		qdf_mem_free(scan_freqs);

	if (reg_chan_list)
		qdf_mem_free(reg_chan_list);

	return QDF_STATUS_SUCCESS;
}

static inline enum scm_scan_status
get_scan_status_from_serialization_status(
	enum wlan_serialization_cmd_status status)
{
	enum scm_scan_status scan_status;

	switch (status) {
	case WLAN_SER_CMD_IN_PENDING_LIST:
		scan_status = SCAN_IS_PENDING;
		break;
	case WLAN_SER_CMD_IN_ACTIVE_LIST:
		scan_status = SCAN_IS_ACTIVE;
		break;
	case WLAN_SER_CMDS_IN_ALL_LISTS:
		scan_status = SCAN_IS_ACTIVE_AND_PENDING;
		break;
	case WLAN_SER_CMD_NOT_FOUND:
		scan_status = SCAN_NOT_IN_PROGRESS;
		break;
	default:
		scm_warn("invalid serialization status %d", status);
		QDF_ASSERT(0);
		scan_status = SCAN_NOT_IN_PROGRESS;
		break;
	}

	return scan_status;
}

enum scm_scan_status
ucfg_scan_get_vdev_status(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_serialization_cmd_status status;

	if (!vdev) {
		scm_err("null vdev");
		return SCAN_NOT_IN_PROGRESS;
	}
	status = wlan_serialization_vdev_scan_status(vdev);

	return get_scan_status_from_serialization_status(status);
}

enum scm_scan_status
ucfg_scan_get_pdev_status(struct wlan_objmgr_pdev *pdev)
{
	enum wlan_serialization_cmd_status status;

	if (!pdev) {
		scm_err("null pdev");
		return SCAN_NOT_IN_PROGRESS;
	}
	status = wlan_serialization_pdev_scan_status(pdev);

	return get_scan_status_from_serialization_status(status);
}

static void
ucfg_scan_register_unregister_bcn_cb(struct wlan_objmgr_psoc *psoc,
	bool enable)
{
	QDF_STATUS status;
	struct mgmt_txrx_mgmt_frame_cb_info cb_info[2];

	cb_info[0].frm_type = MGMT_PROBE_RESP;
	cb_info[0].mgmt_rx_cb = tgt_scan_bcn_probe_rx_callback;
	cb_info[1].frm_type = MGMT_BEACON;
	cb_info[1].mgmt_rx_cb = tgt_scan_bcn_probe_rx_callback;

	if (enable)
		status = wlan_mgmt_txrx_register_rx_cb(psoc,
					 WLAN_UMAC_COMP_SCAN, cb_info, 2);
	else
		status = wlan_mgmt_txrx_deregister_rx_cb(psoc,
					 WLAN_UMAC_COMP_SCAN, cb_info, 2);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("%s the Handle with MGMT TXRX layer has failed",
			enable ? "Registering" : "Deregistering");
}

QDF_STATUS ucfg_scan_update_user_config(struct wlan_objmgr_psoc *psoc,
	struct scan_user_cfg *scan_cfg)
{
	struct wlan_scan_obj *scan_obj;
	struct scan_default_params *scan_def;

	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}
	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_FAILURE;
	}

	scan_def = &scan_obj->scan_def;
	scan_obj->ie_whitelist = scan_cfg->ie_whitelist;
	scan_def->sta_miracast_mcc_rest_time =
				scan_cfg->sta_miracast_mcc_rest_time;

	qdf_mem_copy(&scan_def->score_config, &scan_cfg->score_config,
		sizeof(struct scoring_config));
	scm_validate_scoring_config(&scan_def->score_config);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
static QDF_STATUS
ucfg_scan_cancel_pdev_scan(struct wlan_objmgr_pdev *pdev)
{
	struct scan_cancel_request *req;
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	req = qdf_mem_malloc_atomic(sizeof(*req));
	if (!req) {
		scm_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_SCAN_ID);
	if (!vdev) {
		scm_err("Failed to get vdev");
		qdf_mem_free(req);
		return QDF_STATUS_E_INVAL;
	}
	req->vdev = vdev;
	req->cancel_req.scan_id = INVAL_SCAN_ID;
	req->cancel_req.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	req->cancel_req.vdev_id = INVAL_VDEV_ID;
	req->cancel_req.req_type = WLAN_SCAN_CANCEL_PDEV_ALL;
	status = ucfg_scan_cancel_sync(req);
	if (QDF_IS_STATUS_ERROR(status))
		scm_err("Cancel scan request failed");
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SCAN_ID);

	return status;
}

static QDF_STATUS
ucfg_scan_suspend_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i;

	ucfg_scan_psoc_set_disable(psoc, REASON_SUSPEND);
	/* Check all pdev */
	for (i = 0; i < WLAN_UMAC_MAX_PDEVS; i++) {
		pdev = wlan_objmgr_get_pdev_by_id(psoc, i, WLAN_SCAN_ID);
		if (!pdev)
			continue;
		if (ucfg_scan_get_pdev_status(pdev) !=
		    SCAN_NOT_IN_PROGRESS)
			status = ucfg_scan_cancel_pdev_scan(pdev);
		wlan_objmgr_pdev_release_ref(pdev, WLAN_SCAN_ID);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_err("failed to cancel scan for pdev_id %d", i);
			return status;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
ucfg_scan_resume_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	ucfg_scan_psoc_set_enable(psoc, REASON_SUSPEND);
	return QDF_STATUS_SUCCESS;
}

static inline void
ucfg_scan_register_pmo_handler(void)
{
	pmo_register_suspend_handler(WLAN_UMAC_COMP_SCAN,
		ucfg_scan_suspend_handler, NULL);
	pmo_register_resume_handler(WLAN_UMAC_COMP_SCAN,
		ucfg_scan_resume_handler, NULL);
}

static inline void
ucfg_scan_unregister_pmo_handler(void)
{
	pmo_unregister_suspend_handler(WLAN_UMAC_COMP_SCAN,
		ucfg_scan_suspend_handler);
	pmo_unregister_resume_handler(WLAN_UMAC_COMP_SCAN,
		ucfg_scan_resume_handler);
}

#else
static inline void
ucfg_scan_register_pmo_handler(void)
{
}

static inline void
ucfg_scan_unregister_pmo_handler(void)
{
}
#endif

QDF_STATUS
ucfg_scan_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scm_debug("psoc open: 0x%pK", psoc);
	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}
	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_FAILURE;
	}
	/* Initialize the scan Globals */
	wlan_scan_global_init(psoc, scan_obj);
	qdf_spinlock_create(&scan_obj->lock);
	ucfg_scan_register_pmo_handler();
	scm_db_init(psoc);
	scm_channel_list_db_init(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_scan_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scm_debug("psoc close: 0x%pK", psoc);
	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}
	scm_db_deinit(psoc);
	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return QDF_STATUS_E_FAILURE;
	}
	ucfg_scan_unregister_pmo_handler();
	qdf_spinlock_destroy(&scan_obj->lock);
	wlan_scan_global_deinit(psoc);
	scm_channel_list_db_deinit(psoc);

	return QDF_STATUS_SUCCESS;
}

static bool scm_serialization_scan_rules_cb(
		union wlan_serialization_rules_info *comp_info,
		uint8_t comp_id)
{
	switch (comp_id) {
	case WLAN_UMAC_COMP_TDLS:
		if (comp_info->scan_info.is_tdls_in_progress) {
			scm_debug("Cancel scan. Tdls in progress");
			return false;
		}
		break;
	case WLAN_UMAC_COMP_DFS:
		if (comp_info->scan_info.is_cac_in_progress) {
			scm_debug("Cancel scan. CAC in progress");
			return false;
		}
		break;
	case WLAN_UMAC_COMP_MLME:
		if (comp_info->scan_info.is_mlme_op_in_progress) {
			scm_debug("Cancel scan. MLME operation in progress");
			return false;
		}
		break;
	default:
		scm_debug("not handled comp_id %d", comp_id);
		break;
	}

	return true;
}

QDF_STATUS
ucfg_scan_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	scm_debug("psoc enable: 0x%pK", psoc);
	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}
	/* Subscribe for scan events from lmac layesr */
	status = tgt_scan_register_ev_handler(psoc);
	QDF_ASSERT(status == QDF_STATUS_SUCCESS);
	if (!wlan_reg_is_11d_offloaded(psoc))
		scm_11d_cc_db_init(psoc);
	ucfg_scan_register_unregister_bcn_cb(psoc, true);
	status = wlan_serialization_register_apply_rules_cb(psoc,
				WLAN_SER_CMD_SCAN,
				scm_serialization_scan_rules_cb);
	QDF_ASSERT(status == QDF_STATUS_SUCCESS);
	return status;
}

QDF_STATUS
ucfg_scan_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	scm_debug("psoc disable: 0x%pK", psoc);
	if (!psoc) {
		scm_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}
	/* Unsubscribe for scan events from lmac layesr */
	status = tgt_scan_unregister_ev_handler(psoc);
	QDF_ASSERT(status == QDF_STATUS_SUCCESS);
	ucfg_scan_register_unregister_bcn_cb(psoc, false);
	if (!wlan_reg_is_11d_offloaded(psoc))
		scm_11d_cc_db_deinit(psoc);

	return status;
}

uint32_t
ucfg_scan_get_max_active_scans(struct wlan_objmgr_psoc *psoc)
{
	struct scan_default_params *scan_params = NULL;

	if (!psoc) {
		scm_err("null psoc");
		return 0;
	}
	scan_params = wlan_scan_psoc_get_def_params(psoc);
	if (!scan_params) {
		scm_err("Failed to get scan object");
		return 0;
	}

	return scan_params->max_active_scans_allowed;
}

bool ucfg_copy_ie_whitelist_attrs(struct wlan_objmgr_psoc *psoc,
				  struct probe_req_whitelist_attr *ie_whitelist)
{
	struct wlan_scan_obj *scan_obj = NULL;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return false;

	qdf_mem_copy(ie_whitelist, &scan_obj->ie_whitelist,
		     sizeof(*ie_whitelist));

	return true;
}

bool ucfg_ie_whitelist_enabled(struct wlan_objmgr_psoc *psoc,
			       struct wlan_objmgr_vdev *vdev)
{
	struct wlan_scan_obj *scan_obj = NULL;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return false;

	if ((wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE) ||
	    wlan_vdev_is_up(vdev) == QDF_STATUS_SUCCESS)
		return false;

	if (!scan_obj->ie_whitelist.white_list)
		return false;

	return true;
}

void ucfg_scan_set_bt_activity(struct wlan_objmgr_psoc *psoc,
			       bool bt_a2dp_active)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return;
	}
	scan_obj->bt_a2dp_enabled = bt_a2dp_active;
}

bool ucfg_scan_get_bt_activity(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return false;
	}

	return scan_obj->bt_a2dp_enabled;
}

bool ucfg_scan_wake_lock_in_user_scan(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj)
		return false;

	return scan_obj->scan_def.use_wake_lock_in_user_scan;
}

bool ucfg_scan_is_connected_scan_enabled(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return cfg_default(CFG_ENABLE_CONNECTED_SCAN);
	}

	return scan_obj->scan_def.enable_connected_scan;
}

bool ucfg_scan_is_mac_spoofing_enabled(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return cfg_default(CFG_ENABLE_MAC_ADDR_SPOOFING);
	}

	return scan_obj->scan_def.enable_mac_spoofing;
}

enum scan_dwelltime_adaptive_mode
ucfg_scan_get_extscan_adaptive_dwell_mode(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return cfg_default(CFG_ADAPTIVE_EXTSCAN_DWELL_MODE);
	}

	return scan_obj->scan_def.extscan_adaptive_dwell_mode;
}

QDF_STATUS
ucfg_scan_set_global_config(struct wlan_objmgr_psoc *psoc,
			       enum scan_config config, uint32_t val)
{
	struct wlan_scan_obj *scan_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object config:%d, val:%d",
				config, val);
		return QDF_STATUS_E_INVAL;
	}
	switch (config) {
	case SCAN_CFG_DISABLE_SCAN_COMMAND_TIMEOUT:
		scan_obj->disable_timeout = !!val;
		break;
	case SCAN_CFG_DROP_BCN_ON_CHANNEL_MISMATCH:
		scan_obj->drop_bcn_on_chan_mismatch = !!val;
		break;

	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

QDF_STATUS ucfg_scan_update_mlme_by_bssinfo(struct wlan_objmgr_pdev *pdev,
		struct bss_info *bss_info, struct mlme_info *mlme)
{
	QDF_STATUS status;

	status = scm_scan_update_mlme_by_bssinfo(pdev, bss_info, mlme);

	return status;
}

QDF_STATUS
ucfg_scan_get_global_config(struct wlan_objmgr_psoc *psoc,
			       enum scan_config config, uint32_t *val)
{
	struct wlan_scan_obj *scan_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj || !val) {
		scm_err("scan object:%pK config:%d, val:0x%pK",
				scan_obj, config, val);
		return QDF_STATUS_E_INVAL;
	}
	switch (config) {
	case SCAN_CFG_DISABLE_SCAN_COMMAND_TIMEOUT:
		*val = scan_obj->disable_timeout;
		break;
	case SCAN_CFG_DROP_BCN_ON_CHANNEL_MISMATCH:
		*val = scan_obj->drop_bcn_on_chan_mismatch;
		break;

	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

#ifdef FEATURE_WLAN_SCAN_PNO
bool ucfg_scan_is_pno_offload_enabled(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("NULL scan obj");
		return false;
	}

	return scan_obj->pno_cfg.pno_offload_enabled;
}

void ucfg_scan_set_pno_offload(struct wlan_objmgr_psoc *psoc, bool value)
{
	 struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("NULL scan obj");
		return;
	}

	scan_obj->pno_cfg.pno_offload_enabled = value;
}

bool ucfg_scan_get_pno_scan_support(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("NULL scan obj");
		return cfg_default(CFG_PNO_SCAN_SUPPORT);
	}

	return scan_obj->pno_cfg.scan_support_enabled;
}

uint8_t ucfg_get_scan_backoff_multiplier(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("NULL scan obj");
		return cfg_default(CFG_SCAN_BACKOFF_MULTIPLIER);
	}
	return scan_obj->pno_cfg.scan_backoff_multiplier;
}

bool ucfg_scan_is_dfs_chnl_scan_enabled(struct wlan_objmgr_psoc *psoc)
{
		struct wlan_scan_obj *scan_obj;

		scan_obj = wlan_psoc_get_scan_obj(psoc);
		if (!scan_obj) {
			scm_err("NULL scan obj");
			return cfg_default(CFG_ENABLE_DFS_PNO_CHNL_SCAN);
		}
		return scan_obj->pno_cfg.dfs_chnl_scan_enabled;
}

uint32_t ucfg_scan_get_scan_timer_repeat_value(struct wlan_objmgr_psoc *psoc)
{
		struct wlan_scan_obj *scan_obj;

		scan_obj = wlan_psoc_get_scan_obj(psoc);
		if (!scan_obj) {
			scm_err("NULL scan obj");
			return cfg_default(CFG_PNO_SCAN_TIMER_REPEAT_VALUE);
		}
		return scan_obj->pno_cfg.scan_timer_repeat_value;
}

uint32_t ucfg_scan_get_slow_scan_multiplier(struct wlan_objmgr_psoc *psoc)
{
		struct wlan_scan_obj *scan_obj;

		scan_obj = wlan_psoc_get_scan_obj(psoc);
		if (!scan_obj) {
			scm_err("NULL scan obj");
			return cfg_default(CFG_PNO_SLOW_SCAN_MULTIPLIER);
		}
		return scan_obj->pno_cfg.slow_scan_multiplier;
}

uint32_t
ucfg_scan_get_max_sched_scan_plan_interval(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return cfg_default(CFG_MAX_SCHED_SCAN_PLAN_INTERVAL);
	}

	return scan_obj->pno_cfg.max_sched_scan_plan_interval;
}

uint32_t
ucfg_scan_get_max_sched_scan_plan_iterations(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("Failed to get scan object");
		return cfg_default(CFG_MAX_SCHED_SCAN_PLAN_ITERATIONS);
	}

	return scan_obj->pno_cfg.max_sched_scan_plan_iterations;
}

#endif

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
 * DOC: contains scan manager functionality
 */

#include <wlan_serialization_api.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_tgt_api.h>
#include "wlan_scan_main.h"
#include "wlan_scan_manager.h"
#include "wlan_utility.h"
#include <wlan_reg_services_api.h>
#ifdef FEATURE_WLAN_SCAN_PNO
#include <host_diag_core_event.h>
#endif
#ifdef WLAN_POLICY_MGR_ENABLE
#include <wlan_policy_mgr_api.h>
#endif
#include <wlan_dfs_utils_api.h>
#include <cfg_scan.h>

QDF_STATUS
scm_scan_free_scan_request_mem(struct scan_start_request *req)
{
	void *ie;

	if (!req) {
		scm_err("null request");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	/* Free vendor(extra) ie */
	ie = req->scan_req.extraie.ptr;
	if (ie) {
		req->scan_req.extraie.ptr = NULL;
		req->scan_req.extraie.len = 0;
		qdf_mem_free(ie);
	}

	/* Free htcap ie */
	ie = req->scan_req.htcap.ptr;
	if (ie) {
		req->scan_req.htcap.len = 0;
		req->scan_req.htcap.ptr = NULL;
		qdf_mem_free(ie);
	}

	/* Free vhtcap ie */
	ie = req->scan_req.vhtcap.ptr;
	if (ie) {
		req->scan_req.vhtcap.len = 0;
		req->scan_req.vhtcap.ptr = NULL;
		qdf_mem_free(ie);
	}
	/* free scan_start_request memory */
	qdf_mem_free(req);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
scm_scan_get_pdev_global_event_handlers(struct scan_event_listeners *listeners,
		struct pdev_scan_ev_handler *pdev_ev_handler)
{
	uint32_t i;
	struct cb_handler *cb_handlers  = &(pdev_ev_handler->cb_handlers[0]);

	for (i = 0; i < MAX_SCAN_EVENT_HANDLERS_PER_PDEV; i++, cb_handlers++) {
		if ((cb_handlers->func) &&
		    (listeners->count < MAX_SCAN_EVENT_LISTENERS)) {
			listeners->cb[listeners->count].func =
				cb_handlers->func;
			listeners->cb[listeners->count].arg =
				cb_handlers->arg;
			listeners->count++;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
scm_scan_get_requester_event_handler(struct scan_event_listeners *listeners,
		struct scan_requester_info *requesters,
		wlan_scan_requester requester_id)
{
	uint32_t idx;
	struct cb_handler *ev_handler;

	idx = requester_id & WLAN_SCAN_REQUESTER_ID_PREFIX;
	if (idx != WLAN_SCAN_REQUESTER_ID_PREFIX)
		return QDF_STATUS_SUCCESS;

	idx = requester_id & WLAN_SCAN_REQUESTER_ID_MASK;
	if (idx < WLAN_MAX_REQUESTORS) {
		ev_handler = &(requesters[idx].ev_handler);
		if (ev_handler->func) {
			if (listeners->count < MAX_SCAN_EVENT_LISTENERS) {
				listeners->cb[listeners->count].func =
							     ev_handler->func;
				listeners->cb[listeners->count].arg =
							     ev_handler->arg;
				listeners->count++;
			}
		}
		return QDF_STATUS_SUCCESS;
	} else {
		scm_err("invalid requester id");
		return QDF_STATUS_E_INVAL;
	}

}

static void scm_scan_post_event(struct wlan_objmgr_vdev *vdev,
		struct scan_event *event)
{
	uint32_t i = 0;
	struct wlan_scan_obj *scan;
	struct pdev_scan_ev_handler *pdev_ev_handler;
	struct cb_handler *cb_handlers;
	struct scan_requester_info *requesters;
	struct scan_event_listeners *listeners;

	if (!vdev || !event) {
		scm_err("vdev: 0x%pK, event: 0x%pK", vdev, event);
		return;
	}
	if (!event->requester) {
		scm_err("invalid requester id");
		QDF_ASSERT(0);
	}
	scan = wlan_vdev_get_scan_obj(vdev);
	pdev_ev_handler = wlan_vdev_get_pdev_scan_ev_handlers(vdev);
	if (!pdev_ev_handler)
		return;
	cb_handlers = &(pdev_ev_handler->cb_handlers[0]);
	requesters = scan->requesters;

	listeners = qdf_mem_malloc_atomic(sizeof(*listeners));
	if (!listeners) {
		scm_warn("couldn't allocate listeners list");
		return;
	}

	/* initialize number of listeners */
	listeners->count = 0;

	/*
	 * Initiator of scan request decides which all scan events
	 * he is interested in and FW will send only those scan events
	 * to host driver.
	 * All the events received by scan module will be notified
	 * to all registered handlers.
	 */

	qdf_spin_lock_bh(&scan->lock);
	/* find all global scan event handlers on this pdev */
	scm_scan_get_pdev_global_event_handlers(listeners, pdev_ev_handler);
	/* find owner who triggered this scan request */
	scm_scan_get_requester_event_handler(listeners, requesters,
			event->requester);
	qdf_spin_unlock_bh(&scan->lock);

	scm_listener_duration_init(scan);

	/* notify all interested handlers */
	for (i = 0; i < listeners->count; i++) {
		scm_listener_cb_exe_dur_start(scan, i);
		listeners->cb[i].func(vdev, event, listeners->cb[i].arg);
		scm_listener_cb_exe_dur_end(scan, i);
	}
	qdf_mem_free(listeners);
}

static QDF_STATUS
scm_release_serialization_command(struct wlan_objmgr_vdev *vdev,
		uint32_t scan_id)
{
	struct wlan_serialization_queued_cmd_info cmd = {0};

	cmd.requestor = WLAN_UMAC_COMP_SCAN;
	cmd.cmd_type = WLAN_SER_CMD_SCAN;
	cmd.cmd_id = scan_id;
	cmd.req_type = WLAN_SER_CANCEL_SINGLE_SCAN;
	cmd.vdev = vdev;
	cmd.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;

	/* Inform serialization for command completion */
	wlan_serialization_remove_cmd(&cmd);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
scm_post_internal_scan_complete_event(struct scan_start_request *req,
		enum scan_completion_reason reason)
{
	struct scan_event event = {0, };

	/* prepare internal scan complete event */
	event.type = SCAN_EVENT_TYPE_COMPLETED;
	event.reason = reason;
	event.chan_freq = 0; /* Invalid frequency */
	event.vdev_id =  req->scan_req.vdev_id;
	event.requester = req->scan_req.scan_req_id;
	event.scan_id = req->scan_req.scan_id;
	/* Fill scan_start_request used to trigger this scan */
	event.scan_start_req = req;
	/* post scan event to registered handlers */
	scm_scan_post_event(req->vdev, &event);

	return QDF_STATUS_SUCCESS;
}

static inline struct pdev_scan_info *
scm_scan_get_pdev_priv_info(uint8_t pdev_id, struct wlan_scan_obj *scan_obj)
{
	return &scan_obj->pdev_info[pdev_id];
}

static QDF_STATUS
scm_update_last_scan_time(struct scan_start_request *req)
{
	uint8_t pdev_id;
	struct wlan_scan_obj *scan_obj;
	struct pdev_scan_info *pdev_scan_info;

	scan_obj = wlan_vdev_get_scan_obj(req->vdev);
	pdev_id = wlan_scan_vdev_get_pdev_id(req->vdev);
	pdev_scan_info = scm_scan_get_pdev_priv_info(pdev_id, scan_obj);
	/* update last scan start time */
	pdev_scan_info->last_scan_time = qdf_system_ticks();

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
scm_activate_scan_request(struct scan_start_request *req)
{
	QDF_STATUS status;

	status = tgt_scan_start(req);
	if (status != QDF_STATUS_SUCCESS) {
		scm_err("tgt_scan_start failed, status: %d", status);
		/* scan could not be started and hence
		 * we will not receive any completions.
		 * post scan cancelled
		 */
		scm_post_internal_scan_complete_event(req,
				SCAN_REASON_CANCELLED);
		return status;
	}
	/* save last scan start time */
	status = scm_update_last_scan_time(req);

	return status;
}

static QDF_STATUS
scm_cancel_scan_request(struct scan_start_request *req)
{
	struct scan_cancel_request cancel_req = {0, };
	QDF_STATUS status;

	cancel_req.vdev = req->vdev;
	cancel_req.cancel_req.scan_id = req->scan_req.scan_id;
	cancel_req.cancel_req.requester = req->scan_req.scan_req_id;
	cancel_req.cancel_req.req_type = WLAN_SCAN_CANCEL_SINGLE;
	cancel_req.cancel_req.vdev_id = req->scan_req.vdev_id;
	/* send scan cancel to fw */
	status = tgt_scan_cancel(&cancel_req);
	if (status != QDF_STATUS_SUCCESS)
		scm_err("tgt_scan_cancel failed: status: %d, scanid: %d",
			status, req->scan_req.scan_id);
	/* notify event handler about scan cancellation */
	scm_post_internal_scan_complete_event(req, SCAN_REASON_CANCELLED);

	return status;
}

static QDF_STATUS
scm_scan_serialize_callback(struct wlan_serialization_command *cmd,
	enum wlan_serialization_cb_reason reason)
{
	struct scan_start_request *req;
	QDF_STATUS status;

	if (!cmd) {
		scm_err("cmd is NULL, reason: %d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!cmd->umac_cmd) {
		scm_err("cmd->umac_cmd is NULL , reason: %d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = cmd->umac_cmd;
	if (!req->vdev) {
		scm_err("NULL vdev. req:0x%pK, reason:%d\n", req, reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_mtrace(QDF_MODULE_ID_SERIALIZATION, QDF_MODULE_ID_SCAN, reason,
		   req->scan_req.vdev_id, req->scan_req.scan_id);

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		/* command moved to active list
		 * modify the params if required for concurency case.
		 */
		status = scm_activate_scan_request(req);
		break;

	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list.
		 * notify registered scan event handlers with
		 * status completed and reason cancelled.
		 */
		status = scm_post_internal_scan_complete_event(req,
				SCAN_REASON_CANCELLED);
		break;

	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		/* active command timed out.
		 * prepare internal scan cancel request
		 */
		status = scm_cancel_scan_request(req);
		break;

	case WLAN_SER_CB_RELEASE_MEM_CMD:
		/* command successfully completed.
		 * Release vdev reference and free scan_start_request memory
		 */
		cmd->umac_cmd = NULL;
		wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);
		status = scm_scan_free_scan_request_mem(req);
		break;

	default:
		/* Do nothing but logging */
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

bool scm_is_scan_allowed(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_scan_obj *scan_psoc_obj;
	struct scan_vdev_obj *scan_vdev_obj;

	if (!vdev) {
		scm_err("vdev is NULL");
		return false;
	}

	scan_psoc_obj = wlan_vdev_get_scan_obj(vdev);
	if (!scan_psoc_obj) {
		scm_err("Couldn't find scan psoc object");
		return false;
	}

	if (scan_psoc_obj->scan_disabled) {
		scm_err_rl("scan disabled %x, for psoc",
			   scan_psoc_obj->scan_disabled);
		return false;
	}

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	if (!scan_vdev_obj) {
		scm_err("Couldn't find scan vdev object");
		return false;
	}

	if (scan_vdev_obj->scan_disabled) {
		scm_err_rl("scan disabled %x on vdev_id:%d",
			   scan_vdev_obj->scan_disabled,
			   wlan_vdev_get_id(vdev));
		return false;
	}

	return true;
}

#ifdef WLAN_POLICY_MGR_ENABLE
/**
 * scm_update_dbs_scan_ctrl_ext_flag() - update dbs scan ctrl flags
 * @req: pointer to scan request
 *
 * This function sets scan_ctrl_flags_ext value depending on the type of
 * scan and the channel lists.
 *
 * Non-DBS scan is requested if any of the below case is met:
 *     1. HW is DBS incapable
 *     2. A high accuracy scan request is sent by kernel.
 *
 * DBS scan is enabled for these conditions:
 *     1. A low power or low span scan request is sent by kernel.
 * For remaining cases DBS is enabled by default.
 * Return: void
 */
static void
scm_update_dbs_scan_ctrl_ext_flag(struct scan_start_request *req)
{
	struct wlan_objmgr_psoc *psoc;
	uint32_t scan_dbs_policy = SCAN_DBS_POLICY_DEFAULT;
	bool ndi_present;

	psoc = wlan_vdev_get_psoc(req->vdev);

	if (!policy_mgr_is_dbs_scan_allowed(psoc)) {
		scan_dbs_policy = SCAN_DBS_POLICY_FORCE_NONDBS;
		goto end;
	}

	if (!wlan_scan_cfg_honour_nl_scan_policy_flags(psoc)) {
		scm_debug_rl("nl scan policy flags not honoured, goto end");
		goto end;
	}

	ndi_present = policy_mgr_mode_specific_connection_count(psoc,
								PM_NDI_MODE,
								NULL);

	if (ndi_present || policy_mgr_is_hw_dbs_2x2_capable(psoc)) {
		scm_debug("NDP present or HW 2x2 capable, go for DBS scan");
		goto end;
	}

	if (req->scan_req.scan_policy_high_accuracy) {
		scm_debug("high accuracy scan received, going for non-dbs scan");
		scan_dbs_policy = SCAN_DBS_POLICY_FORCE_NONDBS;
		goto end;
	}

	if ((req->scan_req.scan_policy_low_power) ||
	    (req->scan_req.scan_policy_low_span)) {
		scm_debug("low power/span scan received, going for dbs scan");
		scan_dbs_policy = SCAN_DBS_POLICY_IGNORE_DUTY;
		goto end;
	}

end:
	req->scan_req.scan_ctrl_flags_ext |=
		((scan_dbs_policy << SCAN_FLAG_EXT_DBS_SCAN_POLICY_BIT)
		 & SCAN_FLAG_EXT_DBS_SCAN_POLICY_MASK);
}

/**
 * scm_update_passive_dwell_time() - update dwell passive time
 * @vdev: vdev object
 * @req: scan request
 *
 * Return: None
 */
static void
scm_update_passive_dwell_time(struct wlan_objmgr_vdev *vdev,
			      struct scan_start_request *req)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return;

	if (policy_mgr_is_sta_connected_2g(psoc) &&
	    !policy_mgr_is_hw_dbs_capable(psoc) &&
	    ucfg_scan_get_bt_activity(psoc))
		req->scan_req.dwell_time_passive =
				PASSIVE_DWELL_TIME_BT_A2DP_ENABLED;
}

static const struct probe_time_dwell_time
	scan_probe_time_dwell_time_map[SCAN_DWELL_TIME_PROBE_TIME_MAP_SIZE] = {
	{28, 11},               /* 0 SSID */
	{28, 20},               /* 1 SSID */
	{28, 20},               /* 2 SSID */
	{28, 20},               /* 3 SSID */
	{28, 20},               /* 4 SSID */
	{28, 20},               /* 5 SSID */
	{28, 20},               /* 6 SSID */
	{28, 11},               /* 7 SSID */
	{28, 11},               /* 8 SSID */
	{28, 11},               /* 9 SSID */
	{28, 8}                 /* 10 SSID */
};

/**
 * scm_scan_get_burst_duration() - get burst duration depending on max chan
 * and miracast.
 * @max_ch_time: max channel time
 * @miracast_enabled: if miracast is enabled
 *
 * Return: burst_duration
 */
static inline
int scm_scan_get_burst_duration(int max_ch_time, bool miracast_enabled)
{
	int burst_duration = 0;

	if (miracast_enabled) {
		/*
		 * When miracast is running, burst
		 * duration needs to be minimum to avoid
		 * any stutter or glitch in miracast
		 * during station scan
		 */
		if (max_ch_time <= SCAN_GO_MIN_ACTIVE_SCAN_BURST_DURATION)
			burst_duration = max_ch_time;
		else
			burst_duration = SCAN_GO_MIN_ACTIVE_SCAN_BURST_DURATION;
	} else {
		/*
		 * If miracast is not running, accommodate max
		 * stations to make the scans faster
		 */
		burst_duration = SCAN_GO_BURST_SCAN_MAX_NUM_OFFCHANNELS *
							max_ch_time;

		if (burst_duration > SCAN_GO_MAX_ACTIVE_SCAN_BURST_DURATION) {
			uint8_t channels = SCAN_P2P_SCAN_MAX_BURST_DURATION /
								 max_ch_time;

			if (channels)
				burst_duration = channels * max_ch_time;
			else
				burst_duration =
					 SCAN_GO_MAX_ACTIVE_SCAN_BURST_DURATION;
		}
	}
	return burst_duration;
}

#define SCM_ACTIVE_DWELL_TIME_NAN      60
#define SCM_ACTIVE_DWELL_TIME_SAP      40

/**
 * scm_req_update_concurrency_params() - update scan req params depending on
 * concurrent mode present.
 * @vdev: vdev object pointer
 * @req: scan request
 * @scan_obj: scan object
 *
 * Return: void
 */
static void scm_req_update_concurrency_params(struct wlan_objmgr_vdev *vdev,
					      struct scan_start_request *req,
					      struct wlan_scan_obj *scan_obj)
{
	bool ap_present, go_present, sta_active, p2p_cli_present, ndi_present;
	struct wlan_objmgr_psoc *psoc;
	uint16_t sap_peer_count = 0;
	uint16_t go_peer_count = 0;
	struct wlan_objmgr_pdev *pdev;

	psoc = wlan_vdev_get_psoc(vdev);
	pdev = wlan_vdev_get_pdev(vdev);

	if (!psoc || !pdev)
		return;

	ap_present = policy_mgr_mode_specific_connection_count(
				psoc, PM_SAP_MODE, NULL);
	go_present = policy_mgr_mode_specific_connection_count(
				psoc, PM_P2P_GO_MODE, NULL);
	p2p_cli_present = policy_mgr_mode_specific_connection_count(
				psoc, PM_P2P_CLIENT_MODE, NULL);
	sta_active = policy_mgr_mode_specific_connection_count(
				psoc, PM_STA_MODE, NULL);
	ndi_present = policy_mgr_mode_specific_connection_count(
				psoc, PM_NDI_MODE, NULL);
	if (ap_present)
		sap_peer_count =
		wlan_util_get_peer_count_for_mode(pdev, QDF_SAP_MODE);
	if (go_present)
		go_peer_count =
		wlan_util_get_peer_count_for_mode(pdev, QDF_P2P_GO_MODE);

	if (!req->scan_req.scan_f_passive)
		scm_update_passive_dwell_time(vdev, req);

	if (policy_mgr_get_connection_count(psoc)) {
		if (!req->scan_req.scan_f_passive)
			req->scan_req.dwell_time_active =
				scan_obj->scan_def.conc_active_dwell;
		req->scan_req.dwell_time_passive =
			scan_obj->scan_def.conc_passive_dwell;
		req->scan_req.max_rest_time =
				scan_obj->scan_def.conc_max_rest_time;
		req->scan_req.min_rest_time =
			scan_obj->scan_def.conc_min_rest_time;
		req->scan_req.idle_time = scan_obj->scan_def.conc_idle_time;
	}

	if (wlan_vdev_is_up(req->vdev) != QDF_STATUS_SUCCESS)
		req->scan_req.adaptive_dwell_time_mode =
			scan_obj->scan_def.adaptive_dwell_time_mode_nc;
	/*
	 * If AP/GO is active and has connected clients :
	 * 1.set min rest time same as max rest time, so that
	 * firmware spends more time on home channel which will
	 * increase the probability of sending beacon at TBTT
	 * 2.if DBS is supported and SAP is not on 2g,
	 * do not reset active dwell time for 2g.
	 */

	/*
	 * For SAP, the dwell time cannot exceed 32 ms as it can't go
	 * offchannel more than 32 ms. For Go, since we
	 * advertise NOA, GO can have regular dwell time which is 40 ms.
	 */
	if ((ap_present && sap_peer_count) ||
	    (go_present && go_peer_count)) {
		if ((policy_mgr_is_hw_dbs_capable(psoc) &&
		     policy_mgr_is_sap_go_on_2g(psoc)) ||
		     !policy_mgr_is_hw_dbs_capable(psoc)) {
			if (ap_present)
				req->scan_req.dwell_time_active_2g =
					QDF_MIN(req->scan_req.dwell_time_active,
						(SCAN_CTS_DURATION_MS_MAX -
						SCAN_ROAM_SCAN_CHANNEL_SWITCH_TIME));
			else
				req->scan_req.dwell_time_active_2g = 0;
		}
		req->scan_req.min_rest_time = req->scan_req.max_rest_time;
	}

	if (policy_mgr_current_concurrency_is_mcc(psoc))
		req->scan_req.min_rest_time =
			scan_obj->scan_def.conc_max_rest_time;

	/*
	 * If scan req for SAP (ACS Sacn) use dwell_time_active_def as dwell
	 * time for 2g channels instead of dwell_time_active_2g
	 */
	if (vdev->vdev_mlme.vdev_opmode == QDF_SAP_MODE)
		req->scan_req.dwell_time_active_2g = SCM_ACTIVE_DWELL_TIME_SAP;

	if (req->scan_req.scan_type == SCAN_TYPE_DEFAULT) {
		/*
		 * Decide burst_duration and dwell_time_active based on
		 * what type of devices are active.
		 */
		do {
			if (ap_present && go_present && sta_active) {
				if (req->scan_req.dwell_time_active <=
					SCAN_3PORT_CONC_SCAN_MAX_BURST_DURATION)
					req->scan_req.burst_duration =
						req->scan_req.dwell_time_active;
				else
					req->scan_req.burst_duration =
					SCAN_3PORT_CONC_SCAN_MAX_BURST_DURATION;

				break;
			}

			if (scan_obj->miracast_enabled &&
			    policy_mgr_is_mcc_in_24G(psoc))
				req->scan_req.max_rest_time =
				  scan_obj->scan_def.sta_miracast_mcc_rest_time;

			if (go_present) {
				/*
				 * Background scan while GO is sending beacons.
				 * Every off-channel transition has overhead of
				 * 2 beacon intervals for NOA. Maximize number
				 * of channels in every transition by using
				 * burst scan.
				 */
				if (scan_obj->scan_def.go_scan_burst_duration)
					req->scan_req.burst_duration =
						scan_obj->
						scan_def.go_scan_burst_duration;
				else
					req->scan_req.burst_duration =
						scm_scan_get_burst_duration(
							req->scan_req.
							dwell_time_active,
							scan_obj->
							miracast_enabled);
				break;
			}
			if ((sta_active || p2p_cli_present)) {
				if (scan_obj->scan_def.sta_scan_burst_duration)
					req->scan_req.burst_duration =
						scan_obj->scan_def.
						sta_scan_burst_duration;
				break;
			}

			if (go_present && sta_active) {
				req->scan_req.burst_duration =
					req->scan_req.dwell_time_active;
				break;
			}

			if (ndi_present || (p2p_cli_present && sta_active)) {
				req->scan_req.burst_duration = 0;
				break;
			}
		} while (0);

		if (ap_present) {
			uint8_t ssid_num;

			ssid_num = req->scan_req.num_ssids *
					req->scan_req.num_bssid;
			req->scan_req.repeat_probe_time =
				scan_probe_time_dwell_time_map[
					QDF_MIN(ssid_num,
					SCAN_DWELL_TIME_PROBE_TIME_MAP_SIZE
					- 1)].probe_time;
			req->scan_req.n_probes =
				(req->scan_req.repeat_probe_time > 0) ?
				req->scan_req.dwell_time_active /
				req->scan_req.repeat_probe_time : 0;
		}
	}

	if (ap_present) {
		uint16_t ap_chan_freq;
		struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);

		ap_chan_freq = policy_mgr_get_channel(psoc, PM_SAP_MODE, NULL);
		/*
		 * P2P/STA scan while SoftAP is sending beacons.
		 * Max duration of CTS2self is 32 ms, which limits the
		 * dwell time.
		 * If DBS is supported and:
		 * 1.if SAP is on 2G channel then keep passive
		 * dwell time default.
		 * 2.if SAP is on 5G/6G channel then update dwell time active.
		 */
		if (sap_peer_count) {
			if (policy_mgr_is_hw_dbs_capable(psoc) &&
			    (WLAN_REG_IS_5GHZ_CH_FREQ(ap_chan_freq) ||
			    WLAN_REG_IS_6GHZ_CHAN_FREQ(ap_chan_freq))) {
				req->scan_req.dwell_time_active =
					QDF_MIN(req->scan_req.dwell_time_active,
						(SCAN_CTS_DURATION_MS_MAX -
					SCAN_ROAM_SCAN_CHANNEL_SWITCH_TIME));
			}
			if (!policy_mgr_is_hw_dbs_capable(psoc) ||
			    (policy_mgr_is_hw_dbs_capable(psoc) &&
			     WLAN_REG_IS_5GHZ_CH_FREQ(ap_chan_freq))) {
				req->scan_req.dwell_time_passive =
					req->scan_req.dwell_time_active;
			}
		}

		if (scan_obj->scan_def.ap_scan_burst_duration) {
			req->scan_req.burst_duration =
				scan_obj->scan_def.ap_scan_burst_duration;
		} else {
			req->scan_req.burst_duration = 0;
			if (wlan_reg_is_dfs_for_freq(pdev, ap_chan_freq))
				req->scan_req.burst_duration =
					SCAN_BURST_SCAN_MAX_NUM_OFFCHANNELS *
					req->scan_req.dwell_time_active;
		}
	}

	if (ndi_present) {
		req->scan_req.dwell_time_active =
						SCM_ACTIVE_DWELL_TIME_NAN;
		req->scan_req.dwell_time_active_2g =
			QDF_MIN(req->scan_req.dwell_time_active_2g,
			SCM_ACTIVE_DWELL_TIME_NAN);
		scm_debug("NDP active modify dwell time 2ghz %d",
			req->scan_req.dwell_time_active_2g);
	}

	if (sta_active) {
		req->scan_req.dwell_time_active_6g =
				scan_obj->scan_def.active_dwell_time_6g_conc;
		req->scan_req.dwell_time_passive_6g =
				scan_obj->scan_def.passive_dwell_time_6g_conc;
	}
}

/**
 * scm_scan_chlist_concurrency_modify() - modify chan list to skip 5G if
 *    required
 * @vdev: vdev object
 * @req: scan request
 *
 * Check and skip 5G chan list based on DFS AP present and current hw mode.
 *
 * Return: void
 */
static inline void scm_scan_chlist_concurrency_modify(
	struct wlan_objmgr_vdev *vdev, struct scan_start_request *req)
{
	struct wlan_objmgr_psoc *psoc;
	uint32_t i;
	uint32_t num_scan_channels;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return;
	/* do this only for STA and P2P-CLI mode */
	if (!(wlan_vdev_mlme_get_opmode(req->vdev) == QDF_STA_MODE) &&
	    !(wlan_vdev_mlme_get_opmode(req->vdev) == QDF_P2P_CLIENT_MODE))
		return;
	if (!policy_mgr_scan_trim_5g_chnls_for_dfs_ap(psoc))
		return;
	num_scan_channels = 0;
	for (i = 0; i < req->scan_req.chan_list.num_chan; i++) {
		if (WLAN_REG_IS_5GHZ_CH_FREQ(
			req->scan_req.chan_list.chan[i].freq)) {
			continue;
		}
		req->scan_req.chan_list.chan[num_scan_channels++] =
			req->scan_req.chan_list.chan[i];
	}
	if (num_scan_channels < req->scan_req.chan_list.num_chan)
		scm_debug("5g chan skipped (%d, %d)",
			  req->scan_req.chan_list.num_chan, num_scan_channels);
	req->scan_req.chan_list.num_chan = num_scan_channels;
}
#else
static inline
void scm_req_update_concurrency_params(struct wlan_objmgr_vdev *vdev,
				       struct scan_start_request *req,
				       struct wlan_scan_obj *scan_obj)
{
}

static inline void
scm_update_dbs_scan_ctrl_ext_flag(struct scan_start_request *req)
{
}

static inline void scm_scan_chlist_concurrency_modify(
	struct wlan_objmgr_vdev *vdev, struct scan_start_request *req)
{
}
#endif

/**
 * scm_update_channel_list() - update scan req params depending on dfs inis
 * and initial scan request.
 * @req: scan request
 * @scan_obj: scan object
 *
 * Return: void
 */
static void
scm_update_channel_list(struct scan_start_request *req,
			struct wlan_scan_obj *scan_obj)
{
	uint8_t i;
	uint8_t num_scan_channels = 0;
	struct scan_vdev_obj *scan_vdev_obj;
	struct wlan_objmgr_pdev *pdev;
	bool first_scan_done = true;
	bool p2p_search = false;
	bool skip_dfs_ch = true;
	uint32_t first_freq;

	pdev = wlan_vdev_get_pdev(req->vdev);

	scan_vdev_obj = wlan_get_vdev_scan_obj(req->vdev);
	if (!scan_vdev_obj) {
		scm_err("null scan_vdev_obj");
		return;
	}

	if (!scan_vdev_obj->first_scan_done) {
		first_scan_done = false;
		scan_vdev_obj->first_scan_done = true;
	}

	if (req->scan_req.scan_type == SCAN_TYPE_P2P_SEARCH)
		p2p_search = true;
	/*
	 * No need to update channels if req is single channel* ie ROC,
	 * Preauth or a single channel scan etc.
	 * If the single chan in the scan channel list is an NOL channel,it is
	 * removed and it would reduce the number of scan channels to 0.
	 */
	first_freq = req->scan_req.chan_list.chan[0].freq;
	if ((req->scan_req.chan_list.num_chan == 1) &&
	    (!utils_dfs_is_freq_in_nol(pdev, first_freq)))
		return;

	/* do this only for STA and P2P-CLI mode */
	if ((!(wlan_vdev_mlme_get_opmode(req->vdev) == QDF_STA_MODE) &&
	    !(wlan_vdev_mlme_get_opmode(req->vdev) == QDF_P2P_CLIENT_MODE)) &&
	    !p2p_search)
		skip_dfs_ch = false;

	if ((scan_obj->scan_def.allow_dfs_chan_in_scan &&
	    (scan_obj->scan_def.allow_dfs_chan_in_first_scan ||
	     first_scan_done)) &&
	     !(scan_obj->scan_def.skip_dfs_chan_in_p2p_search && p2p_search) &&
	     !scan_obj->miracast_enabled)
		skip_dfs_ch = false;

	for (i = 0; i < req->scan_req.chan_list.num_chan; i++) {
		uint32_t freq;

		freq = req->scan_req.chan_list.chan[i].freq;
		if (skip_dfs_ch &&
		    wlan_reg_chan_has_dfs_attribute_for_freq(pdev, freq)) {
			scm_nofl_debug("Skip DFS freq %d", freq);
			continue;
		}
		if (utils_dfs_is_freq_in_nol(pdev, freq)) {
			scm_nofl_debug("Skip NOL freq %d", freq);
			continue;
		}

		req->scan_req.chan_list.chan[num_scan_channels++] =
			req->scan_req.chan_list.chan[i];
	}

	req->scan_req.chan_list.num_chan = num_scan_channels;

	scm_update_6ghz_channel_list(req, scan_obj);
	scm_scan_chlist_concurrency_modify(req->vdev, req);
}

/**
 * scm_scan_req_update_params() - update scan req params depending on modes
 * and scan type.
 * @vdev: vdev object pointer
 * @req: scan request
 * @scan_obj: scan object
 *
 * Return: void
 */
static void
scm_scan_req_update_params(struct wlan_objmgr_vdev *vdev,
			   struct scan_start_request *req,
			   struct wlan_scan_obj *scan_obj)
{
	struct chan_list *custom_chan_list;
	struct wlan_objmgr_pdev *pdev;
	uint8_t pdev_id;

	/* Ensure correct number of probes are sent on active channel */
	if (!req->scan_req.repeat_probe_time)
		req->scan_req.repeat_probe_time =
			req->scan_req.dwell_time_active / SCAN_NPROBES_DEFAULT;

	if (req->scan_req.scan_f_passive)
		req->scan_req.scan_ctrl_flags_ext |=
			SCAN_FLAG_EXT_FILTER_PUBLIC_ACTION_FRAME;

	if (!req->scan_req.n_probes)
		req->scan_req.n_probes = (req->scan_req.repeat_probe_time > 0) ?
					  req->scan_req.dwell_time_active /
					  req->scan_req.repeat_probe_time : 0;

	if (req->scan_req.scan_type == SCAN_TYPE_P2P_SEARCH ||
	    req->scan_req.scan_type == SCAN_TYPE_P2P_LISTEN) {
		req->scan_req.adaptive_dwell_time_mode = SCAN_DWELL_MODE_STATIC;
		req->scan_req.dwell_time_active_2g = 0;
		if (req->scan_req.scan_type == SCAN_TYPE_P2P_LISTEN) {
			req->scan_req.repeat_probe_time = 0;
		} else {
			req->scan_req.scan_f_filter_prb_req = true;
			if (!req->scan_req.num_ssids)
				req->scan_req.scan_f_bcast_probe = true;

			req->scan_req.dwell_time_active +=
					P2P_SEARCH_DWELL_TIME_INC;
			/*
			 * 3 channels with default max dwell time 40 ms.
			 * Cap limit will be set by
			 * P2P_SCAN_MAX_BURST_DURATION. Burst duration
			 * should be such that no channel is scanned less
			 * than the dwell time in normal scenarios.
			 */
			if (req->scan_req.chan_list.num_chan ==
			    WLAN_P2P_SOCIAL_CHANNELS &&
			    !scan_obj->miracast_enabled)
				req->scan_req.repeat_probe_time =
					req->scan_req.dwell_time_active / 5;
			else
				req->scan_req.repeat_probe_time =
					req->scan_req.dwell_time_active / 3;
			if (scan_obj->scan_def.p2p_scan_burst_duration) {
				req->scan_req.burst_duration =
					scan_obj->scan_def.
					p2p_scan_burst_duration;
			} else {
				req->scan_req.burst_duration =
						BURST_SCAN_MAX_NUM_OFFCHANNELS *
						req->scan_req.dwell_time_active;
				if (req->scan_req.burst_duration >
				    P2P_SCAN_MAX_BURST_DURATION) {
					uint8_t channels =
						P2P_SCAN_MAX_BURST_DURATION /
						req->scan_req.dwell_time_active;
					if (channels)
						req->scan_req.burst_duration =
						channels *
						req->scan_req.dwell_time_active;
					else
						req->scan_req.burst_duration =
						P2P_SCAN_MAX_BURST_DURATION;
				}
			}
			req->scan_req.scan_ev_bss_chan = false;
		}
	} else {
		req->scan_req.scan_f_cck_rates = true;
		if (!req->scan_req.num_ssids)
			req->scan_req.scan_f_bcast_probe = true;
		req->scan_req.scan_f_add_ds_ie_in_probe = true;
		req->scan_req.scan_f_filter_prb_req = true;
		req->scan_req.scan_f_add_tpc_ie_in_probe = true;
	}

	scm_update_dbs_scan_ctrl_ext_flag(req);

	/*
	 * No need to update conncurrency parmas if req is passive scan on
	 * single channel ie ROC, Preauth etc
	 */
	if (!(req->scan_req.scan_f_passive &&
	      req->scan_req.chan_list.num_chan == 1) &&
	      req->scan_req.scan_type != SCAN_TYPE_RRM)
		scm_req_update_concurrency_params(vdev, req, scan_obj);

	if (req->scan_req.scan_type == SCAN_TYPE_RRM)
		req->scan_req.scan_ctrl_flags_ext |= SCAN_FLAG_EXT_RRM_SCAN_IND;

	scm_debug("scan_ctrl_flags_ext %0x", req->scan_req.scan_ctrl_flags_ext);
	/*
	 * Set wide band flag if enabled. This will cause
	 * phymode TLV being sent to FW.
	 */
	pdev = wlan_vdev_get_pdev(vdev);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (ucfg_scan_get_wide_band_scan(pdev))
		req->scan_req.scan_f_wide_band = true;
	else
		req->scan_req.scan_f_wide_band = false;

	/*
	 * Overwrite scan channles with custom scan channel
	 * list if configured.
	 */
	custom_chan_list = &scan_obj->pdev_info[pdev_id].custom_chan_list;
	if (custom_chan_list->num_chan)
		qdf_mem_copy(&req->scan_req.chan_list, custom_chan_list,
			     sizeof(struct chan_list));
	else if (!req->scan_req.chan_list.num_chan)
		ucfg_scan_init_chanlist_params(req, 0, NULL, NULL);

	scm_update_channel_list(req, scan_obj);
}

static inline void scm_print_scan_req_info(struct scan_req_params *req)
{
	uint32_t buff_len;
	char *chan_buff;
	uint32_t len = 0;
	uint8_t idx, count = 0;
	struct chan_list *chan_lst;
#define MAX_SCAN_FREQ_TO_PRINT 25

	scm_nofl_debug("Scan start: scan id %d vdev %d Dwell time: act %d pass %d act_2G %d act_6G %d pass_6G %d, probe time %d n_probes %d flags %x ext_flag %x events %x policy %d wide_bw %d pri %d",
		       req->scan_id, req->vdev_id, req->dwell_time_active,
		       req->dwell_time_passive, req->dwell_time_active_2g,
		       req->dwell_time_active_6g, req->dwell_time_passive_6g,
		       req->repeat_probe_time, req->n_probes, req->scan_flags,
		       req->scan_ctrl_flags_ext, req->scan_events,
		       req->scan_policy_type, req->scan_f_wide_band,
		       req->scan_priority);

	for (idx = 0; idx < req->num_ssids; idx++)
		scm_nofl_debug("SSID[%d]: %.*s", idx, req->ssid[idx].length,
			       req->ssid[idx].ssid);

	chan_lst  = &req->chan_list;

	if (!chan_lst->num_chan)
		return;
	/*
	 * Buffer of (num channl * 11) + 1  to consider the 4 char freq, 6 char
	 * flags and 1 space after it for each channel and 1 to end the string
	 * with NULL.
	 */
	buff_len =
		(QDF_MIN(MAX_SCAN_FREQ_TO_PRINT, chan_lst->num_chan) * 11) + 1;
	chan_buff = qdf_mem_malloc(buff_len);
	if (!chan_buff)
		return;
	scm_nofl_debug("Total freq %d", chan_lst->num_chan);
	for (idx = 0; idx < chan_lst->num_chan; idx++) {
		len += qdf_scnprintf(chan_buff + len, buff_len - len,
				     "%d(0x%02x) ", chan_lst->chan[idx].freq,
				     chan_lst->chan[idx].flags);
		count++;
		if (count >= MAX_SCAN_FREQ_TO_PRINT) {
			/* Print the MAX_SCAN_FREQ_TO_PRINT channels */
			scm_nofl_debug("Freq list: %s", chan_buff);
			len = 0;
			count = 0;
		}
	}
	if (len)
		scm_nofl_debug("Freq list: %s", chan_buff);

	qdf_mem_free(chan_buff);
}
QDF_STATUS
scm_scan_start_req(struct scheduler_msg *msg)
{
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;
	struct scan_start_request *req = NULL;
	struct wlan_scan_obj *scan_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;


	if (!msg) {
		scm_err("msg received is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!msg->bodyptr) {
		scm_err("bodyptr is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = msg->bodyptr;

	if (!scm_is_scan_allowed(req->vdev)) {
		scm_err("scan disabled, rejecting the scan req");
		status = QDF_STATUS_E_NULL_VALUE;
		goto err;
	}

	scan_obj = wlan_vdev_get_scan_obj(req->vdev);
	if (!scan_obj) {
		scm_debug("Couldn't find scan object");
		status = QDF_STATUS_E_NULL_VALUE;
		goto err;
	}

	scm_scan_req_update_params(req->vdev, req, scan_obj);
	scm_print_scan_req_info(&req->scan_req);

	if (!req->scan_req.chan_list.num_chan) {
		scm_info("Reject 0 channel Scan");
		status = QDF_STATUS_E_NULL_VALUE;
		goto err;
	}

	cmd.cmd_type = WLAN_SER_CMD_SCAN;
	cmd.cmd_id = req->scan_req.scan_id;
	cmd.cmd_cb = scm_scan_serialize_callback;
	cmd.umac_cmd = req;
	cmd.source = WLAN_UMAC_COMP_SCAN;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = req->scan_req.max_scan_time +
		SCAN_TIMEOUT_GRACE_PERIOD;
	cmd.vdev = req->vdev;

	if (scan_obj->disable_timeout)
		cmd.cmd_timeout_duration = 0;

	qdf_mtrace(QDF_MODULE_ID_SCAN, QDF_MODULE_ID_SERIALIZATION,
		   WLAN_SER_CMD_SCAN, req->vdev->vdev_objmgr.vdev_id,
		   req->scan_req.scan_id);

	ser_cmd_status = wlan_serialization_request(&cmd);
	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list.Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	default:
		scm_debug("ser cmd status %d", ser_cmd_status);
		goto err;
	}

	return status;
err:
	/*
	 * notify registered scan event handlers
	 * about internal error
	 */
	scm_post_internal_scan_complete_event(req,
					      SCAN_REASON_INTERNAL_FAILURE);
	/*
	 * cmd can't be serviced.
	 * release vdev reference and free scan_start_request memory
	 */
	if (req) {
		wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);
		scm_scan_free_scan_request_mem(req);
	}

	return status;
}

static inline enum wlan_serialization_cancel_type
get_serialization_cancel_type(enum scan_cancel_req_type type)
{
	enum wlan_serialization_cancel_type serialization_type;

	switch (type) {
	case WLAN_SCAN_CANCEL_SINGLE:
		serialization_type = WLAN_SER_CANCEL_SINGLE_SCAN;
		break;
	case WLAN_SCAN_CANCEL_VDEV_ALL:
		serialization_type = WLAN_SER_CANCEL_VDEV_SCANS;
		break;
	case WLAN_SCAN_CANCEL_PDEV_ALL:
		serialization_type = WLAN_SER_CANCEL_PDEV_SCANS;
		break;
	case WLAN_SCAN_CANCEL_HOST_VDEV_ALL:
		serialization_type = WLAN_SER_CANCEL_VDEV_HOST_SCANS;
		break;
	default:
		QDF_ASSERT(0);
		scm_warn("invalid scan_cancel_req_type: %d", type);
		serialization_type = WLAN_SER_CANCEL_PDEV_SCANS;
		break;
	}

	return serialization_type;
}

QDF_STATUS
scm_scan_cancel_req(struct scheduler_msg *msg)
{
	struct wlan_serialization_queued_cmd_info cmd = {0,};
	struct wlan_serialization_command ser_cmd = {0,};
	enum wlan_serialization_cmd_status ser_cmd_status;
	struct scan_cancel_request *req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!msg) {
		scm_err("msg received is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!msg->bodyptr) {
		scm_err("Bodyptr is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = msg->bodyptr;
	/*
	 * If requester wants to wait for target scan cancel event
	 * instead of internally generated cancel event, just check
	 * which queue this scan request belongs to and send scan
	 * cancel request to FW accordingly.
	 * Else generate internal scan cancel event and notify
	 * handlers and free scan request resources.
	 */
	if (req->wait_tgt_cancel &&
			(req->cancel_req.req_type == WLAN_SCAN_CANCEL_SINGLE)) {
		ser_cmd.cmd_type = WLAN_SER_CMD_SCAN;
		ser_cmd.cmd_id = req->cancel_req.scan_id;
		ser_cmd.cmd_cb = NULL;
		ser_cmd.umac_cmd = NULL;
		ser_cmd.source = WLAN_UMAC_COMP_SCAN;
		ser_cmd.is_high_priority = false;
		ser_cmd.vdev = req->vdev;
		if (wlan_serialization_is_cmd_present_in_active_queue(NULL, &ser_cmd))
			ser_cmd_status = WLAN_SER_CMD_IN_ACTIVE_LIST;
		else if (wlan_serialization_is_cmd_present_in_pending_queue(NULL, &ser_cmd))
			ser_cmd_status = WLAN_SER_CMD_IN_PENDING_LIST;
		else
			ser_cmd_status = WLAN_SER_CMD_NOT_FOUND;
	} else {
		cmd.requestor = 0;
		cmd.cmd_type = WLAN_SER_CMD_SCAN;
		cmd.cmd_id = req->cancel_req.scan_id;
		cmd.vdev = req->vdev;
		cmd.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE |
			WLAN_SERIALIZATION_PENDING_QUEUE;
		cmd.req_type = get_serialization_cancel_type(req->cancel_req.req_type);

		ser_cmd_status = wlan_serialization_cancel_request(&cmd);
	}

	scm_debug("status: %d, reqid: %d, scanid: %d, vdevid: %d, type: %d",
		ser_cmd_status, req->cancel_req.requester,
		req->cancel_req.scan_id, req->cancel_req.vdev_id,
		req->cancel_req.req_type);

	switch (ser_cmd_status) {
	case WLAN_SER_CMD_IN_PENDING_LIST:
		/* do nothing */
		break;
	case WLAN_SER_CMD_IN_ACTIVE_LIST:
	case WLAN_SER_CMDS_IN_ALL_LISTS:
		/* send wmi scan cancel to fw */
		status = tgt_scan_cancel(req);
		break;
	case WLAN_SER_CMD_NOT_FOUND:
		/* do nothing */
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	/* Release vdev reference and scan cancel request
	 * processing is complete
	 */
	wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);
	/* Free cancel request memory */
	qdf_mem_free(req);

	return status;
}

#ifdef FEATURE_WLAN_SCAN_PNO
static QDF_STATUS
scm_pno_event_handler(struct wlan_objmgr_vdev *vdev,
	struct scan_event *event)
{
	struct scan_vdev_obj *scan_vdev_obj;
	struct wlan_scan_obj *scan_psoc_obj;
	scan_event_handler pno_cb;
	void *cb_arg;

	scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
	scan_psoc_obj = wlan_vdev_get_scan_obj(vdev);
	if (!scan_vdev_obj || !scan_psoc_obj) {
		scm_err("null scan_vdev_obj %pK scan_obj %pK",
			scan_vdev_obj, scan_psoc_obj);
		return QDF_STATUS_E_INVAL;
	}

	switch (event->type) {
	case SCAN_EVENT_TYPE_NLO_COMPLETE:
		if (!scan_vdev_obj->pno_match_evt_received)
			return QDF_STATUS_SUCCESS;
		qdf_wake_lock_release(&scan_psoc_obj->pno_cfg.pno_wake_lock,
			WIFI_POWER_EVENT_WAKELOCK_PNO);
		qdf_wake_lock_timeout_acquire(
			&scan_psoc_obj->pno_cfg.pno_wake_lock,
			SCAN_PNO_SCAN_COMPLETE_WAKE_LOCK_TIMEOUT);
		qdf_runtime_pm_allow_suspend(
			&scan_psoc_obj->pno_cfg.pno_runtime_pm_lock);
		scan_vdev_obj->pno_match_evt_received = false;
		break;
	case SCAN_EVENT_TYPE_NLO_MATCH:
		scan_vdev_obj->pno_match_evt_received = true;
		qdf_wake_lock_timeout_acquire(
			&scan_psoc_obj->pno_cfg.pno_wake_lock,
			SCAN_PNO_MATCH_WAKE_LOCK_TIMEOUT);
		qdf_runtime_pm_prevent_suspend(
			&scan_psoc_obj->pno_cfg.pno_runtime_pm_lock);
		return QDF_STATUS_SUCCESS;
	default:
		return QDF_STATUS_E_INVAL;
	}
	qdf_spin_lock_bh(&scan_psoc_obj->lock);
	pno_cb = scan_psoc_obj->pno_cfg.pno_cb.func;
	cb_arg = scan_psoc_obj->pno_cfg.pno_cb.arg;
	qdf_spin_unlock_bh(&scan_psoc_obj->lock);

	if (pno_cb)
		pno_cb(vdev, event, cb_arg);

	return QDF_STATUS_SUCCESS;
}
#else

static QDF_STATUS
scm_pno_event_handler(struct wlan_objmgr_vdev *vdev,
	struct scan_event *event)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * scm_scan_update_scan_event() - update scan event
 * @scan: scan object
 * @event: scan event
 * @scan_start_req: scan_start_req used for triggering scan
 *
 * update scan params in scan event
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
scm_scan_update_scan_event(struct wlan_scan_obj *scan,
		struct scan_event *event,
		struct scan_start_request *scan_start_req)
{
	if (!event)
		return QDF_STATUS_E_NULL_VALUE;

	if (!scan || !scan_start_req) {
		event->scan_start_req = NULL;
		return QDF_STATUS_E_NULL_VALUE;
	}
	/* copy scan start request to pass back buffer */
	qdf_mem_copy(&scan->scan_start_request_buff, scan_start_req,
			sizeof(struct scan_start_request));
	/* reset all pointers */
	scan->scan_start_request_buff.scan_req.extraie.ptr = NULL;
	scan->scan_start_request_buff.scan_req.extraie.len = 0;
	scan->scan_start_request_buff.scan_req.htcap.ptr = NULL;
	scan->scan_start_request_buff.scan_req.htcap.len = 0;
	scan->scan_start_request_buff.scan_req.vhtcap.ptr = NULL;
	scan->scan_start_request_buff.scan_req.vhtcap.len = 0;

	event->scan_start_req = &scan->scan_start_request_buff;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
scm_scan_event_handler(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev;
	struct scan_event *event;
	struct scan_event_info *event_info;
	struct wlan_serialization_command cmd = {0,};
	struct wlan_serialization_command *queued_cmd;
	struct scan_start_request *scan_start_req;
	struct wlan_scan_obj *scan;

	if (!msg) {
		scm_err("NULL msg received ");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!msg->bodyptr) {
		scm_err("NULL scan event received");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	event_info = msg->bodyptr;
	vdev = event_info->vdev;
	event = &(event_info->event);

	scan = wlan_vdev_get_scan_obj(vdev);

	scm_duration_init(scan);

	scm_event_duration_start(scan);

	scm_debug("vdevid:%d, type:%d, reason:%d, freq:%d, reqstr:%d,"
		  "scanid:%d (0x%x), timestamp:%u", event->vdev_id,
		  event->type, event->reason, event->chan_freq,
		  event->requester, event->scan_id, event->scan_id,
		  event->timestamp);
	/*
	 * NLO requests are never queued, so post NLO events
	 * without checking for their presence in active queue.
	 */
	switch (event->type) {
	case SCAN_EVENT_TYPE_NLO_COMPLETE:
	case SCAN_EVENT_TYPE_NLO_MATCH:
		scm_pno_event_handler(vdev, event);
		goto exit;
	default:
		break;
	}

	cmd.cmd_type = WLAN_SER_CMD_SCAN;
	cmd.cmd_id = event->scan_id;
	cmd.cmd_cb = NULL;
	cmd.umac_cmd = NULL;
	cmd.source = WLAN_UMAC_COMP_SCAN;
	cmd.is_high_priority = false;
	cmd.vdev = vdev;
	if (!wlan_serialization_is_cmd_present_in_active_queue(NULL, &cmd)) {
		/*
		 * We received scan event for an already completed/cancelled
		 * scan request. Drop this event.
		 */
		scm_debug("Received scan event while request not in active queue");
		goto exit;
	}

	/* Fill scan_start_request used to trigger this scan */
	queued_cmd = wlan_serialization_get_scan_cmd_using_scan_id(
			wlan_vdev_get_psoc(vdev), wlan_vdev_get_id(vdev),
			event->scan_id, true);

	if (!queued_cmd) {
		scm_err("NULL queued_cmd");
		goto exit;
	}
	if (!queued_cmd->umac_cmd) {
		scm_err("NULL umac_cmd");
		goto exit;
	}
	scan_start_req = queued_cmd->umac_cmd;

	if (scan_start_req->scan_req.scan_req_id != event->requester) {
		scm_err("req ID mismatch, scan_req_id:%d, event_req_id:%d",
			scan_start_req->scan_req.scan_req_id, event->requester);
		goto exit;
	}

	if (scan)
		scm_scan_update_scan_event(scan, event, scan_start_req);

	switch (event->type) {
	case SCAN_EVENT_TYPE_COMPLETED:
		if (event->reason == SCAN_REASON_COMPLETED)
			scm_11d_decide_country_code(vdev);
		/* release the command */
		/* fallthrough */
	case SCAN_EVENT_TYPE_START_FAILED:
	case SCAN_EVENT_TYPE_DEQUEUED:
		scm_release_serialization_command(vdev, event->scan_id);
		break;
	default:
		break;
	}

	scm_to_post_scan_duration_set(scan);
	/* Notify all interested parties */
	scm_scan_post_event(vdev, event);

exit:
	/* free event info memory */
	qdf_mem_free(event_info);

	scm_event_duration_end(scan);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SCAN_ID);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_scan_event_flush_callback(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev;
	struct scan_event_info *event_info;
	struct scan_event *event;

	if (!msg || !msg->bodyptr) {
		scm_err("msg or msg->bodyptr is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	event_info = msg->bodyptr;
	vdev = event_info->vdev;
	event = &event_info->event;

	scm_debug("Flush scan event vdev %d type %d reason %d freq: %d req %d scanid %d",
		  event->vdev_id, event->type, event->reason, event->chan_freq,
		  event->requester, event->scan_id);

	/* free event info memory */
	qdf_mem_free(event_info);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SCAN_ID);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_bcn_probe_flush_callback(struct scheduler_msg *msg)
{
	struct scan_bcn_probe_event *bcn;

	bcn = msg->bodyptr;

	if (!bcn) {
		scm_err("bcn is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (bcn->psoc)
		wlan_objmgr_psoc_release_ref(bcn->psoc, WLAN_SCAN_ID);
	if (bcn->rx_data)
		qdf_mem_free(bcn->rx_data);
	if (bcn->buf)
		qdf_nbuf_free(bcn->buf);
	qdf_mem_free(bcn);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_scan_start_flush_callback(struct scheduler_msg *msg)
{
	struct scan_start_request *req;

	if (!msg || !msg->bodyptr) {
		scm_err("msg or msg->bodyptr is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = msg->bodyptr;
	scm_post_internal_scan_complete_event(req, SCAN_REASON_CANCELLED);
	wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);
	scm_scan_free_scan_request_mem(req);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_scan_cancel_flush_callback(struct scheduler_msg *msg)
{
	struct scan_cancel_request *req;

	if (!msg || !msg->bodyptr) {
		scm_err("msg or msg->bodyptr is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = msg->bodyptr;
	wlan_objmgr_vdev_release_ref(req->vdev, WLAN_SCAN_ID);
	/* Free cancel request memory */
	qdf_mem_free(req);

	return QDF_STATUS_SUCCESS;
}

void scm_disable_obss_pdev_scan(struct wlan_objmgr_psoc *psoc,
				struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_scan_obj *scan_obj;
	struct scan_vdev_obj *scan_vdev_obj;
	QDF_STATUS status;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan object null");
		return;
	}

	if (scan_obj->obss_scan_offload) {
		vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_SCAN_ID);
		if (!vdev)
			return;

		scan_vdev_obj = wlan_get_vdev_scan_obj(vdev);
		if (!scan_vdev_obj) {
			scm_err("null scan_vdev_obj");
			wlan_objmgr_vdev_release_ref(vdev, WLAN_SCAN_ID);
			return;
		}

		status = tgt_scan_obss_disable(vdev);
		if (QDF_IS_STATUS_ERROR(status))
			scm_err("disable obss scan failed");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SCAN_ID);
	}
}

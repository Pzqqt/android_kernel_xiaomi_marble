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

/**
 *  DOC:    wma_dev_if.c
 *  This file contains vdev & peer related operations.
 */

/* Header files */

#include "wma.h"
#include "wma_api.h"
#include "cds_api.h"
#include "wmi_unified_api.h"
#include "wlan_qct_sys.h"
#include "wni_api.h"
#include "ani_global.h"
#include "wmi_unified.h"
#include "wni_cfg.h"

#include "qdf_nbuf.h"
#include "qdf_types.h"
#include "qdf_mem.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
#include "csr_api.h"

#include "wma_internal.h"

#include "wma_ocb.h"
#include "cdp_txrx_cfg.h"
#include "cdp_txrx_flow_ctrl_legacy.h"
#include <cdp_txrx_peer_ops.h>
#include <cdp_txrx_cfg.h>
#include <cdp_txrx_cmn.h>
#include <cdp_txrx_misc.h>

#include "wlan_policy_mgr_api.h"
#include "wma_nan_datapath.h"
#if defined(CONFIG_HL_SUPPORT)
#include "wlan_tgt_def_config_hl.h"
#else
#include "wlan_tgt_def_config.h"
#endif
#include <wlan_dfs_tgt_api.h>
#include <cdp_txrx_handle.h>
#include "wlan_pmo_ucfg_api.h"
#include "wlan_reg_services_api.h"
#include <include/wlan_vdev_mlme.h>
#include "wma_he.h"
#include "wlan_roam_debug.h"
#include "wlan_ocb_ucfg_api.h"
#include "init_deinit_lmac.h"
#include <target_if.h>
#include "wlan_policy_mgr_ucfg.h"
#include "wlan_mlme_public_struct.h"
#include "wlan_mlme_api.h"
#include "wlan_mlme_main.h"
#include <wlan_dfs_utils_api.h>
#include "../../core/src/vdev_mgr_ops.h"
#include "wlan_utility.h"

struct cdp_vdev *wma_find_vdev_by_addr(tp_wma_handle wma, uint8_t *addr,
				   uint8_t *vdev_id)
{
	uint8_t i;
	struct wlan_objmgr_vdev *vdev;

	for (i = 0; i < wma->max_bssid; i++) {
		vdev = wma->interfaces[i].vdev;
		if (!vdev)
			continue;

		if (qdf_is_macaddr_equal(
			(struct qdf_mac_addr *)wlan_vdev_mlme_get_macaddr(vdev),
			(struct qdf_mac_addr *)addr) == true) {
			*vdev_id = i;
			return wlan_vdev_get_dp_handle(wma->interfaces[i].vdev);
		}
	}
	return NULL;
}


/**
 * wma_is_vdev_in_ap_mode() - check that vdev is in ap mode or not
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Helper function to know whether given vdev id
 * is in AP mode or not.
 *
 * Return: True/False
 */
bool wma_is_vdev_in_ap_mode(tp_wma_handle wma, uint8_t vdev_id)
{
	struct wma_txrx_node *intf = wma->interfaces;

	if (vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %hu", __func__, vdev_id);
		QDF_ASSERT(0);
		return false;
	}

	if ((intf[vdev_id].type == WMI_VDEV_TYPE_AP) &&
	    ((intf[vdev_id].sub_type == WMI_UNIFIED_VDEV_SUBTYPE_P2P_GO) ||
	     (intf[vdev_id].sub_type == 0)))
		return true;

	return false;
}

#ifdef QCA_IBSS_SUPPORT
bool wma_is_vdev_in_ibss_mode(tp_wma_handle wma, uint8_t vdev_id)
{
	struct wma_txrx_node *intf = wma->interfaces;

	if (vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %hu", __func__, vdev_id);
		QDF_ASSERT(0);
		return false;
	}

	if (intf[vdev_id].type == WMI_VDEV_TYPE_IBSS)
		return true;

	return false;
}

/**
 * wma_send_peer_atim_window_len() - send peer atim window length
 * @wma: wma handle
 * @add_sta: add sta  parameters
 *
 * This API sends the peer Atim Window length if IBSS
 * power save is enabled by the firmware.
 *
 * Return: none
 */
static void
wma_send_peer_atim_window_len(tp_wma_handle wma, tpAddStaParams add_sta)
{
	if (wma_is_vdev_in_ibss_mode(wma, add_sta->smesessionId) &&
	    wmi_service_enabled(wma->wmi_handle,
				wmi_service_ibss_pwrsave)) {
		/*
		 * If ATIM Window is present in the peer
		 * beacon then send it to firmware else
		 * configure Zero ATIM Window length to
		 * firmware.
		 */
		if (add_sta->atimIePresent) {
			wma_set_peer_param(wma, add_sta->staMac,
					   WMI_PEER_IBSS_ATIM_WINDOW_LENGTH,
					   add_sta->peerAtimWindowLength,
					   add_sta->smesessionId);
		} else {
			wma_set_peer_param(wma, add_sta->staMac,
					   WMI_PEER_IBSS_ATIM_WINDOW_LENGTH,
					   0, add_sta->smesessionId);
		}
	}
}
#else
static inline void
wma_send_peer_atim_window_len(tp_wma_handle wma, tpAddStaParams add_sta)
{
}
#endif /* QCA_IBSS_SUPPORT */

uint8_t *wma_get_vdev_bssid(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *mlme_obj;

	if (!vdev) {
		WMA_LOGE("%s vdev is NULL", __func__);
		return NULL;
	}

	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!mlme_obj) {
		WMA_LOGE("%s Failed to get mlme_obj", __func__);
		return NULL;
	}

	return mlme_obj->mgmt.generic.bssid;
}

struct cdp_vdev *wma_find_vdev_by_bssid(tp_wma_handle wma, uint8_t *bssid,
				    uint8_t *vdev_id)
{
	int i;
	uint8_t *bssid_addr;

	for (i = 0; i < wma->max_bssid; i++) {
		if (!wma->interfaces[i].vdev)
			continue;
		bssid_addr = wma_get_vdev_bssid(wma->interfaces[i].vdev);
		if (!bssid_addr)
			continue;

		if (qdf_is_macaddr_equal(
			(struct qdf_mac_addr *)bssid_addr,
			(struct qdf_mac_addr *)bssid) == true) {
			*vdev_id = i;
			return wlan_vdev_get_dp_handle(wma->interfaces[i].vdev);
		}
	}

	return NULL;
}

/**
 * wma_find_req_on_timer_expiry() - find request by address
 * @wma: wma handle
 * @req: pointer to the target request
 *
 * On timer expiry, the pointer to the req message is received from the
 * timer callback. Lookup the wma_hold_req_queue for the request with the
 * same address and return success if found.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wma_find_req_on_timer_expiry(tp_wma_handle wma,
					       struct wma_target_req *req)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	QDF_STATUS status;

	qdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(&wma->wma_hold_req_queue,
						      &next_node)) {
		qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGE(FL("unable to get msg node from request queue"));
		return QDF_STATUS_E_FAILURE;
	}

	do {
		cur_node = next_node;
		req_msg = qdf_container_of(cur_node,
					   struct wma_target_req, node);
		if (req_msg != req)
			continue;

		found = true;
		status = qdf_list_remove_node(&wma->wma_hold_req_queue,
					      cur_node);
		if (QDF_STATUS_SUCCESS != status) {
			qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
			WMA_LOGD(FL("Failed to remove request for req %pK"),
				 req);
			return QDF_STATUS_E_FAILURE;
		}
		break;
	} while (QDF_STATUS_SUCCESS  ==
		 qdf_list_peek_next(&wma->wma_hold_req_queue,
				    cur_node, &next_node));

	qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
	if (!found) {
		WMA_LOGE(FL("target request not found for req %pK"),
			 req);
		return QDF_STATUS_E_INVAL;
	}

	WMA_LOGD(FL("target request found for vdev id: %d type %d"),
		 req_msg->vdev_id, req_msg->type);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_find_req() - find target request for vdev id
 * @wma: wma handle
 * @vdev_id: vdev id
 * @type: request type
 *
 * Find target request for given vdev id & type of request.
 * Remove that request from active list.
 *
 * Return: return target request if found or NULL.
 */
static struct wma_target_req *wma_find_req(tp_wma_handle wma,
					   uint8_t vdev_id, uint8_t type)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	qdf_list_node_t *node1 = NULL, *node2 = NULL;
	QDF_STATUS status;

	qdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(&wma->wma_hold_req_queue,
						      &node2)) {
		qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGE(FL("unable to get msg node from request queue"));
		return NULL;
	}

	do {
		node1 = node2;
		req_msg = qdf_container_of(node1, struct wma_target_req, node);
		if (req_msg->vdev_id != vdev_id)
			continue;
		if (req_msg->type != type)
			continue;

		found = true;
		status = qdf_list_remove_node(&wma->wma_hold_req_queue, node1);
		if (QDF_STATUS_SUCCESS != status) {
			qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
			WMA_LOGD(FL("Failed to remove request for vdev_id %d type %d"),
				 vdev_id, type);
			return NULL;
		}
		break;
	} while (QDF_STATUS_SUCCESS  ==
			qdf_list_peek_next(&wma->wma_hold_req_queue, node1,
					   &node2));

	qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
	if (!found) {
		WMA_LOGE(FL("target request not found for vdev_id %d type %d"),
			 vdev_id, type);
		return NULL;
	}

	WMA_LOGD(FL("target request found for vdev id: %d type %d"),
		 vdev_id, type);

	return req_msg;
}

/**
 * wma_find_remove_req_msgtype() - find and remove request for vdev id
 * @wma: wma handle
 * @vdev_id: vdev id
 * @msg_type: message request type
 *
 * Find target request for given vdev id & sub type of request.
 * Remove the same from active list.
 *
 * Return: Success if request found, failure other wise
 */
static struct wma_target_req *wma_find_remove_req_msgtype(tp_wma_handle wma,
					   uint8_t vdev_id, uint32_t msg_type)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	qdf_list_node_t *node1 = NULL, *node2 = NULL;
	QDF_STATUS status;

	qdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(&wma->wma_hold_req_queue,
						      &node2)) {
		qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGE(FL("unable to get msg node from request queue"));
		return NULL;
	}

	do {
		node1 = node2;
		req_msg = qdf_container_of(node1, struct wma_target_req, node);
		if (req_msg->vdev_id != vdev_id)
			continue;
		if (req_msg->msg_type != msg_type)
			continue;

		found = true;
		status = qdf_list_remove_node(&wma->wma_hold_req_queue, node1);
		if (QDF_STATUS_SUCCESS != status) {
			qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
			WMA_LOGD(FL("Failed to remove request. vdev_id %d type %d"),
				 vdev_id, msg_type);
			return NULL;
		}
		break;
	} while (QDF_STATUS_SUCCESS  ==
			qdf_list_peek_next(&wma->wma_hold_req_queue, node1,
					   &node2));

	qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
	if (!found) {
		WMA_LOGE(FL("target request not found for vdev_id %d type %d"),
			 vdev_id, msg_type);
		return NULL;
	}

	WMA_LOGD(FL("target request found for vdev id: %d type %d"),
		 vdev_id, msg_type);

	return req_msg;
}

/**
 * wma_send_vdev_del_resp() - send vdev del resp to Upper layer
 * @param: params of del vdev response
 *
 * Return: none
 */
static inline void wma_send_vdev_del_resp(struct del_vdev_params *param)
{
	struct scheduler_msg sme_msg = {0};
	QDF_STATUS status;

	sme_msg.type = eWNI_SME_VDEV_DELETE_RSP;
	sme_msg.bodyptr = param;

	status = scheduler_post_message(QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_SME,
					QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(status))
		qdf_mem_free(param);
}

QDF_STATUS wma_vdev_detach_callback(struct vdev_delete_response *rsp)
{
	tp_wma_handle wma;
	struct wma_txrx_node *iface = NULL;
	struct del_vdev_params *param;

	wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma) {
		wma_err("wma handle is NULL for VDEV_%d", rsp->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	/* Sanitize the vdev id*/
	if (rsp->vdev_id > wma->max_bssid) {
		wma_err("vdev delete response with invalid vdev_id :%d",
			rsp->vdev_id);
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	iface = &wma->interfaces[rsp->vdev_id];

	if (!iface->del_staself_req) {
		wma_err(" iface handle is NULL for VDEV_%d", rsp->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	wma_debug("vdev del response received for VDEV_%d", rsp->vdev_id);
	param = (struct del_vdev_params *)iface->del_staself_req;
	iface->del_staself_req = NULL;

	if (iface->roam_scan_stats_req) {
		struct sir_roam_scan_stats *roam_scan_stats_req =
						iface->roam_scan_stats_req;

		iface->roam_scan_stats_req = NULL;
		qdf_mem_free(roam_scan_stats_req);
	}

	wma_vdev_deinit(iface);
	qdf_mem_zero(iface, sizeof(*iface));
	wma_vdev_init(iface);

	param->status = QDF_STATUS_SUCCESS;
	wma_send_vdev_del_resp(param);

	return param->status;
}

static void
wma_cdp_vdev_detach(ol_txrx_soc_handle soc, tp_wma_handle wma_handle,
		    uint8_t vdev_id)
{
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	struct wlan_objmgr_vdev *vdev = iface->vdev;

	if (!vdev) {
		WMA_LOGE(FL("vdev is NULL"));
		return;
	}

	if (soc && wlan_vdev_get_id(vdev) != WLAN_INVALID_VDEV_ID) {
		wlan_vdev_set_dp_handle(vdev, NULL);
		cdp_vdev_detach(soc, vdev_id, NULL, NULL);
	}
}

/**
 * wma_release_vdev_ref() - Release vdev object reference count
 * @iface: wma interface txrx node
 *
 * Purpose of this function is to release vdev object reference count
 * from wma interface txrx node.
 *
 * Return: None
 */
static void
wma_release_vdev_ref(struct wma_txrx_node *iface)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = iface->vdev;

	iface->vdev_active = false;
	iface->vdev = NULL;
	if (vdev)
		wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_WMA_ID);
}

/**
 * wma_handle_monitor_mode_vdev_detach() - Stop and down monitor mode vdev
 * @wma_handle: wma handle
 * @vdev_id: used to get wma interface txrx node
 *
 * Monitor mode is unconneted mode, so do explicit vdev stop and down
 *
 * Return: None
 */
static void wma_handle_monitor_mode_vdev_detach(tp_wma_handle wma,
						uint8_t vdev_id)
{
	struct wma_txrx_node *iface;

	iface = &wma->interfaces[vdev_id];
	wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
				      WLAN_VDEV_SM_EV_DOWN,
				      0, NULL);
	iface->vdev_active = false;
}

/**
 * wma_handle_vdev_detach() - wma vdev detach handler
 * @wma_handle: pointer to wma handle
 * @del_vdev_req_param: pointer to del req param
 *
 * Return: none.
 */
static QDF_STATUS wma_handle_vdev_detach(tp_wma_handle wma_handle,
			struct del_vdev_params *del_vdev_req_param)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint8_t vdev_id = del_vdev_req_param->vdev_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct vdev_mlme_obj *vdev_mlme;

	if (!soc) {
		WMA_LOGE("%s:SOC context is NULL", __func__);
		goto rel_ref;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
	if (!vdev_mlme) {
		wma_err("Failed to get vdev mlme obj for vdev id %d",
			del_vdev_req_param->vdev_id);
		goto rel_ref;
	}

	if (cds_get_conparam() == QDF_GLOBAL_MONITOR_MODE)
		wma_handle_monitor_mode_vdev_detach(wma_handle, vdev_id);

	iface->del_staself_req = del_vdev_req_param;
	wma_cdp_vdev_detach(soc, wma_handle, vdev_id);
	wma_release_vdev_ref(iface);

	status = vdev_mgr_delete_send(vdev_mlme);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Unable to remove an interface");
		goto out;
	}

	return status;

rel_ref:
	wma_cdp_vdev_detach(soc, wma_handle, vdev_id);
	wma_release_vdev_ref(iface);
out:
	wma_vdev_deinit(iface);
	qdf_mem_zero(iface, sizeof(*iface));
	wma_vdev_init(iface);
	return status;
}

/**
 * wma_self_peer_remove() - Self peer remove handler
 * @wma: wma handle
 * @del_vdev_req_param: vdev id
 * @generate_vdev_rsp: request type
 *
 * Return: success if peer delete command sent to firmware, else failure.
 */
static QDF_STATUS wma_self_peer_remove(tp_wma_handle wma_handle,
				       struct del_vdev_params *del_vdev_req)
{
	void *peer;
	struct cdp_pdev *pdev;
	QDF_STATUS qdf_status;
	uint8_t vdev_id = del_vdev_req->vdev_id;
	struct wma_target_req *msg = NULL;
	struct del_sta_self_rsp_params *sta_self_wmi_rsp;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	wma_debug("P2P Device: removing self peer %pM",
		  del_vdev_req->self_mac_addr);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		wma_err("Failed to get pdev");
		qdf_status = QDF_STATUS_E_FAULT;
		goto error;
	}

	peer = cdp_peer_find_by_addr(soc, pdev, del_vdev_req->self_mac_addr);
	if (!peer) {
		wma_err("Failed to find peer %pM", del_vdev_req->self_mac_addr);
		qdf_status = QDF_STATUS_E_FAULT;
		goto error;
	}

	qdf_status = wma_remove_peer(wma_handle, del_vdev_req->self_mac_addr,
				     vdev_id, peer, false);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		wma_err("wma_remove_peer is failed");
		goto error;
	}

	if (wmi_service_enabled(wma_handle->wmi_handle,
				wmi_service_sync_delete_cmds)) {
		sta_self_wmi_rsp =
			qdf_mem_malloc(sizeof(struct del_sta_self_rsp_params));
		if (!sta_self_wmi_rsp) {
			qdf_status = QDF_STATUS_E_NOMEM;
			goto error;
		}
		sta_self_wmi_rsp->self_sta_param = del_vdev_req;
		msg = wma_fill_hold_req(wma_handle, vdev_id,
					WMA_DELETE_STA_REQ,
					WMA_DEL_P2P_SELF_STA_RSP_START,
					sta_self_wmi_rsp,
					WMA_DELETE_STA_TIMEOUT);
		if (!msg) {
			wma_err("Failed to allocate request for vdev_id %d",
				vdev_id);
			wma_remove_req(wma_handle, vdev_id,
				       WMA_DEL_P2P_SELF_STA_RSP_START);
			qdf_mem_free(sta_self_wmi_rsp);
			qdf_status = QDF_STATUS_E_FAILURE;
			goto error;
		}
	}

error:
	return qdf_status;
}

static bool wma_vdev_uses_self_peer(uint32_t vdev_type, uint32_t vdev_subtype)
{
	switch (vdev_type) {
	case WMI_VDEV_TYPE_AP:
		return vdev_subtype == WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE;

	case WMI_VDEV_TYPE_MONITOR:
	case WMI_VDEV_TYPE_OCB:
		return true;

	default:
		return false;
	}
}

/**
 * wma_remove_objmgr_peer() - remove objmgr peer information from host driver
 * @wma: wma handle
 * @vdev_id: vdev id
 * @peer_addr: peer mac address
 *
 * Return: none
 */
static void wma_remove_objmgr_peer(tp_wma_handle wma, uint8_t vdev_id,
				   uint8_t *peer_addr)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_peer *obj_peer;
	struct wlan_objmgr_vdev *obj_vdev;
	struct wlan_objmgr_pdev *obj_pdev;
	uint8_t pdev_id = 0;

	psoc = wma->psoc;
	if (!psoc) {
		WMA_LOGE("%s:PSOC is NULL", __func__);
		return;
	}

	obj_vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
							WLAN_LEGACY_WMA_ID);
	if (!obj_vdev) {
		WMA_LOGE("Obj vdev not found. Unable to remove peer");
		return;
	}
	obj_pdev = wlan_vdev_get_pdev(obj_vdev);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(obj_pdev);
	obj_peer = wlan_objmgr_get_peer(psoc, pdev_id, peer_addr,
					WLAN_LEGACY_WMA_ID);
	if (obj_peer) {
		wlan_objmgr_peer_obj_delete(obj_peer);
		/* Unref to decrement ref happened in find_peer */
		wlan_objmgr_peer_release_ref(obj_peer, WLAN_LEGACY_WMA_ID);
		WMA_LOGD("Peer %pM deleted", peer_addr);
	} else {
		WMA_LOGE("Peer %pM not found", peer_addr);
	}

	wlan_objmgr_vdev_release_ref(obj_vdev, WLAN_LEGACY_WMA_ID);
}

static QDF_STATUS wma_check_for_deffered_peer_delete(tp_wma_handle wma_handle,
						     struct del_vdev_params
						     *pdel_vdev_req_param)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = pdel_vdev_req_param->vdev_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	uint32_t vdev_stop_type;

	if (qdf_atomic_read(&iface->bss_status) == WMA_BSS_STATUS_STARTED) {
		status = mlme_get_vdev_stop_type(iface->vdev, &vdev_stop_type);
		if (QDF_IS_STATUS_ERROR(status)) {
			wma_err("Failed to get wma req msg_type for vdev_id: %d",
				vdev_id);
			status = QDF_STATUS_E_INVAL;
			return status;
		}

		if (vdev_stop_type != WMA_DELETE_BSS_REQ) {
			status = QDF_STATUS_E_INVAL;
			return status;
		}

		wma_debug("BSS is not yet stopped. Defering vdev(vdev id %x) deletion",
			  vdev_id);
		iface->del_staself_req = pdel_vdev_req_param;
		iface->is_del_sta_defered = true;
	}

	return status;
}

static QDF_STATUS wma_vdev_self_peer_delete(tp_wma_handle wma_handle,
					    struct del_vdev_params
					    *pdel_vdev_req_param)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = pdel_vdev_req_param->vdev_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];

	if (wma_vdev_uses_self_peer(iface->type, iface->sub_type)) {
		status = wma_self_peer_remove(wma_handle, pdel_vdev_req_param);
		if (QDF_IS_STATUS_ERROR(status)) {
			wma_err("can't remove selfpeer, send rsp session: %d",
				vdev_id);
			status = wma_handle_vdev_detach(wma_handle,
							pdel_vdev_req_param);
			if (QDF_IS_STATUS_ERROR(status)) {
				wma_err("Trigger recovery for vdev %d",
					vdev_id);
				cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
			}
			return status;
		}
	} else if (iface->type == WMI_VDEV_TYPE_STA) {
		wma_remove_objmgr_peer(wma_handle, vdev_id,
				       pdel_vdev_req_param->self_mac_addr);
	}

	return status;
}

QDF_STATUS wma_vdev_detach(tp_wma_handle wma_handle,
			struct del_vdev_params *pdel_vdev_req_param)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = pdel_vdev_req_param->vdev_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];

	if (!iface->vdev) {
		WMA_LOGE("vdev %d is NULL", vdev_id);
		goto send_rsp;
	}

	if (!wlan_vdev_get_dp_handle(iface->vdev)) {
		WMA_LOGE("%s: Failed to get dp handle for vdev id %d",
			 __func__, vdev_id);
		goto send_rsp;
	}

	status = wma_check_for_deffered_peer_delete(wma_handle,
						    pdel_vdev_req_param);
	if (QDF_IS_STATUS_ERROR(status))
		goto  send_fail_rsp;

	if (iface->is_del_sta_defered)
		return status;

	iface->is_del_sta_defered = false;

	status = wma_vdev_self_peer_delete(wma_handle, pdel_vdev_req_param);

	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err("Failed to send self peer delete:%d", status);
		goto send_rsp;
	}

	if (iface->type != WMI_VDEV_TYPE_MONITOR)
		iface->vdev_active = false;

	if (!wma_vdev_uses_self_peer(iface->type, iface->sub_type) ||
	    !wmi_service_enabled(wma_handle->wmi_handle,
	    wmi_service_sync_delete_cmds)) {
		status = wma_handle_vdev_detach(wma_handle,
						pdel_vdev_req_param);
	}

	if (QDF_IS_STATUS_ERROR(status))
		goto send_rsp;

	return status;

send_fail_rsp:
	WMA_LOGE("rcvd del_self_sta without del_bss; vdev_id:%d", vdev_id);
	cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
	status = QDF_STATUS_E_FAILURE;

send_rsp:
	pdel_vdev_req_param->status = status;
	wma_send_vdev_del_resp(pdel_vdev_req_param);

	return status;
}

/**
 * wma_send_start_resp() - send vdev start response to upper layer
 * @wma: wma handle
 * @add_bss: add bss params
 * @resp_event: response params
 *
 * Return: none
 */
static void wma_send_start_resp(tp_wma_handle wma,
				struct add_bss_rsp *add_bss_rsp,
				struct vdev_start_response *rsp)
{
	struct wma_txrx_node *iface = &wma->interfaces[rsp->vdev_id];
	QDF_STATUS status;

	if (QDF_IS_STATUS_SUCCESS(rsp->status) &&
	    QDF_IS_STATUS_SUCCESS(add_bss_rsp->status)) {
		status =
		  wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
						WLAN_VDEV_SM_EV_START_RESP,
						sizeof(*add_bss_rsp),
						add_bss_rsp);
		if (QDF_IS_STATUS_SUCCESS(status))
			return;

		add_bss_rsp->status = status;
	}

	/* Send vdev stop if vdev start was success */
	if (QDF_IS_STATUS_ERROR(add_bss_rsp->status) &&
	    QDF_IS_STATUS_SUCCESS(rsp->status)) {
		wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
					      WLAN_VDEV_SM_EV_DOWN,
					      sizeof(*add_bss_rsp),
					      add_bss_rsp);
		return;
	}

	wma_remove_bss_peer_on_vdev_start_failure(wma, rsp->vdev_id);

	WMA_LOGD(FL("Sending add bss rsp to umac(vdev %d status %d)"),
		 rsp->vdev_id, add_bss_rsp->status);
	lim_handle_add_bss_rsp(wma->mac_context, add_bss_rsp);
}

/**
 * wma_vdev_start_rsp() - send vdev start response to upper layer
 * @wma: wma handle
 * @vdev: vdev
 * @resp_event: response params
 *
 * Return: none
 */
static void wma_vdev_start_rsp(tp_wma_handle wma, struct wlan_objmgr_vdev *vdev,
			       struct vdev_start_response *rsp)
{
	struct beacon_info *bcn;
	enum QDF_OPMODE opmode;
	struct add_bss_rsp *add_bss_rsp;

	opmode = wlan_vdev_mlme_get_opmode(vdev);

	add_bss_rsp = qdf_mem_malloc(sizeof(*add_bss_rsp));
	if (!add_bss_rsp)
		return;

	add_bss_rsp->vdev_id = rsp->vdev_id;
	add_bss_rsp->status = rsp->status;
	add_bss_rsp->chain_mask = rsp->chain_mask;
	add_bss_rsp->smps_mode  = host_map_smps_mode(rsp->smps_mode);

#ifdef QCA_IBSS_SUPPORT
	WMA_LOGD("%s: vdev start response received for %s mode", __func__,
		 opmode == QDF_IBSS_MODE ? "IBSS" : "non-IBSS");
#endif /* QCA_IBSS_SUPPORT */

	if (rsp->status)
		goto send_fail_resp;

	if ((opmode == QDF_P2P_GO_MODE) ||
	    (opmode == QDF_SAP_MODE)
#ifdef QCA_IBSS_SUPPORT
	    || (opmode == QDF_IBSS_MODE)
#endif /* QCA_IBSS_SUPPORT */
	    ) {
		wma->interfaces[rsp->vdev_id].beacon =
			qdf_mem_malloc(sizeof(struct beacon_info));

		bcn = wma->interfaces[rsp->vdev_id].beacon;
		if (!bcn) {
			add_bss_rsp->status = QDF_STATUS_E_NOMEM;
			goto send_fail_resp;
		}
		bcn->buf = qdf_nbuf_alloc(NULL, SIR_MAX_BEACON_SIZE, 0,
					  sizeof(uint32_t), 0);
		if (!bcn->buf) {
			qdf_mem_free(bcn);
			add_bss_rsp->status = QDF_STATUS_E_FAILURE;
			goto send_fail_resp;
		}
		bcn->seq_no = MIN_SW_SEQ;
		qdf_spinlock_create(&bcn->lock);
		qdf_atomic_set(&wma->interfaces[rsp->vdev_id].bss_status,
			       WMA_BSS_STATUS_STARTED);
		WMA_LOGD("%s: AP mode (type %d subtype %d) BSS is started",
			 __func__, wma->interfaces[rsp->vdev_id].type,
			 wma->interfaces[rsp->vdev_id].sub_type);

		WMA_LOGD("%s: Allocated beacon struct %pK, template memory %pK",
			 __func__, bcn, bcn->buf);
	}

send_fail_resp:
	wma_send_start_resp(wma, add_bss_rsp, rsp);
}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/**
 * wma_find_mcc_ap() - finds if device is operating AP in MCC mode or not
 * @wma: wma handle.
 * @vdev_id: vdev ID of device for which MCC has to be checked
 * @add: flag indicating if current device is added or deleted
 *
 * This function parses through all the interfaces in wma and finds if
 * any of those devces are in MCC mode with AP. If such a vdev is found
 * involved AP vdevs are sent WDA_UPDATE_Q2Q_IE_IND msg to update their
 * beacon template to include Q2Q IE.
 *
 * Return: none
 */
static void wma_find_mcc_ap(tp_wma_handle wma, uint8_t vdev_id, bool add)
{
	uint8_t i;
	uint16_t prev_ch_freq = 0;
	bool is_ap = false;
	bool result = false;
	uint8_t *ap_vdev_ids = NULL;
	uint8_t num_ch = 0;

	ap_vdev_ids = qdf_mem_malloc(wma->max_bssid);
	if (!ap_vdev_ids)
		return;

	for (i = 0; i < wma->max_bssid; i++) {
		ap_vdev_ids[i] = -1;
		if (add == false && i == vdev_id)
			continue;

		if (wma_is_vdev_up(vdev_id) || (i == vdev_id && add)) {
			if (wma->interfaces[i].type == WMI_VDEV_TYPE_AP) {
				is_ap = true;
				ap_vdev_ids[i] = i;
			}

			if (wma->interfaces[i].mhz != prev_ch_freq) {
				num_ch++;
				prev_ch_freq = wma->interfaces[i].mhz;
			}
		}
	}

	if (is_ap && (num_ch > 1))
		result = true;
	else
		result = false;

	wma_send_msg(wma, WMA_UPDATE_Q2Q_IE_IND, (void *)ap_vdev_ids, result);
}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

/**
 * wma_handle_hidden_ssid_restart() - handle hidden ssid restart
 * @wma: wma handle
 * @iface: interfcae pointer
 *
 * Return: none
 */
static void wma_handle_hidden_ssid_restart(tp_wma_handle wma,
					   struct wma_txrx_node *iface)
{
	WMA_LOGE("%s: vdev restart event recevied for hidden ssid set using IOCTL",
		 __func__);

	wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
				      WLAN_VDEV_SM_EV_RESTART_RESP,
				      0, NULL);
}

/**
 * wma_update_peer_phymode_after_vdev_restart() - for sta set new phymode to
 * bss peer after vdev restart.
 * @wma: wma handle
 * @iface: interfcae pointer
 *
 * Return: none
 */
static
void wma_update_peer_phymode_after_vdev_restart(tp_wma_handle wma,
						struct wma_txrx_node *iface)
{
	wmi_host_channel_width ch_width;
	uint8_t vdev_id;
	enum wlan_phymode bss_phymode;
	uint32_t fw_phymode;
	uint8_t *bssid;
	struct wlan_channel *des_chan;
	QDF_STATUS status;

	vdev_id = wlan_vdev_get_id(iface->vdev);
	/* for CSA case firmware expects phymode before ch_wd */
	bssid = wma_get_vdev_bssid(iface->vdev);
	if (!bssid) {
		WMA_LOGE("%s:Failed to get bssid for vdev_id %d",
			 __func__, vdev_id);
		return;
	}

	des_chan = wlan_vdev_mlme_get_des_chan(iface->vdev);
	bss_phymode = des_chan->ch_phymode;

	/* update new phymode to peer */
	wma_objmgr_set_peer_mlme_phymode(wma, bssid, bss_phymode);
	fw_phymode = wma_host_to_fw_phymode(bss_phymode);

	/* for CSA case firmware expects phymode before ch_wd */
	status = wma_set_peer_param(wma, bssid, WMI_PEER_PHYMODE, fw_phymode,
				    vdev_id);
	WMA_LOGD("%s:vdev_id %d fw_phy_mode %d bss_phymode %d status %d",
		 __func__, vdev_id, fw_phymode, bss_phymode, status);

	ch_width = wmi_get_ch_width_from_phy_mode(wma->wmi_handle, fw_phymode);
	status = wma_set_peer_param(wma, bssid, WMI_PEER_CHWIDTH, ch_width,
				    vdev_id);
	WMA_LOGD("%s:vdev_id %d chanwidth %d status %d", __func__, vdev_id,
		 ch_width, status);
}


QDF_STATUS wma_handle_channel_switch_resp(tp_wma_handle wma,
					  struct vdev_start_response *rsp)
{
	enum wlan_vdev_sm_evt  event;
	struct wma_txrx_node *iface;

	iface = &wma->interfaces[rsp->vdev_id];
	WMA_LOGD("%s: Send channel switch resp vdev %d status %d",
		 __func__, rsp->vdev_id, rsp->status);

	/* Indicate channel switch failure to LIM */
	if (QDF_IS_STATUS_ERROR(rsp->status) &&
	    (iface->type == WMI_VDEV_TYPE_MONITOR ||
	     wma_is_vdev_in_ap_mode(wma, rsp->vdev_id) ||
	     mlme_is_chan_switch_in_progress(iface->vdev))) {
		mlme_set_chan_switch_in_progress(iface->vdev, false);
		lim_process_switch_channel_rsp(wma->mac_context, rsp);
		return QDF_STATUS_SUCCESS;
	}

	if (QDF_IS_STATUS_SUCCESS(rsp->status) &&
	    rsp->resp_type == WMI_VDEV_RESTART_RESP_EVENT &&
	    iface->type == WMI_VDEV_TYPE_STA)
		wma_update_peer_phymode_after_vdev_restart(wma, iface);

	if (wma_is_vdev_in_ap_mode(wma, rsp->vdev_id) ||
	    mlme_is_chan_switch_in_progress(iface->vdev))
		event = WLAN_VDEV_SM_EV_RESTART_RESP;
	else
		event = WLAN_VDEV_SM_EV_START_RESP;
	wlan_vdev_mlme_sm_deliver_evt(iface->vdev, event,
				      sizeof(rsp), rsp);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wma_vdev_start_resp_handler(struct vdev_mlme_obj *vdev_mlme,
				       struct vdev_start_response *rsp)
{
	tp_wma_handle wma;
	struct wma_txrx_node *iface;
	target_resource_config *wlan_res_cfg;
	struct wlan_objmgr_psoc *psoc;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	struct mac_context *mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
#endif
	QDF_STATUS status;
	enum vdev_assoc_type assoc_type = VDEV_ASSOC;
	struct vdev_mlme_obj *mlme_obj;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		wma_err("wma wma is NULL for VDEV_%d", rsp->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}
	psoc = wma->psoc;
	if (!psoc) {
		WMA_LOGE("%s: psoc is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (!mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		policy_mgr_set_do_hw_mode_change_flag(
			psoc, false);
		return QDF_STATUS_E_FAILURE;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	WMA_LOGD("%s: Enter", __func__);

	wlan_res_cfg = lmac_get_tgt_res_cfg(psoc);
	if (!wlan_res_cfg) {
		WMA_LOGE("%s: Wlan resource config is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (rsp->vdev_id >= wma->max_bssid) {
		WMA_LOGE("Invalid vdev id received from firmware");
		return QDF_STATUS_E_FAILURE;
	}

	if (wma_is_vdev_in_ap_mode(wma, rsp->vdev_id))
		tgt_dfs_radar_enable(wma->pdev, 0, 0, true);

	if (rsp->status == QDF_STATUS_SUCCESS) {
		wma->interfaces[rsp->vdev_id].tx_streams =
			rsp->cfgd_tx_streams;

		if (wlan_res_cfg->use_pdev_id) {
			if (rsp->mac_id == WMI_PDEV_ID_SOC) {
				wma_err("soc level id received for mac id");
				return -QDF_STATUS_E_INVAL;
			}
			wma->interfaces[rsp->vdev_id].mac_id =
				WMA_PDEV_TO_MAC_MAP(rsp->mac_id);
		} else {
			wma->interfaces[rsp->vdev_id].mac_id =
			rsp->mac_id;
		}

		WMA_LOGD("%s: vdev:%d tx ss=%d rx ss=%d chain mask=%d mac=%d",
				__func__,
				rsp->vdev_id,
				rsp->cfgd_tx_streams,
				rsp->cfgd_rx_streams,
				rsp->chain_mask,
				wma->interfaces[rsp->vdev_id].mac_id);
	}

	iface = &wma->interfaces[rsp->vdev_id];

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (rsp->status == QDF_STATUS_SUCCESS
		&& mac_ctx->sap.sap_channel_avoidance)
		wma_find_mcc_ap(wma, rsp->vdev_id, true);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	if (wma_get_hidden_ssid_restart_in_progress(iface) &&
	    wma_is_vdev_in_ap_mode(wma, rsp->vdev_id)) {
		wma_handle_hidden_ssid_restart(wma, iface);
		return QDF_STATUS_SUCCESS;
	}

	if (iface->type == WMI_VDEV_TYPE_STA)
		assoc_type = mlme_get_assoc_type(vdev_mlme->vdev);

	if (mlme_is_chan_switch_in_progress(iface->vdev) ||
	    iface->type == WMI_VDEV_TYPE_MONITOR ||
	    (iface->type == WMI_VDEV_TYPE_STA &&
	     (assoc_type == VDEV_ASSOC || assoc_type == VDEV_REASSOC))) {
		status = wma_handle_channel_switch_resp(wma,
							rsp);
		if (QDF_IS_STATUS_ERROR(status))
			return QDF_STATUS_E_FAILURE;
	}  else if (iface->type == WMI_VDEV_TYPE_OCB) {
		mlme_obj = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
		mlme_obj->proto.sta.assoc_id = iface->aid;
		if (vdev_mgr_up_send(mlme_obj) != QDF_STATUS_SUCCESS) {
			WMA_LOGE(FL("failed to send vdev up"));
			policy_mgr_set_do_hw_mode_change_flag(
				wma->psoc, false);
			return QDF_STATUS_E_FAILURE;
		}
		ucfg_ocb_config_channel(wma->pdev);
	} else {
		struct qdf_mac_addr bss_peer;

		status =
			mlme_get_vdev_bss_peer_mac_addr(iface->vdev, &bss_peer);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("%s: Failed to get bssid", __func__);
			return QDF_STATUS_E_INVAL;
		}
		mlme_obj = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
		if (!mlme_obj) {
			WMA_LOGE("%s: Failed to get mlme obj", __func__);
			return QDF_STATUS_E_INVAL;
		}
		qdf_mem_copy(mlme_obj->mgmt.generic.bssid, bss_peer.bytes,
			     QDF_MAC_ADDR_SIZE);
		wma_vdev_start_rsp(wma, vdev_mlme->vdev, rsp);
	}
	if (iface->type == WMI_VDEV_TYPE_AP && wma_is_vdev_up(rsp->vdev_id))
		wma_set_sap_keepalive(wma, rsp->vdev_id);

	return QDF_STATUS_SUCCESS;
}

bool wma_is_vdev_valid(uint32_t vdev_id)
{
	tp_wma_handle wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		WMA_LOGD("%s: vdev_id: %d, null wma_handle", __func__, vdev_id);
		return false;
	}

	/* No of interface are allocated based on max_bssid value */
	if (vdev_id >= wma_handle->max_bssid) {
		WMA_LOGD("%s: vdev_id: %d is invalid, max_bssid: %d",
				__func__, vdev_id, wma_handle->max_bssid);
		return false;
	}

	WMA_LOGD("%s: vdev_id: %d, vdev_active: %d", __func__, vdev_id,
		 wma_handle->interfaces[vdev_id].vdev_active);

	return wma_handle->interfaces[vdev_id].vdev_active;
}

/**
 * wma_vdev_set_param() - set per vdev params in fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 * @param_id: parameter id
 * @param_value: parameter value
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS
wma_vdev_set_param(wmi_unified_t wmi_handle, uint32_t if_id,
				uint32_t param_id, uint32_t param_value)
{
	struct vdev_set_params param = {0};

	if (!wma_is_vdev_valid(if_id)) {
		WMA_LOGE(FL("vdev_id: %d is not active reject the req: param id %d val %d"),
			if_id, param_id, param_value);
		return QDF_STATUS_E_INVAL;
	}

	param.vdev_id = if_id;
	param.param_id = param_id;
	param.param_value = param_value;

	return wmi_unified_vdev_set_param_send(wmi_handle, &param);
}

/**
 * wma_set_peer_authorized_cb() - set peer authorized callback function
 * @wma_ctx: wma handle
 * @auth_cb: peer authorized callback
 *
 * Return: none
 */
void wma_set_peer_authorized_cb(void *wma_ctx, wma_peer_authorized_fp auth_cb)
{
	tp_wma_handle wma_handle = (tp_wma_handle) wma_ctx;

	wma_handle->peer_authorized_cb = auth_cb;
}

/**
 * wma_set_peer_param() - set peer parameter in fw
 * @wma_ctx: wma handle
 * @peer_addr: peer mac address
 * @param_id: parameter id
 * @param_value: parameter value
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_set_peer_param(void *wma_ctx, uint8_t *peer_addr,
			      uint32_t param_id, uint32_t param_value,
			      uint32_t vdev_id)
{
	tp_wma_handle wma_handle = (tp_wma_handle) wma_ctx;
	struct peer_set_params param = {0};
	QDF_STATUS status;

	param.vdev_id = vdev_id;
	param.param_value = param_value;
	param.param_id = param_id;

	status = wmi_set_peer_param_send(wma_handle->wmi_handle,
					 peer_addr,
					 &param);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("vdev_id: %d peer set failed, id %d, val %d",
			 vdev_id, param_id, param_value);
	return status;
}

/**
 * wma_peer_unmap_conf_send - send peer unmap conf cmnd to fw
 * @wma_ctx: wma handle
 * @msg: peer unmap conf params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_peer_unmap_conf_send(tp_wma_handle wma,
				    struct send_peer_unmap_conf_params *msg)
{
	QDF_STATUS qdf_status;

	if (!msg) {
		WMA_LOGE("%s: null input params", __func__);
		return QDF_STATUS_E_INVAL;
	}

	qdf_status = wmi_unified_peer_unmap_conf_send(
					wma->wmi_handle,
					msg->vdev_id,
					msg->peer_id_cnt,
					msg->peer_id_list);

	if (qdf_status != QDF_STATUS_SUCCESS)
		WMA_LOGE("%s: peer_unmap_conf_send failed %d",
			 __func__, qdf_status);

	qdf_mem_free(msg->peer_id_list);
	msg->peer_id_list = NULL;

	return qdf_status;
}

/**
 * wma_peer_unmap_conf_cb - send peer unmap conf cmnd to fw
 * @vdev_id: vdev id
 * @peer_id_cnt: no of peer id
 * @peer_id_list: list of peer ids
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_peer_unmap_conf_cb(uint8_t vdev_id,
				  uint32_t peer_id_cnt,
				  uint16_t *peer_id_list)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	QDF_STATUS qdf_status;

	if (!wma) {
		WMA_LOGE("%s: peer_id_cnt: %d, null wma_handle",
			 __func__, peer_id_cnt);
		return QDF_STATUS_E_INVAL;
	}

	qdf_status = wmi_unified_peer_unmap_conf_send(
						wma->wmi_handle,
						vdev_id, peer_id_cnt,
						peer_id_list);

	if (qdf_status == QDF_STATUS_E_BUSY) {
		QDF_STATUS retcode;
		struct scheduler_msg msg = {0};
		struct send_peer_unmap_conf_params *peer_unmap_conf_req;
		void *mac_ctx = cds_get_context(QDF_MODULE_ID_PE);

		WMA_LOGD("%s: post unmap_conf cmd to MC thread", __func__);

		if (!mac_ctx) {
			WMA_LOGE("%s: mac_ctx is NULL", __func__);
			return QDF_STATUS_E_FAILURE;
		}

		peer_unmap_conf_req = qdf_mem_malloc(sizeof(
					struct send_peer_unmap_conf_params));

		if (!peer_unmap_conf_req) {
			WMA_LOGE("%s: peer_unmap_conf_req memory alloc failed",
				 __func__);
			return QDF_STATUS_E_NOMEM;
		}

		peer_unmap_conf_req->vdev_id = vdev_id;
		peer_unmap_conf_req->peer_id_cnt = peer_id_cnt;
		peer_unmap_conf_req->peer_id_list =  qdf_mem_malloc(
					sizeof(uint16_t) * peer_id_cnt);
		if (!peer_unmap_conf_req->peer_id_list) {
			WMA_LOGE("%s: peer_id_list memory alloc failed",
				 __func__);
			qdf_mem_free(peer_unmap_conf_req);
			peer_unmap_conf_req = NULL;
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(peer_unmap_conf_req->peer_id_list,
			     peer_id_list, sizeof(uint16_t) * peer_id_cnt);

		msg.type = WMA_SEND_PEER_UNMAP_CONF;
		msg.reserved = 0;
		msg.bodyptr = peer_unmap_conf_req;
		msg.bodyval = 0;

		retcode = wma_post_ctrl_msg(mac_ctx, &msg);
		if (retcode != QDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: wma_post_ctrl_msg failed", __func__);
			qdf_mem_free(peer_unmap_conf_req->peer_id_list);
			qdf_mem_free(peer_unmap_conf_req);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return qdf_status;
}

/**
 * wma_remove_peer() - remove peer information from host driver and fw
 * @wma: wma handle
 * @bssid: mac address
 * @vdev_id: vdev id
 * @peer: peer ptr
 * @roam_synch_in_progress: roam in progress flag
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_remove_peer(tp_wma_handle wma, uint8_t *bssid,
			   uint8_t vdev_id, void *peer,
			   bool roam_synch_in_progress)
{
#define PEER_ALL_TID_BITMASK 0xffffffff
	uint32_t peer_tid_bitmap = PEER_ALL_TID_BITMASK;
	uint8_t *peer_addr = bssid;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE] = {0};
	struct peer_flush_params param = {0};
	uint8_t *peer_mac_addr;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	uint32_t bitmap = 1 << CDP_PEER_DELETE_NO_SPECIAL;
	bool peer_unmap_conf_support_enabled;

	if (!wma->interfaces[vdev_id].peer_count) {
		WMA_LOGE("%s: Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
			 __func__, bssid, vdev_id,
			wma->interfaces[vdev_id].peer_count);
		cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc) {
		WMA_LOGE("%s:SOC context is NULL", __func__);
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}

	if (!peer) {
		WMA_LOGE("%s: PEER is NULL for vdev_id: %d", __func__, vdev_id);
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}
	peer_unmap_conf_support_enabled =
				cdp_cfg_get_peer_unmap_conf_support(soc);

	peer_mac_addr = cdp_peer_get_peer_mac_addr(soc, peer);
	if (!peer_mac_addr) {
		WMA_LOGE("%s: peer mac addr is NULL, Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
			 __func__, bssid, vdev_id,
			 wma->interfaces[vdev_id].peer_count);
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}

	cdp_peer_teardown(soc, vdev_id, peer);

	if (roam_synch_in_progress)
		goto peer_detach;
	/* Flush all TIDs except MGMT TID for this peer in Target */
	peer_tid_bitmap &= ~(0x1 << WMI_MGMT_TID);
	param.peer_tid_bitmap = peer_tid_bitmap;
	param.vdev_id = vdev_id;
	wmi_unified_peer_flush_tids_send(wma->wmi_handle, bssid,
			&param);

	if (wma_is_vdev_in_ibss_mode(wma, vdev_id)) {
		WMA_LOGD("%s: bssid %pM peer->mac_addr %pM", __func__,
			 bssid, peer_mac_addr);
		peer_addr = peer_mac_addr;
	}

	/* peer->ref_cnt is not visible in WMA */
	wlan_roam_debug_log(vdev_id, DEBUG_PEER_DELETE_SEND,
			    DEBUG_INVALID_PEER_ID, peer_addr, peer,
			    0, 0);
	qdf_status = wmi_unified_peer_delete_send(wma->wmi_handle, peer_addr,
						  vdev_id);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMA_LOGE("%s Peer delete could not be sent to firmware %d",
			 __func__, qdf_status);
		/* Clear default bit and set to NOT_START_UNMAP */
		bitmap = 1 << CDP_PEER_DO_NOT_START_UNMAP_TIMER;
		qdf_status = QDF_STATUS_E_FAILURE;
	}

peer_detach:
	WMA_LOGD("%s: vdevid %d is detaching with peer_addr %pM peer_count %d",
		__func__, vdev_id, peer_mac_addr,
		wma->interfaces[vdev_id].peer_count);
	/* Copy peer mac to find and delete objmgr peer */
	qdf_mem_copy(peer_mac, peer_mac_addr, QDF_MAC_ADDR_SIZE);
	if (roam_synch_in_progress &&
	    is_cdp_peer_detach_force_delete_supported(soc)) {
		if (!peer_unmap_conf_support_enabled) {
			WMA_LOGD("%s: LFR3: trigger force delete for peer %pM",
				 __func__, peer_mac_addr);
			cdp_peer_detach_force_delete(soc, peer);
		} else {
			cdp_peer_delete_sync(soc, vdev_id, peer_mac_addr,
					     wma_peer_unmap_conf_cb,
					     bitmap);
		}
	} else {
		if (roam_synch_in_progress) {
			WMA_LOGD("%s: LFR3: normal peer delete for peer %pM",
				 __func__, peer_mac_addr);
		}
		if (peer_unmap_conf_support_enabled)
			cdp_peer_delete_sync(soc, vdev_id, peer_mac_addr,
					     wma_peer_unmap_conf_cb,
					     bitmap);
		else
			cdp_peer_delete(soc, vdev_id, peer_mac_addr, bitmap);
	}

	wma_remove_objmgr_peer(wma, vdev_id, peer_mac);

	wma->interfaces[vdev_id].peer_count--;
#undef PEER_ALL_TID_BITMASK

	return qdf_status;
}

/**
 * wma_find_duplicate_peer_on_other_vdev() - Find if same peer exist
 * on other vdevs
 * @wma: wma handle
 * @pdev: txrx pdev ptr
 * @vdev_id: vdev id of vdev on which the peer
 *           needs to be added
 * @peer_mac: peer mac addr which needs to be added
 *
 * Check if peer with same MAC is present on vdev other then
 * the provided vdev_id
 *
 * Return: true if same peer is present on vdev other then vdev_id
 * else return false
 */
static bool wma_find_duplicate_peer_on_other_vdev(tp_wma_handle wma,
	struct cdp_pdev *pdev, uint8_t vdev_id, uint8_t *peer_mac)
{
	int i;
	struct cdp_vdev *handle;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_objmgr_vdev *vdev;

	for (i = 0; i < wma->max_bssid; i++) {
		vdev = wma->interfaces[i].vdev;
		if (!vdev)
			continue;
		handle = wlan_vdev_get_dp_handle(wma->interfaces[i].vdev);
		/* Need to check vdevs other than the vdev_id */
		if (vdev_id == i ||
		   !handle)
			continue;
		if (cdp_peer_find_by_addr_and_vdev(soc, pdev,
			handle, peer_mac)) {
			WMA_LOGE("%s :Duplicate peer %pM already exist on vdev %d",
				__func__, peer_mac, i);
			return true;
		}
	}
	return false;
}

/**
 * wma_get_peer_type() - Determine the type of peer(eg. STA/AP) and return it
 * @wma: wma handle
 * @vdev_id: vdev id
 * @peer_addr: peer mac address
 * @wma_peer_type: wma peer type
 *
 * Return: Peer type
 */
static int wma_get_obj_mgr_peer_type(tp_wma_handle wma, uint8_t vdev_id,
				     uint8_t *peer_addr, uint32_t wma_peer_type)

{
	uint32_t obj_peer_type = 0;
	struct wlan_objmgr_vdev *vdev;
	uint8_t *addr;

	vdev = wma->interfaces[vdev_id].vdev;
	if (!vdev) {
		WMA_LOGE("Couldnt find vdev for VDEV_%d", vdev_id);
		return obj_peer_type;
	}
	addr = wlan_vdev_mlme_get_macaddr(vdev);

	WMA_LOGD("vdev id %d vdev type %d vdev subtype %d peer addr %pM vdev addr %pM",
		 vdev_id, wma->interfaces[vdev_id].type,
		 wma->interfaces[vdev_id].sub_type, peer_addr,
		 addr);

	if (wma_peer_type == WMI_PEER_TYPE_TDLS)
		return WLAN_PEER_TDLS;

	if (!qdf_mem_cmp(addr, peer_addr, QDF_MAC_ADDR_SIZE)) {
		obj_peer_type = WLAN_PEER_SELF;
	} else if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_STA) {
		if (wma->interfaces[vdev_id].sub_type ==
					WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT)
			obj_peer_type = WLAN_PEER_P2P_GO;
		else
			obj_peer_type = WLAN_PEER_AP;
	} else if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_AP) {
			obj_peer_type = WLAN_PEER_STA;
	} else if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_IBSS) {
		obj_peer_type = WLAN_PEER_IBSS;
	} else if (wma->interfaces[vdev_id].type == WMI_VDEV_TYPE_NDI) {
		obj_peer_type = WLAN_PEER_NDP;
	} else {
		WMA_LOGE("Couldnt find peertype for type %d and sub type %d",
			 wma->interfaces[vdev_id].type,
			 wma->interfaces[vdev_id].sub_type);
	}

	return obj_peer_type;

}

/**
 * wma_create_objmgr_peer() - create objmgr peer information in host driver
 * @wma: wma handle
 * @vdev_id: vdev id
 * @peer_addr: peer mac address
 * @wma_peer_type: peer type
 *
 * Return: objmgr peer pointer
 */

static struct wlan_objmgr_peer *wma_create_objmgr_peer(tp_wma_handle wma,
						       uint8_t vdev_id,
						       uint8_t *peer_addr,
						       uint32_t wma_peer_type)
{
	uint32_t obj_peer_type = 0;
	struct wlan_objmgr_peer *obj_peer = NULL;
	struct wlan_objmgr_vdev *obj_vdev = NULL;
	struct wlan_objmgr_psoc *psoc = wma->psoc;

	obj_peer_type = wma_get_obj_mgr_peer_type(wma, vdev_id, peer_addr,
						  wma_peer_type);
	if (!obj_peer_type) {
		WMA_LOGE("Invalid obj peer type. Unable to create peer %d",
							obj_peer_type);
		return NULL;
	}

	/* Create obj_mgr peer */
	obj_vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_LEGACY_WMA_ID);

	if (!obj_vdev) {
		WMA_LOGE("Invalid obj vdev. Unable to create peer %d",
							obj_peer_type);
		return NULL;
	}

	obj_peer = wlan_objmgr_peer_obj_create(obj_vdev, obj_peer_type,
						peer_addr);
	wlan_objmgr_vdev_release_ref(obj_vdev, WLAN_LEGACY_WMA_ID);
	if (obj_peer)
		WMA_LOGD("Peer %pM added successfully! Type: %d", peer_addr,
			 obj_peer_type);

	return obj_peer;

}
/**
 * wma_create_peer() - send peer create command to fw
 * @wma: wma handle
 * @pdev: txrx pdev ptr
 * @vdev: txrx vdev ptr
 * @peer_addr: peer mac addr
 * @peer_type: peer type
 * @vdev_id: vdev id
 * @roam_synch_in_progress: roam in progress
 *
 * Return: QDF status
 */
QDF_STATUS wma_create_peer(tp_wma_handle wma, struct cdp_pdev *pdev,
			  struct cdp_vdev *vdev,
			  uint8_t peer_addr[QDF_MAC_ADDR_SIZE],
			  uint32_t peer_type, uint8_t vdev_id,
			  bool roam_synch_in_progress)
{
	void *peer = NULL;
	struct peer_create_params param = {0};
	uint8_t *mac_addr_raw;
	void *dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_objmgr_psoc *psoc = wma->psoc;
	target_resource_config *wlan_res_cfg;
	struct wlan_objmgr_peer *obj_peer = NULL;

	if (!psoc) {
		WMA_LOGE("%s: psoc is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	wlan_res_cfg = lmac_get_tgt_res_cfg(psoc);
	if (!wlan_res_cfg) {
		WMA_LOGE("%s: psoc target res cfg is null", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (++wma->interfaces[vdev_id].peer_count >
	    wlan_res_cfg->num_peers) {
		WMA_LOGE("%s, the peer count exceeds the limit %d", __func__,
			 wma->interfaces[vdev_id].peer_count - 1);
		goto err;
	}

	if (!dp_soc) {
		WMA_LOGE("%s:DP SOC context is NULL", __func__);
		goto err;
	}

	if (qdf_is_macaddr_group((struct qdf_mac_addr *)peer_addr) ||
	    qdf_is_macaddr_zero((struct qdf_mac_addr *)peer_addr)) {
		WMA_LOGE("Invalid peer address received reject it");
		goto err;
	}

	/*
	 * Check if peer with same MAC exist on other Vdev, If so avoid
	 * adding this peer, as it will cause FW to crash.
	 */
	if (wma_find_duplicate_peer_on_other_vdev(wma, pdev,
	   vdev_id, peer_addr))
		goto err;

	obj_peer = wma_create_objmgr_peer(wma, vdev_id, peer_addr, peer_type);
	if (!obj_peer)
		goto err;

	/* The peer object should be created before sending the WMI peer
	 * create command to firmware. This is to prevent a race condition
	 * where the HTT peer map event is received before the peer object
	 * is created in the data path
	 */
	peer = cdp_peer_create(dp_soc, vdev_id, peer_addr);
	if (!peer) {
		WMA_LOGE("%s : Unable to attach peer %pM", __func__, peer_addr);
		wlan_objmgr_peer_obj_delete(obj_peer);
		goto err;
	}
	WMA_LOGD("%s: vdev %pK is attaching peer:%pK peer_addr %pM to vdev_id %d, peer_count - %d",
		 __func__, vdev, peer, peer_addr, vdev_id,
		 wma->interfaces[vdev_id].peer_count);

	wlan_peer_set_dp_handle(obj_peer, peer);
	if (peer_type == WMI_PEER_TYPE_TDLS)
		cdp_peer_set_peer_as_tdls(dp_soc, peer, true);

	if (roam_synch_in_progress) {
		WMA_LOGD("%s: LFR3: Created peer %pK with peer_addr %pM vdev_id %d, peer_count - %d",
			 __func__, peer, peer_addr, vdev_id,
			 wma->interfaces[vdev_id].peer_count);
		cdp_peer_setup(dp_soc, vdev_id, peer_addr);
		return QDF_STATUS_SUCCESS;
	}
	param.peer_addr = peer_addr;
	param.peer_type = peer_type;
	param.vdev_id = vdev_id;
	if (wmi_unified_peer_create_send(wma->wmi_handle,
					 &param) != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s : Unable to create peer in Target", __func__);
		if (cdp_cfg_get_peer_unmap_conf_support(dp_soc))
			cdp_peer_delete_sync(
				dp_soc, vdev_id, peer_addr,
				wma_peer_unmap_conf_cb,
				1 << CDP_PEER_DO_NOT_START_UNMAP_TIMER);
		else
			cdp_peer_delete(
				dp_soc, vdev_id, peer_addr,
				1 << CDP_PEER_DO_NOT_START_UNMAP_TIMER);
		wlan_objmgr_peer_obj_delete(obj_peer);
		goto err;
	}

	WMA_LOGD("%s: Created peer %pK with peer_addr %pM vdev_id %d, peer_count - %d",
		  __func__, peer, peer_addr, vdev_id,
		  wma->interfaces[vdev_id].peer_count);

	wlan_roam_debug_log(vdev_id, DEBUG_PEER_CREATE_SEND,
			    DEBUG_INVALID_PEER_ID, peer_addr, peer, 0, 0);
	cdp_peer_setup(dp_soc, vdev_id, peer_addr);

	WMA_LOGD("%s: Initialized peer with peer_addr %pM vdev_id %d",
		__func__, peer_addr, vdev_id);

	mac_addr_raw = cdp_get_vdev_mac_addr(dp_soc, vdev_id);
	if (!mac_addr_raw) {
		WMA_LOGE("%s: peer mac addr is NULL", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* for each remote ibss peer, clear its keys */
	if (wma_is_vdev_in_ibss_mode(wma, vdev_id) &&
	    qdf_mem_cmp(peer_addr, mac_addr_raw, QDF_MAC_ADDR_SIZE)) {
		tpSetStaKeyParams key_info;

		key_info = qdf_mem_malloc(sizeof(*key_info));
		if (!key_info) {
			return QDF_STATUS_E_NOMEM;
		}
		WMA_LOGD("%s: remote ibss peer %pM key clearing\n", __func__,
			 peer_addr);
		qdf_mem_zero(key_info, sizeof(*key_info));
		key_info->vdev_id = vdev_id;
		qdf_mem_copy(key_info->peer_macaddr.bytes, peer_addr,
				QDF_MAC_ADDR_SIZE);
		key_info->sendRsp = false;

		wma_set_stakey(wma, key_info);
	}

	return QDF_STATUS_SUCCESS;
err:
	wma->interfaces[vdev_id].peer_count--;
	return QDF_STATUS_E_FAILURE;
}

/**
 * wma_remove_bss_peer() - remove BSS peer
 * @wma: pointer to WMA handle
 * @pdev: pointer to PDEV
 * @vdev_id: vdev id on which delete BSS request was received
 * @vdev_stop_resp: pointer to Delete BSS response
 *
 * This function is called on receiving vdev stop response from FW or
 * vdev stop response timeout. In case of IBSS/NDI, use vdev's self MAC
 * for removing the peer. In case of STA/SAP use bssid passed as part of
 * delete STA parameter.
 *
 * Return: 0 on success, ERROR code on failure
 */
static int wma_remove_bss_peer(tp_wma_handle wma, void *pdev, uint32_t vdev_id,
			       struct del_bss_resp *vdev_stop_resp,
			       uint8_t type)
{
	void *peer;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t *mac_addr = NULL;
	struct wma_target_req *del_req;
	int ret_value = 0;
	QDF_STATUS qdf_status;
	struct qdf_mac_addr bssid;

	if (wma_is_vdev_in_ibss_mode(wma, vdev_id) ||
	    WMA_IS_VDEV_IN_NDI_MODE(wma->interfaces, vdev_id)) {
		mac_addr = cdp_get_vdev_mac_addr(soc, vdev_id);
		if (!mac_addr) {
			WMA_LOGE(FL("mac_addr is NULL for vdev_id = %d"),
				 vdev_id);
			return -EINVAL;
		}
	} else {
		qdf_status = mlme_get_vdev_bss_peer_mac_addr(
				wma->interfaces[vdev_id].vdev,
				&bssid);
		if (QDF_IS_STATUS_ERROR(qdf_status)) {
			WMA_LOGE(FL("Failed to get bssid for vdev_id: %d"),
				 vdev_id);
			return -EINVAL;
		}
		mac_addr = bssid.bytes;
	}

	peer = cdp_peer_find_by_addr(soc, pdev, mac_addr);
	if (!peer) {
		WMA_LOGE(FL("peer NULL for vdev_id = %d"), vdev_id);
		return -EINVAL;
	}

	qdf_status = wma_remove_peer(wma, mac_addr, vdev_id, peer, false);

	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMA_LOGE(FL("wma_remove_peer failed vdev_id:%d"), vdev_id);
		return -EINVAL;
	}

	if (wmi_service_enabled(wma->wmi_handle,
				wmi_service_sync_delete_cmds)) {
		WMA_LOGD(FL("Wait for the peer delete. vdev_id %d"),
			 vdev_id);
		del_req = wma_fill_hold_req(wma, vdev_id,
					    WMA_DELETE_STA_REQ,
					    type,
					    vdev_stop_resp,
					    WMA_DELETE_STA_TIMEOUT);
		if (!del_req) {
			WMA_LOGE(FL("Failed to allocate request. vdev_id %d"),
				 vdev_id);
			vdev_stop_resp->status = QDF_STATUS_E_NOMEM;
			ret_value = -EINVAL;
		}
	}

	return ret_value;
}

#ifdef FEATURE_WLAN_APF
/*
 * get_fw_active_apf_mode() - convert HDD APF mode to FW configurable APF
 * mode
 * @mode: APF mode maintained in HDD
 *
 * Return: FW configurable BP mode
 */
static enum wmi_host_active_apf_mode
get_fw_active_apf_mode(enum active_apf_mode mode)
{
	switch (mode) {
	case ACTIVE_APF_DISABLED:
		return WMI_HOST_ACTIVE_APF_DISABLED;
	case ACTIVE_APF_ENABLED:
		return WMI_HOST_ACTIVE_APF_ENABLED;
	case ACTIVE_APF_ADAPTIVE:
		return WMI_HOST_ACTIVE_APF_ADAPTIVE;
	default:
		WMA_LOGE("Invalid Active APF Mode %d; Using 'disabled'", mode);
		return WMI_HOST_ACTIVE_APF_DISABLED;
	}
}

/**
 * wma_config_active_apf_mode() - Config active APF mode in FW
 * @wma: the WMA handle
 * @vdev_id: the Id of the vdev for which the configuration should be applied
 *
 * Return: QDF status
 */
static QDF_STATUS wma_config_active_apf_mode(t_wma_handle *wma, uint8_t vdev_id)
{
	enum wmi_host_active_apf_mode uc_mode, mcbc_mode;

	uc_mode = get_fw_active_apf_mode(wma->active_uc_apf_mode);
	mcbc_mode = get_fw_active_apf_mode(wma->active_mc_bc_apf_mode);

	WMA_LOGD("Configuring Active APF Mode UC:%d MC/BC:%d for vdev %u",
		 uc_mode, mcbc_mode, vdev_id);

	return wmi_unified_set_active_apf_mode_cmd(wma->wmi_handle, vdev_id,
						   uc_mode, mcbc_mode);
}
#else /* FEATURE_WLAN_APF */
static QDF_STATUS wma_config_active_apf_mode(t_wma_handle *wma, uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_APF */

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/**
 * wma_check_and_find_mcc_ap() - finds if device is operating AP
 * in MCC mode or not
 * @wma: wma handle.
 * @vdev_id: vdev ID of device for which MCC has to be checked
 *
 * This function internally calls wma_find_mcc_ap finds if
 * device is operating AP in MCC mode or not
 *
 * Return: none
 */
static void
wma_check_and_find_mcc_ap(tp_wma_handle wma, uint8_t vdev_id)
{
	struct mac_context *mac_ctx = cds_get_context(QDF_MODULE_ID_PE);

	if (!mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		return;
	}
	if (mac_ctx->sap.sap_channel_avoidance)
		wma_find_mcc_ap(wma, vdev_id, false);
}
#else
static inline void
wma_check_and_find_mcc_ap(tp_wma_handle wma, uint8_t vdev_id)
{}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

void wma_send_del_bss_response(tp_wma_handle wma, struct del_bss_resp *resp)
{
	struct wma_txrx_node *iface;
	struct beacon_info *bcn;
	uint8_t vdev_id;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!resp) {
		WMA_LOGE("%s req is NULL", __func__);
		return;
	}

	vdev_id = resp->vdev_id;
	iface = &wma->interfaces[vdev_id];

	if (!iface->vdev) {
		WMA_LOGE("%s vdev id %d iface->vdev is NULL",
			 __func__, vdev_id);
		if (resp)
			qdf_mem_free(resp);
		return;
	}

	cdp_fc_vdev_flush(soc, vdev_id);
	WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for VDEV_STOP rsp",
		 __func__, vdev_id);
	cdp_fc_vdev_unpause(soc, vdev_id, OL_TXQ_PAUSE_REASON_VDEV_STOP, 0);
	wma_vdev_clear_pause_bit(vdev_id, PAUSE_TYPE_HOST);
	qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
	WMA_LOGD("%s: (type %d subtype %d) BSS is stopped",
		 __func__, iface->type, iface->sub_type);

	bcn = wma->interfaces[vdev_id].beacon;
	if (bcn) {
		WMA_LOGD("%s: Freeing beacon struct %pK, template memory %pK",
			 __func__, bcn, bcn->buf);
		if (bcn->dma_mapped)
			qdf_nbuf_unmap_single(wma->qdf_dev, bcn->buf,
					  QDF_DMA_TO_DEVICE);
		qdf_nbuf_free(bcn->buf);
		qdf_mem_free(bcn);
		wma->interfaces[vdev_id].beacon = NULL;
	}

	/* Timeout status means its WMA generated DEL BSS REQ when ADD
	 * BSS REQ was timed out to stop the VDEV in this case no need
	 * to send response to UMAC
	 */
	if (resp->status == QDF_STATUS_FW_MSG_TIMEDOUT) {
		qdf_mem_free(resp);
		WMA_LOGE("%s: DEL BSS from ADD BSS timeout do not send resp to UMAC (vdev id %x)",
			 __func__, vdev_id);
	} else {
		resp->status = QDF_STATUS_SUCCESS;
		wma_send_msg_high_priority(wma, WMA_DELETE_BSS_RSP,
					   (void *)resp, 0);
	}

	if (iface->del_staself_req && iface->is_del_sta_defered) {
		iface->is_del_sta_defered = false;
		WMA_LOGA("scheduling defered deletion (vdev id %x)",
			 vdev_id);
		wma_vdev_detach(wma, iface->del_staself_req);
	}
}

void wma_send_vdev_down(tp_wma_handle wma, struct del_bss_resp *resp)
{
	uint8_t vdev_id;
	struct wma_txrx_node *iface = &wma->interfaces[resp->vdev_id];
	uint32_t vdev_stop_type;
	QDF_STATUS status;

	if (!resp) {
		WMA_LOGE("%s req is NULL", __func__);
		return;
	}

	vdev_id = resp->vdev_id;
	status = mlme_get_vdev_stop_type(iface->vdev, &vdev_stop_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s : Failed to get vdev stop type", __func__);
		qdf_mem_free(resp);
		return;
	}

	if (vdev_stop_type != WMA_DELETE_BSS_HO_FAIL_REQ) {
		if (wma_send_vdev_down_to_fw(wma, vdev_id) !=
		    QDF_STATUS_SUCCESS)
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
				 vdev_id);
		else
			wma_check_and_find_mcc_ap(wma, vdev_id);
	}
	wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
				      WLAN_VDEV_SM_EV_DOWN_COMPLETE,
				      sizeof(*resp), resp);
}

/**
 * wma_send_vdev_down_req() - handle vdev down req
 * @wma: wma handle
 * @resp: pointer to vde del bss response
 *
 * Return: none
 */
static void wma_send_vdev_down_req(tp_wma_handle wma,
				   struct del_bss_resp *resp)
{
	struct wma_txrx_node *iface = &wma->interfaces[resp->vdev_id];

	wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
				      WLAN_VDEV_SM_EV_MLME_DOWN_REQ,
				      sizeof(*resp), resp);
}

#ifdef WLAN_FEATURE_11W
static void wma_clear_iface_key(struct wma_txrx_node *iface)
{
	qdf_mem_zero(&iface->key, sizeof(iface->key));
}
#else
static void wma_clear_iface_key(struct wma_txrx_node *iface)
{
}
#endif

QDF_STATUS
__wma_handle_vdev_stop_rsp(struct vdev_stop_response *resp_event)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct cdp_pdev *pdev;
	void *peer = NULL;
	struct wma_txrx_node *iface;
	int status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr bssid;
	uint32_t vdev_stop_type;
	struct del_bss_resp *vdev_stop_resp;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!wma) {
		WMA_LOGE("%s: wma is null", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* Ignore stop_response in Monitor mode */
	if (cds_get_conparam() == QDF_GLOBAL_MONITOR_MODE)
		return  QDF_STATUS_SUCCESS;

	iface = &wma->interfaces[resp_event->vdev_id];

	/* vdev in stopped state, no more waiting for key */
	iface->is_waiting_for_key = false;

	/*
	 * Reset the rmfEnabled as there might be MGMT action frames
	 * sent on this vdev before the next session is established.
	 */
	if (iface->rmfEnabled) {
		iface->rmfEnabled = 0;
		WMA_LOGD(FL("Reset rmfEnabled for vdev %d"),
			 resp_event->vdev_id);
	}

	status = mlme_get_vdev_bss_peer_mac_addr(iface->vdev, &bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to get bssid", __func__);
		return QDF_STATUS_E_INVAL;
	}

	/* Clear key information */
	wma_clear_iface_key(iface);
	status = mlme_get_vdev_stop_type(iface->vdev, &vdev_stop_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to get wma req msg type for vdev id %d",
			 __func__, resp_event->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	vdev_stop_resp = qdf_mem_malloc(sizeof(*vdev_stop_resp));
	if (!vdev_stop_resp) {
		WMA_LOGE("%s: Failed to alloc vdev_stop_resp for vdev id %d",
			 __func__, resp_event->vdev_id);
		return QDF_STATUS_E_NOMEM;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is NULL", __func__);
		status = -EINVAL;
		goto free_params;
	}

	if (vdev_stop_type == WMA_DELETE_BSS_HO_FAIL_REQ) {
		peer = cdp_peer_find_by_addr(soc, pdev, bssid.bytes);
		if (!peer) {
			WMA_LOGE("%s: Failed to find peer %pM", __func__,
				 bssid.bytes);
			status = QDF_STATUS_E_FAILURE;
			goto free_params;
		}

		if (!iface->peer_count) {
			WMA_LOGE("%s: Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
				 __func__, bssid.bytes, resp_event->vdev_id,
				 iface->peer_count);
			goto free_params;
		}

		if (peer) {
			WMA_LOGD("%s: vdev %pK is peer_addr %pM to vdev_id %d, peer_count - %d",
				 __func__, peer, bssid.bytes,
				 resp_event->vdev_id, iface->peer_count);
			if (cdp_cfg_get_peer_unmap_conf_support(soc))
				cdp_peer_delete_sync(soc, resp_event->vdev_id,
						     bssid.bytes,
						     wma_peer_unmap_conf_cb,
						     1 << CDP_PEER_DELETE_NO_SPECIAL);
			else
				cdp_peer_delete(soc, resp_event->vdev_id,
						bssid.bytes,
						1 << CDP_PEER_DELETE_NO_SPECIAL);
			wma_remove_objmgr_peer(wma, resp_event->vdev_id,
					       bssid.bytes);
		}
		iface->peer_count--;

		WMA_LOGI("%s: Removed peer %pK with peer_addr %pM vdevid %d peer_count %d",
			 __func__, peer, bssid.bytes,  resp_event->vdev_id,
			 iface->peer_count);
		vdev_stop_resp->status = status;
		vdev_stop_resp->vdev_id = resp_event->vdev_id;
		wma_send_vdev_down_req(wma, vdev_stop_resp);
	} else if (vdev_stop_type == WMA_DELETE_BSS_REQ ||
	    vdev_stop_type == WMA_SET_LINK_STATE) {
		uint8_t type;

		if (!wlan_vdev_get_dp_handle(iface->vdev)) {
			WMA_LOGE("%s: Failed to get dp handle for vdev id %d",
				 __func__, resp_event->vdev_id);
			status = -EINVAL;
			goto free_params;
		}

		/* CCA is required only for sta interface */
		if (iface->type == WMI_VDEV_TYPE_STA)
			wma_get_cca_stats(wma, resp_event->vdev_id);
		if (vdev_stop_type == WMA_DELETE_BSS_REQ)
			type = WMA_DELETE_PEER_RSP;
		else
			type = WMA_SET_LINK_PEER_RSP;

		vdev_stop_resp->vdev_id = resp_event->vdev_id;
		vdev_stop_resp->status = status;
		status = wma_remove_bss_peer(wma, pdev, resp_event->vdev_id,
					     vdev_stop_resp, type);
		if (status) {
			WMA_LOGE("%s Del bss failed vdev:%d", __func__,
				 resp_event->vdev_id);
			wma_send_vdev_down_req(wma, vdev_stop_resp);
			return status;
		}

		if (wmi_service_enabled(wma->wmi_handle,
					wmi_service_sync_delete_cmds))
			return status;

		wma_send_vdev_down_req(wma, vdev_stop_resp);
	}

	return status;

free_params:
	qdf_mem_free(vdev_stop_resp);
	return status;
}

/**
 * wma_handle_vdev_stop_rsp() - handle vdev stop resp
 * @wma: wma handle
 * @resp_event: fw resp
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wma_handle_vdev_stop_rsp(tp_wma_handle wma,
			 struct vdev_stop_response *resp_event)
{
	struct wma_txrx_node *iface;

	iface = &wma->interfaces[resp_event->vdev_id];
	return wlan_vdev_mlme_sm_deliver_evt(iface->vdev,
					     WLAN_VDEV_SM_EV_STOP_RESP,
					     sizeof(*resp_event), resp_event);
}

QDF_STATUS wma_vdev_stop_resp_handler(struct vdev_mlme_obj *vdev_mlme,
				      struct vdev_stop_response *rsp)
{
	tp_wma_handle wma;
	struct wma_txrx_node *iface = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		wma_err("wma handle is NULL for VDEV_%d", rsp->vdev_id);
		return status;
	}

	iface = &wma->interfaces[vdev_mlme->vdev->vdev_objmgr.vdev_id];

	if (rsp->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %d from FW",
				__func__, rsp->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	status = wma_handle_vdev_stop_rsp(wma, rsp);

	return status;
}

void wma_cleanup_vdev(struct wlan_objmgr_vdev *vdev)
{
	tp_wma_handle wma_handle;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	struct vdev_mlme_obj *vdev_mlme;

	if (!soc) {
		wma_err("SOC handle is NULL");
		return;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		wma_err("WMA context is invalid");
		return;
	}

	if (!wma_handle->interfaces[vdev_id].vdev) {
		wma_err("vdev is NULL");
		return;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		wma_err("Failed to get vdev mlme obj for vdev id %d", vdev_id);
		return;
	}

	wma_cdp_vdev_detach(soc, wma_handle, vdev_id);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_WMA_ID);
	wma_handle->interfaces[vdev_id].vdev = NULL;
	wma_handle->interfaces[vdev_id].vdev_active = false;
}

QDF_STATUS wma_vdev_self_peer_create(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_peer *obj_peer;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_vdev *vdev = vdev_mlme->vdev;
	struct cdp_pdev *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	tp_wma_handle wma_handle;

	if (!txrx_pdev) {
		wma_err("TXRX PDEV is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		wma_err("WMA context is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	if (wma_vdev_uses_self_peer(vdev_mlme->mgmt.generic.type,
				    vdev_mlme->mgmt.generic.subtype)) {
		status = wma_create_peer(wma_handle, txrx_pdev,
					 wlan_vdev_get_dp_handle(vdev),
					 vdev->vdev_mlme.macaddr,
					 WMI_PEER_TYPE_DEFAULT,
					 wlan_vdev_get_id(vdev), false);
		if (QDF_IS_STATUS_ERROR(status))
			wma_err("Failed to create peer %d", status);
	} else if (vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_STA) {
		obj_peer = wma_create_objmgr_peer(wma_handle,
						  wlan_vdev_get_id(vdev),
						  vdev->vdev_mlme.macaddr,
						  WMI_PEER_TYPE_DEFAULT);
		if (!obj_peer) {
			wma_err("Failed to create obj mgr peer for self");
			status = QDF_STATUS_E_INVAL;
		}
	}

	return status;
}

#define DOT11AX_HEMU_MODE 0x30
#define HE_SUBFEE 0
#define HE_SUBFER 1
#define HE_MUBFEE 2
#define HE_MUBFER 3

#ifdef WLAN_FEATURE_11AX
static inline uint32_t wma_get_txbf_cap(struct mac_context *mac)
{
	return
	(mac->mlme_cfg->he_caps.dot11_he_cap.su_beamformer << HE_SUBFER) |
	(mac->mlme_cfg->he_caps.dot11_he_cap.su_beamformee << HE_SUBFEE) |
	(1 << HE_MUBFEE) |
	(mac->mlme_cfg->he_caps.dot11_he_cap.mu_beamformer << HE_MUBFER);
}
#else
static inline uint32_t wma_get_txbf_cap(struct mac_context *mac)
{
	return 0;
}
#endif

QDF_STATUS wma_post_vdev_create_setup(struct wlan_objmgr_vdev *vdev)
{
	struct cdp_vdev *txrx_vdev_handle = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t cfg_val;
	uint8_t mcc_adapt_sch;
	QDF_STATUS ret;
	struct mlme_ht_capabilities_info *ht_cap_info;
	u_int8_t vdev_id;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_mlme_qos *qos_aggr;
	struct vdev_mlme_obj *vdev_mlme;
	tp_wma_handle wma_handle;
	uint8_t amsdu_val;
	uint32_t hemu_mode;

	if (!mac) {
		WMA_LOGE("%s: Failed to get mac", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	wma_handle = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma_handle) {
		WMA_LOGE("%s: WMA context is invalid", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!soc) {
		WMA_LOGE("%s: SOC context is invalid", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_vdev_try_get_ref(vdev, WLAN_LEGACY_WMA_ID) !=
		QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	vdev_id = wlan_vdev_get_id(vdev);
	wma_handle->interfaces[vdev_id].vdev = vdev;
	wma_handle->interfaces[vdev_id].vdev_active = true;
	txrx_vdev_handle = wlan_vdev_get_dp_handle(vdev);
	if (!txrx_vdev_handle) {
		WMA_LOGE("%s: Failed to get dp handle for vdev id %d",
			 __func__, vdev_id);
		goto end;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		WMA_LOGE("%s: Failed to get vdev mlme obj!", __func__);
		goto end;
	}

	wma_vdev_update_pause_bitmap(vdev_id, 0);

	wma_handle->interfaces[vdev_id].type =
		vdev_mlme->mgmt.generic.type;
	wma_handle->interfaces[vdev_id].sub_type =
		vdev_mlme->mgmt.generic.subtype;

	qos_aggr = &mac->mlme_cfg->qos_mlme_params;
	status = wma_set_tx_rx_aggr_size(vdev_id, qos_aggr->tx_aggregation_size,
					 qos_aggr->rx_aggregation_size,
					 WMI_VDEV_CUSTOM_AGGR_TYPE_AMPDU);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to set aggregation sizes(status = %d)",
			 status);

	status = wlan_mlme_get_max_amsdu_num(wma_handle->psoc, &amsdu_val);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("failed to get amsdu aggr.size %d", status);
	} else {
		status = wma_set_tx_rx_aggr_size(vdev_id, amsdu_val,
						 amsdu_val,
					    WMI_VDEV_CUSTOM_AGGR_TYPE_AMSDU);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("failed to set amsdu aggr.size %d", status);
		}
	}

	if (vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_STA) {
		status = wma_set_tx_rx_aggr_size_per_ac(
					wma_handle, vdev_id,
					qos_aggr,
					WMI_VDEV_CUSTOM_AGGR_TYPE_AMPDU);

		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("failed to set aggr size per ac(status = %d)",
				 status);

		wma_set_sta_keep_alive(
				wma_handle, vdev_id,
				SIR_KEEP_ALIVE_NULL_PKT,
				mac->mlme_cfg->sta.sta_keep_alive_period,
				NULL, NULL, NULL);

		/* offload STA SA query related params to fwr */
		if (wmi_service_enabled(wma_handle->wmi_handle,
					wmi_service_sta_pmf_offload)) {
			wma_set_sta_sa_query_param(wma_handle, vdev_id);
		}

		status = wma_set_sw_retry_threshold(
					vdev_id,
					qos_aggr->tx_aggr_sw_retry_threshold,
					WMI_PDEV_PARAM_AGG_SW_RETRY_TH);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("failed to set sw retry threshold (status = %d)",
				 status);

		status = wma_set_sw_retry_threshold(
				vdev_id,
				qos_aggr->tx_non_aggr_sw_retry_threshold,
				WMI_PDEV_PARAM_AGG_SW_RETRY_TH);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("failed to set sw retry threshold tx non aggr(status = %d)",
				 status);

		status = wma_set_sw_retry_threshold_per_ac(wma_handle, vdev_id,
							   qos_aggr);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("failed to set sw retry threshold per ac(status = %d)",
				 status);
	}

	WMA_LOGD("Setting WMI_VDEV_PARAM_DISCONNECT_TH: %d",
		 mac->mlme_cfg->gen.dropped_pkt_disconnect_thresh);
	status  = wma_vdev_set_param(
			wma_handle->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_DISCONNECT_TH,
			mac->mlme_cfg->gen.dropped_pkt_disconnect_thresh);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to set DISCONNECT_TH(status = %d)", status);

	status = wma_vdev_set_param(
			wma_handle->wmi_handle,
			vdev_id,
			WMI_VDEV_PARAM_MCC_RTSCTS_PROTECTION_ENABLE,
			mac->roam.configParam.mcc_rts_cts_prot_enable);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to set MCC_RTSCTS_PROTECTION_ENABLE(status = %d)",
			 status);

	status = wma_vdev_set_param(
			wma_handle->wmi_handle, vdev_id,
			WMI_VDEV_PARAM_MCC_BROADCAST_PROBE_ENABLE,
			mac->roam.configParam.mcc_bcast_prob_resp_enable);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to set MCC_BROADCAST_PROBE_ENABLE(status = %d)",
			 status);

	if (wlan_mlme_get_rts_threshold(mac->psoc,
					&cfg_val) ==
					QDF_STATUS_SUCCESS) {
		status = wma_vdev_set_param(wma_handle->wmi_handle,
					    vdev_id,
					    WMI_VDEV_PARAM_RTS_THRESHOLD,
					    cfg_val);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("failed to set RTS_THRESHOLD(status = %d)",
				 status);
	} else {
		WMA_LOGE("Fail to get val for rts threshold, leave unchanged");
	}

	if (wlan_mlme_get_frag_threshold(mac->psoc,
					 &cfg_val) ==
					 QDF_STATUS_SUCCESS) {
		wma_vdev_set_param(wma_handle->wmi_handle,
				   vdev_id,
				   WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD,
				   cfg_val);
	} else {
		WMA_LOGE("Fail to get val for frag threshold, leave unchanged");
	}

	ht_cap_info = &mac->mlme_cfg->ht_caps.ht_cap_info;
	status = wma_vdev_set_param(wma_handle->wmi_handle,
				    vdev_id,
				    WMI_VDEV_PARAM_TX_STBC,
				    ht_cap_info->tx_stbc);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("failed to set TX_STBC(status = %d)", status);

	wma_set_vdev_mgmt_rate(wma_handle, vdev_id);
	if (IS_FEATURE_SUPPORTED_BY_FW(DOT11AX)) {
		hemu_mode = DOT11AX_HEMU_MODE;
		hemu_mode |= wma_get_txbf_cap(mac);
		/*
		 * Enable / disable trigger access for a AP vdev's peers.
		 * For a STA mode vdev this will enable/disable triggered
		 * access and enable/disable Multi User mode of operation.
		 * A value of 0 in a given bit disables corresponding mode.
		 * bit | hemu mode
		 * ---------------
		 *  0  | HE SUBFEE
		 *  1  | HE SUBFER
		 *  2  | HE MUBFEE
		 *  3  | HE MUBFER
		 *  4  | DL OFDMA, for AP its DL Tx OFDMA for Sta its Rx OFDMA
		 *  5  | UL OFDMA, for AP its Tx OFDMA trigger for Sta its
		 *                 Rx OFDMA trigger receive & UL response
		 *  6  | UL MUMIMO
		 */
		status = wma_vdev_set_param(wma_handle->wmi_handle,
					    vdev_id,
					    WMI_VDEV_PARAM_SET_HEMU_MODE,
					    hemu_mode);
		WMA_LOGD("set HEMU_MODE (hemu_mode = 0x%x)", hemu_mode);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("failed to set HEMU_MODE(status = %d)",
				 status);
	}

	/* Initialize roaming offload state */
	if (vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_STA &&
	    vdev_mlme->mgmt.generic.subtype == 0) {
		/* Pass down enable/disable bcast probe rsp to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				vdev_id,
				WMI_VDEV_PARAM_ENABLE_BCAST_PROBE_RESPONSE,
				mac->mlme_cfg->oce.enable_bcast_probe_rsp);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_ENABLE_BCAST_PROBE_RESPONSE");

		/* Pass down the FILS max channel guard time to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				vdev_id,
				WMI_VDEV_PARAM_FILS_MAX_CHANNEL_GUARD_TIME,
				mac->mlme_cfg->sta.fils_max_chan_guard_time);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_FILS_MAX_CHANNEL_GUARD_TIME");

		/* Pass down the Probe Request tx delay(in ms) to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				vdev_id,
				WMI_VDEV_PARAM_PROBE_DELAY,
				PROBE_REQ_TX_DELAY);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_PROBE_DELAY");

		/* Pass down the probe request tx time gap(in ms) to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				vdev_id,
				WMI_VDEV_PARAM_REPEAT_PROBE_TIME,
				PROBE_REQ_TX_TIME_GAP);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_REPEAT_PROBE_TIME");
	}

	if ((vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_STA ||
	     vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_AP) &&
	     vdev_mlme->mgmt.generic.subtype == 0) {
		wma_vdev_set_param(wma_handle->wmi_handle,
				   vdev_id,
				   WMI_VDEV_PARAM_ENABLE_DISABLE_OCE_FEATURES,
				   mac->mlme_cfg->oce.feature_bitmap);
	}

	/* Initialize BMISS parameters */
	if (vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_STA &&
	    vdev_mlme->mgmt.generic.subtype == 0)
		wma_roam_scan_bmiss_cnt(wma_handle,
		mac->mlme_cfg->lfr.roam_bmiss_first_bcnt,
		mac->mlme_cfg->lfr.roam_bmiss_final_bcnt,
		vdev_id);

	if (policy_mgr_get_mcc_adaptive_sch(mac->psoc,
					    &mcc_adapt_sch) ==
	    QDF_STATUS_SUCCESS) {
		WMA_LOGD("%s: setting ini value for WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED: %d",
			 __func__, mcc_adapt_sch);
		ret =
		wma_set_enable_disable_mcc_adaptive_scheduler(mcc_adapt_sch);
		if (QDF_IS_STATUS_ERROR(ret)) {
			WMA_LOGE("Failed to set WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED");
		}
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED, leaving unchanged");
	}

	if (vdev_mlme->mgmt.generic.type == WMI_VDEV_TYPE_STA &&
	    ucfg_pmo_is_apf_enabled(wma_handle->psoc)) {
		ret = wma_config_active_apf_mode(wma_handle,
						 vdev_id);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to configure active APF mode");
	}

	cdp_data_tx_cb_set(soc, vdev_id,
			   wma_data_tx_ack_comp_hdlr,
			   wma_handle);

	return QDF_STATUS_SUCCESS;

end:
	wma_cleanup_vdev(vdev);
	return QDF_STATUS_E_FAILURE;
}

enum mlme_bcn_tx_rate_code wma_get_bcn_rate_code(uint16_t rate)
{
	/* rate in multiples of 100 Kbps */
	switch (rate) {
	case WMA_BEACON_TX_RATE_1_M:
		return MLME_BCN_TX_RATE_CODE_1_M;
	case WMA_BEACON_TX_RATE_2_M:
		return MLME_BCN_TX_RATE_CODE_2_M;
	case WMA_BEACON_TX_RATE_5_5_M:
		return MLME_BCN_TX_RATE_CODE_5_5_M;
	case WMA_BEACON_TX_RATE_11_M:
		return MLME_BCN_TX_RATE_CODE_11M;
	case WMA_BEACON_TX_RATE_6_M:
		return MLME_BCN_TX_RATE_CODE_6_M;
	case WMA_BEACON_TX_RATE_9_M:
		return MLME_BCN_TX_RATE_CODE_9_M;
	case WMA_BEACON_TX_RATE_12_M:
		return MLME_BCN_TX_RATE_CODE_12_M;
	case WMA_BEACON_TX_RATE_18_M:
		return MLME_BCN_TX_RATE_CODE_18_M;
	case WMA_BEACON_TX_RATE_24_M:
		return MLME_BCN_TX_RATE_CODE_24_M;
	case WMA_BEACON_TX_RATE_36_M:
		return MLME_BCN_TX_RATE_CODE_36_M;
	case WMA_BEACON_TX_RATE_48_M:
		return MLME_BCN_TX_RATE_CODE_48_M;
	case WMA_BEACON_TX_RATE_54_M:
		return MLME_BCN_TX_RATE_CODE_54_M;
	default:
		return MLME_BCN_TX_RATE_CODE_1_M;
	}
}

QDF_STATUS wma_vdev_pre_start(uint8_t vdev_id, bool restart)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wma_txrx_node *intr = wma->interfaces;
	struct mac_context *mac_ctx =  cds_get_context(QDF_MODULE_ID_PE);
	struct wma_txrx_node *iface = &wma->interfaces[vdev_id];
	struct wlan_mlme_nss_chains *ini_cfg;
	struct vdev_mlme_obj *mlme_obj;
	struct wlan_objmgr_vdev *vdev = intr[vdev_id].vdev;
	struct wlan_channel *des_chan;

	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!mlme_obj) {
		wma_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	des_chan = vdev->vdev_mlme.des_chan;

	ini_cfg = mlme_get_ini_vdev_config(iface->vdev);
	if (!ini_cfg) {
		wma_err("nss chain ini config NULL");
		return QDF_STATUS_E_FAILURE;
	}

	intr[vdev_id].config.gtx_info.gtxRTMask[0] =
		CFG_TGT_DEFAULT_GTX_HT_MASK;
	intr[vdev_id].config.gtx_info.gtxRTMask[1] =
		CFG_TGT_DEFAULT_GTX_VHT_MASK;

	intr[vdev_id].config.gtx_info.gtxUsrcfg =
		mac_ctx->mlme_cfg->sta.tgt_gtx_usr_cfg;

	intr[vdev_id].config.gtx_info.gtxPERThreshold =
		CFG_TGT_DEFAULT_GTX_PER_THRESHOLD;
	intr[vdev_id].config.gtx_info.gtxPERMargin =
		CFG_TGT_DEFAULT_GTX_PER_MARGIN;
	intr[vdev_id].config.gtx_info.gtxTPCstep =
		CFG_TGT_DEFAULT_GTX_TPC_STEP;
	intr[vdev_id].config.gtx_info.gtxTPCMin =
		CFG_TGT_DEFAULT_GTX_TPC_MIN;
	intr[vdev_id].config.gtx_info.gtxBWMask =
		CFG_TGT_DEFAULT_GTX_BW_MASK;
	intr[vdev_id].mhz = des_chan->ch_freq;
	intr[vdev_id].chan_width = des_chan->ch_width;
	intr[vdev_id].ch_freq = des_chan->ch_freq;
	intr[vdev_id].ch_flagext = des_chan->ch_flagext;

	/*
	 * If the channel has DFS set, flip on radar reporting.
	 *
	 * It may be that this should only be done for IBSS/hostap operation
	 * as this flag may be interpreted (at some point in the future)
	 * by the firmware as "oh, and please do radar DETECTION."
	 *
	 * If that is ever the case we would insert the decision whether to
	 * enable the firmware flag here.
	 */
	if (QDF_GLOBAL_MONITOR_MODE != cds_get_conparam() &&
	    utils_is_dfs_ch(wma->pdev, des_chan->ch_ieee))
		mlme_obj->mgmt.generic.disable_hw_ack = true;

	if (mlme_obj->mgmt.rate_info.bcn_tx_rate) {
		wma_debug("beacon tx rate [%u * 100 Kbps]",
			  mlme_obj->mgmt.rate_info.bcn_tx_rate);
		/*
		 * beacon_tx_rate is in multiples of 100 Kbps.
		 * Convert the data rate to hw rate code.
		 */
		mlme_obj->mgmt.rate_info.bcn_tx_rate =
		wma_get_bcn_rate_code(mlme_obj->mgmt.rate_info.bcn_tx_rate);
	}

	if (!restart) {
		WMA_LOGD("%s, vdev_id: %d, unpausing tx_ll_queue at VDEV_START",
			 __func__, vdev_id);

		cdp_fc_vdev_unpause(cds_get_context(QDF_MODULE_ID_SOC),
				    vdev_id, 0xffffffff, 0);
		wma_vdev_update_pause_bitmap(vdev_id, 0);
	}

	/* Send the dynamic nss chain params before vdev start to fw */
	if (wma->dynamic_nss_chains_support)
		wma_vdev_nss_chain_params_send(vdev_id, ini_cfg);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_peer_assoc_conf_handler() - peer assoc conf handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_peer_assoc_conf_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_PEER_ASSOC_CONF_EVENTID_param_tlvs *param_buf;
	wmi_peer_assoc_conf_event_fixed_param *event;
	struct wma_target_req *req_msg;
	uint8_t macaddr[QDF_MAC_ADDR_SIZE];
	int status = 0;

	WMA_LOGD(FL("Enter"));
	param_buf = (WMI_PEER_ASSOC_CONF_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid peer assoc conf event buffer");
		return -EINVAL;
	}

	event = param_buf->fixed_param;
	if (!event) {
		WMA_LOGE("Invalid peer assoc conf event buffer");
		return -EINVAL;
	}

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->peer_macaddr, macaddr);
	WMA_LOGD(FL("peer assoc conf for vdev:%d mac=%pM"),
		 event->vdev_id, macaddr);

	req_msg = wma_find_req(wma, event->vdev_id,
				    WMA_PEER_ASSOC_CNF_START);

	if (!req_msg) {
		WMA_LOGE(FL("Failed to lookup request message for vdev %d"),
			 event->vdev_id);
		return -EINVAL;
	}

	qdf_mc_timer_stop(&req_msg->event_timeout);

	if (req_msg->msg_type == WMA_ADD_STA_REQ) {
		tpAddStaParams params = (tpAddStaParams)req_msg->user_data;

		if (!params) {
			WMA_LOGE(FL("add STA params is NULL for vdev %d"),
				 event->vdev_id);
			status = -EINVAL;
			goto free_req_msg;
		}

		/* peer assoc conf event means the cmd succeeds */
		params->status = QDF_STATUS_SUCCESS;
		WMA_LOGD(FL("Send ADD_STA_RSP: statype %d vdev_id %d aid %d bssid %pM status %d"),
			 params->staType, params->smesessionId,
			 params->assocId, params->bssId,
			 params->status);
		wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP,
					   (void *)params, 0);
	} else if (req_msg->msg_type == WMA_ADD_BSS_REQ) {
		WMA_LOGD(FL("Send ADD BSS RSP: vdev_id %d status %d"),
			 event->vdev_id, QDF_STATUS_SUCCESS);
		wma_send_add_bss_resp(wma, event->vdev_id, QDF_STATUS_SUCCESS);
	} else {
		WMA_LOGE(FL("Unhandled request message type: %d"),
		req_msg->msg_type);
	}

free_req_msg:
	qdf_mc_timer_destroy(&req_msg->event_timeout);
	qdf_mem_free(req_msg);

	return status;
}

/**
 * wma_peer_delete_handler() - peer delete response handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_peer_delete_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_PEER_DELETE_RESP_EVENTID_param_tlvs *param_buf;
	wmi_peer_delete_cmd_fixed_param *event;
	struct wma_target_req *req_msg;
	tDeleteStaParams *del_sta;
	uint8_t macaddr[QDF_MAC_ADDR_SIZE];
	int status = 0;

	param_buf = (WMI_PEER_DELETE_RESP_EVENTID_param_tlvs *)cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid vdev delete event buffer");
		return -EINVAL;
	}

	event = (wmi_peer_delete_cmd_fixed_param *)param_buf->fixed_param;
	if (!event) {
		WMA_LOGE("Invalid vdev delete event buffer");
		return -EINVAL;
	}

	WMI_MAC_ADDR_TO_CHAR_ARRAY(&event->peer_macaddr, macaddr);
	WMA_LOGD(FL("Peer Delete Response, vdev %d Peer %pM"),
			event->vdev_id, macaddr);
	wlan_roam_debug_log(event->vdev_id, DEBUG_PEER_DELETE_RESP,
			    DEBUG_INVALID_PEER_ID, macaddr, NULL, 0, 0);
	req_msg = wma_find_remove_req_msgtype(wma, event->vdev_id,
					WMA_DELETE_STA_REQ);
	if (!req_msg) {
		WMA_LOGD("Peer Delete response is not handled");
		return -EINVAL;
	}

	wma_release_wakelock(&wma->wmi_cmd_rsp_wake_lock);

	/* Cleanup timeout handler */
	qdf_mc_timer_stop(&req_msg->event_timeout);
	qdf_mc_timer_destroy(&req_msg->event_timeout);

	if (req_msg->type == WMA_DELETE_STA_RSP_START) {
		del_sta = req_msg->user_data;
		if (del_sta->respReqd) {
			WMA_LOGD(FL("Sending peer del rsp to umac"));
			wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP,
				(void *)del_sta, QDF_STATUS_SUCCESS);
		} else {
			qdf_mem_free(del_sta);
		}
	} else if (req_msg->type == WMA_DEL_P2P_SELF_STA_RSP_START) {
		struct del_sta_self_rsp_params *data;

		data = (struct del_sta_self_rsp_params *)req_msg->user_data;
		WMA_LOGD(FL("Calling vdev detach handler"));
		wma_handle_vdev_detach(wma, data->self_sta_param);
		qdf_mem_free(data);
	} else if (req_msg->type == WMA_SET_LINK_PEER_RSP ||
		   req_msg->type == WMA_DELETE_PEER_RSP) {
		wma_send_vdev_down_req(wma, req_msg->user_data);
	}
	qdf_mem_free(req_msg);

	return status;
}

static void wma_trigger_recovery_assert_on_fw_timeout(uint16_t wma_msg)
{
	WMA_LOGE("%s timed out, triggering recovery",
		 mac_trace_get_wma_msg_string(wma_msg));
	cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
}

static inline bool wma_crash_on_fw_timeout(bool crash_enabled)
{
	/* Discard FW timeouts and dont crash during SSR */
	if (cds_is_driver_recovering())
		return false;

	/* Firmware is down send failure response */
	if (cds_is_fw_down())
		return false;

	if (cds_is_driver_unloading())
		return false;

	return crash_enabled;
}

/**
 * wma_hold_req_timer() - wma hold request timeout function
 * @data: target request params
 *
 * Return: none
 */
void wma_hold_req_timer(void *data)
{
	tp_wma_handle wma;
	struct wma_target_req *tgt_req = (struct wma_target_req *)data;
	QDF_STATUS status;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE(FL("Failed to get wma"));
		return;
	}

	status = wma_find_req_on_timer_expiry(wma, tgt_req);

	if (QDF_IS_STATUS_ERROR(status)) {
		/*
		 * if find request failed, then firmware rsp should have
		 * consumed the buffer. Do not free.
		 */
		WMA_LOGD(FL("Failed to lookup request message - %pK"),
			 tgt_req);
		return;
	}
	WMA_LOGA(FL("request %d is timed out for vdev_id - %d"),
		 tgt_req->msg_type, tgt_req->vdev_id);

	if (tgt_req->msg_type == WMA_ADD_STA_REQ) {
		tpAddStaParams params = (tpAddStaParams) tgt_req->user_data;

		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA(FL("WMA_ADD_STA_REQ timed out"));
		WMA_LOGD(FL("Sending add sta rsp to umac (mac:%pM, status:%d)"),
			 params->staMac, params->status);
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
				WMA_ADD_STA_REQ);
		wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP,
					   (void *)params, 0);
	} else if (tgt_req->msg_type == WMA_ADD_BSS_REQ) {

		WMA_LOGA(FL("WMA_ADD_BSS_REQ timed out"));
		WMA_LOGD(FL("Sending add bss rsp to umac (vdev %d, status:%d)"),
			 tgt_req->vdev_id, QDF_STATUS_E_TIMEOUT);

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
				WMA_ADD_BSS_REQ);

		wma_send_add_bss_resp(wma, tgt_req->vdev_id,
				      QDF_STATUS_E_TIMEOUT);
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
		(tgt_req->type == WMA_DELETE_STA_RSP_START)) {
		tpDeleteStaParams params =
				(tpDeleteStaParams) tgt_req->user_data;
		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGE(FL("WMA_DEL_STA_REQ timed out"));
		WMA_LOGE(FL("Sending del sta rsp to umac (mac:%pM, status:%d)"),
			 params->staMac, params->status);

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
				WMA_DELETE_STA_REQ);
		wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP,
					   (void *)params, 0);
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
		(tgt_req->type == WMA_DEL_P2P_SELF_STA_RSP_START)) {
		struct del_sta_self_rsp_params *del_sta;

		del_sta = (struct del_sta_self_rsp_params *)tgt_req->user_data;

		del_sta->self_sta_param->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA(FL("wma delete sta p2p request timed out"));

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
				WMA_DELETE_STA_REQ);
		wma_handle_vdev_detach(wma, del_sta->self_sta_param);
		qdf_mem_free(tgt_req->user_data);
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
		   (tgt_req->type == WMA_SET_LINK_PEER_RSP ||
		    tgt_req->type == WMA_DELETE_PEER_RSP)) {
		struct del_bss_resp *params =
			(struct del_bss_resp *)tgt_req->user_data;

		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGE(FL("wma delete peer for del bss req timed out"));

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
				WMA_DELETE_STA_REQ);
		wma_send_vdev_down_req(wma, params);
	} else if ((tgt_req->msg_type == SIR_HAL_PDEV_SET_HW_MODE) &&
			(tgt_req->type == WMA_PDEV_SET_HW_MODE_RESP)) {
		struct sir_set_hw_mode_resp *params =
			qdf_mem_malloc(sizeof(*params));

		WMA_LOGE(FL("set hw mode req timed out"));

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
						SIR_HAL_PDEV_SET_HW_MODE);
		if (!params) {
			WMA_LOGE(FL("Failed to allocate memory for params"));
			goto timer_destroy;
		}
		params->status = SET_HW_MODE_STATUS_ECANCELED;
		params->cfgd_hw_mode_index = 0;
		params->num_vdev_mac_entries = 0;
		wma_send_msg_high_priority(wma, SIR_HAL_PDEV_SET_HW_MODE_RESP,
					   params, 0);
	} else if ((tgt_req->msg_type == SIR_HAL_PDEV_DUAL_MAC_CFG_REQ) &&
			(tgt_req->type == WMA_PDEV_MAC_CFG_RESP)) {
		struct sir_dual_mac_config_resp *resp =
						qdf_mem_malloc(sizeof(*resp));

		WMA_LOGE(FL("set dual mac config timeout"));
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash))
			wma_trigger_recovery_assert_on_fw_timeout(
						SIR_HAL_PDEV_DUAL_MAC_CFG_REQ);
		if (!resp) {
			WMA_LOGE(FL("Failed to allocate memory for resp"));
			goto timer_destroy;
		}

		resp->status = SET_HW_MODE_STATUS_ECANCELED;
		wma_send_msg_high_priority(wma, SIR_HAL_PDEV_MAC_CFG_RESP,
					   resp, 0);
	} else {
		WMA_LOGE(FL("Unhandled timeout for msg_type:%d and type:%d"),
				tgt_req->msg_type, tgt_req->type);
		QDF_BUG(0);
	}

timer_destroy:
	qdf_mc_timer_destroy(&tgt_req->event_timeout);
	qdf_mem_free(tgt_req);
}

/**
 * wma_fill_hold_req() - fill wma request
 * @wma: wma handle
 * @msg_type: message type
 * @type: request type
 * @params: request params
 * @timeout: timeout value
 *
 * Return: wma_target_req ptr
 */
struct wma_target_req *wma_fill_hold_req(tp_wma_handle wma,
					 uint8_t vdev_id,
					 uint32_t msg_type, uint8_t type,
					 void *params, uint32_t timeout)
{
	struct wma_target_req *req;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return NULL;

	WMA_LOGD(FL("vdev_id %d msg %d type %d"), vdev_id, msg_type, type);
	qdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	req->vdev_id = vdev_id;
	req->msg_type = msg_type;
	req->type = type;
	req->user_data = params;
	status = qdf_list_insert_back(&wma->wma_hold_req_queue, &req->node);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGE(FL("Failed add request in queue"));
		qdf_mem_free(req);
		return NULL;
	}
	qdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
	qdf_mc_timer_init(&req->event_timeout, QDF_TIMER_TYPE_SW,
			  wma_hold_req_timer, req);
	qdf_mc_timer_start(&req->event_timeout, timeout);
	return req;
}

/**
 * wma_remove_req() - remove request
 * @wma: wma handle
 * @vdev_id: vdev id
 * @type: type
 *
 * Return: none
 */
void wma_remove_req(tp_wma_handle wma, uint8_t vdev_id,
		    uint8_t type)
{
	struct wma_target_req *req_msg;

	WMA_LOGD(FL("Remove req for vdev: %d type: %d"), vdev_id, type);
	req_msg = wma_find_req(wma, vdev_id, type);
	if (!req_msg) {
		WMA_LOGE(FL("target req not found for vdev: %d type: %d"),
			 vdev_id, type);
		return;
	}

	qdf_mc_timer_stop(&req_msg->event_timeout);
	qdf_mc_timer_destroy(&req_msg->event_timeout);
	qdf_mem_free(req_msg);
}

/**
 * wma_vdev_set_bss_params() - BSS set params functions
 * @wma: wma handle
 * @vdev_id: vdev id
 * @beaconInterval: beacon interval
 * @dtimPeriod: DTIM period
 * @shortSlotTimeSupported: short slot time
 * @llbCoexist: llbCoexist
 * @maxTxPower: max tx power
 *
 * Return: none
 */
static void
wma_vdev_set_bss_params(tp_wma_handle wma, int vdev_id,
			tSirMacBeaconInterval beaconInterval,
			uint8_t dtimPeriod, uint8_t shortSlotTimeSupported,
			uint8_t llbCoexist, int8_t maxTxPower)
{
	QDF_STATUS ret;
	uint32_t slot_time;
	struct wma_txrx_node *intr = wma->interfaces;

	/* Beacon Interval setting */
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_BEACON_INTERVAL,
					      beaconInterval);

	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("failed to set WMI_VDEV_PARAM_BEACON_INTERVAL");
	ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle, vdev_id,
						&intr[vdev_id].config.gtx_info);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("failed to set WMI_VDEV_PARAM_DTIM_PERIOD");
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_DTIM_PERIOD,
					      dtimPeriod);
	intr[vdev_id].dtimPeriod = dtimPeriod;
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("failed to set WMI_VDEV_PARAM_DTIM_PERIOD");

	if (!maxTxPower)
		WMA_LOGW("Setting Tx power limit to 0");
	WMA_LOGD("Set maxTx pwr [WMI_VDEV_PARAM_TX_PWRLIMIT] to %d",
						maxTxPower);
	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_TX_PWRLIMIT,
					      maxTxPower);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("failed to set WMI_VDEV_PARAM_TX_PWRLIMIT");
	else
		mlme_set_max_reg_power(intr[vdev_id].vdev, maxTxPower);

	/* Slot time */
	if (shortSlotTimeSupported)
		slot_time = WMI_VDEV_SLOT_TIME_SHORT;
	else
		slot_time = WMI_VDEV_SLOT_TIME_LONG;

	ret = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_SLOT_TIME,
					      slot_time);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("failed to set WMI_VDEV_PARAM_SLOT_TIME");

	/* Initialize protection mode in case of coexistence */
	wma_update_protection_mode(wma, vdev_id, llbCoexist);

}

#ifdef WLAN_FEATURE_11W
static void wma_set_mgmt_frame_protection(tp_wma_handle wma)
{
	struct pdev_params param = {0};
	QDF_STATUS ret;

	/*
	 * when 802.11w PMF is enabled for hw encr/decr
	 * use hw MFP Qos bits 0x10
	 */
	param.param_id = WMI_PDEV_PARAM_PMF_QOS;
	param.param_value = true;
	ret = wmi_unified_pdev_param_send(wma->wmi_handle,
					 &param, WMA_WILDCARD_PDEV_ID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		WMA_LOGE("%s: Failed to set QOS MFP/PMF (%d)",
			 __func__, ret);
	} else {
		WMA_LOGD("%s: QOS MFP/PMF set", __func__);
	}
}

/**
 * wma_set_peer_pmf_status() - Get the peer and update PMF capability of it
 * @wma: wma handle
 * @peer_mac: peer mac addr
 * @is_pmf_enabled: Carries the status whether PMF is enabled or not
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wma_set_peer_pmf_status(tp_wma_handle wma, uint8_t *peer_mac,
			bool is_pmf_enabled)
{
	struct wlan_objmgr_peer *peer;

	peer = wlan_objmgr_get_peer(wma->psoc,
				    wlan_objmgr_pdev_get_pdev_id(wma->pdev),
				    peer_mac, WLAN_LEGACY_WMA_ID);
	if (!peer) {
		WMA_LOGE("Peer of peer_mac %pM not found",
			 peer_mac);
		return QDF_STATUS_E_INVAL;
	}
	mlme_set_peer_pmf_status(peer, is_pmf_enabled);
	wlan_objmgr_peer_release_ref(peer, WLAN_LEGACY_WMA_ID);
	WMA_LOGD("set is_pmf_enabled %d for %pM", is_pmf_enabled, peer_mac);

	return QDF_STATUS_SUCCESS;
}
#else
static inline void wma_set_mgmt_frame_protection(tp_wma_handle wma)
{
}

static QDF_STATUS
wma_set_peer_pmf_status(tp_wma_handle wma, uint8_t *peer_mac,
			bool is_pmf_enabled)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_11W */

QDF_STATUS wma_pre_vdev_start_setup(uint8_t vdev_id,
				    struct bss_params *add_bss)
{
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	void *peer;
	QDF_STATUS status;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wma_txrx_node *iface;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct vdev_mlme_obj *mlme_obj;
	uint8_t *mac_addr;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		wma_err("Failed to get pdev");
		return QDF_STATUS_E_FAILURE;
	}
	iface = &wma->interfaces[vdev_id];

	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
	if (!mlme_obj) {
		wma_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_vdev_get_dp_handle(iface->vdev);
	if (!vdev) {
		wma_err("Failed to get dp handle fro vdev id %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	wma_set_bss_rate_flags(wma, vdev_id, add_bss);
	if (wlan_vdev_mlme_get_opmode(iface->vdev) == QDF_NDI_MODE ||
	    wlan_vdev_mlme_get_opmode(iface->vdev) == QDF_IBSS_MODE)
		mac_addr = mlme_obj->mgmt.generic.bssid;
	else
		mac_addr = wlan_vdev_mlme_get_macaddr(iface->vdev);

	status = wma_create_peer(wma, pdev, vdev, mac_addr,
				 WMI_PEER_TYPE_DEFAULT, vdev_id, false);
	if (status != QDF_STATUS_SUCCESS) {
		wma_err("Failed to create peer");
		return status;
	}

	peer = cdp_peer_find_by_addr(soc, pdev, mac_addr);
	if (!peer) {
		wma_err("Failed to find peer %pM", mac_addr);
		return QDF_STATUS_E_FAILURE;
	}

	iface->rmfEnabled = add_bss->rmfEnabled;
	if (add_bss->rmfEnabled)
		wma_set_mgmt_frame_protection(wma);

	if (wlan_vdev_mlme_get_opmode(iface->vdev) == QDF_IBSS_MODE) {
		tSetBssKeyParams key_info;
		/* clear leftover ibss keys on bss peer */
		wma_debug("ibss bss key clearing");
		qdf_mem_zero(&key_info, sizeof(key_info));
		key_info.vdev_id = vdev_id;
		key_info.numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
		qdf_mem_copy(&wma->ibsskey_info, &key_info,
			     sizeof(tSetBssKeyParams));
		/*
		 * If IBSS Power Save is supported by firmware
		 * set the IBSS power save params to firmware.
		 */
		if (wmi_service_enabled(wma->wmi_handle,
					wmi_service_ibss_pwrsave)) {
			status = wma_set_ibss_pwrsave_params(wma, vdev_id);
		}
	}
	return status;
}

QDF_STATUS wma_post_vdev_start_setup(uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);
	struct wma_txrx_node *intr = &wma->interfaces[vdev_id];
	struct vdev_mlme_obj *mlme_obj;
	struct wlan_objmgr_vdev *vdev = intr->vdev;
	uint8_t bss_power;

	if (!vdev) {
		wma_err("vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_NDI_MODE ||
	    wlan_vdev_mlme_get_opmode(vdev) == QDF_IBSS_MODE) {
		/* Initialize protection mode to no protection */
		wma_vdev_set_param(wma->wmi_handle, vdev_id,
				   WMI_VDEV_PARAM_PROTECTION_MODE,
				   IEEE80211_PROT_NONE);
		return status;
	}
	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!mlme_obj) {
		wma_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	bss_power = wlan_reg_get_channel_reg_power_for_freq(wma->pdev,
							    vdev->vdev_mlme.bss_chan->ch_freq);
	wma_vdev_set_bss_params(wma, vdev_id,
				mlme_obj->proto.generic.beacon_interval,
				mlme_obj->proto.generic.dtim_period,
				mlme_obj->proto.generic.slot_time,
				mlme_obj->proto.generic.protection_mode,
				bss_power);

	wma_vdev_set_he_bss_params(wma, vdev_id,
				   &mlme_obj->proto.he_ops_info);

	return status;
}

static QDF_STATUS wma_update_iface_params(tp_wma_handle wma,
					  struct bss_params *add_bss)
{
	struct wma_txrx_node *iface;
	uint8_t vdev_id;

	vdev_id = add_bss->staContext.smesessionId;
	iface = &wma->interfaces[vdev_id];
	wma_set_bss_rate_flags(wma, vdev_id, add_bss);

	if (iface->addBssStaContext)
		qdf_mem_free(iface->addBssStaContext);
	iface->addBssStaContext = qdf_mem_malloc(sizeof(tAddStaParams));
	if (!iface->addBssStaContext)
		return QDF_STATUS_E_RESOURCES;
	*iface->addBssStaContext = add_bss->staContext;
	if (iface->staKeyParams) {
		qdf_mem_free(iface->staKeyParams);
		iface->staKeyParams = NULL;
	}
	if (add_bss->extSetStaKeyParamValid) {
		iface->staKeyParams =
			qdf_mem_malloc(sizeof(tSetStaKeyParams));
		if (!iface->staKeyParams) {
			qdf_mem_free(iface->addBssStaContext);
			iface->addBssStaContext = NULL;
			return QDF_STATUS_E_RESOURCES;
		}
		*iface->staKeyParams = add_bss->extSetStaKeyParam;
	}
	/* Save parameters later needed by WMA_ADD_STA_REQ */
	iface->rmfEnabled = add_bss->rmfEnabled;
	if (add_bss->rmfEnabled)
		wma_set_peer_pmf_status(wma, add_bss->bssId, true);
	iface->beaconInterval = add_bss->beaconInterval;
	iface->llbCoexist = add_bss->llbCoexist;
	iface->shortSlotTimeSupported = add_bss->shortSlotTimeSupported;
	iface->nwType = add_bss->nwType;

	return QDF_STATUS_SUCCESS;
}

static inline
void *wma_cdp_find_peer_by_addr(uint8_t *peer_addr)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct cdp_pdev *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (! soc || !pdev) {
		WMA_LOGE("%s Failed to get pdev/soc", __func__);
		return NULL;
	}

	return cdp_peer_find_by_addr(soc, pdev, peer_addr);
}

static
QDF_STATUS wma_save_bss_params(tp_wma_handle wma, struct bss_params *add_bss)
{
	QDF_STATUS status;

	wma_vdev_set_he_config(wma, add_bss->staContext.smesessionId, add_bss);
	if (!wma_cdp_find_peer_by_addr(add_bss->bssId))
		status = QDF_STATUS_E_FAILURE;
	else
		status = QDF_STATUS_SUCCESS;
	qdf_mem_copy(add_bss->staContext.staMac, add_bss->bssId,
		     sizeof(add_bss->staContext.staMac));

	WMA_LOGD("%s: update_bss %d nw_type %d bssid %pM status %d",
		 __func__, add_bss->updateBss, add_bss->nwType, add_bss->bssId,
		 status);

	return status;
}

QDF_STATUS wma_pre_assoc_req(struct bss_params *add_bss)
{
	QDF_STATUS status;
	tp_wma_handle wma;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("Invalid pdev or wma");
		return QDF_STATUS_E_INVAL;
	}

	status = wma_update_iface_params(wma, add_bss);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = wma_save_bss_params(wma, add_bss);

	return status;
}

void wma_add_bss_lfr3(tp_wma_handle wma, struct bss_params *add_bss)
{
	QDF_STATUS status;
	void *peer = NULL;

	status = wma_update_iface_params(wma, add_bss);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	peer = wma_cdp_find_peer_by_addr(add_bss->bssId);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 add_bss->bssId);
		return;
	}
	WMA_LOGD("LFR3:%s: bssid %pM", __func__, add_bss->bssId);
}


#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
static
QDF_STATUS wma_set_cdp_vdev_pause_reason(tp_wma_handle wma, uint8_t vdev_id)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	cdp_fc_vdev_pause(soc, vdev_id,
			  OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED, 0);

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS wma_set_cdp_vdev_pause_reason(tp_wma_handle wma, uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}
#endif

void wma_send_add_bss_resp(tp_wma_handle wma, uint8_t vdev_id,
			   QDF_STATUS status)
{
	struct add_bss_rsp *add_bss_rsp;

	add_bss_rsp = qdf_mem_malloc(sizeof(*add_bss_rsp));
	if (!add_bss_rsp)
		return;

	add_bss_rsp->vdev_id = vdev_id;
	add_bss_rsp->status = status;
	lim_handle_add_bss_rsp(wma->mac_context, add_bss_rsp);
}

#ifdef WLAN_FEATURE_HOST_ROAM
QDF_STATUS wma_add_bss_lfr2_vdev_start(struct wlan_objmgr_vdev *vdev,
				       struct bss_params *add_bss)
{
	tp_wma_handle wma;
	void *peer = NULL;
	QDF_STATUS status;
	struct vdev_mlme_obj *mlme_obj;
	uint8_t vdev_id;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma || !vdev) {
		WMA_LOGE("Invalid wma or vdev");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = vdev->vdev_objmgr.vdev_id;
	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!mlme_obj) {
		wma_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = wma_update_iface_params(wma, add_bss);
	if (QDF_IS_STATUS_ERROR(status))
		goto send_fail_resp;

	peer = wma_cdp_find_peer_by_addr(mlme_obj->mgmt.generic.bssid);
	if (!peer) {
		wma_err("Failed to find peer %pM",
			mlme_obj->mgmt.generic.bssid);
		goto send_fail_resp;
	}

	status = wma_vdev_pre_start(vdev_id, false);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err("failed, status: %d", status);
		goto peer_cleanup;
	}
	status = vdev_mgr_start_send(mlme_obj, false);
	if (QDF_IS_STATUS_ERROR(status)) {
		wma_err("failed, status: %d", status);
		goto peer_cleanup;
	}
	status = wma_set_cdp_vdev_pause_reason(wma, vdev_id);
	if (QDF_IS_STATUS_ERROR(status))
		goto peer_cleanup;

	/* ADD_BSS_RESP will be deferred to completion of VDEV_START */
	return QDF_STATUS_SUCCESS;

peer_cleanup:
	if (peer)
		wma_remove_peer(wma, mlme_obj->mgmt.generic.bssid, vdev_id,
				peer, false);

send_fail_resp:
	wma_send_add_bss_resp(wma, vdev_id, QDF_STATUS_E_FAILURE);

	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS wma_send_peer_assoc_req(struct bss_params *add_bss)
{
	struct cdp_pdev *pdev;
	struct wma_target_req *msg;
	tp_wma_handle wma;
	uint8_t vdev_id;
	void *peer = NULL;
	QDF_STATUS status;
	struct wma_txrx_node *iface;
	int pps_val = 0;
	struct vdev_mlme_obj *mlme_obj;
	struct mac_context *mac = cds_get_context(QDF_MODULE_ID_PE);
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("Invalid pdev or wma");
		return QDF_STATUS_E_INVAL;
	}

	vdev_id = add_bss->staContext.smesessionId;
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s Failed to get pdev", __func__);
		status = QDF_STATUS_E_FAILURE;
		goto send_resp;
	}

	iface = &wma->interfaces[vdev_id];
	status = wma_update_iface_params(wma, add_bss);
	if (QDF_IS_STATUS_ERROR(status))
		goto send_resp;

	peer = wma_cdp_find_peer_by_addr(add_bss->bssId);
	if (add_bss->nonRoamReassoc && peer)
		goto send_resp;

	if (!add_bss->updateBss)
		goto send_resp;

	if (!peer) {
		WMA_LOGE("%s: %d Failed to find peer %pM",
			 __func__, __LINE__, add_bss->bssId);
		status = QDF_STATUS_E_FAILURE;
		goto send_resp;
	}


	if (add_bss->staContext.encryptType == eSIR_ED_NONE) {
		WMA_LOGD("%s: Update peer(%pM) state into auth",
			 __func__, add_bss->bssId);
		cdp_peer_state_update(soc, pdev, add_bss->bssId,
				      OL_TXRX_PEER_STATE_AUTH);
	} else {
		WMA_LOGD("%s: Update peer(%pM) state into conn",
			 __func__, add_bss->bssId);
		cdp_peer_state_update(soc, pdev, add_bss->bssId,
				      OL_TXRX_PEER_STATE_CONN);
		status = wma_set_cdp_vdev_pause_reason(wma, vdev_id);
		if (QDF_IS_STATUS_ERROR(status))
			goto peer_cleanup;
	}

	wmi_unified_send_txbf(wma, &add_bss->staContext);

	pps_val = ((mac->mlme_cfg->sta.enable_5g_ebt << 31) &
		 0xffff0000) | (PKT_PWR_SAVE_5G_EBT & 0xffff);
	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
				    WMI_VDEV_PARAM_PACKET_POWERSAVE,
				    pps_val);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("Failed to send wmi packet power save cmd");
	else
		WMA_LOGD("Sent PKT_PWR_SAVE_5G_EBT cmd to target, val = %x, status = %d",
			 pps_val, status);

	add_bss->staContext.no_ptk_4_way = add_bss->no_ptk_4_way;

	status = wma_send_peer_assoc(wma, add_bss->nwType,
				     &add_bss->staContext);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Failed to send peer assoc status:%d", status);
		goto peer_cleanup;
	}

	/* we just had peer assoc, so install key will be done later */
	if (add_bss->staContext.encryptType != eSIR_ED_NONE)
		iface->is_waiting_for_key = true;

	if (add_bss->rmfEnabled)
		wma_set_mgmt_frame_protection(wma);

	wma_vdev_set_bss_params(wma, add_bss->staContext.smesessionId,
				add_bss->beaconInterval,
				add_bss->dtimPeriod,
				add_bss->shortSlotTimeSupported,
				add_bss->llbCoexist,
				add_bss->maxTxPower);

	mlme_obj = wlan_vdev_mlme_get_cmpt_obj(iface->vdev);
	if (!mlme_obj) {
		WMA_LOGE("Failed to mlme obj");
		status = QDF_STATUS_E_FAILURE;
		goto peer_cleanup;
	}
	/*
	 * Store the bssid in interface table, bssid will
	 * be used during group key setting sta mode.
	 */
	qdf_mem_copy(mlme_obj->mgmt.generic.bssid,
		     add_bss->bssId, QDF_MAC_ADDR_SIZE);

	wma_save_bss_params(wma, add_bss);

	if (!wmi_service_enabled(wma->wmi_handle,
				 wmi_service_peer_assoc_conf)) {
		WMA_LOGI(FL("WMI_SERVICE_PEER_ASSOC_CONF not enabled"));
		goto send_resp;
	}

	msg = wma_fill_hold_req(wma, vdev_id, WMA_ADD_BSS_REQ,
				WMA_PEER_ASSOC_CNF_START, NULL,
				WMA_PEER_ASSOC_TIMEOUT);
	if (!msg) {
		WMA_LOGE(FL("Failed to allocate request for vdev_id %d"),
			 vdev_id);
		wma_remove_req(wma, vdev_id, WMA_PEER_ASSOC_CNF_START);
		status = QDF_STATUS_E_FAILURE;
		goto peer_cleanup;
	}

	return QDF_STATUS_SUCCESS;

peer_cleanup:
	if (peer)
		wma_remove_peer(wma, add_bss->bssId, vdev_id, peer, false);
send_resp:
	wma_send_add_bss_resp(wma, vdev_id, status);

	return QDF_STATUS_SUCCESS;
}

/**
 * wma_add_sta_req_ap_mode() - process add sta request in ap mode
 * @wma: wma handle
 * @add_sta: add sta params
 *
 * Return: none
 */
static void wma_add_sta_req_ap_mode(tp_wma_handle wma, tpAddStaParams add_sta)
{
	enum ol_txrx_peer_state state = OL_TXRX_PEER_STATE_CONN;
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	void *peer;
	QDF_STATUS status;
	int32_t ret;
	struct wma_txrx_node *iface = NULL;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint32_t i, j;
	uint16_t mcs_limit;
	uint8_t *rate_pos;
	struct mac_context *mac = wma->mac_context;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}
	/* UMAC sends WMA_ADD_STA_REQ msg twice to WMA when the station
	 * associates. First WMA_ADD_STA_REQ will have staType as
	 * STA_ENTRY_PEER and second posting will have STA_ENTRY_SELF.
	 * Peer creation is done in first WMA_ADD_STA_REQ and second
	 * WMA_ADD_STA_REQ which has STA_ENTRY_SELF is ignored and
	 * send fake response with success to UMAC. Otherwise UMAC
	 * will get blocked.
	 */
	if (add_sta->staType != STA_ENTRY_PEER) {
		add_sta->status = QDF_STATUS_SUCCESS;
		goto send_rsp;
	}

	vdev = wma_find_vdev_by_id(wma, add_sta->smesessionId);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	iface = &wma->interfaces[add_sta->smesessionId];
	peer = cdp_peer_find_by_addr_and_vdev(soc, pdev, vdev,
				add_sta->staMac);
	if (peer) {
		wma_remove_peer(wma, add_sta->staMac, add_sta->smesessionId,
				peer, false);
		WMA_LOGE("%s: Peer already exists, Deleted peer with peer_addr %pM",
			__func__, add_sta->staMac);
	}
	/* The code above only checks the peer existence on its own vdev.
	 * Need to check whether the peer exists on other vDevs because firmware
	 * can't create the peer if the peer with same MAC address already
	 * exists on the pDev. As this peer belongs to other vDevs, just return
	 * here.
	 */
	peer = cdp_peer_find_by_addr(soc, pdev,
			add_sta->staMac);
	if (peer) {
		WMA_LOGE("%s: My vdev:%pK, but Peer exists on other vdev with peer_addr %pM",
			 __func__, vdev, add_sta->staMac);
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	wma_delete_invalid_peer_entries(add_sta->smesessionId, add_sta->staMac);

	status = wma_create_peer(wma, pdev, vdev, add_sta->staMac,
				 WMI_PEER_TYPE_DEFAULT, add_sta->smesessionId,
				 false);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to create peer for %pM",
			 __func__, add_sta->staMac);
		add_sta->status = status;
		goto send_rsp;
	}

	peer = cdp_peer_find_by_addr_and_vdev(soc, pdev,
				vdev,
				add_sta->staMac);
	if (!peer) {
		WMA_LOGE("%s: Failed to find peer handle using peer mac %pM",
			 __func__, add_sta->staMac);
		add_sta->status = QDF_STATUS_E_FAILURE;
		wma_remove_peer(wma, add_sta->staMac, add_sta->smesessionId,
				peer, false);
		goto send_rsp;
	}

	wmi_unified_send_txbf(wma, add_sta);

	/*
	 * Get MCS limit from ini configure, and map it to rate parameters
	 * This will limit HT rate upper bound. CFG_CTRL_MASK is used to
	 * check whether ini config is enabled and CFG_DATA_MASK to get the
	 * MCS value.
	 */
#define CFG_CTRL_MASK              0xFF00
#define CFG_DATA_MASK              0x00FF

	mcs_limit = mac->mlme_cfg->rates.sap_max_mcs_txdata;

	if (mcs_limit & CFG_CTRL_MASK) {
		WMA_LOGD("%s: set mcs_limit %x", __func__, mcs_limit);

		mcs_limit &= CFG_DATA_MASK;
		rate_pos = (u_int8_t *)add_sta->supportedRates.supportedMCSSet;
		for (i = 0, j = 0; i < MAX_SUPPORTED_RATES;) {
			if (j < mcs_limit / 8) {
				rate_pos[j] = 0xff;
				j++;
				i += 8;
			} else if (j < mcs_limit / 8 + 1) {
				if (i <= mcs_limit)
					rate_pos[i / 8] |= 1 << (i % 8);
				else
					rate_pos[i / 8] &= ~(1 << (i % 8));
				i++;

				if (i >= (j + 1) * 8)
					j++;
			} else {
				rate_pos[j++] = 0;
				i += 8;
			}
		}
	}

	if (wmi_service_enabled(wma->wmi_handle,
				    wmi_service_peer_assoc_conf)) {
		peer_assoc_cnf = true;
		msg = wma_fill_hold_req(wma, add_sta->smesessionId,
				   WMA_ADD_STA_REQ, WMA_PEER_ASSOC_CNF_START,
				   add_sta, WMA_PEER_ASSOC_TIMEOUT);
		if (!msg) {
			WMA_LOGE(FL("Failed to alloc request for vdev_id %d"),
				 add_sta->smesessionId);
			add_sta->status = QDF_STATUS_E_FAILURE;
			wma_remove_req(wma, add_sta->smesessionId,
				       WMA_PEER_ASSOC_CNF_START);
			wma_remove_peer(wma, add_sta->staMac,
				add_sta->smesessionId, peer, false);
			peer_assoc_cnf = false;
			goto send_rsp;
		}
	} else {
		WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF not enabled"));
	}

	ret = wma_send_peer_assoc(wma, add_sta->nwType, add_sta);
	if (ret) {
		add_sta->status = QDF_STATUS_E_FAILURE;
		wma_remove_peer(wma, add_sta->staMac, add_sta->smesessionId,
				peer, false);
		goto send_rsp;
	}

	wma_send_peer_atim_window_len(wma, add_sta);
	if (add_sta->rmfEnabled)
		wma_set_peer_pmf_status(wma, add_sta->staMac, true);

	if (add_sta->uAPSD) {
		status = wma_set_ap_peer_uapsd(wma, add_sta->smesessionId,
					    add_sta->staMac,
					    add_sta->uAPSD, add_sta->maxSPLen);
		if (QDF_IS_STATUS_ERROR(status)) {
			WMA_LOGE("Failed to set peer uapsd param for %pM",
				 add_sta->staMac);
			add_sta->status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			goto send_rsp;
		}
	}

	WMA_LOGD("%s: Moving peer %pM to state %d",
		 __func__, add_sta->staMac, state);
	cdp_peer_state_update(soc, pdev, add_sta->staMac, state);

	add_sta->nss    = iface->nss;
	add_sta->status = QDF_STATUS_SUCCESS;
send_rsp:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf) {
		WMA_LOGD(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
		return;
	}

	WMA_LOGD(FL("statype %d vdev_id %d aid %d bssid %pM status %d"),
		 add_sta->staType, add_sta->smesessionId,
		 add_sta->assocId, add_sta->bssId,
		 add_sta->status);
	wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP, (void *)add_sta, 0);
}

#ifdef FEATURE_WLAN_TDLS

/**
 * wma_add_tdls_sta() - process add sta request in TDLS mode
 * @wma: wma handle
 * @add_sta: add sta params
 *
 * Return: none
 */
static void wma_add_tdls_sta(tp_wma_handle wma, tpAddStaParams add_sta)
{
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	void *peer;
	QDF_STATUS status;
	int32_t ret;
	struct tdls_peer_update_state *peer_state;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("%s: staType: %d, updateSta: %d, bssId: %pM, staMac: %pM",
		 __func__, add_sta->staType,
		 add_sta->updateSta, add_sta->bssId, add_sta->staMac);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	vdev = wma_find_vdev_by_id(wma, add_sta->smesessionId);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	if (wma_is_roam_synch_in_progress(wma, add_sta->smesessionId)) {
		WMA_LOGE("%s: roaming in progress, reject add sta!", __func__);
		add_sta->status = QDF_STATUS_E_PERM;
		goto send_rsp;
	}

	if (0 == add_sta->updateSta) {
		/* its a add sta request * */

		cdp_peer_copy_mac_addr_raw(soc, vdev, add_sta->bssId);

		WMA_LOGD("%s: addSta, calling wma_create_peer for %pM, vdev_id %hu",
			__func__, add_sta->staMac, add_sta->smesessionId);

		status = wma_create_peer(wma, pdev, vdev, add_sta->staMac,
					 WMI_PEER_TYPE_TDLS,
					 add_sta->smesessionId, false);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to create peer for %pM",
				 __func__, add_sta->staMac);
			add_sta->status = status;
			goto send_rsp;
		}

		peer = cdp_peer_find_by_addr(soc, pdev,
				add_sta->staMac);
		if (!peer) {
			WMA_LOGE("%s: addSta, failed to find peer handle for mac %pM",
				__func__, add_sta->staMac);
			add_sta->status = QDF_STATUS_E_FAILURE;
			cdp_peer_add_last_real_peer(soc, pdev, vdev);
			goto send_rsp;
		}

		WMA_LOGD("%s: addSta, after calling cdp_local_peer_id, staMac: %pM",
			 __func__, add_sta->staMac);

		peer_state = qdf_mem_malloc(sizeof(*peer_state));
		if (!peer_state) {
			add_sta->status = QDF_STATUS_E_NOMEM;
			goto send_rsp;
		}

		peer_state->peer_state = WMI_TDLS_PEER_STATE_PEERING;
		peer_state->vdev_id = add_sta->smesessionId;
		qdf_mem_copy(&peer_state->peer_macaddr,
			     &add_sta->staMac, sizeof(tSirMacAddr));
		wma_update_tdls_peer_state(wma, peer_state);
	} else {
		/* its a change sta request * */
		peer =
			cdp_peer_find_by_addr(soc, pdev,
				add_sta->staMac);
		if (!peer) {
			WMA_LOGE("%s: changeSta,failed to find peer handle for mac %pM",
				__func__, add_sta->staMac);
			add_sta->status = QDF_STATUS_E_FAILURE;

			cdp_peer_add_last_real_peer(soc, pdev, vdev);

			goto send_rsp;
		}

		if (wmi_service_enabled(wma->wmi_handle,
					    wmi_service_peer_assoc_conf)) {
			WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
			peer_assoc_cnf = true;
			msg = wma_fill_hold_req(wma, add_sta->smesessionId,
				WMA_ADD_STA_REQ, WMA_PEER_ASSOC_CNF_START,
				add_sta, WMA_PEER_ASSOC_TIMEOUT);
			if (!msg) {
				WMA_LOGE(FL("Failed to alloc request for vdev_id %d"),
					 add_sta->smesessionId);
				add_sta->status = QDF_STATUS_E_FAILURE;
				wma_remove_req(wma, add_sta->smesessionId,
					       WMA_PEER_ASSOC_CNF_START);
				wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
				peer_assoc_cnf = false;
				goto send_rsp;
			}
		} else {
			WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF not enabled"));
		}

		WMA_LOGD("%s: changeSta, calling wma_send_peer_assoc",
			 __func__);
		if (add_sta->rmfEnabled)
			wma_set_peer_pmf_status(wma, add_sta->staMac, true);

		ret =
			wma_send_peer_assoc(wma, add_sta->nwType, add_sta);
		if (ret) {
			add_sta->status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			cdp_peer_add_last_real_peer(soc, pdev, vdev);
			wma_remove_req(wma, add_sta->smesessionId,
				       WMA_PEER_ASSOC_CNF_START);
			peer_assoc_cnf = false;

			goto send_rsp;
		}
	}

send_rsp:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf)
		return;

	WMA_LOGD(FL("statype %d vdev_id %d aid %d bssid %pM status %d"),
		 add_sta->staType, add_sta->smesessionId,
		 add_sta->assocId, add_sta->bssId,
		 add_sta->status);
	wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP, (void *)add_sta, 0);
}
#endif

/**
 * wma_send_bss_color_change_enable() - send bss color change enable cmd.
 * @wma: wma handle
 * @params: add sta params
 *
 * Send bss color change command to firmware, to enable firmware to update
 * internally if any change in bss color in advertised by associated AP.
 *
 * Return: none
 */
#ifdef WLAN_FEATURE_11AX
static void wma_send_bss_color_change_enable(tp_wma_handle wma,
					     tpAddStaParams params)
{
	QDF_STATUS status;
	uint32_t vdev_id = params->smesessionId;

	if (!params->he_capable) {
		WMA_LOGD("%s: he_capable is not set for vdev_id:%d",
			 __func__, vdev_id);
		return;
	}

	status = wmi_unified_send_bss_color_change_enable_cmd(wma->wmi_handle,
							      vdev_id,
							      true);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Failed to enable bss color change offload, vdev:%d",
			 vdev_id);
	}

	return;
}
#else
static void wma_send_bss_color_change_enable(tp_wma_handle wma,
					     tpAddStaParams params)
{
}
#endif

/**
 * wma_add_sta_req_sta_mode() - process add sta request in sta mode
 * @wma: wma handle
 * @params: add sta params
 *
 * Return: none
 */
static void wma_add_sta_req_sta_mode(tp_wma_handle wma, tpAddStaParams params)
{
	struct cdp_pdev *pdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	void *peer;
	struct wma_txrx_node *iface;
	int8_t maxTxPower;
	int ret = 0;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;
	int smps_param;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType) {
		wma_add_tdls_sta(wma, params);
		return;
	}
#endif

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s: Unable to get pdev", __func__);
		goto out;
	}

	iface = &wma->interfaces[params->smesessionId];
	if (params->staType != STA_ENTRY_SELF) {
		WMA_LOGE("%s: unsupported station type %d",
			 __func__, params->staType);
		goto out;
	}
	peer = cdp_peer_find_by_addr(soc, pdev, params->bssId);
	if (!peer) {
		WMA_LOGE("%s: Peer is not present vdev id %d for %pM", __func__,
			params->smesessionId, params->bssId);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}
	if (params->nonRoamReassoc) {
		cdp_peer_state_update(soc, pdev, params->bssId,
					  OL_TXRX_PEER_STATE_AUTH);
		qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STARTED);
		iface->aid = params->assocId;
		goto out;
	}

	if (wma_is_vdev_up(params->smesessionId)) {
		WMA_LOGD("%s: vdev id %d is already UP for %pM", __func__,
			 params->smesessionId, params->bssId);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	if (peer &&
	    (cdp_peer_state_get(soc, peer) == OL_TXRX_PEER_STATE_DISC)) {
		/*
		 * This is the case for reassociation.
		 * peer state update and peer_assoc is required since it
		 * was not done by WMA_ADD_BSS_REQ.
		 */

		/* Update peer state */
		if (params->encryptType == eSIR_ED_NONE) {
			WMA_LOGD("%s: Update peer(%pM) state into auth",
				 __func__, params->bssId);
			cdp_peer_state_update(soc, pdev, params->bssId,
						  OL_TXRX_PEER_STATE_AUTH);
		} else {
			WMA_LOGD("%s: Update peer(%pM) state into conn",
				 __func__, params->bssId);
			cdp_peer_state_update(soc, pdev, params->bssId,
						  OL_TXRX_PEER_STATE_CONN);
		}

		if (wma_is_roam_synch_in_progress(wma, params->smesessionId)) {
			/* iface->nss = params->nss; */
			/*In LFR2.0, the following operations are performed as
			 * part of wma_send_peer_assoc. As we are
			 * skipping this operation, we are just executing the
			 * following which are useful for LFR3.0
			 */
			cdp_peer_state_update(soc, pdev, params->bssId,
						  OL_TXRX_PEER_STATE_AUTH);
			qdf_atomic_set(&iface->bss_status,
				       WMA_BSS_STATUS_STARTED);
			iface->aid = params->assocId;
			WMA_LOGD("LFR3:statype %d vdev %d aid %d bssid %pM",
					params->staType, params->smesessionId,
					params->assocId, params->bssId);
			return;
		}
		wmi_unified_send_txbf(wma, params);

		if (wmi_service_enabled(wma->wmi_handle,
					    wmi_service_peer_assoc_conf)) {
			WMA_LOGD(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
			peer_assoc_cnf = true;
			msg = wma_fill_hold_req(wma, params->smesessionId,
				WMA_ADD_STA_REQ, WMA_PEER_ASSOC_CNF_START,
				params, WMA_PEER_ASSOC_TIMEOUT);
			if (!msg) {
				WMA_LOGD(FL("Failed to alloc request for vdev_id %d"),
					 params->smesessionId);
				params->status = QDF_STATUS_E_FAILURE;
				wma_remove_req(wma, params->smesessionId,
					       WMA_PEER_ASSOC_CNF_START);
				wma_remove_peer(wma, params->staMac,
					params->smesessionId, peer, false);
				peer_assoc_cnf = false;
				goto out;
			}
		} else {
			WMA_LOGD(FL("WMI_SERVICE_PEER_ASSOC_CONF not enabled"));
		}

		((tAddStaParams *)iface->addBssStaContext)->no_ptk_4_way =
						params->no_ptk_4_way;

		qdf_mem_copy(((tAddStaParams *)iface->addBssStaContext)->
			     supportedRates.supportedMCSSet,
			     params->supportedRates.supportedMCSSet,
			     SIR_MAC_MAX_SUPPORTED_MCS_SET);


		ret = wma_send_peer_assoc(wma,
				iface->nwType,
				(tAddStaParams *) iface->addBssStaContext);
		if (ret) {
			status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, params->bssId,
					params->smesessionId, peer, false);
			goto out;
		}

		if (params->rmfEnabled) {
			wma_set_mgmt_frame_protection(wma);
			wma_set_peer_pmf_status(wma, params->bssId, true);
		}

		/*
		 * Set the PTK in 11r mode because we already have it.
		 */
		if (iface->staKeyParams) {
			wma_set_stakey(wma,
				       (tpSetStaKeyParams) iface->staKeyParams);
			iface->staKeyParams = NULL;
		}
	}
	maxTxPower = params->maxTxPower;
	wma_vdev_set_bss_params(wma, params->smesessionId,
				iface->beaconInterval, iface->dtimPeriod,
				iface->shortSlotTimeSupported,
				iface->llbCoexist, maxTxPower);

	params->csaOffloadEnable = 0;
	if (wmi_service_enabled(wma->wmi_handle,
				   wmi_service_csa_offload)) {
		params->csaOffloadEnable = 1;
		if (wma_unified_csa_offload_enable(wma, params->smesessionId) <
		    0) {
			WMA_LOGE("Unable to enable CSA offload for vdev_id:%d",
				 params->smesessionId);
		}
	}

	if (wmi_service_enabled(wma->wmi_handle,
				wmi_service_filter_ipsec_natkeepalive)) {
		if (wmi_unified_nat_keepalive_en_cmd(wma->wmi_handle,
						     params->smesessionId)) {
			WMA_LOGE("Unable to enable NAT keepalive for vdev_id:%d",
				params->smesessionId);
		}
	}
	qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STARTED);
	WMA_LOGD("%s: STA mode (type %d subtype %d) BSS is started",
		 __func__, iface->type, iface->sub_type);
	/* Sta is now associated, configure various params */

	/* Send SMPS force command to FW to send the required
	 * action frame only when SM power save is enbaled in
	 * from INI. In case dynamic antenna selection, the
	 * action frames are sent by the chain mask manager
	 * In addition to the action frames, The SM power save is
	 * published in the assoc request HT SMPS IE for both cases.
	 */
	if ((params->enableHtSmps) && (params->send_smps_action)) {
		smps_param = wma_smps_mode_to_force_mode_param(
			params->htSmpsconfig);
		if (smps_param >= 0) {
			WMA_LOGD("%s: Send SMPS force mode: %d",
				__func__, params->htSmpsconfig);
			wma_set_mimops(wma, params->smesessionId,
				smps_param);
		}
	}

	wma_send_bss_color_change_enable(wma, params);

	/* Partial AID match power save, enable when SU bformee */
	if (params->enableVhtpAid && params->vhtTxBFCapable)
		wma_set_ppsconfig(params->smesessionId,
				  WMA_VHT_PPS_PAID_MATCH, 1);

	/* Enable AMPDU power save, if htCapable/vhtCapable */
	if (params->enableAmpduPs && (params->htCapable || params->vhtCapable))
		wma_set_ppsconfig(params->smesessionId,
				  WMA_VHT_PPS_DELIM_CRC_FAIL, 1);
	if (wmi_service_enabled(wma->wmi_handle,
				wmi_service_listen_interval_offload_support)) {
		WMA_LOGD("%s: listen interval offload enabled, setting params",
			 __func__);
		status = wma_vdev_set_param(wma->wmi_handle,
					    params->smesessionId,
					    WMI_VDEV_PARAM_MAX_LI_OF_MODDTIM,
					    wma->staMaxLIModDtim);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE(FL("can't set MAX_LI for session: %d"),
				 params->smesessionId);
		}
		status = wma_vdev_set_param(wma->wmi_handle,
					    params->smesessionId,
					    WMI_VDEV_PARAM_DYNDTIM_CNT,
					    wma->staDynamicDtim);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE(FL("can't set DYNDTIM_CNT for session: %d"),
				 params->smesessionId);
		}
		status  = wma_vdev_set_param(wma->wmi_handle,
					     params->smesessionId,
					     WMI_VDEV_PARAM_MODDTIM_CNT,
					     wma->staModDtim);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE(FL("can't set DTIM_CNT for session: %d"),
				 params->smesessionId);
		}

	} else {
		WMA_LOGD("%s: listen interval offload is not set",
			 __func__);
	}

	iface->aid = params->assocId;
	params->nss = iface->nss;
out:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf)
		return;

	params->status = status;
	WMA_LOGD(FL("statype %d vdev_id %d aid %d sta mac " QDF_MAC_ADDR_STR
		 " status %d"), params->staType, params->smesessionId,
		 params->assocId, QDF_MAC_ADDR_ARRAY(params->bssId),
		 params->status);
	/* Don't send a response during roam sync operation */
	if (!wma_is_roam_synch_in_progress(wma, params->smesessionId))
		wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP,
					   (void *)params, 0);
}

/**
 * wma_delete_sta_req_ap_mode() - process delete sta request from UMAC in AP mode
 * @wma: wma handle
 * @del_sta: delete sta params
 *
 * Return: none
 */
static void wma_delete_sta_req_ap_mode(tp_wma_handle wma,
				       tpDeleteStaParams del_sta)
{
	struct cdp_pdev *pdev;
	void *peer;
	struct wma_target_req *msg;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	QDF_STATUS qdf_status;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	peer = cdp_peer_find_by_addr(soc, pdev, del_sta->staMac);
	if (!peer) {
		WMA_LOGE("%s: Failed to get peer handle using peer addr %pM",
			 __func__, del_sta->staMac);
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	qdf_status = wma_remove_peer(wma, del_sta->staMac,
				     del_sta->smesessionId,
				     peer, false);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		WMA_LOGE(FL("wma_remove_peer failed"));
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}
	del_sta->status = QDF_STATUS_SUCCESS;

	if (wmi_service_enabled(wma->wmi_handle,
				    wmi_service_sync_delete_cmds)) {
		msg = wma_fill_hold_req(wma, del_sta->smesessionId,
				   WMA_DELETE_STA_REQ,
				   WMA_DELETE_STA_RSP_START, del_sta,
				   WMA_DELETE_STA_TIMEOUT);
		if (!msg) {
			WMA_LOGE(FL("Failed to allocate request. vdev_id %d"),
				 del_sta->smesessionId);
			wma_remove_req(wma, del_sta->smesessionId,
				WMA_DELETE_STA_RSP_START);
			del_sta->status = QDF_STATUS_E_NOMEM;
			goto send_del_rsp;
		}

		wma_acquire_wakelock(&wma->wmi_cmd_rsp_wake_lock,
				     WMA_FW_RSP_EVENT_WAKE_LOCK_DURATION);

		return;
	}

send_del_rsp:
	if (del_sta->respReqd) {
		WMA_LOGD("%s: Sending del rsp to umac (status: %d)",
			 __func__, del_sta->status);
		wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP,
					   (void *)del_sta, 0);
	}
}

#ifdef FEATURE_WLAN_TDLS
/**
 * wma_del_tdls_sta() - process delete sta request from UMAC in TDLS
 * @wma: wma handle
 * @del_sta: delete sta params
 *
 * Return: none
 */
static void wma_del_tdls_sta(tp_wma_handle wma, tpDeleteStaParams del_sta)
{
	struct tdls_peer_update_state *peer_state;
	struct wma_target_req *msg;
	int status;

	peer_state = qdf_mem_malloc(sizeof(*peer_state));
	if (!peer_state) {
		del_sta->status = QDF_STATUS_E_NOMEM;
		goto send_del_rsp;
	}

	if (wma_is_roam_synch_in_progress(wma, del_sta->smesessionId)) {
		WMA_LOGE("%s: roaming in progress, reject del sta!", __func__);
		del_sta->status = QDF_STATUS_E_PERM;
		qdf_mem_free(peer_state);
		goto send_del_rsp;
	}

	peer_state->peer_state = TDLS_PEER_STATE_TEARDOWN;
	peer_state->vdev_id = del_sta->smesessionId;
	peer_state->resp_reqd = del_sta->respReqd;
	qdf_mem_copy(&peer_state->peer_macaddr,
		     &del_sta->staMac, sizeof(tSirMacAddr));

	WMA_LOGD("%s: sending tdls_peer_state for peer mac: %pM, peerState: %d",
		 __func__, peer_state->peer_macaddr,
		 peer_state->peer_state);

	status = wma_update_tdls_peer_state(wma, peer_state);

	if (status < 0) {
		WMA_LOGE("%s: wma_update_tdls_peer_state returned failure",
				__func__);
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	if (del_sta->respReqd &&
			wmi_service_enabled(wma->wmi_handle,
				wmi_service_sync_delete_cmds)) {
		del_sta->status = QDF_STATUS_SUCCESS;
		msg = wma_fill_hold_req(wma,
				del_sta->smesessionId,
				WMA_DELETE_STA_REQ,
				WMA_DELETE_STA_RSP_START, del_sta,
				WMA_DELETE_STA_TIMEOUT);
		if (!msg) {
			WMA_LOGE(FL("Failed to allocate vdev_id %d"),
					del_sta->smesessionId);
			wma_remove_req(wma,
					del_sta->smesessionId,
					WMA_DELETE_STA_RSP_START);
			del_sta->status = QDF_STATUS_E_NOMEM;
			goto send_del_rsp;
		}

		wma_acquire_wakelock(&wma->wmi_cmd_rsp_wake_lock,
				WMA_FW_RSP_EVENT_WAKE_LOCK_DURATION);
	}

	return;

send_del_rsp:
	if (del_sta->respReqd) {
		WMA_LOGD("%s: Sending del rsp to umac (status: %d)",
			 __func__, del_sta->status);
		wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP,
					   (void *)del_sta, 0);
	}
}
#endif

/**
 * wma_delete_sta_req_sta_mode() - process delete sta request from UMAC
 * @wma: wma handle
 * @params: delete sta params
 *
 * Return: none
 */
static void wma_delete_sta_req_sta_mode(tp_wma_handle wma,
					tpDeleteStaParams params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wma_txrx_node *iface;

	iface = &wma->interfaces[params->smesessionId];
	iface->uapsd_cached_val = 0;
	if (wma_is_roam_synch_in_progress(wma, params->smesessionId))
		return;
#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType) {
		wma_del_tdls_sta(wma, params);
		return;
	}
#endif
	params->status = status;
	if (params->respReqd) {
		WMA_LOGD("%s: vdev_id %d status %d", __func__,
			 params->smesessionId, status);
		wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP,
					   (void *)params, 0);
	}
}

static void wma_sap_prevent_runtime_pm(tp_wma_handle wma)
{
	qdf_runtime_pm_prevent_suspend(&wma->sap_prevent_runtime_pm_lock);
}

static void wma_sap_allow_runtime_pm(tp_wma_handle wma)
{
	qdf_runtime_pm_allow_suspend(&wma->sap_prevent_runtime_pm_lock);
}

void wma_add_sta(tp_wma_handle wma, tpAddStaParams add_sta)
{
	uint8_t oper_mode = BSS_OPERATIONAL_MODE_STA;
	void *htc_handle;

	htc_handle = lmac_get_htc_hdl(wma->psoc);
	if (!htc_handle) {
		WMA_LOGE(":%sHTC handle is NULL:%d", __func__, __LINE__);
		return;
	}

	WMA_LOGD("%s: add_sta->sessionId = %d.", __func__,
		 add_sta->smesessionId);
	WMA_LOGD("%s: add_sta->bssId = %x:%x:%x:%x:%x:%x", __func__,
		 add_sta->bssId[0], add_sta->bssId[1], add_sta->bssId[2],
		 add_sta->bssId[3], add_sta->bssId[4], add_sta->bssId[5]);

	if (wma_is_vdev_in_ap_mode(wma, add_sta->smesessionId))
		oper_mode = BSS_OPERATIONAL_MODE_AP;
	else if (wma_is_vdev_in_ibss_mode(wma, add_sta->smesessionId))
		oper_mode = BSS_OPERATIONAL_MODE_IBSS;

	if (WMA_IS_VDEV_IN_NDI_MODE(wma->interfaces, add_sta->smesessionId))
		oper_mode = BSS_OPERATIONAL_MODE_NDI;
	switch (oper_mode) {
	case BSS_OPERATIONAL_MODE_STA:
		wma_add_sta_req_sta_mode(wma, add_sta);
		break;

	/* IBSS should share the same code as AP mode */
	case BSS_OPERATIONAL_MODE_IBSS:
	case BSS_OPERATIONAL_MODE_AP:
		if (qdf_is_drv_connected()) {
			wma_debug("drv wow enabled prevent runtime pm");
			wma_sap_prevent_runtime_pm(wma);
		} else {
			wma_debug("non-drv wow enabled vote for link up");
			htc_vote_link_up(htc_handle);
		}
		wma_add_sta_req_ap_mode(wma, add_sta);
		break;
	case BSS_OPERATIONAL_MODE_NDI:
		wma_add_sta_ndi_mode(wma, add_sta);
		break;
	}

	/* adjust heart beat thresold timer value for detecting ibss peer
	 * departure
	 */
	if (oper_mode == BSS_OPERATIONAL_MODE_IBSS)
		wma_adjust_ibss_heart_beat_timer(wma, add_sta->smesessionId, 1);
}

void wma_delete_sta(tp_wma_handle wma, tpDeleteStaParams del_sta)
{
	uint8_t oper_mode = BSS_OPERATIONAL_MODE_STA;
	uint8_t smesession_id = del_sta->smesessionId;
	bool rsp_requested = del_sta->respReqd;
	void *htc_handle;

	htc_handle = lmac_get_htc_hdl(wma->psoc);
	if (!htc_handle) {
		WMA_LOGE(":%sHTC handle is NULL:%d", __func__, __LINE__);
		return;
	}

	if (wma_is_vdev_in_ap_mode(wma, smesession_id))
		oper_mode = BSS_OPERATIONAL_MODE_AP;
	if (wma_is_vdev_in_ibss_mode(wma, smesession_id)) {
		oper_mode = BSS_OPERATIONAL_MODE_IBSS;
		WMA_LOGD("%s: to delete sta for IBSS mode", __func__);
	}
	if (del_sta->staType == STA_ENTRY_NDI_PEER)
		oper_mode = BSS_OPERATIONAL_MODE_NDI;

	WMA_LOGD(FL("oper_mode %d"), oper_mode);

	switch (oper_mode) {
	case BSS_OPERATIONAL_MODE_STA:
		wma_delete_sta_req_sta_mode(wma, del_sta);
		if (wma_is_roam_synch_in_progress(wma, smesession_id)) {
			WMA_LOGD(FL("LFR3: Del STA on vdev_id %d"),
				 del_sta->smesessionId);
			qdf_mem_free(del_sta);
			return;
		}
		if (!rsp_requested) {
			WMA_LOGD(FL("vdev_id %d status %d"),
				 del_sta->smesessionId, del_sta->status);
			qdf_mem_free(del_sta);
		}
		break;

	case BSS_OPERATIONAL_MODE_IBSS: /* IBSS shares AP code */
	case BSS_OPERATIONAL_MODE_AP:
		if (qdf_is_drv_connected()) {
			wma_debug("drv wow enabled allow runtime pm");
			wma_sap_allow_runtime_pm(wma);
		} else {
			wma_debug("drv wow disabled vote for link down");
			htc_vote_link_down(htc_handle);
		}
		wma_delete_sta_req_ap_mode(wma, del_sta);
		/* free the memory here only if sync feature is not enabled */
		if (!rsp_requested &&
		    !wmi_service_enabled(wma->wmi_handle,
				wmi_service_sync_delete_cmds)) {
			WMA_LOGD(FL("vdev_id %d status %d"),
				 del_sta->smesessionId, del_sta->status);
			qdf_mem_free(del_sta);
		} else if (!rsp_requested &&
				(del_sta->status != QDF_STATUS_SUCCESS)) {
			WMA_LOGD(FL("Release del_sta mem vdev_id %d status %d"),
				 del_sta->smesessionId, del_sta->status);
			qdf_mem_free(del_sta);
		}
		break;
	case BSS_OPERATIONAL_MODE_NDI:
		wma_delete_sta_req_ndi_mode(wma, del_sta);
		break;
	default:
		WMA_LOGE(FL("Incorrect oper mode %d"), oper_mode);
		qdf_mem_free(del_sta);
	}

	/* adjust heart beat thresold timer value for
	 * detecting ibss peer departure
	 */
	if (oper_mode == BSS_OPERATIONAL_MODE_IBSS)
		wma_adjust_ibss_heart_beat_timer(wma, smesession_id, -1);
}

void wma_delete_bss_ho_fail(tp_wma_handle wma, uint8_t vdev_id)
{
	struct cdp_pdev *pdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wma_txrx_node *iface;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct vdev_stop_response resp_event;
	struct del_bss_resp *vdev_stop_resp;
	uint8_t *bssid;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s:Unable to get TXRX context", __func__);
		goto fail_del_bss_ho_fail;
	}

	iface = &wma->interfaces[vdev_id];
	if (!iface) {
		WMA_LOGE("%s vdev id %d is already deleted",
			 __func__, vdev_id);
		goto fail_del_bss_ho_fail;
	}
	bssid = wma_get_vdev_bssid(iface->vdev);
	if (!bssid) {
		WMA_LOGE("%s:Invalid bssid", __func__);
		status = QDF_STATUS_E_FAILURE;
		goto fail_del_bss_ho_fail;
	}
	qdf_mem_zero(bssid, QDF_MAC_ADDR_SIZE);

	if (iface->psnr_req) {
		qdf_mem_free(iface->psnr_req);
		iface->psnr_req = NULL;
	}

	if (iface->rcpi_req) {
		struct sme_rcpi_req *rcpi_req = iface->rcpi_req;

		iface->rcpi_req = NULL;
		qdf_mem_free(rcpi_req);
	}

	if (iface->roam_scan_stats_req) {
		struct sir_roam_scan_stats *roam_scan_stats_req =
						iface->roam_scan_stats_req;

		iface->roam_scan_stats_req = NULL;
		qdf_mem_free(roam_scan_stats_req);
	}

	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP (del_bss)",
		 __func__,  vdev_id);
	cdp_fc_vdev_pause(soc, vdev_id, OL_TXQ_PAUSE_REASON_VDEV_STOP, 0);
	wma_vdev_set_pause_bit(vdev_id, PAUSE_TYPE_HOST);
	cdp_fc_vdev_flush(soc, vdev_id);
	WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for VDEV_STOP rsp",
		 __func__,  vdev_id);
	cdp_fc_vdev_unpause(soc, vdev_id, OL_TXQ_PAUSE_REASON_VDEV_STOP, 0);
	wma_vdev_clear_pause_bit(vdev_id, PAUSE_TYPE_HOST);
	qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
	WMA_LOGD("%s: (type %d subtype %d) BSS is stopped",
			__func__, iface->type, iface->sub_type);

	status = mlme_set_vdev_stop_type(iface->vdev,
					 WMA_DELETE_BSS_HO_FAIL_REQ);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to set wma req msg_type for vdev_id: %d",
			 __func__, vdev_id);
		goto fail_del_bss_ho_fail;
	}

	/* Try to use the vdev stop response path */
	resp_event.vdev_id = vdev_id;
	status = wma_handle_vdev_stop_rsp(wma, &resp_event);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to handle vdev stop rsp for vdev_id %d",
			 __func__, vdev_id);
		goto fail_del_bss_ho_fail;
	}

	return;

fail_del_bss_ho_fail:
	vdev_stop_resp = qdf_mem_malloc(sizeof(*vdev_stop_resp));
	if (!vdev_stop_resp) {
		WMA_LOGE("%s: Failed to alloc del bss resp ", __func__);
		return;
	}
	vdev_stop_resp->vdev_id = vdev_id;
	vdev_stop_resp->status = status;
	wma_send_msg_high_priority(wma, WMA_DELETE_BSS_HO_FAIL_RSP,
				   (void *)vdev_stop_resp, 0);
}

/**
 * wma_wait_tx_complete() - Wait till tx packets are drained
 * @wma: wma handle
 * @session_id: vdev id
 *
 * Return: none
 */
static void wma_wait_tx_complete(tp_wma_handle wma,
				uint32_t session_id)
{
	struct cdp_pdev *pdev;
	uint8_t max_wait_iterations = 0;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!wma_is_vdev_valid(session_id)) {
		WMA_LOGE("%s: Vdev is not valid: %d",
			 __func__, session_id);
		return;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is not valid: %d",
			 __func__, session_id);
		return;
	}
	max_wait_iterations =
		wma->interfaces[session_id].delay_before_vdev_stop /
		WMA_TX_Q_RECHECK_TIMER_WAIT;

	while (cdp_get_tx_pending(soc, pdev) && max_wait_iterations) {
		WMA_LOGW(FL("Waiting for outstanding packet to drain."));
		qdf_wait_for_event_completion(&wma->tx_queue_empty_event,
				      WMA_TX_Q_RECHECK_TIMER_WAIT);
		max_wait_iterations--;
	}
}

void wma_delete_bss(tp_wma_handle wma, uint8_t vdev_id)
{
	struct cdp_pdev *pdev;
	void *peer = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct cdp_vdev *txrx_vdev = NULL;
	bool roam_synch_in_progress = false;
	struct wma_txrx_node *iface;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct qdf_mac_addr bssid;
	struct del_bss_resp *params;
	uint8_t *addr, *bssid_addr;
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s:Unable to get TXRX context", __func__);
		goto out;
	}

	iface = &wma->interfaces[vdev_id];
	if (!iface || !iface->vdev || !wlan_vdev_get_dp_handle(iface->vdev)) {
		WMA_LOGE("%s vdev id %d is already deleted",
			 __func__, vdev_id);
		goto out;
	}

	status = mlme_get_vdev_bss_peer_mac_addr(iface->vdev, &bssid);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s vdev id %d : failed to get bssid",
			 __func__, vdev_id);
		goto out;
	}

	addr = wlan_vdev_mlme_get_macaddr(iface->vdev);
	if (!addr) {
		WMA_LOGE("%s vdev id %d : failed to get macaddr",
			 __func__, vdev_id);
		goto out;
	}
	if (wma_is_vdev_in_ibss_mode(wma, vdev_id))
		/* in rome ibss case, self mac is used to create the bss peer */
		peer = cdp_peer_find_by_addr(soc, pdev, addr);
	else if (WMA_IS_VDEV_IN_NDI_MODE(wma->interfaces,
			vdev_id))
		/* In ndi case, self mac is used to create the self peer */
		peer = cdp_peer_find_by_addr(soc, pdev, addr);
	else
		peer = cdp_peer_find_by_addr(soc, pdev,
				bssid.bytes);

	if (!peer) {
		WMA_LOGE("%s: Failed to find peer %pM", __func__,
			 bssid.bytes);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}
	bssid_addr = wma_get_vdev_bssid(wma->interfaces[vdev_id].vdev);
	if (!bssid_addr) {
		WMA_LOGE("%s: Failed to bssid for vdev_%d", __func__,
			 vdev_id);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}
	qdf_mem_zero(bssid_addr,
		     QDF_MAC_ADDR_SIZE);

	txrx_vdev = wma_find_vdev_by_id(wma, vdev_id);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	wma_delete_invalid_peer_entries(vdev_id, NULL);

	if (iface->psnr_req) {
		qdf_mem_free(iface->psnr_req);
		iface->psnr_req = NULL;
	}

	if (iface->rcpi_req) {
		struct sme_rcpi_req *rcpi_req = iface->rcpi_req;

		iface->rcpi_req = NULL;
		qdf_mem_free(rcpi_req);
	}

	if (iface->roam_scan_stats_req) {
		struct sir_roam_scan_stats *roam_scan_stats_req =
						iface->roam_scan_stats_req;

		iface->roam_scan_stats_req = NULL;
		qdf_mem_free(roam_scan_stats_req);
	}

	if (wlan_op_mode_ibss == cdp_get_opmode(soc, vdev_id))
		wma->ibss_started = 0;

	if (wma_is_roam_synch_in_progress(wma, vdev_id)) {
		roam_synch_in_progress = true;
		WMA_LOGD("LFR3:%s: Setting vdev_up to FALSE for session %d",
			__func__, vdev_id);

		goto detach_peer;
	}

	status = mlme_set_vdev_stop_type(iface->vdev,
					 WMA_DELETE_BSS_REQ);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to set wma req msg_type for vdev_id: %d",
			 __func__, vdev_id);
		goto out;
	}

	WMA_LOGD(FL("Outstanding msdu packets: %d"),
		 cdp_get_tx_pending(soc, pdev));
	wma_wait_tx_complete(wma, vdev_id);

	if (cdp_get_tx_pending(soc, pdev)) {
		WMA_LOGW(FL("Outstanding msdu packets before VDEV_STOP : %d"),
			 cdp_get_tx_pending(soc, pdev));
	}

	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP (del_bss)",
		 __func__, vdev_id);
	wma_vdev_set_pause_bit(vdev_id, PAUSE_TYPE_HOST);
	cdp_fc_vdev_pause(soc, vdev_id,
			  OL_TXQ_PAUSE_REASON_VDEV_STOP, 0);

	if (wma_send_vdev_stop_to_fw(wma, vdev_id)) {
		WMA_LOGE("%s: %d Failed to send vdev stop", __func__, __LINE__);
		status = QDF_STATUS_E_FAILURE;
		qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
		goto detach_peer;
	}
	WMA_LOGD("%s: bssid %pM vdev_id %d",
		 __func__, bssid.bytes, vdev_id);

	return;

detach_peer:
	wma_remove_peer(wma, bssid.bytes, vdev_id, peer,
			roam_synch_in_progress);
	if (wma_is_roam_synch_in_progress(wma, vdev_id))
		return;

out:
	params = qdf_mem_malloc(sizeof(*params));
	if (!params) {
		WMA_LOGE("%s vdev id %d : failed to alloc del bss resp",
			 __func__, vdev_id);
		return;
	}
	params->vdev_id = vdev_id;
	params->status = status;
	wma_send_msg_high_priority(wma, WMA_DELETE_BSS_RSP, params, 0);
}

/**
 * wma_find_ibss_vdev() - This function finds vdev_id based on input type
 * @wma: wma handle
 * @type: vdev type
 *
 * Return: vdev id
 */
int32_t wma_find_vdev_by_type(tp_wma_handle wma, int32_t type)
{
	int32_t vdev_id = 0;
	struct wma_txrx_node *intf = wma->interfaces;

	for (vdev_id = 0; vdev_id < wma->max_bssid; vdev_id++) {
		if (intf) {
			if (intf[vdev_id].type == type)
				return vdev_id;
		}
	}

	return -EFAULT;
}

void wma_set_vdev_intrabss_fwd(tp_wma_handle wma_handle,
				      tpDisableIntraBssFwd pdis_intra_fwd)
{
	struct wlan_objmgr_vdev *vdev;

	WMA_LOGD("%s:intra_fwd:vdev(%d) intrabss_dis=%s",
		 __func__, pdis_intra_fwd->sessionId,
		 (pdis_intra_fwd->disableintrabssfwd ? "true" : "false"));

	vdev = wma_handle->interfaces[pdis_intra_fwd->sessionId].vdev;
	cdp_cfg_vdev_rx_set_intrabss_fwd(cds_get_context(QDF_MODULE_ID_SOC),
					 pdis_intra_fwd->sessionId,
					 pdis_intra_fwd->disableintrabssfwd);
}

void wma_store_pdev(void *wma_ctx, struct wlan_objmgr_pdev *pdev)
{
	tp_wma_handle wma = (tp_wma_handle)wma_ctx;
	QDF_STATUS status;

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_LEGACY_WMA_ID);
	if (QDF_STATUS_SUCCESS != status) {
		wma->pdev = NULL;
		return;
	}

	wma->pdev = pdev;
}

/**
 * wma_vdev_reset_beacon_interval_timer() - reset beacon interval back
 * to its original value after the channel switch.
 *
 * @data: data
 *
 * Return: void
 */
static void wma_vdev_reset_beacon_interval_timer(void *data)
{
	tp_wma_handle wma;
	struct wma_beacon_interval_reset_req *req =
		(struct wma_beacon_interval_reset_req *)data;
	uint16_t beacon_interval = req->interval;
	uint8_t vdev_id = req->vdev_id;

	wma = (tp_wma_handle)cds_get_context(QDF_MODULE_ID_WMA);
	if (!wma) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		goto end;
	}

	/* Change the beacon interval back to its original value */
	WMA_LOGE("%s: Change beacon interval back to %d",
			__func__, beacon_interval);
	wma_update_beacon_interval(wma, vdev_id, beacon_interval);

end:
	qdf_timer_stop(&req->event_timeout);
	qdf_timer_free(&req->event_timeout);
	qdf_mem_free(req);
}

int wma_fill_beacon_interval_reset_req(tp_wma_handle wma, uint8_t vdev_id,
				uint16_t beacon_interval, uint32_t timeout)
{
	struct wma_beacon_interval_reset_req *req;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return -ENOMEM;

	WMA_LOGD("%s: vdev_id %d ", __func__, vdev_id);
	req->vdev_id = vdev_id;
	req->interval = beacon_interval;
	qdf_timer_init(NULL, &req->event_timeout,
		wma_vdev_reset_beacon_interval_timer, req, QDF_TIMER_TYPE_SW);
	qdf_timer_start(&req->event_timeout, timeout);

	return 0;
}

QDF_STATUS wma_set_wlm_latency_level(void *wma_ptr,
			struct wlm_latency_level_param *latency_params)
{
	QDF_STATUS ret;
	tp_wma_handle wma = (tp_wma_handle)wma_ptr;

	WMA_LOGD("%s: set latency level %d, flags flag 0x%x",
		 __func__, latency_params->wlm_latency_level,
		 latency_params->wlm_latency_flags);

	ret = wmi_unified_wlm_latency_level_cmd(wma->wmi_handle,
						latency_params);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGW("Failed to set latency level");

	return ret;
}

QDF_STATUS wma_add_bss_peer_sta(uint8_t *self_mac, uint8_t *bssid,
				bool roam_synch)
{
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	uint8_t vdev_id;
	tp_wma_handle wma;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!pdev || !wma) {
		WMA_LOGE("Invalid pdev or wma");
		goto err;
	}
	vdev = wma_find_vdev_by_addr(wma, self_mac, &vdev_id);
	if (!vdev) {
		WMA_LOGE("vdev not found for addr: %pM", self_mac);
		goto err;
	}
	status = wma_create_peer(wma, pdev, vdev, bssid, WMI_PEER_TYPE_DEFAULT,
				 vdev_id, roam_synch);
err:
	return status;
}

QDF_STATUS wma_send_vdev_stop(uint8_t vdev_id)
{
	tp_wma_handle wma;
	struct cdp_pdev *pdev;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	QDF_STATUS status;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (!pdev || !wma) {
		WMA_LOGE("Invalid pdev or wma");
		return QDF_STATUS_E_FAILURE;
	}

	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP",
		 __func__, vdev_id);
	cdp_fc_vdev_pause(soc, vdev_id,
			  OL_TXQ_PAUSE_REASON_VDEV_STOP, 0);

	status = mlme_set_vdev_stop_type(
				wma->interfaces[vdev_id].vdev,
				WMA_SET_LINK_STATE);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGP(FL("Failed to set wma req msg_type for vdev_id %d"),
			 vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	wma_vdev_set_pause_bit(vdev_id, PAUSE_TYPE_HOST);
	if (wma_send_vdev_stop_to_fw(wma, vdev_id)) {
		WMA_LOGP("%s: %d Failed to send vdev stop",
			 __func__, __LINE__);
	}

	WMA_LOGP("%s: %d vdev stop sent vdev %d", __func__, __LINE__,
		 vdev_id);
	/*
	 * Remove peer, Vdev down and sending set link
	 * response will be handled in vdev stop response
	 * handler
	 */

	return QDF_STATUS_SUCCESS;
}

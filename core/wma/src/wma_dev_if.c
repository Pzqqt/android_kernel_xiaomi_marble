/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
#include "cfg_api.h"

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
#include "wlan_tgt_def_config.h"
#include <wlan_dfs_tgt_api.h>
#include <cdp_txrx_handle.h>
#include "wlan_pmo_ucfg_api.h"
#include "wlan_reg_services_api.h"

#include "wma_he.h"
#include "wlan_roam_debug.h"
#include "wlan_ocb_ucfg_api.h"
#include "init_deinit_ucfg.h"
#include <target_if.h>

/**
 * wma_find_vdev_by_addr() - find vdev_id from mac address
 * @wma: wma handle
 * @addr: mac address
 * @vdev_id: return vdev_id
 *
 * Return: Returns vdev handle or NULL if mac address don't match
 */
struct cdp_vdev *wma_find_vdev_by_addr(tp_wma_handle wma, uint8_t *addr,
				   uint8_t *vdev_id)
{
	uint8_t i;

	for (i = 0; i < wma->max_bssid; i++) {
		if (qdf_is_macaddr_equal(
			(struct qdf_mac_addr *) wma->interfaces[i].addr,
			(struct qdf_mac_addr *) addr) == true) {
			*vdev_id = i;
			return wma->interfaces[i].handle;
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
/**
 * wma_is_vdev_in_ibss_mode() - check that vdev is in ibss mode or not
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Helper function to know whether given vdev id
 * is in IBSS mode or not.
 *
 * Return: True/False
 */
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
#endif /* QCA_IBSS_SUPPORT */

/**
 * wma_find_vdev_by_bssid() - Get the corresponding vdev_id from BSSID
 * @wma - wma handle
 * @vdev_id - vdev ID
 *
 * Return: fill vdev_id with appropriate vdev id and return vdev
 *         handle or NULL if not found.
 */
struct cdp_vdev *wma_find_vdev_by_bssid(tp_wma_handle wma, uint8_t *bssid,
				    uint8_t *vdev_id)
{
	int i;

	for (i = 0; i < wma->max_bssid; i++) {
		if (qdf_is_macaddr_equal(
			(struct qdf_mac_addr *) wma->interfaces[i].bssid,
			(struct qdf_mac_addr *) bssid) == true) {
			*vdev_id = i;
			return wma->interfaces[i].handle;
		}
	}

	return NULL;
}

/**
 * wma_get_txrx_vdev_type() - return operating mode of vdev
 * @type: vdev_type
 *
 * Return: return operating mode as enum wlan_op_mode type
 */
static enum wlan_op_mode wma_get_txrx_vdev_type(uint32_t type)
{
	enum wlan_op_mode vdev_type = wlan_op_mode_unknown;

	switch (type) {
	case WMI_VDEV_TYPE_AP:
		vdev_type = wlan_op_mode_ap;
		break;
	case WMI_VDEV_TYPE_STA:
		vdev_type = wlan_op_mode_sta;
		break;
#ifdef QCA_IBSS_SUPPORT
	case WMI_VDEV_TYPE_IBSS:
		vdev_type = wlan_op_mode_ibss;
		break;
#endif /* QCA_IBSS_SUPPORT */
	case WMI_VDEV_TYPE_OCB:
		vdev_type = wlan_op_mode_ocb;
		break;
	case WMI_VDEV_TYPE_MONITOR:
		vdev_type = wlan_op_mode_monitor;
		break;
	case WMI_VDEV_TYPE_NDI:
		vdev_type = wlan_op_mode_ndi;
		break;
	default:
		WMA_LOGE("Invalid vdev type %u", type);
		vdev_type = wlan_op_mode_unknown;
	}

	return vdev_type;
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
 * wma_find_vdev_req() - find target request for vdev id
 * @wma: wma handle
 * @vdev_id: vdev id
 * @type: request type
 * @remove_req_from_list: flag to indicate remove req or not.
 *
 * Return: return target request if found or NULL.
 */
static struct wma_target_req *wma_find_vdev_req(tp_wma_handle wma,
						uint8_t vdev_id, uint8_t type,
						bool remove_req_from_list)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	qdf_list_node_t *node1 = NULL, *node2 = NULL;
	QDF_STATUS status;

	qdf_spin_lock_bh(&wma->vdev_respq_lock);
	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(&wma->vdev_resp_queue,
						      &node2)) {
		qdf_spin_unlock_bh(&wma->vdev_respq_lock);
		WMA_LOGD(FL("unable to get target req from vdev resp queue vdev_id: %d type: %d"),
				vdev_id, type);
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
		if (remove_req_from_list) {
			status = qdf_list_remove_node(&wma->vdev_resp_queue,
					node1);
			if (QDF_STATUS_SUCCESS != status) {
				qdf_spin_unlock_bh(&wma->vdev_respq_lock);
				WMA_LOGD(FL(
				"Failed to target req for vdev_id %d type %d"),
						vdev_id, type);
				return NULL;
			}
		}
		break;
	} while (QDF_STATUS_SUCCESS  ==
			qdf_list_peek_next(&wma->vdev_resp_queue,
					   node1, &node2));

	qdf_spin_unlock_bh(&wma->vdev_respq_lock);
	if (!found) {
		WMA_LOGD(FL("target request not found for vdev_id %d type %d"),
			 vdev_id, type);
		return NULL;
	}
	WMA_LOGD(FL("target request found for vdev id: %d type %d msg %d"),
		 vdev_id, type, req_msg->msg_type);
	return req_msg;
}

/**
 * wma_send_del_sta_self_resp() - send del sta self resp to Upper layer
 * @param: params of del sta resp
 *
 * Return: none
 */
static inline void wma_send_del_sta_self_resp(struct del_sta_self_params *param)
{
	struct scheduler_msg sme_msg = {0};
	QDF_STATUS status;

	sme_msg.type = eWNI_SME_DEL_STA_SELF_RSP;
	sme_msg.bodyptr = param;

	status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		WMA_LOGE("Failed to post eWNI_SME_DEL_STA_SELF_RSP");
		qdf_mem_free(param);
	}
}

/**
 * wma_vdev_detach_callback() - send vdev detach response to upper layer
 * @ctx: txrx node ptr
 *
 * Return: none
 */
static void wma_vdev_detach_callback(void *ctx)
{
	tp_wma_handle wma;
	struct wma_txrx_node *iface = (struct wma_txrx_node *)ctx;
	struct del_sta_self_params *param;
	struct wma_target_req *req_msg;

	wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma || !iface->del_staself_req) {
		WMA_LOGE("%s: wma %pK iface %pK", __func__, wma,
			 iface->del_staself_req);
		return;
	}
	param = (struct del_sta_self_params *) iface->del_staself_req;
	iface->del_staself_req = NULL;
	WMA_LOGD("%s: sending eWNI_SME_DEL_STA_SELF_RSP for vdev %d",
		 __func__, param->session_id);
	if (!wmi_service_enabled(wma->wmi_handle,
				    wmi_service_sync_delete_cmds)) {
		req_msg = wma_find_vdev_req(wma, param->session_id,
					    WMA_TARGET_REQ_TYPE_VDEV_DEL,
					    true);
		if (req_msg) {
			WMA_LOGD("%s: Found vdev request for vdev id %d",
				 __func__, param->session_id);
			qdf_mc_timer_stop(&req_msg->event_timeout);
			qdf_mc_timer_destroy(&req_msg->event_timeout);
			qdf_mem_free(req_msg);
		}
	}
	if (iface->addBssStaContext)
		qdf_mem_free(iface->addBssStaContext);


	if (iface->staKeyParams)
		qdf_mem_free(iface->staKeyParams);

	if (iface->stats_rsp)
		qdf_mem_free(iface->stats_rsp);

	wma_vdev_deinit(iface);
	qdf_mem_zero(iface, sizeof(*iface));
	wma_vdev_init(iface);

	param->status = QDF_STATUS_SUCCESS;
	wma_send_del_sta_self_resp(param);
}


/**
 * wma_self_peer_remove() - Self peer remove handler
 * @wma: wma handle
 * @del_sta_self_req_param: vdev id
 * @generate_vdev_rsp: request type
 *
 * Return: success if peer delete command sent to firmware, else failure.
 */

static QDF_STATUS wma_self_peer_remove(tp_wma_handle wma_handle,
			struct del_sta_self_params *del_sta_self_req_param,
			uint8_t generate_vdev_rsp)
{
	void *peer;
	struct cdp_pdev *pdev;
	uint8_t peer_id;
	uint8_t vdev_id = del_sta_self_req_param->session_id;
	struct wma_target_req *msg = NULL;
	struct del_sta_self_rsp_params *sta_self_wmi_rsp;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("P2P Device: removing self peer %pM",
		 del_sta_self_req_param->self_mac_addr);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
			return QDF_STATUS_E_FAULT;
	}

	peer = cdp_peer_find_by_addr(soc, pdev,
			del_sta_self_req_param->self_mac_addr,
			&peer_id);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 del_sta_self_req_param->self_mac_addr);
		return QDF_STATUS_SUCCESS;
	}
	wma_remove_peer(wma_handle,
			del_sta_self_req_param->self_mac_addr,
			vdev_id, peer, false);

	if (wmi_service_enabled(wma_handle->wmi_handle,
				wmi_service_sync_delete_cmds)) {
		sta_self_wmi_rsp =
			qdf_mem_malloc(sizeof(struct del_sta_self_rsp_params));
		if (sta_self_wmi_rsp == NULL) {
			WMA_LOGE(FL("Failed to allocate memory"));
			return QDF_STATUS_E_NOMEM;
		}
		sta_self_wmi_rsp->self_sta_param = del_sta_self_req_param;
		sta_self_wmi_rsp->generate_rsp = generate_vdev_rsp;
		msg = wma_fill_hold_req(wma_handle, vdev_id,
				   WMA_DELETE_STA_REQ,
				   WMA_DEL_P2P_SELF_STA_RSP_START,
				   sta_self_wmi_rsp,
				   WMA_DELETE_STA_TIMEOUT);
		if (!msg) {
			WMA_LOGE(FL("Failed to allocate request for vdev_id %d"),
				 vdev_id);
			wma_remove_req(wma_handle, vdev_id,
				WMA_DEL_P2P_SELF_STA_RSP_START);
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}

static void
wma_cdp_vdev_detach(ol_txrx_soc_handle soc,
			tp_wma_handle wma_handle,
			uint8_t vdev_id)
{
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];

	cdp_vdev_detach(soc,
		iface->handle, NULL, NULL);
	iface->handle = NULL;
	iface->is_vdev_valid = false;
}

#define WAIT_FW_PEER_DELETE_TIMEOUT (5*1000)

static QDF_STATUS wma_handle_vdev_detach(tp_wma_handle wma_handle,
			struct del_sta_self_params *del_sta_self_req_param,
			uint8_t generate_rsp)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = del_sta_self_req_param->session_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	struct wma_target_req *msg = NULL;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc) {
		WMA_LOGE("%s:SOC context is NULL", __func__);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	while (qdf_atomic_read(&iface->fw_peer_count)) {
		qdf_wait_for_event_completion(&iface->fw_peer_delete,
					      WAIT_FW_PEER_DELETE_TIMEOUT);
		qdf_event_reset(&iface->fw_peer_delete);
	}

	status = wmi_unified_vdev_delete_send(wma_handle->wmi_handle, vdev_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("Unable to remove an interface");
		goto out;
	}

	qdf_event_destroy(&iface->fw_peer_delete);

	WMA_LOGD("vdev_id:%hu vdev_hdl:%pK", vdev_id, iface->handle);
	if (!generate_rsp) {
		WMA_LOGE("Call txrx detach w/o callback for vdev %d", vdev_id);
		goto out;
	}

	iface->del_staself_req = del_sta_self_req_param;
	msg = wma_fill_vdev_req(wma_handle, vdev_id, WMA_DEL_STA_SELF_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_DEL, iface, 6000);
	if (!msg) {
		WMA_LOGE("%s: Failed to fill vdev request for vdev_id %d",
			 __func__, vdev_id);
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	/* Acquire wake lock only when you expect a response from firmware */
	if (wmi_service_enabled(wma_handle->wmi_handle,
				   wmi_service_sync_delete_cmds)) {
		wma_acquire_wakelock(&wma_handle->wmi_cmd_rsp_wake_lock,
				     WMA_FW_RSP_EVENT_WAKE_LOCK_DURATION);
	}
	WMA_LOGD("Call txrx detach with callback for vdev %d", vdev_id);
	wma_cdp_vdev_detach(soc, wma_handle, vdev_id);

	/*
	 * send the response immediately if WMI_SERVICE_SYNC_DELETE_CMDS
	 * service is not supported by firmware
	 */
	if (!wmi_service_enabled(wma_handle->wmi_handle,
				    wmi_service_sync_delete_cmds))
		wma_vdev_detach_callback(iface);
	return status;
out:
	WMA_LOGE("Call txrx detach callback for vdev %d, generate_rsp %u",
		vdev_id, generate_rsp);
	wma_cdp_vdev_detach(soc, wma_handle, vdev_id);

	if (iface->addBssStaContext)
		qdf_mem_free(iface->addBssStaContext);
	if (iface->staKeyParams)
		qdf_mem_free(iface->staKeyParams);

	wma_vdev_deinit(iface);
	qdf_mem_zero(iface, sizeof(*iface));
	wma_vdev_init(iface);

	del_sta_self_req_param->status = status;
	if (generate_rsp)
		wma_send_del_sta_self_resp(del_sta_self_req_param);
	return status;
}

/**
 * wma_peer_remove_for_vdev_callback() - remove peer for vdev when SSR
 * @handle: wma handle
 * @bssid: mac address
 * @vdev_id: vdev id
 * @peer: peer ptr
 *
 * Wapper wma_remove_peer for callback, this function will remove peer
 * without waiting for peer unmap event.
 * Return: none
 */
static void wma_peer_remove_for_vdev_callback(void *handle, uint8_t *bssid,
		uint8_t vdev_id, void *peer)
{
	wma_remove_peer(handle, bssid, vdev_id, peer, true);
}

/**
 * wma_force_vdev_cleanup() - Cleanup vdev resource when SSR
 * @wma_handle: WMA handle
 * @vdev_id: vdev ID
 *
 * Return: none
 */
static void wma_force_vdev_cleanup(tp_wma_handle wma_handle, uint8_t vdev_id)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;

	WMA_LOGE("SSR: force cleanup vdev(%d) resouce", vdev_id);
	iface->vdev_active = false;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto VDEV_DETACH;
	}

	vdev = cdp_get_vdev_from_vdev_id(soc, pdev, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: Failed to get vdev (%d)", __func__, vdev_id);
		goto VDEV_DETACH;
	}

	/* force remove all peer for vdev */
	cdp_peer_remove_for_vdev_no_lock(soc, vdev,
					 wma_peer_remove_for_vdev_callback,
					 wma_handle);

VDEV_DETACH:
	wma_cdp_vdev_detach(soc, wma_handle, vdev_id);
}

/**
 * wma_vdev_detach() - send vdev delete command to fw
 * @wma_handle: wma handle
 * @pdel_sta_self_req_param: del sta params
 * @generateRsp: generate Response flag
 *
 * Return: QDF status
 */
QDF_STATUS wma_vdev_detach(tp_wma_handle wma_handle,
			struct del_sta_self_params *pdel_sta_self_req_param,
			uint8_t generateRsp)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id = pdel_sta_self_req_param->session_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	struct wma_target_req *req_msg;

	if (!iface->handle) {
		WMA_LOGE("handle of vdev_id %d is NULL vdev is already freed",
			 vdev_id);
		goto send_rsp;
	}

	/*
	 * In SSR case, there is no need to destroy vdev in firmware since
	 * it has already asserted.
	 */
	if (cds_is_driver_recovering()) {
		wma_force_vdev_cleanup(wma_handle, vdev_id);
		goto send_rsp;
	}

	if (qdf_atomic_read(&iface->bss_status) == WMA_BSS_STATUS_STARTED) {
		req_msg = wma_find_vdev_req(wma_handle, vdev_id,
				WMA_TARGET_REQ_TYPE_VDEV_STOP, false);
		if (!req_msg)
			goto send_fail_rsp;
		if (req_msg->msg_type != WMA_DELETE_BSS_REQ)
			goto send_fail_rsp;
		WMA_LOGA("BSS is not yet stopped. Defering vdev(vdev id %x) deletion",
			vdev_id);
		iface->del_staself_req = pdel_sta_self_req_param;
		iface->is_del_sta_defered = true;
		return status;
	}
	iface->is_del_sta_defered = false;

	/* P2P Device */
	if ((iface->type == WMI_VDEV_TYPE_AP) &&
	    (iface->sub_type == WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE)) {
		wma_self_peer_remove(wma_handle, pdel_sta_self_req_param,
					generateRsp);
		if (!wmi_service_enabled(wma_handle->wmi_handle,
				wmi_service_sync_delete_cmds))
			status = wma_handle_vdev_detach(wma_handle,
				pdel_sta_self_req_param, generateRsp);
	} else {  /* other than P2P */
		status = wma_handle_vdev_detach(wma_handle,
				pdel_sta_self_req_param, generateRsp);
	}

	if (QDF_IS_STATUS_SUCCESS(status))
		iface->vdev_active = false;

	return status;

send_fail_rsp:
	if (!cds_is_driver_recovering()) {
		if (cds_is_self_recovery_enabled()) {
			WMA_LOGE("rcvd del_self_sta without del_bss, trigger recovery, vdev_id %d",
				 vdev_id);
			cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
		} else {
			WMA_LOGE("rcvd del_self_sta without del_bss, BUG_ON(), vdev_id %d",
				 vdev_id);
			QDF_BUG(0);
		}
	}
	status = QDF_STATUS_E_FAILURE;

send_rsp:
	if (generateRsp) {
		pdel_sta_self_req_param->status = status;
		wma_send_del_sta_self_resp(pdel_sta_self_req_param);
	} else {
		qdf_mem_free(pdel_sta_self_req_param);
		pdel_sta_self_req_param = NULL;
	}
	return status;
}

/**
 * wma_vdev_start_rsp() - send vdev start response to upper layer
 * @wma: wma handle
 * @add_bss: add bss params
 * @resp_event: response params
 *
 * Return: none
 */
static void wma_vdev_start_rsp(tp_wma_handle wma,
			       tpAddBssParams add_bss,
			       wmi_vdev_start_response_event_fixed_param *
			       resp_event)
{
	struct beacon_info *bcn;
	struct cdp_pdev *pdev;
	void *peer = NULL;
	uint8_t peer_id;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

#ifdef QCA_IBSS_SUPPORT
	WMA_LOGD("%s: vdev start response received for %s mode", __func__,
		 add_bss->operMode ==
		 BSS_OPERATIONAL_MODE_IBSS ? "IBSS" : "non-IBSS");
#endif /* QCA_IBSS_SUPPORT */

	if (resp_event->status) {
		add_bss->status = QDF_STATUS_E_FAILURE;
		goto send_fail_resp;
	}

	if ((add_bss->operMode == BSS_OPERATIONAL_MODE_AP)
#ifdef QCA_IBSS_SUPPORT
	    || (add_bss->operMode == BSS_OPERATIONAL_MODE_IBSS)
#endif /* QCA_IBSS_SUPPORT */
	    ) {
		wma->interfaces[resp_event->vdev_id].beacon =
			qdf_mem_malloc(sizeof(struct beacon_info));

		bcn = wma->interfaces[resp_event->vdev_id].beacon;
		if (!bcn) {
			WMA_LOGE("%s: Failed alloc memory for beacon struct",
				 __func__);
			add_bss->status = QDF_STATUS_E_NOMEM;
			goto send_fail_resp;
		}
		bcn->buf = qdf_nbuf_alloc(NULL, WMA_BCN_BUF_MAX_SIZE, 0,
					  sizeof(uint32_t), 0);
		if (!bcn->buf) {
			WMA_LOGE("%s: No memory allocated for beacon buffer",
				 __func__);
			qdf_mem_free(bcn);
			add_bss->status = QDF_STATUS_E_FAILURE;
			goto send_fail_resp;
		}
		bcn->seq_no = MIN_SW_SEQ;
		qdf_spinlock_create(&bcn->lock);
		qdf_atomic_set(&wma->interfaces[resp_event->vdev_id].bss_status,
			       WMA_BSS_STATUS_STARTED);
		WMA_LOGD("%s: AP mode (type %d subtype %d) BSS is started",
			 __func__, wma->interfaces[resp_event->vdev_id].type,
			 wma->interfaces[resp_event->vdev_id].sub_type);

		WMA_LOGD("%s: Allocated beacon struct %pK, template memory %pK",
			 __func__, bcn, bcn->buf);
	}
	add_bss->status = QDF_STATUS_SUCCESS;
	add_bss->bssIdx = resp_event->vdev_id;
	add_bss->chainMask = resp_event->chain_mask;
	if ((2 != resp_event->cfgd_rx_streams) ||
		(2 != resp_event->cfgd_tx_streams)) {
		add_bss->nss = 1;
	}
	add_bss->smpsMode = host_map_smps_mode(resp_event->smps_mode);
send_fail_resp:
	if (add_bss->status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: ADD BSS failure %d", __func__, add_bss->status);

		pdev = cds_get_context(QDF_MODULE_ID_TXRX);
		if (NULL == pdev)
			WMA_LOGE("%s: Failed to get pdev", __func__);

		if (pdev)
			peer = cdp_peer_find_by_addr(soc, pdev,
				add_bss->bssId, &peer_id);
		if (!peer)
			WMA_LOGE("%s Failed to find peer %pM", __func__,
				add_bss->bssId);

		if (peer)
			wma_remove_peer(wma, add_bss->bssId,
				resp_event->vdev_id, peer, false);
	}

	/* Send vdev stop if vdev start was success */
	if ((add_bss->status != QDF_STATUS_SUCCESS) &&
	   !resp_event->status)
		if (wma_send_vdev_stop_to_fw(wma, resp_event->vdev_id))
			WMA_LOGE("%s: %d Failed to send vdev stop", __func__, __LINE__);

	WMA_LOGD("%s: Sending add bss rsp to umac(vdev %d status %d)",
		 __func__, resp_event->vdev_id, add_bss->status);
	wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
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

static const wmi_channel_width mode_to_width[MODE_MAX] = {
	[MODE_11A]           = WMI_CHAN_WIDTH_20,
	[MODE_11G]           = WMI_CHAN_WIDTH_20,
	[MODE_11B]           = WMI_CHAN_WIDTH_20,
	[MODE_11GONLY]       = WMI_CHAN_WIDTH_20,
	[MODE_11NA_HT20]     = WMI_CHAN_WIDTH_20,
	[MODE_11NG_HT20]     = WMI_CHAN_WIDTH_20,
	[MODE_11AC_VHT20]    = WMI_CHAN_WIDTH_20,
	[MODE_11AC_VHT20_2G] = WMI_CHAN_WIDTH_20,
	[MODE_11NA_HT40]     = WMI_CHAN_WIDTH_40,
	[MODE_11NG_HT40]     = WMI_CHAN_WIDTH_40,
	[MODE_11AC_VHT40]    = WMI_CHAN_WIDTH_40,
	[MODE_11AC_VHT40_2G] = WMI_CHAN_WIDTH_40,
	[MODE_11AC_VHT80]    = WMI_CHAN_WIDTH_80,
	[MODE_11AC_VHT80_2G] = WMI_CHAN_WIDTH_80,
#if CONFIG_160MHZ_SUPPORT
	[MODE_11AC_VHT80_80] = WMI_CHAN_WIDTH_80P80,
	[MODE_11AC_VHT160]   = WMI_CHAN_WIDTH_160,
#endif

#if SUPPORT_11AX
	[MODE_11AX_HE20]     = WMI_CHAN_WIDTH_20,
	[MODE_11AX_HE40]     = WMI_CHAN_WIDTH_40,
	[MODE_11AX_HE80]     = WMI_CHAN_WIDTH_80,
	[MODE_11AX_HE80_80]  = WMI_CHAN_WIDTH_80P80,
	[MODE_11AX_HE160]    = WMI_CHAN_WIDTH_160,
	[MODE_11AX_HE20_2G]  = WMI_CHAN_WIDTH_20,
	[MODE_11AX_HE40_2G]  = WMI_CHAN_WIDTH_40,
	[MODE_11AX_HE80_2G]  = WMI_CHAN_WIDTH_80,
#endif
};

/**
 * chanmode_to_chanwidth() - get channel width through channel mode
 * @chanmode:   channel phy mode
 *
 * Return: channel width
 */
wmi_channel_width chanmode_to_chanwidth(WLAN_PHY_MODE chanmode)
{
	wmi_channel_width chan_width;

	if (chanmode >= MODE_11A && chanmode < MODE_MAX)
		chan_width = mode_to_width[chanmode];
	else
		chan_width = WMI_CHAN_WIDTH_20;

	return chan_width;
}

/**
 * wma_vdev_start_resp_handler() - vdev start response handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_vdev_start_resp_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_VDEV_START_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_start_response_event_fixed_param *resp_event;
	struct wma_target_req *req_msg;
	struct wma_txrx_node *iface;
	struct vdev_up_params param = {0};
	QDF_STATUS status;
	int err;
	wmi_channel_width chanwidth;
	target_resource_config *wlan_res_cfg;
	struct wlan_objmgr_psoc *psoc = wma->psoc;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	tpAniSirGlobal mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
#endif

	if (!psoc) {
		WMA_LOGE("%s: psoc is NULL", __func__);
		return -EINVAL;
	}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (NULL == mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		policy_mgr_set_do_hw_mode_change_flag(
			psoc, false);
		return -EINVAL;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	WMA_LOGD("%s: Enter", __func__);

	wlan_res_cfg = ucfg_get_tgt_res_cfg(psoc);
	if (!wlan_res_cfg) {
		WMA_LOGE("%s: Wlan resource config is NULL", __func__);
		return -EINVAL;
	}

	param_buf = (WMI_VDEV_START_RESP_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid start response event buffer");
		policy_mgr_set_do_hw_mode_change_flag(
			wma->psoc, false);
		return -EINVAL;
	}

	resp_event = param_buf->fixed_param;
	if (!resp_event) {
		WMA_LOGE("Invalid start response event buffer");
		policy_mgr_set_do_hw_mode_change_flag(
			wma->psoc, false);
		return -EINVAL;
	}

	if (resp_event->vdev_id >= wma->max_bssid) {
		WMA_LOGE("Invalid vdev id received from firmware");
		return -EINVAL;
	}

	if (wma_is_vdev_in_ap_mode(wma, resp_event->vdev_id))
		tgt_dfs_radar_enable(wma->pdev, 0, 0);

	if (resp_event->status == QDF_STATUS_SUCCESS) {
		wma->interfaces[resp_event->vdev_id].tx_streams =
			resp_event->cfgd_tx_streams;
		wma->interfaces[resp_event->vdev_id].rx_streams =
			resp_event->cfgd_rx_streams;
		wma->interfaces[resp_event->vdev_id].chain_mask =
			resp_event->chain_mask;
		if (wlan_res_cfg->use_pdev_id) {
			if (resp_event->pdev_id == WMI_PDEV_ID_SOC) {
				WMA_LOGE("%s: soc level id received for mac id",
					__func__);
				QDF_BUG(0);
				return -EINVAL;
			}
			wma->interfaces[resp_event->vdev_id].mac_id =
				WMA_PDEV_TO_MAC_MAP(resp_event->pdev_id);
		} else {
			wma->interfaces[resp_event->vdev_id].mac_id =
				resp_event->mac_id;
		}

		WMA_LOGD("%s: vdev:%d tx ss=%d rx ss=%d chain mask=%d mac=%d",
				__func__,
				resp_event->vdev_id,
				wma->interfaces[resp_event->vdev_id].tx_streams,
				wma->interfaces[resp_event->vdev_id].rx_streams,
				wma->interfaces[resp_event->vdev_id].chain_mask,
				wma->interfaces[resp_event->vdev_id].mac_id);
	}

	iface = &wma->interfaces[resp_event->vdev_id];

	req_msg = wma_find_vdev_req(wma, resp_event->vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START,
				    true);

	if (!req_msg) {
		WMA_LOGE("%s: Failed to lookup request message for vdev %d",
			 __func__, resp_event->vdev_id);
		policy_mgr_set_do_hw_mode_change_flag(wma->psoc, false);
		return -EINVAL;
	}

	if ((resp_event->vdev_id < wma->max_bssid) &&
	    (qdf_atomic_read(
	    &wma->interfaces[resp_event->vdev_id].vdev_restart_params.hidden_ssid_restart_in_progress))
	    && (wma_is_vdev_in_ap_mode(wma, resp_event->vdev_id) == true)) {
		tpHalHiddenSsidVdevRestart hidden_ssid_restart =
			(tpHalHiddenSsidVdevRestart)req_msg->user_data;
		WMA_LOGE("%s: vdev restart event recevied for hidden ssid set using IOCTL",
			__func__);

		param.vdev_id = resp_event->vdev_id;
		param.assoc_id = 0;
		qdf_atomic_set(&wma->interfaces[resp_event->vdev_id].
			       vdev_restart_params.
			       hidden_ssid_restart_in_progress, 0);

		wma_send_msg(wma, WMA_HIDDEN_SSID_RESTART_RSP,
				(void *)hidden_ssid_restart, 0);
		/*
		 * Unpause TX queue in SAP case while configuring hidden ssid
		 * enable or disable, else the data path is paused forever
		 * causing data packets(starting from DHCP offer) to get stuck
		 */
		cdp_fc_vdev_unpause(cds_get_context(QDF_MODULE_ID_SOC),
				iface->handle,
				OL_TXQ_PAUSE_REASON_VDEV_STOP);
		wma_vdev_clear_pause_bit(resp_event->vdev_id, PAUSE_TYPE_HOST);
	}

	qdf_mc_timer_stop(&req_msg->event_timeout);

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (resp_event->status == QDF_STATUS_SUCCESS
		&& mac_ctx->sap.sap_channel_avoidance)
		wma_find_mcc_ap(wma, resp_event->vdev_id, true);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	if (req_msg->msg_type == WMA_CHNL_SWITCH_REQ) {
		tpSwitchChannelParams params =
			(tpSwitchChannelParams) req_msg->user_data;

		if (!params) {
			WMA_LOGE("%s: channel switch params is NULL for vdev %d",
				__func__, resp_event->vdev_id);
			policy_mgr_set_do_hw_mode_change_flag(
				wma->psoc, false);
			return -EINVAL;
		}

		WMA_LOGD("%s: Send channel switch resp vdev %d status %d",
			 __func__, resp_event->vdev_id, resp_event->status);
		params->chainMask = resp_event->chain_mask;
		if ((2 != resp_event->cfgd_rx_streams) ||
			(2 != resp_event->cfgd_tx_streams)) {
			params->nss = 1;
		}
		params->smpsMode = host_map_smps_mode(resp_event->smps_mode);
		params->status = resp_event->status;
		if (wma->interfaces[resp_event->vdev_id].is_channel_switch) {
			wma->interfaces[resp_event->vdev_id].is_channel_switch =
				false;
		}
		if (((resp_event->resp_type == WMI_VDEV_RESTART_RESP_EVENT) &&
			((iface->type == WMI_VDEV_TYPE_STA) ||
				(iface->type == WMI_VDEV_TYPE_MONITOR))) ||
			((resp_event->resp_type == WMI_VDEV_START_RESP_EVENT) &&
			 (iface->type == WMI_VDEV_TYPE_MONITOR))) {
			chanwidth = chanmode_to_chanwidth(iface->chanmode);
			err = wma_set_peer_param(wma, iface->bssid,
					WMI_PEER_CHWIDTH, chanwidth,
					resp_event->vdev_id);
			WMA_LOGD("%s:vdev_id %d chanwidth %d status %d",
				__func__, resp_event->vdev_id,
				chanwidth, err);

			err = wma_set_peer_param(wma, iface->bssid,
					WMI_PEER_PHYMODE, iface->chanmode,
					resp_event->vdev_id);
			WMA_LOGD("%s:vdev_id %d chanmode %d status %d",
				__func__, resp_event->vdev_id,
				iface->chanmode, err);

			param.vdev_id = resp_event->vdev_id;
			param.assoc_id = iface->aid;
			status = wma_send_vdev_up_to_fw(wma, &param,
							iface->bssid);
			if (QDF_IS_STATUS_ERROR(status)) {
				WMA_LOGE("%s:vdev_up failed vdev_id %d",
					 __func__, resp_event->vdev_id);
				wma_vdev_set_mlme_state(wma,
					resp_event->vdev_id, WLAN_VDEV_S_STOP);
				policy_mgr_set_do_hw_mode_change_flag(
					wma->psoc, false);
			} else {
				wma_vdev_set_mlme_state(wma,
					resp_event->vdev_id, WLAN_VDEV_S_RUN);
				if (iface->beacon_filter_enabled)
					wma_add_beacon_filter(wma,
							&iface->beacon_filter);
			}
		}

		wma_send_msg_high_priority(wma, WMA_SWITCH_CHANNEL_RSP,
					   (void *)params, 0);
	} else if (req_msg->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams bssParams = (tpAddBssParams) req_msg->user_data;

		qdf_mem_copy(iface->bssid, bssParams->bssId,
				IEEE80211_ADDR_LEN);
		wma_vdev_start_rsp(wma, bssParams, resp_event);
	} else if (req_msg->msg_type == WMA_OCB_SET_CONFIG_CMD) {
		param.vdev_id = resp_event->vdev_id;
		param.assoc_id = iface->aid;
		if (wma_send_vdev_up_to_fw(wma, &param, iface->bssid) !=
		    QDF_STATUS_SUCCESS) {
			WMA_LOGE(FL("failed to send vdev up"));
			policy_mgr_set_do_hw_mode_change_flag(
				wma->psoc, false);
			return -EEXIST;
		}
		wma_vdev_set_mlme_state(wma, resp_event->vdev_id,
			WLAN_VDEV_S_RUN);
		ucfg_ocb_config_channel(wma->pdev);
	}

	if ((wma->interfaces[resp_event->vdev_id].type == WMI_VDEV_TYPE_AP) &&
		wma_is_vdev_up(resp_event->vdev_id))
		wma_set_sap_keepalive(wma, resp_event->vdev_id);

	qdf_mc_timer_destroy(&req_msg->event_timeout);
	qdf_mem_free(req_msg);

	return 0;
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

	WMA_LOGD("%s: vdev_id: %d, vdev_active: %d, is_vdev_valid %d",
		 __func__, vdev_id, wma_handle->interfaces[vdev_id].vdev_active,
		 wma_handle->interfaces[vdev_id].is_vdev_valid);

	return wma_handle->interfaces[vdev_id].vdev_active ||
		wma_handle->interfaces[vdev_id].is_vdev_valid;
}

/**
 * wma_vdev_set_param() - set per vdev params in fw
 * @wmi_handle: wmi handle
 * @if_if: vdev id
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

	param.if_id = if_id;
	param.param_id = param_id;
	param.param_value = param_value;

	return wmi_unified_vdev_set_param_send(wmi_handle, &param);
}

/**
 * wma_set_peer_authorized_cb() - set peer authorized callback function
 * @wma_Ctx: wma handle
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
	int err;

	param.vdev_id = vdev_id;
	param.param_value = param_value;
	param.param_id = param_id;

	err = wmi_set_peer_param_send(wma_handle->wmi_handle, peer_addr,
					   &param);

	return err;
}

/**
 * wma_remove_peer() - remove peer information from host driver and fw
 * @wma: wma handle
 * @bssid: mac address
 * @vdev_id: vdev id
 * @peer: peer ptr
 * @roam_synch_in_progress: roam in progress flag
 *
 * Return: none
 */
void wma_remove_peer(tp_wma_handle wma, uint8_t *bssid,
			    uint8_t vdev_id, void *peer,
			    bool roam_synch_in_progress)
{
#define PEER_ALL_TID_BITMASK 0xffffffff
	uint32_t peer_tid_bitmap = PEER_ALL_TID_BITMASK;
	uint8_t *peer_addr = bssid;
	struct peer_flush_params param = {0};
	uint8_t *peer_mac_addr;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	QDF_STATUS qdf_status;
	uint32_t bitmap = 1 << CDP_PEER_DELETE_NO_SPECIAL;

	if (!wma->interfaces[vdev_id].peer_count) {
		WMA_LOGE("%s: Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
			__func__, bssid, vdev_id,
			wma->interfaces[vdev_id].peer_count);
		return;
	}

	if (!soc) {
		WMA_LOGE("%s:SOC context is NULL", __func__);
		return;
	}

	peer_mac_addr = cdp_peer_get_peer_mac_addr(soc, peer);
	if (peer_mac_addr == NULL) {
		WMA_LOGE("%s: peer mac addr is NULL, Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
			 __func__, bssid, vdev_id,
			 wma->interfaces[vdev_id].peer_count);
		return;
	}

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
	} else {
		qdf_atomic_dec(&wma->interfaces[vdev_id].fw_peer_count);
		qdf_event_set(&wma->interfaces[vdev_id].fw_peer_delete);
		WMA_LOGD("%s: vdev-%d fw_peer_count %d", __func__, vdev_id,
			 qdf_atomic_read(&wma->interfaces[vdev_id].fw_peer_count));
	}

peer_detach:
	WMA_LOGE("%s: Remove peer %pK with peer_addr %pM vdevid %d peer_count %d",
		 __func__, peer, bssid, vdev_id,
		 wma->interfaces[vdev_id].peer_count);

	if (peer) {
		if (roam_synch_in_progress)
			cdp_peer_detach_force_delete(soc, peer);
		else
			cdp_peer_delete(soc, peer, bitmap);
	}

	wma->interfaces[vdev_id].peer_count--;
#undef PEER_ALL_TID_BITMASK
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
	uint8_t peer_id;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	for (i = 0; i < wma->max_bssid; i++) {
		/* Need to check vdevs other than the vdev_id */
		if (vdev_id == i ||
		   !wma->interfaces[i].handle)
			continue;
		if (cdp_peer_find_by_addr_and_vdev(soc, pdev,
			wma->interfaces[i].handle, peer_mac, &peer_id)) {
			WMA_LOGE("%s :Duplicate peer %pM (peer id %d) already exist on vdev %d",
				__func__, peer_mac, peer_id, i);
			return true;
		}
	}
	return false;
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
			  u8 peer_addr[IEEE80211_ADDR_LEN],
			  uint32_t peer_type, uint8_t vdev_id,
			  bool roam_synch_in_progress)
{
	void *peer = NULL;
	struct peer_create_params param = {0};
	uint8_t *mac_addr_raw;
	void *dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct wlan_objmgr_psoc *psoc = wma->psoc;
	target_resource_config *wlan_res_cfg;

	if (!psoc) {
		WMA_LOGE("%s: psoc is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	wlan_res_cfg = ucfg_get_tgt_res_cfg(psoc);
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

	/*
	 * Check if peer with same MAC exist on other Vdev, If so avoid
	 * adding this peer, as it will cause FW to crash.
	 */
	if (wma_find_duplicate_peer_on_other_vdev(wma, pdev,
	   vdev_id, peer_addr))
		goto err;

	/* The peer object should be created before sending the WMI peer
	 * create command to firmware. This is to prevent a race condition
	 * where the HTT peer map event is received before the peer object
	 * is created in the data path
	 */
	peer = cdp_peer_create(dp_soc, vdev, peer_addr);
	if (!peer) {
		WMA_LOGE("%s : Unable to attach peer %pM", __func__, peer_addr);
		goto err;
	}

	if (roam_synch_in_progress) {
		WMA_LOGI("%s: LFR3: Created peer %pK with peer_addr %pM vdev_id %d, peer_count - %d",
			 __func__, peer, peer_addr, vdev_id,
			 wma->interfaces[vdev_id].peer_count);
		return QDF_STATUS_SUCCESS;
	}
	param.peer_addr = peer_addr;
	param.peer_type = peer_type;
	param.vdev_id = vdev_id;
	if (wmi_unified_peer_create_send(wma->wmi_handle,
					 &param) != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s : Unable to create peer in Target", __func__);
		cdp_peer_delete(dp_soc, peer,
				1 << CDP_PEER_DO_NOT_START_UNMAP_TIMER);
		goto err;
	} else {
		qdf_atomic_inc(&wma->interfaces[vdev_id].fw_peer_count);
		WMA_LOGD("%s: vdev-%d fw_peer_count %d", __func__, vdev_id,
			 qdf_atomic_read(&wma->interfaces[vdev_id].fw_peer_count));
	}

	WMA_LOGD("%s: Created peer %pK with peer_addr %pM vdev_id %d, peer_count - %d",
		  __func__, peer, peer_addr, vdev_id,
		  wma->interfaces[vdev_id].peer_count);

	wlan_roam_debug_log(vdev_id, DEBUG_PEER_CREATE_SEND,
			    DEBUG_INVALID_PEER_ID, peer_addr, peer, 0, 0);
	cdp_peer_setup(dp_soc, vdev, peer);

	WMA_LOGD("%s: Initialized peer with peer_addr %pM vdev_id %d",
		__func__, peer_addr, vdev_id);

	mac_addr_raw = cdp_get_vdev_mac_addr(dp_soc, vdev);
	if (mac_addr_raw == NULL) {
		WMA_LOGE("%s: peer mac addr is NULL", __func__);
		return QDF_STATUS_E_FAULT;
	}

	/* for each remote ibss peer, clear its keys */
	if (wma_is_vdev_in_ibss_mode(wma, vdev_id) &&
	    qdf_mem_cmp(peer_addr, mac_addr_raw, IEEE80211_ADDR_LEN)) {
		tSetStaKeyParams key_info;

		WMA_LOGD("%s: remote ibss peer %pM key clearing\n", __func__,
			 peer_addr);
		qdf_mem_set(&key_info, sizeof(key_info), 0);
		key_info.smesessionId = vdev_id;
		qdf_mem_copy(key_info.peer_macaddr.bytes, peer_addr,
				IEEE80211_ADDR_LEN);
		key_info.sendRsp = false;

		wma_set_stakey(wma, &key_info);
	}

	return QDF_STATUS_SUCCESS;
err:
	wma->interfaces[vdev_id].peer_count--;
	return QDF_STATUS_E_FAILURE;
}

/**
 * wma_hidden_ssid_vdev_restart_on_vdev_stop() - restart vdev to set hidden ssid
 * @wma_handle: wma handle
 * @sessionId: session id
 *
 * Return: none
 */
static void wma_hidden_ssid_vdev_restart_on_vdev_stop(tp_wma_handle wma_handle,
						      uint8_t sessionId)
{
	struct wma_txrx_node *intr = wma_handle->interfaces;
	struct hidden_ssid_vdev_restart_params params;
	QDF_STATUS status;

	params.session_id = sessionId;
	params.ssid_len = intr[sessionId].vdev_restart_params.ssid.ssid_len;
	qdf_mem_copy(params.ssid,
		     intr[sessionId].vdev_restart_params.ssid.ssid,
		     params.ssid_len);
	params.flags = intr[sessionId].vdev_restart_params.flags;
	if (intr[sessionId].vdev_restart_params.ssidHidden)
		params.flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;
	else
		params.flags &= (0xFFFFFFFE);
	params.requestor_id = intr[sessionId].vdev_restart_params.requestor_id;
	params.disable_hw_ack =
		intr[sessionId].vdev_restart_params.disable_hw_ack;

	params.mhz = intr[sessionId].vdev_restart_params.chan.mhz;
	params.band_center_freq1 =
		intr[sessionId].vdev_restart_params.chan.band_center_freq1;
	params.band_center_freq2 =
		intr[sessionId].vdev_restart_params.chan.band_center_freq2;
	params.info = intr[sessionId].vdev_restart_params.chan.info;
	params.reg_info_1 = intr[sessionId].vdev_restart_params.chan.reg_info_1;
	params.reg_info_2 = intr[sessionId].vdev_restart_params.chan.reg_info_2;

	status = wmi_unified_hidden_ssid_vdev_restart_send(
			wma_handle->wmi_handle,	&params);
	if (status == QDF_STATUS_E_FAILURE) {
		WMA_LOGE("%s: Failed to send vdev restart command", __func__);
		qdf_atomic_set(&intr[sessionId].vdev_restart_params.
			       hidden_ssid_restart_in_progress, 0);
	}
}

/**
 * wma_cleanup_target_req_param() - free param memory of target request
 * @tgt_req: target request params
 *
 * Return: none
 */
static void wma_cleanup_target_req_param(struct wma_target_req *tgt_req)
{
	if (tgt_req->msg_type == WMA_CHNL_SWITCH_REQ ||
	   tgt_req->msg_type == WMA_DELETE_BSS_REQ ||
	   tgt_req->msg_type == WMA_ADD_BSS_REQ) {
		qdf_mem_free(tgt_req->user_data);
		tgt_req->user_data = NULL;
	}

	if (tgt_req->msg_type == WMA_SET_LINK_STATE && tgt_req->user_data) {
		tpLinkStateParams params =
			(tpLinkStateParams) tgt_req->user_data;
		qdf_mem_free(params->callbackArg);
		qdf_mem_free(tgt_req->user_data);
		tgt_req->user_data = NULL;
	}
}

/**
 * wma_remove_bss_peer() - remove BSS peer
 * @wma: pointer to WMA handle
 * @pdev: pointer to PDEV
 * @req_msg: pointer to WMA target Request
 * @vdev_id: vdev id on which delete BSS request was received
 * @params: pointer to Delete BSS params
 *
 * This function is called on receiving vdev stop response from FW or
 * vdev stop response timeout. In case of IBSS/NDI, use vdev's self MAC
 * for removing the peer. In case of STA/SAP use bssid passed as part of
 * delete STA parameter.
 *
 * Return: 0 on success, ERROR code on failure
 */
static int wma_remove_bss_peer(tp_wma_handle wma, void *pdev,
		struct wma_target_req *req_msg, uint32_t vdev_id,
		tpDeleteBssParams params)
{
	void *peer, *vdev;
	uint8_t peer_id;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t *mac_addr = NULL;
	struct wma_target_req *del_req;

	vdev = cdp_get_vdev_from_vdev_id(soc, pdev, vdev_id);
	if (!vdev) {
		WMA_LOGE(FL("vdev is NULL for vdev_id = %d"), vdev_id);
		wma_cleanup_target_req_param(req_msg);
		return -EINVAL;
	}

	if (wma_is_vdev_in_ibss_mode(wma, vdev_id) ||
	    WMA_IS_VDEV_IN_NDI_MODE(wma->interfaces, vdev_id)) {
		mac_addr = cdp_get_vdev_mac_addr(soc, vdev);
		if (!mac_addr) {
			WMA_LOGE(FL("mac_addr is NULL for vdev_id = %d"),
				 vdev_id);
			wma_cleanup_target_req_param(req_msg);
			return -EINVAL;
		}
	} else {
		mac_addr = params->bssid;
	}

	peer = cdp_peer_find_by_addr(soc, pdev, mac_addr, &peer_id);
	if (!peer) {
		WMA_LOGE(FL("peer NULL for vdev_id = %d"), vdev_id);
		wma_cleanup_target_req_param(req_msg);
		return -EINVAL;
	}

	wma_remove_peer(wma, mac_addr, vdev_id, peer, false);
	if (wmi_service_enabled(
	   wma->wmi_handle,
	   wmi_service_sync_delete_cmds)) {
		WMA_LOGD(FL("Wait for the peer delete. vdev_id %d"),
				 req_msg->vdev_id);
		del_req = wma_fill_hold_req(wma,
				   req_msg->vdev_id,
				   WMA_DELETE_STA_REQ,
				   WMA_DELETE_PEER_RSP,
				   params,
				   WMA_DELETE_STA_TIMEOUT);
		if (!del_req) {
			WMA_LOGE(FL("Failed to allocate request. vdev_id %d"),
				 req_msg->vdev_id);
			params->status = QDF_STATUS_E_NOMEM;
			return -EINVAL;
		}
	}
	return 0;
}

/*
 * get_fw_active_bpf_mode() - convert HDD BPF mode to FW configurable BPF
 * mode
 * @mode: BPF mode maintained in HDD
 *
 * Return: FW configurable BP mode
 */
static enum wmi_host_active_bpf_mode
get_fw_active_bpf_mode(enum active_bpf_mode mode)
{
	switch (mode) {
	case ACTIVE_BPF_DISABLED:
		return WMI_HOST_ACTIVE_BPF_DISABLED;
	case ACTIVE_BPF_ENABLED:
		return WMI_HOST_ACTIVE_BPF_ENABLED;
	case ACTIVE_BPF_ADAPTIVE:
		return WMI_HOST_ACTIVE_BPF_ADAPTIVE;
	default:
		WMA_LOGE("Invalid Active BPF Mode %d; Using 'disabled'", mode);
		return WMI_HOST_ACTIVE_BPF_DISABLED;
	}
}

/**
 * wma_config_active_bpf_mode() - Config active BPF mode in FW
 * @wma: the WMA handle
 * @vdev_id: the Id of the vdev for which the configuration should be applied
 *
 * Return: QDF status
 */
static QDF_STATUS wma_config_active_bpf_mode(t_wma_handle *wma, uint8_t vdev_id)
{
	enum wmi_host_active_bpf_mode uc_mode, mcbc_mode;

	uc_mode = get_fw_active_bpf_mode(wma->active_uc_bpf_mode);
	mcbc_mode = get_fw_active_bpf_mode(wma->active_mc_bc_bpf_mode);

	WMA_LOGD("Configuring Active BPF Mode UC:%d MC/BC:%d for vdev %u",
		  uc_mode, mcbc_mode, vdev_id);

	return wmi_unified_set_active_bpf_mode_cmd(wma->wmi_handle, vdev_id,
						   uc_mode, mcbc_mode);
}

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
	tpAniSirGlobal mac_ctx = cds_get_context(QDF_MODULE_ID_PE);

	if (NULL == mac_ctx) {
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

/**
 * wma_send_del_bss_response() - send del bss resp to upper layer
 * @wma: wma handle.
 * @vdev_id: vdev ID of device for which MCC has to be checked
 *
 * This function sends del bss resp to upper layer
 *
 * Return: none
 */
static void
wma_send_del_bss_response(tp_wma_handle wma, struct wma_target_req *req,
	uint8_t vdev_id)
{
	struct wma_txrx_node *iface;
	struct beacon_info *bcn;
	tpDeleteBssParams params;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!req) {
		WMA_LOGE("%s req is NULL", __func__);
		return;
	}

	iface = &wma->interfaces[vdev_id];
	if (!iface->handle) {
		WMA_LOGE("%s vdev id %d is already deleted",
			 __func__, vdev_id);
		if (req->user_data)
			qdf_mem_free(req->user_data);
		req->user_data = NULL;
		return;
	}

	params = (tpDeleteBssParams)req->user_data;
	if (wma_send_vdev_down_to_fw(wma, vdev_id) != QDF_STATUS_SUCCESS) {
		WMA_LOGE("Failed to send vdev down cmd: vdev %d", vdev_id);
	} else {
		wma_vdev_set_mlme_state(wma, vdev_id, WLAN_VDEV_S_STOP);
		wma_check_and_find_mcc_ap(wma, vdev_id);
	}

	cdp_fc_vdev_flush(soc, iface->handle);
	WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for VDEV_STOP rsp",
		 __func__, vdev_id);
	cdp_fc_vdev_unpause(soc, iface->handle,
		OL_TXQ_PAUSE_REASON_VDEV_STOP);
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
	if (params->status == QDF_STATUS_FW_MSG_TIMEDOUT) {
		qdf_mem_free(req->user_data);
		req->user_data = NULL;
		WMA_LOGE("%s: DEL BSS from ADD BSS timeout do not send resp to UMAC (vdev id %x)",
			 __func__, vdev_id);
	} else {
		params->status = QDF_STATUS_SUCCESS;
		wma_send_msg_high_priority(wma, WMA_DELETE_BSS_RSP,
					   (void *)params, 0);
	}

	if (iface->del_staself_req && iface->is_del_sta_defered) {
		iface->is_del_sta_defered = false;
		WMA_LOGA("scheduling defered deletion (vdev id %x)",
			 vdev_id);
		wma_vdev_detach(wma, iface->del_staself_req, 1);
	}
}


/**
 * wma_vdev_stop_resp_handler() - vdev stop response handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_vdev_stop_resp_handler(void *handle, uint8_t *cmd_param_info,
			       u32 len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_VDEV_STOPPED_EVENTID_param_tlvs *param_buf;
	wmi_vdev_stopped_event_fixed_param *resp_event;
	struct wma_target_req *req_msg, *del_req, *new_req_msg;
	struct cdp_pdev *pdev;
	void *peer;
	uint8_t peer_id;
	struct wma_txrx_node *iface;
	int32_t status = 0;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("%s: Enter", __func__);

	param_buf = (WMI_VDEV_STOPPED_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid event buffer");
		return -EINVAL;
	}

	resp_event = param_buf->fixed_param;

	if (resp_event->vdev_id >= wma->max_bssid) {
		WMA_LOGE("%s: Invalid vdev_id %d from FW",
				__func__, resp_event->vdev_id);
		return -EINVAL;
	}

	iface = &wma->interfaces[resp_event->vdev_id];
	wma_release_wakelock(&iface->vdev_stop_wakelock);

	req_msg = wma_find_vdev_req(wma, resp_event->vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_STOP, true);
	if (!req_msg) {
		WMA_LOGE("%s: Failed to lookup vdev request for vdev id %d",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}

	if ((qdf_atomic_read
		     (&wma->interfaces[resp_event->vdev_id].vdev_restart_params.
		     hidden_ssid_restart_in_progress))
	    && ((wma->interfaces[resp_event->vdev_id].type == WMI_VDEV_TYPE_AP)
		&& (wma->interfaces[resp_event->vdev_id].sub_type == 0))) {
		WMA_LOGE("%s: vdev stop event recevied for hidden ssid set using IOCTL ",
			__func__);

		wma_vdev_set_mlme_state(wma,
				resp_event->vdev_id, WLAN_VDEV_S_STOP);
		new_req_msg = wma_fill_vdev_req(wma, resp_event->vdev_id,
				WMA_HIDDEN_SSID_VDEV_RESTART,
				WMA_TARGET_REQ_TYPE_VDEV_START,
				req_msg->user_data,
				WMA_VDEV_START_REQUEST_TIMEOUT);
		if (!new_req_msg) {
			WMA_LOGE("%s: Failed to fill vdev request, vdev_id %d",
					__func__, resp_event->vdev_id);
			return -EINVAL;
		}

		wma_hidden_ssid_vdev_restart_on_vdev_stop(wma,
							  resp_event->vdev_id);
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is NULL", __func__);
		status = -EINVAL;
		wma_cleanup_target_req_param(req_msg);
		qdf_mc_timer_stop(&req_msg->event_timeout);
		goto free_req_msg;
	}

	qdf_mc_timer_stop(&req_msg->event_timeout);
	if (req_msg->msg_type == WMA_DELETE_BSS_REQ) {
		tpDeleteBssParams params =
			(tpDeleteBssParams) req_msg->user_data;

		if (iface->handle == NULL) {
			WMA_LOGE("%s vdev id %d is already deleted",
				 __func__, resp_event->vdev_id);
			wma_cleanup_target_req_param(req_msg);
			status = -EINVAL;
			goto free_req_msg;
		}

		/* CCA is required only for sta interface */
		if (iface->type == WMI_VDEV_TYPE_STA)
			wma_get_cca_stats(wma, resp_event->vdev_id);

		/* Clear arp and ns offload cache */
		qdf_mem_zero(&iface->ns_offload_req,
			sizeof(iface->ns_offload_req));
		qdf_mem_zero(&iface->arp_offload_req,
			sizeof(iface->arp_offload_req));

		status = wma_remove_bss_peer(wma, pdev, req_msg,
					     resp_event->vdev_id, params);
		if (status != 0)
			goto free_req_msg;

		if (wmi_service_enabled(
		   wma->wmi_handle,
		   wmi_service_sync_delete_cmds))
			goto free_req_msg;

		wma_send_del_bss_response(wma, req_msg, resp_event->vdev_id);
	} else if (req_msg->msg_type == WMA_SET_LINK_STATE) {
		tpLinkStateParams params =
			(tpLinkStateParams) req_msg->user_data;

		peer = cdp_peer_find_by_addr(soc, pdev, params->bssid, &peer_id);
		if (peer) {
			WMA_LOGP(FL("Deleting peer %pM vdev id %d"),
				 params->bssid, req_msg->vdev_id);
			wma_remove_peer(wma, params->bssid, req_msg->vdev_id,
				peer, false);
			if (wmi_service_enabled(wma->wmi_handle,
				    wmi_service_sync_delete_cmds)) {
				WMA_LOGI(FL("Wait for the peer delete. vdev_id %d"),
						 req_msg->vdev_id);
				del_req = wma_fill_hold_req(wma,
						   req_msg->vdev_id,
						   WMA_DELETE_STA_REQ,
						   WMA_SET_LINK_PEER_RSP,
						   params,
						   WMA_DELETE_STA_TIMEOUT);
				if (!del_req) {
					WMA_LOGE(FL("Failed to allocate request. vdev_id %d"),
						 req_msg->vdev_id);
					params->status = QDF_STATUS_E_NOMEM;
				} else {
					goto free_req_msg;
				}
			}
		}
		if (wma_send_vdev_down_to_fw(wma, req_msg->vdev_id) !=
		    QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
				req_msg->vdev_id);
		}
		wma_send_msg(wma, WMA_SET_LINK_STATE_RSP, (void *)params, 0);
	}

free_req_msg:
	qdf_mc_timer_destroy(&req_msg->event_timeout);
	qdf_mem_free(req_msg);
	return status;
}

/**
 * wma_vdev_attach() - create vdev in fw
 * @wma_handle: wma handle
 * @self_sta_req: self sta request
 * @generateRsp: generate response
 *
 * This function creates vdev in target and
 * attach this vdev to txrx module. It also set
 * vdev related params to fw.
 *
 * Return: txrx vdev handle
 */
struct cdp_vdev *wma_vdev_attach(tp_wma_handle wma_handle,
				struct add_sta_self_params *self_sta_req,
				uint8_t generateRsp)
{
	struct cdp_vdev *txrx_vdev_handle = NULL;
	struct cdp_pdev *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	enum wlan_op_mode txrx_vdev_type;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct sAniSirGlobal *mac = cds_get_context(QDF_MODULE_ID_PE);
	uint32_t cfg_val;
	uint16_t val16;
	QDF_STATUS ret;
	tSirMacHTCapabilityInfo *phtCapInfo;
	struct scheduler_msg sme_msg = { 0 };
	struct vdev_create_params params = { 0 };
	u_int8_t vdev_id;
	struct sir_set_tx_rx_aggregation_size tx_rx_aggregation_size;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("mac %pM, vdev_id %hu, type %d, sub_type %d, nss 2g %d, 5g %d",
		self_sta_req->self_mac_addr, self_sta_req->session_id,
		self_sta_req->type, self_sta_req->sub_type,
		self_sta_req->nss_2g, self_sta_req->nss_5g);
	if (NULL == mac) {
		WMA_LOGE("%s: Failed to get mac", __func__);
		goto end;
	}

	vdev_id = self_sta_req->session_id;
	if (wma_is_vdev_valid(vdev_id)) {
		WMA_LOGE("%s: vdev %d already active", __func__, vdev_id);
		goto end;
	}

	params.if_id = self_sta_req->session_id;
	params.type = self_sta_req->type;
	params.subtype = self_sta_req->sub_type;
	params.nss_2g = self_sta_req->nss_2g;
	params.nss_5g = self_sta_req->nss_5g;

	/* Create a vdev in target */
	status = wmi_unified_vdev_create_send(wma_handle->wmi_handle,
						self_sta_req->self_mac_addr,
						&params);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Unable to add an interface for ath_dev",
			 __func__);
		goto end;
	}

	txrx_vdev_type = wma_get_txrx_vdev_type(self_sta_req->type);

	if (wlan_op_mode_unknown == txrx_vdev_type) {
		WMA_LOGE("Failed to get txrx vdev type");
		wmi_unified_vdev_delete_send(wma_handle->wmi_handle,
					     self_sta_req->session_id);
		goto end;
	}

	txrx_vdev_handle = cdp_vdev_attach(soc,
					txrx_pdev,
					self_sta_req->self_mac_addr,
					self_sta_req->session_id,
					txrx_vdev_type);
	wma_vdev_update_pause_bitmap(self_sta_req->session_id, 0);

	WMA_LOGD("vdev_id %hu, txrx_vdev_handle = %pK", self_sta_req->session_id,
		 txrx_vdev_handle);

	if (NULL == txrx_vdev_handle) {
		WMA_LOGE("%s: cdp_vdev_attach failed", __func__);
		status = QDF_STATUS_E_FAILURE;
		wmi_unified_vdev_delete_send(wma_handle->wmi_handle,
					     self_sta_req->session_id);
		goto end;
	}
	wma_handle->interfaces[self_sta_req->session_id].vdev_active = true;

	wma_handle->interfaces[self_sta_req->session_id].handle =
		txrx_vdev_handle;

	wma_handle->interfaces[self_sta_req->session_id].ptrn_match_enable =
		wma_handle->ptrn_match_enable_all_vdev ? true : false;

	if (wlan_cfg_get_int(mac, WNI_CFG_WOWLAN_DEAUTH_ENABLE, &cfg_val)
	    != eSIR_SUCCESS)
		wma_handle->wow.deauth_enable = true;
	else
		wma_handle->wow.deauth_enable = cfg_val ? true : false;

	if (wlan_cfg_get_int(mac, WNI_CFG_WOWLAN_DISASSOC_ENABLE, &cfg_val)
	    != eSIR_SUCCESS)
		wma_handle->wow.disassoc_enable = true;
	else
		wma_handle->wow.disassoc_enable = cfg_val ? true : false;

	if (wlan_cfg_get_int(mac, WNI_CFG_WOWLAN_MAX_MISSED_BEACON, &cfg_val)
	    != eSIR_SUCCESS)
		wma_handle->wow.bmiss_enable = true;
	else
		wma_handle->wow.bmiss_enable = cfg_val ? true : false;

	qdf_mem_copy(wma_handle->interfaces[self_sta_req->session_id].addr,
		     self_sta_req->self_mac_addr,
		     sizeof(wma_handle->interfaces[self_sta_req->session_id].
			    addr));

	tx_rx_aggregation_size.tx_aggregation_size =
				self_sta_req->tx_aggregation_size;
	tx_rx_aggregation_size.rx_aggregation_size =
				self_sta_req->rx_aggregation_size;
	tx_rx_aggregation_size.vdev_id = self_sta_req->session_id;

	status = wma_set_tx_rx_aggregation_size(&tx_rx_aggregation_size);
	if (status != QDF_STATUS_SUCCESS)
		WMA_LOGE("failed to set aggregation sizes(err=%d)", status);

	switch (self_sta_req->type) {
	case WMI_VDEV_TYPE_STA:
		if (wlan_cfg_get_int(mac, WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD,
				     &cfg_val) != eSIR_SUCCESS) {
			WMA_LOGE("Failed to get value for WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD");
			cfg_val = DEFAULT_INFRA_STA_KEEP_ALIVE_PERIOD;
		}

		wma_set_sta_keep_alive(wma_handle,
				       self_sta_req->session_id,
				       SIR_KEEP_ALIVE_NULL_PKT,
				       cfg_val, NULL, NULL, NULL);

		/* offload STA SA query related params to fwr */
		if (wmi_service_enabled(wma_handle->wmi_handle,
			wmi_service_sta_pmf_offload)) {
			wma_set_sta_sa_query_param(wma_handle,
						   self_sta_req->session_id);
		}
		break;
	}

	wma_handle->interfaces[self_sta_req->session_id].type =
		self_sta_req->type;
	wma_handle->interfaces[self_sta_req->session_id].sub_type =
		self_sta_req->sub_type;
	qdf_atomic_init(&wma_handle->interfaces
			[self_sta_req->session_id].bss_status);
	qdf_atomic_init(&wma_handle->interfaces
			[self_sta_req->session_id].fw_peer_count);
	status = qdf_event_create(&wma_handle->interfaces
				  [self_sta_req->session_id].fw_peer_delete);
	if (status != QDF_STATUS_SUCCESS)
		WMA_LOGE("%s: Failed to create fw_peer_delete event", __func__);

	if (((self_sta_req->type == WMI_VDEV_TYPE_AP) &&
	    (self_sta_req->sub_type == WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE)) ||
	    (self_sta_req->type == WMI_VDEV_TYPE_OCB) ||
	    (self_sta_req->type == WMI_VDEV_TYPE_MONITOR) ||
	    (self_sta_req->type == WMI_VDEV_TYPE_NDI)) {
		status = wma_create_peer(wma_handle, txrx_pdev,
					 txrx_vdev_handle,
					 self_sta_req->self_mac_addr,
					 WMI_PEER_TYPE_DEFAULT,
					 self_sta_req->session_id, false);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to create peer", __func__);
			status = QDF_STATUS_E_FAILURE;
			wmi_unified_vdev_delete_send(wma_handle->wmi_handle,
						     self_sta_req->session_id);
		}
	}

	WMA_LOGD("Setting WMI_VDEV_PARAM_DISCONNECT_TH: %d",
		self_sta_req->pkt_err_disconn_th);
	ret = wma_vdev_set_param(wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_DISCONNECT_TH,
				self_sta_req->pkt_err_disconn_th);
	if (ret)
		WMA_LOGE("Failed to set WMI_VDEV_PARAM_DISCONNECT_TH");

	wma_handle->interfaces[vdev_id].is_vdev_valid = true;
	ret = wma_vdev_set_param(wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_MCC_RTSCTS_PROTECTION_ENABLE,
				mac->roam.configParam.mcc_rts_cts_prot_enable);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to set WMI VDEV MCC_RTSCTS_PROTECTION_ENABLE");

	ret = wma_vdev_set_param(wma_handle->wmi_handle,
			self_sta_req->session_id,
			WMI_VDEV_PARAM_MCC_BROADCAST_PROBE_ENABLE,
			mac->roam.configParam.mcc_bcast_prob_resp_enable);
	if (QDF_IS_STATUS_ERROR(ret))
		WMA_LOGE("Failed to set WMI VDEV MCC_BROADCAST_PROBE_ENABLE");

	if (wlan_cfg_get_int(mac, WNI_CFG_RTS_THRESHOLD,
			     &cfg_val) == eSIR_SUCCESS) {
		ret = wma_vdev_set_param(wma_handle->wmi_handle,
					self_sta_req->session_id,
					WMI_VDEV_PARAM_RTS_THRESHOLD,
					cfg_val);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_RTS_THRESHOLD");
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_RTS_THRESHOLD, leaving unchanged");
	}

	if (wlan_cfg_get_int(mac, WNI_CFG_FRAGMENTATION_THRESHOLD,
			     &cfg_val) == eSIR_SUCCESS) {
		ret = wma_vdev_set_param(wma_handle->wmi_handle,
					self_sta_req->session_id,
					WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD,
					cfg_val);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD");
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_FRAGMENTATION_THRESHOLD, leaving unchanged");
	}

	if (wlan_cfg_get_int(mac, WNI_CFG_HT_CAP_INFO, &cfg_val) ==
								 eSIR_SUCCESS) {
		val16 = (uint16_t) cfg_val;
		phtCapInfo = (tSirMacHTCapabilityInfo *) &cfg_val;

		ret = wma_vdev_set_param(wma_handle->wmi_handle,
						      self_sta_req->session_id,
						      WMI_VDEV_PARAM_TX_STBC,
						      phtCapInfo->txSTBC);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_TX_STBC");
	} else {
		WMA_LOGE("Failed to get value of HT_CAP, TX STBC unchanged");
	}

	wma_set_vdev_mgmt_rate(wma_handle, self_sta_req->session_id);

	/* Initialize roaming offload state */
	if ((self_sta_req->type == WMI_VDEV_TYPE_STA) &&
	    (self_sta_req->sub_type == 0)) {
		wma_handle->roam_offload_enabled = true;
		ret = wma_vdev_set_param(wma_handle->wmi_handle,
					self_sta_req->session_id,
					WMI_VDEV_PARAM_ROAM_FW_OFFLOAD,
					(WMI_ROAM_FW_OFFLOAD_ENABLE_FLAG |
					WMI_ROAM_BMISS_FINAL_SCAN_ENABLE_FLAG));
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_ROAM_FW_OFFLOAD");

		/* Pass down enable/disable bcast probe rsp to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_ENABLE_BCAST_PROBE_RESPONSE,
				self_sta_req->enable_bcast_probe_rsp);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_ENABLE_BCAST_PROBE_RESPONSE");

		/* Pass down the FILS max channel guard time to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_FILS_MAX_CHANNEL_GUARD_TIME,
				self_sta_req->fils_max_chan_guard_time);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_FILS_MAX_CHANNEL_GUARD_TIME");

		/* Pass down the Probe Request tx delay(in ms) to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_PROBE_DELAY,
				PROBE_REQ_TX_DELAY);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_PROBE_DELAY");

		/* Pass down the probe request tx time gap(in ms) to FW */
		ret = wma_vdev_set_param(
				wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_REPEAT_PROBE_TIME,
				PROBE_REQ_TX_TIME_GAP);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_REPEAT_PROBE_TIME");
	}

	if ((self_sta_req->type == WMI_VDEV_TYPE_STA ||
	     self_sta_req->type == WMI_VDEV_TYPE_AP) &&
	    self_sta_req->sub_type == 0) {
		ret = wma_vdev_set_param(wma_handle->wmi_handle,
				     self_sta_req->session_id,
				     WMI_VDEV_PARAM_ENABLE_DISABLE_OCE_FEATURES,
				     self_sta_req->oce_feature_bitmap);
		if (QDF_IS_STATUS_ERROR(ret))
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_ENABLE_DISABLE_OCE_FEATURES");
	}

	/* Initialize BMISS parameters */
	if ((self_sta_req->type == WMI_VDEV_TYPE_STA) &&
	    (self_sta_req->sub_type == 0))
		wma_roam_scan_bmiss_cnt(wma_handle,
		mac->roam.configParam.neighborRoamConfig.nRoamBmissFirstBcnt,
		mac->roam.configParam.neighborRoamConfig.nRoamBmissFinalBcnt,
		self_sta_req->session_id);

	if (wlan_cfg_get_int(mac, WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED,
			     &cfg_val) == eSIR_SUCCESS) {
		WMA_LOGD("%s: setting ini value for WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED: %d",
			__func__, cfg_val);
		ret = wma_set_enable_disable_mcc_adaptive_scheduler(cfg_val);
		if (ret != QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to set WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED");
		}
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED, leaving unchanged");
	}

	if (self_sta_req->type == WMI_VDEV_TYPE_STA) {
		status = wma_config_active_bpf_mode(wma_handle,
						    self_sta_req->session_id);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("Failed to configure active BPF mode");
	}

end:
	self_sta_req->status = status;

#ifdef QCA_IBSS_SUPPORT
	if (generateRsp)
#endif
	{
		sme_msg.type = eWNI_SME_ADD_STA_SELF_RSP;
		sme_msg.bodyptr = self_sta_req;
		sme_msg.bodyval = 0;

		status = scheduler_post_msg(QDF_MODULE_ID_SME, &sme_msg);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			WMA_LOGE("Failed to post eWNI_SME_ADD_STA_SELF_RSP");
			qdf_mem_free(self_sta_req);
		}
	}
	return txrx_vdev_handle;
}

uint32_t wma_get_bcn_rate_code(uint16_t rate)
{
	/* rate in multiples of 100 Kbps */
	switch (rate) {
	case WMA_BEACON_TX_RATE_1_M:
		return WMI_BCN_TX_RATE_CODE_1_M;
	case WMA_BEACON_TX_RATE_2_M:
		return WMI_BCN_TX_RATE_CODE_2_M;
	case WMA_BEACON_TX_RATE_5_5_M:
		return WMI_BCN_TX_RATE_CODE_5_5_M;
	case WMA_BEACON_TX_RATE_11_M:
		return WMI_BCN_TX_RATE_CODE_11M;
	case WMA_BEACON_TX_RATE_6_M:
		return WMI_BCN_TX_RATE_CODE_6_M;
	case WMA_BEACON_TX_RATE_9_M:
		return WMI_BCN_TX_RATE_CODE_9_M;
	case WMA_BEACON_TX_RATE_12_M:
		return WMI_BCN_TX_RATE_CODE_12_M;
	case WMA_BEACON_TX_RATE_18_M:
		return WMI_BCN_TX_RATE_CODE_18_M;
	case WMA_BEACON_TX_RATE_24_M:
		return WMI_BCN_TX_RATE_CODE_24_M;
	case WMA_BEACON_TX_RATE_36_M:
		return WMI_BCN_TX_RATE_CODE_36_M;
	case WMA_BEACON_TX_RATE_48_M:
		return WMI_BCN_TX_RATE_CODE_48_M;
	case WMA_BEACON_TX_RATE_54_M:
		return WMI_BCN_TX_RATE_CODE_54_M;
	default:
		return WMI_BCN_TX_RATE_CODE_1_M;
	}
}

/**
 * wma_vdev_start() - send vdev start request to fw
 * @wma: wma handle
 * @req: vdev start params
 * @isRestart: isRestart flag
 *
 * Return: QDF status
 */
QDF_STATUS wma_vdev_start(tp_wma_handle wma,
			  struct wma_vdev_start_req *req, bool isRestart)
{
	struct vdev_start_params params = { 0 };
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	struct wma_txrx_node *intr = wma->interfaces;
	tpAniSirGlobal mac_ctx = NULL;
	uint32_t temp_ssid_len = 0;
	uint32_t temp_flags = 0;
	uint32_t temp_chan_info = 0;
	uint32_t temp_reg_info_1 = 0;
	uint32_t temp_reg_info_2 = 0;
	uint16_t bw_val;
	struct wma_txrx_node *iface = &wma->interfaces[req->vdev_id];
	struct wma_target_req *req_msg;
	uint32_t chan_mode;
	enum phy_ch_width ch_width;

	mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (mac_ctx == NULL) {
		WMA_LOGE("%s: vdev start failed as mac_ctx is NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	params.band_center_freq1 = cds_chan_to_freq(req->chan);
	ch_width = req->chan_width;
	bw_val = wlan_reg_get_bw_value(req->chan_width);
	if (20 < bw_val) {
		if (req->ch_center_freq_seg0) {
			params.band_center_freq1 =
				cds_chan_to_freq(req->ch_center_freq_seg0);
		} else {
			WMA_LOGE("%s: invalid cntr_freq for bw %d, drop to 20",
					__func__, bw_val);
			params.band_center_freq1 = cds_chan_to_freq(req->chan);
			ch_width = CH_WIDTH_20MHZ;
			bw_val = 20;
		}
	}
	if (80 < bw_val) {
		if (req->ch_center_freq_seg1) {
			params.band_center_freq2 =
				cds_chan_to_freq(req->ch_center_freq_seg1);
		} else {
			WMA_LOGE("%s: invalid cntr_freq for bw %d, drop to 80",
					__func__, bw_val);
			params.band_center_freq2 = 0;
			ch_width = CH_WIDTH_80MHZ;
		}
	} else {
		params.band_center_freq2 = 0;
	}
	chan_mode = wma_chan_phy_mode(req->chan, ch_width,
				      req->dot11_mode);

	if (chan_mode == MODE_UNKNOWN) {
		WMA_LOGE("%s: invalid phy mode!", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!params.band_center_freq1) {
		WMA_LOGE("%s: invalid center freq1", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (((ch_width == CH_WIDTH_160MHZ) || (ch_width == CH_WIDTH_80P80MHZ))
				&& !params.band_center_freq2) {
		WMA_LOGE("%s: invalid center freq2 for 160MHz", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* Fill channel info */
	params.chan_freq = cds_chan_to_freq(req->chan);
	params.chan_mode = chan_mode;

	if (!isRestart &&
	    qdf_atomic_read(&iface->bss_status) == WMA_BSS_STATUS_STARTED) {
		req_msg = wma_find_vdev_req(wma, req->vdev_id,
					    WMA_TARGET_REQ_TYPE_VDEV_STOP,
					    false);
		if (!req_msg || req_msg->msg_type != WMA_DELETE_BSS_REQ) {
			if (!cds_is_driver_recovering()) {
				if (cds_is_self_recovery_enabled()) {
					WMA_LOGE("BSS is in started state before vdev start, trigger recovery");
					cds_trigger_recovery(
						QDF_REASON_UNSPECIFIED);
				} else {
					WMA_LOGE("BSS is in started state before vdev start, BUG_ON()");
					QDF_BUG(0);
				}
			}
		}
	}

	WMA_LOGD("%s: Enter isRestart=%d vdev=%d", __func__, isRestart,
		 req->vdev_id);
	params.vdev_id = req->vdev_id;

	intr[params.vdev_id].chanmode = params.chan_mode;
	intr[params.vdev_id].ht_capable = req->ht_capable;
	intr[params.vdev_id].vht_capable = req->vht_capable;
	intr[params.vdev_id].config.gtx_info.gtxRTMask[0] =
		CFG_TGT_DEFAULT_GTX_HT_MASK;
	intr[params.vdev_id].config.gtx_info.gtxRTMask[1] =
		CFG_TGT_DEFAULT_GTX_VHT_MASK;

	if (wlan_cfg_get_int(mac_ctx, WNI_CFG_TGT_GTX_USR_CFG,
	    &intr[params.vdev_id].config.gtx_info.gtxUsrcfg) != eSIR_SUCCESS) {
		intr[params.vdev_id].config.gtx_info.gtxUsrcfg =
						WNI_CFG_TGT_GTX_USR_CFG_STADEF;
		QDF_TRACE(QDF_MODULE_ID_WMA, QDF_TRACE_LEVEL_WARN,
			  "Failed to get WNI_CFG_TGT_GTX_USR_CFG");
	}

	intr[params.vdev_id].config.gtx_info.gtxPERThreshold =
		CFG_TGT_DEFAULT_GTX_PER_THRESHOLD;
	intr[params.vdev_id].config.gtx_info.gtxPERMargin =
		CFG_TGT_DEFAULT_GTX_PER_MARGIN;
	intr[params.vdev_id].config.gtx_info.gtxTPCstep =
		CFG_TGT_DEFAULT_GTX_TPC_STEP;
	intr[params.vdev_id].config.gtx_info.gtxTPCMin =
		CFG_TGT_DEFAULT_GTX_TPC_MIN;
	intr[params.vdev_id].config.gtx_info.gtxBWMask =
		CFG_TGT_DEFAULT_GTX_BW_MASK;
	intr[params.vdev_id].mhz = params.chan_freq;
	intr[params.vdev_id].chan_width = ch_width;
	wma_copy_txrxnode_he_ops(&intr[params.vdev_id], req);

	temp_chan_info &= 0xffffffc0;
	temp_chan_info |= params.chan_mode;

	/* Set half or quarter rate WMI flags */
	params.is_half_rate = req->is_half_rate;
	params.is_quarter_rate = req->is_quarter_rate;

	if (req->is_half_rate)
		temp_chan_info |=  (1 << WMI_CHAN_FLAG_HALF_RATE);
	else if (req->is_quarter_rate)
		temp_chan_info |=  (1 << WMI_CHAN_FLAG_QUARTER_RATE);

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

	params.is_dfs = req->is_dfs;
	params.is_restart = isRestart;
	params.cac_duration_ms = req->cac_duration_ms;
	params.regdomain = req->dfs_regdomain;
	if ((QDF_GLOBAL_MONITOR_MODE != cds_get_conparam()) && req->is_dfs) {
		temp_chan_info |=  (1 << WMI_CHAN_FLAG_DFS);
		params.dis_hw_ack = true;

		/*
		 * If channel is DFS and operating in AP mode,
		 * set the WMI_CHAN_FLAG_DFS flag.
		 */
		if (wma_is_vdev_in_ap_mode(wma, params.vdev_id) == true)
			params.flag_dfs = WMI_CHAN_FLAG_DFS;
	}

	params.beacon_intval = req->beacon_intval;
	params.dtim_period = req->dtim_period;

	if (req->beacon_tx_rate) {
		WMA_LOGD("%s: beacon tx rate [%hu * 100 Kbps]",
				__func__, req->beacon_tx_rate);
		temp_flags |= WMI_UNIFIED_VDEV_START_BCN_TX_RATE_PRESENT;
		/*
		 * beacon_tx_rate is in multiples of 100 Kbps.
		 * Convert the data rate to hw rate code.
		 */
		params.bcn_tx_rate_code =
			wma_get_bcn_rate_code(req->beacon_tx_rate);
	}

	/* FIXME: Find out min, max and regulatory power levels */
	params.max_txpow = req->max_txpow;
	temp_reg_info_1 &= 0xff00ffff;
	temp_reg_info_1 |= ((req->max_txpow&0xff) << 16);

	temp_reg_info_2 &= 0xffff00ff;
	temp_reg_info_2 |= ((req->max_txpow&0xff)<<8);

	/* TODO: Handle regulatory class, max antenna */
	if (!isRestart) {
		params.beacon_intval = req->beacon_intval;
		params.dtim_period = req->dtim_period;

		/* Copy the SSID */
		if (req->ssid.length) {
			params.ssid.length = req->ssid.length;
			if (req->ssid.length < sizeof(cmd->ssid.ssid))
				temp_ssid_len = req->ssid.length;
			else
				temp_ssid_len = sizeof(cmd->ssid.ssid);
			qdf_mem_copy(params.ssid.mac_ssid, req->ssid.ssId,
				     temp_ssid_len);
		}

		params.pmf_enabled = req->pmf_enabled;
		if (req->pmf_enabled)
			temp_flags |= WMI_UNIFIED_VDEV_START_PMF_ENABLED;
	}

	params.hidden_ssid = req->hidden_ssid;
	if (req->hidden_ssid)
		temp_flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;

	params.num_noa_descriptors = 0;
	params.preferred_rx_streams = req->preferred_rx_streams;
	params.preferred_tx_streams = req->preferred_tx_streams;

	wma_copy_vdev_start_he_ops(&params, req);

	/* Store vdev params in SAP mode which can be used in vdev restart */
	if (intr[req->vdev_id].type == WMI_VDEV_TYPE_AP &&
	    intr[req->vdev_id].sub_type == 0) {
		intr[req->vdev_id].vdev_restart_params.vdev_id = req->vdev_id;
		intr[req->vdev_id].vdev_restart_params.ssid.ssid_len =
			temp_ssid_len;
		qdf_mem_copy(intr[req->vdev_id].vdev_restart_params.ssid.ssid,
			     params.ssid.mac_ssid, temp_ssid_len);
		intr[req->vdev_id].vdev_restart_params.flags = temp_flags;
		intr[req->vdev_id].vdev_restart_params.requestor_id = 0;
		intr[req->vdev_id].vdev_restart_params.disable_hw_ack =
			params.dis_hw_ack;
		intr[req->vdev_id].vdev_restart_params.chan.mhz =
			params.chan_freq;
		intr[req->vdev_id].vdev_restart_params.chan.band_center_freq1 =
			params.band_center_freq1;
		intr[req->vdev_id].vdev_restart_params.chan.band_center_freq2 =
			params.band_center_freq2;
		intr[req->vdev_id].vdev_restart_params.chan.info =
			temp_chan_info;
		intr[req->vdev_id].vdev_restart_params.chan.reg_info_1 =
			temp_reg_info_1;
		intr[req->vdev_id].vdev_restart_params.chan.reg_info_2 =
			temp_reg_info_2;
	}

	if (isRestart) {
		/*
		 * Marking the VDEV UP STATUS to false
		 * since, VDEV RESTART will do a VDEV DOWN
		 * in the firmware.
		 */
		wma_vdev_set_mlme_state(wma, params.vdev_id, WLAN_VDEV_S_STOP);
	} else {
		WMA_LOGD("%s, vdev_id: %d, unpausing tx_ll_queue at VDEV_START",
			 __func__, params.vdev_id);
		cdp_fc_vdev_unpause(cds_get_context(QDF_MODULE_ID_SOC),
			wma->interfaces[params.vdev_id].handle,
			0xffffffff);
		wma_vdev_update_pause_bitmap(params.vdev_id, 0);
	}

	return wma_send_vdev_start_to_fw(wma, &params);
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
	uint8_t macaddr[IEEE80211_ADDR_LEN];
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
		WMA_LOGD(FL("Send ADD_STA_RSP: statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
			 params->staType, params->smesessionId,
			 params->assocId, params->bssId, params->staIdx,
			 params->status);
		wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP,
					   (void *)params, 0);
	} else if (req_msg->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams  params = (tpAddBssParams) req_msg->user_data;

		if (!params) {
			WMA_LOGE(FL("add BSS params is NULL for vdev %d"),
				 event->vdev_id);
			status = -EINVAL;
			goto free_req_msg;
		}

		/* peer assoc conf event means the cmd succeeds */
		params->status = QDF_STATUS_SUCCESS;
		WMA_LOGD(FL("Send ADD BSS RSP: opermode: %d update_bss: %d nw_type: %d bssid: %pM staIdx %d status %d"),
			params->operMode,
			params->updateBss, params->nwType, params->bssId,
			params->staContext.staIdx, params->status);
		wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP,
					   (void *)params, 0);
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
 * wma_vdev_delete_handler() - vdev delete response handler
 * @handle: wma handle
 * @cmd_param_info: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int wma_vdev_delete_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len)
{
	tp_wma_handle wma = (tp_wma_handle) handle;
	WMI_VDEV_DELETE_RESP_EVENTID_param_tlvs *param_buf;
	wmi_vdev_delete_cmd_fixed_param *event;
	struct wma_target_req *req_msg;
	int status = 0;

	param_buf = (WMI_VDEV_DELETE_RESP_EVENTID_param_tlvs *)cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid vdev delete event buffer");
		return -EINVAL;
	}

	event = (wmi_vdev_delete_cmd_fixed_param *)param_buf->fixed_param;
	if (!event) {
		WMA_LOGE("Invalid vdev delete event buffer");
		return -EINVAL;
	}

	WMA_LOGD("%s Vdev delete resp vdev id %d", __func__, event->vdev_id);
	req_msg = wma_find_vdev_req(wma, event->vdev_id,
				WMA_TARGET_REQ_TYPE_VDEV_DEL, true);
	if (!req_msg) {
		WMA_LOGD(FL("Vdev delete resp is not handled! vdev id %d"),
				event->vdev_id);
		return -EINVAL;
	}
	qdf_mc_timer_stop(&req_msg->event_timeout);
	qdf_mc_timer_destroy(&req_msg->event_timeout);

	wma_release_wakelock(&wma->wmi_cmd_rsp_wake_lock);

	/* Send response to upper layers */
	wma_vdev_detach_callback(req_msg->user_data);
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
	uint8_t macaddr[IEEE80211_ADDR_LEN];
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
		wma_handle_vdev_detach(wma, data->self_sta_param,
				data->generate_rsp);
		qdf_mem_free(data);
	} else if (req_msg->type == WMA_SET_LINK_PEER_RSP) {
		tpLinkStateParams params =
			(tpLinkStateParams) req_msg->user_data;
		if (wma_send_vdev_down_to_fw(wma, req_msg->vdev_id) !=
		    QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
					req_msg->vdev_id);
		}
		wma_send_msg(wma, WMA_SET_LINK_STATE_RSP, (void *)params, 0);
	} else if (req_msg->type == WMA_DELETE_PEER_RSP) {
		wma_send_del_bss_response(wma, req_msg, req_msg->vdev_id);
	}
	qdf_mem_free(req_msg);
	return status;
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
	struct wma_target_req *msg;

	wma = cds_get_context(QDF_MODULE_ID_WMA);
	if (NULL == wma) {
		WMA_LOGE(FL("Failed to get wma"));
		return;
	}

	WMA_LOGA(FL("request %d is timed out for vdev_id - %d"),
		 tgt_req->msg_type, tgt_req->vdev_id);
	msg = wma_find_req(wma, tgt_req->vdev_id, tgt_req->type);

	if (!msg) {
		WMA_LOGE(FL("Failed to lookup request message - %d"),
			 tgt_req->msg_type);
		/*
		 * if find request failed, then firmware rsp should have
		 * consumed the buffer. Do not free.
		 */
		return;
	}

	if (tgt_req->msg_type == WMA_ADD_STA_REQ) {
		tpAddStaParams params = (tpAddStaParams) tgt_req->user_data;

		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA(FL("WMA_ADD_STA_REQ timed out"));
		WMA_LOGD(FL("Sending add sta rsp to umac (mac:%pM, status:%d)"),
			 params->staMac, params->status);
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
			QDF_BUG(0);
		else
			wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP,
						   (void *)params, 0);
	} else if (tgt_req->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams  params = (tpAddBssParams) tgt_req->user_data;

		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA(FL("WMA_ADD_BSS_REQ timed out"));
		WMA_LOGD(FL("Sending add bss rsp to umac (mac:%pM, status:%d)"),
			params->selfMacAddr, params->status);
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
			QDF_BUG(0);
		else
			wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP,
						   (void *)params, 0);
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
		(tgt_req->type == WMA_DELETE_STA_RSP_START)) {
		tpDeleteStaParams params =
				(tpDeleteStaParams) tgt_req->user_data;
		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGE(FL("WMA_DEL_STA_REQ timed out"));
		WMA_LOGE(FL("Sending del sta rsp to umac (mac:%pM, status:%d)"),
			 params->staMac, params->status);

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true) {
			QDF_BUG(0);
		} else {
			/*
			 * Assert in development build only.
			 * Send response in production builds.
			 */
			QDF_ASSERT(0);
			wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP,
				    (void *)params, 0);
		}
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
		(tgt_req->type == WMA_DEL_P2P_SELF_STA_RSP_START)) {
		struct del_sta_self_rsp_params *del_sta;

		del_sta = (struct del_sta_self_rsp_params *)tgt_req->user_data;
		del_sta->self_sta_param->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA(FL("wma delete sta p2p request timed out"));

		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash)) {
			QDF_BUG(0);
		} else {
			if (del_sta->generate_rsp)
				wma_send_del_sta_self_resp(
					del_sta->self_sta_param);
		}
		qdf_mem_free(tgt_req->user_data);
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
			(tgt_req->type == WMA_SET_LINK_PEER_RSP)) {
		tpLinkStateParams params =
			(tpLinkStateParams) tgt_req->user_data;

		params->status = false;
		WMA_LOGA(FL("wma delete peer for set link timed out"));
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
			QDF_BUG(0);
		else
			wma_send_msg(wma, WMA_SET_LINK_STATE_RSP,
					params, 0);
	} else if ((tgt_req->msg_type == WMA_DELETE_STA_REQ) &&
			(tgt_req->type == WMA_DELETE_PEER_RSP)) {
		tpDeleteBssParams params =
			(tpDeleteBssParams) tgt_req->user_data;

		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGE(FL("wma delete peer for del bss req timed out"));
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
			QDF_BUG(0);
		else
			wma_send_msg_high_priority(wma, WMA_DELETE_BSS_RSP,
						   params, 0);
	} else if ((tgt_req->msg_type == SIR_HAL_PDEV_SET_HW_MODE) &&
			(tgt_req->type == WMA_PDEV_SET_HW_MODE_RESP)) {
		struct sir_set_hw_mode_resp *params =
			qdf_mem_malloc(sizeof(*params));

		WMA_LOGE(FL("set hw mode req timed out"));

		if (!params)
			WMA_LOGE("%s: Memory allocation failed", __func__);
		else {
			params->status = SET_HW_MODE_STATUS_ECANCELED;
			params->cfgd_hw_mode_index = 0;
			params->num_vdev_mac_entries = 0;

			if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
				QDF_BUG(0);
			else
				wma_send_msg_high_priority(wma,
					SIR_HAL_PDEV_SET_HW_MODE_RESP, params, 0);
		}
	} else {
		WMA_LOGE(FL("Unhandled timeout for msg_type:%d and type:%d"),
				tgt_req->msg_type, tgt_req->type);
		QDF_BUG(0);
	}
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
	if (!req) {
		WMA_LOGE(FL("Failed to allocate memory for msg %d vdev %d"),
			 msg_type, vdev_id);
		return NULL;
	}

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
 * wma_vdev_resp_timer() - wma response timeout function
 * @data: target request params
 *
 * Return: none
 */
void wma_vdev_resp_timer(void *data)
{
	tp_wma_handle wma;
	struct wma_target_req *tgt_req = (struct wma_target_req *)data;
	struct cdp_pdev *pdev;
	struct wma_target_req *msg;
	uint8_t peer_id;
	int status;
	void *peer;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	tpAniSirGlobal mac_ctx;
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (NULL == wma) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		wma_cleanup_target_req_param(tgt_req);
		goto free_tgt_req;
	}

	WMA_LOGA("%s: request %d is timed out for vdev_id - %d", __func__,
		 tgt_req->msg_type, tgt_req->vdev_id);
	msg = wma_find_vdev_req(wma, tgt_req->vdev_id, tgt_req->type, true);

	if (!msg) {
		WMA_LOGE("%s: Failed to lookup request message - %d",
			 __func__, tgt_req->msg_type);
		return;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		wma_cleanup_target_req_param(tgt_req);
		qdf_mc_timer_stop(&tgt_req->event_timeout);
		goto free_tgt_req;
	}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	mac_ctx = cds_get_context(QDF_MODULE_ID_PE);
	if (!mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		wma_cleanup_target_req_param(tgt_req);
		goto free_tgt_req;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	if (tgt_req->msg_type == WMA_CHNL_SWITCH_REQ) {
		tpSwitchChannelParams params =
			(tpSwitchChannelParams) tgt_req->user_data;
		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_SWITCH_CHANNEL_REQ timedout", __func__);

		/*
		 * Trigger host crash if the flag is set or if the timeout
		 * is not due to fw down
		 */
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
			QDF_BUG(0);
		else
			wma_send_msg_high_priority(wma, WMA_SWITCH_CHANNEL_RSP,
				    (void *)params, 0);
		if (wma->interfaces[tgt_req->vdev_id].is_channel_switch) {
			wma->interfaces[tgt_req->vdev_id].is_channel_switch =
				false;
		}
	} else if (tgt_req->msg_type == WMA_DELETE_BSS_REQ) {
		tpDeleteBssParams params =
			(tpDeleteBssParams) tgt_req->user_data;
		struct beacon_info *bcn;
		struct wma_txrx_node *iface;

		if (tgt_req->vdev_id >= wma->max_bssid) {
			WMA_LOGE("%s: Invalid vdev_id %d", __func__,
				 tgt_req->vdev_id);
			wma_cleanup_target_req_param(tgt_req);
			qdf_mc_timer_stop(&tgt_req->event_timeout);
			goto free_tgt_req;
		}

		iface = &wma->interfaces[tgt_req->vdev_id];
		if (iface->handle == NULL) {
			WMA_LOGE("%s vdev id %d is already deleted",
				 __func__, tgt_req->vdev_id);
			wma_cleanup_target_req_param(tgt_req);
			qdf_mc_timer_stop(&tgt_req->event_timeout);
			goto free_tgt_req;
		}
		/*
		 * Trigger host crash if the flag is set or if the timeout
		 * is not due to fw down
		 */
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true) {
			QDF_BUG(0);
			return;
		}

		status = wma_remove_bss_peer(wma, pdev, tgt_req,
					     tgt_req->vdev_id, params);
		if (status != 0)
			goto free_tgt_req;

		if (wmi_service_enabled(
		   wma->wmi_handle,
		   wmi_service_sync_delete_cmds))
			goto free_tgt_req;

		if (wma_send_vdev_down_to_fw(wma, tgt_req->vdev_id) !=
		    QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
				 tgt_req->vdev_id);
		} else {
			wma_vdev_set_mlme_state(wma, tgt_req->vdev_id,
				WLAN_VDEV_S_STOP);
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		if (mac_ctx->sap.sap_channel_avoidance)
			wma_find_mcc_ap(wma, tgt_req->vdev_id, false);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
		}
		cdp_fc_vdev_flush(soc, iface->handle);
		WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for WDA_DELETE_BSS_REQ timeout",
			 __func__, tgt_req->vdev_id);
		cdp_fc_vdev_unpause(soc, iface->handle,
				     OL_TXQ_PAUSE_REASON_VDEV_STOP);
		wma_vdev_clear_pause_bit(tgt_req->vdev_id, PAUSE_TYPE_HOST);
		qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
		WMA_LOGD("%s: (type %d subtype %d) BSS is stopped",
			 __func__, iface->type, iface->sub_type);

		bcn = wma->interfaces[tgt_req->vdev_id].beacon;

		if (bcn) {
			WMA_LOGD("%s: Freeing beacon struct %pK, template memory %pK",
				 __func__, bcn, bcn->buf);
			if (bcn->dma_mapped)
				qdf_nbuf_unmap_single(wma->qdf_dev, bcn->buf,
						      QDF_DMA_TO_DEVICE);
			qdf_nbuf_free(bcn->buf);
			qdf_mem_free(bcn);
			wma->interfaces[tgt_req->vdev_id].beacon = NULL;
		}
		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_DELETE_BSS_REQ timedout", __func__);
		wma_send_msg_high_priority(wma, WMA_DELETE_BSS_RSP,
					   (void *)params, 0);
		if (iface->del_staself_req && iface->is_del_sta_defered) {
			iface->is_del_sta_defered = false;
			WMA_LOGA("scheduling defered deletion(vdev id %x)",
				 tgt_req->vdev_id);
			wma_vdev_detach(wma, iface->del_staself_req, 1);
		}
	} else if (tgt_req->msg_type == WMA_DEL_STA_SELF_REQ) {
		struct wma_txrx_node *iface =
			(struct wma_txrx_node *)tgt_req->user_data;
		struct del_sta_self_params *params =
			(struct del_sta_self_params *) iface->del_staself_req;

		if (wmi_service_enabled(wma->wmi_handle,
					   wmi_service_sync_delete_cmds)) {
			wma_release_wakelock(&wma->wmi_cmd_rsp_wake_lock);
		}
		params->status = QDF_STATUS_E_TIMEOUT;

		WMA_LOGA("%s: WMA_DEL_STA_SELF_REQ timedout", __func__);
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true)
			QDF_BUG(0);
		else
			wma_send_del_sta_self_resp(iface->del_staself_req);
		if (iface->addBssStaContext)
			qdf_mem_free(iface->addBssStaContext);
		if (iface->staKeyParams)
			qdf_mem_free(iface->staKeyParams);

		wma_vdev_deinit(iface);
		qdf_mem_zero(iface, sizeof(*iface));
		wma_vdev_init(iface);
	} else if (tgt_req->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams params = (tpAddBssParams) tgt_req->user_data;

		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_ADD_BSS_REQ timedout", __func__);
		WMA_LOGD("%s: bssid %pM vdev_id %d", __func__, params->bssId,
			 tgt_req->vdev_id);
		if (wma_crash_on_fw_timeout(wma->fw_timeout_crash) == true) {
			QDF_BUG(0);
		} else {
			wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP,
						   (void *)params, 0);
			QDF_ASSERT(0);
		}
		goto free_tgt_req;

	} else if (tgt_req->msg_type == WMA_OCB_SET_CONFIG_CMD) {
		struct wma_txrx_node *iface;

		WMA_LOGE(FL("Failed to send OCB set config cmd"));
		iface = &wma->interfaces[tgt_req->vdev_id];
		wma_vdev_set_mlme_state(wma, tgt_req->vdev_id,
			WLAN_VDEV_S_STOP);
		wma_ocb_set_config_resp(wma, QDF_STATUS_E_TIMEOUT);
	} else if (tgt_req->msg_type == WMA_HIDDEN_SSID_VDEV_RESTART) {
		WMA_LOGE("Hidden ssid vdev restart Timed Out; vdev_id: %d, type = %d",
				tgt_req->vdev_id, tgt_req->type);
	} else if (tgt_req->msg_type == WMA_SET_LINK_STATE) {
		tpLinkStateParams params =
			(tpLinkStateParams) tgt_req->user_data;

		peer = cdp_peer_find_by_addr(soc, pdev, params->bssid, &peer_id);
		if (peer) {
			WMA_LOGP(FL("Deleting peer %pM vdev id %d"),
				 params->bssid, tgt_req->vdev_id);
			wma_remove_peer(wma, params->bssid, tgt_req->vdev_id,
					peer, false);
		}
		if (wma_send_vdev_down_to_fw(wma, tgt_req->vdev_id) !=
		    QDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
				tgt_req->vdev_id);
		}
		params->status = QDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_SET_LINK_STATE timedout vdev %d", __func__,
			tgt_req->vdev_id);
		wma_send_msg(wma, WMA_SET_LINK_STATE_RSP, (void *)params, 0);
	}
free_tgt_req:
	qdf_mc_timer_destroy(&tgt_req->event_timeout);
	qdf_mem_free(tgt_req);
}

/**
 * wma_fill_vdev_req() - fill vdev request
 * @wma: wma handle
 * @msg_type: message type
 * @type: request type
 * @params: request params
 * @timeout: timeout value
 *
 * Return: wma_target_req ptr
 */
struct wma_target_req *wma_fill_vdev_req(tp_wma_handle wma,
					 uint8_t vdev_id,
					 uint32_t msg_type, uint8_t type,
					 void *params, uint32_t timeout)
{
	struct wma_target_req *req;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		WMA_LOGE("%s: Failed to allocate memory for msg %d vdev %d",
			 __func__, msg_type, vdev_id);
		return NULL;
	}

	WMA_LOGD("%s: vdev_id %d msg %d", __func__, vdev_id, msg_type);
	qdf_spin_lock_bh(&wma->vdev_respq_lock);
	req->vdev_id = vdev_id;
	req->msg_type = msg_type;
	req->type = type;
	req->user_data = params;
	status = qdf_list_insert_back(&wma->vdev_resp_queue, &req->node);
	if (QDF_STATUS_SUCCESS != status) {
		qdf_spin_unlock_bh(&wma->vdev_respq_lock);
		WMA_LOGE(FL("Failed add request in queue for vdev_id %d type %d"),
			 vdev_id, type);
		qdf_mem_free(req);
		return NULL;
	}
	qdf_spin_unlock_bh(&wma->vdev_respq_lock);
	qdf_mc_timer_init(&req->event_timeout, QDF_TIMER_TYPE_SW,
			  wma_vdev_resp_timer, req);
	qdf_mc_timer_start(&req->event_timeout, timeout);
	return req;
}

/**
 * wma_remove_vdev_req() - remove vdev request
 * @wma: wma handle
 * @vdev_id: vdev id
 * @type: type
 *
 * Return: none
 */
void wma_remove_vdev_req(tp_wma_handle wma, uint8_t vdev_id,
				uint8_t type)
{
	struct wma_target_req *req_msg;

	req_msg = wma_find_vdev_req(wma, vdev_id, type, true);
	if (!req_msg)
		return;

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
		intr[vdev_id].max_tx_power = maxTxPower;

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
#else
static inline void wma_set_mgmt_frame_protection(tp_wma_handle wma)
{
}
#endif /* WLAN_FEATURE_11W */

/**
 * wma_add_bss_ap_mode() - process add bss request in ap mode
 * @wma: wma handle
 * @add_bss: add bss parameters
 *
 * Return: none
 */
static void wma_add_bss_ap_mode(tp_wma_handle wma, tpAddBssParams add_bss)
{
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	struct wma_vdev_start_req req;
	void *peer;
	struct wma_target_req *msg;
	uint8_t vdev_id, peer_id;
	QDF_STATUS status;
	int8_t maxTxPower;
	struct policy_mgr_hw_mode_params hw_mode = {0};
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto send_fail_resp;
	}

	vdev = wma_find_vdev_by_addr(wma, add_bss->bssId, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: Failed to get vdev handle:"MAC_ADDRESS_STR,
			__func__, MAC_ADDR_ARRAY(add_bss->bssId));

		goto send_fail_resp;
	}
	if (SAP_WPS_DISABLED == add_bss->wps_state)
		pmo_ucfg_disable_wakeup_event(wma->psoc, vdev_id,
			(1 << WOW_PROBE_REQ_WPS_IE_EVENT));
	wma_set_bss_rate_flags(&wma->interfaces[vdev_id], add_bss);
	status = wma_create_peer(wma, pdev, vdev, add_bss->bssId,
				 WMI_PEER_TYPE_DEFAULT, vdev_id, false);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to create peer", __func__);
		goto send_fail_resp;
	}

	peer = cdp_peer_find_by_addr(soc, pdev,
			add_bss->bssId, &peer_id);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 add_bss->bssId);
		goto send_fail_resp;
	}
	msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_START, add_bss,
				WMA_VDEV_START_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGE("%s Failed to allocate vdev request vdev_id %d",
			 __func__, vdev_id);
		goto peer_cleanup;
	}

	add_bss->staContext.staIdx = cdp_peer_get_local_peer_id(soc, peer);

	qdf_mem_zero(&req, sizeof(req));
	req.vdev_id = vdev_id;
	req.chan = add_bss->currentOperChannel;
	req.chan_width = add_bss->ch_width;

	if (add_bss->ch_width == CH_WIDTH_10MHZ)
		req.is_half_rate = 1;
	else if (add_bss->ch_width == CH_WIDTH_5MHZ)
		req.is_quarter_rate = 1;

	req.ch_center_freq_seg0 = add_bss->ch_center_freq_seg0;
	req.ch_center_freq_seg1 = add_bss->ch_center_freq_seg1;
	req.vht_capable = add_bss->vhtCapable;
	wma_update_vdev_he_ops(&req, add_bss);

	req.max_txpow = add_bss->maxTxPower;
	maxTxPower = add_bss->maxTxPower;

	if (add_bss->rmfEnabled)
		wma_set_mgmt_frame_protection(wma);

	req.dot11_mode = add_bss->dot11_mode;
	req.beacon_intval = add_bss->beaconInterval;
	req.dtim_period = add_bss->dtimPeriod;
	req.beacon_tx_rate = add_bss->beacon_tx_rate;
	req.hidden_ssid = add_bss->bHiddenSSIDEn;
	req.is_dfs = add_bss->bSpectrumMgtEnabled;
	req.oper_mode = BSS_OPERATIONAL_MODE_AP;
	req.ssid.length = add_bss->ssId.length;
	req.cac_duration_ms = add_bss->cac_duration_ms;
	req.dfs_regdomain = add_bss->dfs_regdomain;
	if (req.ssid.length > 0)
		qdf_mem_copy(req.ssid.ssId, add_bss->ssId.ssId,
			     add_bss->ssId.length);
	status = policy_mgr_get_current_hw_mode(wma->psoc, &hw_mode);
	if (!QDF_IS_STATUS_SUCCESS(status))
		WMA_LOGE("policy_mgr_get_current_hw_mode failed");

	if (add_bss->nss == 2) {
		req.preferred_rx_streams = 2;
		req.preferred_tx_streams = 2;
	} else {
		req.preferred_rx_streams = 1;
		req.preferred_tx_streams = 1;
	}

	status = wma_vdev_start(wma, &req, false);
	if (status != QDF_STATUS_SUCCESS) {
		wma_remove_vdev_req(wma, vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);
		goto peer_cleanup;
	}

	wma_vdev_set_bss_params(wma, vdev_id,
				add_bss->beaconInterval, add_bss->dtimPeriod,
				add_bss->shortSlotTimeSupported,
				add_bss->llbCoexist, maxTxPower);

	wma_vdev_set_he_bss_params(wma, vdev_id, &req);
	return;

peer_cleanup:
	wma_remove_peer(wma, add_bss->bssId, vdev_id, peer, false);
send_fail_resp:
	add_bss->status = QDF_STATUS_E_FAILURE;
	wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
}

#ifdef QCA_IBSS_SUPPORT
/**
 * wma_add_bss_ibss_mode() -  process add bss request in IBSS mode
 * @wma: wma handle
 * @add_bss: add bss parameters
 *
 * Return: none
 */
static void wma_add_bss_ibss_mode(tp_wma_handle wma, tpAddBssParams add_bss)
{
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	struct wma_vdev_start_req req;
	void *peer = NULL;
	struct wma_target_req *msg;
	uint8_t vdev_id, peer_id;
	QDF_STATUS status;
	tSetBssKeyParams key_info;
	struct policy_mgr_hw_mode_params hw_mode = {0};
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	vdev = wma_find_vdev_by_addr(wma, add_bss->selfMacAddr, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: vdev not found for vdev id %d.",
				__func__, vdev_id);
		goto send_fail_resp;
	}
	WMA_LOGD("%s: add_bss->sessionId = %d", __func__, vdev_id);
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto send_fail_resp;
	}
	wma_set_bss_rate_flags(&wma->interfaces[vdev_id], add_bss);

	/* create ibss bss peer */
	status = wma_create_peer(wma, pdev, vdev, add_bss->selfMacAddr,
				 WMI_PEER_TYPE_DEFAULT, vdev_id,
				 false);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to create peer", __func__);
		goto send_fail_resp;
	}
	WMA_LOGA("IBSS BSS peer created with mac %pM",
		 add_bss->selfMacAddr);

	peer = cdp_peer_find_by_addr(soc, pdev,
			add_bss->selfMacAddr, &peer_id);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 add_bss->selfMacAddr);
		goto send_fail_resp;
	}

	/* clear leftover ibss keys on bss peer */

	WMA_LOGD("%s: ibss bss key clearing", __func__);
	qdf_mem_set(&key_info, sizeof(key_info), 0);
	key_info.smesessionId = vdev_id;
	key_info.numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
	qdf_mem_copy(&wma->ibsskey_info, &key_info, sizeof(tSetBssKeyParams));

	/* start ibss vdev */

	add_bss->operMode = BSS_OPERATIONAL_MODE_IBSS;

	msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_START, add_bss,
				WMA_VDEV_START_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGE("%s Failed to allocate vdev request vdev_id %d",
			 __func__, vdev_id);
		goto peer_cleanup;
	}
	WMA_LOGD("%s: vdev start request for IBSS enqueued", __func__);

	add_bss->staContext.staIdx = cdp_peer_get_local_peer_id(soc, peer);

	/*
	 * If IBSS Power Save is supported by firmware
	 * set the IBSS power save params to firmware.
	 */
	if (wmi_service_enabled(wma->wmi_handle,
				   wmi_service_ibss_pwrsave)) {
		status = wma_set_ibss_pwrsave_params(wma, vdev_id);
		if (status != QDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to Set IBSS Power Save Params to firmware",
				__func__);
			goto peer_cleanup;
		}
	}

	qdf_mem_zero(&req, sizeof(req));
	req.vdev_id = vdev_id;
	req.chan = add_bss->currentOperChannel;
	req.chan_width = add_bss->ch_width;
	req.ch_center_freq_seg0 = add_bss->ch_center_freq_seg0;
	req.ch_center_freq_seg1 = add_bss->ch_center_freq_seg1;
	req.vht_capable = add_bss->vhtCapable;
#if defined WLAN_FEATURE_VOWIF
	req.max_txpow = add_bss->maxTxPower;
#else
	req.max_txpow = 0;
#endif /* WLAN_FEATURE_VOWIF */
	req.beacon_intval = add_bss->beaconInterval;
	req.dtim_period = add_bss->dtimPeriod;
	req.hidden_ssid = add_bss->bHiddenSSIDEn;
	req.is_dfs = add_bss->bSpectrumMgtEnabled;
	req.oper_mode = BSS_OPERATIONAL_MODE_IBSS;
	req.ssid.length = add_bss->ssId.length;
	if (req.ssid.length > 0)
		qdf_mem_copy(req.ssid.ssId, add_bss->ssId.ssId,
			     add_bss->ssId.length);
	status = policy_mgr_get_current_hw_mode(wma->psoc, &hw_mode);
	if (!QDF_IS_STATUS_SUCCESS(status))
		WMA_LOGE("policy_mgr_get_current_hw_mode failed");

	if (add_bss->nss == 2) {
		req.preferred_rx_streams = 2;
		req.preferred_tx_streams = 2;
	} else {
		req.preferred_rx_streams = 1;
		req.preferred_tx_streams = 1;
	}

	WMA_LOGD("%s: chan %d chan_width %d", __func__, req.chan,
		 req.chan_width);
	WMA_LOGD("%s: ssid = %s", __func__, req.ssid.ssId);

	status = wma_vdev_start(wma, &req, false);
	if (status != QDF_STATUS_SUCCESS) {
		wma_remove_vdev_req(wma, vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);
		goto peer_cleanup;
	}
	WMA_LOGD("%s: vdev start request for IBSS sent to target", __func__);

	/* Initialize protection mode to no protection */
	status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
					 WMI_VDEV_PARAM_PROTECTION_MODE,
					 IEEE80211_PROT_NONE);
	if (QDF_IS_STATUS_ERROR(status))
		WMA_LOGE("Failed to initialize protection mode");

	return;

peer_cleanup:
	if (peer)
		wma_remove_peer(wma, add_bss->bssId, vdev_id, peer, false);
send_fail_resp:
	add_bss->status = QDF_STATUS_E_FAILURE;
	wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
}
#endif /* QCA_IBSS_SUPPORT */

/**
 * wma_add_bss_sta_mode() -  process add bss request in sta mode
 * @wma: wma handle
 * @add_bss: add bss parameters
 *
 * Return: none
 */
static void wma_add_bss_sta_mode(tp_wma_handle wma, tpAddBssParams add_bss)
{
	struct cdp_pdev *pdev;
	struct wma_vdev_start_req req;
	struct wma_target_req *msg;
	uint8_t vdev_id = 0, peer_id;
	void *peer = NULL;
	QDF_STATUS status;
	struct wma_txrx_node *iface;
	int pps_val = 0;
	bool roam_synch_in_progress = false;
	tpAniSirGlobal pMac = cds_get_context(QDF_MODULE_ID_PE);
	struct policy_mgr_hw_mode_params hw_mode = {0};
	bool peer_assoc_sent = false;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (NULL == pMac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		goto send_fail_resp;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s Failed to get pdev", __func__);
		goto send_fail_resp;
	}

	vdev_id = add_bss->staContext.smesessionId;
	iface = &wma->interfaces[vdev_id];

	wma_set_bss_rate_flags(iface, add_bss);
	if (add_bss->operMode) {
		/* Save parameters later needed by WMA_ADD_STA_REQ */
		if (iface->addBssStaContext)
			qdf_mem_free(iface->addBssStaContext);
		iface->addBssStaContext = qdf_mem_malloc(sizeof(tAddStaParams));
		if (!iface->addBssStaContext) {
			WMA_LOGE("%s Failed to allocat memory", __func__);
			goto send_fail_resp;
		}
		qdf_mem_copy(iface->addBssStaContext, &add_bss->staContext,
			     sizeof(tAddStaParams));

		if (iface->staKeyParams) {
			qdf_mem_free(iface->staKeyParams);
			iface->staKeyParams = NULL;
		}
		if (add_bss->extSetStaKeyParamValid) {
			iface->staKeyParams =
				qdf_mem_malloc(sizeof(tSetStaKeyParams));
			if (!iface->staKeyParams) {
				WMA_LOGE("%s Failed to allocat memory",
					 __func__);
				goto send_fail_resp;
			}
			qdf_mem_copy(iface->staKeyParams,
				     &add_bss->extSetStaKeyParam,
				     sizeof(tSetStaKeyParams));
		}
		/* Save parameters later needed by WMA_ADD_STA_REQ */
		iface->rmfEnabled = add_bss->rmfEnabled;
		iface->beaconInterval = add_bss->beaconInterval;
		iface->llbCoexist = add_bss->llbCoexist;
		iface->shortSlotTimeSupported = add_bss->shortSlotTimeSupported;
		iface->nwType = add_bss->nwType;
		if (add_bss->nonRoamReassoc) {
			peer = cdp_peer_find_by_addr(soc,
					pdev, add_bss->bssId,
					&peer_id);
			if (peer) {
				add_bss->staContext.staIdx =
					cdp_peer_get_local_peer_id(soc, peer);
				goto send_bss_resp;
			}
		}
		if (add_bss->reassocReq) {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			struct cdp_vdev *vdev;
#endif
			/* Called in preassoc state. BSSID peer is already
			 * added by set_linkstate
			 */
			peer = cdp_peer_find_by_addr(soc,
					pdev,
					add_bss->bssId,
					&peer_id);
			if (!peer) {
				WMA_LOGE("%s Failed to find peer %pM", __func__,
					 add_bss->bssId);
				goto send_fail_resp;
			}
			if (wma_is_roam_synch_in_progress(wma, vdev_id)) {
				add_bss->staContext.staIdx =
					cdp_peer_get_local_peer_id(soc, peer);
				WMA_LOGD("LFR3:%s: bssid %pM staIdx %d",
					__func__, add_bss->bssId,
					add_bss->staContext.staIdx);
				return;
			}
			msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
						WMA_TARGET_REQ_TYPE_VDEV_START,
						add_bss,
						WMA_VDEV_START_REQUEST_TIMEOUT);
			if (!msg) {
				WMA_LOGE("%s Failed to allocate vdev request vdev_id %d",
					__func__, vdev_id);
				goto peer_cleanup;
			}

			add_bss->staContext.staIdx =
				cdp_peer_get_local_peer_id(soc, peer);

			qdf_mem_zero(&req, sizeof(req));
			req.vdev_id = vdev_id;
			req.chan = add_bss->currentOperChannel;
			req.chan_width = add_bss->ch_width;

			if (add_bss->ch_width == CH_WIDTH_10MHZ)
				req.is_half_rate = 1;
			else if (add_bss->ch_width == CH_WIDTH_5MHZ)
				req.is_quarter_rate = 1;

			req.ch_center_freq_seg0 = add_bss->ch_center_freq_seg0;
			req.ch_center_freq_seg1 = add_bss->ch_center_freq_seg1;
			req.max_txpow = add_bss->maxTxPower;
			req.beacon_intval = add_bss->beaconInterval;
			req.dtim_period = add_bss->dtimPeriod;
			req.hidden_ssid = add_bss->bHiddenSSIDEn;
			req.is_dfs = add_bss->bSpectrumMgtEnabled;
			req.ssid.length = add_bss->ssId.length;
			req.oper_mode = BSS_OPERATIONAL_MODE_STA;
			if (req.ssid.length > 0)
				qdf_mem_copy(req.ssid.ssId, add_bss->ssId.ssId,
					     add_bss->ssId.length);
			status = policy_mgr_get_current_hw_mode(wma->psoc,
				&hw_mode);
			if (!QDF_IS_STATUS_SUCCESS(status))
				WMA_LOGE("policy_mgr_get_current_hw_mode failed");

			if (add_bss->nss == 2) {
				req.preferred_rx_streams = 2;
				req.preferred_tx_streams = 2;
			} else {
				req.preferred_rx_streams = 1;
				req.preferred_tx_streams = 1;
			}

			status = wma_vdev_start(wma, &req, false);
			if (status != QDF_STATUS_SUCCESS) {
				wma_remove_vdev_req(wma, vdev_id,
					    WMA_TARGET_REQ_TYPE_VDEV_START);
				goto peer_cleanup;
			}
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			vdev = wma_find_vdev_by_id(wma, vdev_id);
			if (!vdev) {
				WMA_LOGE("%s Invalid txrx vdev", __func__);
				goto peer_cleanup;
			}
			cdp_fc_vdev_pause(soc, vdev,
				   OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED);
#endif
			/* ADD_BSS_RESP will be deferred to completion of
			 * VDEV_START
			 */
			return;
		}
		if (!add_bss->updateBss)
			goto send_bss_resp;
		/* Update peer state */
		if (add_bss->staContext.encryptType == eSIR_ED_NONE) {
			WMA_LOGD("%s: Update peer(%pM) state into auth",
				 __func__, add_bss->bssId);
			cdp_peer_state_update(soc, pdev, add_bss->bssId,
						  OL_TXRX_PEER_STATE_AUTH);
		} else {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			struct cdp_vdev *vdev;
#endif
			WMA_LOGD("%s: Update peer(%pM) state into conn",
				 __func__, add_bss->bssId);
			cdp_peer_state_update(soc, pdev, add_bss->bssId,
						  OL_TXRX_PEER_STATE_CONN);
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			peer = cdp_peer_find_by_addr(soc, pdev, add_bss->bssId,
					&peer_id);
			if (!peer) {
				WMA_LOGE("%s:%d Failed to find peer %pM",
					 __func__, __LINE__, add_bss->bssId);
				goto send_fail_resp;
			}

			vdev = wma_find_vdev_by_id(wma, vdev_id);
			if (!vdev) {
				WMA_LOGE("%s Invalid txrx vdev", __func__);
				goto peer_cleanup;
			}
			cdp_fc_vdev_pause(soc, vdev,
					OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED);
#endif
		}

		wmi_unified_send_txbf(wma, &add_bss->staContext);

		pps_val = ((pMac->enable5gEBT << 31) & 0xffff0000) |
				(PKT_PWR_SAVE_5G_EBT & 0xffff);
		status = wma_vdev_set_param(wma->wmi_handle, vdev_id,
						WMI_VDEV_PARAM_PACKET_POWERSAVE,
						pps_val);
		if (QDF_IS_STATUS_ERROR(status))
			WMA_LOGE("Failed to send wmi packet power save cmd");
		else
			WMA_LOGD("Sent PKT_PWR_SAVE_5G_EBT cmd to target, val = %x, status = %d",
				pps_val, status);
		wma_send_peer_assoc(wma, add_bss->nwType,
					    &add_bss->staContext);
		/* we just had peer assoc, so install key will be done later */
		iface->is_waiting_for_key = true;
		peer_assoc_sent = true;

		if (add_bss->rmfEnabled)
			wma_set_mgmt_frame_protection(wma);

		wma_vdev_set_bss_params(wma, add_bss->staContext.smesessionId,
					add_bss->beaconInterval,
					add_bss->dtimPeriod,
					add_bss->shortSlotTimeSupported,
					add_bss->llbCoexist,
					add_bss->maxTxPower);

		/*
		 * Store the bssid in interface table, bssid will
		 * be used during group key setting sta mode.
		 */
		qdf_mem_copy(iface->bssid, add_bss->bssId, IEEE80211_ADDR_LEN);

	}
send_bss_resp:

	wma_vdev_set_he_config(wma, vdev_id, add_bss);
	if (NULL == cdp_peer_find_by_addr(soc, pdev, add_bss->bssId,
					&add_bss->staContext.staIdx))
		add_bss->status = QDF_STATUS_E_FAILURE;
	else
		add_bss->status = QDF_STATUS_SUCCESS;
	add_bss->bssIdx = add_bss->staContext.smesessionId;
	qdf_mem_copy(add_bss->staContext.staMac, add_bss->bssId,
		     sizeof(add_bss->staContext.staMac));

	if (!wmi_service_enabled(wma->wmi_handle,
				    wmi_service_peer_assoc_conf)) {
		WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF not enabled"));
		goto send_final_rsp;
	}

	/* In case of reassoc, peer assoc cmd will not be sent */
	if (!peer_assoc_sent)
		goto send_final_rsp;

	msg = wma_fill_hold_req(wma, vdev_id, WMA_ADD_BSS_REQ,
			   WMA_PEER_ASSOC_CNF_START, add_bss,
			   WMA_PEER_ASSOC_TIMEOUT);
	if (!msg) {
		WMA_LOGE(FL("Failed to allocate request for vdev_id %d"),
			 vdev_id);
		wma_remove_req(wma, vdev_id, WMA_PEER_ASSOC_CNF_START);
		goto peer_cleanup;
	}
	return;

send_final_rsp:
	WMA_LOGD("%s: opermode %d update_bss %d nw_type %d bssid %pM staIdx %d status %d",
		 __func__, add_bss->operMode,
		 add_bss->updateBss, add_bss->nwType, add_bss->bssId,
		 add_bss->staContext.staIdx, add_bss->status);
	wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
	return;

peer_cleanup:
	if (peer)
		wma_remove_peer(wma, add_bss->bssId, vdev_id, peer,
				roam_synch_in_progress);
send_fail_resp:
	add_bss->status = QDF_STATUS_E_FAILURE;
	if (!wma_is_roam_synch_in_progress(wma, vdev_id))
		wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP,
					   (void *)add_bss, 0);
}

/**
 * wma_add_bss() - Add BSS request to fw as per opmode
 * @wma: wma handle
 * @params: add bss params
 *
 * Return: none
 */
void wma_add_bss(tp_wma_handle wma, tpAddBssParams params)
{
	WMA_LOGD("%s: add_bss_param.halPersona = %d",
		 __func__, params->halPersona);

	switch (params->halPersona) {

	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
		wma_add_bss_ap_mode(wma, params);
		break;

#ifdef QCA_IBSS_SUPPORT
	case QDF_IBSS_MODE:
		wma_add_bss_ibss_mode(wma, params);
		break;
#endif

	case QDF_NDI_MODE:
		wma_add_bss_ndi_mode(wma, params);
		break;

	default:
		wma_add_bss_sta_mode(wma, params);
		break;
	}
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
	uint8_t peer_id;
	QDF_STATUS status;
	int32_t ret;
	struct wma_txrx_node *iface = NULL;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint32_t mcs_limit, i, j;
	uint8_t *rate_pos;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
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
				add_sta->staMac, &peer_id);
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
			add_sta->staMac, &peer_id);
	if (peer) {
		WMA_LOGE("%s: My vdev:%pK, but Peer exists on other vdev with peer_addr %pM and peer_id %d",
			__func__, vdev, add_sta->staMac, peer_id);
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

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
				add_sta->staMac, &peer_id);
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

	if (wlan_cfg_get_int(wma->mac_context, WNI_CFG_SAP_MAX_MCS_DATA,
				&mcs_limit) != eSIR_SUCCESS) {
		mcs_limit = WNI_CFG_SAP_MAX_MCS_DATA_STADEF;
	}

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
#ifdef QCA_IBSS_SUPPORT
	/*
	 * In IBSS mode send the peer
	 * Atim Window length if IBSS
	 * power save is enabled by the
	 * firmware.
	 */
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
#endif

	iface->rmfEnabled = add_sta->rmfEnabled;
	if (add_sta->rmfEnabled)
		wma_set_mgmt_frame_protection(wma);

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

	add_sta->staIdx = cdp_peer_get_local_peer_id(soc, peer);
	add_sta->nss    = iface->nss;
	add_sta->status = QDF_STATUS_SUCCESS;
send_rsp:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf) {
		WMA_LOGD(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
		return;
	}

	WMA_LOGD(FL("statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
		 add_sta->staType, add_sta->smesessionId,
		 add_sta->assocId, add_sta->bssId, add_sta->staIdx,
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
	uint8_t peer_id;
	QDF_STATUS status;
	int32_t ret;
	tTdlsPeerStateParams *peerStateParams;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("%s: staType: %d, staIdx: %d, updateSta: %d, bssId: %pM, staMac: %pM",
		 __func__, add_sta->staType, add_sta->staIdx,
		 add_sta->updateSta, add_sta->bssId, add_sta->staMac);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
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
				add_sta->staMac,
				&peer_id);
		if (!peer) {
			WMA_LOGE("%s: addSta, failed to find peer handle for mac %pM",
				__func__, add_sta->staMac);
			add_sta->status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);

			cdp_peer_add_last_real_peer(soc, pdev, vdev, &peer_id);
			goto send_rsp;
		}

		add_sta->staIdx = cdp_peer_get_local_peer_id(soc, peer);
		WMA_LOGD("%s: addSta, after calling cdp_local_peer_id, staIdx: %d, staMac: %pM",
			 __func__, add_sta->staIdx, add_sta->staMac);

		peerStateParams = qdf_mem_malloc(sizeof(tTdlsPeerStateParams));
		if (!peerStateParams) {
			WMA_LOGE
				("%s: Failed to allocate memory for peerStateParams for %pM",
				__func__, add_sta->staMac);
			add_sta->status = QDF_STATUS_E_NOMEM;
			goto send_rsp;
		}

		peerStateParams->peerState = WMI_TDLS_PEER_STATE_PEERING;
		peerStateParams->vdevId = add_sta->smesessionId;
		qdf_mem_copy(&peerStateParams->peerMacAddr,
			     &add_sta->staMac, sizeof(tSirMacAddr));
		wma_update_tdls_peer_state(wma, peerStateParams);
	} else {
		/* its a change sta request * */
		peer =
			cdp_peer_find_by_addr(soc, pdev,
				add_sta->staMac,
				&peer_id);
		if (!peer) {
			WMA_LOGE("%s: changeSta,failed to find peer handle for mac %pM",
				__func__, add_sta->staMac);
			add_sta->status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);

			cdp_peer_add_last_real_peer(soc, pdev, vdev, &peer_id);

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

		ret =
			wma_send_peer_assoc(wma, add_sta->nwType, add_sta);
		if (ret) {
			add_sta->status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			cdp_peer_add_last_real_peer(soc, pdev, vdev, &peer_id);

			goto send_rsp;
		}
	}

send_rsp:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf)
		return;

	WMA_LOGD(FL("statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
		 add_sta->staType, add_sta->smesessionId,
		 add_sta->assocId, add_sta->bssId, add_sta->staIdx,
		 add_sta->status);
	wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP, (void *)add_sta, 0);
}
#endif

/**
 * wma_send_bss_color_change_enable() - send bss color chnage enable cmd.
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
	struct vdev_up_params param = {0};
	int smps_param;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType) {
		wma_add_tdls_sta(wma, params);
		return;
	}
#endif

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Unable to get pdev", __func__);
		goto out;
	}

	iface = &wma->interfaces[params->smesessionId];
	if (params->staType != STA_ENTRY_SELF) {
		WMA_LOGE("%s: unsupported station type %d",
			 __func__, params->staType);
		goto out;
	}
	peer = cdp_peer_find_by_addr(soc,
			pdev,
			params->bssId, &params->staIdx);
	if (peer == NULL) {
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
		WMA_LOGE("%s: vdev id %d is already UP for %pM", __func__,
			params->smesessionId, params->bssId);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	if (peer != NULL &&
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

		ret = wma_send_peer_assoc(wma,
				iface->nwType,
				(tAddStaParams *) iface->addBssStaContext);
		if (ret) {
			status = QDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, params->bssId,
					params->smesessionId, peer, false);
			goto out;
		}

		if (params->rmfEnabled)
			wma_set_mgmt_frame_protection(wma);

		/*
		 * Set the PTK in 11r mode because we already have it.
		 */
		if (iface->staKeyParams) {
			wma_set_stakey(wma,
				       (tpSetStaKeyParams) iface->staKeyParams);
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
		if (wmi_unified_nat_keepalive_enable(wma, params->smesessionId)
		    < 0) {
			WMA_LOGE("Unable to enable NAT keepalive for vdev_id:%d",
				params->smesessionId);
		}
	}

	param.vdev_id = params->smesessionId;
	param.assoc_id = params->assocId;
	if (wma_send_vdev_up_to_fw(wma, &param, params->bssId) !=
	    QDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to send vdev up cmd: vdev %d bssid %pM",
			 __func__, params->smesessionId, params->bssId);
		policy_mgr_set_do_hw_mode_change_flag(
			wma->psoc, false);
		status = QDF_STATUS_E_FAILURE;
	} else {
		wma_set_vdev_mgmt_rate(wma, params->smesessionId);
		wma_vdev_set_mlme_state(wma, params->smesessionId,
				WLAN_VDEV_S_RUN);
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
	if (WMI_SERVICE_EXT_IS_ENABLED(wma->wmi_service_bitmap,
			wma->wmi_service_ext_bitmap,
			WMI_SERVICE_LISTEN_INTERVAL_OFFLOAD_SUPPORT)) {
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
	WMA_LOGD(FL("statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
		 params->staType, params->smesessionId,
		 params->assocId, params->bssId, params->staIdx,
		 params->status);
	/* Don't send a response during roam sync operation */
	if (!wma_is_roam_synch_in_progress(wma, params->smesessionId))
		wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP,
					   (void *)params, 0);
}

/**
 * wma_delete_sta_req_ap_mode() - proces delete sta request from UMAC in AP mode
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
	uint8_t *peer_mac_addr;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	peer = cdp_peer_find_by_local_id(soc,
			pdev, del_sta->staIdx);
	if (!peer) {
		WMA_LOGE("%s: Failed to get peer handle using peer id %d",
			 __func__, del_sta->staIdx);
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}
	peer_mac_addr = cdp_peer_get_peer_mac_addr(soc, peer);

	wma_remove_peer(wma, peer_mac_addr, del_sta->smesessionId, peer,
			false);
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
 * wma_del_tdls_sta() - proces delete sta request from UMAC in TDLS
 * @wma: wma handle
 * @del_sta: delete sta params
 *
 * Return: none
 */
static void wma_del_tdls_sta(tp_wma_handle wma, tpDeleteStaParams del_sta)
{
	tTdlsPeerStateParams *peerStateParams;
	struct wma_target_req *msg;
	int status;

	peerStateParams = qdf_mem_malloc(sizeof(tTdlsPeerStateParams));
	if (!peerStateParams) {
		WMA_LOGE("%s: Failed to allocate memory for peerStateParams for: %pM",
			__func__, del_sta->staMac);
		del_sta->status = QDF_STATUS_E_NOMEM;
		goto send_del_rsp;
	}

	if (wma_is_roam_synch_in_progress(wma, del_sta->smesessionId)) {
		WMA_LOGE("%s: roaming in progress, reject del sta!", __func__);
		del_sta->status = QDF_STATUS_E_PERM;
		goto send_del_rsp;
	}

	peerStateParams->peerState = WMA_TDLS_PEER_STATE_TEARDOWN;
	peerStateParams->vdevId = del_sta->smesessionId;
	peerStateParams->resp_reqd = del_sta->respReqd;
	qdf_mem_copy(&peerStateParams->peerMacAddr,
		     &del_sta->staMac, sizeof(tSirMacAddr));

	WMA_LOGD("%s: sending tdls_peer_state for peer mac: %pM, peerState: %d",
		 __func__, peerStateParams->peerMacAddr,
		 peerStateParams->peerState);

	status = wma_update_tdls_peer_state(wma, peerStateParams);

	if (status < 0) {
		WMA_LOGE("%s: wma_update_tdls_peer_state returned failure",
				__func__);
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
					peerStateParams->vdevId);
			wma_remove_req(wma,
					peerStateParams->vdevId,
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
 * wma_delete_sta_req_sta_mode() - proces delete sta request from UMAC
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

/**
 * wma_add_sta() - process add sta request as per opmode
 * @wma: wma handle
 * @add_Sta: add sta params
 *
 * Return: none
 */
void wma_add_sta(tp_wma_handle wma, tpAddStaParams add_sta)
{
	uint8_t oper_mode = BSS_OPERATIONAL_MODE_STA;
	void *htc_handle;

	htc_handle = ucfg_get_htc_hdl(wma->psoc);
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
		htc_vote_link_up(htc_handle);
		wma_add_sta_req_ap_mode(wma, add_sta);
		break;
	case BSS_OPERATIONAL_MODE_NDI:
		wma_add_sta_ndi_mode(wma, add_sta);
		break;
	}

#ifdef QCA_IBSS_SUPPORT
	/* adjust heart beat thresold timer value for detecting ibss peer
	 * departure
	 */
	if (oper_mode == BSS_OPERATIONAL_MODE_IBSS)
		wma_adjust_ibss_heart_beat_timer(wma, add_sta->smesessionId, 1);
#endif

}

/**
 * wma_delete_sta() - process del sta request as per opmode
 * @wma: wma handle
 * @del_sta: delete sta params
 *
 * Return: none
 */
void wma_delete_sta(tp_wma_handle wma, tpDeleteStaParams del_sta)
{
	uint8_t oper_mode = BSS_OPERATIONAL_MODE_STA;
	uint8_t smesession_id = del_sta->smesessionId;
	bool rsp_requested = del_sta->respReqd;
	void *htc_handle;

	htc_handle = ucfg_get_htc_hdl(wma->psoc);
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
		htc_vote_link_down(htc_handle);
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

#ifdef QCA_IBSS_SUPPORT
	/* adjust heart beat thresold timer value for
	 * detecting ibss peer departure
	 */
	if (oper_mode == BSS_OPERATIONAL_MODE_IBSS)
		wma_adjust_ibss_heart_beat_timer(wma, smesession_id, -1);
#endif
}

/**
 * wma_delete_bss_ho_fail() - process delete bss request for handoff failure
 * @wma: wma handle
 * @params: del bss parameters
 *
 * Delete BSS in case of ROAM_HO_FAIL processing is handled separately in
 * this routine. It needs to be done without sending any commands to firmware
 * because firmware has already stopped and deleted peer and vdev is down.
 * Relevent logic is aggregated from other routines. It changes the host
 * data structures without sending VDEV_STOP, PEER_FLUSH_TIDS, PEER_DELETE
 * and VDEV_DOWN commands to firmware.
 *
 * Return: none
 */
void wma_delete_bss_ho_fail(tp_wma_handle wma, tpDeleteBssParams params)
{
	struct cdp_pdev *pdev;
	void *peer = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t peer_id;
	struct cdp_vdev *txrx_vdev = NULL;
	struct wma_txrx_node *iface;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s:Unable to get TXRX context", __func__);
		goto fail_del_bss_ho_fail;
	}

	peer = cdp_peer_find_by_addr(soc,
			pdev,
			params->bssid, &peer_id);
	if (!peer) {
		WMA_LOGE("%s: Failed to find peer %pM", __func__,
			 params->bssid);
		status = QDF_STATUS_E_FAILURE;
		goto fail_del_bss_ho_fail;
	}

	iface = &wma->interfaces[params->smesessionId];
	if (!iface || !iface->handle) {
		WMA_LOGE("%s vdev id %d is already deleted",
				__func__, params->smesessionId);
		goto fail_del_bss_ho_fail;
	}
	qdf_mem_zero(iface->bssid, IEEE80211_ADDR_LEN);

	txrx_vdev = wma_find_vdev_by_id(wma, params->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		status = QDF_STATUS_E_FAILURE;
		goto fail_del_bss_ho_fail;
	}

	/* Free the allocated stats response buffer for the the session */
	if (iface->stats_rsp) {
		qdf_mem_free(iface->stats_rsp);
		iface->stats_rsp = NULL;
	}

	if (iface->psnr_req) {
		qdf_mem_free(iface->psnr_req);
		iface->psnr_req = NULL;
	}

	if (iface->rcpi_req) {
		struct sme_rcpi_req *rcpi_req = iface->rcpi_req;

		iface->rcpi_req = NULL;
		qdf_mem_free(rcpi_req);
	}

	qdf_mem_zero(&iface->ns_offload_req,
			sizeof(iface->ns_offload_req));
	qdf_mem_zero(&iface->arp_offload_req,
			sizeof(iface->arp_offload_req));

	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP (del_bss)",
		 __func__, params->smesessionId);
	cdp_fc_vdev_pause(soc, iface->handle,
			   OL_TXQ_PAUSE_REASON_VDEV_STOP);
	wma_vdev_set_pause_bit(params->smesessionId, PAUSE_TYPE_HOST);

	cdp_fc_vdev_flush(soc, iface->handle);
	WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for VDEV_STOP rsp",
			__func__, params->smesessionId);
	cdp_fc_vdev_unpause(soc, iface->handle,
			OL_TXQ_PAUSE_REASON_VDEV_STOP);
	wma_vdev_clear_pause_bit(params->smesessionId, PAUSE_TYPE_HOST);
	qdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
	WMA_LOGD("%s: (type %d subtype %d) BSS is stopped",
			__func__, iface->type, iface->sub_type);
	wma_vdev_set_mlme_state(wma, params->smesessionId, WLAN_VDEV_S_STOP);
	params->status = QDF_STATUS_SUCCESS;
	if (!iface->peer_count) {
		WMA_LOGE("%s: Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
			__func__, params->bssid,  params->smesessionId,
			iface->peer_count);
		goto fail_del_bss_ho_fail;
	}

	if (peer)
		cdp_peer_delete(soc, peer, 1 << CDP_PEER_DELETE_NO_SPECIAL);
	iface->peer_count--;
	WMA_LOGI("%s: Removed peer %pK with peer_addr %pM vdevid %d peer_count %d",
		 __func__, peer, params->bssid,  params->smesessionId,
		 iface->peer_count);
fail_del_bss_ho_fail:
	params->status = status;
	wma_send_msg_high_priority(wma, WMA_DELETE_BSS_HO_FAIL_RSP,
				   (void *)params, 0);
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

	if (!wma->interfaces[session_id].is_vdev_valid) {
		WMA_LOGE("%s: Vdev is not valid: %d",
			 __func__, session_id);
		return;
	}

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (pdev == NULL) {
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
/**
 * wma_delete_bss() - process delete bss request from upper layer
 * @wma: wma handle
 * @params: del bss parameters
 *
 * Return: none
 */
void wma_delete_bss(tp_wma_handle wma, tpDeleteBssParams params)
{
	struct cdp_pdev *pdev;
	void *peer = NULL;
	struct wma_target_req *msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t peer_id;
	struct cdp_vdev *txrx_vdev = NULL;
	bool roam_synch_in_progress = false;
	struct wma_txrx_node *iface;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s:Unable to get TXRX context", __func__);
		goto out;
	}
	if (wma_is_vdev_in_ibss_mode(wma, params->smesessionId))
		/* in rome ibss case, self mac is used to create the bss peer */
		peer = cdp_peer_find_by_addr(soc,
			pdev,
			wma->interfaces[params->smesessionId].addr,
			&peer_id);
	else if (WMA_IS_VDEV_IN_NDI_MODE(wma->interfaces,
			params->smesessionId))
		/* In ndi case, self mac is used to create the self peer */
		peer = cdp_peer_find_by_addr(soc, pdev,
				wma->interfaces[params->smesessionId].addr,
				&peer_id);
	else
		peer = cdp_peer_find_by_addr(soc, pdev,
				params->bssid,
				&peer_id);

	if (!peer) {
		WMA_LOGE("%s: Failed to find peer %pM", __func__,
			 params->bssid);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	qdf_mem_zero(wma->interfaces[params->smesessionId].bssid,
			IEEE80211_ADDR_LEN);

	txrx_vdev = wma_find_vdev_by_id(wma, params->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	iface = &wma->interfaces[params->smesessionId];
	if (!iface || !iface->handle) {
		WMA_LOGE("%s vdev id %d is already deleted",
				__func__, params->smesessionId);
		goto out;
	}
	/*Free the allocated stats response buffer for the the session */
	if (iface->stats_rsp) {
		qdf_mem_free(iface->stats_rsp);
		iface->stats_rsp = NULL;
	}

	if (iface->psnr_req) {
		qdf_mem_free(iface->psnr_req);
		iface->psnr_req = NULL;
	}

	if (iface->rcpi_req) {
		struct sme_rcpi_req *rcpi_req = iface->rcpi_req;

		iface->rcpi_req = NULL;
		qdf_mem_free(rcpi_req);
	}

	if (wlan_op_mode_ibss == cdp_get_opmode(soc, txrx_vdev))
		wma->ibss_started = 0;

	if (wma_is_roam_synch_in_progress(wma, params->smesessionId)) {
		roam_synch_in_progress = true;
		WMA_LOGD("LFR3:%s: Setting vdev_up to FALSE for session %d",
			__func__, params->smesessionId);
		wma_vdev_set_mlme_state(wma, params->smesessionId,
			WLAN_VDEV_S_STOP);
		goto detach_peer;
	}
	msg = wma_fill_vdev_req(wma, params->smesessionId, WMA_DELETE_BSS_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_STOP, params,
				WMA_VDEV_STOP_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGE("%s: Failed to fill vdev request for vdev_id %d",
			 __func__, params->smesessionId);
		status = QDF_STATUS_E_NOMEM;
		goto detach_peer;
	}

	WMA_LOGW(FL("Outstanding msdu packets: %d"),
		 cdp_get_tx_pending(soc, pdev));
	wma_wait_tx_complete(wma, params->smesessionId);

	if (cdp_get_tx_pending(soc, pdev)) {
		WMA_LOGW(FL("Outstanding msdu packets before VDEV_STOP : %d"),
			 cdp_get_tx_pending(soc, pdev));
	}

	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP (del_bss)",
		 __func__, params->smesessionId);
	wma_vdev_set_pause_bit(params->smesessionId, PAUSE_TYPE_HOST);
	cdp_fc_vdev_pause(soc,
		wma->interfaces[params->smesessionId].handle,
		OL_TXQ_PAUSE_REASON_VDEV_STOP);

	if (wma_send_vdev_stop_to_fw(wma, params->smesessionId)) {
		WMA_LOGE("%s: %d Failed to send vdev stop", __func__, __LINE__);
		wma_remove_vdev_req(wma, params->smesessionId,
				WMA_TARGET_REQ_TYPE_VDEV_STOP);
		status = QDF_STATUS_E_FAILURE;
		goto detach_peer;
		}
	WMA_LOGD("%s: bssid %pM vdev_id %d",
		 __func__, params->bssid, params->smesessionId);
	return;
detach_peer:
	wma_remove_peer(wma, params->bssid, params->smesessionId, peer,
			roam_synch_in_progress);
	if (wma_is_roam_synch_in_progress(wma, params->smesessionId))
		return;

out:
	params->status = status;
	wma_send_msg_high_priority(wma, WMA_DELETE_BSS_RSP, (void *)params, 0);
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
		if (NULL != intf) {
			if (intf[vdev_id].type == type)
				return vdev_id;
		}
	}

	return -EFAULT;
}

/**
 * wma_set_vdev_intrabss_fwd() - set intra_fwd value to wni_in.
 * @wma_handle: wma handle
 * @pdis_intra_fwd: Pointer to DisableIntraBssFwd struct
 *
 * Return: none
 */
void wma_set_vdev_intrabss_fwd(tp_wma_handle wma_handle,
				      tpDisableIntraBssFwd pdis_intra_fwd)
{
	struct cdp_vdev *txrx_vdev;

	WMA_LOGD("%s:intra_fwd:vdev(%d) intrabss_dis=%s",
		 __func__, pdis_intra_fwd->sessionId,
		 (pdis_intra_fwd->disableintrabssfwd ? "true" : "false"));

	txrx_vdev = wma_handle->interfaces[pdis_intra_fwd->sessionId].handle;
	cdp_cfg_vdev_rx_set_intrabss_fwd(cds_get_context(QDF_MODULE_ID_SOC),
				    txrx_vdev,
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
	if (NULL == wma) {
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
	if (!req) {
		WMA_LOGE("%s: Failed to allocate memory for beacon_interval_reset_req vdev %d",
			__func__, vdev_id);
		return -ENOMEM;
	}

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

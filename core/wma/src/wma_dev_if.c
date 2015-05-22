/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#include "ol_txrx_ctrl_api.h"
#include "wlan_tgt_def_config.h"

#include "cdf_nbuf.h"
#include "cdf_types.h"
#include "ol_txrx_api.h"
#include "cdf_memory.h"
#include "ol_txrx_types.h"
#include "ol_txrx_peer_find.h"

#include "wma_types.h"
#include "lim_api.h"
#include "lim_session_utils.h"

#include "cds_utils.h"

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif /* REMOVE_PKT_LOG */

#include "dbglog_host.h"
/* FIXME: Inclusion of .c looks odd but this is how it is in internal codebase */
#include "csr_api.h"
#include "ol_fw.h"

#include "dfs.h"
#include "wma_internal.h"

#include "wma_ocb.h"

/**
 * wma_find_vdev_by_addr() - find vdev_id from mac address
 * @wma: wma handle
 * @addr: mac address
 * @vdev_id: return vdev_id
 *
 * Return: Returns vdev handle or NULL if mac address don't match
 */
void *wma_find_vdev_by_addr(tp_wma_handle wma, uint8_t *addr,
				   uint8_t *vdev_id)
{
	uint8_t i;

	for (i = 0; i < wma->max_bssid; i++) {
		if (cdf_is_macaddr_equal(
			(struct cdf_mac_addr *) wma->interfaces[i].addr,
			(struct cdf_mac_addr *) addr) == true) {
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

	if (vdev_id > wma->max_bssid) {
		WMA_LOGP("%s: Invalid vdev_id %hu", __func__, vdev_id);
		CDF_ASSERT(0);
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

	if (vdev_id > wma->max_bssid) {
		WMA_LOGP("%s: Invalid vdev_id %hu", __func__, vdev_id);
		CDF_ASSERT(0);
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
void *wma_find_vdev_by_bssid(tp_wma_handle wma, uint8_t *bssid,
				    uint8_t *vdev_id)
{
	int i;

	for (i = 0; i < wma->max_bssid; i++) {
		if (cdf_is_macaddr_equal(
			(struct cdf_mac_addr *) wma->interfaces[i].bssid,
			(struct cdf_mac_addr *) bssid) == true) {
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
enum wlan_op_mode wma_get_txrx_vdev_type(uint32_t type)
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
	default:
		WMA_LOGE("Invalid vdev type %u", type);
		vdev_type = wlan_op_mode_unknown;
	}

	return vdev_type;
}

/**
 * wma_unified_vdev_create_send() - send VDEV create command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 * @type: vdev type
 * @subtype: vdev subtype
 * @macaddr: vdev mac address
 *
 * Return: 0 for success or error code
 */
int wma_unified_vdev_create_send(wmi_unified_t wmi_handle, uint8_t if_id,
				 uint32_t type, uint32_t subtype,
				 uint8_t macaddr[IEEE80211_ADDR_LEN])
{
	wmi_vdev_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int len = sizeof(*cmd);
	int ret;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed", __FUNCTION__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_create_cmd_fixed_param));
	cmd->vdev_id = if_id;
	cmd->vdev_type = type;
	cmd->vdev_subtype = subtype;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->vdev_macaddr);
	WMA_LOGE("%s: ID = %d VAP Addr = %02x:%02x:%02x:%02x:%02x:%02x",
		 __func__, if_id,
		 macaddr[0], macaddr[1], macaddr[2],
		 macaddr[3], macaddr[4], macaddr[5]);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len, WMI_VDEV_CREATE_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send WMI_VDEV_CREATE_CMDID");
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * wma_unified_vdev_delete_send() - send VDEV delete command to fw
 * @wmi_handle: wmi handle
 * @if_id: vdev id
 *
 * Return: 0 for success or error code
 */
static int wma_unified_vdev_delete_send(wmi_unified_t wmi_handle, uint8_t if_id)
{
	wmi_vdev_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int ret;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMA_LOGP("%s:wmi_buf_alloc failed", __FUNCTION__);
		return -ENOMEM;
	}

	cmd = (wmi_vdev_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_delete_cmd_fixed_param));
	cmd->vdev_id = if_id;
	ret = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(wmi_vdev_delete_cmd_fixed_param),
				   WMI_VDEV_DELETE_CMDID);
	if (ret != EOK) {
		WMA_LOGE("Failed to send WMI_VDEV_DELETE_CMDID");
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * wma_find_req() - find target request for vdev id
 * @wma: wma handle
 * @vdev_id: vdev id
 * @type: request type
 *
 * Return: return target request if found or NULL.
 */
static struct wma_target_req *wma_find_req(tp_wma_handle wma,
					   uint8_t vdev_id, uint8_t type)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	cdf_list_node_t *node1 = NULL, *node2 = NULL;
	CDF_STATUS status;

	cdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	if (CDF_STATUS_SUCCESS != cdf_list_peek_front(&wma->wma_hold_req_queue,
						      &node2)) {
		cdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGE(FL("unable to get msg node from request queue"));
		return NULL;
	}

	do {
		node1 = node2;
		req_msg = cdf_container_of(node1, struct wma_target_req, node);
		if (req_msg->vdev_id != vdev_id)
			continue;
		if (req_msg->type != type)
			continue;

		found = true;
		status = cdf_list_remove_node(&wma->wma_hold_req_queue, node1);
		if (CDF_STATUS_SUCCESS != status) {
			cdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
			WMA_LOGD(FL("Failed to remove request for vdev_id %d type %d"),
				 vdev_id, type);
			return NULL;
		}
		break;
	} while (CDF_STATUS_SUCCESS  ==
			cdf_list_peek_next(&wma->wma_hold_req_queue, node1,
					   &node2));

	cdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
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
 * wma_find_vdev_req() - find target request for vdev id
 * @wma: wma handle
 * @vdev_id: vdev id
 * @type: request type
 *
 * Return: return target request if found or NULL.
 */
static struct wma_target_req *wma_find_vdev_req(tp_wma_handle wma,
						uint8_t vdev_id, uint8_t type)
{
	struct wma_target_req *req_msg = NULL;
	bool found = false;
	cdf_list_node_t *node1 = NULL, *node2 = NULL;
	CDF_STATUS status;

	cdf_spin_lock_bh(&wma->vdev_respq_lock);
	if (CDF_STATUS_SUCCESS != cdf_list_peek_front(&wma->vdev_resp_queue,
						      &node2)) {
		cdf_spin_unlock_bh(&wma->vdev_respq_lock);
		WMA_LOGE(FL("unable to get target req from vdev resp queue"));
		return NULL;
	}

	do {
		node1 = node2;
		req_msg = cdf_container_of(node1, struct wma_target_req, node);
		if (req_msg->vdev_id != vdev_id)
			continue;
		if (req_msg->type != type)
			continue;

		found = true;
		status = cdf_list_remove_node(&wma->vdev_resp_queue, node1);
		if (CDF_STATUS_SUCCESS != status) {
			cdf_spin_unlock_bh(&wma->vdev_respq_lock);
			WMA_LOGD(FL("Failed to target req for vdev_id %d type %d"),
				 vdev_id, type);
			return NULL;
		}
		break;
	} while (CDF_STATUS_SUCCESS  ==
			cdf_list_peek_next(&wma->vdev_resp_queue,
					   node1, &node2));

	cdf_spin_unlock_bh(&wma->vdev_respq_lock);
	if (!found) {
		WMA_LOGP(FL("target request not found for vdev_id %d type %d"),
			 vdev_id, type);
		return NULL;
	}
	WMA_LOGD(FL("target request found for vdev id: %d type %d msg %d"),
		 vdev_id, type, req_msg->msg_type);
	return req_msg;
}

/**
 * wma_vdev_detach_callback() - send vdev detach response to upper layer
 * @ctx: txrx node ptr
 *
 * Return: none
 */
void wma_vdev_detach_callback(void *ctx)
{
	tp_wma_handle wma;
	struct wma_txrx_node *iface = (struct wma_txrx_node *)ctx;
	struct del_sta_self_params *param;
	struct wma_target_req *req_msg;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	cds_msg_t sme_msg = { 0 };

	wma = cds_get_context(CDF_MODULE_ID_WMA);

	if (!wma || !iface->del_staself_req) {
		WMA_LOGP("%s: wma %p iface %p", __func__, wma,
			 iface->del_staself_req);
		return;
	}
	param = (struct del_sta_self_params *) iface->del_staself_req;
	WMA_LOGD("%s: sending eWNI_SME_DEL_STA_SELF_RSP for vdev %d",
		 __func__, param->session_id);

	req_msg = wma_find_vdev_req(wma, param->session_id,
				    WMA_TARGET_REQ_TYPE_VDEV_DEL);
	if (req_msg) {
		WMA_LOGD("%s: Found vdev request for vdev id %d",
			 __func__, param->session_id);
		cdf_mc_timer_stop(&req_msg->event_timeout);
		cdf_mc_timer_destroy(&req_msg->event_timeout);
		cdf_mem_free(req_msg);
	}
	if (iface->addBssStaContext)
		cdf_mem_free(iface->addBssStaContext);

#if defined WLAN_FEATURE_VOWIFI_11R
	if (iface->staKeyParams)
		cdf_mem_free(iface->staKeyParams);
#endif /* WLAN_FEATURE_VOWIFI_11R */
	cdf_mem_zero(iface, sizeof(*iface));
	param->status = CDF_STATUS_SUCCESS;
	sme_msg.type = eWNI_SME_DEL_STA_SELF_RSP;
	sme_msg.bodyptr = param;
	sme_msg.bodyval = 0;

	status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
	if (!CDF_IS_STATUS_SUCCESS(status)) {
		WMA_LOGE("Failed to post eWNI_SME_ADD_STA_SELF_RSP");
		cdf_mem_free(param);
	}
}

/**
 * wma_vdev_detach() - send vdev delete command to fw
 * @wma_handle: wma handle
 * @pdel_sta_self_req_param: del sta params
 * @generateRsp: generate Response flag
 *
 * Return: CDF status
 */
CDF_STATUS wma_vdev_detach(tp_wma_handle wma_handle,
			struct del_sta_self_params *pdel_sta_self_req_param,
			uint8_t generateRsp)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	ol_txrx_peer_handle peer;
	ol_txrx_pdev_handle pdev;
	uint8_t peer_id;
	uint8_t vdev_id = pdel_sta_self_req_param->session_id;
	struct wma_txrx_node *iface = &wma_handle->interfaces[vdev_id];
	struct wma_target_req *msg;
	cds_msg_t sme_msg = { 0 };

	if ((iface->type == WMI_VDEV_TYPE_AP) &&
	    (iface->sub_type == WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE)) {

		WMA_LOGA("P2P Device: removing self peer %pM",
			 pdel_sta_self_req_param->self_mac_addr);

		pdev = cds_get_context(CDF_MODULE_ID_TXRX);

		if (NULL == pdev) {
			WMA_LOGE("%s: Failed to get pdev", __func__);
			return CDF_STATUS_E_FAULT;
		}

		peer = ol_txrx_find_peer_by_addr(pdev,
				 pdel_sta_self_req_param->self_mac_addr,
				 &peer_id);
		if (!peer) {
			WMA_LOGE("%s Failed to find peer %pM", __func__,
				 pdel_sta_self_req_param->self_mac_addr);
		}
		wma_remove_peer(wma_handle,
				pdel_sta_self_req_param->self_mac_addr,
				vdev_id, peer, false);
	}
	if (cdf_atomic_read(&iface->bss_status) == WMA_BSS_STATUS_STARTED) {
		WMA_LOGA("BSS is not yet stopped. Defering vdev(vdev id %x) deletion",
			vdev_id);
		iface->del_staself_req = pdel_sta_self_req_param;
		return status;
	}

	if (!iface->handle) {
		WMA_LOGE("handle of vdev_id %d is NULL vdev is already freed",
			 vdev_id);
		cdf_mem_free(pdel_sta_self_req_param);
		pdel_sta_self_req_param = NULL;
		return status;
	}


	/* remove the interface from ath_dev */
	if (wma_unified_vdev_delete_send(wma_handle->wmi_handle, vdev_id)) {
		WMA_LOGE("Unable to remove an interface for ath_dev.");
		status = CDF_STATUS_E_FAILURE;
		goto out;
	}

	WMA_LOGD("vdev_id:%hu vdev_hdl:%p", vdev_id, iface->handle);
	if (!generateRsp) {
		WMA_LOGE("Call txrx detach w/o callback for vdev %d", vdev_id);
		ol_txrx_vdev_detach(iface->handle, NULL, NULL);
		goto out;
	}

	iface->del_staself_req = pdel_sta_self_req_param;
	msg = wma_fill_vdev_req(wma_handle, vdev_id, WMA_DEL_STA_SELF_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_DEL, iface, 2000);
	if (!msg) {
		WMA_LOGE("%s: Failed to fill vdev request for vdev_id %d",
			 __func__, vdev_id);
		status = CDF_STATUS_E_NOMEM;
		goto out;
	}
	WMA_LOGD("Call txrx detach with callback for vdev %d", vdev_id);
	ol_txrx_vdev_detach(iface->handle, NULL, NULL);
	wma_vdev_detach_callback(iface);
	return status;
out:
	if (iface->addBssStaContext)
		cdf_mem_free(iface->addBssStaContext);
#if defined WLAN_FEATURE_VOWIFI_11R
	if (iface->staKeyParams)
		cdf_mem_free(iface->staKeyParams);
#endif /* WLAN_FEATURE_VOWIFI_11R */
	cdf_mem_zero(iface, sizeof(*iface));
	pdel_sta_self_req_param->status = status;
	if (generateRsp) {
		sme_msg.type = eWNI_SME_DEL_STA_SELF_RSP;
		sme_msg.bodyptr = pdel_sta_self_req_param;
		sme_msg.bodyval = 0;

		status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			WMA_LOGE("Failed to post eWNI_SME_ADD_STA_SELF_RSP");
			cdf_mem_free(pdel_sta_self_req_param);
		}
	}
	return status;
}

/**
 * wmi_unified_peer_delete_send() - send PEER delete command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac addr
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
static int32_t wmi_unified_peer_delete_send(wmi_unified_t wmi,
					    uint8_t
					    peer_addr[IEEE80211_ADDR_LEN],
					    uint8_t vdev_id)
{
	wmi_peer_delete_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_delete_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_delete_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_delete_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_DELETE_CMDID)) {
		WMA_LOGP("%s: Failed to send peer delete command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d", __func__, peer_addr, vdev_id);
	return 0;
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

#ifdef QCA_IBSS_SUPPORT
	WMA_LOGD("%s: vdev start response received for %s mode", __func__,
		 add_bss->operMode ==
		 BSS_OPERATIONAL_MODE_IBSS ? "IBSS" : "non-IBSS");
#endif /* QCA_IBSS_SUPPORT */

	if (resp_event->status) {
		add_bss->status = CDF_STATUS_E_FAILURE;
		goto send_fail_resp;
	}

	if ((add_bss->operMode == BSS_OPERATIONAL_MODE_AP)
#ifdef QCA_IBSS_SUPPORT
	    || (add_bss->operMode == BSS_OPERATIONAL_MODE_IBSS)
#endif /* QCA_IBSS_SUPPORT */
	    ) {
		wma->interfaces[resp_event->vdev_id].beacon =
			cdf_mem_malloc(sizeof(struct beacon_info));

		bcn = wma->interfaces[resp_event->vdev_id].beacon;
		if (!bcn) {
			WMA_LOGE("%s: Failed alloc memory for beacon struct",
				 __func__);
			add_bss->status = CDF_STATUS_E_NOMEM;
			goto send_fail_resp;
		}
		cdf_mem_zero(bcn, sizeof(*bcn));
		bcn->buf = cdf_nbuf_alloc(NULL, WMA_BCN_BUF_MAX_SIZE, 0,
					  sizeof(uint32_t), 0);
		if (!bcn->buf) {
			WMA_LOGE("%s: No memory allocated for beacon buffer",
				 __func__);
			cdf_mem_free(bcn);
			add_bss->status = CDF_STATUS_E_FAILURE;
			goto send_fail_resp;
		}
		bcn->seq_no = MIN_SW_SEQ;
		cdf_spinlock_init(&bcn->lock);
		cdf_atomic_set(&wma->interfaces[resp_event->vdev_id].bss_status,
			       WMA_BSS_STATUS_STARTED);
		WMA_LOGD("%s: AP mode (type %d subtype %d) BSS is started",
			 __func__, wma->interfaces[resp_event->vdev_id].type,
			 wma->interfaces[resp_event->vdev_id].sub_type);

		WMA_LOGD("%s: Allocated beacon struct %p, template memory %p",
			 __func__, bcn, bcn->buf);
	}
	add_bss->status = CDF_STATUS_SUCCESS;
	add_bss->bssIdx = resp_event->vdev_id;
	add_bss->chainMask = resp_event->chain_mask;
	if ((2 != resp_event->cfgd_rx_streams) ||
		(2 != resp_event->cfgd_tx_streams)) {
		add_bss->nss = 1;
	}
	add_bss->smpsMode = host_map_smps_mode(resp_event->smps_mode);
send_fail_resp:
	WMA_LOGD("%s: Sending add bss rsp to umac(vdev %d status %d)",
		 __func__, resp_event->vdev_id, add_bss->status);
	wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
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
void wma_find_mcc_ap(tp_wma_handle wma, uint8_t vdev_id, bool add)
{
	uint8_t i;
	uint16_t prev_ch_freq = 0;
	bool is_ap = false;
	bool result = false;
	uint8_t *ap_vdev_ids = NULL;
	uint8_t num_ch = 0;

	ap_vdev_ids = cdf_mem_malloc(wma->max_bssid);
	if (!ap_vdev_ids)
		return;

	for (i = 0; i < wma->max_bssid; i++) {
		ap_vdev_ids[i] = -1;
		if (add == false && i == vdev_id)
			continue;

		if (wma->interfaces[i].vdev_up || (i == vdev_id && add)) {
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
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	tpAniSirGlobal mac_ctx = cds_get_context(CDF_MODULE_ID_PE);
	if (NULL == mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		return -EINVAL;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	WMA_LOGD("%s: Enter", __func__);
	param_buf = (WMI_VDEV_START_RESP_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid start response event buffer");
		return -EINVAL;
	}

	resp_event = param_buf->fixed_param;
	if (!resp_event) {
		WMA_LOGE("Invalid start response event buffer");
		return -EINVAL;
	}

	if (wma_is_vdev_in_ap_mode(wma, resp_event->vdev_id)) {
		cdf_spin_lock_bh(&wma->dfs_ic->chan_lock);
		wma->dfs_ic->disable_phy_err_processing = false;
		cdf_spin_unlock_bh(&wma->dfs_ic->chan_lock);
	}

	if (resp_event->status == CDF_STATUS_SUCCESS) {
		wma->interfaces[resp_event->vdev_id].tx_streams =
			resp_event->cfgd_tx_streams;
		wma->interfaces[resp_event->vdev_id].rx_streams =
			resp_event->cfgd_rx_streams;
		wma->interfaces[resp_event->vdev_id].chain_mask =
			resp_event->chain_mask;
		wma->interfaces[resp_event->vdev_id].mac_id =
			resp_event->mac_id;
		WMA_LOGI("%s: vdev:%d tx ss=%d rx ss=%d chain mask=%d mac=%d",
				__func__,
				resp_event->vdev_id,
				wma->interfaces[resp_event->vdev_id].tx_streams,
				wma->interfaces[resp_event->vdev_id].rx_streams,
				wma->interfaces[resp_event->vdev_id].chain_mask,
				wma->interfaces[resp_event->vdev_id].mac_id);
	}

	if ((resp_event->vdev_id <= wma->max_bssid) &&
	    (cdf_atomic_read
		(&wma->interfaces[resp_event->vdev_id].vdev_restart_params.hidden_ssid_restart_in_progress))
	    && (wma_is_vdev_in_ap_mode(wma, resp_event->vdev_id) == true)) {
		WMA_LOGE("%s: vdev restart event recevied for hidden ssid set using IOCTL",
			__func__);

		if (wmi_unified_vdev_up_send
			    (wma->wmi_handle, resp_event->vdev_id, 0,
			    wma->interfaces[resp_event->vdev_id].bssid) < 0) {
			WMA_LOGE("%s : failed to send vdev up", __func__);
			return -EEXIST;
		}
		cdf_atomic_set(&wma->interfaces[resp_event->vdev_id].
			       vdev_restart_params.
			       hidden_ssid_restart_in_progress, 0);
		wma->interfaces[resp_event->vdev_id].vdev_up = true;
	}

	req_msg = wma_find_vdev_req(wma, resp_event->vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);

	if (!req_msg) {
		WMA_LOGE("%s: Failed to lookup request message for vdev %d",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}

	cdf_mc_timer_stop(&req_msg->event_timeout);

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	if (resp_event->status == CDF_STATUS_SUCCESS
		&& mac_ctx->sap.sap_channel_avoidance)
		wma_find_mcc_ap(wma, resp_event->vdev_id, true);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	iface = &wma->interfaces[resp_event->vdev_id];
	if (req_msg->msg_type == WMA_CHNL_SWITCH_REQ) {
		tpSwitchChannelParams params =
			(tpSwitchChannelParams) req_msg->user_data;
		if (!params) {
			WMA_LOGE("%s: channel switch params is NULL for vdev %d",
				__func__, resp_event->vdev_id);
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
		if (resp_event->resp_type == WMI_VDEV_RESTART_RESP_EVENT &&
		    (iface->type == WMI_VDEV_TYPE_STA)) {
			if (wmi_unified_vdev_up_send(wma->wmi_handle,
						     resp_event->vdev_id,
						     iface->aid,
						     iface->bssid)) {
				WMA_LOGE("%s:vdev_up failed vdev_id %d",
					 __func__, resp_event->vdev_id);
				wma->interfaces[resp_event->vdev_id].vdev_up =
					false;
			} else {
				wma->interfaces[resp_event->vdev_id].vdev_up =
					true;
			}
		}

		wma_send_msg(wma, WMA_SWITCH_CHANNEL_RSP, (void *)params, 0);
	} else if (req_msg->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams bssParams = (tpAddBssParams) req_msg->user_data;
		cdf_mem_copy(iface->bssid, bssParams->bssId,
				IEEE80211_ADDR_LEN);
		wma_vdev_start_rsp(wma, bssParams, resp_event);
	} else if (req_msg->msg_type == WMA_OCB_SET_CONFIG_CMD) {
		if (wmi_unified_vdev_up_send(wma->wmi_handle,
					     resp_event->vdev_id, iface->aid,
					     iface->bssid) < 0) {
			WMA_LOGE(FL("failed to send vdev up"));
			return -EEXIST;
		}
		iface->vdev_up = true;

		wma_ocb_start_resp_ind_cont(wma);
	}

	if ((wma->interfaces[resp_event->vdev_id].type == WMI_VDEV_TYPE_AP) &&
		wma->interfaces[resp_event->vdev_id].vdev_up)
		wma_set_sap_keepalive(wma, resp_event->vdev_id);

	cdf_mc_timer_destroy(&req_msg->event_timeout);
	cdf_mem_free(req_msg);

	return 0;
}

/**
 * wmi_unified_vdev_set_param_send() - set per vdev params in fw
 * @wmi_handle: wmi handle
 * @if_if: vdev id
 * @param_id: parameter id
 * @param_value: parameter value
 *
 * Return: 0 for success or error code
 */
int
wmi_unified_vdev_set_param_send(wmi_unified_t wmi_handle, uint32_t if_id,
				uint32_t param_id, uint32_t param_value)
{
	int ret;
	wmi_vdev_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint16_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s:wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_set_param_cmd_fixed_param));
	cmd->vdev_id = if_id;
	cmd->param_id = param_id;
	cmd->param_value = param_value;
	WMA_LOGD("Setting vdev %d param = %x, value = %u",
		 if_id, param_id, param_value);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_VDEV_SET_PARAM_CMDID);
	if (ret < 0) {
		WMA_LOGE("Failed to send set param command ret = %d", ret);
		wmi_buf_free(buf);
	}
	return ret;
}

/**
 * wmi_unified_peer_flush_tids_send() - flush peer tids packets in fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_tid_bitmap: peer tid bitmap
 * @vdev_id: vdev id
 *
 * Return: 0 for sucess or error code
 */
int32_t wmi_unified_peer_flush_tids_send(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 uint32_t peer_tid_bitmap,
					 uint8_t vdev_id)
{
	wmi_peer_flush_tids_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_flush_tids_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_flush_tids_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_flush_tids_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_tid_bitmap = peer_tid_bitmap;
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_FLUSH_TIDS_CMDID)) {
		WMA_LOGP("%s: Failed to send flush tid command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d", __func__, peer_addr, vdev_id);
	return 0;
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
 * Return: 0 for success or error code
 */
int wma_set_peer_param(void *wma_ctx, uint8_t *peer_addr, uint32_t param_id,
		       uint32_t param_value, uint32_t vdev_id)
{
	tp_wma_handle wma_handle = (tp_wma_handle) wma_ctx;
	wmi_peer_set_param_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int err;

	buf = wmi_buf_alloc(wma_handle->wmi_handle, sizeof(*cmd));
	if (!buf) {
		WMA_LOGE("Failed to allocate buffer to send set_param cmd");
		return -ENOMEM;
	}
	cmd = (wmi_peer_set_param_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_set_param_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
				(wmi_peer_set_param_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->param_id = param_id;
	cmd->param_value = param_value;
	err = wmi_unified_cmd_send(wma_handle->wmi_handle, buf,
				   sizeof(wmi_peer_set_param_cmd_fixed_param),
				   WMI_PEER_SET_PARAM_CMDID);
	if (err) {
		WMA_LOGE("Failed to send set_param cmd");
		cdf_mem_free(buf);
		return -EIO;
	}
	return 0;
}

/**
 * wma_remove_peer() - send remove peer command to fw
 * @wma: wma handle
 * @bssid: mac address
 * @vdev_id: vdev id
 * @peer: peer ptr
 * @roam_synch_in_progress: roam in progress flag
 *
 * Return: none
 */
void wma_remove_peer(tp_wma_handle wma, uint8_t *bssid,
			    uint8_t vdev_id, ol_txrx_peer_handle peer,
			    bool roam_synch_in_progress)
{
#define PEER_ALL_TID_BITMASK 0xffffffff
	uint32_t peer_tid_bitmap = PEER_ALL_TID_BITMASK;
	uint8_t *peer_addr = bssid;
	if (!wma->interfaces[vdev_id].peer_count) {
		WMA_LOGE("%s: Can't remove peer with peer_addr %pM vdevid %d peer_count %d",
			__func__, bssid, vdev_id,
			wma->interfaces[vdev_id].peer_count);
		return;
	}
	if (peer)
		ol_txrx_peer_detach(peer);

	wma->interfaces[vdev_id].peer_count--;
	WMA_LOGE("%s: Removed peer with peer_addr %pM vdevid %d peer_count %d",
		 __func__, bssid, vdev_id, wma->interfaces[vdev_id].peer_count);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (roam_synch_in_progress)
		return;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	/* Flush all TIDs except MGMT TID for this peer in Target */
	peer_tid_bitmap &= ~(0x1 << WMI_MGMT_TID);
	wmi_unified_peer_flush_tids_send(wma->wmi_handle, bssid,
					 peer_tid_bitmap, vdev_id);

#if defined(QCA_IBSS_SUPPORT)
	if ((peer) && (wma_is_vdev_in_ibss_mode(wma, vdev_id))) {
		WMA_LOGD("%s: bssid %pM peer->mac_addr %pM", __func__,
			 bssid, peer->mac_addr.raw);
		peer_addr = peer->mac_addr.raw;
	}
#endif /* QCA_IBSS_SUPPORT */

	wmi_unified_peer_delete_send(wma->wmi_handle, peer_addr, vdev_id);
#undef PEER_ALL_TID_BITMASK
}

/**
 * wmi_unified_peer_create_send() - send peer create command to fw
 * @wmi: wmi handle
 * @peer_addr: peer mac address
 * @peer_type: peer type
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
static int wmi_unified_peer_create_send(wmi_unified_t wmi,
					const uint8_t *peer_addr,
					uint32_t peer_type, uint32_t vdev_id)
{
	wmi_peer_create_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_peer_create_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_peer_create_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_peer_create_cmd_fixed_param));
	WMI_CHAR_ARRAY_TO_MAC_ADDR(peer_addr, &cmd->peer_macaddr);
	cmd->peer_type = peer_type;
	cmd->vdev_id = vdev_id;

	if (wmi_unified_cmd_send(wmi, buf, len, WMI_PEER_CREATE_CMDID)) {
		WMA_LOGP("%s: failed to send WMI_PEER_CREATE_CMDID", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGD("%s: peer_addr %pM vdev_id %d", __func__, peer_addr, vdev_id);
	return 0;
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
 * Return: CDF status
 */
CDF_STATUS wma_create_peer(tp_wma_handle wma, ol_txrx_pdev_handle pdev,
			  ol_txrx_vdev_handle vdev,
			  u8 peer_addr[IEEE80211_ADDR_LEN],
			  uint32_t peer_type, uint8_t vdev_id,
			  bool roam_synch_in_progress)
{
	ol_txrx_peer_handle peer;

	if (++wma->interfaces[vdev_id].peer_count >
	    wma->wlan_resource_config.num_peers) {
		WMA_LOGP("%s, the peer count exceeds the limit %d", __func__,
			 wma->interfaces[vdev_id].peer_count - 1);
		goto err;
	}
	peer = ol_txrx_peer_attach(pdev, vdev, peer_addr);
	if (!peer) {
		WMA_LOGE("%s : Unable to attach peer %pM", __func__, peer_addr);
		goto err;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (roam_synch_in_progress) {

		WMA_LOGE("%s: Created peer with peer_addr %pM vdev_id %d,"
			 "peer_count - %d", __func__, peer_addr, vdev_id,
			 wma->interfaces[vdev_id].peer_count);
		return CDF_STATUS_SUCCESS;
	}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	if (wmi_unified_peer_create_send(wma->wmi_handle, peer_addr,
					 peer_type, vdev_id) < 0) {
		WMA_LOGP("%s : Unable to create peer in Target", __func__);
		ol_txrx_peer_detach(peer);
		goto err;
	}
	WMA_LOGE("%s: Created peer with peer_addr %pM vdev_id %d, peer_count - %d",
		__func__, peer_addr, vdev_id, wma->interfaces[vdev_id].peer_count);

#ifdef QCA_IBSS_SUPPORT
	/* for each remote ibss peer, clear its keys */
	if (wma_is_vdev_in_ibss_mode(wma, vdev_id) &&
	    !cdf_mem_compare(peer_addr, vdev->mac_addr.raw,
				IEEE80211_ADDR_LEN)) {

		tSetStaKeyParams key_info;
		WMA_LOGD("%s: remote ibss peer %pM key clearing\n", __func__,
			 peer_addr);
		cdf_mem_set(&key_info, sizeof(key_info), 0);
		key_info.smesessionId = vdev_id;
		cdf_mem_copy(key_info.peerMacAddr, peer_addr,
				IEEE80211_ADDR_LEN);
		key_info.sendRsp = false;

		wma_set_stakey(wma, &key_info);
	}
#endif /* QCA_IBSS_SUPPORT */

	return CDF_STATUS_SUCCESS;
err:
	wma->interfaces[vdev_id].peer_count--;
	return CDF_STATUS_E_FAILURE;
}

/**
 * wmi_unified_vdev_down_send() - send vdev down command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or error code
 */
static int wmi_unified_vdev_down_send(wmi_unified_t wmi, uint8_t vdev_id)
{
	wmi_vdev_down_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_down_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_down_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_down_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_DOWN_CMDID)) {
		WMA_LOGP("%s: Failed to send vdev down", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	WMA_LOGE("%s: vdev_id %d", __func__, vdev_id);
	return 0;
}

#ifdef QCA_IBSS_SUPPORT

/**
 * wma_delete_all_ibss_peers() - delete all ibss peer for vdev_id
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * This function send peer delete command to fw for all
 * peers in peer_list  and remove ref count for peer id
 * peer will actually remove from list after receving
 * unmap event from firmware.
 *
 * Return: none
 */
static void wma_delete_all_ibss_peers(tp_wma_handle wma, A_UINT32 vdev_id)
{
	ol_txrx_vdev_handle vdev;
	ol_txrx_peer_handle peer, temp;

	if (!wma || vdev_id > wma->max_bssid)
		return;

	vdev = wma->interfaces[vdev_id].handle;
	if (!vdev)
		return;

	/* remove all remote peers of IBSS */
	cdf_spin_lock_bh(&vdev->pdev->peer_ref_mutex);

	temp = NULL;
	TAILQ_FOREACH_REVERSE(peer, &vdev->peer_list, peer_list_t, peer_list_elem) {
		if (temp) {
			cdf_spin_unlock_bh(&vdev->pdev->peer_ref_mutex);
			if (cdf_atomic_read(&temp->delete_in_progress) == 0) {
				wma_remove_peer(wma, temp->mac_addr.raw,
					vdev_id, temp, false);
			}
			cdf_spin_lock_bh(&vdev->pdev->peer_ref_mutex);
		}
		/* self peer is deleted last */
		if (peer == TAILQ_FIRST(&vdev->peer_list)) {
			WMA_LOGE("%s: self peer removed by caller ", __func__);
			break;
		} else
			temp = peer;
	}
	cdf_spin_unlock_bh(&vdev->pdev->peer_ref_mutex);

	/* remove IBSS bss peer last */
	peer = TAILQ_FIRST(&vdev->peer_list);
	wma_remove_peer(wma, wma->interfaces[vdev_id].bssid, vdev_id, peer,
			false);
}

#endif /* QCA_IBSS_SUPPORT */

/**
 * wma_delete_all_ap_remote_peers() - delete all ap peer for vdev_id
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * This function send peer delete command to fw for all
 * peers in peer_list  and remove ref count for peer id
 * peer will actually remove from list after receving
 * unmap event from firmware.
 *
 * Return: none
 */
static void wma_delete_all_ap_remote_peers(tp_wma_handle wma, A_UINT32 vdev_id)
{
	ol_txrx_vdev_handle vdev;
	ol_txrx_peer_handle peer, temp;

	if (!wma || vdev_id > wma->max_bssid)
		return;

	vdev = wma->interfaces[vdev_id].handle;
	if (!vdev)
		return;

	WMA_LOGE("%s: vdev_id - %d", __func__, vdev_id);
	/* remove all remote peers of SAP */
	cdf_spin_lock_bh(&vdev->pdev->peer_ref_mutex);

	temp = NULL;
	TAILQ_FOREACH_REVERSE(peer, &vdev->peer_list, peer_list_t,
			      peer_list_elem) {
		if (temp) {
			cdf_spin_unlock_bh(&vdev->pdev->peer_ref_mutex);
			if (cdf_atomic_read(&temp->delete_in_progress) == 0) {
				wma_remove_peer(wma, temp->mac_addr.raw,
						vdev_id, temp, false);
			}
			cdf_spin_lock_bh(&vdev->pdev->peer_ref_mutex);
		}
		/* self peer is deleted by caller */
		if (peer == TAILQ_FIRST(&vdev->peer_list)) {
			WMA_LOGE("%s: self peer removed by caller ", __func__);
			break;
		} else
			temp = peer;
	}

	cdf_spin_unlock_bh(&vdev->pdev->peer_ref_mutex);
}

#ifdef QCA_IBSS_SUPPORT

/**
 * wma_recreate_ibss_vdev_and_bss_peer() - recreate IBSS vdev and create peer
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return: none
 */
static void wma_recreate_ibss_vdev_and_bss_peer(tp_wma_handle wma,
						uint8_t vdev_id)
{
	ol_txrx_vdev_handle vdev;
	struct add_sta_self_params add_sta_self_param;
	struct del_sta_self_params del_sta_param;
	CDF_STATUS status;

	if (!wma) {
		WMA_LOGE("%s: Null wma handle", __func__);
		return;
	}

	vdev = wma_find_vdev_by_id(wma, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: Can't find vdev with id %d", __func__, vdev_id);
		return;
	}

	cdf_copy_macaddr(
		(struct cdf_mac_addr *) &(add_sta_self_param.self_mac_addr),
		(struct cdf_mac_addr *) &(vdev->mac_addr));
	add_sta_self_param.session_id = vdev_id;
	add_sta_self_param.type = WMI_VDEV_TYPE_IBSS;
	add_sta_self_param.sub_type = 0;
	add_sta_self_param.status = 0;

	/* delete old ibss vdev */
	del_sta_param.session_id = vdev_id;
	cdf_mem_copy((void *)del_sta_param.self_mac_addr,
		     (void *)&(vdev->mac_addr), CDF_MAC_ADDR_SIZE);
	wma_vdev_detach(wma, &del_sta_param, 0);

	/* create new vdev for ibss */
	vdev = wma_vdev_attach(wma, &add_sta_self_param, 0);
	if (!vdev) {
		WMA_LOGE("%s: Failed to create vdev", __func__);
		return;
	}

	/* Register with TxRx Module for Data Ack Complete Cb */
	ol_txrx_data_tx_cb_set(vdev, wma_data_tx_ack_comp_hdlr, wma);
	WMA_LOGA("new IBSS vdev created with mac %pM",
		 add_sta_self_param.self_mac_addr);

	/* create ibss bss peer */
	status = wma_create_peer(wma, vdev->pdev, vdev, vdev->mac_addr.raw,
				 WMI_PEER_TYPE_DEFAULT, vdev_id, false);
	if (status != CDF_STATUS_SUCCESS)
		WMA_LOGE("%s: Failed to create IBSS bss peer", __func__);
	else
		WMA_LOGA("IBSS BSS peer created with mac %pM",
			 vdev->mac_addr.raw);
}
#endif /* QCA_IBSS_SUPPORT */

/**
 * wma_hidden_ssid_vdev_restart_on_vdev_stop() - restart vdev to set hidden ssid
 * @wma_handle: wma handle
 * @sessionId: session id
 *
 * Return: none
 */
void wma_hidden_ssid_vdev_restart_on_vdev_stop(tp_wma_handle wma_handle,
					       uint8_t sessionId)
{
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	wmi_channel *chan;
	int32_t len;
	uint8_t *buf_ptr;
	struct wma_txrx_node *intr = wma_handle->interfaces;
	int32_t ret = 0;

	len = sizeof(*cmd) + sizeof(wmi_channel) + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wma_handle->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s : wmi_buf_alloc failed", __func__);
		cdf_atomic_set(&intr[sessionId].vdev_restart_params.
			       hidden_ssid_restart_in_progress, 0);
		return;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_start_request_cmd_fixed_param *) buf_ptr;
	chan = (wmi_channel *) (buf_ptr + sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_start_request_cmd_fixed_param));

	WMITLV_SET_HDR(&chan->tlv_header,
		       WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));

	cmd->vdev_id = sessionId;
	cmd->ssid.ssid_len = intr[sessionId].vdev_restart_params.ssid.ssid_len;
	cdf_mem_copy(cmd->ssid.ssid,
		     intr[sessionId].vdev_restart_params.ssid.ssid,
		     cmd->ssid.ssid_len);
	cmd->flags = intr[sessionId].vdev_restart_params.flags;
	if (intr[sessionId].vdev_restart_params.ssidHidden)
		cmd->flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;
	else
		cmd->flags &= (0xFFFFFFFE);
	cmd->requestor_id = intr[sessionId].vdev_restart_params.requestor_id;
	cmd->disable_hw_ack =
		intr[sessionId].vdev_restart_params.disable_hw_ack;

	chan->mhz = intr[sessionId].vdev_restart_params.chan.mhz;
	chan->band_center_freq1 =
		intr[sessionId].vdev_restart_params.chan.band_center_freq1;
	chan->band_center_freq2 =
		intr[sessionId].vdev_restart_params.chan.band_center_freq2;
	chan->info = intr[sessionId].vdev_restart_params.chan.info;
	chan->reg_info_1 = intr[sessionId].vdev_restart_params.chan.reg_info_1;
	chan->reg_info_2 = intr[sessionId].vdev_restart_params.chan.reg_info_2;

	cmd->num_noa_descriptors = 0;
	buf_ptr = (uint8_t *) (((uint8_t *) cmd) + sizeof(*cmd) +
			       sizeof(wmi_channel));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->num_noa_descriptors *
		       sizeof(wmi_p2p_noa_descriptor));

	ret = wmi_unified_cmd_send(wma_handle->wmi_handle, buf, len,
				   WMI_VDEV_RESTART_REQUEST_CMDID);
	if (ret < 0) {
		WMA_LOGE("%s: Failed to send vdev restart command", __func__);
		cdf_atomic_set(&intr[sessionId].vdev_restart_params.
			       hidden_ssid_restart_in_progress, 0);
		cdf_nbuf_free(buf);
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
	struct wma_target_req *req_msg;
	ol_txrx_peer_handle peer;
	ol_txrx_pdev_handle pdev;
	uint8_t peer_id;
	struct wma_txrx_node *iface;
	int32_t status = 0;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	tpAniSirGlobal mac_ctx = cds_get_context(CDF_MODULE_ID_PE);
	if (NULL == mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		return -EINVAL;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	WMA_LOGI("%s: Enter", __func__);
	param_buf = (WMI_VDEV_STOPPED_EVENTID_param_tlvs *) cmd_param_info;
	if (!param_buf) {
		WMA_LOGE("Invalid event buffer");
		return -EINVAL;
	}
	resp_event = param_buf->fixed_param;

	if ((resp_event->vdev_id <= wma->max_bssid) &&
	    (cdf_atomic_read
		     (&wma->interfaces[resp_event->vdev_id].vdev_restart_params.
		     hidden_ssid_restart_in_progress))
	    && ((wma->interfaces[resp_event->vdev_id].type == WMI_VDEV_TYPE_AP)
		&& (wma->interfaces[resp_event->vdev_id].sub_type == 0))) {
		WMA_LOGE("%s: vdev stop event recevied for hidden ssid set using IOCTL ",
			__func__);
		wma_hidden_ssid_vdev_restart_on_vdev_stop(wma,
							  resp_event->vdev_id);
	}

	req_msg = wma_find_vdev_req(wma, resp_event->vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_STOP);
	if (!req_msg) {
		WMA_LOGP("%s: Failed to lookup vdev request for vdev id %d",
			 __func__, resp_event->vdev_id);
		return -EINVAL;
	}
	pdev = cds_get_context(CDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE("%s: pdev is NULL", __func__);
		status = -EINVAL;
		cdf_mc_timer_stop(&req_msg->event_timeout);
		goto free_req_msg;
	}

	cdf_mc_timer_stop(&req_msg->event_timeout);
	if (req_msg->msg_type == WMA_DELETE_BSS_REQ) {
		tpDeleteBssParams params =
			(tpDeleteBssParams) req_msg->user_data;
		struct beacon_info *bcn;
		if (resp_event->vdev_id > wma->max_bssid) {
			WMA_LOGE("%s: Invalid vdev_id %d", __func__,
				 resp_event->vdev_id);
			status = -EINVAL;
			goto free_req_msg;
		}

		iface = &wma->interfaces[resp_event->vdev_id];
		if (iface->handle == NULL) {
			WMA_LOGE("%s vdev id %d is already deleted",
				 __func__, resp_event->vdev_id);
			status = -EINVAL;
			goto free_req_msg;
		}
#ifdef QCA_IBSS_SUPPORT
		if (wma_is_vdev_in_ibss_mode(wma, resp_event->vdev_id))
			wma_delete_all_ibss_peers(wma, resp_event->vdev_id);
		else
#endif /* QCA_IBSS_SUPPORT */
		{
			if (wma_is_vdev_in_ap_mode(wma, resp_event->vdev_id)) {
				wma_delete_all_ap_remote_peers(wma,
						resp_event->vdev_id);
			}
			peer = ol_txrx_find_peer_by_addr(pdev, params->bssid,
							 &peer_id);
			if (!peer)
				WMA_LOGD("%s Failed to find peer %pM",
					 __func__, params->bssid);
			wma_remove_peer(wma, params->bssid, resp_event->vdev_id,
					peer, false);
		}

		if (wmi_unified_vdev_down_send
			    (wma->wmi_handle, resp_event->vdev_id) < 0) {
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
				 resp_event->vdev_id);
		} else {
			wma->interfaces[resp_event->vdev_id].vdev_up = false;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		if (mac_ctx->sap.sap_channel_avoidance)
			wma_find_mcc_ap(wma, resp_event->vdev_id, false);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
		}
		ol_txrx_vdev_flush(iface->handle);
		WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for VDEV_STOP rsp",
			 __func__, resp_event->vdev_id);
		ol_txrx_vdev_unpause(iface->handle,
				     OL_TXQ_PAUSE_REASON_VDEV_STOP);
		iface->pause_bitmap &= ~(1 << PAUSE_TYPE_HOST);
		cdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
		WMA_LOGD("%s: (type %d subtype %d) BSS is stopped",
			 __func__, iface->type, iface->sub_type);
		bcn = wma->interfaces[resp_event->vdev_id].beacon;

		if (bcn) {
			WMA_LOGD("%s: Freeing beacon struct %p, "
				 "template memory %p", __func__, bcn, bcn->buf);
			if (bcn->dma_mapped)
				cdf_nbuf_unmap_single(pdev->osdev, bcn->buf,
						      CDF_DMA_TO_DEVICE);
			cdf_nbuf_free(bcn->buf);
			cdf_mem_free(bcn);
			wma->interfaces[resp_event->vdev_id].beacon = NULL;
		}
#ifdef QCA_IBSS_SUPPORT
		/* recreate ibss vdev and bss peer for scan purpose */
		if (wma_is_vdev_in_ibss_mode(wma, resp_event->vdev_id))
			wma_recreate_ibss_vdev_and_bss_peer(wma,
						resp_event->vdev_id);
#endif /* QCA_IBSS_SUPPORT */
		/* Timeout status means its WMA generated DEL BSS REQ when ADD
		 * BSS REQ was timed out to stop the VDEV in this case no need
		 * to send response to UMAC
		 */
		if (params->status == CDF_STATUS_FW_MSG_TIMEDOUT) {
			cdf_mem_free(params);
			WMA_LOGE("%s: DEL BSS from ADD BSS timeout do not send "
				 "resp to UMAC (vdev id %x)",
				 __func__, resp_event->vdev_id);
		} else {
			params->status = CDF_STATUS_SUCCESS;
			wma_send_msg(wma, WMA_DELETE_BSS_RSP, (void *)params,
				     0);
		}

		if (iface->del_staself_req) {
			WMA_LOGA("scheduling defered deletion (vdev id %x)",
				 resp_event->vdev_id);
			wma_vdev_detach(wma, iface->del_staself_req, 1);
		}
	}
free_req_msg:
	cdf_mc_timer_destroy(&req_msg->event_timeout);
	cdf_mem_free(req_msg);
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
ol_txrx_vdev_handle wma_vdev_attach(tp_wma_handle wma_handle,
				struct add_sta_self_params *self_sta_req,
				uint8_t generateRsp)
{
	ol_txrx_vdev_handle txrx_vdev_handle = NULL;
	ol_txrx_pdev_handle txrx_pdev = cds_get_context(CDF_MODULE_ID_TXRX);
	enum wlan_op_mode txrx_vdev_type;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	struct sAniSirGlobal *mac = cds_get_context(CDF_MODULE_ID_PE);
	uint32_t cfg_val;
	uint16_t val16;
	int ret;
	tSirMacHTCapabilityInfo *phtCapInfo;
	cds_msg_t sme_msg = { 0 };

	if (NULL == mac) {
		WMA_LOGE("%s: Failed to get mac", __func__);
		goto end;
	}

	/* Create a vdev in target */
	if (wma_unified_vdev_create_send(wma_handle->wmi_handle,
					 self_sta_req->session_id,
					 self_sta_req->type,
					 self_sta_req->sub_type,
					 self_sta_req->self_mac_addr)) {
		WMA_LOGP("%s: Unable to add an interface for ath_dev",
			 __func__);
		status = CDF_STATUS_E_RESOURCES;
		goto end;
	}

	txrx_vdev_type = wma_get_txrx_vdev_type(self_sta_req->type);

	if (wlan_op_mode_unknown == txrx_vdev_type) {
		WMA_LOGE("Failed to get txrx vdev type");
		wma_unified_vdev_delete_send(wma_handle->wmi_handle,
					     self_sta_req->session_id);
		goto end;
	}

	txrx_vdev_handle = ol_txrx_vdev_attach(txrx_pdev,
					       self_sta_req->self_mac_addr,
					       self_sta_req->session_id,
					       txrx_vdev_type);
	wma_handle->interfaces[self_sta_req->session_id].pause_bitmap = 0;

	WMA_LOGD("vdev_id %hu, txrx_vdev_handle = %p", self_sta_req->session_id,
		 txrx_vdev_handle);

	if (NULL == txrx_vdev_handle) {
		WMA_LOGP("%s: ol_txrx_vdev_attach failed", __func__);
		status = CDF_STATUS_E_FAILURE;
		wma_unified_vdev_delete_send(wma_handle->wmi_handle,
					     self_sta_req->session_id);
		goto end;
	}
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

	cdf_mem_copy(wma_handle->interfaces[self_sta_req->session_id].addr,
		     self_sta_req->self_mac_addr,
		     sizeof(wma_handle->interfaces[self_sta_req->session_id].
			    addr));
	switch (self_sta_req->type) {
	case WMI_VDEV_TYPE_STA:
		if (wlan_cfg_get_int(mac, WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD,
				     &cfg_val) != eSIR_SUCCESS) {
			WMA_LOGE("Failed to get value for "
				 "WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD");
			cfg_val = DEFAULT_INFRA_STA_KEEP_ALIVE_PERIOD;
		}

		wma_set_sta_keep_alive(wma_handle,
				       self_sta_req->session_id,
				       SIR_KEEP_ALIVE_NULL_PKT,
				       cfg_val, NULL, NULL, NULL);

		/* offload STA SA query related params to fwr */
		if (WMI_SERVICE_IS_ENABLED(wma_handle->wmi_service_bitmap,
			WMI_SERVICE_STA_PMF_OFFLOAD)) {
			wma_set_sta_sa_query_param(wma_handle,
						   self_sta_req->session_id);
		}
		break;
	}

	wma_handle->interfaces[self_sta_req->session_id].type =
		self_sta_req->type;
	wma_handle->interfaces[self_sta_req->session_id].sub_type =
		self_sta_req->sub_type;
	cdf_atomic_init(&wma_handle->interfaces
			[self_sta_req->session_id].bss_status);

	if (((self_sta_req->type == WMI_VDEV_TYPE_AP) &&
	    (self_sta_req->sub_type == WMI_UNIFIED_VDEV_SUBTYPE_P2P_DEVICE)) ||
	    (self_sta_req->type == WMI_VDEV_TYPE_OCB)) {
		WMA_LOGA("P2P Device: creating self peer %pM, vdev_id %hu",
			 self_sta_req->self_mac_addr, self_sta_req->session_id);
		status = wma_create_peer(wma_handle, txrx_pdev,
					 txrx_vdev_handle,
					 self_sta_req->self_mac_addr,
					 WMI_PEER_TYPE_DEFAULT,
					 self_sta_req->session_id, false);
		if (status != CDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to create peer", __func__);
			status = CDF_STATUS_E_FAILURE;
			wma_unified_vdev_delete_send(wma_handle->wmi_handle,
						     self_sta_req->session_id);
		}
	}

	ret = wmi_unified_vdev_set_param_send(wma_handle->wmi_handle,
				self_sta_req->session_id,
				WMI_VDEV_PARAM_MCC_RTSCTS_PROTECTION_ENABLE,
				mac->roam.configParam.mcc_rts_cts_prot_enable);
	if (ret)
		WMA_LOGE("Failed to set WMI VDEV MCC_RTSCTS_PROTECTION_ENABLE");

	ret = wmi_unified_vdev_set_param_send(wma_handle->wmi_handle,
			self_sta_req->session_id,
			WMI_VDEV_PARAM_MCC_BROADCAST_PROBE_ENABLE,
			mac->roam.configParam.mcc_bcast_prob_resp_enable);
	if (ret)
		WMA_LOGE("Failed to set WMI VDEV MCC_BROADCAST_PROBE_ENABLE");

	if (wlan_cfg_get_int(mac, WNI_CFG_RTS_THRESHOLD,
			     &cfg_val) == eSIR_SUCCESS) {
		ret = wmi_unified_vdev_set_param_send(wma_handle->wmi_handle,
						      self_sta_req->session_id,
						      WMI_VDEV_PARAM_RTS_THRESHOLD,
						      cfg_val);
		if (ret)
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_RTS_THRESHOLD");
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_RTS_THRESHOLD, leaving unchanged");
	}

	if (wlan_cfg_get_int(mac, WNI_CFG_FRAGMENTATION_THRESHOLD,
			     &cfg_val) == eSIR_SUCCESS) {
		ret = wmi_unified_vdev_set_param_send(wma_handle->wmi_handle,
						      self_sta_req->session_id,
						      WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD,
						      cfg_val);
		if (ret)
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_FRAGMENTATION_THRESHOLD");
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_FRAGMENTATION_THRESHOLD, leaving unchanged");
	}

	if (wlan_cfg_get_int(mac, WNI_CFG_HT_CAP_INFO, &cfg_val) == eSIR_SUCCESS) {
		val16 = (uint16_t) cfg_val;
		phtCapInfo = (tSirMacHTCapabilityInfo *) &cfg_val;
		ret = wmi_unified_vdev_set_param_send(wma_handle->wmi_handle,
						      self_sta_req->session_id,
						      WMI_VDEV_PARAM_TX_STBC,
						      phtCapInfo->txSTBC);
		if (ret)
			WMA_LOGE("Failed to set WMI_VDEV_PARAM_TX_STBC");
	} else {
		WMA_LOGE("Failed to get value of HT_CAP, TX STBC unchanged");
	}
	/* Initialize roaming offload state */
	if ((self_sta_req->type == WMI_VDEV_TYPE_STA) &&
	    (self_sta_req->sub_type == 0)) {
		wma_handle->roam_offload_enabled = true;
		wmi_unified_vdev_set_param_send(wma_handle->wmi_handle,
						self_sta_req->session_id,
						WMI_VDEV_PARAM_ROAM_FW_OFFLOAD,
						(WMI_ROAM_FW_OFFLOAD_ENABLE_FLAG |
						 WMI_ROAM_BMISS_FINAL_SCAN_ENABLE_FLAG));
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
		if (ret != CDF_STATUS_SUCCESS) {
			WMA_LOGE("Failed to set WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED");
		}
	} else {
		WMA_LOGE("Failed to get value for WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED, leaving unchanged");
	}

	wma_register_wow_wakeup_events(wma_handle, self_sta_req->session_id,
					self_sta_req->type,
					self_sta_req->sub_type);
	wma_register_wow_default_patterns(wma_handle, self_sta_req->session_id);

end:
	self_sta_req->status = status;

#ifdef QCA_IBSS_SUPPORT
	if (generateRsp)
#endif
	{
		sme_msg.type = eWNI_SME_ADD_STA_SELF_RSP;
		sme_msg.bodyptr = self_sta_req;
		sme_msg.bodyval = 0;

		status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			WMA_LOGE("Failed to post eWNI_SME_ADD_STA_SELF_RSP");
			cdf_mem_free(self_sta_req);
		}
	}
	return txrx_vdev_handle;
}

/**
 * wma_get_center_channel() - get center channel
 * @chan: channel number
 * @chan_offset: channel offset
 *
 * Return: center channel
 */
uint8_t wma_get_center_channel(uint8_t chan, uint8_t chan_offset)
{
	uint8_t band_center_chan = 0;

	if ((chan_offset == PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_CENTERED) ||
	    (chan_offset == PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW))
		band_center_chan = chan + 2;
	else if (chan_offset == PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW)
		band_center_chan = chan + 6;
	else if ((chan_offset == PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH) ||
		 (chan_offset ==
		  PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_CENTERED))
		band_center_chan = chan - 2;
	else if (chan_offset == PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH)
		band_center_chan = chan - 6;

	return band_center_chan;
}

/**
 * wma_vdev_start() - send vdev start request to fw
 * @wma: wma handle
 * @req: vdev start params
 * @isRestart: isRestart flag
 *
 * Return: CDF status
 */
CDF_STATUS wma_vdev_start(tp_wma_handle wma,
			  struct wma_vdev_start_req *req, bool isRestart)
{
	wmi_vdev_start_request_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	wmi_channel *chan;
	int32_t len, ret;
	WLAN_PHY_MODE chanmode;
	uint8_t *buf_ptr;
	struct wma_txrx_node *intr = wma->interfaces;
	tpAniSirGlobal mac_ctx = NULL;
	struct ath_dfs *dfs;

	mac_ctx = cds_get_context(CDF_MODULE_ID_PE);
	if (mac_ctx == NULL) {
		WMA_LOGE("%s: vdev start failed as mac_ctx is NULL", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	dfs = (struct ath_dfs *)wma->dfs_ic->ic_dfs;

	WMA_LOGD("%s: Enter isRestart=%d vdev=%d", __func__, isRestart,
		 req->vdev_id);
	len = sizeof(*cmd) + sizeof(wmi_channel) + WMI_TLV_HDR_SIZE;
	buf = wmi_buf_alloc(wma->wmi_handle, len);
	if (!buf) {
		WMA_LOGE("%s : wmi_buf_alloc failed", __func__);
		return CDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (wmi_vdev_start_request_cmd_fixed_param *) buf_ptr;
	chan = (wmi_channel *) (buf_ptr + sizeof(*cmd));
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_start_request_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_vdev_start_request_cmd_fixed_param));
	WMITLV_SET_HDR(&chan->tlv_header, WMITLV_TAG_STRUC_wmi_channel,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_channel));
	cmd->vdev_id = req->vdev_id;

	/* Fill channel info */
	chan->mhz = cds_chan_to_freq(req->chan);
	chanmode = wma_chan_to_mode(req->chan, req->chan_width,
				    req->vht_capable, req->dot11_mode);

	intr[cmd->vdev_id].chanmode = chanmode; /* save channel mode */
	intr[cmd->vdev_id].ht_capable = req->ht_capable;
	intr[cmd->vdev_id].vht_capable = req->vht_capable;
	intr[cmd->vdev_id].config.gtx_info.gtxRTMask[0] =
		CFG_TGT_DEFAULT_GTX_HT_MASK;
	intr[cmd->vdev_id].config.gtx_info.gtxRTMask[1] =
		CFG_TGT_DEFAULT_GTX_VHT_MASK;
	intr[cmd->vdev_id].config.gtx_info.gtxUsrcfg =
		CFG_TGT_DEFAULT_GTX_USR_CFG;
	intr[cmd->vdev_id].config.gtx_info.gtxPERThreshold =
		CFG_TGT_DEFAULT_GTX_PER_THRESHOLD;
	intr[cmd->vdev_id].config.gtx_info.gtxPERMargin =
		CFG_TGT_DEFAULT_GTX_PER_MARGIN;
	intr[cmd->vdev_id].config.gtx_info.gtxTPCstep =
		CFG_TGT_DEFAULT_GTX_TPC_STEP;
	intr[cmd->vdev_id].config.gtx_info.gtxTPCMin =
		CFG_TGT_DEFAULT_GTX_TPC_MIN;
	intr[cmd->vdev_id].config.gtx_info.gtxBWMask =
		CFG_TGT_DEFAULT_GTX_BW_MASK;
	intr[cmd->vdev_id].mhz = chan->mhz;

	WMI_SET_CHANNEL_MODE(chan, chanmode);
	chan->band_center_freq1 = chan->mhz;

	if (CH_WIDTH_20MHZ != req->chan_width)
		chan->band_center_freq1 =
			cds_chan_to_freq(req->ch_center_freq_seg0);
	if (CH_WIDTH_80P80MHZ == req->chan_width)
		chan->band_center_freq2 =
			cds_chan_to_freq(req->ch_center_freq_seg1);
	else
		chan->band_center_freq2 = 0;

	/* Set half or quarter rate WMI flags */
	if (req->is_half_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_HALF_RATE);
	else if (req->is_quarter_rate)
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_QUARTER_RATE);

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

	/*
	 * If the Channel is DFS,
	 * set the WMI_CHAN_FLAG_DFS flag
	 */
	if (req->is_dfs) {
		WMI_SET_CHANNEL_FLAG(chan, WMI_CHAN_FLAG_DFS);
		cmd->disable_hw_ack = true;

		req->dfs_pri_multiplier = wma->dfs_pri_multiplier;

		/*
		 * Configure the current operating channel
		 * to DFS module only if the device operating
		 * mode is AP.
		 * Enable/Disable Phyerr filtering offload
		 * depending on dfs_phyerr_filter_offload
		 * flag status as set in ini for SAP mode.
		 * Currently, only AP supports DFS master
		 * mode operation on DFS channels, P2P-GO
		 * does not support operation on DFS Channels.
		 */
		if (wma_is_vdev_in_ap_mode(wma, cmd->vdev_id) == true) {
			/*
			 * If DFS regulatory domain is invalid,
			 * then, DFS radar filters intialization
			 * will fail. So, do not configure the
			 * channel in to DFS modlue, do not
			 * indicate if phyerror filtering offload
			 * is enabled or not to the firmware, simply
			 * fail the VDEV start on the DFS channel
			 * early on, to protect the DFS module from
			 * processing phyerrors without being intialized.
			 */
			if (DFS_UNINIT_DOMAIN ==
			    wma->dfs_ic->current_dfs_regdomain) {
				WMA_LOGE("%s[%d]:DFS Configured with Invalid regdomain"
					" Failed to send VDEV START command",
					__func__, __LINE__);

				cdf_nbuf_free(buf);
				return CDF_STATUS_E_FAILURE;
			}

			cdf_spin_lock_bh(&wma->dfs_ic->chan_lock);
			if (isRestart)
				wma->dfs_ic->disable_phy_err_processing = true;

			/* provide the current channel to DFS */
			wma->dfs_ic->ic_curchan =
				wma_dfs_configure_channel(wma->dfs_ic, chan,
							  chanmode, req);
			cdf_spin_unlock_bh(&wma->dfs_ic->chan_lock);

			wma_unified_dfs_phyerr_filter_offload_enable(wma);
			dfs->disable_dfs_ch_switch =
				mac_ctx->sap.SapDfsInfo.disable_dfs_ch_switch;
		}
	}

	cmd->beacon_interval = req->beacon_intval;
	cmd->dtim_period = req->dtim_period;
	/* FIXME: Find out min, max and regulatory power levels */
	WMI_SET_CHANNEL_REG_POWER(chan, req->max_txpow);
	WMI_SET_CHANNEL_MAX_TX_POWER(chan, req->max_txpow);

	/* TODO: Handle regulatory class, max antenna */
	if (!isRestart) {
		cmd->beacon_interval = req->beacon_intval;
		cmd->dtim_period = req->dtim_period;

		/* Copy the SSID */
		if (req->ssid.length) {
			if (req->ssid.length < sizeof(cmd->ssid.ssid))
				cmd->ssid.ssid_len = req->ssid.length;
			else
				cmd->ssid.ssid_len = sizeof(cmd->ssid.ssid);
			cdf_mem_copy(cmd->ssid.ssid, req->ssid.ssId,
				     cmd->ssid.ssid_len);
		}

		if (req->hidden_ssid)
			cmd->flags |= WMI_UNIFIED_VDEV_START_HIDDEN_SSID;

		if (req->pmf_enabled)
			cmd->flags |= WMI_UNIFIED_VDEV_START_PMF_ENABLED;
	}

	cmd->num_noa_descriptors = 0;
	cmd->preferred_rx_streams = req->preferred_rx_streams;
	cmd->preferred_tx_streams = req->preferred_tx_streams;

	buf_ptr = (uint8_t *) (((uintptr_t) cmd) + sizeof(*cmd) +
			       sizeof(wmi_channel));
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       cmd->num_noa_descriptors *
		       sizeof(wmi_p2p_noa_descriptor));
	WMA_LOGA("\n%s: vdev_id %d freq %d channel %d chanmode %d is_dfs %d "
		"beacon interval %d dtim %d center_chan %d center_freq2 %d "
		"reg_info_1: 0x%x reg_info_2: 0x%x, req->max_txpow: 0x%x "
		"Tx SS %d, Rx SS %d",
		__func__, req->vdev_id, chan->mhz, req->chan, chanmode,
		req->is_dfs, req->beacon_intval, cmd->dtim_period,
		chan->band_center_freq1, chan->band_center_freq2,
		chan->reg_info_1, chan->reg_info_2, req->max_txpow,
		req->preferred_tx_streams, req->preferred_rx_streams);

	/* Store vdev params in SAP mode which can be used in vdev restart */
	if (intr[req->vdev_id].type == WMI_VDEV_TYPE_AP &&
	    intr[req->vdev_id].sub_type == 0) {
		intr[req->vdev_id].vdev_restart_params.vdev_id = req->vdev_id;
		intr[req->vdev_id].vdev_restart_params.ssid.ssid_len =
			cmd->ssid.ssid_len;
		cdf_mem_copy(intr[req->vdev_id].vdev_restart_params.ssid.ssid,
			     cmd->ssid.ssid, cmd->ssid.ssid_len);
		intr[req->vdev_id].vdev_restart_params.flags = cmd->flags;
		intr[req->vdev_id].vdev_restart_params.requestor_id =
			cmd->requestor_id;
		intr[req->vdev_id].vdev_restart_params.disable_hw_ack =
			cmd->disable_hw_ack;
		intr[req->vdev_id].vdev_restart_params.chan.mhz = chan->mhz;
		intr[req->vdev_id].vdev_restart_params.chan.band_center_freq1 =
			chan->band_center_freq1;
		intr[req->vdev_id].vdev_restart_params.chan.band_center_freq2 =
			chan->band_center_freq2;
		intr[req->vdev_id].vdev_restart_params.chan.info = chan->info;
		intr[req->vdev_id].vdev_restart_params.chan.reg_info_1 =
			chan->reg_info_1;
		intr[req->vdev_id].vdev_restart_params.chan.reg_info_2 =
			chan->reg_info_2;
	}

	if (isRestart) {
		/*
		 * Marking the VDEV UP STATUS to false
		 * since, VDEV RESTART will do a VDEV DOWN
		 * in the firmware.
		 */
		intr[cmd->vdev_id].vdev_up = false;

		ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
					   WMI_VDEV_RESTART_REQUEST_CMDID);

	} else {
		WMA_LOGD("%s, vdev_id: %d, unpausing tx_ll_queue at VDEV_START",
			 __func__, cmd->vdev_id);
		ol_txrx_vdev_unpause(wma->interfaces[cmd->vdev_id].handle,
				     0xffffffff);
		wma->interfaces[cmd->vdev_id].pause_bitmap = 0;
		ret = wmi_unified_cmd_send(wma->wmi_handle, buf, len,
					   WMI_VDEV_START_REQUEST_CMDID);
	}

	if (ret < 0) {
		WMA_LOGP("%s: Failed to send vdev start command", __func__);
		cdf_nbuf_free(buf);
		return CDF_STATUS_E_FAILURE;
	}

	return CDF_STATUS_SUCCESS;
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

	cdf_mc_timer_stop(&req_msg->event_timeout);

	if (req_msg->msg_type == WMA_ADD_STA_REQ) {
		tpAddStaParams params = (tpAddStaParams)req_msg->user_data;
		if (!params) {
			WMA_LOGE(FL("add STA params is NULL for vdev %d"),
				 event->vdev_id);
			status = -EINVAL;
			goto free_req_msg;
		}

		/* peer assoc conf event means the cmd succeeds */
		params->status = CDF_STATUS_SUCCESS;
		WMA_LOGE(FL("Send ADD_STA_RSP: statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
			 params->staType, params->smesessionId,
			 params->assocId, params->bssId, params->staIdx,
			 params->status);
		wma_send_msg(wma, WMA_ADD_STA_RSP, (void *)params, 0);
	} else if (req_msg->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams  params = (tpAddBssParams) req_msg->user_data;
		if (!params) {
			WMA_LOGE(FL("add BSS params is NULL for vdev %d"),
				 event->vdev_id);
			status = -EINVAL;
			goto free_req_msg;
		}

		/* peer assoc conf event means the cmd succeeds */
		params->status = CDF_STATUS_SUCCESS;
		WMA_LOGE(FL("Send ADD BSS RSP: opermode %d update_bss %d nw_type %d bssid %pM"
			" staIdx %d status %d"), params->operMode,
			params->updateBss, params->nwType, params->bssId,
			params->staContext.staIdx, params->status);
		wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)params, 0);
	} else {
		WMA_LOGE(FL("Unhandled request message type: %d"),
		req_msg->msg_type);
	}

free_req_msg:
	cdf_mc_timer_destroy(&req_msg->event_timeout);
	cdf_mem_free(req_msg);

	return status;
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

	wma = cds_get_context(CDF_MODULE_ID_WMA);
	if (NULL == wma) {
		WMA_LOGE(FL("Failed to get wma"));
		goto free_tgt_req;
	}

	WMA_LOGA(FL("request %d is timed out for vdev_id - %d"),
		 tgt_req->msg_type, tgt_req->vdev_id);
	msg = wma_find_req(wma, tgt_req->vdev_id, tgt_req->type);

	if (!msg) {
		WMA_LOGE(FL("Failed to lookup request message - %d"),
			 tgt_req->msg_type);
		goto free_tgt_req;
	}

	if (tgt_req->msg_type == WMA_ADD_STA_REQ) {
		tpAddStaParams params = (tpAddStaParams) tgt_req->user_data;
		params->status = CDF_STATUS_E_TIMEOUT;
		WMA_LOGA(FL("WMA_ADD_STA_REQ timed out"));
		WMA_LOGD(FL("Sending add sta rsp to umac (mac:%pM, status:%d)"),
			 params->staMac, params->status);
		wma_send_msg(wma, WMA_ADD_STA_RSP, (void *)params, 0);
	}
free_tgt_req:
	cdf_mc_timer_destroy(&tgt_req->event_timeout);
	cdf_mem_free(tgt_req);
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
	CDF_STATUS status;

	req = cdf_mem_malloc(sizeof(*req));
	if (!req) {
		WMA_LOGP(FL("Failed to allocate memory for msg %d vdev %d"),
			 msg_type, vdev_id);
		return NULL;
	}

	WMA_LOGE(FL("vdev_id %d msg %d type %d"), vdev_id, msg_type, type);
	req->vdev_id = vdev_id;
	req->msg_type = msg_type;
	req->type = type;
	req->user_data = params;
	cdf_mc_timer_init(&req->event_timeout, CDF_TIMER_TYPE_SW,
			  wma_hold_req_timer, req);
	cdf_mc_timer_start(&req->event_timeout, timeout);
	cdf_spin_lock_bh(&wma->wma_hold_req_q_lock);
	status = cdf_list_insert_back(&wma->wma_hold_req_queue, &req->node);
	if (CDF_STATUS_SUCCESS != status) {
		cdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
		WMA_LOGE(FL("Failed add request in queue"));
		cdf_mem_free(req);
		return NULL;
	}
	cdf_spin_unlock_bh(&wma->wma_hold_req_q_lock);
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

	WMA_LOGE(FL("Remove req for vdev: %d type: %d"), vdev_id, type);
	req_msg = wma_find_req(wma, vdev_id, type);
	if (!req_msg) {
		WMA_LOGE(FL("target req not found for vdev: %d type: %d"),
			 vdev_id, type);
		return;
	}

	cdf_mc_timer_stop(&req_msg->event_timeout);
	cdf_mc_timer_destroy(&req_msg->event_timeout);
	cdf_mem_free(req_msg);
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
	ol_txrx_peer_handle peer;
	ol_txrx_pdev_handle pdev;
	uint8_t peer_id;
	struct wma_target_req *msg;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	cds_msg_t sme_msg = { 0 };
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
	tpAniSirGlobal mac_ctx = cds_get_context(CDF_MODULE_ID_PE);
	if (NULL == mac_ctx) {
		WMA_LOGE("%s: Failed to get mac_ctx", __func__);
		goto free_tgt_req;
	}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

	wma = cds_get_context(CDF_MODULE_ID_WMA);

	if (NULL == wma) {
		WMA_LOGE("%s: Failed to get wma", __func__);
		goto free_tgt_req;
	}

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		cdf_mc_timer_stop(&tgt_req->event_timeout);
		goto free_tgt_req;
	}

	WMA_LOGA("%s: request %d is timed out for vdev_id - %d", __func__,
		 tgt_req->msg_type, tgt_req->vdev_id);
	msg = wma_find_vdev_req(wma, tgt_req->vdev_id, tgt_req->type);

	if (!msg) {
		WMA_LOGE("%s: Failed to lookup request message - %d",
			 __func__, tgt_req->msg_type);
		goto free_tgt_req;
	}

	if (tgt_req->msg_type == WMA_CHNL_SWITCH_REQ) {
		tpSwitchChannelParams params =
			(tpSwitchChannelParams) tgt_req->user_data;
		params->status = CDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_SWITCH_CHANNEL_REQ timedout", __func__);
		wma_send_msg(wma, WMA_SWITCH_CHANNEL_RSP, (void *)params, 0);
		wma->roam_preauth_chan_context = NULL;
		wma->roam_preauth_scan_id = -1;
	} else if (tgt_req->msg_type == WMA_DELETE_BSS_REQ) {
		tpDeleteBssParams params =
			(tpDeleteBssParams) tgt_req->user_data;
		struct beacon_info *bcn;
		struct wma_txrx_node *iface;

		if (tgt_req->vdev_id > wma->max_bssid) {
			WMA_LOGE("%s: Invalid vdev_id %d", __func__,
				 tgt_req->vdev_id);
			cdf_mc_timer_stop(&tgt_req->event_timeout);
			goto free_tgt_req;
		}

		iface = &wma->interfaces[tgt_req->vdev_id];
		if (iface->handle == NULL) {
			WMA_LOGE("%s vdev id %d is already deleted",
				 __func__, tgt_req->vdev_id);
			cdf_mc_timer_stop(&tgt_req->event_timeout);
			goto free_tgt_req;
		}
#ifdef QCA_IBSS_SUPPORT
		if (wma_is_vdev_in_ibss_mode(wma, tgt_req->vdev_id))
			wma_delete_all_ibss_peers(wma, tgt_req->vdev_id);
		else
#endif /* QCA_IBSS_SUPPORT */
		{
			if (wma_is_vdev_in_ap_mode(wma, tgt_req->vdev_id)) {
				wma_delete_all_ap_remote_peers(wma,
							       tgt_req->
							       vdev_id);
			}
			peer = ol_txrx_find_peer_by_addr(pdev, params->bssid,
							 &peer_id);
			wma_remove_peer(wma, params->bssid, tgt_req->vdev_id,
					peer, false);
		}

		if (wmi_unified_vdev_down_send(wma->wmi_handle,
					       tgt_req->vdev_id) < 0) {
			WMA_LOGE("Failed to send vdev down cmd: vdev %d",
				 tgt_req->vdev_id);
		} else {
			wma->interfaces[tgt_req->vdev_id].vdev_up = false;
#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		if (mac_ctx->sap.sap_channel_avoidance)
			wma_find_mcc_ap(wma, tgt_req->vdev_id, false);
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */
		}
		ol_txrx_vdev_flush(iface->handle);
		WMA_LOGD("%s, vdev_id: %d, un-pausing tx_ll_queue for WDA_DELETE_BSS_REQ timeout",
			 __func__, tgt_req->vdev_id);
		ol_txrx_vdev_unpause(iface->handle,
				     OL_TXQ_PAUSE_REASON_VDEV_STOP);
		iface->pause_bitmap &= ~(1 << PAUSE_TYPE_HOST);
		cdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STOPPED);
		WMA_LOGD("%s: (type %d subtype %d) BSS is stopped",
			 __func__, iface->type, iface->sub_type);

		bcn = wma->interfaces[tgt_req->vdev_id].beacon;

		if (bcn) {
			WMA_LOGD("%s: Freeing beacon struct %p, "
				 "template memory %p", __func__, bcn, bcn->buf);
			if (bcn->dma_mapped)
				cdf_nbuf_unmap_single(pdev->osdev, bcn->buf,
						      CDF_DMA_TO_DEVICE);
			cdf_nbuf_free(bcn->buf);
			cdf_mem_free(bcn);
			wma->interfaces[tgt_req->vdev_id].beacon = NULL;
		}
#ifdef QCA_IBSS_SUPPORT
		/* recreate ibss vdev and bss peer for scan purpose */
		if (wma_is_vdev_in_ibss_mode(wma, tgt_req->vdev_id))
			wma_recreate_ibss_vdev_and_bss_peer(wma,
							    tgt_req->vdev_id);
#endif /* QCA_IBSS_SUPPORT */
		params->status = CDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_DELETE_BSS_REQ timedout", __func__);
		wma_send_msg(wma, WMA_DELETE_BSS_RSP, (void *)params, 0);
		if (iface->del_staself_req) {
			WMA_LOGA("scheduling defered deletion(vdev id %x)",
				 tgt_req->vdev_id);
			wma_vdev_detach(wma, iface->del_staself_req, 1);
		}
	} else if (tgt_req->msg_type == WMA_DEL_STA_SELF_REQ) {
		struct wma_txrx_node *iface =
			(struct wma_txrx_node *)tgt_req->user_data;
		struct del_sta_self_params *params =
			(struct del_sta_self_params *) iface->del_staself_req;

		params->status = CDF_STATUS_E_TIMEOUT;
		WMA_LOGA("%s: WMA_DEL_STA_SELF_REQ timedout", __func__);
		sme_msg.type = eWNI_SME_DEL_STA_SELF_RSP;
		sme_msg.bodyptr = iface->del_staself_req;
		sme_msg.bodyval = 0;

		status = cds_mq_post_message(CDF_MODULE_ID_SME, &sme_msg);
		if (!CDF_IS_STATUS_SUCCESS(status)) {
			WMA_LOGE("Failed to post eWNI_SME_ADD_STA_SELF_RSP");
			cdf_mem_free(iface->del_staself_req);
		}
		if (iface->addBssStaContext)
			cdf_mem_free(iface->addBssStaContext);
#if defined WLAN_FEATURE_VOWIFI_11R
		if (iface->staKeyParams)
			cdf_mem_free(iface->staKeyParams);
#endif /* WLAN_FEATURE_VOWIFI_11R */
		cdf_mem_zero(iface, sizeof(*iface));
	} else if (tgt_req->msg_type == WMA_ADD_BSS_REQ) {
		tpAddBssParams params = (tpAddBssParams) tgt_req->user_data;
		tDeleteBssParams *del_bss_params =
			cdf_mem_malloc(sizeof(tDeleteBssParams));
		if (NULL == del_bss_params) {
			WMA_LOGE("Failed to allocate memory for del_bss_params");
			peer = ol_txrx_find_peer_by_addr(pdev, params->bssId,
							  &peer_id);
			goto error0;
		}

		del_bss_params->status = params->status =
						 CDF_STATUS_FW_MSG_TIMEDOUT;
		del_bss_params->sessionId = params->sessionId;
		del_bss_params->bssIdx = params->bssIdx;
		cdf_mem_copy(del_bss_params->bssid, params->bssId,
			     sizeof(tSirMacAddr));

		WMA_LOGA("%s: WMA_ADD_BSS_REQ timedout", __func__);
		peer = ol_txrx_find_peer_by_addr(pdev, params->bssId, &peer_id);
		if (!peer) {
			WMA_LOGP("%s: Failed to find peer %pM", __func__,
				 params->bssId);
		}
		msg = wma_fill_vdev_req(wma, tgt_req->vdev_id, WMA_DELETE_BSS_REQ,
					  WMA_TARGET_REQ_TYPE_VDEV_STOP,
					  del_bss_params,
					  WMA_VDEV_STOP_REQUEST_TIMEOUT);
		if (!msg) {
			WMA_LOGP("%s: Failed to fill vdev request for vdev_id %d",
				__func__, tgt_req->vdev_id);
			goto error0;
		}
		WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP (WDA_ADD_BSS_REQ timedout)",
			 __func__, tgt_req->vdev_id);
		ol_txrx_vdev_pause(wma->interfaces[tgt_req->vdev_id].handle,
				   OL_TXQ_PAUSE_REASON_VDEV_STOP);
		wma->interfaces[tgt_req->vdev_id].pause_bitmap |=
							(1 << PAUSE_TYPE_HOST);
		if (wmi_unified_vdev_stop_send
			    (wma->wmi_handle, tgt_req->vdev_id)) {
			WMA_LOGP("%s: %d Failed to send vdev stop", __func__,
				 __LINE__);
			wma_remove_vdev_req(wma, tgt_req->vdev_id,
					    WMA_TARGET_REQ_TYPE_VDEV_STOP);
			goto error0;
		}
		WMA_LOGI("%s: bssid %pM vdev_id %d", __func__, params->bssId,
			 tgt_req->vdev_id);
		wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)params, 0);
		goto free_tgt_req;
error0:
		if (peer)
			wma_remove_peer(wma, params->bssId,
					tgt_req->vdev_id, peer, false);
		wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)params, 0);
	} else if (tgt_req->msg_type == WMA_OCB_SET_CONFIG_CMD) {
		struct wma_txrx_node *iface;

		WMA_LOGE(FL("Failed to send OCB set config cmd"));
		iface = &wma->interfaces[tgt_req->vdev_id];
		iface->vdev_up = false;
		wma_ocb_set_config_resp(wma, CDF_STATUS_E_TIMEOUT);
	}
free_tgt_req:
	cdf_mc_timer_destroy(&tgt_req->event_timeout);
	cdf_mem_free(tgt_req);
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
	CDF_STATUS status;

	req = cdf_mem_malloc(sizeof(*req));
	if (!req) {
		WMA_LOGP("%s: Failed to allocate memory for msg %d vdev %d",
			 __func__, msg_type, vdev_id);
		return NULL;
	}

	WMA_LOGD("%s: vdev_id %d msg %d", __func__, vdev_id, msg_type);
	req->vdev_id = vdev_id;
	req->msg_type = msg_type;
	req->type = type;
	req->user_data = params;
	cdf_mc_timer_init(&req->event_timeout, CDF_TIMER_TYPE_SW,
			  wma_vdev_resp_timer, req);
	cdf_mc_timer_start(&req->event_timeout, timeout);
	cdf_spin_lock_bh(&wma->vdev_respq_lock);
	status = cdf_list_insert_back(&wma->vdev_resp_queue, &req->node);
	if (CDF_STATUS_SUCCESS != status) {
		cdf_spin_unlock_bh(&wma->vdev_respq_lock);
		WMA_LOGE(FL("Failed add request in queue for vdev_id %d type %d"),
			 vdev_id, type);
		cdf_mem_free(req);
		return NULL;
	}

	cdf_spin_unlock_bh(&wma->vdev_respq_lock);
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

	req_msg = wma_find_vdev_req(wma, vdev_id, type);
	if (!req_msg)
		return;

	cdf_mc_timer_stop(&req_msg->event_timeout);
	cdf_mc_timer_destroy(&req_msg->event_timeout);
	cdf_mem_free(req_msg);
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
			uint8_t llbCoexist, tPowerdBm maxTxPower)
{
	int ret;
	uint32_t slot_time;
	struct wma_txrx_node *intr = wma->interfaces;

	/* Beacon Interval setting */
	ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_BEACON_INTERVAL,
					      beaconInterval);

	if (ret)
		WMA_LOGE("failed to set WMI_VDEV_PARAM_BEACON_INTERVAL");

	ret = wmi_unified_vdev_set_gtx_cfg_send(wma->wmi_handle, vdev_id,
						&intr[vdev_id].config.gtx_info);
	if (ret)
		WMA_LOGE("failed to set WMI_VDEV_PARAM_DTIM_PERIOD");

	ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_DTIM_PERIOD,
					      dtimPeriod);
	if (ret)
		WMA_LOGE("failed to set WMI_VDEV_PARAM_DTIM_PERIOD");

	if (!maxTxPower) {
		WMA_LOGW("Setting Tx power limit to 0");
	}

	ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_TX_PWRLIMIT,
					      maxTxPower);
	if (ret)
		WMA_LOGE("failed to set WMI_VDEV_PARAM_TX_PWRLIMIT");
	else
		intr[vdev_id].max_tx_power = maxTxPower;

	/* Slot time */
	if (shortSlotTimeSupported)
		slot_time = WMI_VDEV_SLOT_TIME_SHORT;
	else
		slot_time = WMI_VDEV_SLOT_TIME_LONG;

	ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					      WMI_VDEV_PARAM_SLOT_TIME,
					      slot_time);
	if (ret)
		WMA_LOGE("failed to set WMI_VDEV_PARAM_SLOT_TIME");

	/* Initialize protection mode in case of coexistence */
	wma_update_protection_mode(wma, vdev_id, llbCoexist);
}

/**
 * wma_add_bss_ap_mode() - process add bss request in ap mode
 * @wma: wma handle
 * @add_bss: add bss parameters
 *
 * Return: none
 */
static void wma_add_bss_ap_mode(tp_wma_handle wma, tpAddBssParams add_bss)
{
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	struct wma_vdev_start_req req;
	ol_txrx_peer_handle peer;
	struct wma_target_req *msg;
	uint8_t vdev_id, peer_id;
	CDF_STATUS status;
	tPowerdBm maxTxPower;
#ifdef WLAN_FEATURE_11W
	int ret = 0;
#endif /* WLAN_FEATURE_11W */
	struct sir_hw_mode_params hw_mode = {0};

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto send_fail_resp;
	}

	vdev = wma_find_vdev_by_addr(wma, add_bss->bssId, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: Failed to get vdev handle", __func__);
		goto send_fail_resp;
	}
	if (SAP_WPS_DISABLED == add_bss->wps_state)
		wma_enable_disable_wakeup_event(wma, vdev_id,
			(1 << WOW_PROBE_REQ_WPS_IE_EVENT), false);
	wma_set_bss_rate_flags(&wma->interfaces[vdev_id], add_bss);
	status = wma_create_peer(wma, pdev, vdev, add_bss->bssId,
				 WMI_PEER_TYPE_DEFAULT, vdev_id, false);
	if (status != CDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to create peer", __func__);
		goto send_fail_resp;
	}

	peer = ol_txrx_find_peer_by_addr(pdev, add_bss->bssId, &peer_id);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 add_bss->bssId);
		goto send_fail_resp;
	}
	msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_START, add_bss,
				WMA_VDEV_START_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGP("%s Failed to allocate vdev request vdev_id %d",
			 __func__, vdev_id);
		goto peer_cleanup;
	}

	add_bss->staContext.staIdx = ol_txrx_local_peer_id(peer);

	cdf_mem_zero(&req, sizeof(req));
	req.vdev_id = vdev_id;
	req.chan = add_bss->currentOperChannel;
	req.chan_width = add_bss->ch_width;
	req.ch_center_freq_seg0 = add_bss->ch_center_freq_seg0;
	req.ch_center_freq_seg1 = add_bss->ch_center_freq_seg1;
	req.vht_capable = add_bss->vhtCapable;
#if defined WLAN_FEATURE_VOWIFI
	req.max_txpow = add_bss->maxTxPower;
	maxTxPower = add_bss->maxTxPower;
#else
	req.max_txpow = 0;
	maxTxPower = 0;
#endif /* WLAN_FEATURE_VOWIFI */
#ifdef WLAN_FEATURE_11W
	if (add_bss->rmfEnabled) {
		/*
		 * when 802.11w PMF is enabled for hw encr/decr
		 * use hw MFP Qos bits 0x10
		 */
		ret = wmi_unified_pdev_set_param(wma->wmi_handle,
						 WMI_PDEV_PARAM_PMF_QOS, true);
		if (ret) {
			WMA_LOGE("%s: Failed to set QOS MFP/PMF (%d)",
				 __func__, ret);
		} else {
			WMA_LOGI("%s: QOS MFP/PMF set to %d", __func__, true);
		}
	}
#endif /* WLAN_FEATURE_11W */

	req.beacon_intval = add_bss->beaconInterval;
	req.dtim_period = add_bss->dtimPeriod;
	req.hidden_ssid = add_bss->bHiddenSSIDEn;
	req.is_dfs = add_bss->bSpectrumMgtEnabled;
	req.oper_mode = BSS_OPERATIONAL_MODE_AP;
	req.ssid.length = add_bss->ssId.length;
	if (req.ssid.length > 0)
		cdf_mem_copy(req.ssid.ssId, add_bss->ssId.ssId,
			     add_bss->ssId.length);
	status = wma_get_current_hw_mode(&hw_mode);
	if (!CDF_IS_STATUS_SUCCESS(status))
		WMA_LOGE("wma_get_current_hw_mode failed");

	if ((add_bss->nss == 2) && !hw_mode.dbs_cap) {
		req.preferred_rx_streams = 2;
		req.preferred_tx_streams = 2;
	} else {
		req.preferred_rx_streams = 1;
		req.preferred_tx_streams = 1;
	}

	status = wma_vdev_start(wma, &req, false);
	if (status != CDF_STATUS_SUCCESS) {
		wma_remove_vdev_req(wma, vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);
		goto peer_cleanup;
	}

	wma_vdev_set_bss_params(wma, vdev_id,
				add_bss->beaconInterval, add_bss->dtimPeriod,
				add_bss->shortSlotTimeSupported,
				add_bss->llbCoexist, maxTxPower);

	return;

peer_cleanup:
	wma_remove_peer(wma, add_bss->bssId, vdev_id, peer, false);
send_fail_resp:
	add_bss->status = CDF_STATUS_E_FAILURE;
	wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
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
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	struct wma_vdev_start_req req;
	ol_txrx_peer_handle peer = NULL;
	struct wma_target_req *msg;
	uint8_t vdev_id, peer_id;
	CDF_STATUS status;
	struct add_sta_self_params add_sta_self_param;
	struct del_sta_self_params del_sta_param;
	tSetBssKeyParams key_info;
	struct sir_hw_mode_params hw_mode = {0};

	WMA_LOGD("%s: add_bss->sessionId = %d", __func__, add_bss->sessionId);
	vdev_id = add_bss->sessionId;
	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto send_fail_resp;
	}
	wma_set_bss_rate_flags(&wma->interfaces[vdev_id], add_bss);

	vdev = wma_find_vdev_by_id(wma, vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: vdev not found for vdev id %d.",
			 __func__, vdev_id);
		goto send_fail_resp;
	}

	/* only change vdev type to ibss during 1st time join_ibss handling */

	if (false == wma_is_vdev_in_ibss_mode(wma, vdev_id)) {

		WMA_LOGD("%s: vdev found for vdev id %d. deleting the vdev",
			 __func__, vdev_id);

		/* remove peers on the existing non-ibss vdev */
		TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
			WMA_LOGE("%s: peer found for vdev id %d. deleting the peer",
				__func__, vdev_id);
			wma_remove_peer(wma, (uint8_t *) &vdev->mac_addr,
					vdev_id, peer, false);
		}

		/* remove the non-ibss vdev */
		cdf_copy_macaddr(
			(struct cdf_mac_addr *) &(del_sta_param.self_mac_addr),
			(struct cdf_mac_addr *) &(vdev->mac_addr));
		del_sta_param.session_id = vdev_id;
		del_sta_param.status = 0;

		wma_vdev_detach(wma, &del_sta_param, 0);

		/* create new vdev for ibss */
		cdf_copy_macaddr((struct cdf_mac_addr *) &
			 (add_sta_self_param.self_mac_addr),
			 (struct cdf_mac_addr *) &(add_bss->selfMacAddr));
		add_sta_self_param.session_id = vdev_id;
		add_sta_self_param.type = WMI_VDEV_TYPE_IBSS;
		add_sta_self_param.sub_type = 0;
		add_sta_self_param.status = 0;

		vdev = wma_vdev_attach(wma, &add_sta_self_param, 0);
		if (!vdev) {
			WMA_LOGE("%s: Failed to create vdev", __func__);
			goto send_fail_resp;
		}

		/* Register with TxRx Module for Data Ack Complete Cb */
		ol_txrx_data_tx_cb_set(vdev, wma_data_tx_ack_comp_hdlr, wma);
		WMA_LOGA("new IBSS vdev created with mac %pM",
			 add_bss->selfMacAddr);

		/* create ibss bss peer */
		status = wma_create_peer(wma, pdev, vdev, add_bss->selfMacAddr,
					 WMI_PEER_TYPE_DEFAULT, vdev_id,
					 false);
		if (status != CDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to create peer", __func__);
			goto send_fail_resp;
		}
		WMA_LOGA("IBSS BSS peer created with mac %pM",
			 add_bss->selfMacAddr);
	}

	peer = ol_txrx_find_peer_by_addr(pdev, add_bss->selfMacAddr, &peer_id);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 add_bss->selfMacAddr);
		goto send_fail_resp;
	}

	/* clear leftover ibss keys on bss peer */

	WMA_LOGD("%s: ibss bss key clearing", __func__);
	cdf_mem_set(&key_info, sizeof(key_info), 0);
	key_info.smesessionId = vdev_id;
	key_info.numKeys = SIR_MAC_MAX_NUM_OF_DEFAULT_KEYS;
	cdf_mem_copy(&wma->ibsskey_info, &key_info, sizeof(tSetBssKeyParams));

	/* start ibss vdev */

	add_bss->operMode = BSS_OPERATIONAL_MODE_IBSS;

	msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_START, add_bss,
				WMA_VDEV_START_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGP("%s Failed to allocate vdev request vdev_id %d",
			 __func__, vdev_id);
		goto peer_cleanup;
	}
	WMA_LOGD("%s: vdev start request for IBSS enqueued", __func__);

	add_bss->staContext.staIdx = ol_txrx_local_peer_id(peer);

	/*
	 * If IBSS Power Save is supported by firmware
	 * set the IBSS power save params to firmware.
	 */
	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				   WMI_SERVICE_IBSS_PWRSAVE)) {
		status = wma_set_ibss_pwrsave_params(wma, vdev_id);
		if (status != CDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to Set IBSS Power Save Params to firmware",
				__func__);
			goto peer_cleanup;
		}
	}

	cdf_mem_zero(&req, sizeof(req));
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
		cdf_mem_copy(req.ssid.ssId, add_bss->ssId.ssId,
			     add_bss->ssId.length);
	status = wma_get_current_hw_mode(&hw_mode);
	if (!CDF_IS_STATUS_SUCCESS(status))
		WMA_LOGE("wma_get_current_hw_mode failed");

	if ((add_bss->nss == 2) && !hw_mode.dbs_cap) {
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
	if (status != CDF_STATUS_SUCCESS) {
		wma_remove_vdev_req(wma, vdev_id,
				    WMA_TARGET_REQ_TYPE_VDEV_START);
		goto peer_cleanup;
	}
	WMA_LOGD("%s: vdev start request for IBSS sent to target", __func__);

	/* Initialize protection mode to no protection */
	if (wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
					    WMI_VDEV_PARAM_PROTECTION_MODE,
					    IEEE80211_PROT_NONE)) {
		WMA_LOGE("Failed to initialize protection mode");
	}

	return;

peer_cleanup:
	if (peer) {
		wma_remove_peer(wma, add_bss->bssId, vdev_id, peer, false);
	}
send_fail_resp:
	add_bss->status = CDF_STATUS_E_FAILURE;
	wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
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
	ol_txrx_pdev_handle pdev;
	struct wma_vdev_start_req req;
	struct wma_target_req *msg;
	uint8_t vdev_id, peer_id;
	ol_txrx_peer_handle peer;
	CDF_STATUS status;
	struct wma_txrx_node *iface;
	int ret = 0;
	int pps_val = 0;
	bool roam_synch_in_progress = false;
	tpAniSirGlobal pMac = cds_get_context(CDF_MODULE_ID_PE);
	struct sir_hw_mode_params hw_mode = {0};
	bool peer_assoc_sent = false;

	if (NULL == pMac) {
		WMA_LOGE("%s: Unable to get PE context", __func__);
		goto send_fail_resp;
	}

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s Failed to get pdev", __func__);
		goto send_fail_resp;
	}

	vdev_id = add_bss->staContext.smesessionId;
	iface = &wma->interfaces[vdev_id];

	wma_set_bss_rate_flags(iface, add_bss);
	if (add_bss->operMode) {
		/* Save parameters later needed by WMA_ADD_STA_REQ */
		if (iface->addBssStaContext) {
			cdf_mem_free(iface->addBssStaContext);
		}
		iface->addBssStaContext = cdf_mem_malloc(sizeof(tAddStaParams));
		if (!iface->addBssStaContext) {
			WMA_LOGE("%s Failed to allocat memory", __func__);
			goto send_fail_resp;
		}
		cdf_mem_copy(iface->addBssStaContext, &add_bss->staContext,
			     sizeof(tAddStaParams));

#if defined WLAN_FEATURE_VOWIFI_11R
		if (iface->staKeyParams) {
			cdf_mem_free(iface->staKeyParams);
			iface->staKeyParams = NULL;
		}
		if (add_bss->extSetStaKeyParamValid) {
			iface->staKeyParams =
				cdf_mem_malloc(sizeof(tSetStaKeyParams));
			if (!iface->staKeyParams) {
				WMA_LOGE("%s Failed to allocat memory",
					 __func__);
				goto send_fail_resp;
			}
			cdf_mem_copy(iface->staKeyParams,
				     &add_bss->extSetStaKeyParam,
				     sizeof(tSetStaKeyParams));
		}
#endif /* WLAN_FEATURE_VOWIFI_11R */
		/* Save parameters later needed by WMA_ADD_STA_REQ */
		iface->rmfEnabled = add_bss->rmfEnabled;
		iface->beaconInterval = add_bss->beaconInterval;
		iface->dtimPeriod = add_bss->dtimPeriod;
		iface->llbCoexist = add_bss->llbCoexist;
		iface->shortSlotTimeSupported = add_bss->shortSlotTimeSupported;
		iface->nwType = add_bss->nwType;
		if (add_bss->nonRoamReassoc) {
			peer = ol_txrx_find_peer_by_addr(pdev, add_bss->bssId,
							  &peer_id);
			if (peer) {
				add_bss->staContext.staIdx =
					ol_txrx_local_peer_id(peer);
				goto send_bss_resp;
			}
		}
		if (add_bss->reassocReq) {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			ol_txrx_vdev_handle vdev;
#endif
			/* Called in preassoc state. BSSID peer is already added by set_linkstate */
			peer = ol_txrx_find_peer_by_addr(pdev, add_bss->bssId,
							  &peer_id);
			if (!peer) {
				WMA_LOGE("%s Failed to find peer %pM", __func__,
					 add_bss->bssId);
				goto send_fail_resp;
			}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
			if (iface->roam_synch_in_progress) {
				add_bss->staContext.staIdx =
					ol_txrx_local_peer_id(peer);
				goto send_bss_resp;
			}
#endif
			msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
						WMA_TARGET_REQ_TYPE_VDEV_START,
						add_bss,
						WMA_VDEV_START_REQUEST_TIMEOUT);
			if (!msg) {
				WMA_LOGP("%s Failed to allocate vdev request vdev_id %d",
					__func__, vdev_id);
				goto peer_cleanup;
			}

			add_bss->staContext.staIdx =
				ol_txrx_local_peer_id(peer);

			cdf_mem_zero(&req, sizeof(req));
			req.vdev_id = vdev_id;
			req.chan = add_bss->currentOperChannel;
			req.chan_width = add_bss->ch_width;
			req.ch_center_freq_seg0 = add_bss->ch_center_freq_seg0;
			req.ch_center_freq_seg1 = add_bss->ch_center_freq_seg1;
#if defined WLAN_FEATURE_VOWIFI
			req.max_txpow = add_bss->maxTxPower;
#else
			req.max_txpow = 0;
#endif
			req.beacon_intval = add_bss->beaconInterval;
			req.dtim_period = add_bss->dtimPeriod;
			req.hidden_ssid = add_bss->bHiddenSSIDEn;
			req.is_dfs = add_bss->bSpectrumMgtEnabled;
			req.ssid.length = add_bss->ssId.length;
			req.oper_mode = BSS_OPERATIONAL_MODE_STA;
			if (req.ssid.length > 0)
				cdf_mem_copy(req.ssid.ssId, add_bss->ssId.ssId,
					     add_bss->ssId.length);
			status = wma_get_current_hw_mode(&hw_mode);
			if (!CDF_IS_STATUS_SUCCESS(status))
				WMA_LOGE("wma_get_current_hw_mode failed");

			if ((add_bss->nss == 2) && !hw_mode.dbs_cap) {
				req.preferred_rx_streams = 2;
				req.preferred_tx_streams = 2;
			} else {
				req.preferred_rx_streams = 1;
				req.preferred_tx_streams = 1;
			}

			status = wma_vdev_start(wma, &req, false);
			if (status != CDF_STATUS_SUCCESS) {
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
			ol_txrx_vdev_pause(vdev,
					   OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED);
#endif
			/* ADD_BSS_RESP will be deferred to completion of VDEV_START */

			return;
		}
		if (!add_bss->updateBss) {
			goto send_bss_resp;

		}
		/* Update peer state */
		if (add_bss->staContext.encryptType == eSIR_ED_NONE) {
			WMA_LOGD("%s: Update peer(%pM) state into auth",
				 __func__, add_bss->bssId);
			ol_txrx_peer_state_update(pdev, add_bss->bssId,
						  ol_txrx_peer_state_auth);
		} else {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			ol_txrx_vdev_handle vdev;
#endif
			WMA_LOGD("%s: Update peer(%pM) state into conn",
				 __func__, add_bss->bssId);
			ol_txrx_peer_state_update(pdev, add_bss->bssId,
						  ol_txrx_peer_state_conn);
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			peer = ol_txrx_find_peer_by_addr(pdev, add_bss->bssId,
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
			ol_txrx_vdev_pause(vdev,
					  OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED);
#endif
		}

		wmi_unified_send_txbf(wma, &add_bss->staContext);

		pps_val =
			((pMac->
			  enable5gEBT << 31) & 0xffff0000) | (PKT_PWR_SAVE_5G_EBT &
							      0xffff);
		ret = wmi_unified_vdev_set_param_send(wma->wmi_handle, vdev_id,
							WMI_VDEV_PARAM_PACKET_POWERSAVE,
							pps_val);
		if (ret)
			WMA_LOGE("Failed to send wmi packet power save cmd");
		else
			WMA_LOGD("Sent PKT_PWR_SAVE_5G_EBT cmd to target, val = %x, ret = %d",
				pps_val, ret);

		wmi_unified_send_peer_assoc(wma, add_bss->nwType,
					    &add_bss->staContext);
		peer_assoc_sent = true;
#ifdef WLAN_FEATURE_11W
		if (add_bss->rmfEnabled) {
			/* when 802.11w PMF is enabled for hw encr/decr
			   use hw MFP Qos bits 0x10 */
			ret = wmi_unified_pdev_set_param(wma->wmi_handle,
							 WMI_PDEV_PARAM_PMF_QOS,
							 true);
			if (ret) {
				WMA_LOGE("%s: Failed to set QOS MFP/PMF (%d)",
					 __func__, ret);
			} else {
				WMA_LOGI("%s: QOS MFP/PMF set to %d",
					 __func__, true);
			}
		}
#endif /* WLAN_FEATURE_11W */

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
		cdf_mem_copy(iface->bssid, add_bss->bssId, IEEE80211_ADDR_LEN);

	}
send_bss_resp:
	ol_txrx_find_peer_by_addr(pdev, add_bss->bssId,
				  &add_bss->staContext.staIdx);
	add_bss->status = (add_bss->staContext.staIdx < 0) ?
			  CDF_STATUS_E_FAILURE : CDF_STATUS_SUCCESS;
	add_bss->bssIdx = add_bss->staContext.smesessionId;
	cdf_mem_copy(add_bss->staContext.staMac, add_bss->bssId,
		     sizeof(add_bss->staContext.staMac));

	if (!WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				    WMI_SERVICE_PEER_ASSOC_CONF)) {
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
		WMA_LOGP(FL("Failed to allocate request for vdev_id %d"),
			 vdev_id);
		wma_remove_req(wma, vdev_id, WMA_PEER_ASSOC_CNF_START);
		goto peer_cleanup;
	}
	return;

send_final_rsp:
	WMA_LOGD("%s: opermode %d update_bss %d nw_type %d bssid %pM"
		 " staIdx %d status %d", __func__, add_bss->operMode,
		 add_bss->updateBss, add_bss->nwType, add_bss->bssId,
		 add_bss->staContext.staIdx, add_bss->status);
	wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
	return;

peer_cleanup:
	wma_remove_peer(wma, add_bss->bssId, vdev_id, peer,
			roam_synch_in_progress);
send_fail_resp:
	add_bss->status = CDF_STATUS_E_FAILURE;
	wma_send_msg(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
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

	case CDF_SAP_MODE:
	case CDF_P2P_GO_MODE:
		/*If current bring up SAP/P2P channel matches the previous
		 *radar found channel then reset the last_radar_found_chan
		 *variable to avoid race conditions.
		 */
		if (params->currentOperChannel ==
			wma->dfs_ic->last_radar_found_chan)
			wma->dfs_ic->last_radar_found_chan = 0;

		wma_add_bss_ap_mode(wma, params);
		break;

#ifdef QCA_IBSS_SUPPORT
	case CDF_IBSS_MODE:
		wma_add_bss_ibss_mode(wma, params);
		break;
#endif

	default:
		wma_add_bss_sta_mode(wma, params);
		break;
	}
}

/**
 * wmi_unified_vdev_up_send() - send vdev up command in fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 * @aid: association ID
 * @bssid: bssid
 *
 * Return: 0 for success or error code
 */
int wmi_unified_vdev_up_send(wmi_unified_t wmi,
			     uint8_t vdev_id, uint16_t aid,
			     uint8_t bssid[IEEE80211_ADDR_LEN])
{
	wmi_vdev_up_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	WMA_LOGD("%s: VDEV_UP", __func__);
	WMA_LOGD("%s: vdev_id %d aid %d bssid %pM", __func__,
		 vdev_id, aid, bssid);
	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s: wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_up_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_up_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_up_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->vdev_assoc_id = aid;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(bssid, &cmd->vdev_bssid);
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_UP_CMDID)) {
		WMA_LOGP("%s: Failed to send vdev up command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	return 0;
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
	enum ol_txrx_peer_state state = ol_txrx_peer_state_conn;
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	ol_txrx_peer_handle peer;
	uint8_t peer_id;
	CDF_STATUS status;
	int32_t ret;
	struct wma_txrx_node *iface = NULL;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		add_sta->status = CDF_STATUS_E_FAILURE;
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
		add_sta->status = CDF_STATUS_SUCCESS;
		goto send_rsp;
	}

	vdev = wma_find_vdev_by_id(wma, add_sta->smesessionId);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		add_sta->status = CDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	iface = &wma->interfaces[vdev->vdev_id];
	peer = ol_txrx_find_peer_by_addr_and_vdev(pdev,
						  vdev,
						  add_sta->staMac, &peer_id);
	if (peer) {
		wma_remove_peer(wma, add_sta->staMac, add_sta->smesessionId,
				peer, false);
		WMA_LOGE("%s: Peer already exists, Deleted peer with peer_addr %pM",
			__func__, add_sta->staMac);
	}

	status = wma_create_peer(wma, pdev, vdev, add_sta->staMac,
				 WMI_PEER_TYPE_DEFAULT, add_sta->smesessionId,
				 false);
	if (status != CDF_STATUS_SUCCESS) {
		WMA_LOGE("%s: Failed to create peer for %pM",
			 __func__, add_sta->staMac);
		add_sta->status = status;
		goto send_rsp;
	}

	peer = ol_txrx_find_peer_by_addr_and_vdev(pdev,
						  vdev,
						  add_sta->staMac, &peer_id);
	if (!peer) {
		WMA_LOGE("%s: Failed to find peer handle using peer mac %pM",
			 __func__, add_sta->staMac);
		add_sta->status = CDF_STATUS_E_FAILURE;
		wma_remove_peer(wma, add_sta->staMac, add_sta->smesessionId,
				peer, false);
		goto send_rsp;
	}

	wmi_unified_send_txbf(wma, add_sta);

	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				    WMI_SERVICE_PEER_ASSOC_CONF)) {
		peer_assoc_cnf = true;
		msg = wma_fill_hold_req(wma, add_sta->smesessionId,
				   WMA_ADD_STA_REQ, WMA_PEER_ASSOC_CNF_START,
				   add_sta, WMA_PEER_ASSOC_TIMEOUT);
		if (!msg) {
			WMA_LOGP(FL("Failed to alloc request for vdev_id %d"),
				 add_sta->smesessionId);
			add_sta->status = CDF_STATUS_E_FAILURE;
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

	ret = wmi_unified_send_peer_assoc(wma, add_sta->nwType, add_sta);
	if (ret) {
		add_sta->status = CDF_STATUS_E_FAILURE;
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
	    WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				   WMI_SERVICE_IBSS_PWRSAVE)) {
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

#ifdef WLAN_FEATURE_11W
	if (add_sta->rmfEnabled) {
		/*
		 * We have to store the state of PMF connection
		 * per STA for SAP case
		 * We will isolate the ifaces based on vdevid
		 */
		iface->rmfEnabled = add_sta->rmfEnabled;
		/*
		 * when 802.11w PMF is enabled for hw encr/decr
		 * use hw MFP Qos bits 0x10
		 */
		ret = wmi_unified_pdev_set_param(wma->wmi_handle,
						 WMI_PDEV_PARAM_PMF_QOS, true);
		if (ret) {
			WMA_LOGE("%s: Failed to set QOS MFP/PMF (%d)",
				 __func__, ret);
		} else {
			WMA_LOGI("%s: QOS MFP/PMF set to %d", __func__, true);
		}
	}
#endif /* WLAN_FEATURE_11W */

	if (add_sta->uAPSD) {
		ret = wma_set_ap_peer_uapsd(wma, add_sta->smesessionId,
					    add_sta->staMac,
					    add_sta->uAPSD, add_sta->maxSPLen);
		if (ret) {
			WMA_LOGE("Failed to set peer uapsd param for %pM",
				 add_sta->staMac);
			add_sta->status = CDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			goto send_rsp;
		}
	}

	WMA_LOGD("%s: Moving peer %pM to state %d",
		 __func__, add_sta->staMac, state);
	ol_txrx_peer_state_update(pdev, add_sta->staMac, state);

	add_sta->staIdx = ol_txrx_local_peer_id(peer);
	add_sta->nss    = iface->nss;
	add_sta->status = CDF_STATUS_SUCCESS;
send_rsp:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf) {
		WMA_LOGI(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
		return;
	}

	WMA_LOGE(FL("statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
		 add_sta->staType, add_sta->smesessionId,
		 add_sta->assocId, add_sta->bssId, add_sta->staIdx,
		 add_sta->status);
	wma_send_msg(wma, WMA_ADD_STA_RSP, (void *)add_sta, 0);
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
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	ol_txrx_peer_handle peer;
	uint8_t peer_id;
	CDF_STATUS status;
	int32_t ret;
	tTdlsPeerStateParams *peerStateParams;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;

	WMA_LOGD("%s: staType: %d, staIdx: %d, updateSta: %d, "
		 "bssId: %pM, staMac: %pM",
		 __func__, add_sta->staType, add_sta->staIdx,
		 add_sta->updateSta, add_sta->bssId, add_sta->staMac);

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		add_sta->status = CDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	vdev = wma_find_vdev_by_id(wma, add_sta->smesessionId);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		add_sta->status = CDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	if (0 == add_sta->updateSta) {
		/* its a add sta request * */
		WMA_LOGD("%s: addSta, calling wma_create_peer for %pM, vdev_id %hu",
			__func__, add_sta->staMac, add_sta->smesessionId);

		status = wma_create_peer(wma, pdev, vdev, add_sta->staMac,
					 WMI_PEER_TYPE_TDLS,
					 add_sta->smesessionId, false);
		if (status != CDF_STATUS_SUCCESS) {
			WMA_LOGE("%s: Failed to create peer for %pM",
				 __func__, add_sta->staMac);
			add_sta->status = status;
			goto send_rsp;
		}

		peer = ol_txrx_find_peer_by_addr(pdev, add_sta->staMac, &peer_id);
		if (!peer) {
			WMA_LOGE("%s: addSta, failed to find peer handle for mac %pM",
				__func__, add_sta->staMac);
			add_sta->status = CDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			goto send_rsp;
		}

		add_sta->staIdx = ol_txrx_local_peer_id(peer);
		WMA_LOGD("%s: addSta, after calling ol_txrx_local_peer_id, "
			 "staIdx: %d, staMac: %pM",
			 __func__, add_sta->staIdx, add_sta->staMac);

		peerStateParams = cdf_mem_malloc(sizeof(tTdlsPeerStateParams));
		if (!peerStateParams) {
			WMA_LOGE
				("%s: Failed to allocate memory for peerStateParams for %pM",
				__func__, add_sta->staMac);
			add_sta->status = CDF_STATUS_E_NOMEM;
			goto send_rsp;
		}

		cdf_mem_zero(peerStateParams, sizeof(*peerStateParams));
		peerStateParams->peerState = WMI_TDLS_PEER_STATE_PEERING;
		peerStateParams->vdevId = vdev->vdev_id;
		cdf_mem_copy(&peerStateParams->peerMacAddr,
			     &add_sta->staMac, sizeof(tSirMacAddr));
		wma_update_tdls_peer_state(wma, peerStateParams);
	} else {
		/* its a change sta request * */
		peer =
			ol_txrx_find_peer_by_addr(pdev, add_sta->staMac, &peer_id);
		if (!peer) {
			WMA_LOGE("%s: changeSta,failed to find peer handle for mac %pM",
				__func__, add_sta->staMac);
			add_sta->status = CDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			goto send_rsp;
		}

		if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
					    WMI_SERVICE_PEER_ASSOC_CONF)) {
			WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
			peer_assoc_cnf = true;
			msg = wma_fill_hold_req(wma, add_sta->smesessionId,
				WMA_ADD_STA_REQ, WMA_PEER_ASSOC_CNF_START,
				add_sta, WMA_PEER_ASSOC_TIMEOUT);
			if (!msg) {
				WMA_LOGP(FL("Failed to alloc request for vdev_id %d"),
					 add_sta->smesessionId);
				add_sta->status = CDF_STATUS_E_FAILURE;
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

		WMA_LOGD("%s: changeSta, calling wmi_unified_send_peer_assoc",
			 __func__);

		ret =
			wmi_unified_send_peer_assoc(wma, add_sta->nwType, add_sta);
		if (ret) {
			add_sta->status = CDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, add_sta->staMac,
					add_sta->smesessionId, peer, false);
			goto send_rsp;
		}
	}

send_rsp:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf)
		return;

	WMA_LOGE(FL("statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
		 add_sta->staType, add_sta->smesessionId,
		 add_sta->assocId, add_sta->bssId, add_sta->staIdx,
		 add_sta->status);
	wma_send_msg(wma, WMA_ADD_STA_RSP, (void *)add_sta, 0);
}
#endif

/**
 * wma_add_sta_req_sta_mode() - process add sta request in sta mode
 * @wma: wma handle
 * @add_sta: add sta params
 *
 * Return: none
 */
static void wma_add_sta_req_sta_mode(tp_wma_handle wma, tpAddStaParams params)
{
	ol_txrx_pdev_handle pdev;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	ol_txrx_peer_handle peer;
	struct wma_txrx_node *iface;
	tPowerdBm maxTxPower;
	int ret = 0;
	struct wma_target_req *msg;
	bool peer_assoc_cnf = false;

#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType) {
		wma_add_tdls_sta(wma, params);
		return;
	}
#endif

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Unable to get pdev", __func__);
		goto out;
	}

	iface = &wma->interfaces[params->smesessionId];
	if (params->staType != STA_ENTRY_SELF) {
		WMA_LOGP("%s: unsupported station type %d",
			 __func__, params->staType);
		goto out;
	}
	peer = ol_txrx_find_peer_by_addr(pdev, params->bssId, &params->staIdx);
	if (params->nonRoamReassoc) {
		ol_txrx_peer_state_update(pdev, params->bssId,
					  ol_txrx_peer_state_auth);
		cdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STARTED);
		iface->aid = params->assocId;
		goto out;
	}

	if (wma->interfaces[params->smesessionId].vdev_up == true) {
		WMA_LOGE("%s: vdev id %d is already UP for %pM", __func__,
			params->smesessionId, params->bssId);
		status = CDF_STATUS_E_FAILURE;
		goto out;
	}

	if (peer != NULL && peer->state == ol_txrx_peer_state_disc) {
		/*
		 * This is the case for reassociation.
		 * peer state update and peer_assoc is required since it
		 * was not done by WMA_ADD_BSS_REQ.
		 */

		/* Update peer state */
		if (params->encryptType == eSIR_ED_NONE) {
			WMA_LOGD("%s: Update peer(%pM) state into auth",
				 __func__, params->bssId);
			ol_txrx_peer_state_update(pdev, params->bssId,
						  ol_txrx_peer_state_auth);
		} else {
			WMA_LOGD("%s: Update peer(%pM) state into conn",
				 __func__, params->bssId);
			ol_txrx_peer_state_update(pdev, params->bssId,
						  ol_txrx_peer_state_conn);
		}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (iface->roam_synch_in_progress) {
			/* iface->nss = params->nss; */
			/*In LFR2.0, the following operations are performed as
			 * part of wmi_unified_send_peer_assoc. As we are
			 * skipping this operation, we are just executing the
			 * following which are useful for LFR3.0.*/
			ol_txrx_peer_state_update(pdev, params->bssId,
						  ol_txrx_peer_state_auth);
			cdf_atomic_set(&iface->bss_status,
				       WMA_BSS_STATUS_STARTED);
			iface->aid = params->assocId;
			goto out;
		}
#endif
		wmi_unified_send_txbf(wma, params);

		if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
					    WMI_SERVICE_PEER_ASSOC_CONF)) {
			WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF is enabled"));
			peer_assoc_cnf = true;
			msg = wma_fill_hold_req(wma, params->smesessionId,
				WMA_ADD_STA_REQ, WMA_PEER_ASSOC_CNF_START,
				params, WMA_PEER_ASSOC_TIMEOUT);
			if (!msg) {
				WMA_LOGP(FL("Failed to alloc request for vdev_id %d"),
					 params->smesessionId);
				params->status = CDF_STATUS_E_FAILURE;
				wma_remove_req(wma, params->smesessionId,
					       WMA_PEER_ASSOC_CNF_START);
				wma_remove_peer(wma, params->staMac,
					params->smesessionId, peer, false);
				peer_assoc_cnf = false;
				goto out;
			}
		} else {
			WMA_LOGE(FL("WMI_SERVICE_PEER_ASSOC_CONF not enabled"));
		}

		ret = wmi_unified_send_peer_assoc(wma,
				iface->nwType,
				(tAddStaParams *) iface->addBssStaContext);
		if (ret) {
			status = CDF_STATUS_E_FAILURE;
			wma_remove_peer(wma, params->bssId,
					params->smesessionId, peer, false);
			goto out;
		}
#ifdef WLAN_FEATURE_11W
		if (params->rmfEnabled) {
			/* when 802.11w PMF is enabled for hw encr/decr
			   use hw MFP Qos bits 0x10 */
			ret = wmi_unified_pdev_set_param(wma->wmi_handle,
							 WMI_PDEV_PARAM_PMF_QOS,
							 true);
			if (ret) {
				WMA_LOGE("%s: Failed to set QOS MFP/PMF (%d)",
					 __func__, ret);
			} else {
				WMA_LOGI("%s: QOS MFP/PMF set to %d",
					 __func__, true);
			}
		}
#endif /* WLAN_FEATURE_11W */
#if defined WLAN_FEATURE_VOWIFI_11R
		/*
		 * Set the PTK in 11r mode because we already have it.
		 */
		if (iface->staKeyParams) {
			wma_set_stakey(wma,
				       (tpSetStaKeyParams) iface->staKeyParams);
		}
#endif
	}
#if defined WLAN_FEATURE_VOWIFI
	maxTxPower = params->maxTxPower;
#else
	maxTxPower = 0;
#endif
	wma_vdev_set_bss_params(wma, params->smesessionId,
				iface->beaconInterval, iface->dtimPeriod,
				iface->shortSlotTimeSupported,
				iface->llbCoexist, maxTxPower);

	params->csaOffloadEnable = 0;
	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				   WMI_SERVICE_CSA_OFFLOAD)) {
		params->csaOffloadEnable = 1;
		if (wmi_unified_csa_offload_enable(wma, params->smesessionId) <
		    0) {
			WMA_LOGE("Unable to enable CSA offload for vdev_id:%d",
				 params->smesessionId);
		}
	}

	if (WMI_SERVICE_IS_ENABLED(wma->wmi_service_bitmap,
				   WMI_SERVICE_FILTER_IPSEC_NATKEEPALIVE)) {
		if (wmi_unified_nat_keepalive_enable(wma, params->smesessionId)
		    < 0) {
			WMA_LOGE("Unable to enable NAT keepalive for vdev_id:%d",
				params->smesessionId);
		}
	}

	if (wmi_unified_vdev_up_send(wma->wmi_handle, params->smesessionId,
				     params->assocId, params->bssId) < 0) {
		WMA_LOGP("%s: Failed to send vdev up cmd: vdev %d bssid %pM",
			 __func__, params->smesessionId, params->bssId);
		status = CDF_STATUS_E_FAILURE;
	} else {
		wma->interfaces[params->smesessionId].vdev_up = true;
	}

	cdf_atomic_set(&iface->bss_status, WMA_BSS_STATUS_STARTED);
	WMA_LOGD("%s: STA mode (type %d subtype %d) BSS is started",
		 __func__, iface->type, iface->sub_type);
	/* Sta is now associated, configure various params */

	/* SM power save, configure the h/w as configured
	 * in the ini file. SMPS is not published in assoc
	 * request. Once configured, fw sends the required
	 * action frame to AP.
	 */
	if (params->enableHtSmps)
		wma_set_mimops(wma, params->smesessionId, params->htSmpsconfig);

#ifdef WLAN_FEATURE_11AC
	/* Partial AID match power save, enable when SU bformee */
	if (params->enableVhtpAid && params->vhtTxBFCapable)
		wma_set_ppsconfig(params->smesessionId,
				  WMA_VHT_PPS_PAID_MATCH, 1);
#endif

	/* Enable AMPDU power save, if htCapable/vhtCapable */
	if (params->enableAmpduPs && (params->htCapable || params->vhtCapable))
		wma_set_ppsconfig(params->smesessionId,
				  WMA_VHT_PPS_DELIM_CRC_FAIL, 1);
	iface->aid = params->assocId;
	params->nss = iface->nss;
out:
	/* Do not send add stat resp when peer assoc cnf is enabled */
	if (peer_assoc_cnf)
		return;

	params->status = status;
	WMA_LOGE(FL("statype %d vdev_id %d aid %d bssid %pM staIdx %d status %d"),
		 params->staType, params->smesessionId,
		 params->assocId, params->bssId, params->staIdx,
		 params->status);
	wma_send_msg(wma, WMA_ADD_STA_RSP, (void *)params, 0);
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
	ol_txrx_pdev_handle pdev;
	struct ol_txrx_peer_t *peer;

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		del_sta->status = CDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	peer = ol_txrx_peer_find_by_local_id(pdev, del_sta->staIdx);
	if (!peer) {
		WMA_LOGE("%s: Failed to get peer handle using peer id %d",
			 __func__, del_sta->staIdx);
		del_sta->status = CDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	wma_remove_peer(wma, peer->mac_addr.raw, del_sta->smesessionId, peer,
			false);
	del_sta->status = CDF_STATUS_SUCCESS;

send_del_rsp:
	if (del_sta->respReqd) {
		WMA_LOGD("%s: Sending del rsp to umac (status: %d)",
			 __func__, del_sta->status);
		wma_send_msg(wma, WMA_DELETE_STA_RSP, (void *)del_sta, 0);
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
	ol_txrx_pdev_handle pdev;
	ol_txrx_vdev_handle vdev;
	struct ol_txrx_peer_t *peer;
	tTdlsPeerStateParams *peerStateParams;

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s: Failed to find pdev", __func__);
		del_sta->status = CDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	vdev = wma_find_vdev_by_id(wma, del_sta->smesessionId);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		del_sta->status = CDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	peer = ol_txrx_peer_find_by_local_id(pdev, del_sta->staIdx);
	if (!peer) {
		WMA_LOGE("%s: Failed to get peer handle using peer id %d",
			 __func__, del_sta->staIdx);
		del_sta->status = CDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	peerStateParams = cdf_mem_malloc(sizeof(tTdlsPeerStateParams));
	if (!peerStateParams) {
		WMA_LOGE("%s: Failed to allocate memory for peerStateParams for: %pM",
			__func__, del_sta->staMac);
		del_sta->status = CDF_STATUS_E_NOMEM;
		goto send_del_rsp;
	}

	cdf_mem_zero(peerStateParams, sizeof(*peerStateParams));
	peerStateParams->peerState = WMA_TDLS_PEER_STATE_TEARDOWN;
	peerStateParams->vdevId = vdev->vdev_id;
	cdf_mem_copy(&peerStateParams->peerMacAddr,
		     &del_sta->staMac, sizeof(tSirMacAddr));

	WMA_LOGD("%s: sending tdls_peer_state for peer mac: %pM, "
		 " peerState: %d",
		 __func__, peerStateParams->peerMacAddr,
		 peerStateParams->peerState);

	wma_update_tdls_peer_state(wma, peerStateParams);

	del_sta->status = CDF_STATUS_SUCCESS;

send_del_rsp:
	if (del_sta->respReqd) {
		WMA_LOGD("%s: Sending del rsp to umac (status: %d)",
			 __func__, del_sta->status);
		wma_send_msg(wma, WMA_DELETE_STA_RSP, (void *)del_sta, 0);
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
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	struct wma_txrx_node *iface;
	iface = &wma->interfaces[params->smesessionId];
	iface->uapsd_cached_val = 0;

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	/* In case of LFR3.0 we need not send any
	 * WMI commands to FW before SYNCH_CONFIRM */
	if (iface->roam_synch_in_progress)
		goto send_del_sta_rsp;
#endif
#ifdef FEATURE_WLAN_TDLS
	if (STA_ENTRY_TDLS_PEER == params->staType) {
		wma_del_tdls_sta(wma, params);
		return;
	}
#endif
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
send_del_sta_rsp:
#endif
	params->status = status;
	if (params->respReqd) {
		WMA_LOGD("%s: vdev_id %d status %d", __func__,
			 params->smesessionId, status);
		wma_send_msg(wma, WMA_DELETE_STA_RSP, (void *)params, 0);
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

	WMA_LOGD("%s: add_sta->sessionId = %d.", __func__,
		 add_sta->smesessionId);
	WMA_LOGD("%s: add_sta->bssId = %x:%x:%x:%x:%x:%x", __func__,
		 add_sta->bssId[0], add_sta->bssId[1], add_sta->bssId[2],
		 add_sta->bssId[3], add_sta->bssId[4], add_sta->bssId[5]);

	if (wma_is_vdev_in_ap_mode(wma, add_sta->smesessionId))
		oper_mode = BSS_OPERATIONAL_MODE_AP;
#ifdef QCA_IBSS_SUPPORT
	else if (wma_is_vdev_in_ibss_mode(wma, add_sta->smesessionId))
		oper_mode = BSS_OPERATIONAL_MODE_IBSS;
#endif

	switch (oper_mode) {
	case BSS_OPERATIONAL_MODE_STA:
		wma_add_sta_req_sta_mode(wma, add_sta);
		break;

#ifdef QCA_IBSS_SUPPORT
	case BSS_OPERATIONAL_MODE_IBSS: /* IBSS should share the same code as AP mode */
#endif
	case BSS_OPERATIONAL_MODE_AP:
		hif_vote_link_up();
		wma_add_sta_req_ap_mode(wma, add_sta);
		break;
	}

#ifdef QCA_IBSS_SUPPORT
	/* adjust heart beat thresold timer value for detecting ibss peer departure */
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

	if (wma_is_vdev_in_ap_mode(wma, smesession_id))
		oper_mode = BSS_OPERATIONAL_MODE_AP;
#ifdef QCA_IBSS_SUPPORT
	if (wma_is_vdev_in_ibss_mode(wma, smesession_id)) {
		oper_mode = BSS_OPERATIONAL_MODE_IBSS;
		WMA_LOGD("%s: to delete sta for IBSS mode", __func__);
	}
#endif

	switch (oper_mode) {
	case BSS_OPERATIONAL_MODE_STA:
		wma_delete_sta_req_sta_mode(wma, del_sta);
		break;

#ifdef QCA_IBSS_SUPPORT
	case BSS_OPERATIONAL_MODE_IBSS: /* IBSS shares AP code */
#endif
	case BSS_OPERATIONAL_MODE_AP:
		hif_vote_link_down();
		wma_delete_sta_req_ap_mode(wma, del_sta);
		break;
	}

#ifdef QCA_IBSS_SUPPORT
	/* adjust heart beat thresold timer value for
	 * detecting ibss peer departure
	 */
	if (oper_mode == BSS_OPERATIONAL_MODE_IBSS)
		wma_adjust_ibss_heart_beat_timer(wma, smesession_id, -1);
#endif
	if (!rsp_requested) {
		WMA_LOGD("%s: vdev_id %d status %d", __func__,
			 del_sta->smesessionId, del_sta->status);
		cdf_mem_free(del_sta);
	}
}

/**
 * wmi_unified_vdev_stop_send() - send vdev stop command to fw
 * @wmi: wmi handle
 * @vdev_id: vdev id
 *
 * Return: 0 for success or erro code
 */
int32_t wmi_unified_vdev_stop_send(wmi_unified_t wmi, uint8_t vdev_id)
{
	wmi_vdev_stop_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len = sizeof(*cmd);

	buf = wmi_buf_alloc(wmi, len);
	if (!buf) {
		WMA_LOGP("%s : wmi_buf_alloc failed", __func__);
		return -ENOMEM;
	}
	cmd = (wmi_vdev_stop_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_vdev_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_vdev_stop_cmd_fixed_param));
	cmd->vdev_id = vdev_id;
	if (wmi_unified_cmd_send(wmi, buf, len, WMI_VDEV_STOP_CMDID)) {
		WMA_LOGP("%s: Failed to send vdev stop command", __func__);
		cdf_nbuf_free(buf);
		return -EIO;
	}
	return 0;
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
	ol_txrx_pdev_handle pdev;
	ol_txrx_peer_handle peer = NULL;
	struct wma_target_req *msg;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	uint8_t peer_id;
	uint8_t max_wait_iterations = 0;
	ol_txrx_vdev_handle txrx_vdev = NULL;
	bool roam_synch_in_progress = false;

	pdev = cds_get_context(CDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE("%s:Unable to get TXRX context", __func__);
		goto out;
	}
#ifdef QCA_IBSS_SUPPORT
	if (wma_is_vdev_in_ibss_mode(wma, params->smesessionId))
		/* in rome ibss case, self mac is used to create the bss peer */
		peer = ol_txrx_find_peer_by_addr(pdev,
			wma->interfaces[params->smesessionId].addr,
			&peer_id);
	else
#endif
	peer = ol_txrx_find_peer_by_addr(pdev, params->bssid, &peer_id);

	if (!peer) {
		WMA_LOGP("%s: Failed to find peer %pM", __func__,
			 params->bssid);
		status = CDF_STATUS_E_FAILURE;
		goto out;
	}

	cdf_mem_zero(wma->interfaces[params->smesessionId].bssid,
			IEEE80211_ADDR_LEN);

	txrx_vdev = wma_find_vdev_by_id(wma, params->smesessionId);
	if (!txrx_vdev) {
		WMA_LOGE("%s:Invalid vdev handle", __func__);
		status = CDF_STATUS_E_FAILURE;
		goto out;
	}

	/*Free the allocated stats response buffer for the the session */
	if (wma->interfaces[params->smesessionId].stats_rsp) {
		cdf_mem_free(wma->interfaces[params->smesessionId].stats_rsp);
		wma->interfaces[params->smesessionId].stats_rsp = NULL;
	}

	if (wma->interfaces[params->smesessionId].psnr_req) {
		cdf_mem_free(wma->interfaces[params->smesessionId].psnr_req);
		wma->interfaces[params->smesessionId].psnr_req = NULL;
	}

	if (wlan_op_mode_ibss == txrx_vdev->opmode) {
		wma->ibss_started = 0;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (wma->interfaces[params->smesessionId].roam_synch_in_progress) {
		roam_synch_in_progress = true;
		WMA_LOGD("LFR3:%s: Setting vdev_up to FALSE for session %d",
			__func__, params->smesessionId);
		wma->interfaces[params->smesessionId].vdev_up = false;
		goto detach_peer;
	}
#endif
	msg = wma_fill_vdev_req(wma, params->smesessionId, WMA_DELETE_BSS_REQ,
				WMA_TARGET_REQ_TYPE_VDEV_STOP, params,
				WMA_VDEV_STOP_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGP("%s: Failed to fill vdev request for vdev_id %d",
			 __func__, params->smesessionId);
		status = CDF_STATUS_E_NOMEM;
		goto detach_peer;
	}

	WMA_LOGW(FL("Outstanding msdu packets: %d"),
		 ol_txrx_get_tx_pending(pdev));

	max_wait_iterations =
		wma->interfaces[params->smesessionId].delay_before_vdev_stop /
		WMA_TX_Q_RECHECK_TIMER_WAIT;

	while (ol_txrx_get_tx_pending(pdev) && max_wait_iterations) {
		WMA_LOGW(FL("Waiting for outstanding packet to drain."));
		cdf_wait_single_event(&wma->tx_queue_empty_event,
				      WMA_TX_Q_RECHECK_TIMER_MAX_WAIT);
		max_wait_iterations--;
	}

	if (ol_txrx_get_tx_pending(pdev)) {
		WMA_LOGW(FL("Outstanding msdu packets before VDEV_STOP : %d"),
			 ol_txrx_get_tx_pending(pdev));
	}

	WMA_LOGD("%s, vdev_id: %d, pausing tx_ll_queue for VDEV_STOP (del_bss)",
		 __func__, params->smesessionId);
	ol_txrx_vdev_pause(wma->interfaces[params->smesessionId].handle,
			   OL_TXQ_PAUSE_REASON_VDEV_STOP);
	wma->interfaces[params->smesessionId].pause_bitmap |=
							(1 << PAUSE_TYPE_HOST);

	if (wmi_unified_vdev_stop_send(wma->wmi_handle, params->smesessionId)) {
		WMA_LOGP("%s: %d Failed to send vdev stop", __func__, __LINE__);
		wma_remove_vdev_req(wma, params->smesessionId,
				    WMA_TARGET_REQ_TYPE_VDEV_STOP);
		status = CDF_STATUS_E_FAILURE;
		goto detach_peer;
	}
	WMA_LOGD("%s: bssid %pM vdev_id %d",
		 __func__, params->bssid, params->smesessionId);
	return;
detach_peer:
	wma_remove_peer(wma, params->bssid, params->smesessionId, peer,
			roam_synch_in_progress);
out:
	params->status = status;
	wma_send_msg(wma, WMA_DELETE_BSS_RSP, (void *)params, 0);
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
	ol_txrx_vdev_handle txrx_vdev;
	WMA_LOGD("%s:intra_fwd:vdev(%d) intrabss_dis=%s",
		 __func__, pdis_intra_fwd->sessionId,
		 (pdis_intra_fwd->disableintrabssfwd ? "true" : "false"));

	txrx_vdev = wma_handle->interfaces[pdis_intra_fwd->sessionId].handle;
	ol_vdev_rx_set_intrabss_fwd(txrx_vdev,
				    pdis_intra_fwd->disableintrabssfwd);
}

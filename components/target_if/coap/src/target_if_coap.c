/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains CoAP target if functions
 */
#include <wlan_coap_main.h>
#include <target_if_coap.h>
#include <wmi_unified_coap_api.h>

/**
 * target_if_wow_coap_buf_info_event_handler() - function to handle CoAP
 * buf info event from firmware.
 * @scn: scn handle
 * @data: data buffer for event
 * @datalen: data length
 *
 * Return: status of operation.
 */
static int
target_if_wow_coap_buf_info_event_handler(ol_scn_t scn, uint8_t *data,
					  uint32_t datalen)
{
	QDF_STATUS status;
	struct wlan_objmgr_psoc *psoc;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_coap_comp_priv *coap_priv;
	struct coap_buf_info info = {0};
	struct coap_buf_node *cur, *next;

	if (!scn || !data) {
		coap_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		coap_err("null psoc");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		coap_err("wmi_handle is null");
		return -EINVAL;
	}

	qdf_list_create(&info.info_list, 0);
	status = wmi_unified_coap_extract_buf_info(wmi_handle, data,
						   &info);
	if (QDF_IS_STATUS_ERROR(status))
		goto out;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, info.vdev_id,
						    WLAN_COAP_ID);
	if (!vdev) {
		coap_err("vdev is NULL, vdev_id: %d", info.vdev_id);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	coap_priv = wlan_get_vdev_coap_obj(vdev);
	if (!coap_priv->cache_get_cbk || !coap_priv->cache_get_context) {
		coap_err("req id %d: callback or context is NULL",
			 coap_priv->req_id);
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	coap_priv->cache_get_cbk(coap_priv->cache_get_context, &info);
out:
	qdf_list_for_each_del(&info.info_list, cur, next, node) {
		qdf_list_remove_node(&info.info_list, &cur->node);
		qdf_mem_free(cur->payload);
		qdf_mem_free(cur);
	}

	if (vdev)
		wlan_objmgr_vdev_release_ref(vdev, WLAN_COAP_ID);
	return qdf_status_to_os_return(status);
}

/**
 * target_if_coap_register_event_handler() - Register CoAP related wmi events
 * @psoc: psoc handle
 *
 * Register CoAP related WMI events
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
target_if_coap_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS ret_val;
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		coap_err("PSOC is NULL!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		coap_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}

	ret_val = wmi_unified_register_event_handler(wmi_handle,
			wmi_wow_coap_buf_info_eventid,
			target_if_wow_coap_buf_info_event_handler,
			WMI_RX_WORK_CTX);
	if (QDF_IS_STATUS_ERROR(ret_val))
		coap_err("Failed to register coap buf info event cb");

	return ret_val;
}

/**
 * target_if_coap_unregister_event_handler() - Unregister CoAP related wmi
 * events
 * @psoc: psoc handle
 *
 * Register CoAP related WMI events
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
target_if_coap_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	if (!psoc) {
		coap_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		coap_err("wmi_handle is null");
		return QDF_STATUS_E_INVAL;
	}
	wmi_unified_unregister_event_handler(wmi_handle,
					     wmi_wow_coap_buf_info_eventid);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_coap_offload_reply_enable() - enable CoAP offload reply
 * @vdev: pointer to vdev object
 * @param: parameters for CoAP offload reply
 *
 * Return: status of operation
 */
static QDF_STATUS
target_if_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
				    struct coap_offload_reply_param *param)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_handle) {
		coap_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_coap_add_pattern_cmd(wmi_handle, param);
}

/**
 * target_if_coap_offload_reply_disable() - disable CoAP offload reply
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
static QDF_STATUS
target_if_coap_offload_reply_disable(struct wlan_objmgr_vdev *vdev,
				     uint32_t req_id)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_handle) {
		coap_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_coap_del_pattern_cmd(wmi_handle,
						wlan_vdev_get_id(vdev),
						req_id);
}

/**
 * target_if_coap_offload_periodic_tx_enable() - enable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @param: parameters for CoAP periodic transmitting
 *
 * Return: status of operation
 */
static QDF_STATUS
target_if_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
			struct coap_offload_periodic_tx_param *param)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_handle) {
		coap_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_coap_add_keepalive_pattern_cmd(wmi_handle, param);
}

/**
 * target_if_coap_offload_periodic_tx_disable() - disable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
static QDF_STATUS
target_if_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
					   uint32_t req_id)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_pdev *pdev;
	uint8_t vdev_id;

	pdev = wlan_vdev_get_pdev(vdev);
	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_handle) {
		coap_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_id = wlan_vdev_get_id(vdev);
	return wmi_unified_coap_del_keepalive_pattern_cmd(wmi_handle,
							  vdev_id, req_id);
}

/**
 * target_if_coap_offload_cache_get() - get cached CoAP messages
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
static QDF_STATUS
target_if_coap_offload_cache_get(struct wlan_objmgr_vdev *vdev,
				 uint32_t req_id)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_pdev *pdev;

	pdev = wlan_vdev_get_pdev(vdev);
	wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!wmi_handle) {
		coap_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_coap_cache_get(wmi_handle, wlan_vdev_get_id(vdev),
					  req_id);
}

QDF_STATUS
target_if_coap_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_coap_tx_ops *coap_ops;

	if (!tx_ops) {
		coap_err("target if tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	coap_ops = &tx_ops->coap_ops;
	coap_ops->attach = target_if_coap_register_event_handler;
	coap_ops->detach = target_if_coap_unregister_event_handler;
	coap_ops->offload_reply_enable =
		target_if_coap_offload_reply_enable;
	coap_ops->offload_reply_disable =
		target_if_coap_offload_reply_disable;
	coap_ops->offload_periodic_tx_enable =
		target_if_coap_offload_periodic_tx_enable;
	coap_ops->offload_periodic_tx_disable =
		target_if_coap_offload_periodic_tx_disable;
	coap_ops->offload_cache_get = target_if_coap_offload_cache_get;

	return QDF_STATUS_SUCCESS;
}

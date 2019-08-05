/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

/**
 * DOC: target_if_reg_11d.c
 * This file contains regulatory target interface
 */

#include "target_if_reg_11d.h"

#ifdef HOST_11D_SCAN
bool tgt_if_regulatory_is_11d_offloaded(struct wlan_objmgr_psoc *psoc)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);

	if (!wmi_handle)
		return false;

	if (reg_rx_ops->reg_ignore_fw_reg_offload_ind &&
		reg_rx_ops->reg_ignore_fw_reg_offload_ind(psoc)) {
		target_if_debug("Ignore fw reg 11d offload indication");
		return 0;
	}

	return wmi_service_enabled(wmi_handle, wmi_service_11d_offload);
}

/**
 * tgt_reg_11d_new_cc_handler() - 11d country code event handler
 * @handle: scn handle
 * @event_buf: event buffer
 * @len: legth of @event_buf
 *
 * Return: 0 on success
 */
static int tgt_reg_11d_new_cc_handler(ol_scn_t handle, uint8_t *event_buf,
				      uint32_t len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;
	struct reg_11d_new_country reg_11d_new_cc;
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);

	if (!reg_rx_ops->reg_11d_new_cc_handler) {
		target_if_err("reg_11d_new_cc_handler is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid WMI handle");
		return -EINVAL;
	}
	if (wmi_extract_reg_11d_new_cc_event(wmi_handle, event_buf,
					     &reg_11d_new_cc, len)
	    != QDF_STATUS_SUCCESS) {
		target_if_err("Extraction of new country event failed");
		return -EFAULT;
	}

	status = reg_rx_ops->reg_11d_new_cc_handler(psoc, &reg_11d_new_cc);
	if (status != QDF_STATUS_SUCCESS) {
		target_if_err("Failed to process new country code event");
		return -EFAULT;
	}

	target_if_debug("processed 11d new country code event");

	return 0;
}

QDF_STATUS tgt_if_regulatory_register_11d_new_cc_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_register_event(wmi_handle,
					  wmi_11d_new_country_event_id,
					  tgt_reg_11d_new_cc_handler);
}

QDF_STATUS tgt_if_regulatory_unregister_11d_new_cc_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_unregister_event(wmi_handle,
					    wmi_11d_new_country_event_id);
}

QDF_STATUS tgt_if_regulatory_start_11d_scan(
		struct wlan_objmgr_psoc *psoc,
		struct reg_start_11d_scan_req *reg_start_11d_scan_req)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_send_start_11d_scan_cmd(wmi_handle,
						   reg_start_11d_scan_req);
}

QDF_STATUS tgt_if_regulatory_stop_11d_scan(
		   struct wlan_objmgr_psoc *psoc,
		   struct reg_stop_11d_scan_req *reg_stop_11d_scan_req)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_send_stop_11d_scan_cmd(wmi_handle,
						  reg_stop_11d_scan_req);
}
#endif

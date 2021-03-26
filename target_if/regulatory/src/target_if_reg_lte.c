/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_reg_lte.c
 * This file contains regulatory target LTE interface
 */

#ifdef LTE_COEX

#include "target_if_reg_lte.h"

/**
 * tgt_reg_ch_avoid_event_handler() - Avoid channel list event handler.
 * @handle: Pointer to scn handler.
 * @event_buf: Pointer to event buffer.
 * @len: Buffer length.
 *
 * Return: Error code.
 */
static int tgt_reg_ch_avoid_event_handler(ol_scn_t handle, uint8_t *event_buf,
					  uint32_t len)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_rx_ops *reg_rx_ops;
	struct ch_avoid_ind_type ch_avoid_event;
	QDF_STATUS status;
	struct wmi_unified *wmi_handle;

	TARGET_IF_ENTER();

	psoc = target_if_get_psoc_from_scn_hdl(handle);
	if (!psoc) {
		target_if_err("psoc ptr is NULL");
		return -EINVAL;
	}

	reg_rx_ops = target_if_regulatory_get_rx_ops(psoc);
	if (!reg_rx_ops) {
		target_if_err("reg_rx_ops is NULL");
		return -EINVAL;
	}

	if (!reg_rx_ops->reg_ch_avoid_event_handler) {
		target_if_err("reg_ch_avoid_event_handler is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("Invalid WMI handle");
		return -EINVAL;
	}

	if (wmi_extract_reg_ch_avoid_event(
				wmi_handle, event_buf, &ch_avoid_event, len)
	    != QDF_STATUS_SUCCESS) {
		target_if_err("Extraction of CH avoid event failed");
		return -EFAULT;
	}

	if (!ch_avoid_event.ch_avoid_range_cnt) {
		target_if_debug_rl("ch avoid range count is 0, dropping the event");
		return 0;
	}

	status = reg_rx_ops->reg_ch_avoid_event_handler(psoc, &ch_avoid_event);
	if (status != QDF_STATUS_SUCCESS) {
		target_if_err("Failed to process CH avoid event");
		return -EFAULT;
	}

	TARGET_IF_EXIT();

	return 0;
}

QDF_STATUS tgt_if_regulatory_register_ch_avoid_event_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_register_event(wmi_handle,
					  wmi_wlan_freq_avoid_event_id,
					  tgt_reg_ch_avoid_event_handler);
}

QDF_STATUS tgt_if_regulatory_unregister_ch_avoid_event_handler(
	struct wlan_objmgr_psoc *psoc, void *arg)
{
	wmi_unified_t wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);

	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_unregister_event(wmi_handle,
			wmi_wlan_freq_avoid_event_id);
}
#endif

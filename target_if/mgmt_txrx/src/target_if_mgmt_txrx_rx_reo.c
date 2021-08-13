/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 *  DOC: target_if_mgmt_txrx_rx_reo.c
 *  This file contains definitions of management rx re-ordering related APIs.
 */

#include <wlan_objmgr_psoc_obj.h>
#include <qdf_status.h>
#include <target_if.h>

QDF_STATUS
target_if_mgmt_rx_reo_register_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mgmt_txrx_err("Invalid WMI handle");
		return QDF_STATUS_E_INVAL;
	}

	/* Register mgmt rx fw consumed event handler here */

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_mgmt_rx_reo_unregister_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		mgmt_txrx_err("Invalid WMI handle");
		return QDF_STATUS_E_INVAL;
	}

	/* Unregister mgmt rx fw consumed event handler here */

	return QDF_STATUS_SUCCESS;
}

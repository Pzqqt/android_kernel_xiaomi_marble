/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *  DOC: target_if_twt_evt.c
 *  This file contains twt component's target related function definitions
 */
#include <target_if_twt.h>
#include <target_if_twt_evt.h>
#include <target_if_ext_twt.h>
#include "twt/core/src/wlan_twt_priv.h"
#include <wlan_twt_api.h>
#include <wmi_unified_twt_api.h>

int
target_if_twt_en_complete_event_handler(ol_scn_t scn,
					uint8_t *data, uint32_t datalen)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_twt_rx_ops *rx_ops;
	struct twt_enable_complete_event_param event;
	QDF_STATUS status;

	TARGET_IF_ENTER();

	if (!scn || !data) {
		target_if_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->twt_enable_comp_cb) {
		target_if_err("TWT rx_ops is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	status = wmi_extract_twt_enable_comp_event(wmi_handle, data, &event);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("TWT enable extract event failed(status=%d)",
				status);
		goto end;
	}

	status = rx_ops->twt_enable_comp_cb(psoc, &event);

end:
	return qdf_status_to_os_return(status);
}

int
target_if_twt_disable_comp_event_handler(ol_scn_t scn,
					 uint8_t *data, uint32_t datalen)
{
	wmi_unified_t wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_twt_rx_ops *rx_ops;
	struct twt_disable_complete_event_param event;
	QDF_STATUS status;

	TARGET_IF_ENTER();

	if (!scn || !data) {
		target_if_err("scn: 0x%pK, data: 0x%pK", scn, data);
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	rx_ops = wlan_twt_get_rx_ops(psoc);
	if (!rx_ops || !rx_ops->twt_disable_comp_cb) {
		target_if_err("TWT rx_ops is NULL");
		return -EINVAL;
	}

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null");
		return -EINVAL;
	}

	status = wmi_extract_twt_disable_comp_event(wmi_handle, data, &event);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("TWT disable extract event failed(status=%d)",
				status);
		goto end;
	}

	status = rx_ops->twt_disable_comp_cb(psoc, &event);

end:
	return qdf_status_to_os_return(status);
}


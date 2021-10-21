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
 * DOC: Implement API's specific to 11be.
 */

#include "wmi_unified_11be_api.h"

/**
 * wmi_extract_mlo_link_set_active_resp() - extract mlo link set active resp
 *  from event
 * @wmi: WMI handle for this pdev
 * @evt_buf: pointer to event buffer
 * @resp: Pointer to hold mlo link set active resp
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS
wmi_extract_mlo_link_set_active_resp(wmi_unified_t wmi,
				     void *evt_buf,
				     struct wmi_mlo_link_set_active_resp *resp)
{
	if (wmi->ops->extract_mlo_link_set_active_resp) {
		return wmi->ops->extract_mlo_link_set_active_resp(wmi,
								  evt_buf,
								  resp);
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * wmi_send_mlo_link_set_active_cmd() - send mlo link set active command
 * @wmi: WMI handle for this pdev
 * @param: Pointer to mlo link set active param
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS
wmi_send_mlo_link_set_active_cmd(wmi_unified_t wmi,
				 struct wmi_mlo_link_set_active_param *param)
{
	if (wmi->ops->send_mlo_link_set_active_cmd)
		return wmi->ops->send_mlo_link_set_active_cmd(wmi, param);

	return QDF_STATUS_E_FAILURE;
}

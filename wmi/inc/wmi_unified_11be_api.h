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
/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) specific to 11be.
 */

#ifndef _WMI_UNIFIED_11BE_API_H_
#define _WMI_UNIFIED_11BE_API_H_

#include <wmi_unified_api.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_11be_param.h>

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * wmi_extract_mlo_link_set_active_resp() - extract mlo link set active
 *  response event
 * @wmi: wmi handle
 * @evt_buf: pointer to event buffer
 * @evt: Pointer to hold mlo link set active response event
 *
 * This function gets called to extract mlo link set active response event
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_mlo_link_set_active_resp(wmi_unified_t wmi,
				     void *evt_buf,
				     struct wmi_mlo_link_set_active_resp *evt);

/**
 * wmi_send_mlo_link_set_active_cmd() - send mlo link set active command
 * @wmi: WMI handle for this pdev
 * @param: Pointer to mlo link set active param
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS
wmi_send_mlo_link_set_active_cmd(wmi_unified_t wmi_handle,
				 struct wmi_mlo_link_set_active_param *param);
#endif /*WLAN_FEATURE_11BE_MLO*/
#endif /*_WMI_UNIFIED_11BE_API_H_*/

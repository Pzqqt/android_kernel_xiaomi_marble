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

#ifndef _WMI_UNIFIED_11BE_API_H_
#define _WMI_UNIFIED_11BE_API_H_

#include <wmi_unified_11be_param.h>

#ifdef WLAN_FEATURE_11BE_MLO
/**
 *  wmi_mlo_setup_cmd_send() - Send MLO setup command
 *  @wmi_handle: WMI handle for this pdev
 *  @params: MLO setup params
 *
 *  Return: QDF_STATUS code
 */
QDF_STATUS wmi_mlo_setup_cmd_send(wmi_unified_t wmi_handle,
				  struct wmi_mlo_setup_params *params);

/**
 *  wmi_mlo_teardown_cmd_send() - Send MLO teardown command
 *  @wmi_handle: WMI handle for this pdev
 *  @params: MLO teardown params
 *
 *  Return: QDF_STATUS code
 */
QDF_STATUS wmi_mlo_teardown_cmd_send(wmi_unified_t wmi_handle,
				     struct wmi_mlo_teardown_params *params);

/**
 *  wmi_mlo_ready_cmd_send() - Send MLO ready command
 *  @wmi_handle: WMI handle for this pdev
 *  @params: MLO ready params
 *
 *  Return: QDF_STATUS code
 */
QDF_STATUS wmi_mlo_ready_cmd_send(wmi_unified_t wmi_handle,
				  struct wmi_mlo_ready_params *params);

/**
 *  wmi_extract_mlo_setup_cmpl_event() - Extract MLO setup completion event
 *  @wmi_handle: WMI handle for this pdev
 *  @buf: Event buffer
 *  @params: MLO setup event params
 *
 *  Return: QDF_STATUS code
 */
QDF_STATUS
wmi_extract_mlo_setup_cmpl_event(wmi_unified_t wmi_handle,
				 uint8_t *buf,
				 struct wmi_mlo_setup_complete_params *params);

/**
 *  wmi_extract_mlo_teardown_cmpl_event() - Extract MLO teardown completion
 *  @wmi_handle: WMI handle for this pdev
 *  @buf: Event buffer
 *  @params: MLO teardown event params
 *
 *  Return: QDF_STATUS code
 */
QDF_STATUS
wmi_extract_mlo_teardown_cmpl_event(wmi_unified_t wmi_handle,
				    uint8_t *buf,
				    struct wmi_mlo_teardown_cmpl_params *param);
#endif /*WLAN_FEATURE_11BE_MLO*/
#endif /*_WMI_UNIFIED_11BE_API_H_*/

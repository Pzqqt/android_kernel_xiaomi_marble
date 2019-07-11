/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * This file contains the API definitions for ATF
 */

#ifndef _WMI_UNIFIED_ATF_API_H_
#define _WMI_UNIFIED_ATF_API_H_

#include "wmi_unified_atf_param.h"

/**
 *  wmi_unified_set_bwf_cmd_send() - WMI set bwf function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to set bwf param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_bwf_cmd_send(wmi_unified_t wmi_handle,
					struct set_bwf_params *param);

#ifdef WLAN_ATF_ENABLE
/**
 *  wmi_unified_set_atf_cmd_send() - WMI set atf function
 *  @wmi_handle: handle to WMI.
 *  @param: pointer to set atf param
 *
 *  Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_unified_set_atf_cmd_send(wmi_unified_t wmi_handle,
			     struct set_atf_params *param);

/**
 * wmi_send_atf_peer_request_cmd() - send atf peer request command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to atf peer request param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_atf_peer_request_cmd(wmi_unified_t wmi_handle,
			      struct atf_peer_request_params *param);

/**
 * wmi_send_set_atf_grouping_cmd() - send set atf grouping command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf grouping param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_set_atf_grouping_cmd(wmi_unified_t wmi_handle,
			      struct atf_grouping_params *param);

/**
 * wmi_send_set_atf_group_ac_cmd() - send set atf AC command to fw
 * @wmi_handle: wmi handle
 * @param: pointer to set atf AC group param
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_send_set_atf_group_ac_cmd(wmi_unified_t wmi_handle,
			      struct atf_group_ac_params *param);

/**
 * wmi_extract_atf_peer_stats_ev() - extract atf peer stats
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @ev: Pointer to hold atf peer stats
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_atf_peer_stats_ev(wmi_unified_t wmi_handle, void *evt_buf,
			      wmi_host_atf_peer_stats_event *ev);

/**
 * wmi_extract_atf_token_info_ev() - extract atf token info
 * from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @idx: Index indicating the peer number
 * @atf_token_info: Pointer to hold atf token info
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS
wmi_extract_atf_token_info_ev(wmi_unified_t wmi_handle,
			      void *evt_buf, uint8_t idx,
			      wmi_host_atf_peer_stats_info *atf_token_info);
#endif

void wmi_atf_attach_tlv(struct wmi_unified *wmi_handle);
#endif /* _WMI_UNIFIED_ATF_API_H_ */

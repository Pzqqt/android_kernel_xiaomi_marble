/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI) which are specific to Regulatory module.
 */

#ifndef _WMI_UNIFIED_REG_API_H_
#define _WMI_UNIFIED_REG_API_H_

#include "reg_services_public_struct.h"
/**
 * wmi_extract_reg_chan_list_update_event() - function to update channel list
 * @wmi_handle: wmi handle
 * @event_buf: event buffer
 * @reg_info: regulatory info
 * @len: length of buffer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_reg_chan_list_update_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct cur_regulatory_info *reg_info,
		uint32_t len);

#ifdef CONFIG_BAND_6GHZ
/**
 * wmi_extract_reg_chan_list_ext_update_event() - function to update the
 * extended channel list
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @reg_info: regulatory info
 * @len: length of buffer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_extract_reg_chan_list_ext_update_event(wmi_unified_t wmi_handle,
					uint8_t *evt_buf,
					struct cur_regulatory_info *reg_info,
					uint32_t len);
#endif

/*
 * wmi_unified_send_stop_11d_scan_cmd() - stop 11d scan
 * @wmi_handle: wmi handle
 * @stop_11d_scan: pointer to 11d scan stop req.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_stop_11d_scan_cmd(wmi_unified_t wmi_handle,
		struct reg_stop_11d_scan_req *stop_11d_scan);

/*
 * wmi_unified_send_start_11d_scan_cmd() - start 11d scan
 * @wmi_handle: wmi handle
 * @start_11d_scan: pointer to 11d scan start req.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wmi_unified_send_start_11d_scan_cmd(wmi_unified_t wmi_handle,
		struct reg_start_11d_scan_req *start_11d_scan);

/**
 * wmi_extract_reg_11d_new_cc_event() - function to extract the 11d new country
 * @wmi_handle: wmi handle
 * @evt_buf: event buffer
 * @reg_11d_new_cc: pointer to new 11d country info
 * @len: length of buffer
 *
 * Return: 0 for success or error code
 */
QDF_STATUS wmi_extract_reg_11d_new_cc_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct reg_11d_new_country *reg_11d_new_cc,
		uint32_t len);

/**
 * wmi_unified_set_user_country_code_cmd_send() - WMI set country function
 * @wmi_handle: wmi handle.
 * @pdev_id: Pdev id
 * @rd: User country code or regdomain
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_user_country_code_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t pdev_id, struct cc_regdmn_s *rd);

/**
 * wmi_extract_reg_ch_avoid_event() - process freq avoid event
 * @wmi_handle: wmi handle.
 * @evt_buf: event buffer
 * @ch_avoid_ind: buffer pointer to save the event processed data
 * @len: length of buffer
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_extract_reg_ch_avoid_event(
		wmi_unified_t wmi_handle,
		uint8_t *evt_buf,
		struct ch_avoid_ind_type *ch_avoid_ind,
		uint32_t len);

#endif /* _WMI_UNIFIED_REG_API_H_ */

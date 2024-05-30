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
 * DOC: Implement API's specific to CoAP component.
 */

#ifndef _WMI_UNIFIED_COAP_API_H_
#define _WMI_UNIFIED_COAP_API_H_

#include <wmi_unified_priv.h>

#ifdef WLAN_FEATURE_COAP
/**
 * wmi_unified_coap_add_pattern_cmd() - Add pattern for CoAP offload reply
 * @wmi_handle: wmi handle
 * @param: parameters for CoAP offload reply
 *
 * Return: status of operation
 */
static inline QDF_STATUS
wmi_unified_coap_add_pattern_cmd(wmi_unified_t wmi_handle,
				 struct coap_offload_reply_param *param)
{
	if (wmi_handle->ops->send_coap_add_pattern_cmd)
		return wmi_handle->ops->send_coap_add_pattern_cmd(wmi_handle,
								  param);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * wmi_unified_coap_del_pattern_cmd() - Delete pattern for CoAP offload reply
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: status of operation
 */
static inline QDF_STATUS
wmi_unified_coap_del_pattern_cmd(wmi_unified_t wmi_handle,
				 uint8_t vdev_id, uint32_t pattern_id)
{
	if (wmi_handle->ops->send_coap_del_pattern_cmd)
		return wmi_handle->ops->send_coap_del_pattern_cmd(wmi_handle,
		vdev_id, pattern_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * wmi_unified_coap_add_keepalive_pattern_cmd() - Add pattern for CoAP offload
 * periodic transmitting
 * @wmi_handle: wmi handle
 * @param: parameters for CoAP offload periodic transmitting
 *
 * Return: status of operation
 */
static inline QDF_STATUS
wmi_unified_coap_add_keepalive_pattern_cmd(wmi_unified_t wmi_handle,
			struct coap_offload_periodic_tx_param *param)
{
	if (wmi_handle->ops->send_coap_add_keepalive_pattern_cmd)
		return wmi_handle->ops->send_coap_add_keepalive_pattern_cmd(
					wmi_handle, param);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * wmi_unified_coap_del_pattern_cmd() - Delete pattern for CoAP offload
 * periodic transmitting
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: status of operation
 */
static inline QDF_STATUS
wmi_unified_coap_del_keepalive_pattern_cmd(wmi_unified_t wmi_handle,
					   uint8_t vdev_id,
					   uint32_t pattern_id)
{
	if (wmi_handle->ops->send_coap_del_keepalive_pattern_cmd)
		return wmi_handle->ops->send_coap_del_keepalive_pattern_cmd(
					wmi_handle, vdev_id, pattern_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * wmi_unified_coap_del_pattern_cmd() - Get cached CoAP messages
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 *
 * Return: status of operation
 */
static inline QDF_STATUS
wmi_unified_coap_cache_get(wmi_unified_t wmi_handle,
			   uint8_t vdev_id, uint32_t pattern_id)
{
	if (wmi_handle->ops->send_coap_cache_get_cmd)
		return wmi_handle->ops->send_coap_cache_get_cmd(wmi_handle,
		vdev_id, pattern_id);

	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * wmi_unified_coap_extract_buf_info() - extract CoAP buf info from event
 * @wmi_handle: wmi handle
 * @evt_buf: pointer to event buffer
 * @info: Pointer to hold CoAP buf info
 *
 * The caller needs to free any possible nodes in info->info_list
 * regardless of failure or success.
 *
 * Return: status of operation
 */
static inline QDF_STATUS
wmi_unified_coap_extract_buf_info(wmi_unified_t wmi_handle, void *evt_buf,
				  struct coap_buf_info *info)
{
	if (wmi_handle->ops->extract_coap_buf_info) {
		return wmi_handle->ops->extract_coap_buf_info(wmi_handle,
							      evt_buf,
							      info);
	}

	return QDF_STATUS_E_NOSUPPORT;
}
#endif
#endif /* _WMI_UNIFIED_ROAM_API_H_ */

/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_APF_TLV_H_
#define _WMI_UNIFIED_APF_TLV_H_

#ifdef FEATURE_WLAN_APF

#include "wmi_unified.h"
#include "wmi_unified_api.h"
#include "wmi_unified_param.h"

/**
 * wmi_send_set_active_apf_mode_cmd_tlv() - configure active APF mode in FW
 * @wmi_handle: the WMI handle
 * @vdev_id: the Id of the vdev to apply the configuration to
 * @ucast_mode: the active APF mode to configure for unicast packets
 * @mcast_bcast_mode: the active APF mode to configure for multicast/broadcast
 * packets
 *
 * Return: QDF status
 */
QDF_STATUS
wmi_send_set_active_apf_mode_cmd_tlv(wmi_unified_t wmi_handle,
				     uint8_t vdev_id,
				     enum wmi_host_active_apf_mode ucast_mode,
				     enum wmi_host_active_apf_mode
							      mcast_bcast_mode);

/**
 * wmi_send_apf_enable_cmd_tlv() - send cmd to enable/disable APF interpreter
 * @wmi_handle: the WMI handle
 * @vdev_id: VDEV on which APF interpreter is to be enabled/disabled
 * @enable: true: enable, false: disable
 *
 * Return: QDF status
 */
QDF_STATUS
wmi_send_apf_enable_cmd_tlv(wmi_unified_t wmi_handle, uint32_t vdev_id,
			    bool enable);

/**
 * wmi_send_apf_write_work_memory_cmd_tlv() - send cmd to write into the APF
 * work
 * memory
 * @wmi_handle: the WMI handle
 * @apf_write_params: parameters and buffer pointer for the write
 *
 * Return: QDF status
 */
QDF_STATUS
wmi_send_apf_write_work_memory_cmd_tlv(wmi_unified_t wmi_handle,
				       struct wmi_apf_write_memory_params
							     *apf_write_params);

/**
 * wmi_send_apf_read_work_memory_cmd_tlv() - send cmd to read part of APF
 * work memory
 * @wmi_handle: the WMI handle
 * @apf_read_params: contains relative address and length to read from
 *
 * Return: QDF status
 */
QDF_STATUS
wmi_send_apf_read_work_memory_cmd_tlv(wmi_unified_t wmi_handle,
				      struct wmi_apf_read_memory_params
							      *apf_read_params);

/**
 * wmi_extract_apf_read_memory_resp_event_tlv() - extract read memory response
 * event into the given structure pointer
 * @wmi_handle: the WMI handle
 * @evt_buf: Pointer to the event buffer
 * @resp: pointer to memory to extract event parameters into
 *
 * Return: QDF status
 */
QDF_STATUS
wmi_extract_apf_read_memory_resp_event_tlv(wmi_unified_t wmi_handle,
				void *evt_buf,
				struct wmi_apf_read_memory_resp_event_params
									 *resp);
#endif /* FEATURE_WLAN_APF */

#endif /* _WMI_UNIFIED_APF_TLV_H_ */

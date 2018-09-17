/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to concurrency component.
 */

#ifndef _WMI_UNIFIED_CONCURRENCY_API_H_
#define _WMI_UNIFIED_CONCURRENCY_API_H_

/**
 * wmi_unified_set_mcc_channel_time_quota_cmd() - set MCC channel time quota
 * @wmi: wmi handle
 * @adapter_1_chan_number: adapter 1 channel number
 * @adapter_1_quota: adapter 1 quota
 * @adapter_2_chan_number: adapter 2 channel number
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_mcc_channel_time_quota_cmd
	(void *wmi_hdl,
	uint32_t adapter_1_chan_freq,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq);

/**
 * wmi_unified_set_mcc_channel_time_latency_cmd() - set MCC channel time latency
 * @wmi: wmi handle
 * @mcc_channel: mcc channel
 * @mcc_channel_time_latency: MCC channel time latency.
 *
 * Currently used to set time latency for an MCC vdev/adapter using operating
 * channel of it and channel number. The info is provided run time using
 * iwpriv command: iwpriv <wlan0 | p2p0> setMccLatency <latency in ms>.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_set_mcc_channel_time_latency_cmd
	(void *wmi_hdl,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency);

/**
 * wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd() - control mcc
 *								 scheduler
 * @wmi_handle: wmi handle
 * @mcc_adaptive_scheduler: enable/disable
 *
 * This function enable/disable mcc adaptive scheduler in fw.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd(
		   void *wmi_hdl, uint32_t mcc_adaptive_scheduler,
		   uint32_t pdev_id);

#endif /* _WMI_UNIFIED_CONCURRENCY_API_H_ */

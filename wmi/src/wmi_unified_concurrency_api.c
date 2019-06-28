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
/**
 * DOC: Implement API's specific to concurrency component.
 */

#include <wmi_unified_priv.h>
#include <wmi_unified_concurrency_api.h>

QDF_STATUS wmi_unified_set_enable_disable_mcc_adaptive_scheduler_cmd(
		wmi_unified_t wmi_handle, uint32_t mcc_adaptive_scheduler,
		uint32_t pdev_id)
{
	if (wmi_handle->ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd)
		return wmi_handle->ops->send_set_enable_disable_mcc_adaptive_scheduler_cmd(wmi_handle,
					mcc_adaptive_scheduler, pdev_id);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_mcc_channel_time_latency_cmd(
	wmi_unified_t wmi_handle,
	uint32_t mcc_channel_freq, uint32_t mcc_channel_time_latency)
{
	if (wmi_handle->ops->send_set_mcc_channel_time_latency_cmd)
		return wmi_handle->ops->send_set_mcc_channel_time_latency_cmd(wmi_handle,
					mcc_channel_freq,
					mcc_channel_time_latency);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_set_mcc_channel_time_quota_cmd(
		wmi_unified_t wmi_handle,
		uint32_t adapter_1_chan_freq,
		uint32_t adapter_1_quota, uint32_t adapter_2_chan_freq)
{
	if (wmi_handle->ops->send_set_mcc_channel_time_quota_cmd)
		return wmi_handle->ops->send_set_mcc_channel_time_quota_cmd(wmi_handle,
						adapter_1_chan_freq,
						adapter_1_quota,
						adapter_2_chan_freq);

	return QDF_STATUS_E_FAILURE;
}


/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "wmi_unified_priv.h"
#include "wmi_unified_param.h"
#include "qdf_module.h"

#ifdef WMI_SMART_ANT_SUPPORT
QDF_STATUS wmi_unified_set_ant_switch_tbl_cmd_send(
		wmi_unified_t wmi_handle,
		struct ant_switch_tbl_params *param)
{
	if (wmi_handle->ops->send_set_ant_switch_tbl_cmd)
		return wmi_handle->ops->send_set_ant_switch_tbl_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_smart_ant_enable_cmd_send(
		wmi_unified_t wmi_handle,
		struct smart_ant_enable_params *param)
{
	if (wmi_handle->ops->send_smart_ant_enable_cmd)
		return wmi_handle->ops->send_smart_ant_enable_cmd(wmi_handle,
				param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_smart_ant_set_rx_ant_cmd_send(
		wmi_unified_t wmi_handle,
		struct smart_ant_rx_ant_params *param)
{
	if (wmi_handle->ops->send_smart_ant_set_rx_ant_cmd)
		return wmi_handle->ops->send_smart_ant_set_rx_ant_cmd(
					wmi_handle, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_smart_ant_set_tx_ant_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_tx_ant_params *param)
{
	if (wmi_handle->ops->send_smart_ant_set_tx_ant_cmd)
		return wmi_handle->ops->send_smart_ant_set_tx_ant_cmd(
					wmi_handle, macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_smart_ant_set_training_info_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_training_info_params *param)
{
	if (wmi_handle->ops->send_smart_ant_set_training_info_cmd)
		return wmi_handle->ops->send_smart_ant_set_training_info_cmd(
					wmi_handle, macaddr, param);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wmi_unified_smart_ant_node_config_cmd_send(
		wmi_unified_t wmi_handle,
		uint8_t macaddr[QDF_MAC_ADDR_SIZE],
		struct smart_ant_node_config_params *param)
{
	if (wmi_handle->ops->send_smart_ant_set_node_config_cmd)
		return wmi_handle->ops->send_smart_ant_set_node_config_cmd(
					wmi_handle, macaddr, param);

	return QDF_STATUS_E_FAILURE;
}
#endif

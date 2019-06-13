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

#include <osdep.h>
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_smart_ant_param.h"
#include "wmi_unified_smart_ant_api.h"

/**
 *  send_smart_ant_enable_cmd_tlv() - WMI smart ant enable function
 *
 *  @param wmi_handle  : handle to WMI.
 *  @param param       : pointer to antenna param
 *
 *  This function sends smart antenna enable command to FW
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct smart_ant_enable_params *param)
{
	/* Send WMI COMMAND to Enable */
	wmi_pdev_smart_ant_enable_cmd_fixed_param *cmd;
	wmi_pdev_smart_ant_gpio_handle *gpio_param;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int len = 0;
	QDF_STATUS ret;
	int loop = 0;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += WMI_HAL_MAX_SANTENNA * sizeof(wmi_pdev_smart_ant_gpio_handle);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_pdev_smart_ant_enable_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_smart_ant_enable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				wmi_pdev_smart_ant_enable_cmd_fixed_param));

	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
	cmd->enable = param->enable;
	cmd->mode = param->mode;
	cmd->rx_antenna = param->rx_antenna;
	cmd->tx_default_antenna = param->rx_antenna;

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(wmi_pdev_smart_ant_enable_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       WMI_HAL_MAX_SANTENNA *
		       sizeof(wmi_pdev_smart_ant_gpio_handle));

	buf_ptr += WMI_TLV_HDR_SIZE;
	gpio_param = (wmi_pdev_smart_ant_gpio_handle *)buf_ptr;

	for (loop = 0; loop < WMI_HAL_MAX_SANTENNA; loop++) {
		WMITLV_SET_HDR(&gpio_param->tlv_header,
			       WMITLV_TAG_STRUC_wmi_pdev_smart_ant_gpio_handle,
			       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_pdev_smart_ant_gpio_handle));
		if (param->mode == SMART_ANT_MODE_SERIAL) {
			if (loop < WMI_HOST_MAX_SERIAL_ANTENNA) {
				gpio_param->gpio_pin = param->gpio_pin[loop];
				gpio_param->gpio_func = param->gpio_func[loop];
			} else {
				gpio_param->gpio_pin = 0;
				gpio_param->gpio_func = 0;
			}
		} else if (param->mode == SMART_ANT_MODE_PARALLEL) {
			gpio_param->gpio_pin = param->gpio_pin[loop];
			gpio_param->gpio_func = param->gpio_func[loop];
		}
		/* Setting it to 0 for now */
		gpio_param->pdev_id =
			wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);
		gpio_param++;
	}

	wmi_mtrace(WMI_PDEV_SMART_ANT_ENABLE_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				buf,
				len,
				WMI_PDEV_SMART_ANT_ENABLE_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		WMI_LOGE("enable:%d mode:%d  rx_antenna: 0x%08x PINS: [%d %d %d %d] Func[%d %d %d %d] cmdstatus=%d\n",
			 cmd->enable,
			 cmd->mode,
			 cmd->rx_antenna,
			 param->gpio_pin[0], param->gpio_pin[1],
			 param->gpio_pin[2], param->gpio_pin[3],
			 param->gpio_func[0], param->gpio_func[1],
			 param->gpio_func[2], param->gpio_func[3],
			 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_smart_ant_set_rx_ant_cmd_tlv() - WMI set rx antenna function
 *
 *  @param wmi_handle     : handle to WMI.
 *  @param param	  : pointer to rx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_rx_ant_cmd_tlv(wmi_unified_t wmi_handle,
				struct smart_ant_rx_ant_params *param)
{
	wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	uint32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	WMI_LOGD("%s:\n", __func__);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = wmi_buf_data(buf);
	cmd = (wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param *)buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	    WMITLV_TAG_STRUC_wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param,
	    WMITLV_GET_STRUCT_TLVLEN(
		wmi_pdev_smart_ant_set_rx_antenna_cmd_fixed_param));
	cmd->rx_antenna = param->antenna;
	cmd->pdev_id = wmi_handle->ops->convert_pdev_id_host_to_target(
								param->pdev_id);

	wmi_mtrace(WMI_PDEV_SMART_ANT_SET_RX_ANTENNA_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				buf,
				len,
				WMI_PDEV_SMART_ANT_SET_RX_ANTENNA_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		WMI_LOGE("%s: rx_antenna: 0x%08x cmdstatus=%d\n",
			 __func__,
			 cmd->rx_antenna,
			 ret);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 *  send_smart_ant_set_tx_ant_cmd_tlv() - WMI set tx antenna function
 *  @param wmi_handle  : handle to WMI.
 *  @param macaddr     : vdev mac address
 *  @param param       : pointer to tx antenna param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_tx_ant_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct smart_ant_tx_ant_params *param)
{
	wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param *cmd;
	wmi_peer_smart_ant_set_tx_antenna_series *ant_tx_series;
	wmi_buf_t buf;
	int32_t len = 0;
	int i;
	uint8_t *buf_ptr;
	QDF_STATUS ret;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += (WMI_SMART_ANT_MAX_RATE_SERIES) *
		sizeof(wmi_peer_smart_ant_set_tx_antenna_series);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	    WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param,
	    WMITLV_GET_STRUCT_TLVLEN(
			wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);

	buf_ptr += sizeof(wmi_peer_smart_ant_set_tx_antenna_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_smart_ant_set_tx_antenna_series) *
		       WMI_SMART_ANT_MAX_RATE_SERIES);
	buf_ptr += WMI_TLV_HDR_SIZE;
	ant_tx_series = (wmi_peer_smart_ant_set_tx_antenna_series *)buf_ptr;

	for (i = 0; i < WMI_SMART_ANT_MAX_RATE_SERIES; i++) {
		WMITLV_SET_HDR(&ant_tx_series->tlv_header,
		    WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_tx_antenna_series,
		    WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_smart_ant_set_tx_antenna_series));
		ant_tx_series->antenna_series = param->antenna_array[i];
		ant_tx_series++;
	}

	wmi_mtrace(WMI_PEER_SMART_ANT_SET_TX_ANTENNA_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				   buf,
				   len,
				   WMI_PEER_SMART_ANT_SET_TX_ANTENNA_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_set_ant_switch_tbl_cmd_tlv() - send ant switch tbl cmd to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold ant switch tbl param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS
send_set_ant_switch_tbl_cmd_tlv(wmi_unified_t wmi_handle,
				struct ant_switch_tbl_params *param)
{
	uint8_t len;
	wmi_buf_t buf;
	wmi_pdev_set_ant_switch_tbl_cmd_fixed_param *cmd;
	wmi_pdev_set_ant_ctrl_chain *ctrl_chain;
	uint8_t *buf_ptr;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += sizeof(wmi_pdev_set_ant_ctrl_chain);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_pdev_set_ant_switch_tbl_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_pdev_set_ant_switch_tbl_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_pdev_set_ant_switch_tbl_cmd_fixed_param));

	cmd->antCtrlCommon1 = param->ant_ctrl_common1;
	cmd->antCtrlCommon2 = param->ant_ctrl_common2;
	cmd->mac_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(param->pdev_id);

	/* TLV indicating array of structures to follow */
	buf_ptr += sizeof(wmi_pdev_set_ant_switch_tbl_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_pdev_set_ant_ctrl_chain));
	buf_ptr += WMI_TLV_HDR_SIZE;
	ctrl_chain = (wmi_pdev_set_ant_ctrl_chain *)buf_ptr;

	ctrl_chain->pdev_id =
		wmi_handle->ops->convert_pdev_id_host_to_target(param->pdev_id);
	ctrl_chain->antCtrlChain = param->antCtrlChain;

	wmi_mtrace(WMI_PDEV_SET_ANTENNA_SWITCH_TABLE_CMDID, NO_SESSION, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_PDEV_SET_ANTENNA_SWITCH_TABLE_CMDID)) {
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 *  send_smart_ant_set_training_info_cmd_tlv() - WMI set smart antenna
 *  training information function
 *  @param wmi_handle  : handle to WMI.
 *  @macaddr	   : vdev mac address
 *  @param param       : pointer to tx antenna param
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_training_info_cmd_tlv(
				wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct smart_ant_training_info_params *param)
{
	wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param *cmd;
	wmi_peer_smart_ant_set_train_antenna_param *train_param;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len = 0;
	QDF_STATUS ret;
	int loop;

	len = sizeof(*cmd) + WMI_TLV_HDR_SIZE;
	len += (WMI_SMART_ANT_MAX_RATE_SERIES) *
		 sizeof(wmi_peer_smart_ant_set_train_antenna_param);
	buf = wmi_buf_alloc(wmi_handle, len);

	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	qdf_mem_zero(buf_ptr, len);
	cmd = (wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param *)buf_ptr;

	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param));

	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->num_pkts = param->numpkts;

	buf_ptr += sizeof(wmi_peer_smart_ant_set_train_antenna_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(wmi_peer_smart_ant_set_train_antenna_param) *
		       WMI_SMART_ANT_MAX_RATE_SERIES);

	buf_ptr += WMI_TLV_HDR_SIZE;
	train_param = (wmi_peer_smart_ant_set_train_antenna_param *)buf_ptr;

	for (loop = 0; loop < WMI_SMART_ANT_MAX_RATE_SERIES; loop++) {
		WMITLV_SET_HDR(&train_param->tlv_header,
		WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_train_antenna_param,
			    WMITLV_GET_STRUCT_TLVLEN(
				wmi_peer_smart_ant_set_train_antenna_param));
		train_param->train_rate_series = param->rate_array[loop];
		train_param->train_antenna_series = param->antenna_array[loop];
		train_param->rc_flags = 0;
		WMI_LOGI(FL("Series number:%d\n"), loop);
		WMI_LOGI(FL("Rate [0x%02x] Tx_Antenna [0x%08x]\n"),
			 train_param->train_rate_series,
			 train_param->train_antenna_series);
		train_param++;
	}

	wmi_mtrace(WMI_PEER_SMART_ANT_SET_TRAIN_INFO_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
				buf,
				len,
				WMI_PEER_SMART_ANT_SET_TRAIN_INFO_CMDID);

	if (ret != 0) {
		WMI_LOGE(" %s :WMI Failed\n", __func__);
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return ret;
}

/**
 *  send_smart_ant_set_node_config_cmd_tlv() - WMI set node
 *  configuration function
 *  @param wmi_handle		   : handle to WMI.
 *  @macaddr			   : vdev mad address
 *  @param param		   : pointer to tx antenna param
 *
 *  @return QDF_STATUS_SUCCESS  on success and -ve on failure.
 */
static QDF_STATUS send_smart_ant_set_node_config_cmd_tlv(
				wmi_unified_t wmi_handle,
				uint8_t macaddr[QDF_MAC_ADDR_SIZE],
				struct smart_ant_node_config_params *param)
{
	wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len = 0, args_tlv_len;
	int ret;
	int i = 0;
	uint32_t *node_config_args;

	args_tlv_len = WMI_TLV_HDR_SIZE + param->args_count * sizeof(uint32_t);
	len = sizeof(*cmd) + args_tlv_len;

	if (param->args_count == 0) {
		WMI_LOGE("%s: Can't send a command with %d arguments\n",
			  __func__, param->args_count);
		return QDF_STATUS_E_FAILURE;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		WMI_LOGE("%s:wmi_buf_alloc failed\n", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param *)
						wmi_buf_data(buf);
	buf_ptr = (uint8_t *)cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
		wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param));
	cmd->vdev_id = param->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(macaddr, &cmd->peer_macaddr);
	cmd->cmd_id = param->cmd_id;
	cmd->args_count = param->args_count;
	buf_ptr += sizeof(
		wmi_peer_smart_ant_set_node_config_ops_cmd_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
			(cmd->args_count * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE;
	node_config_args = (uint32_t *)buf_ptr;

	for (i = 0; i < param->args_count; i++) {
		node_config_args[i] = param->args_arr[i];
		WMI_LOGI("%d", param->args_arr[i]);
	}

	wmi_mtrace(WMI_PEER_SMART_ANT_SET_NODE_CONFIG_OPS_CMDID,
		   cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle,
			   buf,
			   len,
			   WMI_PEER_SMART_ANT_SET_NODE_CONFIG_OPS_CMDID);

	if (ret != 0) {
		WMI_LOGE("%s: WMI FAILED:Sent cmd_id: 0x%x\n Node: %02x:%02x:%02x:%02x:%02x:%02x cmdstatus=%d\n",
			 __func__, param->cmd_id, macaddr[0],
			 macaddr[1], macaddr[2], macaddr[3],
			 macaddr[4], macaddr[5], ret);
		wmi_buf_free(buf);
	}

	return ret;
}

void wmi_smart_ant_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_smart_ant_enable_cmd = send_smart_ant_enable_cmd_tlv;
	ops->send_smart_ant_set_rx_ant_cmd = send_smart_ant_set_rx_ant_cmd_tlv;
	ops->send_smart_ant_set_tx_ant_cmd = send_smart_ant_set_tx_ant_cmd_tlv;
	ops->send_smart_ant_set_training_info_cmd =
				send_smart_ant_set_training_info_cmd_tlv;
	ops->send_smart_ant_set_node_config_cmd =
				send_smart_ant_set_node_config_cmd_tlv;
	ops->send_set_ant_switch_tbl_cmd = send_set_ant_switch_tbl_cmd_tlv;
}

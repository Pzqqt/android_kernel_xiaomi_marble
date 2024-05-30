/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include <wmi.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_gpio_api.h>

/**
 * convert_gpio_dir() - Function to convert unified gpio direction
 * @dir: pointer to enum gpio_direction
 *
 * Convert the wmi unified gpio direction to FW TLV WMI gpio direction
 *
 * Return:
 * 0 - Output
 * 1 - Input
 */
static uint32_t
convert_gpio_direction(enum gpio_direction dir)
{
	switch (dir) {
	case WMI_HOST_GPIO_INPUT:
		return WMI_FW_GPIO_INPUT;
	case WMI_HOST_GPIO_OUTPUT:
		return WMI_FW_GPIO_OUTPUT;
	default:
		return WMI_FW_GPIO_OUTPUT;
	}
}

/**
 * convert_gpio_pull_type() - Function to convert unified pull type
 * @pull_type: pointer to enum gpio_pull_type
 *
 * Convert the wmi unified pull type to FW TLV WMI gpio pull type
 *
 * Return: FW TLV WMI gpio pull type
 */
static uint32_t
convert_gpio_pull_type(enum gpio_pull_type pull_type)
{
	switch (pull_type) {
	case WMI_HOST_GPIO_PULL_NONE:
		return WMI_GPIO_PULL_NONE;
	case WMI_HOST_GPIO_PULL_UP:
		return WMI_GPIO_PULL_UP;
	case WMI_HOST_GPIO_PULL_DOWN:
		return WMI_GPIO_PULL_DOWN;
	default:
		return WMI_GPIO_PULL_NONE;
	}
}

/**
 * convert_gpio_interrupt_mode() - Function to convert unified interrupt mode
 * @intr_mode: pointer to enum gpio_interrupt_mode
 *
 * Convert the wmi unified interrupt mode to FW TLV WMI gpio interrupt mode
 *
 * Return: FW TLV WMI gpio interrupt mode
 */
static uint32_t
convert_gpio_interrupt_mode(enum gpio_interrupt_mode intr_mode)
{
	switch (intr_mode) {
	case WMI_HOST_GPIO_INTMODE_DISABLE:
		return WMI_GPIO_INTTYPE_DISABLE;
	case WMI_HOST_GPIO_INTMODE_RISING_EDGE:
		return WMI_GPIO_INTTYPE_RISING_EDGE;
	case WMI_HOST_GPIO_INTMODE_FALLING_EDGE:
		return WMI_GPIO_INTTYPE_FALLING_EDGE;
	case WMI_HOST_GPIO_INTMODE_BOTH_EDGE:
		return WMI_GPIO_INTTYPE_BOTH_EDGE;
	case WMI_HOST_GPIO_INTMODE_LEVEL_LOW:
		return WMI_GPIO_INTTYPE_LEVEL_LOW;
	case WMI_HOST_GPIO_INTMODE_LEVEL_HIGH:
		return WMI_GPIO_INTTYPE_LEVEL_HIGH;
	default:
		return WMI_GPIO_INTTYPE_DISABLE;
	}
}

/**
 * convert_gpio_output_value() - Function to convert unified gpio output value
 * @value: pointer to enum gpio_value
 *
 * Convert the wmi unified gpio output value to FW TLV WMI gpio output value
 *
 * Return:
 * 0 - Output low level
 * 1 - Output high level
 */
static uint32_t
convert_gpio_output_value(enum gpio_value value)
{
	switch (value) {
	case WMI_HOST_GPIO_LEVEL_LOW:
		return 0;
	case WMI_HOST_GPIO_LEVEL_HIGH:
		return 1;
	default:
		return 0;
	}
}

/**
 * send_gpio_config_cmd_tlv() - send gpio config to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold gpio config params
 *
 * Send gpio configuration to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_gpio_config_cmd_tlv(wmi_unified_t wmi_handle,
			 struct gpio_config_params *param)
{
	wmi_gpio_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	/* Sanity Checks */
	if (param->pin_pull_type >= WMI_HOST_GPIO_PULL_MAX ||
	    param->pin_intr_mode >= WMI_HOST_GPIO_INTMODE_MAX ||
	    param->pin_dir >= WMI_HOST_GPIO_DIR_MAX) {
		return QDF_STATUS_E_FAILURE;
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_gpio_config_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_gpio_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_gpio_config_cmd_fixed_param));
	cmd->gpio_num = param->pin_num;
	cmd->input = convert_gpio_direction(param->pin_dir);
	cmd->pull_type = convert_gpio_pull_type(param->pin_pull_type);
	cmd->intr_mode = convert_gpio_interrupt_mode(param->pin_intr_mode);
	cmd->mux_config_val = param->mux_config_val;
	cmd->drive = param->drive;
	cmd->init_enable = param->init_enable;

	wmi_debug("GPIO num %d, input-dir %d, pull_type %d, intr_mode %d"
		 " mux_config_val %d drive %d init_enable %d",
		 cmd->gpio_num, cmd->input, cmd->pull_type, cmd->intr_mode,
		 cmd->mux_config_val, cmd->drive, cmd->init_enable);

	wmi_mtrace(WMI_GPIO_CONFIG_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				   WMI_GPIO_CONFIG_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Sending GPIO config cmd failed");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_gpio_output_cmd_tlv() - send gpio output to fw
 * @wmi_handle: wmi handle
 * @param: pointer to hold gpio output param
 *
 * Send gpio output value to firmware.
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
static QDF_STATUS
send_gpio_output_cmd_tlv(wmi_unified_t wmi_handle,
			 struct gpio_output_params *param)
{
	wmi_gpio_output_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS ret;

	len = sizeof(*cmd);

	/* Sanity Checks */
	if (param->pin_set >= WMI_HOST_GPIO_LEVEL_MAX)
		return QDF_STATUS_E_FAILURE;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf)
		return QDF_STATUS_E_FAILURE;

	cmd = (wmi_gpio_output_cmd_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_gpio_output_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_gpio_output_cmd_fixed_param));
	cmd->gpio_num = param->pin_num;
	cmd->set = convert_gpio_output_value(param->pin_set);

	wmi_debug("GPIO num %d, set %d", cmd->gpio_num, cmd->set);
	wmi_mtrace(WMI_GPIO_OUTPUT_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, sizeof(*cmd),
				   WMI_GPIO_OUTPUT_CMDID);

	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Sending GPIO output cmd failed");
		wmi_buf_free(buf);
	}

	return ret;
}

void wmi_gpio_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_gpio_config_cmd = send_gpio_config_cmd_tlv;
	ops->send_gpio_output_cmd = send_gpio_output_cmd_tlv;
}


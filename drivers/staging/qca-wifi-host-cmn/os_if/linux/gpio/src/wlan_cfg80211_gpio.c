/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: defines driver functions interfacing with linux kernel
 */

#include <qdf_list.h>
#include <qdf_status.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>
#include <wlan_cfg80211.h>
#include <wlan_osif_priv.h>
#include <wlan_gpio_ucfg_api.h>
#include <wlan_cfg80211_gpio.h>
#include "qdf_module.h"

const struct nla_policy
wlan_cfg80211_gpio_config_policy[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PINNUM] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_VALUE] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PULL_TYPE] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTR_MODE] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_DIR] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MUX_CONFIG] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_DRIVE] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
	[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTERNAL_CONFIG] = {
						.type = NLA_U32,
						.len = sizeof(uint32_t) },
};

/**
 * convert_vendor_gpio_direction() - Function to convert vendor gpio direction
 * @dir: pointer to enum qca_gpio_direction
 *
 * Convert the vendor gpio direction to wmi unified gpio direction
 *
 * Return: wmi unified gpio direction
 */
static enum gpio_direction
convert_vendor_gpio_direction(enum qca_gpio_direction dir)
{
	switch (dir) {
	case QCA_WLAN_GPIO_INPUT:
		return WMI_HOST_GPIO_INPUT;
	case QCA_WLAN_GPIO_OUTPUT:
		return WMI_HOST_GPIO_OUTPUT;
	default:
		return WMI_HOST_GPIO_INPUT;
	}
}

/**
 * convert_vendor_gpio_pull_type() - Function to convert vendor pull type
 * @pull_type: pointer to enum qca_gpio_pull_type
 *
 * Convert the vendor pull type to wmi unified pull type
 *
 * Return: wmi unified gpio pull type
 */
static enum gpio_pull_type
convert_vendor_gpio_pull_type(enum qca_gpio_pull_type pull_type)
{
	switch (pull_type) {
	case QCA_WLAN_GPIO_PULL_NONE:
		return WMI_HOST_GPIO_PULL_NONE;
	case QCA_WLAN_GPIO_PULL_UP:
		return WMI_HOST_GPIO_PULL_UP;
	case QCA_WLAN_GPIO_PULL_DOWN:
		return WMI_HOST_GPIO_PULL_DOWN;
	default:
		return WMI_HOST_GPIO_PULL_NONE;
	}
}

/**
 * convert_vendor_gpio_interrupt_mode() - Function to convert
 * vendor interrupt mode
 * @intr_mode: pointer to enum qca_gpio_interrupt_mode
 *
 * Convert the vendor interrupt mode to wmi unified interrupt mode
 *
 * Return: wmi unified gpio interrupt mode
 */
static enum gpio_interrupt_mode
convert_vendor_gpio_interrupt_mode(enum qca_gpio_interrupt_mode intr_mode)
{
	switch (intr_mode) {
	case QCA_WLAN_GPIO_INTMODE_DISABLE:
		return WMI_HOST_GPIO_INTMODE_DISABLE;
	case QCA_WLAN_GPIO_INTMODE_RISING_EDGE:
		return WMI_HOST_GPIO_INTMODE_RISING_EDGE;
	case QCA_WLAN_GPIO_INTMODE_FALLING_EDGE:
		return WMI_HOST_GPIO_INTMODE_FALLING_EDGE;
	case QCA_WLAN_GPIO_INTMODE_BOTH_EDGE:
		return WMI_HOST_GPIO_INTMODE_BOTH_EDGE;
	case QCA_WLAN_GPIO_INTMODE_LEVEL_LOW:
		return WMI_HOST_GPIO_INTMODE_LEVEL_LOW;
	case QCA_WLAN_GPIO_INTMODE_LEVEL_HIGH:
		return WMI_HOST_GPIO_INTMODE_LEVEL_HIGH;
	default:
		return WMI_HOST_GPIO_INTMODE_DISABLE;
	}
}

/**
 * convert_vendor_gpio_output_value() - Function to convert vendor
 * gpio output value
 * @value: pointer to enum qca_gpio_value
 *
 * Convert the vendor gpio value to wmi unified gpio output value
 *
 * Return: wmi unified gpio output value
 */
static enum gpio_value
convert_vendor_gpio_output_value(enum qca_gpio_value value)
{
	switch (value) {
	case QCA_WLAN_GPIO_LEVEL_LOW:
		return WMI_HOST_GPIO_LEVEL_LOW;
	case QCA_WLAN_GPIO_LEVEL_HIGH:
		return WMI_HOST_GPIO_LEVEL_HIGH;
	default:
		return WMI_HOST_GPIO_LEVEL_LOW;
	}
}

/**
 * convert_vendor_gpio_drive() - Function to convert vendor
 * gpio drive
 * @drive: value of enum gpio_drive
 *
 * Convert the vendor gpio drive to wmi unified gpio output drive
 *
 * Return: wmi unified gpio output drive config
 */
static enum gpio_drive
convert_vendor_gpio_drive(enum qca_gpio_drive drive)
{
	switch (drive) {
	case QCA_WLAN_GPIO_DRIVE_2MA:
		return WMI_HOST_GPIO_DRIVE_2MA;
	case QCA_WLAN_GPIO_DRIVE_4MA:
		return WMI_HOST_GPIO_DRIVE_4MA;
	case QCA_WLAN_GPIO_DRIVE_6MA:
		return WMI_HOST_GPIO_DRIVE_6MA;
	case QCA_WLAN_GPIO_DRIVE_8MA:
		return WMI_HOST_GPIO_DRIVE_8MA;
	case QCA_WLAN_GPIO_DRIVE_10MA:
		return WMI_HOST_GPIO_DRIVE_10MA;
	case QCA_WLAN_GPIO_DRIVE_12MA:
		return WMI_HOST_GPIO_DRIVE_12MA;
	case QCA_WLAN_GPIO_DRIVE_14MA:
		return WMI_HOST_GPIO_DRIVE_14MA;
	case QCA_WLAN_GPIO_DRIVE_16MA:
		return WMI_HOST_GPIO_DRIVE_16MA;
	default:
		return WMI_HOST_GPIO_DRIVE_2MA;
	}
}

/**
 * convert_vendor_gpio_init_enable() - Function to convert vendor
 * gpio init_enable
 * @internal_config: Param to decide whether to use internal config
 *
 * Convert the vendor internal_config to wmi unified gpio output init_enable
 *
 * Return: wmi unified gpio output init_enable config
 */
static enum gpio_init_enable
convert_vendor_gpio_init_enable(uint32_t internal_config)
{
	if(internal_config)
		return WMI_HOST_GPIO_INIT_DISABLE;
	else
		return WMI_HOST_GPIO_INIT_ENABLE;
}

/**
 * wlan_set_gpio_config() - set the gpio configuration info
 * @psoc: the pointer of wlan_objmgr_psoc
 * @attr: list of attributes
 *
 * Return: 0 on success; errno on failure
 */
static int
wlan_set_gpio_config(struct wlan_objmgr_psoc *psoc,
		     struct nlattr **attr)
{
	struct gpio_config_params cfg_param;
	struct nlattr *gpio_attr;
	enum qca_gpio_direction pin_dir;
	enum qca_gpio_pull_type pull_type;
	enum qca_gpio_interrupt_mode intr_mode;
	enum qca_gpio_drive drive;
	uint32_t internal_config;
	QDF_STATUS status;

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PINNUM];
	if (!gpio_attr) {
		osif_err_rl("attr gpio number failed");
		return -EINVAL;
	}
	cfg_param.pin_num = nla_get_u32(gpio_attr);

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_DIR];
	if (!gpio_attr) {
		osif_err_rl("attr gpio dir failed");
		return -EINVAL;
	}
	pin_dir = nla_get_u32(gpio_attr);
	if (pin_dir >= QCA_WLAN_GPIO_DIR_MAX) {
		osif_err_rl("attr gpio direction invalid");
		return -EINVAL;
	}
	cfg_param.pin_dir = convert_vendor_gpio_direction(pin_dir);

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PULL_TYPE];
	if (!gpio_attr) {
		osif_err_rl("attr gpio pull failed");
		return -EINVAL;
	}
	pull_type = nla_get_u32(gpio_attr);
	if (pull_type >= QCA_WLAN_GPIO_PULL_MAX) {
		osif_err_rl("attr gpio pull type invalid");
		return -EINVAL;
	}
	cfg_param.pin_pull_type = convert_vendor_gpio_pull_type(pull_type);

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTR_MODE];
	if (!gpio_attr) {
		osif_err_rl("attr gpio interrupt mode failed");
		return -EINVAL;
	}
	intr_mode = nla_get_u32(gpio_attr);
	if (intr_mode >= QCA_WLAN_GPIO_INTMODE_MAX) {
		osif_err_rl("attr gpio interrupt mode invalid");
		return -EINVAL;
	}
	cfg_param.pin_intr_mode = convert_vendor_gpio_interrupt_mode(intr_mode);

	/* Below are optional parameters. Initialize to zero */
	cfg_param.mux_config_val = WMI_HOST_GPIO_MUX_DEFAULT;
	cfg_param.drive = WMI_HOST_GPIO_DRIVE_2MA;
	cfg_param.init_enable = WMI_HOST_GPIO_INIT_DISABLE;

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MUX_CONFIG];
	if (gpio_attr) {
		cfg_param.mux_config_val = nla_get_u32(gpio_attr);
	}

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_DRIVE];
	if (gpio_attr) {
		drive = nla_get_u32(gpio_attr);
		if (drive >= QCA_WLAN_GPIO_DRIVE_MAX) {
			osif_err_rl("attr gpio drive invalid");
			return -EINVAL;
		}
		cfg_param.drive = convert_vendor_gpio_drive(drive);
	}

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_INTERNAL_CONFIG];
	if (gpio_attr) {
		internal_config = nla_get_u32(gpio_attr);
		cfg_param.init_enable =
			convert_vendor_gpio_init_enable(internal_config);
	}

	status = ucfg_set_gpio_config(psoc, &cfg_param);
	return status;
}

/**
 * wlan_set_gpio_output() - set the gpio output info
 * @psoc: the pointer of wlan_objmgr_psoc
 * @attr: list of attributes
 *
 * Return: 0 on success; errno on failure
 */
static int
wlan_set_gpio_output(struct wlan_objmgr_psoc *psoc,
		     struct nlattr **attr)
{
	struct gpio_output_params out_param;
	struct nlattr *gpio_attr;
	enum qca_gpio_value pin_set;
	QDF_STATUS status;

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_PINNUM];
	if (!gpio_attr) {
		osif_err_rl("attr gpio number failed");
		return -EINVAL;
	}
	out_param.pin_num = nla_get_u32(gpio_attr);

	gpio_attr = attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_VALUE];
	if (!gpio_attr) {
		osif_err_rl("attr gpio value failed");
		return -EINVAL;
	}
	pin_set = nla_get_u32(gpio_attr);
	if (pin_set >= QCA_WLAN_GPIO_LEVEL_MAX) {
		osif_err_rl("attr gpio level invalid");
		return -EINVAL;
	}
	out_param.pin_set = convert_vendor_gpio_output_value(pin_set);

	status = ucfg_set_gpio_output(psoc, &out_param);
	return status;
}

/**
 * wlan_cfg80211_start_gpio_config - Set the gpio configuration
 * @wiphy: pointer to wiphy
 * @psoc: the pointer of wlan_objmgr_psoc
 * @data: pointer to data
 * @data_len: data length
 *
 * __wlan_cfg80211_set_gpio_config will forward the GPIO setting to FW by
 * WMI_GPIO_CONFIG/OUTPUT_CMDID
 *
 * Return: 0 on success; errno on failure
 */
int
wlan_cfg80211_start_gpio_config(struct wiphy *wiphy,
				struct wlan_objmgr_psoc *psoc,
				const void *data,
				int data_len)
{
	uint32_t command;
	struct nlattr *attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MAX + 1];
	int ret;

	if (wlan_cfg80211_nla_parse(attr, QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_MAX,
				    data, data_len,
				    wlan_cfg80211_gpio_config_policy)) {
		return -EINVAL;
	}

	if (attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND]) {
		command = nla_get_u32(
			attr[QCA_WLAN_VENDOR_ATTR_GPIO_PARAM_COMMAND]);

		if (command == QCA_WLAN_VENDOR_GPIO_CONFIG) {
			ret = wlan_set_gpio_config(psoc, attr);
		} else if (command == QCA_WLAN_VENDOR_GPIO_OUTPUT) {
			ret = wlan_set_gpio_output(psoc, attr);
		} else {
			osif_err_rl("Invalid command");
			return -EINVAL;
		}
	} else {
		osif_err_rl("Invalid command");
		return -EINVAL;
	}

	return ret;
}
qdf_export_symbol(wlan_cfg80211_start_gpio_config);


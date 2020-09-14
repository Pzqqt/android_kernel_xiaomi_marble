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
 * DOC: target_if_gpio.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <qdf_status.h>
#include <target_if.h>
#include <wlan_gpio_priv_api.h>
#include <target_if_gpio.h>
#include <wmi_unified_gpio_api.h>

/**
 * target_if_set_gpio_config() - API to send gpio config request to wmi
 * @psoc: pointer to psoc object
 * @param: pointer to gpio info
 *
 * Return: status of operation.
 */
static QDF_STATUS
target_if_set_gpio_config(struct wlan_objmgr_psoc *psoc,
			  struct gpio_config_params *param)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_gpio_config_cmd_send(wmi_handle, param);
}

/**
 * target_if_set_gpio_output() - API to send gpio output request to wmi
 * @psoc: pointer to psoc object
 * @param: pointer to gpio info
 *
 * Return: status of operation.
 */
static QDF_STATUS
target_if_set_gpio_output(struct wlan_objmgr_psoc *psoc,
			  struct gpio_output_params *param)
{
	struct wmi_unified *wmi_handle;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle) {
		target_if_err("wmi_handle is null.");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return wmi_unified_gpio_output_cmd_send(wmi_handle, param);
}

QDF_STATUS
target_if_gpio_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_gpio_tx_ops *gpio_ops;

	if (!tx_ops) {
		target_if_err("tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}
	gpio_ops = &tx_ops->gpio_ops;

	gpio_ops->set_gpio_config = target_if_set_gpio_config;
	gpio_ops->set_gpio_output = target_if_set_gpio_output;

	return QDF_STATUS_SUCCESS;
}


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
 * DOC:wlan_gpio_tgt_api.c
 *
 * This file provide API definitions to update gpio configuration from interface
 */
#include <wlan_gpio_priv_api.h>
#include <wlan_gpio_tgt_api.h>
#include <target_type.h>
#include <qdf_module.h>

QDF_STATUS tgt_set_gpio_config_req(struct wlan_objmgr_psoc *psoc,
				   struct gpio_config_params *param)
{
	struct wlan_lmac_if_gpio_tx_ops *gpio_tx_ops;

	if (!psoc) {
		gpio_err("NULL psoc");
		return QDF_STATUS_E_NULL_VALUE;
	}
	gpio_tx_ops = wlan_psoc_get_gpio_txops(psoc);
	if (!gpio_tx_ops)
		return QDF_STATUS_E_NULL_VALUE;

	return gpio_tx_ops->set_gpio_config(psoc, param);
}

QDF_STATUS tgt_set_gpio_output_req(struct wlan_objmgr_psoc *psoc,
				   struct gpio_output_params *param)
{
	struct wlan_lmac_if_gpio_tx_ops *gpio_tx_ops;

	if (!psoc) {
		gpio_err("NULL psoc");
		return QDF_STATUS_E_NULL_VALUE;
	}

	gpio_tx_ops = wlan_psoc_get_gpio_txops(psoc);
	if (!gpio_tx_ops)
		return QDF_STATUS_E_NULL_VALUE;

	return gpio_tx_ops->set_gpio_output(psoc, param);
}

QDF_STATUS tgt_gpio_config(struct wlan_objmgr_psoc *psoc, uint32_t gpio_num,
			   uint32_t input, uint32_t pull_type,
			   uint32_t intr_mode,  uint32_t mux_config_val,
			   uint32_t drive, uint32_t init_enable)
{
	struct gpio_config_params param;

	if (!psoc) {
		gpio_err("psoc_obj is null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_set(&param, sizeof(param), 0);
	param.pin_pull_type = pull_type;
	param.pin_num = gpio_num;
	param.pin_dir = input;
	param.pin_intr_mode = intr_mode;
	param.mux_config_val = mux_config_val;
	param.drive = drive;
	param.init_enable = init_enable;

	return tgt_set_gpio_config_req(psoc, &param);
}

qdf_export_symbol(tgt_gpio_config);

static bool tgt_gpio_disabled(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		gpio_err("tx_ops is NULL");
		return false;
	}
	target_type_tx_ops = &tx_ops->target_tx_ops;

	if (target_type_tx_ops->tgt_get_tgt_type)
		target_type = target_type_tx_ops->tgt_get_tgt_type(psoc);

	if ((target_type == TARGET_TYPE_QCA8074) ||
	    (target_type == TARGET_TYPE_QCN9100) ||
	    (target_type == TARGET_TYPE_QCA8074V2) ||
	    (target_type == TARGET_TYPE_QCA5018) ||
	    (target_type == TARGET_TYPE_QCA6018)) {
		return true;
	}

	return false;
}

QDF_STATUS tgt_gpio_output(struct wlan_objmgr_psoc *psoc, uint32_t gpio_num,
			   uint32_t set)
{
	struct gpio_output_params param;

	if (!psoc) {
		gpio_err("psoc_obj is null");
		return QDF_STATUS_E_INVAL;
	}

	if (tgt_gpio_disabled(psoc))
		return QDF_STATUS_E_INVAL;

	qdf_mem_set(&param, sizeof(param), 0);
	param.pin_num = gpio_num;
	param.pin_set = set;

	return tgt_set_gpio_output_req(psoc, &param);
}

qdf_export_symbol(tgt_gpio_output);

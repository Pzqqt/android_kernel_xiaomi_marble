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

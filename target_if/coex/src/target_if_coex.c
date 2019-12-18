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
 * DOC: contains coex target if functions
 */
#include <wlan_coex_main.h>
#include <target_if_coex.h>

static QDF_STATUS
target_if_coex_config_send(struct wlan_objmgr_pdev *pdev,
			   struct coex_config_params *param)
{
	wmi_unified_t pdev_wmi_handle;

	pdev_wmi_handle = GET_WMI_HDL_FROM_PDEV(pdev);
	if (!pdev_wmi_handle) {
		coex_err("Invalid PDEV WMI handle");
		return QDF_STATUS_E_FAILURE;
	}

	return wmi_unified_send_coex_config_cmd(pdev_wmi_handle, param);
}

QDF_STATUS
target_if_coex_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_coex_tx_ops *coex_ops;

	if (!tx_ops) {
		coex_err("target if tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	coex_ops = &tx_ops->coex_ops;
	coex_ops->coex_config_send = target_if_coex_config_send;

	return QDF_STATUS_SUCCESS;
}

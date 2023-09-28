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

#include <wlan_iot_sim_tgt_api.h>
#include <wlan_iot_sim_utils_api.h>

QDF_STATUS tgt_send_simulation_cmd(struct wlan_objmgr_pdev *pdev,
				   struct simulation_test_params *param)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		iot_sim_err("psoc is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		iot_sim_err("tx_ops is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}
	return tx_ops->iot_sim_tx_ops.iot_sim_send_cmd(pdev, param);
}

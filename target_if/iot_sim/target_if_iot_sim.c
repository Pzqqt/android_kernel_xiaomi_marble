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

#include <wmi_unified_api.h>
#include <target_if_iot_sim.h>
#include <init_deinit_lmac.h>
#include <target_if.h>
#include <qdf_module.h>

QDF_STATUS
target_if_iot_sim_send_cmd(struct wlan_objmgr_pdev *pdev,
			   struct simulation_test_params *param)
{
	if (pdev && pdev->tgt_if_handle) {
		struct wmi_unified *wmi_hdl;

		wmi_hdl = pdev->tgt_if_handle->wmi_handle;
		return wmi_unified_simulation_test_cmd_send(wmi_hdl, param);
	} else
		return QDF_STATUS_E_FAILURE;
}

void target_if_iot_sim_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->iot_sim_tx_ops.iot_sim_send_cmd =
						target_if_iot_sim_send_cmd;
}

qdf_export_symbol(target_if_iot_sim_register_tx_ops);


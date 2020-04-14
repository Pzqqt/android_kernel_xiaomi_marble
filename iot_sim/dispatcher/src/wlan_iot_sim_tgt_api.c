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

void *
tgt_get_target_handle(struct wlan_objmgr_pdev *pdev)
{
	struct iot_sim_context *isc;

	if (!pdev) {
		iot_sim_err("pdev is NULL!");
		return NULL;
	}
	isc = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						    WLAN_IOT_SIM_COMP);
	if (!isc) {
		iot_sim_err("pdev IOT_SIM object is NULL!");
		return NULL;
	}
	return isc->p_iot_sim_target_handle;
}

QDF_STATUS tgt_send_simulation_cmd(struct wlan_objmgr_pdev *pdev,
				   struct simulation_test_params *param)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	return psoc->soc_cb.tx_ops.iot_sim_tx_ops.iot_sim_send_cmd(pdev, param);
}

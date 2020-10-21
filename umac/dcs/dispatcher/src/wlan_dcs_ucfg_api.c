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
 * DOC: This file has the dcs dispatcher API implementation which is exposed
 * to outside of dcs component.
 */

#include "wlan_dcs_ucfg_api.h"
#include "../../core/src/wlan_dcs.h"

void ucfg_dcs_register_cb(
			struct wlan_objmgr_psoc *psoc,
			dcs_callback cbk,
			void *arg)
{
	struct dcs_psoc_priv_obj *dcs_psoc_priv;

	dcs_psoc_priv = wlan_objmgr_psoc_get_comp_private_obj(
							psoc,
							WLAN_UMAC_COMP_DCS);
	if (!dcs_psoc_priv) {
		dcs_err("dcs psoc private object is null");
		return;
	}

	dcs_psoc_priv->dcs_cbk.cbk = cbk;
	dcs_psoc_priv->dcs_cbk.arg = arg;
}

QDF_STATUS
ucfg_wlan_dcs_cmd(struct wlan_objmgr_psoc *psoc,
		  uint32_t pdev_id,
		  bool is_host_pdev_id)
{
	return wlan_dcs_cmd_send(psoc, pdev_id, is_host_pdev_id);
}

void ucfg_config_dcs_enable(struct wlan_objmgr_psoc *psoc,
			    uint32_t pdev_id,
			    uint8_t interference_type)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	dcs_pdev_priv->dcs_host_params.dcs_enable |= interference_type;
}

void ucfg_config_dcs_disable(struct wlan_objmgr_psoc *psoc,
			     uint32_t pdev_id,
			     uint8_t interference_type)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	dcs_pdev_priv->dcs_host_params.dcs_enable &= (~interference_type);
}

uint8_t ucfg_get_dcs_enable(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return 0;
	}

	return dcs_pdev_priv->dcs_host_params.dcs_enable;
}

void ucfg_dcs_clear(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id)
{
	wlan_dcs_clear(psoc, pdev_id);
}

void ucfg_config_dcs_event_data(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id,
				bool dcs_algorithm_process)
{
	wlan_dcs_set_algorithm_process(psoc, pdev_id, dcs_algorithm_process);
}

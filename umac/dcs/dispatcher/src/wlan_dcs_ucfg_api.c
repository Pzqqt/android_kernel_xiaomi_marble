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

void
ucfg_dcs_register_user_cb(struct wlan_objmgr_psoc *psoc,
			  uint8_t pdev_id, uint8_t vdev_id,
			  void (*cb)(uint8_t vdev_id,
				     struct wlan_host_dcs_im_user_stats *stats,
				     int status))
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	dcs_pdev_priv->requestor_vdev_id = vdev_id;
	dcs_pdev_priv->user_cb = cb;
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

uint8_t ucfg_get_dcs_enable(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	uint8_t enable = 0;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return 0;
	}

	if (dcs_pdev_priv->dcs_host_params.dcs_enable_cfg)
		enable = dcs_pdev_priv->dcs_host_params.dcs_enable;

	return enable;
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

void ucfg_dcs_reset_user_stats(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	struct wlan_host_dcs_im_user_stats *user_stats;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	wlan_dcs_pdev_obj_lock(dcs_pdev_priv);
	dcs_pdev_priv->dcs_host_params.user_request_count = 0;
	dcs_pdev_priv->dcs_host_params.notify_user = 0;
	user_stats = &dcs_pdev_priv->dcs_im_stats.user_dcs_im_stats;
	user_stats->cycle_count = 0;
	user_stats->rxclr_count = 0;
	user_stats->rx_frame_count = 0;
	user_stats->my_bss_rx_cycle_count = 0;
	user_stats->max_rssi = 0;
	user_stats->min_rssi = 0;
	wlan_dcs_pdev_obj_unlock(dcs_pdev_priv);
}

void ucfg_dcs_set_user_request(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			       uint32_t user_request_count)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	wlan_dcs_pdev_obj_lock(dcs_pdev_priv);
	dcs_pdev_priv->dcs_host_params.user_request_count = user_request_count;
	wlan_dcs_pdev_obj_unlock(dcs_pdev_priv);
}

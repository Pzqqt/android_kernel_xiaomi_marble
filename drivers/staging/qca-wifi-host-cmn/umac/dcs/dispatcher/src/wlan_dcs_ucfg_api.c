/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
#include "wlan_objmgr_vdev_obj.h"

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
			  uint8_t mac_id, uint8_t vdev_id,
			  void (*cb)(uint8_t vdev_id,
				     struct wlan_host_dcs_im_user_stats *stats,
				     int status))
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	dcs_pdev_priv->requestor_vdev_id = vdev_id;
	dcs_pdev_priv->user_cb = cb;
}

QDF_STATUS ucfg_dcs_register_awgn_cb(struct wlan_objmgr_psoc *psoc,
				     dcs_switch_chan_cb cb)
{
	struct dcs_psoc_priv_obj *dcs_psoc_priv;

	dcs_psoc_priv =
		wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_DCS);
	if (!dcs_psoc_priv) {
		dcs_err("dcs psoc private object is null");
		return QDF_STATUS_E_INVAL;
	}

	dcs_psoc_priv->switch_chan_cb = cb;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_wlan_dcs_cmd(struct wlan_objmgr_psoc *psoc,
		  uint32_t mac_id,
		  bool is_host_pdev_id)
{
	return wlan_dcs_cmd_send(psoc, mac_id, is_host_pdev_id);
}

void ucfg_config_dcs_enable(struct wlan_objmgr_psoc *psoc,
			    uint32_t mac_id,
			    uint8_t interference_type)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	dcs_pdev_priv->dcs_host_params.dcs_enable |= interference_type;
}

void ucfg_config_dcs_disable(struct wlan_objmgr_psoc *psoc,
			     uint32_t mac_id,
			     uint8_t interference_type)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	dcs_pdev_priv->dcs_host_params.dcs_enable &= (~interference_type);
}

uint8_t ucfg_get_dcs_enable(struct wlan_objmgr_psoc *psoc, uint8_t mac_id)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	uint8_t enable = 0;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return 0;
	}

	if (dcs_pdev_priv->dcs_host_params.dcs_enable_cfg)
		enable = dcs_pdev_priv->dcs_host_params.dcs_enable;

	return enable;
}

void ucfg_dcs_clear(struct wlan_objmgr_psoc *psoc, uint32_t mac_id)
{
	wlan_dcs_clear(psoc, mac_id);
}

void ucfg_config_dcs_event_data(struct wlan_objmgr_psoc *psoc, uint32_t mac_id,
				bool dcs_algorithm_process)
{
	wlan_dcs_set_algorithm_process(psoc, mac_id, dcs_algorithm_process);
}

void ucfg_dcs_reset_user_stats(struct wlan_objmgr_psoc *psoc, uint8_t mac_id)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	struct wlan_host_dcs_im_user_stats *user_stats;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
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

void ucfg_dcs_set_user_request(struct wlan_objmgr_psoc *psoc, uint8_t mac_id,
			       uint32_t user_request_count)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	wlan_dcs_pdev_obj_lock(dcs_pdev_priv);
	dcs_pdev_priv->dcs_host_params.user_request_count = user_request_count;
	wlan_dcs_pdev_obj_unlock(dcs_pdev_priv);
}

QDF_STATUS ucfg_dcs_get_ch_util(struct wlan_objmgr_psoc *psoc, uint8_t mac_id,
				struct wlan_host_dcs_ch_util_stats *dcs_stats)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, mac_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return QDF_STATUS_E_INVAL;
	}

	wlan_dcs_pdev_obj_lock(dcs_pdev_priv);
	qdf_mem_copy(dcs_stats,
		     &dcs_pdev_priv->dcs_im_stats.dcs_ch_util_im_stats,
		     sizeof(*dcs_stats));
	wlan_dcs_pdev_obj_unlock(dcs_pdev_priv);

	return QDF_STATUS_SUCCESS;
}

/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 *  DOC: wlan_mgmt_txrx_rx_reo_utils_api.c
 *  This file contains mgmt rx re-ordering related public function definitions
 */

#include <wlan_mgmt_txrx_rx_reo_utils_api.h>
#include "../../core/src/wlan_mgmt_txrx_rx_reo_i.h"
#include <cfg_ucfg_api.h>

QDF_STATUS
wlan_mgmt_rx_reo_deinit(void)
{
	return mgmt_rx_reo_deinit_context();
}

QDF_STATUS
wlan_mgmt_rx_reo_init(void)
{
	return mgmt_rx_reo_init_context();
}

QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_create_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	return mgmt_rx_reo_pdev_obj_create_notification(pdev,
							mgmt_txrx_pdev_ctx);
}

QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_destroy_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	return mgmt_rx_reo_pdev_obj_destroy_notification(pdev,
							 mgmt_txrx_pdev_ctx);
}

bool
wlan_mgmt_rx_reo_is_feature_enabled_at_psoc(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		mgmt_rx_reo_err("psoc is NULL!");
		return false;
	}

	if (!cfg_get(psoc, CFG_MGMT_RX_REO_ENABLE))
		return false;

	return wlan_psoc_nif_feat_cap_get(psoc, WLAN_SOC_F_MGMT_RX_REO_CAPABLE);
}

bool
wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(struct wlan_objmgr_pdev *pdev)
{
	if (!pdev) {
		mgmt_rx_reo_err("pdev is NULL!");
		return false;
	}

	return wlan_mgmt_rx_reo_is_feature_enabled_at_psoc(
			wlan_pdev_get_psoc(pdev));
}

/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains green ap north bound interface definitions
 */

#include <qdf_status.h>
#include <wlan_green_ap_ucfg_api.h>
#include <../../core/src/wlan_green_ap_main_i.h>

QDF_STATUS ucfg_green_ap_update_user_config(
			struct wlan_objmgr_pdev *pdev,
			struct green_ap_user_cfg *green_ap_cfg)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	struct wlan_green_ap_egap_params *egap_params;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	egap_params = &green_ap_ctx->egap_params;

	egap_params->host_enable_egap = green_ap_cfg->host_enable_egap;
	egap_params->egap_inactivity_time = green_ap_cfg->egap_inactivity_time;
	egap_params->egap_wait_time = green_ap_cfg->egap_wait_time;
	egap_params->egap_feature_flags = green_ap_cfg->egap_feature_flags;
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_green_ap_enable_egap(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	struct wlan_lmac_if_green_ap_tx_ops *green_ap_tx_ops;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_tx_ops = wlan_psoc_get_green_ap_tx_ops(green_ap_ctx);
	if (!green_ap_tx_ops) {
		green_ap_err("green ap tx ops obtained are NULL");
		return  QDF_STATUS_E_FAILURE;
	}

	if (!green_ap_tx_ops->enable_egap) {
		green_ap_err("tx op for sending enbale/disable green ap is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return green_ap_tx_ops->enable_egap(pdev, &green_ap_ctx->egap_params);
}

QDF_STATUS ucfg_green_ap_set_ps_config(struct wlan_objmgr_pdev *pdev,
				       uint8_t value)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		return QDF_STATUS_SUCCESS;
	}

	green_ap_ctx->ps_enable = value;
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_green_ap_get_ps_config(struct wlan_objmgr_pdev *pdev,
				       uint8_t *ps_enable)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		return QDF_STATUS_SUCCESS;
	}

	*ps_enable = green_ap_ctx->ps_enable;
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_green_ap_set_transition_time(struct wlan_objmgr_pdev *pdev,
					     uint32_t val)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		return QDF_STATUS_SUCCESS;
	}

	green_ap_ctx->ps_trans_time = val;
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_green_ap_get_transition_time(struct wlan_objmgr_pdev *pdev,
					     uint32_t *ps_trans_time)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		return QDF_STATUS_SUCCESS;
	}

	*ps_trans_time = green_ap_ctx->ps_trans_time;
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_green_ap_config(struct wlan_objmgr_pdev *pdev, uint8_t val)
{

	uint8_t flag;

	if (wlan_green_ap_get_capab(pdev) == QDF_STATUS_E_NOSUPPORT) {
		green_ap_err("GreenAP not supported on radio\n");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if (val) {
		struct wlan_pdev_green_ap_ctx *green_ap_ctx;

		wlan_objmgr_pdev_iterate_obj_list(pdev,
					WLAN_VDEV_OP,
					wlan_green_ap_check_mode,
					&flag, 0, WLAN_GREEN_AP_ID);
		if (flag == 1) {
			green_ap_err("Radio not in AP mode."
					"Feature not supported");
			return QDF_STATUS_E_NOSUPPORT;
		}

		green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(pdev,
					WLAN_UMAC_COMP_GREEN_AP);

		if (!green_ap_ctx) {
			green_ap_err("green ap context obtained is NULL");
			return QDF_STATUS_E_NOSUPPORT;
		}

		ucfg_green_ap_set_ps_config(pdev, val);

		if (wlan_util_is_vap_active(pdev, WLAN_GREEN_AP_ID) ==
					    QDF_STATUS_SUCCESS)
			wlan_green_ap_start(pdev);
	} else {
		wlan_green_ap_stop(pdev);
	}

	return QDF_STATUS_SUCCESS;
}

void ucfg_green_ap_enable_debug_prints(struct wlan_objmgr_pdev *pdev,
				uint32_t val)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return;
	}

	green_ap_ctx->dbg_enable = val;
}

bool ucfg_green_ap_get_debug_prints(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return false;
	}

	return green_ap_ctx->dbg_enable;
}


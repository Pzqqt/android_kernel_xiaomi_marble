/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
#include <wlan_green_ap_api.h>
#include <../../core/src/wlan_green_ap_main_i.h>
#include <wlan_objmgr_global_obj.h>
#include "cfg_green_ap_params.h"
#include "cfg_ucfg_api.h"

QDF_STATUS wlan_green_ap_get_capab(
			struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_green_ap_tx_ops *green_ap_tx_ops;
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(pdev,
					WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}


	green_ap_tx_ops = wlan_psoc_get_green_ap_tx_ops(green_ap_ctx);
	if (!green_ap_tx_ops) {
		green_ap_err("green ap tx ops obtained are NULL");
		return QDF_STATUS_E_EXISTS;
	}

	if (green_ap_tx_ops->get_capab)
		return green_ap_tx_ops->get_capab(pdev);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_green_ap_pdev_obj_create_notification() - called from objmgr when pdev
 *                                                is created
 * @pdev: pdev context
 * @arg: argument
 *
 * This function gets called from object manager when pdev is being created and
 * creates green ap context and attach it to objmgr.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_green_ap_pdev_obj_create_notification(
			struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = qdf_mem_malloc(sizeof(*green_ap_ctx));
	if (!green_ap_ctx)
		return QDF_STATUS_E_NOMEM;

	green_ap_ctx->ps_state = WLAN_GREEN_AP_PS_IDLE_STATE;
	green_ap_ctx->ps_event = WLAN_GREEN_AP_PS_WAIT_EVENT;
	green_ap_ctx->ps_mode = WLAN_GREEN_AP_MODE_NO_STA;
	green_ap_ctx->num_nodes = 0;
	green_ap_ctx->num_nodes_multistream = 0;
	green_ap_ctx->ps_on_time = WLAN_GREEN_AP_PS_ON_TIME;
	green_ap_ctx->ps_trans_time = WLAN_GREEN_AP_PS_TRANS_TIME;

	green_ap_ctx->pdev = pdev;

	qdf_timer_init(NULL, &green_ap_ctx->ps_timer,
		       wlan_green_ap_timer_fn,
		       pdev, QDF_TIMER_TYPE_WAKE_APPS);

	qdf_spinlock_create(&green_ap_ctx->lock);
	if (wlan_objmgr_pdev_component_obj_attach(pdev,
				WLAN_UMAC_COMP_GREEN_AP,
				green_ap_ctx, QDF_STATUS_SUCCESS)
			!= QDF_STATUS_SUCCESS) {
		green_ap_err("Failed to attach green ap ctx in pdev ctx");
		status = QDF_STATUS_E_FAILURE;
		goto err_pdev_attach;
	}

	green_ap_info("Green AP creation successful, green ap ctx: %pK, pdev: %pK",
		      green_ap_ctx, pdev);

	return QDF_STATUS_SUCCESS;

err_pdev_attach:
	qdf_spinlock_destroy(&green_ap_ctx->lock);
	qdf_timer_free(&green_ap_ctx->ps_timer);
	qdf_mem_free(green_ap_ctx);
	return status;
}

/**
 * wlan_green_ap_pdev_obj_destroy_notification() - called from objmgr when
 *                                                 pdev is destroyed
 * @pdev: pdev context
 * @arg: argument
 *
 * This function gets called from object manager when pdev is being destroyed
 * and deletes green ap context and detach it from objmgr.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_green_ap_pdev_obj_destroy_notification(
			struct wlan_objmgr_pdev *pdev, void *arg)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context is already NULL");
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_info("Deleting green ap pdev obj, green ap ctx: %pK, pdev: %pK",
		      green_ap_ctx, pdev);

	if (wlan_objmgr_pdev_component_obj_detach(pdev,
				WLAN_UMAC_COMP_GREEN_AP, green_ap_ctx) !=
				QDF_STATUS_SUCCESS) {
		green_ap_err("Failed to detach green ap ctx in psoc ctx");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_timer_free(&green_ap_ctx->ps_timer);
	qdf_spinlock_destroy(&green_ap_ctx->lock);

	qdf_mem_free(green_ap_ctx);
	green_ap_info("green ap deletion successful, pdev: %pK", pdev);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_green_ap_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_pdev_create_handler(
				WLAN_UMAC_COMP_GREEN_AP,
				wlan_green_ap_pdev_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		green_ap_err("Failed to register green ap obj create handler");
		goto err_pdev_create;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
				WLAN_UMAC_COMP_GREEN_AP,
				wlan_green_ap_pdev_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		green_ap_err("Failed to register green ap obj destroy handler");
		goto err_pdev_delete;
	}

	green_ap_info("Successfully registered create and destroy handlers with objmgr");
	return QDF_STATUS_SUCCESS;

err_pdev_delete:
	wlan_objmgr_unregister_pdev_create_handler(
				WLAN_UMAC_COMP_GREEN_AP,
				wlan_green_ap_pdev_obj_create_notification,
				NULL);
err_pdev_create:
	return status;
}

QDF_STATUS wlan_green_ap_deinit(void)
{
	if (wlan_objmgr_unregister_pdev_create_handler(
				WLAN_UMAC_COMP_GREEN_AP,
				wlan_green_ap_pdev_obj_create_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_pdev_destroy_handler(
				WLAN_UMAC_COMP_GREEN_AP,
				wlan_green_ap_pdev_obj_destroy_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_info("Successfully unregistered create and destroy handlers with objmgr");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_green_ap_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev) {
		green_ap_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		green_ap_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	green_ap_ctx->ps_enable = cfg_get(psoc,
					CFG_ENABLE_GREEN_AP_FEATURE);
	green_ap_ctx->egap_params.host_enable_egap = cfg_get(psoc,
					CFG_ENABLE_EGAP_FEATURE);
	green_ap_ctx->egap_params.egap_inactivity_time = cfg_get(psoc,
					CFG_EGAP_INACT_TIME_FEATURE);
	green_ap_ctx->egap_params.egap_wait_time = cfg_get(psoc,
					CFG_EGAP_WAIT_TIME_FEATURE);
	green_ap_ctx->egap_params.egap_feature_flags = cfg_get(psoc,
					CFG_EGAP_FLAGS_FEATURE);

	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_green_ap_start(struct wlan_objmgr_pdev *pdev)
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

	green_ap_debug("Green AP start received");

	/* Make sure the start function does not get called 2 times */
	qdf_spin_lock_bh(&green_ap_ctx->lock);

	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		green_ap_debug("enhanced green ap support is enabled");
		return QDF_STATUS_SUCCESS;
	}

	if (green_ap_ctx->ps_state == WLAN_GREEN_AP_PS_IDLE_STATE) {
		if (green_ap_ctx->ps_enable) {
			qdf_spin_unlock_bh(&green_ap_ctx->lock);
			return wlan_green_ap_state_mc(green_ap_ctx,
					      WLAN_GREEN_AP_PS_START_EVENT);
		}
	}

	qdf_spin_unlock_bh(&green_ap_ctx->lock);
	return QDF_STATUS_E_ALREADY;
}

QDF_STATUS wlan_green_ap_stop(struct wlan_objmgr_pdev *pdev)
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

	green_ap_debug("Green AP stop received");

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		green_ap_debug("enhanced green ap support is enabled");
		return QDF_STATUS_SUCCESS;
	}

	/* Delete the timer just to be sure */
	qdf_timer_stop(&green_ap_ctx->ps_timer);

	/* Disable the power save */
	green_ap_ctx->ps_enable = WLAN_GREEN_AP_PS_DISABLE;

	qdf_spin_unlock_bh(&green_ap_ctx->lock);
	return wlan_green_ap_state_mc(green_ap_ctx,
				      WLAN_GREEN_AP_PS_STOP_EVENT);
}

QDF_STATUS wlan_green_ap_add_sta(struct wlan_objmgr_pdev *pdev)
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

	green_ap_debug("Green AP add sta received");

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		green_ap_debug("enhanced green ap support is enabled");
		return QDF_STATUS_SUCCESS;
	}
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return wlan_green_ap_state_mc(green_ap_ctx,
				      WLAN_GREEN_AP_ADD_STA_EVENT);
}

QDF_STATUS wlan_green_ap_add_multistream_sta(struct wlan_objmgr_pdev *pdev)
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

	green_ap_debug("Green AP add multistream sta received");

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		green_ap_debug("enhanced green ap support is enabled");
		return QDF_STATUS_SUCCESS;
	}
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return wlan_green_ap_state_mc(green_ap_ctx,
			WLAN_GREEN_AP_ADD_MULTISTREAM_STA_EVENT);
}

QDF_STATUS wlan_green_ap_del_sta(struct wlan_objmgr_pdev *pdev)
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

	green_ap_debug("Green AP del sta received");

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		green_ap_info("enhanced green ap support is enabled");
		return QDF_STATUS_SUCCESS;
	}
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return wlan_green_ap_state_mc(green_ap_ctx,
				      WLAN_GREEN_AP_DEL_STA_EVENT);
}

QDF_STATUS wlan_green_ap_del_multistream_sta(struct wlan_objmgr_pdev *pdev)
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

	green_ap_debug("Green AP del multistream sta received");

	qdf_spin_lock_bh(&green_ap_ctx->lock);
	if (wlan_is_egap_enabled(green_ap_ctx)) {
		qdf_spin_unlock_bh(&green_ap_ctx->lock);
		green_ap_info("enhanced green ap support is enabled");
		return QDF_STATUS_SUCCESS;
	}
	qdf_spin_unlock_bh(&green_ap_ctx->lock);

	return wlan_green_ap_state_mc(green_ap_ctx,
			WLAN_GREEN_AP_DEL_MULTISTREAM_STA_EVENT);
}

bool wlan_green_ap_is_ps_enabled(struct wlan_objmgr_pdev *pdev)
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

	if ((green_ap_ctx->ps_state == WLAN_GREEN_AP_PS_ON_STATE) &&
			(green_ap_ctx->ps_enable))
		return true;

	return false;

}

void wlan_green_ap_suspend_handle(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;

	if (!pdev) {
		green_ap_err("pdev context passed is NULL");
		return;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
				pdev, WLAN_UMAC_COMP_GREEN_AP);

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return;
	}

	wlan_green_ap_stop(pdev);

	green_ap_ctx->ps_enable = WLAN_GREEN_AP_PS_SUSPEND;
}

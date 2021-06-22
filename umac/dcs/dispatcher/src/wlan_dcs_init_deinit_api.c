/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: This file init/deint functions for dcs module.
 */

#include "wlan_dcs_init_deinit_api.h"
#include "../../core/src/wlan_dcs.h"
#include "cfg_dcs.h"
#include "cfg_ucfg_api.h"

/**
 * wlan_dcs_psoc_obj_create_notification() - dcs psoc cretae handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
wlan_dcs_psoc_obj_create_notification(struct wlan_objmgr_psoc *psoc,
				      void *arg_list)
{
	QDF_STATUS status;
	struct dcs_psoc_priv_obj *dcs_psoc_obj;
	uint8_t loop;

	dcs_psoc_obj = qdf_mem_malloc(sizeof(*dcs_psoc_obj));

	if (!dcs_psoc_obj)
		return QDF_STATUS_E_NOMEM;

	for (loop = 0; loop < WLAN_DCS_MAX_PDEVS; loop++)
		qdf_spinlock_create(&dcs_psoc_obj->dcs_pdev_priv[loop].lock);

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_DCS,
						       dcs_psoc_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		dcs_err("dcs pdev obj attach failed");
		qdf_mem_free(dcs_psoc_obj);
		return status;
	}

	dcs_info("dcs psoc object attached");

	return status;
}

/**
 * wlan_dcs_psoc_obj_destroy_notification() - dcs psoc destroy handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS
wlan_dcs_psoc_obj_destroy_notification(struct wlan_objmgr_psoc *psoc,
				       void *arg_list)
{
	QDF_STATUS status;
	uint8_t loop;
	struct dcs_psoc_priv_obj *dcs_psoc_obj =
		wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_DCS);

	if (!dcs_psoc_obj) {
		dcs_err("invalid wifi dcs obj");
		return QDF_STATUS_E_FAULT;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_DCS,
						       dcs_psoc_obj);
	for (loop = 0; loop < WLAN_DCS_MAX_PDEVS; loop++) {
		qdf_timer_free(&dcs_psoc_obj->dcs_pdev_priv[loop].
							dcs_disable_timer);
		qdf_spinlock_destroy(&dcs_psoc_obj->dcs_pdev_priv[loop].lock);
	}
	qdf_mem_free(dcs_psoc_obj);

	return status;
}

QDF_STATUS wlan_dcs_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_DCS,
			wlan_dcs_psoc_obj_create_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		goto err_psoc_create;

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_DCS,
			wlan_dcs_psoc_obj_destroy_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		goto err_psoc_delete;

	return QDF_STATUS_SUCCESS;

err_psoc_delete:
	wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_DCS,
			wlan_dcs_psoc_obj_create_notification,
			NULL);
err_psoc_create:
	return status;
}

QDF_STATUS wlan_dcs_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_DCS,
			wlan_dcs_psoc_obj_create_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_DCS,
			wlan_dcs_psoc_obj_destroy_notification,
			NULL);

	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_dcs_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_dcs_attach(psoc);
}

QDF_STATUS wlan_dcs_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_dcs_detach(psoc);
}

QDF_STATUS wlan_dcs_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	struct dcs_psoc_priv_obj *dcs_psoc_obj;
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	uint8_t loop;

	if (!psoc) {
		dcs_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	dcs_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(
			psoc, WLAN_UMAC_COMP_DCS);
	if (!dcs_psoc_obj) {
		dcs_err("dcs psoc private object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (loop = 0; loop < WLAN_DCS_MAX_PDEVS; loop++) {
		dcs_pdev_priv = &dcs_psoc_obj->dcs_pdev_priv[loop];
		dcs_pdev_priv->dcs_host_params.dcs_enable_cfg =
					cfg_get(psoc, CFG_DCS_ENABLE);
		dcs_pdev_priv->dcs_host_params.dcs_algorithm_process = false;
		dcs_pdev_priv->dcs_host_params.dcs_debug =
					cfg_get(psoc, CFG_DCS_DEBUG);
		dcs_pdev_priv->dcs_host_params.phy_err_penalty =
				cfg_get(psoc, CFG_DCS_PHY_ERR_PENALTY);
		dcs_pdev_priv->dcs_host_params.phy_err_threshold =
				cfg_get(psoc, CFG_DCS_PHY_ERR_THRESHOLD);
		dcs_pdev_priv->dcs_host_params.radar_err_threshold =
				cfg_get(psoc, CFG_DCS_RADAR_ERR_THRESHOLD);
		dcs_pdev_priv->dcs_host_params.coch_intfr_threshold =
				cfg_get(psoc, CFG_DCS_COCH_INTFR_THRESHOLD);
		dcs_pdev_priv->dcs_host_params.user_max_cu =
				cfg_get(psoc, CFG_DCS_USER_MAX_CU);
		dcs_pdev_priv->dcs_host_params.intfr_detection_threshold =
			cfg_get(psoc, CFG_DCS_INTFR_DETECTION_THRESHOLD);
		dcs_pdev_priv->dcs_host_params.intfr_detection_window =
				cfg_get(psoc, CFG_DCS_INTFR_DETECTION_WINDOW);
		dcs_pdev_priv->dcs_host_params.tx_err_threshold =
				cfg_get(psoc, CFG_DCS_TX_ERR_THRESHOLD);
		dcs_pdev_priv->dcs_freq_ctrl_params.
					disable_threshold_per_5mins =
			cfg_get(psoc, CFG_DCS_DISABLE_THRESHOLD_PER_5MINS);
		dcs_pdev_priv->dcs_freq_ctrl_params.restart_delay =
				cfg_get(psoc, CFG_DCS_RESTART_DELAY);

		qdf_timer_init(NULL, &dcs_pdev_priv->dcs_disable_timer,
			       wlan_dcs_disable_timer_fn,
			       &dcs_pdev_priv->dcs_timer_args,
			       QDF_TIMER_TYPE_WAKE_APPS);
	}

	return QDF_STATUS_SUCCESS;
}

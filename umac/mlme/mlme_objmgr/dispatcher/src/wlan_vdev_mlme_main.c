/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implements MLME component object creation/initialization/destroy
 */

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_mlme_cmn.h"
#include "include/wlan_vdev_mlme.h"
#include "include/wlan_pdev_mlme.h"
#include "vdev_mgr/core/src/vdev_mlme_sm.h"
#include "wlan_pdev_mlme_api.h"
#include "wlan_vdev_mlme_api.h"
#include "wlan_serialization_api.h"
#include "wlan_utility.h"
#include <cdp_txrx_cmn.h>

static QDF_STATUS mlme_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev,
					       void *arg)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_pdev *pdev;
	struct pdev_mlme_obj *pdev_mlme;

	if (!vdev) {
		mlme_err(" VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(" PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = qdf_mem_malloc(sizeof(*vdev_mlme));
	if (!vdev_mlme)
		return QDF_STATUS_E_NOMEM;

	vdev_mlme->vdev = vdev;

	if (pdev_mlme->mlme_register_ops(vdev_mlme) != QDF_STATUS_SUCCESS) {
		mlme_err("Callbacks registration is failed");
		goto init_failed;
	}

	if (mlme_vdev_sm_create(vdev_mlme) != QDF_STATUS_SUCCESS) {
		mlme_err("SME creation failed");
		goto init_failed;
	}

	if (mlme_vdev_ops_ext_hdl_create(vdev_mlme) !=
						QDF_STATUS_SUCCESS) {
		mlme_err("Legacy vdev object creation failed");
		goto ext_hdl_create_failed;
	}

	wlan_objmgr_vdev_component_obj_attach((struct wlan_objmgr_vdev *)vdev,
					      WLAN_UMAC_COMP_MLME,
					      (void *)vdev_mlme,
					      QDF_STATUS_SUCCESS);

	if (mlme_vdev_ops_ext_hdl_post_create(vdev_mlme) !=
						QDF_STATUS_SUCCESS) {
		mlme_err("Legacy vdev object post creation failed");
		goto ext_hdl_post_create_failed;
	}

	return QDF_STATUS_SUCCESS;

ext_hdl_post_create_failed:
	mlme_vdev_ops_ext_hdl_destroy(vdev_mlme);
	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_MLME,
					      vdev_mlme);
ext_hdl_create_failed:
	mlme_vdev_sm_destroy(vdev_mlme);
init_failed:
	qdf_mem_free(vdev_mlme);

	return QDF_STATUS_E_FAILURE;
}

#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
static void mlme_vdev_obj_timer_deinit(
				struct vdev_mlme_obj *vdev_mlme)
{
	struct vdev_response_timer *vdev_rsp;

	vdev_rsp = &vdev_mlme->vdev_rt;
	qdf_timer_free(&vdev_rsp->rsp_timer);
}
#else
static void mlme_vdev_obj_timer_deinit(
				struct vdev_mlme_obj *vdev_mlme)
{
}
#endif
static QDF_STATUS mlme_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev,
						void *arg)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_psoc *psoc;
	struct cdp_soc_t *soc_txrx_handle;
	struct cdp_vdev *vdev_txrx_handle;

	if (!vdev) {
		mlme_err(" VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_info(" VDEV MLME component object is NULL");
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	soc_txrx_handle = (struct cdp_soc_t *)wlan_psoc_get_dp_handle(psoc);
	vdev_txrx_handle = wlan_vdev_get_dp_handle(vdev);
	if (soc_txrx_handle && vdev_txrx_handle) {
		wlan_vdev_set_dp_handle(vdev, NULL);
		cdp_vdev_detach(soc_txrx_handle, vdev_txrx_handle,
				NULL, NULL);
	}

	mlme_vdev_obj_timer_deinit(vdev_mlme);

	mlme_vdev_sm_destroy(vdev_mlme);

	mlme_vdev_ops_ext_hdl_destroy(vdev_mlme);

	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_MLME,
					      vdev_mlme);
	qdf_mem_free(vdev_mlme);

	return QDF_STATUS_SUCCESS;
}

static void mlme_scan_serialization_comp_info_cb(
		struct wlan_objmgr_vdev *vdev,
		union wlan_serialization_rules_info *comp_info)
{
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;

	if (!comp_info || !vdev) {
		mlme_err("comp_info or vdev is NULL");
		return;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("pdev is NULL");
		return;
	}

	comp_info->scan_info.is_mlme_op_in_progress = false;

	status = wlan_util_is_pdev_scan_allowed(pdev, WLAN_MLME_SER_IF_ID);
	if (status != QDF_STATUS_SUCCESS)
		comp_info->scan_info.is_mlme_op_in_progress = true;
}

QDF_STATUS wlan_mlme_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *tx_ops;

	status = wlan_serialization_register_comp_info_cb
			(psoc,
			 WLAN_UMAC_COMP_MLME,
			 WLAN_SER_CMD_SCAN,
			 mlme_scan_serialization_comp_info_cb);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Serialize scan cmd register failed");
		return status;
	}

	/* Register for WMI events into target_if rx  */
	tx_ops = wlan_mlme_get_lmac_tx_ops(psoc);
	if (tx_ops && tx_ops->vdev_mlme_attach)
		tx_ops->vdev_mlme_attach(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlme_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wlan_lmac_if_mlme_tx_ops *tx_ops;

	status = wlan_serialization_deregister_comp_info_cb
						(psoc,
						 WLAN_UMAC_COMP_MLME,
						 WLAN_SER_CMD_SCAN);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Serialize scan cmd deregister failed");
		return status;
	}

	/* Unregister WMI events  */
	tx_ops = wlan_mlme_get_lmac_tx_ops(psoc);
	if (tx_ops && tx_ops->vdev_mlme_detach)
		tx_ops->vdev_mlme_detach(psoc);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_vdev_mlme_init(void)
{
	if (wlan_objmgr_register_vdev_create_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_vdev_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_register_vdev_destroy_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_vdev_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS) {
		if (wlan_objmgr_unregister_vdev_create_handler
					(WLAN_UMAC_COMP_MLME,
					 mlme_vdev_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
			return QDF_STATUS_E_FAILURE;

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_vdev_mlme_deinit(void)
{
	if (wlan_objmgr_unregister_vdev_create_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_vdev_obj_create_handler, NULL)
					!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_unregister_vdev_destroy_handler
				(WLAN_UMAC_COMP_MLME,
				 mlme_vdev_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

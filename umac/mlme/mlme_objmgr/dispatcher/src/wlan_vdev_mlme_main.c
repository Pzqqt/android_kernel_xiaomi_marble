/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Implements MLME component object creation/initialization/destroy
 */

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_vdev_mlme.h"
#include "vdev_mgr/core/src/vdev_mlme_sm.h"

static QDF_STATUS mlme_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev,
					       void *arg)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct wlan_objmgr_pdev *pdev;

	if (!vdev) {
		mlme_err(" VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err(" PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = qdf_mem_malloc(sizeof(*vdev_mlme));
	if (!vdev_mlme) {
		mlme_err(" MLME component object alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	vdev_mlme->vdev = vdev;

	if (pdev->pdev_mlme.mlme_register_ops(vdev_mlme)
							!= QDF_STATUS_SUCCESS) {
		mlme_err("Callbacks registration is failed");
		goto init_failed;
	}

	if (mlme_vdev_sm_create(vdev_mlme) != QDF_STATUS_SUCCESS) {
		mlme_err("SME creation failed");
		goto init_failed;
	}

	if (mlme_vdev_ext_hdl_create(vdev_mlme) != QDF_STATUS_SUCCESS) {
		mlme_err("Legacy vdev object creation failed");
		goto ext_hdl_create_failed;
	}

	wlan_objmgr_vdev_component_obj_attach((struct wlan_objmgr_vdev *)vdev,
					      WLAN_UMAC_COMP_MLME,
					      (void *)vdev_mlme,
					      QDF_STATUS_SUCCESS);

	if (mlme_vdev_ext_hdl_post_create(vdev_mlme) != QDF_STATUS_SUCCESS) {
		mlme_err("Legacy vdev object post creation failed");
		goto ext_hdl_post_create_failed;
	}

	return QDF_STATUS_SUCCESS;

ext_hdl_post_create_failed:
	mlme_vdev_ext_hdl_destroy(vdev_mlme);
	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_MLME,
					      vdev_mlme);
ext_hdl_create_failed:
	mlme_vdev_sm_destroy(vdev_mlme);
init_failed:
	qdf_mem_free(vdev_mlme);

	return QDF_STATUS_E_FAILURE;
}

static QDF_STATUS mlme_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev,
						void *arg)
{
	struct vdev_mlme_obj *vdev_mlme;

	if (!vdev) {
		mlme_err(" VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							  WLAN_UMAC_COMP_MLME);
	if (!vdev_mlme) {
		mlme_err(" VDEV MLME component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_vdev_sm_destroy(vdev_mlme);

	mlme_vdev_ext_hdl_destroy(vdev_mlme);

	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_MLME,
					      vdev_mlme);
	qdf_mem_free(vdev_mlme);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_vdev_mlme_init(void)
{
	if (wlan_objmgr_register_vdev_create_handler(WLAN_UMAC_COMP_MLME,
				mlme_vdev_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_register_vdev_destroy_handler(WLAN_UMAC_COMP_MLME,
				mlme_vdev_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS) {
		if (wlan_objmgr_unregister_vdev_create_handler(
					WLAN_UMAC_COMP_MLME,
					mlme_vdev_obj_create_handler, NULL)
						!= QDF_STATUS_SUCCESS)
			return QDF_STATUS_E_FAILURE;

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_vdev_mlme_deinit(void)
{
	if (wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_MLME,
				mlme_vdev_obj_create_handler, NULL)
					!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	if (wlan_objmgr_unregister_vdev_destroy_handler(WLAN_UMAC_COMP_MLME,
				mlme_vdev_obj_destroy_handler, NULL)
						!= QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

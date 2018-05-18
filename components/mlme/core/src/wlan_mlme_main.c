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
 * DOC: define utility API related to the mlme component
 * called by other components
 */

#include "wlan_mlme_main.h"

/**
 * wlan_psoc_get_mlme_obj() - private API to get mlme object from psoc
 * @psoc: psoc object
 *
 * Return: mlme object
 */
static inline struct wlan_mlme_psoc_obj *
wlan_psoc_get_mlme_obj(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = (struct wlan_mlme_psoc_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
						      WLAN_UMAC_COMP_MLME);

	return mlme_obj;
}

QDF_STATUS mlme_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_created_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("unable to register psoc create handle");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_destroyed_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS)
		mlme_err("unable to register psoc create handle");

	return status;
}

QDF_STATUS mlme_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_created_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("unable to unregister psoc create handle");
		return status;
	}

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_MLME,
			mlme_psoc_object_destroyed_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS)
		mlme_err("unable to unregister psoc destroy handle");

	return status;
}

QDF_STATUS mlme_psoc_object_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	QDF_STATUS status;
	struct wlan_mlme_psoc_obj *mlme_obj;

	mlme_obj = qdf_mem_malloc(sizeof(struct wlan_mlme_psoc_obj));
	if (!mlme_obj) {
		mlme_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       mlme_obj,
						       QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Failed to attach psoc_ctx with psoc");
		qdf_mem_free(mlme_obj);
	}

	return status;
}

QDF_STATUS mlme_psoc_object_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct wlan_mlme_psoc_obj *mlme_obj = NULL;
	QDF_STATUS status;

	mlme_obj = wlan_psoc_get_mlme_obj(psoc);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_MLME,
						       mlme_obj);
	if (status != QDF_STATUS_SUCCESS) {
		mlme_err("Failed to detach psoc_ctx from psoc");
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	qdf_mem_free(mlme_obj);

out:
	return status;
}


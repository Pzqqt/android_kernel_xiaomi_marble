/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_tdls_main.c
 *
 * TDLS core function definitions
 */

#include "wlan_tdls_main.h"

QDF_STATUS tdls_psoc_obj_create_notification(struct wlan_objmgr_psoc *psoc,
					     void *arg_list)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *tdls_soc_obj;

	tdls_soc_obj = qdf_mem_malloc(sizeof(*tdls_soc_obj));
	if (tdls_soc_obj == NULL) {
		tdls_err("Failed to allocate memory for tdls object");
		return QDF_STATUS_E_NOMEM;
	}

	tdls_soc_obj->soc = psoc;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_TDLS,
						       (void *)tdls_soc_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to attach psoc tdls component");
		qdf_mem_free(tdls_soc_obj);
	}

	tdls_notice("TDLS obj attach to psoc successfully");

	return status;
}

QDF_STATUS tdls_psoc_obj_destroy_notification(struct wlan_objmgr_psoc *psoc,
					      void *arg_list)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *tdls_soc_obj;

	tdls_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						WLAN_UMAC_COMP_TDLS);
	if (tdls_soc_obj == NULL) {
		tdls_err("Failed to get tdls obj in psoc");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_TDLS,
						       tdls_soc_obj);

	if (QDF_IS_STATUS_ERROR(status))
		tdls_err("Failed to detach psoc tdls component");

	qdf_mem_free(tdls_soc_obj);

	return status;
}

static void tdls_vdev_init(struct tdls_vdev_priv_obj *vdev)
{
	uint8_t i;

	for (i = 0; i < WLAN_TDLS_PEER_LIST_SIZE; i++) {
		qdf_list_create(&vdev->peer_list[i],
				WLAN_TDLS_PEER_SUB_LIST_SIZE);
	}

	qdf_mc_timer_init(&vdev->peer_update_timer, QDF_TIMER_TYPE_SW,
			  NULL, vdev);
	qdf_mc_timer_init(&vdev->peer_update_timer, QDF_TIMER_TYPE_SW,
			  NULL, vdev);
}

QDF_STATUS tdls_vdev_obj_create_notification(struct wlan_objmgr_vdev *vdev,
					     void *arg)
{
	QDF_STATUS status;
	struct tdls_vdev_priv_obj *tdls_vdev_obj;

	tdls_notice("tdls vdev mode %d", wlan_vdev_mlme_get_opmode(vdev));
	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE &&
	    wlan_vdev_mlme_get_opmode(vdev) != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	/* TODO: Add concurrency check */

	tdls_vdev_obj = qdf_mem_malloc(sizeof(*tdls_vdev_obj));
	if (tdls_vdev_obj == NULL) {
		tdls_err("Failed to allocate memory for tdls vdev object");
		return QDF_STATUS_E_NOMEM;
	}

	tdls_vdev_obj->vdev = vdev;
	tdls_vdev_init(tdls_vdev_obj);

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
						       WLAN_UMAC_COMP_TDLS,
						       (void *)tdls_vdev_obj,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to attach vdev tdls component");
		qdf_mem_free(tdls_vdev_obj);
	}

	tdls_notice("tdls object attach to vdev successfully");

	return status;
}

QDF_STATUS tdls_vdev_obj_destroy_notification(struct wlan_objmgr_vdev *vdev,
					      void *arg)
{
	QDF_STATUS status;
	void *tdls_vdev_obj;

	tdls_notice("tdls vdev mode %d", wlan_vdev_mlme_get_opmode(vdev));
	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE &&
	    wlan_vdev_mlme_get_opmode(vdev) != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	tdls_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							WLAN_UMAC_COMP_TDLS);
	if (tdls_vdev_obj == NULL) {
		tdls_err("Failed to get tdls vdev object");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_vdev_component_obj_detach(vdev,
						       WLAN_UMAC_COMP_TDLS,
						       tdls_vdev_obj);
	if (QDF_IS_STATUS_ERROR(status))
		tdls_err("Failed to detach vdev tdls component");

	qdf_mem_free(tdls_vdev_obj);

	return status;
}

/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implement various notification handlers which are accessed
 * internally in pkt_capture component only.
 */

#include "wlan_pkt_capture_main.h"

/**
 * pkt_capture_vdev_create_notification() - Handler for vdev create notify.
 * @vdev: vdev which is going to be created by objmgr
 * @arg: argument for notification handler.
 *
 * Allocate and attach vdev private object.
 *
 * Return: QDF_STATUS status in case of success else return error.
 */
QDF_STATUS
pkt_capture_vdev_create_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct pkt_capture_vdev_priv *vdev_priv;
	QDF_STATUS status;

	vdev_priv = qdf_mem_malloc(sizeof(*vdev_priv));
	if (!vdev_priv) {
		status = QDF_STATUS_E_NOMEM;
		goto exit;
	}

	status = wlan_objmgr_vdev_component_obj_attach(
					vdev,
					WLAN_UMAC_COMP_PKT_CAPTURE,
					(void *)vdev_priv, QDF_STATUS_SUCCESS);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		pkt_capture_err("Failed to attach priv with vdev");
		goto free_vdev_priv;
	}

	vdev_priv->vdev = vdev;
	goto exit;

free_vdev_priv:
	qdf_mem_free(vdev_priv);
	status = QDF_STATUS_E_INVAL;
exit:
	return status;
}

/**
 * pkt_capture_vdev_destroy_notification() - Handler for vdev destroy notify.
 * @vdev: vdev which is going to be destroyed by objmgr
 * @arg: argument for notification handler.
 *
 * Deallocate and detach vdev private object.
 *
 * Return QDF_STATUS status in case of success else return error
 */
QDF_STATUS
pkt_capture_vdev_destroy_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct pkt_capture_vdev_priv *vdev_priv = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (!vdev_priv) {
		pkt_capture_err("vdev priv is NULL");
		goto exit;
	}

	status = wlan_objmgr_vdev_component_obj_detach(
					vdev,
					WLAN_UMAC_COMP_PKT_CAPTURE,
					(void *)vdev_priv);
	if (!QDF_IS_STATUS_SUCCESS(status))
		pkt_capture_err("Failed to detach priv with psoc");

	qdf_mem_free(vdev_priv);
exit:
	return status;
}

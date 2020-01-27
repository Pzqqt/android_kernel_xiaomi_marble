/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: Implement various notification handlers which are accessed
 * internally in ftm_timesync component only.
 */

#include "ftm_time_sync_main.h"

QDF_STATUS
ftm_timesync_vdev_create_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct ftm_timesync_vdev_priv *vdev_priv;
	QDF_STATUS status;

	vdev_priv = qdf_mem_malloc(sizeof(*vdev_priv));
	if (!vdev_priv) {
		status = QDF_STATUS_E_NOMEM;
		goto exit;
	}

	status = wlan_objmgr_vdev_component_obj_attach(
				vdev, WLAN_UMAC_COMP_FTM_TIME_SYNC,
				(void *)vdev_priv, QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		ftm_time_sync_err("Failed to attach priv with vdev");
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

QDF_STATUS
ftm_timesync_vdev_destroy_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct ftm_timesync_vdev_priv *vdev_priv = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	vdev_priv = ftm_timesync_vdev_get_priv(vdev);
	if (!vdev_priv) {
		ftm_time_sync_err("vdev priv is NULL");
		goto exit;
	}

	status = wlan_objmgr_vdev_component_obj_detach(
					vdev, WLAN_UMAC_COMP_FTM_TIME_SYNC,
					(void *)vdev_priv);
	if (QDF_IS_STATUS_ERROR(status))
		ftm_time_sync_err("Failed to detach priv with vdev");

	qdf_mem_free(vdev_priv);
	vdev_priv = NULL;

exit:
	return status;
}

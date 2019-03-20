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

/*
 * DOC: contains core scan function definitions
 */
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_utils_api.h>
#include "wlan_scan_main.h"

QDF_STATUS wlan_scan_psoc_created_notification(struct wlan_objmgr_psoc *psoc,
						void *arg_list)
{
	struct wlan_scan_obj *scan_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	scan_obj = qdf_mem_malloc_atomic(sizeof(struct wlan_scan_obj));
	if (!scan_obj) {
		scm_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	/* Attach scan private date to psoc */
	status = wlan_objmgr_psoc_component_obj_attach(psoc,
		WLAN_UMAC_COMP_SCAN, (void *)scan_obj,
		QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status))
		scm_err("Failed to attach psoc scan component");
	else
		scm_debug("Scan object attach to psoc successful");

	return status;
}

QDF_STATUS wlan_scan_psoc_destroyed_notification(
				struct wlan_objmgr_psoc *psoc,
				void *arg_list)
{
	void *scan_obj = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	scan_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
			WLAN_UMAC_COMP_SCAN);

	if (!scan_obj) {
		scm_err("Failed to detach scan in psoc ctx");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
		WLAN_UMAC_COMP_SCAN, scan_obj);
	if (QDF_IS_STATUS_ERROR(status))
		scm_err("Failed to detach psoc scan component");

	qdf_mem_free(scan_obj);

	return status;
}

QDF_STATUS wlan_scan_vdev_created_notification(struct wlan_objmgr_vdev *vdev,
	void *arg_list)
{
	struct scan_vdev_obj *scan_vdev_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	scan_vdev_obj = qdf_mem_malloc_atomic(sizeof(struct scan_vdev_obj));
	if (!scan_vdev_obj) {
		scm_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	/* Attach scan private date to vdev */
	status = wlan_objmgr_vdev_component_obj_attach(vdev,
		WLAN_UMAC_COMP_SCAN, (void *)scan_vdev_obj,
		QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		scm_err("Failed to attach vdev scan component");
		qdf_mem_free(scan_vdev_obj);
	} else {
		scm_debug("vdev scan object attach successful");
	}

	return status;
}

QDF_STATUS wlan_scan_vdev_destroyed_notification(
	struct wlan_objmgr_vdev *vdev,
	void *arg_list)
{
	void *scan_vdev_obj = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	scan_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
			WLAN_UMAC_COMP_SCAN);

	if (!scan_vdev_obj) {
		scm_err("Failed to detach scan in vdev ctx");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_vdev_component_obj_detach(vdev,
		WLAN_UMAC_COMP_SCAN, scan_vdev_obj);
	if (QDF_IS_STATUS_ERROR(status))
		scm_err("Failed to detach vdev scan component");

	qdf_mem_free(scan_vdev_obj);

	return status;
}

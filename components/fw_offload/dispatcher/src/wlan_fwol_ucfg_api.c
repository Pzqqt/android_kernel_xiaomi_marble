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
 * DOC: define internal APIs related to the fwol component
 */

#include "cfg_ucfg_api.h"
#include "wlan_fw_offload_main.h"
#include "wlan_fwol_ucfg_api.h"

QDF_STATUS ucfg_fwol_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = fwol_cfg_on_psoc_enable(psoc);
	if (QDF_IS_STATUS_ERROR(status))
		fwol_err("Failed to initialize FWOL CFG");

	return status;
}

void ucfg_fwol_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	/* Clear the FWOL CFG Structure */
}

/**
 * fwol_psoc_object_created_notification(): fwol psoc create handler
 * @psoc: psoc which is going to created by objmgr
 * @arg: argument for vdev create handler
 *
 * Register this api with objmgr to detect psoc is created
 *
 * Return QDF_STATUS status in case of success else return error
 */
static QDF_STATUS
fwol_psoc_object_created_notification(struct wlan_objmgr_psoc *psoc, void *arg)
{
	QDF_STATUS status;
	struct wlan_fwol_psoc_obj *fwol_obj;

	fwol_obj = qdf_mem_malloc(sizeof(*fwol_obj));
	if (!fwol_obj)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_FWOL,
						       fwol_obj,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("Failed to attach psoc_ctx with psoc");
		qdf_mem_free(fwol_obj);
	}

	return status;
}

/**
 * fwol_psoc_object_destroyed_notification(): fwol psoc delete handler
 * @psoc: psoc which is going to delete by objmgr
 * @arg: argument for vdev delete handler
 *
 * Register this api with objmgr to detect psoc is deleted
 *
 * Return QDF_STATUS status in case of success else return error
 */
static QDF_STATUS fwol_psoc_object_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct wlan_fwol_psoc_obj *fwol_obj;
	QDF_STATUS status;

	fwol_obj = fwol_get_psoc_obj(psoc);
	if (!fwol_obj)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_FWOL,
						       fwol_obj);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("Failed to detach psoc_ctx from psoc");
		return status;
	}

	qdf_mem_free(fwol_obj);

	return status;
}

QDF_STATUS ucfg_fwol_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("unable to register psoc create handle");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		fwol_err("unable to register psoc create handle");
		wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_created_notification,
			NULL);
	}

	return status;
}

void ucfg_fwol_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		fwol_err("unable to unregister psoc destroy handle");

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_FWOL,
			fwol_psoc_object_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status))
		fwol_err("unable to unregister psoc create handle");
}


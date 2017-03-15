/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
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
 * DOC: reg_main.c
 * This file provides the regulatory component initialization and
 * registration functions
 */

#include "qdf_status.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "scheduler_api.h"
#include "reg_db.h"
#include "reg_services.h"
#include "reg_priv.h"
#include "reg_main.h"

/**
 * wlan_regulatory_psoc_obj_created_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is created.
 *
 * Return: Success or Failure
 */
static QDF_STATUS wlan_regulatory_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg_obj;
	QDF_STATUS status;

	soc_reg_obj =
		qdf_mem_malloc(sizeof(*soc_reg_obj));

	if (NULL == soc_reg_obj) {
		reg_alert("Mem alloc failed for reg psoc priv obj");
		return QDF_STATUS_E_NOMEM;
	}

	soc_reg_obj->offload_enabled  = false;
	soc_reg_obj->psoc_ptr = psoc;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
			WLAN_UMAC_COMP_REGULATORY, soc_reg_obj,
			QDF_STATUS_SUCCESS);
	reg_info("reg psoc obj created with status %d", status);

	return status;
}

/**
 * wlan_regulatory_psoc_obj_destroyed_notification() - PSOC obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is deleted.
 *
 * Return: Success or Failure
 */
static QDF_STATUS  wlan_regulatory_psoc_obj_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_regulatory_psoc_priv_obj *soc_reg =
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);
	if (NULL == soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}
	status = wlan_objmgr_psoc_component_obj_detach(psoc,
			WLAN_UMAC_COMP_REGULATORY,
			soc_reg);
	if (status != QDF_STATUS_SUCCESS)
		reg_err("soc_reg private obj detach failed");
	reg_info("reg psoc obj deleted with status %d", status);
	qdf_mem_free(soc_reg);

	return status;
}

/**
 * wlan_regulatory_init() - init regulatory component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("Failed to register reg obj create handler");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("Failed to register reg obj delete handler");
		wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_REGULATORY,
				wlan_regulatory_psoc_obj_created_notification,
				NULL);
		return status;
	}

	reg_info("regulatory handlers registered with obj mgr");

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_regulatory_deinit() - deinit regulatory component
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_created_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS)
		reg_err("deregister fail for psoc create notif:%d",
				status);
	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_REGULATORY,
			wlan_regulatory_psoc_obj_destroyed_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		reg_err("deregister fail for psoc delete notif:%d",
				status);
		return status;
	}
	reg_alert("deregistered callbacks with obj mgr successfully");

	return QDF_STATUS_SUCCESS;
}


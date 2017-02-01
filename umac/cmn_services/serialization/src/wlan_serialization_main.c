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
 * DOC: wlan_serialization_main.c
 * This file defines the important functions pertinent to
 * serialization to initialize and de-initialize the
 * component.
 */
#include "qdf_status.h"
#include "qdf_list.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_rules_i.h"

/**
 * wlan_serialization_apply_rules_cb_init() - Apply rule callback init
 * @psoc: PSOC object
 *
 * Apply rules Command callback registration function is
 * called from component during its initialization
 * It initializes all cmd callback handler for given CMDIDX in
 * 1-D Array
 *
 * Return: QDF Status
 */
static QDF_STATUS
wlan_serialization_apply_rules_cb_init(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj =
		wlan_serialization_get_psoc_priv_obj(psoc);

	if (ser_soc_obj == NULL) {
		serialization_alert("Serialization PSOC private obj is NULL");
		return QDF_STATUS_E_PERM;
	}
	ser_soc_obj->apply_rules_cb[WLAN_SER_CMD_SCAN] = wlan_apply_scan_rules;

	return QDF_STATUS_SUCCESS;
}


/**
 * wlan_serialization_apply_rules_cb_deinit() - Apply rule callback deinit
 * @psoc: PSOC object
 *
 * Apply rules Command callback De-registration function
 * called from component during its initialization
 * It initializes all cmd callback handler for given CMDIDX in
 * 1-D Array
 *
 * Return: QDF Status
 */
static QDF_STATUS
wlan_serialization_apply_rules_cb_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj =
		wlan_serialization_get_psoc_priv_obj(psoc);
	uint8_t i;

	if (ser_soc_obj == NULL) {
		serialization_alert("Serialization PSOC private obj is NULL");
		return QDF_STATUS_E_PERM;
	}
	for (i = 0; i < WLAN_SER_CMD_MAX; i++)
		ser_soc_obj->apply_rules_cb[i] = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_serialization_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj =
		wlan_serialization_get_psoc_priv_obj(psoc);

	qdf_mem_free(ser_soc_obj->timers);
	ser_soc_obj->timers = NULL;
	ser_soc_obj->max_active_cmds = 0;
	status = wlan_serialization_apply_rules_cb_deinit(psoc);

	return status;
}

QDF_STATUS wlan_serialization_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	uint8_t pdev_count;
	QDF_STATUS status;
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj =
		wlan_serialization_get_psoc_priv_obj(psoc);

	/* TODO:Get WLAN_SERIALIZATION_MAX_ACTIVE_SCAN_CMDS frm service ready */
	pdev_count = wlan_psoc_get_pdev_count(psoc);
	ser_soc_obj->max_active_cmds = WLAN_SERIALIZATION_MAX_ACTIVE_SCAN_CMDS +
		pdev_count;
	ser_soc_obj->timers =
		qdf_mem_malloc(sizeof(struct wlan_serialization_timer) *
				ser_soc_obj->max_active_cmds);
	if (NULL == ser_soc_obj->timers) {
		serialization_alert("Mem alloc failed for ser timers");
		return QDF_STATUS_E_NOMEM;
	}
	status = wlan_serialization_apply_rules_cb_init(psoc);

	return status;
}

QDF_STATUS wlan_serialization_psoc_obj_create_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	struct wlan_serialization_psoc_priv_obj *soc_ser_obj;

	soc_ser_obj =
		qdf_mem_malloc(sizeof(*soc_ser_obj));
	if (NULL == soc_ser_obj) {
		serialization_alert("Mem alloc failed for ser psoc priv obj");
		return QDF_STATUS_E_NOMEM;
	}
	wlan_objmgr_psoc_component_obj_attach(psoc,
			WLAN_UMAC_COMP_SERIALIZATION, (void *)soc_ser_obj,
			QDF_STATUS_SUCCESS);
	serialization_info("ser psoc obj created");

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_serialization_destroy_cmd_pool() - Destroy the global cmd pool
 * @ser_pdev_obj: Serialization private pdev object
 *
 * Return: None
 */
static void wlan_serialization_destroy_cmd_pool(
		  struct wlan_serialization_pdev_priv_obj *ser_pdev_obj)
{

	qdf_list_node_t *node = NULL;
	struct wlan_serialization_command_list *cmd_list_node;

	while (!qdf_list_empty(&ser_pdev_obj->global_cmd_pool_list)) {
		qdf_list_remove_front(&ser_pdev_obj->global_cmd_pool_list,
				&node);
		cmd_list_node = (struct wlan_serialization_command_list *)node;
		serialization_info("Node being freed from global pool %p",
				cmd_list_node);
		qdf_mem_free(cmd_list_node);

	}
	qdf_list_destroy(&ser_pdev_obj->global_cmd_pool_list);
}

/**
 * wlan_serialization_create_cmd_pool() - Create the global cmd pool
 * @pdev: PDEV Object
 * @ser_pdev_obj: Serialization private pdev object
 *
 * Global command pool of memory is created here.
 * It is safe to allocate memory individually for each command rather than
 * requesting for a huge chunk of memory at once.
 *
 * The individual command nodes allocated above will keep moving between
 * the active, pending and global pool lists dynamically, but all the
 * memory will be freed during driver unload only.
 *
 * Return: QDF Status
 */
static QDF_STATUS
wlan_serialization_create_cmd_pool(struct wlan_objmgr_pdev *pdev,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj)
{
	struct wlan_serialization_command_list *cmd_list_ptr;
	uint8_t i;

	for (i = 0; i < WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS; i++) {
		cmd_list_ptr = qdf_mem_malloc(sizeof(*cmd_list_ptr));
		if (NULL == cmd_list_ptr) {
			serialization_alert("Mem alloc failed for cmd node");
			wlan_serialization_destroy_cmd_pool(ser_pdev_obj);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_list_insert_back(
				&ser_pdev_obj->global_cmd_pool_list,
				&cmd_list_ptr->node);
		serialization_info("Created node at %p and inserted to pool",
				cmd_list_ptr);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_serialization_pdev_obj_create_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	QDF_STATUS status;

	ser_pdev_obj =
		qdf_mem_malloc(sizeof(*ser_pdev_obj));
	if (NULL == ser_pdev_obj) {
		serialization_alert("Mem alloc failed for ser pdev obj");
		return QDF_STATUS_E_NOMEM;
	}
	qdf_list_create(&ser_pdev_obj->active_list,
			WLAN_SERIALIZATION_MAX_ACTIVE_CMDS);
	qdf_list_create(&ser_pdev_obj->pending_list,
			WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS);
	qdf_list_create(&ser_pdev_obj->active_scan_list,
			WLAN_SERIALIZATION_MAX_ACTIVE_SCAN_CMDS);
	qdf_list_create(&ser_pdev_obj->pending_scan_list,
			WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS);
	qdf_list_create(&ser_pdev_obj->global_cmd_pool_list,
			WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS);
	status = wlan_serialization_create_cmd_pool(pdev, ser_pdev_obj);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("ser_pdev_obj failed status %d", status);
		return status;
	}
	status = wlan_objmgr_pdev_component_obj_attach(pdev,
		WLAN_UMAC_COMP_SERIALIZATION, (void *)ser_pdev_obj,
		QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("serialization pdev obj attach failed");
		return status;
	}

	return status;
}

QDF_STATUS  wlan_serialization_psoc_obj_destroy_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj =
		wlan_serialization_get_psoc_priv_obj(psoc);

	if (NULL == ser_soc_obj) {
		serialization_err("ser psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}
	status = wlan_objmgr_psoc_component_obj_detach(psoc,
			WLAN_UMAC_COMP_SERIALIZATION,
			ser_soc_obj);
	if (status != QDF_STATUS_SUCCESS)
		serialization_err("ser psoc private obj detach failed");
	serialization_info("ser psoc obj deleted with status %d", status);
	qdf_mem_free(ser_soc_obj);

	return status;
}

QDF_STATUS wlan_serialization_pdev_obj_destroy_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj =
		wlan_serialization_get_pdev_priv_obj(pdev);

	status = wlan_objmgr_pdev_component_obj_detach(pdev,
			WLAN_UMAC_COMP_SERIALIZATION,
			(void *)ser_pdev_obj);
	wlan_serialization_destroy_list(ser_pdev_obj,
					&ser_pdev_obj->active_list);
	wlan_serialization_destroy_list(ser_pdev_obj,
					&ser_pdev_obj->pending_list);
	wlan_serialization_destroy_list(ser_pdev_obj,
					&ser_pdev_obj->active_scan_list);
	wlan_serialization_destroy_list(ser_pdev_obj,
					&ser_pdev_obj->pending_scan_list);
	wlan_serialization_destroy_cmd_pool(ser_pdev_obj);
	serialization_info("ser pdev obj deleted with status %d", status);
	qdf_mem_free(ser_pdev_obj);

	return status;
}

QDF_STATUS wlan_serialization_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_psoc_obj_create_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("Failed to reg soc ser obj create handler");
		goto err_psoc_create;
	}

	status = wlan_objmgr_register_psoc_delete_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_psoc_obj_destroy_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("Failed to reg soc ser obj delete handler");
		goto err_psoc_delete;
	}

	status = wlan_objmgr_register_pdev_create_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_pdev_obj_create_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("Failed to reg pdev ser obj create handler");
		goto err_pdev_create;
	}

	status = wlan_objmgr_register_pdev_delete_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_pdev_obj_destroy_notification, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("Failed to reg pdev ser obj delete handler");
		goto err_pdev_delete;
	}

	serialization_info("serialization handlers registered with obj mgr");

	return QDF_STATUS_SUCCESS;

err_pdev_delete:
	wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_pdev_obj_create_notification, NULL);
err_pdev_create:
	wlan_objmgr_unregister_psoc_delete_handler(WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_psoc_obj_destroy_notification, NULL);
err_psoc_delete:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_SERIALIZATION,
			 wlan_serialization_psoc_obj_create_notification, NULL);
err_psoc_create:
	return status;

}

QDF_STATUS wlan_serialization_deinit(void)
{
	QDF_STATUS status;
	QDF_STATUS ret_status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_psoc_obj_create_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("unreg fail for psoc ser obj create notf:%d",
				status);
		ret_status = QDF_STATUS_E_FAILURE;
	}
	status = wlan_objmgr_unregister_psoc_delete_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_psoc_obj_destroy_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("unreg fail for psoc ser obj destroy notf:%d",
				status);
		ret_status = QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_unregister_pdev_create_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_pdev_obj_create_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("unreg fail for pdev ser obj create notf:%d",
				status);
		ret_status = QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_unregister_pdev_delete_handler(
			WLAN_UMAC_COMP_SERIALIZATION,
			wlan_serialization_pdev_obj_destroy_notification,
			NULL);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("unreg fail for pdev ser destory notf:%d",
				status);
		ret_status = QDF_STATUS_E_FAILURE;
	}

	serialization_alert("deregistered callbacks with obj mgr successfully");

	return ret_status;
}

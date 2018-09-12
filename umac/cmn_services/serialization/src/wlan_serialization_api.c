/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_api.c
 * This file provides an interface for the external components
 * to utilize the services provided by the serialization
 * component.
 */

#ifdef CONFIG_SERIALIZATION_V1
/* Include files */
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"

bool wlan_serialization_is_cmd_present_in_pending_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd)
{
	if (!cmd) {
		serialization_err("invalid cmd");
		return false;
	}
	return wlan_serialization_is_cmd_present_queue(cmd, false);
}

bool wlan_serialization_is_cmd_present_in_active_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd)
{
	if (!cmd) {
		serialization_err("invalid cmd");
		return false;
	}
	return wlan_serialization_is_cmd_present_queue(cmd, true);
}

QDF_STATUS
wlan_serialization_register_apply_rules_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_apply_rules_cb cb)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmdtype(cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("invalid cmd_type %d",
				cmd_type);
		return status;
	}
	ser_soc_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	if (!ser_soc_obj) {
		serialization_err("invalid ser_soc_obj");
		return QDF_STATUS_E_FAILURE;
	}
	ser_soc_obj->apply_rules_cb[cmd_type] = cb;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_deregister_apply_rules_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmdtype(cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("invalid cmd_type %d",
				cmd_type);
		return status;
	}
	ser_soc_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	if (!ser_soc_obj) {
		serialization_err("invalid ser_soc_obj");
		return QDF_STATUS_E_FAILURE;
	}
	ser_soc_obj->apply_rules_cb[cmd_type] = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_register_comp_info_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_comp_info_cb cb)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmd(comp_id, cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("invalid comp_id %d or cmd_type %d",
				comp_id, cmd_type);
		return status;
	}
	ser_soc_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	if (!ser_soc_obj) {
		serialization_err("invalid ser_soc_obj");
		return QDF_STATUS_E_FAILURE;
	}
	ser_soc_obj->comp_info_cb[cmd_type][comp_id] = cb;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_deregister_comp_info_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmd(comp_id, cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("invalid comp_id %d or cmd_type %d",
				comp_id, cmd_type);
		return status;
	}
	ser_soc_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	if (!ser_soc_obj) {
		serialization_err("invalid ser_soc_obj");
		return QDF_STATUS_E_FAILURE;
	}
	ser_soc_obj->comp_info_cb[cmd_type][comp_id] = NULL;

	return QDF_STATUS_SUCCESS;
}

enum wlan_serialization_cmd_status
wlan_serialization_non_scan_cmd_status(struct wlan_objmgr_pdev *pdev,
		enum wlan_serialization_cmd_type cmd_id)
{
	serialization_enter();

	return WLAN_SER_CMD_NOT_FOUND;
}

enum wlan_serialization_cmd_status
wlan_serialization_cancel_request(
		struct wlan_serialization_queued_cmd_info *req)
{
	QDF_STATUS status;

	serialization_enter();
	if (!req) {
		serialization_err("given request is empty");
		return WLAN_SER_CMD_NOT_FOUND;
	}
	status = wlan_serialization_validate_cmd(req->requestor, req->cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("req is not valid");
		return WLAN_SER_CMD_NOT_FOUND;
	}

	return wlan_serialization_find_and_cancel_cmd(req);
}

void wlan_serialization_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd)
{
	QDF_STATUS status;

	serialization_enter();
	if (!cmd) {
		serialization_err("given request is empty");
		QDF_ASSERT(0);
		return;
	}
	status = wlan_serialization_validate_cmd(cmd->requestor, cmd->cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("cmd is not valid");
		QDF_ASSERT(0);
		return;
	}
	wlan_serialization_find_and_remove_cmd(cmd);

	return;
}

enum wlan_serialization_status
wlan_serialization_request(struct wlan_serialization_command *cmd)
{
	bool is_active_cmd_allowed;
	QDF_STATUS status;
	enum wlan_serialization_status serialization_status;
	uint8_t comp_id;
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	union wlan_serialization_rules_info info;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_serialization_command_list *cmd_list = NULL;

	serialization_enter();
	if (!cmd) {
		serialization_err("serialization cmd is null");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}
	status = wlan_serialization_validate_cmd(cmd->source, cmd->cmd_type);
	if (status != QDF_STATUS_SUCCESS) {
		serialization_err("cmd is not valid");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	ser_soc_obj = wlan_serialization_get_psoc_obj(cmd);
	if (!ser_soc_obj) {
		serialization_err("ser_soc_obj is invalid");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		serialization_err("pdev is invalid");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	ser_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_SERIALIZATION);
	if (!ser_pdev_obj) {
		serialization_err("Invalid ser_pdev_obj");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}
	/*
	 * Get Component Info callback by calling
	 * each registered module
	 */
	for (comp_id = 0; comp_id < WLAN_UMAC_COMP_ID_MAX; comp_id++) {
		if (!ser_soc_obj->comp_info_cb[cmd->cmd_type][comp_id])
			continue;
		(ser_soc_obj->comp_info_cb[cmd->cmd_type][comp_id])(cmd->vdev,
			&info);
		if (!ser_soc_obj->apply_rules_cb[cmd->cmd_type])
			continue;
		if (!ser_soc_obj->apply_rules_cb[cmd->cmd_type](&info, comp_id))
			return WLAN_SER_CMD_DENIED_RULES_FAILED;
	}

	is_active_cmd_allowed = wlan_serialization_is_active_cmd_allowed(cmd);
	serialization_status = wlan_serialization_enqueue_cmd(
				cmd, is_active_cmd_allowed, &cmd_list);
	if (WLAN_SER_CMD_ACTIVE == serialization_status)
		wlan_serialization_activate_cmd(cmd_list, ser_pdev_obj);

	return serialization_status;
}

enum wlan_serialization_cmd_status
wlan_serialization_vdev_scan_status(struct wlan_objmgr_vdev *vdev)
{
	bool cmd_in_active, cmd_in_pending;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj =
		wlan_serialization_get_pdev_priv_obj(pdev);

	cmd_in_active =
	wlan_serialization_is_cmd_in_vdev_list(
			vdev, &ser_pdev_obj->active_scan_list);

	cmd_in_pending =
	wlan_serialization_is_cmd_in_vdev_list(
			vdev, &ser_pdev_obj->pending_scan_list);

	return wlan_serialization_is_cmd_in_active_pending(
			cmd_in_active, cmd_in_pending);
}

void wlan_serialization_flush_cmd(
		struct wlan_serialization_queued_cmd_info *cmd)
{
	serialization_enter();
	if (!cmd) {
		serialization_err("cmd is null, can't flush");
		return;
	}
	/* TODO: discuss and fill this API later */

	return;
}

enum wlan_serialization_cmd_status
wlan_serialization_pdev_scan_status(struct wlan_objmgr_pdev *pdev)
{
	bool cmd_in_active, cmd_in_pending;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj =
		wlan_serialization_get_pdev_priv_obj(pdev);

	cmd_in_active = !qdf_list_empty(&ser_pdev_obj->active_scan_list);
	cmd_in_pending = !qdf_list_empty(&ser_pdev_obj->pending_scan_list);

	return wlan_serialization_is_cmd_in_active_pending(
			cmd_in_active, cmd_in_pending);
}

struct wlan_serialization_command*
wlan_serialization_get_scan_cmd_using_scan_id(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id, uint16_t scan_id,
		uint8_t is_scan_cmd_from_active_queue)
{
	uint32_t qlen;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;

	if (!psoc) {
		serialization_err("invalid psoc");
		return cmd;
	}
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						WLAN_SERIALIZATION_ID);
	if (!vdev) {
		serialization_err("invalid vdev");
		return cmd;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		serialization_err("invalid pdev");
		goto release_vdev_ref;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj(pdev);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		goto release_vdev_ref;
	}
	if (is_scan_cmd_from_active_queue)
		queue = &ser_pdev_obj->active_scan_list;
	else
		queue = &ser_pdev_obj->pending_scan_list;
	qlen = wlan_serialization_list_size(queue, ser_pdev_obj);
	while (qlen--) {
		if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(
						queue, &nnode, ser_pdev_obj)) {
			serialization_debug("Node not found");
			break;
		}
		if (wlan_serialization_match_cmd_scan_id(nnode, &cmd, scan_id,
							 vdev, ser_pdev_obj)) {
			serialization_debug("Cmd matched with the scan_id");
			break;
		}
	}
release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);

	return cmd;
}

void *wlan_serialization_get_active_cmd(struct wlan_objmgr_psoc *psoc,
				      uint8_t vdev_id,
				      enum wlan_serialization_cmd_type cmd_type)
{
	uint32_t qlen;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	void *umac_cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;

	if (!psoc) {
		serialization_err("invalid psoc");
		return umac_cmd;
	}
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_SERIALIZATION_ID);
	if (!vdev) {
		serialization_err("invalid vdev");
		return umac_cmd;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		serialization_err("invalid pdev");
		goto release_vdev_ref;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj(pdev);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		goto release_vdev_ref;
	}

	queue = &ser_pdev_obj->active_list;

	qlen = qdf_list_size(queue);
	if (!qlen) {
		serialization_err("Empty Queue");
		goto release_vdev_ref;
	}
	while (qlen--) {
		if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(
							queue, &nnode,
							ser_pdev_obj)) {
			serialization_err("unsuccessful attempt");
			break;
		}
		cmd_list = qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 node);
		if (cmd_list->cmd.cmd_type == cmd_type &&
		    cmd_list->cmd.vdev == vdev) {
			serialization_debug("cmd_type[%d] matched", cmd_type);
			umac_cmd = cmd_list->cmd.umac_cmd;
			break;
		}
	}
release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);

	return umac_cmd;
}
#else /* New serialization code*/
/* Include files */
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_queue_i.h"
#include "wlan_serialization_scan_i.h"
#include "wlan_serialization_internal_i.h"

bool wlan_serialization_is_cmd_present_in_pending_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd)
{
	bool status = false;

	if (!cmd) {
		ser_err("invalid cmd");
		goto error;
	}

	status = wlan_serialization_is_cmd_present_queue(cmd, false);

error:
	return status;
}

bool wlan_serialization_is_cmd_present_in_active_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd)
{
	bool status;

	if (!cmd) {
		ser_err("invalid cmd");
		status = false;
		goto error;
	}

	status = wlan_serialization_is_cmd_present_queue(cmd, true);

	ser_debug("Cmd type:%d id:%d present: %d",
		  cmd->cmd_type, cmd->cmd_id, status);

error:
	return status;
}

QDF_STATUS
wlan_serialization_register_apply_rules_cb(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_apply_rules_cb cb)
{
	struct wlan_ser_psoc_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmdtype(cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("invalid cmd_type %d", cmd_type);
		goto error;
	}

	ser_soc_obj = wlan_serialization_get_psoc_obj(psoc);
	if (!ser_soc_obj) {
		ser_err("invalid ser_soc_obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	ser_soc_obj->apply_rules_cb[cmd_type] = cb;
	status = QDF_STATUS_SUCCESS;

error:
	return status;
}

QDF_STATUS
wlan_serialization_deregister_apply_rules_cb(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_ser_psoc_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmdtype(cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("invalid cmd_type %d", cmd_type);
		goto error;
	}
	ser_soc_obj = wlan_serialization_get_psoc_obj(psoc);
	if (!ser_soc_obj) {
		ser_err("invalid ser_soc_obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	ser_soc_obj->apply_rules_cb[cmd_type] = NULL;
	status = QDF_STATUS_SUCCESS;

error:
	return status;
}

QDF_STATUS
wlan_serialization_register_comp_info_cb(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_comp_info_cb cb)
{
	struct wlan_ser_psoc_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmd(comp_id, cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("invalid comp_id %d or cmd_type %d",
			comp_id, cmd_type);
		goto error;
	}
	ser_soc_obj = wlan_serialization_get_psoc_obj(psoc);
	if (!ser_soc_obj) {
		ser_err("invalid ser_soc_obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	ser_soc_obj->comp_info_cb[cmd_type][comp_id] = cb;
	status = QDF_STATUS_SUCCESS;

error:
	return status;
}

QDF_STATUS
wlan_serialization_deregister_comp_info_cb(struct wlan_objmgr_psoc *psoc,
					   enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_ser_psoc_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmd(comp_id, cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("invalid comp_id %d or cmd_type %d",
			comp_id, cmd_type);
		goto error;
	}
	ser_soc_obj = wlan_serialization_get_psoc_obj(psoc);
	if (!ser_soc_obj) {
		ser_err("invalid ser_soc_obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}
	ser_soc_obj->comp_info_cb[cmd_type][comp_id] = NULL;
	status = QDF_STATUS_SUCCESS;

error:
	return status;
}

enum wlan_serialization_cmd_status
wlan_serialization_non_scan_cmd_status(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_serialization_cmd_type cmd_type)
{
	bool cmd_in_active = 0;
	bool cmd_in_pending = 0;
	struct wlan_ser_pdev_obj *ser_pdev_obj =
		wlan_serialization_get_pdev_obj(pdev);
	enum wlan_serialization_cmd_status cmd_status = WLAN_SER_CMD_NOT_FOUND;
	struct wlan_serialization_pdev_queue *pdev_q;
	qdf_list_node_t *node = NULL;
	qdf_list_t *queue = NULL;

	ser_enter();

	pdev_q = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];

	/* Look in the pdev non scan active queue */
	queue = &pdev_q->active_list;

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_CMD_TYPE,
			NULL, cmd_type, NULL, NULL,  WLAN_SER_PDEV_NODE);

	if (node)
		cmd_in_active = true;

	node = NULL;

	/* Look in the pdev non scan pending queue */
	queue = &pdev_q->pending_list;

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_CMD_TYPE,
			NULL, cmd_type, NULL, NULL,  WLAN_SER_PDEV_NODE);

	if (node)
		cmd_in_pending = true;

	cmd_status = wlan_serialization_is_cmd_in_active_pending(
			cmd_in_active, cmd_in_pending);

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

	ser_exit();
	return cmd_status;
}

enum wlan_serialization_cmd_status
wlan_serialization_cancel_request(
		struct wlan_serialization_queued_cmd_info *req)
{
	QDF_STATUS status;
	enum wlan_serialization_cmd_status cmd_status;

	struct wlan_serialization_command cmd;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_pdev_queue *pdev_queue;

	ser_enter();

	if (!req) {
		ser_err("given request is empty");
		cmd_status = WLAN_SER_CMD_NOT_FOUND;
		goto error;
	}

	status = wlan_serialization_validate_cmd(req->requestor, req->cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("req is not valid");
		cmd_status = WLAN_SER_CMD_NOT_FOUND;
		goto error;
	}

	cmd.cmd_type = req->cmd_type;
	cmd.cmd_id = req->cmd_id;
	cmd.source = req->requestor;
	cmd.vdev = req->vdev;

	pdev = wlan_serialization_get_pdev_from_cmd(&cmd);
	if (!pdev) {
		ser_err("pdev is invalid");
		cmd_status = WLAN_SER_CMD_NOT_FOUND;
		goto error;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);

	pdev_queue = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj,
							   cmd.cmd_type);

	if (!pdev_queue) {
		ser_err("pdev_queue is invalid");
		cmd_status = WLAN_SER_CMD_NOT_FOUND;
		goto error;
	}

	cmd_status = wlan_serialization_find_and_cancel_cmd(
			&cmd, req->req_type, req->queue_type);

error:
	ser_exit();
	return cmd_status;
}

void wlan_serialization_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd_info)
{
	QDF_STATUS status;
	struct wlan_serialization_command cmd = {0};

	ser_enter();

	if (!cmd_info) {
		ser_err("given request is empty");
		QDF_ASSERT(0);
		return;
	}
	status = wlan_serialization_validate_cmd(cmd_info->requestor,
						 cmd_info->cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("cmd is not valid");
		QDF_ASSERT(0);
		goto error;
	}

	cmd.cmd_type = cmd_info->cmd_type;
	cmd.cmd_id = cmd_info->cmd_id;
	cmd.source = cmd_info->requestor;
	cmd.vdev = cmd_info->vdev;

	if (wlan_serialization_dequeue_cmd(&cmd, true) !=
			WLAN_SER_CMD_IN_ACTIVE_LIST) {
		ser_err("Can't dequeue requested cmd_id[%d] type[%d]",
			cmd.cmd_id, cmd.cmd_type);
	}

error:
	ser_exit();
}

enum wlan_serialization_status
wlan_serialization_request(struct wlan_serialization_command *cmd)
{
	QDF_STATUS status;
	enum wlan_serialization_status serialization_status;
	uint8_t comp_id;
	struct wlan_ser_psoc_obj *ser_soc_obj;
	union wlan_serialization_rules_info info;
	struct wlan_objmgr_psoc *psoc;

	ser_enter();

	serialization_status = WLAN_SER_CMD_DENIED_UNSPECIFIED;

	if (!cmd) {
		ser_err("serialization cmd is null");
		goto error;
	}
	status = wlan_serialization_validate_cmd(cmd->source, cmd->cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("cmd is not valid");
		goto error;
	}

	psoc = wlan_serialization_get_psoc_from_cmd(cmd);
	if (!psoc) {
		ser_err("psoc _obj is invalid");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}
	ser_soc_obj = wlan_serialization_get_psoc_obj(psoc);

	if (!ser_soc_obj) {
		ser_err("ser_soc_obj is invalid");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	/*
	 * Get Component Info callback by calling
	 * each registered module
	 */
	for (comp_id = 0; comp_id < WLAN_UMAC_COMP_ID_MAX; comp_id++) {
		if (!ser_soc_obj->comp_info_cb[cmd->cmd_type][comp_id])
			continue;
		ser_soc_obj->comp_info_cb[cmd->cmd_type][comp_id](cmd->vdev,
			&info);
		if (!ser_soc_obj->apply_rules_cb[cmd->cmd_type])
			continue;
		if (!ser_soc_obj->apply_rules_cb[cmd->cmd_type](&info, comp_id))
			return WLAN_SER_CMD_DENIED_RULES_FAILED;
	}

	serialization_status = wlan_serialization_enqueue_cmd(cmd);

error:
	ser_exit();
	return serialization_status;
}

enum wlan_serialization_cmd_status
wlan_serialization_vdev_scan_status(struct wlan_objmgr_vdev *vdev)
{
	bool cmd_in_active = 0, cmd_in_pending = 0;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	struct wlan_ser_pdev_obj *ser_pdev_obj =
		wlan_serialization_get_pdev_obj(pdev);
	struct wlan_serialization_pdev_queue *pdev_q;
	enum wlan_serialization_cmd_status status;

	ser_enter();

	pdev_q = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

	cmd_in_active =
	wlan_serialization_is_cmd_in_vdev_list(
			vdev, &pdev_q->active_list, WLAN_SER_PDEV_NODE);

	cmd_in_pending =
	wlan_serialization_is_cmd_in_vdev_list(
			vdev, &pdev_q->pending_list, WLAN_SER_PDEV_NODE);

	status = wlan_serialization_is_cmd_in_active_pending(
			cmd_in_active, cmd_in_pending);

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);
	ser_exit();

	return status;
}

void wlan_serialization_flush_cmd(
		struct wlan_serialization_queued_cmd_info *cmd)
{
	ser_enter();

	if (!cmd) {
		ser_err("cmd is null, can't flush");
		goto error;
	}

error:
	ser_exit();
}

enum wlan_serialization_cmd_status
wlan_serialization_pdev_scan_status(struct wlan_objmgr_pdev *pdev)
{
	bool cmd_in_active, cmd_in_pending;
	struct wlan_ser_pdev_obj *ser_pdev_obj =
		wlan_serialization_get_pdev_obj(pdev);
	struct wlan_serialization_pdev_queue *pdev_q;
	enum wlan_serialization_cmd_status status;

	pdev_q = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

	cmd_in_active = !qdf_list_empty(&pdev_q->active_list);
	cmd_in_pending = !qdf_list_empty(&pdev_q->pending_list);

	status = wlan_serialization_is_cmd_in_active_pending(
			cmd_in_active, cmd_in_pending);

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

	return status;
}

struct wlan_serialization_command*
wlan_serialization_get_scan_cmd_using_scan_id(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id, uint16_t scan_id,
		uint8_t is_scan_cmd_from_active_queue)
{
	uint32_t qlen;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;
	struct wlan_serialization_pdev_queue *pdev_q;

	if (!psoc) {
		ser_err("invalid psoc");
		goto error;
	}
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_SERIALIZATION_ID);
	if (!vdev) {
		ser_err("invalid vdev");
		goto error;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		ser_err("invalid pdev");
		goto release_vdev_ref;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		goto release_vdev_ref;
	}

	pdev_q = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

	if (is_scan_cmd_from_active_queue)
		queue = &pdev_q->active_list;
	else
		queue = &pdev_q->pending_list;

	qlen = wlan_serialization_list_size(queue);

	while (qlen--) {
		if (QDF_IS_STATUS_ERROR(wlan_serialization_get_cmd_from_queue(
					queue, &nnode))) {
			ser_debug("Node not found");
			break;
		}
		if (wlan_ser_match_cmd_scan_id(nnode, &cmd, scan_id,
					       vdev)) {
			break;
		}
	}

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
error:
	return cmd;
}

void *wlan_serialization_get_active_cmd(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	void *umac_cmd = NULL;
	qdf_list_node_t *node = NULL;
	qdf_list_t *queue;
	struct wlan_serialization_pdev_queue *pdev_q;

	ser_enter();

	if (!psoc) {
		ser_err("invalid psoc");
		goto error;
	}
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_SERIALIZATION_ID);
	if (!vdev) {
		ser_err("invalid vdev");
		goto error;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		ser_err("invalid pdev");
		goto release_vdev_ref;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		goto release_vdev_ref;
	}

	pdev_q = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj, cmd_type);

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

	queue = &pdev_q->active_list;

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_CMD_TYPE_VDEV,
			NULL, cmd_type, NULL, vdev,  WLAN_SER_PDEV_NODE);

	if (node) {
		cmd_list = qdf_container_of(
				node,
				struct wlan_serialization_command_list,
				pdev_node);

		umac_cmd = cmd_list->cmd.umac_cmd;
	}

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
error:
	ser_exit();
	return umac_cmd;
}
#endif

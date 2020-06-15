/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_pdev_queue *pdev_queue;

	if (!cmd) {
		ser_err("invalid cmd");
		goto error;
	}

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		ser_err("invalid pdev");
		goto error;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser pdev obj");
		goto error;
	}

	pdev_queue = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj,
							   cmd->cmd_type);
	if (!pdev_queue) {
		ser_err("pdev_queue is invalid");
		goto error;
	}

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	status = wlan_serialization_is_cmd_present_queue(cmd, false);

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
error:
	return status;
}

bool wlan_serialization_is_cmd_present_in_active_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd)
{
	bool status = false;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_pdev_queue *pdev_queue;

	if (!cmd) {
		ser_err("invalid cmd");
		goto error;
	}

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		ser_err("invalid pdev");
		goto error;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser pdev obj");
		goto error;
	}

	pdev_queue = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj,
							   cmd->cmd_type);
	if (!pdev_queue) {
		ser_err("pdev_queue is invalid");
		goto error;
	}

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	status = wlan_serialization_is_cmd_present_queue(cmd, true);

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

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

	ser_debug("Type %d id %d source %d req type %d queue type %d",
		  cmd.cmd_type, cmd.cmd_id, cmd.source, req->req_type,
		  req->queue_type);
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

	return cmd_status;
}

void wlan_serialization_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd_info)
{
	QDF_STATUS status;
	enum wlan_serialization_cmd_status ser_status;
	struct wlan_serialization_command cmd = {0};

	if (!cmd_info) {
		ser_err("given request is empty");
		QDF_ASSERT(0);
		return;
	}
	status = wlan_serialization_validate_cmd(cmd_info->requestor,
						 cmd_info->cmd_type);
	if (QDF_IS_STATUS_ERROR(status)) {
		ser_err("cmd type %d is not valid", cmd_info->cmd_type);
		QDF_ASSERT(0);
		return;
	}

	cmd.cmd_type = cmd_info->cmd_type;
	cmd.cmd_id = cmd_info->cmd_id;
	cmd.source = cmd_info->requestor;
	cmd.vdev = cmd_info->vdev;

	ser_status = wlan_serialization_dequeue_cmd(
			&cmd, SER_REMOVE, true);

	if (ser_status != WLAN_SER_CMD_IN_ACTIVE_LIST) {
		if (ser_status != WLAN_SER_CMD_MARKED_FOR_ACTIVATION)
			ser_debug("Can't dequeue requested id %d type %d requestor %d",
				  cmd.cmd_id, cmd.cmd_type,
				  cmd_info->requestor);
	}
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

	serialization_status = WLAN_SER_CMD_DENIED_UNSPECIFIED;

	if (!cmd) {
		ser_err("serialization cmd is null");
		goto error;
	}
	status = wlan_serialization_validate_cmd(cmd->source, cmd->cmd_type);
	if (QDF_IS_STATUS_ERROR(status))
		goto error;

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
			&info, cmd);
		if (!ser_soc_obj->apply_rules_cb[cmd->cmd_type])
			continue;
		if (!ser_soc_obj->apply_rules_cb[cmd->cmd_type](&info, comp_id))
			return WLAN_SER_CMD_DENIED_RULES_FAILED;
	}

	serialization_status = wlan_serialization_enqueue_cmd(cmd, SER_REQUEST);

error:
	return serialization_status;
}

QDF_STATUS
wlan_serialization_update_timer(struct wlan_serialization_command *cmd)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;

	if (!cmd) {
		ser_err("NULL command");
		goto error;
	}

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		ser_err("invalid pdev");
		goto error;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		ser_err("invalid psoc");
		goto error;
	}

	status = wlan_serialization_find_and_update_timer(psoc, cmd);

error:
	return status;
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

	return status;
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
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command cmd = {0};
	struct wlan_serialization_command *pcmd = NULL;
	struct wlan_serialization_command_list *cmd_list;
	qdf_list_node_t *node = NULL;
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

	cmd.cmd_type = WLAN_SER_CMD_SCAN;
	cmd.cmd_id = scan_id;
	cmd.vdev = vdev;

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_CMD_ID_VDEV,
			&cmd, 0, NULL, vdev,  WLAN_SER_PDEV_NODE);

	if (node) {
		cmd_list = qdf_container_of(
				node,
				struct wlan_serialization_command_list,
				pdev_node);

		pcmd = &cmd_list->cmd;
	}

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
error:
	return pcmd;
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

	return umac_cmd;
}

enum wlan_serialization_cmd_type
wlan_serialization_get_vdev_active_cmd_type(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_serialization_cmd_type cmd_type = WLAN_SER_CMD_MAX;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	struct wlan_serialization_pdev_queue *pdev_queue;
	struct wlan_serialization_vdev_queue *vdev_queue;
	struct wlan_serialization_command_list *cmd_list = NULL;
	qdf_list_node_t *node;

	ser_pdev_obj = wlan_serialization_get_pdev_obj(
			wlan_vdev_get_pdev(vdev));

	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		goto error;
	}
	pdev_queue = wlan_serialization_get_pdev_queue_obj(
			ser_pdev_obj, cmd_type);

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);
	if (!ser_vdev_obj) {
		ser_err("invalid ser_vdev_obj");
		goto error;
	}
	vdev_queue = wlan_serialization_get_vdev_queue_obj(
			ser_vdev_obj, WLAN_SER_CMD_NONSCAN);

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	if (wlan_serialization_peek_front(
		&vdev_queue->active_list, &node) == QDF_STATUS_SUCCESS) {
		cmd_list = qdf_container_of(
				node,
				struct wlan_serialization_command_list,
				vdev_node);

		cmd_type = cmd_list->cmd.cmd_type;
	}

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

error:
	return cmd_type;
}

bool wlan_ser_is_vdev_queue_enabled(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	struct wlan_serialization_vdev_queue *vdev_queue;

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);
	if (!ser_vdev_obj) {
		ser_err("invalid ser_vdev_obj");
		return false;
	}

	vdev_queue = wlan_serialization_get_vdev_queue_obj(
			ser_vdev_obj, WLAN_SER_CMD_NONSCAN);
	if (vdev_queue->queue_disable)
		return false;
	else
		return true;
}

QDF_STATUS
wlan_ser_get_cmd_activation_status(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	struct wlan_serialization_pdev_queue *pdev_queue;
	struct wlan_serialization_vdev_queue *vdev_queue;
	struct wlan_serialization_command_list *cmd_list = NULL;
	qdf_list_node_t *node;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	ser_pdev_obj = wlan_serialization_get_pdev_obj(
			wlan_vdev_get_pdev(vdev));

	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_queue = wlan_serialization_get_pdev_queue_obj(
			ser_pdev_obj, WLAN_SER_CMD_NONSCAN);

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);
	if (!ser_vdev_obj) {
		ser_err("invalid ser_vdev_obj");
		return QDF_STATUS_E_FAILURE;
	}
	vdev_queue = wlan_serialization_get_vdev_queue_obj(
			ser_vdev_obj, WLAN_SER_CMD_NONSCAN);

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	if (wlan_serialization_peek_front(
		&vdev_queue->active_list, &node) == QDF_STATUS_SUCCESS) {
		cmd_list = qdf_container_of(
				node,
				struct wlan_serialization_command_list,
				vdev_node);

		if (qdf_atomic_test_bit(CMD_MARKED_FOR_ACTIVATION,
					&cmd_list->cmd_in_use))
			status = QDF_STATUS_SUCCESS;
	}

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

	return status;
}

QDF_STATUS
wlan_ser_validate_umac_cmd(struct wlan_objmgr_vdev *vdev,
			   enum wlan_serialization_cmd_type cmd_type,
			   wlan_ser_umac_cmd_cb umac_cmd_cb)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	void *umac_cmd = NULL;
	qdf_list_node_t *node = NULL;
	qdf_list_t *queue;
	struct wlan_serialization_pdev_queue *pdev_q;
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (!vdev) {
		ser_err("invalid vdev");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		ser_err("invalid pdev");
		return QDF_STATUS_E_INVAL;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return QDF_STATUS_E_INVAL;
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
		status = umac_cmd_cb(umac_cmd);
	}

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

	return status;
}

void wlan_serialization_purge_all_pdev_cmd(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_ser_pdev_obj *ser_pdev_obj;

	if (!pdev) {
		ser_err("NULL pdev");
		return;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return;
	}

	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, NULL, NULL,
				 WLAN_SER_CMD_SCAN, false);
	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, NULL, NULL,
				 WLAN_SER_CMD_SCAN, true);
	wlan_ser_cancel_non_scan_cmd(ser_pdev_obj, pdev, NULL, NULL,
				     WLAN_SER_CMD_NONSCAN, false,
				     WLAN_SER_CMD_ATTR_NONE);
	wlan_ser_cancel_non_scan_cmd(ser_pdev_obj, pdev, NULL, NULL,
				     WLAN_SER_CMD_NONSCAN, true,
				     WLAN_SER_CMD_ATTR_NONE);
}

static inline
void wlan_ser_purge_pdev_cmd_cb(struct wlan_objmgr_psoc *psoc,
				void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;

	wlan_serialization_purge_all_pdev_cmd(pdev);
}

void wlan_serialization_purge_all_cmd(struct wlan_objmgr_psoc *psoc)
{
	wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
				     wlan_ser_purge_pdev_cmd_cb, NULL, 1,
				     WLAN_SERIALIZATION_ID);
}

void wlan_serialization_purge_all_pending_cmd_by_vdev_id(
					struct wlan_objmgr_pdev *pdev,
					uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;

	if (!pdev) {
		ser_err("Invalid pdev");
		return;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_SERIALIZATION_ID);
	if (!vdev) {
		ser_err("Invalid vdev");
		return;
	}

	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				 WLAN_SER_CMD_SCAN, false);
	wlan_ser_cancel_non_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				     WLAN_SER_CMD_NONSCAN, false,
				     WLAN_SER_CMD_ATTR_NONE);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
}

void wlan_serialization_purge_all_cmd_by_vdev_id(struct wlan_objmgr_pdev *pdev,
						 uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;

	if (!pdev) {
		ser_err("Invalid pdev");
		return;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_SERIALIZATION_ID);
	if (!vdev) {
		ser_err("Invalid vdev");
		return;
	}

	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				 WLAN_SER_CMD_SCAN, false);
	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				 WLAN_SER_CMD_SCAN, true);
	wlan_ser_cancel_non_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				     WLAN_SER_CMD_NONSCAN, false,
				     WLAN_SER_CMD_ATTR_NONE);
	wlan_ser_cancel_non_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				     WLAN_SER_CMD_NONSCAN, true,
				     WLAN_SER_CMD_ATTR_NONE);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
}

void wlan_serialization_purge_all_scan_cmd_by_vdev_id(
					struct wlan_objmgr_pdev *pdev,
					uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;

	if (!pdev) {
		ser_err("Invalid pdev");
		return;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_SERIALIZATION_ID);
	if (!vdev) {
		ser_err("Invalid vdev");
		return;
	}

	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				 WLAN_SER_CMD_SCAN, false);
	wlan_ser_cancel_scan_cmd(ser_pdev_obj, pdev, vdev, NULL,
				 WLAN_SER_CMD_SCAN, true);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
}

QDF_STATUS wlan_ser_vdev_queue_disable(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	struct wlan_serialization_vdev_queue *vdev_queue;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_pdev_queue *pdev_q;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		ser_err("invalid PDEV object");
		return QDF_STATUS_E_INVAL;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return QDF_STATUS_E_INVAL;
	}

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);
	if (!ser_vdev_obj) {
		ser_err("invalid ser_vdev_obj");
		return QDF_STATUS_E_INVAL;
	}

	pdev_q = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];

	vdev_queue = wlan_serialization_get_vdev_queue_obj(
			ser_vdev_obj, WLAN_SER_CMD_NONSCAN);
	if (!vdev_queue) {
		ser_err("invalid vdev_queue object");
		return QDF_STATUS_E_INVAL;
	}

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);
	vdev_queue->queue_disable = true;
	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);
	ser_debug("Disabling the serialization for vdev:%d",
		  wlan_vdev_get_id(vdev));

	return QDF_STATUS_SUCCESS;
}

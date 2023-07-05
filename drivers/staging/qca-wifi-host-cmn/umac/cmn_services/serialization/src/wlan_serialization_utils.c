/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_utils.c
 * This file defines the utility helper functions for serialization component.
 */

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_mc_timer.h>
#include <wlan_utility.h>
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_queue_i.h"
#include "wlan_serialization_api.h"

#ifndef WLAN_SER_DEBUG
void wlan_ser_update_cmd_history(
		struct wlan_serialization_pdev_queue *pdev_queue,
		struct wlan_serialization_command *cmd,
		enum ser_queue_reason ser_reason,
		bool add_remove,
		bool active_queue){ }
#endif

struct wlan_objmgr_pdev*
wlan_serialization_get_pdev_from_cmd(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_pdev *pdev = NULL;

	if (!cmd) {
		ser_err("invalid cmd");
		return pdev;
	}
	if (!cmd->vdev) {
		ser_err("invalid cmd->vdev");
		return pdev;
	}
	pdev = wlan_vdev_get_pdev(cmd->vdev);

	return pdev;
}

struct wlan_objmgr_psoc*
wlan_serialization_get_psoc_from_cmd(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_psoc *psoc = NULL;

	if (!cmd) {
		ser_err("invalid cmd");
		return psoc;
	}
	if (!cmd->vdev) {
		ser_err("invalid cmd->vdev");
		return psoc;
	}
	psoc = wlan_vdev_get_psoc(cmd->vdev);

	return psoc;
}

struct wlan_objmgr_vdev*
wlan_serialization_get_vdev_from_cmd(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_vdev *vdev = NULL;

	if (!cmd) {
		ser_err("invalid cmd");
		goto error;
	}

	vdev = cmd->vdev;

error:
	return vdev;
}

QDF_STATUS
wlan_serialization_get_cmd_from_queue(qdf_list_t *queue,
				      qdf_list_node_t **nnode)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	qdf_list_node_t *pnode;

	if (!queue) {
		ser_err("input parameters are invalid");
		goto error;
	}

	pnode = *nnode;
	if (!pnode)
		status = wlan_serialization_peek_front(queue, nnode);
	else
		status = wlan_serialization_peek_next(queue, pnode, nnode);

	if (status != QDF_STATUS_SUCCESS)
		ser_err("can't get next node from queue");

error:
	return status;
}

QDF_STATUS wlan_serialization_timer_destroy(
		struct wlan_serialization_timer *ser_timer)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!ser_timer || !ser_timer->cmd) {
		ser_debug("Invalid ser_timer");
		qdf_status =  QDF_STATUS_E_FAILURE;
		goto error;
	}
	/* Wait till timeout CB is completed */
	qdf_timer_sync_cancel(&ser_timer->timer);
	ser_timer->cmd = NULL;

error:
	return qdf_status;
}

/**
 * wlan_serialization_stop_timer() - to stop particular timer
 * @ser_timer: pointer to serialization timer
 *
 * This API stops the particular timer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_stop_timer(struct wlan_serialization_timer *ser_timer)
{
	wlan_serialization_timer_destroy(ser_timer);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_serialization_cleanup_vdev_timers(
			struct wlan_objmgr_vdev *vdev)
{
	struct wlan_ser_psoc_obj *psoc_ser_obj;
	struct wlan_serialization_timer *ser_timer;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i = 0;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_objmgr_psoc *psoc = NULL;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		QDF_BUG(0);
		ser_err("pdev is null");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		QDF_BUG(0);
		ser_err("psoc is null");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	psoc_ser_obj = wlan_serialization_get_psoc_obj(psoc);

	if (!psoc_ser_obj) {
		ser_err("Invalid psoc_ser_obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	wlan_serialization_acquire_lock(&psoc_ser_obj->timer_lock);

	for (i = 0; psoc_ser_obj->max_active_cmds > i; i++) {
		ser_timer = &psoc_ser_obj->timers[i];
		if (!ser_timer->cmd)
			continue;
		/*
		 * Check if the timer is for the given vdev
		 */
		if (ser_timer->cmd->vdev != vdev)
			continue;

		ser_debug("Stopping the timer for vdev id[%d]",
			  wlan_vdev_get_id(vdev));

		status = wlan_serialization_stop_timer(ser_timer);
		if (QDF_STATUS_SUCCESS != status) {
			/* lets not break the loop but report error */
			ser_err("some error in stopping timer");
		}
	}

	wlan_serialization_release_lock(&psoc_ser_obj->timer_lock);
error:
	return status;
}

QDF_STATUS wlan_serialization_cleanup_all_timers(
			struct wlan_ser_psoc_obj *psoc_ser_obj)
{
	struct wlan_serialization_timer *ser_timer;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t i = 0;

	if (!psoc_ser_obj) {
		ser_err("Invalid psoc_ser_obj");
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	wlan_serialization_acquire_lock(&psoc_ser_obj->timer_lock);

	for (i = 0; psoc_ser_obj->max_active_cmds > i; i++) {
		ser_timer = &psoc_ser_obj->timers[i];
		if (!ser_timer->cmd)
			continue;
		status = wlan_serialization_stop_timer(ser_timer);
		if (QDF_STATUS_SUCCESS != status) {
			/* lets not break the loop but report error */
			ser_err("some error in stopping timer");
		}
	}

	wlan_serialization_release_lock(&psoc_ser_obj->timer_lock);
error:

	return status;
}

QDF_STATUS wlan_serialization_validate_cmdtype(
		 enum wlan_serialization_cmd_type cmd_type)
{
	if (cmd_type < 0 || cmd_type >= WLAN_SER_CMD_MAX) {
		ser_err("Invalid cmd %d passed", cmd_type);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_serialization_validate_cmd(
		 enum wlan_umac_comp_id comp_id,
		 enum wlan_serialization_cmd_type cmd_type)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (cmd_type < 0 || comp_id < 0 || cmd_type >= WLAN_SER_CMD_MAX ||
	    comp_id >= WLAN_UMAC_COMP_ID_MAX) {
		ser_err("Invalid cmd or comp passed comp %d type %d",
			comp_id, cmd_type);
		goto error;
	}

	status = QDF_STATUS_SUCCESS;
error:
	return status;
}

QDF_STATUS wlan_serialization_validate_cmd_list(
		struct wlan_serialization_command_list *cmd_list)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	if (!cmd_list->cmd.cmd_cb) {
		ser_err("no cmd_cb for cmd type:%d, id: %d",
			cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
		QDF_ASSERT(0);
		goto error;
	}

	if (!cmd_list->cmd.vdev) {
		ser_err("invalid cmd.vdev");
		goto error;
	}

	status = QDF_STATUS_SUCCESS;

error:
	return status;
}

static void wlan_serialization_release_pdev_list_cmds(
		struct wlan_serialization_pdev_queue *pdev_queue)
{
	qdf_list_node_t *node = NULL;

	while (!wlan_serialization_list_empty(&pdev_queue->active_list)) {
		wlan_serialization_remove_front(
				&pdev_queue->active_list, &node);
		wlan_serialization_insert_back(
				&pdev_queue->cmd_pool_list, node);
	}

	while (!wlan_serialization_list_empty(&pdev_queue->pending_list)) {
		wlan_serialization_remove_front(
				&pdev_queue->pending_list, &node);
		wlan_serialization_insert_back(
				&pdev_queue->cmd_pool_list, node);
	}

}

static void wlan_serialization_release_vdev_list_cmds(qdf_list_t *list)
{
	qdf_list_node_t *node = NULL;


	while (!wlan_serialization_list_empty(list))
		wlan_serialization_remove_front(list, &node);

}

void wlan_serialization_destroy_pdev_list(
		struct wlan_serialization_pdev_queue *pdev_queue)
{

	wlan_serialization_release_pdev_list_cmds(pdev_queue);
	qdf_list_destroy(&pdev_queue->pending_list);
	qdf_list_destroy(&pdev_queue->active_list);

}

void wlan_serialization_destroy_vdev_list(qdf_list_t *list)
{

	wlan_serialization_release_vdev_list_cmds(list);
	qdf_list_destroy(list);

}

struct wlan_ser_psoc_obj *wlan_serialization_get_psoc_obj(
		struct wlan_objmgr_psoc *psoc)
{
	struct wlan_ser_psoc_obj *ser_soc_obj;

	ser_soc_obj =
		wlan_objmgr_psoc_get_comp_private_obj(
				psoc, WLAN_UMAC_COMP_SERIALIZATION);

	return ser_soc_obj;
}

struct wlan_ser_pdev_obj *wlan_serialization_get_pdev_obj(
		struct wlan_objmgr_pdev *pdev)
{
	struct wlan_ser_pdev_obj *obj;

	obj = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_SERIALIZATION);

	return obj;
}

struct wlan_ser_vdev_obj *wlan_serialization_get_vdev_obj(
		struct wlan_objmgr_vdev *vdev)
{
	struct wlan_ser_vdev_obj *obj;

	obj = wlan_objmgr_vdev_get_comp_private_obj(
			vdev, WLAN_UMAC_COMP_SERIALIZATION);

	return obj;
}

bool wlan_serialization_is_cmd_in_vdev_list(
		struct wlan_objmgr_vdev *vdev,
		qdf_list_t *queue,
		enum wlan_serialization_node node_type)
{
	qdf_list_node_t *node = NULL;
	bool cmd_found = false;

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_VDEV,
			NULL, 0, NULL, vdev, node_type);

	if (node)
		cmd_found = true;

	return cmd_found;
}

bool wlan_serialization_is_cmd_in_pdev_list(
			struct wlan_objmgr_pdev *pdev,
			qdf_list_t *queue)
{
	qdf_list_node_t *node = NULL;
	bool cmd_found = false;

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_PDEV,
			NULL, 0, pdev, NULL,  WLAN_SER_PDEV_NODE);

	if (node)
		cmd_found = true;

	return cmd_found;
}

enum wlan_serialization_cmd_status
wlan_serialization_is_cmd_in_active_pending(bool cmd_in_active,
					    bool cmd_in_pending)
{
	enum wlan_serialization_cmd_status status;

	if (cmd_in_active && cmd_in_pending)
		status = WLAN_SER_CMDS_IN_ALL_LISTS;
	else if (cmd_in_active)
		status = WLAN_SER_CMD_IN_ACTIVE_LIST;
	else if (cmd_in_pending)
		status = WLAN_SER_CMD_IN_PENDING_LIST;
	else
		status = WLAN_SER_CMD_NOT_FOUND;

	return status;
}

bool
wlan_serialization_is_cmd_present_in_given_queue(
		qdf_list_t *queue,
		struct wlan_serialization_command *cmd,
		enum wlan_serialization_node node_type)
{
	qdf_list_node_t *node = NULL;
	bool found = false;

	node = wlan_serialization_find_cmd(
			queue, WLAN_SER_MATCH_CMD_ID_VDEV,
			cmd, 0, NULL, cmd->vdev, node_type);

	if (node)
		found = true;

	return found;
}

/**
 * wlan_serialization_remove_cmd_from_queue() - to remove command from
 *							given queue
 * @queue: queue from which command needs to be removed
 * @cmd: command to match in the queue
 * @ser_pdev_obj: pointer to private pdev serialization object
 *
 * This API takes the queue, it matches the provided command from this queue
 * and removes it. Before removing the command, it will notify the caller
 * that if it needs to remove any memory allocated by caller.
 *
 * Return: none
 */
QDF_STATUS
wlan_serialization_remove_cmd_from_queue(
		qdf_list_t *queue,
		struct wlan_serialization_command *cmd,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		enum wlan_serialization_node node_type)
{
	struct wlan_serialization_command_list *cmd_list;
	qdf_list_node_t *node = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!cmd)
		goto error;

	if (!queue || wlan_serialization_list_empty(queue)) {
		ser_debug("Empty queue");
		goto error;
	}

	node = wlan_serialization_find_cmd(queue, WLAN_SER_MATCH_CMD_ID_VDEV,
					   cmd, 0, NULL, cmd->vdev, node_type);

	if (!node) {
		ser_info("fail to find node %d for removal", node_type);
		goto error;
	}

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list =
			qdf_container_of(node,
					 struct wlan_serialization_command_list,
					 pdev_node);
	else
		cmd_list =
			qdf_container_of(node,
					 struct wlan_serialization_command_list,
					 vdev_node);

	if (qdf_atomic_test_bit(CMD_MARKED_FOR_ACTIVATION,
				&cmd_list->cmd_in_use)) {
		qdf_atomic_set_bit(CMD_ACTIVE_MARKED_FOR_REMOVAL,
				   &cmd_list->cmd_in_use);
		status = QDF_STATUS_E_PENDING;
		goto error;
	}

	status = wlan_serialization_remove_node(queue, node);

	if (QDF_STATUS_SUCCESS != status)
		ser_err("Fail to add to free pool type %d",
			cmd->cmd_type);

	*pcmd_list = cmd_list;

error:
	return status;
}

enum wlan_serialization_status
wlan_serialization_add_cmd_to_queue(
		qdf_list_t *queue,
		struct wlan_serialization_command_list *cmd_list,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		uint8_t is_cmd_for_active_queue,
		enum wlan_serialization_node node_type)
{
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	QDF_STATUS qdf_status;
	qdf_list_node_t *node;

	if (!cmd_list || !queue || !ser_pdev_obj) {
		ser_err("Input arguments are not valid");
		goto error;
	}

	if (node_type == WLAN_SER_PDEV_NODE)
		node = &cmd_list->pdev_node;
	else
		node = &cmd_list->vdev_node;

	if (qdf_list_size(queue) == qdf_list_max_size(queue)) {
		status = WLAN_SER_CMD_DENIED_LIST_FULL;
		ser_err("Queue size reached max %d, fail to add type %d id %d",
			qdf_list_max_size(queue), cmd_list->cmd.cmd_type,
			cmd_list->cmd.cmd_id);
		goto error;
	}

	if (cmd_list->cmd.is_high_priority)
		qdf_status = wlan_serialization_insert_front(queue, node);
	else
		qdf_status = wlan_serialization_insert_back(queue, node);

	if (QDF_IS_STATUS_ERROR(qdf_status))
		goto error;

	if (is_cmd_for_active_queue)
		status = WLAN_SER_CMD_ACTIVE;
	else
		status = WLAN_SER_CMD_PENDING;

error:
	return status;
}

bool wlan_serialization_list_empty(qdf_list_t *queue)
{
	bool is_empty;

	if (qdf_list_empty(queue))
		is_empty = true;
	else
		is_empty = false;

	return is_empty;
}

uint32_t wlan_serialization_list_size(qdf_list_t *queue)
{
	uint32_t size;

	size = qdf_list_size(queue);

	return size;
}

QDF_STATUS wlan_serialization_remove_front(qdf_list_t *list,
					   qdf_list_node_t **node)
{
	QDF_STATUS status;

	if (wlan_serialization_list_empty(list)) {
		ser_err("The list is empty");
		status = QDF_STATUS_E_EMPTY;
		goto error;
	}

	status = qdf_list_remove_front(list, node);
error:
	return status;
}

QDF_STATUS wlan_serialization_remove_node(qdf_list_t *list,
					  qdf_list_node_t *node)
{
	QDF_STATUS status;

	if (wlan_serialization_list_empty(list)) {
		ser_err("The list is empty");
		status = QDF_STATUS_E_EMPTY;
		goto error;
	}
	status = qdf_list_remove_node(list, node);

error:
	return status;
}

QDF_STATUS wlan_serialization_insert_front(qdf_list_t *list,
					   qdf_list_node_t *node)
{
	QDF_STATUS status;

	status = qdf_list_insert_front(list, node);

	return status;
}

QDF_STATUS wlan_serialization_insert_back(qdf_list_t *list,
					  qdf_list_node_t *node)
{
	QDF_STATUS status;

	status = qdf_list_insert_back(list, node);

	return status;
}

QDF_STATUS wlan_serialization_peek_front(qdf_list_t *list,
					 qdf_list_node_t **node)
{
	QDF_STATUS status;

	status = qdf_list_peek_front(list, node);

	return status;
}

QDF_STATUS wlan_serialization_peek_next(qdf_list_t *list,
					qdf_list_node_t *node1,
					qdf_list_node_t **node2)
{
	QDF_STATUS status;

	status = qdf_list_peek_next(list, node1, node2);

	return status;
}

bool
wlan_serialization_match_cmd_type(qdf_list_node_t *nnode,
				  enum wlan_serialization_cmd_type cmd_type,
				  enum wlan_serialization_node node_type)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	bool match_found = true;

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 pdev_node);
	else
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 vdev_node);

	if (cmd_list->cmd.cmd_type != cmd_type)
		match_found = false;

	return match_found;
}

bool
wlan_serialization_match_cmd_id_type(qdf_list_node_t *nnode,
				     struct wlan_serialization_command *cmd,
				     enum wlan_serialization_node node_type)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	bool match_found = true;

	if (!cmd) {
		match_found = false;
		goto error;
	}

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 pdev_node);
	else
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 vdev_node);

	if ((cmd_list->cmd.cmd_id != cmd->cmd_id) ||
	    (cmd_list->cmd.cmd_type != cmd->cmd_type)) {
		match_found = false;
	};

error:
	return match_found;
}

bool wlan_serialization_match_cmd_vdev(qdf_list_node_t *nnode,
				       struct wlan_objmgr_vdev *vdev,
				       enum wlan_serialization_node node_type)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	bool match_found = false;

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 pdev_node);
	else
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 vdev_node);

	if (cmd_list->cmd.vdev == vdev)
		match_found = true;

	if (!match_found)
		ser_debug("matching cmd not found for (vdev:%pK)", vdev);

	return match_found;
}

bool wlan_serialization_match_cmd_pdev(qdf_list_node_t *nnode,
				       struct wlan_objmgr_pdev *pdev,
				       enum wlan_serialization_node node_type)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	bool match_found = false;
	struct wlan_objmgr_pdev *node_pdev = NULL;

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 pdev_node);
	else
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 vdev_node);

	node_pdev = wlan_vdev_get_pdev(cmd_list->cmd.vdev);
	if (node_pdev == pdev)
		match_found = true;

	return match_found;
}

bool wlan_serialization_match_cmd_blocking(
		qdf_list_node_t *nnode,
		enum wlan_serialization_node node_type)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	bool match_found = false;

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 pdev_node);
	else
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 vdev_node);

	if (cmd_list->cmd.is_blocking)
		match_found = true;

	return match_found;
}

qdf_list_node_t *
wlan_serialization_find_cmd(qdf_list_t *queue,
			    enum wlan_serialization_match_type match_type,
			    struct wlan_serialization_command *cmd,
			    enum wlan_serialization_cmd_type cmd_type,
			    struct wlan_objmgr_pdev *pdev,
			    struct wlan_objmgr_vdev *vdev,
			    enum wlan_serialization_node node_type)
{
	qdf_list_node_t *cmd_node = NULL;
	uint32_t queuelen;
	qdf_list_node_t *nnode = NULL;
	QDF_STATUS status;
	bool node_found = 0;

	queuelen = wlan_serialization_list_size(queue);

	if (!queuelen)
		goto error;

	while (queuelen--) {
		status = wlan_serialization_get_cmd_from_queue(queue, &nnode);
		if (status != QDF_STATUS_SUCCESS)
			break;

		switch (match_type) {
		case WLAN_SER_MATCH_PDEV:
			if (wlan_serialization_match_cmd_pdev(
					nnode, pdev, WLAN_SER_PDEV_NODE))
				node_found = 1;
			break;
		case WLAN_SER_MATCH_VDEV:
			if (wlan_serialization_match_cmd_vdev(
					nnode, vdev, node_type))
				node_found = 1;
			break;
		case WLAN_SER_MATCH_CMD_TYPE:
			if (wlan_serialization_match_cmd_type(
					nnode, cmd_type, node_type))
				node_found = 1;
			break;
		case WLAN_SER_MATCH_CMD_ID:
			if (wlan_serialization_match_cmd_id_type(
					nnode, cmd, node_type))
				node_found = 1;
			break;
		case WLAN_SER_MATCH_CMD_TYPE_VDEV:
			if (wlan_serialization_match_cmd_type(
					nnode, cmd_type, node_type) &&
			    wlan_serialization_match_cmd_vdev(
					nnode, vdev, node_type))
				node_found = 1;
			break;
		case WLAN_SER_MATCH_CMD_ID_VDEV:
			if (wlan_serialization_match_cmd_id_type(
					nnode, cmd, node_type) &&
			    wlan_serialization_match_cmd_vdev(
					nnode, vdev, node_type))
				node_found = 1;
			break;
		default:
			break;
		}

		if (node_found) {
			cmd_node = nnode;
			break;
		}
	}
error:
	return cmd_node;
}

QDF_STATUS
wlan_serialization_acquire_lock(qdf_spinlock_t *lock)
{
	qdf_spin_lock_bh(lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_release_lock(qdf_spinlock_t *lock)
{
	qdf_spin_unlock_bh(lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_create_lock(qdf_spinlock_t *lock)
{
	qdf_spinlock_create(lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_destroy_lock(qdf_spinlock_t *lock)
{
	qdf_spinlock_destroy(lock);

	return QDF_STATUS_SUCCESS;
}

bool wlan_serialization_any_vdev_cmd_active(
		struct wlan_serialization_pdev_queue *pdev_queue)
{
	uint32_t vdev_bitmap_size;

	vdev_bitmap_size =
		(QDF_CHAR_BIT * sizeof(pdev_queue->vdev_active_cmd_bitmap));

	return !qdf_bitmap_empty(pdev_queue->vdev_active_cmd_bitmap,
				 vdev_bitmap_size);
}

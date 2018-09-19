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
 * DOC: wlan_serialization_non_scan.c
 * This file defines the functions which deals with
 * serialization non scan commands.
 */

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_non_scan_i.h"

/**
 * wlan_serialization_is_active_nonscan_cmd_allowed() - find if cmd allowed
 * @pdev: pointer to pdev object
 *
 * This API will be called to find out if non scan cmd is allowed.
 *
 * Return: true or false
 */
bool
wlan_serialization_is_active_non_scan_cmd_allowed(
		struct wlan_serialization_command *cmd)
{
	struct wlan_serialization_pdev_queue *pdev_queue;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	uint32_t vdev_active_cmd_bitmap;
	bool blocking_cmd_active = 0;
	uint8_t blocking_cmd_waiting = 0;
	bool status = false;
	uint32_t vdev_id;

	ser_pdev_obj = wlan_serialization_get_pdev_obj(
			wlan_serialization_get_pdev_from_cmd(cmd));

	pdev_queue = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj,
							   cmd->cmd_type);

	vdev_active_cmd_bitmap = pdev_queue->vdev_active_cmd_bitmap;
	blocking_cmd_active = pdev_queue->blocking_cmd_active;
	blocking_cmd_waiting = pdev_queue->blocking_cmd_waiting;

	/*
	 * Command is blocking
	 */
	if (cmd->is_blocking) {
		/*
		 * For blocking commands, no other
		 * commands from any vdev should be active
		 */
		if (vdev_active_cmd_bitmap) {
			status = false;
			pdev_queue->blocking_cmd_waiting++;
		} else {
			status = true;
		}
	} else {
		/*
		 * Command is non blocking
		 * For activating non blocking commands, if there any blocking
		 * commands, waiting or active, put it to pending queue
		 */
		if (blocking_cmd_active || blocking_cmd_waiting) {
			status = false;
		} else {
		/*
		 * For non blocking command, and no blocking commands
		 * waiting or active, check if a cmd for that vdev is active
		 * If not active, put to active else pending queue
		 */
			vdev_id = wlan_vdev_get_id(cmd->vdev);
			status = vdev_active_cmd_bitmap & (1 << vdev_id)
						? false : true;
		}
	}
	return status;
}

enum wlan_serialization_status wlan_ser_add_non_scan_cmd(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_serialization_command_list *cmd_list,
		uint8_t is_cmd_for_active_queue)
{
	enum wlan_serialization_status pdev_status, vdev_status;
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	struct wlan_serialization_command_list *pcmd_list;
	uint8_t vdev_id;
	struct wlan_serialization_pdev_queue *pdev_queue;

	ser_debug("add non scan cmd: type[%d] id[%d] prio[%d] blocking[%d]",
		  cmd_list->cmd.cmd_type,
		  cmd_list->cmd.cmd_id,
		  cmd_list->cmd.is_high_priority,
		  cmd_list->cmd.is_blocking);

	vdev_status = wlan_serialization_add_cmd_to_vdev_queue(
			ser_pdev_obj, cmd_list, is_cmd_for_active_queue);

	if (vdev_status == WLAN_SER_CMD_DENIED_LIST_FULL) {
		status = vdev_status;
		goto vdev_error;
	}

	if (is_cmd_for_active_queue) {
		if (vdev_status != WLAN_SER_CMD_ACTIVE) {
			ser_err("Failed to add to vdev active queue");
			QDF_ASSERT(0);
			goto vdev_error;
		}
	} else {
		if (vdev_status != WLAN_SER_CMD_PENDING) {
			ser_err("Failed to add to vdev pending queue");
			QDF_ASSERT(0);
			goto vdev_error;
		}
	}

	pdev_status = wlan_serialization_add_cmd_to_pdev_queue(
			ser_pdev_obj, cmd_list, is_cmd_for_active_queue);

	if (pdev_status == WLAN_SER_CMD_DENIED_LIST_FULL) {
		status = pdev_status;
		goto pdev_error;
	}

	if (is_cmd_for_active_queue) {
		if (pdev_status != WLAN_SER_CMD_ACTIVE) {
			ser_err("Failed to add to pdev active queue");
			QDF_ASSERT(0);
			goto pdev_error;
		}
	} else {
		if (pdev_status != WLAN_SER_CMD_PENDING) {
			ser_err("Failed to add to pdev pending queue");
			QDF_ASSERT(0);
			goto pdev_error;
		}
	}
pdev_error:
	/*
	 * If cmd added to vdev queue, but failed while
	 * adding to pdev queue, remove cmd from vdev queue as well
	 */
	if (pdev_status != vdev_status) {
		wlan_serialization_remove_cmd_from_vdev_queue(
			ser_pdev_obj, &pcmd_list,
			&cmd_list->cmd,
			is_cmd_for_active_queue);
	} else {
		status = pdev_status;
	}

	if (is_cmd_for_active_queue) {
		pdev_queue = wlan_serialization_get_pdev_queue_obj(
				ser_pdev_obj, cmd_list->cmd.cmd_type);
		vdev_id = wlan_vdev_get_id(cmd_list->cmd.vdev);
		pdev_queue->vdev_active_cmd_bitmap |= (1 << vdev_id);

		if (cmd_list->cmd.is_blocking)
			pdev_queue->blocking_cmd_active = 1;
	}

vdev_error:
	return status;
}

static
struct wlan_serialization_command_list*
wlan_serialization_get_next_non_scan_active_cmd(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_vdev *vdev)
{
	qdf_list_t *pending_queue;
	qdf_list_node_t *pending_node = NULL;
	struct wlan_serialization_command_list *pending_cmd_list = NULL;
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	struct wlan_serialization_vdev_queue *vdev_q_obj;
	QDF_STATUS status;

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);

	vdev_q_obj = &ser_vdev_obj->vdev_q[SER_VDEV_QUEUE_COMP_NON_SCAN];
	pending_queue = &vdev_q_obj->pending_list;

	if (wlan_serialization_list_empty(pending_queue)) {
		ser_debug("nothing to move from pending to active queue");
		goto error;
	}

	status = wlan_serialization_get_cmd_from_queue(pending_queue,
						       &pending_node);

	if (status != QDF_STATUS_SUCCESS) {
		ser_err("can't read cmd from queue");
		pending_cmd_list = NULL;
		goto error;
	}

	pending_cmd_list =
		qdf_container_of(pending_node,
				 struct wlan_serialization_command_list,
				 vdev_node);

	if (pending_cmd_list)
		ser_debug("next non scan active cmd found from pending queue");

error:
	return pending_cmd_list;
}

enum wlan_serialization_status
wlan_ser_move_multiple_non_scan_pending_to_active(
		struct wlan_ser_pdev_obj *ser_pdev_obj)
{
	struct wlan_serialization_pdev_queue *pdev_queue;
	qdf_list_t *active_queue;
	qdf_list_t *pending_queue;
	qdf_list_node_t *nnode = NULL;
	struct wlan_serialization_command_list *pending_cmd_list;
	struct wlan_serialization_command_list *active_cmd_list;
	struct wlan_serialization_command cmd_to_remove;
	uint32_t blocking_cmd_waiting;
	uint32_t vdev_id;
	uint32_t qsize;
	bool vdev_cmd_active = 0;

	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	QDF_STATUS peek_status = QDF_STATUS_E_FAILURE;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];
	active_queue = &pdev_queue->active_list;
	pending_queue = &pdev_queue->pending_list;
	blocking_cmd_waiting = pdev_queue->blocking_cmd_waiting;

	qsize =  wlan_serialization_list_size(pending_queue);
	while (qsize--) {
		peek_status = wlan_serialization_get_cmd_from_queue(
				pending_queue, &nnode);
		if (peek_status != QDF_STATUS_SUCCESS) {
			ser_err("can't peek cmd");
			break;
		}

		pending_cmd_list = qdf_container_of(
				nnode, struct wlan_serialization_command_list,
				pdev_node);
		if (!pending_cmd_list) {
			ser_debug(
				"non scan cmd cannot move from pending to active");
			goto error;
		}

		if (pending_cmd_list->cmd.is_blocking &&
		    pdev_queue->vdev_active_cmd_bitmap) {
			break;
		}

		vdev_id = wlan_vdev_get_id(pending_cmd_list->cmd.vdev);
		vdev_cmd_active =
			pdev_queue->vdev_active_cmd_bitmap & (1 << vdev_id);
		if (vdev_cmd_active)
			continue;

		status = WLAN_SER_CMD_ACTIVE;

		qdf_mem_copy(&cmd_to_remove, &pending_cmd_list->cmd,
			     sizeof(struct wlan_serialization_command));

		qdf_status = wlan_ser_remove_non_scan_cmd(ser_pdev_obj,
							  &pending_cmd_list,
							  &cmd_to_remove,
							  false);

		if (QDF_STATUS_SUCCESS != qdf_status) {
			ser_err("Can't remove cmd from pendingQ id-%d type-%d",
				pending_cmd_list->cmd.cmd_id,
				pending_cmd_list->cmd.cmd_type);
			QDF_ASSERT(0);
			status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
			goto error;
		}

		active_cmd_list = pending_cmd_list;

		status = wlan_ser_add_non_scan_cmd(
				ser_pdev_obj, active_cmd_list, true);

		if (WLAN_SER_CMD_ACTIVE != status) {
			ser_err("Can't move cmd to activeQ id-%d type-%d",
				pending_cmd_list->cmd.cmd_id,
				pending_cmd_list->cmd.cmd_type);
			wlan_serialization_insert_back(
				&pdev_queue->cmd_pool_list,
				&active_cmd_list->pdev_node);
			status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
			goto error;
		}

		qdf_atomic_set_bit(CMD_MARKED_FOR_ACTIVATION,
				   &active_cmd_list->cmd_in_use);

		nnode = NULL;
		if (active_cmd_list->cmd.is_blocking)
			pdev_queue->blocking_cmd_waiting--;

		if (pending_cmd_list->cmd.is_blocking)
			break;
	}

error:
	return status;
}

enum wlan_serialization_status
wlan_ser_move_non_scan_pending_to_active(
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_vdev *vdev)
{
	struct wlan_serialization_command_list *pending_cmd_list;
	struct wlan_serialization_command_list *active_cmd_list;
	struct wlan_serialization_command cmd_to_remove;
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	QDF_STATUS qdf_status;
	struct wlan_serialization_pdev_queue *pdev_queue;

	pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];

	ser_enter();

	if (!ser_pdev_obj) {
		ser_err("Can't find ser_pdev_obj");
		goto error;
	}

	pending_cmd_list =
		wlan_serialization_get_next_non_scan_active_cmd(
						ser_pdev_obj,
						vdev);

	if (!pending_cmd_list) {
		ser_debug("non scan cmd cannot move from pending to active");
		goto error;
	}

	qdf_mem_copy(&cmd_to_remove, &pending_cmd_list->cmd,
		     sizeof(struct wlan_serialization_command));

	qdf_status = wlan_ser_remove_non_scan_cmd(ser_pdev_obj,
						  &pending_cmd_list,
						  &cmd_to_remove, false);

	if (QDF_STATUS_SUCCESS != qdf_status) {
		ser_err("Can't remove cmd from pendingQ id-%d type-%d",
			pending_cmd_list->cmd.cmd_id,
			pending_cmd_list->cmd.cmd_type);
		QDF_ASSERT(0);
		status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
		goto error;
	}

	active_cmd_list = pending_cmd_list;

	status = wlan_ser_add_non_scan_cmd(
				ser_pdev_obj, active_cmd_list, true);

	if (WLAN_SER_CMD_ACTIVE != status) {
		ser_err("Can't move cmd to activeQ id-%d type-%d",
			pending_cmd_list->cmd.cmd_id,
			pending_cmd_list->cmd.cmd_type);
		wlan_serialization_insert_back(
			&pdev_queue->cmd_pool_list,
			&active_cmd_list->pdev_node);
		goto error;
	}

	qdf_atomic_set_bit(CMD_MARKED_FOR_ACTIVATION,
			   &active_cmd_list->cmd_in_use);

	*pcmd_list = active_cmd_list;

error:
	ser_exit();
	return status;
}

QDF_STATUS wlan_ser_remove_non_scan_cmd(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_serialization_command *cmd,
		uint8_t is_active_cmd)
{
	QDF_STATUS pdev_status, vdev_status;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t vdev_id;
	bool blocking_cmd_removed = 0;
	struct wlan_serialization_pdev_queue *pdev_queue;

	ser_debug("remove non scan cmd: type[%d] id[%d] prio[%d] blocking[%d]",
		  cmd->cmd_type,
		  cmd->cmd_id,
		  cmd->is_high_priority,
		  cmd->is_blocking);

	vdev_status =
		wlan_serialization_remove_cmd_from_vdev_queue(ser_pdev_obj,
							      pcmd_list,
							      cmd,
							      is_active_cmd);

	if (vdev_status != QDF_STATUS_SUCCESS) {
		ser_err("Failed to remove cmd from vdev active/pending queue");
		goto error;
	}

	pdev_status =
		wlan_serialization_remove_cmd_from_pdev_queue(ser_pdev_obj,
							      pcmd_list,
							      cmd,
							      is_active_cmd);

	if (pdev_status != QDF_STATUS_SUCCESS) {
		ser_err("Failed to remove cmd from pdev active/pending queue");
		goto error;
	}

	if (is_active_cmd) {
		blocking_cmd_removed = (*pcmd_list)->cmd.is_blocking;
		pdev_queue = wlan_serialization_get_pdev_queue_obj(
				ser_pdev_obj, (*pcmd_list)->cmd.cmd_type);

		if (blocking_cmd_removed)
			pdev_queue->blocking_cmd_active = 0;

		vdev_id = wlan_vdev_get_id(cmd->vdev);
		pdev_queue->vdev_active_cmd_bitmap &= ~(1 << vdev_id);
	}

	status = QDF_STATUS_SUCCESS;

error:
	return status;
}

enum wlan_serialization_cmd_status
wlan_ser_cancel_non_scan_cmd(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_pdev *pdev, struct wlan_objmgr_vdev *vdev,
		struct wlan_serialization_command *cmd,
		enum wlan_serialization_cmd_type cmd_type,
		uint8_t is_active_queue)
{
	qdf_list_t *pdev_queue;
	qdf_list_t *vdev_queue;
	struct wlan_serialization_pdev_queue *pdev_q;
	uint32_t qsize;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command cmd_bkup;
	qdf_list_node_t *nnode = NULL, *pnode = NULL;
	enum wlan_serialization_cmd_status status = WLAN_SER_CMD_NOT_FOUND;
	struct wlan_objmgr_psoc *psoc = NULL;
	QDF_STATUS qdf_status;
	QDF_STATUS pdev_status, vdev_status;
	struct wlan_ser_vdev_obj *ser_vdev_obj;

	ser_enter();

	pdev_q = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj, cmd_type);

	pdev_queue = wlan_serialization_get_list_from_pdev_queue(
			ser_pdev_obj, cmd_type, is_active_queue);

	if (pdev)
		psoc = wlan_pdev_get_psoc(pdev);
	else if (vdev)
		psoc = wlan_vdev_get_psoc(vdev);
	else if (cmd && cmd->vdev)
		psoc = wlan_vdev_get_psoc(cmd->vdev);
	else
		ser_debug("Can't find psoc");

	wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

	qsize = wlan_serialization_list_size(pdev_queue);
	while (!wlan_serialization_list_empty(pdev_queue) && qsize--) {
		if (wlan_serialization_get_cmd_from_queue(pdev_queue, &nnode)
		    != QDF_STATUS_SUCCESS) {
			ser_err("can't read cmd from queue");
			status = WLAN_SER_CMD_NOT_FOUND;
			break;
		}
		cmd_list =
			qdf_container_of(nnode,
					 struct wlan_serialization_command_list,
					 pdev_node);
		if (cmd && !wlan_serialization_match_cmd_id_type(
							nnode, cmd,
							WLAN_SER_PDEV_NODE)) {
			pnode = nnode;
			continue;
		}

		if (vdev &&
		    !wlan_serialization_match_cmd_vdev(nnode,
						      vdev,
						      WLAN_SER_PDEV_NODE)) {
			pnode = nnode;
			continue;
		}

		if (pdev &&
		    !wlan_serialization_match_cmd_pdev(nnode,
						       pdev,
						       WLAN_SER_PDEV_NODE)) {
			pnode = nnode;
			continue;
		}

		/*
		 * active queue can't be removed directly, requester needs to
		 * wait for active command response and send remove request for
		 * active command separately
		 */
		if (is_active_queue) {
			if (!psoc || !cmd_list) {
				ser_err("psoc:0x%pK, cmd_list:0x%pK",
					psoc, cmd_list);
				status = WLAN_SER_CMD_NOT_FOUND;
				break;
			}

			/* Cancel request received for a cmd in active
			 * queue which has not been activated yet, we
			 * should assert here
			 */
			if (qdf_atomic_test_bit(CMD_MARKED_FOR_ACTIVATION,
						&cmd_list->cmd_in_use)) {
				wlan_serialization_release_lock(
						&pdev_q->pdev_queue_lock);
				status = WLAN_SER_CMD_MARKED_FOR_ACTIVATION;
				goto error;
			}

			qdf_status = wlan_serialization_find_and_stop_timer(
							psoc, &cmd_list->cmd);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				ser_err("Can't fix timer for active cmd");
				status = WLAN_SER_CMD_NOT_FOUND;
				break;
			}
			status = WLAN_SER_CMD_IN_ACTIVE_LIST;
		}

		qdf_mem_copy(&cmd_bkup, &cmd_list->cmd,
			     sizeof(struct wlan_serialization_command));

		pdev_status =
			wlan_serialization_remove_node(pdev_queue,
						       &cmd_list->pdev_node);

		ser_vdev_obj = wlan_serialization_get_vdev_obj(
					cmd_list->cmd.vdev);

		vdev_queue = wlan_serialization_get_list_from_vdev_queue(
			ser_vdev_obj, cmd_type, is_active_queue);

		vdev_status =
			wlan_serialization_remove_node(vdev_queue,
						       &cmd_list->vdev_node);

		if (pdev_status != QDF_STATUS_SUCCESS ||
		    vdev_status != QDF_STATUS_SUCCESS) {
			ser_err("can't remove cmd from pdev/vdev queue");
			status = WLAN_SER_CMD_NOT_FOUND;
			break;
		}

		qdf_mem_zero(&cmd_list->cmd,
			     sizeof(struct wlan_serialization_command));

		qdf_status = wlan_serialization_insert_back(
			&pdev_q->cmd_pool_list,
			&cmd_list->pdev_node);

		if (QDF_STATUS_SUCCESS != qdf_status) {
			ser_err("can't remove cmd from queue");
			status = WLAN_SER_CMD_NOT_FOUND;
			break;
		}
		nnode = pnode;

		wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);
		/*
		 * call pending cmd's callback to notify that
		 * it is being removed
		 */
		if (cmd_bkup.cmd_cb) {
			/* caller should now do necessary clean up */
			ser_debug("cmd cb: type[%d] id[%d]",
				  cmd_bkup.cmd_type,
				  cmd_bkup.cmd_id);
			ser_debug("reason: WLAN_SER_CB_CANCEL_CMD");
			cmd_bkup.cmd_cb(&cmd_bkup,
					WLAN_SER_CB_CANCEL_CMD);
			/* caller should release the memory */
			ser_debug("reason: WLAN_SER_CB_RELEASE_MEM_CMD");
			cmd_bkup.cmd_cb(&cmd_bkup,
					WLAN_SER_CB_RELEASE_MEM_CMD);
		}

		wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

		if (!is_active_queue)
			status = WLAN_SER_CMD_IN_PENDING_LIST;

		if (!vdev && !pdev)
			break;
	}

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

error:
	ser_exit();
	return status;
}

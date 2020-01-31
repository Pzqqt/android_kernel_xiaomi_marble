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

bool
wlan_serialization_is_non_scan_pending_queue_empty(
		struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_vdev *vdev = NULL;
	struct wlan_ser_vdev_obj *ser_vdev_obj = NULL;
	struct wlan_serialization_vdev_queue *vdev_q;
	bool status = false;

	vdev = wlan_serialization_get_vdev_from_cmd(cmd);

	if (!vdev) {
		ser_err("vdev object  is invalid");
		goto error;
	}

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);
	vdev_q = &ser_vdev_obj->vdev_q[SER_VDEV_QUEUE_COMP_NON_SCAN];

	if (qdf_list_empty(&vdev_q->pending_list))
		status = true;

error:
	return status;
}

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

	vdev_status = wlan_serialization_add_cmd_to_vdev_queue(
			ser_pdev_obj, cmd_list, is_cmd_for_active_queue);

	if (vdev_status == WLAN_SER_CMD_DENIED_LIST_FULL) {
		ser_err_rl("List is full cannot add type %d cmd id %d",
			   cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
		status = vdev_status;
		goto vdev_error;
	}

	if (is_cmd_for_active_queue) {
		if (vdev_status != WLAN_SER_CMD_ACTIVE) {
			ser_err("Failed to add type %d cmd id %d to vdev active queue",
				cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
			QDF_ASSERT(0);
			goto vdev_error;
		}
	} else {
		if (vdev_status != WLAN_SER_CMD_PENDING) {
			ser_err("Failed to add type %d cmd id %d to vdev pending queue",
				cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
			QDF_ASSERT(0);
			goto vdev_error;
		}
	}

	pdev_status = wlan_serialization_add_cmd_to_pdev_queue(
			ser_pdev_obj, cmd_list, is_cmd_for_active_queue);

	if (pdev_status == WLAN_SER_CMD_DENIED_LIST_FULL) {
		ser_err_rl("pdev List is full cannot add type %d cmd id %d",
			   cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
		status = pdev_status;
		goto pdev_error;
	}

	if (is_cmd_for_active_queue) {
		if (pdev_status != WLAN_SER_CMD_ACTIVE) {
			ser_err("Failed to add type %d cmd id %d to pdev active queue",
				cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
			QDF_ASSERT(0);
			goto pdev_error;
		}
	} else {
		if (pdev_status != WLAN_SER_CMD_PENDING) {
			ser_err("Failed to add type %d cmd id %d to pdev pending queue",
				cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);
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

enum wlan_serialization_status
wlan_ser_move_non_scan_pending_to_active(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_vdev *vdev,
		bool blocking_cmd_removed)
{
	struct wlan_serialization_command_list *pending_cmd_list = NULL;
	struct wlan_serialization_command_list *active_cmd_list;
	struct wlan_serialization_command cmd_to_remove;
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	struct wlan_serialization_pdev_queue *pdev_queue;
	struct wlan_serialization_vdev_queue *vdev_queue;

	struct wlan_ser_vdev_obj *ser_vdev_obj;

	qdf_list_t *pending_queue;
	qdf_list_node_t *pending_node = NULL;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint32_t blocking_cmd_waiting = 0;
	uint32_t vdev_id;
	uint32_t qsize;
	bool vdev_cmd_active = 0;
	bool vdev_queue_lookup = false;

	pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];

	ser_vdev_obj = wlan_serialization_get_vdev_obj(vdev);
	vdev_queue = &ser_vdev_obj->vdev_q[SER_VDEV_QUEUE_COMP_NON_SCAN];

	if (!ser_pdev_obj) {
		ser_err("Can't find ser_pdev_obj");
		goto error;
	}

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	blocking_cmd_waiting = pdev_queue->blocking_cmd_waiting;

	if (!blocking_cmd_removed && !blocking_cmd_waiting) {
		pending_queue = &vdev_queue->pending_list;
		vdev_queue_lookup = true;
	} else {
		pending_queue = &pdev_queue->pending_list;
	}

	qsize =  wlan_serialization_list_size(pending_queue);
	if (!qsize) {
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		goto error;
	}

	while (qsize--) {
		qdf_status = wlan_serialization_get_cmd_from_queue(
				pending_queue, &pending_node);
		if (qdf_status != QDF_STATUS_SUCCESS) {
			ser_err("can't peek cmd");
			break;
		}

		if (vdev_queue_lookup) {
			pending_cmd_list =
				qdf_container_of(
				pending_node,
				struct wlan_serialization_command_list,
				vdev_node);
		} else {
			pending_cmd_list =
				qdf_container_of(
				pending_node,
				struct wlan_serialization_command_list,
				pdev_node);
		}

		if (!pending_cmd_list) {
			wlan_serialization_release_lock(
				&pdev_queue->pdev_queue_lock);
			ser_debug(
				"non scan cmd cannot move frm pendin to actv");
			goto error;
		}

		vdev_id = wlan_vdev_get_id(pending_cmd_list->cmd.vdev);
		vdev_cmd_active =
			pdev_queue->vdev_active_cmd_bitmap &
			(1 << vdev_id);

		if (!vdev_queue_lookup) {
			if (pending_cmd_list->cmd.is_blocking &&
			    pdev_queue->vdev_active_cmd_bitmap) {
				break;
			}
			if (vdev_cmd_active)
				continue;
		} else {
			if (vdev_cmd_active)
				break;
		}

		qdf_mem_copy(&cmd_to_remove, &pending_cmd_list->cmd,
			     sizeof(struct wlan_serialization_command));

		qdf_status = wlan_ser_remove_non_scan_cmd(ser_pdev_obj,
							  &pending_cmd_list,
							  &cmd_to_remove,
							  false);

		wlan_ser_update_cmd_history(
				pdev_queue, &pending_cmd_list->cmd,
				SER_PENDING_TO_ACTIVE,
				false, false);

		if (QDF_STATUS_SUCCESS != qdf_status) {
			wlan_serialization_release_lock(
					&pdev_queue->pdev_queue_lock);
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
			wlan_serialization_release_lock(
					&pdev_queue->pdev_queue_lock);
			ser_err("Can't move cmd to activeQ id-%d type-%d",
				pending_cmd_list->cmd.cmd_id,
				pending_cmd_list->cmd.cmd_type);
			wlan_serialization_insert_back(
				&pdev_queue->cmd_pool_list,
				&active_cmd_list->pdev_node);
			status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
			QDF_ASSERT(0);
			goto error;
		}

		wlan_ser_update_cmd_history(
				pdev_queue, &active_cmd_list->cmd,
				SER_PENDING_TO_ACTIVE,
				true, true);

		qdf_atomic_set_bit(CMD_MARKED_FOR_ACTIVATION,
				   &active_cmd_list->cmd_in_use);

		if (active_cmd_list->cmd.is_blocking)
			pdev_queue->blocking_cmd_waiting--;

		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

		wlan_serialization_activate_cmd(active_cmd_list, ser_pdev_obj,
						SER_PENDING_TO_ACTIVE);

		wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

		if (vdev_queue_lookup || pdev_queue->blocking_cmd_active)
			break;

		pending_node = NULL;

	}

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
error:

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

	vdev_status =
		wlan_serialization_remove_cmd_from_vdev_queue(ser_pdev_obj,
							      pcmd_list,
							      cmd,
							      is_active_cmd);

	/* Here command removal can fail for 2 reasons
	 * 1. The cmd is not present
	 * 2. The command had not returned from activation
	 *    and will not be removed now.
	 *
	 *  In the second case, we should not flag it as error
	 *  since it will removed after the activation completes.
	 */

	if (vdev_status != QDF_STATUS_SUCCESS) {
		status = vdev_status;
		if (vdev_status != QDF_STATUS_E_PENDING)
			ser_debug("Failed to remove type %d id %d from vdev queue",
				  cmd->cmd_type, cmd->cmd_id);
		goto error;
	}

	pdev_status =
		wlan_serialization_remove_cmd_from_pdev_queue(ser_pdev_obj,
							      pcmd_list,
							      cmd,
							      is_active_cmd);

	if (pdev_status != QDF_STATUS_SUCCESS) {
		ser_debug("Failed to remove type %d id %d from pdev active/pending queue",
			  cmd->cmd_type, cmd->cmd_id);
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
		uint8_t is_active_queue, enum wlan_ser_cmd_attr cmd_attr)
{
	qdf_list_t *pdev_queue;
	qdf_list_t *vdev_queue;
	struct wlan_serialization_pdev_queue *pdev_q;
	uint32_t qsize;
	uint8_t vdev_id;
	bool is_blocking;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command cmd_bkup;
	qdf_list_node_t *nnode = NULL, *pnode = NULL;
	enum wlan_serialization_cmd_status status = WLAN_SER_CMD_NOT_FOUND;
	struct wlan_objmgr_psoc *psoc = NULL;
	QDF_STATUS qdf_status;
	QDF_STATUS pdev_status, vdev_status;
	struct wlan_ser_vdev_obj *ser_vdev_obj;

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

		if (cmd_type > WLAN_SER_CMD_NONSCAN && vdev &&
		    (!wlan_serialization_match_cmd_type(nnode, cmd_type,
							WLAN_SER_PDEV_NODE) ||
		    !wlan_serialization_match_cmd_vdev(nnode, vdev,
						       WLAN_SER_PDEV_NODE))) {
			pnode = nnode;
			continue;
		}

		/*
		 * If a non-blocking cmd is required to be cancelled, but
		 * the nnode cmd is a blocking cmd then continue with the
		 * next command in the list else proceed with cmd cancel.
		 */
		if ((cmd_attr == WLAN_SER_CMD_ATTR_NONBLOCK) &&
		    wlan_serialization_match_cmd_blocking(nnode,
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
			 * queue which has not been activated yet, we mark
			 * it as CMD_ACTIVE_MARKED_FOR_CANCEL and remove
			 * the cmd after activation
			 */
			if (qdf_atomic_test_bit(CMD_MARKED_FOR_ACTIVATION,
						&cmd_list->cmd_in_use)) {
				qdf_atomic_set_bit(CMD_ACTIVE_MARKED_FOR_CANCEL,
						   &cmd_list->cmd_in_use);
				status = WLAN_SER_CMD_MARKED_FOR_ACTIVATION;
				continue;
			}

			qdf_status = wlan_serialization_find_and_stop_timer(
							psoc, &cmd_list->cmd,
							SER_CANCEL);
			if (QDF_IS_STATUS_ERROR(qdf_status)) {
				ser_err("Can't find timer for active cmd");
				status = WLAN_SER_CMD_NOT_FOUND;
				/*
				 * This should not happen, as an active command
				 * should always have the timer.
				 */
				QDF_BUG(0);
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
		cmd_list->cmd_in_use = 0;
		qdf_status = wlan_serialization_insert_back(
			&pdev_q->cmd_pool_list,
			&cmd_list->pdev_node);

		if (QDF_STATUS_SUCCESS != qdf_status) {
			ser_err("can't remove cmd from queue");
			status = WLAN_SER_CMD_NOT_FOUND;
			break;
		}
		nnode = pnode;

		vdev_id = wlan_vdev_get_id(cmd_bkup.vdev);
		is_blocking = cmd_bkup.is_blocking;

		wlan_ser_update_cmd_history(pdev_q, &cmd_bkup,
					    SER_CANCEL, false, is_active_queue);

		wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);
		/*
		 * call pending cmd's callback to notify that
		 * it is being removed
		 */
		if (cmd_bkup.cmd_cb) {
			/* caller should now do necessary clean up */
			ser_debug("Cancel command: type %d id %d and Release memory",
				  cmd_bkup.cmd_type, cmd_bkup.cmd_id);
			cmd_bkup.cmd_cb(&cmd_bkup, WLAN_SER_CB_CANCEL_CMD);
			/* caller should release the memory */
			cmd_bkup.cmd_cb(&cmd_bkup, WLAN_SER_CB_RELEASE_MEM_CMD);
		}

		wlan_serialization_acquire_lock(&pdev_q->pdev_queue_lock);

		if (is_active_queue) {
			if (is_blocking)
				pdev_q->blocking_cmd_active = 0;
			pdev_q->vdev_active_cmd_bitmap &= ~(1 << vdev_id);
			ser_debug("pdev_q->vdev_active_cmd_bitmap %x after reseting for vdev %d",
				  pdev_q->vdev_active_cmd_bitmap,
				  vdev_id);
		} else {
			if (is_blocking)
				pdev_q->blocking_cmd_waiting--;

			status = WLAN_SER_CMD_IN_PENDING_LIST;
		}


		if (!vdev && !pdev)
			break;
	}

	wlan_serialization_release_lock(&pdev_q->pdev_queue_lock);

	return status;
}

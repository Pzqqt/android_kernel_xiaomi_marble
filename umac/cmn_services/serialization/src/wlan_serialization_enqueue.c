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
 * DOC: wlan_serialization_enqueue.c
 * This file defines the routines which are pertinent
 * to the queuing of commands.
 */
#include <wlan_serialization_api.h>
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>

static enum wlan_serialization_status
wlan_serialization_add_cmd_to_given_queue(qdf_list_t *queue,
			struct wlan_serialization_command *cmd,
			struct wlan_objmgr_psoc *psoc,
			struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
			uint8_t is_cmd_for_active_queue)
{
	struct wlan_serialization_command_list *cmd_list;
	enum wlan_serialization_status status;
	QDF_STATUS qdf_status;
	qdf_list_node_t *nnode;

	if (!cmd || !queue || !ser_pdev_obj || !psoc) {
		serialization_err("Input arguments are not valid");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}
	if (qdf_list_empty(&ser_pdev_obj->global_cmd_pool_list)) {
		serialization_err("list is full, can't add more");
		QDF_BUG(0);
		return WLAN_SER_CMD_DENIED_LIST_FULL;
	}
	if (qdf_list_remove_front(&ser_pdev_obj->global_cmd_pool_list,
				&nnode) != QDF_STATUS_SUCCESS) {
		serialization_err("Failed to get cmd buffer from global pool");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}
	cmd_list = qdf_container_of(nnode,
			struct wlan_serialization_command_list, node);
	qdf_mem_copy(&cmd_list->cmd, cmd,
			sizeof(struct wlan_serialization_command));
	if (cmd->is_high_priority)
		qdf_status = qdf_list_insert_front(queue, &cmd_list->node);
	else
		qdf_status = qdf_list_insert_back(queue, &cmd_list->node);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		serialization_err("can't queue command for cmd_id-%d type-%d",
				cmd->cmd_id, cmd->cmd_type);
		qdf_mem_zero(&cmd_list->cmd,
				sizeof(struct wlan_serialization_command));
		qdf_status = qdf_list_insert_back(
					&ser_pdev_obj->global_cmd_pool_list,
					&cmd_list->node);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			serialization_err("can't put cmd back to global pool");
			QDF_ASSERT(0);
		}
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	if (is_cmd_for_active_queue) {
		/*
		 * command is already pushed to active queue above
		 * now start the timer and notify requestor
		 */
		wlan_serialization_find_and_start_timer(psoc,
							&cmd_list->cmd);
		if (cmd_list->cmd.cmd_cb) {
			/*
			 * Remember that serialization module may send
			 * this callback in same context through which it
			 * received the serialization request. Due to which
			 * it is caller's responsibility to ensure acquiring
			 * and releasing its own lock appropriately.
			 */
			qdf_status = cmd_list->cmd.cmd_cb(&cmd_list->cmd,
						WLAN_SER_CB_ACTIVATE_CMD);
			if (qdf_status != QDF_STATUS_SUCCESS) {
				wlan_serialization_find_and_stop_timer(psoc,
								&cmd_list->cmd);
				cmd_list->cmd.cmd_cb(&cmd_list->cmd,
						WLAN_SER_CB_RELEASE_MEM_CMD);
				wlan_serialization_put_back_to_global_list(
						queue, ser_pdev_obj, cmd_list);
				wlan_serialization_move_pending_to_active(
						cmd_list->cmd.cmd_type,
						ser_pdev_obj);
			}
		}
		status = WLAN_SER_CMD_ACTIVE;
	} else {
		status = WLAN_SER_CMD_PENDING;
	}

	return status;
}

enum wlan_serialization_status
wlan_serialization_enqueue_cmd(struct wlan_serialization_command *cmd,
			       uint8_t is_cmd_for_active_queue)
{
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	qdf_list_t *queue;

	/* Enqueue process
	 * 1) peek through command structure and see what is the command type
	 * 2) two main types of commands to process
	 *    a) SCAN
	 *    b) NON-SCAN
	 * 3) for each command there are seperate command queues per pdev
	 * 4) pull pdev from vdev structure and get the command queue associated
	 *    with that pdev and try to enqueue on those queue
	 * 5) Thumb rule:
	 *    a) There could be only 1 active non-scan command at a
	 *       time including all total non-scan commands of all pdevs.
	 *
	 *       example: pdev1 has 1 non-scan active command and
	 *       pdev2 got 1 non-scan command then that command should go to
	 *       pdev2's pending queue
	 *
	 *    b) There could be only N number of scan commands at a time
	 *       including all total scan commands of all pdevs
	 *
	 *       example: Let's say N=8,
	 *       pdev1's vdev1 has 5 scan command, pdev2's vdev1 has 3
	 *       scan commands, if we get scan request on vdev2 then it will go
	 *       to pending queue of vdev2 as we reached max allowed scan active
	 *       command.
	 */
	if (!cmd) {
		serialization_err("NULL command");
		return status;
	}
	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (pdev == NULL) {
		serialization_err("invalid pdev");
		return status;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (psoc == NULL) {
		serialization_err("invalid psoc");
		return status;
	}

	/* get priv object by wlan_objmgr_vdev_get_comp_private_obj */
	ser_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_SERIALIZATION);
	if (!ser_pdev_obj) {
		serialization_err("Can't find ser_pdev_obj");
		return status;
	}

	serialization_debug("command high_priority[%d] cmd_type[%d] cmd_id[%d]",
			cmd->is_high_priority, cmd->cmd_type, cmd->cmd_id);
	if (cmd->cmd_type < WLAN_SER_CMD_NONSCAN) {
		if (is_cmd_for_active_queue)
			queue = &ser_pdev_obj->active_scan_list;
		else
			queue = &ser_pdev_obj->pending_scan_list;
	} else {
		if (is_cmd_for_active_queue)
			queue = &ser_pdev_obj->active_list;
		else
			queue = &ser_pdev_obj->pending_list;
	}

	if (wlan_serialization_is_cmd_present_queue(cmd,
				is_cmd_for_active_queue)) {
		serialization_err("duplicate command, can't enqueue");
		return status;
	}

	return wlan_serialization_add_cmd_to_given_queue(queue, cmd, psoc,
			ser_pdev_obj, is_cmd_for_active_queue);
}

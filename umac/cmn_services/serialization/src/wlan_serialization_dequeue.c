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
 * DOC: wlan_serialization_dequeue.c
 * This file defines the routines which are pertinent
 * to the dequeue of commands.
 */
#include <wlan_serialization_api.h>
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>

void wlan_serialization_move_pending_to_active(
		enum wlan_serialization_cmd_type cmd_type,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj)
{
	qdf_list_t *pending_queue;
	struct wlan_serialization_command_list *cmd_list;
	enum wlan_serialization_status status;
	qdf_list_node_t *nnode = NULL;

	if (!ser_pdev_obj) {
		serialization_err("Can't find ser_pdev_obj");
		return;
	}

	if (cmd_type < WLAN_SER_CMD_NONSCAN)
		pending_queue = &ser_pdev_obj->pending_scan_list;
	else
		pending_queue = &ser_pdev_obj->pending_list;

	if (qdf_list_empty(pending_queue)) {
		serialization_info("nothing to move from pend to active queue");
		serialization_info("cmd_type - %d", cmd_type);
		return;
	}
	if (QDF_STATUS_SUCCESS != qdf_list_peek_front(pending_queue, &nnode)) {
		serialization_err("can't read from pending queue");
		serialization_info("cmd_type - %d", cmd_type);
		return;
	}
	cmd_list = qdf_container_of(nnode,
			struct wlan_serialization_command_list, node);
	/*
	 * Idea is to peek command from pending queue, and try to
	 * push to active queue. If command goes to active queue
	 * successfully then remove the command from pending queue which
	 * we previously peeked.
	 *
	 * By doing this way, we will make sure that command will be removed
	 * from pending queue only when it was able to make it to active queue
	 */
	status = wlan_serialization_enqueue_cmd(&cmd_list->cmd, true);
	if (WLAN_SER_CMD_ACTIVE != status) {
		serialization_err("Can't move cmd to activeQ id-%d type-%d",
				cmd_list->cmd.cmd_id, cmd_list->cmd.cmd_type);
		return;
	}
	wlan_serialization_put_back_to_global_list(pending_queue,
				ser_pdev_obj, cmd_list);

	return;
}


enum wlan_serialization_cmd_status
wlan_serialization_remove_all_cmd_from_queue(qdf_list_t *queue,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		struct wlan_objmgr_pdev *pdev, struct wlan_objmgr_vdev *vdev,
		struct wlan_serialization_command *cmd, uint8_t is_active_queue)
{
	uint32_t qsize;
	struct wlan_serialization_command_list *cmd_list;
	qdf_list_node_t *nnode = NULL, *pnode = NULL;
	enum wlan_serialization_cmd_status status = WLAN_SER_CMD_NOT_FOUND;
	QDF_STATUS qdf_status;

	qsize = qdf_list_size(queue);
	while (qsize--) {
		if (wlan_serialization_get_cmd_from_queue(queue, &nnode)
				!= QDF_STATUS_SUCCESS) {
			serialization_err("can't read cmd from queue");
			status = WLAN_SER_CMD_NOT_FOUND;
			break;
		}
		cmd_list = qdf_container_of(nnode,
				struct wlan_serialization_command_list, node);
		if ((cmd && ((cmd->cmd_id != cmd_list->cmd.cmd_id) ||
				(cmd->cmd_type != cmd_list->cmd.cmd_type))) ||
			(vdev && (vdev != cmd_list->cmd.vdev)) ||
			(pdev && (pdev != wlan_vdev_get_pdev(
						cmd_list->cmd.vdev)))) {
			pnode = nnode;
			continue;
		}
		/*
		 * active queue can't be removed directly, requester needs to
		 * wait for active command response and send remove request for
		 * active command seperately
		 */
		if (is_active_queue) {
			status = WLAN_SER_CMD_IN_ACTIVE_LIST;
			break;
		}
		/*
		 * call pending cmd's callback to notify that
		 * it is being removed
		 */
		if (cmd_list->cmd.cmd_cb) {
			/* caller should now do necessary clean up */
			cmd_list->cmd.cmd_cb(&cmd_list->cmd,
					WLAN_SER_CB_CANCEL_CMD);
			/* caller should release the memory */
			cmd_list->cmd.cmd_cb(&cmd_list->cmd,
					WLAN_SER_CB_RELEASE_MEM_CMD);
		}

		qdf_status = wlan_serialization_put_back_to_global_list(queue,
				ser_pdev_obj, cmd_list);
		if (QDF_STATUS_SUCCESS != qdf_status) {
			serialization_err("can't remove cmd from queue");
			status = WLAN_SER_CMD_NOT_FOUND;
			break;
		}
		nnode = pnode;
		/* If cmd was on active list then we wouldn't have come here */
		status = WLAN_SER_CMD_IN_PENDING_LIST;
	}

	return status;
}

/**
 * wlan_serialization_remove_cmd_from_given_queue() - to remove command from
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
static void wlan_serialization_remove_cmd_from_given_queue(qdf_list_t *queue,
		struct wlan_serialization_command *cmd,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj)
{
	uint32_t qsize;
	struct wlan_serialization_command_list *cmd_list;
	qdf_list_node_t *nnode = NULL;
	QDF_STATUS status;

	if (!cmd)
		return;

	qsize = qdf_list_size(queue);

	while (qsize--) {
		status = wlan_serialization_get_cmd_from_queue(queue, &nnode);
		if (status != QDF_STATUS_SUCCESS) {
			serialization_err("can't peek cmd_id[%d] type[%d]",
					  cmd->cmd_id, cmd->cmd_type);
			break;
		}
		cmd_list = qdf_container_of(nnode,
				struct wlan_serialization_command_list, node);
		if ((cmd_list->cmd.cmd_id != cmd->cmd_id) ||
				(cmd_list->cmd.cmd_type != cmd->cmd_type))
			continue;
		if (cmd_list->cmd.vdev != cmd->vdev)
			continue;
		if (cmd_list->cmd.cmd_cb) {
			/* caller should release the memory */
			cmd_list->cmd.cmd_cb(&cmd_list->cmd,
					WLAN_SER_CB_RELEASE_MEM_CMD);
		}
		status = wlan_serialization_put_back_to_global_list(queue,
				ser_pdev_obj, cmd_list);

		if (QDF_STATUS_SUCCESS != status)
			serialization_err("Fail to add to free pool type[%d]",
					  cmd->cmd_type);
		/*
		 * zero out the command, so caller would know that command has
		 * been removed
		 */
		qdf_mem_zero(cmd, sizeof(struct wlan_serialization_command));
		break;
	}
}

/**
 * wlan_serialization_remove_cmd_from_active_queue() - helper function to remove
 *							cmd from active queue
 * @psoc: pointer to psoc
 * @obj: pointer to object getting passed by object manager
 * @arg: argument passed by caller to object manager which comes to this cb
 *
 * caller provide this API as callback to object manager, and in turn
 * object manager iterate through each pdev and call this API callback.
 *
 * Return: none
 */
static void
wlan_serialization_remove_cmd_from_active_queue(struct wlan_objmgr_psoc *psoc,
		void *obj, void *arg)
{
	qdf_list_t *queue;
	struct wlan_objmgr_pdev *pdev = obj;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command *cmd = arg;

	if (!pdev || !cmd) {
		serialization_err("Invalid param");
		return;
	}

	ser_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_SERIALIZATION);
	if (!ser_pdev_obj) {
		serialization_err("Invalid ser_pdev_obj");
		return;
	}

	if (cmd->cmd_type < WLAN_SER_CMD_NONSCAN)
		queue = &ser_pdev_obj->active_scan_list;
	else
		queue = &ser_pdev_obj->active_list;

	if (qdf_list_empty(queue)) {
		serialization_err("Empty queue");
		return;
	}

	wlan_serialization_remove_cmd_from_given_queue(queue, cmd,
			ser_pdev_obj);

	return;
}

/**
 * wlan_serialization_remove_cmd_from_active_queue() - helper function to remove
 *							cmd from pending queue
 * @psoc: pointer to psoc
 * @obj: pointer to object getting passed by object manager
 * @arg: argument passed by caller to object manager which comes to this cb
 *
 * caller provide this API as callback to object manager, and in turn
 * object manager iterate through each pdev and call this API callback.
 *
 * Return: none
 */
static void
wlan_serialization_remove_cmd_from_pending_queue(struct wlan_objmgr_psoc *psoc,
		void *obj, void *arg)
{
	qdf_list_t *queue;
	struct wlan_objmgr_pdev *pdev = obj;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command *cmd = arg;

	if (!pdev || !cmd) {
		serialization_err("Invalid param");
		return;
	}

	ser_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_SERIALIZATION);
	if (!ser_pdev_obj) {
		serialization_err("Invalid ser_pdev_obj");
		return;
	}

	if (cmd->cmd_type < WLAN_SER_CMD_NONSCAN)
		queue = &ser_pdev_obj->pending_scan_list;
	else
		queue = &ser_pdev_obj->pending_list;

	if (qdf_list_empty(queue)) {
		serialization_err("Empty queue");
		return;
	}
	wlan_serialization_remove_cmd_from_given_queue(queue,
			cmd, ser_pdev_obj);

	return;
}

/**
 * wlan_serialization_is_cmd_removed() - to check if requested command is
 *						removed
 * @psoc: pointer to soc
 * @cmd: given command to remove
 * @check_active_queue: flag to find out whether command needs to be removed
 *			from active queue or pending queue
 *
 * Return: true if removed else false
 */
static bool
wlan_serialization_is_cmd_removed(struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd,
		bool check_active_queue)
{
	if (!psoc) {
		serialization_err("Invalid psoc");
		return false;
	}

	if (check_active_queue)
		wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
			wlan_serialization_remove_cmd_from_active_queue,
				cmd, 1, WLAN_SERIALIZATION_ID);
	else
		wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
			wlan_serialization_remove_cmd_from_pending_queue,
			cmd, 1, WLAN_SERIALIZATION_ID);

	if (cmd->vdev == NULL)
		return true;

	return false;
}

enum wlan_serialization_cmd_status
wlan_serialization_dequeue_cmd(struct wlan_serialization_command *cmd,
		uint8_t only_active_cmd)
{
	enum wlan_serialization_cmd_status status = WLAN_SER_CMD_NOT_FOUND;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command cmd_backup;
	enum wlan_serialization_cmd_type cmd_type;
	bool is_cmd_removed;

	if (!cmd) {
		serialization_err("NULL command");
		return status;
	}
	/* Dequeue process
	 * 1) peek through command structure and see what is the command type
	 * 2) two main types of commands to process
	 *    a) SCAN
	 *    b) NON-SCAN
	 * 3) for each command there are seperate command queues per pdev
	 * 4) iterate through every pdev object and find the command and remove
	 */

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
		serialization_err("ser_pdev_obj is empty");
		return status;
	}
	serialization_info("command high_priority[%d] cmd_type[%d] cmd_id[%d]",
			cmd->is_high_priority, cmd->cmd_type, cmd->cmd_id);
	/*
	 *  Pass the copy of command, instead of actual command because
	 *  wlan_serialization_is_cmd_removed() api cleans the command
	 *  buffer up on successful removal. We may need to use the command's
	 *  content to stop the timer and etc.
	 */
	qdf_mem_copy(&cmd_backup, cmd,
			sizeof(struct wlan_serialization_command));

	cmd_type = cmd->cmd_type;
	/* find and remove from active list */
	if (only_active_cmd) {
		wlan_serialization_find_and_stop_timer(psoc, cmd);
		is_cmd_removed = wlan_serialization_is_cmd_removed(psoc,
						&cmd_backup, true);
		if (true == is_cmd_removed) {
			/*
			 * command is removed from active queue. now we have a
			 * room in active queue, so we will move from relevant
			 * pending queue to active queue
			 */
			wlan_serialization_move_pending_to_active(cmd_type,
					ser_pdev_obj);
			status = WLAN_SER_CMD_IN_ACTIVE_LIST;
		} else {
			serialization_err("cmd_type[%d], cmd_id[%d], vdev[%pK]",
					cmd->cmd_type, cmd->cmd_id, cmd->vdev);
			/*
			 * if you come here means there is a possibility
			 * that we couldn't find the command in active queue
			 * which user has requested to remove or we couldn't
			 * remove command from active queue and timer has been
			 * stopped, so active queue may possibly stuck.
			 */
			QDF_ASSERT(0);
			status = WLAN_SER_CMD_NOT_FOUND;
		}
		serialization_info("Request to remove only from active queue");
		return status;
	}
	qdf_mem_copy(&cmd_backup, cmd,
			sizeof(struct wlan_serialization_command));
	/* find and remove from pending list */
	if (wlan_serialization_is_cmd_removed(psoc, &cmd_backup, false)) {
		if (status != WLAN_SER_CMD_IN_ACTIVE_LIST)
			status = WLAN_SER_CMD_IN_PENDING_LIST;
		else
			status = WLAN_SER_CMDS_IN_ALL_LISTS;
	}

	return status;
}

/**
 * wlan_serialization_cmd_cancel_handler() - helper func to cancel cmd
 * @ser_obj: private pdev ser obj
 * @cmd: pointer to command
 * @pdev: pointer to pdev
 * @vdev: pointer to vdev
 * @cmd_type: pointer to cmd_type
 *
 * This API will decide from which queue, command needs to be cancelled
 * and pass that queue and other parameter required to cancel the command
 * to helper function.
 *
 * Return: wlan_serialization_cmd_status
 */
static enum wlan_serialization_cmd_status
wlan_serialization_cmd_cancel_handler(
		struct wlan_serialization_pdev_priv_obj *ser_obj,
		struct wlan_serialization_command *cmd,
		struct wlan_objmgr_pdev *pdev, struct wlan_objmgr_vdev *vdev,
		enum wlan_serialization_cmd_type cmd_type)
{
	enum wlan_serialization_cmd_status status;
	qdf_list_t *queue;

	if (!ser_obj) {
		serialization_err("invalid serial object");
		return WLAN_SER_CMD_NOT_FOUND;
	}
	/* remove pending commands first */
	if (cmd_type < WLAN_SER_CMD_NONSCAN)
		queue = &ser_obj->pending_scan_list;
	else
		queue = &ser_obj->pending_list;
	/* try and remove first from pending list */
	status = wlan_serialization_remove_all_cmd_from_queue(queue,
			ser_obj, pdev, vdev, cmd, false);
	if (cmd_type < WLAN_SER_CMD_NONSCAN)
		queue = &ser_obj->active_scan_list;
	else
		queue = &ser_obj->active_list;
	/* try and remove next from active list */
	if (WLAN_SER_CMD_IN_ACTIVE_LIST ==
			wlan_serialization_remove_all_cmd_from_queue(queue,
				ser_obj, pdev, vdev, cmd, true)) {
		if (WLAN_SER_CMD_IN_PENDING_LIST == status)
			status = WLAN_SER_CMDS_IN_ALL_LISTS;
		else
			status = WLAN_SER_CMD_IN_ACTIVE_LIST;
	}

	return status;
}

enum wlan_serialization_cmd_status
wlan_serialization_find_and_cancel_cmd(
		struct wlan_serialization_queued_cmd_info *cmd_info)
{
	struct wlan_serialization_command cmd;
	enum wlan_serialization_cmd_status status = WLAN_SER_CMD_NOT_FOUND;
	struct wlan_serialization_pdev_priv_obj *ser_obj = NULL;
	struct wlan_objmgr_pdev *pdev;

	if (!cmd_info) {
		serialization_err("Invalid cmd_info");
		return WLAN_SER_CMD_NOT_FOUND;
	}
	cmd.cmd_id = cmd_info->cmd_id;
	cmd.cmd_type = cmd_info->cmd_type;
	cmd.vdev = cmd_info->vdev;
	pdev = wlan_serialization_get_pdev_from_cmd(&cmd);
	if (!pdev) {
		serialization_err("Invalid pdev");
		return WLAN_SER_CMD_NOT_FOUND;
	}
	ser_obj = wlan_serialization_get_pdev_priv_obj(pdev);
	if (!ser_obj) {
		serialization_err("Invalid ser_obj");
		return WLAN_SER_CMD_NOT_FOUND;
	}

	switch (cmd_info->req_type) {
	case WLAN_SER_CANCEL_SINGLE_SCAN:
		/* remove scan cmd which matches the given cmd struct */
		status =  wlan_serialization_cmd_cancel_handler(ser_obj,
				&cmd, NULL, NULL, cmd.cmd_type);
		break;
	case WLAN_SER_CANCEL_PDEV_SCANS:
		/* remove all scan cmds which matches the pdev object */
		status = wlan_serialization_cmd_cancel_handler(ser_obj,
				NULL,
				wlan_vdev_get_pdev(cmd.vdev),
				NULL, cmd.cmd_type);
		break;
	case WLAN_SER_CANCEL_VDEV_SCANS:
		/* remove all scan cmds which matches the vdev object */
		status = wlan_serialization_cmd_cancel_handler(ser_obj,
				NULL, NULL,
				cmd.vdev, cmd.cmd_type);
		break;
	case WLAN_SER_CANCEL_NON_SCAN_CMD:
		/* remove nonscan cmd which matches the given cmd */
		status = wlan_serialization_cmd_cancel_handler(ser_obj,
				&cmd, NULL, NULL, cmd.cmd_type);
		break;
	default:
		serialization_err("Invalid request");
	}

	return status;
}

QDF_STATUS wlan_serialization_find_and_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd_info)
{
	struct wlan_serialization_command cmd;

	if (!cmd_info) {
		serialization_err("Invalid cmd_info");
		return QDF_STATUS_E_FAILURE;
	}

	cmd.cmd_id = cmd_info->cmd_id;
	cmd.cmd_type = cmd_info->cmd_type;
	cmd.vdev = cmd_info->vdev;
	if (WLAN_SER_CMD_IN_ACTIVE_LIST !=
			wlan_serialization_dequeue_cmd(&cmd, true)) {
		serialization_err("Can't dequeue requested cmd_id[%d] type[%d]",
				cmd_info->cmd_id, cmd_info->cmd_type);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}


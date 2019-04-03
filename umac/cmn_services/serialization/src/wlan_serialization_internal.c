/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_internal.c
 * This file defines the functions which are called
 * from serialization public API's and are internal
 * to serialization.
 */

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>
#include <qdf_status.h>
#include <wlan_utility.h>
#include "wlan_serialization_api.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_non_scan_i.h"
#include "wlan_serialization_scan_i.h"
#include "wlan_serialization_internal_i.h"

bool wlan_serialization_is_cmd_present_queue(
			struct wlan_serialization_command *cmd,
			uint8_t is_active_queue)
{
	qdf_list_t *queue;
	bool status = false;
	enum wlan_serialization_node node_type;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	enum wlan_serialization_cmd_type cmd_type;

	if (!cmd) {
		ser_err("invalid cmd");
		goto error;
	}

	cmd_type = cmd->cmd_type;

	ser_pdev_obj = wlan_serialization_get_pdev_obj(
			wlan_serialization_get_pdev_from_cmd(cmd));

	if (!ser_pdev_obj) {
		ser_err("invalid ser vdev obj");
		goto error;
	}

	ser_vdev_obj = wlan_serialization_get_vdev_obj(
			wlan_serialization_get_vdev_from_cmd(cmd));
	if (!ser_vdev_obj) {
		ser_err("invalid ser pdev obj");
		goto error;
	}

	if (cmd_type < WLAN_SER_CMD_NONSCAN) {
		queue = wlan_serialization_get_list_from_pdev_queue(
				ser_pdev_obj, cmd_type, is_active_queue);
		node_type = WLAN_SER_PDEV_NODE;
	} else {
		queue = wlan_serialization_get_list_from_vdev_queue(
				ser_vdev_obj, cmd_type, is_active_queue);
		node_type = WLAN_SER_VDEV_NODE;
	}

	status = wlan_serialization_is_cmd_present_in_given_queue(queue, cmd,
								  node_type);

error:
	return status;
}

enum wlan_serialization_status
wlan_serialization_enqueue_cmd(struct wlan_serialization_command *cmd,
			       enum ser_queue_reason ser_reason)
{
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
	struct wlan_serialization_command_list *cmd_list;
	qdf_list_node_t *nnode;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_pdev_queue *pdev_queue;
	struct wlan_ser_vdev_obj *ser_vdev_obj;
	struct wlan_serialization_vdev_queue *vdev_queue;
	bool active_queue;

	/* Enqueue process
	 * 1) peek through command structure and see what is the command type
	 * 2) two main types of commands to process
	 *    a) SCAN
	 *    b) NON-SCAN
	 * 3) for each command there are separate command queues per pdev
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

	ser_enter();

	if (!cmd) {
		ser_err("NULL command");
		goto error;
	}

	if (!cmd->cmd_cb) {
		ser_err("no cmd_cb for cmd type:%d, id: %d",
			cmd->cmd_type,
			cmd->cmd_id);
		goto error;
	}

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		ser_err("pdev is invalid");
		goto error;
	}

	ser_pdev_obj =
		wlan_objmgr_pdev_get_comp_private_obj(
				pdev,
				WLAN_UMAC_COMP_SERIALIZATION);
	if (!ser_pdev_obj) {
		ser_err("Invalid ser_pdev_obj");
		goto error;
	}

	pdev_queue = wlan_serialization_get_pdev_queue_obj(ser_pdev_obj,
							   cmd->cmd_type);
	if (!pdev_queue) {
		ser_err("pdev_queue is invalid");
		goto error;
	}

	ser_debug("enqueue cmd: type[%d] id[%d] high_priority[%d] blocking[%d]",
		  cmd->cmd_type,
		  cmd->cmd_id,
		  cmd->is_high_priority,
		  cmd->is_blocking);

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	/* Before queuing any non scan command,
	 * as part of wlan_serialization_request,
	 * we check if the vdev queues are disabled.
	 *
	 * The serialization command structure has an
	 * attribute, where after a given command is queued,
	 * we can block the vdev queues.
	 *
	 * For example, after VDEV_DOWN command is queued as
	 * part of a vdev deletion, no other commands should be queued
	 * until the deletion is complete, so with VDEV_DOWN(in case of
	 * vdev deletion) with pass the attribute to disable vdev queues
	 */
	if (cmd->cmd_type > WLAN_SER_CMD_SCAN &&
	    ser_reason == SER_REQUEST) {
		ser_vdev_obj =
			wlan_serialization_get_vdev_obj(
				wlan_serialization_get_vdev_from_cmd(cmd));

		if (!ser_vdev_obj) {
			wlan_serialization_release_lock(
				&pdev_queue->pdev_queue_lock);
			goto error;
		}

		vdev_queue =
			wlan_serialization_get_vdev_queue_obj(
				ser_vdev_obj,
				cmd->cmd_type);

		if (!vdev_queue) {
			wlan_serialization_release_lock(
				&pdev_queue->pdev_queue_lock);
			goto error;
		}

		if (vdev_queue->queue_disable) {
			wlan_serialization_release_lock(
				&pdev_queue->pdev_queue_lock);
			ser_err("VDEV queue is disabled, ser request denied");
			ser_err("cmd id[%d] cmd type[%d]", cmd->cmd_id,
				cmd->cmd_type);
			status = WLAN_SER_CMD_QUEUE_DISABLED;
			goto error;
		}
	}

	active_queue = wlan_serialization_is_active_cmd_allowed(cmd);

	if (wlan_serialization_is_cmd_present_queue(cmd, active_queue)) {
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		ser_err("duplicate command, can't enqueue");
		goto error;
	}

	if (wlan_serialization_remove_front(
				&pdev_queue->cmd_pool_list,
				&nnode) != QDF_STATUS_SUCCESS) {
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		ser_err("Failed to get cmd buffer from global pool");
		status = WLAN_SER_CMD_DENIED_LIST_FULL;
		goto error;
	}

	ser_debug("Global pool node: %pK", nnode);

	cmd_list =
		qdf_container_of(nnode,
				 struct wlan_serialization_command_list,
				 pdev_node);

	qdf_mem_copy(&cmd_list->cmd, cmd,
		     sizeof(struct wlan_serialization_command));

	if (cmd->cmd_type < WLAN_SER_CMD_NONSCAN) {
		status = wlan_ser_add_scan_cmd(ser_pdev_obj,
					       cmd_list,
					       active_queue);
	} else {
		status = wlan_ser_add_non_scan_cmd(ser_pdev_obj,
						   cmd_list,
						   active_queue);
	}

	if (status != WLAN_SER_CMD_PENDING && status != WLAN_SER_CMD_ACTIVE) {
		qdf_mem_zero(&cmd_list->cmd,
			     sizeof(struct wlan_serialization_command));
		cmd_list->cmd_in_use = 0;
		wlan_serialization_insert_back(
			&pdev_queue->cmd_pool_list,
			&cmd_list->pdev_node);
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		ser_err("Failed to add cmd to active/pending queue");
		goto error;
	}

	if (WLAN_SER_CMD_ACTIVE == status) {
		qdf_atomic_set_bit(CMD_MARKED_FOR_ACTIVATION,
				   &cmd_list->cmd_in_use);
	}

	wlan_ser_update_cmd_history(pdev_queue, &cmd_list->cmd,
				    ser_reason, true, active_queue);

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

	if (WLAN_SER_CMD_ACTIVE == status)
		wlan_serialization_activate_cmd(cmd_list,
						ser_pdev_obj, ser_reason);

error:
	ser_exit();

	return status;
}

QDF_STATUS wlan_serialization_activate_cmd(
			struct wlan_serialization_command_list *cmd_list,
			struct wlan_ser_pdev_obj *ser_pdev_obj,
			enum ser_queue_reason ser_reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_serialization_pdev_queue *pdev_queue;

	pdev_queue = wlan_serialization_get_pdev_queue_obj(
			ser_pdev_obj, cmd_list->cmd.cmd_type);

	psoc = wlan_vdev_get_psoc(cmd_list->cmd.vdev);
	if (!psoc) {
		ser_err("invalid psoc");
		goto error;
	}

	/*
	 * command is already pushed to active queue above
	 * now start the timer and notify requestor
	 */
	wlan_serialization_find_and_start_timer(psoc, &cmd_list->cmd);
	/*
	 * Remember that serialization module may send
	 * this callback in same context through which it
	 * received the serialization request. Due to which
	 * it is caller's responsibility to ensure acquiring
	 * and releasing its own lock appropriately.
	 */

	ser_debug("cmd cb: type[%d] id[%d] : reason: %s",
		  cmd_list->cmd.cmd_type,
		  cmd_list->cmd.cmd_id,
		  "WLAN_SER_CB_ACTIVATE_CMD");

	cmd_list->cmd.activation_reason = ser_reason;

	status = cmd_list->cmd.cmd_cb(&cmd_list->cmd,
				WLAN_SER_CB_ACTIVATE_CMD);

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	qdf_atomic_clear_bit(CMD_MARKED_FOR_ACTIVATION,
			     &cmd_list->cmd_in_use);
	qdf_atomic_set_bit(CMD_IS_ACTIVE,
			   &cmd_list->cmd_in_use);

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_serialization_dequeue_cmd(&cmd_list->cmd,
					       SER_ACTIVATION_FAILED,
					       true);
		return status;
	}

	/*
	 * Cmd was marked for activation and delete or cancel
	 * is received before activation completed, then the command
	 * should be immediately removed after activation
	 */
	if (qdf_atomic_test_bit(CMD_ACTIVE_MARKED_FOR_REMOVAL,
				&cmd_list->cmd_in_use)) {
		wlan_serialization_dequeue_cmd(&cmd_list->cmd,
					       SER_REMOVE,
					       true);
		return status;
	}

	if (qdf_atomic_test_bit(CMD_ACTIVE_MARKED_FOR_CANCEL,
				&cmd_list->cmd_in_use))
		wlan_serialization_cmd_cancel_handler(
				ser_pdev_obj, &cmd_list->cmd,
				NULL, NULL, cmd_list->cmd.cmd_type,
				WLAN_SERIALIZATION_ACTIVE_QUEUE);
error:
	return status;
}

bool
wlan_serialization_is_active_cmd_allowed(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_pdev *pdev;
	bool active_cmd_allowed = 0;

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		ser_err("NULL pdev");
		goto error;
	}

	if (cmd->cmd_type < WLAN_SER_CMD_NONSCAN)
		active_cmd_allowed =
		(wlan_serialization_is_active_scan_cmd_allowed(cmd) &&
			wlan_serialization_is_scan_pending_queue_empty(cmd));
	else
		active_cmd_allowed =
		(wlan_serialization_is_active_non_scan_cmd_allowed(cmd) &&
		 wlan_serialization_is_non_scan_pending_queue_empty(cmd));

	ser_debug("active cmd_type[%d] cmd_id[%d] allowed: %d",
		  cmd->cmd_type,
		  cmd->cmd_id,
		  active_cmd_allowed);

error:
	return active_cmd_allowed;
}

enum wlan_serialization_status
wlan_serialization_move_pending_to_active(
		enum wlan_serialization_cmd_type cmd_type,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_vdev *vdev,
		bool blocking_cmd_removed)
{
	enum wlan_serialization_status status;

	if (cmd_type < WLAN_SER_CMD_NONSCAN) {
		status =
		wlan_ser_move_scan_pending_to_active(
				ser_pdev_obj);
	} else {
		status =
		wlan_ser_move_non_scan_pending_to_active(
				ser_pdev_obj,
				vdev,
				blocking_cmd_removed);
	}

	return status;
}

enum wlan_serialization_cmd_status
wlan_serialization_dequeue_cmd(struct wlan_serialization_command *cmd,
			       enum ser_queue_reason ser_reason,
			       uint8_t active_cmd)
{
	enum wlan_serialization_cmd_status status =
		WLAN_SER_CMD_NOT_FOUND;
	enum wlan_serialization_status ser_status =
		WLAN_SER_CMD_DENIED_UNSPECIFIED;

	QDF_STATUS qdf_status;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command cmd_bkup;
	struct wlan_serialization_command_list *cmd_list;
	struct wlan_serialization_pdev_queue *pdev_queue;
	bool blocking_cmd_removed = 0;

	ser_enter();

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

	ser_pdev_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_pdev_obj) {
		ser_err("ser_pdev_obj is empty");
		goto error;
	}

	pdev_queue = wlan_serialization_get_pdev_queue_obj(
			ser_pdev_obj, cmd->cmd_type);

	ser_debug("dequeue cmd: type[%d] id[%d] high_priority[%d] blocking[%d]",
		  cmd->cmd_type,
		  cmd->cmd_id,
		  cmd->is_high_priority,
		  cmd->is_blocking);

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);

	if (cmd->cmd_type < WLAN_SER_CMD_NONSCAN)
		qdf_status = wlan_ser_remove_scan_cmd(
				ser_pdev_obj, &cmd_list, cmd, active_cmd);
	else {
		qdf_status = wlan_ser_remove_non_scan_cmd(
				ser_pdev_obj, &cmd_list, cmd, active_cmd);
	}

	if (qdf_status == QDF_STATUS_E_PENDING) {
		status = WLAN_SER_CMD_MARKED_FOR_ACTIVATION;
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		goto error;
	}

	if (qdf_status != QDF_STATUS_SUCCESS) {
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		status = WLAN_SER_CMD_NOT_FOUND;
		goto error;
	}

	if (active_cmd) {
		if (cmd_list->cmd.cmd_type >= WLAN_SER_CMD_NONSCAN)
			blocking_cmd_removed = cmd_list->cmd.is_blocking;
	}

	if (active_cmd)
		wlan_serialization_find_and_stop_timer(psoc, &cmd_list->cmd);

	qdf_mem_copy(&cmd_bkup, &cmd_list->cmd,
		     sizeof(struct wlan_serialization_command));
	qdf_mem_zero(&cmd_list->cmd,
		     sizeof(struct wlan_serialization_command));
	cmd_list->cmd_in_use = 0;
	qdf_status = wlan_serialization_insert_back(
			&pdev_queue->cmd_pool_list,
			&cmd_list->pdev_node);

	wlan_ser_update_cmd_history(pdev_queue, &cmd_bkup, ser_reason,
				    false, active_cmd);

	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

	/* Call cmd cb for remove request*/
	if (cmd_bkup.cmd_cb) {
		/* caller should release the memory */
		ser_debug("cmd cb: type[%d] id[%d]: reason: %s",
			  cmd_bkup.cmd_type,
			  cmd_bkup.cmd_id,
			  "WLAN_SER_CB_RELEASE_MEM_CMD");
		cmd_bkup.cmd_cb(&cmd_bkup,
				     WLAN_SER_CB_RELEASE_MEM_CMD);
	}

	if (active_cmd) {
		ser_status = wlan_serialization_move_pending_to_active(
			cmd_bkup.cmd_type, ser_pdev_obj,
			cmd_bkup.vdev,
			blocking_cmd_removed);
	}

	if (active_cmd)
		status = WLAN_SER_CMD_IN_ACTIVE_LIST;
	else
		status = WLAN_SER_CMD_IN_PENDING_LIST;

error:
	ser_exit();
	return status;
}

void wlan_serialization_generic_timer_cb(void *arg)
{
	struct wlan_serialization_timer *timer = arg;
	struct wlan_serialization_command *cmd = timer->cmd;

	if (!cmd) {
		ser_err("Command not found");
		return;
	}

	ser_err("active cmd timeout for cmd_type[%d] vdev[%pK]",
		cmd->cmd_type, cmd->vdev);

	if (cmd->cmd_cb)
		cmd->cmd_cb(cmd, WLAN_SER_CB_ACTIVE_CMD_TIMEOUT);

	/*
	 * dequeue cmd API will cleanup and destroy the timer. If it fails to
	 * dequeue command then we have to destroy the timer.
	 */
	wlan_serialization_dequeue_cmd(cmd, SER_TIMEOUT, true);
}

static QDF_STATUS wlan_serialization_mc_flush_noop(struct scheduler_msg *msg)
{
	return QDF_STATUS_SUCCESS;
}

static void
wlan_serialization_timer_cb_mc_ctx(void *arg)
{
	struct scheduler_msg msg = {0};

	msg.type = SYS_MSG_ID_MC_TIMER;
	msg.reserved = SYS_MSG_COOKIE;
	msg.callback = wlan_serialization_generic_timer_cb;
	msg.bodyptr = arg;
	msg.bodyval = 0;
	msg.flush_callback = wlan_serialization_mc_flush_noop;

	if (scheduler_post_message(QDF_MODULE_ID_SERIALIZATION,
				   QDF_MODULE_ID_SERIALIZATION,
				   QDF_MODULE_ID_SYS, &msg) ==
							QDF_STATUS_SUCCESS)
		return;

	ser_err("Could not enqueue timer to timer queue");
}

static void wlan_serialization_timer_handler(void *arg)
{
	ser_enter();

	wlan_serialization_timer_cb_mc_ctx(arg);

	ser_exit();
}

QDF_STATUS
wlan_serialization_find_and_update_timer(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd)
{
	struct wlan_ser_psoc_obj *psoc_ser_obj;
	struct wlan_serialization_timer *ser_timer;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int i = 0;

	if (!psoc || !cmd) {
		ser_err("invalid param");
		goto exit;
	}

	psoc_ser_obj = wlan_serialization_get_psoc_obj(psoc);
	/*
	 * Here cmd_id and cmd_type are used to locate the timer being
	 * associated with command.
	 */
	wlan_serialization_acquire_lock(&psoc_ser_obj->timer_lock);

	for (i = 0; psoc_ser_obj->max_active_cmds > i; i++) {
		ser_timer = &psoc_ser_obj->timers[i];
		if (!(ser_timer->cmd) ||
		    (ser_timer->cmd->cmd_id != cmd->cmd_id) ||
		    (ser_timer->cmd->cmd_type != cmd->cmd_type) ||
		    (ser_timer->cmd->vdev != cmd->vdev))
			continue;

		qdf_timer_mod(&ser_timer->timer,
			      cmd->cmd_timeout_duration);
		status = QDF_STATUS_SUCCESS;
		break;
	}

	wlan_serialization_release_lock(&psoc_ser_obj->timer_lock);

	if (QDF_IS_STATUS_SUCCESS(status))
		ser_debug("Updated the timer for cmd type:%d, id: %d",
			  cmd->cmd_type, cmd->cmd_id);
	else
		ser_err("Can't find timer for cmd_type[%d]", cmd->cmd_type);

exit:
	return status;
}

QDF_STATUS
wlan_serialization_find_and_stop_timer(struct wlan_objmgr_psoc *psoc,
				       struct wlan_serialization_command *cmd)
{
	struct wlan_ser_psoc_obj *psoc_ser_obj;
	struct wlan_serialization_timer *ser_timer;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	int i = 0;
	uint32_t phy_version;

	if (!psoc || !cmd) {
		ser_err("invalid param");
		goto exit;
	}

	if (cmd->cmd_timeout_duration == 0) {
		phy_version = wlan_psoc_get_nif_phy_version(psoc);
		if (wlan_is_emulation_platform(phy_version)) {
			ser_err("[SCAN-EMULATION]: Not performing timer funcs");
			status = QDF_STATUS_SUCCESS;
		goto exit;
		}
	}

	psoc_ser_obj = wlan_serialization_get_psoc_obj(psoc);
	/*
	 * Here cmd_id and cmd_type are used to locate the timer being
	 * associated with command.
	 */
	wlan_serialization_acquire_lock(&psoc_ser_obj->timer_lock);

	for (i = 0; psoc_ser_obj->max_active_cmds > i; i++) {
		ser_timer = &psoc_ser_obj->timers[i];
		if (!(ser_timer->cmd) ||
		    (ser_timer->cmd->cmd_id != cmd->cmd_id) ||
		    (ser_timer->cmd->cmd_type != cmd->cmd_type) ||
		    (ser_timer->cmd->vdev != cmd->vdev))
			continue;

		status = wlan_serialization_stop_timer(ser_timer);
		break;
	}

	wlan_serialization_release_lock(&psoc_ser_obj->timer_lock);

	if (QDF_IS_STATUS_SUCCESS(status))
		ser_debug("Stopped timer for cmd_type %d cmd id %d",
			  cmd->cmd_type, cmd->cmd_id);
	else
		ser_err("Can't find timer for cmd_type %d cmd id %d",
			cmd->cmd_type, cmd->cmd_id);

exit:
	return status;
}

QDF_STATUS
wlan_serialization_find_and_start_timer(struct wlan_objmgr_psoc *psoc,
					struct wlan_serialization_command *cmd)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_ser_psoc_obj *psoc_ser_obj;
	struct wlan_serialization_timer *ser_timer;
	int i = 0;
	uint32_t nif_phy_ver;

	if (!psoc || !cmd) {
		ser_err("invalid param");
		goto error;
	}

	nif_phy_ver = wlan_psoc_get_nif_phy_version(psoc);
	if ((cmd->cmd_timeout_duration == 0) &&
	    (wlan_is_emulation_platform(nif_phy_ver))) {
		ser_err("[SCAN-EMULATION]: Not performing timer functions\n");
		status = QDF_STATUS_SUCCESS;
		goto exit;
	}

	psoc_ser_obj = wlan_serialization_get_psoc_obj(psoc);

	wlan_serialization_acquire_lock(&psoc_ser_obj->timer_lock);

	for (i = 0; psoc_ser_obj->max_active_cmds > i; i++) {
		/* Keep trying timer */
		ser_timer = &psoc_ser_obj->timers[i];
		if (ser_timer->cmd)
			continue;

		/* Remember timer is pointing to command */
		ser_timer->cmd = cmd;
		status = QDF_STATUS_SUCCESS;
		break;
	}

	wlan_serialization_release_lock(&psoc_ser_obj->timer_lock);

	if (QDF_IS_STATUS_SUCCESS(status)) {
		qdf_timer_init(NULL,
			       &ser_timer->timer,
			       wlan_serialization_timer_handler,
			       ser_timer,
			       QDF_TIMER_TYPE_SW);
			       qdf_timer_mod(&ser_timer->timer,
			       cmd->cmd_timeout_duration);

		ser_debug("Started timer for cmd: type[%d] id[%d] high_priority[%d] blocking[%d]",
			  cmd->cmd_type,
			  cmd->cmd_id,
			  cmd->is_high_priority,
			  cmd->is_blocking);
	} else {
		ser_err("Failed to start timer for cmd: type[%d] id[%d] high_priority[%d] blocking[%d]",
			  cmd->cmd_type,
			  cmd->cmd_id,
			  cmd->is_high_priority,
			  cmd->is_blocking);
	}

error:
exit:

	return status;
}

enum wlan_serialization_cmd_status
wlan_serialization_cmd_cancel_handler(
		struct wlan_ser_pdev_obj *ser_obj,
		struct wlan_serialization_command *cmd,
		struct wlan_objmgr_pdev *pdev, struct wlan_objmgr_vdev *vdev,
		enum wlan_serialization_cmd_type cmd_type, uint8_t queue_type)
{
	enum wlan_serialization_cmd_status active_status =
		WLAN_SER_CMD_NOT_FOUND;
	enum wlan_serialization_cmd_status pending_status =
		WLAN_SER_CMD_NOT_FOUND;
	enum wlan_serialization_cmd_status status =
		WLAN_SER_CMD_NOT_FOUND;

	ser_enter();

	if (!ser_obj) {
		ser_err("invalid serial object");
		goto error;
	}

	if (queue_type & WLAN_SERIALIZATION_ACTIVE_QUEUE) {
		if (cmd_type < WLAN_SER_CMD_NONSCAN)
			active_status = wlan_ser_cancel_scan_cmd(
					ser_obj, pdev, vdev, cmd,
					cmd_type, true);
		else
			active_status = wlan_ser_cancel_non_scan_cmd(
					ser_obj, pdev, vdev, cmd,
					cmd_type, true);
	}

	if (queue_type & WLAN_SERIALIZATION_PENDING_QUEUE) {
		if (cmd_type < WLAN_SER_CMD_NONSCAN)
			pending_status = wlan_ser_cancel_scan_cmd(
					ser_obj, pdev, vdev, cmd,
					cmd_type, false);
		else
			pending_status = wlan_ser_cancel_non_scan_cmd(
					ser_obj, pdev, vdev, cmd,
					cmd_type, false);
	}

	if (active_status == WLAN_SER_CMD_IN_ACTIVE_LIST &&
	    pending_status == WLAN_SER_CMD_IN_PENDING_LIST)
		status = WLAN_SER_CMDS_IN_ALL_LISTS;
	else if (active_status == WLAN_SER_CMD_IN_ACTIVE_LIST)
		status = active_status;
	else if (pending_status == WLAN_SER_CMD_IN_PENDING_LIST)
		status = pending_status;

error:
	ser_exit();
	return status;
}

enum wlan_serialization_cmd_status
wlan_serialization_find_and_cancel_cmd(
		struct wlan_serialization_command *cmd,
		enum wlan_serialization_cancel_type req_type,
		uint8_t queue_type)
{
	enum wlan_serialization_cmd_status status = WLAN_SER_CMD_NOT_FOUND;
	struct wlan_ser_pdev_obj *ser_obj = NULL;
	struct wlan_objmgr_pdev *pdev;

	ser_enter();

	if (!cmd) {
		ser_err("Invalid cmd");
		goto error;
	}

	pdev = wlan_serialization_get_pdev_from_cmd(cmd);
	if (!pdev) {
		ser_err("Invalid pdev");
		goto error;
	}
	ser_obj = wlan_serialization_get_pdev_obj(pdev);
	if (!ser_obj) {
		ser_err("Invalid ser_obj");
		goto error;
	}

	switch (req_type) {
	case WLAN_SER_CANCEL_SINGLE_SCAN:
		/* remove scan cmd which matches the given cmd struct */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, cmd, NULL, NULL,
				WLAN_SER_CMD_SCAN, queue_type);
		break;
	case WLAN_SER_CANCEL_PDEV_SCANS:
		/* remove all scan cmds which matches the pdev object */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, NULL, pdev, NULL,
				WLAN_SER_CMD_SCAN, queue_type);
		break;
	case WLAN_SER_CANCEL_VDEV_SCANS:
		/* remove all scan cmds which matches the vdev object */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, NULL, NULL, cmd->vdev,
				WLAN_SER_CMD_SCAN, queue_type);
		break;
	case WLAN_SER_CANCEL_NON_SCAN_CMD:
		/* remove nonscan cmd which matches the given cmd */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, cmd, NULL, NULL,
				WLAN_SER_CMD_NONSCAN, queue_type);
		break;
	case WLAN_SER_CANCEL_PDEV_NON_SCAN_CMD:
		/* remove all non scan cmds which matches the pdev object */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, NULL, pdev, NULL,
				WLAN_SER_CMD_NONSCAN, queue_type);
		break;
	case WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD:
		/* remove all non scan cmds which matches the vdev object */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, NULL, NULL, cmd->vdev,
				WLAN_SER_CMD_NONSCAN, queue_type);
		break;
	case WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD_TYPE:
		/*
		 * remove all non scan cmds which matches the vdev
		 * and given cmd type
		 */
		status = wlan_serialization_cmd_cancel_handler(
				ser_obj, NULL, NULL, cmd->vdev,
				cmd->cmd_type, queue_type);
		break;
	default:
		ser_err("Invalid request");
	}

error:
	ser_exit();
	return status;
}

/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file wlan_vdev_mlme_ser.c
 * This file contains the APIs to support interface between vdev_mlme and
 * serialization module
 */

#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_mem.h>
#include <wlan_serialization_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_cmn.h>
#include <wlan_mlme_dbg.h>
#include <include/wlan_mlme_cmn.h>
#include <wlan_vdev_mlme_api.h>
#include <wlan_vdev_mlme_ser_if.h>

enum wlan_serialization_status
wlan_vdev_mlme_ser_start_bss(struct wlan_serialization_command *cmd)
{
	struct vdev_mlme_obj *vdev_mlme;

	if (!cmd || !cmd->vdev) {
		mlme_err("Null input");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	if (!wlan_ser_is_vdev_queue_enabled(cmd->vdev))
		return WLAN_SER_CMD_QUEUE_DISABLED;
	/*
	 * Serialization command filtering logic
	 * a. Cancel any existing start bss cmd in the pending queue
	 * b. If there is an start bss cmd in active queue and
	 * there is no stop bss cmd in pending queue,
	 * then explicitly enqueue a stop bss cmd to avoid back to
	 * back execution of UP cmd.
	 * c. Enqueue the new start bss cmd with serialization
	 */
	wlan_vdev_mlme_ser_cancel_request(
			cmd->vdev,
			WLAN_SER_CMD_VDEV_START_BSS,
			WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD_TYPE);

	if (wlan_serialization_is_cmd_present_in_active_queue(NULL, cmd)) {
		vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(cmd->vdev);
		if (mlme_vdev_enqueue_exp_ser_cmd(vdev_mlme,
						  WLAN_SER_CMD_VDEV_STOP_BSS)) {
			mlme_err("Unable to add the exception cmd request");
			return WLAN_SER_CMD_DENIED_UNSPECIFIED;
		}
	}

	return wlan_serialization_request(cmd);
}

enum wlan_serialization_status
wlan_vdev_mlme_ser_stop_bss(struct wlan_serialization_command *cmd)
{
	uint8_t stop_cmd_pending;
	uint8_t ret;

	if (!cmd || !cmd->vdev) {
		mlme_err("Null input");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	if (!wlan_ser_is_vdev_queue_enabled(cmd->vdev))
		return WLAN_SER_CMD_QUEUE_DISABLED;
	/*
	 * Serialization command filtering logic
	 * a. Cancel any existing start/stop/restart command in the pending
	 *  queue.
	 * b. If there is a stop cmd in active queue then return
	 * c. Else enqueue the cmd
	 * d. If stop cmd already existed in pending queue then return with
	 *  already exists else return the enqueued return value.
	 */
	stop_cmd_pending =
		wlan_serialization_is_cmd_present_in_pending_queue(NULL, cmd);
	wlan_vdev_mlme_ser_cancel_request(cmd->vdev,
					  WLAN_SER_CMD_NONSCAN,
					  WLAN_SER_CANCEL_VDEV_NON_SCAN_NB_CMD);

	if (wlan_serialization_is_cmd_present_in_active_queue(NULL, cmd)) {
		mlme_debug("Cmd already exist in the active queue");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	ret = wlan_serialization_request(cmd);

	if (stop_cmd_pending && ret == WLAN_SER_CMD_PENDING)
		return WLAN_SER_CMD_ALREADY_EXISTS;
	else
		return ret;
}

enum wlan_serialization_status
wlan_vdev_mlme_ser_vdev_restart(struct wlan_serialization_command *cmd)
{
	if (!cmd || !cmd->vdev) {
		mlme_err("Null input");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	if (!wlan_ser_is_vdev_queue_enabled(cmd->vdev))
		return WLAN_SER_CMD_QUEUE_DISABLED;
	/*
	 * Serialization command filtering logic
	 * a. If there exists START or PDEV/VDEV restart command in the pending
	 * queue then ignore this new vdev restart request.
	 * b. Else enqueue the new VDEV RESTART cmd
	 */
	cmd->cmd_type = WLAN_SER_CMD_VDEV_START_BSS;
	if (wlan_serialization_is_cmd_present_in_pending_queue(NULL, cmd)) {
		mlme_debug("Start cmd already in the pending queue");
		return WLAN_SER_CMD_ALREADY_EXISTS;
	}

	cmd->cmd_type = WLAN_SER_CMD_PDEV_RESTART;
	if (wlan_serialization_is_cmd_present_in_pending_queue(NULL, cmd)) {
		mlme_debug("Pdev restart already in the pending queue");
		return WLAN_SER_CMD_ALREADY_EXISTS;
	}

	cmd->cmd_type = WLAN_SER_CMD_VDEV_RESTART;
	if (wlan_serialization_is_cmd_present_in_pending_queue(NULL, cmd)) {
		mlme_debug("Vdev restart already in the pending queue");
		return WLAN_SER_CMD_ALREADY_EXISTS;
	}

	return wlan_serialization_request(cmd);
}

void wlan_mlme_restart_pdev_iter_cb(struct wlan_objmgr_pdev *pdev,
				    void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *pdev_restart_pending = (uint8_t *)arg;
	struct wlan_serialization_command cmd = {0};
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	cmd.vdev = vdev;
	cmd.cmd_id = vdev_id;
	cmd.cmd_type = WLAN_SER_CMD_PDEV_RESTART;
	/*
	 * Serialization command filtering logic
	 * a. Cancel any existing VDEV restart cmd in the pending queue
	 * b. If Pdev restart already exist in pending queue then return else
	 * enqueue the new PDEV RESTART cmd
	 */
	wlan_vdev_mlme_ser_cancel_request(
			vdev,
			WLAN_SER_CMD_VDEV_RESTART,
			WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD_TYPE);

	if (wlan_serialization_is_cmd_present_in_pending_queue(NULL, &cmd)) {
		mlme_debug("Cmd already exist in the pending queue vdev:%u",
			   vdev_id);
		*pdev_restart_pending = 1;
	}
}

enum wlan_serialization_status
wlan_vdev_mlme_ser_pdev_restart(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_pdev *pdev;
	uint8_t pdev_restart_in_pending = 0;

	if (!cmd || !cmd->vdev) {
		mlme_err("Null input");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	if (!wlan_ser_is_vdev_queue_enabled(cmd->vdev))
		return WLAN_SER_CMD_QUEUE_DISABLED;

	pdev = wlan_vdev_get_pdev(cmd->vdev);
	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_mlme_restart_pdev_iter_cb,
					  &pdev_restart_in_pending, 0,
					  WLAN_MLME_SER_IF_ID);

	if (pdev_restart_in_pending)
		return WLAN_SER_CMD_ALREADY_EXISTS;

	return wlan_serialization_request(cmd);
}

static void
wlan_mlme_cancel_pending_csa_restart(struct wlan_objmgr_pdev *pdev,
				     void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	bool *csa_restart_pending = arg;
	struct wlan_serialization_command cmd = {0};
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	cmd.vdev = vdev;
	cmd.cmd_id = vdev_id;
	cmd.cmd_type = WLAN_SER_CMD_PDEV_CSA_RESTART;
	if (wlan_serialization_is_cmd_present_in_pending_queue(NULL, &cmd)) {
		mlme_debug("Cmd already exist in the pending queue vdev:%u",
			   vdev_id);
		*csa_restart_pending = true;
	}

	wlan_vdev_mlme_ser_cancel_request(
			vdev,
			WLAN_SER_CMD_PDEV_CSA_RESTART,
			WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD_TYPE);
}

static void
wlan_mlme_check_pdev_restart(struct wlan_objmgr_pdev *pdev,
			     void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	bool *pdev_restart_pending  = arg;
	struct wlan_serialization_command cmd = {0};
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	cmd.vdev = vdev;
	cmd.cmd_id = vdev_id;
	cmd.cmd_type = WLAN_SER_CMD_PDEV_RESTART;
	if (wlan_serialization_is_cmd_present_in_active_queue(NULL, &cmd)) {
		mlme_debug("Pdev restart already in the active queue vdev:%u",
			   vdev_id);
		*pdev_restart_pending = true;
	}
}

enum wlan_serialization_status
wlan_vdev_mlme_ser_pdev_csa_restart(struct wlan_serialization_command *cmd)
{
	struct wlan_objmgr_pdev *pdev;
	bool csa_restart_pending = false;
	bool pdev_restart_pending = false;
	enum wlan_serialization_status ret;

	if (!cmd || !cmd->vdev) {
		mlme_err("Null input");
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;
	}

	if (!wlan_ser_is_vdev_queue_enabled(cmd->vdev))
		return WLAN_SER_CMD_QUEUE_DISABLED;

	/*
	 * Serialization command filtering logic
	 * a. Cancel any existing PDEV CSA restart cmd in the pending queue
	 * b. If there exists PDEV RESTART command in the active queue
	 *    then deny this request
	 * c. If PDEV CSA RESTART cmd already existed in pending queue
	 *    then enqueue and return already exists
	 * d. Else enqueue this PDEV CSA RESTART cmd
	 */
	pdev = wlan_vdev_get_pdev(cmd->vdev);
	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_mlme_cancel_pending_csa_restart,
					  &csa_restart_pending, 0,
					  WLAN_MLME_SER_IF_ID);

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_mlme_check_pdev_restart,
					  &pdev_restart_pending, 0,
					  WLAN_MLME_SER_IF_ID);

	if (pdev_restart_pending)
		return WLAN_SER_CMD_DENIED_UNSPECIFIED;

	ret = wlan_serialization_request(cmd);

	if (csa_restart_pending && ret == WLAN_SER_CMD_PENDING)
		return WLAN_SER_CMD_ALREADY_EXISTS;

	return ret;
}

void
wlan_vdev_mlme_ser_remove_request(struct wlan_objmgr_vdev *vdev,
				  uint32_t cmd_id,
				  enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_serialization_queued_cmd_info cmd = {0};

	mlme_debug("Vdev:%d remove cmd:%d", wlan_vdev_get_id(vdev), cmd_type);

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.cmd_type = cmd_type;
	cmd.requestor = WLAN_UMAC_COMP_MLME;
	cmd.req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
	cmd.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;

	/* Inform serialization for command completion */
	wlan_serialization_remove_cmd(&cmd);
}

void
wlan_vdev_mlme_ser_cancel_request(struct wlan_objmgr_vdev *vdev,
				  enum wlan_serialization_cmd_type cmd_type,
				  enum wlan_serialization_cancel_type req_type)
{
	struct wlan_serialization_queued_cmd_info cmd = {0};

	cmd.vdev = vdev;
	cmd.cmd_type = cmd_type;
	cmd.req_type = req_type;
	cmd.requestor = WLAN_UMAC_COMP_MLME;
	cmd.queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;

	wlan_serialization_cancel_request(&cmd);
}

void
mlme_ser_inc_act_cmd_timeout(struct wlan_serialization_command *cmd)
{
	mlme_debug("Increase timeout of cmd type:%d", cmd->cmd_type);
	wlan_serialization_update_timer(cmd);
}

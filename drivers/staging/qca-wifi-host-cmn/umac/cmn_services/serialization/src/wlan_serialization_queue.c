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
 * DOC: wlan_serialization_queue.c
 * This file defines the functions which deals with the
 * serialization queue objects.
 */
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>
#include <qdf_status.h>
#include "wlan_serialization_api.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_queue_i.h"

struct wlan_serialization_pdev_queue *wlan_serialization_get_pdev_queue_obj(
		struct wlan_ser_pdev_obj *pdev_obj,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_serialization_pdev_queue *pdev_queue = NULL;

	if (cmd_type < WLAN_SER_CMD_NONSCAN)
		pdev_queue = &pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];
	else
		pdev_queue = &pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];

	return pdev_queue;
}

struct wlan_serialization_vdev_queue *wlan_serialization_get_vdev_queue_obj(
		struct wlan_ser_vdev_obj *vdev_obj,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_serialization_vdev_queue *vdev_queue = NULL;

	vdev_queue = &vdev_obj->vdev_q[SER_VDEV_QUEUE_COMP_NON_SCAN];

	return vdev_queue;
}

qdf_list_t *wlan_serialization_get_list_from_pdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		enum wlan_serialization_cmd_type cmd_type,
		uint8_t is_active_cmd)
{
	struct wlan_serialization_pdev_queue *pdev_queue;
	qdf_list_t *queue = NULL;

	pdev_queue = wlan_serialization_get_pdev_queue_obj(pdev_obj, cmd_type);
	if (is_active_cmd)
		queue = &pdev_queue->active_list;
	else
		queue = &pdev_queue->pending_list;

	return queue;
}

qdf_list_t *wlan_serialization_get_list_from_vdev_queue(
		struct wlan_ser_vdev_obj *vdev_obj,
		enum wlan_serialization_cmd_type cmd_type,
		uint8_t is_active_cmd)
{
	struct wlan_serialization_vdev_queue *vdev_queue;
	qdf_list_t *queue = NULL;

	vdev_queue = wlan_serialization_get_vdev_queue_obj(vdev_obj, cmd_type);
	if (is_active_cmd)
		queue = &vdev_queue->active_list;
	else
		queue = &vdev_queue->pending_list;

	return queue;
}

enum wlan_serialization_status
wlan_serialization_add_cmd_to_pdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list *cmd_list,
		uint8_t for_active_queue)
{
	qdf_list_t *queue;
	enum wlan_serialization_status status = WLAN_SER_CMD_DENIED_UNSPECIFIED;

	if (!pdev_obj) {
		ser_err("invalid serialization pdev");
		status = WLAN_SER_CMD_DENIED_UNSPECIFIED;
		goto error;
	}

	queue = wlan_serialization_get_list_from_pdev_queue(
			pdev_obj, cmd_list->cmd.cmd_type, for_active_queue);

	status = wlan_serialization_add_cmd_to_queue(queue, cmd_list,
						     pdev_obj,
						     for_active_queue,
						     WLAN_SER_PDEV_NODE);

error:
	return status;
}

enum wlan_serialization_status
wlan_serialization_add_cmd_to_vdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list *cmd_list,
		uint8_t for_active_queue)
{
	qdf_list_t *queue;
	enum wlan_serialization_status status;
	struct wlan_serialization_command *cmd;
	struct wlan_ser_vdev_obj *vdev_obj;
	struct wlan_serialization_vdev_queue *vdev_queue_obj;

	cmd = &cmd_list->cmd;

	vdev_obj = wlan_serialization_get_vdev_obj(
			wlan_serialization_get_vdev_from_cmd(cmd));

	vdev_queue_obj =
			wlan_serialization_get_vdev_queue_obj(
				vdev_obj,
				cmd->cmd_type);

	queue = wlan_serialization_get_list_from_vdev_queue(vdev_obj,
							    cmd->cmd_type,
							    for_active_queue);

	status = wlan_serialization_add_cmd_to_queue(queue, cmd_list,
						     pdev_obj,
						     for_active_queue,
						     WLAN_SER_VDEV_NODE);

	if (cmd->queue_disable)
		vdev_queue_obj->queue_disable = true;

	return status;
}

QDF_STATUS
wlan_serialization_remove_cmd_from_pdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_serialization_command *cmd,
		uint8_t is_active_queue)
{
	qdf_list_t *queue;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!pdev_obj) {
		ser_err("Invalid pdev");
		return status;
	}

	queue = wlan_serialization_get_list_from_pdev_queue(
			pdev_obj, cmd->cmd_type, is_active_queue);

	status = wlan_serialization_remove_cmd_from_queue(queue, cmd,
							  pcmd_list,
							  pdev_obj,
							  WLAN_SER_PDEV_NODE);

	return status;
}

QDF_STATUS
wlan_serialization_remove_cmd_from_vdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_serialization_command *cmd,
		uint8_t is_active_queue)
{
	qdf_list_t *queue;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_ser_vdev_obj *vdev_obj;

	vdev_obj = wlan_serialization_get_vdev_obj(
			wlan_serialization_get_vdev_from_cmd(cmd));

	queue = wlan_serialization_get_list_from_vdev_queue(vdev_obj,
							    cmd->cmd_type,
							    is_active_queue);

	status = wlan_serialization_remove_cmd_from_queue(queue, cmd,
							  pcmd_list,
							  pdev_obj,
							  WLAN_SER_VDEV_NODE);

	return status;
}


/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_debug.c
 * This file defines the debug functions for serialization component.
 */

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_utility.h>
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_queue_i.h"
#include "wlan_serialization_debug_i.h"

#ifdef WLAN_SER_DEBUG
const char *ser_reason_string[SER_QUEUE_ACTION_MAX] = {
	"REQUEST",
	"REMOVE",
	"CANCEL",
	"TIMEOUT",
	"ACTIVATION_FAILED",
	"PENDING_TO_ACTIVE",
};

static void wlan_ser_print_queues(
		qdf_list_t *queue,
		enum wlan_serialization_node node_type,
		bool is_active_queue)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	uint32_t queuelen;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	qdf_list_node_t *nnode = NULL;
	bool is_pdev_queue = false;

	if (node_type == WLAN_SER_PDEV_NODE)
		is_pdev_queue = true;

	ser_err_no_fl(WLAN_SER_LINE);
	ser_err_no_fl("%s %s Queue", (is_pdev_queue) ? "PDEV" : "VDEV",
		      (is_active_queue ? "Active" : "Pending"));

	ser_err_no_fl(WLAN_SER_LINE);
	ser_err_no_fl("|CMD_TYPE|CMD_ID|VDEV_ID|BLOCKING|PRIORITY|");
	ser_err_no_fl(WLAN_SER_LINE);

	queuelen = wlan_serialization_list_size(queue);
	while (queuelen--) {
		status = wlan_serialization_get_cmd_from_queue(queue, &nnode);
		if (status != QDF_STATUS_SUCCESS)
			break;

	if (node_type == WLAN_SER_PDEV_NODE)
		cmd_list = qdf_container_of(
				nnode,
				struct wlan_serialization_command_list,
				pdev_node);
	else
		cmd_list = qdf_container_of(
				nnode,
				struct wlan_serialization_command_list,
				vdev_node);

	ser_err_no_fl("|%8u|%6u|%6u|%8u|%8u|",
		      cmd_list->cmd.cmd_type,
		cmd_list->cmd.cmd_id,
		wlan_vdev_get_id(cmd_list->cmd.vdev),
		cmd_list->cmd.is_blocking,
		cmd_list->cmd.is_high_priority);
	}
}

static void wlan_ser_print_pdev_queue(
		struct wlan_serialization_pdev_queue *ser_pdev_q_obj,
		enum wlan_serialization_node node_type)
{
	/*Dump the active queue*/
	wlan_ser_print_queues(&ser_pdev_q_obj->active_list,
			      node_type, true);

	/*Dump the pending queue*/
	wlan_ser_print_queues(&ser_pdev_q_obj->pending_list,
			      node_type, false);
}

static void wlan_ser_print_vdev_queue(
		struct wlan_serialization_vdev_queue *ser_vdev_q_obj,
		enum wlan_serialization_node node_type)
{
	/*Dump the active queue*/
	wlan_ser_print_queues(&ser_vdev_q_obj->active_list,
			      node_type, true);

	/*Dump the pending queue*/
	wlan_ser_print_queues(&ser_vdev_q_obj->pending_list,
			      node_type, false);
}

static void wlan_ser_print_all_history(
		struct wlan_serialization_pdev_queue *pdev_queue,
		bool for_vdev_queue,
		uint32_t vdev_id)
{
	uint8_t idx;
	uint8_t data_idx;
	struct ser_history *history_info;
	struct ser_data *data;

	history_info = &pdev_queue->history;

	ser_err_no_fl(WLAN_SER_LINE WLAN_SER_LINE);
	ser_err_no_fl("Queue Commands History");
	ser_err_no_fl(WLAN_SER_LINE WLAN_SER_LINE);
	ser_err_no_fl(WLAN_SER_HISTORY_HEADER);
	ser_err_no_fl(WLAN_SER_LINE WLAN_SER_LINE);

	for (idx = 0; idx < SER_MAX_HISTORY_CMDS; idx++) {
		data_idx = (history_info->index + idx) % SER_MAX_HISTORY_CMDS;

		data = &history_info->data[data_idx];

		if (data->ser_reason >= SER_QUEUE_ACTION_MAX) {
			ser_debug("Invalid Serialization Reason");
			continue;
		}

		if (!data->data_updated)
			continue;

		if (for_vdev_queue) {
			if (vdev_id != data->vdev_id)
				continue;
		}
		ser_err_no_fl(
			"%8d|%6d|%7d|%8d|%8d|%6s|%7s|%17s|",
			data->cmd_type,
			data->cmd_id,
			data->vdev_id,
			data->is_blocking,
			data->is_high_priority,
			data->add_remove ? "ADD" : "REMOVE",
			data->active_pending ? "ACTIVE" : "PENDING",
			ser_reason_string[data->ser_reason]);
	}
}

QDF_STATUS wlan_ser_print_history(
		struct wlan_objmgr_vdev *vdev, uint8_t val,
		uint32_t sub_val)
{
	struct wlan_ser_pdev_obj *ser_pdev;
	struct wlan_ser_vdev_obj *ser_vdev;
	struct wlan_serialization_pdev_queue *pdev_q;
	struct wlan_serialization_vdev_queue *vdev_q;
	bool for_vdev_queue = false;
	uint32_t vdev_id = WLAN_INVALID_VDEV_ID;

	ser_pdev = wlan_serialization_get_pdev_obj(
			wlan_vdev_get_pdev(vdev));

	ser_vdev = wlan_serialization_get_vdev_obj(vdev);

	switch (val) {
	/*
	 * Print scan pdev queues
	 */
	case SER_PDEV_QUEUE_COMP_SCAN:
		ser_err_no_fl("Serialization SCAN Queues(LIVE)");
		pdev_q = &ser_pdev->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];
		wlan_ser_print_pdev_queue(pdev_q, WLAN_SER_PDEV_NODE);
		break;
	/*
	 * Print non scan queues
	 */
	case SER_PDEV_QUEUE_COMP_NON_SCAN:
		pdev_q = &ser_pdev->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];
		ser_err_no_fl("Serialization NON SCAN Queues(LIVE)");
		switch (sub_val) {
		/*
		 * Print non scan pdev queues
		 */
		case SER_PDEV_QUEUE_TYPE:
			wlan_ser_print_pdev_queue(pdev_q, WLAN_SER_PDEV_NODE);
			break;
		/*
		 * Print non scan pdev queues
		 */
		case SER_VDEV_QUEUE_TYPE:
			vdev_q =
			    &ser_vdev->vdev_q[SER_VDEV_QUEUE_COMP_NON_SCAN];
			for_vdev_queue = true;
			vdev_id = wlan_vdev_get_id(vdev);
			wlan_ser_print_vdev_queue(vdev_q, WLAN_SER_VDEV_NODE);
			break;
		default:
			ser_err("Invalid parameter for queue type(pdev/vdev)");
		}
		break;
	default:
		ser_err("Invalid pramater for queue type(scan/non_scan");
		goto error;
	}

	wlan_ser_print_all_history(pdev_q, for_vdev_queue, vdev_id);
error:
	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_ser_print_history);

void wlan_ser_update_cmd_history(
		struct wlan_serialization_pdev_queue *pdev_queue,
		struct wlan_serialization_command *cmd,
		enum ser_queue_reason ser_reason,
		bool add_remove,
		bool active_queue)
{
	struct ser_data *ser_data_info;
	struct ser_history *ser_history_info;

	ser_history_info = &pdev_queue->history;
	ser_history_info->index %= SER_MAX_HISTORY_CMDS;

	ser_data_info = &ser_history_info->data[ser_history_info->index];

	ser_data_info->cmd_type = cmd->cmd_type;
	ser_data_info->cmd_id = cmd->cmd_id;
	ser_data_info->is_blocking = cmd->is_blocking;
	ser_data_info->is_high_priority = cmd->is_high_priority;
	ser_data_info->add_remove = add_remove;
	ser_data_info->active_pending = active_queue;
	ser_data_info->ser_reason = ser_reason;
	ser_data_info->vdev_id = wlan_vdev_get_id(cmd->vdev);
	ser_data_info->data_updated = true;

	ser_history_info->index++;
}
#endif

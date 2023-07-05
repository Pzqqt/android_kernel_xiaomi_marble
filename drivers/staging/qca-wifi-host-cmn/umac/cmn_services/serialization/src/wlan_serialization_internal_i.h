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
 * DOC: wlan_serialization_internal_i.h
 * This file defines the prototypes of functions which are called
 * from serialization public API's and are internal
 * to serialization.
 */
#ifndef __WLAN_SERIALIZATION_PVT_I_H
#define __WLAN_SERIALIZATION_PVT_I_H

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>
#include <qdf_status.h>
#include "wlan_serialization_api.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_non_scan_i.h"

/**
 * wlan_serialization_is_cmd_present_queue() - Check if same command
 *				is already present active or pending queue
 * @cmd: pointer to command which we need to find
 * @is_active_queue: flag to find the command in active or pending queue
 *
 * This API will check the given command is already present in active or
 * pending queue based on flag
 * If present then return true otherwise false
 *
 * Return: true or false
 */
bool
wlan_serialization_is_cmd_present_queue(
					struct wlan_serialization_command *cmd,
					uint8_t is_active_queue);

/**
 * wlan_serialization_is_active_cmd_allowed() - Check if the given command
 *			can be moved to active queue
 * @cmd: Serialization command information
 *
 * Return: true or false
 */
bool
wlan_serialization_is_active_cmd_allowed(
		struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_enqueue_cmd() - Enqueue the cmd to pending/active Queue
 * @cmd: Command information
 * @ser_reason: action for dequeue
 *
 * Return: Status of the serialization request
 */
enum wlan_serialization_status
wlan_serialization_enqueue_cmd(struct wlan_serialization_command *cmd,
			       enum ser_queue_reason ser_reason);

/**
 * wlan_serialization_activate_cmd() - activate cmd in active queue
 * @cmd_list: Command needs to be activated
 * @ser_pdev_obj: Serialization private pdev object
 * @ser_reason: reason the activation cb would be called
 *
 * Return: Status of activation of the command
 */
QDF_STATUS
wlan_serialization_activate_cmd(
		struct wlan_serialization_command_list *cmd_list,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		enum ser_queue_reason ser_reason);

/**
 * wlan_serialization_move_pending_to_active() - Move a cmd from pending
 *			queue to active queue
 * @cmd_type: Type of command to be moved i.e scan or non scan
 * @pcmd_list: Pointer to command list containing the command
 * @ser_pdev_obj: Serialization private pdev object
 * @vdev: Pointer to vdev object manager
 * @blocking_cmd_removed: If a blocking cmd is removed from active queue
 * @blocking_cmd_waiting: If a blocking cmd is waiting in pending queue
 *
 * Return: Status of command request
 */
enum wlan_serialization_status
wlan_serialization_move_pending_to_active(
		enum wlan_serialization_cmd_type cmd_type,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_vdev *vdev,
		bool blocking_cmd_removed);

/**
 * wlan_serialization_dequeue_cmd() - dequeue the cmd to pending/active Queue
 * @cmd: Command information
 * @ser_reason: action for dequeue
 * @active_cmd: whether command is for active queue
 *
 * Return: Status of the serialization request
 */
enum wlan_serialization_cmd_status
wlan_serialization_dequeue_cmd(struct wlan_serialization_command *cmd,
			       enum ser_queue_reason ser_reason,
			       uint8_t active_cmd);

/**
 * wlan_serialization_find_and_start_timer() - to find and start the timer
 * @psoc: pointer to psoc
 * @cmd: pointer to actual command
 * @ser_reason: serialization reason
 *
 * find the free timer, initialize it, and start it
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_find_and_start_timer(struct wlan_objmgr_psoc *psoc,
					struct wlan_serialization_command *cmd,
					enum ser_queue_reason ser_reason);

/**
 * wlan_serialization_find_and_update_timer() - to find and update the timer
 * @psoc: pointer to psoc
 * @cmd: pointer to command attributes
 *
 * Find the timer associated with command, and update it
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_find_and_update_timer(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_find_and_stop_timer() - to find and stop the timer
 * @psoc: pointer to psoc
 * @cmd: pointer to actual command
 * @ser_reason: serialization reason
 *
 * find the timer associated with command, stop it and destroy it
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_find_and_stop_timer(struct wlan_objmgr_psoc *psoc,
				       struct wlan_serialization_command *cmd,
				       enum ser_queue_reason ser_reason);


/**
 * wlan_serialization_find_and_cancel_cmd() - to find cmd from queue and cancel
 * @cmd: pointer to serialization command
 * @req_type: Command cancel request type
 * @queue_type: Bitmask for member queue type i.e active or pending or both
 *
 * This api will find command from active queue and pending queue and
 * removes the command. If it is in active queue then it will notifies the
 * requester that it is in active queue and from there it expects requester
 * to send remove command
 *
 * Return: wlan_serialization_cmd_status
 */

enum wlan_serialization_cmd_status
wlan_serialization_find_and_cancel_cmd(
		struct wlan_serialization_command *cmd,
		enum wlan_serialization_cancel_type req_type,
		uint8_t queue_type);

/**
 * wlan_serialization_cmd_cancel_handler() - helper func to cancel cmd
 * @ser_obj: private pdev ser obj
 * @cmd: pointer to command
 * @pdev: pointer to pdev
 * @vdev: pointer to vdev
 * @cmd_type: pointer to cmd_type
 * @queue_type: If active queue or pending queue
 * @cmd_attr: Attrbute to indicate a blocking or a non-blocking command
 *
 * This API will decide from which queue, command needs to be cancelled
 * and pass that queue and other parameter required to cancel the command
 * to helper function.
 *
 * Return: wlan_serialization_cmd_status
 */
enum wlan_serialization_cmd_status
wlan_serialization_cmd_cancel_handler(
				      struct wlan_ser_pdev_obj *ser_obj,
				      struct wlan_serialization_command *cmd,
				      struct wlan_objmgr_pdev *pdev,
				      struct wlan_objmgr_vdev *vdev,
				      enum wlan_serialization_cmd_type cmd_type,
				      uint8_t queue_type,
				      enum wlan_ser_cmd_attr cmd_attr);
#endif

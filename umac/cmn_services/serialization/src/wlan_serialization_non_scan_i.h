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
 * DOC: wlan_serialization_non_scan_i.h
 * This file defines the prototypes for functions which deals with
 * serialization non scan commands.
 */

#ifndef __WLAN_SERIALIZATION_NON_SCAN_I_H
#define __WLAN_SERIALIZATION_NON_SCAN_I_H

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_mc_timer.h>
#include <wlan_utility.h>
#include "wlan_serialization_utils_i.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_api.h"
#include "wlan_serialization_internal_i.h"
#include "wlan_serialization_queue_i.h"

/**
 * wlan_serialization_is_non_scan_pending_queue_empty()
 *
 * @cmd: Serialization command information
 *
 * This API will be find out if non scan cmd pending queue is empty.
 *
 * Return: true or false
 */
bool
wlan_serialization_is_non_scan_pending_queue_empty(
		struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_is_active_nonscan_cmd_allowed() - find if cmd allowed
 *			to be enqueued in active queue
 * @cmd: Serialization command information
 *
 * This API will be called to find out if non scan cmd is allowed.
 *
 * Return: true or false
 */
bool
wlan_serialization_is_active_non_scan_cmd_allowed(
		struct wlan_serialization_command *cmd);

/**
 * wlan_ser_add_non_scan_cmd() - Add a non-scan cmd to serialization queue
 * @ser_pdev_obj: Serialization private pdev object
 * @cmd_list: Command list with command info that is to be queued
 * @is_cmd_for_active_queue: If the cmd to be enqueued in active queue or
 *			pending queue
 *
 * Return: Status of the cmd's serialization request
 */
enum wlan_serialization_status
wlan_ser_add_non_scan_cmd(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_serialization_command_list *cmd_list,
		uint8_t is_cmd_for_active_queue);
/**
 * wlan_ser_move_non_scan_pending_to_active() - Move a non-scan cmd from pending
 *			queue to active queue
 * @pcmd_list: Pointer to command list containing the command
 * @ser_pdev_obj: Serialization private pdev object
 * @vdev: Pointer to object manager vdev
 *
 * Return: Status of the cmd's serialization request
 */
enum wlan_serialization_status
wlan_ser_move_non_scan_pending_to_active(
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		struct wlan_objmgr_vdev *vdev,
		bool blocking_cmd_removed);

/**
 * wlan_ser_remove_non_scan_cmd() - Remove a non-scan cmd from the given queue
 * @ser_pdev_obj: Serialization private pdev object
 * @pcmd_list: Pointer to command list containing the command
 * @cmd: Serialization command information
 * @is_active_cmd: If the cmd has to be removed from active queue or pending
 *			queue
 *
 * Return: QDF_STATUS_SUCCESS on successfully removing the cmd else
 *			QDF_STATUS_E_FAILURE
 */
QDF_STATUS
wlan_ser_remove_non_scan_cmd(struct wlan_ser_pdev_obj *ser_pdev_obj,
			     struct wlan_serialization_command_list **pcmd_list,
			     struct wlan_serialization_command *cmd,
			     uint8_t is_active_cmd);
/**
 * wlan_ser_cancel_non_scan_cmd() - Cancel a non-scan cmd from the given queue
 * @ser_obj: Serialization private pdev object
 * @pdev: Pointer to object manager pdev
 * @vdev: Pointer to object manager vdev
 * @cmd: Serialization command information
 * @cmd_type: Serialization command type to be cancelled
 * @is_active_queue: If the cmd has to be removed from active queue or pending
 *			queue
 * @cmd_attr: Indicate the attribute of the cmd to be cancelled
 *      i.e blocking/non-blocking
 *
 * Return: Status specifying the cancel of a command from the given queue
 */
enum wlan_serialization_cmd_status
wlan_ser_cancel_non_scan_cmd(struct wlan_ser_pdev_obj *ser_obj,
			     struct wlan_objmgr_pdev *pdev,
			     struct wlan_objmgr_vdev *vdev,
			     struct wlan_serialization_command *cmd,
			     enum wlan_serialization_cmd_type cmd_type,
			     uint8_t is_active_queue,
			     enum wlan_ser_cmd_attr cmd_attr);
#endif

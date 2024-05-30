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
 * DOC: wlan_serialization_queue_i.h
 * This file defines the prototpye for functions which deals with the
 * serialization queue objects.
 */
#ifndef __WLAN_SERIALIZATION_QUEUE_I_H
#define __WLAN_SERIALIZATION_QUEUE_I_H

#include <wlan_serialization_api.h>
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_list.h>
#include <qdf_status.h>

/**
 * wlan_serialization_get_pdev_queue_obj() - Get serialization pdev queue for
 *			the given cmd_type
 * @pdev_obj: Serialization private pdev object
 * @cmd_type: Serialization command type i.e scan or non-scan
 *
 * Return: Pointer to serialization pdev queue
 */
struct wlan_serialization_pdev_queue *wlan_serialization_get_pdev_queue_obj(
		struct wlan_ser_pdev_obj *pdev_obj,
		enum wlan_serialization_cmd_type cmd_type);

/**
 * wlan_serialization_get_vdev_queue_obj() - Get serialization vdev queue for
 *			the given cmd_type
 * @vdev_obj: Serialization private vdev object
 * @cmd_type: Serialization command type i.e scan or non-scan
 *
 * Return: Pointer to serialization vdev queue
 */
struct wlan_serialization_vdev_queue *wlan_serialization_get_vdev_queue_obj(
		struct wlan_ser_vdev_obj *vdev_obj,
		enum wlan_serialization_cmd_type cmd_type);

/**
 * wlan_serialization_get_list_from_pdev_queue() - Get list member from the pdev
 *			queue for the given cmd type
 * @pdev_obj: Serialization private pdev object
 * @cmd_type: Serialization command type i.e scan or non-scan
 * @is_active_cmd: Get list from active queue or pending queue
 *
 * Return: Pointer to the obtained list member
 */
qdf_list_t *wlan_serialization_get_list_from_pdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		enum wlan_serialization_cmd_type cmd_type,
		uint8_t is_active_cmd);

/**
 * wlan_serialization_get_list_from_vdev_queue() - Get list member from the vdev
 *			queue for the given cmd type
 * @vdev_obj: Serialization private vdev object
 * @cmd_type: Serialization command type i.e scan or non-scan
 * @is_active_cmd: Get list from active queue or pending queue
 *
 * Return: Pointer to the obtained list member
 */
qdf_list_t *wlan_serialization_get_list_from_vdev_queue(
		struct wlan_ser_vdev_obj *vdev_obj,
		enum wlan_serialization_cmd_type cmd_type,
		uint8_t is_active_cmd);

/**
 * wlan_serialization_add_cmd_to_pdev_queue() - Add given cmd to the pdev
 *			queue for the given cmd type
 * @pdev_obj: Serialization private pdev object
 * @cmd_list: Pointer to command list containing the command
 * @is_cmd_for_active_queue: Add to active queue or pending queue
 *
 * Return: Status of the cmd's serialization request
 */
enum wlan_serialization_status wlan_serialization_add_cmd_to_pdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list *cmd_list,
		uint8_t is_cmd_for_active_queue);

/**
 * wlan_serialization_add_cmd_to_vdev_queue() - Add given cmd to the vdev
 *			queue for the given cmd type
 * @pdev_obj: Serialization private pdev object
 * @cmd_list: Pointer to command list containing the command
 * @is_cmd_for_active_queue: Add to active queue or pending queue
 *
 * Return: Status of the cmd's serialization request
 */
enum wlan_serialization_status wlan_serialization_add_cmd_to_vdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list *cmd_list,
		uint8_t is_cmd_for_active_queue);

/**
 * wlan_serialization_remove_cmd_from_pdev_queue() - Remove given cmd from
 *			the pdev queue for the given cmd type
 * @pdev_obj: Serialization private pdev object
 * @pcmd_list: Pointer to command list containing the command
 * @cmd: Serialization command information
 * @is_active_queue: Remove from active queue or pending queue
 *
 * Return: QDF_STATUS_SUCCESS on success, error code on failure
 */
QDF_STATUS wlan_serialization_remove_cmd_from_pdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_serialization_command *cmd,
		uint8_t is_active_queue);

/**
 * wlan_serialization_remove_cmd_from_vdev_queue() - Remove given cmd from
 *			the vdev queue for the given cmd type
 * @pdev_obj: Serialization private pdev object
 * @pcmd_list: Pointer to command list containing the command
 * @cmd: Serialization command information
 * @is_active_queue: Remove from active queue or pending queue
 *
 * Return: QDF_STATUS_SUCCESS on success, error code on failure
 */
QDF_STATUS wlan_serialization_remove_cmd_from_vdev_queue(
		struct wlan_ser_pdev_obj *pdev_obj,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_serialization_command *cmd,
		uint8_t is_active_queue);

#endif

/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * @file wlan_vdev_mlme_ser.h
 * This file implements the APIs to support interface between vdev_mlme and
 * serialization module
 */

#ifndef _WLAN_VDEV_MLME_SER_IF_H_
#define _WLAN_VDEV_MLME_SER_IF_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_serialization_api.h>
#include <wlan_objmgr_vdev_obj.h>

/**
 * wlan_vdev_mlme_ser_start_bss() - Add start_bss cmd to serialization
 * @cmd: Serialization command
 *
 * Return: Status of enqueue in the serialization module
 */
enum wlan_serialization_status
wlan_vdev_mlme_ser_start_bss(struct wlan_serialization_command *cmd);

/**
 * wlan_vdev_mlme_ser_stop_bss() - Add stop_bss cmd to serialization
 * @cmd: Serialization command
 *
 * Return: Status of enqueue in the serialization module
 */
enum wlan_serialization_status
wlan_vdev_mlme_ser_stop_bss(struct wlan_serialization_command *cmd);

/**
 * wlan_vdev_mlme_ser_restart_bss() - Add restart bss cmd to serialization
 * @cmd: Serialization command
 *
 * Return: Status of enqueue in the serialization module
 */
enum wlan_serialization_status
wlan_vdev_mlme_ser_restart_bss(struct wlan_serialization_command *cmd);

/**
 * wlan_vdev_mlme_ser_connect() - Add connect cmd to serialization
 * @cmd: Serialization command
 *
 * Return: Status of enqueue in the serialization module
 */
enum wlan_serialization_status
wlan_vdev_mlme_ser_connect(struct wlan_serialization_command *cmd);

/**
 * wlan_vdev_mlme_ser_disconnect() - Add disconnect cmd to serialization
 * @cmd: Serialization command
 *
 * Return: Status of enqueue in the serialization module
 */
enum wlan_serialization_status
wlan_vdev_mlme_ser_disconnect(struct wlan_serialization_command *cmd);

/**
 * wlan_vdev_mlme_ser_remove_request() - Remove a request from to
 * serialization
 * @vdev: Object manager vdev object
 * @cmd_id: Serialization command id
 * @cmd_type: Serialization command type
 *
 * Return: void
 */
void
wlan_vdev_mlme_ser_remove_request(struct wlan_objmgr_vdev *vdev,
				  uint32_t cmd_id,
				  enum wlan_serialization_cmd_type cmd_type);

/**
 * wlan_vdev_mlme_ser_cancel_request() - Cancel a request from to
 * serialization
 * @vdev: Object manager vdev object
 * @cmd_type: Serialization command type
 * @req_type: Type of command cancellation. i.e single/vdev/pdev
 *
 * Return: void
 */
void
wlan_vdev_mlme_ser_cancel_request(struct wlan_objmgr_vdev *vdev,
				  enum wlan_serialization_cmd_type cmd_type,
				  enum wlan_serialization_cancel_type req_type);
/**
 * mlme_ser_inc_act_cmd_timeout() - Increase timeout of active cmd
 * @cmd: Serialization command
 *
 * Return: void
 */
void mlme_ser_inc_act_cmd_timeout(struct wlan_serialization_command *cmd);

#endif /* _WLAN_VDEV_MLME_SER_IF_H_ */

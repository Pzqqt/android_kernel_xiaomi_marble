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
 * DOC: wlan_serialization_legacy_api.h
 * This file provides prototypes of the routines needed for the
 * legacy mcl serialization to utilize the services provided by the
 * serialization component.
 */
#ifndef __WLAN_SERIALIZATION_LEGACY_API_H
#define __WLAN_SERIALIZATION_LEGACY_API_H

#include "wlan_serialization_api.h"

/**
 * wlan_serialization_peek_head_pending_cmd_using_psoc() - Return command from
 *				scan or non-scan pending queue based on flag
 * @psoc: pointer to psoc
 * @is_cmd_from_pending_scan_queue: flag to determine whether command needed
 *				from scan or non-scan pending queue
 *
 * This API finds the first active pdev, and loops through scan or non-scan
 * pending queue (based on is_cmd_from_pending_scan_queue flag) and fetches
 * first pending command from queue
 *
 * Return: pointer to serialization command
 */
struct wlan_serialization_command*
wlan_serialization_peek_head_pending_cmd_using_psoc(
		struct wlan_objmgr_psoc *psoc,
		uint8_t is_cmd_from_pending_scan_queue);
/**
 * wlan_serialization_peek_head_active_cmd_using_psoc() - Return command from
 *				scan or non-scan active queue based on flag
 * @psoc: pointer to psoc
 * @is_cmd_from_active_scan_queue: flag to determine whether command needed
 *				from scan or non-scan active queue
 *
 * This API finds the first active pdev, and loops through scan or non-scan
 * active queue (based on is_cmd_from_active_scan_queue flag) and fetches
 * first active command from queue
 *
 * Return: pointer to serialization command
 */
struct wlan_serialization_command*
wlan_serialization_peek_head_active_cmd_using_psoc(
		struct wlan_objmgr_psoc *psoc,
		uint8_t is_cmd_from_active_scan_queue);

/**
 * wlan_serialization_get_pending_list_next_node_using_psoc() - Return next
 *				scan or non-scan pending command from queue
 * @psoc: pointer to psoc
 * @prev_cmd: previous command given by caller, find next command after this
 * @is_cmd_for_pending_scan_queue: to find from scan or non-scan pending queue
 *
 * This API finds the first active pdev, and loops through scan or non-scan
 * pending queue (based on is_cmd_from_pending_scan_queue flag) and fetches
 * next pending command after prev_cmd
 *
 * Return: pointer to serialization command
 */
struct wlan_serialization_command*
wlan_serialization_get_pending_list_next_node_using_psoc(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *prev_cmd,
		uint8_t is_cmd_for_pending_scan_queue);
/**
 * wlan_serialization_get_active_list_next_node_using_psoc() - Return next
 *				scan or non-scan pending command from queue
 * @psoc: pointer to psoc
 * @prev_cmd: previous command given by caller, find next command after this
 * @is_cmd_for_active_scan_queue: to find from active scan or non-scan queue
 *
 * This API finds the first active pdev, and loops through scan or non-scan
 * pending queue (based on is_cmd_from_pending_scan_queue flag) and fetches
 * next pending command after prev_cmd
 *
 * Return: pointer to serialization command
 */
struct wlan_serialization_command*
wlan_serialization_get_active_list_next_node_using_psoc(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *prev_cmd,
		uint8_t is_cmd_for_active_scan_queue);
/**
 * wlan_serialization_get_active_list_count() - Return Active list count
 * @psoc: pointer to soc
 * @is_cmd_from_active_scan_queue: flag to determine whether command needed
 *				from scan or non-scan active queue
 *
 * Get the number of nodes present in active list
 *
 * Return: count number of active commands in queue
 */

uint32_t wlan_serialization_get_active_list_count(struct wlan_objmgr_psoc *psoc,
		uint8_t is_cmd_from_active_scan_queue);
/**
 * wlan_serialization_get_pending_list_count() - Return pending list count
 * @psoc: pointer to soc
 * @is_cmd_from_pending_scan_queue: flag to determine whether command needed
 *				from scan or non-scan pending queue
 *
 * Get the number of nodes present in pending list
 *
 * Return: count number of pending commands in queue
 */
uint32_t wlan_serialization_get_pending_list_count(
		struct wlan_objmgr_psoc *psoc,
		uint8_t is_cmd_from_pending_scan_queue);

/**
 * wlan_serialization_purge_cmd_list_by_vdev_id() - Purge given list
 * @psoc: pointer to soc
 * @vdev_id: vdev_id variable
 * @purge_scan_active_queue: whether to purge active scan queue
 * @purge_scan_pending_queue: whether to purge pending scan queue
 * @purge_nonscan_active_queue: whether to purge active nonscan queue
 * @purge_nonscan_pending_queue: whether to purge pending nonscan queue
 * @purge_all_queues: whether to purge all queues.
 *
 * This API will purge queue based given flags and vdev_id. If vdev
 * is invalid then it will return immediately. If correct vdev_id is given then
 * it will purge the queues per vdev.
 *
 * Example:
 * 1) If you want to purge scan active queue for particular vdev then
 *    provide correct vdev_id value and purge_scan_active_queue flag set to
 *    TRUE and rest of the flags set to false.
 * 2) If you want to purge all queues for particular vdev then provide
 *    correct vdev_id value and set purge_all_queues flag set to TRUE and rest
 *    of the flags set to false.
 * 3) If you want to purge active scan and active non-scan queues to be flushed
 *    then set purge_scan_active_queue and purge_nonscan_active_queue flags to
 *    be set TRUE and rest of the flags to be FALSE
 *
 * Return: none
 */
void wlan_serialization_purge_cmd_list_by_vdev_id(struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id,
		bool purge_scan_active_queue,
		bool purge_scan_pending_queue,
		bool purge_nonscan_active_queue,
		bool purge_nonscan_pending_queue,
		bool purge_all_queues);
/**
 * wlan_serialization_purge_cmd_list() - Purge given list
 * @psoc: pointer to soc
 * @vdev: pointer to vdev object
 * @purge_scan_active_queue: whether to purge active scan queue
 * @purge_scan_pending_queue: whether to purge pending scan queue
 * @purge_nonscan_active_queue: whether to purge active nonscan queue
 * @purge_nonscan_pending_queue: whether to purge pending nonscan queue
 * @purge_all_queues: whether to purge all queues.
 *
 * This API will purge queue based given flags and vdev object. If vdev
 * is null then it will purge the queues per pdev by default.
 * If vdev is given then it will purge the queues per vdev.
 *
 * Example:
 * 1) If you want to purge scan active queue for particular vdev then
 *    provide correct vdev object and purge_scan_active_queue flag set to
 *    TRUE and rest of the flags set to false.
 * 2) If you want to purge all queues for particular vdev then provide
 *    correct vdev object value & set purge_all_queues flag set to TRUE and rest
 *    of the flags set to false.
 * 3) If you want to purge active scan and active non-scan queues to be flushed
 *    for pdev then set purge_scan_active_queue and purge_nonscan_active_queue
 *    flags to be set TRUE and rest of the flags to be FALSE with vdev object
 *    passed as NULL.
 *
 * Return: none
 */
void wlan_serialization_purge_cmd_list(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_vdev *vdev,
		bool purge_scan_active_queue,
		bool purge_scan_pending_queue,
		bool purge_nonscan_active_queue,
		bool purge_nonscan_pending_queue,
		bool purge_all_queues);
#endif

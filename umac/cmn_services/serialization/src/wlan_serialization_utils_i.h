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
 * DOC: wlan_serialization_utils_i.h
 * This file defines the prototypes for the utility helper functions
 * for the serialization component.
 */
#ifndef __WLAN_SERIALIZATION_UTILS_I_H
#define __WLAN_SERIALIZATION_UTILS_I_H
/* Include files */
#include "qdf_status.h"
#include "qdf_list.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_rules_i.h"

/**
 * struct wlan_serialization_psoc_priv_obj - psoc obj data for serialization
 * @wlan_serialization_module_state_cb - module level callback
 * @wlan_serialization_apply_rules_cb - pointer to apply rules on the cmd
 * @timers - Timers associated with the active commands
 * @max_axtive_cmds - Maximum active commands allowed
 *
 * Serialization component takes a command as input and checks whether to
 * allow/deny the command. It will use the module level callback registered
 * by each component to fetch the information needed to apply the rules.
 * Once the information is available, the rules callback registered for each
 * command internally by serialization will be applied to determine the
 * checkpoint for the command. If allowed, command will be put into active/
 * pending list and each active command is associated with a timer.
 */
struct wlan_serialization_psoc_priv_obj {
	wlan_serialization_comp_info_cb comp_info_cb[
		WLAN_SER_CMD_MAX][WLAN_UMAC_COMP_ID_MAX];
	wlan_serialization_apply_rules_cb apply_rules_cb[WLAN_SER_CMD_MAX];
	struct wlan_serialization_timer *timers;
	uint8_t max_active_cmds;
};

/**
 * struct wlan_serialization_pdev_priv_obj - pdev obj data for serialization
 * @active_list: list to hold the non-scan commands currently being executed
 * @pending_list list: to hold the non-scan commands currently pending
 * @active_scan_list: list to hold the scan commands currently active
 * @pending_scan_list: list to hold the scan commands currently pending
 * @global_cmd_pool_list: list to hold the global buffers
 * @cmd_ptr: pointer to globally allocated cmd pool
 *
 * Serialization component maintains linked lists to store the commands
 * sent by other components to get serialized. All the lists are per
 * pdev. The maximum number of active scans is determined by the firmware.
 * There is only one non-scan active command per pdev at a time as per the
 * current software architecture. cmd_ptr holds the memory allocated for
 * each of the global cmd pool nodes and it is useful in freeing up these
 * nodes when needed.
 */
struct wlan_serialization_pdev_priv_obj {
	qdf_list_t active_list;
	qdf_list_t pending_list;
	qdf_list_t active_scan_list;
	qdf_list_t pending_scan_list;
	qdf_list_t global_cmd_pool_list;
};

/**
 * wlan_serialization_validate_cmd() - Validate the command
 * @comp_id: Component ID
 * @cmd_type: Command Type
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_serialization_validate_cmd(
		 enum wlan_umac_comp_id comp_id,
		 enum wlan_serialization_cmd_type cmd_type);


/**
 * wlan_serialization_release_list_cmds() - Release the list cmds to global pool
 * @ser_pdev_obj: Serialization private pdev object
 * @list: List for which the commands have to be returned to the global pool
 *
 * Return: None
 */
void wlan_serialization_release_list_cmds(
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		qdf_list_t *list);

/**
 * wlan_serialization_destroy_list() - Release the cmds and destroy list
 * @ser_pdev_obj: Serialization private pdev object
 * @list: List to be destroyed
 *
 * Return: None
 */
void wlan_serialization_destroy_list(
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		qdf_list_t *list);

/**
 * wlan_serialization_get_psoc_priv_obj() - Return the component private obj
 * @psoc: Pointer to the PSOC object
 *
 * Return: Serialization component's PSOC level private data object
 */
struct wlan_serialization_psoc_priv_obj *wlan_serialization_get_psoc_priv_obj(
		struct wlan_objmgr_psoc *psoc);

/**
 * wlan_serialization_get_pdev_priv_obj() - Return the component private obj
 * @psoc: Pointer to the PDEV object
 *
 * Return: Serialization component's PDEV level private data object
 */
struct wlan_serialization_pdev_priv_obj *wlan_serialization_get_pdev_priv_obj(
		struct wlan_objmgr_pdev *pdev);

#endif

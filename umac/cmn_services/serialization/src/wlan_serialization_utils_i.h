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
#include "qdf_mc_timer.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_serialization_rules_i.h"
#include "wlan_scan_ucfg_api.h"

/**
 * struct wlan_serialization_timer - Timer used for serialization
 * @cmd:      Cmd to which the timer is linked
 * @timer:    Timer associated with the command
 *
 * Timers are allocated statically during init, one each for the
 * maximum active commands permitted in the system. Once a cmd is
 * moved from pending list to active list, the timer is activated
 * and once the cmd is completed, the timer is cancelled. Timer is
 * also cancelled if the command is aborted
 *
 * The timers are maintained per psoc. A timer is associated to
 * unique combination of pdev, cmd_type and cmd_id.
 */
struct wlan_serialization_timer {
	struct wlan_serialization_command *cmd;
	qdf_mc_timer_t timer;
};

/**
 * struct wlan_serialization_command_list - List of commands to be serialized
 * @node: Node identifier in the list
 * @cmd: Command to be serialized
 */
struct wlan_serialization_command_list {
	qdf_list_node_t node;
	struct wlan_serialization_command cmd;
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
 * wlan_serialization_put_back_to_global_list() - put back cmd in global pool
 * @queue: queue from which cmd needs to be taken out
 * @ser_pdev_obj: pdev private object
 * @cmd_list: cmd which needs to be matched
 *
 * command will be taken off from the queue and will be put back to global
 * pool of free command buffers.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_put_back_to_global_list(qdf_list_t *queue,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		struct wlan_serialization_command_list *cmd_list);
/**
 * wlan_serialization_move_pending_to_active() - to move pending command to
 *						 active queue
 * @cmd_type: cmd type to device to which queue the command needs to go
 * @ser_pdev_obj: pointer to ser_pdev_obj
 *
 * Return: none
 */
void wlan_serialization_move_pending_to_active(
		enum wlan_serialization_cmd_type cmd_type,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj);
/**
 * wlan_serialization_get_pdev_from_cmd() - get pdev from provided cmd
 * @cmd: pointer to actual command
 *
 * This API will get the pointer to pdev through checking type of cmd
 *
 * Return: pointer to pdev
 */
struct wlan_objmgr_pdev*
wlan_serialization_get_pdev_from_cmd(struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_get_cmd_from_queue() - to extract command from given queue
 * @queue: pointer to queue
 * @nnode: next node to extract
 *
 * This API will try to extract node from queue which is next to prev node. If
 * no previous node is given then take out the front node of the queue.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_get_cmd_from_queue(qdf_list_t *queue,
						 qdf_list_node_t **nnode);

/**
 * wlan_serialization_is_active_cmd_allowed() - check to see if command
 *						is allowed in active queue
 * @pdev: pointer to pdev structure
 * @cmd_type: type of command to check against
 *
 * Takes the command type and based on the type, it checks scan command queue
 * or nonscan command queue to see if active command is allowed or no
 *
 * Return: true if allowed else false
 */
bool wlan_serialization_is_active_cmd_allowed(
			struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_cleanup_all_timers() - to clean-up all timers
 *
 * @psoc_ser_ob: pointer to serialization psoc private object
 *
 * This API is to cleanup all the timers. it can be used when serialization
 * module is exiting. it will make sure that if timer is running then it will
 * stop and destroys the timer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_cleanup_all_timers(
	struct wlan_serialization_psoc_priv_obj *psoc_ser_ob);

/**
 * wlan_serialization_find_and_remove_cmd() - to find cmd from queue and remove
 * @cmd_info: pointer to command related information
 *
 * This api will find command from active queue and removes the command
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_find_and_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd_info);

/**
 * wlan_serialization_find_and_cancel_cmd() - to find cmd from queue and cancel
 * @cmd_info: pointer to command related information
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
		struct wlan_serialization_queued_cmd_info *cmd_info);
/**
 * wlan_serialization_enqueue_cmd() - Enqueue the cmd to pending/active Queue
 * @cmd: Command information
 * @is_cmd_for_active_queue: whether command is for active queue
 * @cmd_list: command which needs to be inserted in active queue
 * Return: Status of the serialization request
 */
enum wlan_serialization_status
wlan_serialization_enqueue_cmd(
		struct wlan_serialization_command *cmd,
		uint8_t is_cmd_for_active_queue,
		struct wlan_serialization_command_list **pcmd_list);

/**
 * wlan_serialization_dequeue_cmd() - dequeue the cmd to pending/active Queue
 * @cmd: Command information
 * @is_cmd_for_active_queue: whether command is for active queue
 *
 * Return: Status of the serialization request
 */
enum wlan_serialization_cmd_status
wlan_serialization_dequeue_cmd(struct wlan_serialization_command *cmd,
			       uint8_t is_cmd_for_active_queue);
/**
 * wlan_serialization_find_and_stop_timer() - to find and stop the timer
 * @psoc: pointer to psoc
 * @cmd: pointer to actual command
 *
 * find the timer associated with command, stop it and destory it
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_find_and_stop_timer(struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd);
/**
 * wlan_serialization_find_and_stop_timer() - to find and start the timer
 * @psoc: pointer to psoc
 * @cmd: pointer to actual command
 *
 * find the free timer, initialize it, and start it
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_find_and_start_timer(struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd);

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
 * wlan_serialization_validate_cmdtype() - Validate the command type
 * @cmd_type: Command Type
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_serialization_validate_cmdtype(
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

/**
 * wlan_serialization_get_obj() - Return the component private obj
 * @psoc: Pointer to the SERIALIZATION object
 *
 * Return: Serialization component's level private data object
 */
struct wlan_serialization_psoc_priv_obj *
wlan_serialization_get_obj(struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_is_cmd_in_vdev_list() - Check Node present in VDEV list
 * @vdev: Pointer to the VDEV object
 * @queue: Pointer to the qdf_list_t
 *
 * Return: Boolean true or false
 */
bool
wlan_serialization_is_cmd_in_vdev_list(
		struct wlan_objmgr_vdev *vdev, qdf_list_t *queue);

/**
 * wlan_serialization_is_cmd_in_pdev_list() - Check Node present in PDEV list
 * @pdev: Pointer to the PDEV object
 * @queue: Pointer to the qdf_list_t
 *
 * Return: Boolean true or false
 */
bool
wlan_serialization_is_cmd_in_pdev_list(
		struct wlan_objmgr_pdev *pdev, qdf_list_t *queue);

/**
 * wlan_serialization_is_cmd_in_active_pending() - return cmd status
 *						active/pending queue
 * @cmd_in_active: CMD in active list
 * @cmd_in_pending: CMD in pending list
 *
 * Return: enum wlan_serialization_cmd_status
 */
enum wlan_serialization_cmd_status
wlan_serialization_is_cmd_in_active_pending(bool cmd_in_active,
		bool cmd_in_pending);

/**
 * wlan_serialization_remove_all_cmd_from_queue() - Remove cmd which matches
 * @queue: queue from where command needs to be removed
 * @ser_pdev_obj: pointer to serialization object
 * @pdev: pointer to pdev
 * @vdev: pointer to vdev
 * @cmd: pointer to cmd
 * @is_active_queue: to check if command matching is for active queue
 *
 * This API will remove one or more commands which match the given parameters
 * interms of argument. For example, if user request all commands to removed
 * which matches "vdev" then iterate through all commands, find out and remove
 * command which matches vdev object.
 *
 * Return: enum wlan_serialization_cmd_status
 */
enum wlan_serialization_cmd_status
wlan_serialization_remove_all_cmd_from_queue(qdf_list_t *queue,
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		struct wlan_objmgr_pdev *pdev, struct wlan_objmgr_vdev *vdev,
		struct wlan_serialization_command *cmd,
		uint8_t is_active_queue);
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
bool wlan_serialization_is_cmd_present_queue(
			struct wlan_serialization_command *cmd,
			uint8_t is_active_queue);

/**
 * wlan_serialization_activate_cmd() - activate cmd in active queue
 * @cmd_list: Command needs to be activated
 * @ser_pdev_obj: Serialization private pdev object
 *
 * Return: None
 */
void wlan_serialization_activate_cmd(
			struct wlan_serialization_command_list *cmd_list,
			struct wlan_serialization_pdev_priv_obj *ser_pdev_obj);
#endif

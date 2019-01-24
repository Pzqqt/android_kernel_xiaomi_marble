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
 * DOC: wlan_serialization_utils_i.h
 * This file defines the prototypes for the utility helper functions
 * for the serialization component.
 */
#ifndef __WLAN_SERIALIZATION_UTILS_I_H
#define __WLAN_SERIALIZATION_UTILS_I_H

#include <qdf_status.h>
#include <qdf_list.h>
#include <qdf_mc_timer.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_scan_ucfg_api.h>
#include "wlan_serialization_rules_i.h"
#ifdef WLAN_SER_DEBUG
#include "wlan_serialization_debug_i.h"
#endif

/*
 * Below bit positions are used to identify if a
 * serialization command is in use or marked for
 * deletion.
 * CMD_MARKED_FOR_ACTIVATION - The command is about to be activated
 * CMD_IS_ACTIVE - The command is active and currently in use
 */
#define CMD_MARKED_FOR_ACTIVATION     1
#define CMD_IS_ACTIVE                 2
#define CMD_ACTIVE_MARKED_FOR_CANCEL  3
#define CMD_ACTIVE_MARKED_FOR_REMOVAL 4
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
	qdf_timer_t timer;
};

/**
 * enum wlan_serialization_node - Types of available nodes in serialization list
 * @WLAN_SER_PDEV_NODE: pdev node from the pdev queue
 * @WLAN_SER_VDEV_NODE: vdev node from the vdev queue
 */
enum wlan_serialization_node {
	WLAN_SER_PDEV_NODE,
	WLAN_SER_VDEV_NODE,
};

/**
 * struct wlan_serialization_command_list - List of commands to be serialized
 * @pdev_node: PDEV node identifier in the list
 * @vdev_node: VDEV node identifier in the list
 * @cmd: Command to be serialized
 * @cmd_in_use: flag to check if the node/entry is logically active
 */
struct wlan_serialization_command_list {
	qdf_list_node_t pdev_node;
	qdf_list_node_t vdev_node;
	struct wlan_serialization_command cmd;
	unsigned long cmd_in_use;
};

/**
 * struct wlan_serialization_pdev_queue - queue data related to pdev
 * @active_list: list to hold the commands currently being executed
 * @pending_list: list to hold the commands currently pending
 * @cmd_pool_list: list to hold the global command pool
 * @vdev_active_cmd_bitmap: Active cmd bitmap of vdev for the given pdev
 * @blocking_cmd_active: Indicate if a blocking cmd is in active execution
 * @blocking_cmd_waiting: Indicate if a blocking cmd is in pending queue
 * @pdev_queue_lock: pdev lock to protect concurrent operations on the queues
 */
struct wlan_serialization_pdev_queue {
	qdf_list_t active_list;
	qdf_list_t pending_list;
	qdf_list_t cmd_pool_list;
	uint32_t vdev_active_cmd_bitmap;
	bool blocking_cmd_active;
	uint16_t blocking_cmd_waiting;
	qdf_spinlock_t pdev_queue_lock;
#ifdef WLAN_SER_DEBUG
	struct ser_history history;
#endif
};

/**
 * struct wlan_serialization_vdev_queue - queue data related to vdev
 * @active_list: list to hold the commands currently being executed
 * @pending_list list: to hold the commands currently pending
 * @queue_disable: is the queue disabled
 */
struct wlan_serialization_vdev_queue {
	qdf_list_t active_list;
	qdf_list_t pending_list;
	bool queue_disable;
};

/**
 * enum wlan_serialization_pdev_queue_type - Types of available pdev queues
 * @QUEUE_COMP_SCAN: Scan queue
 * @QUEUE_COMP_NON_SCAN: Non Scan queue
 */
enum serialization_pdev_queue_type {
	SER_PDEV_QUEUE_COMP_SCAN,
	SER_PDEV_QUEUE_COMP_NON_SCAN,
	SER_PDEV_QUEUE_COMP_MAX,
};

/**
 * enum wlan_serialization_vdev_queue_type - Types of available vdev queues
 * @QUEUE_COMP_NON_SCAN: Non Scan queue
 */
enum serialization_vdev_queue_type {
	SER_VDEV_QUEUE_COMP_NON_SCAN,
	SER_VDEV_QUEUE_COMP_MAX,
};

/**
 * enum wlan_serialization_match_type - Comparison options for a command
 * @WLAN_SER_MATCH_VDEV: Compare vdev
 * @WLAN_SER_MATCH_PDEV: Compare pdev
 * @WLAN_SER_MATCH_CMD_TYPE: Compare command type
 * @WLAN_SER_MATCH_CMD_TYPE_VDEV: Compare command type and vdev
 * @WLAN_SER_MATCH_CMD_ID: Compare command id
 * @WLAN_SER_MATCH_CMD_ID_VDEV: Compare command id and vdev
 */
enum wlan_serialization_match_type {
	WLAN_SER_MATCH_VDEV,
	WLAN_SER_MATCH_PDEV,
	WLAN_SER_MATCH_CMD_TYPE,
	WLAN_SER_MATCH_CMD_TYPE_VDEV,
	WLAN_SER_MATCH_CMD_ID,
	WLAN_SER_MATCH_CMD_ID_VDEV,
	WLAN_SER_MATCH_MAX,
};

/**
 * struct wlan_ser_pdev_obj - pdev obj data for serialization
 * @pdev_q: Array of pdev queues
 */
struct wlan_ser_pdev_obj {
	struct wlan_serialization_pdev_queue pdev_q[SER_PDEV_QUEUE_COMP_MAX];
};

/**
 * struct wlan_ser_vdev_priv_obj - Serialization private object of vdev
 * @vdev_q: Array of vdev queues
 */
struct wlan_ser_vdev_obj {
	struct wlan_serialization_vdev_queue vdev_q[SER_VDEV_QUEUE_COMP_MAX];
};

/**
 * struct wlan_ser_psoc_obj - psoc obj data for serialization
 * @comp_info_cb - module level callback
 * @apply_rules_cb - pointer to apply rules on the cmd
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
struct wlan_ser_psoc_obj {
	wlan_serialization_comp_info_cb comp_info_cb[
		WLAN_SER_CMD_MAX][WLAN_UMAC_COMP_ID_MAX];
	wlan_serialization_apply_rules_cb apply_rules_cb[WLAN_SER_CMD_MAX];
	struct wlan_serialization_timer *timers;
	uint8_t max_active_cmds;
	qdf_spinlock_t timer_lock;
};

/**
 * wlan_serialization_remove_cmd_from_queue() - to remove command from
 *							given queue
 * @queue: queue from which command needs to be removed
 * @cmd: command to match in the queue
 * @pcmd_list: Pointer to command list containing the command
 * @ser_pdev_obj: pointer to private pdev serialization object
 * @node_type: Pdev node or vdev node
 *
 * This API takes the queue, it matches the provided command from this queue
 * and removes it. Before removing the command, it will notify the caller
 * that if it needs to remove any memory allocated by caller.
 *
 * Return: QDF_STATUS_SUCCESS on success, error code on failure
 */
QDF_STATUS
wlan_serialization_remove_cmd_from_queue(
		qdf_list_t *queue,
		struct wlan_serialization_command *cmd,
		struct wlan_serialization_command_list **pcmd_list,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		enum wlan_serialization_node node_type);

/**
 * wlan_serialization_add_cmd_from_queue() - Add a cmd to
 *							given queue
 * @queue: queue from which command needs to be removed
 * @cmd_list: Pointer to command list containing the command
 * @ser_pdev_obj: pointer to private pdev serialization object
 * @is_cmd_for_active_queue: Add cmd to active or pending queue
 * @node_type: Pdev node or vdev node
 *
 * Return: Status of the serialization request
 */
enum wlan_serialization_status
wlan_serialization_add_cmd_to_queue(
		qdf_list_t *queue,
		struct wlan_serialization_command_list *cmd_list,
		struct wlan_ser_pdev_obj *ser_pdev_obj,
		uint8_t is_cmd_for_active_queue,
		enum wlan_serialization_node node_type);

/**
 * wlan_serialization_get_psoc_from_cmd() - get psoc from provided cmd
 * @cmd: pointer to actual command
 *
 * This API will get the pointer to psoc through checking type of cmd
 *
 * Return: pointer to psoc
 */
struct wlan_objmgr_psoc*
wlan_serialization_get_psoc_from_cmd(struct wlan_serialization_command *cmd);

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
 * wlan_serialization_get_vdev_from_cmd() - get vdev from provided cmd
 * @cmd: pointer to actual command
 *
 * This API will get the pointer to vdev through checking type of cmd
 *
 * Return: pointer to vdev
 */
struct wlan_objmgr_vdev*
wlan_serialization_get_vdev_from_cmd(struct wlan_serialization_command *cmd);

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
QDF_STATUS wlan_serialization_get_cmd_from_queue(
		qdf_list_t *queue, qdf_list_node_t **nnode);

/**
 * wlan_serialization_stop_timer() - to stop particular timer
 * @ser_timer: pointer to serialization timer
 *
 * This API stops the particular timer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_serialization_stop_timer(struct wlan_serialization_timer *ser_timer);
/**
 * wlan_serialization_cleanup_vdev_timers() - clean-up all timers for a vdev
 *
 * @vdev: pointer to vdev object
 *
 * This API is to cleanup all the timers for a vdev.
 * It can be used when serialization vdev destroy is called.
 * It will make sure that if timer is running then it will
 * stop and destroys the timer
 *
 * Return: QDF_STATUS
 */

QDF_STATUS wlan_serialization_cleanup_vdev_timers(
			struct wlan_objmgr_vdev *vdev);

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
	struct wlan_ser_psoc_obj *psoc_ser_ob);

/**
 * wlan_serialization_validate_cmd() - Validate the command
 * @comp_id: Component ID
 * @cmd_type: Command Type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_validate_cmd(
		 enum wlan_umac_comp_id comp_id,
		 enum wlan_serialization_cmd_type cmd_type);

/**
 * wlan_serialization_validate_cmd_list() - Validate the command list
 * @cmd_list: Serialization command list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_validate_cmd_list(
		struct wlan_serialization_command_list *cmd_list);

/**
 * wlan_serialization_validate_cmdtype() - Validate the command type
 * @cmd_type: Command Type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_validate_cmdtype(
		 enum wlan_serialization_cmd_type cmd_type);

/**
 * wlan_serialization_destroy_pdev_list() - Release the pdev cmds and
 * destroy list
 * @pdev_queue: Pointer to the pdev queue
 *
 * Return: None
 */
void wlan_serialization_destroy_pdev_list(
		struct wlan_serialization_pdev_queue *pdev_queue);

/**
 * wlan_serialization_destroy_vdev_list() - Release the vdev cmds and
 * destroy list
 * @list: List to be destroyed
 *
 * Return: None
 */
void wlan_serialization_destroy_vdev_list(qdf_list_t *list);

/**
 * wlan_serialization_get_psoc_obj() - Return the component private obj
 * @psoc: Pointer to the PSOC object
 *
 * Return: Serialization component's PSOC level private data object
 */
struct wlan_ser_psoc_obj *wlan_serialization_get_psoc_obj(
		struct wlan_objmgr_psoc *psoc);

/**
 * wlan_serialization_get_pdev_obj() - Return the component private obj
 * @psoc: Pointer to the PDEV object
 *
 * Return: Serialization component's PDEV level private data object
 */
struct wlan_ser_pdev_obj *wlan_serialization_get_pdev_obj(
		struct wlan_objmgr_pdev *pdev);

/**
 * wlan_serialization_get_vdev_obj() - Return the component private obj
 * @vdev: Pointer to the VDEV object
 *
 * Return: Serialization component's VDEV level private data object
 */
struct wlan_ser_vdev_obj *wlan_serialization_get_vdev_obj(
		struct wlan_objmgr_vdev *vdev);

/**
 * wlan_serialization_is_cmd_in_vdev_list() - Check Node present in VDEV list
 * @vdev: Pointer to the VDEV object
 * @queue: Pointer to the qdf_list_t
 * @node_type: Pdev node or vdev node
 *
 * Return: Boolean true or false
 */
bool
wlan_serialization_is_cmd_in_vdev_list(
		struct wlan_objmgr_vdev *vdev, qdf_list_t *queue,
		enum wlan_serialization_node node_type);

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
wlan_serialization_is_cmd_in_active_pending(
		bool cmd_in_active, bool cmd_in_pending);

/**
 * wlan_serialization_is_cmd_present_in_given_queue() - Check if the cmd is
 * present in the given queue
 * @queue: List of commands which has to be searched
 * @cmd: Serialization command information
 * @node_type: Pdev node or vdev node
 *
 * Return: Boolean true or false
 */
bool wlan_serialization_is_cmd_present_in_given_queue(
		qdf_list_t *queue,
		struct wlan_serialization_command *cmd,
		enum wlan_serialization_node node_type);

/**
 * wlan_serialization_timer_destroy() - destroys the timer
 * @ser_timer: pointer to particular timer
 *
 * This API destroys the memory allocated by timer and assigns cmd member of
 * that timer structure to NULL
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_serialization_timer_destroy(
		struct wlan_serialization_timer *ser_timer);

/**
 * wlan_serialization_list_empty() - check if the list is empty
 * @queue: Queue/List that needs to be checked for emptiness
 *
 * Return: true if list is empty and false otherwise
 */
bool wlan_serialization_list_empty(qdf_list_t *queue);

/**
 * wlan_serialization_list_size() - Find the size of the provided queue
 * @queue: Queue/List for which the size/length is to be returned
 *
 * Return: size/length of the queue/list
 */
uint32_t wlan_serialization_list_size(qdf_list_t *queue);

/**
 * wlan_serialization_match_cmd_type() - Check for a match on given nnode
 * @nnode: The node on which the matching has to be done
 * @cmd_type: Command type that needs to be matched
 * @node_type: Pdev node or vdev node
 *
 * This API will check if the cmd ID and cmd type of the given nnode are
 * matching with the one's that are being passed to this function.
 *
 * Return: True if matched,false otherwise.
 */
bool wlan_serialization_match_cmd_type(
			qdf_list_node_t *nnode,
			enum wlan_serialization_cmd_type,
			enum wlan_serialization_node node_type);

/**
 * wlan_serialization_match_cmd_id_type() - Check for a match on given nnode
 * @nnode: The node on which the matching has to be done
 * @cmd: Command that needs to be matched
 * @node_type: Pdev node or vdev node
 *
 * This API will check if the cmd ID and cmd type of the given nnode are
 * matching with the one's that are being passed to this function.
 *
 * Return: True if matched,false otherwise.
 */
bool wlan_serialization_match_cmd_id_type(
			qdf_list_node_t *nnode,
			struct wlan_serialization_command *cmd,
			enum wlan_serialization_node node_type);

/**
 * wlan_serialization_match_cmd_vdev() - Check for a match on given nnode
 * @nnode: The node on which the matching has to be done
 * @vdev: VDEV object that needs to be matched
 * @node_type: Pdev node or vdev node
 *
 * This API will check if the VDEV object of the given nnode are
 * matching with the one's that are being passed to this function.
 *
 * Return: True if matched,false otherwise.
 */
bool wlan_serialization_match_cmd_vdev(qdf_list_node_t *nnode,
				       struct wlan_objmgr_vdev *vdev,
				       enum wlan_serialization_node node_type);

/**
 * wlan_serialization_match_cmd_pdev() - Check for a match on given nnode
 * @nnode: The node on which the matching has to be done
 * @pdev: pdev object that needs to be matched
 * @node_type: Node type. Pdev node or vdev node
 *
 * This API will check if the PDEV object of the given nnode are
 * matching with the one's that are being passed to this function.
 *
 * Return: True if matched,false otherwise.
 */
bool wlan_serialization_match_cmd_pdev(qdf_list_node_t *nnode,
				       struct wlan_objmgr_pdev *pdev,
				       enum wlan_serialization_node node_type);

/**
 * wlan_serialization_find_cmd() - Find the cmd matching the given criterias
 * @cmd: Serialization command information
 * @cmd_type: Command type to be matched
 * @pdev: pdev object that needs to be matched
 * @vdev: vdev object that needs to be matched
 * @node_type: Node type. Pdev node or vdev node
 *
 * Return: Pointer to the node member in the list
 */
qdf_list_node_t *
wlan_serialization_find_cmd(qdf_list_t *queue, uint32_t match_type,
			    struct wlan_serialization_command *cmd,
			    enum wlan_serialization_cmd_type cmd_type,
			    struct wlan_objmgr_pdev *pdev,
			    struct wlan_objmgr_vdev *vdev,
			    enum wlan_serialization_node node_type);

/**
 * wlan_serialization_remove_front() - Remove the front node of the list
 * @list: List from which the node is to be removed
 * @node: Pointer to store the node that is removed
 *
 * Return: QDF_STATUS Success or Failure
 */
QDF_STATUS wlan_serialization_remove_front(
			qdf_list_t *list,
			qdf_list_node_t **node);

/**
 * wlan_serialization_remove_node() - Remove the given node from the list
 * @list: List from which the node is to be removed
 * @node: Pointer to the node that is to be removed
 *
 * Return: QDF_STATUS Success or Failure
 */
QDF_STATUS wlan_serialization_remove_node(
			qdf_list_t *list,
			qdf_list_node_t *node);

/**
 * wlan_serialization_insert_front() - Insert a node into the front of the list
 * @list: List to which the node is to be inserted
 * @node: Pointer to the node that is to be inserted
 *
 * Return: QDF_STATUS Success or Failure
 */
QDF_STATUS wlan_serialization_insert_front(
			qdf_list_t *list,
			qdf_list_node_t *node);

/**
 * wlan_serialization_insert_back() - Insert a node into the back of the list
 * @list: List to which the node is to be inserted
 * @node: Pointer to the node that is to be inserted
 *
 * Return: QDF_STATUS Success or Failure
 */
QDF_STATUS wlan_serialization_insert_back(
			qdf_list_t *list,
			qdf_list_node_t *node);

/**
 * wlan_serialization_peek_front() - Peek the front node of the list
 * @list: List on which the node is to be peeked
 * @node: Pointer to the store the node that is being peeked
 *
 * Return: QDF_STATUS Success or Failure
 */
QDF_STATUS wlan_serialization_peek_front(
			qdf_list_t *list,
			qdf_list_node_t **node);

/**
 * wlan_serialization_peek_next() - Peek the next node of the list
 * @list: List on which the node is to be peeked
 * @node1: Pointer to the node1 from where the next node has to be peeked
 * @node2: Pointer to the store the node that is being peeked
 *
 * Return: QDF_STATUS Success or Failure
 */
QDF_STATUS wlan_serialization_peek_next(
			qdf_list_t *list,
			qdf_list_node_t *node1,
			qdf_list_node_t **node2);

/**
 * wlan_serialization_acquire_lock() - Acquire lock to the given queue
 * @lock: Pointer to the lock
 *
 * Return: QDF_STATUS success or failure
 */
QDF_STATUS
wlan_serialization_acquire_lock(qdf_spinlock_t *lock);

/**
 * wlan_serialization_release_lock() - Release lock to the given queue
 * @lock: Pointer to the lock
 *
 * Return: QDF_STATUS success or failure
 */
QDF_STATUS
wlan_serialization_release_lock(qdf_spinlock_t *lock);

/**
 * wlan_serialization_create_lock() - Init the lock to the given queue
 * @lock: Pointer to the lock
 *
 * Return: QDF_STATUS success or failure
 */
QDF_STATUS
wlan_serialization_create_lock(qdf_spinlock_t *lock);

/**
 * wlan_serialization_destroy_lock() - Deinit the lock to the given queue
 * @lock: Pointer to the lock
 *
 * Return: QDF_STATUS success or failure
 */
QDF_STATUS
wlan_serialization_destroy_lock(qdf_spinlock_t *lock);

/**
 * wlan_ser_update_cmd_history() - Update serialization queue history
 * @pdev_queue:serialization pdev queue
 * @cmd: cmd to be added/remeoved
 * @ser_reason: serialization action that resulted in addition/removal
 * @add_remove: added or removed from queue
 * @active_queue:for active queue
 *
 * Return: QDF_STATUS success or failure
 */

void wlan_ser_update_cmd_history(
		struct wlan_serialization_pdev_queue *pdev_queue,
		struct wlan_serialization_command *cmd,
		enum ser_queue_reason ser_reason,
		bool add_remove,
		bool active_queue);

#endif

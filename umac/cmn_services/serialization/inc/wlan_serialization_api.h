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
 * DOC: wlan_serialization_api.h
 * This file provides prototypes of the routines needed for the
 * external components to utilize the services provided by the
 * serialization component.
 */

/* Include files */
#ifndef __WLAN_SERIALIZATION_API_H
#define __WLAN_SERIALIZATION_API_H

#include "qdf_status.h"
#include "wlan_objmgr_cmn.h"

/* Preprocessor Definitions and Constants */

/*
 * struct wlan_serialization_queued_cmd_info member queue_type specifies the
 * below values to cancel the commands in these queues. Setting both the
 * bits will cancel the commands in both the queues.
 */
#define WLAN_SERIALIZATION_ACTIVE_QUEUE  0x1
#define WLAN_SERIALIZATION_PENDING_QUEUE 0x2

/**
 * enum wlan_serialization_cb_reason - reason for calling the callback
 * @WLAN_SERIALIZATION_REASON_ACTIVATE_CMD: activate the cmd by sending it to FW
 * @WLAN_SERIALIZATION_REASON_CANCEL_CMD: Cancel the cmd in the pending list
 * @WLAN_SERIALIZATION_REASON_RELEASE_MEM_CMD:cmd execution complete. Release
 *                                           the memory allocated while
 *                                           building the command
 * @WLAN_SER_CB_ACTIVE_CMD_TIMEOUT: active cmd has been timeout.
 */
enum wlan_serialization_cb_reason {
	WLAN_SER_CB_ACTIVATE_CMD,
	WLAN_SER_CB_CANCEL_CMD,
	WLAN_SER_CB_RELEASE_MEM_CMD,
	WLAN_SER_CB_ACTIVE_CMD_TIMEOUT,
};

/**
 * struct wlan_serialization_scan_info - Information needed for scan cmd
 * @is_cac_in_progress: boolean to check the cac status
 * @is_tdls_in_progress: boolean to check the tdls status
 *
 * This information is needed for scan command from other components
 * to apply the rules and check whether the cmd is allowed or not
 */
struct wlan_serialization_scan_info {
	bool is_cac_in_progress;
	bool is_tdls_in_progress;
};

/**
 * union wlan_serialization_rules_info - union of all rules info structures
 * @scan_info: information needed to apply rules on scan command
 */
union wlan_serialization_rules_info {
	struct wlan_serialization_scan_info scan_info;
};

/**
 * wlan_serialization_cmd_callback() - Callback registered by the component
 * @wlan_cmd: Command passed by the component for serialization
 * @reason: Reason code for which the callback is being called
 *
 * Reason specifies the reason for which the callback is being called. callback
 * should return success or failure based up on overall success of callback.
 * if callback returns failure then serialization will remove the command from
 * active queue and proceed for next pending command.
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE
 */
typedef QDF_STATUS (*wlan_serialization_cmd_callback) (void *wlan_cmd,
				 enum wlan_serialization_cb_reason reason);

/**
 * wlan_serialization_comp_info_cb() - callback to fill the rules information
 * @comp_info: Information filled by the component
 *
 * This callback is registered dynamically by the component with the
 * serialization component. Serialization component invokes the callback
 * while applying the rules for a particular command and the component
 * fills in the required information to apply the rules
 *
 * Return: None
 */
typedef void (*wlan_serialization_comp_info_cb)(
		union wlan_serialization_rules_info *comp_info);

/**
 * wlan_serialization_apply_rules_cb() - callback per command to apply rules
 * @comp_info: information needed to apply the rules
 *
 * The rules are applied using this callback and decided whether to
 * allow or deny the command
 *
 * Return: true, if rules are successfull and cmd can be queued
 *         false, if rules failed and cmd should not be queued
 */
typedef bool (*wlan_serialization_apply_rules_cb)(
		union wlan_serialization_rules_info *comp_info,
		uint8_t comp_id);

/**
 * enum wlan_umac_cmd_id - Command Type
 * @WLAN_SER_CMD_SCAN:     Scan command
 */
enum wlan_serialization_cmd_type {
	/* all scan command before non-scan */
	WLAN_SER_CMD_SCAN,
	WLAN_SER_CMD_SCAN_OTHER,
	WLAN_SER_CMD_SCAN_LOST_LINK1,
	WLAN_SER_CMD_SCAN_LOST_LINK2,
	WLAN_SER_CMD_SCAN_LOST_LINK3,
	WLAN_SER_CMD_SCAN_11D_TYPE1,
	WLAN_SER_CMD_SCAN_11D_TYPE2,
	WLAN_SER_CMD_SCAN_11D_DONE,
	WLAN_SER_CMD_SCAN_USER_REQ,
	WLAN_SER_CMD_SCAN_FOR_SSID,
	WLAN_SER_CMD_SCAN_IDLE_SCAN,
	WLAN_SER_CMD_SCAN_PROBE_BSS,
	WLAN_SER_CMD_SCAN_ABORT_NORMAL_SCAN,
	WLAN_SER_CMD_SCAN_P2P_FIND_PEER,
	WLAN_SER_CMD_SCAN_CANDIDATE_FOUND,
	WLAN_SER_CMD_REMAIN_ON_CHANNEL,
	/* all non-scan command below */
	WLAN_SER_CMD_NONSCAN,
	WLAN_SER_CMD_FORCE_DISASSOC,
	WLAN_SER_CMD_HDD_ISSUED,
	WLAN_SER_CMD_LOST_LINK1,
	WLAN_SER_CMD_LOST_LINK2,
	WLAN_SER_CMD_LOST_LINK3,
	WLAN_SER_CMD_FORCE_DISASSOC_MIC_FAIL,
	WLAN_SER_CMD_HDD_ISSUE_REASSOC_SAME_AP,
	WLAN_SER_CMD_SME_ISSUE_REASSOC_SAME_AP,
	WLAN_SER_CMD_SME_ISSUE_REASSOC_DIFF_AP,
	WLAN_SER_CMD_FORCE_DEAUTH,
	WLAN_SER_CMD_SME_ISSUE_DISASSOC_FOR_HANDOFF,
	WLAN_SER_CMD_SME_ISSUE_ASSOC_TO_SIMILAR_AP,
	WLAN_SER_CMD_SME_ISSUE_IBSS_JOIN_FAIL,
	WLAN_SER_CMD_FORCE_IBSS_LEAVE,
	WLAN_SER_CMD_STOP_BSS,
	WLAN_SER_CMD_SME_ISSUE_FT_REASSOC,
	WLAN_SER_CMD_FORCE_DISASSOC_STA,
	WLAN_SER_CMD_FORCE_DEAUTH_STA,
	WLAN_SER_CMD_PERFORM_PRE_AUTH,
	WLAN_SER_CMD_LOST_LINK1_ABORT,
	WLAN_SER_CMD_LOST_LINK2_ABORT,
	WLAN_SER_CMD_LOST_LINK3_ABORT,
	WLAN_SER_CMD_WM_STATUS_CHANGE,
	WLAN_SER_CMD_SET_KEY,
	WLAN_SER_CMD_NDP_INIT_REQ,
	WLAN_SER_CMD_NDP_RESP_REQ,
	WLAN_SER_CMD_NDP_DATA_END_INIT_REQ,
	WLAN_SER_CMD_ENTER_STANDBY,
	WLAN_SER_CMD_ADDTS,
	WLAN_SER_CMD_DELTS,
	WLAN_SER_CMD_TDLS_SEND_MGMT,
	WLAN_SER_CMD_TDLS_ADD_PEER,
	WLAN_SER_CMD_TDLS_DEL_PEER,
	WLAN_SER_CMD_TDLS_LINK_EST,
	WLAN_SER_CMD_SET_HW_MODE,
	WLAN_SER_CMD_NSS_UPDATE,
	WLAN_SER_CMD_SET_DUAL_MAC_CONFIG,
	WLAN_SER_CMD_SET_ANTENNA_MODE,
	WLAN_SER_CMD_ENTER_BMPS,
	WLAN_SER_CMD_EXIT_BMPS,
	WLAN_SER_CMD_ENTER_UAPSD,
	WLAN_SER_CMD_EXIT_UAPSD,
	WLAN_SER_CMD_EXIT_WOWL,
	WLAN_SER_CMD_MAX
};

/**
 * enum wlan_serialization_cancel_type - Type of commands to be cancelled
 * @WLAN_SER_CANCEL_SINGLE_SCAN: Cancel a single scan with a given ID
 * @WLAN_SER_CANCEL_PDEV_SCANS: Cancel all the scans on a given pdev
 * @WLAN_SER_CANCEL_VDEV_SCANS: Cancel all the scans on given vdev
 * @WLAN_SER_CANCEL_NON_SCAN_CMD: Cancel the given non scan command
 */
enum wlan_serialization_cancel_type {
	WLAN_SER_CANCEL_SINGLE_SCAN,
	WLAN_SER_CANCEL_PDEV_SCANS,
	WLAN_SER_CANCEL_VDEV_SCANS,
	WLAN_SER_CANCEL_NON_SCAN_CMD,
	WLAN_SER_CANCEL_MAX,
};

/**
 * enum wlan_serialization_status - Return status of cmd serialization request
 * @WLAN_SER_CMD_PENDING: Command is put into the pending queue
 * @WLAN_SER_CMD_ACTIVE: Command is activated and put in active queue
 * @WLAN_SER_CMD_DENIED_RULES_FAILED: Command denied as the rules fail
 * @WLAN_SER_CMD_DENIED_LIST_FULL: Command denied as the pending list is full
 * @WLAN_SER_CMD_DENIED_UNSPECIFIED: Command denied due to unknown reason
 */
enum wlan_serialization_status {
	WLAN_SER_CMD_PENDING,
	WLAN_SER_CMD_ACTIVE,
	WLAN_SER_CMD_DENIED_RULES_FAILED,
	WLAN_SER_CMD_DENIED_LIST_FULL,
	WLAN_SER_CMD_DENIED_UNSPECIFIED,
};

/**
 * enum wlan_serialization_cmd_status - Return status for a cancel request
 * @WLAN_SER_CMD_IN_PENDING_LIST: Command cancelled from pending list
 * @WLAN_SER_CMD_IN_ACTIVE_LIST: Command cancelled from active list
 * @WLAN_SER_CMDS_IN_ALL_LISTS: Command cancelled from all lists
 * @WLAN_SER_CMD_NOT_FOUND: Specified command to be cancelled
 *                                    not found in the lists
 */
enum wlan_serialization_cmd_status {
	WLAN_SER_CMD_IN_PENDING_LIST,
	WLAN_SER_CMD_IN_ACTIVE_LIST,
	WLAN_SER_CMDS_IN_ALL_LISTS,
	WLAN_SER_CMD_NOT_FOUND,
};

/**
 * struct wlan_serialization_command - Command to be serialized
 * @wlan_serialization_cmd_type: Type of command
 * @cmd_id: Command Identifier
 * @cmd_cb: Command callback
 * @source: component ID of the source of the command
 * @is_high_priority: Normal/High Priority at which the cmd has to be queued
 * @cmd_timeout_cb: Command timeout callback
 * @cmd_timeout_duration: Timeout duration in milliseconds
 * @vdev: VDEV object associated to the command
 * @umac_cmd: Actual command that needs to be sent to WMI/firmware
 *
 * Note: Unnamed union has been used in this structure, so that in future if
 * somebody wants to add pdev or psoc structure then that person can add without
 * modifying existing code.
 */
struct wlan_serialization_command {
	enum wlan_serialization_cmd_type cmd_type;
	uint32_t cmd_id;
	wlan_serialization_cmd_callback cmd_cb;
	enum wlan_umac_comp_id source;
	bool is_high_priority;
	uint16_t cmd_timeout_duration;
	union {
		struct wlan_objmgr_vdev *vdev;
	};
	void *umac_cmd;
};

/**
 * struct wlan_serialization_queued_cmd_info  - cmd that has to be cancelled
 * @requestor: component ID of the source requesting this action
 * @cmd_type: Command type
 * @cmd_id: Command ID
 * @req_type: Commands that need to be cancelled
 * @vdev: VDEV object associated to the command
 * @queue_type: Queues from which the command to be cancelled
 */
struct wlan_serialization_queued_cmd_info {
	enum wlan_umac_comp_id requestor;
	enum wlan_serialization_cmd_type cmd_type;
	uint32_t cmd_id;
	enum wlan_serialization_cancel_type req_type;
	union {
		struct wlan_objmgr_vdev *vdev;
	};
	uint8_t queue_type;
};

/**
 * wlan_serialization_cancel_request() - Request to cancel a command
 * @req: Request information
 *
 * This API is used by external components to cancel a command
 * that is either in the pending or active queue. Based on the
 * req_type, it is decided whether to use pdev or vdev
 * object. For all non-scan commands, it will be pdev.
 *
 * Return: Status specifying the removal of a command from a certain queue
 */
enum wlan_serialization_cmd_status
wlan_serialization_cancel_request(
		struct wlan_serialization_queued_cmd_info *req);

/**
 * wlan_serialization_remove_cmd() - Request to release a command
 * @cmd: Command information
 *
 * This API is used to release a command sitting in the active
 * queue upon successful completion of the command
 *
 * Return: None
 */
void wlan_serialization_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd);

/**
 * wlan_serialization_flush_cmd() - Request to flush command
 * @cmd: Command information
 *
 * This API is used to flush a cmd sitting in the queue. It
 * simply flushes the cmd from the queue and does not call
 * any callbacks in between. If the request is for active
 * queue, and if the active queue becomes empty upon flush,
 * then it will pick the next pending cmd and put in the active
 * queue before returning.
 *
 * Return: None
 */
void wlan_serialization_flush_cmd(
		struct wlan_serialization_queued_cmd_info *cmd);
/**
 * wlan_serialization_request() - Request to serialize a command
 * @cmd: Command information
 *
 * Return: Status of the serialization request
 */
enum wlan_serialization_status
wlan_serialization_request(struct wlan_serialization_command *cmd);

/**
 * wlan_serialization_register_comp_info_cb() - Register component's info
 * 						callback
 * @psoc: PSOC object information
 * @comp_id: Component ID
 * @cmd_type: Command Type
 * @cb: Callback
 *
 * This is called from component during its initialization.It initializes
 * callback handler for given comp_id/cmd_id in a 2-D array.
 *
 * Return: QDF Status
 */
QDF_STATUS
wlan_serialization_register_comp_info_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_comp_info_cb cb);

/**
 * wlan_serialization_deregister_comp_info_cb() - Deregister component's info
 *						callback
 * @psoc: PSOC object information
 * @comp_id: Component ID
 * @cmd_type: Command Type
 *
 * This routine is called from other component during its de-initialization.
 *
 * Return: QDF Status
 */
QDF_STATUS
wlan_serialization_deregister_comp_info_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type);

/**
 * wlan_serialization_register_apply_rules_cb() - Register component's rules
 *						callback
 * @psoc: PSOC object information
 * @cmd_type: Command Type
 * @cb: Callback
 *
 * This is called from component during its initialization.It initializes
 * callback handler for given cmd_type in a 1-D array.
 *
 * Return: QDF Status
 */
QDF_STATUS
wlan_serialization_register_apply_rules_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_apply_rules_cb apply_rules_cb);

/**
 * wlan_serialization_deregister_apply_rules_cb() - Deregister component's rules
 *						callback
 * @psoc: PSOC object information
 * @cmd_type: Command Type
 *
 * This routine is called from other component during its de-initialization.
 *
 * Return: QDF Status
 */
QDF_STATUS
wlan_serialization_deregister_apply_rules_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_serialization_cmd_type cmd_type);

/**
 * @wlan_serialization_init() - Serialization component initialization routine
 *
 * Return - QDF Status
 */
QDF_STATUS wlan_serialization_init(void);

/**
 * @wlan_serialization_deinit() - Serialization component de-init routine
 *
 * Return - QDF Status
 */
QDF_STATUS wlan_serialization_deinit(void);

/**
 * @wlan_serialization_psoc_open() - Serialization component open routine
 *
 * Return - QDF Status
 */
QDF_STATUS wlan_serialization_psoc_open(struct wlan_objmgr_psoc *psoc);

/**
 * @wlan_serialization_psoc_close() - Serialization component close routine
 *
 * Return - QDF Status
 */
QDF_STATUS wlan_serialization_psoc_close(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_serialization_vdev_scan_status() - Return the status of the vdev scan
 * @vdev: VDEV Object
 *
 * Return: Status of the scans for the corresponding vdev
 */
enum wlan_serialization_cmd_status
wlan_serialization_vdev_scan_status(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_serialization_pdev_scan_status() - Return the status of the pdev scan
 * @pdev: PDEV Object
 *
 * Return: Status of the scans for the corresponding pdev
 */
enum wlan_serialization_cmd_status
wlan_serialization_pdev_scan_status(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_serialization_non_scan_cmd_status() - Return status of pdev non-scan cmd
 * @pdev: PDEV Object
 * @cmd_id: ID of the command for which the status has to be checked
 *
 * Return: Status of the command for the corresponding pdev
 */
enum wlan_serialization_cmd_status
wlan_serialization_non_scan_cmd_status(struct wlan_objmgr_pdev *pdev,
		enum wlan_serialization_cmd_type cmd_id);

/**
 * wlan_serialization_is_cmd_present_in_pending_queue() - Return if the command
 *				is already present in pending queue
 * @cmd: pointer to serialization command to check
 *
 * This API will check if command is present in pending queue. If present
 * then return true, so use know that it is duplicated command
 *
 * Return: true or false
 */
bool wlan_serialization_is_cmd_present_in_pending_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd);
/**
 * wlan_serialization_is_cmd_present_in_active_queue() - Return if the command
 *			is already present in active queue
 * @cmd: pointer to serialization command to check
 *
 * This API will check if command is present in active queue. If present
 * then return true, so use know that it is duplicated command
 *
 * Return: true or false
 */
bool wlan_serialization_is_cmd_present_in_active_queue(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_serialization_command *cmd);
#endif

/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: This file contains various structures, macros used by the TWT
 * component and other components interfacing with TWT component.
 */

#ifndef _WLAN_TWT_PUBLIC_STRUCTS_H_
#define _WLAN_TWT_PUBLIC_STRUCTS_H_

#include <qdf_types.h>

#define WLAN_MAX_TWT_SESSIONS_PER_PEER 1
#define TWT_ALL_SESSIONS_DIALOG_ID 255

/**
 * enum wlan_twt_commands  - TWT commands
 * @WLAN_TWT_NONE: Indicates none of the TWT commands are active.
 * @WLAN_TWT_SETUP: TWT setup
 * @WLAN_TWT_TERMINATE: TWT terminate
 * @WLAN_TWT_SUSPEND: TWT suspend
 * @WLAN_TWT_RESUME: TWT resume
 * @WLAN_TWT_NUDGE: TWT nudge
 * @WLAN_TWT_STATISTICS: TWT statistics
 * @WLAN_TWT_CLEAR_STATISTICS: TWT clear statistics
 * @WLAN_TWT_ANY: Indicates one of the commands is in progress.
 */
enum wlan_twt_commands {
	WLAN_TWT_NONE             = 0,
	WLAN_TWT_SETUP            = BIT(0),
	WLAN_TWT_TERMINATE        = BIT(1),
	WLAN_TWT_SUSPEND          = BIT(2),
	WLAN_TWT_RESUME           = BIT(3),
	WLAN_TWT_NUDGE            = BIT(4),
	WLAN_TWT_STATISTICS       = BIT(5),
	WLAN_TWT_CLEAR_STATISTICS = BIT(6),
	WLAN_TWT_ANY              = 0xFF,
};

/**
 * enum wlan_twt_capabilities  - Represents the Bitmap of TWT capabilities
 * supported by device and peer.
 * @WLAN_TWT_CAPA_REQUESTOR: TWT requestor support is advertised by TWT
 * non-scheduling STA.
 * @WLAN_TWT_CAPA_RESPONDER: TWT responder support is advertised by TWT
 * AP.
 * @WLAN_TWT_CAPA_BROADCAST: This indicates support for the role of broadcast
 * TWT scheduling/receiving functionality.
 * @WLAN_TWT_CAPA_FLEXIBLE: Device supports flexible TWT schedule.
 * @WLAN_TWT_CAPA_REQUIRED: The TWT Required is advertised by AP to indicate
 * that it mandates the associated HE STAs to support TWT.
 */
enum wlan_twt_capabilities {
	WLAN_TWT_CAPA_REQUESTOR = BIT(0),
	WLAN_TWT_CAPA_RESPONDER = BIT(1),
	WLAN_TWT_CAPA_BROADCAST = BIT(2),
	WLAN_TWT_CAPA_FLEXIBLE =  BIT(3),
	WLAN_TWT_CAPA_REQUIRED =  BIT(4),
};

/**
 * enum wlan_twt_session_state  - TWT session state for a dialog id
 * @WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED: Session doesn't exist
 * @WLAN_TWT_SETUP_STATE_ACTIVE: TWT session is active
 * @WLAN_TWT_SETUP_STATE_SUSPEND: TWT session is suspended
 */
enum wlan_twt_session_state {
	WLAN_TWT_SETUP_STATE_NOT_ESTABLISHED = 0,
	WLAN_TWT_SETUP_STATE_ACTIVE          = 1,
	WLAN_TWT_SETUP_STATE_SUSPEND         = 2,
};

/* enum TWT_ROLE - role specified in ext conf in wmi_twt_enable/disable_cmd
 * TWT_ROLE_REQUESTOR: TWT role is requestor
 * TWT_ROLE_RESPONDER: TWT role is responder
 */
enum TWT_ROLE {
	TWT_ROLE_REQUESTOR,
	TWT_ROLE_RESPONDER,
};

/* enum TWT_OPERATION - specified in ext conf in wmi_twt_enable/disable_cmd
 * TWT_OPERATION_INDIVIDUAL: Individual TWT operation
 * TWT_OPERATION_BROADCAST: Broadcast TWT operation
 */
enum TWT_OPERATION {
	TWT_OPERATION_INDIVIDUAL,
	TWT_OPERATION_BROADCAST,
};

/**
 * struct twt_enable_param:
 * @pdev_id: pdev_id for identifying the MAC.
 * @sta_cong_timer_ms: STA TWT congestion timer TO value in terms of ms
 * @mbss_support: Flag indicating if AP TWT feature supported in
 *                MBSS mode or not.
 * @default_slot_size: This is the default value for the TWT slot setup
 *                by AP (units = microseconds)
 * @congestion_thresh_setup: Minimum congestion required to start setting
 *                up TWT sessions
 * @congestion_thresh_teardown: Minimum congestion below which TWT will be
 *                torn down (in percent of occupied airtime)
 * @congestion_thresh_critical: Threshold above which TWT will not be active
 *                (in percent of occupied airtime)
 * @interference_thresh_teardown: Minimum interference above that TWT
 *                 will not be active. The interference parameters use an
 *                 abstract method of evaluating interference.
 *                 The parameters are in percent, ranging from 0 for no
 *                 interference, to 100 for interference extreme enough
 *                 to completely block the signal of interest.
 * @interference_thresh_setup: Minimum interference below that TWT session
 *                 can be setup. The interference parameters use an
 *                 abstract method of evaluating interference.
 *                 The parameters are in percent, ranging from 0 for no
 *                 interference, to 100 for interference extreme enough
 *                 to completely block the signal of interest.
 * @min_no_sta_setup: Minimum no of STA required to start TWT setup
 * @min_no_sta_teardown: Minimum no of STA below which TWT will be torn down
 * @no_of_bcast_mcast_slots: Number of default slot sizes reserved for
 *                 BCAST/MCAST delivery
 * @min_no_twt_slots: Minimum no of available slots for TWT to be operational
 * @max_no_sta_twt: Max no of STA with which TWT is possible
 *                 (must be <= the wmi_resource_config's twt_ap_sta_count value)
 *      * The below interval parameters have units of milliseconds.
 * @mode_check_interval: Interval between two successive check to decide the
 *                 mode of TWT. (units = milliseconds)
 * @add_sta_slot_interval: Interval between decisions making to create
 *                 TWT slots for STAs. (units = milliseconds)
 * @remove_sta_slot_interval: Inrerval between decisions making to remove TWT
 *                 slot of STAs. (units = milliseconds)
 * @twt_role: values from enum TWT_ROLE.
 * @twt_oper: values from enum TWT_OPERATION.
 * @ext_conf_present: If requestor/responder extend config is present.
 * @b_twt_enable: Enable or disable broadcast TWT.
 * @b_twt_legacy_mbss_enable: Enable or disable legacy MBSSID TWT.
 * @b_twt_ax_mbss_enable: Enable or disable 11AX MBSSID TWT.
 */
struct twt_enable_param {
	uint32_t pdev_id;
	uint32_t sta_cong_timer_ms;
	uint32_t mbss_support;
	uint32_t default_slot_size;
	uint32_t congestion_thresh_setup;
	uint32_t congestion_thresh_teardown;
	uint32_t congestion_thresh_critical;
	uint32_t interference_thresh_teardown;
	uint32_t interference_thresh_setup;
	uint32_t min_no_sta_setup;
	uint32_t min_no_sta_teardown;
	uint32_t no_of_bcast_mcast_slots;
	uint32_t min_no_twt_slots;
	uint32_t max_no_sta_twt;
	uint32_t mode_check_interval;
	uint32_t add_sta_slot_interval;
	uint32_t remove_sta_slot_interval;
	enum TWT_ROLE twt_role;
	enum TWT_OPERATION twt_oper;
	bool ext_conf_present;
	uint32_t b_twt_enable:1,
		 b_twt_legacy_mbss_enable:1,
		 b_twt_ax_mbss_enable:1;
};

/* HOST_TWT_DISABLE_REASON - reason code of disable TWT
 * @HOST_TWT_DISABLE_REASON_NONE: Host sends TWT disable command to firmware
 *                              in normal case.
 * @HOST_TWT_DISABLE_REASON_CONCURRENCY_SCC: Host sends TWT disable command
 *                              to firmware when SCC concurrency exist.
 * @HOST_TWT_DISABLE_REASON_CONCURRENCY_MCC: Host sends TWT disable command
 *                              to firmware when MCC concurrency exist.
 * @HOST_TWT_DISABLE_REASON_CHANGE_CONGESTION_TIMEOUT: Host sends TWT disable
 *                              command to firmware to change congestion timeout
 * @HOST_TWT_DISABLE_REASON_P2P_GO_NOA: Host sends TWT disable command to
 *                              firmware when P2P GO NOA exist.
 */
enum HOST_TWT_DISABLE_REASON {
	HOST_TWT_DISABLE_REASON_NONE,
	HOST_TWT_DISABLE_REASON_CONCURRENCY_SCC,
	HOST_TWT_DISABLE_REASON_CONCURRENCY_MCC,
	HOST_TWT_DISABLE_REASON_CHANGE_CONGESTION_TIMEOUT,
	HOST_TWT_DISABLE_REASON_P2P_GO_NOA,
};

/**
 * struct twt_disable_param:
 * @pdev_id: pdev_id for identifying the MAC.
 * @ext_conf_present: If requestor/responder extend config is present.
 * @twt_role: values from enum TWT_ROLE.
 * @twt_oper: values from enum TWT_OPERATION.
 * @dis_reason_code: values from enum HOST_TWT_DISABLE_REASON.
 */
struct twt_disable_param {
	uint32_t pdev_id;
	bool ext_conf_present;
	enum TWT_ROLE twt_role;
	enum TWT_OPERATION twt_oper;
	enum HOST_TWT_DISABLE_REASON dis_reason_code;
};

/* status code of enabling TWT
 * HOST_TWT_ENABLE_STATUS_OK: enabling TWT successfully completed
 * HOST_TWT_ENABLE_STATUS_ALREADY_ENABLED: TWT already enabled
 * HOST_TWT_ENABLE_STATUS_NOT_READY: FW not ready for enabling TWT
 * HOST_TWT_ENABLE_INVALID_PARAM: invalid parameters
 * HOST_TWT_ENABLE_STATUS_UNKNOWN_ERROR: enabling TWT failed with an
 *                                      unknown reason
 * HOST_TWT_ENABLE_STATUS_INVALID_COMMAND: If the host has sent TWT enable
 *                     command and received TWT disable event or any other event
 *                     then host sets this status internally.
 */
enum HOST_TWT_ENABLE_STATUS {
	HOST_TWT_ENABLE_STATUS_OK,
	HOST_TWT_ENABLE_STATUS_ALREADY_ENABLED,
	HOST_TWT_ENABLE_STATUS_NOT_READY,
	HOST_TWT_ENABLE_INVALID_PARAM,
	HOST_TWT_ENABLE_STATUS_UNKNOWN_ERROR,
};

/* enum - status code of adding TWT dialog
 * @HOST_ADD_TWT_STATUS_OK: adding TWT dialog successfully completed
 * @HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED: TWT not enabled
 * @HOST_ADD_TWT_STATUS_USED_DIALOG_ID: TWT dialog ID is already used
 * @HOST_ADD_TWT_STATUS_INVALID_PARAM: invalid parameters
 * @HOST_ADD_TWT_STATUS_NOT_READY: FW not ready
 * @HOST_ADD_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @HOST_ADD_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @HOST_ADD_TWT_STATUS_NO_RESPONSE: peer AP did not send the response frame
 * @HOST_ADD_TWT_STATUS_DENIED: AP did not accept the request
 * @HOST_ADD_TWT_STATUS_UNKNOWN_ERROR: adding TWT dialog failed with
 * an unknown reason
 * @HOST_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE: peer AP wake interval,
 * duration not in range
 * @HOST_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED: peer AP IE Validation
 * Failed
 * @HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum HOST_ADD_TWT_STATUS {
	HOST_ADD_TWT_STATUS_OK,
	HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED,
	HOST_ADD_TWT_STATUS_USED_DIALOG_ID,
	HOST_ADD_TWT_STATUS_INVALID_PARAM,
	HOST_ADD_TWT_STATUS_NOT_READY,
	HOST_ADD_TWT_STATUS_NO_RESOURCE,
	HOST_ADD_TWT_STATUS_NO_ACK,
	HOST_ADD_TWT_STATUS_NO_RESPONSE,
	HOST_ADD_TWT_STATUS_DENIED,
	HOST_ADD_TWT_STATUS_UNKNOWN_ERROR,
	HOST_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE,
	HOST_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED,
	HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS,
	HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct twt_enable_complete_event_param:
 * @pdev_id: pdev_id for identifying the MAC.
 * @status: From enum TWT_ENABLE_STATUS
 */
struct twt_enable_complete_event_param {
	uint32_t pdev_id;
	uint32_t status;
};

/* enum HOST_TWT_DISABLE_STATUS  - status code of disable TWT
 * @HOST_TWT_DISABLE_STATUS_OK: TWT disable is successfully completed
 * @HOST_TWT_DISABLE_STATUS_ROAM_IN_PROGRESS: roaming in progress
 * @HOST_TWT_DISABLE_STATUS_CHAN_SW_IN_PROGRESS: channel switch in progress
 * @HOST_TWT_DISABLE_STATUS_SCAN_IN_PROGRESS: scan in progress
 * @HOST_TWT_DISABLE_STATUS_UNKNOWN_ERROR: unknown error
 */
enum HOST_TWT_DISABLE_STATUS {
	HOST_TWT_DISABLE_STATUS_OK,
	HOST_TWT_DISABLE_STATUS_ROAM_IN_PROGRESS,
	HOST_TWT_DISABLE_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_TWT_DISABLE_STATUS_SCAN_IN_PROGRESS,
	HOST_TWT_DISABLE_STATUS_UNKNOWN_ERROR,
};

/**
 * struct twt_disable_complete_event:
 * @pdev_id: pdev_id for identifying the MAC.
 * @status: From enum HOST_TWT_DISABLE_STATUS
 */
struct twt_disable_complete_event_param {
	uint32_t pdev_id;
	uint32_t status;
};

/**
 * twt_ack_context - twt ack private context
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @dialog_id: dialog id
 * @twt_cmd_ack: twt ack command
 * @status: twt command status
 */
struct twt_ack_context {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t twt_cmd_ack;
	uint32_t status;
};

/**
 * twt_en_dis_context - twt enable/disable private context
 * @twt_role: twt role. When the upper layer is invoking requestor/responder
 * enable/disable command via the TWT component API, Firmare WMI event
 * doesn't have the info in the event params to distinguish if this enable or
 * disable is for requestor or responder, hence TWT component stores this role
 * before the request is sent to the firmware.
 * @context: cookie. This is the cookie information passed back to the upper
 * layer to help identify the request structure
 */
struct twt_en_dis_context {
	enum TWT_ROLE twt_role;
	void *context;
};

/**
 * struct twt_ack_complete_event_param:
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @dialog_id: dialog id
 * @twt_cmd_ack: ack event to the corresponding twt command
 * @status: twt command status
 */
struct twt_ack_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t twt_cmd_ack;
	uint32_t status;
};

/**
 * struct twt_session_stats_info:
 * @vdev_id: id of VDEV for twt session
 * @peer_mac: MAC address of node
 * @event_type: Indicates TWT session type (SETUP/TEARDOWN/UPDATE)
 * @flow_id: TWT flow identifier established with TWT peer
 * @bcast:  If this is a broacast TWT session
 * @trig: If the TWT session is trigger enabled
 * @announ: If the flow type is announced/unannounced
 * @protection: If the TWT protection field is set
 * @info_frame_disabled: If the TWT Information frame is disabled
 * @dialog_id: Dialog_id of current session
 * @wake_dura_us: wake duration in us
 * @wake_intvl_us: wake time interval in us
 * @sp_offset_us: Time until initial TWT SP occurs
 * @sp_tsf_us_lo: TWT wake time TSF in usecs lower bits - 31:0
 * @sp_tsf_us_hi: TWT wake time TSF in usecs higher bits - 63:32
 */
struct twt_session_stats_info {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_mac;
	uint32_t event_type;
	uint32_t flow_id:16,
		 bcast:1,
		 trig:1,
		 announ:1,
		 protection:1,
		 info_frame_disabled:1,
		 pm_responder_bit_valid:1,
		 pm_responder_bit:1;
	uint32_t dialog_id;
	uint32_t wake_dura_us;
	uint32_t wake_intvl_us;
	uint32_t sp_offset_us;
	uint32_t sp_tsf_us_lo;
	uint32_t sp_tsf_us_hi;
};

/**
 * struct twt_session_stats_event_param:
 * @pdev_id: pdev_id for identifying the MAC.
 * @num_sessions: number of TWT sessions
 * @twt_sessions: received TWT sessions
 */
struct twt_session_stats_event_param {
	uint32_t pdev_id;
	uint32_t num_sessions;
};

/* from IEEE 802.11ah section 9.4.2.200 */
enum HOST_TWT_COMMAND {
	HOST_TWT_COMMAND_REQUEST_TWT    = 0,
	HOST_TWT_COMMAND_SUGGEST_TWT    = 1,
	HOST_TWT_COMMAND_DEMAND_TWT     = 2,
	HOST_TWT_COMMAND_TWT_GROUPING   = 3,
	HOST_TWT_COMMAND_ACCEPT_TWT     = 4,
	HOST_TWT_COMMAND_ALTERNATE_TWT  = 5,
	HOST_TWT_COMMAND_DICTATE_TWT    = 6,
	HOST_TWT_COMMAND_REJECT_TWT     = 7,
};

/**
 * struct twt_add_dialog_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: peer MAC address when vdev is AP VDEV
 * @dialog_id: diaglog_id (TWT dialog ID)
 *             This dialog ID must be unique within its vdev.
 * @wake_intvl_us: TWT Wake Interval in units of us
 * @wake_intvl_mantis: TWT Wake Interval Mantissa
 *                 - wake_intvl_mantis must be <= 0xFFFF
 *                 - wake_intvl_us must be divided evenly by wake_intvl_mantis,
 *                   i.e., wake_intvl_us % wake_intvl_mantis == 0
 *                 - the quotient of wake_intvl_us/wake_intvl_mantis must be
 *                   2 to N-th(0<=N<=31) power,
 *                   i.e., wake_intvl_us/wake_intvl_mantis == 2^N, 0<=N<=31
 * @min_wake_intvl_us: Min tolerance limit of TWT wake interval
 * @max_wake_intvl_us: Max tolerance limit of TWT wake interval
 * @wake_dura_us: TWT Wake Duration in units of us, must be <= 0xFFFF
 *                wake_dura_us must be divided evenly by 256,
 *                i.e., wake_dura_us % 256 == 0
 * @min_wake_dura_us: Min tolerance limit of TWT wake duration.
 * @max_wake_dura_us: Max tolerance limit of TWT wake duration.
 * @sp_offset_us: this long time after TWT setup the 1st SP will start.
 * @twt_cmd: cmd from enum HOST_TWT_COMMAND
 * @flag_bcast: 0 means Individual TWT,
 *              1 means Broadcast TWT
 * @flag_trigger: 0 means non-Trigger-enabled TWT,
 *                1 means  means Trigger-enabled TWT
 * @flag_flow_type:  0 means announced TWT,
 *                   1 means un-announced TWT
 * @flag_protection: 0 means TWT protection is required,
 *                   1 means TWT protection is not required
 * @b_twt_id0: 0 means BTWT recommendation will not be used
 *             1 means BTWT recommendation will be used
 * @flag_reserved: unused bits
 * @b_twt_recommendation: defines types of frames tx during bTWT SP
 * @b_twt_persistence: Countdown VAL frames to param update/teardown
 * @wake_time_tsf: Absolute TSF value to start first TWT service period
 * @annouce_timeout_us: Timeout value before sending QoS NULL frame.
 */
struct twt_add_dialog_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t wake_intvl_us;
	uint32_t wake_intvl_mantis;
	uint32_t min_wake_intvl_us;
	uint32_t max_wake_intvl_us;
	uint32_t wake_dura_us;
	uint32_t min_wake_dura_us;
	uint32_t max_wake_dura_us;
	uint32_t sp_offset_us;
	enum HOST_TWT_COMMAND twt_cmd;
	uint32_t
		flag_bcast:1,
		flag_trigger:1,
		flag_flow_type:1,
		flag_protection:1,
		flag_b_twt_id0:1,
		flag_reserved:11,
		b_twt_persistence:8,
		b_twt_recommendation:3;
	uint64_t wake_time_tsf;
	uint32_t announce_timeout_us;
};

/* enum HOST_TWT_GET_STATS_STATUS - status code of TWT Get stats dialog id
 * @HOST_TWT_GET_STATS_STATUS_OK: Get status TWT dialog successfully completed
 * @HOST_TWT_GET_STATS_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID does not exist
 * @HOST_TWT_GET_STATS_STATUS_INVALID_PARAM: Invalid parameters
 * @HOST_TWT_GET_STATS_STATUS_UNKNOWN_ERROR: Unknown error
 */
enum HOST_TWT_GET_STATS_STATUS {
	HOST_TWT_GET_STATS_STATUS_OK,
	HOST_TWT_GET_STATS_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_GET_STATS_STATUS_INVALID_PARAM,
	HOST_TWT_GET_STATS_STATUS_UNKNOWN_ERROR,
};

/* enum HOST_TWT_ADD_STATUS - status code of TWT add dialog
 * @HOST_TWT_ADD_STATUS_OK: add TWT dialog successfully completed
 * @HOST_TWT_ADD_STATUS_TWT_NOT_ENABLED: TWT not enabled
 * @HOST_TWT_ADD_STATUS_USED_DIALOG_ID: TWT dialog ID is already used
 * @HOST_TWT_ADD_STATUS_INVALID_PARAM: invalid parameters
 * @HOST_TWT_ADD_STATUS_NOT_READY: FW not ready
 * @HOST_TWT_ADD_STATUS_NO_RESOURCE: FW resource exhausted
 * @HOST_TWT_ADD_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @HOST_TWT_ADD_STATUS_NO_RESPONSE: peer AP did not send the response frame
 * @HOST_TWT_ADD_STATUS_DENIED: AP did not accept the request
 * @HOST_TWT_ADD_STATUS_UNKNOWN_ERROR: adding TWT dialog failed with
 * an unknown reason
 * @HOST_TWT_ADD_STATUS_AP_PARAMS_NOT_IN_RANGE: peer AP wake interval,
 * duration not in range
 * @HOST_TWT_ADD_STATUS_AP_IE_VALIDATION_FAILED: peer AP IE Validation
 * Failed
 * @HOST_TWT_ADD_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @HOST_TWT_ADD_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @HOST_TWT_ADD_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum HOST_TWT_ADD_STATUS {
	HOST_TWT_ADD_STATUS_OK,
	HOST_TWT_ADD_STATUS_TWT_NOT_ENABLED,
	HOST_TWT_ADD_STATUS_USED_DIALOG_ID,
	HOST_TWT_ADD_STATUS_INVALID_PARAM,
	HOST_TWT_ADD_STATUS_NOT_READY,
	HOST_TWT_ADD_STATUS_NO_RESOURCE,
	HOST_TWT_ADD_STATUS_NO_ACK,
	HOST_TWT_ADD_STATUS_NO_RESPONSE,
	HOST_TWT_ADD_STATUS_DENIED,
	HOST_TWT_ADD_STATUS_UNKNOWN_ERROR,
	HOST_TWT_ADD_STATUS_AP_PARAMS_NOT_IN_RANGE,
	HOST_TWT_ADD_STATUS_AP_IE_VALIDATION_FAILED,
	HOST_TWT_ADD_STATUS_ROAM_IN_PROGRESS,
	HOST_TWT_ADD_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_TWT_ADD_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct twt_add_dialog_additional_params -
 * @twt_cmd: TWT command
 * @bcast: 0 means Individual TWT
 *         1 means Broadcast TWT
 * @trig_en: 0 means non-Trigger-enabled TWT
 *           1 means Trigger-enabled TWT
 * @announce: 0 means announced TWT
 *            1 means un-announced TWT
 * @protection: 0 means TWT protection is required
 *              1 means TWT protection is not required
 * @b_twt_id0: 0 means non-0 B-TWT ID or I-TWT
 *             1 means B-TWT ID 0
 * @info_frame_disabled: 0 means TWT Information frame is enabled
 *                       1 means TWT Information frame is disabled
 * @pm_responder_bit_valid: 1 means responder pm mode field is valid
 *                          0 means responder pm mode field is not valid
 * @pm_responder_bit: 1 means that responder set responder pm mode to 1
 *                    0 means that responder set responder pm mode to 0
 * @wake_dura_us: wake duration in us
 * @wake_intvl_us: wake time interval in us
 * @sp_offset_us: Time until initial TWT SP occurs
 * @sp_tsf_us_lo: TWT service period tsf in usecs lower bits - 31:0
 * @sp_tsf_us_hi: TWT service period tsf in usecs higher bits - 63:32
 */
struct twt_add_dialog_additional_params {
	uint32_t twt_cmd:8,
		 bcast:1,
		 trig_en:1,
		 announce:1,
		 protection:1,
		 b_twt_id0:1,
		 info_frame_disabled:1,
		 pm_responder_bit_valid:1,
		 pm_responder_bit:1;
	uint32_t wake_dur_us;
	uint32_t wake_intvl_us;
	uint32_t sp_offset_us;
	uint32_t sp_tsf_us_lo;
	uint32_t sp_tsf_us_hi;
};

/**
 * struct twt_add_dialog_complete_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to HOST_TWT_ADD_STATUS enum
 * @num_additional_twt_params: no of additional_twt_params available
 */
struct twt_add_dialog_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t status;
	uint32_t num_additional_twt_params;
};

/**
 * struct twt_add_dialog_complete_event - TWT add dialog complete event
 * @params: Fixed parameters for TWT add dialog complete event
 * @additional_params: additional parameters for TWT add dialog complete event
 *
 * Holds the fixed and additional parameters from add dialog
 * complete event
 */
struct twt_add_dialog_complete_event {
	struct twt_add_dialog_complete_event_param params;
	struct twt_add_dialog_additional_params additional_params;
};

/**
 * struct twt_del_dialog_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @b_twt_persistence: persistence val for b-twt
 */
struct twt_del_dialog_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
#ifdef WLAN_SUPPORT_BCAST_TWT
	uint32_t b_twt_persistence;
#endif
};

/**
 * enum HOST_TWT_DEL_STATUS - status code of delete TWT dialog
 * @HOST_TWT_DEL_STATUS_OK: deleting TWT dialog successfully completed
 * @HOST_TWT_DEL_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @HOST_TWT_DEL_STATUS_INVALID_PARAM: invalid parameters
 * @HOST_TWT_DEL_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @HOST_TWT_DEL_STATUS_NO_RESOURCE: FW resource exhausted
 * @HOST_TWT_DEL_STATUS_NO_ACK: peer AP/STA did not ACK the request/response
 * frame
 * @HOST_TWT_DEL_STATUS_UNKNOWN_ERROR: deleting TWT dialog failed with an
 * unknown reason
 * @HOST_TWT_DEL_STATUS_PEER_INIT_TEARDOWN: Peer initiated TWT teardown
 * @HOST_TWT_DEL_STATUS_ROAMING: TWT teardown due to roaming.
 * @HOST_TWT_DEL_STATUS_CONCURRENCY: TWT session teardown due to
 * concurrent session coming up.
 * @HOST_TWT_DEL_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @HOST_TWT_DEL_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 * @HOST_TWT_DEL_STATUS_PS_DISABLE_TEARDOWN: PS disable TWT teardown
 */
enum HOST_TWT_DEL_STATUS {
	HOST_TWT_DEL_STATUS_OK,
	HOST_TWT_DEL_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_DEL_STATUS_INVALID_PARAM,
	HOST_TWT_DEL_STATUS_DIALOG_ID_BUSY,
	HOST_TWT_DEL_STATUS_NO_RESOURCE,
	HOST_TWT_DEL_STATUS_NO_ACK,
	HOST_TWT_DEL_STATUS_UNKNOWN_ERROR,
	HOST_TWT_DEL_STATUS_PEER_INIT_TEARDOWN,
	HOST_TWT_DEL_STATUS_ROAMING,
	HOST_TWT_DEL_STATUS_CONCURRENCY,
	HOST_TWT_DEL_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_TWT_DEL_STATUS_SCAN_IN_PROGRESS,
	HOST_TWT_DEL_STATUS_PS_DISABLE_TEARDOWN,
};

/**
 * struct twt_del_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @b_twt_persistence: persistence val for b-twt
 * @status: refer to HOST_TWT_DEL_STATUS enum
 */
struct twt_del_dialog_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
#ifdef WLAN_SUPPORT_BCAST_TWT
	uint32_t b_twt_persistence;
#endif
	uint32_t status;
};

/**
 * struct twt_pause_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 */
struct twt_pause_dialog_cmd_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
};

/**
 * struct twt_nudge_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @suspend_duration: TWT suspend duration in microseconds
 * @next_twt_size: next TWT size
 */
struct twt_nudge_dialog_cmd_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t suspend_duration;
	uint32_t next_twt_size;
};

/* enum HOST_TWT_PAUSE_STATUS - status code of pause TWT dialog
 * @HOST_TWT_PAUSE_STATUS_OK: pausing TWT dialog successfully completed
 * @HOST_TWT_PAUSE_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @HOST_TWT_PAUSE_STATUS_INVALID_PARAM: invalid parameters
 * @HOST_TWT_PAUSE_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @HOST_TWT_PAUSE_STATUS_NO_RESOURCE: FW resource exhausted
 * @HOST_TWT_PAUSE_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @HOST_TWT_PAUSE_STATUS_UNKNOWN_ERROR: pausing TWT dialog failed with an
 * unknown reason
 * @HOST_TWT_PAUSE_STATUS_ALREADY_PAUSED: TWT dialog already in paused state
 * @HOST_TWT_PAUSE_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @HOST_TWT_PAUSE_STATUS_ROAM_IN_PROGRESS: Roaming is in progress
 * @HOST_TWT_PAUSE_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum HOST_TWT_PAUSE_STATUS {
	HOST_TWT_PAUSE_STATUS_OK,
	HOST_TWT_PAUSE_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_PAUSE_STATUS_INVALID_PARAM,
	HOST_TWT_PAUSE_STATUS_DIALOG_ID_BUSY,
	HOST_TWT_PAUSE_STATUS_NO_RESOURCE,
	HOST_TWT_PAUSE_STATUS_NO_ACK,
	HOST_TWT_PAUSE_STATUS_UNKNOWN_ERROR,
	HOST_TWT_PAUSE_STATUS_ALREADY_PAUSED,
	HOST_TWT_PAUSE_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_TWT_PAUSE_STATUS_ROAM_IN_PROGRESS,
	HOST_TWT_PAUSE_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct twt_pause_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to HOST_TWT_PAUSE_STATUS
 */
struct twt_pause_dialog_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	enum HOST_TWT_PAUSE_STATUS status;
};

/* enum HOST_TWT_NUDGE_STATUS - status code of nudge TWT dialog
 * @HOST_TWT_NUDGE_STATUS_OK: nudge TWT dialog successfully completed
 * @HOST_TWT_NUDGE_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @HOST_TWT_NUDGE_STATUS_INVALID_PARAM: invalid parameters
 * @HOST_TWT_NUDGE_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @HOST_TWT_NUDGE_STATUS_NO_RESOURCE: FW resource exhausted
 * @HOST_TWT_NUDGE_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @HOST_TWT_NUDGE_STATUS_UNKNOWN_ERROR: nudge TWT dialog failed with an
 * unknown reason
 * @HOST_TWT_NUDGE_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @HOST_TWT_NUDGE_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @HOST_TWT_NUDGE_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum HOST_TWT_NUDGE_STATUS {
	HOST_TWT_NUDGE_STATUS_OK,
	HOST_TWT_NUDGE_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_NUDGE_STATUS_INVALID_PARAM,
	HOST_TWT_NUDGE_STATUS_DIALOG_ID_BUSY,
	HOST_TWT_NUDGE_STATUS_NO_RESOURCE,
	HOST_TWT_NUDGE_STATUS_NO_ACK,
	HOST_TWT_NUDGE_STATUS_UNKNOWN_ERROR,
	HOST_TWT_NUDGE_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_TWT_NUDGE_STATUS_ROAM_IN_PROGRESS,
	HOST_TWT_NUDGE_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct twt_nudge_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to HOST_TWT_PAUSE_STATUS
 * @next_twt_tsf_us_lo: TSF lower bits (31:0) of next wake time
 * @next_twt_tsf_us_hi: TSF higher bits (32:63) of next wake time
 */
struct twt_nudge_dialog_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	enum     HOST_TWT_NUDGE_STATUS status;
	uint32_t next_twt_tsf_us_lo;
	uint32_t next_twt_tsf_us_hi;
};

/**
 * struct twt_resume_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @sp_offset_us: this long time after TWT resumed the 1st SP will start
 * @next_twt_size: Next TWT subfield Size.
 *                 Refer IEEE 802.11ax section "9.4.1.60 TWT Information field"
 */
struct twt_resume_dialog_cmd_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t sp_offset_us;
	uint32_t next_twt_size;
};

/* enum HOST_TWT_RESUME_STATUS - status code of resume TWT dialog
 * @HOST_TWT_RESUME_STATUS_OK: resuming TWT dialog successfully completed
 * @HOST_TWT_RESUME_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @HOST_TWT_RESUME_STATUS_INVALID_PARAM: invalid parameters
 * @HOST_TWT_RESUME_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @HOST_TWT_RESUME_STATUS_NOT_PAUSED: dialog not paused currently
 * @HOST_TWT_RESUME_STATUS_NO_RESOURCE: FW resource exhausted
 * @HOST_TWT_RESUME_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @HOST_TWT_RESUME_STATUS_UNKNOWN_ERROR: resuming TWT dialog failed with an
 * unknown reason
 * @HOST_TWT_RESUME_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @HOST_TWT_RESUME_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @HOST_TWT_RESUME_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum HOST_TWT_RESUME_STATUS {
	HOST_TWT_RESUME_STATUS_OK,
	HOST_TWT_RESUME_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_RESUME_STATUS_INVALID_PARAM,
	HOST_TWT_RESUME_STATUS_DIALOG_ID_BUSY,
	HOST_TWT_RESUME_STATUS_NOT_PAUSED,
	HOST_TWT_RESUME_STATUS_NO_RESOURCE,
	HOST_TWT_RESUME_STATUS_NO_ACK,
	HOST_TWT_RESUME_STATUS_UNKNOWN_ERROR,
	HOST_TWT_RESUME_STATUS_CHAN_SW_IN_PROGRESS,
	HOST_TWT_RESUME_STATUS_ROAM_IN_PROGRESS,
	HOST_TWT_RESUME_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct twt_resume_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to HOST_TWT_RESUME_STATUS
 */
struct twt_resume_dialog_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t status;
};

/* enum HOST_TWT_NOTIFY_STATUS - status code of notify TWT event
 * @HOST_TWT_NOTIFY_EVENT_READY: FW is ready to process the TWT setup request.
 * @HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_SET: AP set the TWT required bit
 * @HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_CLEAR: AP cleared the TWT required bit
 */
enum HOST_TWT_NOTIFY_STATUS {
	HOST_TWT_NOTIFY_EVENT_READY,
	HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_SET,
	HOST_TWT_NOTIFY_EVENT_AP_TWT_REQ_BIT_CLEAR,
};

/**
 * struct twt_notify_event_param -
 * @vdev_id: VDEV identifier
 * @status: refer to HOST_TWT_NOTIFY_STATUS
 */
struct twt_notify_event_param {
	uint32_t vdev_id;
	enum HOST_TWT_NOTIFY_STATUS status;
};

#ifdef WLAN_SUPPORT_BCAST_TWT
/**
 * struct twt_btwt_invite_sta_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 */
struct twt_btwt_invite_sta_cmd_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
};

/* enum HOST_TWT_INVITATION_BTWT_STATUS - status code of TWT Invitation
 *                              dialog
 * HOST_TWT_INVITATION_BTWT_STATUS_OK: BTWT invitation successfully
 *                              completed
 * HOST_TWT_INVITATION_TWT_STATUS_DIALOG_ID_NOT_EXIST: BTWT dialog ID not
 *                              exists
 * HOST_TWT_INVITATION_BTWT_STATUS_INVALID_PARAM: invalid parameters
 * HOST_TWT_INVITATION_BTWT_STATUS_DIALOG_ID_BUSY: FW is in the process of
 *                              handling this dialog
 * HOST_TWT_INVITATION_BTWT_STATUS_NO_RESOURCE: FW resource exhausted
 * HOST_TWT_INVITATION_BTWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                              request/response frame
 * HOST_TWT_INVITATION_BTWT_STATUS_UNKNOWN_ERROR: BTWT invitation failed
 *                              with an unknown reason
 */
enum HOST_TWT_INVITATION_BTWT_STATUS {
	HOST_TWT_INVITATION_BTWT_STATUS_OK,
	HOST_TWT_INVITATION_BTWT_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_INVITATION_BTWT_STATUS_INVALID_PARAM,
	HOST_TWT_INVITATION_BTWT_STATUS_DIALOG_ID_BUSY,
	HOST_TWT_INVITATION_BTWT_STATUS_NO_RESOURCE,
	HOST_TWT_INVITATION_BTWT_STATUS_NO_ACK,
	HOST_TWT_INVITATION_BTWT_STATUS_UNKNOWN_ERROR,
};

/**
 * struct twt_btwt_invite_sta_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: BTWT dialog ID
 * @status: refer to HOST_TWT_INVITATION_BTWT_STATUS
 */
struct twt_btwt_invite_sta_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t status;
};

/**
 * struct twt_btwt_remove_sta_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: BTWT dialog ID
 */
struct twt_btwt_remove_sta_cmd_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
};

/* enum HOST_TWT_KICKOFF_BTWT_STATUS - status code of kickoff BTWT dialog
 * HOST_TWT_KICKOFF_BTWT_STATUS_OK: TWT kickoff successfully completed
 * HOST_TWT_KICKOFF_BTWT_STATUS_DIALOG_ID_NOT_EXIST: BTWT dialog ID not
 *                              exists
 * HOST_TWT_KICKOFF_BTWT_STATUS_INVALID_PARAM: invalid parameters
 * HOST_TWT_KICKOFF_BTWT_STATUS_DIALOG_ID_BUSY: FW is in the process of
 *                              handling this dialog
 * HOST_TWT_KICKOFF_BTWT_STATUS_NOT_PAUSED: Dialog not currently paused
 * HOST_TWT_KICKOFF_BTWT_STATUS_NO_RESOURCE: FW resource exhausted
 * HOST_TWT_KICKOFF_BTWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                              request/response frame
 * HOST_TWT_KICKOFF_BTWT_STATUS_UNKNOWN_ERROR: BTWT kickoff failed with an
 *                              unknown reason
 */
enum HOST_TWT_KICKOFF_BTWT_STATUS {
	HOST_TWT_KICKOFF_BTWT_STATUS_OK,
	HOST_TWT_KICKOFF_BTWT_STATUS_DIALOG_ID_NOT_EXIST,
	HOST_TWT_KICKOFF_BTWT_STATUS_INVALID_PARAM,
	HOST_TWT_KICKOFF_BTWT_STATUS_DIALOG_ID_BUSY,
	HOST_TWT_KICKOFF_BTWT_STATUS_NOT_PAUSED,
	HOST_TWT_KICKOFF_BTWT_STATUS_NO_RESOURCE,
	HOST_TWT_KICKOFF_BTWT_STATUS_NO_ACK,
	HOST_TWT_KICKOFF_BTWT_STATUS_UNKNOWN_ERROR,
};

/**
 * struct twt_btwt_remove_sta_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: BTWT dialog ID
 * @status: refer to HOST_TWT_KICKOFF_BTWT_STATUS
 */
struct twt_btwt_remove_sta_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t status;
};
#endif

/**
 * enum HOST_TWT_CMD_FOR_ACK_EVENT - Ack event for different TWT command
 * HOST_TWT_ADD_DIALOG_CMDID: Ack event for add dialog command
 * HOST_TWT_DEL_DIALOG_CMDID: Ack event for delete dialog command
 * HOST_TWT_PAUSE_DIALOG_CMDID: Ack event for pause command
 * HOST_TWT_RESUME_DIALOG_CMDID: Ack event for resume command
 * HOST_TWT_NUDGE_DIALOG_CMDID: Ack event for nudge command
 * HOST_TWT_UNKNOWN_CMDID: Ack event for unknown TWT command
 */
enum HOST_TWT_CMD_FOR_ACK_EVENT {
	HOST_TWT_ADD_DIALOG_CMDID = 0,
	HOST_TWT_DEL_DIALOG_CMDID,
	HOST_TWT_PAUSE_DIALOG_CMDID,
	HOST_TWT_RESUME_DIALOG_CMDID,
	HOST_TWT_NUDGE_DIALOG_CMDID,
	HOST_TWT_UNKNOWN_CMDID,
};

#endif /* _WLAN_TWT_PUBLIC_STRUCTS_H_ */


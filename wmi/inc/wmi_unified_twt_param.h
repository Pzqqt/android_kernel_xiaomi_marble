/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
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

/*
 * This file contains the API definitions for the TWT WMI APIs.
 */

#ifndef _WMI_UNIFIED_TWT_PARAM_H_
#define _WMI_UNIFIED_TWT_PARAM_H_

/* enum WMI_TWT_ROLE - role specified in ext conf in wmi_twt_enable/disable_cmd
 * WMI_TWT_ROLE_REQUESTOR: TWT role is requestor
 * WMI_TWT_ROLE_RESPONDER: TWT role is responder
 */
enum WMI_TWT_ROLE {
	WMI_TWT_ROLE_REQUESTOR,
	WMI_TWT_ROLE_RESPONDER,
};

/* enum WMI_TWT_OPERATION - specified in ext conf in wmi_twt_enable/disable_cmd
 * WMI_TWT_OPERATION_INDIVIDUAL: Individual TWT operation
 * WMI_TWT_OPERATION_BROADCAST: Broadcast TWT operation
 */
enum WMI_TWT_OPERATION {
	WMI_TWT_OPERATION_INDIVIDUAL,
	WMI_TWT_OPERATION_BROADCAST,
};

/**
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
 * @twt_role: values from enum WMI_TWT_ROLE.
 * @twt_oper: values from enum WMI_TWT_OPERATION.
 * @ext_conf_present: If requestor/responder extend config is present.
 * @b_twt_enable: Enable or disable broadcast TWT.
 * @b_twt_legacy_mbss_enable: Enable or disable legacy MBSSID TWT.
 * @b_twt_ax_mbss_enable: Enable or disable 11AX MBSSID TWT.
 */
struct wmi_twt_enable_param {
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
	enum WMI_TWT_ROLE twt_role;
	enum WMI_TWT_OPERATION twt_oper;
	bool ext_conf_present;
	uint32_t b_twt_enable:1,
		 b_twt_legacy_mbss_enable:1,
		 b_twt_ax_mbss_enable:1;
};

/* status code of enabling TWT
 * WMI_ENABLE_TWT_STATUS_OK: enabling TWT successfully completed
 * WMI_ENABLE_TWT_STATUS_ALREADY_ENABLED: TWT already enabled
 * WMI_ENABLE_TWT_STATUS_NOT_READY: FW not ready for enabling TWT
 * WMI_ENABLE_TWT_INVALID_PARAM: invalid parameters
 * WMI_ENABLE_TWT_STATUS_UNKNOWN_ERROR: enabling TWT failed with an
 *                                      unknown reason
 */
enum WMI_HOST_ENABLE_TWT_STATUS {
	WMI_HOST_ENABLE_TWT_STATUS_OK,
	WMI_HOST_ENABLE_TWT_STATUS_ALREADY_ENABLED,
	WMI_HOST_ENABLE_TWT_STATUS_NOT_READY,
	WMI_HOST_ENABLE_TWT_INVALID_PARAM,
	WMI_HOST_ENABLE_TWT_STATUS_UNKNOWN_ERROR,
};

/**
 * struct wmi_twt_enable_complete_event_param:
 * @pdev_is: pdev_id for identifying the MAC.
 * @status: From enum WMI_HOST_ENABLE_TWT_STATUS
 */
struct wmi_twt_enable_complete_event_param {
	uint32_t pdev_id;
	uint32_t status;
};

/**
 * struct wmi_twt_disable_param:
 * @pdev_id: pdev_id for identifying the MAC.
 * @ext_conf_present: If requestor/responder extend config is present.
 * @twt_role: values from enum WMI_TWT_ROLE.
 * @twt_oper: values from enum WMI_TWT_OPERATION.
 */
struct wmi_twt_disable_param {
	uint32_t pdev_id;
	bool ext_conf_present;
	enum WMI_TWT_ROLE twt_role;
	enum WMI_TWT_OPERATION twt_oper;
};

/**
 * struct wmi_twt_disable_complete_event:
 * @pdev_id: pdev_id for identifying the MAC.
 */
struct wmi_twt_disable_complete_event {
	uint32_t pdev_id;
};

/**
 * wmi_twt_ack_complete_event_param:
 * @vdev_id: vdev id
 * @peer_macaddr: peer mac address
 * @dialog_id: dialog id
 * @twt_cmd_ack: ack event to the corresponding twt command
 * @status: twt command status
 */
struct wmi_twt_ack_complete_event_param {
	uint32_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t twt_cmd_ack;
	uint32_t status;
};

/* TWT event types
 *  refer to wmi_unified.h enum wmi_twt_session_stats_type
 */
enum host_twt_session_stats_type {
	HOST_TWT_SESSION_SETUP     = 1,
	HOST_TWT_SESSION_TEARDOWN  = 2,
	HOST_TWT_SESSION_UPDATE    = 3,
};

/**
 * struct wmi_host_twt_session_stats_info:
 * @vdev_id: id of VDEV for twt session
 * @peer_mac: MAC address of node
 * @event_type: Indicates TWT session type (SETUP/TEARDOWN/UPDATE)
 * @flow_id: TWT flow identifier established with TWT peer
 * @bcast:  If this is a broacast TWT session
 * @trig: If the TWT session is trigger enabled
 * @announ: If the flow type is announced/unannounced
 * @protection: If the TWT protection field is set
 * @info_frame_disabled: If the TWT Information frame is disabled
 * @pm_responder_bit_valid: pm responder bit is valid or not
 * @pm_responder_bit: pm responder value
 * @dialog_id: Dialog_id of current session
 * @wake_dura_us: wake duration in us
 * @wake_intvl_us: wake time interval in us
 * @sp_offset_us: Time until initial TWT SP occurs
 * @sp_tsf_us_lo: TWT wake time TSF in usecs lower bits - 31:0
 * @sp_tsf_us_hi: TWT wake time TSF in usecs higher bits - 63:32
 */
struct wmi_host_twt_session_stats_info {
	uint32_t vdev_id;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
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
 * struct wmi_twt_session_stats_event:
 * @pdev_id: pdev_id for identifying the MAC.
 * @num_sessions: number of TWT sessions
 * @twt_sessions: received TWT sessions
 */
struct wmi_twt_session_stats_event_param {
	uint32_t pdev_id;
	uint32_t num_sessions;
};

/* from IEEE 802.11ah section 9.4.2.200 */
enum WMI_HOST_TWT_COMMAND {
	WMI_HOST_TWT_COMMAND_REQUEST_TWT    = 0,
	WMI_HOST_TWT_COMMAND_SUGGEST_TWT    = 1,
	WMI_HOST_TWT_COMMAND_DEMAND_TWT     = 2,
	WMI_HOST_TWT_COMMAND_TWT_GROUPING   = 3,
	WMI_HOST_TWT_COMMAND_ACCEPT_TWT     = 4,
	WMI_HOST_TWT_COMMAND_ALTERNATE_TWT  = 5,
	WMI_HOST_TWT_COMMAND_DICTATE_TWT    = 6,
	WMI_HOST_TWT_COMMAND_REJECT_TWT     = 7,
};

/**
 * struct wmi_twt_add_dialog_param -
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
 * @twt_cmd: cmd from enum WMI_HOST_TWT_COMMAND
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
struct wmi_twt_add_dialog_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t wake_intvl_us;
	uint32_t wake_intvl_mantis;
	uint32_t min_wake_intvl_us;
	uint32_t max_wake_intvl_us;
	uint32_t wake_dura_us;
	uint32_t min_wake_dura_us;
	uint32_t max_wake_dura_us;
	uint32_t sp_offset_us;
	enum WMI_HOST_TWT_COMMAND twt_cmd;
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

/* enum - status code of Get stats TWT dialog
 * @WMI_HOST_GET_STATS_TWT_STATUS_OK: Get status TWT dialog successfully completed
 * @WMI_HOST_GET_STATS_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID does not exist
 * @WMI_HOST_GET_STATS_TWT_STATUS_INVALID_PARAM: Invalid parameters
 * @WMI_HOST_GET_STATS_TWT_STATUS_UNKNOWN_ERROR: Unknown error
 */
enum WMI_HOST_GET_STATS_TWT_STATUS {
	WMI_HOST_GET_STATS_TWT_STATUS_OK,
	WMI_HOST_GET_STATS_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_GET_STATS_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_GET_STATS_TWT_STATUS_UNKNOWN_ERROR,
};

/* enum - status code of adding TWT dialog
 * @WMI_HOST_ADD_TWT_STATUS_OK: adding TWT dialog successfully completed
 * @WMI_HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED: TWT not enabled
 * @WMI_HOST_ADD_TWT_STATUS_USED_DIALOG_ID: TWT dialog ID is already used
 * @WMI_HOST_ADD_TWT_STATUS_INVALID_PARAM: invalid parameters
 * @WMI_HOST_ADD_TWT_STATUS_NOT_READY: FW not ready
 * @WMI_HOST_ADD_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @WMI_HOST_ADD_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @WMI_HOST_ADD_TWT_STATUS_NO_RESPONSE: peer AP did not send the response frame
 * @WMI_HOST_ADD_TWT_STATUS_DENIED: AP did not accept the request
 * @WMI_HOST_ADD_TWT_STATUS_UNKNOWN_ERROR: adding TWT dialog failed with
 * an unknown reason
 * @WMI_HOST_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE: peer AP wake interval,
 * duration not in range
 * @WMI_HOST_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED: peer AP IE Validation
 * Failed
 * @WMI_HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @WMI_HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @WMI_HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum WMI_HOST_ADD_TWT_STATUS {
	WMI_HOST_ADD_TWT_STATUS_OK,
	WMI_HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED,
	WMI_HOST_ADD_TWT_STATUS_USED_DIALOG_ID,
	WMI_HOST_ADD_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_ADD_TWT_STATUS_NOT_READY,
	WMI_HOST_ADD_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_ADD_TWT_STATUS_NO_ACK,
	WMI_HOST_ADD_TWT_STATUS_NO_RESPONSE,
	WMI_HOST_ADD_TWT_STATUS_DENIED,
	WMI_HOST_ADD_TWT_STATUS_UNKNOWN_ERROR,
	WMI_HOST_ADD_TWT_STATUS_AP_PARAMS_NOT_IN_RANGE,
	WMI_HOST_ADD_TWT_STATUS_AP_IE_VALIDATION_FAILED,
	WMI_HOST_ADD_TWT_STATUS_ROAM_IN_PROGRESS,
	WMI_HOST_ADD_TWT_STATUS_CHAN_SW_IN_PROGRESS,
	WMI_HOST_ADD_TWT_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct wmi_twt_add_dialog_additional_params -
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
struct wmi_twt_add_dialog_additional_params {
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
 * struct wmi_twt_add_dialog_complete_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to WMI_HOST_ADD_TWT_STATUS enum
 * @num_additional_twt_params: no of additional_twt_params available
 */
struct wmi_twt_add_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t status;
	uint32_t num_additional_twt_params;
};

/**
 * struct wmi_twt_cap_bitmap_params -
 * twt_cap_bitmap: TWT capability bitmap
 *
 */
struct wmi_twt_cap_bitmap_params {
	uint32_t twt_ack_support_cap:1;
};

/**
 * struct wmi_twt_del_dialog_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @b_twt_persistence: persistence val for b-twt
 */
struct wmi_twt_del_dialog_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
#ifdef WLAN_SUPPORT_BCAST_TWT
	uint32_t b_twt_persistence;
#endif
};

/**
 * enum WMI_HOST_DEL_TWT_STATUS - status code of deleting TWT dialog
 * @WMI_HOST_DEL_TWT_STATUS_OK: deleting TWT dialog successfully completed
 * @WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @WMI_HOST_DEL_TWT_STATUS_INVALID_PARAM: invalid parameters
 * @WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @WMI_HOST_DEL_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @WMI_HOST_DEL_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the request/response
 * frame
 * @WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR: deleting TWT dialog failed with an
 * unknown reason
 * @WMI_HOST_DEL_TWT_STATUS_PEER_INIT_TEARDOWN: Peer initiated TWT teardown
 * @WMI_HOST_DEL_TWT_STATUS_ROAMING: TWT teardown due to roaming.
 * @WMI_HOST_DEL_TWT_STATUS_CONCURRENCY: TWT session teardown due to
 * concurrent session comming up.
 * @WMI_HOST_DEL_TWT_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @WMI_HOST_DEL_TWT_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 * @WMI_HOST_DEL_TWT_STATUS_PS_DISABLE_TEARDOWN: PS disable TWT teardown
 */
enum WMI_HOST_DEL_TWT_STATUS {
	WMI_HOST_DEL_TWT_STATUS_OK,
	WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_DEL_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_DEL_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_DEL_TWT_STATUS_NO_ACK,
	WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR,
	WMI_HOST_DEL_TWT_STATUS_PEER_INIT_TEARDOWN,
	WMI_HOST_DEL_TWT_STATUS_ROAMING,
	WMI_HOST_DEL_TWT_STATUS_CONCURRENCY,
	WMI_HOST_DEL_TWT_STATUS_CHAN_SW_IN_PROGRESS,
	WMI_HOST_DEL_TWT_STATUS_SCAN_IN_PROGRESS,
	WMI_HOST_DEL_TWT_STATUS_PS_DISABLE_TEARDOWN,
};

/**
 * struct wmi_twt_del_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @b_twt_persistence: persistence val for b-twt
 * @status: refer to WMI_HOST_DEL_TWT_STATUS enum
 */
struct wmi_twt_del_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
#ifdef WLAN_SUPPORT_BCAST_TWT
	uint32_t b_twt_persistence;
#endif
	uint32_t status;
};

/**
 * struct wmi_twt_pause_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 */
struct wmi_twt_pause_dialog_cmd_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
};

/**
 * struct wmi_twt_nudge_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @suspend_duration: TWT suspend duration in microseconds
 * @next_twt_size: next TWT size
 */
struct wmi_twt_nudge_dialog_cmd_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t suspend_duration;
	uint32_t next_twt_size;
};

/* enum WMI_HOST_PAUSE_TWT_STATUS - status code of pausing TWT dialog
 * @WMI_HOST_PAUSE_TWT_STATUS_OK: pausing TWT dialog successfully completed
 * @WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @WMI_HOST_PAUSE_TWT_STATUS_INVALID_PARAM: invalid parameters
 * @WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @WMI_HOST_PAUSE_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @WMI_HOST_PAUSE_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR: pausing TWT dialog failed with an
 * unknown reason
 * @WMI_HOST_PAUSE_TWT_STATUS_ALREADY_PAUSED: TWT dialog already in paused state
 * @WMI_HOST_PAUSE_TWT_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @WMI_HOST_PAUSE_TWT_STATUS_ROAM_IN_PROGRESS: Roaming is in progress
 * @WMI_HOST_PAUSE_TWT_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum WMI_HOST_PAUSE_TWT_STATUS {
	WMI_HOST_PAUSE_TWT_STATUS_OK,
	WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_PAUSE_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_PAUSE_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_PAUSE_TWT_STATUS_NO_ACK,
	WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR,
	WMI_HOST_PAUSE_TWT_STATUS_ALREADY_PAUSED,
	WMI_HOST_PAUSE_TWT_STATUS_CHAN_SW_IN_PROGRESS,
	WMI_HOST_PAUSE_TWT_STATUS_ROAM_IN_PROGRESS,
	WMI_HOST_PAUSE_TWT_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct wmi_twt_pause_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to WMI_HOST_PAUSE_TWT_STATUS
 */
struct wmi_twt_pause_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	enum WMI_HOST_PAUSE_TWT_STATUS status;
};

/* enum WMI_HOST_NUDGE_TWT_STATUS - status code of nudge TWT dialog
 * @WMI_HOST_NUDGE_TWT_STATUS_OK: nudge TWT dialog successfully completed
 * @WMI_HOST_NUDGE_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @WMI_HOST_NUDGE_TWT_STATUS_INVALID_PARAM: invalid parameters
 * @WMI_HOST_NUDGE_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @WMI_HOST_NUDGE_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @WMI_HOST_NUDGE_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @WMI_HOST_NUDGE_TWT_STATUS_UNKNOWN_ERROR: nudge TWT dialog failed with an
 * unknown reason
 * @WMI_HOST_NUDGE_TWT_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @WMI_HOST_NUDGE_TWT_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @WMI_HOST_NUDGE_TWT_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum WMI_HOST_NUDGE_TWT_STATUS {
	WMI_HOST_NUDGE_TWT_STATUS_OK,
	WMI_HOST_NUDGE_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_NUDGE_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_NUDGE_TWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_NUDGE_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_NUDGE_TWT_STATUS_NO_ACK,
	WMI_HOST_NUDGE_TWT_STATUS_UNKNOWN_ERROR,
	WMI_HOST_NUDGE_TWT_STATUS_CHAN_SW_IN_PROGRESS,
	WMI_HOST_NUDGE_TWT_STATUS_ROAM_IN_PROGRESS,
	WMI_HOST_NUDGE_TWT_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct wmi_twt_nudge_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to WMI_HOST_PAUSE_TWT_STATUS
 * @next_twt_tsf_us_lo: TSF lower bits (31:0) of next wake time
 * @next_twt_tsf_us_hi: TSF higher bits (32:63) of next wake time
 */
struct wmi_twt_nudge_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	enum WMI_HOST_NUDGE_TWT_STATUS status;
	uint32_t next_twt_tsf_us_lo;
	uint32_t next_twt_tsf_us_hi;
};

/**
 * struct wmi_twt_resume_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @sp_offset_us: this long time after TWT resumed the 1st SP will start
 * @next_twt_size: Next TWT subfield Size.
 *                 Refer IEEE 802.11ax section "9.4.1.60 TWT Information field"
 */
struct wmi_twt_resume_dialog_cmd_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t sp_offset_us;
	uint32_t next_twt_size;
};

/* enum WMI_HOST_RESUME_TWT_STATUS - status code of resuming TWT dialog
 * @WMI_HOST_RESUME_TWT_STATUS_OK: resuming TWT dialog successfully completed
 * @WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * @WMI_HOST_RESUME_TWT_STATUS_INVALID_PARAM: invalid parameters
 * @WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 * this dialog
 * @WMI_HOST_RESUME_TWT_STATUS_NOT_PAUSED: dialog not paused currently
 * @WMI_HOST_RESUME_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * @WMI_HOST_RESUME_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 * request/response frame
 * @WMI_HOST_RESUME_TWT_STATUS_UNKNOWN_ERROR: resuming TWT dialog failed with an
 * unknown reason
 * @WMI_HOST_RESUME_TWT_STATUS_CHAN_SW_IN_PROGRESS: Channel switch in progress
 * @WMI_HOST_RESUME_TWT_STATUS_ROAM_IN_PROGRESS: Roaming in progress
 * @WMI_HOST_RESUME_TWT_STATUS_SCAN_IN_PROGRESS: Scan is in progress
 */
enum WMI_HOST_RESUME_TWT_STATUS {
	WMI_HOST_RESUME_TWT_STATUS_OK,
	WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_RESUME_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_RESUME_TWT_STATUS_NOT_PAUSED,
	WMI_HOST_RESUME_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_RESUME_TWT_STATUS_NO_ACK,
	WMI_HOST_RESUME_TWT_STATUS_UNKNOWN_ERROR,
	WMI_HOST_RESUME_TWT_STATUS_CHAN_SW_IN_PROGRESS,
	WMI_HOST_RESUME_TWT_STATUS_ROAM_IN_PROGRESS,
	WMI_HOST_RESUME_TWT_STATUS_SCAN_IN_PROGRESS,
};

/**
 * struct wmi_twt_resume_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to WMI_HOST_RESUME_TWT_STATUS
 */
struct wmi_twt_resume_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t status;
};

/**
 * struct wmi_twt_notify_event_param -
 * @vdev_id: VDEV identifier
 */
struct wmi_twt_notify_event_param {
	uint32_t vdev_id;
};

#ifdef WLAN_SUPPORT_BCAST_TWT
/**
 * struct wmi_twt_btwt_invite_sta_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 */
struct wmi_twt_btwt_invite_sta_cmd_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
};

/* enum WMI_HOST_INVITATION_TWT_BTWT_STATUS - status code of TWT Invitation
 *                              dialog
 * WMI_HOST_INVITATION_TWT_BTWT_STATUS_OK: BTWT invitation successfully
 *                              completed
 * WMI_HOST_INVITATION_TWT_TWT_STATUS_DIALOG_ID_NOT_EXIST: BTWT dialog ID not
 *                              exists
 * WMI_HOST_INVITATION_TWT_BTWT_STATUS_INVALID_PARAM: invalid parameters
 * WMI_HOST_INVITATION_TWT_BTWT_STATUS_DIALOG_ID_BUSY: FW is in the process of
 *                              handling this dialog
 * WMI_HOST_INVITATION_TWT_BTWT_STATUS_NO_RESOURCE: FW resource exhausted
 * WMI_HOST_INVITATION_TWT_BTWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                              request/response frame
 * WMI_HOST_INVITATION_TWT_BTWT_STATUS_UNKNOWN_ERROR: BTWT invitation failed
 *                              with an unknown reason
 */
enum WMI_HOST_INVITATION_TWT_BTWT_STATUS {
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_OK,
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_INVALID_PARAM,
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_NO_RESOURCE,
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_NO_ACK,
	WMI_HOST_INVITATION_TWT_BTWT_STATUS_UNKNOWN_ERROR,
};

/**
 * struct wmi_twt_btwt_invite_sta_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: BTWT dialog ID
 * @status: refer to WMI_HOST_INVITATION_TWT_BTWT_STATUS
 */
struct wmi_twt_btwt_invite_sta_complete_event_param {
		uint32_t vdev_id;
		uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
		uint32_t dialog_id;
		uint32_t status;
};

/**
 * struct wmi_twt_btwt_remove_sta_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: BTWT dialog ID
 */
struct wmi_twt_btwt_remove_sta_cmd_param {
		uint32_t vdev_id;
		uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
		uint32_t dialog_id;
};

/* enum WMI_HOST_KICKOFF_TWT_BTWT_STATUS - status code of resuming TWT dialog
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_OK: TWT kickoff successfully completed
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_DIALOG_ID_NOT_EXIST: BTWT dialog ID not
 *                              exists
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_INVALID_PARAM: invalid parameters
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_DIALOG_ID_BUSY: FW is in the process of
 *                              handling this dialog
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_NOT_PAUSED: Dialog not currently paused
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_NO_RESOURCE: FW resource exhausted
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                              request/response frame
 * WMI_HOST_KICKOFF_TWT_BTWT_STATUS_UNKNOWN_ERROR: BTWT kickoff failed with an
 *                              unknown reason
 */
enum WMI_HOST_KICKOFF_TWT_BTWT_STATUS {
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_OK,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_INVALID_PARAM,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_NOT_PAUSED,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_NO_RESOURCE,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_NO_ACK,
	WMI_HOST_KICKOFF_TWT_BTWT_STATUS_UNKNOWN_ERROR,
};

/**
 * struct wmi_twt_btwt_remove_sta_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: BTWT dialog ID
 * @status: refer to WMI_HOST_KICKOFF_TWT_BTWT_STATUS
 */
struct wmi_twt_btwt_remove_sta_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t status;
};
#endif

/**
 * enum WMI_HOST_TWT_CMD_FOR_ACK_EVENT - Ack event for different TWT command
 * WMI_HOST_TWT_ADD_DIALOG_CMDID: Ack event for add dialog command
 * WMI_HOST_TWT_DEL_DIALOG_CMDID: Ack event for delete dialog command
 * WMI_HOST_TWT_PAUSE_DIALOG_CMDID: Ack event for pause command
 * WMI_HOST_TWT_RESUME_DIALOG_CMDID: Ack event for resume command
 * WMI_HOST_TWT_NUDGE_DIALOG_CMDID: Ack event for nudge command
 * WMI_HOST_TWT_UNKNOWN_CMDID: Ack event for unknown TWT command
 */
enum WMI_HOST_TWT_CMD_FOR_ACK_EVENT {
	WMI_HOST_TWT_ADD_DIALOG_CMDID = 0,
	WMI_HOST_TWT_DEL_DIALOG_CMDID,
	WMI_HOST_TWT_PAUSE_DIALOG_CMDID,
	WMI_HOST_TWT_RESUME_DIALOG_CMDID,
	WMI_HOST_TWT_NUDGE_DIALOG_CMDID,
	WMI_HOST_TWT_UNKNOWN_CMDID,
};
#endif /* _WMI_UNIFIED_TWT_PARAM_H_ */

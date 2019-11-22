/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * @flags: Flag to enable or disable capabilities, example bcast twt.
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
	uint32_t flags;
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

/** struct wmi_twt_enable_complete_event_param:
 * @pdev_is: pdev_id for identifying the MAC.
 * @status: From enum WMI_HOST_ENABLE_TWT_STATUS
 */
struct wmi_twt_enable_complete_event_param {
	uint32_t pdev_id;
	uint32_t status;
};

/** struct wmi_twt_disable_param:
 * @pdev_id: pdev_id for identifying the MAC.
 */
struct wmi_twt_disable_param {
	uint32_t pdev_id;
};

/** struct wmi_twt_disable_complete_event:
 * @pdev_id: pdev_id for identifying the MAC.
 */
struct wmi_twt_disable_complete_event {
	uint32_t pdev_id;
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

/** struct wmi_twt_add_dialog_param -
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
 * @wake_dura_us: TWT Wake Duration in units of us, must be <= 0xFFFF
 *                wake_dura_us must be divided evenly by 256,
 *                i.e., wake_dura_us % 256 == 0
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
 */
struct wmi_twt_add_dialog_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t wake_intvl_us;
	uint32_t wake_intvl_mantis;
	uint32_t wake_dura_us;
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
};

/* enum - status code of adding TWT dialog
 * WMI_HOST_ADD_TWT_STATUS_OK: adding TWT dialog successfully completed
 * WMI_HOST_ADD_TWT_STATUS_TWT_NOT_ENABLED: TWT not enabled
 * WMI_HOST_ADD_TWT_STATUS_USED_DIALOG_ID: TWT dialog ID is already used
 * WMI_HOST_ADD_TWT_STATUS_INVALID_PARAM: invalid parameters
 * WMI_HOST_ADD_TWT_STATUS_NOT_READY: FW not ready
 * WMI_HOST_ADD_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * WMI_HOST_ADD_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                                 request/response frame
 * WMI_HOST_ADD_TWT_STATUS_NO_RESPONSE: peer AP did not send the response frame
 * WMI_HOST_ADD_TWT_STATUS_DENIED: AP did not accept the request
 * WMI_HOST_ADD_TWT_STATUS_UNKNOWN_ERROR: adding TWT dialog failed with
 *                                 an unknown reason
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
};

/** struct wmi_twt_add_dialog_complete_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to WMI_HOST_ADD_TWT_STATUS enum
 */
struct wmi_twt_add_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t status;
};

/** struct wmi_twt_del_dialog_param -
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

/* status code of deleting TWT dialog
 * WMI_HOST_DEL_TWT_STATUS_OK: deleting TWT dialog successfully completed
 * WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * WMI_HOST_DEL_TWT_STATUS_INVALID_PARAM: invalid parameters
 * WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 *                                    this dialog
 * WMI_HOST_DEL_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * WMI_HOST_DEL_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the request/response
 *                            frame
 * WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR: deleting TWT dialog failed with an
 *                            unknown reason
 */
enum WMI_HOST_DEL_TWT_STATUS {
	WMI_HOST_DEL_TWT_STATUS_OK,
	WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_DEL_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_DEL_TWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_DEL_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_DEL_TWT_STATUS_NO_ACK,
	WMI_HOST_DEL_TWT_STATUS_UNKNOWN_ERROR,
};

/** struct wmi_twt_del_dialog_complete_event_param -
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

/** struct wmi_twt_pause_dialog_cmd_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 */
struct wmi_twt_pause_dialog_cmd_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
};

/* enum WMI_HOST_PAUSE_TWT_STATUS - status code of pausing TWT dialog
 * WMI_HOST_PAUSE_TWT_STATUS_OK: pausing TWT dialog successfully completed
 * WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * WMI_HOST_PAUSE_TWT_STATUS_INVALID_PARAM: invalid parameters
 * WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 *                          this dialog
 * WMI_HOST_PAUSE_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * WMI_HOST_PAUSE_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                          request/response frame
 * WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR: pausing TWT dialog failed with an
 *                          unknown reason
 */
enum WMI_HOST_PAUSE_TWT_STATUS {
	WMI_HOST_PAUSE_TWT_STATUS_OK,
	WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_NOT_EXIST,
	WMI_HOST_PAUSE_TWT_STATUS_INVALID_PARAM,
	WMI_HOST_PAUSE_TWT_STATUS_DIALOG_ID_BUSY,
	WMI_HOST_PAUSE_TWT_STATUS_NO_RESOURCE,
	WMI_HOST_PAUSE_TWT_STATUS_NO_ACK,
	WMI_HOST_PAUSE_TWT_STATUS_UNKNOWN_ERROR,
};

/** struct wmi_twt_pause_dialog_complete_event_param -
 * @vdev_id: VDEV identifier
 * @peer_macaddr: Peer mac address
 * @dialog_id: TWT dialog ID
 * @status: refer to WMI_HOST_PAUSE_TWT_STATUS
 */
struct wmi_twt_pause_dialog_complete_event_param {
	uint32_t vdev_id;
	uint8_t  peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t dialog_id;
	uint32_t status;
};

/** struct wmi_twt_resume_dialog_cmd_param -
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
 * WMI_HOST_RESUME_TWT_STATUS_OK: resuming TWT dialog successfully completed
 * WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_NOT_EXIST: TWT dialog ID not exists
 * WMI_HOST_RESUME_TWT_STATUS_INVALID_PARAM: invalid parameters
 * WMI_HOST_RESUME_TWT_STATUS_DIALOG_ID_BUSY: FW is in the process of handling
 *                            this dialog
 * WMI_HOST_RESUME_TWT_STATUS_NOT_PAUSED: dialog not paused currently
 * WMI_HOST_RESUME_TWT_STATUS_NO_RESOURCE: FW resource exhausted
 * WMI_HOST_RESUME_TWT_STATUS_NO_ACK: peer AP/STA did not ACK the
 *                            request/response frame
 * WMI_HOST_RESUME_TWT_STATUS_UNKNOWN_ERROR: resuming TWT dialog failed with an
 *                            unknown reason
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
};

/** struct wmi_twt_resume_dialog_complete_event_param -
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

#ifdef WLAN_SUPPORT_BCAST_TWT
/** struct wmi_twt_btwt_invite_sta_cmd_param -
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

/** struct wmi_twt_btwt_invite_sta_complete_event_param -
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

/** struct wmi_twt_btwt_remove_sta_cmd_param -
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

/** struct wmi_twt_btwt_remove_sta_complete_event_param -
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

#endif /* _WMI_UNIFIED_TWT_PARAM_H_ */

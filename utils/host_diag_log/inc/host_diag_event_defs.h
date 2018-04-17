/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

#ifndef EVENT_DEFS_H
#define EVENT_DEFS_H

typedef enum {
	EVENT_DROP_ID = 0,

	/* Events between 0x1 to 0x674 are not used */

	/*
	 * <diag_event>
	 * EVENT_WLAN_SECURITY
	 * @ eventId: Event id
	 * @ authMode: Shows the auth mode
	 * @ encryptionModeUnicast: Encryption Mode Unicast
	 * @ encryptionModeMulticast: Encryption Mode Multicast
	 * @ pmkIDMatch: PMK ID Match
	 * @ bssid: BSSID < 0 to 5 >
	 * @ keyId: Key ID
	 * @ status: Shows the status 0 is Success and 1 is failure
	 *
	 * This event is used in SECURITY to send various wlan security modes
	 * Values for parameters are defined below:
	 * Event ID: offset: 0 length: 1
	 * 5  - Remove Key Req
	 * 6  - Remove Key Rsp
	 * 7  - PMKID Candidate Found
	 * 8  - PMKID Update
	 * 9  - Mic Error
	 * 10 - Set UniCast Key Req
	 * 11 - Set UniCast Key Rsp
	 * 12 - Set BCast Key Req
	 * 13 - Set BCast Key Rsp
	 *
	 * Auth Mode: offset: 1 length: 1
	 * 0 - Open
	 * 1 - Shared
	 * 2 - WPA EAP
	 * 3 - WPA PSK
	 * 4 - WPA2 EAP
	 * 5 - WPA2 PSK
	 *
	 * Encryption Mode Unicast: offset: 2 length: 1
	 * 0 - Open
	 * 1 - WEP40
	 * 2 - WEP104
	 * 3 - TKIP
	 * 4 - AES
	 *
	 * Encryption Mode Multicast: offset: 3 length: 1
	 * 0 - Open
	 * 1 - WEP40
	 * 2 - WEP104
	 * 3 - TKIP
	 * 4 - AES
	 *
	 * ENC_MODE_SMS4: offset: 4 length:1
	 *
	 * PMK ID Match: offset: 5 length: 1
	 * 0 - No Match
	 * 1 - Match
	 *
	 * BSSID[0]: offset: 6 length: 1
	 * BSSID[1]: offset: 7 length: 1
	 * BSSID[2]: offset: 8 length: 1
	 * BSSID[3]: offset: 9 length: 1
	 * BSSID[4]: offset: 10 length: 1
	 * BSSID[5]: offset: 11 length: 1
	 *
	 * Key ID: offset: 12 length: 1
	 * Status: offset: 13 length: 1
	 * 0 - Success
	 * 1 - Failure
	 *
	 * Supported Feature: wlan security
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_SECURITY = 0x675, /* 13 byte payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_STATUS
	 * @ eventId: Event id
	 * @ ssid: SSID
	 * @ bssType: BSS Type
	 * @ rssi: RSSI
	 * @ channel: Channel
	 * @ qosCapability: QoS Capability
	 * @ authmode: Auth Mode
	 * @ encryptionType: Encryption Type
	 * @ reason: Reason
	 * @ reasonDisconnect: Reason Disconnect
	 *
	 * This event is used to send wlan status
	 * Values for parameters are defined below:
	 * Event ID: offset: 0 length: 1
	 * 0 - Connect
	 * 1 - Disconnect
	 *
	 * SSID: offset: 1 length: 32
	 * BSS Type: offset: 33 + SSID length, length: 1
	 * 0 - None
	 * 1 - BSS_TYPE_INFRASTRUCTURE
	 * 2 - BSS_TYPE_INFRA_AP
	 * 3 - BSS_TYPE_IBSS
	 * 4 - BSS_TYPE_START_IBSS
	 * 5 - BSS_TYPE_NDI
	 * 6 - BSS_TYPE_ANY
	 *
	 * RSSI: offset: 34 length: 1
	 * Channel: offset: 35 length: 1
	 * QoS Capability: offset: 36 length: 1
	 * Auth Mode: offset: 37 length: 1
	 * 0 - Open
	 * 1 - Shared
	 * 2 - WPA EAP
	 * 3 - WPA PSK
	 * 4 - WPA2 EAP
	 * 5 - WPA2 PSK
	 * 6 - WAPI CERT
	 * 7 - WAPI PSK
	 *
	 * Encryption Type: offset: 38 length: 1
	 * 0 - Open
	 * 1 - WEP40
	 * 2 - WEP104
	 * 3 - TKIP
	 * 4 - AES
	 *
	 * ENC_MODE_SMS4: offset: 39 length: 1
	 *
	 * Reason: offset: 40 length: 1
	 * 0 - Unspecified
	 * 1 - User Requested
	 * 2 - Mic Error
	 * 3 - Diassoc
	 * 4 - Deauth
	 * 5 - Handoff
	 *
	 * Reason Disconnect: offset: 41 length: 1
	 *
	 * Supported Feature: wlan status
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_STATUS, /* 15 byte payload */

	/* Events 0x677 and 0x678 are not used */

	/*
	 * <diag_event>
	 * EVENT_WLAN_QOS
	 * @ eventId: event id
	 * @ reasonCode: Reason for event
	 *
	 * This event is used to send quality set services
	 * Values for parameters are defined below:
	 * Event ID: offset: 0 length: 1
	 * 0 - Add TS Req
	 * 1 - Add TS Rsp
	 * 2 - Delts
	 *
	 * Reason Code: offset: 1 length: 1
	 * 0 - Admission Accepted
	 * 1 - Invalid Params
	 * 2 - Reserved
	 * 3 - Refused
	 * 4 - User Requested
	 * 5 - Ind From AP
	 *
	 * Supported Feature: Qos wlan
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_QOS = 0x679, /* 2 byte payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_PE
	 * @bssid: BSSID
	 * @ event_type: Event type
	 * @ sme_state: SME state
	 * @ mlm_state: MLM state
	 * @ status: 0 - Success, 1 - Failure < majority 0 is success >
	 * @reason_code: reason for event report
	 *
	 * This event is used in PE to send different diag events.
	 * Values for parameters are defined below:
	 *
	 * bssid[0]: offset: 0 length: 1
	 * bssid[1]: offset: 1 length: 1
	 * bssid[2]: offset: 2 length: 1
	 * bssid[3]: offset: 3 length: 1
	 * bssid[4]: offset: 4 length: 1
	 * bssid[5]: offset: 5 length: 1
	 *
	 * Event Type: offset: 6 length: 2
	 * 0  - SCAN REQ EVENT
	 * 1  - SCAN ABORT IND EVENT
	 * 2  - SCAN_RSP_EVENT
	 * 3  - JOIN_REQ_EVENT
	 * 4  - JOIN_RSP_EVENT
	 * 5  - SETCONTEXT_REQ_EVENT
	 * 6  - SETCONTEXT_RSP_EVENT
	 * 7  - REASSOC_REQ_EVENT
	 * 8  - REASSOC_RSP_EVENT
	 * 9  - AUTH_REQ_EVENT
	 * 10 - AUTH_RSP_EVENT
	 * 11 - DISASSOC_REQ_EVENT
	 * 12 - DISASSOC_RSP_EVENT
	 * 13 - DISASSOC_IND_EVENT
	 * 14 - DISASSOC_CNF_EVENT
	 * 15 - DEAUTH_REQ_EVENT
	 * 16 - DEAUTH_RSP_EVENT
	 * 17 - DEAUTH_IND_EVENT
	 * 18 - START_BSS_REQ_EVENT
	 * 19 - START_BSS_RSP_EVENT
	 * 20 - AUTH_IND_EVENT
	 * 21 - ASSOC_IND_EVENT
	 * 22 - ASSOC_CNF_EVENT
	 * 23 - REASSOC_IND_EVENT
	 * 24 - SWITCH_CHL_IND_EVENT
	 * 25 - SWITCH_CHL_RSP_EVENT
	 * 26 - STOP_BSS_REQ_EVENT
	 * 27 - STOP_BSS_RSP_EVENT
	 * 28 - DEAUTH_CNF_EVENT
	 * 29 - ADDTS_REQ_EVENT
	 * 30 - ADDTS_RSP_EVENT
	 * 31 - DELTS_REQ_EVENT
	 * 32 - DELTS_RSP_EVENT
	 * 33 - DELTS_IND_EVENT
	 * 34 - ENTER_BMPS_REQ_EVENT
	 * 35 - ENTER_BMPS_RSP_EVENT
	 * 36 - EXIT_BMPS_REQ_EVENT
	 * 37 - BMPS_RSP_EVENT
	 * 38 - EXIT_BMPS_IND_EVENT
	 * 39 - ENTER_IMPS_REQ_EVENT
	 * 40 - ENTER_IMPS_RSP_EVENT
	 * 41 - EXIT_IMPS_REQ_EVENT
	 * 42 - EXIT_IMPS_RSP_EVENT
	 * 43 - ENTER_UAPSD_REQ_EVENT
	 * 44 - ENTER_UAPSD_RSP_EVENT
	 * 45 - EXIT_UAPSD_REQ_EVENT
	 * 46 - EXIT_UAPSD_RSP_EVENT
	 * 47 - WOWL_ADD_BCAST_PTRN_EVENT
	 * 48 - WOWL_DEL_BCAST_PTRN_EVENT
	 * 49 - ENTER_WOWL_REQ_EVENT
	 * 50 - ENTER_WOWL_RSP_EVENT
	 * 51 - EXIT_WOWL_REQ_EVENT
	 * 52 - EXIT_WOWL_RSP_EVENT
	 * 53 - HAL_ADDBA_REQ_EVENT
	 * 54 - HAL_ADDBA_RSP_EVENT
	 * 55 - HAL_DELBA_IND_EVENT
	 * 56 - HB_FAILURE_TIMEOUT
	 * 57 - PRE_AUTH_REQ_EVENT
	 * 58 - PRE_AUTH_RSP_EVENT
	 * 59 - PREAUTH_DONE
	 * 60 - REASSOCIATING
	 * 61 - CONNECTED
	 * 62 - ASSOC_REQ_EVENT
	 * 63 - AUTH_COMP_EVENT
	 * 64 - ASSOC_COMP_EVENT
	 * 65 - AUTH_START_EVENT
	 * 66 - ASSOC_START_EVENT
	 * 67 - REASSOC_START_EVENT
	 * 68 - ROAM_AUTH_START_EVENT
	 * 69 - ROAM_AUTH_COMP_EVENT
	 * 70 - ROAM_ASSOC_START_EVENT
	 * 71 - ROAM_ASSOC_COMP_EVENT
	 * 72 - SCAN_COMPLETE_EVENT
	 * 73 - SCAN_RESULT_FOUND_EVENT
	 * 74 - ASSOC_TIMEOUT
	 * 75 - AUTH_TIMEOUT
	 * 76 - DEAUTH_FRAME_EVENT
	 * 77 - DISASSOC_FRAME_EVENT
	 *
	 * SME State: offset: 8 length: 2
	 * 0  - OFFLINE
	 * 1  - IDLE
	 * 2  - SUSPEND
	 * 3  - WT SCAN
	 * 4  - WT JOIN
	 * 5  - WT AUTH
	 * 6  - WT ASSOC
	 * 7  - WT REASSOC
	 * 8  - WT REASSOC LINK FAIL
	 * 9  - JOIN FAILURE
	 * 10 - ASSOCIATED
	 * 11 - REASSOCIATED
	 * 12 - LINK EST
	 * 13 - LINK EST WT SCAN
	 * 14 - WT PRE AUTH
	 * 15 - WT DISASSOC
	 * 16 - WT DEAUTH
	 * 17 - WT START BSS
	 * 18 - WT STOP BSS
	 * 19 - NORMAL
	 * 20 - CHANNEL SCAN
	 * 21 - NORMAL CHANNEL SCAN
	 *
	 * MLM State: offset: 10 legth: 2
	 * 0  - MLM OFFLINE
	 * 1  - MLM IDLE
	 * 2  - MLM WT PROBE RESP
	 * 3  - MLM PASSIVE SCAN
	 * 4  - MLM WT JOIN BEACON
	 * 5  - MLM JOINED
	 * 6  - MLM BSS STARTED
	 * 7  - MLM WT AUTH FRAME
	 * 8  - MLM WT AUTH FRAME
	 * 9  - MLM WT AUTH FRAME
	 * 10 - MLM AUTH RSP TIMEOUT
	 * 11 - MLM AUTHENTICATED
	 * 12 - MLM WT ASSOC RSP
	 * 13 - MLM WT REASSOC RSP
	 * 14 - MLM ASSOCIATED
	 * 15 - MLM REASSOCIATED
	 * 16 - MLM LINK ESTABLISHED
	 * 17 - MLM WT ASSOC CNF
	 * 18 - MLM LEARN
	 * 19 - MLM WT ADD BSS RSP
	 * 20 - MLM WT DEL BSS RSP
	 * 21 - MLM WT ADD BSS RSP ASSOC
	 * 22 - MLM WT ADD BSS RSP REASSOC
	 * 23 - MLM WT ADD BSS RSP PREASSOC
	 * 24 - MLM WT ADD STA RSP
	 * 25 - MLM WT DEL STA RSP
	 * 26 - MLM WT ASSOC DEL STA RSP
	 * 27 - MLM WT SET BSS KEY
	 * 28 - MLM WT SET STA KEY
	 * 29 - MLM WT SET STA BCASTKEY
	 * 30 - MLM WT ADDBA RSP
	 * 31 - MLM WT REMOVE BSS KEY
	 * 32 - MLM WT REMOVE STA KEY
	 * 33 - MLM WT SET MIMOPS
	 *
	 * Status: offset: 12 length: 2
	 * Reason Code: offset: 14 length: 2
	 *
	 * Supported Feature: STA
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_PE, /* 16 byte payload */

	/* Events between 0x67b to 0x67f are not used */

	/*
	 * <diag_event>
	 * EVENT_WLAN_BRINGUP_STATUS
	 * @ wlanStatus: Describe wlan status
	 * @ driverVersion: Driver version between 0 to 9
	 *
	 * This event is used in BRINGUP to send wlan status
	 * Values for parameters are defined below:
	 * WLAN Status: offset: 0 length: 2
	 * 0 - WLAN Disabled
	 * 1 - WLAN Enabled
	 * 2 - Reset Fail
	 * 3 - Reset Success
	 * 4 - Device Removed
	 * 5 - Devide Inserted
	 * 6 - Driver Unloaded
	 * 7 - Driver Loaded
	 *
	 * driverVersion: offset: 2 length: 10
	 *
	 * Supported Feature: Bringup
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_BRINGUP_STATUS = 0x680, /* 12 byte payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_POWERSAVE_GENERIC
	 * @ event_subtype: Event subtype
	 * @ full_power_request_reason: Full power request reason
	 * @ pmc_current_state: Pmc current state
	 * @ enable_disable_powersave_mode: Enable disable powersave mode
	 * @ winmob_d_power_state: winmob d power state
	 * @ dtim_period: DTIM period
	 * @ final_listen_intv: Final listen int
	 * @ bmps_auto_timer_duration: BMPS auto timer duration
	 * @ bmps_period: BMPS period
	 *
	 * This event is used in POWERSAVE to send wlan status
	 * Values for parameters are defined below:
	 * Event Sub Type: offset: 0 length: 1
	 * Full Power Req Reason: offset: 1 length: 1
	 * PMC Current State: offset: 2 length: 1
	 * Enable disable powersave mode: 3 length: 1
	 * Winmob D Power State: offset: 4 length: 1
	 * DTIM Period: offset:5 length: 1
	 * Final Listen INTV: offset:6 length: 2
	 * BMPS Auto Timer Duration: 8 length: 2
	 * BMPS Period: offset: 10 length:2
	 *
	 * Supported Feature: POWERSAVE GENERIC
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_POWERSAVE_GENERIC, /* 16 byte payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_POWERSAVE_WOW
	 * @ event_subtype: Event subtype
	 * @ wow_type: Wow type
	 * @ wow_magic_pattern: It will use pattern from 0 to 5
	 * @ wow_del_ptrn_id: Wow delete pattern id
	 * @ wow_wakeup_cause: Wow wakeup cause
	 * @ wow_wakeup_cause_pbm_ptrn_id: Wow wakeup cause pbm pattern id
	 *
	 * This event is used in POWERSAVE WOW to send the wow wakeup pattern,
	 * cause etc
	 * Values for parameters are defined below:
	 * Event Sub Type: offset: 0 length: 1
	 * 0 - Enter
	 * 1 - Exit
	 * 2 - Del Pattern
	 * 3 - Wakup
	 *
	 * WOW Type: offset: 1 length: 1
	 * 0 - None
	 * 1 - Magic Pkt Only
	 * 2 - Byte Match Only
	 * 3 - Magic Pkt Byte Match
	 *
	 * WOW Magic Pattern: offset:2 length: 6
	 * WOW Del Pattern ID: offset:8 length: 1
	 * WOW Wakeup Cause: offset: 9 length: 1
	 * 0 - Magic Pkt Match
	 * 1 - Ptrn Byte Match
	 * WOW Wakeup Cause PBM Ptrn ID: offset: 10 length: 1
	 *
	 * Supported Feature: Powersave wow
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_POWERSAVE_WOW, /* 11 byte payload */

	/* Events between 0x683 to 0x690 are not used */

	/*
	 * <diag_event>
	 * EVENT_WLAN_BTC
	 * @ eventId: Event id
	 * @ btAddr: BT address
	 * @ connHandle: Connection handle
	 * @ connStatus: Connection status
	 * @ linkType: Link Type
	 * @ scoInterval: Synchronous Connection Oriented interval
	 * @ scoWindow: Synchronous Connection Oriented window
	 * @ retransWindow: Retransmisson window
	 * @ mode: Mode
	 *
	 * This event is used in Bluetooth to send the btc status
	 * Values for parameters are defined below:
	 * Event ID: offset: 0 length: 1
	 * 0  - DEVICE SWITCHED ON
	 * 1  - DEVICE SWITCHED OFF
	 * 2  - INQUIRY STARTED
	 * 3  - INQUIRY STOPPED
	 * 4  - PAGE STARTED
	 * 5  - PAGE STOPPED
	 * 6  - CREATE ACL CONNECTION
	 * 7  - ACL CONNECTION COMPLETE
	 * 8  - CREATE SYNC CONNECTION
	 * 9  - SYNC CONNECTION COMPLETE
	 * 10 - SYNC CONNECTION UPDATED
	 * 11 - DISCONNECTION COMPLETE
	 * 12 - MODE CHANGED
	 * 13 - A2DP STREAM START
	 * 14 - A2DP STREAM STOP
	 *
	 * BT Addr[0]: offset: 1 length: 1
	 * BT Addr[1]: offset: 2 length: 1
	 * BT Addr[2]: offset: 3 length: 1
	 * BT Addr[3]: offset: 4 length: 1
	 * BT Addr[4]: offset: 5 length: 1
	 * BT Addr[5]: offset: 6 length: 1
	 *
	 * Conn Handle: offset: 7 length: 2
	 * 65535 - Invalid
	 *
	 * Conn Status: offset:9 length: 1
	 * 0 - Fail
	 * 1 - success
	 *
	 * Link Type: offset: 10 length: 1
	 * 0 - SCO
	 * 1 - ACL
	 * 2 - ESCO
	 *
	 * Sco Interval: offset: 11 length: 1
	 * Sco Window: offset: 12 length: 1
	 * Retrans Window: offset: 13 length: 1
	 *
	 * Mode: offset: 14 length: 1
	 * 0 - Active
	 * 1 - Hold
	 * 2 - Sniff
	 * 3 - Park
	 *
	 * Supported Feature: Bluetooth
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_BTC = 0x691, /* 15 byte payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_EAPOL
	 * @ event_sub_type: 0-Transmitted, 1-Received
	 * @ eapol_packet_type: 0 - EAP Start, 1 - EAPOL Start, 2 - EAPOL
	 * Logoff, 3 - EAPOL Key, 4 - EAPOL Encapsulated Alert
	 * @ eapol_key_info: This field from the driver is in big endian format
	 * @ eapol_rate: Rate at which the frame is received
	 * @ dest_addr: Destination address
	 * * @ src_addr: Source address
	 *
	 * This event is used to send Extensible Authentication Protocol
	 * information
	 * Values for parameters are defined below:
	 * event_sub_type: offset: 0 length: 1
	 * eapol_packet_type: offset: 1 length: 1
	 * eapol_key_info: offset:2 length: 2
	 * eapol_rate: offset: 4 length: 2
	 * dest_addr[0]: offset: 6 length: 1
	 * dest_addr[1]: offset: 7 length: 1
	 * dest_addr[2]: offset: 8 length: 1
	 * dest_addr[3]: offset: 9 length: 1
	 * dest_addr[4]: offset: 10 length: 1
	 * dest_addr[5]: offset: 11 length: 1
	 * src_addr[0]: offset: 12 length: 1
	 * src_addr[1]: offset: 13 length: 1
	 * src_addr[2]: offset: 14 length: 1
	 * src_addr[3]: offset: 15 length: 1
	 * src_addr[4]: offset: 16 length: 1
	 * src_addr[5]: offset: 17 length: 1
	 *
	 * Supported Feature: Extensible Authentication Protocol
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_EAPOL = 0xA8D,/* 18 bytes payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_WAKE_LOCK
	 * @ status: Whether the wakelock is taken/released
	 * @ reason: Reason for taking this wakelock
	 * @ timeout: Timeout value in case of timed wakelocks
	 * @ name_len: Length of the name of the wakelock that will follow
	 * @ name: Name of the wakelock
	 *
	 * This event is used to send wakelock information
	 * Values for parameters are defined below:
	 * status: offset: 0 length: 4
	 * reason: offset: 4 length: 4
	 * timeout: offset: 8 length: 4
	 * name_len: offset: 12 length: 4
	 *
	 * Supported Feature: wlan wakelock
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_WAKE_LOCK = 0xAA2, /* 96 bytes payload */
	EVENT_WLAN_BEACON_RECEIVED = 0xAA6, /* FW event: 2726 */

	/*
	 * <diag_event>
	 * EVENT_WLAN_LOG_COMPLETE
	 * @ is_fatal: Indicates if the event is fatal or not
	 * @ indicator: Source of the bug report - Framework/Host/Firmware
	 * @ reason_code: Reason for triggering bug report
	 * @ reserved: Reserved field
	 *
	 * This event is used to send log completion related information
	 * Values for parameters are defined below:
	 * is_fatal: offset: 0 length: 4
	 * indicator: offset: 4 length: 4
	 * reason_code: offset: 8 length: 4
	 * reserved: offset: 12 length: 4
	 *
	 * Supported Feature: Logging
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_LOG_COMPLETE = 0xAA7, /* 16 bytes payload */

	/*
	 * <diag_event>
	 * EVENT_WLAN_STATUS_V2
	 * @ event_id: Event id
	 * @ ssid: Network SSID
	 * @ bssType: BSS Type
	 * @ rssi: RSSI
	 * @ channel: Channel Numbers
	 * @ qosCapability: quality of service capability
	 * @ authType: Authentication type
	 * @ encryptionType: Encryption type
	 * @ reason: Reason for triggering status
	 * @ reasonDisconnect:Reason for disconnection
	 *
	 * This event is used to send varius wlan status
	 * Values for parameters are defined below:
	 * eventId: offset: 0 length: 1
	 * ssid[0] - ssid[31]: offset: 1 to 32, length: 1
	 * bssType: offset: 33 length: 1
	 * rssi: offset: 34 length: 1
	 * channel: offset: 35 length: 1
	 * qosCapability: offset: 36 length: 1
	 * authType: offset: 37 length: 1
	 * encryptionType: offset: 38 length: 1
	 * reason: offset: 39 length: 1
	 * reasonDisconnect: offset: 40 length: 1
	 *
	 * Supported Feature: Wlan status
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_STATUS_V2 = 0xAB3,

	/*
	 * <diag_event>
	 * EVENT_WLAN_TDLS_TEARDOWN
	 * @ reason: reason for tear down.
	 * @peer_mac: Peer mac address
	 *
	 *
	 * This event is sent when TDLS tear down happens.
	 *
	 * Supported Feature: TDLS
	 *
	 * </diag_event>
	 */
	EVENT_WLAN_TDLS_TEARDOWN = 0xAB5,

	/*
	 * <diag_event>
	 * EVENT_WLAN_TDLS_ENABLE_LINK
	 * @peer_mac: peer mac
	 * @is_off_chan_supported: If peer supports off channel
	 * @is_off_chan_configured: If off channel is configured
	 * @is_off_chan_established: If off channel is established
	 *
	 *
	 * This event is sent when TDLS enable link happens.
	 *
	 * Supported Feature: TDLS
	 *
	 * </diag_event>
	 */
	EVENT_WLAN_TDLS_ENABLE_LINK = 0XAB6,

	/*
	 * <diag_event>
	 * EVENT_WLAN_SUSPEND_RESUME
	 * @ state: suspend/resume state
	 *
	 * This event is used to send suspend resume info
	 * Values for parameters are defined below:
	 * suspend: offset: 0 length: 1
	 * 0 - HDD_WLAN_EARLY_SUSPEND
	 * 1 - HDD_WLAN_SUSPEND
	 * 2 - HDD_WLAN_EARLY_RESUME
	 * 3 - HDD_WLAN_RESUME
	 *
	 * Supported Feature: suspend/resume
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_SUSPEND_RESUME = 0xAB7,

	/*
	 * <diag_event>
	 * EVENT_WLAN_OFFLOAD_REQ
	 * @ offload_type: offload type
	 * @ state: enabled or disabled state
	 *
	 * This event is used to send offload info
	 * Values for parameters are defined below:
	 * offloadType: offset: 0 length: 1
	 * 0 - SIR_IPV4_ARP_REPLY_OFFLOAD
	 * 1 - SIR_IPV6_NEIGHBOR_DISCOVERY_OFFLOAD
	 * 2 - SIR_IPV6_NS_OFFLOAD
	 *
	 * enableOrDisable: offset: 1 length: 1
	 * 0 - SIR_OFFLOAD_DISABLE
	 * 1 - SIR_OFFLOAD_ENABLE
	 *
	 * Supported Feature: offload
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_OFFLOAD_REQ = 0xAB8,

	/*
	 * <diag_event>
	 * EVENT_TDLS_SCAN_BLOCK
	 * @status: rejected status
	 *
	 *
	 * This event is sent when scan is rejected due to TDLS.
	 *
	 * Supported Feature: TDLS
	 *
	 * </diag_event>
	 */
	EVENT_TDLS_SCAN_BLOCK = 0xAB9,

	/*
	 * <diag_event>
	 * EVENT_WLAN_TDLS_TX_RX_MGMT
	 * @event_id: event id
	 * @tx_rx: tx or rx
	 * @type: type of frame
	 * @action_sub_type: action frame type
	 * @peer_mac: peer mac
	 *
	 *
	 * This event is sent when TDLS mgmt rx tx happens.
	 *
	 * Supported Feature: TDLS
	 *
	 * </diag_event>
	 */
	EVENT_WLAN_TDLS_TX_RX_MGMT = 0xABA,

	/*
	 * <diag_event>
	 * EVENT_WLAN_LOW_RESOURCE_FAILURE
	 * @ WIFI_EVENT_MEMORY_FAILURE: Memory failure
	 *
	 * This event is used to send reason why low resource situation
	 * is observed
	 *
	 * Supported Feature: Memory
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_LOW_RESOURCE_FAILURE = 0xABB,

	/*
	 * <diag_event>
	 * EVENT_WLAN_POWERSAVE_WOW_STATS
	 * @ wow_ucast_wake_up_count: send unicast packet count
	 * @ wow_bcast_wake_up_count: send broadcast packet count
	 * @ wow_ipv4_mcast_wake_up_coun: send ipv4 multicast packet count
	 * @ wow_ipv6_mcast_wake_up_count: send ipv6 multicast packet count
	 * @ wow_ipv6_mcast_ra_stats: send ipv6 multicast ra packet count
	 * @ wow_ipv6_mcast_ns_stats: send ipv6 multicast ns packet count
	 * @ wow_ipv6_mcast_na_stats: send ipv6 multicast na packet count
	 * @ wow_pno_match_wake_up_count: preferred network offload match count
	 * @ wow_pno_complete_wake_up_count: preferred network offload complete
	 * @ wow_gscan_wake_up_count:Reason: send external scan packet count
	 * @ wow_low_rssi_wake_up_count: send low rssi packet count
	 * @ wow_rssi_breach_wake_up_count: send rssi breach packet count
	 * @ wow_icmpv4_count: Send icmpv4 packet count
	 * @ wow_icmpv6_count: send icmpv6 packet count
	 * @ wow_oem_response_wake_up_count: Send oem response packet count
	 *
	 * This event is used to send wow wakeup stats information
	 *
	 * Supported Feature: Wlan powersave wow
	 *
	 * </diag_event>
	 */
	EVENT_WLAN_POWERSAVE_WOW_STATS = 0xB33,

	/*
	 * <diag_event>
	 * EVENT_WLAN_STA_KICKOUT
	 * @reasoncode: Indicates the reasoncode of event
	 * @peer_macaddr: Indicates the peer macaddr
	 * @vdev_id: Indicate unique id for identifying the VDEV
	 *
	 * This event is used to send sta kickout information
	 * Values for parameters are defined below:
	 * Reasoncode: offset: 0 length: 4
	 * Peer macaddr: offset: 4 length: 6
	 * VDEV ID: offset: 10 length 1
	 *
	 * Supported Feature: STA
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_STA_KICKOUT = 0xB39,

	/*
	 * <diag_event>
	 * EVENT_WLAN_STA_DATASTALL
	 * @reason: Indicates the reason of event
	 *
	 * This event is used to send sta datastall information
	 * Values for parameters are defined below:
	 * Reason: offset:0 length: 4
	 *
	 * Supported Feature: STA
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_STA_DATASTALL = 0xB3A,

	/*
	 * <diag_event>
	 * EVENT_WLAN_SOFTAP_DATASTALL
	 * @reason: Indicates the reason of event
	 *
	 * This event is used to send SAP datastall information
	 * Values for parameters are defined below:
	 * Reason: offset:0 length: 4
	 *
	 * Supported Feature: SAP
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_SOFTAP_DATASTALL = 0xB3B,

	/*
	 * <diag_event>
	 * EVENT_WLAN_SSR_REINIT_SUBSYSTEM
	 * @status: Indicates the status of event
	 *
	 * This event is used to send ssr reinit status
	 * Values for parameters are defined below:
	 * Status: offset: 0 length: 4
	 *
	 * Supported Feature: SSR
	 *
	 *  </diag_event>
	 */

	EVENT_WLAN_SSR_REINIT_SUBSYSTEM = 0xB3C,

	/*
	 * <diag_event>
	 * EVENT_WLAN_SSR_SHUTDOWN_SUBSYSTEM
	 * @status: Indicates the status of event
	 *
	 * This event is used to send ssr shutdown status
	 * Values for parameters are defined below:
	 * Status: offset: 0 length: 4
	 *
	 * Supported Feature: SSR
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_SSR_SHUTDOWN_SUBSYSTEM = 0xB3D,
	EVENT_WLAN_ACS_REQ = 0xC4A,
	EVENT_WLAN_ACS_SCAN_START = 0xC4B,
	EVENT_WLAN_ACS_SCAN_DONE = 0xC4C,
	EVENT_WLAN_ACS_CHANNEL_SPECTRAL_WEIGHT = 0xC4D,
	EVENT_WLAN_ACS_BEST_CHANNEL = 0xC4E,
	EVENT_WLAN_HOST_MGMT_TX_V2 = 0xC52,
	EVENT_WLAN_HOST_MGMT_RX_V2 = 0xC53,
	EVENT_WLAN_CONN_STATS_V2 = 0xC56,

	/*
	 * <diag_event>
	 * EVENT_WLAN_RSN_INFO
	 * @akm_suite: Gives information about akm suites used in assoc request
	 * @ucast_cipher: Unicast cipher used in assoc request
	 * @mcast_cipher: Multi cast cipher used in assoc request
	 * @group_mgmt: Requested group mgmt cipher suite
	 *
	 * This event is used to send RSN information used
	 * in assoc request.
	 *
	 * Supported Feature: STA
	 *
	 * </diag_event>
	 */

	EVENT_WLAN_RSN_INFO = 0xC5B,


	EVENT_MAX_ID = 0x0FFF
} event_id_enum_type;

#endif /* EVENT_DEFS_H */

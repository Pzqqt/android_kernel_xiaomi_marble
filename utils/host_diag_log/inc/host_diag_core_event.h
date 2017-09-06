/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined(__HOST_DIAG_CORE_EVENT_H)
#define __HOST_DIAG_CORE_EVENT_H

/**=========================================================================

   \file  host_diag_core_event.h

   \brief WLAN UTIL host DIAG Events

   Definitions for DIAG Events

   ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include "qdf_types.h"
#include "i_host_diag_core_event.h"

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define WAKE_LOCK_NAME_LEN 80

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_SECURITY
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t eventId;
	uint8_t authMode;
	uint8_t encryptionModeUnicast;
	uint8_t encryptionModeMulticast;
	uint8_t pmkIDMatch;
	uint8_t bssid[6];
	uint8_t keyId;
	uint8_t status;
} host_event_wlan_security_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_STATUS_V2
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t eventId;
	uint8_t ssid[32];
	uint8_t bssType;
	uint8_t rssi;
	uint8_t channel;
	uint8_t qosCapability;
	uint8_t authType;
	uint8_t encryptionType;
	uint8_t reason;
	uint8_t reasonDisconnect;
} host_event_wlan_status_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_HANDOFF
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t eventId;
	uint8_t currentApBssid[6];
	uint8_t currentApRssi;
	uint8_t candidateApBssid[6];
	uint8_t candidateApRssi;
} host_event_wlan_handoff_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_VCC
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t eventId;
	uint8_t rssi;
	uint8_t txPer;
	uint8_t rxPer;
	int linkQuality;
} host_event_wlan_vcc_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_QOS
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t eventId;
	uint8_t reasonCode;
} host_event_wlan_qos_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_PE
   ------------------------------------------------------------------------*/
typedef struct {
	char bssid[6];
	uint16_t event_type;
	uint16_t sme_state;
	uint16_t mlm_state;
	uint16_t status;
	uint16_t reason_code;
} host_event_wlan_pe_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_ADD_BLOCK_ACK_SUCCESS
   ------------------------------------------------------------------------*/
typedef struct {
	char ucBaPeerMac[6];
	uint8_t ucBaTid;
	uint8_t ucBaBufferSize;
	uint16_t usBaSSN;
	uint8_t fInitiator;
} host_event_wlan_add_block_ack_success_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_ADD_BLOCK_ACK_FAILED
   ------------------------------------------------------------------------*/
typedef struct {
	char ucBaPeerMac[6];
	uint8_t ucBaTid;
	uint8_t ucReasonCode;
	uint8_t fInitiator;
} host_event_wlan_add_block_ack_failed_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_DELETE_BLOCK_ACK_SUCCESS
   ------------------------------------------------------------------------*/
typedef struct {
	char ucBaPeerMac[6];
	uint8_t ucBaTid;
	uint8_t ucDeleteReasonCode;
} host_event_wlan_add_block_ack_deleted_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_DELETE_BLOCK_ACK_FAILED
   ------------------------------------------------------------------------*/
typedef struct {
	char ucBaPeerMac[6];
	uint8_t ucBaTid;
	uint8_t ucDeleteReasonCode;
	uint8_t ucFailReasonCode;
} host_event_wlan_add_block_ack_delete_failed_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_BSS_PROTECTION
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t event_type;
	uint8_t prot_type;
} host_event_wlan_bss_prot_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_BRINGUP_STATUS
   ------------------------------------------------------------------------*/
typedef struct {
	uint16_t wlanStatus;
	char driverVersion[10];
} host_event_wlan_bringup_status_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_POWERSAVE_WOW
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t event_subtype;
	uint8_t wow_type;
	uint8_t wow_magic_pattern[6];
	uint8_t wow_del_ptrn_id;
	uint8_t wow_wakeup_cause;
	uint8_t wow_wakeup_cause_pbm_ptrn_id;
} host_event_wlan_powersave_wow_payload_type;

/*-------------------------------------------------------------------------
   Event ID: EVENT_WLAN_BTC
   ------------------------------------------------------------------------*/
typedef struct {
	uint8_t eventId;
	uint8_t btAddr[6];
	uint16_t connHandle;
	uint8_t connStatus;
	uint8_t linkType;
	uint8_t scoInterval;
	uint8_t scoWindow;
	uint8_t retransWindow;
	uint8_t mode;
} host_event_wlan_btc_type;

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_EAPOL
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_eapol - Structure holding the eapol information
 * @event_sub_type:	0-Transmitted, 1-Received
 * @eapol_packet_type:	0 - EAP Start, 1 - EAPOL Start, 2 - EAPOL Logoff
			3 - EAPOL Key, 4 - EAPOL Encapsulated Alert
 * @eapol_key_info:	This field from the driver is in big endian format.
 *			So, the masks .0x8013. can be used to extract the
 *			message type. After masking, the values corresponding
 *			to messages 1/2/3/4 are given below:
 *			Msg. 1	0x8000
 *			Msg. 2	0x0001
 *			Msg. 3	0x8013
 *			Msg. 4	0x0003
 * @eapol_rate:		Rate at which the frame is received
 * @dest_addr:		Destination address
 * @src_addr:		Source address
 *
 * This structure contains the EAPOL information related to logging
 */
struct host_event_wlan_eapol {
	uint8_t   event_sub_type;
	uint8_t   eapol_packet_type;
	uint16_t  eapol_key_info;
	uint16_t  eapol_rate;
	uint8_t   dest_addr[6];
	uint8_t   src_addr[6];
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_LOW_RESOURCE_FAILURE
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_low_resource_failure - Structure holding the
 * low resource failure information
 * @event_sub_type: Gives further information about reason for
 * low resource condition
 *
 * This structure will hold the low resource failure information
 */
struct host_event_wlan_low_resource_failure {
	uint8_t event_sub_type;
};

/**
 * enum resource_failure_type - Reason for low resource failure condition
 * @WIFI_EVENT_MEMORY_FAILURE: Memory failure
 *
 * This enum has the reason codes why the low resource situation is observed
 */
enum resource_failure_type {
	WIFI_EVENT_MEMORY_FAILURE,
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_WAKE_LOCK
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_wake_lock - Structure holding the wakelock information
 * @status: Whether the wakelock is taken/released
 * @reason: Reason for taking this wakelock
 * @timeout: Timeout value in case of timed wakelocks
 * @name_len: Length of the name of the wakelock that will follow
 * @name: Name of the wakelock
 *
 * This structure will hold the wakelock information
 */
struct host_event_wlan_wake_lock {
	uint32_t status;
	uint32_t reason;
	uint32_t timeout;
	uint32_t name_len;
	char     name[WAKE_LOCK_NAME_LEN];
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_LOG_COMPLETE
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_log_complete - Holds log completion details
 * @is_fatal: Indicates if the event is fatal or not
 * @indicator: Source of the bug report - Framework/Host/Firmware
 * @reason_code: Reason for triggering bug report
 * @reserved: Reserved field
 *
 * This structure holds the log completion related information
 */
struct host_event_wlan_log_complete {
	uint32_t is_fatal;
	uint32_t indicator;
	uint32_t reason_code;
	uint32_t reserved;
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_STA_KICKOUT
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_kickout - Holds diag event details
 * @reasoncode: Indicates the reasoncode of event
 * @peer_macaddr: Indicates the peer macaddr
 * @vdev_id: Indicate unique id for identifying the VDEV
 *
 * This structure holds the diag event related information
 */

struct host_event_wlan_kickout {
	uint32_t reasoncode;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
	uint8_t vdev_id;
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_SOFTAP_DATASTALL/EVENT_WLAN_STA_DATASTALL
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_softap_datastall - Holds diag event details
 * @reason: Indicates the reason of event
 *
 *This structure holds the host diag event related information
 */

struct host_event_wlan_datastall {
	uint32_t reason;
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_SSR_REINIT_SUBSYSTEM
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_ssr_reinit - Holds diag event details
 * @status: Indicates the status of event
 *
 *This structure holds the host diag event related information
 */

struct host_event_wlan_ssr_reinit {
	uint32_t status;
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_SSR_SHUTDOWN_SUBSYSTEM
  ------------------------------------------------------------------------*/
/**
 * struct host_event_wlan_ssr_shutdown - Holds diag event details
 * @status: Indicates the status of event
 *
 *This structure holds the host diag event related information
 */

struct host_event_wlan_ssr_shutdown {
	uint32_t status;
};


/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/
/**
 * enum host_sta_kickout_events - Enum containing sta kickout subtype
 * @HOST_STA_KICKOUT_REASON_BMISS: Indicate sta got disconnected reason
 * beacon miss
 * @HOST_STA_KICKOUT_REASON_XRETRY: Indicate sta got disconnected reason xretry
 * @HOST_STA_KICKOUT_REASON_UNSPECIFIED: Indicate sta disconnection
 * reason unspecified
 * @HOST_STA_KICKOUT_REASON_KEEP_ALIVE: Indicate sta is disconnected
 * because of keep alive
 *
 * This enum contains the event subtype
 */
enum host_sta_kickout_events {
	HOST_STA_KICKOUT_REASON_BMISS,
	HOST_STA_KICKOUT_REASON_XRETRY,
	HOST_STA_KICKOUT_REASON_UNSPECIFIED,
	HOST_STA_KICKOUT_REASON_KEEP_ALIVE,
};

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/
/**
 * enum host_datastall_events - Enum containing datastall subtype
 * @STA_TX_TIMEOUT: Indicate sta tx timeout
 * @SOFTAP_TX_TIMEOUT:Indicate softap tx timeout
 *
 * This enum contains the event subtype
 */
enum host_datastall_events {
	STA_TX_TIMEOUT,
	SOFTAP_TX_TIMEOUT,
};

/*-------------------------------------------------------------------------
  Function declarations and documenation
  ------------------------------------------------------------------------*/
/**
 * enum host_ssr_events - Enum containing ssr subtype
 * @SSR_SUB_SYSTEM_REINIT: Indicate ssr reinit state
 * @SSR_SUB_SYSTEM_SHUTDOWN: Indicate ssr shutdown state
 *
 * This enum contains the event subtype
 */
enum host_ssr_events {
	SSR_SUB_SYSTEM_REINIT,
	SSR_SUB_SYSTEM_SHUTDOWN,
};

/**
 * struct host_event_tdls_teardown - tdls teardown diag event
 * @reason: reason for tear down
 * @peer_mac: peer mac
 *
 * This structure contains tdls teardown diag event info
 */
struct host_event_tdls_teardown {
	uint32_t reason;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
};

/**
 * struct host_event_tdls_enable_link - tdls enable link event
 * @peer_mac: peer mac
 * @is_off_chan_supported: if off channel supported
 * @is_off_chan_configured: if off channel configured
 * @is_off_chan_established: if off channel established
 *
 * This structure contain tdls enable link diag event info
 */
struct host_event_tdls_enable_link {
	uint8_t   peer_mac[QDF_MAC_ADDR_SIZE];
	uint8_t   is_off_chan_supported;
	uint8_t   is_off_chan_configured;
	uint8_t   is_off_chan_established;
};

/**
 * struct host_event_suspend - suspend/resume state
 * @state: suspend/resume state
 *
 * This structure contains suspend resume diag event info
 */
struct host_event_suspend {
	uint8_t state;
};

/**
 * struct host_event_offload_req - offload state
 * @offload_type: offload type
 * @state: enabled or disabled state
 *
 * This structure contains offload diag event info
 */
struct host_event_offload_req {
	uint8_t offload_type;
	uint8_t state;
};

/**
 * struct host_event_tdls_scan_rejected - scan
 * rejected due to tdls
 * @status: rejected status
 *
 * This structure contains scan rejected due to
 * tdls event info
 */
struct host_event_tdls_scan_rejected {
	uint8_t status;
};

/**
 * struct host_event_tdls_tx_rx_mgmt - for TX RX management frame
 * @event_id: event ID
 * @tx_rx: tx or rx
 * @type: type of frame
 * @action_sub_type: action frame type
 * @peer_mac: peer mac
 *
 * This structure contains tdls TX RX management frame info
 */
struct host_event_tdls_tx_rx_mgmt {
	uint8_t event_id;
	uint8_t tx_rx;
	uint8_t type;
	uint8_t action_sub_type;
	uint8_t peer_mac[QDF_MAC_ADDR_SIZE];
};

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/
/**
 * enum wifi_connectivity_events - Enum containing EAPOL sub type
 * @WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED:	EAPOL transmitted
 * @WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED:		EAPOL received
 *
 * This enum contains the EAPOL subtype
 */
enum wifi_connectivity_events {
	WIFI_EVENT_DRIVER_EAPOL_FRAME_TRANSMIT_REQUESTED,
	WIFI_EVENT_DRIVER_EAPOL_FRAME_RECEIVED,
};

/**
 * enum wake_lock_reason - Reason for taking/releasing wakelock
 * @WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT: Driver initialization
 * @WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT: Driver re-initialization
 * @WIFI_POWER_EVENT_WAKELOCK_DRIVER_EXIT: Driver shutdown
 * @WIFI_POWER_EVENT_WAKELOCK_SCAN: Scan request/response handling
 * @WIFI_POWER_EVENT_WAKELOCK_EXT_SCAN: Extended scan request/response handling
 * @WIFI_POWER_EVENT_WAKELOCK_RESUME_WLAN: Driver resume
 * @WIFI_POWER_EVENT_WAKELOCK_ROC: Remain on channel request/response handling
 * @WIFI_POWER_EVENT_WAKELOCK_AUTO_SUSPEND: Auto suspend related handling
 * @WIFI_POWER_EVENT_WAKELOCK_IPA: IPA related handling
 * @WIFI_POWER_EVENT_WAKELOCK_ADD_STA: Addition of STA
 * @WIFI_POWER_EVENT_WAKELOCK_HOLD_RX: Wakelocks taken for receive
 * @WIFI_POWER_EVENT_WAKELOCK_SAP: SoftAP related wakelocks
 * @WIFI_POWER_EVENT_WAKELOCK_WOW: WoW feature related
 * @WIFI_POWER_EVENT_WAKELOCK_PNO: PNO feature related
 * @WIFI_POWER_EVENT_WAKELOCK_DEL_STA: Deletion of a station
 * @WIFI_POWER_EVENT_WAKELOCK_DFS: DFS related wakelocks
 * @WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP: Firmware response
 * @WIFI_POWER_EVENT_WAKELOCK_MISC: Miscellaneous wakelocks
 * @WIFI_POWER_EVENT_WAKELOCK_DHCP: DHCP negotiation under way
 * @WIFI_POWER_EVENT_WAKELOCK_CONNECT: connection in progress
 * @WIFI_POWER_EVENT_WAKELOCK_IFACE_CHANGE_TIMER: iface change timer running
 *
 * Indicates the reason for which the wakelock was taken/released
 */
enum wake_lock_reason {
	WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT,
	WIFI_POWER_EVENT_WAKELOCK_DRIVER_REINIT,
	WIFI_POWER_EVENT_WAKELOCK_DRIVER_EXIT,
	WIFI_POWER_EVENT_WAKELOCK_SCAN,
	WIFI_POWER_EVENT_WAKELOCK_EXT_SCAN,
	WIFI_POWER_EVENT_WAKELOCK_RESUME_WLAN,
	WIFI_POWER_EVENT_WAKELOCK_ROC,
	WIFI_POWER_EVENT_WAKELOCK_AUTO_SUSPEND,
	WIFI_POWER_EVENT_WAKELOCK_IPA,
	WIFI_POWER_EVENT_WAKELOCK_ADD_STA,
	WIFI_POWER_EVENT_WAKELOCK_HOLD_RX,
	WIFI_POWER_EVENT_WAKELOCK_SAP,
	WIFI_POWER_EVENT_WAKELOCK_WOW,
	WIFI_POWER_EVENT_WAKELOCK_PNO,
	WIFI_POWER_EVENT_WAKELOCK_DEL_STA,
	WIFI_POWER_EVENT_WAKELOCK_DFS,
	WIFI_POWER_EVENT_WAKELOCK_WMI_CMD_RSP,
	WIFI_POWER_EVENT_WAKELOCK_MISC,
	WIFI_POWER_EVENT_WAKELOCK_DHCP,
	WIFI_POWER_EVENT_WAKELOCK_CONNECT,
	WIFI_POWER_EVENT_WAKELOCK_IFACE_CHANGE_TIMER,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HOST_DIAG_CORE_EVENT_H */

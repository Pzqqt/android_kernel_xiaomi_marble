/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#define RSN_OUI_SIZE 4

/**
 * enum wifi_frm_type: type of frame
 *
 * @MGMT: Indicates management frames
 * @CTRL: Indicates control frames
 * @DATA: Inidcates data frames
 */
enum wifi_frm_type {
	MGMT = 0x00,
	CTRL = 0x01,
	DATA = 0x02,
};

/*
 * enum mgmt_frm_subtype: sub types of mgmt frames
 *
 * @ASSOC_REQ:       association request frame
 * @ASSOC_RESP:      association response frame
 * @REASSOC_REQ:     reassociation request frame
 * @REASSOC_RESP:    reassociation response frame
 * @PROBE_REQ:       probe request frame
 * @PROBE_RESP:      probe response frame
 * @BEACON:          beacon frame
 * @ATIM:            ATIM frame
 * @DISASSOC:        disassociation frame
 * @AUTH:            authentication frame
 * @DEAUTH:          deauthentication frame
 * @ACTION:          action frame
 * @ACTION_NO_ACK:   action no ack frame
 */
enum mgmt_frm_subtype {
	ASSOC_REQ = 0x00,
	ASSOC_RESP = 0x01,
	REASSOC_REQ = 0x02,
	REASSOC_RESP = 0x03,
	PROBE_REQ = 0x04,
	PROBE_RESP = 0x05,
	BEACON = 0x08,
	ATIM = 0x09,
	DISASSOC = 0x0a,
	AUTH = 0x0b,
	DEAUTH = 0x0c,
	ACTION = 0x0d,
	ACTION_NO_ACK = 0x0e,
};

/**
 * enum mgmt_auth_type: type of authentication
 *
 * @AUTH_OPEN: no security applied
 * @AUTH_SHARED: WEP type of auth
 * @AUTH_WPA_EAP: WPA1 EAP based auth
 * @AUTH_WPA_PSK: WPA1 PSK based auth
 * @AUTH_WPA2_EAP: WPA2 EAP based auth
 * @AUTH_WPA2_PSK: WPA2 PSK based auth
 * @AUTH_WAPI_CERT: WAPI CERT based auth
 * @AUTH_WAPI_PSK: WAPI PSK based auth
 */
enum mgmt_auth_type {
	AUTH_OPEN = 0x00,
	AUTH_SHARED = 0x01,
	AUTH_WPA_EAP = 0x02,
	AUTH_WPA_PSK = 0x03,
	AUTH_WPA2_EAP = 0x04,
	AUTH_WPA2_PSK = 0x05,
	AUTH_WAPI_CERT = 0x06,
	AUTH_WAPI_PSK = 0x07,
	AUTH_MAX = 0xff,
};

/**
 * enum mgmt_encrypt_type: type of encryption
 *
 * @ENC_MODE_OPEN: no encryption applied
 * @ENC_MODE_WEP40: WEP 40 bits encryption
 * @ENC_MODE_WEP104: WEP 104 bits encryption
 * @ENC_MODE_TKIP: TKIP based encryption
 * @ENC_MODE_AES: AES based encryption
 * @ENC_MODE_AES_GCMP: AES with GCMP encryption
 * @ENC_MODE_AES_GCMP_256: AES with 256 bit GCMP encryption
 * @ENC_MODE_SMS4: WAPI based SMS4 encryption
 */
enum mgmt_encrypt_type {
	ENC_MODE_OPEN = 0x00,
	ENC_MODE_WEP40 = 0x01,
	ENC_MODE_WEP104 = 0x02,
	ENC_MODE_TKIP = 0x03,
	ENC_MODE_AES = 0x04,
	ENC_MODE_AES_GCMP = 0x05,
	ENC_MODE_AES_GCMP_256 = 0x06,
	ENC_MODE_SMS4 = 0x07,
	ENC_MODE_MAX = 0x0f,
};

/**
 * enum mgmt_ch_width: channel width of connection
 *
 * @BW_20MHZ: 20 MHz of channel bonding
 * @BW_40MHZ: 40 MHz of channel bonding
 * @BW_80MHZ: 80 MHz of channel bonding
 * @BW_160MHZ: 160 MHz of channel bonding
 * @BW_80P80MHZ: 80 + 80 MHz of channel bonding
 * @BW_5MHZ: 5 MHz of channel bonding
 * @BW_10MHZ: 10 MHz of channel bonding
 * @BW_320MHZ: 320 MHz of channel bonding
 */
enum mgmt_ch_width {
	BW_20MHZ = 0x00,
	BW_40MHZ = 0x01,
	BW_80MHZ = 0x02,
	BW_160MHZ = 0x03,
	BW_80P80MHZ = 0x04,
	BW_5MHZ = 0x05,
	BW_10MHZ = 0x06,
	BW_320MHZ = 0x07,
	BW_MAX = 0xff,
};

/**
 * enum mgmt_dot11_mode: 80211 mode of operation
 *
 * @DOT11_MODE_ABG: 802.11-ABG mix mode
 * @DOT11_MODE_11A: 802.11-A mode
 * @DOT11_MODE_11B: 802.11-B mode
 * @DOT11_MODE_11G: 802.11-G mode
 * @DOT11_MODE_11N: 802.11-N mode
 * @DOT11_MODE_11AC: 802.11-AC mode
 * @DOT11_MODE_11G_ONLY: 802.11-G only mode
 * @DOT11_MODE_11N_ONLY: 802.11-N only mode
 * @DOT11_MODE_11AC_ONLY: 802.11-AC only mode
 * @DOT11_MODE_AUTO: 802.11 auto mode
 * @DOT11_MODE_11AX: 802.11-AX mode
 * @DOT11_MODE_11AX_ONLY: 802.11-AX only mode
 * @DOT11_MODE_11BE: 802.11-BE mode
 * @DOT11_MODE_11BE_ONLY: 802.11-BE only mode
 */
enum mgmt_dot11_mode {
	DOT11_MODE_ABG = 0x00,
	DOT11_MODE_11A = 0x01,
	DOT11_MODE_11B = 0x02,
	DOT11_MODE_11G = 0x03,
	DOT11_MODE_11N = 0x04,
	DOT11_MODE_11AC = 0x05,
	DOT11_MODE_11G_ONLY = 0x06,
	DOT11_MODE_11N_ONLY = 0x07,
	DOT11_MODE_11AC_ONLY = 0x08,
	DOT11_MODE_AUTO = 0x09,
	DOT11_MODE_11AX = 0x0a,
	DOT11_MODE_11AX_ONLY = 0x0b,
	DOT11_MODE_11BE = 0x0c,
	DOT11_MODE_11BE_ONLY = 0x0d,
	DOT11_MODE_MAX = 0xff,
};

/**
 * enum mgmt_bss_type: persona type
 *
 * @STA_PERSONA: STA mode
 * @SAP_PERSONA: SAP mode
 * @P2P_CLIENT_PERSONA: P2P cli mode
 * @P2P_GO_PERSONA: P2P go mode
 * @FTM_PERSONA: FTM mode
 * @IBSS_PERSONA: IBSS mode
 * @MONITOR_PERSONA: monitor mode
 * @P2P_DEVICE_PERSONA: P2P device mode
 * @OCB_PERSONA: OCB mode
 * @EPPING_PERSONA: epping mode
 * @QVIT_PERSONA: QVIT mode
 * @NDI_PERSONA: NDI mode
 * @WDS_PERSONA: WDS mode
 * @BTAMP_PERSONA: BT amp mode
 * @AHDEMO_PERSONA: AH demo mode
 */
enum mgmt_bss_type {
	STA_PERSONA = 0x00,
	SAP_PERSONA = 0x01,
	P2P_CLIENT_PERSONA = 0x02,
	P2P_GO_PERSONA = 0x03,
	FTM_PERSONA = 0x04,
	IBSS_PERSONA = 0x05,
	MONITOR_PERSONA = 0x06,
	P2P_DEVICE_PERSONA = 0x07,
	OCB_PERSONA = 0x08,
	EPPING_PERSONA = 0x09,
	QVIT_PERSONA = 0x0a,
	NDI_PERSONA = 0x0b,
	WDS_PERSONA = 0x0c,
	BTAMP_PERSONA = 0x0d,
	AHDEMO_PERSONA = 0x0e,
	MAX_PERSONA = 0xff,
};

/**
 * enum wlan_bringup_status: driver/device status
 *
 * @WLAN_STATUS_DISABLED: WLAN Disabled
 * @WLAN_STATUS_ENABLED: WLAN Enabled
 * @WLAN_STATUS_RESET_FAIL: Reset Fail
 * @WLAN_STATUS_RESET_SUCCESS: Reset Success
 * @WLAN_STATUS_DEVICE_REMOVED: Device Removed
 * @WLAN_STATUS_DEVICE_INSERTED: Devide Inserted
 * @WLAN_STATUS_DRIVER_UNLOADED: Driver Unloaded
 * @WLAN_STATUS_DRIVER_LOADED: Driver Loaded
 * @WLAN_STATUS_BUS_EXCEPTION: bus/link exception
 * @WLAN_STATUS_DEVICE_TEMPERATURE_HIGH: chip temperature high
 */
enum wlan_bringup_status {
	WLAN_STATUS_DISABLED = 0,
	WLAN_STATUS_ENABLED = 1,
	WLAN_STATUS_RESET_FAIL = 2,
	WLAN_STATUS_RESET_SUCCESS = 3,
	WLAN_STATUS_DEVICE_REMOVED = 4,
	WLAN_STATUS_DEVICE_INSERTED = 5,
	WLAN_STATUS_DRIVER_UNLOADED = 6,
	WLAN_STATUS_DRIVER_LOADED = 7,
	WLAN_STATUS_BUS_EXCEPTION = 8,
	WLAN_STATUS_DEVICE_TEMPERATURE_HIGH = 9,

	WLAN_STATUS_MAX = 0xffff,
};

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

/**
 * host_event_wlan_connection_stats: to capture connection details
 *
 * @rssi: RSSI signal strength of connected AP, units in dbM
 * @ssid_len: length of SSID
 * @ssid: SSID of AP where STA is connected
 * @bssid: bssid of AP where STA is connected
 * @operating_channel: channel on which AP is connected
 * @qos_capability: QoS is enabled or no
 * @chnl_bw: channel BW of connection, units in MHz
 *		Range: enum mgmt_ch_width
 * @dot11mode: 802.11 mode of current connection
 *		Range: enum mgmt_dot11_mode
 * @bss_type: type of the BSS whether AP/IBSS/P2PGO
 *		Range: enum mgmt_bss_type bss_type
 * @auth_type: type of authentication for connected AP
 *		Range: enum mgmt_auth_type
 * @encryption_type: type of encryption applied
 *		Range: enum mgmt_encrypt_type
 * @reserved1: reserved for future use
 * @est_link_speed: link speed of connection, units in Mbps
 * @result_code: result code of connection success or failure
 * @reason_code: if failed then what is the reason
 * @op_freq: channel frequency in MHz on which AP is connected
 */
struct host_event_wlan_connection_stats {
	int8_t rssi;
	uint8_t ssid_len;
	char ssid[32];
	uint8_t bssid[6];
	uint8_t operating_channel;
	uint8_t qos_capability;
	uint8_t chnl_bw;
	uint8_t dot11mode;
	uint8_t bss_type;
	uint8_t auth_type;
	uint8_t encryption_type;
	uint8_t reserved1;
	uint32_t est_link_speed;
	uint16_t result_code;
	uint16_t reason_code;
	uint32_t op_freq;
} qdf_packed;

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

/**
 * host_event_wlan_mgmt_payload_type: To capture TX/RX mgmt frames' payload
 *
 * @mgmt_type: type of frames, value: enum wifi_frm_type
 * @mgmt_subtype: subtype of mgmt frame, value: enum mgmt_frm_subtype
 * @operating_channel: operating channel of AP
 * @ssid_len: length of SSID, max 32 bytes long as per standard
 * @ssid: SSID of connected AP
 * @self_mac_addr: mac address of self interface
 * @bssid: BSSID for which frame is received
 * @result_code: result code TX/RX OTA delivery
 * @reason_code: reason code given in TX/RX frame
 */
struct host_event_wlan_mgmt_payload_type {
	uint8_t mgmt_type;
	uint8_t mgmt_subtype;
	uint8_t operating_channel;
	uint8_t ssid_len;
	char ssid[32];
	char self_mac_addr[6];
	char bssid[6];
	uint16_t result_code;
	uint16_t reason_code;
} qdf_packed;

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
/**
 * struct host_event_wlan_bringup_status_payload_type - Structure holding the
 * device/driver status info
 *
 * @wlan_status: status code as defined by enum wlan_bringup_status
 * @driver_version: version of WLAN driver
 *
 * This structure will hold WLAN device basic status and driver version
 */
typedef struct {
	uint16_t wlan_status;
	char driver_version[10];
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
  Event ID: EVENT_WLAN_POWERSAVE_WOW_STATS
  ------------------------------------------------------------------------*/
/**
 * host_event_wlan_powersave_wow_stats - Structure holding wow stats information
 * @wow_ucast_wake_up_count: wow unicast packet wakeup count
 * @wow_bcast_wake_up_count: wow broadcast packet wakeup count
 * @wow_ipv4_mcast_wake_up_count: wow ipv4 multicast packet wakeup count
 * @wow_ipv6_mcast_wake_up_count: wow ipv6 multicast packet wakeup count
 * @wow_ipv6_mcast_ra_stats: wow ipv6 multicast router advertisement
 *                           packet wakeup count
 * @wow_ipv6_mcast_ns_stats: wow ipv6 multicast Neighbor Solicitation
 *                           packet wakeup count
 * @wow_ipv6_mcast_na_stats: wow ipv6 multicast address space
 *                           packet wakeup count
 * @wow_pno_match_wake_up_count: wow preferred network offload match
 *                           packet wakeup count
 * @wow_pno_complete_wake_up_count: wow preferred network offload complete
 *                                  packet wakeup count
 * @wow_gscan_wake_up_count: wow external scan packet wakeup count
 * @wow_low_rssi_wake_up_count: wow low rssi packet wakeup count
 * @wow_rssi_breach_wake_up_count: wow rssi breach packet wakeup count
 * @wow_icmpv4_count: wow icmpv4 packet count
 * @wow_icmpv6_count: wow icmpv6 packet count
 * @wow_oem_response_wake_up_count: wow  oem response packet wakeup count
 *
 * This structure contains the wow stats information related to diag event
 */
struct host_event_wlan_powersave_wow_stats {
	uint32_t    wow_ucast_wake_up_count;
	uint32_t    wow_bcast_wake_up_count;
	uint32_t    wow_ipv4_mcast_wake_up_count;
	uint32_t    wow_ipv6_mcast_wake_up_count;
	uint32_t    wow_ipv6_mcast_ra_stats;
	uint32_t    wow_ipv6_mcast_ns_stats;
	uint32_t    wow_ipv6_mcast_na_stats;
	uint32_t    wow_pno_match_wake_up_count;
	uint32_t    wow_pno_complete_wake_up_count;
	uint32_t    wow_gscan_wake_up_count;
	uint32_t    wow_low_rssi_wake_up_count;
	uint32_t    wow_rssi_breach_wake_up_count;
	uint32_t    wow_icmpv4_count;
	uint32_t    wow_icmpv6_count;
	uint32_t    wow_oem_response_wake_up_count;
	uint32_t    Reserved_1;
	uint32_t    Reserved_2;
	uint32_t    Reserved_3;
	uint32_t    Reserved_4;
};

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
  Event ID: EVENT_WLAN_RSN_INFO
  -------------------------------------------------------------------------
 */
/**
 * struct event_wlan_csr_rsn_info - Structure holding the
 * RSN information for assoc request
 * @akm_suite: Gives information about akm suites used in assoc request
 * @ucast_cipher: Unicast cipher used in assoc request
 * @mcast_cipher: Multi cast cipher used in assoc request
 * @group_mgmt: Requested group mgmt cipher suite
 *
 * This structure will hold the RSN information for assoc request
 */
struct event_wlan_csr_rsn_info {
	uint8_t   akm_suite[RSN_OUI_SIZE];
	uint8_t   ucast_cipher[RSN_OUI_SIZE];
	uint8_t   mcast_cipher[RSN_OUI_SIZE];
	uint8_t   group_mgmt[RSN_OUI_SIZE];
};

/*-------------------------------------------------------------------------
  Event ID: EVENT_WLAN_AUTH_INFO
  -------------------------------------------------------------------------
 */
/**
 * struct event_wlan_lim_auth_info - Structure holding the
 * algo num, seq num and status code for auth request
 * @auth_algo_num: Gives information about algo num used in auth request
 * @auth_transaction_seq_num: seq num of auth request
 * @auth_status_code: status code of auth request
 *
 * This structure will hold the algo num, seq num and status code
 * for auth request
 */
struct event_wlan_lim_auth_info {
	uint16_t   auth_algo_num;
	uint16_t   auth_transaction_seq_num;
	uint16_t   auth_status_code;
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
 * @HOST_STA_KICKOUT_REASON_BTM: BTM request from AP with disassoc imminent
 * reason
 *
 * This enum contains the event subtype
 */
enum host_sta_kickout_events {
	HOST_STA_KICKOUT_REASON_BMISS,
	HOST_STA_KICKOUT_REASON_XRETRY,
	HOST_STA_KICKOUT_REASON_UNSPECIFIED,
	HOST_STA_KICKOUT_REASON_KEEP_ALIVE,
	HOST_STA_KICKOUT_REASON_BTM,
};

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/
/**
 * enum host_datastall_events - Enum containing datastall subtype
 * @DATA_STALL_NONE: Indicate no data stall
 * @FW_VDEV_PAUSE: Indicate FW vdev Pause
 * @HWSCHED_CMD_FILTER:Indicate HW sched command filter
 * @HWSCHED_CMD_FLUSH: Indicate HW sched command flush
 * @FW_RX_REFILL_FAILED:Indicate FW rx refill failed
 * @FW_RX_FCS_LEN_ERROR:Indicate FW fcs len error
 * @FW_WDOG_ERRORS:Indicate watchdog error
 * @FW_BB_WDOG_ERROR:Indicate BB watchdog error
 * @STA_TX_TIMEOUT: Indicate sta tx timeout
 * @SOFTAP_TX_TIMEOUT:Indicate softap tx timeout
 * @NUD_FAILURE: Indicare NUD Failure
 *
 * This enum contains the event subtype
 */
enum host_datastall_events {
	DATA_STALL_NONE,
	FW_VDEV_PAUSE,
	HWSCHED_CMD_FILTER,
	HWSCHED_CMD_FLUSH,
	FW_RX_REFILL_FAILED,
	FW_RX_FCS_LEN_ERROR,
	FW_WDOG_ERRORS,
	FW_BB_WDOG_ERROR,
	STA_TX_TIMEOUT,
	SOFTAP_TX_TIMEOUT,
	NUD_FAILURE,
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
 * @WIFI_POWER_EVENT_WAKELOCK_DISCONNECT: disconnection in progress
 * @WIFI_POWER_EVENT_WAKELOCK_IFACE_CHANGE_TIMER: iface change timer running
 * @WIFI_POWER_EVENT_WAKELOCK_MONITOR_MODE: Montitor mode wakelock
 * @WIFI_POWER_EVENT_WAKELOCK_DRIVER_IDLE_RESTART: Wakelock for Idle Restart
 * @WIFI_POWER_EVENT_WAKELOCK_DRIVER_IDLE_SHUTDOWN: Wakelock for Idle Shutdown
 * @WIFI_POWER_EVENT_WAKELOCK_TDLS: Wakelock for TDLS
 * @WIFI_POWER_EVENT_WAKELOCK_CFR: Wakelock for CFR
 * @WIFI_POWER_EVENT_WAKELOCK_SAP_D3_WOW: Wakelock for SAP D3 WOW max clinets
 * @WIFI_POWER_EVENT_WAKELOCK_GO_D3_WOW: Wakelock for GO D3 WOW max clients
 * @WIFI_POWER_EVENT_WAKELOCK_RTPM_TPUT_POLICY: Wakelock for RTPM Tput policy
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
	WIFI_POWER_EVENT_WAKELOCK_DISCONNECT,
	WIFI_POWER_EVENT_WAKELOCK_IFACE_CHANGE_TIMER,
	WIFI_POWER_EVENT_WAKELOCK_MONITOR_MODE,
	WIFI_POWER_EVENT_WAKELOCK_DRIVER_IDLE_RESTART,
	WIFI_POWER_EVENT_WAKELOCK_DRIVER_IDLE_SHUTDOWN,
	WIFI_POWER_EVENT_WAKELOCK_TDLS,
	WIFI_POWER_EVENT_WAKELOCK_CFR,
	WIFI_POWER_EVENT_WAKELOCK_SAP_D3_WOW,
	WIFI_POWER_EVENT_WAKELOCK_GO_D3_WOW,
	WIFI_POWER_EVENT_WAKELOCK_RTPM_TPUT_POLICY,
};

/* The length of interface name should >= IFNAMSIZ */
#define HOST_EVENT_INTF_STR_LEN 16
#define HOST_EVENT_HW_MODE_STR_LEN 12

/**
 * struct host_event_wlan_acs_req - payload for ACS diag event
 * @intf: network interface name for WLAN
 * @hw_mode: hw mode configured by hostapd
 * @bw: channel bandwidth(MHz)
 * @ht: a flag indicating whether HT phy mode is enabled
 * @vht: a flag indicating whether VHT phy mode is enabled
 * @chan_start: starting channel number for ACS scan
 * @chan_end: ending channel number for ACS scan
 *
 * This structure includes all the payload related to ACS request parameters
 */
struct host_event_wlan_acs_req {
	uint8_t intf[HOST_EVENT_INTF_STR_LEN];
	uint8_t hw_mode[HOST_EVENT_HW_MODE_STR_LEN];
	uint16_t bw;
	uint8_t ht;
	uint8_t vht;
	uint16_t chan_start;
	uint16_t chan_end;
};

/**
 * struct host_event_wlan_acs_scan_start - payload for ACS scan request
 * @scan_id: scan request ID
 * @vdev_id: vdev/session ID
 *
 * This structure includes all the payload related to ACS scan request
 * parameters
 */
struct host_event_wlan_acs_scan_start {
	uint32_t scan_id;
	uint8_t vdev_id;
};

#define HOST_EVENT_STATUS_STR_LEN 24

/**
 * struct host_event_wlan_acs_scan_done - payload for ACS scan done event
 * @status: indicating whether ACS scan is successful
 * @vdev_id: vdev/session ID
 * @scan_id: scan request ID
 *
 * This structure includes all the payload related to ACS scan done event
 */
struct host_event_wlan_acs_scan_done {
	uint8_t status[HOST_EVENT_STATUS_STR_LEN];
	uint32_t scan_id;
	uint8_t vdev_id;
};

/**
 * struct host_event_wlan_acs_chan_spectral_weight - payload for spectral
 * weight event indication
 * @chan: channel number
 * @weight: channel weight
 * @rssi: RSSI value obtained after scanning
 * @bss_count: number of BSS detected on this channel
 *
 * This structure includes all the payload related to a channel's weight
 * evaluation result
 */
struct host_event_wlan_acs_chan_spectral_weight {
	uint16_t chan;
	uint16_t weight;
	int32_t rssi;
	uint16_t bss_count;
};

/**
 * struct host_event_wlan_acs_best_chan - payload for ACS best channel event
 * @chan: channel number
 * @weight: channel weight
 *
 * This structure includes all the payload related to the best channel
 * selected after ACS procedure
 */
struct host_event_wlan_acs_best_chan {
	uint16_t chan;
	uint16_t weight;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HOST_DIAG_CORE_EVENT_H */

/*
 * Copyright (c) 2014-2017, 2019-2020 The Linux Foundation. All rights reserved.
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

#if !defined(__HOST_DIAG_CORE_LOG_H)
#define __HOST_DIAG_CORE_LOG_H

/**=========================================================================

   \file  host_diag_core_log.h

   \brief WLAN UTIL host DIAG logs

   Definitions for WLAN UTIL host diag events

   ========================================================================*/

/* $Header$ */

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include "qdf_types.h"
#include "i_host_diag_core_log.h"

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/
#define HOST_LOG_MAX_NUM_SSID                                (21)
#define HOST_LOG_MAX_NUM_BSSID                               (21)
#define HOST_LOG_MAX_SSID_SIZE                               (32)
#define HOST_LOG_MAX_BSSID_SIZE                              (6)
#define HOST_LOG_MAX_NUM_CHANNEL                             (64)
#define HOST_LOG_MAX_NUM_HO_CANDIDATE_APS                    (20)
#define HOST_LOG_MAX_WOW_PTRN_SIZE                           (128)
#define HOST_LOG_MAX_WOW_PTRN_MASK_SIZE                      (16)
#define VOS_LOG_PKT_LOG_SIZE                                 (2048)
#define HOST_LOG_PKT_LOG_THRESHOLD                           40960
#define HOST_LOG_MAX_COLD_BOOT_CAL_DATA_SIZE                 (2048)

/* Version to be updated whenever format of vos_log_pktlog_info changes */
#define VERSION_LOG_WLAN_PKT_LOG_INFO_C                     1
/* Version to be updated whenever format of host_log_cold_boot_cal_data_type
 * changes
 */
#define VERSION_LOG_WLAN_COLD_BOOT_CAL_DATA_C               1

/*---------------------------------------------------------------------------
   This packet contains the scan results of the recent scan operation
   LOG_WLAN_SCAN_C                                 0x1496
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t eventId;
	uint8_t numSsid;
	uint8_t ssid[HOST_LOG_MAX_NUM_SSID][HOST_LOG_MAX_SSID_SIZE];
	uint8_t bssid[HOST_LOG_MAX_NUM_BSSID][HOST_LOG_MAX_BSSID_SIZE];
	uint8_t totalSsid;
	uint8_t minChnTime;
	uint8_t maxChnTime;
	uint16_t timeBetweenBgScan;
	uint8_t BSSMode;
	uint8_t numChannel;
	uint8_t channels[HOST_LOG_MAX_NUM_CHANNEL];
	uint16_t status;
} host_log_scan_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to IBSS connection setup
   LOG_WLAN_IBSS_C                                 0x1497
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t eventId;
	uint8_t channelSetting;
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr peer_macaddr;
	uint8_t ssid[HOST_LOG_MAX_SSID_SIZE];
	uint8_t operatingChannel;
	uint8_t beaconInterval;
	uint8_t status;
	uint32_t op_freq;
} host_log_ibss_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to 802.11D
   LOG_WLAN_80211D_C                               0x1498
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t eventId;
	uint8_t numChannel;
	uint8_t Channels[HOST_LOG_MAX_NUM_CHANNEL];
	uint8_t TxPwr[HOST_LOG_MAX_NUM_CHANNEL];
	uint8_t countryCode[3];
	uint8_t supportMultipleDomain;
} host_log_802_11d_pkt_type;

/*---------------------------------------------------------------------------
   This is a log packet which contains below handoff information:
   - Current AP + RSSI (if already associated)
   - Candidate AP + RSSI (before association and when the list is updated)
   - For each BSSID in candidate list, provide RSSI, QoS and security compatibility
   LOG_WLAN_HANDOFF_C                              0x1499
   ---------------------------------------------------------------------------*/
typedef struct {
	uint8_t ssid[9];
	uint8_t bssid[HOST_LOG_MAX_BSSID_SIZE];
	uint8_t channel_id;
	uint32_t qos_score;
	uint32_t sec_score;
	uint32_t rssi_score;
	uint32_t overall_score;
	uint32_t tx_per;                /* represented as a %      */
	uint32_t rx_per;                /* represented as a %      */

} host_log_ho_ap_info;

typedef struct {
	log_hdr_type hdr;
	uint32_t num_aps;
	host_log_ho_ap_info current_ap_info;
	host_log_ho_ap_info
		candidate_ap_info[HOST_LOG_MAX_NUM_HO_CANDIDATE_APS];
} host_log_ho_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to the EDCA parameters
   advertised by the AP
   LOG_WLAN_QOS_EDCA_C                             0x149A
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t aci_be;
	uint8_t cw_be;
	uint16_t txoplimit_be;
	uint8_t aci_bk;
	uint8_t cw_bk;
	uint16_t txoplimit_bk;
	uint8_t aci_vi;
	uint8_t cw_vi;
	uint16_t txoplimit_vi;
	uint8_t aci_vo;
	uint8_t cw_vo;
	uint16_t txoplimit_vo;
} host_log_qos_edca_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the total number of beacon received value
   LOG_WLAN_BEACON_UPDATE_C                        0x149B
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint32_t bcn_rx_cnt;
} host_log_beacon_update_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the information related to a WoW patern value when set
   LOG_WLAN_POWERSAVE_WOW_ADD_PTRN_C               0x149C
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t pattern_id;
	uint8_t pattern_byte_offset;
	uint8_t pattern_size;
	uint8_t pattern[HOST_LOG_MAX_WOW_PTRN_SIZE];
	uint8_t pattern_mask_size;
	uint8_t pattern_mask[HOST_LOG_MAX_WOW_PTRN_MASK_SIZE];
} host_log_powersave_wow_add_ptrn_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains the Tspec info negotiated with the AP for the
   specific AC
   LOG_WLAN_QOS_TSPEC_C                            0x14A2
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	uint8_t tsinfo[3];
	uint16_t nominal_msdu_size;
	uint16_t maximum_msdu_size;
	uint32_t min_service_interval;
	uint32_t max_service_interval;
	uint32_t inactivity_interval;
	uint32_t suspension_interval;
	uint32_t svc_start_time;
	uint32_t min_data_rate;
	uint32_t mean_data_rate;
	uint32_t peak_data_rate;
	uint32_t max_burst_size;
	uint32_t delay_bound;
	uint32_t min_phy_rate;
	uint16_t surplus_bw_allowance;
	uint16_t medium_time;
} host_log_qos_tspec_pkt_type;

/*---------------------------------------------------------------------------
   This packet contains data information when stall detected
   LOG_TRSP_DATA_STALL_C                           0x1801
   ---------------------------------------------------------------------------*/

typedef struct {
	char channelName[4];
	uint32_t numDesc;
	uint32_t numFreeDesc;
	uint32_t numRsvdDesc;
	uint32_t headDescOrder;
	uint32_t tailDescOrder;
	uint32_t ctrlRegVal;
	uint32_t statRegVal;
	uint32_t numValDesc;
	uint32_t numInvalDesc;
} host_log_data_stall_channel_type;

typedef struct {
	log_hdr_type hdr;
	uint32_t PowerState;
	uint32_t numFreeBd;
	host_log_data_stall_channel_type dxeChannelInfo[4];
} host_log_data_stall_type;

/*---------------------------------------------------------------------------
   This packet contains the rssi value from BSS descriptor
   LOG_WLAN_RSSI_UPDATE_C                          0x1354
   ---------------------------------------------------------------------------*/
typedef struct {
	log_hdr_type hdr;
	int8_t rssi;
} host_log_rssi_pkt_type;

/**
 * struct host_log_pktlog_info - Packet log info
 * @log_hdr: Log header
 * @buf_len: Length of the buffer that follows
 * @buf:     Buffer containing the packet log info
 *
 * Structure containing the packet log information
 * LOG_WLAN_PKT_LOG_INFO_C          0x18E0
 */
struct host_log_pktlog_info {
	log_hdr_type log_hdr;
	uint32_t version;
	uint32_t seq_no;
	uint32_t buf_len;
	uint8_t buf[];
};

/**
 * struct host_log_cold_boot_cal_data_type - Cold boot cal log info
 * @hdr: Log header
 * @version: version
 * @flags: Flag to indicate if more data follows
 * @cb_cal_data_len: Length of the cal data
 * @cb_cal_data: Cold boot cal data
 *
 * Structure containing the cold boot calibration data
 * log information
 * LOG_WLAN_COLD_BOOT_CAL_DATA_C          0x1A18
 */
struct host_log_cold_boot_cal_data_type {
	log_hdr_type hdr;
	uint32_t version;
	uint32_t flags;
	uint32_t cb_cal_data_len;
	uint8_t cb_cal_data[HOST_LOG_MAX_COLD_BOOT_CAL_DATA_SIZE];
};

#define WLAN_MAX_ROAM_CANDIDATE_AP      8
#define WLAN_MAX_ROAM_SCAN_CHAN         38
#define WLAN_MAX_SSID_SIZE              32
#define WLAN_MAC_ADDR_SIZE              6

/**
 * host_log_wlan_mgmt_tx_rx_info: To capture TX/RX mgmt frames' payload
 * @hdr: Log header
 * @version: Version number of the payload
 * @vdev_id: Vdev id
 * @is_tx: 1 - TX frame, 0 - RX frame
 * @mgmt_type: type of frames, value: enum wifi_frm_type
 * @mgmt_subtype: subtype of mgmt frame, value: enum mgmt_frm_subtype
 * @mgmt_frame_seq_num: Frame sequence number in 802.11 header
 * @operating_freq: operating frequency of AP
 * @ssid_len: length of SSID, max 32 bytes long as per standard
 * @ssid: SSID of connected AP
 * @self_mac_addr: mac address of self interface
 * @bssid: BSSID for which frame is received
 * @mac_failure_reason: Internal driver failure reason
 * @mgmt_status_code: 802.11 management frame response status code from
 * section 9.4.1.9 IEEE 802.11 - 2016
 * @auth_algo: Authentication algorithm number
 * @auth_transaction_num: Authentication transaction sequence number
 * @is_retry: Is retry frame
 * @rssi: RSSI for the received frame
 * @origin: 1- Sent by host. 2- sent by firmware
 */
struct host_log_wlan_mgmt_tx_rx_info {
	log_hdr_type hdr;
	uint8_t version;
	uint8_t vdev_id;
	bool is_tx;
	uint8_t mgmt_type;
	uint8_t mgmt_subtype;
	uint16_t mgmt_frame_seq_num;
	uint8_t operating_freq;
	uint8_t ssid_len;
	char ssid[WLAN_MAX_SSID_SIZE];
	uint8_t self_mac_addr[WLAN_MAC_ADDR_SIZE];
	uint8_t bssid[WLAN_MAC_ADDR_SIZE];
	uint16_t mac_failure_reason;
	uint16_t mgmt_status_code;
	uint8_t auth_algo;
	uint8_t auth_transaction_num;
	uint8_t is_retry;
	uint32_t rssi;
	uint8_t origin;
} qdf_packed;

/**
 * struct wlan_roam_btm_trigger_data - BTM roam trigger related information
 * @btm_request_mode:      BTM request mode - solicited/unsolicited
 * @disassoc_timer:        Number of TBTT before AP disassociates the STA in ms
 * @validity_interval:     Preferred candidate list validity interval in ms
 * @candidate_list_count:  Number of candidates in BTM request.
 * @btm_resp_status:       Status code of the BTM response.
 */
struct wlan_roam_btm_trigger_data {
	uint8_t btm_request_mode;
	uint32_t disassoc_timer;
	uint32_t validity_interval;
	uint16_t candidate_list_count;
	uint16_t btm_resp_status;
} qdf_packed;

/**
 * struct wlan_roam_cu_trigger_data - BSS Load roam trigger parameters
 * @cu_load: Connected AP CU load percentage
 */
struct wlan_roam_cu_trigger_data {
	uint16_t cu_load;
} qdf_packed;

/**
 * Struct wlan_roam_rssi_trigger_data - RSSI roam trigger related
 * parameters
 * @threshold: RSSI threshold value in dBm for LOW rssi roam trigger
 */
struct wlan_roam_rssi_trigger_data {
	uint32_t threshold;
} qdf_packed;

/**
 * struct wlan_roam_deauth_trigger_data - Deauth roaming trigger related
 * parameters
 * @type:   1- Deauthentication 2- Disassociation
 * @reason: Status code of the Deauth/Disassoc received
 */
struct wlan_roam_deauth_trigger_data {
	uint8_t type;
	uint32_t reason;
} qdf_packed;

/**
 * struct host_log_wlan_roam_trigger_info - Roam trigger
 * related info
 * @hdr: Log header
 * @version: Version number of the payload
 * @vdev_id: Vdev id
 * @trigger_reason: Roaming trigger reason
 * @trigger_sub_reason: Roaming trigger sub reason
 * @current_rssi:  Current connected AP RSSI
 * @timestamp: Host driver timestamp in msecs
 * @btm_trig_data: BTM trigger related data
 * @cu_load_data: CU load trigger related data
 * @rssi_trig_data: RSSI roam trigger related data
 * @deauth_trig_data: Deauth Roam trigger related data
 */
struct host_log_wlan_roam_trigger_info {
	log_hdr_type hdr;
	uint8_t version;
	uint8_t vdev_id;
	uint32_t trigger_reason;
	uint32_t trigger_sub_reason;
	uint32_t current_rssi;
	uint32_t timestamp;
	union {
		struct wlan_roam_btm_trigger_data btm_trig_data;
		struct wlan_roam_cu_trigger_data cu_load_data;
		struct wlan_roam_rssi_trigger_data rssi_trig_data;
		struct wlan_roam_deauth_trigger_data deauth_trig_data;
	};
} qdf_packed;

/**
 *  struct host_log_wlan_roam_candidate_info - Roam scan candidate APs related
 *  info
 *  @version:     Payload structure version
 *  @timestamp:   Host timestamp in millisecs
 *  @type:        0 - Candidate AP; 1 - Current connected AP.
 *  @bssid:       AP bssid.
 *  @freq:        Channel frquency
 *  @cu_load:     Channel utilization load of the AP.
 *  @cu_score:    Channel Utilization score.
 *  @rssi:        Candidate AP rssi
 *  @rssi_score:  AP RSSI score
 *  @total_score: Total score of the candidate AP.
 *  @etp:         Estimated throughput value of the AP in Mbps
 */
struct host_log_wlan_roam_candidate_info {
	uint8_t version;
	uint32_t timestamp;
	uint8_t type;
	uint8_t bssid[WLAN_MAC_ADDR_SIZE];
	uint16_t freq;
	uint32_t cu_load;
	uint32_t cu_score;
	uint32_t rssi;
	uint32_t rssi_score;
	uint32_t total_score;
	uint32_t etp;
} qdf_packed;

/**
 * struct host_log_wlan_roam_scan_data - Roam scan event details
 * @hdr: Log header
 * @version:   Version number of the diag log payload
 * @vdev_id:   Vdev ID
 * @type:      0 - Partial roam scan; 1 - Full roam scan
 * @num_ap:    Number of candidate APs.
 * @num_chan:  Number of channels.
 * @timestamp: Time of day in milliseconds at which scan was triggered
 * @trigger_reason: Roam scan trigger reason
 * @next_rssi_threshold: Next roam can trigger rssi threshold
 * @chan_freq: List of frequencies scanned as part of roam scan
 * @ap: List of candidate AP info
 */
struct host_log_wlan_roam_scan_data {
	log_hdr_type hdr;
	uint8_t version;
	uint8_t vdev_id;
	uint16_t type;
	uint8_t num_ap;
	uint8_t num_chan;
	uint32_t timestamp;
	uint32_t trigger_reason;
	uint32_t next_rssi_threshold;
	uint16_t chan_freq[WLAN_MAX_ROAM_SCAN_CHAN];
	struct host_log_wlan_roam_candidate_info ap[WLAN_MAX_ROAM_CANDIDATE_AP];
} qdf_packed;

/**
 * struct host_log_wlan_roam_result_info - Roam result related info.
 * @hdr:                Log header
 * @version:            Payload strcuture version
 * @vdev_id:            Vdev Id
 * @status:             0 - Roaming is success ; 1 - Roaming failed
 * @timestamp:          Host timestamp in millisecs
 * @fail_reason:        One of WMI_ROAM_FAIL_REASON_ID
 */
struct host_log_wlan_roam_result_info {
	log_hdr_type hdr;
	uint8_t version;
	uint8_t vdev_id;
	bool status;
	uint32_t timestamp;
	uint32_t fail_reason;
} qdf_packed;

/**
 * struct wlan_rrm_beacon_report - RRM beacon report related
 * parameters
 * @req_bssid: beacon report requestor BSSID
 * @req_ssid: Requested SSID for beacon report
 * @is_wildcard_bssid: Is the BSSID FF:FF:FF:FF:FF:FF
 * @req_reg_class: Regulatory class mentioned in the request
 * @req_measurement_mode: Measurement mode. Active/Passive/Beacon report Table
 * @req_measurement_duration: Measurement duration requested.
 * @num_reports_in_frame: Number of BSS scanned
 * @is_last_frame_in_req: True if this frame is the last frame sent for the
 * request
 */
struct wlan_rrm_beacon_report {
	uint8_t req_bssid[WLAN_MAC_ADDR_SIZE];
	uint8_t req_ssid[WLAN_MAX_SSID_SIZE];
	bool is_wildcard_bssid;
	uint8_t req_reg_class;
	uint16_t req_measurement_mode;
	uint16_t req_measurement_duration;
	uint8_t num_reports_in_frame;
	bool is_last_frame_in_req;
} qdf_packed;

/**
 * struct host_log_wlan_rrm_tx_rx_info - RRM frame related details
 * @hdr:     Log header
 * @version: Version of the payload struture
 * @vdev_id: Vdev id
 * @orgin:   Sent by host or firmware
 * @is_tx:   Is Tx frame or RX frame
 * @roam_result: Roaming result
 * @timestamp: Time of the day in milliseconds
 * @mgmt_frame_seq_num: Frame sequence number
 * @received_chan_freq: Frame received channel frequency
 * @action_category: Action frame category
 * @rrm_action_code: Radio measurement/Noise measurement
 * @radio_measurement_type: Neighbor report/Beacon report
 * @bssid: BSSID field in frame
 * @req_num_freq: Number of frequencies provided in request
 * @req_freq: Frequencies requested
 * @fail_reason_code: response TX failure status code
 * @rssi: Rx frame rssi
 * @bcn_rpt: Beacon report related parameters
 */
struct host_log_wlan_rrm_tx_rx_info {
	log_hdr_type hdr;
	uint8_t version;
	uint8_t vdev_id;
	uint8_t origin;
	bool is_tx;
	bool roam_result;
	uint32_t timestamp;
	uint16_t mgmt_frame_seq_num;
	uint16_t received_chan_freq;
	uint8_t action_category;
	uint8_t rrm_action_code;
	uint8_t radio_measurement_type;
	uint8_t bssid[WLAN_MAC_ADDR_SIZE];
	uint8_t req_num_freq;
	uint16_t req_freq[WLAN_MAX_ROAM_SCAN_CHAN];
	uint8_t fail_reason_code;
	uint32_t rssi;
	struct wlan_rrm_beacon_report bcn_rpt;
} qdf_packed;

/*-------------------------------------------------------------------------
   Function declarations and documenation
   ------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HOST_DIAG_CORE_LOG_H */

/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: wlan_cm_roam_logging.c
 *
 * Implementation for the Connect/roaming logging.
 */

#ifndef _WLAN_CONNECTIVITY_LOGGING_H_
#define _WLAN_CONNECTIVITY_LOGGING_H_

#include "wlan_policy_mgr_api.h"
#include <wmi_unified_priv.h>
#include "wlan_crypto_global_api.h"
#include <wlan_cm_api.h>
#include "wlan_cm_roam_api.h"

#define WLAN_MAX_LOGGING_FREQ 90

/**
 * enum wlan_main_tag  - Main Tag used in logging
 * @WLAN_CONNECTING: Connecting
 * @WLAN_CONNECTING_FAIL: Connection failure
 * @WLAN_AUTH_REQ: Authentication request frame
 * @WLAN_AUTH_RESP: Authentication response frame
 * @WLAN_ASSOC_REQ: Association request frame
 * @WLAN_ASSOC_RESP: Association response frame
 * @WLAN_DEAUTH_RX: Deauthentication frame received
 * @WLAN_DEAUTH_TX: Deauthentication frame sent
 * @WLAN_DISASSOC_RX: Disassociation frame received
 * @WLAN_DISASSOC_TX: Disassociation frame sent
 * @WLAN_DISCONN_BMISS: Disconnection due to beacon miss
 * @WLAN_ROAM_SCAN_START: ROAM scan start
 * @WLAN_ROAM_SCAN_DONE: Roam scan done
 * @WLAN_ROAM_SCR_CURR_AP: Roam score current AP
 * @WLAN_ROAM_SCORE_CAND_AP: Roam Score Candidate AP
 * @WLAN_ROAM_RESULT: Roam Result
 * @WLAN_ROAM_CANCEL: Roam Cancel
 * @WLAN_BTM_REQ:  BTM request
 * @WLAN_BTM_QUERY: BTM Query frame
 * @WLAN_BTM_RESP: BTM response frame
 * @WLAN_BTM_REQ_CANDI: BTM request candidate info
 * @WLAN_ROAM_WTC: ROAM WTC trigger logs
 * @WLAN_DHCP_DISCOVER: DHCP discover frame
 * @WLAN_DHCP_OFFER: DHCP offer frame
 * @WLAN_DHCP_REQUEST: DHCP Request frame
 * @WLAN_DHCP_ACK: DHCP ACK
 * @WLAN_DHCP_NACK: DHCP NACK
 * @WLAN_EAPOL_M1: EAPOL M1
 * @WLAN_EAPOL_M2: EAPOL M2
 * @WLAN_EAPOL_M3: EAPOL M3
 * @WLAN_EAPOL_M4: EAPOL M4
 * @WLAN_GTK_M1: GTK rekey M1 frame
 * @WLAN_GTK_M2: GTK Rekey M2 frame
 * @WLAN_EAP_REQUEST: EAP request frame
 * @WLAN_EAP_RESPONSE: EAP response frame
 * @WLAN_EAP_SUCCESS: EAP success
 * @WLAN_EAP_FAILURE: EAP failure
 * @WLAN_CUSTOM_LOG: Additional WLAN logs
 * @WLAN_TAG_MAX: MAX tag
 */
enum wlan_main_tag {
	WLAN_CONNECTING,
	WLAN_CONNECTING_FAIL,
	WLAN_AUTH_REQ,
	WLAN_AUTH_RESP,
	WLAN_ASSOC_REQ,
	WLAN_ASSOC_RSP,
	WLAN_DEAUTH_RX,
	WLAN_DEAUTH_TX,
	WLAN_DISASSOC_RX,
	WLAN_DISASSOC_TX,
	WLAN_DISCONN_BMISS,
	WLAN_ROAM_SCAN_START,
	WLAN_ROAM_SCAN_DONE,
	WLAN_ROAM_SCORE_CURR_AP,
	WLAN_ROAM_SCORE_CAND_AP,
	WLAN_ROAM_RESULT,
	WLAN_ROAM_CANCEL,
	WLAN_BTM_REQ,
	WLAN_BTM_QUERY,
	WLAN_BTM_RESP,
	WLAN_BTM_REQ_CANDI,
	WLAN_ROAM_WTC,
	WLAN_DHCP_DISCOVER,
	WLAN_DHCP_OFFER,
	WLAN_DHCP_REQUEST,
	WLAN_DHCP_ACK,
	WLAN_DHCP_NACK,
	WLAN_EAPOL_M1,
	WLAN_EAPOL_M2,
	WLAN_EAPOL_M3,
	WLAN_EAPOL_M4,
	WLAN_GTK_M1,
	WLAN_GTK_M2,
	WLAN_EAP_REQUEST,
	WLAN_EAP_RESPONSE,
	WLAN_EAP_SUCCESS,
	WLAN_EAP_FAILURE,
	WLAN_CUSTOM_LOG,
	WLAN_TAG_MAX = 0xFF,
};

/**
 * struct wlan_roam_candidate_info  - Roam candidate information for logging
 * @cand_bssid: BSSID of the candidate AP
 * @is_current_ap: Is the entry candidate AP or connected AP
 * @idx: Entry index
 * @cu_load: Channel utilization load of the AP
 * @freq: Candidate AP channel frequency
 * @total_score: Total candidate AP score
 * @rssi: Candidate AP RSSI
 * @etp: Estimated throughput value of the AP in Kbps
 */
struct wlan_roam_candidate_info {
	struct qdf_mac_addr cand_bssid;
	bool is_current_ap;
	uint8_t idx;
	uint8_t cu_load;
	uint16_t freq;
	uint16_t total_score;
	int32_t rssi;
	uint32_t etp;
};

/**
 * struct wlan_roam_scan_info  - Roam scan related information
 * @cand_ap_count: Roam candidate AP count
 * @num_scanned_frequencies: Number of scanned frequencies
 * @scan_freq: Scanned frequencies
 */
struct wlan_roam_scan_info {
	uint8_t cand_ap_count;
	uint16_t num_scanned_freq;
	uint16_t scan_freq[WLAN_MAX_LOGGING_FREQ];
};

/**
 * struct wlan_roam_result_info  - Roam result data
 * @roam_fail_reason: Roam failure reason code
 * @roam_status: Roam success or failure
 */
struct wlan_roam_result_info {
	uint8_t roam_fail_reason;
	uint8_t roam_status;
};

/**
 * struct wlan_roam_scan_trigger_info  - Structure to store roam scan trigger
 * related data.
 * @is_full_scan: True if the scan is Full scan. False if the roam scan is
 * partial channel map scan
 * @trigger_reason: Roam trigger reason
 * @cu_load:  Current connected channel load in percentage
 * @current_rssi: Connected AP RSSI
 * @rssi_threshold: Roam scan trigger threshold
 */
struct wlan_roam_trigger_info {
	bool is_full_scan;
	uint8_t trigger_reason;
	uint8_t trigger_sub_reason;
	uint8_t cu_load;
	int32_t current_rssi;
	int32_t rssi_threshold;
};

/**
 * struct wlan_btm_cand_info  - BTM candidate information
 * @preference: Candidate preference
 * @bssid: candidate bssid
 */
struct wlan_btm_cand_info {
	uint8_t preference;
	struct qdf_mac_addr bssid;
};

/**
 * struct wlan_roam_btm_info - BTM frame related logging data
 * @reason: Query Reason field
 * @mode: BTM Request Mode field
 * @sub_reason: WTC sub reason
 * @candidate_list_count: Candidates list in the BTM frame
 * @btm_delay: BSS Termination Delay field
 * @is_disassoc_imminent: Disassociation imminent bit
 * @token: dialog token. Dialog Token is a nonzero value chosen by the STA
 * while sending the BTM frame to identify the query/request/response
 * transaction
 * @validity_timer: Validity interval
 * @disassoc_timer: Disassoc timer
 * @wtc_duration: WTC duration field
 * @target_bssid: BTM response target bssid field
 */
struct wlan_roam_btm_info {
	uint8_t reason;
	uint8_t mode;
	uint8_t sub_reason;
	uint8_t candidate_list_count;
	uint8_t btm_status_code;
	uint8_t btm_delay;
	bool is_disassoc_imminent;
	uint16_t token;
	uint32_t validity_timer;
	uint32_t disassoc_timer;
	uint32_t wtc_duration;
	struct qdf_mac_addr target_bssid;
};

/**
 * struct wlan_packet_info  - Data packets related info
 * @tx_status: Packet TX status
 * @eap_type: EAP type
 * @eap_len: EAP data length
 * @auth_algo: Authentication algorithm
 * @auth_seq_num: Authentication frame transaction sequence number
 * @auth_type: Authentication frame sub-type for SAE authentication
 * @frame_status_code: Frame status code/reason code
 * @seq_num: Frame sequence number
 * @rssi: Peer rssi
 */
struct wlan_packet_info {
	uint8_t tx_status;
	uint8_t eap_type;
	uint8_t eap_len;
	uint8_t auth_algo;
	uint8_t auth_seq_num;
	uint8_t auth_type;
	uint16_t frame_status_code;
	uint16_t seq_num;
	int32_t rssi;
};

/**
 * struct wlan_connect_info  - Connection related info
 * @ssid: SSID
 * @ssid_len: Length of the SSID
 * @bssid_hint: BSSID hint provided in the connect request
 * @freq: Frequency
 * @freq_hint: Frequency Hint
 * @akm: Akm suite
 * @pairwise: Pairwise suite
 * @group: Group cipher suite
 * @group_mgmt: Group manangement cipher suite
 * @auth_type: Authentication Algo
 * @is_bt_coex_active: Is there active bluetooth connection
 */
struct wlan_connect_info {
	char ssid[WLAN_SSID_MAX_LEN];
	uint8_t ssid_len;
	struct qdf_mac_addr bssid_hint;
	uint32_t freq;
	uint32_t freq_hint;
	uint32_t akm;
	uint32_t pairwise;
	uint32_t group;
	uint32_t group_mgmt;
	uint8_t auth_type;
	bool is_bt_coex_active;
};

#define WLAN_MAX_LOG_RECORDS 45
#define WLAN_MAX_LOG_LEN     256
#define MAX_RECORD_IN_SINGLE_EVT 5
/**
 * struct wlan_log_record  - Structure for indvidual records in the ring
 * buffer
 * @timestamp_us: Timestamp(time of the day) in microseconds
 * @fw_timestamp_us: timestamp at which roam scan was triggered
 * @vdev_id: VDEV id
 * @log_subtype: Tag of the log
 * @bssid: AP bssid
 * @is_record_filled: indicates if the current record is empty or not
 * @conn_info: Connection info
 * @pkt_info: Packet info
 * @roam_scan: Roam scan
 * @ap: Roam candidate AP info
 * @roam_result: Roam result
 * @roam_trig: Roam trigger related info
 * @btm_info: BTM info
 * @btm_cand: BTM response candidate info
 */
struct wlan_log_record {
	uint64_t timestamp_us;
	uint64_t fw_timestamp_us;
	uint8_t vdev_id;
	uint32_t log_subtype;
	struct qdf_mac_addr bssid;
	bool is_record_filled;
	union {
		struct wlan_connect_info conn_info;
		struct wlan_packet_info pkt_info;
		struct wlan_roam_scan_info roam_scan;
		struct wlan_roam_candidate_info ap;
		struct wlan_roam_result_info roam_result;
		struct wlan_roam_trigger_info roam_trig;
		struct wlan_roam_btm_info btm_info;
		struct wlan_btm_cand_info btm_cand;
	};
};

/**
 * struct wlan_cl_hdd_cbks  - HDD callbacks to be invoked for connectivity
 * logging
 * @wlan_connectivity_log_send_to_usr: Send the log buffer to user space
 */
struct wlan_cl_hdd_cbks {
	QDF_STATUS (*wlan_connectivity_log_send_to_usr)
			(struct wlan_log_record *rec, uint8_t num_records);
};

/**
 * struct wlan_connectivity_log_buf_data  - Master structure to hold the
 * pointers to the ring buffers.
 * @hdd_cbks: Hdd callbacks
 * @head: Pointer to the 1st record allocated in the ring buffer.
 * @read_ptr: Pointer to the next record that can be read.
 * @write_ptr: Pointer to the next empty record to be written.
 * @write_ptr_lock: Spinlock to protect the write_ptr from multiple producers.
 * @max_records: Maximum records in the ring buffer.
 * @read_idx: Read index
 * @write_idx: Write index
 * @dropped_msgs: Dropped logs counter
 * @is_active: If the global buffer is initialized or not
 */
struct wlan_connectivity_log_buf_data {
	struct wlan_cl_hdd_cbks hdd_cbks;
	struct wlan_log_record *head;
	struct wlan_log_record *read_ptr;
	struct wlan_log_record *write_ptr;
	qdf_spinlock_t write_ptr_lock;
	uint8_t max_records;
	uint8_t read_idx;
	uint8_t write_idx;
	qdf_atomic_t dropped_msgs;
	qdf_atomic_t is_active;
};

#define logging_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_MLME, ## params)
#define logging_warn_rl(params...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_MLME, ## params)
#define logging_info_rl(params...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_MLME, ## params)

#define logging_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_QDF, ## params)
#define logging_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_QDF, ## params)
#define logging_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_QDF, ## params)

#ifdef WLAN_FEATURE_CONNECTIVITY_LOGGING
/**
 * wlan_connectivity_logging_init()  - Initialize the connectivity/roaming
 * logging buffer
 * @hdd_cbks: Hdd callbacks
 *
 * Return: None
 */
void wlan_connectivity_logging_init(struct wlan_cl_hdd_cbks *hdd_cbks);

/**
 * wlan_connectivity_logging_deinit() - Deinitialize the connectivity logging
 * buffers and spinlocks.
 *
 * Return: None
 */
void wlan_connectivity_logging_deinit(void);

/**
 * wlan_connectivity_log_dequeue() - Send the connectivity logs to userspace
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_connectivity_log_dequeue(void);

/**
 * wlan_connectivity_log_enqueue() - Add new record to the logging buffer
 * @new_record: Pointer to the new record to be added
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_connectivity_log_enqueue(struct wlan_log_record *new_record);
#else
static inline void wlan_connectivity_logging_init(void)
{}

static inline void wlan_connectivity_logging_deinit(void)
{}

static inline QDF_STATUS wlan_connectivity_log_dequeue(void)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS wlan_connectivity_log_enqueue(struct wlan_log_record *new_record)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
#endif /* _WLAN_CONNECTIVITY_LOGGING_H_ */

/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "wlan_logging_sock_svc.h"
#include "wlan_cm_roam_public_struct.h"

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
	/* Keep at last */
	WLAN_TAG_MAX,
};

/**
 * struct wlan_roam_candidate_info  - Roam candidate information for logging
 * @cand_bssid: BSSID of the candidate AP
 * @is_current_ap: Is the entry candidate AP or connected AP
 * @idx: Entry index
 * @cu_load: Channel utilization load of the AP in percentage
 * @freq: Candidate AP channel frequency in MHz
 * @total_score: Total candidate AP score
 * @rssi: Candidate AP RSSI in dBm
 * @etp: Estimated throughput value of the AP in Kbps
 */
struct wlan_roam_candidate_info {
	struct qdf_mac_addr cand_bssid;
	bool is_current_ap;
	uint8_t idx;
	uint8_t cu_load;
	qdf_freq_t freq;
	uint16_t total_score;
	int32_t rssi;
	uint32_t etp;
};

/**
 * struct wlan_roam_scan_info  - Roam scan related information
 * @cand_ap_count: Roam candidate AP count
 * @num_scanned_frequencies: Number of scanned frequencies
 * @scan_freq: Array of scanned frequencies value in MHz
 */
struct wlan_roam_scan_info {
	uint8_t cand_ap_count;
	uint16_t num_scanned_freq;
	qdf_freq_t scan_freq[NUM_CHANNELS];
};

/**
 * struct wlan_roam_result_info  - Roam result data
 * @roam_fail_reason: Roam failure reason code defined in enum
 * wlan_roam_failure_reason_code
 * @is_roam_successful: True if roamed successfully or false if roaming failed
 */
struct wlan_roam_result_info {
	enum wlan_roam_failure_reason_code roam_fail_reason;
	bool is_roam_successful;
};

/**
 * struct wlan_roam_scan_trigger_info  - Structure to store roam scan trigger
 * related data.
 * @is_full_scan: True if the scan is Full scan. False if the roam scan is
 * partial channel map scan
 * @trigger_reason: Roam trigger reason defined by enum roam_trigger_reason
 * @trigger_sub_reason: Roam scan trigger sub reason indicating if
 * periodic/inactivity scan timer initiated roam. Defined by enum
 * roam_trigger_sub_reason
 * @cu_load:  Current connected channel load in percentage
 * @current_rssi: Connected AP RSSI in dBm
 * @rssi_threshold: Roam scan trigger threshold in dBm
 */
struct wlan_roam_trigger_info {
	bool is_full_scan;
	enum roam_trigger_reason trigger_reason;
	enum roam_trigger_sub_reason trigger_sub_reason;
	uint8_t cu_load;
	int32_t current_rssi;
	int32_t rssi_threshold;
};

/**
 * struct wlan_btm_cand_info  - BTM candidate information
 * @index: Candidate index
 * @preference: Candidate preference
 * @bssid: candidate bssid
 */
struct wlan_btm_cand_info {
	uint8_t idx;
	uint8_t preference;
	struct qdf_mac_addr bssid;
};

/**
 * struct wlan_roam_btm_info - BTM frame related logging data
 * @reason: Query Reason field. Contains one of the values defined in IEEE
 * Std 802.11‐2020 Table 9-198—Transition and Transition Query reasons
 * @mode: BTM Request Mode field
 * @sub_reason: WTC sub reason code field in the BTM WTC vendor specific IE
 * @candidate_list_count: Candidates list in the BTM frame
 * @btm_status_code: BSS Transition management status codes defined in
 * 802.11‐2020 Table 9-428—BTM status code definitions
 * @btm_delay: BSS Termination Delay field
 * @is_disassoc_imminent: Disassociation imminent bit
 * @token: dialog token. Dialog Token is a nonzero value chosen by the STA
 * while sending the BTM frame to identify the query/request/response
 * transaction
 * @validity_timer: Validity interval in TBTT
 * @disassoc_timer: Time after which the AP disassociates the STA, defined
 * in TBTT.
 * @wtc_duration: WTC duration field in minutes
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
	uint8_t token;
	uint8_t validity_timer;
	uint16_t disassoc_timer;
	uint32_t wtc_duration;
	struct qdf_mac_addr target_bssid;
};

/**
 * struct wlan_packet_info  - Data packets related info
 * @tx_status: Frame TX status defined by enum qdf_dp_tx_rx_status
 * @eap_type: EAP type. Values defined by IANA at:
 * https://www.iana.org/assignments/eap-numbers
 * @eap_len: EAP data length
 * @auth_algo: authentication algorithm number defined in IEEE Std 802.11‐2020
 * Section 9.4.1.1 Authentication Algorithm Number field.
 * @auth_seq_num: Authentication frame transaction sequence number
 * @auth_type: Authentication frame sub-type for SAE authentication. Possible
 * values:
 * 1 - SAE commit frame
 * 2 - SAE confirm frame
 * @frame_status_code: Frame status code as defined in IEEE Std
 * 802.11‐2020 Table 9-50—Status codes.
 * @seq_num: Frame sequence number
 * @rssi: Peer RSSI in dBm
 * @is_retry_frame: is frame retried
 */
struct wlan_packet_info {
	uint8_t tx_status;
	uint8_t eap_type;
	uint16_t eap_len;
	uint8_t auth_algo;
	uint8_t auth_seq_num;
	uint8_t auth_type;
	uint16_t frame_status_code;
	uint16_t seq_num;
	int32_t rssi;
	bool is_retry_frame;
};

/**
 * struct wlan_connect_info  - Connection related info
 * @ssid: SSID
 * @ssid_len: Length of the SSID
 * @bssid_hint: BSSID hint provided in the connect request
 * @freq: Frequency in MHz
 * @freq_hint: Frequency Hint in MHz
 * @akm: Auth key management suite defined in IEEE Std 802.11‐2020
 * Table 9-151—AKM suite selectors.
 * @pairwise: Pairwise suite value as defined in IEEE 802.11 2020
 * Table 12-10—Integrity and key wrap algorithms.
 * @group: Group cipher suite value as defined in
 * Table 12-10—Integrity and key wrap algorithms.
 * @group_mgmt: Group manangement cipher suite as defined in
 * Table 12-10—Integrity and key wrap algorithms.
 * @auth_type: Authentication algorithm number field as defined in
 * IEEE 802.11 - 2020 standard section 9.4.1.1
 * @conn_status: Connection failure status defined by enum
 * wlan_cm_connect_fail_reason
 * @is_bt_coex_active: Is there active bluetooth connection
 */
struct wlan_connect_info {
	char ssid[WLAN_SSID_MAX_LEN];
	uint8_t ssid_len;
	struct qdf_mac_addr bssid_hint;
	qdf_freq_t freq;
	qdf_freq_t freq_hint;
	uint32_t akm;
	uint32_t pairwise;
	uint32_t group;
	uint32_t group_mgmt;
	uint8_t auth_type;
	enum wlan_cm_connect_fail_reason conn_status;
	bool is_bt_coex_active;
};

#define WLAN_MAX_LOG_RECORDS 45
#define WLAN_MAX_LOG_LEN     256
#define WLAN_RECORDS_PER_SEC 20
#define MAX_RECORD_IN_SINGLE_EVT 5

/**
 * struct wlan_log_record  - Structure for indvidual records in the ring
 * buffer
 * @timestamp_us: Timestamp(time of the day) in microseconds
 * @fw_timestamp_us: timestamp at which roam scan was triggered
 * @ktime_us: kernel timestamp (time of the day) in microseconds
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
	uint64_t ktime_us;
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
 * struct wlan_cl_osif_cbks  - OSIF callbacks to be invoked for connectivity
 * logging
 * @wlan_connectivity_log_send_to_usr: Send the log buffer to user space
 */
struct wlan_cl_osif_cbks {
	QDF_STATUS
	(*wlan_connectivity_log_send_to_usr) (struct wlan_log_record *rec,
					      void *context,
					      uint8_t num_records);
};

/**
 * struct wlan_connectivity_log_buf_data  - Master structure to hold the
 * pointers to the ring buffers.
 * @psoc: Global psoc pointer
 * @osif_cbks: OSIF callbacks
 * @osif_cb_context: Pointer to the context to be passed to OSIF
 * callback
 * @first_record_timestamp_in_last_sec: First record timestamp
 * @sent_msgs_count: Total sent messages counter in the last 1 sec
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
	struct wlan_objmgr_psoc *psoc;
	struct wlan_cl_osif_cbks osif_cbks;
	void *osif_cb_context;
	uint64_t first_record_timestamp_in_last_sec;
	uint64_t sent_msgs_count;
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

#if defined(WLAN_FEATURE_CONNECTIVITY_LOGGING) && \
		defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * wlan_print_cached_sae_auth_logs() - Enqueue SAE authentication frame logs
 * @bssid:  BSSID
 * @vdev_id: Vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_print_cached_sae_auth_logs(struct qdf_mac_addr *bssid,
					   uint8_t vdev_id);

/**
 * wlan_is_log_record_present_for_bssid() - Check if there is existing log
 * record for the given bssid
 * @bssid: BSSID
 * @vdev_id: vdev id
 *
 * Return: true if record is present else false
 */
bool wlan_is_log_record_present_for_bssid(struct qdf_mac_addr *bssid,
					  uint8_t vdev_id);

/**
 * wlan_clear_sae_auth_logs_cache() - Clear the cached auth related logs
 * @vdev_id: vdev id
 *
 * Return: None
 */
void wlan_clear_sae_auth_logs_cache(uint8_t vdev_id);
#else
static inline
QDF_STATUS wlan_print_cached_sae_auth_logs(struct qdf_mac_addr *bssid,
					   uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline
bool wlan_is_log_record_present_for_bssid(struct qdf_mac_addr *bssid,
					  uint8_t vdev_id)
{
	return false;
}

static inline
void wlan_clear_sae_auth_logs_cache(uint8_t vdev_id)
{}
#endif

#ifdef WLAN_FEATURE_CONNECTIVITY_LOGGING
/**
 * wlan_connectivity_logging_start()  - Initialize the connectivity/roaming
 * logging buffer
 * @psoc: Global psoc pointer
 * @osif_cbks: OSIF callbacks
 * @osif_cbk_context: OSIF callback context argument
 *
 * Return: None
 */
void wlan_connectivity_logging_start(struct wlan_objmgr_psoc *psoc,
				     struct wlan_cl_osif_cbks *osif_cbks,
				     void *osif_cb_context);

/**
 * wlan_connectivity_logging_stop() - Deinitialize the connectivity logging
 * buffers and spinlocks.
 *
 * Return: None
 */
void wlan_connectivity_logging_stop(void);

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

/**
 * wlan_connectivity_mgmt_event()  - Fill and enqueue a new record
 * for management frame information.
 * @mac_hdr: 802.11 management frame header
 * @vdev_id: Vdev id
 * @status_code: Frame status code as defined in IEEE 802.11 - 2020 standard
 * section 9.4.1.9
 * @tx_status: Frame TX status defined by enum qdf_dp_tx_rx_status
 * @peer_rssi: Peer RSSI in dBm
 * @auth_algo: Authentication algorithm number field as defined in IEEE 802.11 -
 * 2020 standard section 9.4.1.1
 * @auth_type: indicates SAE authentication frame type. Possible values are:
 * 1 - SAE commit frame
 * 2 - SAE confirm frame
 * @auth_seq: Authentication frame transaction sequence number as defined in
 * IEEE 802.11 - 2020 standard section 9.4.1.2
 * @tag: Record type main tag
 *
 * Return: QDF_STATUS
 */
void
wlan_connectivity_mgmt_event(struct wlan_frame_hdr *mac_hdr,
			     uint8_t vdev_id, uint16_t status_code,
			     enum qdf_dp_tx_rx_status tx_status,
			     int8_t peer_rssi,
			     uint8_t auth_algo, uint8_t auth_type,
			     uint8_t auth_seq,
			     enum wlan_main_tag tag);
#else
static inline
void wlan_connectivity_logging_start(struct wlan_objmgr_psoc *psoc,
				     struct wlan_cl_osif_cbks *osif_cbks,
				     void *osif_cb_context)
{}

static inline void wlan_connectivity_logging_stop(void)
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

static inline void
wlan_connectivity_mgmt_event(struct wlan_frame_hdr *mac_hdr,
			     uint8_t vdev_id, uint16_t status_code,
			     enum qdf_dp_tx_rx_status tx_status,
			     int8_t peer_rssi,
			     uint8_t auth_algo, uint8_t auth_type,
			     uint8_t auth_seq,
			     enum wlan_main_tag tag)
{}
#endif
#endif /* _WLAN_CONNECTIVITY_LOGGING_H_ */

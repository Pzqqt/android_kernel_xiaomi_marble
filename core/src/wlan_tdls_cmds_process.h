/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_tdls_cmds_process.h
 *
 * TDLS north bound commands include file
 */

#ifndef _WLAN_TDLS_CMDS_PROCESS_H_
#define _WLAN_TDLS_CMDS_PROCESS_H_

#define TDLS_IS_SETUP_ACTION(action) \
	((TDLS_SETUP_REQUEST <= action) && \
	(TDLS_SETUP_CONFIRM >= action))


/**
 * enum tdls_add_oper - add peer type
 * @TDLS_OPER_NONE: none
 * @TDLS_OPER_ADD: add new peer
 * @TDLS_OPER_UPDATE: used to update peer
 */
enum tdls_add_oper {
	TDLS_OPER_NONE,
	TDLS_OPER_ADD,
	TDLS_OPER_UPDATE
};

/**
 * enum legacy_result_code - defined to comply with tSirResultCodes, need refine
 *                           when mlme converged.
 * @legacy_result_success: success
 * @legacy_result_max: max result value
 */
enum legacy_result_code {
	legacy_result_success,
	legacy_result_max = 0x7FFFFFFF
};

/**
 * struct tdls_send_mgmt_rsp - TDLS Response struct PE --> TDLS module
 *                           same as struct tSirSmeRsp
 * @message_type: message type eWNI_SME_TDLS_SEND_MGMT_RSP
 * @length: message length
 * @session_id: session id
 * @transaction_id: transaction id
 * @status_code: status code as tSirResultCodes
 * @psoc: soc object
 */
struct tdls_send_mgmt_rsp {
	uint16_t message_type;
	uint16_t length;
	uint8_t session_id;
	uint16_t transaction_id;
	enum legacy_result_code status_code;
	struct wlan_objmgr_psoc *psoc;
};

/**
 * struct tdls_mgmt_tx_completion_ind - TDLS TX completion PE --> TDLS module
 *                           same as struct sSirMgmtTxCompletionInd
 * @message_type: message type eWNI_SME_MGMT_FRM_TX_COMPLETION_IND
 * @length: message length
 * @session_id: session id
 * @tx_complete_status: tx complete status
 * @psoc: soc object
 */
struct tdls_mgmt_tx_completion_ind {
	uint16_t message_type;
	uint16_t length;
	uint8_t session_id;      /* Session ID */
	uint32_t tx_complete_status;
	struct wlan_objmgr_psoc *psoc;
};

/**
 * struct tdls_add_sta_req - TDLS request struct TDLS module --> PE
 *                           same as struct tSirTdlsAddStaReq;
 * @message_type: eWNI_SME_TDLS_ADD_STA_REQ
 * @length: message length
 * @session_id: session id
 * @transaction_id: transaction id for cmd
 * @bssid: bssid
 * @tdls_oper: add peer type
 * @peermac: MAC address for TDLS peer
 * @capability: mac capability as sSirMacCapabilityInfo
 * @extn_capability: extent capability
 * @supported_rates_length: rates length
 * @supported_rates: supported rates
 * @htcap_present: ht capability present
 * @ht_cap: ht capability
 * @vhtcap_present: vht capability present
 * @vht_cap: vht capability
 * @uapsd_queues: uapsd queue as sSirMacQosInfoStation
 * @max_sp: maximum service period
 */
struct tdls_add_sta_req {
	uint16_t message_type;
	uint16_t length;
	uint8_t session_id;
	uint16_t transaction_id;
	struct qdf_mac_addr bssid;
	enum tdls_add_oper tdls_oper;
	struct qdf_mac_addr peermac;
	uint16_t capability;
	uint8_t extn_capability[WLAN_MAC_MAX_EXTN_CAP];
	uint8_t supported_rates_length;
	uint8_t supported_rates[WLAN_MAC_MAX_SUPP_RATES];
	uint8_t htcap_present;
	struct htcap_cmn_ie ht_cap;
	uint8_t vhtcap_present;
	struct vhtcap vht_cap;
	uint8_t uapsd_queues;
	uint8_t max_sp;
};

/**
 * struct tdls_add_sta_rsp - TDLS Response struct PE --> TDLS module
 *                           same as struct sSirTdlsAddStaRsp
 * @message_type: message type eWNI_SME_TDLS_ADD_STA_RSP
 * @length: message length
 * @status_code: status code as tSirResultCodes
 * @peermac: MAC address of the TDLS peer
 * @session_id: session id
 * @sta_id: sta id
 * @sta_type: sta type
 * @tdls_oper: add peer type
 * @psoc: soc object
 */
struct tdls_add_sta_rsp {
	uint16_t message_type;
	uint16_t length;
	QDF_STATUS status_code;
	struct qdf_mac_addr peermac;
	uint8_t session_id;
	uint16_t sta_id;
	uint16_t sta_type;
	enum tdls_add_oper tdls_oper;
	struct wlan_objmgr_psoc *psoc;
};

/**
 * struct tdls_del_sta_req - TDLS Request struct TDLS module --> PE
 *                           same as sSirTdlsDelStaReq
 * @message_type: message type eWNI_SME_TDLS_DEL_STA_REQ
 * @length: message length
 * @session_id: session id
 * @transaction_id: transaction id for cmd
 * @bssid: bssid
 * @peermac: MAC address of the TDLS peer
 */
struct tdls_del_sta_req {
	uint16_t message_type;
	uint16_t length;
	uint8_t session_id;
	uint16_t transaction_id;
	struct qdf_mac_addr bssid;
	struct qdf_mac_addr peermac;
};

/**
 * struct tdls_del_sta_rsp - TDLS Response struct PE --> TDLS module
 *                           same as sSirTdlsDelStaRsp
 * @message_type: message type eWNI_SME_TDLS_DEL_STA_RSP
 * @length: message length
 * @session_id: session id
 * @status_code: status code as tSirResultCodes
 * @peermac: MAC address of the TDLS peer
 * @sta_id: sta id
 * @psoc: soc object
 */
struct tdls_del_sta_rsp {
	uint16_t message_type;
	uint16_t length;
	uint8_t session_id;
	QDF_STATUS status_code;
	struct qdf_mac_addr peermac;
	uint16_t sta_id;
	struct wlan_objmgr_psoc *psoc;
};

/**
 * tdls_process_add_peer() - add TDLS peer
 * @req: TDLS add peer request
 *
 * Return: QDF_STATUS_SUCCESS if success; other value if failed
 */
QDF_STATUS tdls_process_add_peer(struct tdls_add_peer_request *req);

/**
 * tdls_process_del_peer() - del TDLS peer
 * @req: TDLS del peer request
 *
 * Return: QDF_STATUS_SUCCESS if success; other value if failed
 */
QDF_STATUS tdls_process_del_peer(struct tdls_oper_request *req);

/**
 * tdls_process_enable_link() - enable TDLS link
 * @req: TDLS enable link request
 *
 * Return: QDF_STATUS_SUCCESS if success; other value if failed
 */
QDF_STATUS tdls_process_enable_link(struct tdls_oper_request *req);

/**
 * tdls_process_setup_peer() - process configure an externally
 *                                    controllable TDLS peer
 * @req: TDLS configure force peer request
 *
 * Return: QDF_STATUS_SUCCESS if success; other values if failed
 */
QDF_STATUS tdls_process_setup_peer(struct tdls_oper_request *req);

/**
 * tdls_process_remove_force_peer() - process remove an externally controllable
 *                                    TDLS peer
 * @req: TDLS operation request
 *
 * Return: QDF_STATUS_SUCCESS if success; other values if failed
 */
QDF_STATUS tdls_process_remove_force_peer(struct tdls_oper_request *req);

/**
 * tdls_process_update_peer() - update TDLS peer
 * @req: TDLS update peer request
 *
 * Return: QDF_STATUS_SUCCESS if success; other value if failed
 */
QDF_STATUS tdls_process_update_peer(struct tdls_update_peer_request *req);

/**
 * tdls_process_antenna_switch() - handle TDLS antenna switch
 * @req: TDLS antenna switch request
 *
 * Rely on callback to indicate the antenna switch state to caller.
 *
 * Return: QDF_STATUS_SUCCESS if success; other value if failed.
 */
QDF_STATUS tdls_process_antenna_switch(struct tdls_antenna_switch_request *req);

/**
 * tdls_antenna_switch_flush_callback() - flush TDLS antenna switch request
 * @msg: scheduler message contains tdls antenna switch event
 *
 * This function call is invoked when scheduler thread is going down
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tdls_antenna_switch_flush_callback(struct scheduler_msg *msg);

/**
 * tdls_pe_del_peer() - send TDLS delete peer request to PE
 * @req: TDLS delete peer request
 *
 * Return: QDF status
 */
QDF_STATUS tdls_pe_del_peer(struct tdls_del_peer_request *req);

/**
 * tdls_process_add_peer_rsp() - handle response for add or update TDLS peer
 * @rsp: TDLS add peer response
 *
 * Return: QDF status
 */
QDF_STATUS tdls_process_add_peer_rsp(struct tdls_add_sta_rsp *rsp);

/**
 * tdls_reset_nss() - reset tdls nss parameters
 * @tdls_soc: TDLS soc object
 * @action_code: action code
 *
 * Return: None
 */
void tdls_reset_nss(struct tdls_soc_priv_obj *tdls_soc,
				  uint8_t action_code);

/**
 * tdls_release_serialization_command() - TDLS wrapper to
 * relases serialization command.
 * @vdev: Object manager vdev
 * @type: command to release.
 *
 * Return: None
 */

void
tdls_release_serialization_command(struct wlan_objmgr_vdev *vdev,
				   enum wlan_serialization_cmd_type type);

/**
 * tdls_set_cap() - set TDLS capability type
 * @tdls_vdev: tdls vdev object
 * @mac: peer mac address
 * @cap: TDLS capability type
 *
 * Return: 0 if successful or negative errno otherwise
 */
int tdls_set_cap(struct tdls_vdev_priv_obj *tdls_vdev, const uint8_t *mac,
			  enum tdls_peer_capab cap);

/**
 * tdls_process_send_mgmt_rsp() - handle response for send mgmt
 * @rsp: TDLS send mgmt response
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
QDF_STATUS tdls_process_send_mgmt_rsp(struct tdls_send_mgmt_rsp *rsp);

/**
 * tdls_send_mgmt_tx_completion() - process tx completion
 * @tx_complete: TDLS mgmt completion info
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
QDF_STATUS tdls_send_mgmt_tx_completion(
			struct tdls_mgmt_tx_completion_ind *tx_complete);

/**
 * tdls_process_add_peer_rsp() - handle response for delete TDLS peer
 * @rsp: TDLS delete peer response
 *
 * Return: QDF status
 */
QDF_STATUS tdls_process_del_peer_rsp(struct tdls_del_sta_rsp *rsp);

/**
 * tdls_process_should_discover() - handle tdls should_discover event
 * @vdev: vdev object
 * @evt: event info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tdls_process_should_discover(struct wlan_objmgr_vdev *vdev,
					struct tdls_event_info *evt);

/**
 * tdls_process_should_teardown() - handle tdls should_teardown event
 * @vdev: vdev object
 * @evt: event info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tdls_process_should_teardown(struct wlan_objmgr_vdev *vdev,
					struct tdls_event_info *evt);

/**
 * tdls_process_connection_tracker_notify() -handle tdls connect tracker notify
 * @vdev: vdev object
 * @evt: event info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tdls_process_connection_tracker_notify(struct wlan_objmgr_vdev *vdev,
						  struct tdls_event_info *evt);

/**
 * tdls_validate_mgmt_request() -validate mgmt request
 * @tdls_validate: action frame request
 *
 * Return: 0 for success or -EINVAL otherwise
 */
int tdls_validate_mgmt_request(struct tdls_action_frame_request *tdls_mgmt_req);

/**
 * tdls_set_responder() - Set/clear TDLS peer's responder role
 * @set_req: set responder request
 *
 * Return: 0 for success or -EINVAL otherwise
 */
int tdls_set_responder(struct tdls_set_responder_req *set_req);

/**
 * tdls_decrement_peer_count() - decrement connected TDLS peer counter
 * @soc_obj: TDLS soc object
 *
 * Used in scheduler thread context, no lock needed.
 *
 * Return: None.
 */
void tdls_decrement_peer_count(struct tdls_soc_priv_obj *soc_obj);

/**
 * wlan_tdls_offchan_parms_callback() - Callback to release ref count
 * @vdev: vdev object
 *
 * Return: none
 */
void wlan_tdls_offchan_parms_callback(struct wlan_objmgr_vdev *vdev);

/**
 * tdls_process_set_offchannel() - Handle set offchannel request for TDLS
 * @req: TDLS set offchannel request
 *
 * Return: int status
 */
int tdls_process_set_offchannel(struct tdls_set_offchannel *req);

/**
 * tdls_process_set_offchan_mode() - Handle set offchan mode request for TDLS
 * @req: TDLS set offchannel mode request
 *
 * Return: int status
 */
int tdls_process_set_offchan_mode(struct tdls_set_offchanmode *req);

/**
 * tdls_process_set_secoffchanneloffset() - Handle set sec offchannel
 * offset request for TDLS
 * @req: TDLS set secoffchannel offchannel request
 *
 * Return: int status
 */
int tdls_process_set_secoffchanneloffset(
		struct tdls_set_secoffchanneloffset *req);

#endif

/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * @ucast_sig: unicast signature
 * @bcast_sig: broadcast signature
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
	uint8_t ucast_sig;
	uint8_t bcast_sig;
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
int tdls_validate_mgmt_request(struct tdls_validate_action_req *tdls_validate);

/**
 * tdls_set_responder() - Set/clear TDLS peer's responder role
 * @set_req: set responder request
 *
 * Return: 0 for success or -EINVAL otherwise
 */
int tdls_set_responder(struct tdls_set_responder_req *set_req);

#endif

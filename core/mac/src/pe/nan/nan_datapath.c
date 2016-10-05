/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: nan_datapath.c
 *
 * MAC NAN Data path API implementation
 */

#include "lim_utils.h"
#include "lim_api.h"
#include "lim_assoc_utils.h"
#include "nan_datapath.h"
#include "lim_types.h"
#include "lim_send_messages.h"
#include "wma_nan_datapath.h"

/**
 * lim_send_ndp_event_to_sme() - generic function to prepare and send NDP
 * message to SME.
 * @mac_ctx: handle to mac context structure
 * @msg_type: sme message type to send
 * @body_ptr: buffer
 * @len: buffer length
 * @body_val: value
 *
 * Return: None
 */
static void lim_send_ndp_event_to_sme(tpAniSirGlobal mac_ctx, uint32_t msg_type,
				void *body_ptr, uint32_t len, uint32_t body_val)
{
	tSirMsgQ mmh_msg = {0};

	mmh_msg.type = msg_type;
	if (len && body_ptr) {
		mmh_msg.bodyptr = qdf_mem_malloc(len);
		if (NULL == mmh_msg.bodyptr) {
			lim_log(mac_ctx, LOGE, FL("Malloc failed"));
			return;
		}
		qdf_mem_copy(mmh_msg.bodyptr, body_ptr, len);
	} else {
		mmh_msg.bodyval = body_val;
	}
	lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);
}

/**
 * lim_add_ndi_peer() - Function to add ndi peer
 * @mac_ctx: handle to mac structure
 * @vdev_id: vdev id on which peer is added
 * @peer_mac_addr: peer to be added
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
static QDF_STATUS lim_add_ndi_peer(tpAniSirGlobal mac_ctx,
	uint32_t vdev_id, struct qdf_mac_addr peer_mac_addr)
{
	tpPESession session;
	tpDphHashNode sta_ds;
	uint16_t assoc_id, peer_idx;
	tSirRetStatus status;

	session = pe_find_session_by_sme_session_id(mac_ctx,
						vdev_id);
	if (session == NULL) {
		/* couldn't find session */
		return QDF_STATUS_E_FAILURE;
	}
	sta_ds = dph_lookup_hash_entry(mac_ctx,
				peer_mac_addr.bytes,
				&assoc_id, &session->dph.dphHashTable);
	/* peer exists, don't do anything */
	if (sta_ds != NULL) {
		lim_log(mac_ctx, LOGE, FL("NDI Peer already exists!!"));
		return QDF_STATUS_SUCCESS;
	}
	lim_log(mac_ctx, LOG1,
		FL("Need to create NDI Peer :" MAC_ADDRESS_STR),
		MAC_ADDR_ARRAY(peer_mac_addr.bytes));
	peer_idx = lim_assign_peer_idx(mac_ctx, session);
	sta_ds = dph_add_hash_entry(mac_ctx, peer_mac_addr.bytes, peer_idx,
			&session->dph.dphHashTable);
	if (sta_ds == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("Couldn't add dph entry"));
		/* couldn't add dph entry */
		return QDF_STATUS_E_FAILURE;
	}
	/* wma decides NDI mode from wma->inferface struct */
	sta_ds->staType = STA_ENTRY_NDI_PEER;
	status = lim_add_sta(mac_ctx, sta_ds, false, session);
	if (eSIR_SUCCESS != status) {
		/* couldn't add peer */
		lim_log(mac_ctx, LOGE,
			FL("limAddSta failed status: %d"),
			status);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_handle_ndp_indication_event() - Function to handle SIR_HAL_NDP_INDICATION
 * event from WMA
 * @mac_ctx: handle to mac structure
 * @ndp_ind: ndp indication event params
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
static QDF_STATUS lim_handle_ndp_indication_event(tpAniSirGlobal mac_ctx,
					struct ndp_indication_event *ndp_ind)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	lim_log(mac_ctx, LOG1,
		FL("role: %d, vdev: %d, peer_mac_addr "MAC_ADDRESS_STR),
		ndp_ind->role, ndp_ind->vdev_id,
		MAC_ADDR_ARRAY(ndp_ind->peer_mac_addr.bytes));

	if ((ndp_ind->role == NDP_ROLE_INITIATOR) ||
	   ((NDP_ROLE_RESPONDER == ndp_ind->role) &&
	   (NDP_ACCEPT_POLICY_ALL == ndp_ind->policy))) {
		status = lim_add_ndi_peer(mac_ctx, ndp_ind->vdev_id,
				ndp_ind->peer_mac_addr);
		if (QDF_STATUS_SUCCESS != status) {
			lim_log(mac_ctx, LOGE,
				FL("Couldn't add ndi peer, ndp_role: %d"),
				ndp_ind->role);
			goto ndp_indication_failed;
		}
	}
	if (NDP_ROLE_RESPONDER == ndp_ind->role)
		lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_INDICATION,
			ndp_ind, sizeof(*ndp_ind), 0);
	/*
	 * With NDP indication if peer does not exists already add_sta is
	 * executed resulting in new peer else no action is taken. Note that
	 * new_peer event is not necessary event and should not be sent if case
	 * anything fails in this function. Rather eWNI_SME_NDP_CONFIRM_IND is
	 * used to indicate success of final operation and abscence of it can be
	 * used by service layer to identify failure.
	 */
ndp_indication_failed:
	/*
	 * Free config if failure or for NDP_ROLE_INITIATOR role
	 * As for success responder case this info is sent till HDD
	 * and will be freed in sme.
	 */
	if (status != QDF_STATUS_SUCCESS ||
			NDP_ROLE_INITIATOR == ndp_ind->role) {
		qdf_mem_free(ndp_ind->ndp_config.ndp_cfg);
		qdf_mem_free(ndp_ind->ndp_info.ndp_app_info);
		ndp_ind->ndp_config.ndp_cfg = NULL;
		ndp_ind->ndp_info.ndp_app_info = NULL;
	}
	return status;
}

/**
 * lim_ndp_responder_rsp_handler() - Handler for NDP responder rsp
 * @mac_ctx: handle to mac structure
 * @ndp_rsp: pointer to rsp message
 * @bodyval: value
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
static QDF_STATUS lim_ndp_responder_rsp_handler(tpAniSirGlobal mac_ctx,
	struct ndp_responder_rsp_event *rsp_ind, uint32_t bodyval)
{
	QDF_STATUS ret_val = QDF_STATUS_SUCCESS;

	if ((NULL == rsp_ind) || bodyval) {
		lim_log(mac_ctx, LOGE,
			FL("rsp_ind is NULL or bodyval %d"), bodyval);
		/* msg to unblock SME, but not send rsp to HDD */
		bodyval = true;
		ret_val = QDF_STATUS_E_INVAL;
		goto responder_rsp;
	}

	if (QDF_STATUS_SUCCESS == rsp_ind->status) {
		ret_val = lim_add_ndi_peer(mac_ctx, rsp_ind->vdev_id,
				rsp_ind->peer_mac_addr);
		if (QDF_STATUS_SUCCESS != ret_val) {
			lim_log(mac_ctx, LOGE,
				FL("Couldn't add ndi peer"));
			rsp_ind->status = QDF_STATUS_E_FAILURE;
		}
	}

responder_rsp:
	/* send eWNI_SME_NDP_RESPONDER_RSP */
	lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_RESPONDER_RSP,
				bodyval ? NULL : rsp_ind,
				bodyval ? 0 : sizeof(*rsp_ind), bodyval);
	return ret_val;
}

/**
 * lim_ndp_delete_peer_by_addr() - Delete NAN data peer, given addr and vdev_id
 * @mac_ctx: handle to mac context
 * @vdev_id: vdev_id on which peer was added
 * @peer_ndi_mac_addr: mac addr of peer
 * This function deletes a peer if there was NDP_Confirm with REJECT
 *
 * Return: None
 */
void lim_ndp_delete_peer_by_addr(tpAniSirGlobal mac_ctx, uint8_t vdev_id,
				 struct qdf_mac_addr peer_ndi_mac_addr)
{
	tpPESession session;
	tpDphHashNode sta_ds;
	uint16_t peer_idx;

	lim_log(mac_ctx, LOG1,
		FL("deleting peer: "MAC_ADDRESS_STR" confirm rejected"),
		MAC_ADDR_ARRAY(peer_ndi_mac_addr.bytes));

	session = pe_find_session_by_sme_session_id(mac_ctx, vdev_id);
	if (!session || (session->bssType != eSIR_NDI_MODE)) {
		lim_log(mac_ctx, LOGE,
			FL("PE session is NULL or non-NDI for sme session %d"),
			vdev_id);
		return;
	}

	sta_ds = dph_lookup_hash_entry(mac_ctx, peer_ndi_mac_addr.bytes,
				    &peer_idx, &session->dph.dphHashTable);
	if (!sta_ds) {
		lim_log(mac_ctx, LOGE, FL("Unknown NDI Peer"));
		return;
	}
	if (sta_ds->staType != STA_ENTRY_NDI_PEER) {
		lim_log(mac_ctx, LOGE, FL("Non-NDI Peer ignored"));
		return;
	}
	/*
	 * Call lim_del_sta() with response required set true. Hence
	 * DphHashEntry will be deleted after receiving that response.
	 */

	lim_del_sta(mac_ctx, sta_ds, true, session);
}

/**
 * lim_ndp_delete_peers() - Delete NAN data peers
 * @mac_ctx: handle to mac context
 * @ndp_map: NDP instance/peer map
 * @num_peers: number of peers entries in peer_map
 * This function deletes a peer if there are no active NDPs left with that peer
 *
 * Return: None
 */
static void lim_ndp_delete_peers(tpAniSirGlobal mac_ctx,
				struct peer_ndp_map *ndp_map, uint8_t num_peers)
{
	tpDphHashNode sta_ds = NULL;
	uint16_t deleted_num = 0;
	int i, j;
	tpPESession session;
	struct qdf_mac_addr *deleted_peers;
	uint16_t peer_idx;
	bool found;

	deleted_peers = qdf_mem_malloc(num_peers * sizeof(*deleted_peers));
	if (!deleted_peers) {
		lim_log(mac_ctx, LOGE, FL("Memory allocation failed"));
		return;
	}

	qdf_mem_zero(deleted_peers, num_peers * sizeof(*deleted_peers));
	for (i = 0; i < num_peers; i++) {
		lim_log(mac_ctx, LOG1,
			FL("ndp_map[%d]: MAC: " MAC_ADDRESS_STR " num_active %d"),
			i,
			MAC_ADDR_ARRAY(ndp_map[i].peer_ndi_mac_addr.bytes),
			ndp_map[i].num_active_ndp_sessions);

		/* Do not delete a peer with active NDPs */
		if (ndp_map[i].num_active_ndp_sessions > 0)
			continue;

		session = pe_find_session_by_sme_session_id(mac_ctx,
						ndp_map[i].vdev_id);
		if (!session || (session->bssType != eSIR_NDI_MODE)) {
			lim_log(mac_ctx, LOGE,
				FL("PE session is NULL or non-NDI for sme session %d"),
				ndp_map[i].vdev_id);
			continue;
		}

		/* Check if this peer is already in the deleted list */
		found = false;
		for (j = 0; j < deleted_num && !found; j++) {
			if (!qdf_mem_cmp(
				&deleted_peers[j].bytes,
				&ndp_map[i].peer_ndi_mac_addr.bytes,
				QDF_MAC_ADDR_SIZE)) {
				found = true;
				break;
			}
		}
		if (found)
			continue;

		sta_ds = dph_lookup_hash_entry(mac_ctx,
				ndp_map[i].peer_ndi_mac_addr.bytes,
				&peer_idx, &session->dph.dphHashTable);
		if (!sta_ds) {
			lim_log(mac_ctx, LOGE, FL("Unknown NDI Peer"));
			continue;
		}
		if (sta_ds->staType != STA_ENTRY_NDI_PEER) {
			lim_log(mac_ctx, LOGE,
				FL("Non-NDI Peer ignored"));
			continue;
		}
		/*
		 * Call lim_del_sta() with response required set true.
		 * Hence DphHashEntry will be deleted after receiving
		 * that response.
		 */
		lim_del_sta(mac_ctx, sta_ds, true, session);
		qdf_copy_macaddr(&deleted_peers[deleted_num++],
			&ndp_map[i].peer_ndi_mac_addr);
	}
	qdf_mem_free(deleted_peers);
}

/**
 * lim_ndp_end_indication_handler() - Handler for NDP end indication
 * @mac_ctx: handle to mac context
 * @ind_buf: pointer to indication buffer
 *
 * It deletes peers from ndp_map. Response of that operation goes
 * to LIM and HDD. But peer information does not go to service layer.
 * ndp_id_list is sent to service layer; it is not interpreted by the
 * driver.
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
static QDF_STATUS lim_ndp_end_indication_handler(tpAniSirGlobal mac_ctx,
					uint32_t *ind_buf)
{

	struct ndp_end_indication_event *ndp_event_buf =
		(struct ndp_end_indication_event *)ind_buf;
	int buf_size;

	if (!ind_buf) {
		lim_log(mac_ctx, LOGE, FL("NDP end indication buffer is NULL"));
		return QDF_STATUS_E_INVAL;
	}
	lim_ndp_delete_peers(mac_ctx, ndp_event_buf->ndp_map,
			ndp_event_buf->num_ndp_ids);

	buf_size = sizeof(*ndp_event_buf) + ndp_event_buf->num_ndp_ids *
			sizeof(ndp_event_buf->ndp_map[0]);
	lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_END_IND,
		ndp_event_buf, buf_size, false);

	return QDF_STATUS_SUCCESS;
}

/**
 * lim_process_ndi_del_sta_rsp() - Handle WDA_DELETE_STA_RSP in eLIM_NDI_ROLE
 * @mac_ctx: Global MAC context
 * @lim_msg: LIM message
 * @pe_session: PE session
 *
 * Return: None
 */
void lim_process_ndi_del_sta_rsp(tpAniSirGlobal mac_ctx, tpSirMsgQ lim_msg,
						tpPESession pe_session)
{
	tpDeleteStaParams del_sta_params = (tpDeleteStaParams) lim_msg->bodyptr;
	tpDphHashNode sta_ds;
	tSirResultCodes status = eSIR_SME_SUCCESS;
	struct sme_ndp_peer_ind peer_ind;

	if (!del_sta_params) {
		lim_log(mac_ctx, LOGE,
			FL("del_sta_params is NULL"));
		return;
	}
	if (!LIM_IS_NDI_ROLE(pe_session)) {
		lim_log(mac_ctx, LOGE,
		FL("Session %d is not NDI role"), del_sta_params->assocId);
		status = eSIR_SME_REFUSED;
		goto skip_event;
	}

	sta_ds = dph_get_hash_entry(mac_ctx, del_sta_params->assocId,
			&pe_session->dph.dphHashTable);
	if (!sta_ds) {
		lim_log(mac_ctx, LOGE,
			FL("DPH Entry for STA %X is missing."),
			del_sta_params->assocId);
		status = eSIR_SME_REFUSED;
		goto skip_event;
	}

	if (QDF_STATUS_SUCCESS != del_sta_params->status) {
		lim_log(mac_ctx, LOGE, FL("DEL STA failed!"));
		status = eSIR_SME_REFUSED;
		goto skip_event;
	}
	lim_log(mac_ctx, LOG1,
		FL("Deleted STA AssocID %d staId %d MAC " MAC_ADDRESS_STR),
		sta_ds->assocId, sta_ds->staIndex,
		MAC_ADDR_ARRAY(sta_ds->staAddr));

	/*
	 * Copy peer info in del peer indication before
	 * lim_delete_dph_hash_entry is called as this will be lost.
	 */
	peer_ind.msg_len = sizeof(peer_ind);
	peer_ind.msg_type = eWNI_SME_NDP_PEER_DEPARTED_IND;
	peer_ind.session_id = pe_session->smeSessionId;
	peer_ind.sta_id = sta_ds->staIndex;
	qdf_mem_copy(&peer_ind.peer_mac_addr.bytes,
		sta_ds->staAddr, sizeof(tSirMacAddr));

	lim_release_peer_idx(mac_ctx, sta_ds->assocId, pe_session);
	lim_delete_dph_hash_entry(mac_ctx, sta_ds->staAddr, sta_ds->assocId,
			pe_session);
	pe_session->limMlmState = eLIM_MLM_IDLE_STATE;

	lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_PEER_DEPARTED_IND,
				&peer_ind, sizeof(peer_ind), false);

skip_event:
	qdf_mem_free(del_sta_params);
	lim_msg->bodyptr = NULL;
}

/**
 * lim_handle_ndp_event_message() - Handler for NDP events/RSP from WMA
 * @mac_ctx: handle to mac structure
 * @msg: pointer to message
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS lim_handle_ndp_event_message(tpAniSirGlobal mac_ctx, cds_msg_t *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (msg->type) {
	case SIR_HAL_NDP_CONFIRM: {
		struct ndp_confirm_event *ndp_confirm = msg->bodyptr;
		if (ndp_confirm->rsp_code != NDP_RESPONSE_ACCEPT &&
			ndp_confirm->num_active_ndps_on_peer == 0) {
			/*
			 * This peer was created at ndp_indication but
			 * ndp_confirm failed, so it needs to be deleted
			 */
			lim_log(mac_ctx, LOGE,
				FL("NDP confirm with reject and no active ndp sessions. deleting peer: "MAC_ADDRESS_STR" on vdev_id: %d"),
				MAC_ADDR_ARRAY(
					ndp_confirm->peer_ndi_mac_addr.bytes),
				ndp_confirm->vdev_id);
			lim_ndp_delete_peer_by_addr(mac_ctx,
						ndp_confirm->vdev_id,
						ndp_confirm->peer_ndi_mac_addr);
		}
		lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_CONFIRM_IND,
				msg->bodyptr, sizeof(*ndp_confirm),
				msg->bodyval);
		break;
	}
	case SIR_HAL_NDP_INITIATOR_RSP:
		lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_INITIATOR_RSP,
				msg->bodyptr, sizeof(struct ndp_initiator_rsp),
				msg->bodyval);
		break;
	case SIR_HAL_NDP_INDICATION: {
		struct ndp_indication_event *ndp_ind = msg->bodyptr;
		status = lim_handle_ndp_indication_event(mac_ctx, ndp_ind);
		break;
	}
	case SIR_HAL_NDP_RESPONDER_RSP:
		status = lim_ndp_responder_rsp_handler(mac_ctx, msg->bodyptr,
					msg->bodyval);
		break;
	case SIR_HAL_NDP_END_RSP: {
		lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_END_RSP,
					  msg->bodyptr,
					  sizeof(struct ndp_end_rsp_event),
					  msg->bodyval);
		break;
	}
	case SIR_HAL_NDP_END_IND:
		status = lim_ndp_end_indication_handler(mac_ctx, msg->bodyptr);
		break;
	default:
		lim_log(mac_ctx, LOGE,
			FL("Unhandled NDP event: %d"), msg->type);
		status = QDF_STATUS_E_NOSUPPORT;
		break;
	}
	/*
	 * No need to free body pointer, since the function is serving purpose
	 * of callback and bodyptr if allocated is freed by caller itself
	 */
	return status;
}

/**
 * lim_process_sme_ndp_initiator_req() - Handler for eWNI_SME_NDP_INITIATOR_REQ
 * from SME.
 * @mac_ctx: handle to mac structure
 * @ndp_msg: ndp initiator request msg
 *
 * Return: Status of operation
 */
QDF_STATUS lim_process_sme_ndp_initiator_req(tpAniSirGlobal mac_ctx,
					     void *ndp_msg)
{
	tSirMsgQ msg;
	QDF_STATUS status;

	struct sir_sme_ndp_initiator_req *sme_req =
		(struct sir_sme_ndp_initiator_req *)ndp_msg;
	struct ndp_initiator_req *wma_req;

	if (NULL == ndp_msg) {
		lim_log(mac_ctx, LOGE, FL("invalid ndp_req"));
		status = QDF_STATUS_E_INVAL;
		goto send_initiator_rsp;
	}
	wma_req = qdf_mem_malloc(sizeof(*wma_req));
	if (wma_req == NULL) {
		lim_log(mac_ctx, LOGE, FL("malloc failed"));
		status = QDF_STATUS_E_NOMEM;
		goto send_initiator_rsp;
	}

	qdf_mem_copy(wma_req, &sme_req->req, sizeof(*wma_req));
	msg.type = SIR_HAL_NDP_INITIATOR_REQ;
	msg.reserved = 0;
	msg.bodyptr = wma_req;
	msg.bodyval = 0;

	lim_log(mac_ctx, LOG1, FL("sending WDA_NDP_INITIATOR_REQ to WMA"));
	MTRACE(mac_trace_msg_tx(mac_ctx, NO_SESSION, msg.type));

	if (eSIR_SUCCESS != wma_post_ctrl_msg(mac_ctx, &msg))
		lim_log(mac_ctx, LOGP, FL("wma_post_ctrl_msg failed"));

	return QDF_STATUS_SUCCESS;
send_initiator_rsp:
	/* msg to unblock SME, but not send rsp to HDD */
	lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_INITIATOR_RSP,
				  NULL, 0, true);
	return status;
}

/**
 * lim_process_sme_ndp_responder_req() - Handler for NDP responder req
 * @mac_ctx: handle to mac structure
 * @ndp_msg: pointer to message
 *
 * Return: QDF_STATUS_SUCCESS on success or failure code in case of failure
 */
static QDF_STATUS lim_process_sme_ndp_responder_req(tpAniSirGlobal mac_ctx,
	struct sir_sme_ndp_responder_req *lim_msg)
{
	tSirMsgQ msg;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct ndp_responder_req *responder_req;

	if (NULL == lim_msg) {
		lim_log(mac_ctx, LOGE, FL("ndp_msg is NULL"));
		status = QDF_STATUS_E_INVAL;
		goto send_failure_rsp;
	}
	responder_req = qdf_mem_malloc(sizeof(*responder_req));
	if (NULL == responder_req) {
		lim_log(mac_ctx, LOGE,
			FL("Unable to allocate memory for responder_req"));
		status = QDF_STATUS_E_NOMEM;
		goto send_failure_rsp;
	}
	qdf_mem_copy(responder_req, &lim_msg->req, sizeof(*responder_req));
	msg.type = SIR_HAL_NDP_RESPONDER_REQ;
	msg.reserved = 0;
	msg.bodyptr = responder_req;
	msg.bodyval = 0;

	lim_log(mac_ctx, LOG1, FL("sending SIR_HAL_NDP_RESPONDER_REQ to WMA"));
	MTRACE(mac_trace_msg_tx(mac_ctx, NO_SESSION, msg.type));

	if (eSIR_SUCCESS != wma_post_ctrl_msg(mac_ctx, &msg)) {
		lim_log(mac_ctx, LOGP, FL("wma_post_ctrl_msg failed"));
		status = QDF_STATUS_E_FAILURE;
		qdf_mem_free(responder_req);
		goto send_failure_rsp;
	}
	return status;
send_failure_rsp:
	/* msg to unblock SME, but not send rsp to HDD */
	lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_RESPONDER_RSP,
				NULL, 0, true);
	return status;
}

/**
 * lim_process_sme_ndp_data_end_req() - Handler for eWNI_SME_NDP_END_REQ
 * from SME.
 * @mac_ctx: handle to mac context
 * @sme_msg: ndp data end request msg
 *
 * Return: Status of operation
 */
QDF_STATUS lim_process_sme_ndp_data_end_req(tpAniSirGlobal mac_ctx,
					    struct sir_sme_ndp_end_req *sme_msg)
{
	tSirMsgQ msg;
	uint32_t len;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (NULL == sme_msg) {
		lim_log(mac_ctx, LOGE, FL("invalid ndp_req"));
		/* msg to unblock SME, but not send rsp to HDD */
		lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_END_RSP, NULL,
					  0, true);
		return QDF_STATUS_E_INVAL;
	}

	msg.type = SIR_HAL_NDP_END_REQ;
	msg.reserved = 0;
	len = sizeof(*sme_msg->req) + (sme_msg->req->num_ndp_instances *
						 sizeof(uint32_t));
	msg.bodyptr = qdf_mem_malloc(len);
	if (NULL == msg.bodyptr) {
		/* msg to unblock SME, but not send rsp to HDD */
		lim_send_ndp_event_to_sme(mac_ctx, eWNI_SME_NDP_END_RSP, NULL,
					  0, true);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(msg.bodyptr, sme_msg->req, len);
	msg.bodyval = 0;

	lim_log(mac_ctx, LOG1, FL("sending SIR_HAL_NDP_END_REQ to WMA"));
	MTRACE(mac_trace_msg_tx(mac_ctx, NO_SESSION, msg.type));

	if (eSIR_SUCCESS != wma_post_ctrl_msg(mac_ctx, &msg)) {
		lim_log(mac_ctx, LOGP, FL("wma_post_ctrl_msg failed"));
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

/**
 * lim_handle_ndp_request_message() - Handler for NDP req from SME
 * @mac_ctx: handle to mac structure
 * @msg: pointer to message
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS lim_handle_ndp_request_message(tpAniSirGlobal mac_ctx,
					  tpSirMsgQ msg)
{
	QDF_STATUS status;

	switch (msg->type) {
	case eWNI_SME_NDP_END_REQ:
		status = lim_process_sme_ndp_data_end_req(mac_ctx,
							  msg->bodyptr);
		break;
	case eWNI_SME_NDP_INITIATOR_REQ:
		status = lim_process_sme_ndp_initiator_req(mac_ctx,
							   msg->bodyptr);
		break;
	case eWNI_SME_NDP_RESPONDER_REQ:
		status = lim_process_sme_ndp_responder_req(mac_ctx,
							 msg->bodyptr);
		break;
	default:
		lim_log(mac_ctx, LOGE, FL("Unhandled NDP request: %d"),
		       msg->type);
		status = QDF_STATUS_E_NOSUPPORT;
		break;
	}
	return status;
}

/**
 * lim_process_ndi_mlm_add_bss_rsp() - Process ADD_BSS response for NDI
 * @mac_ctx: Pointer to Global MAC structure
 * @lim_msgq: The MsgQ header, which contains the response buffer
 * @session_entry: PE session
 *
 * Return: None
 */
void lim_process_ndi_mlm_add_bss_rsp(tpAniSirGlobal mac_ctx, tpSirMsgQ lim_msgq,
		tpPESession session_entry)
{
	tLimMlmStartCnf mlm_start_cnf;
	tpAddBssParams add_bss_params = (tpAddBssParams) lim_msgq->bodyptr;

	if (NULL == add_bss_params) {
		lim_log(mac_ctx, LOGE, FL("Invalid body pointer in message"));
		goto end;
	}
	lim_log(mac_ctx, LOG1, FL("Status %d"), add_bss_params->status);
	if (QDF_STATUS_SUCCESS == add_bss_params->status) {
		lim_log(mac_ctx, LOG1,
		       FL("WDA_ADD_BSS_RSP returned QDF_STATUS_SUCCESS"));
		session_entry->limMlmState = eLIM_MLM_BSS_STARTED_STATE;
		MTRACE(mac_trace(mac_ctx, TRACE_CODE_MLM_STATE,
			session_entry->peSessionId,
			session_entry->limMlmState));
		session_entry->bssIdx = (uint8_t) add_bss_params->bssIdx;
		session_entry->limSystemRole = eLIM_NDI_ROLE;
		session_entry->statypeForBss = STA_ENTRY_SELF;
		session_entry->staId = add_bss_params->staContext.staIdx;
		/* Apply previously set configuration at HW */
		lim_apply_configuration(mac_ctx, session_entry);
		mlm_start_cnf.resultCode = eSIR_SME_SUCCESS;
	} else {
		lim_log(mac_ctx, LOGE,
			FL("WDA_ADD_BSS_REQ failed with status %d"),
			add_bss_params->status);
		mlm_start_cnf.resultCode = eSIR_SME_HAL_SEND_MESSAGE_FAIL;
	}
	mlm_start_cnf.sessionId = session_entry->peSessionId;
	lim_post_sme_message(mac_ctx, LIM_MLM_START_CNF,
				(uint32_t *) &mlm_start_cnf);
end:
	qdf_mem_free(lim_msgq->bodyptr);
	lim_msgq->bodyptr = NULL;
}

/**
 * lim_ndi_del_bss_rsp() - Handler for DEL BSS resp for NDI interface
 * @mac_ctx: handle to mac structure
 * @msg: pointer to message
 * @session_entry: session entry
 *
 * Return: None
 */
void lim_ndi_del_bss_rsp(tpAniSirGlobal  mac_ctx,
			void *msg, tpPESession session_entry)
{
	tSirResultCodes rc = eSIR_SME_SUCCESS;
	tpDeleteBssParams del_bss = (tpDeleteBssParams) msg;

	SET_LIM_PROCESS_DEFD_MESGS(mac_ctx, true);
	if (del_bss == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("NDI: DEL_BSS_RSP with no body!"));
		rc = eSIR_SME_STOP_BSS_FAILURE;
		goto end;
	}
	session_entry =
		pe_find_session_by_session_id(mac_ctx, del_bss->sessionId);
	if (!session_entry) {
		lim_log(mac_ctx, LOGE,
			FL("Session Does not exist for given sessionID"));
		goto end;
	}

	if (del_bss->status != QDF_STATUS_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("NDI: DEL_BSS_RSP error (%x) Bss %d "),
			del_bss->status, del_bss->bssIdx);
		rc = eSIR_SME_STOP_BSS_FAILURE;
		goto end;
	}

	if (lim_set_link_state(mac_ctx, eSIR_LINK_IDLE_STATE,
			session_entry->selfMacAddr,
			session_entry->selfMacAddr, NULL, NULL)
			!= eSIR_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("NDI: DEL_BSS_RSP setLinkState failed"));
		goto end;
	}

	session_entry->limMlmState = eLIM_MLM_IDLE_STATE;

end:
	if (del_bss)
		qdf_mem_free(del_bss);
	/* Delete PE session once BSS is deleted */
	if (NULL != session_entry) {
		lim_send_sme_rsp(mac_ctx, eWNI_SME_STOP_BSS_RSP,
			rc, session_entry->smeSessionId,
			session_entry->transactionId);
		pe_delete_session(mac_ctx, session_entry);
		session_entry = NULL;
	}
}

/**
 * lim_send_sme_ndp_add_sta_rsp() - prepares and send new peer ind to SME
 * @mac_ctx: handle to mac structure
 * @session: session pointer
 * @add_sta_rsp: add sta response struct
 *
 * Return: status of operation
 */
static QDF_STATUS lim_send_sme_ndp_add_sta_rsp(tpAniSirGlobal mac_ctx,
					       tpPESession session,
					       tAddStaParams *add_sta_rsp)
{
	tSirMsgQ  mmh_msg = {0};
	struct sme_ndp_peer_ind *new_peer_ind;

	mmh_msg.type = eWNI_SME_NDP_NEW_PEER_IND;

	if (NULL == add_sta_rsp) {
		lim_log(mac_ctx, LOGE, FL("Invalid add_sta_rsp"));
		return QDF_STATUS_E_INVAL;
	}

	new_peer_ind = qdf_mem_malloc(sizeof(*new_peer_ind));
	if (NULL == new_peer_ind) {
		lim_log(mac_ctx, LOGE, FL("Failed to allocate memory"));
		return QDF_STATUS_E_NOMEM;
	}

	/* this message is going to HDD, fill in sme session id */
	new_peer_ind->session_id = add_sta_rsp->smesessionId;
	new_peer_ind->msg_len = sizeof(struct sme_ndp_peer_ind);
	new_peer_ind->msg_type = eWNI_SME_NDP_NEW_PEER_IND;
	qdf_mem_copy(new_peer_ind->peer_mac_addr.bytes, add_sta_rsp->staMac,
		     sizeof(tSirMacAddr));
	new_peer_ind->sta_id = add_sta_rsp->staIdx;

	mmh_msg.bodyptr = new_peer_ind;
	mmh_msg.bodyval = 0;
	lim_sys_process_mmh_msg_api(mac_ctx, &mmh_msg, ePROT);
	return QDF_STATUS_SUCCESS;
}

/**
 * lim_ndp_add_sta_rsp() - handles add sta rsp for NDP from WMA
 * @mac_ctx: handle to mac structure
 * @session: session pointer
 * @add_sta_rsp: add sta response struct
 *
 * Return: None
 */
void lim_ndp_add_sta_rsp(tpAniSirGlobal mac_ctx, tpPESession session,
			 tAddStaParams *add_sta_rsp)
{
	tpDphHashNode sta_ds;
	uint16_t peer_idx;

	if (NULL == add_sta_rsp) {
		lim_log(mac_ctx, LOGE, FL("Invalid add_sta_rsp"));
		qdf_mem_free(add_sta_rsp);
		return;
	}

	SET_LIM_PROCESS_DEFD_MESGS(mac_ctx, true);
	sta_ds = dph_lookup_hash_entry(mac_ctx, add_sta_rsp->staMac, &peer_idx,
				    &session->dph.dphHashTable);
	if (sta_ds == NULL) {
		lim_log(mac_ctx, LOGE,
			FL("NAN: ADD_STA_RSP for unknown MAC addr "
			MAC_ADDRESS_STR),
			MAC_ADDR_ARRAY(add_sta_rsp->staMac));
		qdf_mem_free(add_sta_rsp);
		return;
	}

	if (add_sta_rsp->status != QDF_STATUS_SUCCESS) {
		lim_log(mac_ctx, LOGE,
			FL("NAN: ADD_STA_RSP error %x for MAC addr: %pM"),
			add_sta_rsp->status, add_sta_rsp->staMac);
		/* delete the sta_ds allocated during ADD STA */
		lim_delete_dph_hash_entry(mac_ctx, add_sta_rsp->staMac,
				      peer_idx, session);
		qdf_mem_free(add_sta_rsp);
		return;
	}
	sta_ds->bssId = add_sta_rsp->bssIdx;
	sta_ds->staIndex = add_sta_rsp->staIdx;
	sta_ds->ucUcastSig = add_sta_rsp->ucUcastSig;
	sta_ds->ucBcastSig = add_sta_rsp->ucBcastSig;
	sta_ds->valid = 1;
	sta_ds->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
	lim_send_sme_ndp_add_sta_rsp(mac_ctx, session, add_sta_rsp);
	qdf_mem_free(add_sta_rsp);
}

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
 * DOC: wlan_tdls_mgmt.c
 *
 * TDLS management frames implementation
 */

#include "wlan_tdls_main.h"
#include "wlan_tdls_tgt_api.h"
#include <wlan_serialization_api.h>
#include "wlan_mgmt_txrx_utils_api.h"
#include "wlan_tdls_peer.h"
#include "wlan_tdls_ct.h"
#include "wlan_tdls_cmds_process.h"
#include "wlan_tdls_mgmt.h"

static
const char *const tdls_action_frames_type[] = { "TDLS Setup Request",
					 "TDLS Setup Response",
					 "TDLS Setup Confirm",
					 "TDLS Teardown",
					 "TDLS Peer Traffic Indication",
					 "TDLS Channel Switch Request",
					 "TDLS Channel Switch Response",
					 "TDLS Peer PSM Request",
					 "TDLS Peer PSM Response",
					 "TDLS Peer Traffic Response",
					 "TDLS Discovery Request"};

/**
 * tdls_set_rssi() - Set TDLS RSSI on peer given by mac
 * @tdls_vdev: tdls vdev object
 * @mac: MAC address of Peer
 * @rx_rssi: rssi value
 *
 * Set RSSI on TDSL peer
 *
 * Return: 0 for success or -EINVAL otherwise
 */
static int tdls_set_rssi(struct tdls_vdev_priv_obj *tdls_vdev,
		  const uint8_t *mac,
		  int8_t rx_rssi)
{
	struct tdls_peer *curr_peer;

	curr_peer = tdls_find_peer(tdls_vdev, mac);
	if (curr_peer == NULL) {
		tdls_err("curr_peer is NULL");
		return -EINVAL;
	}

	curr_peer->rssi = rx_rssi;

	return 0;
}

/**
 * tdls_process_rx_mgmt() - process tdls rx mgmt frames
 * @rx_mgmt_event: tdls rx mgmt event
 * @tdls_vdev: tdls vdev object
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS tdls_process_rx_mgmt(
	struct tdls_rx_mgmt_event *rx_mgmt_event,
	struct tdls_vdev_priv_obj *tdls_vdev)
{
	struct tdls_rx_mgmt_frame *rx_mgmt;
	struct tdls_soc_priv_obj *tdls_soc_obj;
	uint8_t *mac;
	enum tdls_actioncode action_frame_type;

	if (!rx_mgmt_event)
		return QDF_STATUS_E_INVAL;

	tdls_soc_obj = rx_mgmt_event->tdls_soc_obj;
	rx_mgmt = rx_mgmt_event->rx_mgmt;

	if (!tdls_soc_obj || !rx_mgmt) {
		tdls_err("invalid psoc object or rx mgmt");
		return QDF_STATUS_E_INVAL;
	}

	tdls_debug("soc:%p, frame_len:%d, rx_chan:%d, vdev_id:%d, frm_type:%d, rx_rssi:%d, buf:%p",
		tdls_soc_obj->soc, rx_mgmt->frame_len,
		rx_mgmt->rx_chan, rx_mgmt->vdev_id, rx_mgmt->frm_type,
		rx_mgmt->rx_rssi, rx_mgmt->buf);

	if (rx_mgmt->buf[TDLS_PUBLIC_ACTION_FRAME_OFFSET + 1] ==
						TDLS_PUBLIC_ACTION_DISC_RESP) {
		mac = &rx_mgmt->buf[TDLS_80211_PEER_ADDR_OFFSET];
		tdls_notice("[TDLS] TDLS Discovery Response,"
		       QDF_MAC_ADDRESS_STR " RSSI[%d] <--- OTA",
		       QDF_MAC_ADDR_ARRAY(mac), rx_mgmt->rx_rssi);
			tdls_recv_discovery_resp(tdls_vdev, mac);
			tdls_set_rssi(tdls_vdev, mac, rx_mgmt->rx_rssi);
	}

	if (rx_mgmt->buf[TDLS_PUBLIC_ACTION_FRAME_OFFSET] ==
	    TDLS_ACTION_FRAME) {
		action_frame_type =
			rx_mgmt->buf[TDLS_PUBLIC_ACTION_FRAME_OFFSET + 1];
		if (action_frame_type >= TDLS_ACTION_FRAME_TYPE_MAX) {
			tdls_debug("[TDLS] unknown[%d] <--- OTA",
				   action_frame_type);
		} else {
			tdls_notice("[TDLS] %s <--- OTA",
				   tdls_action_frames_type[action_frame_type]);
		}
	}

	/* tdls_soc_obj->tdls_rx_cb ==> wlan_cfg80211_tdls_rx_callback() */
	if (tdls_soc_obj && tdls_soc_obj->tdls_rx_cb)
		tdls_soc_obj->tdls_rx_cb(tdls_soc_obj->tdls_rx_cb_data,
					 rx_mgmt);
	else
		tdls_debug("rx mgmt, but no valid up layer callback");

	qdf_mem_free(rx_mgmt);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tdls_process_rx_frame(struct scheduler_msg *msg)
{
	struct wlan_objmgr_vdev *vdev;
	struct tdls_rx_mgmt_event *tdls_rx;
	struct tdls_vdev_priv_obj *tdls_vdev;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!(msg->bodyptr)) {
		tdls_err("invalid message body");
		return QDF_STATUS_E_INVAL;
	}

	tdls_rx = (struct tdls_rx_mgmt_event *) msg->bodyptr;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(tdls_rx->tdls_soc_obj->soc,
				tdls_rx->rx_mgmt->vdev_id, WLAN_TDLS_NB_ID);

	if (vdev) {
		tdls_debug("tdls rx mgmt frame received");
		tdls_vdev = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							WLAN_UMAC_COMP_TDLS);
		if (tdls_vdev)
			status = tdls_process_rx_mgmt(tdls_rx, tdls_vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
	}

	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;

	return status;
}

QDF_STATUS tdls_mgmt_rx_ops(struct wlan_objmgr_psoc *psoc,
	bool isregister)
{
	struct mgmt_txrx_mgmt_frame_cb_info frm_cb_info[3];

	QDF_STATUS status;
	int num_of_entries;

	tdls_debug("psoc:%p, is register rx:%d", psoc, isregister);

	frm_cb_info[0].frm_type = MGMT_PROBE_REQ;
	frm_cb_info[0].mgmt_rx_cb = tgt_tdls_mgmt_frame_rx_cb;
	frm_cb_info[1].frm_type = MGMT_ACTION_TDLS_DISCRESP;
	frm_cb_info[1].mgmt_rx_cb = tgt_tdls_mgmt_frame_rx_cb;
	frm_cb_info[2].frm_type = MGMT_ACTION_VENDOR_SPECIFIC;
	frm_cb_info[2].mgmt_rx_cb = tgt_tdls_mgmt_frame_rx_cb;
	num_of_entries = 3;

	if (isregister)
		status = wlan_mgmt_txrx_register_rx_cb(psoc,
				WLAN_UMAC_COMP_TDLS, frm_cb_info,
				num_of_entries);
	else
		status = wlan_mgmt_txrx_deregister_rx_cb(psoc,
				WLAN_UMAC_COMP_TDLS, frm_cb_info,
				num_of_entries);

	return status;
}

static QDF_STATUS
tdls_internal_send_mgmt_tx_done(struct tdls_action_frame_request *req,
				QDF_STATUS status)
{
	struct tdls_soc_priv_obj *tdls_soc_obj;
	struct tdls_osif_indication indication;

	if (!req || !req->vdev)
		return QDF_STATUS_E_NULL_VALUE;

	indication.status = status;
	indication.vdev = req->vdev;

	tdls_soc_obj = wlan_vdev_get_tdls_soc_obj(req->vdev);
	if (tdls_soc_obj && tdls_soc_obj->tdls_event_cb)
		tdls_soc_obj->tdls_event_cb(tdls_soc_obj->tdls_evt_cb_data,
			TDLS_EVENT_MGMT_TX_ACK_CNF, &indication);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_activate_send_mgmt_request(
				struct tdls_action_frame_request *action_req)
{
	struct wlan_objmgr_peer *peer;
	struct tdls_soc_priv_obj *tdls_soc_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	struct scheduler_msg msg;
	struct tdls_send_mgmt_request *tdls_mgmt_req;

	if (!action_req || !action_req->vdev)
		return QDF_STATUS_E_NULL_VALUE;

	if (QDF_STATUS_SUCCESS != wlan_objmgr_vdev_try_get_ref(action_req->vdev,
						WLAN_TDLS_SB_ID))
		return QDF_STATUS_E_NULL_VALUE;


	tdls_soc_obj = wlan_vdev_get_tdls_soc_obj(action_req->vdev);
	if (!tdls_soc_obj) {
		status = QDF_STATUS_E_NULL_VALUE;
		goto release_mgmt_ref;
	}

	tdls_mgmt_req = qdf_mem_malloc(sizeof(struct tdls_send_mgmt_request) +
				action_req->tdls_mgmt.len);
	if (NULL == tdls_mgmt_req) {
		status = QDF_STATUS_E_NOMEM;
		tdls_err("mem alloc failed ");
		QDF_ASSERT(0);
		goto release_mgmt_ref;
	}

	tdls_debug("session_id %d "
		   "tdls_mgmt.dialog %d "
		   "tdls_mgmt.frame_type %d "
		   "tdls_mgmt.status_code %d "
		   "tdls_mgmt.responder %d "
		   "tdls_mgmt.peer_capability %d",
		   action_req->session_id,
		   action_req->tdls_mgmt.dialog,
		   action_req->tdls_mgmt.frame_type,
		   action_req->tdls_mgmt.status_code,
		   action_req->tdls_mgmt.responder,
		   action_req->tdls_mgmt.peer_capability);

	tdls_mgmt_req->session_id = action_req->session_id;
	/* Using dialog as transactionId. This can be used to
	 * match response with request
	 */
	tdls_mgmt_req->transaction_id = action_req->tdls_mgmt.dialog;
	tdls_mgmt_req->req_type = action_req->tdls_mgmt.frame_type;
	tdls_mgmt_req->dialog = action_req->tdls_mgmt.dialog;
	tdls_mgmt_req->status_code = action_req->tdls_mgmt.status_code;
	tdls_mgmt_req->responder = action_req->tdls_mgmt.responder;
	tdls_mgmt_req->peer_capability = action_req->tdls_mgmt.peer_capability;

	peer = wlan_vdev_get_bsspeer(action_req->vdev);

	if (QDF_STATUS_SUCCESS != wlan_objmgr_peer_try_get_ref(peer,
							WLAN_TDLS_SB_ID)) {
		qdf_mem_free(tdls_mgmt_req);
		goto release_mgmt_ref;
	}

	qdf_mem_copy(tdls_mgmt_req->bssid.bytes,
		     wlan_peer_get_macaddr(peer), QDF_MAC_ADDR_SIZE);

	qdf_mem_copy(tdls_mgmt_req->peer_mac.bytes,
		     action_req->tdls_mgmt.peer_mac.bytes, QDF_MAC_ADDR_SIZE);

	if (action_req->tdls_mgmt.len && action_req->tdls_mgmt.buf) {
		qdf_mem_copy(tdls_mgmt_req->add_ie, action_req->tdls_mgmt.buf,
			     action_req->tdls_mgmt.len);
	}

	tdls_mgmt_req->length = sizeof(struct tdls_send_mgmt_request) +
				action_req->tdls_mgmt.len;

	/* Send the request to PE. */
	qdf_mem_zero(&msg, sizeof(msg));

	tdls_debug("sending TDLS Mgmt Frame req to PE ");
	tdls_mgmt_req->message_type = tdls_soc_obj->tdls_send_mgmt_req;

	msg.type = tdls_soc_obj->tdls_send_mgmt_req;
	msg.bodyptr = tdls_mgmt_req;

	status = scheduler_post_msg(QDF_MODULE_ID_PE, &msg);

	wlan_objmgr_peer_release_ref(peer, WLAN_TDLS_SB_ID);
release_mgmt_ref:
	wlan_objmgr_vdev_release_ref(action_req->vdev, WLAN_TDLS_SB_ID);

	/*update tdls nss infornation based on action code */
	tdls_reset_nss(tdls_soc_obj, action_req->chk_frame->action_code);

	return status;
}

static QDF_STATUS
tdls_send_mgmt_serialize_callback(struct wlan_serialization_command *cmd,
	 enum wlan_serialization_cb_reason reason)
{
	struct tdls_action_frame_request *req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!cmd || !cmd->umac_cmd) {
		tdls_err("invalid params cmd: %p, ", cmd);
		return QDF_STATUS_E_NULL_VALUE;
	}
	req = cmd->umac_cmd;

	tdls_debug("reason: %d, vdev_id: %d",
		reason, req->vdev_id);

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		/* command moved to active list */
		status = tdls_activate_send_mgmt_request(req);
		break;

	case WLAN_SER_CB_CANCEL_CMD:
	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		/* command removed from pending list.
		 * notify status complete with failure
		 */
		status = tdls_internal_send_mgmt_tx_done(req,
				QDF_STATUS_E_FAILURE);
		qdf_mem_free(req);
		break;

	case WLAN_SER_CB_RELEASE_MEM_CMD:
		/* command successfully completed.
		 * release tdls_action_frame_request memory
		 */
		qdf_mem_free(req);
		break;

	default:
		/* Do nothing but logging */
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

QDF_STATUS tdls_process_mgmt_req(
			struct tdls_action_frame_request *tdls_mgmt_req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_serialization_command cmd = {0, };
	enum wlan_serialization_status ser_cmd_status;

	/* If connected and in Infra. Only then allow this */
	status = tdls_validate_mgmt_request(tdls_mgmt_req->chk_frame);
	if (status != QDF_STATUS_SUCCESS) {
		status = tdls_internal_send_mgmt_tx_done(tdls_mgmt_req,
							 status);
		qdf_mem_free(tdls_mgmt_req);
		return status;
	}

	/* update the responder, status code information
	 * after the  cmd validation
	 */
	tdls_mgmt_req->tdls_mgmt.responder =
			tdls_mgmt_req->chk_frame->responder;
	tdls_mgmt_req->tdls_mgmt.status_code =
			tdls_mgmt_req->chk_frame->status_code;

	cmd.cmd_type = WLAN_SER_CMD_TDLS_SEND_MGMT;
	/* Cmd Id not applicable for non scan cmds */
	cmd.cmd_id = 0;
	cmd.cmd_cb = (wlan_serialization_cmd_callback)
		tdls_send_mgmt_serialize_callback;
	cmd.umac_cmd = tdls_mgmt_req;
	cmd.source = WLAN_UMAC_COMP_TDLS;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = TDLS_DEFAULT_SERIALIZE_CMD_TIMEOUT;

	cmd.vdev = tdls_mgmt_req->vdev;

	ser_cmd_status = wlan_serialization_request(&cmd);
	tdls_debug("wlan_serialization_request status:%d", ser_cmd_status);

	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list.Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	case WLAN_SER_CMD_DENIED_LIST_FULL:
	case WLAN_SER_CMD_DENIED_RULES_FAILED:
	case WLAN_SER_CMD_DENIED_UNSPECIFIED:
		/* free the request */
		qdf_mem_free(tdls_mgmt_req);

	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}
	return status;
}

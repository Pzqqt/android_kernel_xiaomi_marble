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
 * DOC: wlan_tdls_cmds_process.c
 *
 * TDLS north bound commands implementation
 */
#include <wlan_serialization_api.h>
#include "wlan_tdls_main.h"
#include "wlan_tdls_peer.h"
#include "wlan_tdls_cmds_process.h"

static uint16_t tdls_get_connected_peer(struct tdls_soc_priv_obj *soc_obj)
{
	return soc_obj->connected_peer_count;
}

/**
 * tdls_decrement_peer_count() - decrement connected TDLS peer counter
 * @soc_obj: TDLS soc object
 *
 * Return: None.
 */
static void tdls_decrement_peer_count(struct tdls_soc_priv_obj *soc_obj)
{
	if (soc_obj->connected_peer_count)
		soc_obj->connected_peer_count--;

	tdls_debug("Connected peer count %d", soc_obj->connected_peer_count);
}

/**
 * tdls_validate_current_mode() - check current TDL mode
 * @soc_obj: TDLS soc object
 *
 * Return: QDF_STATUS_SUCCESS if TDLS enabled, other for disabled
 */
static QDF_STATUS tdls_validate_current_mode(struct tdls_soc_priv_obj *soc_obj)
{
	if (soc_obj->tdls_current_mode == TDLS_SUPPORT_DISABLED ||
	    soc_obj->tdls_current_mode == TDLS_SUPPORT_SUSPENDED) {
		tdls_err("TDLS mode disabled OR not enabled, current mode %d",
			 soc_obj->tdls_current_mode);
		return QDF_STATUS_E_NOSUPPORT;
	}
	return QDF_STATUS_SUCCESS;
}

static char *tdls_get_ser_cmd_str(enum  wlan_serialization_cmd_type type)
{
	switch (type) {
	case WLAN_SER_CMD_TDLS_ADD_PEER:
		return "TDLS_ADD_PEER_CMD";
	case WLAN_SER_CMD_TDLS_DEL_PEER:
		return "TDLS_DEL_PEER_CMD";
	case WLAN_SER_CMD_TDLS_SEND_MGMT:
		return "TDLS_SEND_MGMT_CMD";
	default:
		return "UNKNOWN";
	}
}

static void
tdls_release_serialization_command(struct wlan_objmgr_vdev *vdev,
				   enum wlan_serialization_cmd_type type)
{
	struct wlan_serialization_queued_cmd_info cmd = {0};

	cmd.cmd_type = type;
	cmd.cmd_id = 0;
	cmd.vdev = vdev;

	tdls_debug("release %s", tdls_get_ser_cmd_str(type));
	/* Inform serialization for command completion */
	wlan_serialization_remove_cmd(&cmd);
}

/**
 * tdls_pe_add_peer() - send TDLS add peer request to PE
 * @req: TDL add peer request
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
static QDF_STATUS tdls_pe_add_peer(struct tdls_add_peer_request *req)
{
	struct tdls_add_sta_req *addstareq;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct tdls_soc_priv_obj *soc_obj;
	struct scheduler_msg msg = {0,};
	QDF_STATUS status;

	addstareq = qdf_mem_malloc(sizeof(*addstareq));
	if (!addstareq) {
		tdls_err("allocate failed");
		return QDF_STATUS_E_NOMEM;
	}
	vdev = req->vdev;
	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev);
	if (!soc_obj) {
		tdls_err("NULL tdls soc object");
		status = QDF_STATUS_E_INVAL;
		goto error;
	}

	addstareq->tdls_oper = TDLS_OPER_ADD;
	addstareq->transaction_id = 0;

	wlan_vdev_obj_lock(vdev);
	addstareq->session_id = wlan_vdev_get_id(vdev);
	peer = wlan_vdev_get_bsspeer(vdev);
	if (!peer) {
		wlan_vdev_obj_unlock(vdev);
		tdls_err("bss peer is NULL");
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	wlan_vdev_obj_unlock(vdev);
	status = wlan_objmgr_peer_try_get_ref(peer, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get bss peer");
		goto error;
	}
	wlan_peer_obj_lock(peer);
	qdf_mem_copy(addstareq->bssid.bytes,
		     wlan_peer_get_macaddr(peer), QDF_MAC_ADDR_SIZE);
	wlan_peer_obj_unlock(peer);
	wlan_objmgr_peer_release_ref(peer, WLAN_TDLS_NB_ID);
	qdf_mem_copy(addstareq->peermac.bytes, req->add_peer_req.peer_addr,
		     QDF_MAC_ADDR_SIZE);

	tdls_debug("for " QDF_MAC_ADDRESS_STR,
		   QDF_MAC_ADDR_ARRAY(addstareq->peermac.bytes));
	msg.type = soc_obj->tdls_add_sta_req;
	msg.bodyptr = addstareq;
	status = scheduler_post_msg(QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("fail to post pe msg to add peer");
		goto error;
	}
	return status;
error:
	qdf_mem_free(addstareq);
	return status;
}

/**
 * tdls_pe_del_peer() - send TDLS delete peer request to PE
 * @req: TDLS delete peer request
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
QDF_STATUS tdls_pe_del_peer(struct tdls_del_peer_request *req)
{
	struct tdls_del_sta_req *delstareq;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct tdls_soc_priv_obj *soc_obj;
	struct scheduler_msg msg = {0,};
	QDF_STATUS status;

	delstareq = qdf_mem_malloc(sizeof(*delstareq));
	if (!delstareq) {
		tdls_err("allocate failed");
		return QDF_STATUS_E_NOMEM;
	}
	vdev = req->vdev;
	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev);
	if (!soc_obj) {
		tdls_err("NULL tdls soc object");
		status = QDF_STATUS_E_INVAL;
		goto error;
	}

	delstareq->transaction_id = 0;

	wlan_vdev_obj_lock(vdev);
	delstareq->session_id = wlan_vdev_get_id(vdev);
	peer = wlan_vdev_get_bsspeer(vdev);
	if (!peer) {
		wlan_vdev_obj_unlock(vdev);
		tdls_err("bss peer is NULL");
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	wlan_vdev_obj_unlock(vdev);
	status = wlan_objmgr_peer_try_get_ref(peer, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get bss peer");
		goto error;
	}
	wlan_peer_obj_lock(peer);
	qdf_mem_copy(delstareq->bssid.bytes,
		     wlan_peer_get_macaddr(peer), QDF_MAC_ADDR_SIZE);
	wlan_peer_obj_unlock(peer);
	wlan_objmgr_peer_release_ref(peer, WLAN_TDLS_NB_ID);
	qdf_mem_copy(delstareq->peermac.bytes, req->del_peer_req.peer_addr,
		     QDF_MAC_ADDR_SIZE);

	tdls_debug("for " QDF_MAC_ADDRESS_STR,
		   QDF_MAC_ADDR_ARRAY(delstareq->peermac.bytes));
	msg.type = soc_obj->tdls_del_sta_req;
	msg.bodyptr = delstareq;
	status = scheduler_post_msg(QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("fail to post pe msg to del peer");
		goto error;
	}
	return status;
error:
	qdf_mem_free(delstareq);
	return status;
}

/**
 * tdls_pe_update_peer() - send TDLS update peer request to PE
 * @req: TDLS update peer request
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
static QDF_STATUS tdls_pe_update_peer(struct tdls_update_peer_request *req)
{
	struct tdls_add_sta_req *addstareq;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_peer *peer;
	struct tdls_soc_priv_obj *soc_obj;
	struct scheduler_msg msg = {0,};
	struct tdls_update_peer_params *update_peer;
	QDF_STATUS status;

	addstareq = qdf_mem_malloc(sizeof(*addstareq));
	if (!addstareq) {
		tdls_err("allocate failed");
		return QDF_STATUS_E_NOMEM;
	}
	vdev = req->vdev;
	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev);
	if (!soc_obj) {
		tdls_err("NULL tdls soc object");
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	update_peer = &req->update_peer_req;

	addstareq->tdls_oper = TDLS_OPER_UPDATE;
	addstareq->transaction_id = 0;

	wlan_vdev_obj_lock(vdev);
	addstareq->session_id = wlan_vdev_get_id(vdev);
	peer = wlan_vdev_get_bsspeer(vdev);
	if (!peer) {
		wlan_vdev_obj_unlock(vdev);
		tdls_err("bss peer is NULL");
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	wlan_vdev_obj_unlock(vdev);
	status = wlan_objmgr_peer_try_get_ref(peer, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get bss peer");
		goto error;
	}
	wlan_peer_obj_lock(peer);
	qdf_mem_copy(addstareq->bssid.bytes,
		     wlan_peer_get_macaddr(peer), QDF_MAC_ADDR_SIZE);
	wlan_peer_obj_unlock(peer);
	wlan_objmgr_peer_release_ref(peer, WLAN_TDLS_NB_ID);
	qdf_mem_copy(addstareq->peermac.bytes, update_peer->peer_addr,
		     QDF_MAC_ADDR_SIZE);
	addstareq->capability = update_peer->capability;
	addstareq->uapsd_queues = update_peer->uapsd_queues;
	addstareq->max_sp = update_peer->max_sp;

	qdf_mem_copy(addstareq->extn_capability,
		     update_peer->extn_capability, WLAN_MAC_MAX_EXTN_CAP);
	addstareq->htcap_present = update_peer->htcap_present;
	qdf_mem_copy(&addstareq->ht_cap,
		     &update_peer->ht_cap,
		     sizeof(update_peer->ht_cap));
	addstareq->vhtcap_present = update_peer->vhtcap_present;
	qdf_mem_copy(&addstareq->vht_cap,
		     &update_peer->vht_cap,
		     sizeof(update_peer->vht_cap));
	addstareq->supported_rates_length = update_peer->supported_rates_len;
	qdf_mem_copy(&addstareq->supported_rates,
		     update_peer->supported_rates,
		     update_peer->supported_rates_len);
	tdls_debug("for " QDF_MAC_ADDRESS_STR,
		   QDF_MAC_ADDR_ARRAY(addstareq->peermac.bytes));

	msg.type = soc_obj->tdls_add_sta_req;
	msg.bodyptr = addstareq;
	status = scheduler_post_msg(QDF_MODULE_ID_PE, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("fail to post pe msg to update peer");
		goto error;
	}
	return status;
error:
	qdf_mem_free(addstareq);
	return status;
}

static QDF_STATUS
tdls_internal_add_peer_rsp(struct tdls_add_peer_request *req,
			   QDF_STATUS status)
{
	struct tdls_soc_priv_obj *soc_obj;
	struct wlan_objmgr_vdev *vdev;
	struct tdls_osif_indication ind;
	QDF_STATUS ret;

	if (!req || !req->vdev) {
		tdls_err("req: %p", req);
		return QDF_STATUS_E_INVAL;
	}
	vdev = req->vdev;
	ret = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_TDLS_SB_ID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		tdls_err("can't get vdev object");
		return ret;
	}

	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev);
	if (soc_obj && soc_obj->tdls_event_cb) {
		ind.vdev = vdev;
		ind.status = status;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_ADD_PEER, &ind);
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_SB_ID);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
tdls_internal_update_peer_rsp(struct tdls_update_peer_request *req,
			      QDF_STATUS status)
{
	struct tdls_soc_priv_obj *soc_obj;
	struct tdls_osif_indication ind;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS ret;

	if (!req || !req->vdev) {
		tdls_err("req: %p", req);
		return QDF_STATUS_E_INVAL;
	}
	vdev = req->vdev;
	ret = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_TDLS_SB_ID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		tdls_err("can't get vdev object");
		return ret;
	}

	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev);
	if (soc_obj && soc_obj->tdls_event_cb) {
		ind.vdev = vdev;
		ind.status = status;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_ADD_PEER, &ind);
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_SB_ID);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_internal_del_peer_rsp(struct tdls_oper_request *req)
{
	struct tdls_soc_priv_obj *soc_obj;
	struct tdls_osif_indication ind;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status;

	if (!req || !req->vdev) {
		tdls_err("req: %p", req);
		return QDF_STATUS_E_INVAL;
	}
	vdev = req->vdev;
	status = wlan_objmgr_vdev_try_get_ref(vdev, WLAN_TDLS_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("can't get vdev object");
		return status;
	}

	soc_obj = wlan_vdev_get_tdls_soc_obj(req->vdev);
	if (soc_obj && soc_obj->tdls_event_cb) {
		ind.vdev = req->vdev;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_DEL_PEER, &ind);
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_activate_add_peer(struct tdls_add_peer_request *req)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *soc_obj;
	struct tdls_vdev_priv_obj *vdev_obj;
	struct tdls_peer *peer;
	uint16_t curr_tdls_peers;
	const uint8_t *mac;
	struct tdls_osif_indication ind;

	if (!req->vdev) {
		tdls_err("vdev null when add tdls peer");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	mac = req->add_peer_req.peer_addr;
	soc_obj = wlan_vdev_get_tdls_soc_obj(req->vdev);
	vdev_obj = wlan_vdev_get_tdls_vdev_obj(req->vdev);

	if (!soc_obj || !vdev_obj) {
		tdls_err("soc_obj: %p, vdev_obj: %p", soc_obj, vdev_obj);
		return QDF_STATUS_E_INVAL;
	}
	status = tdls_validate_current_mode(soc_obj);
	if (QDF_IS_STATUS_ERROR(status))
		goto addrsp;

	peer = tdls_get_peer(vdev_obj, mac);
	if (!peer) {
		tdls_err("peer: " QDF_MAC_ADDRESS_STR " not exist. invalid",
			 QDF_MAC_ADDR_ARRAY(mac));
		status = QDF_STATUS_E_INVAL;
		goto addrsp;
	}

	/* in add station, we accept existing valid sta_id if there is */
	if ((peer->link_status > TDLS_LINK_CONNECTING) ||
	    (TDLS_STA_INDEX_CHECK((peer->sta_id)))) {
		tdls_notice("link_status %d sta_id %d add peer ignored",
			    peer->link_status, peer->sta_id);
		status = QDF_STATUS_SUCCESS;
		goto addrsp;
	}

	/* when others are on-going, we want to change link_status to idle */
	if (tdls_is_progress(vdev_obj, mac, true)) {
		tdls_notice(QDF_MAC_ADDRESS_STR " TDLS setuping. Req declined.",
			    QDF_MAC_ADDR_ARRAY(mac));
		status = QDF_STATUS_E_PERM;
		goto setlink;
	}

	/* first to check if we reached to maximum supported TDLS peer. */
	curr_tdls_peers = tdls_get_connected_peer(soc_obj);
	if (soc_obj->max_num_tdls_sta <= curr_tdls_peers) {
		tdls_err(QDF_MAC_ADDRESS_STR
			 " Request declined. Current %d, Max allowed %d.",
			 QDF_MAC_ADDR_ARRAY(mac), curr_tdls_peers,
			 soc_obj->max_num_tdls_sta);
		status = QDF_STATUS_E_PERM;
		goto setlink;
	}

	tdls_set_peer_link_status(peer,
				  TDLS_LINK_CONNECTING, TDLS_LINK_SUCCESS);

	status = tdls_pe_add_peer(req);
	if (QDF_IS_STATUS_ERROR(status))
		goto setlink;

	return QDF_STATUS_SUCCESS;

setlink:
	tdls_set_link_status(vdev_obj, mac, TDLS_LINK_IDLE,
			     TDLS_LINK_UNSPECIFIED);
addrsp:
	if (soc_obj->tdls_event_cb) {
		ind.status = status;
		ind.vdev = req->vdev;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_ADD_PEER, &ind);
	}

	return QDF_STATUS_E_PERM;
}

static QDF_STATUS
tdls_add_peer_serialize_callback(struct wlan_serialization_command *cmd,
				 enum wlan_serialization_cb_reason reason)
{
	struct tdls_add_peer_request *req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!cmd || !cmd->umac_cmd) {
		tdls_err("cmd: %p, reason: %d", cmd, reason);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = cmd->umac_cmd;
	tdls_debug("reason: %d, req %p", reason, req);

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		/* command moved to active list
		 */
		status = tdls_activate_add_peer(req);
		break;

	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list.
		 * notify os interface the status
		 */
		status = tdls_internal_add_peer_rsp(req, QDF_STATUS_E_FAILURE);
		break;

	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		/* active command time out. */
		status = tdls_internal_add_peer_rsp(req, QDF_STATUS_E_FAILURE);
		break;

	case WLAN_SER_CB_RELEASE_MEM_CMD:
		/* command successfully completed.
		 * release memory & vdev reference count
		 */
		wlan_objmgr_vdev_release_ref(req->vdev, WLAN_TDLS_NB_ID);
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

QDF_STATUS tdls_process_add_peer(struct tdls_add_peer_request *req)
{
	struct wlan_serialization_command cmd = {0,};
	enum wlan_serialization_status ser_cmd_status;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!req || !req->vdev) {
		tdls_err("req: %p", req);
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	vdev = req->vdev;
	cmd.cmd_type = WLAN_SER_CMD_TDLS_ADD_PEER;
	cmd.cmd_id = 0;
	cmd.cmd_cb = (wlan_serialization_cmd_callback)
		tdls_add_peer_serialize_callback;
	cmd.umac_cmd = req;
	cmd.source = WLAN_UMAC_COMP_TDLS;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = WAIT_TIME_TDLS_ADD_STA;
	cmd.vdev = vdev;

	ser_cmd_status = wlan_serialization_request(&cmd);
	tdls_debug("req: 0x%p wlan_serialization_request status:%d", req,
		   ser_cmd_status);

	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list. Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	case WLAN_SER_CMD_DENIED_LIST_FULL:
	case WLAN_SER_CMD_DENIED_RULES_FAILED:
	case WLAN_SER_CMD_DENIED_UNSPECIFIED:
		/* notify os interface about internal error*/
		status = tdls_internal_add_peer_rsp(req, QDF_STATUS_E_FAILURE);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
		/* cmd can't be serviced.
		 * release tdls_add_peer_request memory
		 */
		qdf_mem_free(req);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		goto error;
	}

	return status;
error:
	status = tdls_internal_add_peer_rsp(req, QDF_STATUS_E_FAILURE);
	qdf_mem_free(req);
	return status;
}

static QDF_STATUS
tdls_activate_update_peer(struct tdls_update_peer_request *req)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *soc_obj;
	struct tdls_vdev_priv_obj *vdev_obj;
	struct wlan_objmgr_vdev *vdev;
	struct tdls_peer *curr_peer;
	uint16_t curr_tdls_peers;
	const uint8_t *mac;
	struct tdls_update_peer_params *update_peer;
	struct tdls_osif_indication ind;

	if (!req->vdev) {
		tdls_err("vdev object NULL when add TDLS peer");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	mac = req->update_peer_req.peer_addr;
	vdev = req->vdev;
	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev);
	vdev_obj = wlan_vdev_get_tdls_vdev_obj(vdev);
	if (!soc_obj || !vdev_obj) {
		tdls_err("soc_obj: %p, vdev_obj: %p", soc_obj, vdev_obj);
		return QDF_STATUS_E_INVAL;
	}

	status = tdls_validate_current_mode(soc_obj);
	if (QDF_IS_STATUS_ERROR(status))
		goto updatersp;

	curr_peer = tdls_get_peer(vdev_obj, mac);
	if (!curr_peer) {
		tdls_err(QDF_MAC_ADDRESS_STR " not exist. return invalid",
			 QDF_MAC_ADDR_ARRAY(mac));
		status = QDF_STATUS_E_INVAL;
		goto updatersp;
	}

	/* in change station, we accept only when sta_id is valid */
	if (curr_peer->link_status > TDLS_LINK_CONNECTING ||
	    !(TDLS_STA_INDEX_CHECK(curr_peer->sta_id))) {
		tdls_err(QDF_MAC_ADDRESS_STR " link %d. sta %d. update peer %s",
			 QDF_MAC_ADDR_ARRAY(mac), curr_peer->link_status,
			 curr_peer->sta_id,
			 (TDLS_STA_INDEX_CHECK(curr_peer->sta_id)) ? "ignored"
			 : "declined");
		status = (TDLS_STA_INDEX_CHECK(curr_peer->sta_id)) ?
			QDF_STATUS_SUCCESS : QDF_STATUS_E_PERM;
		goto updatersp;
	}

	/* when others are on-going, we want to change link_status to idle */
	if (tdls_is_progress(vdev_obj, mac, true)) {
		tdls_notice(QDF_MAC_ADDRESS_STR " TDLS setuping. Req declined.",
			    QDF_MAC_ADDR_ARRAY(mac));
		status = QDF_STATUS_E_PERM;
		goto setlink;
	}

	curr_tdls_peers = tdls_get_connected_peer(soc_obj);
	if (soc_obj->max_num_tdls_sta <= curr_tdls_peers) {
		tdls_err(QDF_MAC_ADDRESS_STR
			 " Request declined. Current: %d, Max allowed: %d.",
			 QDF_MAC_ADDR_ARRAY(mac), curr_tdls_peers,
			 soc_obj->max_num_tdls_sta);
		status = QDF_STATUS_E_PERM;
		goto setlink;
	}
	update_peer = &req->update_peer_req;

	if (update_peer->htcap_present)
		curr_peer->spatial_streams = update_peer->ht_cap.mcsset[1];

	tdls_set_peer_caps(vdev_obj, mac, &req->update_peer_req);
	status = tdls_pe_update_peer(req);
	if (QDF_IS_STATUS_ERROR(status))
		goto setlink;

	return QDF_STATUS_SUCCESS;

setlink:
	tdls_set_link_status(vdev_obj, mac, TDLS_LINK_IDLE,
			     TDLS_LINK_UNSPECIFIED);
updatersp:
	if (soc_obj->tdls_event_cb) {
		ind.status = status;
		ind.vdev = vdev;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_ADD_PEER, &ind);
	}

	return QDF_STATUS_E_PERM;
}

static QDF_STATUS
tdls_update_peer_serialize_callback(struct wlan_serialization_command *cmd,
				    enum wlan_serialization_cb_reason reason)
{
	struct tdls_update_peer_request *req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!cmd || !cmd->umac_cmd) {
		tdls_err("cmd: %p, reason: %d", cmd, reason);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = cmd->umac_cmd;
	tdls_debug("reason: %d, req %p", reason, req);

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		/* command moved to active list
		 */
		status = tdls_activate_update_peer(req);
		break;

	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list.
		 * notify os interface the status
		 */
		status = tdls_internal_update_peer_rsp(req,
						       QDF_STATUS_E_FAILURE);
		break;

	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		/* active command time out. */
		status = tdls_internal_update_peer_rsp(req,
						       QDF_STATUS_E_FAILURE);
		break;

	case WLAN_SER_CB_RELEASE_MEM_CMD:
		/* command successfully completed.
		 * release memory & release reference count
		 */
		wlan_objmgr_vdev_release_ref(req->vdev, WLAN_TDLS_NB_ID);
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

QDF_STATUS tdls_process_update_peer(struct tdls_update_peer_request *req)
{
	struct wlan_serialization_command cmd = {0,};
	enum wlan_serialization_status ser_cmd_status;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!req || !req->vdev) {
		tdls_err("req: %p", req);
		status = QDF_STATUS_E_FAILURE;
		goto error;
	}

	vdev = req->vdev;
	cmd.cmd_type = WLAN_SER_CMD_TDLS_ADD_PEER;
	cmd.cmd_id = 0;
	cmd.cmd_cb = (wlan_serialization_cmd_callback)
		tdls_update_peer_serialize_callback;
	cmd.umac_cmd = req;
	cmd.source = WLAN_UMAC_COMP_TDLS;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = WAIT_TIME_TDLS_ADD_STA;
	cmd.vdev = req->vdev;

	ser_cmd_status = wlan_serialization_request(&cmd);
	tdls_debug("req: 0x%p wlan_serialization_request status:%d", req,
		   ser_cmd_status);

	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list. Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	case WLAN_SER_CMD_DENIED_LIST_FULL:
	case WLAN_SER_CMD_DENIED_RULES_FAILED:
	case WLAN_SER_CMD_DENIED_UNSPECIFIED:
		/* notify os interface about internal error*/
		status = tdls_internal_update_peer_rsp(req,
						       QDF_STATUS_E_FAILURE);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
		/* cmd can't be serviced.
		 * release tdls_add_peer_request memory
		 */
		qdf_mem_free(req);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
error:
	status = tdls_internal_update_peer_rsp(req, QDF_STATUS_E_FAILURE);
	qdf_mem_free(req);
	return status;
}

static QDF_STATUS tdls_activate_del_peer(struct tdls_oper_request *req)
{
	struct tdls_del_peer_request request = {0,};

	request.vdev = req->vdev;
	request.del_peer_req.peer_addr = req->peer_addr;

	return tdls_pe_del_peer(&request);
}

static QDF_STATUS
tdls_del_peer_serialize_callback(struct wlan_serialization_command *cmd,
				 enum wlan_serialization_cb_reason reason)
{
	struct tdls_oper_request *req;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!cmd || !cmd->umac_cmd) {
		tdls_err("cmd: %p, reason: %d", cmd, reason);
		return QDF_STATUS_E_NULL_VALUE;
	}

	req = cmd->umac_cmd;
	tdls_debug("reason: %d, req %p", reason, req);

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		/* command moved to active list
		 */
		status = tdls_activate_del_peer(req);
		break;

	case WLAN_SER_CB_CANCEL_CMD:
		/* command removed from pending list.
		 * notify os interface the status
		 */
		status = tdls_internal_del_peer_rsp(req);
		break;

	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		/* active command time out. */
		status = tdls_internal_del_peer_rsp(req);
		break;

	case WLAN_SER_CB_RELEASE_MEM_CMD:
		/* command successfully completed.
		 * release memory & vdev reference count
		 */
		wlan_objmgr_vdev_release_ref(req->vdev, WLAN_TDLS_NB_ID);
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

QDF_STATUS tdls_process_del_peer(struct tdls_oper_request *req)
{
	struct wlan_serialization_command cmd = {0,};
	enum wlan_serialization_status ser_cmd_status;
	struct wlan_objmgr_vdev *vdev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!req || !req->vdev) {
		tdls_err("req: %p", req);
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	vdev = req->vdev;
	cmd.cmd_type = WLAN_SER_CMD_TDLS_DEL_PEER;
	cmd.cmd_id = 0;
	cmd.cmd_cb = (wlan_serialization_cmd_callback)
		tdls_del_peer_serialize_callback;
	cmd.umac_cmd = req;
	cmd.source = WLAN_UMAC_COMP_TDLS;
	cmd.is_high_priority = false;
	cmd.cmd_timeout_duration = WAIT_TIME_TDLS_DEL_STA;
	cmd.vdev = vdev;

	ser_cmd_status = wlan_serialization_request(&cmd);
	tdls_debug("req: 0x%p wlan_serialization_request status:%d", req,
		   ser_cmd_status);

	switch (ser_cmd_status) {
	case WLAN_SER_CMD_PENDING:
		/* command moved to pending list. Do nothing */
		break;
	case WLAN_SER_CMD_ACTIVE:
		/* command moved to active list. Do nothing */
		break;
	case WLAN_SER_CMD_DENIED_LIST_FULL:
	case WLAN_SER_CMD_DENIED_RULES_FAILED:
	case WLAN_SER_CMD_DENIED_UNSPECIFIED:
		/* notify os interface about internal error*/
		status = tdls_internal_del_peer_rsp(req);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
		/* cmd can't be serviced.
		 * release tdls_add_peer_request memory
		 */
		qdf_mem_free(req);
		break;
	default:
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
error:
	status = tdls_internal_del_peer_rsp(req);
	qdf_mem_free(req);
	return status;
}

/**
 * tdls_process_add_peer_rsp() - handle response for update TDLS peer
 * @rsp: TDLS add peer response
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
static QDF_STATUS tdls_update_peer_rsp(struct tdls_add_sta_rsp *rsp)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct tdls_soc_priv_obj *soc_obj;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct tdls_osif_indication ind;

	psoc = rsp->psoc;
	if (!psoc) {
		tdls_err("psoc is NULL");
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->session_id,
						    WLAN_TDLS_SB_ID);
	if (!vdev) {
		tdls_err("invalid vdev: %d", rsp->session_id);
		status = QDF_STATUS_E_INVAL;
		goto error;
	}

	tdls_release_serialization_command(vdev, WLAN_SER_CMD_TDLS_ADD_PEER);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_SB_ID);
error:
	soc_obj = wlan_psoc_get_tdls_soc_obj(psoc);
	if (soc_obj && soc_obj->tdls_event_cb) {
		ind.status = rsp->status_code;
		ind.vdev = vdev;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_ADD_PEER, &ind);
	}
	qdf_mem_free(rsp);

	return status;
}

/**
 * tdls_add_peer_rsp() - handle response for add TDLS peer
 * @rsp: TDLS add peer response
 *
 * Return: QDF_STATUS_SUCCESS for success; other values if failed
 */
static QDF_STATUS tdls_add_peer_rsp(struct tdls_add_sta_rsp *rsp)
{
	uint8_t sta_idx;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct tdls_vdev_priv_obj *vdev_obj;
	struct tdls_soc_priv_obj *soc_obj = NULL;
	struct tdls_conn_info *conn_rec;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct tdls_osif_indication ind;

	psoc = rsp->psoc;
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->session_id,
						    WLAN_TDLS_SB_ID);
	if (!vdev) {
		tdls_err("invalid vdev: %d", rsp->session_id);
		status =  QDF_STATUS_E_INVAL;
		goto error;
	}
	soc_obj = wlan_psoc_get_tdls_soc_obj(psoc);
	vdev_obj = wlan_vdev_get_tdls_vdev_obj(vdev);
	if (!soc_obj || !vdev_obj) {
		tdls_err("soc object:%p, vdev object:%p", soc_obj, vdev_obj);
		status = QDF_STATUS_E_FAILURE;
		goto cmddone;
	}
	if (rsp->status_code) {
		tdls_err("add sta failed. status code(=%d)", rsp->status_code);
		status = QDF_STATUS_E_FAILURE;
	} else {
		conn_rec = soc_obj->tdls_conn_info;
		for (sta_idx = 0; sta_idx < soc_obj->max_num_tdls_sta;
		     sta_idx++) {
			if (0 == conn_rec[sta_idx].sta_id) {
				conn_rec[sta_idx].session_id = rsp->session_id;
				conn_rec[sta_idx].sta_id = rsp->sta_id;
				qdf_copy_macaddr(&conn_rec[sta_idx].peer_mac,
						 &rsp->peermac);
				tdls_warn("TDLS: STA IDX at %d is %d of mac "
					  QDF_MAC_ADDRESS_STR, sta_idx,
					  rsp->sta_id, QDF_MAC_ADDR_ARRAY
					  (rsp->peermac.bytes));
				break;
			}
		}

		if (sta_idx < soc_obj->max_num_tdls_sta) {
			status = tdls_set_sta_id(vdev_obj, rsp->peermac.bytes,
						 rsp->sta_id);
			if (QDF_IS_STATUS_ERROR(status)) {
				tdls_err("set staid failed");
				status = QDF_STATUS_E_FAILURE;
				goto cmddone;
			}

			tdls_set_signature(vdev_obj, rsp->peermac.bytes,
					   rsp->ucast_sig);
		} else {
			status = QDF_STATUS_E_FAILURE;
		}
	}

cmddone:
	tdls_release_serialization_command(vdev, WLAN_SER_CMD_TDLS_ADD_PEER);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_SB_ID);
error:
	if (soc_obj && soc_obj->tdls_event_cb) {
		ind.vdev = vdev;
		ind.status = rsp->status_code;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_ADD_PEER, &ind);
	}
	qdf_mem_free(rsp);

	return status;
}

QDF_STATUS tdls_process_add_peer_rsp(struct tdls_add_sta_rsp *rsp)
{
	tdls_debug("peer oper %d", rsp->tdls_oper);

	if (rsp->tdls_oper == TDLS_OPER_ADD)
		return tdls_add_peer_rsp(rsp);
	else if (rsp->tdls_oper == TDLS_OPER_UPDATE)
		return tdls_update_peer_rsp(rsp);

	return QDF_STATUS_E_INVAL;
}

QDF_STATUS tdls_process_del_peer_rsp(struct tdls_del_sta_rsp *rsp)
{
	uint8_t sta_idx, id;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	struct tdls_vdev_priv_obj *vdev_obj;
	struct tdls_soc_priv_obj *soc_obj = NULL;
	struct tdls_conn_info *conn_rec;
	struct tdls_peer *curr_peer = NULL;
	const uint8_t *macaddr;
	struct tdls_osif_indication ind;

	tdls_debug("del peer rsp: vdev %d  peer " QDF_MAC_ADDRESS_STR,
		   rsp->session_id, QDF_MAC_ADDR_ARRAY(rsp->peermac.bytes));
	psoc = rsp->psoc;
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, rsp->session_id,
						    WLAN_TDLS_SB_ID);
	if (!vdev) {
		tdls_err("invalid vdev: %d", rsp->session_id);
		status = QDF_STATUS_E_INVAL;
		goto error;
	}
	soc_obj = wlan_psoc_get_tdls_soc_obj(psoc);
	vdev_obj = wlan_vdev_get_tdls_vdev_obj(vdev);
	if (!soc_obj || !vdev_obj) {
		tdls_err("soc object:%p, vdev object:%p", soc_obj, vdev_obj);
		status = QDF_STATUS_E_FAILURE;
		goto cmddone;
	}

	conn_rec = soc_obj->tdls_conn_info;
	for (sta_idx = 0; sta_idx < soc_obj->max_num_tdls_sta; sta_idx++) {
		if (conn_rec[sta_idx].session_id != rsp->session_id ||
		    conn_rec[sta_idx].sta_id != rsp->sta_id)
			continue;

		macaddr = rsp->peermac.bytes;
		tdls_warn("TDLS: del STA IDX = %x", rsp->sta_id);
		curr_peer = tdls_find_peer(vdev_obj, macaddr);
		if (curr_peer) {
			tdls_debug(QDF_MAC_ADDRESS_STR " status is %d",
				   QDF_MAC_ADDR_ARRAY(macaddr),
				   curr_peer->link_status);
			wlan_vdev_obj_lock(vdev);
			id = wlan_vdev_get_id(vdev);
			wlan_vdev_obj_unlock(vdev);
			if (TDLS_IS_CONNECTED(curr_peer)) {
				soc_obj->tdls_dereg_tl_peer(
					soc_obj->tdls_tl_peer_data,
					id, curr_peer->sta_id);
				tdls_decrement_peer_count(soc_obj);
			} else if (TDLS_LINK_CONNECTING ==
				   curr_peer->link_status) {
				soc_obj->tdls_dereg_tl_peer(
					soc_obj->tdls_tl_peer_data,
					id, curr_peer->sta_id);
			}
		}
		tdls_reset_peer(vdev_obj, macaddr);
		conn_rec[sta_idx].sta_id = 0;
		conn_rec[sta_idx].session_id = 0xff;
		qdf_mem_zero(&conn_rec[sta_idx].peer_mac,
			     QDF_MAC_ADDR_SIZE);

		status = QDF_STATUS_SUCCESS;
		break;
	}
	macaddr = rsp->peermac.bytes;
	if (!curr_peer) {
		curr_peer = tdls_find_peer(vdev_obj, macaddr);

		if (curr_peer)
			tdls_set_peer_link_status(curr_peer, TDLS_LINK_IDLE,
						  (curr_peer->link_status ==
						   TDLS_LINK_TEARING) ?
						  TDLS_LINK_UNSPECIFIED :
						  TDLS_LINK_DROPPED_BY_REMOTE);
	}

cmddone:
	tdls_release_serialization_command(vdev, WLAN_SER_CMD_TDLS_DEL_PEER);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_SB_ID);
error:

	if (soc_obj && soc_obj->tdls_event_cb) {
		ind.vdev = vdev;
		soc_obj->tdls_event_cb(soc_obj->tdls_evt_cb_data,
				       TDLS_EVENT_DEL_PEER, &ind);
	}
	qdf_mem_free(rsp);

	return status;
}

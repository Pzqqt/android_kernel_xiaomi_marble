/*
 * Copyright (c) 2012-2015, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: Implements general util apis of connection manager
 */

#include "wlan_cm_main_api.h"
#include "wlan_scan_api.h"
#include "wlan_cm_public_struct.h"
#include "wlan_serialization_api.h"
#include "wlan_cm_bss_score_param.h"
#ifdef WLAN_POLICY_MGR_ENABLE
#include <wlan_policy_mgr_api.h>
#endif
#include "wlan_cm_roam.h"
#include <qdf_platform.h>

static uint32_t cm_get_prefix_for_cm_id(enum wlan_cm_source source) {
	switch (source) {
	case CM_OSIF_CONNECT:
	case CM_OSIF_CFG_CONNECT:
		return CONNECT_REQ_PREFIX;
	case CM_ROAMING_HOST:
	case CM_ROAMING_FW:
	case CM_ROAMING_NUD_FAILURE:
		return ROAM_REQ_PREFIX;
	default:
		return DISCONNECT_REQ_PREFIX;
	}
}

wlan_cm_id cm_get_cm_id(struct cnx_mgr *cm_ctx, enum wlan_cm_source source)
{
	wlan_cm_id cm_id;
	uint32_t prefix;
	uint8_t vdev_id = wlan_vdev_get_id(cm_ctx->vdev);

	prefix = cm_get_prefix_for_cm_id(source);

	cm_id = qdf_atomic_inc_return(&cm_ctx->global_cmd_id);
	cm_id = (cm_id & CM_ID_MASK);
	cm_id = CM_ID_SET_VDEV_ID(cm_id, vdev_id);
	cm_id = (cm_id | prefix);

	return cm_id;
}

struct cnx_mgr *cm_get_cm_ctx_fl(struct wlan_objmgr_vdev *vdev,
				 const char *func, uint32_t line)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct cnx_mgr *cm_ctx = NULL;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (vdev_mlme)
		cm_ctx = vdev_mlme->cnx_mgr_ctx;

	if (!cm_ctx)
		mlme_nofl_err("%s:%u: vdev %d cm_ctx is NULL", func, line,
			      wlan_vdev_get_id(vdev));

	return cm_ctx;
}

cm_ext_t *cm_get_ext_hdl_fl(struct wlan_objmgr_vdev *vdev,
			    const char *func, uint32_t line)
{
	struct cnx_mgr *cm_ctx;
	cm_ext_t *ext_ctx = NULL;

	cm_ctx = cm_get_cm_ctx_fl(vdev, func, line);
	if (cm_ctx)
		ext_ctx = cm_ctx->ext_cm_ptr;

	if (!ext_ctx)
		mlme_nofl_err("%s:%u: vdev %d cm ext ctx is NULL", func, line,
			      wlan_vdev_get_id(vdev));
	return ext_ctx;
}

void cm_reset_active_cm_id(struct wlan_objmgr_vdev *vdev, wlan_cm_id cm_id)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	/* Reset active cm id if cm id match */
	if (cm_ctx->active_cm_id == cm_id)
		cm_ctx->active_cm_id = CM_ID_INVALID;
}


#ifdef WLAN_CM_USE_SPINLOCK
/**
 * cm_req_lock_acquire - acquire CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * acquire CM SM mutex/spinlock
 *
 * return: void
 */
inline void cm_req_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_acquire(&cm_ctx->cm_req_lock);
}

/**
 * cm_req_lock_release - release CM SM mutex/spinlock
 * @cm_ctx:  connection manager ctx
 *
 * release CM SM mutex/spinlock
 *
 * return: void
 */
inline void cm_req_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_release(&cm_ctx->cm_req_lock);
}

QDF_STATUS cm_activate_cmd_req_flush_cb(struct scheduler_msg *msg)
{
	struct wlan_serialization_command *cmd = msg->bodyptr;

	if (!cmd || !cmd->vdev) {
		mlme_err("Null input cmd:%pK", cmd);
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_vdev_release_ref(cmd->vdev, WLAN_MLME_CM_ID);
	return QDF_STATUS_SUCCESS;
}

#else
inline void cm_req_lock_acquire(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_acquire(&cm_ctx->cm_req_lock);
}

inline void cm_req_lock_release(struct cnx_mgr *cm_ctx)
{
	qdf_mutex_release(&cm_ctx->cm_req_lock);
}
#endif /* WLAN_CM_USE_SPINLOCK */

#ifdef CRYPTO_SET_KEY_CONVERGED
QDF_STATUS cm_set_key(struct cnx_mgr *cm_ctx, bool unicast,
		      uint8_t key_idx, struct qdf_mac_addr *bssid)
{
	enum wlan_crypto_cipher_type cipher;
	struct wlan_crypto_key *crypto_key;
	uint8_t wep_key_idx = 0;

	cipher = wlan_crypto_get_cipher(cm_ctx->vdev, unicast, key_idx);
	if (IS_WEP_CIPHER(cipher)) {
		wep_key_idx = wlan_crypto_get_default_key_idx(cm_ctx->vdev,
							      false);
		crypto_key = wlan_crypto_get_key(cm_ctx->vdev, wep_key_idx);
		qdf_mem_copy(crypto_key->macaddr, bssid->bytes,
			     QDF_MAC_ADDR_SIZE);
	} else {
		crypto_key = wlan_crypto_get_key(cm_ctx->vdev, key_idx);
	}

	return wlan_crypto_set_key_req(cm_ctx->vdev, crypto_key, (unicast ?
				       WLAN_CRYPTO_KEY_TYPE_UNICAST :
				       WLAN_CRYPTO_KEY_TYPE_GROUP));
}
#endif

#ifdef CONN_MGR_ADV_FEATURE
void cm_store_wep_key(struct cnx_mgr *cm_ctx,
		      struct wlan_cm_connect_crypto_info *crypto,
		      wlan_cm_id cm_id)
{
	struct wlan_crypto_key *crypto_key = NULL;
	QDF_STATUS status;
	enum wlan_crypto_cipher_type cipher_type;
	struct wlan_cm_wep_key_params *wep_keys;

	if (!(crypto->ciphers_pairwise & (1 << WLAN_CRYPTO_CIPHER_WEP_40 |
					  1 << WLAN_CRYPTO_CIPHER_WEP_104)))
		return;

	if (crypto->ciphers_pairwise & 1 << WLAN_CRYPTO_CIPHER_WEP_40)
		cipher_type = WLAN_CRYPTO_CIPHER_WEP_40;
	else
		cipher_type = WLAN_CRYPTO_CIPHER_WEP_104;

	wep_keys = &crypto->wep_keys;
	status = wlan_crypto_validate_key_params(cipher_type,
						 wep_keys->key_idx,
						 wep_keys->key_len,
						 wep_keys->seq_len);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err(CM_PREFIX_FMT "Invalid key params",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		return;
	}

	crypto_key = wlan_crypto_get_key(cm_ctx->vdev, wep_keys->key_idx);
	if (!crypto_key) {
		crypto_key = qdf_mem_malloc(sizeof(*crypto_key));
		if (!crypto_key)
			return;

		status = wlan_crypto_save_key(cm_ctx->vdev, wep_keys->key_idx,
					      crypto_key);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err(CM_PREFIX_FMT "Failed to save key",
				 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					       cm_id));
			qdf_mem_free(crypto_key);
			return;
		}
	}
	qdf_mem_zero(crypto_key, sizeof(*crypto_key));
	crypto_key->cipher_type = cipher_type;
	crypto_key->keylen = wep_keys->key_len;
	crypto_key->keyix = wep_keys->key_idx;
	qdf_mem_copy(&crypto_key->keyval[0], wep_keys->key, wep_keys->key_len);
	qdf_mem_copy(&crypto_key->keyrsc[0], wep_keys->seq, wep_keys->seq_len);
	mlme_debug(CM_PREFIX_FMT "cipher_type %d key_len %d, seq_len %d",
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
		   crypto_key->cipher_type, wep_keys->key_len,
		   wep_keys->seq_len);
}

void cm_trigger_panic_on_cmd_timeout(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return;

	if (qdf_is_recovering() || qdf_is_fw_down())
		return;

	qdf_trigger_self_recovery(psoc, QDF_ACTIVE_LIST_TIMEOUT);
}

#else
void cm_trigger_panic_on_cmd_timeout(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme = NULL;
	struct wlan_sm *vdev_sm = NULL;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
			vdev,
			WLAN_UMAC_COMP_MLME);
	if (!vdev_mlme) {
		mlme_err("VDEV MLME is null");
		goto error;
	}

	vdev_sm = vdev_mlme->sm_hdl;
	if (!vdev_sm) {
		mlme_err("VDEV SM is null");
		goto error;
	}

	wlan_sm_print_history(vdev_sm);
	cm_sm_history_print(vdev);
error:
	QDF_ASSERT(0);
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
void cm_store_fils_key(struct cnx_mgr *cm_ctx, bool unicast,
		       uint8_t key_id, uint16_t key_length,
		       uint8_t *key, struct qdf_mac_addr *bssid,
		       wlan_cm_id cm_id)
{
	struct wlan_crypto_key *crypto_key = NULL;
	QDF_STATUS status;
	uint8_t i;
	int32_t cipher;
	enum wlan_crypto_cipher_type cipher_type = WLAN_CRYPTO_CIPHER_NONE;

	if (unicast)
		cipher = wlan_crypto_get_param(cm_ctx->vdev,
					       WLAN_CRYPTO_PARAM_UCAST_CIPHER);
	else
		cipher = wlan_crypto_get_param(cm_ctx->vdev,
					       WLAN_CRYPTO_PARAM_MCAST_CIPHER);

	for (i = 0; i <= WLAN_CRYPTO_CIPHER_MAX; i++) {
		if (QDF_HAS_PARAM(cipher, i)) {
			cipher_type = i;
			break;
		}
	}
	crypto_key = wlan_crypto_get_key(cm_ctx->vdev, key_id);
	if (!crypto_key) {
		crypto_key = qdf_mem_malloc(sizeof(*crypto_key));
		if (!crypto_key)
			return;
		status = wlan_crypto_save_key(cm_ctx->vdev, key_id, crypto_key);
		if (QDF_IS_STATUS_ERROR(status)) {
			mlme_err(CM_PREFIX_FMT "Failed to save key",
				 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					       cm_id));
			qdf_mem_free(crypto_key);
			return;
		}
	}
	qdf_mem_zero(crypto_key, sizeof(*crypto_key));
	crypto_key->cipher_type = cipher_type;
	crypto_key->keylen = key_length;
	crypto_key->keyix = key_id;
	qdf_mem_copy(&crypto_key->keyval[0], key, key_length);
	qdf_mem_copy(crypto_key->macaddr, bssid->bytes, QDF_MAC_ADDR_SIZE);
	mlme_debug(CM_PREFIX_FMT "cipher_type %d key_len %d, key_id %d mac:" QDF_MAC_ADDR_FMT,
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
		   crypto_key->cipher_type, crypto_key->keylen,
		   crypto_key->keyix, QDF_MAC_ADDR_REF(crypto_key->macaddr));
}
static void cm_set_fils_connection_from_req(struct wlan_cm_connect_req *req,
					    struct wlan_cm_connect_resp *resp)
{
	resp->is_fils_connection = req->fils_info.is_fils_connection;
}
#else
static inline
void cm_set_fils_connection_from_req(struct wlan_cm_connect_req *req,
				     struct wlan_cm_connect_resp *resp)
{}
#endif

bool cm_check_cmid_match_list_head(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id)
{
	qdf_list_node_t *cur_node = NULL;
	struct cm_req *cm_req;
	bool match = false;
	wlan_cm_id head_cm_id = 0;

	if (!cm_id)
		return false;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	if (!cur_node)
		goto exit;

	cm_req = qdf_container_of(cur_node, struct cm_req, node);
	head_cm_id = cm_req->cm_id;
	if (head_cm_id == *cm_id)
		match = true;

exit:
	cm_req_lock_release(cm_ctx);
	if (!match)
		mlme_info("head_cm_id 0x%x didn't match the given cm_id 0x%x",
			  head_cm_id, *cm_id);

	return match;
}

bool cm_check_scanid_match_list_head(struct cnx_mgr *cm_ctx,
				     wlan_scan_id *scan_id)
{
	qdf_list_node_t *cur_node = NULL;
	struct cm_req *cm_req;
	bool match = false;
	wlan_cm_id head_scan_id = 0;
	uint32_t prefix = 0;

	if (!scan_id)
		return false;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	if (!cur_node)
		goto exit;

	cm_req = qdf_container_of(cur_node, struct cm_req, node);
	prefix = CM_ID_GET_PREFIX(cm_req->cm_id);
	/* Check only if head is connect req */
	if (prefix != CONNECT_REQ_PREFIX)
		goto exit;
	head_scan_id = cm_req->connect_req.scan_id;
	if (head_scan_id == *scan_id)
		match = true;

exit:
	cm_req_lock_release(cm_ctx);
	if (!match)
		mlme_info("head_scan_id 0x%x didn't match the given scan_id 0x%x prefix 0x%x",
			  head_scan_id, *scan_id, prefix);

	return match;
}

struct cm_req *cm_get_req_by_cm_id_fl(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id,
				      const char *func, uint32_t line)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req * cm_req;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			cm_req_lock_release(cm_ctx);
			return cm_req;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	mlme_nofl_info("%s:%u: cm req not found for cm id 0x%x", func,
		       line, cm_id);

	return NULL;
}

/**
 * cm_fill_connect_resp_from_req() - Fill connect resp from connect request
 * @resp: cm connect response
 * @cm_req: cm request
 *
 * Context: Can be called from APIs holding cm request list lock
 *
 * Return: void
 */
static void
cm_fill_connect_resp_from_req(struct wlan_cm_connect_resp *resp,
			      struct cm_req *cm_req)
{
	struct scan_cache_node *candidate;
	struct wlan_cm_connect_req *req;

	req = &cm_req->connect_req.req;
	candidate = cm_req->connect_req.cur_candidate;
	if (candidate)
		qdf_copy_macaddr(&resp->bssid, &candidate->entry->bssid);
	else if (!qdf_is_macaddr_zero(&req->bssid))
		qdf_copy_macaddr(&resp->bssid, &req->bssid);
	else
		qdf_copy_macaddr(&resp->bssid, &req->bssid_hint);

	if (candidate)
		resp->freq = candidate->entry->channel.chan_freq;
	else
		resp->freq = req->chan_freq;

	resp->ssid = req->ssid;
	resp->is_wps_connection = req->is_wps_connection;
	resp->is_osen_connection = req->is_osen_connection;
	cm_set_fils_connection_from_req(req, resp);
}

/**
 * cm_handle_connect_flush() - Fill fail connect resp from req and indicate
 * same to osif
 * @cm_ctx: connection manager context
 * @cm_req: cm request
 *
 * Context: Can be called from APIs holding cm request list lock
 *
 * Return: void
 */
static void
cm_handle_connect_flush(struct cnx_mgr *cm_ctx, struct cm_req *cm_req)
{
	struct wlan_cm_connect_resp *resp;

	resp = qdf_mem_malloc(sizeof(*resp));
	if (!resp)
		return;

	resp->connect_status = QDF_STATUS_E_FAILURE;
	resp->cm_id = cm_req->cm_id;
	resp->vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	if (cm_req->failed_req)
		resp->reason = CM_GENERIC_FAILURE;
	else
		resp->reason = CM_ABORT_DUE_TO_NEW_REQ_RECVD;

	/* Get bssid and ssid and freq for the cm id from the req list */
	cm_fill_connect_resp_from_req(resp, cm_req);

	cm_notify_connect_complete(cm_ctx, resp);
	qdf_mem_free(resp);
}

/**
 * cm_handle_disconnect_flush() - Fill disconnect resp from req and indicate
 * same to osif
 * @cm_ctx: connection manager context
 * @cm_req: cm request
 *
 * Context: Can be called from APIs holding cm request list lock
 *
 * Return: void
 */
static void
cm_handle_disconnect_flush(struct cnx_mgr *cm_ctx, struct cm_req *cm_req)
{
	struct wlan_cm_discon_rsp resp;

	qdf_mem_zero(&resp, sizeof(resp));
	resp.req.cm_id = cm_req->cm_id;
	resp.req.req = cm_req->discon_req.req;

	cm_notify_disconnect_complete(cm_ctx, &resp);
}

void cm_remove_cmd_from_serialization(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	struct wlan_serialization_queued_cmd_info cmd_info;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);

	qdf_mem_zero(&cmd_info, sizeof(cmd_info));
	cmd_info.cmd_id = cm_id;
	cmd_info.req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;

	if (prefix == CONNECT_REQ_PREFIX) {
		cmd_info.cmd_type = WLAN_SER_CMD_VDEV_CONNECT;
	} else if (prefix == ROAM_REQ_PREFIX) {
		/*
		 * Try removing PREAUTH command when in preauth state else
		 * try remove ROAM command
		 */
		if (cm_ctx->preauth_in_progress)
			cmd_info.cmd_type = WLAN_SER_CMD_PERFORM_PRE_AUTH;
		else
			cmd_info.cmd_type = WLAN_SER_CMD_VDEV_ROAM;
		cm_ctx->preauth_in_progress = false;
	} else if (prefix == DISCONNECT_REQ_PREFIX) {
		cmd_info.cmd_type = WLAN_SER_CMD_VDEV_DISCONNECT;
	} else {
		mlme_err(CM_PREFIX_FMT "Invalid prefix %x",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
			 prefix);
		return;
	}

	cmd_info.vdev = cm_ctx->vdev;

	if (cm_id == cm_ctx->active_cm_id) {
		mlme_debug(CM_PREFIX_FMT "Remove cmd type %d from active",
			   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
			   cmd_info.cmd_type);
		cmd_info.queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_serialization_remove_cmd(&cmd_info);
	} else {
		mlme_debug(CM_PREFIX_FMT "Remove cmd type %d from pending",
			   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id),
			   cmd_info.cmd_type);
		cmd_info.queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_serialization_cancel_request(&cmd_info);
	}
}

void
cm_flush_pending_request(struct cnx_mgr *cm_ctx, uint32_t prefix,
			 bool only_failed_req)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t req_prefix;
	bool roam_offload = false;

	if (cm_roam_offload_enabled(wlan_vdev_get_psoc(cm_ctx->vdev)) &&
	     prefix == ROAM_REQ_PREFIX)
		roam_offload = true;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		req_prefix = CM_ID_GET_PREFIX(cm_req->cm_id);

		/*
		 * Only remove requests matching the flush prefix and
		 * the pending req for non roam offload(LFR3) commands
		 * (roam command is dummy in FW roam/LFR3 so active
		 * command can be removed)
		 */
		if (req_prefix != prefix ||
		    (!roam_offload && cm_req->cm_id == cm_ctx->active_cm_id))
			goto next;

		/* If only_failed_req is set flush only failed req */
		if (only_failed_req && !cm_req->failed_req)
			goto next;

		if (req_prefix == CONNECT_REQ_PREFIX) {
			cm_handle_connect_flush(cm_ctx, cm_req);
			cm_ctx->connect_count--;
			cm_free_connect_req_mem(&cm_req->connect_req);
		} else if (req_prefix == ROAM_REQ_PREFIX) {
			cm_free_roam_req_mem(&cm_req->roam_req);
		} else if (req_prefix == DISCONNECT_REQ_PREFIX) {
			cm_handle_disconnect_flush(cm_ctx, cm_req);
			cm_ctx->disconnect_count--;
		} else {
			mlme_err(CM_PREFIX_FMT "Invalid prefix %x",
				 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					       cm_req->cm_id), prefix);
		}

		cm_req_history_del(cm_ctx, cm_req, CM_REQ_DEL_FLUSH);
		mlme_debug(CM_PREFIX_FMT,
			   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					 cm_req->cm_id));
		cm_remove_cmd_from_serialization(cm_ctx, cm_req->cm_id);
		qdf_list_remove_node(&cm_ctx->req_list, &cm_req->node);
		qdf_mem_free(cm_req);
next:
		cur_node = next_node;
		next_node = NULL;
	}

	cm_req_lock_release(cm_ctx);
}

QDF_STATUS
cm_fill_bss_info_in_connect_rsp_by_cm_id(struct cnx_mgr *cm_ctx,
					 wlan_cm_id cm_id,
					 struct wlan_cm_connect_resp *resp)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != CONNECT_REQ_PREFIX)
		return QDF_STATUS_E_INVAL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			cm_fill_connect_resp_from_req(resp, cm_req);
			cm_req_lock_release(cm_ctx);
			return QDF_STATUS_SUCCESS;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_E_FAILURE;
}

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
bool cm_is_cm_id_current_candidate_single_pmk(struct cnx_mgr *cm_ctx,
					      wlan_cm_id cm_id)
{
	struct wlan_objmgr_psoc *psoc;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);
	int32_t akm;
	struct scan_cache_node *candidate;
	bool is_single_pmk = false;

	psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_id));
		return is_single_pmk;
	}

	if (prefix != CONNECT_REQ_PREFIX)
		return is_single_pmk;

	akm = wlan_crypto_get_param(cm_ctx->vdev, WLAN_CRYPTO_PARAM_KEY_MGMT);
	if (!QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_SAE))
		return is_single_pmk;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			candidate = cm_req->connect_req.cur_candidate;
			if (candidate &&
			    util_scan_entry_single_pmk(psoc, candidate->entry))
				is_single_pmk = true;
			break;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return is_single_pmk;
}
#endif

QDF_STATUS cm_add_req_to_list_and_indicate_osif(struct cnx_mgr *cm_ctx,
						struct cm_req *cm_req,
						enum wlan_cm_source source)
{
	uint32_t prefix = CM_ID_GET_PREFIX(cm_req->cm_id);

	cm_req_lock_acquire(cm_ctx);
	if (qdf_list_size(&cm_ctx->req_list) >= CM_MAX_REQ) {
		cm_req_lock_release(cm_ctx);
		mlme_err(CM_PREFIX_FMT "List full size %d",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_req->cm_id),
			 qdf_list_size(&cm_ctx->req_list));
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_insert_front(&cm_ctx->req_list, &cm_req->node);
	if (prefix == CONNECT_REQ_PREFIX)
		cm_ctx->connect_count++;
	else if (prefix == DISCONNECT_REQ_PREFIX)
		cm_ctx->disconnect_count++;

	cm_req_history_add(cm_ctx, cm_req);
	cm_req_lock_release(cm_ctx);
	mlme_debug(CM_PREFIX_FMT,
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				 cm_req->cm_id));

	mlme_cm_osif_update_id_and_src(cm_ctx->vdev, source, cm_req->cm_id);

	return QDF_STATUS_SUCCESS;
}

static void cm_zero_and_free_memory(uint8_t *ptr, uint32_t len)
{
	if (!ptr)
		return;

	qdf_mem_zero(ptr, len);
	qdf_mem_free(ptr);
}

void cm_free_connect_req_mem(struct cm_connect_req *connect_req)
{
	struct wlan_cm_connect_req *req;

	req = &connect_req->req;

	if (connect_req->candidate_list)
		wlan_scan_purge_results(connect_req->candidate_list);

	cm_zero_and_free_memory(req->assoc_ie.ptr, req->assoc_ie.len);
	cm_zero_and_free_memory(req->scan_ie.ptr, req->scan_ie.len);

	cm_zero_and_free_memory(req->crypto.wep_keys.key,
				req->crypto.wep_keys.key_len);
	cm_zero_and_free_memory(req->crypto.wep_keys.seq,
				req->crypto.wep_keys.seq_len);

	qdf_mem_zero(connect_req, sizeof(*connect_req));
}

QDF_STATUS
cm_delete_req_from_list(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id)
{
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		if (cm_req->cm_id == cm_id)
			break;

		cur_node = next_node;
		next_node = NULL;
		cm_req = NULL;
	}

	if (!cm_req) {
		cm_req_lock_release(cm_ctx);
		mlme_err(CM_PREFIX_FMT " req not found",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_remove_node(&cm_ctx->req_list, &cm_req->node);
	if (prefix == CONNECT_REQ_PREFIX) {
		cm_ctx->connect_count--;
		cm_free_connect_req_mem(&cm_req->connect_req);
	} else if (prefix == ROAM_REQ_PREFIX) {
		cm_free_roam_req_mem(&cm_req->roam_req);
	} else if (prefix == DISCONNECT_REQ_PREFIX) {
		cm_ctx->disconnect_count--;
	} else {
		mlme_err(CM_PREFIX_FMT "Invalid prefix %x",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				       cm_req->cm_id), prefix);
	}

	if (cm_id == cm_ctx->active_cm_id)
		cm_req_history_del(cm_ctx, cm_req, CM_REQ_DEL_ACTIVE);
	else
		cm_req_history_del(cm_ctx, cm_req, CM_REQ_DEL_PENDING);

	mlme_debug(CM_PREFIX_FMT,
		   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
				 cm_req->cm_id));

	qdf_mem_free(cm_req);
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_SUCCESS;
}

void cm_remove_cmd(struct cnx_mgr *cm_ctx, wlan_cm_id *cm_id_to_remove)
{
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;
	wlan_cm_id cm_id;

	if (!cm_id_to_remove) {
		mlme_err("cm_id_to_remove is null");
		return;
	}

	/*
	 * store local value as cm_delete_req_from_list may free the
	 * cm_id_to_remove pointer
	 */
	cm_id = *cm_id_to_remove;
	/* return if zero or invalid cm_id */
	if (!cm_id || cm_id == CM_ID_INVALID) {
		mlme_debug(CM_PREFIX_FMT " Invalid cm_id",
			   CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev),
					 cm_id));
		return;
	}

	psoc = wlan_vdev_get_psoc(cm_ctx->vdev);
	if (!psoc) {
		mlme_err(CM_PREFIX_FMT "Failed to find psoc",
			 CM_PREFIX_REF(wlan_vdev_get_id(cm_ctx->vdev), cm_id));
		return;
	}

	status = cm_delete_req_from_list(cm_ctx, cm_id);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	cm_remove_cmd_from_serialization(cm_ctx, cm_id);
}

void cm_vdev_scan_cancel(struct wlan_objmgr_pdev *pdev,
			 struct wlan_objmgr_vdev *vdev)
{
	struct scan_cancel_request *req;
	QDF_STATUS status;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req)
		return;

	req->vdev = vdev;
	req->cancel_req.scan_id = INVAL_SCAN_ID;
	req->cancel_req.vdev_id = wlan_vdev_get_id(vdev);
	req->cancel_req.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	req->cancel_req.req_type = WLAN_SCAN_CANCEL_VDEV_ALL;

	status = wlan_scan_cancel(req);
	/* In success/failure case wlan_scan_cancel free the req memory */
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("vdev %d cancel scan request failed",
			 wlan_vdev_get_id(vdev));
}

void cm_set_max_connect_attempts(struct wlan_objmgr_vdev *vdev,
				 uint8_t max_connect_attempts)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	cm_ctx->max_connect_attempts =
		QDF_MIN(max_connect_attempts, CM_MAX_CONNECT_ATTEMPTS);
	mlme_debug("vdev %d max connect attempts set to %d, requested %d",
		   wlan_vdev_get_id(vdev),
		   cm_ctx->max_connect_attempts, max_connect_attempts);
}

void cm_set_max_connect_timeout(struct wlan_objmgr_vdev *vdev,
				uint32_t max_connect_timeout)
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	cm_ctx->connect_timeout = max_connect_timeout;
}

QDF_STATUS
cm_fill_disconnect_resp_from_cm_id(struct cnx_mgr *cm_ctx, wlan_cm_id cm_id,
				   struct wlan_cm_discon_rsp *resp)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);

	if (prefix != DISCONNECT_REQ_PREFIX)
		return QDF_STATUS_E_INVAL;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->cm_id == cm_id) {
			resp->req.cm_id = cm_id;
			resp->req.req = cm_req->discon_req.req;
			cm_req_lock_release(cm_ctx);
			return QDF_STATUS_SUCCESS;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return QDF_STATUS_E_FAILURE;
}

void cm_inform_bcn_probe(struct cnx_mgr *cm_ctx, uint8_t *bcn_probe,
			 uint32_t len, qdf_freq_t freq, int32_t rssi,
			 wlan_cm_id cm_id)
{
	qdf_nbuf_t buf;
	struct wlan_objmgr_pdev *pdev;
	uint8_t *data, i, vdev_id;
	struct mgmt_rx_event_params rx_param = {0};
	struct wlan_frame_hdr *hdr;
	enum mgmt_frame_type frm_type = MGMT_BEACON;

	vdev_id = wlan_vdev_get_id(cm_ctx->vdev);
	if (!bcn_probe || !len || (len < sizeof(*hdr))) {
		mlme_err(CM_PREFIX_FMT "bcn_probe is null or invalid len %d",
			 CM_PREFIX_REF(vdev_id, cm_id), len);
		return;
	}

	pdev = wlan_vdev_get_pdev(cm_ctx->vdev);
	if (!pdev) {
		mlme_err(CM_PREFIX_FMT "Failed to find pdev",
			 CM_PREFIX_REF(vdev_id, cm_id));
		return;
	}

	hdr = (struct wlan_frame_hdr *)bcn_probe;
	if ((hdr->i_fc[0] & QDF_IEEE80211_FC0_SUBTYPE_MASK) ==
	    MGMT_SUBTYPE_PROBE_RESP)
		frm_type = MGMT_PROBE_RESP;

	rx_param.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	rx_param.chan_freq = freq;
	rx_param.rssi = rssi;

	/* Set all per chain rssi as invalid */
	for (i = 0; i < WLAN_MGMT_TXRX_HOST_MAX_ANTENNA; i++)
		rx_param.rssi_ctl[i] = WLAN_INVALID_PER_CHAIN_RSSI;

	buf = qdf_nbuf_alloc(NULL, qdf_roundup(len, 4), 0, 4, false);
	if (!buf)
		return;

	qdf_nbuf_put_tail(buf, len);
	qdf_nbuf_set_protocol(buf, ETH_P_CONTROL);

	data = qdf_nbuf_data(buf);
	qdf_mem_copy(data, bcn_probe, len);
	/* buf will be freed by scan module in error or success case */
	wlan_scan_process_bcn_probe_rx_sync(wlan_pdev_get_psoc(pdev), buf,
					    &rx_param, frm_type);
}

bool cm_is_vdev_connecting(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_CONNECTING)
		return true;

	return false;
}

bool cm_is_vdev_connected(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_CONNECTED)
		return true;

	return false;
}

bool cm_is_vdev_active(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_CONNECTED || state == WLAN_CM_S_ROAMING)
		return true;

	return false;
}

bool cm_is_vdev_disconnecting(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_DISCONNECTING)
		return true;

	return false;
}

bool cm_is_vdev_disconnected(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_INIT)
		return true;

	return false;
}

bool cm_is_vdev_roaming(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);

	if (state == WLAN_CM_S_ROAMING)
		return true;

	return false;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
bool cm_is_vdev_roam_started(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;
	enum wlan_cm_sm_state sub_state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);
	sub_state = cm_get_sub_state(cm_ctx);
	if (state == WLAN_CM_S_ROAMING && sub_state == WLAN_CM_SS_ROAM_STARTED)
		return true;

	return false;
}

bool cm_is_vdev_roam_sync_inprogress(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;
	enum wlan_cm_sm_state sub_state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);
	sub_state = cm_get_sub_state(cm_ctx);
	if (state == WLAN_CM_S_ROAMING && sub_state == WLAN_CM_SS_ROAM_SYNC)
		return true;

	return false;
}
#endif

#ifdef WLAN_FEATURE_HOST_ROAM
bool cm_is_vdev_roam_preauth_state(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;
	enum wlan_cm_sm_state sub_state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);
	sub_state = cm_get_sub_state(cm_ctx);
	if (state == WLAN_CM_S_ROAMING && sub_state == WLAN_CM_SS_PREAUTH)
		return true;

	return false;
}

bool cm_is_vdev_roam_reassoc_state(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	enum wlan_cm_sm_state state;
	enum wlan_cm_sm_state sub_state;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return false;

	state = cm_get_state(cm_ctx);
	sub_state = cm_get_sub_state(cm_ctx);
	if (state == WLAN_CM_S_ROAMING && sub_state == WLAN_CM_SS_REASSOC)
		return true;

	return false;
}
#endif

enum wlan_cm_active_request_type
cm_get_active_req_type(struct wlan_objmgr_vdev *vdev)
{
	struct cnx_mgr *cm_ctx;
	wlan_cm_id cm_id;
	uint32_t active_req_prefix = 0;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return CM_NONE;

	cm_id = cm_ctx->active_cm_id;

	if (cm_id != CM_ID_INVALID)
		active_req_prefix = CM_ID_GET_PREFIX(cm_id);

	if (active_req_prefix == CONNECT_REQ_PREFIX)
		return CM_CONNECT_ACTIVE;
	else if (active_req_prefix == DISCONNECT_REQ_PREFIX)
		return CM_DISCONNECT_ACTIVE;
	else if (active_req_prefix == ROAM_REQ_PREFIX)
		return CM_ROAM_ACTIVE;
	else
		return CM_NONE;
}

#ifdef WLAN_FEATURE_11BE_MLO
static inline
void cm_fill_ml_partner_info(struct wlan_cm_connect_req *req,
			       struct wlan_cm_vdev_connect_req *connect_req)
{
	if (req->ml_parnter_info.num_partner_links)
		qdf_mem_copy(&connect_req->ml_parnter_info,
			     &req->ml_parnter_info,
			     sizeof(struct mlo_partner_info));
}
#else
static inline
void cm_fill_ml_partner_info(struct wlan_cm_connect_req *req,
			     struct wlan_cm_vdev_connect_req *connect_req)
{
}
#endif

bool cm_get_active_connect_req(struct wlan_objmgr_vdev *vdev,
			       struct wlan_cm_vdev_connect_req *req)
{
	struct cnx_mgr *cm_ctx;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;
	bool status = false;
	uint32_t cm_id_prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return status;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		cm_id_prefix = CM_ID_GET_PREFIX((cm_req->cm_id));

		if (cm_req->cm_id == cm_ctx->active_cm_id &&
		    cm_id_prefix == CONNECT_REQ_PREFIX) {
			req->vdev_id = wlan_vdev_get_id(vdev);
			req->cm_id = cm_req->connect_req.cm_id;
			req->bss =  cm_req->connect_req.cur_candidate;
			req->is_wps_connection =
				cm_req->connect_req.req.is_wps_connection;
			req->is_osen_connection =
				cm_req->connect_req.req.is_osen_connection;
			req->is_non_assoc_link = cm_req->connect_req.req.is_non_assoc_link;
			cm_fill_ml_partner_info(&cm_req->connect_req.req, req);
			status = true;
			cm_req_lock_release(cm_ctx);
			return status;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return status;
}

bool cm_get_active_disconnect_req(struct wlan_objmgr_vdev *vdev,
				  struct wlan_cm_vdev_discon_req *req)
{
	struct cnx_mgr *cm_ctx;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req = NULL;
	bool status = false;
	uint32_t cm_id_prefix;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return status;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);

		cm_req = qdf_container_of(cur_node, struct cm_req, node);
		cm_id_prefix = CM_ID_GET_PREFIX((cm_req->cm_id));

		if (cm_req->cm_id == cm_ctx->active_cm_id &&
		    cm_id_prefix == DISCONNECT_REQ_PREFIX) {
			req->cm_id = cm_req->cm_id;
			req->req.vdev_id = wlan_vdev_get_id(vdev);
			req->req.source = cm_req->discon_req.req.source;
			req->req.reason_code =
					cm_req->discon_req.req.reason_code;
			req->req.bssid = cm_req->discon_req.req.bssid;
			req->req.is_no_disassoc_disconnect =
				cm_req->discon_req.req.is_no_disassoc_disconnect;
			status = true;
			cm_req_lock_release(cm_ctx);
			return status;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return status;
}

struct cm_req *cm_get_req_by_scan_id(struct cnx_mgr *cm_ctx,
				     wlan_scan_id scan_id)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->connect_req.scan_id == scan_id) {
			cm_req_lock_release(cm_ctx);
			return cm_req;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return NULL;
}

wlan_cm_id cm_get_cm_id_by_scan_id(struct cnx_mgr *cm_ctx,
				   wlan_scan_id scan_id)
{
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct cm_req *cm_req;

	cm_req_lock_acquire(cm_ctx);
	qdf_list_peek_front(&cm_ctx->req_list, &cur_node);
	while (cur_node) {
		qdf_list_peek_next(&cm_ctx->req_list, cur_node, &next_node);
		cm_req = qdf_container_of(cur_node, struct cm_req, node);

		if (cm_req->connect_req.scan_id == scan_id) {
			cm_req_lock_release(cm_ctx);
			return cm_req->cm_id;
		}

		cur_node = next_node;
		next_node = NULL;
	}
	cm_req_lock_release(cm_ctx);

	return CM_ID_INVALID;
}

#ifdef WLAN_POLICY_MGR_ENABLE
static void
cm_get_pcl_chan_weigtage_for_sta(struct wlan_objmgr_pdev *pdev,
				 struct pcl_freq_weight_list *pcl_lst)
{
	enum QDF_OPMODE opmode = QDF_STA_MODE;
	enum policy_mgr_con_mode pm_mode;
	uint32_t num_entries = 0;
	QDF_STATUS status;

	if (!pcl_lst)
		return;

	if (policy_mgr_map_concurrency_mode(&opmode, &pm_mode)) {
		status = policy_mgr_get_pcl(wlan_pdev_get_psoc(pdev), pm_mode,
					    pcl_lst->pcl_freq_list,
					    &num_entries,
					    pcl_lst->pcl_weight_list,
					    NUM_CHANNELS);
		if (QDF_IS_STATUS_ERROR(status))
			return;
		pcl_lst->num_of_pcl_channels = num_entries;
	}
}

void cm_calculate_scores(struct cnx_mgr *cm_ctx,
			 struct wlan_objmgr_pdev *pdev,
			 struct scan_filter *filter, qdf_list_t *list)
{
	struct pcl_freq_weight_list *pcl_lst = NULL;

	if (!filter->num_of_bssid) {
		pcl_lst = qdf_mem_malloc(sizeof(*pcl_lst));
		cm_get_pcl_chan_weigtage_for_sta(pdev, pcl_lst);
		if (pcl_lst && !pcl_lst->num_of_pcl_channels) {
			qdf_mem_free(pcl_lst);
			pcl_lst = NULL;
		}
	}
	wlan_cm_calculate_bss_score(pdev, pcl_lst, list, &filter->bssid_hint);
	if (pcl_lst)
		qdf_mem_free(pcl_lst);
}
#else
inline
void cm_calculate_scores(struct cnx_mgr *cm_ctx,
			 struct wlan_objmgr_pdev *pdev,
			 struct scan_filter *filter, qdf_list_t *list)
{
	wlan_cm_calculate_bss_score(pdev, NULL, list, &filter->bssid_hint);

	/*
	 * Custom sorting if enabled
	 */
	if (cm_ctx && cm_ctx->cm_candidate_list_custom_sort)
		cm_ctx->cm_candidate_list_custom_sort(cm_ctx->vdev, list);
}
#endif

#ifdef SM_ENG_HIST_ENABLE
static const char *cm_id_to_string(wlan_cm_id cm_id)
{
	uint32_t prefix = CM_ID_GET_PREFIX(cm_id);

	switch (prefix) {
	case CONNECT_REQ_PREFIX:
		return "CONNECT";
	case DISCONNECT_REQ_PREFIX:
		return "DISCONNECT";
	case ROAM_REQ_PREFIX:
		return "ROAM";
	default:
		return "INVALID";
	}
}

void cm_req_history_add(struct cnx_mgr *cm_ctx,
			struct cm_req *cm_req)
{
	struct cm_req_history *history = &cm_ctx->req_history;
	struct cm_req_history_info *data;

	qdf_spin_lock_bh(&history->cm_req_hist_lock);
	data = &history->data[history->index];
	history->index++;
	history->index %= CM_REQ_HISTORY_SIZE;

	qdf_mem_zero(data, sizeof(*data));
	data->cm_id = cm_req->cm_id;
	data->add_time = qdf_get_log_timestamp();
	data->add_cm_state = cm_get_state(cm_ctx);
	qdf_spin_unlock_bh(&history->cm_req_hist_lock);
}

void cm_req_history_del(struct cnx_mgr *cm_ctx,
			struct cm_req *cm_req,
			enum cm_req_del_type del_type)
{
	uint8_t i, idx;
	struct cm_req_history_info *data;
	struct cm_req_history *history = &cm_ctx->req_history;

	qdf_spin_lock_bh(&history->cm_req_hist_lock);
	for (i = 0; i < CM_REQ_HISTORY_SIZE; i++) {
		if (history->index < i)
			idx = CM_REQ_HISTORY_SIZE + history->index - i;
		else
			idx = history->index - i;

		data = &history->data[idx];
		if (data->cm_id == cm_req->cm_id) {
			data->del_time = qdf_get_log_timestamp();
			data->del_cm_state = cm_get_state(cm_ctx);
			data->del_type = del_type;
			break;
		}

		if (!data->cm_id)
			break;
	}
	qdf_spin_unlock_bh(&history->cm_req_hist_lock);
}

void cm_req_history_init(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_create(&cm_ctx->req_history.cm_req_hist_lock);
	qdf_mem_zero(&cm_ctx->req_history, sizeof(struct cm_req_history));
}

void cm_req_history_deinit(struct cnx_mgr *cm_ctx)
{
	qdf_spinlock_destroy(&cm_ctx->req_history.cm_req_hist_lock);
}

static inline void cm_req_history_print_entry(uint16_t idx,
					      struct cm_req_history_info *data)
{
	if (!data->cm_id)
		return;

	mlme_nofl_err("    |%6u | 0x%016llx | 0x%016llx |%12s | 0x%08x |%15s |%15s |%8u",
		      idx, data->add_time, data->del_time,
		      cm_id_to_string(data->cm_id), data->cm_id,
		      cm_sm_info[data->add_cm_state].name,
		      cm_sm_info[data->del_cm_state].name,
		      data->del_type);
}

void cm_req_history_print(struct cnx_mgr *cm_ctx)
{
	struct cm_req_history *history = &cm_ctx->req_history;
	uint8_t i, idx;

	mlme_nofl_err("CM Request history is as below");
	mlme_nofl_err("|%6s |%19s |%19s |%12s |%11s |%15s |%15s |%8s",
		      "Index", "Add Time", "Del Time", "Req type",
		      "Cm Id", "Add State", "Del State", "Del Type");

	qdf_spin_lock_bh(&history->cm_req_hist_lock);
	for (i = 0; i < CM_REQ_HISTORY_SIZE; i++) {
		idx = (history->index + i) % CM_REQ_HISTORY_SIZE;
		cm_req_history_print_entry(idx, &history->data[idx]);
	}
	qdf_spin_unlock_bh(&history->cm_req_hist_lock);
}
#endif

#ifndef CONN_MGR_ADV_FEATURE
void cm_set_candidate_advance_filter_cb(
		struct wlan_objmgr_vdev *vdev,
		void (*filter_fun)(struct wlan_objmgr_vdev *vdev,
				   struct scan_filter *filter))
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	cm_ctx->cm_candidate_advance_filter = filter_fun;
}

void cm_set_candidate_custom_sort_cb(
		struct wlan_objmgr_vdev *vdev,
		void (*sort_fun)(struct wlan_objmgr_vdev *vdev,
				 qdf_list_t *list))
{
	struct cnx_mgr *cm_ctx;

	cm_ctx = cm_get_cm_ctx(vdev);
	if (!cm_ctx)
		return;

	cm_ctx->cm_candidate_list_custom_sort = sort_fun;
}
#endif

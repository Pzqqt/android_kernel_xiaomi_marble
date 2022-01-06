/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wlan_mlo_mgr_main.h"
#include "qdf_types.h"
#include "wlan_cmn.h"
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_msgq.h"
#include "wlan_mlo_mgr_peer.h"

#ifndef MLO_MSGQ_SUPPORT
QDF_STATUS mlo_msgq_post(enum mlo_msg_type type,
			 struct wlan_mlo_dev_context *ml_dev,
			 void *payload)
{
	struct peer_create_notif_s *peer_create;
	struct peer_assoc_notify_s *peer_assoc;
	struct peer_assoc_fail_notify_s *peer_assoc_fail;
	struct peer_discon_notify_s *peer_disconn;
	struct peer_deauth_notify_s *peer_deauth;

	switch (type) {
	case MLO_PEER_CREATE:
		peer_create = (struct peer_create_notif_s *)payload;

		mlo_mlme_peer_create(peer_create->vdev_link,
				     peer_create->ml_peer,
				     &peer_create->addr, peer_create->frm_buf);
		qdf_nbuf_free(peer_create->frm_buf);
		wlan_mlo_peer_release_ref(peer_create->ml_peer);
		wlan_objmgr_vdev_release_ref(peer_create->vdev_link,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_ASSOC:
		peer_assoc = (struct peer_assoc_notify_s *)payload;
		mlo_mlme_peer_assoc(peer_assoc->peer);
		wlan_objmgr_peer_release_ref(peer_assoc->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_ASSOC_FAIL:
		peer_assoc_fail = (struct peer_assoc_fail_notify_s *)payload;
		mlo_mlme_peer_assoc_fail(peer_assoc_fail->peer);
		wlan_objmgr_peer_release_ref(peer_assoc_fail->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_DISCONNECT:
		peer_disconn = (struct peer_discon_notify_s *)payload;
		mlo_mlme_peer_delete(peer_disconn->peer);
		wlan_objmgr_peer_release_ref(peer_disconn->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_DEAUTH:
		peer_deauth = (struct peer_deauth_notify_s *)payload;
		mlo_mlme_peer_deauth(peer_deauth->peer);
		wlan_objmgr_peer_release_ref(peer_deauth->peer,
					     WLAN_MLO_MGR_ID);
		break;

	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}

void mlo_msgq_init(void)
{
}

void mlo_msgq_free(void)
{
}
#else
static void mlo_msgq_timer_start(void)
{
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	bool start_timer = true;

	if (!mlo_ctx)
		return;

	msgq_ctx = mlo_ctx->msgq_ctx;

	qdf_spin_lock_bh(&msgq_ctx->ctxt_lock);
	if (!msgq_ctx->timer_started)
		msgq_ctx->timer_started = true;
	else
		start_timer = false;
	qdf_spin_unlock_bh(&msgq_ctx->ctxt_lock);

	if (start_timer)
		qdf_timer_start(&msgq_ctx->ctxt_mgr_timer, 0);
}

static void mlo_msgq_timer_stop(void)
{
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx)
		return;

	msgq_ctx = mlo_ctx->msgq_ctx;

	qdf_timer_stop(&msgq_ctx->ctxt_mgr_timer);

	qdf_spin_lock_bh(&msgq_ctx->ctxt_lock);
	msgq_ctx->timer_started = false;
	qdf_spin_unlock_bh(&msgq_ctx->ctxt_lock);
}

QDF_STATUS mlo_msgq_post(enum mlo_msg_type type,
			 struct wlan_mlo_dev_context *ml_dev,
			 void *payload)
{
	struct mlo_ctxt_switch_msg_s *msg;
	struct peer_create_notif_s *peer_create, *peer_create_l;
	struct peer_assoc_notify_s *peer_assoc, *peer_assoc_l;
	struct peer_assoc_fail_notify_s *peer_assoc_fail, *peer_assoc_fail_l;
	struct peer_discon_notify_s *peer_disconn, *peer_disconn_l;
	struct peer_deauth_notify_s *peer_deauth, *peer_deauth_l;
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx)
		return QDF_STATUS_E_FAILURE;

	msgq_ctx = mlo_ctx->msgq_ctx;

	if (!msgq_ctx->allow_msg)
		return QDF_STATUS_E_FAILURE;

	msg = qdf_mem_malloc(sizeof(*msg));
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	msg->type = type;
	msg->ml_dev = ml_dev;

	switch (type) {
	case MLO_PEER_CREATE:
		peer_create = &msg->m.peer_create;
		peer_create_l = (struct peer_create_notif_s *)payload;
		peer_create->frm_buf = peer_create_l->frm_buf;
		peer_create->ml_peer = peer_create_l->ml_peer;
		peer_create->vdev_link = peer_create_l->vdev_link;
		qdf_copy_macaddr(&peer_create->addr, &peer_create_l->addr);
		break;

	case MLO_PEER_ASSOC:
		peer_assoc = &msg->m.peer_assoc;
		peer_assoc_l = (struct peer_assoc_notify_s *)payload;
		peer_assoc->peer = peer_assoc_l->peer;
		break;

	case MLO_PEER_ASSOC_FAIL:
		peer_assoc_fail = &msg->m.peer_assoc_fail;
		peer_assoc_fail_l = (struct peer_assoc_fail_notify_s *)payload;
		peer_assoc_fail->peer = peer_assoc_fail_l->peer;
		break;

	case MLO_PEER_DISCONNECT:
		peer_disconn = &msg->m.peer_disconn;
		peer_disconn_l = (struct peer_discon_notify_s *)payload;
		peer_disconn->peer = peer_disconn_l->peer;
		break;

	case MLO_PEER_DEAUTH:
		peer_deauth = &msg->m.peer_deauth;
		peer_deauth_l = (struct peer_deauth_notify_s *)payload;
		peer_deauth->peer = peer_deauth_l->peer;
		break;

	default:
		break;
	}

	qdf_spin_lock_bh(&msgq_ctx->ctxt_lock);
	qdf_list_insert_back(&msgq_ctx->msgq_list, &msg->node);
	qdf_spin_unlock_bh(&msgq_ctx->ctxt_lock);
	mlo_msgq_timer_start();

	return QDF_STATUS_SUCCESS;
}

static void mlo_msgq_msg_process_hdlr(struct mlo_ctxt_switch_msg_s *msg)
{
	enum mlo_msg_type type;
	struct peer_create_notif_s *peer_create;
	struct peer_assoc_notify_s *peer_assoc;
	struct peer_assoc_fail_notify_s *peer_assoc_fail;
	struct peer_discon_notify_s *peer_disconn;
	struct peer_deauth_notify_s *peer_deauth;

	type = msg->type;
	switch (type) {
	case MLO_PEER_CREATE:
		peer_create = &msg->m.peer_create;
		mlo_mlme_peer_create(peer_create->vdev_link,
				     peer_create->ml_peer,
				     &peer_create->addr, peer_create->frm_buf);
		qdf_nbuf_free(peer_create->frm_buf);
		wlan_mlo_peer_release_ref(peer_create->ml_peer);
		wlan_objmgr_vdev_release_ref(peer_create->vdev_link,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_ASSOC:
		peer_assoc = &msg->m.peer_assoc;
		mlo_mlme_peer_assoc(peer_assoc->peer);
		wlan_objmgr_peer_release_ref(peer_assoc->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_ASSOC_FAIL:
		peer_assoc_fail = &msg->m.peer_assoc_fail;
		mlo_mlme_peer_assoc_fail(peer_assoc_fail->peer);
		wlan_objmgr_peer_release_ref(peer_assoc_fail->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_DISCONNECT:
		peer_disconn = &msg->m.peer_disconn;
		mlo_mlme_peer_delete(peer_disconn->peer);
		wlan_objmgr_peer_release_ref(peer_disconn->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_DEAUTH:
		peer_deauth = &msg->m.peer_deauth;
		mlo_mlme_peer_deauth(peer_deauth->peer);
		wlan_objmgr_peer_release_ref(peer_deauth->peer,
					     WLAN_MLO_MGR_ID);
		break;

	default:
		break;
	}
	qdf_mem_free(msg);
}

static void mlo_msgq_msg_flush_hdlr(struct mlo_ctxt_switch_msg_s *msg)
{
	enum mlo_msg_type type;
	struct peer_create_notif_s *peer_create;
	struct peer_assoc_notify_s *peer_assoc;
	struct peer_assoc_fail_notify_s *peer_assoc_fail;
	struct peer_discon_notify_s *peer_disconn;
	struct peer_deauth_notify_s *peer_deauth;

	type = msg->type;
	switch (type) {
	case MLO_PEER_CREATE:
		peer_create = &msg->m.peer_create;
		qdf_nbuf_free(peer_create->frm_buf);
		wlan_mlo_peer_release_ref(peer_create->ml_peer);
		wlan_objmgr_vdev_release_ref(peer_create->vdev_link,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_ASSOC:
		peer_assoc = &msg->m.peer_assoc;
		wlan_objmgr_peer_release_ref(peer_assoc->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_ASSOC_FAIL:
		peer_assoc_fail = &msg->m.peer_assoc_fail;
		wlan_objmgr_peer_release_ref(peer_assoc_fail->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_DISCONNECT:
		peer_disconn = &msg->m.peer_disconn;
		wlan_objmgr_peer_release_ref(peer_disconn->peer,
					     WLAN_MLO_MGR_ID);
		break;

	case MLO_PEER_DEAUTH:
		peer_deauth = &msg->m.peer_deauth;
		wlan_objmgr_peer_release_ref(peer_deauth->peer,
					     WLAN_MLO_MGR_ID);
		break;

	default:
		break;
	}
	qdf_mem_free(msg);
}

static void mlo_msgq_msg_flush(void)
{
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	qdf_list_node_t *msgbuf_node = NULL;
	struct mlo_ctxt_switch_msg_s *msg;
	QDF_STATUS status;

	if (!mlo_ctx)
		return;

	msgq_ctx = mlo_ctx->msgq_ctx;
	do {
		msg = NULL;
		qdf_spin_lock_bh(&msgq_ctx->ctxt_lock);
		status = qdf_list_peek_front(&msgq_ctx->msgq_list,
					     &msgbuf_node);
		if (status != QDF_STATUS_E_EMPTY) {
			qdf_list_remove_node(&msgq_ctx->msgq_list,
					     msgbuf_node);
			msg = qdf_container_of(msgbuf_node,
					       struct mlo_ctxt_switch_msg_s,
					       node);
		}
		qdf_spin_unlock_bh(&msgq_ctx->ctxt_lock);

		if (!msg)
			break;

		mlo_msgq_msg_flush_hdlr(msg);

	} while (true);
}

static void mlo_msgq_msg_handler(void *arg)
{
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	qdf_list_node_t *msgbuf_node = NULL;
	struct mlo_ctxt_switch_msg_s *msg;
	QDF_STATUS status;

	if (!mlo_ctx)
		return;

	msgq_ctx = mlo_ctx->msgq_ctx;
	do {
		msg = NULL;
		qdf_spin_lock_bh(&msgq_ctx->ctxt_lock);
		status = qdf_list_peek_front(&msgq_ctx->msgq_list,
					     &msgbuf_node);
		if (status != QDF_STATUS_E_EMPTY) {
			qdf_list_remove_node(&msgq_ctx->msgq_list,
					     msgbuf_node);
			msg = qdf_container_of(msgbuf_node,
					       struct mlo_ctxt_switch_msg_s,
					       node);
		} else {
			msgq_ctx->timer_started = false;
		}
		qdf_spin_unlock_bh(&msgq_ctx->ctxt_lock);

		if (!msg)
			break;

		mlo_msgq_msg_process_hdlr(msg);

	} while (true);
}

void mlo_msgq_init(void)
{
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	msgq_ctx = qdf_mem_malloc(sizeof(*msgq_ctx));
	if (!msgq_ctx) {
		mlo_err(" message queue context allocation failed");
		return;
	}

	qdf_spinlock_create(&msgq_ctx->ctxt_lock);
	/* Initialize timer with timeout handler */
	qdf_timer_init(NULL, &msgq_ctx->ctxt_mgr_timer,
		       mlo_msgq_msg_handler,
		       NULL, QDF_TIMER_TYPE_WAKE_APPS);

	msgq_ctx->timer_started = false;
	msgq_ctx->allow_msg = true;
	qdf_list_create(&msgq_ctx->msgq_list, MLO_MAX_MSGQ_SIZE);

	mlo_ctx->msgq_ctx = msgq_ctx;
}

void mlo_msgq_free(void)
{
	struct ctxt_switch_mgr *msgq_ctx;
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx)
		return;

	msgq_ctx = mlo_ctx->msgq_ctx;

	msgq_ctx->timer_started = false;
	msgq_ctx->allow_msg = false;
	mlo_msgq_msg_flush();
	qdf_list_destroy(&msgq_ctx->msgq_list);
	qdf_timer_free(&msgq_ctx->ctxt_mgr_timer);
	qdf_spinlock_destroy(&msgq_ctx->ctxt_lock);
	qdf_mem_free(msgq_ctx);
}
#endif

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

	default:
		break;
	}

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS mlo_msgq_post(enum mlo_msg_type type,
			 struct wlan_mlo_dev_context *ml_dev,
			 void *payload)
{
	struct mlo_ctxt_switch_msg_s *msg;
	struct peer_create_notif_s *peer_create, *peer_create_l;
	struct peer_assoc_notify_s *peer_assoc, *peer_assoc_l;
	struct peer_assoc_fail_notify_s *peer_assoc_fail, *peer_assoc_fail_l;
	struct peer_discon_notify_s *peer_disconn, *peer_disconn_l;

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

	default:
		break;
	}
	/* TODO queue message buffer to qdf_list */

	return QDF_STATUS_SUCCESS;
}

void mlo_msgq_msg_process_hdlr(struct mlo_ctxt_switch_msg_s *msg)
{
	enum mlo_msg_type type;
	struct peer_create_notif_s *peer_create;
	struct peer_assoc_notify_s *peer_assoc;
	struct peer_assoc_fail_notify_s *peer_assoc_fail;
	struct peer_discon_notify_s *peer_disconn;

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

	default:
		break;
	}
	qdf_mem_free(msg);
}

void mlo_msgq_msg_flush_hdlr(struct mlo_ctxt_switch_msg_s *msg)
{
	enum mlo_msg_type type;
	struct peer_create_notif_s *peer_create;
	struct peer_assoc_notify_s *peer_assoc;
	struct peer_assoc_fail_notify_s *peer_assoc_fail;
	struct peer_discon_notify_s *peer_disconn;

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

	default:
		break;
	}
	qdf_mem_free(msg);
}
#endif

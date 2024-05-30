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

/*
 * DOC: contains mlo manager msgq structure definitions
 */

#ifndef __MLO_MGR_MSGQ_H
#define __MLO_MGR_MSGQ_H

/*
 * struct ctxt_switch_mgr - MLO context switch manager
 * @ctxt_mgr_timer: Timer to process messages
 * @msgq_list: list to hold pending messages
 * @ctxt_lock: Lock to have atomic context
 * @timer_started: indicates whether timer is running
 * @max_messages_procd: Max messages can be stored
 */
struct ctxt_switch_mgr {
	qdf_timer_t ctxt_mgr_timer;
	qdf_list_t msgq_list;
	qdf_spinlock_t ctxt_lock;
	bool timer_started;
	bool allow_msg;
	uint16_t max_messages_procd;
};

/*
 * enum mlo_msg_type – MLO partner peer message type
 * @MLO_PEER_CREATE:   Partner peer create
 * @MLO_PEER_ASSOC:    Partner peer ASSOC
 * @MLO_PEER_ASSOC_FAIL:  Partner peer ASSOC failure
 * @MLO_PEER_DISCONNECT:  Partner peer Disconnect
 * @MLO_PEER_DEAUTH:  Initiate Deauth for ML connection
 */
enum mlo_msg_type {
	MLO_PEER_CREATE,
	MLO_PEER_ASSOC,
	MLO_PEER_ASSOC_FAIL,
	MLO_PEER_DISCONNECT,
	MLO_PEER_DEAUTH,
};

/*
 * struct peer_create_notif_s - MLO partner peer create notification
 * @vdev_link: Link VDEV
 * @ml_peer: ML peer to attached
 * @addr: Link MAC address
 * @frm_buf: Assoc request buffer
 */
struct peer_create_notif_s {
	struct wlan_objmgr_vdev *vdev_link;
	struct wlan_mlo_peer_context *ml_peer;
	struct qdf_mac_addr addr;
	qdf_nbuf_t frm_buf;
};

/*
 * struct peer_assoc_notify_s - MLO partner peer assoc notification
 * @peer: Link peer on which Peer ASSOC to be sent
 */
struct peer_assoc_notify_s {
	struct wlan_objmgr_peer *peer;
};

/*
 * struct peer_assoc_fail_notify_s - MLO partner peer assoc fail notification
 * @peer: Link peer on which Peer assoc resp failure to be sent
 */
struct peer_assoc_fail_notify_s {
	struct wlan_objmgr_peer *peer;
};

/*
 * struct peer_discon_notify_s - MLO partner peer disconnect notification
 * @peer: Link peer on which Peer disconnect to be sent
 */
struct peer_discon_notify_s {
	struct wlan_objmgr_peer *peer;
};

/*
 * struct peer_deauth_notify_s - MLO partner peer deauth notification
 * @peer: Link peer on which Peer deauth to be sent
 */
struct peer_deauth_notify_s {
	struct wlan_objmgr_peer *peer;
};

/*
 * union msg_payload - MLO message payload
 * @peer_create: peer create notification structure
 * @peer_assoc: peer assoc notification structure
 * @peer_assoc_fail: peer assoc fail notification structure
 * @peer_disconn: peer disconnect notification structure
 * @peer_deauth: peer deauth notification structure
 */
union msg_payload {
	struct peer_create_notif_s peer_create;
	struct peer_assoc_notify_s peer_assoc;
	struct peer_assoc_fail_notify_s peer_assoc_fail;
	struct peer_discon_notify_s peer_disconn;
	struct peer_deauth_notify_s peer_deauth;
};

#define MLO_MAX_MSGQ_SIZE 256
/*
 * struct mlo_ctxt_switch_msg_s - MLO ctxt switch message
 * @type: peer create notification structure
 * @peer_assoc: peer assoc notification structure
 * @peer_assoc_fail: peer assoc fail notification structure
 * @peer_disconn: peer disconnect notification structure
 */
struct mlo_ctxt_switch_msg_s {
	qdf_list_node_t node;
	enum mlo_msg_type type;
	struct wlan_mlo_dev_context *ml_dev;
	union msg_payload m;
};

/**
 * mlo_msgq_post() - Posts message to defer context
 * @type: msg tupe
 * @ml_dev: MLO dev context
 * @payload: msg buf
 *
 * This function post message to defer conext queue for defer processing
 *
 * Return: SUCCESS if msg is posted
 */
QDF_STATUS mlo_msgq_post(enum mlo_msg_type type,
			 struct wlan_mlo_dev_context *ml_dev,
			 void *payload);

/**
 * mlo_msgq_init() - Init MLO message queue
 *
 * This function initializes MLO msg queue module
 *
 * Return: void
 */
void mlo_msgq_init(void);

/**
 * mlo_msgq_free() - Free MLO message queue
 *
 * This function frees MLO msg queue module
 *
 * Return: void
 */
void mlo_msgq_free(void);
#endif

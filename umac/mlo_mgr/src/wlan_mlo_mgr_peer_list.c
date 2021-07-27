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

#include <wlan_mlo_mgr_public_structs.h>
#include "wlan_mlo_mgr_main.h"
#include "qdf_types.h"
#include "wlan_cmn.h"
#include "wlan_mlo_mgr_peer.h"

struct aid_search {
	struct wlan_mlo_peer_context *ml_peer;
	uint16_t aid;
};

struct mlpeerid_search {
	struct wlan_mlo_peer_context *ml_peer;
	uint16_t ml_peerid;
};

struct link_mac_search {
	struct wlan_mlo_peer_context *ml_peer;
	struct qdf_mac_addr mac_addr;
};

static inline struct wlan_mlo_peer_context *wlan_mlo_peer_list_peek_head(
					qdf_list_t *peer_list)
{
	struct wlan_mlo_peer_context *ml_peer;
	qdf_list_node_t *peer_node = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	if (qdf_list_peek_front(peer_list, &peer_node) != QDF_STATUS_SUCCESS)
		return NULL;

	ml_peer = qdf_container_of(peer_node,
				   struct wlan_mlo_peer_context, peer_node);
	return ml_peer;
}

static inline struct wlan_mlo_peer_context *wlan_mlo_peer_get_next_mlpeer(
					qdf_list_t *peer_list,
					struct wlan_mlo_peer_context *ml_peer)
{
	struct wlan_mlo_peer_context *next_peer;
	qdf_list_node_t *node = &ml_peer->peer_node;
	qdf_list_node_t *next_node = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	if (!node)
		return NULL;

	if (qdf_list_peek_next(peer_list, node, &next_node) !=
				QDF_STATUS_SUCCESS)
		return NULL;

	next_peer = qdf_container_of(next_node,
				     struct wlan_mlo_peer_context, peer_node);

	return next_peer;
}

struct wlan_mlo_peer_context *mlo_get_mlpeer(
				struct wlan_mlo_dev_context *ml_dev,
				struct qdf_mac_addr *ml_addr)
{
	uint8_t hash_index;
	struct wlan_mlo_peer_list *mlo_peer_list;
	struct wlan_mlo_peer_context *ml_peer;
	struct wlan_mlo_peer_context *next_ml_peer;
	qdf_list_t *peer_hash_list;

	mlo_peer_list = &ml_dev->mlo_peer_list;
	hash_index = WLAN_PEER_HASH(ml_addr->bytes);

	peer_hash_list = &mlo_peer_list->peer_hash[hash_index];
	/* Get first vdev */
	ml_peer = wlan_mlo_peer_list_peek_head(peer_hash_list);
	/**
	 * Iterate through pdev's vdev list, till vdev id matches with
	 * entry of vdev list
	 */
	while (ml_peer) {
		if (qdf_is_macaddr_equal(&ml_peer->peer_mld_addr, ml_addr))
			return ml_peer;

		/* get next vdev */
		next_ml_peer = wlan_mlo_peer_get_next_mlpeer(peer_hash_list,
							     ml_peer);
		ml_peer = next_ml_peer;
	}

	return NULL;
}

QDF_STATUS wlan_mlo_iterate_ml_peerlist(struct wlan_mlo_dev_context *ml_dev,
					wlan_mlo_op_handler handler,
					void *arg)
{
	uint8_t hash_index;
	struct wlan_mlo_peer_list *peerlist;
	struct wlan_mlo_peer_context *ml_peer;
	struct wlan_mlo_peer_context *next;
	qdf_list_t *peer_hash_list;
	QDF_STATUS status;

	peerlist = &ml_dev->mlo_peer_list;
	ml_peerlist_lock_acquire(peerlist);

	for (hash_index = 0; hash_index < WLAN_PEER_HASHSIZE; hash_index++) {
		peer_hash_list = &peerlist->peer_hash[hash_index];
		/* Get first vdev */
		ml_peer = wlan_mlo_peer_list_peek_head(peer_hash_list);
		/**
		 * Iterate through pdev's vdev list, till vdev id matches with
		 * entry of vdev list
		 */
		while (ml_peer) {
			status = handler(ml_dev, ml_peer, arg);
			if (status == QDF_STATUS_SUCCESS) {
				ml_peerlist_lock_release(peerlist);
				return QDF_STATUS_SUCCESS;
			}
			/* get next ml peer */
			next = wlan_mlo_peer_get_next_mlpeer(peer_hash_list,
							     ml_peer);
			ml_peer = next;
		}
	}
	ml_peerlist_lock_release(peerlist);

	return QDF_STATUS_E_NOENT;
}

static QDF_STATUS
wlan_find_mlpeer_link_mac_addr(struct wlan_mlo_dev_context *ml_dev,
			       void *iter_ml_peer,
			       void *arg)
{
	struct link_mac_search *link_mac_arg = (struct link_mac_search *)arg;
	struct wlan_mlo_link_peer_entry *link_peer;
	struct wlan_mlo_peer_context *ml_peer;
	uint8_t i;

	ml_peer = (struct wlan_mlo_peer_context *)iter_ml_peer;
	for (i = 0; i < MAX_MLO_PEER; i++) {
		link_peer = &ml_peer->peer_list[i];
		if (!link_peer)
			continue;

		if (qdf_is_macaddr_equal(&link_mac_arg->mac_addr,
					 &link_peer->link_addr)) {
			link_mac_arg->ml_peer = ml_peer;
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_E_NOENT;
}

static QDF_STATUS wlan_find_mlpeer_aid(struct wlan_mlo_dev_context *ml_dev,
				       void *iter_ml_peer,
				       void *arg)
{
	struct aid_search *aid_arg = (struct aid_search *)arg;
	struct wlan_mlo_peer_context *ml_peer;

	ml_peer = (struct wlan_mlo_peer_context *)iter_ml_peer;

	if (aid_arg->aid == ml_peer->assoc_id) {
		aid_arg->ml_peer = ml_peer;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_NOENT;
}

static QDF_STATUS
wlan_find_mlpeer_ml_peerid(struct wlan_mlo_dev_context *ml_dev,
			   void *iter_ml_peer,
			   void *arg)
{
	struct mlpeerid_search *mlpeer_id_arg = (struct mlpeerid_search *)arg;
	struct wlan_mlo_peer_context *ml_peer;

	ml_peer = (struct wlan_mlo_peer_context *)iter_ml_peer;

	if (mlpeer_id_arg->ml_peerid == ml_peer->mlo_peer_id) {
		mlpeer_id_arg->ml_peer = ml_peer;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_NOENT;
}

struct wlan_mlo_peer_context *wlan_mlo_get_mlpeer_by_linkmac(
				struct wlan_mlo_dev_context *ml_dev,
				struct qdf_mac_addr *link_mac)
{
	struct link_mac_search link_mac_arg;
	QDF_STATUS status;

	qdf_copy_macaddr(&link_mac_arg.mac_addr, link_mac);
	status = wlan_mlo_iterate_ml_peerlist(ml_dev,
					      wlan_find_mlpeer_link_mac_addr,
					      &link_mac_arg);
	if (status == QDF_STATUS_SUCCESS)
		return link_mac_arg.ml_peer;

	/* TODO: Take ref */

	return NULL;
}

struct wlan_mlo_peer_context *wlan_mlo_get_mlpeer_by_aid(
				struct wlan_mlo_dev_context *ml_dev,
				uint16_t assoc_id)
{
	struct aid_search aid_arg;
	QDF_STATUS status;

	aid_arg.aid = assoc_id;
	status = wlan_mlo_iterate_ml_peerlist(ml_dev,
					      wlan_find_mlpeer_aid,
					      &aid_arg);
	if (status == QDF_STATUS_SUCCESS)
		return aid_arg.ml_peer;

	/* TODO: Take ref */

	return NULL;
}

struct wlan_mlo_peer_context *wlan_mlo_get_mlpeer_by_ml_peerid(
				struct wlan_mlo_dev_context *ml_dev,
				uint16_t ml_peerid)
{
	struct mlpeerid_search peerid_arg;
	QDF_STATUS status;

	peerid_arg.ml_peerid = ml_peerid;
	status = wlan_mlo_iterate_ml_peerlist(ml_dev,
					      wlan_find_mlpeer_ml_peerid,
					      &peerid_arg);
	if (status == QDF_STATUS_SUCCESS)
		return peerid_arg.ml_peer;

	/* TODO: Take ref */

	return NULL;
}

struct wlan_mlo_peer_context *wlan_mlo_get_mlpeer(
				struct wlan_mlo_dev_context *ml_dev,
				struct qdf_mac_addr *ml_addr)
{
	struct wlan_mlo_peer_context *ml_peer;
	struct wlan_mlo_peer_list *mlo_peer_list;

	mlo_peer_list = &ml_dev->mlo_peer_list;
	ml_peerlist_lock_acquire(mlo_peer_list);
	ml_peer = mlo_get_mlpeer(ml_dev, ml_addr);
	if (!ml_peer) {
		ml_peerlist_lock_release(mlo_peer_list);
		return NULL;
	}
	/* TODO: Take ref */

	ml_peerlist_lock_release(mlo_peer_list);
	return ml_peer;
}

static void wlan_mlo_peerlist_add_tail(qdf_list_t *obj_list,
				       struct wlan_mlo_peer_context *obj)
{
	qdf_list_insert_back(obj_list, &obj->peer_node);
}

static QDF_STATUS wlan_mlo_peerlist_remove_mlpeer(
				qdf_list_t *obj_list,
				struct wlan_mlo_peer_context *ml_peer)
{
	qdf_list_node_t *peer_node = NULL;

	if (!ml_peer)
		return QDF_STATUS_E_FAILURE;
	/* get vdev list node element */
	peer_node = &ml_peer->peer_node;
	/* list is empty, return failure */
	if (qdf_list_remove_node(obj_list, peer_node) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_dev_mlpeer_attach(struct wlan_mlo_dev_context *ml_dev,
				 struct wlan_mlo_peer_context *ml_peer)
{
	uint8_t hash_index;
	struct wlan_mlo_peer_list *mlo_peer_list;

	mlo_peer_list = &ml_dev->mlo_peer_list;
	ml_peerlist_lock_acquire(mlo_peer_list);
	if (mlo_get_mlpeer(ml_dev, &ml_peer->peer_mld_addr)) {
		ml_peerlist_lock_release(mlo_peer_list);
		return QDF_STATUS_E_EXISTS;
	}

	hash_index = WLAN_PEER_HASH(ml_peer->peer_mld_addr.bytes);
	wlan_mlo_peerlist_add_tail(&mlo_peer_list->peer_hash[hash_index],
				   ml_peer);
	ml_peerlist_lock_release(mlo_peer_list);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_dev_mlpeer_detach(struct wlan_mlo_dev_context *ml_dev,
				 struct wlan_mlo_peer_context *ml_peer)
{
	uint8_t hash_index;
	QDF_STATUS status;
	struct wlan_mlo_peer_list *mlo_peer_list;

	mlo_peer_list = &ml_dev->mlo_peer_list;
	ml_peerlist_lock_acquire(mlo_peer_list);
	hash_index = WLAN_PEER_HASH(ml_peer->peer_mld_addr.bytes);
	status = wlan_mlo_peerlist_remove_mlpeer(
					&mlo_peer_list->peer_hash[hash_index],
					ml_peer);
	ml_peerlist_lock_release(mlo_peer_list);

	return status;
}

QDF_STATUS mlo_dev_mlpeer_list_init(struct wlan_mlo_dev_context *ml_dev)
{
	struct wlan_mlo_peer_list *mlo_peer_list;
	uint16_t i;

	mlo_peer_list = &ml_dev->mlo_peer_list;
	ml_peerlist_lock_create(mlo_peer_list);
	for (i = 0; i < WLAN_PEER_HASHSIZE; i++)
		qdf_list_create(&mlo_peer_list->peer_hash[i],
				WLAN_UMAC_PSOC_MAX_PEERS +
				WLAN_MAX_PSOC_TEMP_PEERS);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_dev_mlpeer_list_deinit(struct wlan_mlo_dev_context *ml_dev)
{
	uint16_t i;
	struct wlan_mlo_peer_list *mlo_peer_list;

	/* deinit the lock */
	mlo_peer_list = &ml_dev->mlo_peer_list;
	ml_peerlist_lock_destroy(mlo_peer_list);
	for (i = 0; i < WLAN_PEER_HASHSIZE; i++)
		qdf_list_destroy(&mlo_peer_list->peer_hash[i]);

	return QDF_STATUS_SUCCESS;
}

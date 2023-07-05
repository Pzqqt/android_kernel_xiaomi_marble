/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains MLO manager init/deinit api's
 */
#include "wlan_cmn.h"
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_main.h"
#include <wlan_mlo_mgr_ap.h>
#include <wlan_mlo_mgr_peer.h>
#include <wlan_cm_public_struct.h>
#include "wlan_mlo_mgr_msgq.h"
#include <target_if_mlo_mgr.h>

static void mlo_global_ctx_deinit(void)
{
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_mgr_ctx)
		return;

	if (qdf_list_empty(&mlo_mgr_ctx->ml_dev_list))
		mlo_err("ML dev list is not empty");

	mlo_msgq_free();
	ml_peerid_lock_destroy(mlo_mgr_ctx);
	ml_link_lock_destroy(mlo_mgr_ctx);
	ml_aid_lock_destroy(mlo_mgr_ctx);
	qdf_list_destroy(&mlo_mgr_ctx->ml_dev_list);

	qdf_mem_free(mlo_mgr_ctx);
	wlan_objmgr_set_mlo_ctx(NULL);
}

static void mlo_global_ctx_init(void)
{
	struct mlo_mgr_context *mlo_mgr_ctx;

	/* If it is already created, ignore */
	if (wlan_objmgr_get_mlo_ctx()) {
		mlo_err("Global object is already created");
		return;
	}

	/* Allocation of memory for Global object */
	mlo_mgr_ctx = (struct mlo_mgr_context *)
			qdf_mem_malloc(sizeof(*mlo_mgr_ctx));
	if (!mlo_mgr_ctx)
		return;

	wlan_objmgr_set_mlo_ctx(mlo_mgr_ctx);

	qdf_list_create(&mlo_mgr_ctx->ml_dev_list, WLAN_UMAC_MLO_MAX_DEV);
	mlo_mgr_ctx->max_mlo_peer_id = MAX_MLO_PEER_ID;
	ml_peerid_lock_create(mlo_mgr_ctx);
	ml_link_lock_create(mlo_mgr_ctx);
	ml_aid_lock_create(mlo_mgr_ctx);
	mlo_mgr_ctx->mlo_is_force_primary_umac = 0;
	mlo_msgq_init();
}

QDF_STATUS wlan_mlo_mgr_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_mlo_tx_ops *mlo_tx_ops;

	if (!psoc) {
		mlo_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	mlo_tx_ops = target_if_mlo_get_tx_ops(psoc);
	if (!mlo_tx_ops) {
		mlo_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!mlo_tx_ops->register_events) {
		mlo_err("register_events function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mlo_tx_ops->register_events(psoc);
}

QDF_STATUS wlan_mlo_mgr_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_mlo_tx_ops *mlo_tx_ops;

	if (!psoc) {
		mlo_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	mlo_tx_ops = target_if_mlo_get_tx_ops(psoc);
	if (!mlo_tx_ops) {
		mlo_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!mlo_tx_ops->unregister_events) {
		mlo_err("unregister_events function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mlo_tx_ops->unregister_events(psoc);
}

QDF_STATUS wlan_mlo_mgr_init(void)
{
	QDF_STATUS status;

	mlo_global_ctx_init();

	status = wlan_objmgr_register_vdev_create_handler(
		WLAN_UMAC_COMP_MLO_MGR,
		wlan_mlo_mgr_vdev_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		mlo_err("Failed to register vdev create handler");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(WLAN_UMAC_COMP_MLO_MGR,
		wlan_mlo_mgr_vdev_destroyed_notification, NULL);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mlo_debug("MLO vdev create and delete handler registered with objmgr");
		return QDF_STATUS_SUCCESS;
	}
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_MLO_MGR,
				wlan_mlo_mgr_vdev_created_notification, NULL);

	return status;
}

QDF_STATUS wlan_mlo_mgr_deinit(void)
{
	QDF_STATUS status;
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_mgr_ctx) {
		mlo_err("MLO global object is not allocated");
		return QDF_STATUS_E_FAILURE;
	}

	mlo_global_ctx_deinit();

	status = wlan_objmgr_unregister_vdev_create_handler(
		WLAN_UMAC_COMP_MLO_MGR,
		wlan_mlo_mgr_vdev_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		mlo_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_vdev_destroy_handler(
			WLAN_UMAC_COMP_MLO_MGR,
			wlan_mlo_mgr_vdev_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		mlo_err("Failed to unregister vdev delete handler");

	return status;
}

static inline struct wlan_mlo_dev_context *mlo_list_peek_head(
					qdf_list_t *ml_list)
{
	struct wlan_mlo_dev_context *mld_ctx;
	qdf_list_node_t *ml_node = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	if (qdf_list_peek_front(ml_list, &ml_node) != QDF_STATUS_SUCCESS)
		return NULL;

	mld_ctx = qdf_container_of(ml_node, struct wlan_mlo_dev_context,
				   node);

	return mld_ctx;
}

static inline
struct wlan_mlo_dev_context *mlo_get_next_mld_ctx(qdf_list_t *ml_list,
					struct wlan_mlo_dev_context *mld_cur)
{
	struct wlan_mlo_dev_context *mld_next;
	qdf_list_node_t *node = &mld_cur->node;
	qdf_list_node_t *next_node = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	if (!node)
		return NULL;

	if (qdf_list_peek_next(ml_list, node, &next_node) !=
						QDF_STATUS_SUCCESS)
		return NULL;

	mld_next = qdf_container_of(next_node, struct wlan_mlo_dev_context,
				    node);
	return mld_next;
}

static inline struct wlan_mlo_dev_context
*wlan_mlo_get_mld_ctx_by_mldaddr(struct qdf_mac_addr *mldaddr)
{
	struct wlan_mlo_dev_context *mld_cur;
	struct wlan_mlo_dev_context *mld_next;
	qdf_list_t *ml_list;
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_mgr_ctx)
		return NULL;

	ml_link_lock_acquire(mlo_mgr_ctx);
	ml_list = &mlo_mgr_ctx->ml_dev_list;
	/* Get first mld context */
	mld_cur = mlo_list_peek_head(ml_list);
	/**
	 * Iterate through ml list, till ml mldaddr matches with
	 * entry of list
	 */
	while (mld_cur) {
		if (QDF_IS_STATUS_SUCCESS(WLAN_ADDR_EQ(&mld_cur->mld_addr,
					  mldaddr))) {
			ml_link_lock_release(mlo_mgr_ctx);
			return mld_cur;
		}
		/* get next mld node */
		mld_next = mlo_get_next_mld_ctx(ml_list, mld_cur);
		mld_cur = mld_next;
	}
	ml_link_lock_release(mlo_mgr_ctx);

	return NULL;
}

static QDF_STATUS mlo_ap_ctx_deinit(struct wlan_mlo_dev_context *ml_dev)
{
	wlan_mlo_vdev_aid_mgr_deinit(ml_dev);
	qdf_mem_free(ml_dev->ap_ctx);
	ml_dev->ap_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS mlo_ap_ctx_init(struct wlan_mlo_dev_context *ml_dev)
{
	struct wlan_mlo_ap *ap_ctx;

	ap_ctx = qdf_mem_malloc(sizeof(*ap_ctx));
	if (!ap_ctx) {
		mlo_err("MLO AP ctx alloc failure");
		return QDF_STATUS_E_NOMEM;
	}

	ml_dev->ap_ctx = ap_ctx;
	if (wlan_mlo_vdev_aid_mgr_init(ml_dev) != QDF_STATUS_SUCCESS) {
		mlo_ap_ctx_deinit(ml_dev);
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS mlo_dev_ctx_init(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *ml_dev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr *mld_addr;
	struct mlo_mgr_context *g_mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint8_t id = 0;

	mld_addr = (struct qdf_mac_addr *)wlan_vdev_mlme_get_mldaddr(vdev);
	ml_dev = wlan_mlo_get_mld_ctx_by_mldaddr(mld_addr);
	if (ml_dev) {
		mlo_dev_lock_acquire(ml_dev);
		while (id < WLAN_UMAC_MLO_MAX_VDEVS) {
			if (ml_dev->wlan_vdev_list[id]) {
				id++;
				continue;
			}
			ml_dev->wlan_vdev_list[id] = vdev;
			ml_dev->wlan_vdev_count++;
			vdev->mlo_dev_ctx = ml_dev;

			if (wlan_vdev_mlme_get_opmode(vdev) == QDF_SAP_MODE)
				wlan_mlo_vdev_alloc_aid_mgr(ml_dev, vdev);

			break;
		}
		mlo_dev_lock_release(ml_dev);
		return QDF_STATUS_SUCCESS;
	}

	/* Create a new ML dev context */
	ml_dev = qdf_mem_malloc(sizeof(*ml_dev));
	if (!ml_dev) {
		mlo_err("Failed to allocate memory for ML dev");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_copy_macaddr(&ml_dev->mld_addr, mld_addr);
	ml_dev->wlan_vdev_list[0] = vdev;
	ml_dev->wlan_vdev_count++;
	vdev->mlo_dev_ctx = ml_dev;

	mlo_dev_lock_create(ml_dev);
	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE) {
		ml_dev->sta_ctx = qdf_mem_malloc(sizeof(struct wlan_mlo_sta));
		if (!ml_dev->sta_ctx) {
			mlo_dev_lock_destroy(ml_dev);
			qdf_mem_free(ml_dev);
			return QDF_STATUS_E_NOMEM;
		}
		copied_conn_req_lock_create(ml_dev->sta_ctx);
	} else if (wlan_vdev_mlme_get_opmode(vdev) == QDF_SAP_MODE) {
		if (mlo_ap_ctx_init(ml_dev) != QDF_STATUS_SUCCESS) {
			mlo_dev_lock_destroy(ml_dev);
			qdf_mem_free(ml_dev);
			mlo_err("Failed to allocate memory for ap ctx");
			return QDF_STATUS_E_NOMEM;
		}
	}

	mlo_dev_mlpeer_list_init(ml_dev);

	ml_link_lock_acquire(g_mlo_ctx);
	if (qdf_list_size(&g_mlo_ctx->ml_dev_list) < WLAN_UMAC_MLO_MAX_DEV)
		qdf_list_insert_back(&g_mlo_ctx->ml_dev_list, &ml_dev->node);
	ml_link_lock_release(g_mlo_ctx);

	return status;
}

static QDF_STATUS mlo_dev_ctx_deinit(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *ml_dev;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr *mld_addr;
	struct mlo_mgr_context *g_mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint8_t id = 0;
	struct wlan_cm_connect_req *connect_req;

	mld_addr = (struct qdf_mac_addr *)wlan_vdev_mlme_get_mldaddr(vdev);
	ml_dev = wlan_mlo_get_mld_ctx_by_mldaddr(mld_addr);
	if (!ml_dev) {
		mlo_err("Failed to get MLD dev context");
		return QDF_STATUS_SUCCESS;
	} else {
		mlo_debug("deleting vdev from MLD device ctx");
		mlo_dev_lock_acquire(ml_dev);
		while (id < WLAN_UMAC_MLO_MAX_VDEVS) {
			if (ml_dev->wlan_vdev_list[id] == vdev) {
				if (wlan_vdev_mlme_get_opmode(vdev) ==
								QDF_SAP_MODE)
					wlan_mlo_vdev_free_aid_mgr(ml_dev,
								   vdev);
				ml_dev->wlan_vdev_list[id] = NULL;
				ml_dev->wlan_vdev_count--;
				vdev->mlo_dev_ctx = NULL;
				break;
			}
			id++;
		}
		mlo_dev_lock_release(ml_dev);
	}

	ml_link_lock_acquire(g_mlo_ctx);
	if (!ml_dev->wlan_vdev_count) {
		if (ml_dev->ap_ctx)
			mlo_ap_ctx_deinit(ml_dev);

		mlo_dev_mlpeer_list_deinit(ml_dev);
		qdf_list_remove_node(&g_mlo_ctx->ml_dev_list,
				     &ml_dev->node);
		if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE) {
			connect_req = ml_dev->sta_ctx->connect_req;
			if (connect_req) {
				if (connect_req->scan_ie.ptr) {
					qdf_mem_free(connect_req->scan_ie.ptr);
					connect_req->scan_ie.ptr = NULL;
				}

				if (connect_req->assoc_ie.ptr) {
					qdf_mem_free(connect_req->assoc_ie.ptr);
					connect_req->assoc_ie.ptr = NULL;
				}
				qdf_mem_free(ml_dev->sta_ctx->connect_req);
			}
			if (ml_dev->sta_ctx->assoc_rsp.ptr)
				qdf_mem_free(ml_dev->sta_ctx->assoc_rsp.ptr);

			copied_conn_req_lock_destroy(ml_dev->sta_ctx);

			qdf_mem_free(ml_dev->sta_ctx);
		}
		else if (wlan_vdev_mlme_get_opmode(vdev) == QDF_SAP_MODE)
			qdf_mem_free(ml_dev->ap_ctx);

		mlo_dev_lock_destroy(ml_dev);
		qdf_mem_free(ml_dev);
	}
	ml_link_lock_release(g_mlo_ctx);
	return status;
}

QDF_STATUS wlan_mlo_mgr_vdev_created_notification(struct wlan_objmgr_vdev *vdev,
						  void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr *mld_addr;

	mld_addr = (struct qdf_mac_addr *)wlan_vdev_mlme_get_mldaddr(vdev);
	if (qdf_is_macaddr_zero(mld_addr)) {
		/* It's not a ML interface*/
		return QDF_STATUS_SUCCESS;
	}
	mlo_debug("MLD addr" QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(mld_addr->bytes));
	status = mlo_dev_ctx_init(vdev);

	return status;
}

QDF_STATUS wlan_mlo_mgr_vdev_destroyed_notification(struct wlan_objmgr_vdev *vdev,
						    void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct qdf_mac_addr *mld_addr;

	mld_addr = (struct qdf_mac_addr *)wlan_vdev_mlme_get_mldaddr(vdev);
	if (qdf_is_macaddr_zero(mld_addr)) {
		/* It's not a ML interface*/
		return QDF_STATUS_SUCCESS;
	}
	mlo_debug("MLD addr" QDF_MAC_ADDR_FMT,
		  QDF_MAC_ADDR_REF(mld_addr->bytes));

	status = mlo_dev_ctx_deinit(vdev);

	return status;
}

QDF_STATUS wlan_mlo_mgr_update_mld_addr(struct qdf_mac_addr *old_mac,
					struct qdf_mac_addr *new_mac)
{
	struct wlan_mlo_dev_context *ml_dev;

	ml_dev = wlan_mlo_get_mld_ctx_by_mldaddr(old_mac);
	if (!ml_dev) {
		mlo_err("ML dev context not found for MLD:" QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(old_mac->bytes));
		return QDF_STATUS_E_INVAL;
	}
	qdf_copy_macaddr(&ml_dev->mld_addr, new_mac);

	return QDF_STATUS_SUCCESS;
}

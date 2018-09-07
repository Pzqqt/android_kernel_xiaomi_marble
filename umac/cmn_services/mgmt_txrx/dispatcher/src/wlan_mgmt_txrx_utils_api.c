/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 *  DOC:    wlan_mgmt_txrx_utils_api.c
 *  This file contains mgmt txrx public API definitions for umac
 *  converged components.
 */

#include "wlan_mgmt_txrx_utils_api.h"
#include "../../core/src/wlan_mgmt_txrx_main_i.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_objmgr_peer_obj.h"
#include "qdf_nbuf.h"
#include "wlan_lmac_if_api.h"

/**
 * wlan_mgmt_txrx_psoc_obj_create_notification() - called from objmgr when psoc
 *                                                 is created
 * @psoc: psoc context
 * @arg: argument
 *
 * This function gets called from object manager when psoc is being created and
 * creates mgmt_txrx context, mgmt desc pool.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_psoc_obj_create_notification(
			struct wlan_objmgr_psoc *psoc,
			void *arg)
{
	struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx;
	QDF_STATUS status;

	if (!psoc) {
		mgmt_txrx_err("psoc context passed is NULL");
		status = QDF_STATUS_E_INVAL;
		goto err_return;
	}

	mgmt_txrx_psoc_ctx = qdf_mem_malloc(sizeof(*mgmt_txrx_psoc_ctx));
	if (!mgmt_txrx_psoc_ctx) {
		mgmt_txrx_err("Failed to allocate mgmt txrx context");
		status = QDF_STATUS_E_NOMEM;
		goto err_return;
	}

	mgmt_txrx_psoc_ctx->psoc = psoc;

	qdf_spinlock_create(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);

	if (wlan_objmgr_psoc_component_obj_attach(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX,
				mgmt_txrx_psoc_ctx, QDF_STATUS_SUCCESS)
			!= QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to attach mgmt txrx ctx in psoc ctx");
		status = QDF_STATUS_E_FAILURE;
		goto err_psoc_attach;
	}

	mgmt_txrx_info("Mgmt txrx creation successful, mgmt txrx ctx: %pK, psoc: %pK",
			mgmt_txrx_psoc_ctx, psoc);

	return QDF_STATUS_SUCCESS;

err_psoc_attach:
	qdf_spinlock_destroy(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
	qdf_mem_free(mgmt_txrx_psoc_ctx);
err_return:
	return status;
}

/**
 * wlan_mgmt_txrx_psoc_obj_destroy_notification() - called from objmgr when
 *                                                  psoc is destroyed
 * @psoc: psoc context
 * @arg: argument
 *
 * This function gets called from object manager when psoc is being destroyed
 * psoc deletes mgmt_txrx context, mgmt desc pool.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_psoc_obj_destroy_notification(
			struct wlan_objmgr_psoc *psoc,
			void *arg)
{
	struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx;

	if (!psoc) {
		mgmt_txrx_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mgmt_txrx_psoc_ctx = wlan_objmgr_psoc_get_comp_private_obj(
			psoc, WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_psoc_ctx) {
		mgmt_txrx_err("mgmt txrx context is already NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mgmt_txrx_info("deleting mgmt txrx psoc obj, mgmt txrx ctx: %pK, psoc: %pK",
			mgmt_txrx_psoc_ctx, psoc);
	if (wlan_objmgr_psoc_component_obj_detach(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX, mgmt_txrx_psoc_ctx)
			!= QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to detach mgmt txrx ctx in psoc ctx");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spinlock_destroy(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
	qdf_mem_free(mgmt_txrx_psoc_ctx);

	mgmt_txrx_info("mgmt txrx deletion successful, psoc: %pK", psoc);

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_txrx_pdev_obj_create_notification() - called from objmgr when pdev
 *                                                 is created
 * @pdev: pdev context
 * @arg: argument
 *
 * This function gets called from object manager when pdev is being created and
 * creates mgmt_txrx context, mgmt desc pool.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_pdev_obj_create_notification(
			struct wlan_objmgr_pdev *pdev,
			void *arg)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_stats_t *mgmt_txrx_stats;
	QDF_STATUS status;

	if (!pdev) {
		mgmt_txrx_err("pdev context passed is NULL");
		status = QDF_STATUS_E_INVAL;
		goto err_return;

	}

	mgmt_txrx_pdev_ctx = qdf_mem_malloc(sizeof(*mgmt_txrx_pdev_ctx));
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("Failed to allocate mgmt txrx context");
		status = QDF_STATUS_E_NOMEM;
		goto err_return;
	}

	mgmt_txrx_pdev_ctx->pdev = pdev;

	status = wlan_mgmt_txrx_desc_pool_init(mgmt_txrx_pdev_ctx);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err(
			"Failed to initialize mgmt desc. pool with status: %u",
			status);
		goto err_desc_pool_init;
	}

	mgmt_txrx_stats = qdf_mem_malloc(sizeof(*mgmt_txrx_stats));
	if (!mgmt_txrx_stats) {
		mgmt_txrx_err(
			"Failed to allocate memory for mgmt txrx stats structure");
		status = QDF_STATUS_E_NOMEM;
		goto err_mgmt_txrx_stats;
	}
	mgmt_txrx_pdev_ctx->mgmt_txrx_stats = mgmt_txrx_stats;

	qdf_wake_lock_create(&mgmt_txrx_pdev_ctx->wakelock_tx_cmp,
			     "mgmt_txrx tx_cmp");

	if (wlan_objmgr_pdev_component_obj_attach(pdev,
			WLAN_UMAC_COMP_MGMT_TXRX,
			mgmt_txrx_pdev_ctx, QDF_STATUS_SUCCESS)
			!= QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to attach mgmt txrx ctx in pdev ctx");
		status = QDF_STATUS_E_FAILURE;
		goto err_pdev_attach;
	}

	mgmt_txrx_info(
		"Mgmt txrx creation successful, mgmt txrx ctx: %pK, pdev: %pK",
		mgmt_txrx_pdev_ctx, pdev);

	return QDF_STATUS_SUCCESS;

err_pdev_attach:
	qdf_wake_lock_destroy(&mgmt_txrx_pdev_ctx->wakelock_tx_cmp);
	qdf_mem_free(mgmt_txrx_stats);
err_mgmt_txrx_stats:
	wlan_mgmt_txrx_desc_pool_deinit(mgmt_txrx_pdev_ctx);
err_desc_pool_init:
	qdf_mem_free(mgmt_txrx_pdev_ctx);
err_return:
	return status;
}

/**
 * wlan_mgmt_txrx_pdev_obj_destroy_notification() - called from objmgr when
 *                                                  pdev is destroyed
 * @pdev: pdev context
 * @arg: argument
 *
 * This function gets called from object manager when pdev is being destroyed
 * pdev deletes mgmt_txrx context, mgmt desc pool.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_pdev_obj_destroy_notification(
			struct wlan_objmgr_pdev *pdev,
			void *arg)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;

	if (!pdev) {
		mgmt_txrx_err("pdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mgmt_txrx_pdev_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("mgmt txrx context is already NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mgmt_txrx_info("deleting mgmt txrx pdev obj, mgmt txrx ctx: %pK, pdev: %pK",
			mgmt_txrx_pdev_ctx, pdev);
	if (wlan_objmgr_pdev_component_obj_detach(pdev,
				WLAN_UMAC_COMP_MGMT_TXRX, mgmt_txrx_pdev_ctx)
			!= QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to detach mgmt txrx ctx in pdev ctx");
		return QDF_STATUS_E_FAILURE;
	}

	wlan_mgmt_txrx_desc_pool_deinit(mgmt_txrx_pdev_ctx);
	qdf_mem_free(mgmt_txrx_pdev_ctx->mgmt_txrx_stats);
	qdf_wake_lock_destroy(&mgmt_txrx_pdev_ctx->wakelock_tx_cmp);
	qdf_mem_free(mgmt_txrx_pdev_ctx);

	mgmt_txrx_info("mgmt txrx deletion successful, pdev: %pK", pdev);

	return QDF_STATUS_SUCCESS;
}


QDF_STATUS wlan_mgmt_txrx_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_psoc_create_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_psoc_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to register mgmt txrx psoc create handler");
		goto err_psoc_create;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_psoc_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to register mgmt txrx psoc destroy handler");
		goto err_psoc_delete;
	}

	status = wlan_objmgr_register_pdev_create_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_pdev_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to register mgmt txrx pdev obj create handler");
		goto err_pdev_create;
	}

	status = wlan_objmgr_register_pdev_destroy_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_pdev_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to register mgmt txrx obj destroy handler");
		goto err_pdev_delete;
	}

	mgmt_txrx_info("Successfully registered create and destroy handlers with objmgr");
	return QDF_STATUS_SUCCESS;

err_pdev_delete:
	wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_MGMT_TXRX,
			wlan_mgmt_txrx_pdev_obj_create_notification, NULL);
err_pdev_create:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_MGMT_TXRX,
			wlan_mgmt_txrx_psoc_obj_destroy_notification, NULL);
err_psoc_delete:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_MGMT_TXRX,
			wlan_mgmt_txrx_psoc_obj_create_notification, NULL);
err_psoc_create:
	return status;
}

QDF_STATUS wlan_mgmt_txrx_deinit(void)
{
	if (wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_psoc_obj_create_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_psoc_obj_destroy_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_pdev_create_handler(WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_pdev_obj_create_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	if (wlan_objmgr_unregister_pdev_destroy_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_pdev_obj_destroy_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}


	mgmt_txrx_info("Successfully unregistered create and destroy handlers with objmgr");
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_mgmt_frame_tx(struct wlan_objmgr_peer *peer,
					void *context,
					qdf_nbuf_t buf,
					mgmt_tx_download_comp_cb tx_comp_cb,
					mgmt_ota_comp_cb tx_ota_comp_cb,
					enum wlan_umac_comp_id comp_id,
					void *mgmt_tx_params)
{
	struct mgmt_txrx_desc_elem_t *desc;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct mgmt_txrx_priv_pdev_context *txrx_ctx;
	struct wlan_objmgr_vdev *vdev;

	if (!peer) {
		mgmt_txrx_err("peer passed is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	wlan_objmgr_peer_get_ref(peer, WLAN_MGMT_NB_ID);

	vdev = wlan_peer_get_vdev(peer);
	if (!vdev) {
		mgmt_txrx_err("vdev unavailable for peer %pK", peer);
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mgmt_txrx_err("psoc unavailable for peer %pK vdev %pK",
				peer, vdev);
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		return QDF_STATUS_E_NULL_VALUE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mgmt_txrx_err("pdev unavailable for peer %pK vdev %pK",
				peer, vdev);
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		return QDF_STATUS_E_NULL_VALUE;
	}

	txrx_ctx = (struct mgmt_txrx_priv_pdev_context *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!txrx_ctx) {
		mgmt_txrx_err("No txrx context for peer %pK pdev %pK",
				peer, pdev);
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		return QDF_STATUS_E_NULL_VALUE;
	}

	desc = wlan_mgmt_txrx_desc_get(txrx_ctx);
	if (!desc) {
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		return QDF_STATUS_E_RESOURCES;
	}

	desc->nbuf = buf;
	desc->tx_ota_cmpl_cb = tx_ota_comp_cb;
	desc->tx_dwnld_cmpl_cb = tx_comp_cb;
	desc->peer = peer;
	desc->vdev_id = wlan_vdev_get_id(vdev);
	desc->context = context;

	if (!psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.mgmt_tx_send) {
		mgmt_txrx_err(
				"mgmt txrx txop to send mgmt frame is NULL for psoc: %pK",
				psoc);
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		desc->nbuf = NULL;
		wlan_mgmt_txrx_desc_put(txrx_ctx, desc->desc_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.mgmt_tx_send(
			vdev, buf, desc->desc_id, mgmt_tx_params)) {
		mgmt_txrx_err("Mgmt send fail for peer %pK psoc %pK pdev: %pK",
				peer, psoc, pdev);
		wlan_objmgr_peer_release_ref(peer, WLAN_MGMT_NB_ID);
		desc->nbuf = NULL;
		wlan_mgmt_txrx_desc_put(txrx_ctx, desc->desc_id);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_beacon_frame_tx(struct wlan_objmgr_peer *peer,
					  qdf_nbuf_t buf,
					  enum wlan_umac_comp_id comp_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;

	vdev = wlan_peer_get_vdev(peer);
	if (!vdev) {
		mgmt_txrx_err("vdev unavailable for peer %pK", peer);
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mgmt_txrx_err("psoc unavailable for peer %pK", peer);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.beacon_send) {
		mgmt_txrx_err("mgmt txrx tx op to send beacon frame is NULL for psoc: %pK",
				psoc);
		return QDF_STATUS_E_FAILURE;
	}

	if (psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.beacon_send(vdev, buf)) {
		mgmt_txrx_err("Beacon send fail for peer %pK psoc %pK",
				peer, psoc);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_SUPPORT_FILS
QDF_STATUS
wlan_mgmt_txrx_fd_action_frame_tx(struct wlan_objmgr_vdev *vdev,
				  qdf_nbuf_t buf,
				  enum wlan_umac_comp_id comp_id)
{
	struct wlan_objmgr_psoc *psoc;
	uint32_t vdev_id;

	if (!vdev) {
		mgmt_txrx_err("Invalid vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}
	vdev_id = wlan_vdev_get_id(vdev);
	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		mgmt_txrx_err("psoc unavailable for vdev %d", vdev_id);
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.fd_action_frame_send) {
		mgmt_txrx_err("mgmt txrx txop to send fd action frame is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.fd_action_frame_send(
			vdev, buf)) {
		mgmt_txrx_err("FD send fail for vdev %d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_FILS */

/**
 * wlan_mgmt_txrx_create_rx_handler() - creates rx handler node for umac comp.
 * @mgmt_txrx_psoc_ctx: mgmt txrx context
 * @mgmt_rx_cb: mgmt rx callback to be registered
 * @comp_id: umac component id
 * @frm_type: mgmt. frame for which cb to be registered.
 *
 * This function creates rx handler node for frame type and
 * umac component passed in the func.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_create_rx_handler(
				struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx,
				mgmt_frame_rx_callback mgmt_rx_cb,
				enum wlan_umac_comp_id comp_id,
				enum mgmt_frame_type frm_type)
{
	struct mgmt_rx_handler *rx_handler;

	rx_handler = qdf_mem_malloc(sizeof(*rx_handler));
	if (!rx_handler) {
		mgmt_txrx_err("Couldn't allocate memory for rx handler");
		return QDF_STATUS_E_NOMEM;
	}

	rx_handler->comp_id = comp_id;
	rx_handler->rx_cb = mgmt_rx_cb;

	qdf_spin_lock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
	rx_handler->next = mgmt_txrx_psoc_ctx->mgmt_rx_comp_cb[frm_type];
	mgmt_txrx_psoc_ctx->mgmt_rx_comp_cb[frm_type] = rx_handler;
	qdf_spin_unlock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);

	mgmt_txrx_info("Callback registered for comp_id: %d, frm_type: %d",
			comp_id, frm_type);
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_txrx_delete_rx_handler() - deletes rx handler node for umac comp.
 * @mgmt_txrx_psoc_ctx: mgmt txrx context
 * @mgmt_rx_cb: mgmt rx callback to be deregistered
 * @comp_id: umac component id
 * @frm_type: mgmt. frame for which cb to be registered.
 *
 * This function deletes rx handler node for frame type and
 * umac component passed in the func.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_delete_rx_handler(
		struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx,
		mgmt_frame_rx_callback mgmt_rx_cb,
		enum wlan_umac_comp_id comp_id,
		enum mgmt_frame_type frm_type)
{
	struct mgmt_rx_handler *rx_handler = NULL, *rx_handler_prev = NULL;
	bool delete = false;

	qdf_spin_lock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);
	rx_handler = mgmt_txrx_psoc_ctx->mgmt_rx_comp_cb[frm_type];
	while (rx_handler) {
		if (rx_handler->comp_id == comp_id &&
				rx_handler->rx_cb == mgmt_rx_cb) {
			if (rx_handler_prev)
				rx_handler_prev->next =
					rx_handler->next;
			else
				mgmt_txrx_psoc_ctx->mgmt_rx_comp_cb[frm_type] =
					rx_handler->next;

			qdf_mem_free(rx_handler);
			delete = true;
			break;
		}

		rx_handler_prev = rx_handler;
		rx_handler = rx_handler->next;
	}
	qdf_spin_unlock_bh(&mgmt_txrx_psoc_ctx->mgmt_txrx_psoc_ctx_lock);

	if (!delete) {
		mgmt_txrx_err("No callback registered for comp_id: %d, frm_type: %d",
				comp_id, frm_type);
		return QDF_STATUS_E_FAILURE;
	}

	mgmt_txrx_info("Callback deregistered for comp_id: %d, frm_type: %d",
			comp_id, frm_type);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_register_rx_cb(
			struct wlan_objmgr_psoc *psoc,
			enum wlan_umac_comp_id comp_id,
			struct mgmt_txrx_mgmt_frame_cb_info *frm_cb_info,
			uint8_t num_entries)
{
	struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx;
	QDF_STATUS status;
	uint8_t i, j;

	if (!psoc) {
		mgmt_txrx_err("psoc context is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (comp_id >= WLAN_UMAC_MAX_COMPONENTS) {
		mgmt_txrx_err("Invalid component id %d passed", comp_id);
		return QDF_STATUS_E_INVAL;
	}

	if (!num_entries || num_entries >= MGMT_MAX_FRAME_TYPE) {
		mgmt_txrx_err("Invalid value for num_entries: %d passed",
				num_entries);
		return QDF_STATUS_E_INVAL;
	}

	if (!frm_cb_info) {
		mgmt_txrx_err("frame cb info pointer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mgmt_txrx_psoc_ctx = (struct mgmt_txrx_priv_psoc_context *)
			wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_psoc_ctx) {
		mgmt_txrx_err("mgmt txrx context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < num_entries; i++) {
		status = wlan_mgmt_txrx_create_rx_handler(mgmt_txrx_psoc_ctx,
				frm_cb_info[i].mgmt_rx_cb, comp_id,
				frm_cb_info[i].frm_type);
		if (status != QDF_STATUS_SUCCESS) {
			for (j = 0; j < i; j++) {
				wlan_mgmt_txrx_delete_rx_handler(
					mgmt_txrx_psoc_ctx,
					frm_cb_info[j].mgmt_rx_cb,
					comp_id, frm_cb_info[j].frm_type);
			}
			return status;
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_deregister_rx_cb(
			struct wlan_objmgr_psoc *psoc,
			enum wlan_umac_comp_id comp_id,
			struct mgmt_txrx_mgmt_frame_cb_info *frm_cb_info,
			uint8_t num_entries)
{
	struct mgmt_txrx_priv_psoc_context *mgmt_txrx_psoc_ctx;
	uint8_t i;

	if (!psoc) {
		mgmt_txrx_err("psoc context is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (comp_id >= WLAN_UMAC_MAX_COMPONENTS) {
		mgmt_txrx_err("Invalid component id %d passed", comp_id);
		return QDF_STATUS_E_INVAL;
	}

	if (!num_entries || num_entries >= MGMT_MAX_FRAME_TYPE) {
		mgmt_txrx_err("Invalid value for num_entries: %d passed",
				num_entries);
		return QDF_STATUS_E_INVAL;
	}

	if (!frm_cb_info) {
		mgmt_txrx_err("frame cb info pointer is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mgmt_txrx_psoc_ctx = (struct mgmt_txrx_priv_psoc_context *)
			wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_psoc_ctx) {
		mgmt_txrx_err("mgmt txrx context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < num_entries; i++) {
		wlan_mgmt_txrx_delete_rx_handler(mgmt_txrx_psoc_ctx,
				frm_cb_info[i].mgmt_rx_cb, comp_id,
				frm_cb_info[i].frm_type);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_desc_elem_t *mgmt_desc;
	uint32_t pool_size;
	uint32_t index;

	if (!pdev) {
		mgmt_txrx_err("pdev context is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mgmt_txrx_err("psoc unavailable for pdev %pK", pdev);
		return QDF_STATUS_E_NULL_VALUE;
	}

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
		WLAN_UMAC_COMP_MGMT_TXRX);

	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("mgmt txrx context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pool_size = mgmt_txrx_pdev_ctx->mgmt_desc_pool.free_list.max_size;
	if (!pool_size) {
		mgmt_txrx_err("pool size is 0");
		return QDF_STATUS_E_FAILURE;
	}

	for (index = 0; index < pool_size; index++) {
		if (mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[index].in_use) {
			mgmt_txrx_info(
				"mgmt descriptor with desc id: %d not in freelist",
				index);
			mgmt_desc = &mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[index];
			if (psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.
					tx_drain_nbuf_op)
				psoc->soc_cb.tx_ops.mgmt_txrx_tx_ops.
					tx_drain_nbuf_op(pdev, mgmt_desc->nbuf);
			qdf_nbuf_free(mgmt_desc->nbuf);
			wlan_objmgr_peer_release_ref(mgmt_desc->peer,
				WLAN_MGMT_NB_ID);
			wlan_mgmt_txrx_desc_put(mgmt_txrx_pdev_ctx, index);
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_vdev_drain(struct wlan_objmgr_vdev *vdev,
				mgmt_frame_fill_peer_cb mgmt_fill_peer_cb,
				void *status)
{
	struct wlan_objmgr_pdev *pdev;
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;
	struct mgmt_txrx_desc_elem_t *mgmt_desc;
	struct wlan_objmgr_peer *peer;
	struct wlan_objmgr_vdev *peer_vdev;
	uint32_t pool_size;
	int i;

	if (!vdev) {
		mgmt_txrx_err("vdev context is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mgmt_txrx_err("pdev context is NULL");
		return QDF_STATUS_E_INVAL;
	}
	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_pdev_ctx) {
		mgmt_txrx_err("mgmt txrx context is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pool_size = mgmt_txrx_pdev_ctx->mgmt_desc_pool.free_list.max_size;
	if (!pool_size) {
		mgmt_txrx_err("pool size is 0");
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < pool_size; i++) {
		if (mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[i].in_use) {
			mgmt_desc = &mgmt_txrx_pdev_ctx->mgmt_desc_pool.pool[i];
			peer = mgmt_txrx_get_peer(pdev, mgmt_desc->desc_id);
			if (peer) {
				peer_vdev = wlan_peer_get_vdev(peer);
				if (peer_vdev == vdev) {
					if (mgmt_fill_peer_cb)
						mgmt_fill_peer_cb(peer, mgmt_desc->nbuf);
					mgmt_txrx_tx_completion_handler(pdev,
						mgmt_desc->desc_id, 0, status);
				}
			}
		}
	}

	return QDF_STATUS_SUCCESS;
}

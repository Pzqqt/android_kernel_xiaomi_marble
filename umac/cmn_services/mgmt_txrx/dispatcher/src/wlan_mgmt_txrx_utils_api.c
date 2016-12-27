/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#include "wlan_mgmt_txrx_main_i.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_global_obj.h"
#include "qdf_nbuf.h"

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
	struct mgmt_txrx_priv_context *mgmt_txrx_ctx;
	struct mgmt_txrx_stats_t *mgmt_txrx_stats;
	QDF_STATUS status;

	if (!psoc) {
		mgmt_txrx_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mgmt_txrx_ctx = qdf_mem_malloc(sizeof(*mgmt_txrx_ctx));
	if (!mgmt_txrx_ctx) {
		mgmt_txrx_err("Failed to allocate mgmt txrx context");
		return QDF_STATUS_E_NOMEM;
	}

	mgmt_txrx_ctx->psoc = psoc;

	status = wlan_mgmt_txrx_desc_pool_init(mgmt_txrx_ctx,
					       MGMT_DESC_POOL_MAX);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to initialize mgmt desc. pool with status: %u",
				status);
		goto err_desc_pool_init;
	}

	mgmt_txrx_stats = qdf_mem_malloc(sizeof(*mgmt_txrx_stats));
	if (!mgmt_txrx_stats) {
		mgmt_txrx_err("Failed to allocate memory for mgmt txrx stats structure");
		status = QDF_STATUS_E_NOMEM;
		goto err_mgmt_txrx_stats;
	}
	mgmt_txrx_ctx->mgmt_txrx_stats = mgmt_txrx_stats;

	qdf_spinlock_create(&mgmt_txrx_ctx->mgmt_txrx_ctx_lock);

	if (wlan_objmgr_psoc_component_obj_attach(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX,
				mgmt_txrx_ctx, QDF_STATUS_SUCCESS)
			!= QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to attach mgmt txrx ctx in psoc ctx");
		status = QDF_STATUS_E_FAILURE;
		goto err_psoc_attach;
	}

	mgmt_txrx_info("Mgmt txrx creation successful, mgmt txrx ctx: %p, psoc: %p",
			mgmt_txrx_ctx, psoc);

	return QDF_STATUS_SUCCESS;

err_psoc_attach:
	qdf_spinlock_destroy(&mgmt_txrx_ctx->mgmt_txrx_ctx_lock);
	qdf_mem_free(mgmt_txrx_stats);
err_mgmt_txrx_stats:
	wlan_mgmt_txrx_desc_pool_deinit(mgmt_txrx_ctx);
err_desc_pool_init:
	qdf_mem_free(mgmt_txrx_ctx);
	return status;
}

/**
 * wlan_mgmt_txrx_psoc_obj_delete_notification() - called from objmgr when psoc
 *                                                 is deleted
 * @psoc: psoc context
 * @arg: argument
 *
 * This function gets called from object manager when psoc is being deleted and
 * deletes mgmt_txrx context, mgmt desc pool.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS wlan_mgmt_txrx_psoc_obj_delete_notification(
			struct wlan_objmgr_psoc *psoc,
			void *arg)
{
	struct mgmt_txrx_priv_context *mgmt_txrx_ctx;

	if (!psoc) {
		mgmt_txrx_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	mgmt_txrx_ctx = wlan_objmgr_psoc_get_comp_private_obj(
			psoc, WLAN_UMAC_COMP_MGMT_TXRX);
	if (!mgmt_txrx_ctx) {
		mgmt_txrx_err("mgmt txrx context is already NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mgmt_txrx_info("deleting mgmt txrx psoc obj, mgmt txrx ctx: %p, psoc: %p",
			mgmt_txrx_ctx, psoc);
	if (wlan_objmgr_psoc_component_obj_detach(psoc,
				WLAN_UMAC_COMP_MGMT_TXRX, mgmt_txrx_ctx)
			!= QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to detach mgmt txrx ctx in psoc ctx");
		return QDF_STATUS_E_FAILURE;
	}

	wlan_mgmt_txrx_desc_pool_deinit(mgmt_txrx_ctx);
	qdf_mem_free(mgmt_txrx_ctx->mgmt_txrx_stats);
	qdf_spinlock_destroy(&mgmt_txrx_ctx->mgmt_txrx_ctx_lock);
	qdf_mem_free(mgmt_txrx_ctx);

	mgmt_txrx_info("mgmt txrx deletion successful, psoc: %p", psoc);

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
		mgmt_txrx_err("Failed to register mgmt txrx obj create handler");
		goto err_psoc_create;
	}

	status = wlan_objmgr_register_psoc_delete_handler(
				WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_psoc_obj_delete_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		mgmt_txrx_err("Failed to register mgmt txrx obj delete handler");
		goto err_psoc_delete;
	}

	mgmt_txrx_info("Successfully registered create and delete handlers with objmgr");
	return QDF_STATUS_SUCCESS;

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

	if (wlan_objmgr_unregister_psoc_delete_handler(WLAN_UMAC_COMP_MGMT_TXRX,
				wlan_mgmt_txrx_psoc_obj_delete_notification,
				NULL)
			!= QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}

	mgmt_txrx_info("Successfully unregistered create and delete handlers with objmgr");
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
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_beacon_frame_tx(struct wlan_objmgr_peer *peer,
					  qdf_nbuf_t buf,
					  enum wlan_umac_comp_id comp_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_register_rx_cb(struct wlan_objmgr_psoc *psoc,
					 mgmt_frame_rx_callback mgmt_rx_cb,
					 enum wlan_umac_comp_id comp_id,
					 enum mgmt_frame_type frm_type)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mgmt_txrx_deregister_rx_cb(
			struct wlan_objmgr_psoc *psoc,
			enum wlan_umac_comp_id comp_id,
			enum mgmt_frame_type frm_type)
{
	return QDF_STATUS_SUCCESS;
}

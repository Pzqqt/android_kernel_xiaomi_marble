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

/**
 *  DOC: wlan_mgmt_txrx_rx_reo_utils_api.c
 *  This file contains mgmt rx re-ordering related public function definitions
 */

#include <wlan_mgmt_txrx_rx_reo_utils_api.h>
#include <wlan_mgmt_txrx_rx_reo_tgt_api.h>
#include "../../core/src/wlan_mgmt_txrx_rx_reo_i.h"
#include "../../core/src/wlan_mgmt_txrx_main_i.h"

struct mgmt_rx_reo_pdev_info *
wlan_mgmt_rx_reo_get_priv_object(struct wlan_objmgr_pdev *pdev)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return NULL;
	}

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
						      WLAN_UMAC_COMP_MGMT_TXRX);

	if (!mgmt_txrx_pdev_ctx) {
		mgmt_rx_reo_err("mgmt txrx context is NULL");
		return NULL;
	}

	return mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx;
}

QDF_STATUS
wlan_mgmt_rx_reo_set_priv_object(struct wlan_objmgr_pdev *pdev,
				 struct mgmt_rx_reo_pdev_info *reo_pdev_context)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
						      WLAN_UMAC_COMP_MGMT_TXRX);

	if (!mgmt_txrx_pdev_ctx) {
		mgmt_rx_reo_err("mgmt txrx context is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = reo_pdev_context;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_rx_reo_initialize_snapshot_params() - Initialize a given snapshot
 * params object
 * @snapshot_params: Pointer to snapshot params object
 *
 * Return: void
 */
static void
wlan_mgmt_rx_reo_initialize_snapshot_params(
			struct mgmt_rx_reo_snapshot_params *snapshot_params)
{
	snapshot_params->valid = false;
	snapshot_params->mgmt_pkt_ctr = 0;
	snapshot_params->global_timestamp = 0;
}

QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_create_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	QDF_STATUS status;
	QDF_STATUS temp_status;
	struct mgmt_rx_reo_pdev_info *mgmt_rx_reo_pdev_ctx = NULL;
	enum mgmt_rx_reo_shared_snapshot_id snapshot_id;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto failure;
	}

	mgmt_rx_reo_pdev_ctx = qdf_mem_malloc(sizeof(*mgmt_rx_reo_pdev_ctx));
	if (!mgmt_rx_reo_pdev_ctx) {
		mgmt_rx_reo_err("Allocation failure for REO pdev context");
		status = QDF_STATUS_E_NOMEM;
		goto failure;
	}

	snapshot_id = 0;
	while (snapshot_id < MGMT_RX_REO_SHARED_SNAPSHOT_MAX) {
		struct mgmt_rx_reo_snapshot **snapshot_address;

		snapshot_address = &mgmt_rx_reo_pdev_ctx->
				host_target_shared_snapshot[snapshot_id];
		temp_status = tgt_mgmt_rx_reo_get_snapshot_address(
				pdev, snapshot_id, snapshot_address);
		if (QDF_IS_STATUS_ERROR(temp_status)) {
			mgmt_rx_reo_err("Get snapshot address failed, id = %u",
					snapshot_id);
			status = temp_status;
			goto failure;
		}

		wlan_mgmt_rx_reo_initialize_snapshot_params(
				&mgmt_rx_reo_pdev_ctx->
				last_valid_shared_snapshot[snapshot_id]);
		snapshot_id++;
	}

	/* Initialize Host snapshot params */
	wlan_mgmt_rx_reo_initialize_snapshot_params(&mgmt_rx_reo_pdev_ctx->
						    host_snapshot);

	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = mgmt_rx_reo_pdev_ctx;

	return QDF_STATUS_SUCCESS;

failure:
	if (mgmt_rx_reo_pdev_ctx)
		qdf_mem_free(mgmt_rx_reo_pdev_ctx);

	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = NULL;

	return status;
}

QDF_STATUS
wlan_mgmt_rx_reo_pdev_obj_destroy_notification(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	qdf_mem_free(mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx);
	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_mgmt_rx_reo_deinit(void)
{
	return mgmt_rx_reo_deinit_context();
}

QDF_STATUS
wlan_mgmt_rx_reo_init(void)
{
	return mgmt_rx_reo_init_context();
}

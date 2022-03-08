/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_api.c
 * This file defines the APIs of TWT component.
 */
#include <wlan_twt_api.h>
#include "twt/core/src/wlan_twt_objmgr_handler.h"
#include "twt/core/src/wlan_twt_common.h"

struct wlan_lmac_if_twt_tx_ops *
wlan_twt_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!psoc) {
		twt_err("psoc is null");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		twt_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->twt_tx_ops;
}

struct wlan_lmac_if_twt_rx_ops *
wlan_twt_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	if (!psoc) {
		twt_err("psoc is null");
		return NULL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		twt_err("rx_ops is NULL");
		return NULL;
	}

	return &rx_ops->twt_rx_ops;
}

struct twt_psoc_priv_obj*
wlan_twt_psoc_get_comp_private_obj(struct wlan_objmgr_psoc *psoc)
{
	struct twt_psoc_priv_obj *twt_psoc;

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("TWT PSOC component object is NULL");
		return NULL;
	}

	return twt_psoc;
}

QDF_STATUS wlan_twt_init(void)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_register_psoc_create_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_psoc_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("Failed to register psoc create handler");
		goto wlan_twt_psoc_init_fail1;
	}

	status = wlan_objmgr_register_psoc_destroy_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_psoc_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("Failed to register psoc destroy handler");
		goto wlan_twt_psoc_init_fail2;
	}

	status = wlan_objmgr_register_vdev_create_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_vdev_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("Failed to register vdev create handler");
		goto wlan_twt_vdev_init_fail1;
	}

	status = wlan_objmgr_register_vdev_destroy_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_vdev_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("Failed to register vdev destroy handler");
		goto wlan_twt_vdev_init_fail2;
	}

	status = wlan_objmgr_register_peer_create_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_peer_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("Failed to register peer create handler");
		goto wlan_twt_peer_init_fail1;
	}

	status = wlan_objmgr_register_peer_destroy_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_peer_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		twt_err("Failed to register peer destroy handler");
		goto wlan_twt_peer_init_fail2;
	}

	return QDF_STATUS_SUCCESS;

wlan_twt_peer_init_fail2:
	wlan_objmgr_unregister_peer_create_handler
		(WLAN_UMAC_COMP_TWT,
		 wlan_twt_peer_obj_create_handler,
		 NULL);
wlan_twt_peer_init_fail1:
	wlan_objmgr_unregister_vdev_destroy_handler
		(WLAN_UMAC_COMP_TWT,
		wlan_twt_vdev_obj_destroy_handler,
		NULL);
wlan_twt_vdev_init_fail2:
	wlan_objmgr_unregister_vdev_create_handler
		(WLAN_UMAC_COMP_TWT,
		wlan_twt_vdev_obj_create_handler,
		NULL);
wlan_twt_vdev_init_fail1:
	wlan_objmgr_unregister_psoc_destroy_handler
		(WLAN_UMAC_COMP_TWT,
		 wlan_twt_psoc_obj_destroy_handler,
		 NULL);
wlan_twt_psoc_init_fail2:
	wlan_objmgr_unregister_psoc_create_handler
		(WLAN_UMAC_COMP_TWT,
		 wlan_twt_psoc_obj_create_handler,
		 NULL);
wlan_twt_psoc_init_fail1:
	return status;
}

QDF_STATUS wlan_twt_deinit(void)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	status = wlan_objmgr_unregister_psoc_create_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_psoc_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to unregister psoc create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_psoc_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to unregister psoc destroy handler");

	status = wlan_objmgr_unregister_vdev_create_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_vdev_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_vdev_destroy_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_vdev_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to unregister vdev destroy handler");

	status = wlan_objmgr_unregister_peer_create_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_peer_obj_create_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to unregister peer create handler");

	status = wlan_objmgr_unregister_peer_destroy_handler
				(WLAN_UMAC_COMP_TWT,
				 wlan_twt_peer_obj_destroy_handler,
				 NULL);
	if (QDF_IS_STATUS_ERROR(status))
		twt_err("Failed to unregister peer destroy handler");

	return status;
}

QDF_STATUS twt_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_E_NULL_VALUE;
	struct wlan_lmac_if_twt_tx_ops *tx_ops;

	tx_ops = wlan_twt_get_tx_ops(psoc);
	if (!tx_ops) {
		twt_err("tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (tx_ops->register_events) {
		status = tx_ops->register_events(psoc);

		if (QDF_IS_STATUS_ERROR(status))
			twt_err("twt_register_events failed (status=%d)",
				status);
	}

	return status;
}

QDF_STATUS twt_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status = QDF_STATUS_E_NULL_VALUE;
	struct wlan_lmac_if_twt_tx_ops *tx_ops;

	tx_ops = wlan_twt_get_tx_ops(psoc);
	if (!tx_ops) {
		twt_err("tx_ops is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (tx_ops->deregister_events) {
		status = tx_ops->deregister_events(psoc);

		if (QDF_IS_STATUS_ERROR(status))
			twt_err("twt_deregister_events failed (status=%d)",
				status);
	}
	return status;
}

QDF_STATUS
wlan_set_peer_twt_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t peer_cap)
{
	return wlan_twt_set_peer_capabilities(psoc, peer_mac, peer_cap);
}

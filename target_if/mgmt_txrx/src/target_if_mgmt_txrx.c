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
 *  DOC: target_if_mgmt_txrx.c
 *  This file contains mgmt txrx module's target related function definitions
 */

#include <target_if_mgmt_txrx.h>
#include <target_if_mgmt_txrx_rx_reo.h>

/**
 * target_if_mgmt_txrx_unregister_event_handler() - Unregister event handlers
 * for mgmt txrx module
 * @psoc: pointer to psoc
 *
 * Unregister event handlers for management rx-reorder module
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
target_if_mgmt_txrx_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	if (!psoc) {
		mgmt_txrx_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	status = target_if_mgmt_rx_reo_unregister_event_handlers(psoc);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_txrx_err("Failed to unregister mgmt rx reo events");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mgmt_txrx_register_event_handler() - Register event handlers for
 * mgmt txrx module
 * @psoc: pointer to psoc
 *
 * Register event handlers for management rx-reorder module
 *
 * return: QDF_STATUS
 */
static QDF_STATUS
target_if_mgmt_txrx_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	status = target_if_mgmt_rx_reo_register_event_handlers(psoc);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_txrx_err("Failed to register mgmt rx reo events");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_mgmt_txrx_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_mgmt_txrx_tx_ops *mgmt_txrx_tx_ops;
	QDF_STATUS status;

	if (!tx_ops) {
		mgmt_txrx_err("txops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mgmt_txrx_tx_ops = &tx_ops->mgmt_txrx_tx_ops;
	mgmt_txrx_tx_ops->reg_ev_handler =
		target_if_mgmt_txrx_register_event_handler;
	mgmt_txrx_tx_ops->unreg_ev_handler =
		target_if_mgmt_txrx_unregister_event_handler;

	status = target_if_mgmt_rx_reo_tx_ops_register(mgmt_txrx_tx_ops);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_txrx_err("Failed to register mgmt Rx REO tx ops");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

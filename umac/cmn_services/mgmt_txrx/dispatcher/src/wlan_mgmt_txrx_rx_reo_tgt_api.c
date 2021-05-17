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
 *  DOC: wlan_mgmt_txrx_rx_re_tgt_api.c
 *  This file contains mgmt rx re-ordering tgt layer related function
 *  definitions
 */
#include <wlan_mgmt_txrx_rx_reo_tgt_api.h>
#include <wlan_lmac_if_def.h>

/**
 * wlan_pdev_get_mgmt_rx_reo_txops() - Get management rx-reorder txops from pdev
 * @pdev: Pointer to pdev object
 *
 * Read management rx-reorder snapshots from target.
 *
 * Return: Pointer to management rx-reorder txops
 */
static struct wlan_lmac_if_mgmt_rx_reo_tx_ops *
wlan_pdev_get_mgmt_rx_reo_txops(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is NULL");
		return NULL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mgmt_rx_reo_err("psoc is NULL");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		mgmt_rx_reo_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->mgmt_txrx_tx_ops.mgmt_rx_reo_tx_ops;
}

QDF_STATUS
tgt_mgmt_rx_reo_read_snapshot(struct wlan_objmgr_pdev *pdev,
			      struct mgmt_rx_reo_snapshot *address,
			      enum mgmt_rx_reo_snapshot_id id,
			      struct mgmt_rx_reo_snapshot_params *value)
{
	struct wlan_lmac_if_mgmt_rx_reo_tx_ops *mgmt_rx_reo_txops;

	mgmt_rx_reo_txops = wlan_pdev_get_mgmt_rx_reo_txops(pdev);
	if (!mgmt_rx_reo_txops) {
		mgmt_rx_reo_err("mgmt rx reo txops is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!mgmt_rx_reo_txops->read_mgmt_rx_reo_snapshot) {
		mgmt_rx_reo_err("mgmt rx reo read snapshot txops is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return mgmt_rx_reo_txops->read_mgmt_rx_reo_snapshot(address, id, value);
}

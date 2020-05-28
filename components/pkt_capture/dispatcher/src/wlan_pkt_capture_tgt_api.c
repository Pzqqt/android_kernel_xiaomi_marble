/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: This file contains packet capture south bound interface definitions
 */

#include "wlan_pkt_capture_tgt_api.h"

QDF_STATUS
tgt_pkt_capture_register_ev_handler(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct pkt_capture_vdev_priv *vdev_priv;
	struct wlan_pkt_capture_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pkt_capture_err("psoc is NULL");
		return status;
	}

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (!vdev_priv) {
		pkt_capture_err("vdev priv is NULL");
		return status;
	}

	rx_ops = &vdev_priv->rx_ops;

	if (!rx_ops->pkt_capture_register_ev_handlers)
		return status;

	status = rx_ops->pkt_capture_register_ev_handlers(psoc);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Unable to register mgmt offload handler");

	return status;
}

QDF_STATUS
tgt_pkt_capture_unregister_ev_handler(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct pkt_capture_vdev_priv *vdev_priv;
	struct wlan_pkt_capture_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pkt_capture_err("psoc is NULL");
		return status;
	}

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (!vdev_priv) {
		pkt_capture_err("vdev priv is NULL");
		return status;
	}

	rx_ops = &vdev_priv->rx_ops;

	if (!rx_ops->pkt_capture_unregister_ev_handlers)
		return status;

	status = rx_ops->pkt_capture_unregister_ev_handlers(psoc);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Unable to register mgmt offload handler");

	return status;
}

QDF_STATUS
tgt_pkt_capture_send_mode(struct wlan_objmgr_vdev *vdev,
			  enum pkt_capture_mode mode)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct pkt_capture_vdev_priv *vdev_priv;
	struct wlan_pkt_capture_tx_ops *tx_ops;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		pkt_capture_err("psoc is NULL");
		return status;
	}

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (!vdev_priv) {
		pkt_capture_err("vdev priv is NULL");
		return status;
	}

	tx_ops = &vdev_priv->tx_ops;

	if (!tx_ops->pkt_capture_send_mode)
		return status;

	status = tx_ops->pkt_capture_send_mode(psoc, wlan_vdev_get_id(vdev),
					       mode);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Unable to send packet capture mode to fw");

	return status;
}

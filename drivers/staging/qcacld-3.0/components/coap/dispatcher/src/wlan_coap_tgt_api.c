/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains CoAP south bound interface definitions
 */

#include <wlan_coap_main.h>
#include <wlan_coap_tgt_api.h>
#include <wlan_lmac_if_def.h>
#include "wlan_objmgr_pdev_obj.h"

static inline struct wlan_lmac_if_coap_tx_ops *
wlan_psoc_get_coap_txops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		coap_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->coap_ops;
}

static inline struct wlan_lmac_if_coap_tx_ops *
wlan_vdev_get_coap_txops(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		coap_err("NULL psoc");
		return NULL;
	}

	return wlan_psoc_get_coap_txops(psoc);
}

QDF_STATUS tgt_coap_attach(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_coap_tx_ops *coap_tx_ops;

	if (!psoc) {
		coap_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_tx_ops = wlan_psoc_get_coap_txops(psoc);
	if (!coap_tx_ops) {
		coap_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!coap_tx_ops->attach) {
		coap_err("attach function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return coap_tx_ops->attach(psoc);
}

QDF_STATUS tgt_coap_detach(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_coap_tx_ops *coap_tx_ops;

	if (!psoc) {
		coap_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_tx_ops = wlan_psoc_get_coap_txops(psoc);
	if (!coap_tx_ops) {
		coap_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!coap_tx_ops->detach) {
		coap_err("coap_detach function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return coap_tx_ops->detach(psoc);
}

QDF_STATUS
tgt_send_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
				   struct coap_offload_reply_param *param)
{
	struct wlan_lmac_if_coap_tx_ops *coap_ops;

	if (!vdev) {
		coap_err("NULL vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_ops = wlan_vdev_get_coap_txops(vdev);
	if (coap_ops && coap_ops->offload_reply_enable)
		return coap_ops->offload_reply_enable(vdev, param);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_send_coap_offload_reply_disable(struct wlan_objmgr_vdev *vdev,
				    uint32_t req_id)
{
	struct wlan_lmac_if_coap_tx_ops *coap_ops;

	if (!vdev) {
		coap_err("NULL vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_ops = wlan_vdev_get_coap_txops(vdev);
	if (coap_ops && coap_ops->offload_reply_disable)
		return coap_ops->offload_reply_disable(vdev, req_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_send_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
			struct coap_offload_periodic_tx_param *param)
{
	struct wlan_lmac_if_coap_tx_ops *coap_ops;

	if (!vdev) {
		coap_err("NULL vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_ops = wlan_vdev_get_coap_txops(vdev);
	if (coap_ops && coap_ops->offload_periodic_tx_enable)
		return coap_ops->offload_periodic_tx_enable(vdev, param);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_send_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
					  uint32_t req_id)
{
	struct wlan_lmac_if_coap_tx_ops *coap_ops;

	if (!vdev) {
		coap_err("NULL vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_ops = wlan_vdev_get_coap_txops(vdev);
	if (coap_ops && coap_ops->offload_periodic_tx_disable)
		return coap_ops->offload_periodic_tx_disable(vdev, req_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_send_coap_offload_cache_get(struct wlan_objmgr_vdev *vdev, uint32_t req_id)
{
	struct wlan_lmac_if_coap_tx_ops *coap_ops;

	if (!vdev) {
		coap_err("NULL vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coap_ops = wlan_vdev_get_coap_txops(vdev);
	if (coap_ops && coap_ops->offload_cache_get)
		return coap_ops->offload_cache_get(vdev, req_id);

	return QDF_STATUS_SUCCESS;
}

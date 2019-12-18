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

/*
 * DOC: contains coex south bound interface definitions
 */

#include <wlan_coex_main.h>
#include <wlan_coex_tgt_api.h>
#include <wlan_lmac_if_def.h>
#include "wlan_objmgr_pdev_obj.h"

static inline struct wlan_lmac_if_coex_tx_ops *
wlan_psoc_get_coex_txops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.tx_ops.coex_ops;
}

static inline struct wlan_lmac_if_coex_tx_ops *
wlan_vdev_get_coex_txops(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		coex_err("NULL psoc");
		return NULL;
	}

	return wlan_psoc_get_coex_txops(psoc);
}

QDF_STATUS
tgt_send_coex_config(struct wlan_objmgr_vdev *vdev,
		     struct coex_config_params *param)
{
	struct wlan_lmac_if_coex_tx_ops *coex_ops;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;

	if (!vdev) {
		coex_err("NULL vdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		coex_err("NULL psoc");
		return QDF_STATUS_E_NULL_VALUE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		coex_err("NULL pdev");
		return QDF_STATUS_E_NULL_VALUE;
	}

	coex_ops = wlan_psoc_get_coex_txops(psoc);
	QDF_ASSERT(coex_ops->coex_config_send);
	if (coex_ops->coex_config_send)
		return coex_ops->coex_config_send(pdev, param);

	return QDF_STATUS_SUCCESS;
}

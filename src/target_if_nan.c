/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: contains nan target if functions
 */

#include "wlan_nan_api.h"
#include "nan_public_structs.h"
#include "target_if_nan.h"

static QDF_STATUS target_if_nan_req(void *in_req, uint32_t req_type)
{
	return QDF_STATUS_SUCCESS;
}

void target_if_nan_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->nan_tx_ops.nan_req_tx = target_if_nan_req;
}

void target_if_nan_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	/* TBD */
	rx_ops->nan_rx_ops.nan_event_rx = NULL;
}

inline struct wlan_lmac_if_nan_tx_ops *target_if_nan_get_tx_ops(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	return &psoc->soc_cb.tx_ops.nan_tx_ops;
}

inline struct wlan_lmac_if_nan_rx_ops *target_if_nan_get_rx_ops(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	return &psoc->soc_cb.rx_ops.nan_rx_ops;
}

QDF_STATUS target_if_nan_register_events(struct wlan_objmgr_psoc *psoc)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_nan_deregister_events(struct wlan_objmgr_psoc *psoc)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

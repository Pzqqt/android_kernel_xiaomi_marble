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
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_twt_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS twt_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS twt_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}


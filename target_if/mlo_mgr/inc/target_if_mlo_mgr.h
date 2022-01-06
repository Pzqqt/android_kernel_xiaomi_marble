/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: target_if_mlo_mgr.h
 *
 * This header file provide declarations required for Rx and Tx events from
 * firmware
 */

#ifndef __TARGET_IF_MLO_MGR_H__
#define __TARGET_IF_MLO_MGR_H__

#include <target_if.h>
#include <wlan_lmac_if_def.h>

/**
 * target_if_mlo_get_rx_ops() - get rx ops
 * @tx_ops: pointer to target_if tx ops
 *
 * API to retrieve the MLO rx ops from the psoc context
 *
 * Return: pointer to rx ops
 */
static inline struct wlan_lmac_if_mlo_rx_ops *
target_if_mlo_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		target_if_err("rx_ops is NULL");
		return NULL;
	}

	return &rx_ops->mlo_rx_ops;
}

/**
 * target_if_mlo_get_tx_ops() - get tx ops
 * @tx_ops: pointer to target_if tx ops
 *
 * API to retrieve the MLO tx ops from the psoc context
 *
 * Return: pointer to tx ops
 */
static inline struct wlan_lmac_if_mlo_tx_ops *
target_if_mlo_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		target_if_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->mlo_ops;
}

/**
 * target_if_mlo_register_tx_ops() - lmac handler to register mlo tx ops
 *  callback functions
 * @tx_ops: wlan_lmac_if_tx_ops object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mlo_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

#endif /* __TARGET_IF_MLO_MGR_H__ */


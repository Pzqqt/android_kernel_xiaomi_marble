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

/**
 * DOC: contains CoAP target if declarations
 */
#ifndef __TARGET_IF_COAP_H__
#define __TARGET_IF_COAP_H__

#include <target_if.h>

/**
 * target_if_coap_register_tx_ops() - Register CoAP target_if tx ops
 * @tx_ops: pointer to target if tx ops
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
target_if_coap_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_coap_get_tx_ops() - get tx ops
 * @tx_ops: pointer to target_if tx ops
 *
 * API to retrieve the CoAP tx ops from the psoc context
 *
 * Return: pointer to tx ops
 */
static inline struct wlan_lmac_if_coap_tx_ops *
target_if_coap_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		target_if_err("tx_ops is NULL");
		return NULL;
	}
	return &tx_ops->coap_ops;
}

#endif

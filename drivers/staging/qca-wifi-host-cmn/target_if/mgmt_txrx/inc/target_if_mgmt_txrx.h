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
 *  DOC: target_if_mgmt_txrx.h
 *  This file contains mgmt txrx module's target related APIs
 */

#ifndef _TARGET_IF_MGMT_TXRX_H_
#define _TARGET_IF_MGMT_TXRX_H_

#include <wlan_lmac_if_def.h>

/**
 * target_if_mgmt_txrx_tx_ops_register() - Register txops for mgmt_txrx
 * module.
 * @tx_ops: pointer to txops
 *
 * Register txops for mgmt_txrx module.
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_mgmt_txrx_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops);
#endif /*_TARGET_IF_MGMT_TXRX_H_ */


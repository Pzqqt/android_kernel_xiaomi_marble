/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: declares crypto functions interfacing with the target
 */

#ifndef __TARGET_IF_CRYPTO_H__
#define __TARGET_IF_CRYPTO_H__
#include <wlan_lmac_if_def.h>

/**
 * target_if_crypto_register_tx_ops() - lmac handler to register
 * crypto tx_ops callback functions
 * @tx_ops: wlan_lmac_if_tx_ops object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_crypto_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_crypto_set_key() - lmac handler to set key
 * @vdev: VDEV object pointer
 * @req: Key parameters that are required to install the key
 * @key_type: Pairwise or Group Key type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_crypto_set_key(struct wlan_objmgr_vdev *vdev,
				    struct wlan_crypto_key *req,
				    enum wlan_crypto_key_type key_type);

#endif

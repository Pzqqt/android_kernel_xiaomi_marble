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
 * DOC: offload lmac interface APIs for gpio cfg
 */
#ifndef __TARGET_IF_GPIO_CFG_H__
#define __TARGET_IF_GPIO_CFG_H__

#ifdef WLAN_FEATURE_GPIO_CFG
#include <qdf_status.h>
struct wlan_lmac_if_tx_ops;

/**
 * target_if_gpio_register_tx_ops() - register tx ops funcs
 * @tx_ops: pointer to gpio tx ops
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
target_if_gpio_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

#endif /* WLAN_FEATURE_GPIO_CFG */
#endif /* __TARGET_IF_GPIO_CFG_H__ */

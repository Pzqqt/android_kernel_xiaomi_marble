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
 * DOC: wlan_twt_tgt_if_ext_rx_ops.h
 */
#ifndef _WLAN_TWT_TGT_IF_EXT_RX_OPS_H_
#define _WLAN_TWT_TGT_IF_EXT_RX_OPS_H_

#include <wlan_lmac_if_def.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * tgt_twt_register_ext_rx_ops() - API to register rx ops with lmac
 * @rx_ops: rx ops struct
 *
 * Return: none
 */
void tgt_twt_register_ext_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);
#else
static inline
void tgt_twt_register_ext_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
}
#endif

#endif

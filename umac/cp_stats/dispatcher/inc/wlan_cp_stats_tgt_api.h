/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_tgt_api.h
 *
 * This header file provide with API declarations to interface with Southbound
 */
#ifndef __WLAN_CP_STATS_TGT_API_H__
#define __WLAN_CP_STATS_TGT_API_H__
#include <wlan_lmac_if_def.h>
#include <wlan_objmgr_cmn.h>

#ifdef QCA_SUPPORT_CP_STATS
/**
 * tgt_cp_stats_register_rx_ops(): API to register rx ops with lmac
 * @rx_ops: rx ops struct
 *
 * Return: none
 */
void tgt_cp_stats_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);
#else
static inline void tgt_cp_stats_register_rx_ops(
					struct wlan_lmac_if_rx_ops *rx_ops) {}
#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_TGT_API_H__ */

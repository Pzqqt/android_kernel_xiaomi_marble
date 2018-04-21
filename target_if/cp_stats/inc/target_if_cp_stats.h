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
 * DOC: target_if_cp_stats.h
 *
 * This header file provide declarations required for Rx and Tx events from
 * firmware
 */

#ifndef __TARGET_IF_CP_STATS_H__
#define __TARGET_IF_CP_STATS_H__

#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_cp_stats_utils_api.h>

#ifdef QCA_SUPPORT_CP_STATS

/**
 * target_if_cp_stats_get_rx_ops() - get rx ops
 * @tx_ops: pointer to lmac tx ops
 *
 * Return: pointer to rx ops
 */
static inline struct wlan_lmac_if_cp_stats_rx_ops *
target_if_cp_stats_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.cp_stats_rx_ops;
}

/**
 * target_if_cp_stats_get_tx_ops() - get tx ops
 * @tx_ops: pointer to lmac tx ops
 *
 * Return: pointer to tx ops
 */
static inline struct wlan_lmac_if_cp_stats_tx_ops *
target_if_cp_stats_get_tx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.tx_ops.cp_stats_tx_ops;
}

/**
 * target_if_cp_stats_register_tx_ops() - define cp_stats lmac tx ops functions
 * @tx_ops: pointer to lmac tx ops
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS
target_if_cp_stats_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);
#else
static inline QDF_STATUS
target_if_cp_stats_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* QCA_SUPPORT_CP_STATS */

#endif /* __TARGET_IF_CP_STATS_H__ */

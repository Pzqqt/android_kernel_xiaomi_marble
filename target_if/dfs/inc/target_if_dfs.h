/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 *
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
 * DOC: target_if_dfs.h
 * This file contains dfs target interface
 */

/**
 * target_if_register_dfs_tx_ops() - register dfs tx ops
 * @dfs_tx_ops: tx ops pointer
 *
 * Register dfs tx ops
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_register_dfs_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_dfs_get_rx_ops() - Get dfs_rx_ops
 * @psoc: psoc handle.
 *
 * Return: dfs_rx_ops.
 */
static inline struct wlan_lmac_if_dfs_rx_ops *
target_if_dfs_get_rx_ops(struct wlan_objmgr_psoc *psoc)
{
	return &psoc->soc_cb.rx_ops.dfs_rx_ops;
}


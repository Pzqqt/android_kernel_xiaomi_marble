/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __WLAN_INIT_CFG_H
#define __WLAN_INIT_CFG_H

/*
 * Temporary place holders. These should come either from target config
 * or platform configuration
 */
#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
#define WLAN_CFG_DST_RING_CACHED_DESC 0
#define MAX_PDEV_CNT 1

#ifdef CONFIG_BERYLLIUM
/* (8 REOs + 5 WBMs + 2 Near Full + 1 Error handling) */
#define WLAN_CFG_INT_NUM_CONTEXTS 18
#define WLAN_CFG_INT_NUM_CONTEXTS_MAX 18
#else
#define WLAN_CFG_INT_NUM_CONTEXTS 7
#define WLAN_CFG_INT_NUM_CONTEXTS_MAX 14
#endif
#define WLAN_CFG_RXDMA1_ENABLE 1
/*
 * This mask defines how many transmit frames account for 1 NAPI work unit
 * 0 means each tx completion is 1 unit
 */
#define DP_TX_NAPI_BUDGET_DIV_MASK 0

/* PPDU Stats Configuration - Configure bitmask for enabling tx ppdu tlv's */
#define DP_PPDU_TXLITE_STATS_BITMASK_CFG 0x3FFF

#define NUM_RXDMA_RINGS_PER_PDEV 2

#define DP_NUM_MACS_PER_PDEV 2

/*Maximum Number of LMAC instances*/
#define MAX_NUM_LMAC_HW	2
#else
#ifdef DP_NO_CACHE_DESC_SUPPORT
#define WLAN_CFG_DST_RING_CACHED_DESC 0
#else
#define WLAN_CFG_DST_RING_CACHED_DESC 1
#endif
#define MAX_PDEV_CNT 3
#define WLAN_CFG_INT_NUM_CONTEXTS 11
#define WLAN_CFG_INT_NUM_CONTEXTS_MAX 11
#define NUM_RXDMA_RINGS_PER_PDEV 1
#define MAX_NUM_LMAC_HW	3
#define DP_NUM_MACS_PER_PDEV 1

#endif

/* Tx configuration */
#define MAX_LINK_DESC_BANKS 8
#define MAX_TXDESC_POOLS 6

/* Rx configuration */
#define MAX_RXDESC_POOLS 4

#define MAX_REO_DEST_RINGS 8
#define MAX_TCL_DATA_RINGS 5

#define MAX_RX_MAC_RINGS 2

#endif /* __WLAN_INIT_CFG_H */

/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef __WLAN_HDD_LRO_H__
#define __WLAN_HDD_LRO_H__
/**
 * DOC: wlan_hdd_lro.h
 *
 * WLAN LRO interface module headers
 */

/**
 * enum hdd_lro_rx_status - LRO receive frame status
 * @HDD_LRO_RX: frame sent over the LRO interface
 * @HDD_LRO_NO_RX: frame not sent over the LRO interface
 */
enum hdd_lro_rx_status {
	HDD_LRO_RX = 0,
	HDD_LRO_NO_RX = 1,
};

#if defined(FEATURE_LRO)

#include <linux/inet_lro.h>
#include <linux/list.h>

/* LRO_DESC_TABLE_SZ must be a power of 2 */
#define LRO_DESC_TABLE_SZ 16
#define LRO_DESC_TABLE_SZ_MASK (LRO_DESC_TABLE_SZ - 1)
#define LRO_DESC_POOL_SZ 10

/**
 * hdd_lro_desc_entry - defines the LRO descriptor
 * element stored in the list
 * @lro_node: node of the list
 * @lro_desc: the LRO descriptor contained in this list entry
 */
struct hdd_lro_desc_entry {
	struct list_head lro_node;
	struct net_lro_desc *lro_desc;
};

/**
 * hdd_lro_desc_pool - pool of free LRO descriptors
 * @lro_desc_array: array of LRO descriptors allocated
 * @lro_free_list_head: head of the list
 * @lro_pool_lock: lock to protect access to the list
 */
struct hdd_lro_desc_pool {
	struct hdd_lro_desc_entry *lro_desc_array;
	struct list_head lro_free_list_head;
	qdf_spinlock_t lro_pool_lock;
};

/**
 * hdd_lro_desc_table - defines each entry of the LRO
 * hash table
 * @lro_desc_list: list of LRO descriptors
 */
struct hdd_lro_desc_table {
	struct list_head lro_desc_list;
};

/**
 * hdd_lro_desc_info - structure containing the LRO
 * descriptor information
 * @lro_hash_table: hash table used for a quick desc. look-up
 * @lro_hash_lock: lock to protect access to the hash table
 * @lro_desc_pool: Free pool of LRO descriptors
 */
struct hdd_lro_desc_info {
	struct hdd_lro_desc_table *lro_hash_table;
	qdf_spinlock_t lro_hash_lock;
	struct hdd_lro_desc_pool lro_desc_pool;
};

/**
 * enum hdd_lro_pkt_aggr_bucket - idenitifies the bucket holding
 * the count of the aggregated packets
 * @HDD_LRO_BUCKET_0_7: identifies the packet count when the
 * aggregate size is between 0 to 7 packets
 * @HDD_LRO_BUCKET_8_15: identifies the packet count when the
 * aggregate size is between 8 to 15 packets
 * @HDD_LRO_BUCKET_16_23: identifies the packet count when the
 * aggregate size is between 16 to 23 packets
 * @HDD_LRO_BUCKET_24_31: identifies the packet count when the
 * aggregate size is between 24 to 31 packets
 * @HDD_LRO_BUCKET_32_39: identifies the packet count when the
 * aggregate size is between 32 to 39 packets
 * @HDD_LRO_BUCKET_40_47: identifies the packet count when the
 * aggregate size is between 40 to 47 packets
 * @HDD_LRO_BUCKET_48_OR_MORE: identifies the packet count when
 * the aggregate size is 48 or more packets
 * @HDD_LRO_BUCKET_MAX: identifies the packet count when the
 * aggregate size is 48 or more packets
 */
enum hdd_lro_pkt_aggr_bucket {
	HDD_LRO_BUCKET_0_7 = 0,
	HDD_LRO_BUCKET_8_15 = 1,
	HDD_LRO_BUCKET_16_23 = 2,
	HDD_LRO_BUCKET_24_31 = 3,
	HDD_LRO_BUCKET_32_39 = 4,
	HDD_LRO_BUCKET_40_47 = 5,
	HDD_LRO_BUCKET_48_OR_MORE = 6,
	HDD_LRO_BUCKET_MAX = HDD_LRO_BUCKET_48_OR_MORE,
};

/**
 * hdd_lro_stats - structure containing the LRO statistics
 * information
 * @pkt_aggr_hist: histogram of the number of aggregated packets
 * @lro_eligible_tcp: number of LRO elgible TCP packets
 * @lro_ineligible_tcp: number of LRO inelgible TCP packets
 */
struct hdd_lro_stats {
	uint16_t pkt_aggr_hist[HDD_LRO_BUCKET_MAX + 1];
	uint32_t lro_eligible_tcp;
	uint32_t lro_ineligible_tcp;
};

/**
 * hdd_lro_s - LRO information per HDD adapter
 * @lro_mgr: LRO manager
 * @lro_desc_info: LRO descriptor information
 * @lro_mgr_arr_access_lock: Lock to access LRO manager array.
 * @lro_stats: LRO statistics
 */
struct hdd_lro_s {
	struct net_lro_mgr *lro_mgr;
	struct hdd_lro_desc_info lro_desc_info;
	qdf_spinlock_t lro_mgr_arr_access_lock;
	struct hdd_lro_stats lro_stats;
};

int hdd_lro_init(hdd_context_t *hdd_ctx);

int hdd_lro_enable(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter);

void hdd_lro_disable(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter);

enum hdd_lro_rx_status hdd_lro_rx(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter, struct sk_buff *skb);

void hdd_lro_flush_all(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter);

void hdd_lro_display_stats(hdd_context_t *hdd_ctx);
#else
struct hdd_lro_s {};

static inline int hdd_lro_enable(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter)
{
	return 0;
}

static inline enum hdd_lro_rx_status hdd_lro_rx(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter, struct sk_buff *skb)
{
	return HDD_LRO_NO_RX;
}

static inline int hdd_lro_init(hdd_context_t *hdd_ctx)
{
	return 0;
}

static inline void hdd_lro_disable(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter)
{
	return;
}

static inline void hdd_lro_display_stats(hdd_context_t *hdd_ctx)
{
	return;
}
#endif /* FEATURE_LRO */
#endif /* __WLAN_HDD_LRO_H__ */

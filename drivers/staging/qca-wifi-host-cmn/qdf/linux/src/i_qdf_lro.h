/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_lro.h
 * This file provides OS dependent LRO API's.
 */

#ifndef _I_QDF_LRO_H
#define _I_QDF_LRO_H

#if defined(FEATURE_LRO)
#include <qdf_types.h>
#include <i_qdf_nbuf.h>
#include <i_qdf_trace.h>

#include <linux/inet_lro.h>

/**
 * qdf_lro_desc_entry - defines the LRO descriptor
 * element stored in the list
 * @lro_node: node of the list
 * @lro_desc: the LRO descriptor contained in this list entry
 */
struct qdf_lro_desc_entry {
	struct list_head lro_node;
	struct net_lro_desc *lro_desc;
};

/**
 * qdf_lro_desc_pool - pool of free LRO descriptors
 * @lro_desc_array: array of LRO descriptors allocated
 * @lro_free_list_head: head of the list
 * @lro_pool_lock: lock to protect access to the list
 */
struct qdf_lro_desc_pool {
	struct qdf_lro_desc_entry *lro_desc_array;
	struct list_head lro_free_list_head;
};

/**
 * qdf_lro_desc_table - defines each entry of the LRO hash table
 * @lro_desc_list: list of LRO descriptors
 */
struct qdf_lro_desc_table {
	struct list_head lro_desc_list;
};

/**
 * qdf_lro_desc_info - structure containing the LRO descriptor
 * information
 * @lro_hash_table: hash table used for a quick desc. look-up
 * @lro_hash_lock: lock to protect access to the hash table
 * @lro_desc_pool: Free pool of LRO descriptors
 */
struct qdf_lro_desc_info {
	struct qdf_lro_desc_table *lro_hash_table;
	struct qdf_lro_desc_pool lro_desc_pool;
};

/**
 * qdf_lro_info_s - LRO information
 * @lro_mgr: LRO manager
 * @lro_desc_info: LRO descriptor information
 * @lro_mgr_arr_access_lock: Lock to access LRO manager array.
 * @lro_stats: LRO statistics
 */
struct qdf_lro_s {
	struct net_lro_mgr *lro_mgr;
	struct qdf_lro_desc_info lro_desc_info;
};

typedef struct qdf_lro_s *__qdf_lro_ctx_t;

/* LRO_DESC_TABLE_SZ must be a power of 2 */
#define QDF_LRO_DESC_TABLE_SZ 16
#define QDF_LRO_DESC_TABLE_SZ_MASK (QDF_LRO_DESC_TABLE_SZ - 1)
#define QDF_LRO_DESC_POOL_SZ 10

#define QDF_LRO_DESC_TABLE_SZ 16
#define QDF_LRO_DESC_TABLE_SZ_MASK (QDF_LRO_DESC_TABLE_SZ - 1)
#define QDF_LRO_DESC_POOL_SZ 10

#define QDF_LRO_MAX_AGGR_SIZE 100

#else

struct qdf_lro_s {};

typedef struct qdf_lro_s *__qdf_lro_ctx_t;

#endif /* FEATURE_LRO */
#endif /*_I_QDF_NET_BUF_H */

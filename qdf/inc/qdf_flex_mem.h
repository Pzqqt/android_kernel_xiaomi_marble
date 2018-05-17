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
 * DOC: qdf_flex_mem (flexibly sized memory allocator)
 * QCA driver framework (QDF) flex mem APIs
 *
 * A flex memory allocator is a memory pool which not only dynamically expands,
 * but also dynamically reduces as well. Benefits over full dynamic memory
 * allocation are amoritized allocation cost, and reduced memory fragmentation.
 *
 * The allocator consists of 3 parts: the pool, segments, and items. Items are
 * the smallest chuncks of memory that are handed out via the alloc call, and
 * are all of a uniform size. Segments are groups of items, representing the
 * smallest amount of memory that can be dynamically allocated or freed. A pool
 * is simply a collection of segments.
 */

#ifndef __QDF_FLEX_MEM_H
#define __QDF_FLEX_MEM_H

#include "qdf_list.h"
#include "qdf_lock.h"

#define QDF_FM_BITMAP uint32_t
#define QDF_FM_BITMAP_BITS (sizeof(QDF_FM_BITMAP) * 8)

/**
 * qdf_flex_mem_pool - a pool of memory segments
 * @seg_list: the list containing the memory segments
 * @lock: spinlock for protecting internal data structures
 * @reduction_limit: the minimum number of segments to keep during reduction
 * @item_size: the size of the items the pool will allocate
 */
struct qdf_flex_mem_pool {
	qdf_list_t seg_list;
	struct qdf_spinlock lock;
	uint16_t reduction_limit;
	uint16_t item_size;
};

/**
 * qdf_flex_mem_segment - a memory pool segment
 * @node: the list node for membership in the memory pool
 * @dynamic: true if this segment was dynamically allocated
 * @used_bitmap: bitmap for tracking which items in the segment are in use
 * @bytes: raw memory for allocating items from
 */
struct qdf_flex_mem_segment {
	qdf_list_node_t node;
	bool dynamic;
	QDF_FM_BITMAP used_bitmap;
	uint8_t *bytes;
};

/**
 * DEFINE_QDF_FLEX_MEM_POOL() - define a new flex mem pool with one segment
 * @name: the name of the pool variable
 * @size_of_item: size of the items the pool will allocate
 * @rm_limit: min number of segments to keep during reduction
 */
#define DEFINE_QDF_FLEX_MEM_POOL(name, size_of_item, rm_limit) \
	struct qdf_flex_mem_pool name; \
	uint8_t __ ## name ## _head_bytes[QDF_FM_BITMAP_BITS * (size_of_item)];\
	struct qdf_flex_mem_segment __ ## name ## _head = { \
		.node = QDF_LIST_NODE_INIT_SINGLE( \
			QDF_LIST_ANCHOR(name.seg_list)), \
		.bytes = __ ## name ## _head_bytes, \
	}; \
	struct qdf_flex_mem_pool name = { \
		.seg_list = QDF_LIST_INIT_SINGLE(__ ## name ## _head.node), \
		.reduction_limit = (rm_limit), \
		.item_size = (size_of_item), \
	}

/**
 * qdf_flex_mem_init() - initialize a qdf_flex_mem_pool
 * @pool: the pool to initialize
 *
 * Return: None
 */
void qdf_flex_mem_init(struct qdf_flex_mem_pool *pool);

/**
 * qdf_flex_mem_deinit() - deinitialize a qdf_flex_mem_pool
 * @pool: the pool to deinitialize
 *
 * Return: None
 */
void qdf_flex_mem_deinit(struct qdf_flex_mem_pool *pool);

/**
 * qdf_flex_mem_alloc() - logically allocate memory from the pool
 * @pool: the pool to allocate from
 *
 * This function returns any unused item from any existing segment in the pool.
 * If there are no unused items in the pool, a new segment is dynamically
 * allocated to service the request. The size of the allocated memory is the
 * size originally used to create the pool.
 *
 * Return: Point to newly allocated memory, NULL on failure
 */
void *qdf_flex_mem_alloc(struct qdf_flex_mem_pool *pool);

/**
 * qdf_flex_mem_free() - logically frees @ptr from the pool
 * @pool: the pool to return the memory to
 * @ptr: a pointer received via a call to qdf_flex_mem_alloc()
 *
 * This function marks the item corresponding to @ptr as unused. If that item
 * was the last used item in the segment it belongs to, and the segment was
 * dynamically allocated, the segment will be freed.
 *
 * Return: None
 */
void qdf_flex_mem_free(struct qdf_flex_mem_pool *pool, void *ptr);

/**
 * qdf_flex_mem_release() - release unused segments
 * @pool: the pool to operate against
 *
 * This function physically releases as much unused pool memory as possible.
 *
 * Return: None
 */
void qdf_flex_mem_release(struct qdf_flex_mem_pool *pool);

#endif /* __QDF_FLEX_MEM_H */

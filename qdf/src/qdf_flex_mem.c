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

#include "qdf_flex_mem.h"
#include "qdf_list.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_trace.h"
#include "qdf_util.h"

void qdf_flex_mem_init(struct qdf_flex_mem_pool *pool)
{
	qdf_spinlock_create(&pool->lock);
}

void qdf_flex_mem_deinit(struct qdf_flex_mem_pool *pool)
{
	qdf_spinlock_destroy(&pool->lock);
}

static struct qdf_flex_mem_segment *qdf_flex_mem_seg_alloc(uint16_t item_size)
{
	size_t bytes_size = item_size * QDF_FM_BITMAP_BITS;
	size_t total_size = sizeof(struct qdf_flex_mem_segment) + bytes_size;
	struct qdf_flex_mem_segment *seg;

	seg = qdf_mem_malloc(total_size);
	if (!seg)
		return NULL;

	seg->dynamic = true;
	seg->bytes = (uint8_t *)(seg + 1);

	return seg;
}

static void *__qdf_flex_mem_alloc(struct qdf_flex_mem_pool *pool)
{
	struct qdf_flex_mem_segment *seg;

	qdf_list_for_each(&pool->seg_list, seg, node) {
		int index;
		void *ptr;

		index = qdf_ffz(seg->used_bitmap);
		if (index < 0)
			continue;

		QDF_BUG(index < QDF_FM_BITMAP_BITS);

		seg->used_bitmap ^= (QDF_FM_BITMAP)1 << index;
		ptr = &seg->bytes[index * pool->item_size];
		qdf_mem_zero(ptr, pool->item_size);

		return ptr;
	}

	seg = qdf_flex_mem_seg_alloc(pool->item_size);
	if (!seg)
		return NULL;

	seg->used_bitmap = 1;
	qdf_list_insert_back(&pool->seg_list, &seg->node);

	return seg->bytes;
}

void *qdf_flex_mem_alloc(struct qdf_flex_mem_pool *pool)
{
	void *ptr;

	QDF_BUG(pool);
	if (!pool)
		return NULL;

	qdf_spin_lock_bh(&pool->lock);
	ptr = __qdf_flex_mem_alloc(pool);
	qdf_spin_unlock_bh(&pool->lock);

	return ptr;
}

static void qdf_flex_mem_seg_free(struct qdf_flex_mem_pool *pool,
				  struct qdf_flex_mem_segment *seg)
{
	qdf_list_remove_node(&pool->seg_list, &seg->node);
	qdf_mem_free(seg);
}

static void __qdf_flex_mem_free(struct qdf_flex_mem_pool *pool, void *ptr)
{
	struct qdf_flex_mem_segment *seg;
	void *low_addr;
	void *high_addr;
	unsigned long index;

	qdf_list_for_each(&pool->seg_list, seg, node) {
		low_addr = seg->bytes;
		high_addr = low_addr + pool->item_size * QDF_FM_BITMAP_BITS;

		if (ptr < low_addr || ptr > high_addr)
			continue;

		index = (ptr - low_addr) / pool->item_size;
		QDF_BUG(index < QDF_FM_BITMAP_BITS);

		seg->used_bitmap ^= (QDF_FM_BITMAP)1 << index;
		if (!seg->used_bitmap && seg->dynamic)
			qdf_flex_mem_seg_free(pool, seg);

		return;
	}

	qdf_err("Failed to find pointer in segment pool");
	QDF_DEBUG_PANIC();
}

void qdf_flex_mem_free(struct qdf_flex_mem_pool *pool, void *ptr)
{
	QDF_BUG(pool);
	if (!pool)
		return;

	QDF_BUG(ptr);
	if (!ptr)
		return;

	qdf_spin_lock_bh(&pool->lock);
	__qdf_flex_mem_free(pool, ptr);
	qdf_spin_unlock_bh(&pool->lock);
}


/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef DP_TX_DESC_H
#define DP_TX_DESC_H

#include "dp_types.h"
#include "dp_tx.h"
#include "dp_internal.h"

/**
 * 21 bits cookie
 * 3 bits ring id 0 ~ 7,      mask 0x1C0000, offset 18
 * 8 bits page id 0 ~ 255,    mask 0x03C800, offset 10
 * 10 bits offset id 0 ~ 1023 mask 0x0003FF, offset 0
 */
/* ???Ring ID needed??? */
#define DP_TX_DESC_ID_POOL_MASK    0x1C0000
#define DP_TX_DESC_ID_POOL_OS      18
#define DP_TX_DESC_ID_PAGE_MASK    0x03FC00
#define DP_TX_DESC_ID_PAGE_OS      10
#define DP_TX_DESC_ID_OFFSET_MASK  0x0003FF
#define DP_TX_DESC_ID_OFFSET_OS    0

#define TX_DESC_LOCK_CREATE(lock)  qdf_spinlock_create(lock)
#define TX_DESC_LOCK_DESTROY(lock) qdf_spinlock_destroy(lock)
#define TX_DESC_LOCK_LOCK(lock)    qdf_spin_lock(lock)
#define TX_DESC_LOCK_UNLOCK(lock)  qdf_spin_unlock(lock)
#define MAX_POOL_BUFF_COUNT 10000

QDF_STATUS dp_tx_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem);
QDF_STATUS dp_tx_desc_pool_free(struct dp_soc *soc, uint8_t pool_id);
QDF_STATUS dp_tx_ext_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem);
QDF_STATUS dp_tx_ext_desc_pool_free(struct dp_soc *soc, uint8_t pool_id);
QDF_STATUS dp_tx_tso_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem);
void dp_tx_tso_desc_pool_free(struct dp_soc *soc, uint8_t pool_id);

/**
 * dp_tx_desc_alloc() - Allocate a Software Tx Descriptor from given pool
 *
 * @param soc Handle to DP SoC structure
 * @param pool_id
 *
 * Return:
 */
static inline struct dp_tx_desc_s *dp_tx_desc_alloc(struct dp_soc *soc,
		uint8_t desc_pool_id)
{
	struct dp_tx_desc_s *tx_desc = NULL;

	TX_DESC_LOCK_LOCK(&soc->tx_desc[desc_pool_id].lock);

	tx_desc = soc->tx_desc[desc_pool_id].freelist;
	/* Pool is exhausted */
	if (!tx_desc) {
		TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);
		return NULL;
	}
	if (soc->tx_desc[desc_pool_id].freelist) {
		soc->tx_desc[desc_pool_id].freelist =
			soc->tx_desc[desc_pool_id].freelist->next;
		soc->tx_desc[desc_pool_id].num_allocated++;
		soc->tx_desc[desc_pool_id].num_free--;
	}

	DP_STATS_INC(soc, tx.desc_in_use, 1);
	tx_desc->flags = DP_TX_DESC_FLAG_ALLOCATED;
	TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);

	return tx_desc;
}

/**
 * dp_tx_desc_alloc_multiple() - Allocate batch of software Tx Descriptors
 *                            from given pool
 * @soc: Handle to DP SoC structure
 * @pool_id: pool id should pick up
 * @num_requested: number of required descriptor
 *
 * allocate multiple tx descriptor and make a link
 *
 * Return: h_desc first descriptor pointer
 */
static inline struct dp_tx_desc_s *dp_tx_desc_alloc_multiple(
		struct dp_soc *soc, uint8_t desc_pool_id, uint8_t num_requested)
{
	struct dp_tx_desc_s *c_desc = NULL, *h_desc = NULL;
	uint8_t count;

	TX_DESC_LOCK_LOCK(&soc->tx_desc[desc_pool_id].lock);

	if ((num_requested == 0) ||
			(soc->tx_desc[desc_pool_id].num_free < num_requested)) {
		TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s, No Free Desc: Available(%d) num_requested(%d)",
			__func__, soc->tx_desc[desc_pool_id].num_free,
			num_requested);
		return NULL;
	}

	h_desc = soc->tx_desc[desc_pool_id].freelist;

	/* h_desc should never be NULL since num_free > requested */
	qdf_assert_always(h_desc);

	c_desc = h_desc;
	for (count = 0; count < (num_requested - 1); count++) {
		c_desc->flags = DP_TX_DESC_FLAG_ALLOCATED;
		c_desc = c_desc->next;
	}
	soc->tx_desc[desc_pool_id].num_free -= count;
	soc->tx_desc[desc_pool_id].num_allocated += count;
	soc->tx_desc[desc_pool_id].freelist = c_desc->next;
	c_desc->next = NULL;

	TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);
	return h_desc;
}

/**
 * dp_tx_desc_free() - Fee a tx descriptor and attach it to free list
 *
 * @soc Handle to DP SoC structure
 * @pool_id
 * @tx_desc
 */
static inline void
dp_tx_desc_free(struct dp_soc *soc, struct dp_tx_desc_s *tx_desc,
		uint8_t desc_pool_id)
{
	TX_DESC_LOCK_LOCK(&soc->tx_desc[desc_pool_id].lock);

	tx_desc->flags = 0;
	tx_desc->next = soc->tx_desc[desc_pool_id].freelist;
	soc->tx_desc[desc_pool_id].freelist = tx_desc;
	DP_STATS_DEC(soc, tx.desc_in_use, 1);
	soc->tx_desc[desc_pool_id].num_allocated--;
	soc->tx_desc[desc_pool_id].num_free++;


	TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);
}

/**
 * dp_tx_desc_find() - find dp tx descriptor from cokie
 * @soc - handle for the device sending the data
 * @tx_desc_id - the ID of the descriptor in question
 * @return the descriptor object that has the specified ID
 *
 *  Use a tx descriptor ID to find the corresponding descriptor object.
 *
 */
static inline struct dp_tx_desc_s *dp_tx_desc_find(struct dp_soc *soc,
		uint8_t pool_id, uint16_t page_id, uint16_t offset)
{
	return soc->tx_desc[pool_id].desc_pages.cacheable_pages[page_id] +
		soc->tx_desc[pool_id].elem_size * offset;
}

/**
 * dp_tx_ext_desc_alloc() - Get tx extension descriptor from pool
 * @soc: handle for the device sending the data
 * @pool_id: target pool id
 *
 * Return: None
 */
static inline
struct dp_tx_ext_desc_elem_s *dp_tx_ext_desc_alloc(struct dp_soc *soc,
		uint8_t desc_pool_id)
{
	struct dp_tx_ext_desc_elem_s *c_elem;

	TX_DESC_LOCK_LOCK(&soc->tx_ext_desc[desc_pool_id].lock);
	c_elem = soc->tx_ext_desc[desc_pool_id].freelist;
	soc->tx_ext_desc[desc_pool_id].freelist =
		soc->tx_ext_desc[desc_pool_id].freelist->next;
	TX_DESC_LOCK_UNLOCK(&soc->tx_ext_desc[desc_pool_id].lock);
	return c_elem;
}

/**
 * dp_tx_ext_desc_free() - Release tx extension descriptor to the pool
 * @soc: handle for the device sending the data
 * @pool_id: target pool id
 * @elem: ext descriptor pointer should release
 *
 * Return: None
 */
static inline void dp_tx_ext_desc_free(struct dp_soc *soc,
	struct dp_tx_ext_desc_elem_s *elem, uint8_t desc_pool_id)
{
	TX_DESC_LOCK_LOCK(&soc->tx_ext_desc[desc_pool_id].lock);
	elem->next = soc->tx_ext_desc[desc_pool_id].freelist;
	soc->tx_ext_desc[desc_pool_id].freelist = elem;
	TX_DESC_LOCK_UNLOCK(&soc->tx_ext_desc[desc_pool_id].lock);
	return;
}

/**
 * dp_tx_ext_desc_free_multiple() - Fee multiple tx extension descriptor and
 *                           attach it to free list
 * @soc: Handle to DP SoC structure
 * @desc_pool_id: pool id should pick up
 * @elem: tx descriptor should be freed
 * @num_free: number of descriptors should be freed
 *
 * Return: none
 */
static inline void dp_tx_ext_desc_free_multiple(struct dp_soc *soc,
		struct dp_tx_ext_desc_elem_s *elem, uint8_t desc_pool_id,
		uint8_t num_free)
{
	struct dp_tx_ext_desc_elem_s *head, *tail, *c_elem;
	uint8_t freed = num_free;

	/* caller should always guarantee atleast list of num_free nodes */
	qdf_assert_always(head);

	head = elem;
	c_elem = head;
	tail = head;
	while (c_elem && freed) {
		tail = c_elem;
		c_elem = c_elem->next;
		freed--;
	}

	/* caller should always guarantee atleast list of num_free nodes */
	qdf_assert_always(tail);

	TX_DESC_LOCK_LOCK(&soc->tx_ext_desc[desc_pool_id].lock);
	tail->next = soc->tx_ext_desc[desc_pool_id].freelist;
	soc->tx_ext_desc[desc_pool_id].freelist = head;
	soc->tx_ext_desc[desc_pool_id].num_free += num_free;
	TX_DESC_LOCK_UNLOCK(&soc->tx_ext_desc[desc_pool_id].lock);

	return;
}

/**
 * dp_tx_tso_desc_alloc() - function to allocate a TSO segment
 * @soc: device soc instance
 * @pool_id: pool id should pick up tso descriptor
 *
 * Allocates a TSO segment element from the free list held in
 * the soc
 *
 * Return: tso_seg, tso segment memory pointer
 */
static inline struct qdf_tso_seg_elem_t *dp_tx_tso_desc_alloc(
		struct dp_soc *soc, uint8_t pool_id)
{
	struct qdf_tso_seg_elem_t *tso_seg = NULL;

	TX_DESC_LOCK_LOCK(&soc->tx_tso_desc[pool_id].lock);
	if (soc->tx_tso_desc[pool_id].freelist) {
		soc->tx_tso_desc[pool_id].num_free--;
		tso_seg = soc->tx_tso_desc[pool_id].freelist;
		soc->tx_tso_desc[pool_id].freelist =
			soc->tx_tso_desc[pool_id].freelist->next;
	}
	TX_DESC_LOCK_UNLOCK(&soc->tx_tso_desc[pool_id].lock);

	return tso_seg;
}

/**
 * dp_tx_tso_desc_free() - function to free a TSO segment
 * @soc: device soc instance
 * @pool_id: pool id should pick up tso descriptor
 * @tso_seg: tso segment memory pointer
 *
 * Returns a TSO segment element to the free list held in the
 * HTT pdev
 *
 * Return: none
 */

static inline void dp_tx_tso_desc_free(struct dp_soc *soc,
		uint8_t pool_id, struct qdf_tso_seg_elem_t *tso_seg)
{
	TX_DESC_LOCK_LOCK(&soc->tx_tso_desc[pool_id].lock);
	tso_seg->next = soc->tx_tso_desc[pool_id].freelist;
	soc->tx_tso_desc[pool_id].freelist = tso_seg;
	soc->tx_tso_desc[pool_id].num_free++;
	TX_DESC_LOCK_UNLOCK(&soc->tx_tso_desc[pool_id].lock);
}
/*
 * dp_tx_me_alloc_buf() Alloc descriptor from me pool
 * @pdev DP_PDEV handle for datapath
 *
 * Return:dp_tx_me_buf_t(buf)
 */
static inline struct dp_tx_me_buf_t*
dp_tx_me_alloc_buf(struct dp_pdev *pdev)
{
	struct dp_tx_me_buf_t *buf = NULL;
	qdf_spin_lock_bh(&pdev->tx_mutex);
	if (pdev->me_buf.freelist) {
		buf = pdev->me_buf.freelist;
		pdev->me_buf.freelist = pdev->me_buf.freelist->next;
		pdev->me_buf.buf_in_use++;
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				"Error allocating memory in pool");
		qdf_spin_unlock_bh(&pdev->tx_mutex);
		return NULL;
	}
	qdf_spin_unlock_bh(&pdev->tx_mutex);
	return buf;
}

/*
 * dp_tx_me_free_buf() - Free me descriptor and add it to pool
 * @pdev: DP_PDEV handle for datapath
 * @buf : Allocated ME BUF
 *
 * Return:void
 */
static inline void
dp_tx_me_free_buf(struct dp_pdev *pdev, struct dp_tx_me_buf_t *buf)
{
	qdf_spin_lock_bh(&pdev->tx_mutex);
	buf->next = pdev->me_buf.freelist;
	pdev->me_buf.freelist = buf;
	pdev->me_buf.buf_in_use--;
	qdf_spin_unlock_bh(&pdev->tx_mutex);
}
#endif /* DP_TX_DESC_H */

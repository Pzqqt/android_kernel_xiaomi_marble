/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

/**
 * In case of TX descriptor pool and CPU core is combined
 * TX context and TX comp context also should running on the same core
 * in this case, each TX desciptror pool operation will be serialized by core
 * TX and TX_COMP will not race. locking for protection is not requried
 * TX_DESC_POOL_PER_CORE : this is most likely for WIN
 * MCL, TX descriptor pool will be tied to VDEV instance.
 * Then locking protection is required
 */
#ifdef TX_CORE_ALIGNED_SEND
#define TX_DESC_LOCK_CREATE(lock)  /* NOOP */
#define TX_DESC_LOCK_DESTROY(lock) /* NOOP */
#define TX_DESC_LOCK_LOCK(lock)    /* NOOP */
#define TX_DESC_LOCK_UNLOCK(lock)  /* NOOP */
#else
#define TX_DESC_LOCK_CREATE(lock)  qdf_spinlock_create(lock)
#define TX_DESC_LOCK_DESTROY(lock) qdf_spinlock_destroy(lock)
#define TX_DESC_LOCK_LOCK(lock)    qdf_spin_lock(lock)
#define TX_DESC_LOCK_UNLOCK(lock)  qdf_spin_unlock(lock)
#endif /* TX_CORE_ALIGNED_SEND */

QDF_STATUS dp_tx_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem);
QDF_STATUS dp_tx_desc_pool_free(struct dp_soc *soc, uint8_t pool_id);
QDF_STATUS dp_tx_ext_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem);
QDF_STATUS dp_tx_ext_desc_pool_free(struct dp_soc *soc, uint8_t pool_id);

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
	}
	DP_STATS_ADD(pdev, pub.tx.desc_in_use, 1);
	tx_desc->flags |= DP_TX_DESC_FLAG_ALLOCATED;
	TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);

	return tx_desc;
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

	tx_desc->flags &= ~DP_TX_DESC_FLAG_ALLOCATED;
	tx_desc->next = soc->tx_desc[desc_pool_id].freelist;
	soc->tx_desc[desc_pool_id].freelist = tx_desc;
	DP_STATS_SUB(pdev, pub.tx.desc_in_use, 1);

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
#endif /* DP_TX_DESC_H */

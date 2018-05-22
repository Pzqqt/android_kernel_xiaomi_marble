/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifdef TX_PER_PDEV_DESC_POOL
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->vdev_id)
#else /* QCA_LL_TX_FLOW_CONTROL_V2 */
#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->pdev->pdev_id)
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
	#define DP_TX_GET_RING_ID(vdev) (vdev->pdev->pdev_id)
#else
	#ifdef TX_PER_VDEV_DESC_POOL
		#define DP_TX_GET_DESC_POOL_ID(vdev) (vdev->vdev_id)
		#define DP_TX_GET_RING_ID(vdev) (vdev->pdev->pdev_id)
	#endif /* TX_PER_VDEV_DESC_POOL */
#endif /* TX_PER_PDEV_DESC_POOL */

/**
 * 21 bits cookie
 * 2 bits pool id 0 ~ 3,
 * 10 bits page id 0 ~ 1023
 * 5 bits offset id 0 ~ 31 (Desc size = 128, Num descs per page = 4096/128 = 32)
 */
/* ???Ring ID needed??? */
#define DP_TX_DESC_ID_POOL_MASK    0x018000
#define DP_TX_DESC_ID_POOL_OS      15
#define DP_TX_DESC_ID_PAGE_MASK    0x007FE0
#define DP_TX_DESC_ID_PAGE_OS      5
#define DP_TX_DESC_ID_OFFSET_MASK  0x00001F
#define DP_TX_DESC_ID_OFFSET_OS    0

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#define TX_DESC_LOCK_CREATE(lock)
#define TX_DESC_LOCK_DESTROY(lock)
#define TX_DESC_LOCK_LOCK(lock)
#define TX_DESC_LOCK_UNLOCK(lock)
#define TX_DESC_POOL_MEMBER_CLEAN(_tx_desc_pool)       \
do {                                                   \
	(_tx_desc_pool)->elem_size = 0;                \
	(_tx_desc_pool)->freelist = NULL;              \
	(_tx_desc_pool)->pool_size = 0;                \
	(_tx_desc_pool)->avail_desc = 0;               \
	(_tx_desc_pool)->start_th = 0;                 \
	(_tx_desc_pool)->stop_th = 0;                  \
	(_tx_desc_pool)->status = FLOW_POOL_INACTIVE;  \
} while (0)
#else /* !QCA_LL_TX_FLOW_CONTROL_V2 */
#define TX_DESC_LOCK_CREATE(lock)  qdf_spinlock_create(lock)
#define TX_DESC_LOCK_DESTROY(lock) qdf_spinlock_destroy(lock)
#define TX_DESC_LOCK_LOCK(lock)    qdf_spin_lock_bh(lock)
#define TX_DESC_LOCK_UNLOCK(lock)  qdf_spin_unlock_bh(lock)
#define TX_DESC_POOL_MEMBER_CLEAN(_tx_desc_pool)       \
do {                                                   \
	(_tx_desc_pool)->elem_size = 0;                \
	(_tx_desc_pool)->num_allocated = 0;            \
	(_tx_desc_pool)->freelist = NULL;              \
	(_tx_desc_pool)->elem_count = 0;               \
	(_tx_desc_pool)->num_free = 0;                 \
} while (0)
#endif /* !QCA_LL_TX_FLOW_CONTROL_V2 */
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
QDF_STATUS dp_tx_tso_num_seg_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem);
void dp_tx_tso_num_seg_pool_free(struct dp_soc *soc, uint8_t pool_id);

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
void dp_tx_flow_control_init(struct dp_soc *);
void dp_tx_flow_control_deinit(struct dp_soc *);

QDF_STATUS dp_txrx_register_pause_cb(struct cdp_soc_t *soc,
	tx_pause_callback pause_cb);
QDF_STATUS dp_tx_flow_pool_map(struct cdp_soc_t *soc, struct cdp_pdev *pdev,
				uint8_t vdev_id);
void dp_tx_flow_pool_unmap(struct cdp_soc_t *soc, struct cdp_pdev *pdev,
			   uint8_t vdev_id);
void dp_tx_clear_flow_pool_stats(struct dp_soc *soc);
struct dp_tx_desc_pool_s *dp_tx_create_flow_pool(struct dp_soc *soc,
	uint8_t flow_pool_id, uint16_t flow_pool_size);

QDF_STATUS dp_tx_flow_pool_map_handler(struct dp_pdev *pdev, uint8_t flow_id,
	uint8_t flow_type, uint8_t flow_pool_id, uint16_t flow_pool_size);
void dp_tx_flow_pool_unmap_handler(struct dp_pdev *pdev, uint8_t flow_id,
	uint8_t flow_type, uint8_t flow_pool_id);

/**
 * dp_tx_get_desc_flow_pool() - get descriptor from flow pool
 * @pool: flow pool
 *
 * Caller needs to take lock and do sanity checks.
 *
 * Return: tx descriptor
 */
static inline
struct dp_tx_desc_s *dp_tx_get_desc_flow_pool(struct dp_tx_desc_pool_s *pool)
{
	struct dp_tx_desc_s *tx_desc = pool->freelist;

	pool->freelist = pool->freelist->next;
	pool->avail_desc--;
	return tx_desc;
}

/**
 * ol_tx_put_desc_flow_pool() - put descriptor to flow pool freelist
 * @pool: flow pool
 * @tx_desc: tx descriptor
 *
 * Caller needs to take lock and do sanity checks.
 *
 * Return: none
 */
static inline
void dp_tx_put_desc_flow_pool(struct dp_tx_desc_pool_s *pool,
			struct dp_tx_desc_s *tx_desc)
{
	tx_desc->next = pool->freelist;
	pool->freelist = tx_desc;
	pool->avail_desc++;
}


/**
 * dp_tx_desc_alloc() - Allocate a Software Tx Descriptor from given pool
 *
 * @soc Handle to DP SoC structure
 * @pool_id
 *
 * Return:
 */
static inline struct dp_tx_desc_s *
dp_tx_desc_alloc(struct dp_soc *soc, uint8_t desc_pool_id)
{
	struct dp_tx_desc_s *tx_desc = NULL;
	struct dp_tx_desc_pool_s *pool = &soc->tx_desc[desc_pool_id];

	if (pool) {
		qdf_spin_lock_bh(&pool->flow_pool_lock);
		if (pool->status <= FLOW_POOL_ACTIVE_PAUSED &&
		    pool->avail_desc) {
			tx_desc = dp_tx_get_desc_flow_pool(pool);
			tx_desc->pool_id = desc_pool_id;
			tx_desc->flags = DP_TX_DESC_FLAG_ALLOCATED;
			if (qdf_unlikely(pool->avail_desc < pool->stop_th)) {
				pool->status = FLOW_POOL_ACTIVE_PAUSED;
				qdf_spin_unlock_bh(&pool->flow_pool_lock);
				/* pause network queues */
				soc->pause_cb(desc_pool_id,
					       WLAN_STOP_ALL_NETIF_QUEUE,
					       WLAN_DATA_FLOW_CONTROL);
			} else {
				qdf_spin_unlock_bh(&pool->flow_pool_lock);
			}
		} else {
			pool->pkt_drop_no_desc++;
			qdf_spin_unlock_bh(&pool->flow_pool_lock);
		}
	} else {
		soc->pool_stats.pkt_drop_no_pool++;
	}


	return tx_desc;
}

/**
 * dp_tx_desc_free() - Fee a tx descriptor and attach it to free list
 *
 * @soc Handle to DP SoC structure
 * @pool_id
 * @tx_desc
 *
 * Return: None
 */
static inline void
dp_tx_desc_free(struct dp_soc *soc, struct dp_tx_desc_s *tx_desc,
		uint8_t desc_pool_id)
{
	struct dp_tx_desc_pool_s *pool = &soc->tx_desc[desc_pool_id];

	qdf_spin_lock_bh(&pool->flow_pool_lock);
	tx_desc->flags = 0;
	dp_tx_put_desc_flow_pool(pool, tx_desc);
	switch (pool->status) {
	case FLOW_POOL_ACTIVE_PAUSED:
		if (pool->avail_desc > pool->start_th) {
			soc->pause_cb(pool->flow_pool_id,
				       WLAN_WAKE_ALL_NETIF_QUEUE,
				       WLAN_DATA_FLOW_CONTROL);
			pool->status = FLOW_POOL_ACTIVE_UNPAUSED;
		}
		break;
	case FLOW_POOL_INVALID:
		if (pool->avail_desc == pool->pool_size) {
			dp_tx_desc_pool_free(soc, desc_pool_id);
			qdf_spin_unlock_bh(&pool->flow_pool_lock);
			qdf_print("%s %d pool is freed!!\n",
				 __func__, __LINE__);
			return;
		}
		break;

	case FLOW_POOL_ACTIVE_UNPAUSED:
		break;
	default:
		qdf_print("%s %d pool is INACTIVE State!!\n",
				 __func__, __LINE__);
		break;
	};

	qdf_spin_unlock_bh(&pool->flow_pool_lock);

}
#else /* QCA_LL_TX_FLOW_CONTROL_V2 */

static inline void dp_tx_flow_control_init(struct dp_soc *handle)
{
}

static inline void dp_tx_flow_control_deinit(struct dp_soc *handle)
{
}

static inline QDF_STATUS dp_tx_flow_pool_map_handler(struct dp_pdev *pdev,
	uint8_t flow_id, uint8_t flow_type, uint8_t flow_pool_id,
	uint16_t flow_pool_size)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_tx_flow_pool_unmap_handler(struct dp_pdev *pdev,
	uint8_t flow_id, uint8_t flow_type, uint8_t flow_pool_id)
{
}

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

	soc->tx_desc[desc_pool_id].freelist =
		soc->tx_desc[desc_pool_id].freelist->next;
	soc->tx_desc[desc_pool_id].num_allocated++;
	soc->tx_desc[desc_pool_id].num_free--;

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
	soc->tx_desc[desc_pool_id].num_allocated--;
	soc->tx_desc[desc_pool_id].num_free++;


	TX_DESC_LOCK_UNLOCK(&soc->tx_desc[desc_pool_id].lock);
}
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */

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
	struct dp_tx_desc_pool_s *tx_desc_pool = &((soc)->tx_desc[(pool_id)]);

	return tx_desc_pool->desc_pages.cacheable_pages[page_id] +
		tx_desc_pool->elem_size * offset;
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

	qdf_spin_lock_bh(&soc->tx_ext_desc[desc_pool_id].lock);
	if (soc->tx_ext_desc[desc_pool_id].num_free <= 0) {
		qdf_spin_unlock_bh(&soc->tx_ext_desc[desc_pool_id].lock);
		return NULL;
	}
	c_elem = soc->tx_ext_desc[desc_pool_id].freelist;
	soc->tx_ext_desc[desc_pool_id].freelist =
		soc->tx_ext_desc[desc_pool_id].freelist->next;
	soc->tx_ext_desc[desc_pool_id].num_free--;
	qdf_spin_unlock_bh(&soc->tx_ext_desc[desc_pool_id].lock);
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
	qdf_spin_lock_bh(&soc->tx_ext_desc[desc_pool_id].lock);
	elem->next = soc->tx_ext_desc[desc_pool_id].freelist;
	soc->tx_ext_desc[desc_pool_id].freelist = elem;
	soc->tx_ext_desc[desc_pool_id].num_free++;
	qdf_spin_unlock_bh(&soc->tx_ext_desc[desc_pool_id].lock);
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

	qdf_spin_lock_bh(&soc->tx_ext_desc[desc_pool_id].lock);
	tail->next = soc->tx_ext_desc[desc_pool_id].freelist;
	soc->tx_ext_desc[desc_pool_id].freelist = head;
	soc->tx_ext_desc[desc_pool_id].num_free += num_free;
	qdf_spin_unlock_bh(&soc->tx_ext_desc[desc_pool_id].lock);

	return;
}

#if defined(FEATURE_TSO)
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

	qdf_spin_lock_bh(&soc->tx_tso_desc[pool_id].lock);
	if (soc->tx_tso_desc[pool_id].freelist) {
		soc->tx_tso_desc[pool_id].num_free--;
		tso_seg = soc->tx_tso_desc[pool_id].freelist;
		soc->tx_tso_desc[pool_id].freelist =
			soc->tx_tso_desc[pool_id].freelist->next;
	}
	qdf_spin_unlock_bh(&soc->tx_tso_desc[pool_id].lock);

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
	qdf_spin_lock_bh(&soc->tx_tso_desc[pool_id].lock);
	tso_seg->next = soc->tx_tso_desc[pool_id].freelist;
	soc->tx_tso_desc[pool_id].freelist = tso_seg;
	soc->tx_tso_desc[pool_id].num_free++;
	qdf_spin_unlock_bh(&soc->tx_tso_desc[pool_id].lock);
}

static inline
struct qdf_tso_num_seg_elem_t  *dp_tso_num_seg_alloc(struct dp_soc *soc,
		uint8_t pool_id)
{
	struct qdf_tso_num_seg_elem_t *tso_num_seg = NULL;

	qdf_spin_lock_bh(&soc->tx_tso_num_seg[pool_id].lock);
	if (soc->tx_tso_num_seg[pool_id].freelist) {
		soc->tx_tso_num_seg[pool_id].num_free--;
		tso_num_seg = soc->tx_tso_num_seg[pool_id].freelist;
		soc->tx_tso_num_seg[pool_id].freelist =
			soc->tx_tso_num_seg[pool_id].freelist->next;
	}
	qdf_spin_unlock_bh(&soc->tx_tso_num_seg[pool_id].lock);

	return tso_num_seg;
}

static inline
void dp_tso_num_seg_free(struct dp_soc *soc,
		uint8_t pool_id, struct qdf_tso_num_seg_elem_t *tso_num_seg)
{
	qdf_spin_lock_bh(&soc->tx_tso_num_seg[pool_id].lock);
	tso_num_seg->next = soc->tx_tso_num_seg[pool_id].freelist;
	soc->tx_tso_num_seg[pool_id].freelist = tso_num_seg;
	soc->tx_tso_num_seg[pool_id].num_free++;
	qdf_spin_unlock_bh(&soc->tx_tso_num_seg[pool_id].lock);
}
#endif

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

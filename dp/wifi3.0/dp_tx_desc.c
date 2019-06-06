/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "hal_hw_headers.h"
#include "dp_types.h"
#include "dp_tx_desc.h"

#ifndef DESC_PARTITION
#define DP_TX_DESC_SIZE(a) qdf_get_pwr2(a)
#define DP_TX_DESC_PAGE_DIVIDER(soc, num_desc_per_page, pool_id)     \
do {                                                                 \
	uint8_t sig_bit;                                             \
	soc->tx_desc[pool_id].offset_filter = num_desc_per_page - 1; \
	/* Calculate page divider to find page number */             \
	sig_bit = 0;                                                 \
	while (num_desc_per_page) {                                  \
		sig_bit++;                                           \
		num_desc_per_page = num_desc_per_page >> 1;          \
	}                                                            \
	soc->tx_desc[pool_id].page_divider = (sig_bit - 1);          \
} while (0)
#else
#define DP_TX_DESC_SIZE(a) a
#define DP_TX_DESC_PAGE_DIVIDER(soc, num_desc_per_page, pool_id) {}
#endif /* DESC_PARTITION */

/**
 * dp_tx_desc_pool_counter_initialize() - Initialize counters
 * @tx_desc_pool Handle to DP tx_desc_pool structure
 * @num_elem Number of descriptor elements per pool
 *
 * Return: None
 */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
static void
dp_tx_desc_pool_counter_initialize(struct dp_tx_desc_pool_s *tx_desc_pool,
				  uint16_t num_elem)
{
}
#else
static void
dp_tx_desc_pool_counter_initialize(struct dp_tx_desc_pool_s *tx_desc_pool,
				  uint16_t num_elem)
{
	tx_desc_pool->num_free = num_elem;
	tx_desc_pool->num_allocated = 0;
}
#endif

/**
 * dp_tx_desc_pool_alloc() - Allocate Tx Descriptor pool(s)
 * @soc Handle to DP SoC structure
 * @num_pool Number of pools to allocate
 * @num_elem Number of descriptor elements per pool
 *
 * This function allocates memory for SW tx descriptors
 * (used within host for tx data path).
 * The number of tx descriptors required will be large
 * since based on number of clients (1024 clients x 3 radios),
 * outstanding MSDUs stored in TQM queues and LMAC queues will be significantly
 * large.
 *
 * To avoid allocating a large contiguous memory, it uses multi_page_alloc qdf
 * function to allocate memory
 * in multiple pages. It then iterates through the memory allocated across pages
 * and links each descriptor
 * to next descriptor, taking care of page boundaries.
 *
 * Since WiFi 3.0 HW supports multiple Tx rings, multiple pools are allocated,
 * one for each ring;
 * This minimizes lock contention when hard_start_xmit is called
 * from multiple CPUs.
 * Alternately, multiple pools can be used for multiple VDEVs for VDEV level
 * flow control.
 *
 * Return: Status code. 0 for success.
 */
QDF_STATUS dp_tx_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem)
{
	uint32_t id, count, page_id, offset, pool_id_32;
	uint16_t num_desc_per_page;
	struct dp_tx_desc_s *tx_desc_elem;
	uint32_t desc_size;
	struct dp_tx_desc_pool_s *tx_desc_pool = &((soc)->tx_desc[(pool_id)]);

	desc_size = DP_TX_DESC_SIZE(sizeof(*tx_desc_elem));
	tx_desc_pool->elem_size = desc_size;
	if (!dp_is_soc_reinit(soc))
		qdf_mem_multi_pages_alloc(soc->osdev,
					  &tx_desc_pool->desc_pages,
					  desc_size, num_elem,
					  0, true);
	if (!tx_desc_pool->desc_pages.num_pages) {
		dp_err("Multi page alloc fail, tx desc");
		goto fail_exit;
	}


	num_desc_per_page =
		tx_desc_pool->desc_pages.num_element_per_page;
	tx_desc_pool->freelist = (struct dp_tx_desc_s *)
			*tx_desc_pool->desc_pages.cacheable_pages;
	if (qdf_mem_multi_page_link(soc->osdev,
				    &tx_desc_pool->desc_pages,
				    desc_size, num_elem, true)) {
		dp_err("invalid tx desc allocation - overflow num link");
		goto free_tx_desc;
	}

	/* Set unique IDs for each Tx descriptor */
	tx_desc_elem = tx_desc_pool->freelist;
	count = 0;
	pool_id_32 = (uint32_t)pool_id;
	while (tx_desc_elem) {
		page_id = count / num_desc_per_page;
		offset = count % num_desc_per_page;
		id = ((pool_id_32 << DP_TX_DESC_ID_POOL_OS) |
			(page_id << DP_TX_DESC_ID_PAGE_OS) | offset);

		tx_desc_elem->id = id;
		tx_desc_elem->pool_id = pool_id;
		tx_desc_elem = tx_desc_elem->next;
		count++;
	}

	dp_tx_desc_pool_counter_initialize(tx_desc_pool, num_elem);
	TX_DESC_LOCK_CREATE(&tx_desc_pool->lock);
	return QDF_STATUS_SUCCESS;

free_tx_desc:
	qdf_mem_multi_pages_free(soc->osdev,
		&tx_desc_pool->desc_pages, 0, true);

fail_exit:
	return QDF_STATUS_E_FAULT;
}

/**
 * dp_tx_desc_pool_free() - Free the memory pool allocated for Tx Descriptors
 *
 * @soc Handle to DP SoC structure
 * @pool_id
 *
 * Return:
 */
QDF_STATUS dp_tx_desc_pool_free(struct dp_soc *soc, uint8_t pool_id)
{
	struct dp_tx_desc_pool_s *tx_desc_pool =
				&((soc)->tx_desc[(pool_id)]);

	qdf_mem_multi_pages_free(soc->osdev,
		&tx_desc_pool->desc_pages, 0, true);
	TX_DESC_LOCK_DESTROY(&tx_desc_pool->lock);
	TX_DESC_POOL_MEMBER_CLEAN(tx_desc_pool);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_ext_desc_pool_alloc() - Allocate tx ext descriptor pool
 * @soc Handle to DP SoC structure
 * @pool_id
 *
 * Return: NONE
 */
QDF_STATUS dp_tx_ext_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
	uint16_t num_elem)
{
	uint16_t num_page;
	uint32_t count;
	struct dp_tx_ext_desc_elem_s *c_elem, *p_elem;
	struct qdf_mem_dma_page_t *page_info;
	struct qdf_mem_multi_page_t *pages;
	QDF_STATUS status;
	qdf_dma_context_t memctx = 0;

	/* Coherent tx extension descriptor alloc */
	soc->tx_ext_desc[pool_id].elem_size = HAL_TX_EXT_DESC_WITH_META_DATA;
	soc->tx_ext_desc[pool_id].elem_count = num_elem;
	memctx = qdf_get_dma_mem_context((&soc->tx_ext_desc[pool_id]), memctx);
	if (!dp_is_soc_reinit(soc)) {
		qdf_mem_multi_pages_alloc(soc->osdev,
					  &soc->tx_ext_desc[pool_id].
					  desc_pages,
					  soc->tx_ext_desc[pool_id].elem_size,
					  soc->tx_ext_desc[pool_id].elem_count,
					  memctx, false);
	}
	if (!soc->tx_ext_desc[pool_id].desc_pages.num_pages) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "ext desc page alloc fail");
		status = QDF_STATUS_E_NOMEM;
		goto fail_exit;
	}

	num_page = soc->tx_ext_desc[pool_id].desc_pages.num_pages;
	/*
	 * Cacheable ext descriptor link alloc
	 * This structure also large size already
	 * single element is 24bytes, 2K elements are 48Kbytes
	 * Have to alloc multi page cacheable memory
	 */
	soc->tx_ext_desc[pool_id].link_elem_size =
		sizeof(struct dp_tx_ext_desc_elem_s);
	if (!dp_is_soc_reinit(soc)) {
		qdf_mem_multi_pages_alloc(soc->osdev,
					  &soc->tx_ext_desc[pool_id].
					  desc_link_pages,
					  soc->tx_ext_desc[pool_id].
					  link_elem_size,
					  soc->tx_ext_desc[pool_id].
					  elem_count,
					  0, true);
	}
	if (!soc->tx_ext_desc[pool_id].desc_link_pages.num_pages) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "ext link desc page alloc fail");
		status = QDF_STATUS_E_NOMEM;
		goto free_ext_desc_page;
	}

	/* link tx descriptors into a freelist */
	soc->tx_ext_desc[pool_id].freelist = (struct dp_tx_ext_desc_elem_s *)
		*soc->tx_ext_desc[pool_id].desc_link_pages.cacheable_pages;
	if (qdf_mem_multi_page_link(soc->osdev,
		&soc->tx_ext_desc[pool_id].desc_link_pages,
		soc->tx_ext_desc[pool_id].link_elem_size,
		soc->tx_ext_desc[pool_id].elem_count, true)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "ext link desc page linking fail");
		status = QDF_STATUS_E_FAULT;
		goto free_ext_link_desc_page;
	}

	/* Assign coherent memory pointer into linked free list */
	pages = &soc->tx_ext_desc[pool_id].desc_pages;
	page_info = soc->tx_ext_desc[pool_id].desc_pages.dma_pages;
	c_elem = soc->tx_ext_desc[pool_id].freelist;
	p_elem = c_elem;
	for (count = 0; count < soc->tx_ext_desc[pool_id].elem_count; count++) {
		if (!(count % pages->num_element_per_page)) {
			/**
			 * First element for new page,
			 * should point next page
			 */
			if (!pages->dma_pages->page_v_addr_start) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					"link over flow");
				status = QDF_STATUS_E_FAULT;
				goto free_ext_link_desc_page;
			}
			c_elem->vaddr = (void *)page_info->page_v_addr_start;
			c_elem->paddr = page_info->page_p_addr;
			page_info++;
		} else {
			c_elem->vaddr = (void *)(p_elem->vaddr +
				soc->tx_ext_desc[pool_id].elem_size);
			c_elem->paddr = (p_elem->paddr +
				soc->tx_ext_desc[pool_id].elem_size);
		}
		p_elem = c_elem;
		c_elem = c_elem->next;
		if (!c_elem)
			break;
	}

	soc->tx_ext_desc[pool_id].num_free = num_elem;
	qdf_spinlock_create(&soc->tx_ext_desc[pool_id].lock);
	return QDF_STATUS_SUCCESS;

free_ext_link_desc_page:
	qdf_mem_multi_pages_free(soc->osdev,
		&soc->tx_ext_desc[pool_id].desc_link_pages, 0, true);

free_ext_desc_page:
	qdf_mem_multi_pages_free(soc->osdev,
		&soc->tx_ext_desc[pool_id].desc_pages,
		qdf_get_dma_mem_context((&soc->tx_ext_desc[pool_id]), memctx),
		false);

fail_exit:
	return status;

}

/**
 * dp_tx_ext_desc_pool_free() - free tx ext descriptor pool
 * @soc: Handle to DP SoC structure
 * @pool_id: extension descriptor pool id
 *
 * Return: NONE
 */
QDF_STATUS dp_tx_ext_desc_pool_free(struct dp_soc *soc, uint8_t pool_id)
{
	qdf_mem_multi_pages_free(soc->osdev,
		&soc->tx_ext_desc[pool_id].desc_link_pages, 0, true);

	qdf_mem_multi_pages_free(soc->osdev,
		&soc->tx_ext_desc[pool_id].desc_pages,
		qdf_get_dma_mem_context((&soc->tx_ext_desc[pool_id]), memctx),
		false);

	qdf_spinlock_destroy(&soc->tx_ext_desc[pool_id].lock);
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_tso_desc_pool_alloc() - allocate tx tso descriptor pool
 * @soc: Handle to DP SoC structure
 * @pool_id: tso descriptor pool id
 * @num_elem: number of element
 *
 * Return: QDF_STATUS_SUCCESS
 */
#if defined(FEATURE_TSO)
QDF_STATUS dp_tx_tso_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem)
{
	struct dp_tx_tso_seg_pool_s *tso_desc_pool;
	uint32_t desc_size;

	tso_desc_pool = &soc->tx_tso_desc[pool_id];
	tso_desc_pool->num_free = 0;
	desc_size = DP_TX_DESC_SIZE(sizeof(struct qdf_tso_seg_elem_t));
	if (!dp_is_soc_reinit(soc))
		qdf_mem_multi_pages_alloc(soc->osdev,
					  &tso_desc_pool->desc_pages,
					  desc_size,
					  num_elem, 0, true);

	if (!tso_desc_pool->desc_pages.num_pages) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("Alloc Failed %pK pool_id %d"),
			  soc, pool_id);
		return QDF_STATUS_E_NOMEM;
	}

	tso_desc_pool->freelist = (struct qdf_tso_seg_elem_t *)
				  *tso_desc_pool->desc_pages.cacheable_pages;
	tso_desc_pool->num_free = num_elem;
	if (qdf_mem_multi_page_link(soc->osdev,
				    &tso_desc_pool->desc_pages,
				    desc_size,
				    num_elem, true)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "invalid tso desc allocation - overflow num link");
		goto free_tso_desc;
	}
	TSO_DEBUG("Number of free descriptors: %u\n", tso_desc_pool->num_free);
	tso_desc_pool->pool_size = num_elem;
	qdf_spinlock_create(&tso_desc_pool->lock);

	return QDF_STATUS_SUCCESS;

free_tso_desc:
	qdf_mem_multi_pages_free(soc->osdev,
				 &tso_desc_pool->desc_pages, 0, true);

	return QDF_STATUS_E_FAULT;
}

/**
 * dp_tx_tso_desc_pool_free() - free tx tso descriptor pool
 * @soc: Handle to DP SoC structure
 * @pool_id: extension descriptor pool id
 *
 * Return: NONE
 */
void dp_tx_tso_desc_pool_free(struct dp_soc *soc, uint8_t pool_id)
{
	struct dp_tx_tso_seg_pool_s *tso_desc_pool;

	tso_desc_pool = &soc->tx_tso_desc[pool_id];

	qdf_spin_lock_bh(&tso_desc_pool->lock);

	qdf_mem_multi_pages_free(soc->osdev,
				 &tso_desc_pool->desc_pages, 0, true);
	tso_desc_pool->freelist = NULL;
	tso_desc_pool->num_free = 0;
	tso_desc_pool->pool_size = 0;
	qdf_spin_unlock_bh(&tso_desc_pool->lock);
	qdf_spinlock_destroy(&tso_desc_pool->lock);
	return;
}
/**
 * dp_tx_tso_num_seg_pool_alloc() - Allocate descriptors that tracks the
 *                              fragments in each tso segment
 *
 * @soc: handle to dp soc structure
 * @pool_id: descriptor pool id
 * @num_elem: total number of descriptors to be allocated
 */
QDF_STATUS dp_tx_tso_num_seg_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem)
{
	struct dp_tx_tso_num_seg_pool_s *tso_num_seg_pool;
	uint32_t desc_size;

	tso_num_seg_pool = &soc->tx_tso_num_seg[pool_id];
	tso_num_seg_pool->num_free = 0;
	desc_size = DP_TX_DESC_SIZE(sizeof(struct qdf_tso_num_seg_elem_t));
	if (!dp_is_soc_reinit(soc))
		qdf_mem_multi_pages_alloc(soc->osdev,
					  &tso_num_seg_pool->desc_pages,
					  desc_size,
					  num_elem, 0, true);
	if (!tso_num_seg_pool->desc_pages.num_pages) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("Alloc Failed %pK pool_id %d"),
			  soc, pool_id);
		return QDF_STATUS_E_NOMEM;
	}

	if (qdf_mem_multi_page_link(soc->osdev,
				    &tso_num_seg_pool->desc_pages,
				    desc_size,
				    num_elem, true)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "invalid tso desc allocation - overflow num link");
		goto fail;
	}

	tso_num_seg_pool->freelist = (struct qdf_tso_num_seg_elem_t *)
				  *tso_num_seg_pool->desc_pages.cacheable_pages;
	tso_num_seg_pool->num_free = num_elem;
	tso_num_seg_pool->num_seg_pool_size = num_elem;

	qdf_spinlock_create(&tso_num_seg_pool->lock);

	return QDF_STATUS_SUCCESS;

fail:
	qdf_mem_multi_pages_free(soc->osdev,
				 &tso_num_seg_pool->desc_pages, 0, true);

	return QDF_STATUS_E_NOMEM;
}

/**
 * dp_tx_tso_num_seg_pool_free() - free pool of descriptors that tracks
 *			      the fragments in tso segment
 *
 *
 * @soc: handle to dp soc structure
 * @pool_id: descriptor pool_id
 */
void dp_tx_tso_num_seg_pool_free(struct dp_soc *soc, uint8_t pool_id)
{
	struct dp_tx_tso_num_seg_pool_s *tso_num_seg_pool;

	tso_num_seg_pool = &soc->tx_tso_num_seg[pool_id];
	qdf_spin_lock_bh(&tso_num_seg_pool->lock);

	qdf_mem_multi_pages_free(soc->osdev,
				 &tso_num_seg_pool->desc_pages, 0, true);
	tso_num_seg_pool->freelist = NULL;
	tso_num_seg_pool->num_free = 0;
	tso_num_seg_pool->num_seg_pool_size = 0;
	qdf_spin_unlock_bh(&tso_num_seg_pool->lock);
	qdf_spinlock_destroy(&tso_num_seg_pool->lock);
	return;
}

#else
QDF_STATUS dp_tx_tso_desc_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem)
{
	return QDF_STATUS_SUCCESS;
}

void dp_tx_tso_desc_pool_free(struct dp_soc *soc, uint8_t pool_id)
{
	return;
}

QDF_STATUS dp_tx_tso_num_seg_pool_alloc(struct dp_soc *soc, uint8_t pool_id,
		uint16_t num_elem)
{
	return QDF_STATUS_SUCCESS;
}

void dp_tx_tso_num_seg_pool_free(struct dp_soc *soc, uint8_t pool_id)
{
	return;
}
#endif

/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include "dp_rx.h"
#include "dp_ipa.h"

#ifdef RX_DESC_MULTI_PAGE_ALLOC
A_COMPILE_TIME_ASSERT(cookie_size_check,
		      PAGE_SIZE / sizeof(union dp_rx_desc_list_elem_t) <=
		      1 << DP_RX_DESC_PAGE_ID_SHIFT);

/*
 * dp_rx_desc_pool_is_allocated() - check if memory is allocated for the
 *					rx descriptor pool
 *
 * @rx_desc_pool: rx descriptor pool pointer
 * Return: QDF_STATUS  QDF_STATUS_SUCCESS
 *		       QDF_STATUS_E_NOMEM
 */
QDF_STATUS dp_rx_desc_pool_is_allocated(struct rx_desc_pool *rx_desc_pool)
{
	if (!rx_desc_pool->desc_pages.num_pages) {
		dp_err("Multi page alloc fail, size=%d, elem=%d",
		       rx_desc_pool->elem_size, rx_desc_pool->pool_size);
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_desc_pool_alloc() - Allocate a memory pool for software rx
 *			     descriptors
 *
 * @soc: core txrx main context
 * @num_elem: number of rx descriptors (size of the pool)
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: QDF_STATUS  QDF_STATUS_SUCCESS
 *		       QDF_STATUS_E_NOMEM
 *		       QDF_STATUS_E_FAULT
 */
QDF_STATUS dp_rx_desc_pool_alloc(struct dp_soc *soc,
				 uint32_t num_elem,
				 struct rx_desc_pool *rx_desc_pool)
{
	uint32_t desc_size;
	union dp_rx_desc_list_elem_t *rx_desc_elem;

	desc_size = sizeof(*rx_desc_elem);
	rx_desc_pool->elem_size = desc_size;

	dp_desc_multi_pages_mem_alloc(soc, rx_desc_pool->desc_type,
				      &rx_desc_pool->desc_pages,
				      desc_size, num_elem, 0, true);
	if (!rx_desc_pool->desc_pages.num_pages) {
		qdf_err("Multi page alloc fail,size=%d, elem=%d",
			desc_size, num_elem);
		return QDF_STATUS_E_NOMEM;
	}

	if (qdf_mem_multi_page_link(soc->osdev,
				    &rx_desc_pool->desc_pages,
				    desc_size, num_elem, true)) {
		qdf_err("overflow num link,size=%d, elem=%d",
			desc_size, num_elem);
		goto free_rx_desc_pool;
	}
	return QDF_STATUS_SUCCESS;

free_rx_desc_pool:
	dp_rx_desc_pool_free(soc, rx_desc_pool);

	return QDF_STATUS_E_FAULT;
}

/*
 * dp_rx_desc_pool_init() - Initialize the software RX descriptor pool
 *			convert the pool of memory into a list of
 *			rx descriptors and create locks to access this
 *			list of rx descriptors.
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @pool_size: size of the rx descriptor pool
 * @rx_desc_pool: rx descriptor pool pointer
 */
void dp_rx_desc_pool_init(struct dp_soc *soc, uint32_t pool_id,
			  uint32_t pool_size, struct rx_desc_pool *rx_desc_pool)
{
	uint32_t id, page_id, offset, num_desc_per_page;
	uint32_t count = 0;
	union dp_rx_desc_list_elem_t *rx_desc_elem;

	/* Initialize the lock */
	qdf_spinlock_create(&rx_desc_pool->lock);

	qdf_spin_lock_bh(&rx_desc_pool->lock);
	rx_desc_pool->pool_size = pool_size;

	num_desc_per_page = rx_desc_pool->desc_pages.num_element_per_page;
	rx_desc_pool->freelist = (union dp_rx_desc_list_elem_t *)
				  *rx_desc_pool->desc_pages.cacheable_pages;

	rx_desc_elem = rx_desc_pool->freelist;
	while (rx_desc_elem) {
		page_id = count / num_desc_per_page;
		offset = count % num_desc_per_page;
		/*
		 * Below cookie size is from REO destination ring
		 * reo_destination_ring -> buffer_addr_info -> sw_buffer_cookie
		 * cookie size = 21 bits
		 * 8 bits - offset
		 * 8 bits - page ID
		 * 4 bits - pool ID
		 */
		id = ((pool_id << DP_RX_DESC_POOL_ID_SHIFT) |
		      (page_id << DP_RX_DESC_PAGE_ID_SHIFT) |
		      offset);
		rx_desc_elem->rx_desc.cookie = id;
		rx_desc_elem->rx_desc.pool_id = pool_id;
		rx_desc_elem->rx_desc.in_use = 0;
		rx_desc_elem = rx_desc_elem->next;
		count++;
	}
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
}

union dp_rx_desc_list_elem_t *dp_rx_desc_find(uint16_t page_id, uint16_t offset,
					      struct rx_desc_pool *rx_desc_pool)
{
	return rx_desc_pool->desc_pages.cacheable_pages[page_id] +
		rx_desc_pool->elem_size * offset;
}

static QDF_STATUS dp_rx_desc_nbuf_collect(struct dp_soc *soc,
					  struct rx_desc_pool *rx_desc_pool,
					  qdf_nbuf_t *nbuf_unmap_list,
					  qdf_nbuf_t *nbuf_free_list)
{
	uint32_t i, num_desc, page_id, offset, num_desc_per_page;
	union dp_rx_desc_list_elem_t *rx_desc_elem;
	struct dp_rx_desc *rx_desc;

	if (qdf_unlikely(!(rx_desc_pool->desc_pages.cacheable_pages))) {
		qdf_err("No pages found on this desc pool");
		return QDF_STATUS_E_INVAL;
	}
	num_desc = rx_desc_pool->pool_size;
	num_desc_per_page = rx_desc_pool->desc_pages.num_element_per_page;
	for (i = 0; i < num_desc; i++) {
		page_id = i / num_desc_per_page;
		offset = i % num_desc_per_page;
		rx_desc_elem = dp_rx_desc_find(page_id, offset, rx_desc_pool);
		rx_desc = &rx_desc_elem->rx_desc;
		dp_rx_desc_free_dbg_info(rx_desc);
		if (rx_desc->in_use) {
			if (!rx_desc->unmapped) {
				DP_RX_HEAD_APPEND(*nbuf_unmap_list,
						  rx_desc->nbuf);
				rx_desc->unmapped = 1;
			} else {
				DP_RX_HEAD_APPEND(*nbuf_free_list,
						  rx_desc->nbuf);
			}
		}
	}
	return QDF_STATUS_SUCCESS;
}

static void dp_rx_desc_nbuf_cleanup(struct dp_soc *soc,
				    qdf_nbuf_t nbuf_unmap_list,
				    qdf_nbuf_t nbuf_free_list,
				    uint16_t buf_size)
{
	qdf_nbuf_t nbuf = nbuf_unmap_list;
	qdf_nbuf_t next;

	while (nbuf) {
		next = nbuf->next;
		if (dp_ipa_handle_rx_buf_smmu_mapping(soc, nbuf, buf_size,
						      false))
			dp_info_rl("Unable to unmap nbuf: %pK", nbuf);
		qdf_nbuf_unmap_nbytes_single(soc->osdev, nbuf,
					     QDF_DMA_BIDIRECTIONAL, buf_size);
		qdf_nbuf_free(nbuf);
		nbuf = next;
	}

	nbuf = nbuf_free_list;
	while (nbuf) {
		next = nbuf->next;
		qdf_nbuf_free(nbuf);
		nbuf = next;
	}
}

void dp_rx_desc_nbuf_and_pool_free(struct dp_soc *soc, uint32_t pool_id,
				   struct rx_desc_pool *rx_desc_pool)
{
	qdf_nbuf_t nbuf_unmap_list = NULL;
	qdf_nbuf_t nbuf_free_list = NULL;

	qdf_spin_lock_bh(&rx_desc_pool->lock);
	dp_rx_desc_nbuf_collect(soc, rx_desc_pool,
				&nbuf_unmap_list, &nbuf_free_list);
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
	dp_rx_desc_nbuf_cleanup(soc, nbuf_unmap_list, nbuf_free_list,
				rx_desc_pool->buf_size);
	qdf_spinlock_destroy(&rx_desc_pool->lock);
}

void dp_rx_desc_nbuf_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool)
{
	qdf_nbuf_t nbuf_unmap_list = NULL;
	qdf_nbuf_t nbuf_free_list = NULL;
	qdf_spin_lock_bh(&rx_desc_pool->lock);
	dp_rx_desc_nbuf_collect(soc, rx_desc_pool,
				&nbuf_unmap_list, &nbuf_free_list);
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
	dp_rx_desc_nbuf_cleanup(soc, nbuf_unmap_list, nbuf_free_list,
				rx_desc_pool->buf_size);
}

void dp_rx_desc_pool_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool)
{
	if (qdf_unlikely(!(rx_desc_pool->desc_pages.cacheable_pages)))
		return;

	dp_desc_multi_pages_mem_free(soc, rx_desc_pool->desc_type,
				     &rx_desc_pool->desc_pages, 0, true);
}

void dp_rx_desc_pool_deinit(struct dp_soc *soc,
			    struct rx_desc_pool *rx_desc_pool)
{
	qdf_spin_lock_bh(&rx_desc_pool->lock);

	rx_desc_pool->freelist = NULL;
	rx_desc_pool->pool_size = 0;

	/* Deinitialize rx mon desr frag flag */
	rx_desc_pool->rx_mon_dest_frag_enable = false;

	qdf_spin_unlock_bh(&rx_desc_pool->lock);
	qdf_spinlock_destroy(&rx_desc_pool->lock);
}
#else
/*
 * dp_rx_desc_pool_is_allocated() - check if memory is allocated for the
 *					rx descriptor pool
 *
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: QDF_STATUS  QDF_STATUS_SUCCESS
 *		       QDF_STATUS_E_NOMEM
 */
QDF_STATUS dp_rx_desc_pool_is_allocated(struct rx_desc_pool *rx_desc_pool)
{
	if (!rx_desc_pool->array) {
		dp_err("nss-wifi<4> skip Rx refil");
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_desc_pool_alloc() - Allocate a memory pool for software rx
 *			     descriptors
 *
 * @soc: core txrx main context
 * @num_elem: number of rx descriptors (size of the pool)
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: QDF_STATUS  QDF_STATUS_SUCCESS
 *		       QDF_STATUS_E_NOMEM
 *		       QDF_STATUS_E_FAULT
 */
QDF_STATUS dp_rx_desc_pool_alloc(struct dp_soc *soc,
				 uint32_t pool_size,
				 struct rx_desc_pool *rx_desc_pool)
{
	rx_desc_pool->array = qdf_mem_malloc(pool_size *
				     sizeof(union dp_rx_desc_list_elem_t));

	if (!(rx_desc_pool->array)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "RX Desc Pool allocation failed");
		return QDF_STATUS_E_NOMEM;
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_desc_pool_init() - Initialize the software RX descriptor pool
 *			convert the pool of memory into a list of
 *			rx descriptors and create locks to access this
 *			list of rx descriptors.
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @pool_size: size of the rx descriptor pool
 * @rx_desc_pool: rx descriptor pool pointer
 */
void dp_rx_desc_pool_init(struct dp_soc *soc, uint32_t pool_id,
			  uint32_t pool_size, struct rx_desc_pool *rx_desc_pool)
{
	int i;
	/* Initialize the lock */
	qdf_spinlock_create(&rx_desc_pool->lock);

	qdf_spin_lock_bh(&rx_desc_pool->lock);
	rx_desc_pool->pool_size = pool_size;

	/* link SW rx descs into a freelist */
	rx_desc_pool->freelist = &rx_desc_pool->array[0];
	qdf_mem_zero(rx_desc_pool->freelist, rx_desc_pool->pool_size);
	for (i = 0; i <= rx_desc_pool->pool_size - 1; i++) {
		if (i == rx_desc_pool->pool_size - 1)
			rx_desc_pool->array[i].next = NULL;
		else
			rx_desc_pool->array[i].next =
				&rx_desc_pool->array[i + 1];
		rx_desc_pool->array[i].rx_desc.cookie = i | (pool_id << 18);
		rx_desc_pool->array[i].rx_desc.pool_id = pool_id;
		rx_desc_pool->array[i].rx_desc.in_use = 0;
	}

	qdf_spin_unlock_bh(&rx_desc_pool->lock);
}

void dp_rx_desc_nbuf_and_pool_free(struct dp_soc *soc, uint32_t pool_id,
				   struct rx_desc_pool *rx_desc_pool)
{
	qdf_nbuf_t nbuf;
	int i;

	qdf_spin_lock_bh(&rx_desc_pool->lock);
	for (i = 0; i < rx_desc_pool->pool_size; i++) {
		if (rx_desc_pool->array[i].rx_desc.in_use) {
			nbuf = rx_desc_pool->array[i].rx_desc.nbuf;

			if (!(rx_desc_pool->array[i].rx_desc.unmapped)) {
				dp_ipa_handle_rx_buf_smmu_mapping(
							soc, nbuf,
							rx_desc_pool->buf_size,
							false);
				qdf_nbuf_unmap_nbytes_single(
							soc->osdev, nbuf,
							QDF_DMA_FROM_DEVICE,
							rx_desc_pool->buf_size);
				rx_desc_pool->array[i].rx_desc.unmapped = 1;
			}
			qdf_nbuf_free(nbuf);
		}
	}
	qdf_mem_free(rx_desc_pool->array);
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
	qdf_spinlock_destroy(&rx_desc_pool->lock);
}

void dp_rx_desc_nbuf_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool)
{
	qdf_nbuf_t nbuf;
	int i;

	qdf_spin_lock_bh(&rx_desc_pool->lock);
	for (i = 0; i < rx_desc_pool->pool_size; i++) {
		if (rx_desc_pool->array[i].rx_desc.in_use) {
			nbuf = rx_desc_pool->array[i].rx_desc.nbuf;

			if (!(rx_desc_pool->array[i].rx_desc.unmapped)) {
				dp_ipa_handle_rx_buf_smmu_mapping(
						soc, nbuf,
						rx_desc_pool->buf_size,
						false);
				qdf_nbuf_unmap_nbytes_single(
							soc->osdev, nbuf,
							QDF_DMA_FROM_DEVICE,
							rx_desc_pool->buf_size);
				rx_desc_pool->array[i].rx_desc.unmapped = 1;
			}
			qdf_nbuf_free(nbuf);
		}
	}
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
}

/**
 * dp_rx_desc_frag_free() - Free desc frag buffer
 *
 * @soc: core txrx main context
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * Return: None
 */
#ifdef DP_RX_MON_MEM_FRAG
void dp_rx_desc_frag_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool)
{
	qdf_dma_addr_t paddr;
	qdf_frag_t vaddr;
	int i;

	qdf_spin_lock_bh(&rx_desc_pool->lock);
	for (i = 0; i < rx_desc_pool->pool_size; i++) {
		if (rx_desc_pool->array[i].rx_desc.in_use) {
			paddr = rx_desc_pool->array[i].rx_desc.paddr_buf_start;
			vaddr = rx_desc_pool->array[i].rx_desc.rx_buf_start;

			if (!(rx_desc_pool->array[i].rx_desc.unmapped)) {
				qdf_mem_unmap_page(soc->osdev, paddr,
						   rx_desc_pool->buf_size,
						   QDF_DMA_FROM_DEVICE);
				rx_desc_pool->array[i].rx_desc.unmapped = 1;
			}
			qdf_frag_free(vaddr);
		}
	}
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
}
#endif

void dp_rx_desc_pool_free(struct dp_soc *soc,
			  struct rx_desc_pool *rx_desc_pool)
{
	qdf_mem_free(rx_desc_pool->array);
}

void dp_rx_desc_pool_deinit(struct dp_soc *soc,
			    struct rx_desc_pool *rx_desc_pool)
{
	qdf_spin_lock_bh(&rx_desc_pool->lock);

	rx_desc_pool->freelist = NULL;
	rx_desc_pool->pool_size = 0;

	/* Deinitialize rx mon desr frag flag */
	rx_desc_pool->rx_mon_dest_frag_enable = false;

	qdf_spin_unlock_bh(&rx_desc_pool->lock);
	qdf_spinlock_destroy(&rx_desc_pool->lock);
}

#endif /* RX_DESC_MULTI_PAGE_ALLOC */

/*
 * dp_rx_get_free_desc_list() - provide a list of descriptors from
 *				the free rx desc pool.
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @rx_desc_pool: rx descriptor pool pointer
 * @num_descs: number of descs requested from freelist
 * @desc_list: attach the descs to this list (output parameter)
 * @tail: attach the point to last desc of free list (output parameter)
 *
 * Return: number of descs allocated from free list.
 */
uint16_t dp_rx_get_free_desc_list(struct dp_soc *soc, uint32_t pool_id,
				struct rx_desc_pool *rx_desc_pool,
				uint16_t num_descs,
				union dp_rx_desc_list_elem_t **desc_list,
				union dp_rx_desc_list_elem_t **tail)
{
	uint16_t count;

	qdf_spin_lock_bh(&rx_desc_pool->lock);

	*desc_list = *tail = rx_desc_pool->freelist;

	for (count = 0; count < num_descs; count++) {

		if (qdf_unlikely(!rx_desc_pool->freelist)) {
			qdf_spin_unlock_bh(&rx_desc_pool->lock);
			return count;
		}
		*tail = rx_desc_pool->freelist;
		rx_desc_pool->freelist = rx_desc_pool->freelist->next;
	}
	(*tail)->next = NULL;
	qdf_spin_unlock_bh(&rx_desc_pool->lock);
	return count;
}

/*
 * dp_rx_add_desc_list_to_free_list() - append unused desc_list back to
 *					freelist.
 *
 * @soc: core txrx main context
 * @local_desc_list: local desc list provided by the caller
 * @tail: attach the point to last desc of local desc list
 * @pool_id: pool_id which is one of 3 mac_ids
 * @rx_desc_pool: rx descriptor pool pointer
 */
void dp_rx_add_desc_list_to_free_list(struct dp_soc *soc,
				union dp_rx_desc_list_elem_t **local_desc_list,
				union dp_rx_desc_list_elem_t **tail,
				uint16_t pool_id,
				struct rx_desc_pool *rx_desc_pool)
{
	union dp_rx_desc_list_elem_t *temp_list = NULL;

	qdf_spin_lock_bh(&rx_desc_pool->lock);


	temp_list = rx_desc_pool->freelist;
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
	"temp_list: %pK, *local_desc_list: %pK, *tail: %pK (*tail)->next: %pK",
	temp_list, *local_desc_list, *tail, (*tail)->next);
	rx_desc_pool->freelist = *local_desc_list;
	(*tail)->next = temp_list;
	*tail = NULL;
	*local_desc_list = NULL;

	qdf_spin_unlock_bh(&rx_desc_pool->lock);
}

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

#include "dp_types.h"
#include "dp_rx.h"

/*
 * dp_rx_desc_pool_alloc() - create a pool of software rx_descs
 *			     at the time of dp rx initialization
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @pool_size: number of Rx descriptor in the pool
 * @rx_desc_pool: rx descriptor pool pointer
 *
 * return success or failure
 */
QDF_STATUS dp_rx_desc_pool_alloc(struct dp_soc *soc, uint32_t pool_id,
	uint32_t pool_size, struct rx_desc_pool *rx_desc_pool)
{
	uint32_t i;

	rx_desc_pool->array =
		qdf_mem_malloc(pool_size*sizeof(union dp_rx_desc_list_elem_t));

	if (!(rx_desc_pool->array)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s: RX Desc Pool[%d] allocation failed\n",
			__func__, pool_id);

		return QDF_STATUS_E_NOMEM;
	}

	qdf_spin_lock_bh(&soc->rx_desc_mutex[pool_id]);
	rx_desc_pool->pool_size = pool_size;

	/* link SW rx descs into a freelist */
	rx_desc_pool->freelist = &rx_desc_pool->array[0];
	for (i = 0; i < rx_desc_pool->pool_size-1; i++) {
		rx_desc_pool->array[i].next = &rx_desc_pool->array[i+1];
		rx_desc_pool->array[i].rx_desc.cookie = i | (pool_id << 18);
		rx_desc_pool->array[i].rx_desc.pool_id = pool_id;
		rx_desc_pool->array[i].rx_desc.nbuf = NULL;
	}

	rx_desc_pool->array[i].next = NULL;
	rx_desc_pool->array[i].rx_desc.cookie = i | (pool_id << 18);
	rx_desc_pool->array[i].rx_desc.pool_id = pool_id;
	rx_desc_pool->array[i].rx_desc.nbuf = NULL;
	qdf_spin_unlock_bh(&soc->rx_desc_mutex[pool_id]);
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_desc_pool_free() - free the sw rx desc pool called during
 *			    de-initialization of wifi module.
 *
 * @soc: core txrx main context
 * @pool_id: pool_id which is one of 3 mac_ids
 * @rx_desc_pool: rx descriptor pool pointer
 */
void dp_rx_desc_pool_free(struct dp_soc *soc, uint32_t pool_id,
	struct rx_desc_pool *rx_desc_pool)
{
	int i;

	qdf_spin_lock_bh(&soc->rx_desc_mutex[pool_id]);
	for (i = 0; i < rx_desc_pool->pool_size; i++) {
		if (rx_desc_pool->array[i].rx_desc.nbuf)
			qdf_nbuf_free(rx_desc_pool->array[i].rx_desc.nbuf);
	}
	qdf_mem_free(rx_desc_pool->array);
	qdf_spin_unlock_bh(&soc->rx_desc_mutex[pool_id]);
}

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

	qdf_spin_lock_bh(&soc->rx_desc_mutex[pool_id]);

	*desc_list = rx_desc_pool->freelist;

	if (!(*desc_list)) {
		qdf_spin_unlock_bh(&soc->rx_desc_mutex[pool_id]);
		return 0;
	}

	for (count = 0; count < num_descs; count++) {

		*tail = rx_desc_pool->freelist;
		rx_desc_pool->freelist = rx_desc_pool->freelist->next;
		if (qdf_unlikely(!rx_desc_pool->freelist)) {
			qdf_spin_unlock_bh(&soc->rx_desc_mutex[pool_id]);
			return count;
		}
	}
	qdf_spin_unlock_bh(&soc->rx_desc_mutex[pool_id]);
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

	qdf_spin_lock_bh(&soc->rx_desc_mutex[pool_id]);


	temp_list = rx_desc_pool->freelist;
	qdf_print(
	"temp_list: %p, *local_desc_list: %p, *tail: %p (*tail)->next: %p\n",
	temp_list, *local_desc_list, *tail, (*tail)->next);
	rx_desc_pool->freelist = *local_desc_list;
	(*tail)->next = temp_list;

	qdf_spin_unlock_bh(&soc->rx_desc_mutex[pool_id]);
}

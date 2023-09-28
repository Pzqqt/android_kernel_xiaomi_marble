/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021, 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <dp_types.h>
#include "dp_rx.h"
#include "dp_peer.h"
#include <dp_htt.h>
#include <dp_mon_filter.h>
#include <dp_mon.h>
#include <dp_rx_mon.h>
#include <dp_rx_mon_2.0.h>
#include <dp_mon_2.0.h>
#include <dp_mon_filter_2.0.h>
#include <dp_tx_mon_2.0.h>
#include <hal_be_api_mon.h>
#include <dp_be.h>

extern QDF_STATUS dp_srng_alloc(struct dp_soc *soc, struct dp_srng *srng,
				int ring_type, uint32_t num_entries,
				bool cached);
extern void dp_srng_free(struct dp_soc *soc, struct dp_srng *srng);
extern QDF_STATUS dp_srng_init(struct dp_soc *soc, struct dp_srng *srng,
			       int ring_type, int ring_num, int mac_id);
extern void dp_srng_deinit(struct dp_soc *soc, struct dp_srng *srng,
			   int ring_type, int ring_num);

#if !defined(DISABLE_MON_CONFIG)
/*
 * dp_mon_add_desc_list_to_free_list() - append unused desc_list back to
 *					freelist.
 *
 * @soc: core txrx main context
 * @local_desc_list: local desc list provided by the caller
 * @tail: attach the point to last desc of local desc list
 * @mon_desc_pool: monitor descriptor pool pointer
 */
static void
dp_mon_add_desc_list_to_free_list(struct dp_soc *soc,
				  union dp_mon_desc_list_elem_t **local_desc_list,
				  union dp_mon_desc_list_elem_t **tail,
				  struct dp_mon_desc_pool *mon_desc_pool)
{
	union dp_mon_desc_list_elem_t *temp_list = NULL;

	qdf_spin_lock_bh(&mon_desc_pool->lock);

	temp_list = mon_desc_pool->freelist;
	mon_desc_pool->freelist = *local_desc_list;
	(*tail)->next = temp_list;
	*tail = NULL;
	*local_desc_list = NULL;

	qdf_spin_unlock_bh(&mon_desc_pool->lock);
}

/*
 * dp_mon_get_free_desc_list() - provide a list of descriptors from
 *				the free mon desc pool.
 *
 * @soc: core txrx main context
 * @mon_desc_pool: monitor descriptor pool pointer
 * @num_descs: number of descs requested from freelist
 * @desc_list: attach the descs to this list (output parameter)
 * @tail: attach the point to last desc of free list (output parameter)
 *
 * Return: number of descs allocated from free list.
 */
static uint16_t
dp_mon_get_free_desc_list(struct dp_soc *soc,
			  struct dp_mon_desc_pool *mon_desc_pool,
			  uint16_t num_descs,
			  union dp_mon_desc_list_elem_t **desc_list,
			  union dp_mon_desc_list_elem_t **tail)
{
	uint16_t count;

	qdf_spin_lock_bh(&mon_desc_pool->lock);

	*desc_list = *tail = mon_desc_pool->freelist;

	for (count = 0; count < num_descs; count++) {
		if (qdf_unlikely(!mon_desc_pool->freelist)) {
			qdf_spin_unlock_bh(&mon_desc_pool->lock);
			return count;
		}
		*tail = mon_desc_pool->freelist;
		mon_desc_pool->freelist = mon_desc_pool->freelist->next;
	}
	(*tail)->next = NULL;
	qdf_spin_unlock_bh(&mon_desc_pool->lock);
	return count;
}

void dp_mon_pool_frag_unmap_and_free(struct dp_soc *soc,
				     struct dp_mon_desc_pool *mon_desc_pool)
{
	int desc_id;
	qdf_frag_t vaddr;
	qdf_dma_addr_t paddr;

	qdf_spin_lock_bh(&mon_desc_pool->lock);
	for (desc_id = 0; desc_id < mon_desc_pool->pool_size; desc_id++) {
		if (mon_desc_pool->array[desc_id].mon_desc.in_use) {
			vaddr = mon_desc_pool->array[desc_id].mon_desc.buf_addr;
			paddr = mon_desc_pool->array[desc_id].mon_desc.paddr;

			if (!(mon_desc_pool->array[desc_id].mon_desc.unmapped)) {
				qdf_mem_unmap_page(soc->osdev, paddr,
						   QDF_DMA_FROM_DEVICE,
						   mon_desc_pool->buf_size);
				mon_desc_pool->array[desc_id].mon_desc.unmapped = 1;
				mon_desc_pool->array[desc_id].mon_desc.cookie = desc_id;
			}
			qdf_frag_free(vaddr);
		}
	}
	qdf_spin_unlock_bh(&mon_desc_pool->lock);
}

static inline QDF_STATUS
dp_mon_frag_alloc_and_map(struct dp_soc *dp_soc,
			  struct dp_mon_desc *mon_desc,
			  struct dp_mon_desc_pool *mon_desc_pool)
{
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	mon_desc->buf_addr = qdf_frag_alloc(mon_desc_pool->buf_size);

	if (!mon_desc->buf_addr) {
		dp_mon_err("Frag alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	ret = qdf_mem_map_page(dp_soc->osdev,
			       mon_desc->buf_addr,
			       QDF_DMA_FROM_DEVICE,
			       mon_desc_pool->buf_size,
			       &mon_desc->paddr);

	if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
		qdf_frag_free(mon_desc->buf_addr);
		dp_mon_err("Frag map failed");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
dp_mon_buffers_replenish(struct dp_soc *dp_soc,
			 struct dp_srng *dp_mon_srng,
			 struct dp_mon_desc_pool *mon_desc_pool,
			 uint32_t num_req_buffers,
			 union dp_mon_desc_list_elem_t **desc_list,
			 union dp_mon_desc_list_elem_t **tail)
{
	uint32_t num_alloc_desc;
	uint16_t num_desc_to_free = 0;
	uint32_t num_entries_avail;
	uint32_t count = 0;
	int sync_hw_ptr = 1;
	struct dp_mon_desc mon_desc = {0};
	void *mon_ring_entry;
	union dp_mon_desc_list_elem_t *next;
	void *mon_srng;
	QDF_STATUS ret = QDF_STATUS_E_FAILURE;

	mon_srng = dp_mon_srng->hal_srng;

	/*
	 * if desc_list is NULL, allocate the descs from freelist
	 */
	if (!(*desc_list)) {
		num_alloc_desc = dp_mon_get_free_desc_list(dp_soc,
							   mon_desc_pool,
							   num_req_buffers,
							   desc_list,
							   tail);

		if (!num_alloc_desc) {
			dp_mon_err("%pK: no free rx_descs in freelist", dp_soc);
			return QDF_STATUS_E_NOMEM;
		}

		dp_mon_info("%pK: %d rx desc allocated",
			    dp_soc, num_alloc_desc);

		num_req_buffers = num_alloc_desc;
	}

	hal_srng_access_start(dp_soc->hal_soc, mon_srng);
	num_entries_avail = hal_srng_src_num_avail(dp_soc->hal_soc,
						   mon_srng, sync_hw_ptr);

	if (num_entries_avail < num_req_buffers) {
		num_desc_to_free = num_req_buffers - num_entries_avail;
		num_req_buffers = num_entries_avail;
	}

	while (count <= num_req_buffers) {
		ret = dp_mon_frag_alloc_and_map(dp_soc,
						&mon_desc,
						mon_desc_pool);

		if (qdf_unlikely(QDF_IS_STATUS_ERROR(ret))) {
			if (qdf_unlikely(ret  == QDF_STATUS_E_FAULT))
				continue;
			break;
		}

		count++;
		next = (*desc_list)->next;
		mon_ring_entry = hal_srng_src_get_next(
						dp_soc->hal_soc,
						mon_srng);

		qdf_assert_always((*desc_list)->mon_desc.in_use == 0);

		(*desc_list)->mon_desc.in_use = 1;
		(*desc_list)->mon_desc.unmapped = 0;

		hal_mon_buff_addr_info_set(dp_soc->hal_soc,
					   mon_ring_entry,
					   &((*desc_list)->mon_desc),
					   mon_desc.paddr);

		*desc_list = next;
	}

	hal_srng_access_end(dp_soc->hal_soc, mon_srng);

	/*
	 * add any available free desc back to the free list
	 */
	if (*desc_list) {
		dp_mon_add_desc_list_to_free_list(dp_soc, desc_list, tail,
						  mon_desc_pool);
	}

	return ret;
}

QDF_STATUS dp_mon_desc_pool_init(struct dp_mon_desc_pool *mon_desc_pool)
{
	int desc_id;
	/* Initialize monitor desc lock */
	qdf_spinlock_create(&mon_desc_pool->lock);

	qdf_spin_lock_bh(&mon_desc_pool->lock);

	mon_desc_pool->buf_size = DP_MON_DATA_BUFFER_SIZE;
	/* link SW descs into a freelist */
	mon_desc_pool->freelist = &mon_desc_pool->array[0];
	qdf_mem_zero(mon_desc_pool->freelist, mon_desc_pool->pool_size);

	for (desc_id = 0; desc_id <= mon_desc_pool->pool_size - 1; desc_id++) {
		if (desc_id == mon_desc_pool->pool_size - 1)
			mon_desc_pool->array[desc_id].next = NULL;
		else
			mon_desc_pool->array[desc_id].next =
				&mon_desc_pool->array[desc_id + 1];
		mon_desc_pool->array[desc_id].mon_desc.in_use = 0;
		mon_desc_pool->array[desc_id].mon_desc.cookie = desc_id;
	}
	qdf_spin_unlock_bh(&mon_desc_pool->lock);

	return QDF_STATUS_SUCCESS;
}

void dp_mon_desc_pool_deinit(struct dp_mon_desc_pool *mon_desc_pool)
{
	qdf_spin_lock_bh(&mon_desc_pool->lock);

	mon_desc_pool->freelist = NULL;
	mon_desc_pool->pool_size = 0;

	qdf_spin_unlock_bh(&mon_desc_pool->lock);
	qdf_spinlock_destroy(&mon_desc_pool->lock);
}

void dp_mon_desc_pool_free(struct dp_mon_desc_pool *mon_desc_pool)
{
	qdf_mem_free(mon_desc_pool->array);
}

QDF_STATUS dp_mon_desc_pool_alloc(uint32_t pool_size,
				  struct dp_mon_desc_pool *mon_desc_pool)
{
	mon_desc_pool->pool_size = pool_size;
	mon_desc_pool->array = qdf_mem_malloc(pool_size *
				     sizeof(union dp_mon_desc_list_elem_t));

	return QDF_STATUS_SUCCESS;
}

static
void dp_vdev_set_monitor_mode_buf_rings_2_0(struct dp_pdev *pdev)
{
}

static
QDF_STATUS dp_vdev_set_monitor_mode_rings_2_0(struct dp_pdev *pdev,
					      uint8_t delayed_replenish)
{
	return QDF_STATUS_SUCCESS;
}
#else
static inline
void dp_vdev_set_monitor_mode_buf_rings_2_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_vdev_set_monitor_mode_rings_2_0(struct dp_pdev *pdev,
					      uint8_t delayed_replenish)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef QCA_SUPPORT_BPR
static QDF_STATUS
dp_set_bpr_enable_2_0(struct dp_pdev *pdev, int val)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifndef DISABLE_MON_CONFIG
static
QDF_STATUS dp_mon_soc_htt_srng_setup_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	QDF_STATUS status;

	status = htt_srng_setup(soc->htt_handle, 0,
				soc->rxdma_mon_buf_ring[0].hal_srng,
				RXDMA_MONITOR_BUF);

	if (status != QDF_STATUS_SUCCESS) {
		dp_err("Failed to send htt srng setup message for Rx mon buf ring");
		return status;
	}

	status = htt_srng_setup(soc->htt_handle, 0,
				mon_soc->tx_mon_buf_ring.hal_srng,
				TX_MONITOR_BUF);

	if (status != QDF_STATUS_SUCCESS) {
		dp_err("Failed to send htt srng setup message for Tx mon buf ring");
		return status;
	}

	return status;
}

static
QDF_STATUS dp_mon_pdev_htt_srng_setup_2_0(struct dp_soc *soc,
					  struct dp_pdev *pdev,
				     int mac_id,
				     int mac_for_pdev)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;
	QDF_STATUS status;

	status = htt_srng_setup(soc->htt_handle, mac_for_pdev,
				soc->rxdma_mon_dst_ring[mac_id].hal_srng,
				RXDMA_MONITOR_DST);

	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to send htt srng setup message for Rxdma dst ring");
		return status;
	}

	status = htt_srng_setup(soc->htt_handle, mac_for_pdev,
				mon_soc->tx_mon_dst_ring[mac_id].hal_srng,
				TX_MONITOR_DST);

	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to send htt srng message for Tx mon dst ring");
		return status;
	}

	return status;
}

static uint32_t
dp_rx_mon_process_2_0(struct dp_soc *soc, struct dp_intr *int_ctx,
		      uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static
QDF_STATUS dp_mon_soc_detach_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	dp_tx_mon_buffers_free(soc);
	dp_rx_mon_buffers_free(soc);
	dp_tx_mon_buf_desc_pool_free(soc);
	dp_rx_mon_buf_desc_pool_free(soc);

	if (mon_soc) {
		dp_srng_free(soc, &soc->rxdma_mon_buf_ring[0]);
		dp_srng_free(soc, &mon_soc->tx_mon_buf_ring);
		qdf_mem_free(be_soc->monitor_soc_be);
	}

	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_soc_attach_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = NULL;
	int entries;
	struct wlan_cfg_dp_soc_ctxt *soc_cfg_ctx;

	soc_cfg_ctx = soc->wlan_cfg_ctx;
	mon_soc = (struct dp_mon_soc_be *)qdf_mem_malloc(sizeof(*mon_soc));
	if (!mon_soc) {
		dp_mon_err("%pK: mem allocation failed", soc);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(mon_soc, sizeof(*mon_soc));
	be_soc->monitor_soc_be = mon_soc;

	entries = wlan_cfg_get_dp_soc_rx_mon_buf_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &soc->rxdma_mon_buf_ring[0],
			  RXDMA_MONITOR_BUF, entries, 0)) {
		dp_mon_err("%pK: " RNG_ERR "rx_mon_buf_ring", soc);
		goto fail;
	}

	entries = wlan_cfg_get_dp_soc_tx_mon_buf_ring_size(soc_cfg_ctx);
	if (dp_srng_alloc(soc, &be_soc->monitor_soc_be->tx_mon_buf_ring,
			  TX_MONITOR_BUF, entries, 0)) {
		dp_mon_err("%pK: " RNG_ERR "tx_mon_buf_ring", soc);
		goto fail;
	}

	/* sw desc pool for src ring */
	if (dp_rx_mon_buf_desc_pool_alloc(soc)) {
		dp_mon_err("%pK: Rx mon desc pool allocation failed", soc);
		goto fail;
	}

	if (dp_tx_mon_buf_desc_pool_alloc(soc)) {
		dp_mon_err("%pK: Tx mon desc pool allocation failed", soc);
		goto fail;
	}

	/* monitor buffers for src */
	if (dp_rx_mon_buffers_alloc(soc)) {
		dp_mon_err("%pK: Rx mon buffers allocation failed", soc);
		goto fail;
	}

	if (dp_tx_mon_buffers_alloc(soc)) {
		dp_mon_err("%pK: Tx mon buffers allocation failed", soc);
		goto fail;
	}

	return QDF_STATUS_SUCCESS;
fail:
	dp_mon_soc_detach_2_0(soc);
	return QDF_STATUS_E_NOMEM;
}

static
QDF_STATUS dp_mon_soc_deinit_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	dp_tx_mon_buf_desc_pool_deinit(soc);
	dp_rx_mon_buf_desc_pool_deinit(soc);

	dp_srng_deinit(soc, &soc->rxdma_mon_buf_ring[0],
		       RXDMA_MONITOR_BUF, 0);

	dp_srng_deinit(soc, &mon_soc->tx_mon_buf_ring,
		       TX_MONITOR_BUF, 0);
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS dp_mon_soc_init_2_0(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	if (dp_srng_init(soc, &soc->rxdma_mon_buf_ring[0],
			 RXDMA_MONITOR_BUF, 0, 0)) {
		dp_mon_err("%pK: " RNG_ERR "rx_mon_buf_ring", soc);
		goto fail;
	}

	if (dp_srng_init(soc, &mon_soc->tx_mon_buf_ring,
			 TX_MONITOR_BUF, 0, 0)) {
		dp_mon_err("%pK: " RNG_ERR "tx_mon_buf_ring", soc);
		goto fail;
	}

	if (dp_tx_mon_buf_desc_pool_init(soc)) {
		dp_mon_err("%pK: " RNG_ERR "tx mon desc pool init", soc);
		goto fail;
	}

	if (dp_rx_mon_buf_desc_pool_init(soc)) {
		dp_mon_err("%pK: " RNG_ERR "rx mon desc pool init", soc);
		goto fail;
	}

	return QDF_STATUS_SUCCESS;
fail:
	dp_mon_soc_deinit_2_0(soc);
	return QDF_STATUS_E_FAILURE;
}

static
void dp_pdev_mon_rings_deinit_2_0(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct dp_soc *soc = pdev->soc;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	for (mac_id = 0; mac_id < DP_NUM_MACS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_deinit(soc, &soc->rxdma_mon_dst_ring[lmac_id],
			       RXDMA_MONITOR_DST, 0);
		dp_srng_deinit(soc, &mon_soc->tx_mon_dst_ring[lmac_id],
			       TX_MONITOR_DST, 0);
	}
}

static
QDF_STATUS dp_pdev_mon_rings_init_2_0(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	int mac_id = 0;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	for (mac_id = 0; mac_id < DP_NUM_MACS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		if (dp_srng_init(soc, &soc->rxdma_mon_dst_ring[lmac_id],
				 RXDMA_MONITOR_DST, 0, lmac_id)) {
			dp_mon_err("%pK: " RNG_ERR "rxdma_mon_dst_ring", soc);
			goto fail;
		}

		if (dp_srng_init(soc, &mon_soc->tx_mon_dst_ring[lmac_id],
				 TX_MONITOR_DST, 0, lmac_id)) {
			dp_mon_err("%pK: " RNG_ERR "tx_mon_dst_ring", soc);
			goto fail;
		}
	}
	return QDF_STATUS_SUCCESS;

fail:
	dp_pdev_mon_rings_deinit_2_0(pdev);
	return QDF_STATUS_E_NOMEM;
}

static
void dp_pdev_mon_rings_free_2_0(struct dp_pdev *pdev)
{
	int mac_id = 0;
	struct dp_soc *soc = pdev->soc;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	for (mac_id = 0; mac_id < DP_NUM_MACS_PER_PDEV; mac_id++) {
		int lmac_id = dp_get_lmac_id_for_pdev_id(soc, mac_id,
							 pdev->pdev_id);

		dp_srng_free(soc, &soc->rxdma_mon_dst_ring[lmac_id]);
		dp_srng_free(soc, &mon_soc->tx_mon_dst_ring[lmac_id]);
	}
}

static
QDF_STATUS dp_pdev_mon_rings_alloc_2_0(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;
	int mac_id = 0;
	int entries;
	struct wlan_cfg_dp_pdev_ctxt *pdev_cfg_ctx;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	pdev_cfg_ctx = pdev->wlan_cfg_ctx;

	for (mac_id = 0; mac_id < DP_NUM_MACS_PER_PDEV; mac_id++) {
		int lmac_id =
		dp_get_lmac_id_for_pdev_id(soc, mac_id, pdev->pdev_id);

		entries = wlan_cfg_get_dma_rx_mon_dest_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &soc->rxdma_mon_dst_ring[lmac_id],
				  RXDMA_MONITOR_DST, entries, 0)) {
			dp_err("%pK: " RNG_ERR "rxdma_mon_dst_ring", pdev);
			goto fail;
		}

		entries = wlan_cfg_get_dma_tx_mon_dest_ring_size(pdev_cfg_ctx);
		if (dp_srng_alloc(soc, &mon_soc->tx_mon_dst_ring[lmac_id],
				  TX_MONITOR_DST, entries, 0)) {
			dp_err("%pK: " RNG_ERR "tx_mon_dst_ring", pdev);
			goto fail;
		}
	}
	return QDF_STATUS_SUCCESS;

fail:
	dp_pdev_mon_rings_free_2_0(pdev);
	return QDF_STATUS_E_NOMEM;
}

static
void dp_mon_pdev_free_2_0(struct dp_pdev *pdev)
{
	struct dp_pdev_be *be_pdev = dp_get_be_pdev_from_dp_pdev(pdev);

	qdf_mem_free(be_pdev->monitor_pdev_be);
	be_pdev->monitor_pdev_be = NULL;
}

static
QDF_STATUS dp_mon_pdev_alloc_2_0(struct dp_pdev *pdev)
{
	struct dp_mon_pdev_be *mon_pdev = NULL;
	struct dp_pdev_be *be_pdev = dp_get_be_pdev_from_dp_pdev(pdev);

	mon_pdev = (struct dp_mon_pdev_be *)qdf_mem_malloc(sizeof(*mon_pdev));
	if (!mon_pdev) {
		dp_mon_err("%pK: mem allocation failed", pdev);
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_zero(mon_pdev, sizeof(*mon_pdev));
	be_pdev->monitor_pdev_be = mon_pdev;

	return QDF_STATUS_SUCCESS;
}
#else
static inline
QDF_STATUS dp_mon_htt_srng_setup_2_0(struct dp_soc *soc,
				     struct dp_pdev *pdev,
				     int mac_id,
				     int mac_for_pdev)
{
	return QDF_STATUS_SUCCESS;
}

static uint32_t
dp_rx_mon_process_2_0(struct dp_soc *soc, struct dp_intr *int_ctx,
		      uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static uint32_t
dp_tx_mon_process(struct dp_soc *soc, struct dp_intr *int_ctx,
		  uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static inline
QDF_STATUS dp_mon_soc_attach_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
QDF_STATUS dp_mon_soc_detach_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
QDF_STATUS dp_mon_soc_init_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
QDF_STATUS dp_mon_soc_deinit_2_0(struct dp_soc *soc)
{
	return status;
}

static inline
void dp_pdev_mon_rings_deinit_2_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_pdev_mon_rings_init_2_0(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void dp_pdev_mon_rings_free_2_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_pdev_mon_rings_alloc_2_0(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void dp_mon_pdev_free_2_0(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_mon_pdev_alloc_2_0(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static void dp_mon_register_intr_ops_2_0(struct dp_soc *soc)
{
	struct dp_mon_soc *mon_soc = soc->monitor_soc;

	mon_soc->mon_rx_process = dp_rx_mon_process_2_0;
}

/**
 * dp_mon_register_feature_ops_2_0() - register feature ops
 *
 * @soc: dp soc context
 *
 * @return: void
 */
static void
dp_mon_register_feature_ops_2_0(struct dp_soc *soc)
{
	struct dp_mon_ops *mon_ops = dp_mon_ops_get(soc);

	if (!mon_ops) {
		dp_err("mon_ops is NULL, feature ops registration failed");
		return;
	}

	mon_ops->mon_config_debug_sniffer = dp_config_debug_sniffer;
	mon_ops->mon_peer_tx_init = NULL;
	mon_ops->mon_peer_tx_cleanup = NULL;
	mon_ops->mon_htt_ppdu_stats_attach = dp_htt_ppdu_stats_attach;
	mon_ops->mon_htt_ppdu_stats_detach = dp_htt_ppdu_stats_detach;
	mon_ops->mon_print_pdev_rx_mon_stats = dp_print_pdev_rx_mon_stats;
	mon_ops->mon_set_bsscolor = dp_mon_set_bsscolor;
	mon_ops->mon_pdev_get_filter_ucast_data =
				dp_pdev_get_filter_ucast_data;
	mon_ops->mon_pdev_get_filter_mcast_data =
				dp_pdev_get_filter_mcast_data;
	mon_ops->mon_pdev_get_filter_non_data = dp_pdev_get_filter_non_data;
	mon_ops->mon_neighbour_peer_add_ast = NULL;
#ifndef DISABLE_MON_CONFIG
	mon_ops->mon_tx_process = dp_tx_mon_process;
#endif
#ifdef WLAN_TX_PKT_CAPTURE_ENH
	mon_ops->mon_peer_tid_peer_id_update = NULL;
	mon_ops->mon_tx_ppdu_stats_attach = dp_tx_ppdu_stats_attach;
	mon_ops->mon_tx_ppdu_stats_detach = dp_tx_ppdu_stats_detach;
	mon_ops->mon_tx_capture_debugfs_init = NULL;
	mon_ops->mon_tx_add_to_comp_queue = NULL;
	mon_ops->mon_peer_tx_capture_filter_check = NULL;
	mon_ops->mon_print_pdev_tx_capture_stats = NULL;
	mon_ops->mon_config_enh_tx_capture = dp_config_enh_tx_capture;
#endif
#if defined(WDI_EVENT_ENABLE) &&\
	(defined(QCA_ENHANCED_STATS_SUPPORT) || !defined(REMOVE_PKT_LOG))
	mon_ops->mon_ppdu_stats_ind_handler = dp_ppdu_stats_ind_handler;
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	mon_ops->mon_config_enh_rx_capture = NULL;
#endif
#ifdef QCA_SUPPORT_BPR
	mon_ops->mon_set_bpr_enable = dp_set_bpr_enable_2_0;
#endif
#ifdef ATH_SUPPORT_NAC
	mon_ops->mon_set_filter_neigh_peers = dp_set_filter_neigh_peers;
#endif
#ifdef WLAN_ATF_ENABLE
	mon_ops->mon_set_atf_stats_enable = dp_set_atf_stats_enable;
#endif
#ifdef FEATURE_NAC_RSSI
	mon_ops->mon_filter_neighbour_peer = dp_filter_neighbour_peer;
#endif
#ifdef QCA_MCOPY_SUPPORT
	mon_ops->mon_filter_setup_mcopy_mode =
				dp_mon_filter_setup_mcopy_mode_2_0;
	mon_ops->mon_filter_reset_mcopy_mode =
				dp_mon_filter_reset_mcopy_mode_2_0;
	mon_ops->mon_mcopy_check_deliver = NULL;
#endif
#ifdef QCA_ENHANCED_STATS_SUPPORT
	mon_ops->mon_filter_setup_enhanced_stats =
				dp_mon_filter_setup_enhanced_stats_2_0;
	mon_ops->mon_filter_reset_enhanced_stats =
				dp_mon_filter_reset_enhanced_stats_2_0;
#endif
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	mon_ops->mon_filter_setup_smart_monitor =
				dp_mon_filter_setup_smart_monitor_2_0;
#endif
#ifdef WLAN_RX_PKT_CAPTURE_ENH
	mon_ops->mon_filter_setup_rx_enh_capture =
				dp_mon_filter_setup_rx_enh_capture_2_0;
#endif
#ifdef WDI_EVENT_ENABLE
	mon_ops->mon_set_pktlog_wifi3 = dp_set_pktlog_wifi3;
	mon_ops->mon_filter_setup_rx_pkt_log_full =
				dp_mon_filter_setup_rx_pkt_log_full_2_0;
	mon_ops->mon_filter_reset_rx_pkt_log_full =
				dp_mon_filter_reset_rx_pkt_log_full_2_0;
	mon_ops->mon_filter_setup_rx_pkt_log_lite =
				dp_mon_filter_setup_rx_pkt_log_lite_2_0;
	mon_ops->mon_filter_reset_rx_pkt_log_lite =
				dp_mon_filter_reset_rx_pkt_log_lite_2_0;
	mon_ops->mon_filter_setup_rx_pkt_log_cbf =
				dp_mon_filter_setup_rx_pkt_log_cbf_2_0;
	mon_ops->mon_filter_reset_rx_pkt_log_cbf =
				dp_mon_filter_reset_rx_pktlog_cbf_2_0;
	mon_ops->mon_filter_setup_pktlog_hybrid =
				dp_mon_filter_setup_pktlog_hybrid_2_0;
	mon_ops->mon_filter_reset_pktlog_hybrid =
				dp_mon_filter_reset_pktlog_hybrid_2_0;
#endif
#if defined(DP_CON_MON) && !defined(REMOVE_PKT_LOG)
	mon_ops->mon_pktlogmod_exit = dp_pktlogmod_exit;
#endif
	mon_ops->rx_packet_length_set = dp_rx_mon_packet_length_set;
	mon_ops->rx_wmask_subscribe = dp_rx_mon_word_mask_subscribe;
	mon_ops->rx_enable_mpdu_logging = dp_rx_mon_enable_mpdu_logging;
	mon_ops->mon_neighbour_peers_detach = dp_neighbour_peers_detach;
	mon_ops->mon_vdev_set_monitor_mode_buf_rings =
				dp_vdev_set_monitor_mode_buf_rings_2_0;
	mon_ops->mon_vdev_set_monitor_mode_rings =
				dp_vdev_set_monitor_mode_rings_2_0;
}

struct dp_mon_ops monitor_ops_2_0 = {
	.mon_soc_cfg_init = dp_mon_soc_cfg_init,
	.mon_soc_attach = dp_mon_soc_attach_2_0,
	.mon_soc_detach = dp_mon_soc_detach_2_0,
	.mon_soc_init = dp_mon_soc_init_2_0,
	.mon_soc_deinit = dp_mon_soc_deinit_2_0,
	.mon_pdev_alloc = dp_mon_pdev_alloc_2_0,
	.mon_pdev_free = dp_mon_pdev_free_2_0,
	.mon_pdev_attach = dp_mon_pdev_attach,
	.mon_pdev_detach = dp_mon_pdev_detach,
	.mon_pdev_init = dp_mon_pdev_init,
	.mon_pdev_deinit = dp_mon_pdev_deinit,
	.mon_vdev_attach = dp_mon_vdev_attach,
	.mon_vdev_detach = dp_mon_vdev_detach,
	.mon_peer_attach = dp_mon_peer_attach,
	.mon_peer_detach = dp_mon_peer_detach,
	.mon_flush_rings = NULL,
#if !defined(DISABLE_MON_CONFIG)
	.mon_pdev_htt_srng_setup = dp_mon_pdev_htt_srng_setup_2_0,
	.mon_soc_htt_srng_setup = dp_mon_soc_htt_srng_setup_2_0,
#endif
#if defined(DP_CON_MON)
	.mon_service_rings = NULL,
#endif
#ifndef DISABLE_MON_CONFIG
	.mon_rx_process = NULL,
#endif
#if !defined(DISABLE_MON_CONFIG) && defined(MON_ENABLE_DROP_FOR_MAC)
	.mon_drop_packets_for_mac = NULL,
#endif
	.mon_vdev_timer_init = NULL,
	.mon_vdev_timer_start = NULL,
	.mon_vdev_timer_stop = NULL,
	.mon_vdev_timer_deinit = NULL,
	.mon_reap_timer_init = NULL,
	.mon_reap_timer_start = NULL,
	.mon_reap_timer_stop = NULL,
	.mon_reap_timer_deinit = NULL,
	.mon_filter_setup_mon_mode = dp_mon_filter_setup_mon_mode_2_0,
	.mon_filter_reset_mon_mode = dp_mon_filter_reset_mon_mode_2_0,
	.mon_filter_update = dp_mon_filter_update_2_0,
	.mon_rings_alloc = dp_pdev_mon_rings_alloc_2_0,
	.mon_rings_free = dp_pdev_mon_rings_free_2_0,
	.mon_rings_init = dp_pdev_mon_rings_init_2_0,
	.mon_rings_deinit = dp_pdev_mon_rings_deinit_2_0,
	.rx_mon_desc_pool_init = NULL,
	.rx_mon_desc_pool_deinit = NULL,
	.rx_mon_desc_pool_alloc = NULL,
	.rx_mon_desc_pool_free = NULL,
	.rx_mon_buffers_alloc = NULL,
	.rx_mon_buffers_free = NULL,
	.tx_mon_desc_pool_init = NULL,
	.tx_mon_desc_pool_deinit = NULL,
	.tx_mon_desc_pool_alloc = NULL,
	.tx_mon_desc_pool_free = NULL,
#ifndef DISABLE_MON_CONFIG
	.mon_register_intr_ops = dp_mon_register_intr_ops_2_0,
#endif
	.mon_register_feature_ops = dp_mon_register_feature_ops_1_0,
};

struct cdp_mon_ops dp_ops_mon_2_0 = {
	.txrx_reset_monitor_mode = dp_reset_monitor_mode,
	/* Added support for HK advance filter */
	.txrx_set_advance_monitor_filter = dp_pdev_set_advance_monitor_filter,
	.txrx_deliver_tx_mgmt = dp_deliver_tx_mgmt,
	.config_full_mon_mode = NULL,
	.soc_config_full_mon_mode = NULL,
};

struct dp_mon_ops *dp_mon_ops_get_2_0(void)
{
	return &monitor_ops_2_0;
}

struct cdp_mon_ops *dp_mon_cdp_ops_get_2_0(void)
{
	return &dp_ops_mon_2_0;
}

/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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

#ifdef IPA_OFFLOAD

#include <qdf_ipa_wdi3.h>
#include <qdf_types.h>
#include <qdf_lock.h>
#include <hal_hw_headers.h>
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_types.h"
#include "dp_htt.h"
#include "dp_tx.h"
#include "dp_rx.h"
#include "dp_ipa.h"

/* Hard coded config parameters until dp_ops_cfg.cfg_attach implemented */
#define CFG_IPA_UC_TX_BUF_SIZE_DEFAULT            (2048)

/* WAR for IPA_OFFLOAD case. In some cases, its observed that WBM tries to
 * release a buffer into WBM2SW RELEASE ring for IPA, and the ring is full.
 * This causes back pressure, resulting in a FW crash.
 * By leaving some entries with no buffer attached, WBM will be able to write
 * to the ring, and from dumps we can figure out the buffer which is causing
 * this issue.
 */
#define DP_IPA_WAR_WBM2SW_REL_RING_NO_BUF_ENTRIES 16
/**
 *struct dp_ipa_reo_remap_record - history for dp ipa reo remaps
 * @ix0_reg: reo destination ring IX0 value
 * @ix2_reg: reo destination ring IX2 value
 * @ix3_reg: reo destination ring IX3 value
 */
struct dp_ipa_reo_remap_record {
	uint64_t timestamp;
	uint32_t ix0_reg;
	uint32_t ix2_reg;
	uint32_t ix3_reg;
};

#define REO_REMAP_HISTORY_SIZE 32

struct dp_ipa_reo_remap_record dp_ipa_reo_remap_history[REO_REMAP_HISTORY_SIZE];

static qdf_atomic_t dp_ipa_reo_remap_history_index;
static int dp_ipa_reo_remap_record_index_next(qdf_atomic_t *index)
{
	int next = qdf_atomic_inc_return(index);

	if (next == REO_REMAP_HISTORY_SIZE)
		qdf_atomic_sub(REO_REMAP_HISTORY_SIZE, index);

	return next % REO_REMAP_HISTORY_SIZE;
}

/**
 * dp_ipa_reo_remap_history_add() - Record dp ipa reo remap values
 * @ix0_val: reo destination ring IX0 value
 * @ix2_val: reo destination ring IX2 value
 * @ix3_val: reo destination ring IX3 value
 *
 * Return: None
 */
static void dp_ipa_reo_remap_history_add(uint32_t ix0_val, uint32_t ix2_val,
					 uint32_t ix3_val)
{
	int idx = dp_ipa_reo_remap_record_index_next(
				&dp_ipa_reo_remap_history_index);
	struct dp_ipa_reo_remap_record *record = &dp_ipa_reo_remap_history[idx];

	record->timestamp = qdf_get_log_timestamp();
	record->ix0_reg = ix0_val;
	record->ix2_reg = ix2_val;
	record->ix3_reg = ix3_val;
}

static QDF_STATUS __dp_ipa_handle_buf_smmu_mapping(struct dp_soc *soc,
						   qdf_nbuf_t nbuf,
						   uint32_t size,
						   bool create)
{
	qdf_mem_info_t mem_map_table = {0};

	if (!qdf_ipa_is_ready())
		return QDF_STATUS_SUCCESS;

	qdf_update_mem_map_table(soc->osdev, &mem_map_table,
				 qdf_nbuf_get_frag_paddr(nbuf, 0),
				 size);

	if (create)
		qdf_ipa_wdi_create_smmu_mapping(1, &mem_map_table);
	else
		qdf_ipa_wdi_release_smmu_mapping(1, &mem_map_table);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_handle_rx_buf_smmu_mapping(struct dp_soc *soc,
					     qdf_nbuf_t nbuf,
					     uint32_t size,
					     bool create)
{
	struct dp_pdev *pdev;
	int i;

	for (i = 0; i < soc->pdev_count; i++) {
		pdev = soc->pdev_list[i];
		if (pdev && pdev->monitor_configured)
			return QDF_STATUS_SUCCESS;
	}

	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx) ||
	    !qdf_mem_smmu_s1_enabled(soc->osdev))
		return QDF_STATUS_SUCCESS;

	/**
	 * Even if ipa pipes is disabled, but if it's unmap
	 * operation and nbuf has done ipa smmu map before,
	 * do ipa smmu unmap as well.
	 */
	if (!qdf_atomic_read(&soc->ipa_pipes_enabled)) {
		if (!create && qdf_nbuf_is_rx_ipa_smmu_map(nbuf)) {
			DP_STATS_INC(soc, rx.err.ipa_unmap_no_pipe, 1);
		} else {
			return QDF_STATUS_SUCCESS;
		}
	}

	if (qdf_unlikely(create == qdf_nbuf_is_rx_ipa_smmu_map(nbuf))) {
		if (create) {
			DP_STATS_INC(soc, rx.err.ipa_smmu_map_dup, 1);
		} else {
			DP_STATS_INC(soc, rx.err.ipa_smmu_unmap_dup, 1);
		}
		return QDF_STATUS_E_INVAL;
	}

	qdf_nbuf_set_rx_ipa_smmu_map(nbuf, create);

	return __dp_ipa_handle_buf_smmu_mapping(soc, nbuf, size, create);
}

static QDF_STATUS __dp_ipa_tx_buf_smmu_mapping(
	struct dp_soc *soc,
	struct dp_pdev *pdev,
	bool create)
{
	uint32_t index;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	uint32_t tx_buffer_cnt = soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt;
	qdf_nbuf_t nbuf;
	uint32_t buf_len;

	for (index = 0; index < tx_buffer_cnt; index++) {
		nbuf = (qdf_nbuf_t)
			soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[index];
		if (!nbuf)
			continue;
		buf_len = qdf_nbuf_get_data_len(nbuf);
		ret = __dp_ipa_handle_buf_smmu_mapping(
				soc, nbuf, buf_len, true);
	}
	return ret;
}

#ifdef RX_DESC_MULTI_PAGE_ALLOC
static QDF_STATUS dp_ipa_handle_rx_buf_pool_smmu_mapping(struct dp_soc *soc,
							 struct dp_pdev *pdev,
							 bool create)
{
	struct rx_desc_pool *rx_pool;
	uint8_t pdev_id;
	uint32_t num_desc, page_id, offset, i;
	uint16_t num_desc_per_page;
	union dp_rx_desc_list_elem_t *rx_desc_elem;
	struct dp_rx_desc *rx_desc;
	qdf_nbuf_t nbuf;

	if (!qdf_mem_smmu_s1_enabled(soc->osdev))
		return QDF_STATUS_SUCCESS;

	pdev_id = pdev->pdev_id;
	rx_pool = &soc->rx_desc_buf[pdev_id];

	qdf_spin_lock_bh(&rx_pool->lock);
	num_desc = rx_pool->pool_size;
	num_desc_per_page = rx_pool->desc_pages.num_element_per_page;
	for (i = 0; i < num_desc; i++) {
		page_id = i / num_desc_per_page;
		offset = i % num_desc_per_page;
		if (qdf_unlikely(!(rx_pool->desc_pages.cacheable_pages)))
			break;
		rx_desc_elem = dp_rx_desc_find(page_id, offset, rx_pool);
		rx_desc = &rx_desc_elem->rx_desc;
		if ((!(rx_desc->in_use)) || rx_desc->unmapped)
			continue;
		nbuf = rx_desc->nbuf;

		if (qdf_unlikely(create ==
				 qdf_nbuf_is_rx_ipa_smmu_map(nbuf))) {
			if (create) {
				DP_STATS_INC(soc,
					     rx.err.ipa_smmu_map_dup, 1);
			} else {
				DP_STATS_INC(soc,
					     rx.err.ipa_smmu_unmap_dup, 1);
			}
			continue;
		}
		qdf_nbuf_set_rx_ipa_smmu_map(nbuf, create);

		__dp_ipa_handle_buf_smmu_mapping(soc, nbuf,
						 rx_pool->buf_size, create);
	}
	qdf_spin_unlock_bh(&rx_pool->lock);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS dp_ipa_handle_rx_buf_pool_smmu_mapping(struct dp_soc *soc,
							 struct dp_pdev *pdev,
							 bool create)
{
	struct rx_desc_pool *rx_pool;
	uint8_t pdev_id;
	qdf_nbuf_t nbuf;
	int i;

	if (!qdf_mem_smmu_s1_enabled(soc->osdev))
		return QDF_STATUS_SUCCESS;

	pdev_id = pdev->pdev_id;
	rx_pool = &soc->rx_desc_buf[pdev_id];

	qdf_spin_lock_bh(&rx_pool->lock);
	for (i = 0; i < rx_pool->pool_size; i++) {
		if ((!(rx_pool->array[i].rx_desc.in_use)) ||
		    rx_pool->array[i].rx_desc.unmapped)
			continue;

		nbuf = rx_pool->array[i].rx_desc.nbuf;

		if (qdf_unlikely(create ==
				 qdf_nbuf_is_rx_ipa_smmu_map(nbuf))) {
			if (create) {
				DP_STATS_INC(soc,
					     rx.err.ipa_smmu_map_dup, 1);
			} else {
				DP_STATS_INC(soc,
					     rx.err.ipa_smmu_unmap_dup, 1);
			}
			continue;
		}
		qdf_nbuf_set_rx_ipa_smmu_map(nbuf, create);

		__dp_ipa_handle_buf_smmu_mapping(soc, nbuf,
						 rx_pool->buf_size, create);
	}
	qdf_spin_unlock_bh(&rx_pool->lock);

	return QDF_STATUS_SUCCESS;
}
#endif /* RX_DESC_MULTI_PAGE_ALLOC */

/**
 * dp_tx_ipa_uc_detach - Free autonomy TX resources
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * Free allocated TX buffers with WBM SRNG
 *
 * Return: none
 */
static void dp_tx_ipa_uc_detach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int idx;
	qdf_nbuf_t nbuf;
	struct dp_ipa_resources *ipa_res;
	bool is_ipa_ready = qdf_ipa_is_ready();
	uint32_t buf_len;

	for (idx = 0; idx < soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt; idx++) {
		nbuf = (qdf_nbuf_t)
			soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[idx];
		if (!nbuf)
			continue;
		buf_len = qdf_nbuf_get_data_len(nbuf);
		if (qdf_mem_smmu_s1_enabled(soc->osdev) && is_ipa_ready)
			__dp_ipa_handle_buf_smmu_mapping(
					soc, nbuf, buf_len, false);

		qdf_nbuf_unmap_single(soc->osdev, nbuf, QDF_DMA_BIDIRECTIONAL);
		qdf_nbuf_free(nbuf);
		soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[idx] =
						(void *)NULL;
	}

	qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned);
	soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned = NULL;

	ipa_res = &pdev->ipa_resource;
	if (!ipa_res->is_db_ddr_mapped)
		iounmap(ipa_res->tx_comp_doorbell_vaddr);

	qdf_mem_free_sgtable(&ipa_res->tx_ring.sgtable);
	qdf_mem_free_sgtable(&ipa_res->tx_comp_ring.sgtable);
}

/**
 * dp_rx_ipa_uc_detach - free autonomy RX resources
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * This function will detach DP RX into main device context
 * will free DP Rx resources.
 *
 * Return: none
 */
static void dp_rx_ipa_uc_detach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;

	qdf_mem_free_sgtable(&ipa_res->rx_rdy_ring.sgtable);
	qdf_mem_free_sgtable(&ipa_res->rx_refill_ring.sgtable);
}

int dp_ipa_uc_detach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	/* TX resource detach */
	dp_tx_ipa_uc_detach(soc, pdev);

	/* RX resource detach */
	dp_rx_ipa_uc_detach(soc, pdev);

	return QDF_STATUS_SUCCESS;	/* success */
}

/**
 * dp_tx_ipa_uc_attach - Allocate autonomy TX resources
 * @soc: data path instance
 * @pdev: Physical device handle
 *
 * Allocate TX buffer from non-cacheable memory
 * Attache allocated TX buffers with WBM SRNG
 *
 * Return: int
 */
static int dp_tx_ipa_uc_attach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	uint32_t tx_buffer_count;
	uint32_t ring_base_align = 8;
	qdf_dma_addr_t buffer_paddr;
	struct hal_srng *wbm_srng = (struct hal_srng *)
			soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	struct hal_srng_params srng_params;
	uint32_t paddr_lo;
	uint32_t paddr_hi;
	void *ring_entry;
	int num_entries;
	qdf_nbuf_t nbuf;
	int retval = QDF_STATUS_SUCCESS;
	int max_alloc_count = 0;

	/*
	 * Uncomment when dp_ops_cfg.cfg_attach is implemented
	 * unsigned int uc_tx_buf_sz =
	 *		dp_cfg_ipa_uc_tx_buf_size(pdev->osif_pdev);
	 */
	unsigned int uc_tx_buf_sz = CFG_IPA_UC_TX_BUF_SIZE_DEFAULT;
	unsigned int alloc_size = uc_tx_buf_sz + ring_base_align - 1;

	hal_get_srng_params(soc->hal_soc, hal_srng_to_hal_ring_handle(wbm_srng),
			    &srng_params);
	num_entries = srng_params.num_entries;

	max_alloc_count =
		num_entries - DP_IPA_WAR_WBM2SW_REL_RING_NO_BUF_ENTRIES;
	if (max_alloc_count <= 0) {
		dp_err("incorrect value for buffer count %u", max_alloc_count);
		return -EINVAL;
	}

	dp_info("requested %d buffers to be posted to wbm ring",
		max_alloc_count);

	soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned =
		qdf_mem_malloc(num_entries *
		sizeof(*soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned));
	if (!soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned) {
		dp_err("IPA WBM Ring Tx buf pool vaddr alloc fail");
		return -ENOMEM;
	}

	hal_srng_access_start_unlocked(soc->hal_soc,
				       hal_srng_to_hal_ring_handle(wbm_srng));

	/*
	 * Allocate Tx buffers as many as possible.
	 * Leave DP_IPA_WAR_WBM2SW_REL_RING_NO_BUF_ENTRIES empty
	 * Populate Tx buffers into WBM2IPA ring
	 * This initial buffer population will simulate H/W as source ring,
	 * and update HP
	 */
	for (tx_buffer_count = 0;
		tx_buffer_count < max_alloc_count - 1; tx_buffer_count++) {
		nbuf = qdf_nbuf_alloc(soc->osdev, alloc_size, 0, 256, FALSE);
		if (!nbuf)
			break;

		ring_entry = hal_srng_dst_get_next_hp(soc->hal_soc,
				hal_srng_to_hal_ring_handle(wbm_srng));
		if (!ring_entry) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				  "%s: Failed to get WBM ring entry",
				  __func__);
			qdf_nbuf_free(nbuf);
			break;
		}

		qdf_nbuf_map_single(soc->osdev, nbuf,
				    QDF_DMA_BIDIRECTIONAL);
		buffer_paddr = qdf_nbuf_get_frag_paddr(nbuf, 0);

		paddr_lo = ((uint64_t)buffer_paddr & 0x00000000ffffffff);
		paddr_hi = ((uint64_t)buffer_paddr & 0x0000001f00000000) >> 32;
		HAL_RXDMA_PADDR_LO_SET(ring_entry, paddr_lo);
		HAL_RXDMA_PADDR_HI_SET(ring_entry, paddr_hi);
		HAL_RXDMA_MANAGER_SET(ring_entry, (IPA_TCL_DATA_RING_IDX +
				      HAL_WBM_SW0_BM_ID));

		soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[tx_buffer_count]
			= (void *)nbuf;
	}

	hal_srng_access_end_unlocked(soc->hal_soc,
				     hal_srng_to_hal_ring_handle(wbm_srng));

	soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt = tx_buffer_count;

	if (tx_buffer_count) {
		dp_info("IPA WDI TX buffer: %d allocated", tx_buffer_count);
	} else {
		dp_err("No IPA WDI TX buffer allocated!");
		qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned);
		soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned = NULL;
		retval = -ENOMEM;
	}

	return retval;
}

/**
 * dp_rx_ipa_uc_attach - Allocate autonomy RX resources
 * @soc: data path instance
 * @pdev: core txrx pdev context
 *
 * This function will attach a DP RX instance into the main
 * device (SOC) context.
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
static int dp_rx_ipa_uc_attach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

int dp_ipa_uc_attach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	int error;

	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	/* TX resource attach */
	error = dp_tx_ipa_uc_attach(soc, pdev);
	if (error) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: DP IPA UC TX attach fail code %d",
			  __func__, error);
		return error;
	}

	/* RX resource attach */
	error = dp_rx_ipa_uc_attach(soc, pdev);
	if (error) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: DP IPA UC RX attach fail code %d",
			  __func__, error);
		dp_tx_ipa_uc_detach(soc, pdev);
		return error;
	}

	return QDF_STATUS_SUCCESS;	/* success */
}

/*
 * dp_ipa_ring_resource_setup() - setup IPA ring resources
 * @soc: data path SoC handle
 *
 * Return: none
 */
int dp_ipa_ring_resource_setup(struct dp_soc *soc,
		struct dp_pdev *pdev)
{
	struct hal_soc *hal_soc = (struct hal_soc *)soc->hal_soc;
	struct hal_srng *hal_srng;
	struct hal_srng_params srng_params;
	qdf_dma_addr_t hp_addr;
	unsigned long addr_offset, dev_base_paddr;
	uint32_t ix0;

	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	/* IPA TCL_DATA Ring - HAL_SRNG_SW2TCL3 */
	hal_srng = (struct hal_srng *)
			soc->tcl_data_ring[IPA_TCL_DATA_RING_IDX].hal_srng;
	hal_get_srng_params(hal_soc_to_hal_soc_handle(hal_soc),
			    hal_srng_to_hal_ring_handle(hal_srng),
			    &srng_params);

	soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_paddr =
		srng_params.ring_base_paddr;
	soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_vaddr =
		srng_params.ring_base_vaddr;
	soc->ipa_uc_tx_rsc.ipa_tcl_ring_size =
		(srng_params.num_entries * srng_params.entry_size) << 2;
	/*
	 * For the register backed memory addresses, use the scn->mem_pa to
	 * calculate the physical address of the shadow registers
	 */
	dev_base_paddr =
		(unsigned long)
		((struct hif_softc *)(hal_soc->hif_handle))->mem_pa;
	addr_offset = (unsigned long)(hal_srng->u.src_ring.hp_addr) -
		      (unsigned long)(hal_soc->dev_base_addr);
	soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr =
				(qdf_dma_addr_t)(addr_offset + dev_base_paddr);

	dp_info("IPA TCL_DATA Ring addr_offset=%x, dev_base_paddr=%x, hp_paddr=%x paddr=%pK vaddr=%pK size= %u(%u bytes)",
		(unsigned int)addr_offset,
		(unsigned int)dev_base_paddr,
		(unsigned int)(soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr),
		(void *)soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_paddr,
		(void *)soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_vaddr,
		srng_params.num_entries,
		soc->ipa_uc_tx_rsc.ipa_tcl_ring_size);

	/* IPA TX COMP Ring - HAL_SRNG_WBM2SW2_RELEASE */
	hal_srng = (struct hal_srng *)
			soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	hal_get_srng_params(hal_soc_to_hal_soc_handle(hal_soc),
			    hal_srng_to_hal_ring_handle(hal_srng),
			    &srng_params);

	soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_paddr =
						srng_params.ring_base_paddr;
	soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_vaddr =
						srng_params.ring_base_vaddr;
	soc->ipa_uc_tx_rsc.ipa_wbm_ring_size =
		(srng_params.num_entries * srng_params.entry_size) << 2;
	addr_offset = (unsigned long)(hal_srng->u.dst_ring.tp_addr) -
		      (unsigned long)(hal_soc->dev_base_addr);
	soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr =
				(qdf_dma_addr_t)(addr_offset + dev_base_paddr);

	dp_info("IPA TX COMP Ring addr_offset=%x, dev_base_paddr=%x, ipa_wbm_tp_paddr=%x paddr=%pK vaddr=0%pK size= %u(%u bytes)",
		(unsigned int)addr_offset,
		(unsigned int)dev_base_paddr,
		(unsigned int)(soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr),
		(void *)soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_paddr,
		(void *)soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_vaddr,
		srng_params.num_entries,
		soc->ipa_uc_tx_rsc.ipa_wbm_ring_size);

	/* IPA REO_DEST Ring - HAL_SRNG_REO2SW4 */
	hal_srng = (struct hal_srng *)
			soc->reo_dest_ring[IPA_REO_DEST_RING_IDX].hal_srng;
	hal_get_srng_params(hal_soc_to_hal_soc_handle(hal_soc),
			    hal_srng_to_hal_ring_handle(hal_srng),
			    &srng_params);

	soc->ipa_uc_rx_rsc.ipa_reo_ring_base_paddr =
						srng_params.ring_base_paddr;
	soc->ipa_uc_rx_rsc.ipa_reo_ring_base_vaddr =
						srng_params.ring_base_vaddr;
	soc->ipa_uc_rx_rsc.ipa_reo_ring_size =
		(srng_params.num_entries * srng_params.entry_size) << 2;
	addr_offset = (unsigned long)(hal_srng->u.dst_ring.tp_addr) -
		      (unsigned long)(hal_soc->dev_base_addr);
	soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr =
				(qdf_dma_addr_t)(addr_offset + dev_base_paddr);

	dp_info("IPA REO_DEST Ring addr_offset=%x, dev_base_paddr=%x, tp_paddr=%x paddr=%pK vaddr=%pK size= %u(%u bytes)",
		(unsigned int)addr_offset,
		(unsigned int)dev_base_paddr,
		(unsigned int)(soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr),
		(void *)soc->ipa_uc_rx_rsc.ipa_reo_ring_base_paddr,
		(void *)soc->ipa_uc_rx_rsc.ipa_reo_ring_base_vaddr,
		srng_params.num_entries,
		soc->ipa_uc_rx_rsc.ipa_reo_ring_size);

	hal_srng = (struct hal_srng *)
			pdev->rx_refill_buf_ring2.hal_srng;
	hal_get_srng_params(hal_soc_to_hal_soc_handle(hal_soc),
			    hal_srng_to_hal_ring_handle(hal_srng),
			    &srng_params);
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_paddr =
		srng_params.ring_base_paddr;
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_vaddr =
		srng_params.ring_base_vaddr;
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_size =
		(srng_params.num_entries * srng_params.entry_size) << 2;
	hp_addr = hal_srng_get_hp_addr(hal_soc_to_hal_soc_handle(hal_soc),
				       hal_srng_to_hal_ring_handle(hal_srng));
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr =
		qdf_mem_paddr_from_dmaaddr(soc->osdev, hp_addr);

	dp_info("IPA REFILL_BUF Ring hp_paddr=%x paddr=%pK vaddr=%pK size= %u(%u bytes)",
		(unsigned int)(soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr),
		(void *)soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_paddr,
		(void *)soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_vaddr,
		srng_params.num_entries,
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_size);

	/*
	 * Set DEST_RING_MAPPING_4 to SW2 as default value for
	 * DESTINATION_RING_CTRL_IX_0.
	 */
	ix0 = HAL_REO_REMAP_IX0(REO_REMAP_TCL, 0) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW1, 1) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW2, 2) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW3, 3) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW2, 4) |
	      HAL_REO_REMAP_IX0(REO_REMAP_RELEASE, 5) |
	      HAL_REO_REMAP_IX0(REO_REMAP_FW, 6) |
	      HAL_REO_REMAP_IX0(REO_REMAP_FW, 7);

	hal_reo_read_write_ctrl_ix(soc->hal_soc, false, &ix0, NULL, NULL, NULL);

	return 0;
}

static QDF_STATUS dp_ipa_get_shared_mem_info(qdf_device_t osdev,
					     qdf_shared_mem_t *shared_mem,
					     void *cpu_addr,
					     qdf_dma_addr_t dma_addr,
					     uint32_t size)
{
	qdf_dma_addr_t paddr;
	int ret;

	shared_mem->vaddr = cpu_addr;
	qdf_mem_set_dma_size(osdev, &shared_mem->mem_info, size);
	*qdf_mem_get_dma_addr_ptr(osdev, &shared_mem->mem_info) = dma_addr;

	paddr = qdf_mem_paddr_from_dmaaddr(osdev, dma_addr);
	qdf_mem_set_dma_pa(osdev, &shared_mem->mem_info, paddr);

	ret = qdf_mem_dma_get_sgtable(osdev->dev, &shared_mem->sgtable,
				      shared_mem->vaddr, dma_addr, size);
	if (ret) {
		dp_err("Unable to get DMA sgtable");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_dma_get_sgtable_dma_addr(&shared_mem->sgtable);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_get_resource(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct dp_ipa_resources *ipa_res;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	ipa_res = &pdev->ipa_resource;
	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	ipa_res->tx_num_alloc_buffer =
		(uint32_t)soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt;

	dp_ipa_get_shared_mem_info(soc->osdev, &ipa_res->tx_ring,
				   soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_vaddr,
				   soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_paddr,
				   soc->ipa_uc_tx_rsc.ipa_tcl_ring_size);

	dp_ipa_get_shared_mem_info(soc->osdev, &ipa_res->tx_comp_ring,
				   soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_vaddr,
				   soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_paddr,
				   soc->ipa_uc_tx_rsc.ipa_wbm_ring_size);

	dp_ipa_get_shared_mem_info(soc->osdev, &ipa_res->rx_rdy_ring,
				   soc->ipa_uc_rx_rsc.ipa_reo_ring_base_vaddr,
				   soc->ipa_uc_rx_rsc.ipa_reo_ring_base_paddr,
				   soc->ipa_uc_rx_rsc.ipa_reo_ring_size);

	dp_ipa_get_shared_mem_info(
			soc->osdev, &ipa_res->rx_refill_ring,
			soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_vaddr,
			soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_paddr,
			soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_size);

	if (!qdf_mem_get_dma_addr(soc->osdev, &ipa_res->tx_ring.mem_info) ||
	    !qdf_mem_get_dma_addr(soc->osdev,
				  &ipa_res->tx_comp_ring.mem_info) ||
	    !qdf_mem_get_dma_addr(soc->osdev, &ipa_res->rx_rdy_ring.mem_info) ||
	    !qdf_mem_get_dma_addr(soc->osdev,
				  &ipa_res->rx_refill_ring.mem_info))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_set_doorbell_paddr(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct dp_ipa_resources *ipa_res;
	struct hal_srng *wbm_srng = (struct hal_srng *)
			soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	struct hal_srng *reo_srng = (struct hal_srng *)
			soc->reo_dest_ring[IPA_REO_DEST_RING_IDX].hal_srng;
	uint32_t tx_comp_doorbell_dmaaddr;
	uint32_t rx_ready_doorbell_dmaaddr;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	ipa_res = &pdev->ipa_resource;
	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	if (ipa_res->is_db_ddr_mapped)
		ipa_res->tx_comp_doorbell_vaddr =
				phys_to_virt(ipa_res->tx_comp_doorbell_paddr);
	else
		ipa_res->tx_comp_doorbell_vaddr =
				ioremap(ipa_res->tx_comp_doorbell_paddr, 4);

	if (qdf_mem_smmu_s1_enabled(soc->osdev)) {
		pld_smmu_map(soc->osdev->dev, ipa_res->tx_comp_doorbell_paddr,
			     &tx_comp_doorbell_dmaaddr, sizeof(uint32_t));
		ipa_res->tx_comp_doorbell_paddr = tx_comp_doorbell_dmaaddr;

		pld_smmu_map(soc->osdev->dev, ipa_res->rx_ready_doorbell_paddr,
			     &rx_ready_doorbell_dmaaddr, sizeof(uint32_t));
		ipa_res->rx_ready_doorbell_paddr = rx_ready_doorbell_dmaaddr;
	}

	hal_srng_dst_set_hp_paddr(wbm_srng, ipa_res->tx_comp_doorbell_paddr);

	dp_info("paddr %pK vaddr %pK",
		(void *)ipa_res->tx_comp_doorbell_paddr,
		(void *)ipa_res->tx_comp_doorbell_vaddr);

	/*
	 * For RX, REO module on Napier/Hastings does reordering on incoming
	 * Ethernet packets and writes one or more descriptors to REO2IPA Rx
	 * ring.It then updates the ring’s Write/Head ptr and rings a doorbell
	 * to IPA.
	 * Set the doorbell addr for the REO ring.
	 */
	hal_srng_dst_set_hp_paddr(reo_srng, ipa_res->rx_ready_doorbell_paddr);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_op_response(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			      uint8_t *op_msg)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wlan_cfg_is_ipa_enabled(pdev->soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	if (pdev->ipa_uc_op_cb) {
		pdev->ipa_uc_op_cb(op_msg, pdev->usr_ctxt);
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "%s: IPA callback function is not registered", __func__);
		qdf_mem_free(op_msg);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_register_op_cb(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
				 ipa_uc_op_cb_type op_cb,
				 void *usr_ctxt)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wlan_cfg_is_ipa_enabled(pdev->soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	pdev->ipa_uc_op_cb = op_cb;
	pdev->usr_ctxt = usr_ctxt;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_get_stat(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_send_ipa_data_frame() - send IPA data frame
 * @soc_hdl: datapath soc handle
 * @vdev_id: id of the virtual device
 * @skb: skb to transmit
 *
 * Return: skb/ NULL is for success
 */
qdf_nbuf_t dp_tx_send_ipa_data_frame(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
				     qdf_nbuf_t skb)
{
	qdf_nbuf_t ret;

	/* Terminate the (single-element) list of tx frames */
	qdf_nbuf_set_next(skb, NULL);
	ret = dp_tx_send(soc_hdl, vdev_id, skb);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to tx", __func__);
		return ret;
	}

	return NULL;
}

QDF_STATUS dp_ipa_enable_autonomy(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	uint32_t ix0;
	uint32_t ix2;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	if (!hif_is_target_ready(HIF_GET_SOFTC(soc->hif_handle)))
		return QDF_STATUS_E_AGAIN;

	/* Call HAL API to remap REO rings to REO2IPA ring */
	ix0 = HAL_REO_REMAP_IX0(REO_REMAP_TCL, 0) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW4, 1) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW1, 2) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW4, 3) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW4, 4) |
	      HAL_REO_REMAP_IX0(REO_REMAP_RELEASE, 5) |
	      HAL_REO_REMAP_IX0(REO_REMAP_FW, 6) |
	      HAL_REO_REMAP_IX0(REO_REMAP_FW, 7);

	if (wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {
		ix2 = HAL_REO_REMAP_IX2(REO_REMAP_SW4, 16) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 17) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 18) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 19) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 20) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 21) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 22) |
		      HAL_REO_REMAP_IX2(REO_REMAP_SW4, 23);

		hal_reo_read_write_ctrl_ix(soc->hal_soc, false, &ix0, NULL,
					   &ix2, &ix2);
		dp_ipa_reo_remap_history_add(ix0, ix2, ix2);
	} else {
		hal_reo_read_write_ctrl_ix(soc->hal_soc, false, &ix0, NULL,
					   NULL, NULL);
		dp_ipa_reo_remap_history_add(ix0, 0, 0);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_disable_autonomy(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	uint32_t ix0;
	uint32_t ix2;
	uint32_t ix3;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	if (!hif_is_target_ready(HIF_GET_SOFTC(soc->hif_handle)))
		return QDF_STATUS_E_AGAIN;

	/* Call HAL API to remap REO rings to REO2IPA ring */
	ix0 = HAL_REO_REMAP_IX0(REO_REMAP_TCL, 0) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW1, 1) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW2, 2) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW3, 3) |
	      HAL_REO_REMAP_IX0(REO_REMAP_SW2, 4) |
	      HAL_REO_REMAP_IX0(REO_REMAP_RELEASE, 5) |
	      HAL_REO_REMAP_IX0(REO_REMAP_FW, 6) |
	      HAL_REO_REMAP_IX0(REO_REMAP_FW, 7);

	if (wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {
		dp_reo_remap_config(soc, &ix2, &ix3);

		hal_reo_read_write_ctrl_ix(soc->hal_soc, false, &ix0, NULL,
					   &ix2, &ix3);
		dp_ipa_reo_remap_history_add(ix0, ix2, ix3);
	} else {
		hal_reo_read_write_ctrl_ix(soc->hal_soc, false, &ix0, NULL,
					   NULL, NULL);
		dp_ipa_reo_remap_history_add(ix0, 0, 0);
	}

	return QDF_STATUS_SUCCESS;
}

/* This should be configurable per H/W configuration enable status */
#define L3_HEADER_PADDING	2

#ifdef CONFIG_IPA_WDI_UNIFIED_API

#ifndef QCA_LL_TX_FLOW_CONTROL_V2
static inline void dp_setup_mcc_sys_pipes(
		qdf_ipa_sys_connect_params_t *sys_in,
		qdf_ipa_wdi_conn_in_params_t *pipe_in)
{
	/* Setup MCC sys pipe */
	QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in) =
			DP_IPA_MAX_IFACE;
	for (int i = 0; i < DP_IPA_MAX_IFACE; i++)
		memcpy(&QDF_IPA_WDI_CONN_IN_PARAMS_SYS_IN(pipe_in)[i],
		       &sys_in[i], sizeof(qdf_ipa_sys_connect_params_t));
}
#else
static inline void dp_setup_mcc_sys_pipes(
		qdf_ipa_sys_connect_params_t *sys_in,
		qdf_ipa_wdi_conn_in_params_t *pipe_in)
{
	QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in) = 0;
}
#endif

static void dp_ipa_wdi_tx_params(struct dp_soc *soc,
				 struct dp_ipa_resources *ipa_res,
				 qdf_ipa_wdi_pipe_setup_info_t *tx,
				 bool over_gsi)
{
	struct tcl_data_cmd *tcl_desc_ptr;
	uint8_t *desc_addr;
	uint32_t desc_size;

	if (over_gsi)
		QDF_IPA_WDI_SETUP_INFO_CLIENT(tx) = IPA_CLIENT_WLAN2_CONS;
	else
		QDF_IPA_WDI_SETUP_INFO_CLIENT(tx) = IPA_CLIENT_WLAN1_CONS;

	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(tx) =
		qdf_mem_get_dma_addr(soc->osdev,
				     &ipa_res->tx_comp_ring.mem_info);
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(tx) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->tx_comp_ring.mem_info);

	/* WBM Tail Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(tx) =
		soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr;
	QDF_IPA_WDI_SETUP_INFO_IS_TXR_RN_DB_PCIE_ADDR(tx) = true;

	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(tx) =
		qdf_mem_get_dma_addr(soc->osdev,
				     &ipa_res->tx_ring.mem_info);
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(tx) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->tx_ring.mem_info);

	/* TCL Head Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(tx) =
		soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr;
	QDF_IPA_WDI_SETUP_INFO_IS_EVT_RN_DB_PCIE_ADDR(tx) = true;

	QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(tx) =
		ipa_res->tx_num_alloc_buffer;

	QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(tx) = 0;

	/* Preprogram TCL descriptor */
	desc_addr =
		(uint8_t *)QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(tx);
	desc_size = sizeof(struct tcl_data_cmd);
	HAL_TX_DESC_SET_TLV_HDR(desc_addr, HAL_TX_TCL_DATA_TAG, desc_size);
	tcl_desc_ptr = (struct tcl_data_cmd *)
		(QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(tx) + 1);
	tcl_desc_ptr->buf_addr_info.return_buffer_manager =
		HAL_RX_BUF_RBM_SW2_BM;
	tcl_desc_ptr->addrx_en = 1;	/* Address X search enable in ASE */
	tcl_desc_ptr->encap_type = HAL_TX_ENCAP_TYPE_ETHERNET;
	tcl_desc_ptr->packet_offset = 2;	/* padding for alignment */
}

static void dp_ipa_wdi_rx_params(struct dp_soc *soc,
				 struct dp_ipa_resources *ipa_res,
				 qdf_ipa_wdi_pipe_setup_info_t *rx,
				 bool over_gsi)
{
	if (over_gsi)
		QDF_IPA_WDI_SETUP_INFO_CLIENT(rx) =
					IPA_CLIENT_WLAN2_PROD;
	else
		QDF_IPA_WDI_SETUP_INFO_CLIENT(rx) =
					IPA_CLIENT_WLAN1_PROD;

	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(rx) =
		qdf_mem_get_dma_addr(soc->osdev,
				     &ipa_res->rx_rdy_ring.mem_info);
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(rx) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->rx_rdy_ring.mem_info);

	/* REO Tail Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(rx) =
		soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr;
	QDF_IPA_WDI_SETUP_INFO_IS_TXR_RN_DB_PCIE_ADDR(rx) = true;

	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(rx) =
		qdf_mem_get_dma_addr(soc->osdev,
				     &ipa_res->rx_refill_ring.mem_info);
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(rx) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->rx_refill_ring.mem_info);

	/* FW Head Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(rx) =
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr;
	QDF_IPA_WDI_SETUP_INFO_IS_EVT_RN_DB_PCIE_ADDR(rx) = false;

	QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(rx) =
		RX_PKT_TLVS_LEN + L3_HEADER_PADDING;
}

static void
dp_ipa_wdi_tx_smmu_params(struct dp_soc *soc,
			  struct dp_ipa_resources *ipa_res,
			  qdf_ipa_wdi_pipe_setup_info_smmu_t *tx_smmu,
			  bool over_gsi)
{
	struct tcl_data_cmd *tcl_desc_ptr;
	uint8_t *desc_addr;
	uint32_t desc_size;

	if (over_gsi)
		QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(tx_smmu) =
			IPA_CLIENT_WLAN2_CONS;
	else
		QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(tx_smmu) =
			IPA_CLIENT_WLAN1_CONS;

	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(tx_smmu),
		     &ipa_res->tx_comp_ring.sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(tx_smmu) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->tx_comp_ring.mem_info);
	/* WBM Tail Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_DOORBELL_PA(tx_smmu) =
		soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr;
	QDF_IPA_WDI_SETUP_INFO_SMMU_IS_TXR_RN_DB_PCIE_ADDR(tx_smmu) = true;

	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(tx_smmu),
		     &ipa_res->tx_ring.sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(tx_smmu) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->tx_ring.mem_info);
	/* TCL Head Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(tx_smmu) =
		soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr;
	QDF_IPA_WDI_SETUP_INFO_SMMU_IS_EVT_RN_DB_PCIE_ADDR(tx_smmu) = true;

	QDF_IPA_WDI_SETUP_INFO_SMMU_NUM_PKT_BUFFERS(tx_smmu) =
		ipa_res->tx_num_alloc_buffer;
	QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(tx_smmu) = 0;

	/* Preprogram TCL descriptor */
	desc_addr = (uint8_t *)QDF_IPA_WDI_SETUP_INFO_SMMU_DESC_FORMAT_TEMPLATE(
			tx_smmu);
	desc_size = sizeof(struct tcl_data_cmd);
	HAL_TX_DESC_SET_TLV_HDR(desc_addr, HAL_TX_TCL_DATA_TAG, desc_size);
	tcl_desc_ptr = (struct tcl_data_cmd *)
		(QDF_IPA_WDI_SETUP_INFO_SMMU_DESC_FORMAT_TEMPLATE(tx_smmu) + 1);
	tcl_desc_ptr->buf_addr_info.return_buffer_manager =
		HAL_RX_BUF_RBM_SW2_BM;
	tcl_desc_ptr->addrx_en = 1;	/* Address X search enable in ASE */
	tcl_desc_ptr->encap_type = HAL_TX_ENCAP_TYPE_ETHERNET;
	tcl_desc_ptr->packet_offset = 2;	/* padding for alignment */
}

static void
dp_ipa_wdi_rx_smmu_params(struct dp_soc *soc,
			  struct dp_ipa_resources *ipa_res,
			  qdf_ipa_wdi_pipe_setup_info_smmu_t *rx_smmu,
			  bool over_gsi)
{
	if (over_gsi)
		QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(rx_smmu) =
					IPA_CLIENT_WLAN2_PROD;
	else
		QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(rx_smmu) =
					IPA_CLIENT_WLAN1_PROD;

	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(rx_smmu),
		     &ipa_res->rx_rdy_ring.sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(rx_smmu) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->rx_rdy_ring.mem_info);
	/* REO Tail Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_DOORBELL_PA(rx_smmu) =
		soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr;
	QDF_IPA_WDI_SETUP_INFO_SMMU_IS_TXR_RN_DB_PCIE_ADDR(rx_smmu) = true;

	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(rx_smmu),
		     &ipa_res->rx_refill_ring.sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(rx_smmu) =
		qdf_mem_get_dma_size(soc->osdev,
				     &ipa_res->rx_refill_ring.mem_info);

	/* FW Head Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(rx_smmu) =
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr;
	QDF_IPA_WDI_SETUP_INFO_SMMU_IS_EVT_RN_DB_PCIE_ADDR(rx_smmu) = false;

	QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(rx_smmu) =
		RX_PKT_TLVS_LEN + L3_HEADER_PADDING;
}

QDF_STATUS dp_ipa_setup(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			void *ipa_i2w_cb, void *ipa_w2i_cb,
			void *ipa_wdi_meter_notifier_cb,
			uint32_t ipa_desc_size, void *ipa_priv,
			bool is_rm_enabled, uint32_t *tx_pipe_handle,
			uint32_t *rx_pipe_handle, bool is_smmu_enabled,
			qdf_ipa_sys_connect_params_t *sys_in, bool over_gsi)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct dp_ipa_resources *ipa_res;
	qdf_ipa_ep_cfg_t *tx_cfg;
	qdf_ipa_ep_cfg_t *rx_cfg;
	qdf_ipa_wdi_pipe_setup_info_t *tx = NULL;
	qdf_ipa_wdi_pipe_setup_info_t *rx = NULL;
	qdf_ipa_wdi_pipe_setup_info_smmu_t *tx_smmu;
	qdf_ipa_wdi_pipe_setup_info_smmu_t *rx_smmu;
	qdf_ipa_wdi_conn_in_params_t pipe_in;
	qdf_ipa_wdi_conn_out_params_t pipe_out;
	int ret;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	ipa_res = &pdev->ipa_resource;
	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	qdf_mem_zero(&pipe_in, sizeof(pipe_in));
	qdf_mem_zero(&pipe_out, sizeof(pipe_out));

	if (is_smmu_enabled)
		QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(&pipe_in) = true;
	else
		QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(&pipe_in) = false;

	dp_setup_mcc_sys_pipes(sys_in, &pipe_in);

	/* TX PIPE */
	if (QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(&pipe_in)) {
		tx_smmu = &QDF_IPA_WDI_CONN_IN_PARAMS_TX_SMMU(&pipe_in);
		tx_cfg = &QDF_IPA_WDI_SETUP_INFO_SMMU_EP_CFG(tx_smmu);
	} else {
		tx = &QDF_IPA_WDI_CONN_IN_PARAMS_TX(&pipe_in);
		tx_cfg = &QDF_IPA_WDI_SETUP_INFO_EP_CFG(tx);
	}

	QDF_IPA_EP_CFG_NAT_EN(tx_cfg) = IPA_BYPASS_NAT;
	QDF_IPA_EP_CFG_HDR_LEN(tx_cfg) = DP_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(tx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(tx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(tx_cfg) = 0;
	QDF_IPA_EP_CFG_MODE(tx_cfg) = IPA_BASIC;
	QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(tx_cfg) = true;

	/**
	 * Transfer Ring: WBM Ring
	 * Transfer Ring Doorbell PA: WBM Tail Pointer Address
	 * Event Ring: TCL ring
	 * Event Ring Doorbell PA: TCL Head Pointer Address
	 */
	if (is_smmu_enabled)
		dp_ipa_wdi_tx_smmu_params(soc, ipa_res, tx_smmu, over_gsi);
	else
		dp_ipa_wdi_tx_params(soc, ipa_res, tx, over_gsi);

	/* RX PIPE */
	if (QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(&pipe_in)) {
		rx_smmu = &QDF_IPA_WDI_CONN_IN_PARAMS_RX_SMMU(&pipe_in);
		rx_cfg = &QDF_IPA_WDI_SETUP_INFO_SMMU_EP_CFG(rx_smmu);
	} else {
		rx = &QDF_IPA_WDI_CONN_IN_PARAMS_RX(&pipe_in);
		rx_cfg = &QDF_IPA_WDI_SETUP_INFO_EP_CFG(rx);
	}

	QDF_IPA_EP_CFG_NAT_EN(rx_cfg) = IPA_BYPASS_NAT;
	QDF_IPA_EP_CFG_HDR_LEN(rx_cfg) = DP_IPA_UC_WLAN_RX_HDR_LEN;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(rx_cfg) = 1;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(rx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(rx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_OFST_METADATA_VALID(rx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_METADATA_REG_VALID(rx_cfg) = 1;
	QDF_IPA_EP_CFG_MODE(rx_cfg) = IPA_BASIC;
	QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(rx_cfg) = true;

	/**
	 * Transfer Ring: REO Ring
	 * Transfer Ring Doorbell PA: REO Tail Pointer Address
	 * Event Ring: FW ring
	 * Event Ring Doorbell PA: FW Head Pointer Address
	 */
	if (is_smmu_enabled)
		dp_ipa_wdi_rx_smmu_params(soc, ipa_res, rx_smmu, over_gsi);
	else
		dp_ipa_wdi_rx_params(soc, ipa_res, rx, over_gsi);

	QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(&pipe_in) = ipa_w2i_cb;
	QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(&pipe_in) = ipa_priv;

	/* Connect WDI IPA PIPEs */
	ret = qdf_ipa_wdi_conn_pipes(&pipe_in, &pipe_out);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_conn_pipes: IPA pipe setup failed: ret=%d",
			  __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* IPA uC Doorbell registers */
	dp_info("Tx DB PA=0x%x, Rx DB PA=0x%x",
		(unsigned int)QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(&pipe_out),
		(unsigned int)QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(&pipe_out));

	ipa_res->tx_comp_doorbell_paddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(&pipe_out);
	ipa_res->rx_ready_doorbell_paddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(&pipe_out);

	ipa_res->is_db_ddr_mapped =
		QDF_IPA_WDI_CONN_OUT_PARAMS_IS_DB_DDR_MAPPED(&pipe_out);

	soc->ipa_first_tx_db_access = true;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_setup_iface() - Setup IPA header and register interface
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
			      qdf_ipa_client_type_t prod_client,
			      qdf_ipa_client_type_t cons_client,
			      uint8_t session_id, bool is_ipv6_enabled)
{
	qdf_ipa_wdi_reg_intf_in_params_t in;
	qdf_ipa_wdi_hdr_info_t hdr_info;
	struct dp_ipa_uc_tx_hdr uc_tx_hdr;
	struct dp_ipa_uc_tx_hdr uc_tx_hdr_v6;
	int ret = -EINVAL;

	dp_debug("Add Partial hdr: %s, "QDF_MAC_ADDR_FMT, ifname,
		 QDF_MAC_ADDR_REF(mac_addr));
	qdf_mem_zero(&hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	qdf_ether_addr_copy(uc_tx_hdr.eth.h_source, mac_addr);

	/* IPV4 header */
	uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IP);

	QDF_IPA_WDI_HDR_INFO_HDR(&hdr_info) = (uint8_t *)&uc_tx_hdr;
	QDF_IPA_WDI_HDR_INFO_HDR_LEN(&hdr_info) = DP_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_WDI_HDR_INFO_HDR_TYPE(&hdr_info) = IPA_HDR_L2_ETHERNET_II;
	QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(&hdr_info) =
		DP_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(&in) = ifname;
	qdf_mem_copy(&(QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(&in)[IPA_IP_v4]),
		     &hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_ALT_DST_PIPE(&in) = cons_client;
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(&in) = 1;
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(&in) =
		htonl(session_id << 16);
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(&in) = htonl(0x00FF0000);

	/* IPV6 header */
	if (is_ipv6_enabled) {
		qdf_mem_copy(&uc_tx_hdr_v6, &uc_tx_hdr,
			     DP_IPA_UC_WLAN_TX_HDR_LEN);
		uc_tx_hdr_v6.eth.h_proto = qdf_htons(ETH_P_IPV6);
		QDF_IPA_WDI_HDR_INFO_HDR(&hdr_info) = (uint8_t *)&uc_tx_hdr_v6;
		qdf_mem_copy(&(QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(&in)[IPA_IP_v6]),
			     &hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	}

	dp_debug("registering for session_id: %u", session_id);

	ret = qdf_ipa_wdi_reg_intf(&in);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "%s: ipa_wdi_reg_intf: register IPA interface falied: ret=%d",
		    __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#else /* CONFIG_IPA_WDI_UNIFIED_API */
QDF_STATUS dp_ipa_setup(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			void *ipa_i2w_cb, void *ipa_w2i_cb,
			void *ipa_wdi_meter_notifier_cb,
			uint32_t ipa_desc_size, void *ipa_priv,
			bool is_rm_enabled, uint32_t *tx_pipe_handle,
			uint32_t *rx_pipe_handle)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct dp_ipa_resources *ipa_res;
	qdf_ipa_wdi_pipe_setup_info_t *tx;
	qdf_ipa_wdi_pipe_setup_info_t *rx;
	qdf_ipa_wdi_conn_in_params_t pipe_in;
	qdf_ipa_wdi_conn_out_params_t pipe_out;
	struct tcl_data_cmd *tcl_desc_ptr;
	uint8_t *desc_addr;
	uint32_t desc_size;
	int ret;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	ipa_res = &pdev->ipa_resource;
	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return QDF_STATUS_SUCCESS;

	qdf_mem_zero(&tx, sizeof(qdf_ipa_wdi_pipe_setup_info_t));
	qdf_mem_zero(&rx, sizeof(qdf_ipa_wdi_pipe_setup_info_t));
	qdf_mem_zero(&pipe_in, sizeof(pipe_in));
	qdf_mem_zero(&pipe_out, sizeof(pipe_out));

	/* TX PIPE */
	/**
	 * Transfer Ring: WBM Ring
	 * Transfer Ring Doorbell PA: WBM Tail Pointer Address
	 * Event Ring: TCL ring
	 * Event Ring Doorbell PA: TCL Head Pointer Address
	 */
	tx = &QDF_IPA_WDI_CONN_IN_PARAMS_TX(&pipe_in);
	QDF_IPA_WDI_SETUP_INFO_NAT_EN(tx) = IPA_BYPASS_NAT;
	QDF_IPA_WDI_SETUP_INFO_HDR_LEN(tx) = DP_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE_VALID(tx) = 0;
	QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE(tx) = 0;
	QDF_IPA_WDI_SETUP_INFO_HDR_ADDITIONAL_CONST_LEN(tx) = 0;
	QDF_IPA_WDI_SETUP_INFO_MODE(tx) = IPA_BASIC;
	QDF_IPA_WDI_SETUP_INFO_HDR_LITTLE_ENDIAN(tx) = true;
	QDF_IPA_WDI_SETUP_INFO_CLIENT(tx) = IPA_CLIENT_WLAN1_CONS;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(tx) =
		ipa_res->tx_comp_ring_base_paddr;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(tx) =
		ipa_res->tx_comp_ring_size;
	/* WBM Tail Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(tx) =
		soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(tx) =
		ipa_res->tx_ring_base_paddr;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(tx) = ipa_res->tx_ring_size;
	/* TCL Head Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(tx) =
		soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr;
	QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(tx) =
		ipa_res->tx_num_alloc_buffer;
	QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(tx) = 0;

	/* Preprogram TCL descriptor */
	desc_addr =
		(uint8_t *)QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(tx);
	desc_size = sizeof(struct tcl_data_cmd);
	HAL_TX_DESC_SET_TLV_HDR(desc_addr, HAL_TX_TCL_DATA_TAG, desc_size);
	tcl_desc_ptr = (struct tcl_data_cmd *)
		(QDF_IPA_WDI_SETUP_INFO_DESC_FORMAT_TEMPLATE(tx) + 1);
	tcl_desc_ptr->buf_addr_info.return_buffer_manager =
						HAL_RX_BUF_RBM_SW2_BM;
	tcl_desc_ptr->addrx_en = 1;	/* Address X search enable in ASE */
	tcl_desc_ptr->encap_type = HAL_TX_ENCAP_TYPE_ETHERNET;
	tcl_desc_ptr->packet_offset = 2;	/* padding for alignment */

	/* RX PIPE */
	/**
	 * Transfer Ring: REO Ring
	 * Transfer Ring Doorbell PA: REO Tail Pointer Address
	 * Event Ring: FW ring
	 * Event Ring Doorbell PA: FW Head Pointer Address
	 */
	rx = &QDF_IPA_WDI_CONN_IN_PARAMS_RX(&pipe_in);
	QDF_IPA_WDI_SETUP_INFO_NAT_EN(rx) = IPA_BYPASS_NAT;
	QDF_IPA_WDI_SETUP_INFO_HDR_LEN(rx) = DP_IPA_UC_WLAN_RX_HDR_LEN;
	QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE_VALID(rx) = 0;
	QDF_IPA_WDI_SETUP_INFO_HDR_OFST_PKT_SIZE(rx) = 0;
	QDF_IPA_WDI_SETUP_INFO_HDR_ADDITIONAL_CONST_LEN(rx) = 0;
	QDF_IPA_WDI_SETUP_INFO_HDR_OFST_METADATA_VALID(rx) = 0;
	QDF_IPA_WDI_SETUP_INFO_HDR_METADATA_REG_VALID(rx) = 1;
	QDF_IPA_WDI_SETUP_INFO_MODE(rx) = IPA_BASIC;
	QDF_IPA_WDI_SETUP_INFO_HDR_LITTLE_ENDIAN(rx) = true;
	QDF_IPA_WDI_SETUP_INFO_CLIENT(rx) = IPA_CLIENT_WLAN1_PROD;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(rx) =
						ipa_res->rx_rdy_ring_base_paddr;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(rx) =
						ipa_res->rx_rdy_ring_size;
	/* REO Tail Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(rx) =
					soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(rx) =
					ipa_res->rx_refill_ring_base_paddr;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(rx) =
						ipa_res->rx_refill_ring_size;
	/* FW Head Pointer Address */
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(rx) =
				soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr;
	QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(rx) = RX_PKT_TLVS_LEN +
						L3_HEADER_PADDING;
	QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(&pipe_in) = ipa_w2i_cb;
	QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(&pipe_in) = ipa_priv;

	/* Connect WDI IPA PIPE */
	ret = qdf_ipa_wdi_conn_pipes(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_conn_pipes: IPA pipe setup failed: ret=%d",
			  __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* IPA uC Doorbell registers */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Tx DB PA=0x%x, Rx DB PA=0x%x",
		  __func__,
		(unsigned int)QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(&pipe_out),
		(unsigned int)QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(&pipe_out));

	ipa_res->tx_comp_doorbell_paddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(&pipe_out);
	ipa_res->tx_comp_doorbell_vaddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_VA(&pipe_out);
	ipa_res->rx_ready_doorbell_paddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(&pipe_out);

	soc->ipa_first_tx_db_access = true;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Tx: %s=%pK, %s=%d, %s=%pK, %s=%pK, %s=%d, %s=%pK, %s=%d, %s=%pK",
		  __func__,
		  "transfer_ring_base_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(tx),
		  "transfer_ring_size",
		  QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(tx),
		  "transfer_ring_doorbell_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(tx),
		  "event_ring_base_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(tx),
		  "event_ring_size",
		  QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(tx),
		  "event_ring_doorbell_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(tx),
		  "num_pkt_buffers",
		  QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(tx),
		  "tx_comp_doorbell_paddr",
		  (void *)ipa_res->tx_comp_doorbell_paddr);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Rx: %s=%pK, %s=%d, %s=%pK, %s=%pK, %s=%d, %s=%pK, %s=%d, %s=%pK",
		  __func__,
		  "transfer_ring_base_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(rx),
		  "transfer_ring_size",
		  QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(rx),
		  "transfer_ring_doorbell_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(rx),
		  "event_ring_base_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(rx),
		  "event_ring_size",
		  QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(rx),
		  "event_ring_doorbell_pa",
		  (void *)QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(rx),
		  "num_pkt_buffers",
		  QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(rx),
		  "tx_comp_doorbell_paddr",
		  (void *)ipa_res->rx_ready_doorbell_paddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_setup_iface() - Setup IPA header and register interface
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
			      qdf_ipa_client_type_t prod_client,
			      qdf_ipa_client_type_t cons_client,
			      uint8_t session_id, bool is_ipv6_enabled)
{
	qdf_ipa_wdi_reg_intf_in_params_t in;
	qdf_ipa_wdi_hdr_info_t hdr_info;
	struct dp_ipa_uc_tx_hdr uc_tx_hdr;
	struct dp_ipa_uc_tx_hdr uc_tx_hdr_v6;
	int ret = -EINVAL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Add Partial hdr: %s, "QDF_MAC_ADDR_FMT,
		  __func__, ifname, QDF_MAC_ADDR_REF(mac_addr));

	qdf_mem_zero(&hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	qdf_ether_addr_copy(uc_tx_hdr.eth.h_source, mac_addr);

	/* IPV4 header */
	uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IP);

	QDF_IPA_WDI_HDR_INFO_HDR(&hdr_info) = (uint8_t *)&uc_tx_hdr;
	QDF_IPA_WDI_HDR_INFO_HDR_LEN(&hdr_info) = DP_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_WDI_HDR_INFO_HDR_TYPE(&hdr_info) = IPA_HDR_L2_ETHERNET_II;
	QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(&hdr_info) =
		DP_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(&in) = ifname;
	qdf_mem_copy(&(QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(&in)[IPA_IP_v4]),
		     &hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(&in) = 1;
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(&in) =
		htonl(session_id << 16);
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(&in) = htonl(0x00FF0000);

	/* IPV6 header */
	if (is_ipv6_enabled) {
		qdf_mem_copy(&uc_tx_hdr_v6, &uc_tx_hdr,
			     DP_IPA_UC_WLAN_TX_HDR_LEN);
		uc_tx_hdr_v6.eth.h_proto = qdf_htons(ETH_P_IPV6);
		QDF_IPA_WDI_HDR_INFO_HDR(&hdr_info) = (uint8_t *)&uc_tx_hdr_v6;
		qdf_mem_copy(&(QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(&in)[IPA_IP_v6]),
			     &hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	}

	ret = qdf_ipa_wdi_reg_intf(&in);
	if (ret) {
		dp_err("ipa_wdi_reg_intf: register IPA interface falied: ret=%d",
		       ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#endif /* CONFIG_IPA_WDI_UNIFIED_API */

/**
 * dp_ipa_cleanup() - Disconnect IPA pipes
 * @soc_hdl: dp soc handle
 * @pdev_id: dp pdev id
 * @tx_pipe_handle: Tx pipe handle
 * @rx_pipe_handle: Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_cleanup(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			  uint32_t tx_pipe_handle, uint32_t rx_pipe_handle)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct dp_ipa_resources *ipa_res;
	struct dp_pdev *pdev;
	int ret;

	ret = qdf_ipa_wdi_disconn_pipes();
	if (ret) {
		dp_err("ipa_wdi_disconn_pipes: IPA pipe cleanup failed: ret=%d",
		       ret);
		status = QDF_STATUS_E_FAILURE;
	}

	pdev = dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	if (qdf_unlikely(!pdev)) {
		dp_err_rl("Invalid pdev for pdev_id %d", pdev_id);
		status = QDF_STATUS_E_FAILURE;
		goto exit;
	}

	if (qdf_mem_smmu_s1_enabled(soc->osdev)) {
		ipa_res = &pdev->ipa_resource;

		/* unmap has to be the reverse order of smmu map */
		ret = pld_smmu_unmap(soc->osdev->dev,
				     ipa_res->rx_ready_doorbell_paddr,
				     sizeof(uint32_t));
		if (ret)
			dp_err_rl("IPA RX DB smmu unmap failed");

		ret = pld_smmu_unmap(soc->osdev->dev,
				     ipa_res->tx_comp_doorbell_paddr,
				     sizeof(uint32_t));
		if (ret)
			dp_err_rl("IPA TX DB smmu unmap failed");
	}

exit:
	return status;
}

/**
 * dp_ipa_cleanup_iface() - Cleanup IPA header and deregister interface
 * @ifname: Interface name
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_cleanup_iface(char *ifname, bool is_ipv6_enabled)
{
	int ret;

	ret = qdf_ipa_wdi_dereg_intf(ifname);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_dereg_intf: IPA pipe deregistration failed: ret=%d",
			  __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_enable_pipes(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	struct hal_srng *wbm_srng = (struct hal_srng *)
			soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	struct dp_ipa_resources *ipa_res;
	QDF_STATUS result;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	ipa_res = &pdev->ipa_resource;

	qdf_atomic_set(&soc->ipa_pipes_enabled, 1);
	dp_ipa_handle_rx_buf_pool_smmu_mapping(soc, pdev, true);

	result = qdf_ipa_wdi_enable_pipes();
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Enable WDI PIPE fail, code %d",
			  __func__, result);
		qdf_atomic_set(&soc->ipa_pipes_enabled, 0);
		dp_ipa_handle_rx_buf_pool_smmu_mapping(soc, pdev, false);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ipa_first_tx_db_access) {
		hal_srng_dst_init_hp(wbm_srng, ipa_res->tx_comp_doorbell_vaddr);
		soc->ipa_first_tx_db_access = false;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_ipa_disable_pipes(struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	QDF_STATUS result;

	if (!pdev) {
		dp_err("Invalid instance");
		return QDF_STATUS_E_FAILURE;
	}

	result = qdf_ipa_wdi_disable_pipes();
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Disable WDI PIPE fail, code %d",
			  __func__, result);
		qdf_assert_always(0);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_atomic_set(&soc->ipa_pipes_enabled, 0);
	dp_ipa_handle_rx_buf_pool_smmu_mapping(soc, pdev, false);

	return result ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_set_perf_level() - Set IPA clock bandwidth based on data rates
 * @client: Client type
 * @max_supported_bw_mbps: Maximum bandwidth needed (in Mbps)
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_set_perf_level(int client, uint32_t max_supported_bw_mbps)
{
	qdf_ipa_wdi_perf_profile_t profile;
	QDF_STATUS result;

	profile.client = client;
	profile.max_supported_bw_mbps = max_supported_bw_mbps;

	result = qdf_ipa_wdi_set_perf_profile(&profile);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_set_perf_profile fail, code %d",
			  __func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_intrabss_send - send IPA RX intra-bss frames
 * @pdev: pdev
 * @vdev: vdev
 * @nbuf: skb
 *
 * Return: nbuf if TX fails and NULL if TX succeeds
 */
static qdf_nbuf_t dp_ipa_intrabss_send(struct dp_pdev *pdev,
				       struct dp_vdev *vdev,
				       qdf_nbuf_t nbuf)
{
	struct dp_peer *vdev_peer;
	uint16_t len;

	vdev_peer = dp_vdev_bss_peer_ref_n_get(pdev->soc, vdev, DP_MOD_ID_IPA);
	if (qdf_unlikely(!vdev_peer))
		return nbuf;

	qdf_mem_zero(nbuf->cb, sizeof(nbuf->cb));
	len = qdf_nbuf_len(nbuf);

	if (dp_tx_send((struct cdp_soc_t *)pdev->soc, vdev->vdev_id, nbuf)) {
		DP_STATS_INC_PKT(vdev_peer, rx.intra_bss.fail, 1, len);
		dp_peer_unref_delete(vdev_peer, DP_MOD_ID_IPA);
		return nbuf;
	}

	DP_STATS_INC_PKT(vdev_peer, rx.intra_bss.pkts, 1, len);
	dp_peer_unref_delete(vdev_peer, DP_MOD_ID_IPA);
	return NULL;
}

bool dp_ipa_rx_intrabss_fwd(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			    qdf_nbuf_t nbuf, bool *fwd_success)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_vdev *vdev = dp_vdev_get_ref_by_id(soc, vdev_id,
						     DP_MOD_ID_IPA);
	struct dp_pdev *pdev;
	struct dp_peer *da_peer;
	struct dp_peer *sa_peer;
	qdf_nbuf_t nbuf_copy;
	uint8_t da_is_bcmc;
	struct ethhdr *eh;
	bool status = false;

	*fwd_success = false; /* set default as failure */

	/*
	 * WDI 3.0 skb->cb[] info from IPA driver
	 * skb->cb[0] = vdev_id
	 * skb->cb[1].bit#1 = da_is_bcmc
	 */
	da_is_bcmc = ((uint8_t)nbuf->cb[1]) & 0x2;

	if (qdf_unlikely(!vdev))
		return false;

	pdev = vdev->pdev;
	if (qdf_unlikely(!pdev))
		goto out;

	/* no fwd for station mode and just pass up to stack */
	if (vdev->opmode == wlan_op_mode_sta)
		goto out;

	if (da_is_bcmc) {
		nbuf_copy = qdf_nbuf_copy(nbuf);
		if (!nbuf_copy)
			goto out;

		if (dp_ipa_intrabss_send(pdev, vdev, nbuf_copy))
			qdf_nbuf_free(nbuf_copy);
		else
			*fwd_success = true;

		/* return false to pass original pkt up to stack */
		goto out;
	}

	eh = (struct ethhdr *)qdf_nbuf_data(nbuf);

	if (!qdf_mem_cmp(eh->h_dest, vdev->mac_addr.raw, QDF_MAC_ADDR_SIZE))
		goto out;

	da_peer = dp_peer_find_hash_find(soc, eh->h_dest, 0, vdev->vdev_id,
					 DP_MOD_ID_IPA);
	if (!da_peer)
		goto out;

	dp_peer_unref_delete(da_peer, DP_MOD_ID_IPA);

	sa_peer = dp_peer_find_hash_find(soc, eh->h_source, 0, vdev->vdev_id,
					 DP_MOD_ID_IPA);
	if (!sa_peer)
		goto out;

	dp_peer_unref_delete(sa_peer, DP_MOD_ID_IPA);

	/*
	 * In intra-bss forwarding scenario, skb is allocated by IPA driver.
	 * Need to add skb to internal tracking table to avoid nbuf memory
	 * leak check for unallocated skb.
	 */
	qdf_net_buf_debug_acquire_skb(nbuf, __FILE__, __LINE__);

	if (dp_ipa_intrabss_send(pdev, vdev, nbuf))
		qdf_nbuf_free(nbuf);
	else
		*fwd_success = true;

	status = true;
out:
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_IPA);
	return status;
}

#ifdef MDM_PLATFORM
bool dp_ipa_is_mdm_platform(void)
{
	return true;
}
#else
bool dp_ipa_is_mdm_platform(void)
{
	return false;
}
#endif

/**
 * dp_ipa_frag_nbuf_linearize - linearize nbuf for IPA
 * @soc: soc
 * @nbuf: source skb
 *
 * Return: new nbuf if success and otherwise NULL
 */
static qdf_nbuf_t dp_ipa_frag_nbuf_linearize(struct dp_soc *soc,
					     qdf_nbuf_t nbuf)
{
	uint8_t *src_nbuf_data;
	uint8_t *dst_nbuf_data;
	qdf_nbuf_t dst_nbuf;
	qdf_nbuf_t temp_nbuf = nbuf;
	uint32_t nbuf_len = qdf_nbuf_len(nbuf);
	bool is_nbuf_head = true;
	uint32_t copy_len = 0;

	dst_nbuf = qdf_nbuf_alloc(soc->osdev, RX_DATA_BUFFER_SIZE,
				  RX_BUFFER_RESERVATION,
				  RX_DATA_BUFFER_ALIGNMENT, FALSE);

	if (!dst_nbuf) {
		dp_err_rl("nbuf allocate fail");
		return NULL;
	}

	if ((nbuf_len + L3_HEADER_PADDING) > RX_DATA_BUFFER_SIZE) {
		qdf_nbuf_free(dst_nbuf);
		dp_err_rl("nbuf is jumbo data");
		return NULL;
	}

	/* prepeare to copy all data into new skb */
	dst_nbuf_data = qdf_nbuf_data(dst_nbuf);
	while (temp_nbuf) {
		src_nbuf_data = qdf_nbuf_data(temp_nbuf);
		/* first head nbuf */
		if (is_nbuf_head) {
			qdf_mem_copy(dst_nbuf_data, src_nbuf_data,
				     RX_PKT_TLVS_LEN);
			/* leave extra 2 bytes L3_HEADER_PADDING */
			dst_nbuf_data += (RX_PKT_TLVS_LEN + L3_HEADER_PADDING);
			src_nbuf_data += RX_PKT_TLVS_LEN;
			copy_len = qdf_nbuf_headlen(temp_nbuf) -
						RX_PKT_TLVS_LEN;
			temp_nbuf = qdf_nbuf_get_ext_list(temp_nbuf);
			is_nbuf_head = false;
		} else {
			copy_len = qdf_nbuf_len(temp_nbuf);
			temp_nbuf = qdf_nbuf_queue_next(temp_nbuf);
		}
		qdf_mem_copy(dst_nbuf_data, src_nbuf_data, copy_len);
		dst_nbuf_data += copy_len;
	}

	qdf_nbuf_set_len(dst_nbuf, nbuf_len);
	/* copy is done, free original nbuf */
	qdf_nbuf_free(nbuf);

	return dst_nbuf;
}

/**
 * dp_ipa_handle_rx_reo_reinject - Handle RX REO reinject skb buffer
 * @soc: soc
 * @nbuf: skb
 *
 * Return: nbuf if success and otherwise NULL
 */
qdf_nbuf_t dp_ipa_handle_rx_reo_reinject(struct dp_soc *soc, qdf_nbuf_t nbuf)
{

	if (!wlan_cfg_is_ipa_enabled(soc->wlan_cfg_ctx))
		return nbuf;

	/* WLAN IPA is run-time disabled */
	if (!qdf_atomic_read(&soc->ipa_pipes_enabled))
		return nbuf;

	if (!qdf_nbuf_is_frag(nbuf))
		return nbuf;

	/* linearize skb for IPA */
	return dp_ipa_frag_nbuf_linearize(soc, nbuf);
}

QDF_STATUS dp_ipa_tx_buf_smmu_mapping(
	struct cdp_soc_t *soc_hdl, uint8_t pdev_id)
{
	QDF_STATUS ret;

	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (!qdf_mem_smmu_s1_enabled(soc->osdev)) {
		dp_debug("SMMU S1 disabled");
		return QDF_STATUS_SUCCESS;
	}
	ret = __dp_ipa_tx_buf_smmu_mapping(soc, pdev, true);
	return ret;
}

#endif

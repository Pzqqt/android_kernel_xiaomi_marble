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

#include <qdf_types.h>
#include <qdf_lock.h>
#include <qdf_net_types.h>
#include <qdf_lro.h>
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_htt.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "dp_rx.h"
#include <cdp_txrx_handle.h>
#include <wlan_cfg.h>
#include "cdp_txrx_cmn_struct.h"
#include <qdf_util.h>
#include "dp_peer.h"
#include "dp_rx_mon.h"
#include "htt_stats.h"

#define DP_INTR_POLL_TIMER_MS	10
#define DP_MCS_LENGTH (6*MAX_MCS)
#define DP_NSS_LENGTH (6*SS_COUNT)
#define DP_RXDMA_ERR_LENGTH (6*MAX_RXDMA_ERRORS)
#define DP_REO_ERR_LENGTH (6*REO_ERROR_TYPE_MAX)
#define DP_CURR_FW_STATS_AVAIL 19
#define DP_HTT_DBG_EXT_STATS_MAX 256

/**
 * default_dscp_tid_map - Default DSCP-TID mapping
 *
 * DSCP        TID     AC
 * 000000      0       WME_AC_BE
 * 001000      1       WME_AC_BK
 * 010000      1       WME_AC_BK
 * 011000      0       WME_AC_BE
 * 100000      5       WME_AC_VI
 * 101000      5       WME_AC_VI
 * 110000      6       WME_AC_VO
 * 111000      6       WME_AC_VO
 */
static uint8_t default_dscp_tid_map[DSCP_TID_MAP_MAX] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
};

/**
 * @brief Select the type of statistics
 */
enum dp_stats_type {
	STATS_FW   = 0,
	STATS_HOST = 1,
	STATS_TYPE_MAX  = 2,
};

/**
 * @brief General Firmware statistics options
 *
 */
enum dp_fw_stats {
	TXRX_FW_STATS_INVALID	= -1,
};

/**
 * @brief Firmware and Host statistics
 * currently supported
 */
const int dp_stats_mapping_table[][STATS_TYPE_MAX] = {
	{HTT_DBG_EXT_STATS_RESET, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_RX, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_HWQ, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_SCHED, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_ERROR, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TQM, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TQM_CMDQ, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TX_DE_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_RATE, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_RX_RATE, TXRX_HOST_STATS_INVALID},
	{TXRX_FW_STATS_INVALID, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TX_SELFGEN_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_TX_MU_HWQ, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_RING_IF_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_SRNG_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_SFM_INFO, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_PDEV_TX_MU, TXRX_HOST_STATS_INVALID},
	{HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST, TXRX_HOST_STATS_INVALID},
	/* Last ENUM for HTT FW STATS */
	{DP_HTT_DBG_EXT_STATS_MAX, TXRX_HOST_STATS_INVALID},
	{TXRX_FW_STATS_INVALID, TXRX_CLEAR_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_RX_RATE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_TX_RATE_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_TX_HOST_STATS},
	{TXRX_FW_STATS_INVALID, TXRX_RX_HOST_STATS},
};

/**
 * dp_setup_srng - Internal function to setup SRNG rings used by data path
 */
static int dp_srng_setup(struct dp_soc *soc, struct dp_srng *srng,
	int ring_type, int ring_num, int mac_id, uint32_t num_entries)
{
	void *hal_soc = soc->hal_soc;
	uint32_t entry_size = hal_srng_get_entrysize(hal_soc, ring_type);
	/* TODO: See if we should get align size from hal */
	uint32_t ring_base_align = 8;
	struct hal_srng_params ring_params;
	uint32_t max_entries = hal_srng_max_entries(hal_soc, ring_type);

	num_entries = (num_entries > max_entries) ? max_entries : num_entries;
	srng->hal_srng = NULL;
	srng->alloc_size = (num_entries * entry_size) + ring_base_align - 1;
	srng->base_vaddr_unaligned = qdf_mem_alloc_consistent(
		soc->osdev, soc->osdev->dev, srng->alloc_size,
		&(srng->base_paddr_unaligned));

	if (!srng->base_vaddr_unaligned) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("alloc failed - ring_type: %d, ring_num %d"),
			ring_type, ring_num);
		return QDF_STATUS_E_NOMEM;
	}

	ring_params.ring_base_vaddr = srng->base_vaddr_unaligned +
		((unsigned long)srng->base_vaddr_unaligned % ring_base_align);
	ring_params.ring_base_paddr = srng->base_paddr_unaligned +
		((unsigned long)(ring_params.ring_base_vaddr) -
		(unsigned long)srng->base_vaddr_unaligned);
	ring_params.num_entries = num_entries;

	/* TODO: Check MSI support and get MSI settings from HIF layer */
	ring_params.msi_data = 0;
	ring_params.msi_addr = 0;

	/* TODO: Setup interrupt timer and batch counter thresholds for
	 * interrupt mitigation based on ring type
	 */
	ring_params.intr_timer_thres_us = 8;
	ring_params.intr_batch_cntr_thres_entries = 1;

	/* TODO: Currently hal layer takes care of endianness related settings.
	 * See if these settings need to passed from DP layer
	 */
	ring_params.flags = 0;

	/* Enable low threshold interrupts for rx buffer rings (regular and
	 * monitor buffer rings.
	 * TODO: See if this is required for any other ring
	 */
	if ((ring_type == RXDMA_BUF) || (ring_type == RXDMA_MONITOR_BUF)) {
		/* TODO: Setting low threshold to 1/8th of ring size
		 * see if this needs to be configurable
		 */
		ring_params.low_threshold = num_entries >> 3;
		ring_params.flags |= HAL_SRNG_LOW_THRES_INTR_ENABLE;
	}

	srng->hal_srng = hal_srng_setup(hal_soc, ring_type, ring_num,
		mac_id, &ring_params);
	return 0;
}

/**
 * dp_srng_cleanup - Internal function to cleanup SRNG rings used by data path
 * Any buffers allocated and attached to ring entries are expected to be freed
 * before calling this function.
 */
static void dp_srng_cleanup(struct dp_soc *soc, struct dp_srng *srng,
	int ring_type, int ring_num)
{
	if (!srng->hal_srng) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("Ring type: %d, num:%d not setup"),
			ring_type, ring_num);
		return;
	}

	hal_srng_cleanup(soc->hal_soc, srng->hal_srng);

	qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				srng->alloc_size,
				srng->base_vaddr_unaligned,
				srng->base_paddr_unaligned, 0);
}

/* TODO: Need this interface from HIF */
void *hif_get_hal_handle(void *hif_handle);

/*
 * dp_service_srngs() - Top level interrupt handler for DP Ring interrupts
 * @dp_ctx: DP SOC handle
 * @budget: Number of frames/descriptors that can be processed in one shot
 *
 * Return: remaining budget/quota for the soc device
 */
static uint32_t dp_service_srngs(void *dp_ctx, uint32_t dp_budget)
{
	struct dp_intr *int_ctx = (struct dp_intr *)dp_ctx;
	struct dp_soc *soc = int_ctx->soc;
	int ring = 0;
	uint32_t work_done  = 0;
	uint32_t budget = dp_budget;
	uint8_t tx_mask = int_ctx->tx_ring_mask;
	uint8_t rx_mask = int_ctx->rx_ring_mask;
	uint8_t rx_err_mask = int_ctx->rx_err_ring_mask;
	uint8_t rx_wbm_rel_mask = int_ctx->rx_wbm_rel_ring_mask;
	uint8_t reo_status_mask = int_ctx->reo_status_ring_mask;

	/* Process Tx completion interrupts first to return back buffers */
	if (tx_mask) {
		for (ring = 0; ring < soc->num_tcl_data_rings; ring++) {
			if (tx_mask & (1 << ring)) {
				work_done =
					dp_tx_comp_handler(soc, ring, budget);
				budget -= work_done;
				if (work_done)
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_INFO,
						"tx mask 0x%x ring %d,"
						"budget %d",
						tx_mask, ring, budget);
				if (budget <= 0)
					goto budget_done;
			}
		}
	}

	/* Process REO Exception ring interrupt */
	if (rx_err_mask) {
		work_done = dp_rx_err_process(soc,
				soc->reo_exception_ring.hal_srng, budget);
		budget -=  work_done;

		if (work_done)
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"REO Exception Ring: work_done %d budget %d",
				work_done, budget);
		if (budget <= 0) {
			goto budget_done;
		}
	}

	/* Process Rx WBM release ring interrupt */
	if (rx_wbm_rel_mask) {
		work_done = dp_rx_wbm_err_process(soc,
				soc->rx_rel_ring.hal_srng, budget);
		budget -=  work_done;

		if (work_done)
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"WBM Release Ring: work_done %d budget %d",
				work_done, budget);
		if (budget <= 0) {
			goto budget_done;
		}
	}

	/* Process Rx interrupts */
	if (rx_mask) {
		for (ring = 0; ring < soc->num_reo_dest_rings; ring++) {
			if (rx_mask & (1 << ring)) {
				work_done =
					dp_rx_process(int_ctx,
					    soc->reo_dest_ring[ring].hal_srng,
					    budget);
				budget -=  work_done;
				if (work_done)
					QDF_TRACE(QDF_MODULE_ID_DP,
						QDF_TRACE_LEVEL_INFO,
						"rx mask 0x%x ring %d,"
						"budget %d",
						tx_mask, ring, budget);
				if (budget <= 0)
					goto budget_done;
			}
		}
	}

	if (reo_status_mask)
		dp_reo_status_ring_handler(soc);

	/* Process Rx monitor interrupts */
	for  (ring = 0 ; ring < MAX_PDEV_CNT; ring++) {
		if (int_ctx->rx_mon_ring_mask & (1 << ring)) {
			work_done =
				dp_mon_process(soc, ring, budget);
			budget -=  work_done;
		}
	}

	qdf_lro_flush(int_ctx->lro_ctx);

budget_done:
	return dp_budget - budget;
}

/* dp_interrupt_timer()- timer poll for interrupts
 *
 * @arg: SoC Handle
 *
 * Return:
 *
 */
#ifdef DP_INTR_POLL_BASED
static void dp_interrupt_timer(void *arg)
{
	struct dp_soc *soc = (struct dp_soc *) arg;
	int i;

	if (qdf_atomic_read(&soc->cmn_init_done)) {
		for (i = 0;
			i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++)
			dp_service_srngs(&soc->intr_ctx[i], 0xffff);

		qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);
	}
}

/*
 * dp_soc_interrupt_attach() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Host driver will register for “DP_NUM_INTERRUPT_CONTEXTS” number of NAPI
 * contexts. Each NAPI context will have a tx_ring_mask , rx_ring_mask ,and
 * rx_monitor_ring mask to indicate the rings that are processed by the handler.
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_soc_interrupt_attach(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		soc->intr_ctx[i].tx_ring_mask = 0xF;
		soc->intr_ctx[i].rx_ring_mask = 0xF;
		soc->intr_ctx[i].rx_mon_ring_mask = 0x1;
		soc->intr_ctx[i].rx_err_ring_mask = 0x1;
		soc->intr_ctx[i].rx_wbm_rel_ring_mask = 0x1;
		soc->intr_ctx[i].reo_status_ring_mask = 0x1;
		soc->intr_ctx[i].soc = soc;
		soc->intr_ctx[i].lro_ctx = qdf_lro_init();
	}

	qdf_timer_init(soc->osdev, &soc->int_timer,
			dp_interrupt_timer, (void *)soc,
			QDF_TIMER_TYPE_WAKE_APPS);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_soc_interrupt_detach() - Deregister any allocations done for interrupts
 * @txrx_soc: DP SOC handle
 *
 * Return: void
 */
static void dp_soc_interrupt_detach(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	qdf_timer_stop(&soc->int_timer);

	qdf_timer_free(&soc->int_timer);
}
#else
/*
 * dp_soc_interrupt_attach() - Register handlers for DP interrupts
 * @txrx_soc: DP SOC handle
 *
 * Host driver will register for “DP_NUM_INTERRUPT_CONTEXTS” number of NAPI
 * contexts. Each NAPI context will have a tx_ring_mask , rx_ring_mask ,and
 * rx_monitor_ring mask to indicate the rings that are processed by the handler.
 *
 * Return: 0 for success. nonzero for failure.
 */
static QDF_STATUS dp_soc_interrupt_attach(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;

	int i = 0;
	int num_irq = 0;


	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		int j = 0;
		int ret = 0;

		/* Map of IRQ ids registered with one interrupt context */
		int irq_id_map[HIF_MAX_GRP_IRQ];

		int tx_mask =
			wlan_cfg_get_tx_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_mask =
			wlan_cfg_get_rx_ring_mask(soc->wlan_cfg_ctx, i);
		int rx_mon_mask =
			wlan_cfg_get_rx_mon_ring_mask(soc->wlan_cfg_ctx, i);

		soc->intr_ctx[i].tx_ring_mask = tx_mask;
		soc->intr_ctx[i].rx_ring_mask = rx_mask;
		soc->intr_ctx[i].rx_mon_ring_mask = rx_mon_mask;
		soc->intr_ctx[i].soc = soc;

		num_irq = 0;

		for (j = 0; j < HIF_MAX_GRP_IRQ; j++) {

			if (tx_mask & (1 << j)) {
				irq_id_map[num_irq++] =
					(wbm2host_tx_completions_ring1 - j);
			}

			if (rx_mask & (1 << j)) {
				irq_id_map[num_irq++] =
					(reo2host_destination_ring1 - j);
			}

			if (rx_mon_mask & (1 << j)) {
				irq_id_map[num_irq++] =
					(rxdma2host_monitor_destination_mac1
					 - j);
			}
		}


		ret = hif_register_ext_group_int_handler(soc->hif_handle,
				num_irq, irq_id_map,
				dp_service_srngs,
				&soc->intr_ctx[i]);

		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("failed, ret = %d"), ret);

			return QDF_STATUS_E_FAILURE;
		}
		soc->intr_ctx[i].lro_ctx = qdf_lro_init();
	}

	hif_configure_ext_group_interrupts(soc->hif_handle);

	return QDF_STATUS_SUCCESS;
}

/*
 * dp_soc_interrupt_detach() - Deregister any allocations done for interrupts
 * @txrx_soc: DP SOC handle
 *
 * Return: void
 */
static void dp_soc_interrupt_detach(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	for (i = 0; i < wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx); i++) {
		soc->intr_ctx[i].tx_ring_mask = 0;
		soc->intr_ctx[i].rx_ring_mask = 0;
		soc->intr_ctx[i].rx_mon_ring_mask = 0;
		qdf_lro_deinit(soc->intr_ctx[i].lro_ctx);
	}
}
#endif

#define AVG_MAX_MPDUS_PER_TID 128
#define AVG_TIDS_PER_CLIENT 2
#define AVG_FLOWS_PER_TID 2
#define AVG_MSDUS_PER_FLOW 128
#define AVG_MSDUS_PER_MPDU 4

/*
 * Allocate and setup link descriptor pool that will be used by HW for
 * various link and queue descriptors and managed by WBM
 */
static int dp_hw_link_desc_pool_setup(struct dp_soc *soc)
{
	int link_desc_size = hal_get_link_desc_size(soc->hal_soc);
	int link_desc_align = hal_get_link_desc_align(soc->hal_soc);
	uint32_t max_clients = wlan_cfg_get_max_clients(soc->wlan_cfg_ctx);
	uint32_t num_mpdus_per_link_desc =
		hal_num_mpdus_per_link_desc(soc->hal_soc);
	uint32_t num_msdus_per_link_desc =
		hal_num_msdus_per_link_desc(soc->hal_soc);
	uint32_t num_mpdu_links_per_queue_desc =
		hal_num_mpdu_links_per_queue_desc(soc->hal_soc);
	uint32_t max_alloc_size = wlan_cfg_max_alloc_size(soc->wlan_cfg_ctx);
	uint32_t total_link_descs, total_mem_size;
	uint32_t num_mpdu_link_descs, num_mpdu_queue_descs;
	uint32_t num_tx_msdu_link_descs, num_rx_msdu_link_descs;
	uint32_t num_link_desc_banks;
	uint32_t last_bank_size = 0;
	uint32_t entry_size, num_entries;
	int i;

	/* Only Tx queue descriptors are allocated from common link descriptor
	 * pool Rx queue descriptors are not included in this because (REO queue
	 * extension descriptors) they are expected to be allocated contiguously
	 * with REO queue descriptors
	 */
	num_mpdu_link_descs = (max_clients * AVG_TIDS_PER_CLIENT *
		AVG_MAX_MPDUS_PER_TID) / num_mpdus_per_link_desc;

	num_mpdu_queue_descs = num_mpdu_link_descs /
		num_mpdu_links_per_queue_desc;

	num_tx_msdu_link_descs = (max_clients * AVG_TIDS_PER_CLIENT *
		AVG_FLOWS_PER_TID * AVG_MSDUS_PER_FLOW) /
		num_msdus_per_link_desc;

	num_rx_msdu_link_descs = (max_clients * AVG_TIDS_PER_CLIENT *
		AVG_MAX_MPDUS_PER_TID * AVG_MSDUS_PER_MPDU) / 6;

	num_entries = num_mpdu_link_descs + num_mpdu_queue_descs +
		num_tx_msdu_link_descs + num_rx_msdu_link_descs;

	/* Round up to power of 2 */
	total_link_descs = 1;
	while (total_link_descs < num_entries)
		total_link_descs <<= 1;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("total_link_descs: %u, link_desc_size: %d"),
		total_link_descs, link_desc_size);
	total_mem_size =  total_link_descs * link_desc_size;

	total_mem_size += link_desc_align;

	if (total_mem_size <= max_alloc_size) {
		num_link_desc_banks = 0;
		last_bank_size = total_mem_size;
	} else {
		num_link_desc_banks = (total_mem_size) /
			(max_alloc_size - link_desc_align);
		last_bank_size = total_mem_size %
			(max_alloc_size - link_desc_align);
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("total_mem_size: %d, num_link_desc_banks: %u"),
		total_mem_size, num_link_desc_banks);

	for (i = 0; i < num_link_desc_banks; i++) {
		soc->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
			max_alloc_size,
			&(soc->link_desc_banks[i].base_paddr_unaligned));
		soc->link_desc_banks[i].size = max_alloc_size;

		soc->link_desc_banks[i].base_vaddr = (void *)((unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned) +
			((unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned) %
			link_desc_align));

		soc->link_desc_banks[i].base_paddr = (unsigned long)(
			soc->link_desc_banks[i].base_paddr_unaligned) +
			((unsigned long)(soc->link_desc_banks[i].base_vaddr) -
			(unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned));

		if (!soc->link_desc_banks[i].base_vaddr_unaligned) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Link descriptor memory alloc failed"));
			goto fail;
		}
	}

	if (last_bank_size) {
		/* Allocate last bank in case total memory required is not exact
		 * multiple of max_alloc_size
		 */
		soc->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
			last_bank_size,
			&(soc->link_desc_banks[i].base_paddr_unaligned));
		soc->link_desc_banks[i].size = last_bank_size;

		soc->link_desc_banks[i].base_vaddr = (void *)((unsigned long)
			(soc->link_desc_banks[i].base_vaddr_unaligned) +
			((unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned) %
			link_desc_align));

		soc->link_desc_banks[i].base_paddr =
			(unsigned long)(
			soc->link_desc_banks[i].base_paddr_unaligned) +
			((unsigned long)(soc->link_desc_banks[i].base_vaddr) -
			(unsigned long)(
			soc->link_desc_banks[i].base_vaddr_unaligned));
	}


	/* Allocate and setup link descriptor idle list for HW internal use */
	entry_size = hal_srng_get_entrysize(soc->hal_soc, WBM_IDLE_LINK);
	total_mem_size = entry_size * total_link_descs;

	if (total_mem_size <= max_alloc_size) {
		void *desc;

		if (dp_srng_setup(soc, &soc->wbm_idle_link_ring,
			WBM_IDLE_LINK, 0, 0, total_link_descs)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("Link desc idle ring setup failed"));
			goto fail;
		}

		hal_srng_access_start_unlocked(soc->hal_soc,
			soc->wbm_idle_link_ring.hal_srng);

		for (i = 0; i < MAX_LINK_DESC_BANKS &&
			soc->link_desc_banks[i].base_paddr; i++) {
			uint32_t num_entries = (soc->link_desc_banks[i].size -
				(unsigned long)(
				soc->link_desc_banks[i].base_vaddr) -
				(unsigned long)(
				soc->link_desc_banks[i].base_vaddr_unaligned))
				/ link_desc_size;
			unsigned long paddr = (unsigned long)(
				soc->link_desc_banks[i].base_paddr);

			while (num_entries && (desc = hal_srng_src_get_next(
				soc->hal_soc,
				soc->wbm_idle_link_ring.hal_srng))) {
				hal_set_link_desc_addr(desc, i, paddr);
				num_entries--;
				paddr += link_desc_size;
			}
		}
		hal_srng_access_end_unlocked(soc->hal_soc,
			soc->wbm_idle_link_ring.hal_srng);
	} else {
		uint32_t num_scatter_bufs;
		uint32_t num_entries_per_buf;
		uint32_t rem_entries;
		uint8_t *scatter_buf_ptr;
		uint16_t scatter_buf_num;

		soc->wbm_idle_scatter_buf_size =
			hal_idle_list_scatter_buf_size(soc->hal_soc);
		num_entries_per_buf = hal_idle_scatter_buf_num_entries(
			soc->hal_soc, soc->wbm_idle_scatter_buf_size);
		num_scatter_bufs = (total_mem_size /
			soc->wbm_idle_scatter_buf_size) + (total_mem_size %
				soc->wbm_idle_scatter_buf_size) ? 1 : 0;

		for (i = 0; i < num_scatter_bufs; i++) {
			soc->wbm_idle_scatter_buf_base_vaddr[i] =
				qdf_mem_alloc_consistent(soc->osdev, soc->osdev->dev,
				soc->wbm_idle_scatter_buf_size,
				&(soc->wbm_idle_scatter_buf_base_paddr[i]));
			if (soc->wbm_idle_scatter_buf_base_vaddr[i] == NULL) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("Scatter list memory alloc failed"));
				goto fail;
			}
		}

		/* Populate idle list scatter buffers with link descriptor
		 * pointers
		 */
		scatter_buf_num = 0;
		scatter_buf_ptr = (uint8_t *)(
			soc->wbm_idle_scatter_buf_base_vaddr[scatter_buf_num]);
		rem_entries = num_entries_per_buf;

		for (i = 0; i < MAX_LINK_DESC_BANKS &&
			soc->link_desc_banks[i].base_paddr; i++) {
			uint32_t num_link_descs =
				(soc->link_desc_banks[i].size -
				(unsigned long)(
				soc->link_desc_banks[i].base_vaddr) -
				(unsigned long)(
				soc->link_desc_banks[i].base_vaddr_unaligned)) /
				link_desc_size;
			unsigned long paddr = (unsigned long)(
				soc->link_desc_banks[i].base_paddr);
			void *desc = NULL;

			while (num_link_descs && (desc =
				hal_srng_src_get_next(soc->hal_soc,
				soc->wbm_idle_link_ring.hal_srng))) {
				hal_set_link_desc_addr((void *)scatter_buf_ptr,
					i, paddr);
				num_link_descs--;
				paddr += link_desc_size;
				if (rem_entries) {
					rem_entries--;
					scatter_buf_ptr += link_desc_size;
				} else {
					rem_entries = num_entries_per_buf;
					scatter_buf_num++;
					scatter_buf_ptr = (uint8_t *)(
						soc->wbm_idle_scatter_buf_base_vaddr[
						scatter_buf_num]);
				}
			}
		}
		/* Setup link descriptor idle list in HW */
		hal_setup_link_idle_list(soc->hal_soc,
			soc->wbm_idle_scatter_buf_base_paddr,
			soc->wbm_idle_scatter_buf_base_vaddr,
			num_scatter_bufs, soc->wbm_idle_scatter_buf_size,
			(uint32_t)(scatter_buf_ptr -
					(uint8_t *)(soc->wbm_idle_scatter_buf_base_vaddr[
			scatter_buf_num])));
	}
	return 0;

fail:
	if (soc->wbm_idle_link_ring.hal_srng) {
		dp_srng_cleanup(soc->hal_soc, &soc->wbm_idle_link_ring,
			WBM_IDLE_LINK, 0);
	}

	for (i = 0; i < MAX_IDLE_SCATTER_BUFS; i++) {
		if (soc->wbm_idle_scatter_buf_base_vaddr[i]) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->wbm_idle_scatter_buf_size,
				soc->wbm_idle_scatter_buf_base_vaddr[i],
				soc->wbm_idle_scatter_buf_base_paddr[i], 0);
		}
	}

	for (i = 0; i < MAX_LINK_DESC_BANKS; i++) {
		if (soc->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->link_desc_banks[i].size,
				soc->link_desc_banks[i].base_vaddr_unaligned,
				soc->link_desc_banks[i].base_paddr_unaligned,
				0);
		}
	}
	return QDF_STATUS_E_FAILURE;
}

#ifdef notused
/*
 * Free link descriptor pool that was setup HW
 */
static void dp_hw_link_desc_pool_cleanup(struct dp_soc *soc)
{
	int i;

	if (soc->wbm_idle_link_ring.hal_srng) {
		dp_srng_cleanup(soc->hal_soc, &soc->wbm_idle_link_ring,
			WBM_IDLE_LINK, 0);
	}

	for (i = 0; i < MAX_IDLE_SCATTER_BUFS; i++) {
		if (soc->wbm_idle_scatter_buf_base_vaddr[i]) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->wbm_idle_scatter_buf_size,
				soc->wbm_idle_scatter_buf_base_vaddr[i],
				soc->wbm_idle_scatter_buf_base_paddr[i], 0);
		}
	}

	for (i = 0; i < MAX_LINK_DESC_BANKS; i++) {
		if (soc->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				soc->link_desc_banks[i].size,
				soc->link_desc_banks[i].base_vaddr_unaligned,
				soc->link_desc_banks[i].base_paddr_unaligned,
				0);
		}
	}
}
#endif /* notused */

/* TODO: Following should be configurable */
#define WBM_RELEASE_RING_SIZE 64
#define TCL_DATA_RING_SIZE 512
#define TX_COMP_RING_SIZE 1024
#define TCL_CMD_RING_SIZE 32
#define TCL_STATUS_RING_SIZE 32
#define REO_DST_RING_SIZE 2048
#define REO_REINJECT_RING_SIZE 32
#define RX_RELEASE_RING_SIZE 1024
#define REO_EXCEPTION_RING_SIZE 128
#define REO_CMD_RING_SIZE 32
#define REO_STATUS_RING_SIZE 32
#define RXDMA_BUF_RING_SIZE 1024
#define RXDMA_REFILL_RING_SIZE 2048
#define RXDMA_MONITOR_BUF_RING_SIZE 1024
#define RXDMA_MONITOR_DST_RING_SIZE 1024
#define RXDMA_MONITOR_STATUS_RING_SIZE 1024
#define RXDMA_MONITOR_DESC_RING_SIZE 1024

/*
 * dp_soc_cmn_setup() - Common SoC level initializion
 * @soc:		Datapath SOC handle
 *
 * This is an internal function used to setup common SOC data structures,
 * to be called from PDEV attach after receiving HW mode capabilities from FW
 */
static int dp_soc_cmn_setup(struct dp_soc *soc)
{
	int i;
	struct hal_reo_params reo_params;

	if (qdf_atomic_read(&soc->cmn_init_done))
		return 0;

	if (dp_peer_find_attach(soc))
		goto fail0;

	if (dp_hw_link_desc_pool_setup(soc))
		goto fail1;

	/* Setup SRNG rings */
	/* Common rings */
	if (dp_srng_setup(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0, 0,
		WBM_RELEASE_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for wbm_desc_rel_ring"));
		goto fail1;
	}


	soc->num_tcl_data_rings = 0;
	/* Tx data rings */
	if (!wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		soc->num_tcl_data_rings =
			wlan_cfg_num_tcl_data_rings(soc->wlan_cfg_ctx);
		for (i = 0; i < soc->num_tcl_data_rings; i++) {
			if (dp_srng_setup(soc, &soc->tcl_data_ring[i],
				TCL_DATA, i, 0, TCL_DATA_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for tcl_data_ring[%d]"), i);
				goto fail1;
			}
			if (dp_srng_setup(soc, &soc->tx_comp_ring[i],
				WBM2SW_RELEASE, i, 0, TX_COMP_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for tx_comp_ring[%d]"), i);
				goto fail1;
			}
		}
	} else {
		/* This will be incremented during per pdev ring setup */
		soc->num_tcl_data_rings = 0;
	}

	if (dp_tx_soc_attach(soc)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_tx_soc_attach failed"));
		goto fail1;
	}

	/* TCL command and status rings */
	if (dp_srng_setup(soc, &soc->tcl_cmd_ring, TCL_CMD, 0, 0,
		TCL_CMD_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for tcl_cmd_ring"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &soc->tcl_status_ring, TCL_STATUS, 0, 0,
		TCL_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for tcl_status_ring"));
		goto fail1;
	}


	/* TBD: call dp_tx_init to setup Tx SW descriptors and MSDU extension
	 * descriptors
	 */

	/* Rx data rings */
	if (!wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		soc->num_reo_dest_rings =
			wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);
		QDF_TRACE(QDF_MODULE_ID_DP,
			QDF_TRACE_LEVEL_ERROR,
			FL("num_reo_dest_rings %d\n"), soc->num_reo_dest_rings);
		for (i = 0; i < soc->num_reo_dest_rings; i++) {
			if (dp_srng_setup(soc, &soc->reo_dest_ring[i], REO_DST,
				i, 0, REO_DST_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_ERROR,
					FL("dp_srng_setup failed for reo_dest_ring[%d]"), i);
				goto fail1;
			}
		}
	} else {
		/* This will be incremented during per pdev ring setup */
		soc->num_reo_dest_rings = 0;
	}

	/* TBD: call dp_rx_init to setup Rx SW descriptors */

	/* REO reinjection ring */
	if (dp_srng_setup(soc, &soc->reo_reinject_ring, REO_REINJECT, 0, 0,
		REO_REINJECT_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_reinject_ring"));
		goto fail1;
	}


	/* Rx release ring */
	if (dp_srng_setup(soc, &soc->rx_rel_ring, WBM2SW_RELEASE, 3, 0,
		RX_RELEASE_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rx_rel_ring"));
		goto fail1;
	}


	/* Rx exception ring */
	if (dp_srng_setup(soc, &soc->reo_exception_ring, REO_EXCEPTION, 0,
		MAX_REO_DEST_RINGS, REO_EXCEPTION_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_exception_ring"));
		goto fail1;
	}


	/* REO command and status rings */
	if (dp_srng_setup(soc, &soc->reo_cmd_ring, REO_CMD, 0, 0,
		REO_CMD_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_cmd_ring"));
		goto fail1;
	}

	hal_reo_init_cmd_ring(soc->hal_soc, soc->reo_cmd_ring.hal_srng);
	TAILQ_INIT(&soc->rx.reo_cmd_list);
	qdf_spinlock_create(&soc->rx.reo_cmd_lock);

	if (dp_srng_setup(soc, &soc->reo_status_ring, REO_STATUS, 0, 0,
		REO_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for reo_status_ring"));
		goto fail1;
	}

	/* Setup HW REO */
	qdf_mem_zero(&reo_params, sizeof(reo_params));

	if (wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx))
		reo_params.rx_hash_enabled = true;

	hal_reo_setup(soc->hal_soc, &reo_params);

	qdf_atomic_set(&soc->cmn_init_done, 1);
	qdf_nbuf_queue_init(&soc->htt_stats_msg);
	return 0;
fail1:
	/*
	 * Cleanup will be done as part of soc_detach, which will
	 * be called on pdev attach failure
	 */
fail0:
	return QDF_STATUS_E_FAILURE;
}

static void dp_pdev_detach_wifi3(struct cdp_pdev *txrx_pdev, int force);

static void dp_lro_hash_setup(struct dp_soc *soc)
{
	struct cdp_lro_hash_config lro_hash;

	if (!wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx) &&
		!wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 FL("LRO disabled RX hash disabled"));
		return;
	}

	qdf_mem_zero(&lro_hash, sizeof(lro_hash));

	if (wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx)) {
		lro_hash.lro_enable = 1;
		lro_hash.tcp_flag = QDF_TCPHDR_ACK;
		lro_hash.tcp_flag_mask = QDF_TCPHDR_FIN | QDF_TCPHDR_SYN |
				 QDF_TCPHDR_RST | QDF_TCPHDR_ACK | QDF_TCPHDR_URG |
				 QDF_TCPHDR_ECE | QDF_TCPHDR_CWR;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR, FL("enabled"));
		qdf_get_random_bytes(lro_hash.toeplitz_hash_ipv4,
		 (sizeof(lro_hash.toeplitz_hash_ipv4[0]) *
		 LRO_IPV4_SEED_ARR_SZ));

	qdf_get_random_bytes(lro_hash.toeplitz_hash_ipv6,
		 (sizeof(lro_hash.toeplitz_hash_ipv6[0]) *
		 LRO_IPV6_SEED_ARR_SZ));

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		 "lro_hash: lro_enable: 0x%x"
		 "lro_hash: tcp_flag 0x%x tcp_flag_mask 0x%x",
		 lro_hash.lro_enable, lro_hash.tcp_flag,
		 lro_hash.tcp_flag_mask);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		FL("lro_hash: toeplitz_hash_ipv4:"));
	qdf_trace_hex_dump(QDF_MODULE_ID_DP,
		 QDF_TRACE_LEVEL_ERROR,
		 (void *)lro_hash.toeplitz_hash_ipv4,
		 (sizeof(lro_hash.toeplitz_hash_ipv4[0]) *
		 LRO_IPV4_SEED_ARR_SZ));

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		FL("lro_hash: toeplitz_hash_ipv6:"));
	qdf_trace_hex_dump(QDF_MODULE_ID_DP,
		 QDF_TRACE_LEVEL_ERROR,
		 (void *)lro_hash.toeplitz_hash_ipv6,
		 (sizeof(lro_hash.toeplitz_hash_ipv6[0]) *
		 LRO_IPV6_SEED_ARR_SZ));

	qdf_assert(soc->cdp_soc.ol_ops->lro_hash_config);

	if (soc->cdp_soc.ol_ops->lro_hash_config)
		(void)soc->cdp_soc.ol_ops->lro_hash_config
			(soc->osif_soc, &lro_hash);
}

/*
* dp_rxdma_ring_setup() - configure the RX DMA rings
* @soc: data path SoC handle
* @pdev: Physical device handle
*
* Return: 0 - success, > 0 - failure
*/
#ifdef QCA_HOST2FW_RXBUF_RING
static int dp_rxdma_ring_setup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
	int max_mac_rings =
		 wlan_cfg_get_num_mac_rings
			(pdev->wlan_cfg_ctx);
	int i;

	for (i = 0; i < max_mac_rings; i++) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: pdev_id %d mac_id %d\n",
			 __func__, pdev->pdev_id, i);
		if (dp_srng_setup(soc, &pdev->rx_mac_buf_ring[i],
			 RXDMA_BUF, 1, i, RXDMA_BUF_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP,
				 QDF_TRACE_LEVEL_ERROR,
				 FL("failed rx mac ring setup"));
			return QDF_STATUS_E_FAILURE;
		}
	}
	return QDF_STATUS_SUCCESS;
}
#else
static int dp_rxdma_ring_setup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_dscp_tid_map_setup(): Initialize the dscp-tid maps
 * @pdev - DP_PDEV handle
 *
 * Return: void
 */
static inline void
dp_dscp_tid_map_setup(struct dp_pdev *pdev)
{
	uint8_t map_id;
	for (map_id = 0; map_id < DP_MAX_TID_MAPS; map_id++) {
		qdf_mem_copy(pdev->dscp_tid_map[map_id], default_dscp_tid_map,
				sizeof(default_dscp_tid_map));
	}
	for (map_id = 0; map_id < HAL_MAX_HW_DSCP_TID_MAPS; map_id++) {
		hal_tx_set_dscp_tid_map(pdev->soc->hal_soc,
				pdev->dscp_tid_map[map_id],
				map_id);
	}
}

/*
* dp_pdev_attach_wifi3() - attach txrx pdev
* @osif_pdev: Opaque PDEV handle from OSIF/HDD
* @txrx_soc: Datapath SOC handle
* @htc_handle: HTC handle for host-target interface
* @qdf_osdev: QDF OS device
* @pdev_id: PDEV ID
*
* Return: DP PDEV handle on success, NULL on failure
*/
static struct cdp_pdev *dp_pdev_attach_wifi3(struct cdp_soc_t *txrx_soc,
	struct cdp_cfg *ctrl_pdev,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev, uint8_t pdev_id)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct dp_pdev *pdev = qdf_mem_malloc(sizeof(*pdev));

	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("DP PDEV memory allocation failed"));
		goto fail0;
	}

	pdev->wlan_cfg_ctx = wlan_cfg_pdev_attach();

	if (!pdev->wlan_cfg_ctx) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("pdev cfg_attach failed"));

		qdf_mem_free(pdev);
		goto fail0;
	}

	/*
	 * set nss pdev config based on soc config
	 */
	wlan_cfg_set_dp_pdev_nss_enabled(pdev->wlan_cfg_ctx,
			(wlan_cfg_get_dp_soc_nss_cfg(soc->wlan_cfg_ctx) & (1 << pdev->pdev_id)));

	pdev->soc = soc;
	pdev->osif_pdev = ctrl_pdev;
	pdev->pdev_id = pdev_id;
	soc->pdev_list[pdev_id] = pdev;
	soc->pdev_count++;

	TAILQ_INIT(&pdev->vdev_list);
	pdev->vdev_count = 0;

	qdf_spinlock_create(&pdev->neighbour_peer_mutex);
	TAILQ_INIT(&pdev->neighbour_peers_list);

	if (dp_soc_cmn_setup(soc)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_soc_cmn_setup failed"));
		goto fail1;
	}

	/* Setup per PDEV TCL rings if configured */
	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		if (dp_srng_setup(soc, &soc->tcl_data_ring[pdev_id], TCL_DATA,
			pdev_id, pdev_id, TCL_DATA_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for tcl_data_ring"));
			goto fail1;
		}
		if (dp_srng_setup(soc, &soc->tx_comp_ring[pdev_id],
			WBM2SW_RELEASE, pdev_id, pdev_id, TCL_DATA_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for tx_comp_ring"));
			goto fail1;
		}
		soc->num_tcl_data_rings++;
	}

	/* Tx specific init */
	if (dp_tx_pdev_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_tx_pdev_attach failed"));
		goto fail1;
	}

	/* Setup per PDEV REO rings if configured */
	if (wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		if (dp_srng_setup(soc, &soc->reo_dest_ring[pdev_id], REO_DST,
			pdev_id, pdev_id, REO_DST_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_srng_setup failed for reo_dest_ringn"));
			goto fail1;
		}
		soc->num_reo_dest_rings++;

	}

	if (dp_srng_setup(soc, &pdev->rx_refill_buf_ring, RXDMA_BUF, 0, pdev_id,
		RXDMA_REFILL_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 FL("dp_srng_setup failed rx refill ring"));
		goto fail1;
	}

	if (dp_rxdma_ring_setup(soc, pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			 FL("RXDMA ring config failed"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_buf_ring, RXDMA_MONITOR_BUF, 0,
		pdev_id, RXDMA_MONITOR_BUF_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rxdma_mon_buf_ring"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_dst_ring, RXDMA_MONITOR_DST, 0,
		pdev_id, RXDMA_MONITOR_DST_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rxdma_mon_dst_ring"));
		goto fail1;
	}


	if (dp_srng_setup(soc, &pdev->rxdma_mon_status_ring,
		RXDMA_MONITOR_STATUS, 0, pdev_id,
		RXDMA_MONITOR_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("dp_srng_setup failed for rxdma_mon_status_ring"));
		goto fail1;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_desc_ring,
		RXDMA_MONITOR_DESC, 0, pdev_id, RXDMA_MONITOR_DESC_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"dp_srng_setup failed for rxdma_mon_desc_ring\n");
		goto fail1;
	}

	/* Rx specific init */
	if (dp_rx_pdev_attach(pdev)) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("dp_rx_pdev_attach failed "));
			goto fail0;
	}
	DP_STATS_INIT(pdev);

#ifndef CONFIG_WIN
	/* MCL */
	dp_local_peer_id_pool_init(pdev);
#endif
	dp_dscp_tid_map_setup(pdev);

	/* Rx monitor mode specific init */
	if (dp_rx_pdev_mon_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		"dp_rx_pdev_attach failed\n");
		goto fail0;
	}

	/* set the reo destination to 1 during initialization */
	pdev->reo_dest = 1;
	return (struct cdp_pdev *)pdev;

fail1:
	dp_pdev_detach_wifi3((struct cdp_pdev *)pdev, 0);

fail0:
	return NULL;
}

/*
* dp_rxdma_ring_cleanup() - configure the RX DMA rings
* @soc: data path SoC handle
* @pdev: Physical device handle
*
* Return: void
*/
#ifdef QCA_HOST2FW_RXBUF_RING
static void dp_rxdma_ring_cleanup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
	int max_mac_rings =
		 wlan_cfg_get_num_mac_rings(pdev->wlan_cfg_ctx);
	int i;

	max_mac_rings = max_mac_rings < MAX_RX_MAC_RINGS ?
				max_mac_rings : MAX_RX_MAC_RINGS;
	for (i = 0; i < MAX_RX_MAC_RINGS; i++)
		dp_srng_cleanup(soc, &pdev->rx_mac_buf_ring[i],
			 RXDMA_BUF, 1);
}
#else
static void dp_rxdma_ring_cleanup(struct dp_soc *soc,
	 struct dp_pdev *pdev)
{
}
#endif

/*
 * dp_neighbour_peers_detach() - Detach neighbour peers(nac clients)
 * @pdev: device object
 *
 * Return: void
 */
static void dp_neighbour_peers_detach(struct dp_pdev *pdev)
{
	struct dp_neighbour_peer *peer = NULL;
	struct dp_neighbour_peer *temp_peer = NULL;

	TAILQ_FOREACH_SAFE(peer, &pdev->neighbour_peers_list,
			neighbour_peer_list_elem, temp_peer) {
		/* delete this peer from the list */
		TAILQ_REMOVE(&pdev->neighbour_peers_list,
				peer, neighbour_peer_list_elem);
		qdf_mem_free(peer);
	}

	qdf_spinlock_destroy(&pdev->neighbour_peer_mutex);
}

/*
* dp_pdev_detach_wifi3() - detach txrx pdev
* @txrx_pdev: Datapath PDEV handle
* @force: Force detach
*
*/
static void dp_pdev_detach_wifi3(struct cdp_pdev *txrx_pdev, int force)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	struct dp_soc *soc = pdev->soc;

	dp_tx_pdev_detach(pdev);

	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		dp_srng_cleanup(soc, &soc->tcl_data_ring[pdev->pdev_id],
			TCL_DATA, pdev->pdev_id);
		dp_srng_cleanup(soc, &soc->tx_comp_ring[pdev->pdev_id],
			WBM2SW_RELEASE, pdev->pdev_id);
	}

	dp_rx_pdev_detach(pdev);

	dp_rx_pdev_mon_detach(pdev);

	dp_neighbour_peers_detach(pdev);

	/* Setup per PDEV REO rings if configured */
	if (wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		dp_srng_cleanup(soc, &soc->reo_dest_ring[pdev->pdev_id],
			REO_DST, pdev->pdev_id);
	}

	dp_srng_cleanup(soc, &pdev->rx_refill_buf_ring, RXDMA_BUF, 0);

	dp_rxdma_ring_cleanup(soc, pdev);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_buf_ring, RXDMA_MONITOR_BUF, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_dst_ring, RXDMA_MONITOR_DST, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_status_ring,
		RXDMA_MONITOR_STATUS, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_desc_ring,
		RXDMA_MONITOR_DESC, 0);

	soc->pdev_list[pdev->pdev_id] = NULL;
	soc->pdev_count--;
	wlan_cfg_pdev_detach(pdev->wlan_cfg_ctx);
	qdf_mem_free(pdev);
}

/*
 * dp_reo_desc_freelist_destroy() - Flush REO descriptors from deferred freelist
 * @soc: DP SOC handle
 */
static inline void dp_reo_desc_freelist_destroy(struct dp_soc *soc)
{
	struct reo_desc_list_node *desc;
	struct dp_rx_tid *rx_tid;

	qdf_spin_lock_bh(&soc->reo_desc_freelist_lock);
	while (qdf_list_remove_front(&soc->reo_desc_freelist,
		(qdf_list_node_t **)&desc) == QDF_STATUS_SUCCESS) {
		rx_tid = &desc->rx_tid;
		qdf_mem_unmap_nbytes_single(soc->osdev,
			rx_tid->hw_qdesc_paddr,
			QDF_DMA_BIDIRECTIONAL,
			rx_tid->hw_qdesc_alloc_size);
		qdf_mem_free(rx_tid->hw_qdesc_vaddr_unaligned);
		qdf_mem_free(desc);
	}
	qdf_spin_unlock_bh(&soc->reo_desc_freelist_lock);
	qdf_list_destroy(&soc->reo_desc_freelist);
	qdf_spinlock_destroy(&soc->reo_desc_freelist_lock);
}

/*
 * dp_soc_detach_wifi3() - Detach txrx SOC
 * @txrx_soc: DP SOC handle
 *
 */
static void dp_soc_detach_wifi3(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	qdf_atomic_set(&soc->cmn_init_done, 0);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		if (soc->pdev_list[i])
			dp_pdev_detach_wifi3(
				(struct cdp_pdev *)soc->pdev_list[i], 1);
	}

	dp_peer_find_detach(soc);

	/* TBD: Call Tx and Rx cleanup functions to free buffers and
	 * SW descriptors
	 */

	/* Free the ring memories */
	/* Common rings */
	dp_srng_cleanup(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0);

	dp_tx_soc_detach(soc);
	/* Tx data rings */
	if (!wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		for (i = 0; i < soc->num_tcl_data_rings; i++) {
			dp_srng_cleanup(soc, &soc->tcl_data_ring[i],
				TCL_DATA, i);
			dp_srng_cleanup(soc, &soc->tx_comp_ring[i],
				WBM2SW_RELEASE, i);
		}
	}

	/* TCL command and status rings */
	dp_srng_cleanup(soc, &soc->tcl_cmd_ring, TCL_CMD, 0);
	dp_srng_cleanup(soc, &soc->tcl_status_ring, TCL_STATUS, 0);

	/* Rx data rings */
	if (!wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		soc->num_reo_dest_rings =
			wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);
		for (i = 0; i < soc->num_reo_dest_rings; i++) {
			/* TODO: Get number of rings and ring sizes
			 * from wlan_cfg
			 */
			dp_srng_cleanup(soc, &soc->reo_dest_ring[i],
				REO_DST, i);
		}
	}
	/* REO reinjection ring */
	dp_srng_cleanup(soc, &soc->reo_reinject_ring, REO_REINJECT, 0);

	/* Rx release ring */
	dp_srng_cleanup(soc, &soc->rx_rel_ring, WBM2SW_RELEASE, 0);

	/* Rx exception ring */
	/* TODO: Better to store ring_type and ring_num in
	 * dp_srng during setup
	 */
	dp_srng_cleanup(soc, &soc->reo_exception_ring, REO_EXCEPTION, 0);

	/* REO command and status rings */
	dp_srng_cleanup(soc, &soc->reo_cmd_ring, REO_CMD, 0);
	dp_srng_cleanup(soc, &soc->reo_status_ring, REO_STATUS, 0);

	qdf_spinlock_destroy(&soc->rx.reo_cmd_lock);

	qdf_spinlock_destroy(&soc->peer_ref_mutex);
	htt_soc_detach(soc->htt_handle);

	dp_reo_desc_freelist_destroy(soc);
	wlan_cfg_soc_detach(soc->wlan_cfg_ctx);
	qdf_mem_free(soc);
}

/*
* dp_rxdma_ring_config() - configure the RX DMA rings
*
* This function is used to configure the MAC rings.
* On MCL host provides buffers in Host2FW ring
* FW refills (copies) buffers to the ring and updates
* ring_idx in register
*
* @soc: data path SoC handle
* @pdev: Physical device handle
*
* Return: void
*/
#ifdef QCA_HOST2FW_RXBUF_RING
static void dp_rxdma_ring_config(struct dp_soc *soc)
{
	int i;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (pdev) {
			int mac_id = 0;
			int j;
			bool dbs_enable = 0;
			int max_mac_rings =
				 wlan_cfg_get_num_mac_rings
				(pdev->wlan_cfg_ctx);

			htt_srng_setup(soc->htt_handle, 0,
				 pdev->rx_refill_buf_ring.hal_srng,
				 RXDMA_BUF);

			if (soc->cdp_soc.ol_ops->
				is_hw_dbs_2x2_capable) {
				dbs_enable = soc->cdp_soc.ol_ops->
					is_hw_dbs_2x2_capable(soc->psoc);
			}

			if (dbs_enable) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
				QDF_TRACE_LEVEL_ERROR,
				FL("DBS enabled max_mac_rings %d\n"),
					 max_mac_rings);
			} else {
				max_mac_rings = 1;
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					 QDF_TRACE_LEVEL_ERROR,
					 FL("DBS disabled, max_mac_rings %d\n"),
					 max_mac_rings);
			}

			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
					 FL("pdev_id %d max_mac_rings %d\n"),
					 pdev->pdev_id, max_mac_rings);

			for (j = 0; j < max_mac_rings; j++) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					 QDF_TRACE_LEVEL_ERROR,
					 FL("mac_id %d\n"), mac_id);
				htt_srng_setup(soc->htt_handle, mac_id,
					 pdev->rx_mac_buf_ring[j]
						.hal_srng,
					 RXDMA_BUF);
				mac_id++;
			}
		}
	}
}
#else
static void dp_rxdma_ring_config(struct dp_soc *soc)
{
	int i;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];

		if (pdev) {
			htt_srng_setup(soc->htt_handle, i,
				pdev->rx_refill_buf_ring.hal_srng, RXDMA_BUF);

			htt_srng_setup(soc->htt_handle, i,
					pdev->rxdma_mon_buf_ring.hal_srng,
					RXDMA_MONITOR_BUF);
			htt_srng_setup(soc->htt_handle, i,
					pdev->rxdma_mon_dst_ring.hal_srng,
					RXDMA_MONITOR_DST);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_status_ring.hal_srng,
				RXDMA_MONITOR_STATUS);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_desc_ring.hal_srng,
				RXDMA_MONITOR_DESC);
		}
	}
}
#endif

/*
 * dp_soc_attach_target_wifi3() - SOC initialization in the target
 * @txrx_soc: Datapath SOC handle
 */
static int dp_soc_attach_target_wifi3(struct cdp_soc_t *cdp_soc)
{
	struct dp_soc *soc = (struct dp_soc *)cdp_soc;

	htt_soc_attach_target(soc->htt_handle);

	dp_rxdma_ring_config(soc);

	DP_STATS_INIT(soc);
	return 0;
}

/*
 * dp_soc_get_nss_cfg_wifi3() - SOC get nss config
 * @txrx_soc: Datapath SOC handle
 */
static int dp_soc_get_nss_cfg_wifi3(struct cdp_soc_t *cdp_soc)
{
	struct dp_soc *dsoc = (struct dp_soc *)cdp_soc;
	return wlan_cfg_get_dp_soc_nss_cfg(dsoc->wlan_cfg_ctx);
}
/*
 * dp_soc_set_nss_cfg_wifi3() - SOC set nss config
 * @txrx_soc: Datapath SOC handle
 * @nss_cfg: nss config
 */
static void dp_soc_set_nss_cfg_wifi3(struct cdp_soc_t *cdp_soc, int config)
{
	struct dp_soc *dsoc = (struct dp_soc *)cdp_soc;
	wlan_cfg_set_dp_soc_nss_cfg(dsoc->wlan_cfg_ctx, config);
	if (config) {
		/*
		 * disable dp interrupt if nss enabled
		 */
		wlan_cfg_set_num_contexts(dsoc->wlan_cfg_ctx, 0);
	}
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("nss-wifi<0> nss config is enabled"));
}


/*
* dp_vdev_attach_wifi3() - attach txrx vdev
* @txrx_pdev: Datapath PDEV handle
* @vdev_mac_addr: MAC address of the virtual interface
* @vdev_id: VDEV Id
* @wlan_op_mode: VDEV operating mode
*
* Return: DP VDEV handle on success, NULL on failure
*/
static struct cdp_vdev *dp_vdev_attach_wifi3(struct cdp_pdev *txrx_pdev,
	uint8_t *vdev_mac_addr, uint8_t vdev_id, enum wlan_op_mode op_mode)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_vdev *vdev = qdf_mem_malloc(sizeof(*vdev));

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("DP VDEV memory allocation failed"));
		goto fail0;
	}

	vdev->pdev = pdev;
	vdev->vdev_id = vdev_id;
	vdev->opmode = op_mode;
	vdev->osdev = soc->osdev;

	vdev->osif_rx = NULL;
	vdev->osif_rsim_rx_decap = NULL;
	vdev->osif_rx_mon = NULL;
	vdev->osif_tx_free_ext = NULL;
	vdev->osif_vdev = NULL;

	vdev->delete.pending = 0;
	vdev->safemode = 0;
	vdev->drop_unenc = 1;
#ifdef notyet
	vdev->filters_num = 0;
#endif

	qdf_mem_copy(
		&vdev->mac_addr.raw[0], vdev_mac_addr, OL_TXRX_MAC_ADDR_LEN);

	vdev->tx_encap_type = wlan_cfg_pkt_type(soc->wlan_cfg_ctx);
	vdev->rx_decap_type = wlan_cfg_pkt_type(soc->wlan_cfg_ctx);
	vdev->dscp_tid_map_id = 0;
	vdev->mcast_enhancement_en = 0;

	/* TODO: Initialize default HTT meta data that will be used in
	 * TCL descriptors for packets transmitted from this VDEV
	 */

	TAILQ_INIT(&vdev->peer_list);

	/* add this vdev into the pdev's list */
	TAILQ_INSERT_TAIL(&pdev->vdev_list, vdev, vdev_list_elem);
	pdev->vdev_count++;

	dp_tx_vdev_attach(vdev);

#ifdef DP_INTR_POLL_BASED
	if (wlan_cfg_get_num_contexts(soc->wlan_cfg_ctx) != 0) {
		if (pdev->vdev_count == 1)
			qdf_timer_mod(&soc->int_timer, DP_INTR_POLL_TIMER_MS);
	}
#endif

	dp_lro_hash_setup(soc);

	/* LRO */
	if (wlan_cfg_is_lro_enabled(soc->wlan_cfg_ctx) &&
		wlan_op_mode_sta == vdev->opmode)
		vdev->lro_enable = true;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		 "LRO: vdev_id %d lro_enable %d", vdev_id, vdev->lro_enable);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Created vdev %p (%pM)", vdev, vdev->mac_addr.raw);
	DP_STATS_INIT(vdev);

	return (struct cdp_vdev *)vdev;

fail0:
	return NULL;
}

/**
 * dp_vdev_register_wifi3() - Register VDEV operations from osif layer
 * @vdev: Datapath VDEV handle
 * @osif_vdev: OSIF vdev handle
 * @txrx_ops: Tx and Rx operations
 *
 * Return: DP VDEV handle on success, NULL on failure
 */
static void dp_vdev_register_wifi3(struct cdp_vdev *vdev_handle,
	void *osif_vdev,
	struct ol_txrx_ops *txrx_ops)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->osif_vdev = osif_vdev;
	vdev->osif_rx = txrx_ops->rx.rx;
	vdev->osif_rsim_rx_decap = txrx_ops->rx.rsim_rx_decap;
	vdev->osif_rx_mon = txrx_ops->rx.mon;
	vdev->osif_tx_free_ext = txrx_ops->tx.tx_free_ext;
#ifdef notyet
#if ATH_SUPPORT_WAPI
	vdev->osif_check_wai = txrx_ops->rx.wai_check;
#endif
#endif
#ifdef UMAC_SUPPORT_PROXY_ARP
	vdev->osif_proxy_arp = txrx_ops->proxy_arp;
#endif
	vdev->me_convert = txrx_ops->me_convert;

	/* TODO: Enable the following once Tx code is integrated */
	txrx_ops->tx.tx = dp_tx_send;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		"DP Vdev Register success");
}

/*
 * dp_vdev_detach_wifi3() - Detach txrx vdev
 * @txrx_vdev:		Datapath VDEV handle
 * @callback:		Callback OL_IF on completion of detach
 * @cb_context:	Callback context
 *
 */
static void dp_vdev_detach_wifi3(struct cdp_vdev *vdev_handle,
	ol_txrx_vdev_delete_cb callback, void *cb_context)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;

	/* preconditions */
	qdf_assert(vdev);

	/* remove the vdev from its parent pdev's list */
	TAILQ_REMOVE(&pdev->vdev_list, vdev, vdev_list_elem);

	/*
	 * Use peer_ref_mutex while accessing peer_list, in case
	 * a peer is in the process of being removed from the list.
	 */
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	/* check that the vdev has no peers allocated */
	if (!TAILQ_EMPTY(&vdev->peer_list)) {
		/* debug print - will be removed later */
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
			FL("not deleting vdev object %p (%pM)"
			"until deletion finishes for all its peers"),
			vdev, vdev->mac_addr.raw);
		/* indicate that the vdev needs to be deleted */
		vdev->delete.pending = 1;
		vdev->delete.callback = callback;
		vdev->delete.context = cb_context;
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
		return;
	}
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	dp_tx_vdev_detach(vdev);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("deleting vdev object %p (%pM)"), vdev, vdev->mac_addr.raw);

	qdf_mem_free(vdev);

	if (callback)
		callback(cb_context);
}

/*
 * dp_peer_create_wifi3() - attach txrx peer
 * @txrx_vdev: Datapath VDEV handle
 * @peer_mac_addr: Peer MAC address
 *
 * Return: DP peeer handle on success, NULL on failure
 */
static void *dp_peer_create_wifi3(struct cdp_vdev *vdev_handle,
		uint8_t *peer_mac_addr)
{
	struct dp_peer *peer;
	int i;
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;
	struct dp_soc *soc;

	/* preconditions */
	qdf_assert(vdev);
	qdf_assert(peer_mac_addr);

	pdev = vdev->pdev;
	soc = pdev->soc;
#ifdef notyet
	peer = (struct dp_peer *)qdf_mempool_alloc(soc->osdev,
		soc->mempool_ol_ath_peer);
#else
	peer = (struct dp_peer *)qdf_mem_malloc(sizeof(*peer));
#endif

	if (!peer)
		return NULL; /* failure */

	qdf_mem_zero(peer, sizeof(struct dp_peer));

	TAILQ_INIT(&peer->ast_entry_list);
	qdf_mem_copy(&peer->self_ast_entry.mac_addr, peer_mac_addr,
			DP_MAC_ADDR_LEN);
	peer->self_ast_entry.peer = peer;
	TAILQ_INSERT_TAIL(&peer->ast_entry_list, &peer->self_ast_entry,
				ast_entry_elem);

	qdf_spinlock_create(&peer->peer_info_lock);

	/* store provided params */
	peer->vdev = vdev;
	qdf_mem_copy(
		&peer->mac_addr.raw[0], peer_mac_addr, OL_TXRX_MAC_ADDR_LEN);

	/* TODO: See of rx_opt_proc is really required */
	peer->rx_opt_proc = soc->rx_opt_proc;

	/* initialize the peer_id */
	for (i = 0; i < MAX_NUM_PEER_ID_PER_PEER; i++)
		peer->peer_ids[i] = HTT_INVALID_PEER;

	qdf_spin_lock_bh(&soc->peer_ref_mutex);

	qdf_atomic_init(&peer->ref_cnt);

	/* keep one reference for attach */
	qdf_atomic_inc(&peer->ref_cnt);

	/* add this peer into the vdev's list */
	TAILQ_INSERT_TAIL(&vdev->peer_list, peer, peer_list_elem);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	/* TODO: See if hash based search is required */
	dp_peer_find_hash_add(soc, peer);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		"vdev %p created peer %p (%pM) ref_cnt: %d",
		vdev, peer, peer->mac_addr.raw,
		qdf_atomic_read(&peer->ref_cnt));
	/*
	 * For every peer MAp message search and set if bss_peer
	 */
	if (memcmp(peer->mac_addr.raw, vdev->mac_addr.raw, 6) == 0) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			"vdev bss_peer!!!!");
		peer->bss_peer = 1;
		vdev->vap_bss_peer = peer;
	}

#ifndef CONFIG_WIN
	dp_local_peer_id_alloc(pdev, peer);
#endif
	DP_STATS_INIT(peer);
	return (void *)peer;
}

/*
 * dp_peer_setup_wifi3() - initialize the peer
 * @vdev_hdl: virtual device object
 * @peer: Peer object
 *
 * Return: void
 */
static void dp_peer_setup_wifi3(struct cdp_vdev *vdev_hdl, void *peer_hdl)
{
	struct dp_peer *peer = (struct dp_peer *)peer_hdl;
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;
	struct dp_pdev *pdev;
	struct dp_soc *soc;
	bool hash_based = 0;
	enum cdp_host_reo_dest_ring reo_dest;

	/* preconditions */
	qdf_assert(vdev);
	qdf_assert(peer);

	pdev = vdev->pdev;
	soc = pdev->soc;

	dp_peer_rx_init(pdev, peer);

	peer->last_assoc_rcvd = 0;
	peer->last_disassoc_rcvd = 0;
	peer->last_deauth_rcvd = 0;

	hash_based = wlan_cfg_is_rx_hash_enabled(soc->wlan_cfg_ctx);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		FL("hash based steering %d\n"), hash_based);

	if (!hash_based)
		reo_dest = pdev->reo_dest;
	else
		reo_dest = 1;

	if (soc->cdp_soc.ol_ops->peer_set_default_routing) {
		/* TODO: Check the destination ring number to be passed to FW */
		soc->cdp_soc.ol_ops->peer_set_default_routing(
			pdev->osif_pdev, peer->mac_addr.raw,
			 peer->vdev->vdev_id, hash_based, reo_dest);
	}
	return;
}

/*
 * dp_set_vdev_tx_encap_type() - set the encap type of the vdev
 * @vdev_handle: virtual device object
 * @htt_pkt_type: type of pkt
 *
 * Return: void
 */
static void dp_set_vdev_tx_encap_type(struct cdp_vdev *vdev_handle,
	 enum htt_cmn_pkt_type val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->tx_encap_type = val;
}

/*
 * dp_set_vdev_rx_decap_type() - set the decap type of the vdev
 * @vdev_handle: virtual device object
 * @htt_pkt_type: type of pkt
 *
 * Return: void
 */
static void dp_set_vdev_rx_decap_type(struct cdp_vdev *vdev_handle,
	 enum htt_cmn_pkt_type val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->rx_decap_type = val;
}

/*
 * dp_set_pdev_reo_dest() - set the reo destination ring for this pdev
 * @pdev_handle: physical device object
 * @val: reo destination ring index (1 - 4)
 *
 * Return: void
 */
static void dp_set_pdev_reo_dest(struct cdp_pdev *pdev_handle,
	 enum cdp_host_reo_dest_ring val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (pdev)
		pdev->reo_dest = val;
}

/*
 * dp_get_pdev_reo_dest() - get the reo destination for this pdev
 * @pdev_handle: physical device object
 *
 * Return: reo destination ring index
 */
static enum cdp_host_reo_dest_ring
dp_get_pdev_reo_dest(struct cdp_pdev *pdev_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	if (pdev)
		return pdev->reo_dest;
	else
		return cdp_host_reo_dest_ring_unknown;
}

#ifdef QCA_SUPPORT_SON
static void dp_son_peer_authorize(struct dp_peer *peer)
{
	struct dp_soc *soc;
	soc = peer->vdev->pdev->soc;
	peer->peer_bs_inact_flag = 0;
	peer->peer_bs_inact = soc->pdev_bs_inact_reload;
	return;
}
#else
static void dp_son_peer_authorize(struct dp_peer *peer)
{
	return;
}
#endif
/*
 * dp_set_filter_neighbour_peers() - set filter neighbour peers for smart mesh
 * @pdev_handle: device object
 * @val: value to be set
 *
 * Return: void
 */
static int dp_set_filter_neighbour_peers(struct cdp_pdev *pdev_handle,
	 uint32_t val)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;

	/* Enable/Disable smart mesh filtering. This flag will be checked
	 * during rx processing to check if packets are from NAC clients.
	 */
	pdev->filter_neighbour_peers = val;
	return 0;
}

/*
 * dp_update_filter_neighbour_peers() - set neighbour peers(nac clients)
 * address for smart mesh filtering
 * @pdev_handle: device object
 * @cmd: Add/Del command
 * @macaddr: nac client mac address
 *
 * Return: void
 */
static int dp_update_filter_neighbour_peers(struct cdp_pdev *pdev_handle,
	 uint32_t cmd, uint8_t *macaddr)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	struct dp_neighbour_peer *peer = NULL;

	if (!macaddr)
		goto fail0;

	/* Store address of NAC (neighbour peer) which will be checked
	 * against TA of received packets.
	 */
	if (cmd == DP_NAC_PARAM_ADD) {
		peer = (struct dp_neighbour_peer *) qdf_mem_malloc(
				sizeof(*peer));

		if (!peer) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("DP neighbour peer node memory allocation failed"));
			goto fail0;
		}

		qdf_mem_copy(&peer->neighbour_peers_macaddr.raw[0],
			macaddr, DP_MAC_ADDR_LEN);


		qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
		/* add this neighbour peer into the list */
		TAILQ_INSERT_TAIL(&pdev->neighbour_peers_list, peer,
				neighbour_peer_list_elem);
		qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);

		return 1;

	} else if (cmd == DP_NAC_PARAM_DEL) {
		qdf_spin_lock_bh(&pdev->neighbour_peer_mutex);
		TAILQ_FOREACH(peer, &pdev->neighbour_peers_list,
				neighbour_peer_list_elem) {
			if (!qdf_mem_cmp(&peer->neighbour_peers_macaddr.raw[0],
				macaddr, DP_MAC_ADDR_LEN)) {
				/* delete this peer from the list */
				TAILQ_REMOVE(&pdev->neighbour_peers_list,
					peer, neighbour_peer_list_elem);
				qdf_mem_free(peer);
				break;
			}
		}
		qdf_spin_unlock_bh(&pdev->neighbour_peer_mutex);

		return 1;

	}

fail0:
	return 0;
}

/*
 * dp_peer_authorize() - authorize txrx peer
 * @peer_handle:		Datapath peer handle
 * @authorize
 *
 */
static void dp_peer_authorize(void *peer_handle, uint32_t authorize)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_soc *soc;

	if (peer != NULL) {
		soc = peer->vdev->pdev->soc;
		qdf_spin_lock_bh(&soc->peer_ref_mutex);
		dp_son_peer_authorize(peer);
		peer->authorize = authorize ? 1 : 0;
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
	}
}

/*
 * dp_peer_unref_delete() - unref and delete peer
 * @peer_handle:		Datapath peer handle
 *
 */
void dp_peer_unref_delete(void *peer_handle)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_vdev *vdev = peer->vdev;
	struct dp_pdev *pdev = vdev->pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_peer *tmppeer;
	int found = 0;
	uint16_t peer_id;
	uint16_t hw_peer_id;
	struct dp_ast_entry *ast_entry;

	/*
	 * Hold the lock all the way from checking if the peer ref count
	 * is zero until the peer references are removed from the hash
	 * table and vdev list (if the peer ref count is zero).
	 * This protects against a new HL tx operation starting to use the
	 * peer object just after this function concludes it's done being used.
	 * Furthermore, the lock needs to be held while checking whether the
	 * vdev's list of peers is empty, to make sure that list is not modified
	 * concurrently with the empty check.
	 */
	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		  "%s: peer %p ref_cnt(before decrement): %d\n", __func__,
		  peer, qdf_atomic_read(&peer->ref_cnt));
	if (qdf_atomic_dec_and_test(&peer->ref_cnt)) {
		peer_id = peer->peer_ids[0];

		/*
		 * Make sure that the reference to the peer in
		 * peer object map is removed
		 */
		if (peer_id != HTT_INVALID_PEER)
			soc->peer_id_to_obj_map[peer_id] = NULL;

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			"Deleting peer %p (%pM)", peer, peer->mac_addr.raw);

		/* remove the reference to the peer from the hash table */
		dp_peer_find_hash_remove(soc, peer);

		TAILQ_FOREACH(tmppeer, &peer->vdev->peer_list, peer_list_elem) {
			if (tmppeer == peer) {
				found = 1;
				break;
			}
		}
		if (found) {
			TAILQ_REMOVE(&peer->vdev->peer_list, peer,
				peer_list_elem);
		} else {
			/*Ignoring the remove operation as peer not found*/
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
				"peer %p not found in vdev (%p)->peer_list:%p",
				peer, vdev, &peer->vdev->peer_list);
		}

		/* cleanup the peer data */
		dp_peer_cleanup(vdev, peer);

		/* check whether the parent vdev has no peers left */
		if (TAILQ_EMPTY(&vdev->peer_list)) {
			/*
			 * Now that there are no references to the peer, we can
			 * release the peer reference lock.
			 */
			qdf_spin_unlock_bh(&soc->peer_ref_mutex);
			/*
			 * Check if the parent vdev was waiting for its peers
			 * to be deleted, in order for it to be deleted too.
			 */
			if (vdev->delete.pending) {
				ol_txrx_vdev_delete_cb vdev_delete_cb =
					vdev->delete.callback;
				void *vdev_delete_context =
					vdev->delete.context;

				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_INFO_HIGH,
					FL("deleting vdev object %p (%pM)"
					" - its last peer is done"),
					vdev, vdev->mac_addr.raw);
				/* all peers are gone, go ahead and delete it */
				qdf_mem_free(vdev);
				if (vdev_delete_cb)
					vdev_delete_cb(vdev_delete_context);
			}
		} else {
			qdf_spin_unlock_bh(&soc->peer_ref_mutex);
		}
#ifdef notyet
		qdf_mempool_free(soc->osdev, soc->mempool_ol_ath_peer, peer);
#else
		TAILQ_FOREACH(ast_entry, &peer->ast_entry_list,
				ast_entry_elem) {
			hw_peer_id = ast_entry->ast_idx;
			if (peer->self_ast_entry.ast_idx != hw_peer_id)
				qdf_mem_free(ast_entry);
			else
				peer->self_ast_entry.ast_idx =
							HTT_INVALID_PEER;

			soc->ast_table[hw_peer_id] = NULL;
		}
		qdf_mem_free(peer);
#endif
		if (soc->cdp_soc.ol_ops->peer_unref_delete) {
			soc->cdp_soc.ol_ops->peer_unref_delete(pdev->osif_pdev,
					vdev->vdev_id, peer->mac_addr.raw);
		}

	} else {
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
	}
}

/*
 * dp_peer_detach_wifi3() – Detach txrx peer
 * @peer_handle:		Datapath peer handle
 *
 */
static void dp_peer_delete_wifi3(void *peer_handle)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	/* redirect the peer's rx delivery function to point to a
	 * discard func
	 */
	peer->rx_opt_proc = dp_rx_discard;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		FL("peer %p (%pM)"),  peer, peer->mac_addr.raw);

#ifndef CONFIG_WIN
	dp_local_peer_id_free(peer->vdev->pdev, peer);
#endif
	qdf_spinlock_destroy(&peer->peer_info_lock);

	/*
	 * Remove the reference added during peer_attach.
	 * The peer will still be left allocated until the
	 * PEER_UNMAP message arrives to remove the other
	 * reference, added by the PEER_MAP message.
	 */
	dp_peer_unref_delete(peer_handle);
}

/*
 * dp_get_vdev_mac_addr_wifi3() – Detach txrx peer
 * @peer_handle:		Datapath peer handle
 *
 */
static uint8 *dp_get_vdev_mac_addr_wifi3(struct cdp_vdev *pvdev)
{
	struct dp_vdev *vdev = (struct dp_vdev *)pvdev;
	return vdev->mac_addr.raw;
}

/*
 * dp_vdev_set_wds() - Enable per packet stats
 * @vdev_handle: DP VDEV handle
 * @val: value
 *
 * Return: none
 */
static int dp_vdev_set_wds(void *vdev_handle, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	vdev->wds_enabled = val;
	return 0;
}

/*
 * dp_get_vdev_from_vdev_id_wifi3() – Detach txrx peer
 * @peer_handle:		Datapath peer handle
 *
 */
static struct cdp_vdev *dp_get_vdev_from_vdev_id_wifi3(struct cdp_pdev *dev,
						uint8_t vdev_id)
{
	struct dp_pdev *pdev = (struct dp_pdev *)dev;
	struct dp_vdev *vdev = NULL;

	if (qdf_unlikely(!pdev))
		return NULL;

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (vdev->vdev_id == vdev_id)
			break;
	}

	return (struct cdp_vdev *)vdev;
}

static int dp_get_opmode(struct cdp_vdev *vdev_handle)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	return vdev->opmode;
}

static struct cdp_cfg *dp_get_ctrl_pdev_from_vdev_wifi3(struct cdp_vdev *pvdev)
{
	struct dp_vdev *vdev = (struct dp_vdev *)pvdev;
	struct dp_pdev *pdev = vdev->pdev;

	return (struct cdp_cfg *)pdev->wlan_cfg_ctx;
}
/**
 * dp_vdev_set_monitor_mode() - Set DP VDEV to monitor mode
 * @vdev_handle: Datapath VDEV handle
 * @smart_monitor: Flag to denote if its smart monitor mode
 *
 * Return: 0 on success, not 0 on failure
 */
static int dp_vdev_set_monitor_mode(struct cdp_vdev *vdev_handle,
		uint8_t smart_monitor)
{
	/* Many monitor VAPs can exists in a system but only one can be up at
	 * anytime
	 */
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev;
	struct htt_rx_ring_tlv_filter htt_tlv_filter;
	struct dp_soc *soc;
	uint8_t pdev_id;

	qdf_assert(vdev);

	pdev = vdev->pdev;
	pdev_id = pdev->pdev_id;
	soc = pdev->soc;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
		"pdev=%p, pdev_id=%d, soc=%p vdev=%p\n",
		pdev, pdev_id, soc, vdev);

	/*Check if current pdev's monitor_vdev exists */
	if (pdev->monitor_vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"vdev=%p\n", vdev);
		qdf_assert(vdev);
	}

	pdev->monitor_vdev = vdev;

	/* If smart monitor mode, do not configure monitor ring */
	if (smart_monitor)
		return QDF_STATUS_SUCCESS;

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.packet = 1;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 0;
	htt_tlv_filter.ppdu_end = 0;
	htt_tlv_filter.ppdu_end_user_stats = 0;
	htt_tlv_filter.ppdu_end_user_stats_ext = 0;
	htt_tlv_filter.ppdu_end_status_done = 0;
	htt_tlv_filter.enable_fp = 1;
	htt_tlv_filter.enable_md = 0;
	htt_tlv_filter.enable_mo = 1;

	htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
		pdev->rxdma_mon_dst_ring.hal_srng,
		RXDMA_MONITOR_BUF,  RX_BUFFER_SIZE, &htt_tlv_filter);

	htt_tlv_filter.mpdu_start = 1;
	htt_tlv_filter.msdu_start = 1;
	htt_tlv_filter.packet = 0;
	htt_tlv_filter.msdu_end = 1;
	htt_tlv_filter.mpdu_end = 1;
	htt_tlv_filter.packet_header = 1;
	htt_tlv_filter.attention = 1;
	htt_tlv_filter.ppdu_start = 1;
	htt_tlv_filter.ppdu_end = 1;
	htt_tlv_filter.ppdu_end_user_stats = 1;
	htt_tlv_filter.ppdu_end_user_stats_ext = 1;
	htt_tlv_filter.ppdu_end_status_done = 1;
	htt_tlv_filter.enable_fp = 1;
	htt_tlv_filter.enable_md = 1;
	htt_tlv_filter.enable_mo = 1;
	/*
	 * htt_h2t_rx_ring_cfg(soc->htt_handle, pdev_id,
	 * pdev->rxdma_mon_status_ring.hal_srng,
	 * RXDMA_MONITOR_STATUS, RX_BUFFER_SIZE, &htt_tlv_filter);
	 */

	return QDF_STATUS_SUCCESS;
}

#ifdef MESH_MODE_SUPPORT
void dp_peer_set_mesh_mode(struct cdp_vdev *vdev_hdl, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		FL("val %d"), val);
	vdev->mesh_vdev = val;
}

/*
 * dp_peer_set_mesh_rx_filter() - to set the mesh rx filter
 * @vdev_hdl: virtual device object
 * @val: value to be set
 *
 * Return: void
 */
void dp_peer_set_mesh_rx_filter(struct cdp_vdev *vdev_hdl, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_hdl;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		FL("val %d"), val);
	vdev->mesh_rx_filter = val;
}
#endif

/**
 * dp_aggregate_vdev_stats(): Consolidate stats at VDEV level
 * @vdev: DP VDEV handle
 *
 * return: void
 */
void dp_aggregate_vdev_stats(struct dp_vdev *vdev)
{
	struct dp_peer *peer = NULL;
	int i;

	qdf_mem_set(&(vdev->stats.tx), sizeof(vdev->stats.tx), 0x0);
	qdf_mem_set(&(vdev->stats.rx), sizeof(vdev->stats.rx), 0x0);

	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (!peer)
			return;

		for (i = 0; i <= MAX_MCS; i++) {
			DP_STATS_AGGR(vdev, peer, tx.pkt_type[0].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, tx.pkt_type[1].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, tx.pkt_type[2].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, tx.pkt_type[3].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, tx.pkt_type[4].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, rx.pkt_type[0].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, rx.pkt_type[1].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, rx.pkt_type[2].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, rx.pkt_type[3].mcs_count[i]);
			DP_STATS_AGGR(vdev, peer, rx.pkt_type[4].mcs_count[i]);
		}

		for (i = 0; i < SUPPORTED_BW; i++) {
			DP_STATS_AGGR(vdev, peer, tx.bw[i]);
			DP_STATS_AGGR(vdev, peer, rx.bw[i]);
		}

		for (i = 0; i < SS_COUNT; i++)
			DP_STATS_AGGR(vdev, peer, rx.nss[i]);

		for (i = 0; i < WME_AC_MAX; i++) {
			DP_STATS_AGGR(vdev, peer, tx.wme_ac_type[i]);
			DP_STATS_AGGR(vdev, peer, rx.wme_ac_type[i]);
			DP_STATS_AGGR(vdev, peer, tx.excess_retries_ac[i]);

		}

		for (i = 0; i < MAX_MCS + 1; i++) {
			DP_STATS_AGGR(vdev, peer, tx.sgi_count[i]);
			DP_STATS_AGGR(vdev, peer, rx.sgi_count[i]);
		}

		DP_STATS_AGGR_PKT(vdev, peer, tx.comp_pkt);
		DP_STATS_AGGR_PKT(vdev, peer, tx.ucast);
		DP_STATS_AGGR_PKT(vdev, peer, tx.mcast);
		DP_STATS_AGGR_PKT(vdev, peer, tx.tx_success);
		DP_STATS_AGGR(vdev, peer, tx.tx_failed);
		DP_STATS_AGGR(vdev, peer, tx.ofdma);
		DP_STATS_AGGR(vdev, peer, tx.stbc);
		DP_STATS_AGGR(vdev, peer, tx.ldpc);
		DP_STATS_AGGR(vdev, peer, tx.retries);
		DP_STATS_AGGR(vdev, peer, tx.non_amsdu_cnt);
		DP_STATS_AGGR(vdev, peer, tx.amsdu_cnt);
		DP_STATS_AGGR(vdev, peer, tx.dropped.fw_discard);
		DP_STATS_AGGR(vdev, peer, tx.dropped.fw_discard_retired);
		DP_STATS_AGGR(vdev, peer, tx.dropped.mpdu_age_out);
		DP_STATS_AGGR(vdev, peer, tx.dropped.fw_discard_reason1);
		DP_STATS_AGGR(vdev, peer, tx.dropped.fw_discard_reason2);
		DP_STATS_AGGR(vdev, peer, tx.dropped.fw_discard_reason3);

		DP_STATS_AGGR(vdev, peer, rx.err.mic_err);
		DP_STATS_AGGR(vdev, peer, rx.err.decrypt_err);
		DP_STATS_AGGR(vdev, peer, rx.non_ampdu_cnt);
		DP_STATS_AGGR(vdev, peer, rx.ampdu_cnt);
		DP_STATS_AGGR(vdev, peer, rx.non_amsdu_cnt);
		DP_STATS_AGGR(vdev, peer, rx.amsdu_cnt);
		DP_STATS_AGGR_PKT(vdev, peer, rx.to_stack);

		for (i = 0; i <  CDP_MAX_RX_RINGS; i++)
			DP_STATS_AGGR_PKT(vdev, peer, rx.rcvd_reo[i]);

		peer->stats.rx.unicast.num = peer->stats.rx.to_stack.num -
			peer->stats.rx.multicast.num;
		peer->stats.rx.unicast.bytes = peer->stats.rx.to_stack.bytes -
			peer->stats.rx.multicast.bytes;
		DP_STATS_AGGR_PKT(vdev, peer, rx.unicast);
		DP_STATS_AGGR_PKT(vdev, peer, rx.multicast);
		DP_STATS_AGGR_PKT(vdev, peer, rx.wds);
		DP_STATS_AGGR_PKT(vdev, peer, rx.raw);
		DP_STATS_AGGR_PKT(vdev, peer, rx.intra_bss.pkts);
		DP_STATS_AGGR_PKT(vdev, peer, rx.intra_bss.fail);

		vdev->stats.tx.last_ack_rssi =
			peer->stats.tx.last_ack_rssi;
	}
}

/**
 * dp_aggregate_pdev_stats(): Consolidate stats at PDEV level
 * @pdev: DP PDEV handle
 *
 * return: void
 */
static inline void dp_aggregate_pdev_stats(struct dp_pdev *pdev)
{
	struct dp_vdev *vdev = NULL;
	uint8_t i;

	qdf_mem_set(&(pdev->stats.tx), sizeof(pdev->stats.tx), 0x0);
	qdf_mem_set(&(pdev->stats.rx), sizeof(pdev->stats.rx), 0x0);
	qdf_mem_set(&(pdev->stats.tx_i), sizeof(pdev->stats.tx_i), 0x0);

	TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
		if (!vdev)
			return;
		dp_aggregate_vdev_stats(vdev);

		for (i = 0; i <= MAX_MCS; i++) {
			DP_STATS_AGGR(pdev, vdev, tx.pkt_type[0].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, tx.pkt_type[1].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, tx.pkt_type[2].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, tx.pkt_type[3].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, tx.pkt_type[4].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, rx.pkt_type[0].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, rx.pkt_type[1].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, rx.pkt_type[2].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, rx.pkt_type[3].mcs_count[i]);
			DP_STATS_AGGR(pdev, vdev, rx.pkt_type[4].mcs_count[i]);
		}

		for (i = 0; i < SUPPORTED_BW; i++) {
			DP_STATS_AGGR(pdev, vdev, tx.bw[i]);
			DP_STATS_AGGR(pdev, vdev, rx.bw[i]);
		}

		for (i = 0; i < SS_COUNT; i++)
			DP_STATS_AGGR(pdev, vdev, rx.nss[i]);

			for (i = 0; i < WME_AC_MAX; i++) {
				DP_STATS_AGGR(pdev, vdev, tx.wme_ac_type[i]);
				DP_STATS_AGGR(pdev, vdev, rx.wme_ac_type[i]);
				DP_STATS_AGGR(pdev, vdev,
						tx.excess_retries_ac[i]);

			}

			for (i = 0; i < MAX_MCS + 1; i++) {
				DP_STATS_AGGR(pdev, vdev, tx.sgi_count[i]);
				DP_STATS_AGGR(pdev, vdev, rx.sgi_count[i]);
			}

			DP_STATS_AGGR_PKT(pdev, vdev, tx.comp_pkt);
			DP_STATS_AGGR_PKT(pdev, vdev, tx.ucast);
			DP_STATS_AGGR_PKT(pdev, vdev, tx.mcast);
			DP_STATS_AGGR_PKT(pdev, vdev, tx.tx_success);
			DP_STATS_AGGR(pdev, vdev, tx.tx_failed);
			DP_STATS_AGGR(pdev, vdev, tx.ofdma);
			DP_STATS_AGGR(pdev, vdev, tx.stbc);
			DP_STATS_AGGR(pdev, vdev, tx.ldpc);
			DP_STATS_AGGR(pdev, vdev, tx.retries);
			DP_STATS_AGGR(pdev, vdev, tx.non_amsdu_cnt);
			DP_STATS_AGGR(pdev, vdev, tx.amsdu_cnt);
			DP_STATS_AGGR(pdev, vdev, tx.dropped.fw_discard);
			DP_STATS_AGGR(pdev, vdev,
					tx.dropped.fw_discard_retired);
			DP_STATS_AGGR(pdev, vdev, tx.dropped.mpdu_age_out);
			DP_STATS_AGGR(pdev, vdev,
					tx.dropped.fw_discard_reason1);
			DP_STATS_AGGR(pdev, vdev,
					tx.dropped.fw_discard_reason2);
			DP_STATS_AGGR(pdev, vdev,
					tx.dropped.fw_discard_reason3);

			DP_STATS_AGGR(pdev, vdev, rx.err.mic_err);
			DP_STATS_AGGR(pdev, vdev, rx.err.decrypt_err);
			DP_STATS_AGGR(pdev, vdev, rx.non_ampdu_cnt);
			DP_STATS_AGGR(pdev, vdev, rx.ampdu_cnt);
			DP_STATS_AGGR(pdev, vdev, rx.non_amsdu_cnt);
			DP_STATS_AGGR(pdev, vdev, rx.amsdu_cnt);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.to_stack);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.rcvd_reo[0]);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.rcvd_reo[1]);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.rcvd_reo[2]);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.rcvd_reo[3]);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.unicast);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.multicast);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.wds);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.intra_bss.pkts);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.intra_bss.fail);
			DP_STATS_AGGR_PKT(pdev, vdev, rx.raw);

			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.rcvd);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.processed);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.reinject_pkts);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.inspect_pkts);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.raw.raw_pkt);
			DP_STATS_AGGR(pdev, vdev, tx_i.raw.dma_map_error);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.tso.tso_pkt);
			DP_STATS_AGGR(pdev, vdev, tx_i.tso.dropped_host);
			DP_STATS_AGGR(pdev, vdev, tx_i.tso.dropped_target);
			DP_STATS_AGGR(pdev, vdev, tx_i.sg.dropped_host);
			DP_STATS_AGGR(pdev, vdev, tx_i.sg.dropped_target);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.sg.sg_pkt);
			DP_STATS_AGGR_PKT(pdev, vdev, tx_i.mcast_en.mcast_pkt);
			DP_STATS_AGGR(pdev, vdev,
					tx_i.mcast_en.dropped_map_error);
			DP_STATS_AGGR(pdev, vdev,
					tx_i.mcast_en.dropped_self_mac);
			DP_STATS_AGGR(pdev, vdev,
					tx_i.mcast_en.dropped_send_fail);
			DP_STATS_AGGR(pdev, vdev, tx_i.mcast_en.ucast);
			DP_STATS_AGGR(pdev, vdev, tx_i.dropped.dma_error);
			DP_STATS_AGGR(pdev, vdev, tx_i.dropped.ring_full);
			DP_STATS_AGGR(pdev, vdev, tx_i.dropped.enqueue_fail);
			DP_STATS_AGGR(pdev, vdev, tx_i.dropped.desc_na);
			DP_STATS_AGGR(pdev, vdev, tx_i.dropped.res_full);

			pdev->stats.tx_i.dropped.dropped_pkt.num =
				pdev->stats.tx_i.dropped.dma_error +
				pdev->stats.tx_i.dropped.ring_full +
				pdev->stats.tx_i.dropped.enqueue_fail +
				pdev->stats.tx_i.dropped.desc_na +
				pdev->stats.tx_i.dropped.res_full;

			pdev->stats.tx.last_ack_rssi =
				vdev->stats.tx.last_ack_rssi;
			pdev->stats.tx_i.tso.num_seg =
				vdev->stats.tx_i.tso.num_seg;
	}
}

/**
 * dp_print_pdev_tx_stats(): Print Pdev level TX stats
 * @pdev: DP_PDEV Handle
 *
 * Return:void
 */
static inline void
dp_print_pdev_tx_stats(struct dp_pdev *pdev)
{
	DP_TRACE_STATS(FATAL, "WLAN Tx Stats:\n");
	DP_TRACE_STATS(FATAL, "Received From Stack:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.rcvd.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.tx_i.rcvd.bytes);
	DP_TRACE_STATS(FATAL, "Processed:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.processed.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.tx_i.processed.bytes);
	DP_TRACE_STATS(FATAL, "Completions:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx.comp_pkt.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.tx.comp_pkt.bytes);
	DP_TRACE_STATS(FATAL, "Dropped:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.dropped.dropped_pkt.num);
	DP_TRACE_STATS(FATAL, "Dma_map_error = %d",
			pdev->stats.tx_i.dropped.dma_error);
	DP_TRACE_STATS(FATAL, "Ring Full = %d",
			pdev->stats.tx_i.dropped.ring_full);
	DP_TRACE_STATS(FATAL, "Descriptor Not available = %d",
			pdev->stats.tx_i.dropped.desc_na);
	DP_TRACE_STATS(FATAL, "HW enqueue failed= %d",
			pdev->stats.tx_i.dropped.enqueue_fail);
	DP_TRACE_STATS(FATAL, "Resources Full = %d",
			pdev->stats.tx_i.dropped.res_full);
	DP_TRACE_STATS(FATAL, "Fw Discard = %d",
			pdev->stats.tx.dropped.fw_discard);
	DP_TRACE_STATS(FATAL, "Fw Discard Retired = %d",
			pdev->stats.tx.dropped.fw_discard_retired);
	DP_TRACE_STATS(FATAL, "Firmware Discard Untransmitted = %d",
			pdev->stats.tx.dropped.fw_discard_untransmitted);
	DP_TRACE_STATS(FATAL, "Mpdu Age Out = %d",
			pdev->stats.tx.dropped.mpdu_age_out);
	DP_TRACE_STATS(FATAL, "Firmware Discard Reason1 = %d",
			pdev->stats.tx.dropped.fw_discard_reason1);
	DP_TRACE_STATS(FATAL, "Firmware Discard Reason2 = %d",
			pdev->stats.tx.dropped.fw_discard_reason2);
	DP_TRACE_STATS(FATAL, "Firmware Discard Reason3 = %d\n",
			pdev->stats.tx.dropped.fw_discard_reason3);
	DP_TRACE_STATS(FATAL, "Scatter Gather:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.sg.sg_pkt.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d",
			pdev->stats.tx_i.sg.sg_pkt.bytes);
	DP_TRACE_STATS(FATAL, "Dropped By Host = %d",
			pdev->stats.tx_i.sg.dropped_host);
	DP_TRACE_STATS(FATAL, "Dropped By Target = %d\n",
			pdev->stats.tx_i.sg.dropped_target);
	DP_TRACE_STATS(FATAL, "Tso:\n");
	DP_TRACE_STATS(FATAL, "Number of Segments = %d",
			pdev->stats.tx_i.tso.num_seg);
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.tso.tso_pkt.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d",
			pdev->stats.tx_i.tso.tso_pkt.bytes);
	DP_TRACE_STATS(FATAL, "Dropped By Host = %d\n",
			pdev->stats.tx_i.tso.dropped_host);
	DP_TRACE_STATS(FATAL, "Mcast Enhancement:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.mcast_en.mcast_pkt.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d",
			pdev->stats.tx_i.mcast_en.mcast_pkt.bytes);
	DP_TRACE_STATS(FATAL, "Dropped: Map Errors = %d",
			pdev->stats.tx_i.mcast_en.dropped_map_error);
	DP_TRACE_STATS(FATAL, "Dropped: Self Mac = %d",
			pdev->stats.tx_i.mcast_en.dropped_self_mac);
	DP_TRACE_STATS(FATAL, "Dropped: Send Fail = %d",
			pdev->stats.tx_i.mcast_en.dropped_send_fail);
	DP_TRACE_STATS(FATAL, "Unicast sent = %d\n",
			pdev->stats.tx_i.mcast_en.ucast);
	DP_TRACE_STATS(FATAL, "Raw:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.raw.raw_pkt.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d",
			pdev->stats.tx_i.raw.raw_pkt.bytes);
	DP_TRACE_STATS(FATAL, "DMA map error = %d\n",
			pdev->stats.tx_i.raw.dma_map_error);
	DP_TRACE_STATS(FATAL, "Reinjected:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.reinject_pkts.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.tx_i.reinject_pkts.bytes);
	DP_TRACE_STATS(FATAL, "Inspected:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.tx_i.inspect_pkts.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.tx_i.inspect_pkts.bytes);
}

/**
 * dp_print_pdev_rx_stats(): Print Pdev level RX stats
 * @pdev: DP_PDEV Handle
 *
 * Return: void
 */
static inline void
dp_print_pdev_rx_stats(struct dp_pdev *pdev)
{
	DP_TRACE_STATS(FATAL, "WLAN Rx Stats:\n");
	DP_TRACE_STATS(FATAL, "Received From HW (Per Rx Ring):\n");
	DP_TRACE_STATS(FATAL, "Packets = %d %d %d %d",
			pdev->stats.rx.rcvd_reo[0].num,
			pdev->stats.rx.rcvd_reo[1].num,
			pdev->stats.rx.rcvd_reo[2].num,
			pdev->stats.rx.rcvd_reo[3].num);
	DP_TRACE_STATS(FATAL, "Bytes = %d %d %d %d\n",
			pdev->stats.rx.rcvd_reo[0].bytes,
			pdev->stats.rx.rcvd_reo[1].bytes,
			pdev->stats.rx.rcvd_reo[2].bytes,
			pdev->stats.rx.rcvd_reo[3].bytes);
	DP_TRACE_STATS(FATAL, "Replenished:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.replenish.pkts.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d",
			pdev->stats.replenish.pkts.bytes);
	DP_TRACE_STATS(FATAL, "Buffers Added To Freelist = %d\n",
			pdev->stats.buf_freelist);
	DP_TRACE_STATS(FATAL, "Dropped:\n");
	DP_TRACE_STATS(FATAL, "Total Packets With Msdu Not Done = %d\n",
			pdev->stats.dropped.msdu_not_done);
	DP_TRACE_STATS(FATAL, "Sent To Stack:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.rx.to_stack.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.rx.to_stack.bytes);
	DP_TRACE_STATS(FATAL, "Multicast/Broadcast:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			pdev->stats.rx.multicast.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d\n",
			pdev->stats.rx.multicast.bytes);
	DP_TRACE_STATS(FATAL, "Errors:\n");
	DP_TRACE_STATS(FATAL, "Rxdma Ring Un-inititalized = %d",
			pdev->stats.replenish.rxdma_err);
	DP_TRACE_STATS(FATAL, "Desc Alloc Failed: = %d",
			pdev->stats.err.desc_alloc_fail);
}

/**
 * dp_print_soc_tx_stats(): Print SOC level  stats
 * @soc DP_SOC Handle
 *
 * Return: void
 */
static inline void
dp_print_soc_tx_stats(struct dp_soc *soc)
{
	DP_TRACE_STATS(FATAL, "SOC Tx Stats:\n");
	DP_TRACE_STATS(FATAL, "Tx Descriptors In Use = %d",
			soc->stats.tx.desc_in_use);
	DP_TRACE_STATS(FATAL, "Invalid peer:\n");
	DP_TRACE_STATS(FATAL, "Packets = %d",
			soc->stats.tx.tx_invalid_peer.num);
	DP_TRACE_STATS(FATAL, "Bytes = %d",
			soc->stats.tx.tx_invalid_peer.bytes);
	DP_TRACE_STATS(FATAL, "Packets dropped due to TCL ring full = %d %d %d",
			soc->stats.tx.tcl_ring_full[0],
			soc->stats.tx.tcl_ring_full[1],
			soc->stats.tx.tcl_ring_full[2]);

}


/**
 * dp_print_soc_rx_stats: Print SOC level Rx stats
 * @soc: DP_SOC Handle
 *
 * Return:void
 */
static inline void
dp_print_soc_rx_stats(struct dp_soc *soc)
{
	uint32_t i;
	char reo_error[DP_REO_ERR_LENGTH];
	char rxdma_error[DP_RXDMA_ERR_LENGTH];
	uint8_t index = 0;

	DP_TRACE_STATS(FATAL, "SOC Rx Stats:\n");
	DP_TRACE_STATS(FATAL, "Errors:\n");
	DP_TRACE_STATS(FATAL, "Invalid RBM = %d",
			soc->stats.rx.err.invalid_rbm);
	DP_TRACE_STATS(FATAL, "Invalid Vdev = %d",
			soc->stats.rx.err.invalid_vdev);
	DP_TRACE_STATS(FATAL, "Invalid Pdev = %d",
			soc->stats.rx.err.invalid_pdev);
	DP_TRACE_STATS(FATAL, "Invalid Peer = %d",
			soc->stats.rx.err.rx_invalid_peer.num);
	DP_TRACE_STATS(FATAL, "HAL Ring Access Fail = %d",
			soc->stats.rx.err.hal_ring_access_fail);
	for (i = 0; i < MAX_RXDMA_ERRORS; i++) {
		index += qdf_snprint(&rxdma_error[index],
				DP_RXDMA_ERR_LENGTH - index,
				" %d", soc->stats.rx.err.rxdma_error[i]);
	}
	DP_TRACE_STATS(FATAL, "RXDMA Error (0-31):%s",
			rxdma_error);

	index = 0;
	for (i = 0; i < REO_ERROR_TYPE_MAX; i++) {
		index += qdf_snprint(&reo_error[index],
				DP_REO_ERR_LENGTH - index,
				" %d", soc->stats.rx.err.reo_error[i]);
	}
	DP_TRACE_STATS(FATAL, "REO Error(0-14):%s",
			reo_error);
}

/**
 * dp_txrx_host_stats_clr(): Reinitialize the txrx stats
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
static inline void
dp_txrx_host_stats_clr(struct dp_vdev *vdev)
{
	struct dp_peer *peer = NULL;
	DP_STATS_CLR(vdev->pdev);
	DP_STATS_CLR(vdev->pdev->soc);
	DP_STATS_CLR(vdev);
	TAILQ_FOREACH(peer, &vdev->peer_list, peer_list_elem) {
		if (!peer)
			return;
		DP_STATS_CLR(peer);
	}

}

/**
 * dp_print_rx_rates(): Print Rx rate stats
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
static inline void
dp_print_rx_rates(struct dp_vdev *vdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;
	uint8_t i, pkt_type;
	uint8_t index = 0;
	char rx_mcs[DOT11_MAX][DP_MCS_LENGTH];
	char nss[DP_NSS_LENGTH];

	DP_TRACE_STATS(FATAL, "Rx Rate Info:\n");

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (i = 0; i < MAX_MCS; i++) {
			index += qdf_snprint(&rx_mcs[pkt_type][index],
					DP_MCS_LENGTH - index,
					" %d ",
					pdev->stats.rx.pkt_type[pkt_type].
					mcs_count[i]);
		}
	}

	DP_TRACE_STATS(FATAL, "11A MCS(0-7) = %s",
			rx_mcs[0]);
	DP_TRACE_STATS(FATAL, "11A MCS Invalid = %d",
			pdev->stats.rx.pkt_type[DOT11_A].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11B MCS(0-6) = %s",
			rx_mcs[1]);
	DP_TRACE_STATS(FATAL, "11B MCS Invalid = %d",
			pdev->stats.rx.pkt_type[DOT11_B].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11N MCS(0-7) = %s",
			rx_mcs[2]);
	DP_TRACE_STATS(FATAL, "11N MCS Invalid = %d",
			pdev->stats.rx.pkt_type[DOT11_N].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "Type 11AC MCS(0-9) = %s",
			rx_mcs[3]);
	DP_TRACE_STATS(FATAL, "11AC MCS Invalid = %d",
			pdev->stats.rx.pkt_type[DOT11_AC].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11AX MCS(0-11) = %s",
			rx_mcs[4]);
	DP_TRACE_STATS(FATAL, "11AX MCS Invalid = %d",
			pdev->stats.rx.pkt_type[DOT11_AX].mcs_count[MAX_MCS]);
	index = 0;
	for (i = 0; i < SS_COUNT; i++) {
		index += qdf_snprint(&nss[index], DP_NSS_LENGTH - index,
				" %d", pdev->stats.rx.nss[i]);
	}
	DP_TRACE_STATS(FATAL, "NSS(0-7) = %s",
			nss);

	DP_TRACE_STATS(FATAL, "SGI ="
			" 0.8us %d,"
			" 0.4us %d,"
			" 1.6us %d,"
			" 3.2us %d,",
			pdev->stats.rx.sgi_count[0],
			pdev->stats.rx.sgi_count[1],
			pdev->stats.rx.sgi_count[2],
			pdev->stats.rx.sgi_count[3]);
	DP_TRACE_STATS(FATAL, "BW Counts = 20MHZ %d, 40MHZ %d, 80MHZ %d, 160MHZ %d",
			pdev->stats.rx.bw[0], pdev->stats.rx.bw[1],
			pdev->stats.rx.bw[2], pdev->stats.rx.bw[3]);
	DP_TRACE_STATS(FATAL, "Reception Type ="
			" SU: %d,"
			" MU_MIMO:%d,"
			" MU_OFDMA:%d,"
			" MU_OFDMA_MIMO:%d\n",
			pdev->stats.rx.reception_type[0],
			pdev->stats.rx.reception_type[1],
			pdev->stats.rx.reception_type[2],
			pdev->stats.rx.reception_type[3]);
	DP_TRACE_STATS(FATAL, "Aggregation:\n");
	DP_TRACE_STATS(FATAL, "Number of Msdu's Part of Ampdus = %d",
			pdev->stats.rx.ampdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's With No Mpdu Level Aggregation : %d",
			pdev->stats.rx.non_ampdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's Part of Amsdu: %d",
			pdev->stats.rx.amsdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's With No Msdu Level Aggregation: %d",
			pdev->stats.rx.non_amsdu_cnt);
}

/**
 * dp_print_tx_rates(): Print tx rates
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
static inline void
dp_print_tx_rates(struct dp_vdev *vdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;
	uint8_t i, pkt_type;
	char mcs[DOT11_MAX][DP_MCS_LENGTH];
	uint32_t index;

	DP_TRACE_STATS(FATAL, "Tx Rate Info:\n");

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (i = 0; i < MAX_MCS; i++) {
			index += qdf_snprint(&mcs[pkt_type][index],
					DP_MCS_LENGTH - index,
					" %d ",
					pdev->stats.tx.pkt_type[pkt_type].
					mcs_count[i]);
		}
	}

	DP_TRACE_STATS(FATAL, "11A MCS(0-7) = %s",
			mcs[0]);
	DP_TRACE_STATS(FATAL, "11A MCS Invalid = %d",
			pdev->stats.tx.pkt_type[DOT11_A].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11B MCS(0-6) = %s",
			mcs[1]);
	DP_TRACE_STATS(FATAL, "11B MCS Invalid = %d",
			pdev->stats.tx.pkt_type[DOT11_B].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11N MCS(0-7) = %s",
			mcs[2]);
	DP_TRACE_STATS(FATAL, "11N MCS Invalid = %d",
			pdev->stats.tx.pkt_type[DOT11_N].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "Type 11AC MCS(0-9) = %s",
			mcs[3]);
	DP_TRACE_STATS(FATAL, "11AC MCS Invalid = %d",
			pdev->stats.tx.pkt_type[DOT11_AC].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11AX MCS(0-11) = %s",
			mcs[4]);
	DP_TRACE_STATS(FATAL, "11AX MCS Invalid = %d",
			pdev->stats.tx.pkt_type[DOT11_AX].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "SGI ="
			" 0.8us %d"
			" 0.4us %d"
			" 1.6us %d"
			" 3.2us %d",
			pdev->stats.tx.sgi_count[0],
			pdev->stats.tx.sgi_count[1],
			pdev->stats.tx.sgi_count[2],
			pdev->stats.tx.sgi_count[3]);
	DP_TRACE_STATS(FATAL, "BW Counts = 20MHZ %d, 40MHZ %d, 80MHZ %d, 160MHZ %d",
			pdev->stats.tx.bw[0], pdev->stats.tx.bw[1],
			pdev->stats.tx.bw[2], pdev->stats.tx.bw[3]);
	DP_TRACE_STATS(FATAL, "OFDMA = %d", pdev->stats.tx.ofdma);
	DP_TRACE_STATS(FATAL, "STBC = %d", pdev->stats.tx.stbc);
	DP_TRACE_STATS(FATAL, "LDPC = %d", pdev->stats.tx.ldpc);
	DP_TRACE_STATS(FATAL, "Retries = %d", pdev->stats.tx.retries);
	DP_TRACE_STATS(FATAL, "Last ack rssi = %d\n", pdev->stats.tx.last_ack_rssi);
	DP_TRACE_STATS(FATAL, "Aggregation:\n");
	DP_TRACE_STATS(FATAL, "Number of Msdu's Part of Amsdu = %d",
			pdev->stats.tx.amsdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's With No Msdu Level Aggregation = %d",
			pdev->stats.tx.non_amsdu_cnt);
}

/**
 * dp_print_peer_stats():print peer stats
 * @peer: DP_PEER handle
 *
 * return void
 */
static inline void dp_print_peer_stats(struct dp_peer *peer)
{
	uint8_t i, pkt_type;
	char tx_mcs[DOT11_MAX][DP_MCS_LENGTH];
	char rx_mcs[DOT11_MAX][DP_MCS_LENGTH];
	uint32_t index;
	char nss[DP_NSS_LENGTH];
	DP_TRACE_STATS(FATAL, "Node Tx Stats:\n");
	DP_TRACE_STATS(FATAL, "Total Packet Completions = %d",
			peer->stats.tx.comp_pkt.num);
	DP_TRACE_STATS(FATAL, "Total Bytes Completions = %d",
			peer->stats.tx.comp_pkt.bytes);
	DP_TRACE_STATS(FATAL, "Success Packets = %d",
			peer->stats.tx.tx_success.num);
	DP_TRACE_STATS(FATAL, "Success Bytes = %d",
			peer->stats.tx.tx_success.bytes);
	DP_TRACE_STATS(FATAL, "Packets Failed = %d",
			peer->stats.tx.tx_failed);
	DP_TRACE_STATS(FATAL, "Packets In OFDMA = %d",
			peer->stats.tx.ofdma);
	DP_TRACE_STATS(FATAL, "Packets In STBC = %d",
			peer->stats.tx.stbc);
	DP_TRACE_STATS(FATAL, "Packets In LDPC = %d",
			peer->stats.tx.ldpc);
	DP_TRACE_STATS(FATAL, "Packet Retries = %d",
			peer->stats.tx.retries);
	DP_TRACE_STATS(FATAL, "Msdu's Not Part of Ampdu = %d",
			peer->stats.tx.non_amsdu_cnt);
	DP_TRACE_STATS(FATAL, "Mpdu's Part of Ampdu = %d",
			peer->stats.tx.amsdu_cnt);
	DP_TRACE_STATS(FATAL, "Last Packet RSSI = %d",
			peer->stats.tx.last_ack_rssi);
	DP_TRACE_STATS(FATAL, "Dropped At FW: FW Discard = %d",
			peer->stats.tx.dropped.fw_discard);
	DP_TRACE_STATS(FATAL, "Dropped At FW: FW Discard Retired = %d",
			peer->stats.tx.dropped.fw_discard_retired);
	DP_TRACE_STATS(FATAL, "Dropped At FW: FW Discard Untransmitted = %d",
			peer->stats.tx.dropped.fw_discard_untransmitted);
	DP_TRACE_STATS(FATAL, "Dropped : Mpdu Age Out = %d",
			peer->stats.tx.dropped.mpdu_age_out);
	DP_TRACE_STATS(FATAL, "Dropped : FW Discard Reason1 = %d",
			peer->stats.tx.dropped.fw_discard_reason1);
	DP_TRACE_STATS(FATAL, "Dropped : FW Discard Reason2 = %d",
			peer->stats.tx.dropped.fw_discard_reason2);
	DP_TRACE_STATS(FATAL, "Dropped : FW Discard Reason3 = %d",
			peer->stats.tx.dropped.fw_discard_reason3);

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (i = 0; i < MAX_MCS; i++) {
			index += qdf_snprint(&tx_mcs[pkt_type][index],
					DP_MCS_LENGTH - index,
					" %d ",
					peer->stats.tx.pkt_type[pkt_type].
					mcs_count[i]);
		}
	}

	DP_TRACE_STATS(FATAL, "11A MCS(0-7) = %s",
			tx_mcs[0]);
	DP_TRACE_STATS(FATAL, "11A MCS Invalid = %d",
			peer->stats.tx.pkt_type[DOT11_A].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11B MCS(0-6) = %s",
			tx_mcs[1]);
	DP_TRACE_STATS(FATAL, "11B MCS Invalid = %d",
			peer->stats.tx.pkt_type[DOT11_B].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11N MCS(0-7) = %s",
			tx_mcs[2]);
	DP_TRACE_STATS(FATAL, "11N MCS Invalid = %d",
			peer->stats.tx.pkt_type[DOT11_N].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11AC MCS(0-9) = %s",
			tx_mcs[3]);
	DP_TRACE_STATS(FATAL, "11AC MCS Invalid = %d",
			peer->stats.tx.pkt_type[DOT11_AC].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11AX MCS(0-11) = %s",
			tx_mcs[4]);
	DP_TRACE_STATS(FATAL, "11AX MCS Invalid = %d",
			peer->stats.tx.pkt_type[DOT11_AX].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "SGI = "
			" 0.8us %d"
			" 0.4us %d"
			" 1.6us %d"
			" 3.2us %d",
			peer->stats.tx.sgi_count[0],
			peer->stats.tx.sgi_count[1],
			peer->stats.tx.sgi_count[2],
			peer->stats.tx.sgi_count[3]);
	DP_TRACE_STATS(FATAL, "BW Counts = 20MHZ %d 40MHZ %d 80MHZ %d 160MHZ %d\n",
			peer->stats.tx.bw[0], peer->stats.tx.bw[1],
			peer->stats.tx.bw[2], peer->stats.tx.bw[3]);
	DP_TRACE_STATS(FATAL, "Aggregation:\n");
	DP_TRACE_STATS(FATAL, "Number of Msdu's Part of Amsdu = %d",
			peer->stats.tx.amsdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's With No Msdu Level Aggregation = %d\n",
			peer->stats.tx.non_amsdu_cnt);

	DP_TRACE_STATS(FATAL, "Node Rx Stats:\n");
	DP_TRACE_STATS(FATAL, "Packets Sent To Stack = %d",
			peer->stats.rx.to_stack.num);
	DP_TRACE_STATS(FATAL, "Bytes Sent To Stack = %d",
			peer->stats.rx.to_stack.bytes);
	for (i = 0; i <  CDP_MAX_RX_RINGS; i++) {
		DP_TRACE_STATS(FATAL, "Packets Received = %d",
				peer->stats.rx.rcvd_reo[i].num);
		DP_TRACE_STATS(FATAL, "Bytes Received = %d",
				peer->stats.rx.rcvd_reo[i].bytes);
	}
	DP_TRACE_STATS(FATAL, "Multicast Packets Received = %d",
			peer->stats.rx.multicast.num);
	DP_TRACE_STATS(FATAL, "Multicast Bytes Received = %d",
			peer->stats.rx.multicast.bytes);
	DP_TRACE_STATS(FATAL, "WDS Packets Received = %d",
			peer->stats.rx.wds.num);
	DP_TRACE_STATS(FATAL, "WDS Bytes Received = %d",
			peer->stats.rx.wds.bytes);
	DP_TRACE_STATS(FATAL, "Intra BSS Packets Received = %d",
			peer->stats.rx.intra_bss.pkts.num);
	DP_TRACE_STATS(FATAL, "Intra BSS Bytes Received = %d",
			peer->stats.rx.intra_bss.pkts.bytes);
	DP_TRACE_STATS(FATAL, "Raw Packets Received = %d",
			peer->stats.rx.raw.num);
	DP_TRACE_STATS(FATAL, "Raw Bytes Received = %d",
			peer->stats.rx.raw.bytes);
	DP_TRACE_STATS(FATAL, "Errors: MIC Errors = %d",
			peer->stats.rx.err.mic_err);
	DP_TRACE_STATS(FATAL, "Erros: Decryption Errors = %d",
			peer->stats.rx.err.decrypt_err);
	DP_TRACE_STATS(FATAL, "Msdu's Received As Part of Ampdu = %d",
			peer->stats.rx.non_ampdu_cnt);
	DP_TRACE_STATS(FATAL, "Msdu's Recived As Ampdu = %d",
			peer->stats.rx.ampdu_cnt);
	DP_TRACE_STATS(FATAL, "Msdu's Received Not Part of Amsdu's = %d",
			peer->stats.rx.non_amsdu_cnt);
	DP_TRACE_STATS(FATAL, "MSDUs Received As Part of Amsdu = %d",
			peer->stats.rx.amsdu_cnt);
	DP_TRACE_STATS(FATAL, "SGI ="
			" 0.8us %d"
			" 0.4us %d"
			" 1.6us %d"
			" 3.2us %d",
			peer->stats.rx.sgi_count[0],
			peer->stats.rx.sgi_count[1],
			peer->stats.rx.sgi_count[2],
			peer->stats.rx.sgi_count[3]);
	DP_TRACE_STATS(FATAL, "BW Counts = 20MHZ %d 40MHZ %d 80MHZ %d 160MHZ %d",
			peer->stats.rx.bw[0], peer->stats.rx.bw[1],
			peer->stats.rx.bw[2], peer->stats.rx.bw[3]);
	DP_TRACE_STATS(FATAL, "Reception Type ="
			" SU %d,"
			" MU_MIMO %d,"
			" MU_OFDMA %d,"
			" MU_OFDMA_MIMO %d",
			peer->stats.rx.reception_type[0],
			peer->stats.rx.reception_type[1],
			peer->stats.rx.reception_type[2],
			peer->stats.rx.reception_type[3]);

	for (pkt_type = 0; pkt_type < DOT11_MAX; pkt_type++) {
		index = 0;
		for (i = 0; i < MAX_MCS; i++) {
			index += qdf_snprint(&rx_mcs[pkt_type][index],
					DP_MCS_LENGTH - index,
					" %d ",
					peer->stats.rx.pkt_type[pkt_type].
					mcs_count[i]);
		}
	}

	DP_TRACE_STATS(FATAL, "11A MCS(0-7) = %s",
			rx_mcs[0]);
	DP_TRACE_STATS(FATAL, "11A MCS Invalid = %d",
			peer->stats.rx.pkt_type[DOT11_A].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11B MCS(0-6) = %s",
			rx_mcs[1]);
	DP_TRACE_STATS(FATAL, "11B MCS Invalid = %d",
			peer->stats.rx.pkt_type[DOT11_B].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11N MCS(0-7) = %s",
			rx_mcs[2]);
	DP_TRACE_STATS(FATAL, "11N MCS Invalid = %d",
			peer->stats.rx.pkt_type[DOT11_N].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11AC MCS(0-9) = %s",
			rx_mcs[3]);
	DP_TRACE_STATS(FATAL, "11AC MCS Invalid = %d",
			peer->stats.rx.pkt_type[DOT11_AC].mcs_count[MAX_MCS]);
	DP_TRACE_STATS(FATAL, "11AX MCS(0-11) = %s",
			rx_mcs[4]);
	DP_TRACE_STATS(FATAL, "11AX MCS Invalid = %d",
			peer->stats.rx.pkt_type[DOT11_AX].mcs_count[MAX_MCS]);

	index = 0;
	for (i = 0; i < SS_COUNT; i++) {
		index += qdf_snprint(&nss[index], DP_NSS_LENGTH - index,
				" %d", peer->stats.rx.nss[i]);
	}
	DP_TRACE_STATS(FATAL, "NSS(0-7) = %s\n",
			nss);
	DP_TRACE_STATS(FATAL, "Aggregation:\n");
	DP_TRACE_STATS(FATAL, "Number of Msdu's Part of Ampdu = %d",
			peer->stats.rx.ampdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's With No Mpdu Level Aggregation = %d",
			peer->stats.rx.non_ampdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's Part of Amsdu = %d",
			peer->stats.rx.amsdu_cnt);
	DP_TRACE_STATS(FATAL, "Number of Msdu's With No Msdu Level Aggregation = %d",
			peer->stats.rx.non_amsdu_cnt);
}

/**
 * dp_print_host_stats()- Function to print the stats aggregated at host
 * @vdev_handle: DP_VDEV handle
 * @type: host stats type
 *
 * Available Stat types
 * TXRX_CLEAR_STATS  : Clear the stats
 * TXRX_RX_RATE_STATS: Print Rx Rate Info
 * TXRX_TX_RATE_STATS: Print Tx Rate Info
 * TXRX_TX_HOST_STATS: Print Tx Stats
 * TXRX_RX_HOST_STATS: Print Rx Stats
 *
 * Return: 0 on success, print error message in case of failure
 */
static int
dp_print_host_stats(struct cdp_vdev *vdev_handle, enum cdp_host_txrx_stats type)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = (struct dp_pdev *)vdev->pdev;

	dp_aggregate_pdev_stats(pdev);
	switch (type) {
	case TXRX_CLEAR_STATS:
		dp_txrx_host_stats_clr(vdev);
		break;
	case TXRX_RX_RATE_STATS:
		dp_print_rx_rates(vdev);
		break;
	case TXRX_TX_RATE_STATS:
		dp_print_tx_rates(vdev);
		break;
	case TXRX_TX_HOST_STATS:
		dp_print_pdev_tx_stats(pdev);
		dp_print_soc_tx_stats(pdev->soc);
		break;
	case TXRX_RX_HOST_STATS:
		dp_print_pdev_rx_stats(pdev);
		dp_print_soc_rx_stats(pdev->soc);
		break;
	default:
		DP_TRACE(FATAL, "Wrong Input For TxRx Host Stats");
		break;
	}
	return 0;
}

/*
 * dp_get_host_peer_stats()- function to print peer stats
 * @pdev_handle: DP_PDEV handle
 * @mac_addr: mac address of the peer
 *
 * Return: void
 */
static void
dp_get_host_peer_stats(struct cdp_pdev *pdev_handle, char *mac_addr)
{
	struct dp_peer *peer;
	uint8_t local_id;
	peer = (struct dp_peer *)dp_find_peer_by_addr(pdev_handle, mac_addr,
			&local_id);

	dp_print_peer_stats(peer);
	return;
}

/*
 * dp_get_fw_peer_stats()- function to print peer stats
 * @pdev_handle: DP_PDEV handle
 * @mac_addr: mac address of the peer
 * @cap: Type of htt stats requested
 *
 * Currently Supporting only MAC ID based requests Only
 *	1: HTT_PEER_STATS_REQ_MODE_NO_QUERY
 *	2: HTT_PEER_STATS_REQ_MODE_QUERY_TQM
 *	3: HTT_PEER_STATS_REQ_MODE_FLUSH_TQM
 *
 * Return: void
 */
static void
dp_get_fw_peer_stats(struct cdp_pdev *pdev_handle, uint8_t *mac_addr,
		uint32_t cap)
{
	struct dp_pdev *pdev = (struct dp_pdev *)pdev_handle;
	uint32_t config_param0 = 0;
	uint32_t config_param1 = 0;
	uint32_t config_param2 = 0;
	uint32_t config_param3 = 0;

	HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_SET(config_param0, 1);
	config_param0 |= (1 << (cap + 1));

	config_param1 = 0x8f;

	config_param2 |= (mac_addr[0] & 0x000000ff);
	config_param2 |= ((mac_addr[1] << 8) & 0x0000ff00);
	config_param2 |= ((mac_addr[2] << 16) & 0x00ff0000);
	config_param2 |= ((mac_addr[3] << 24) & 0xff000000);

	config_param3 |= (mac_addr[4] & 0x000000ff);
	config_param3 |= ((mac_addr[5] << 8) & 0x0000ff00);

	dp_h2t_ext_stats_msg_send(pdev, HTT_DBG_EXT_STATS_PEER_INFO,
			config_param0, config_param1, config_param2,
			config_param3);
}

/*
 * dp_set_vdev_param: function to set parameters in vdev
 * @param: parameter type to be set
 * @val: value of parameter to be set
 *
 * return: void
 */
static void dp_set_vdev_param(struct cdp_vdev *vdev_handle,
		enum cdp_vdev_param_type param, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;

	switch (param) {
	case CDP_ENABLE_WDS:
		vdev->wds_enabled = val;
		break;
	case CDP_ENABLE_NAWDS:
		vdev->nawds_enabled = val;
		break;
	case CDP_ENABLE_MCAST_EN:
		vdev->mcast_enhancement_en = val;
		break;
	case CDP_ENABLE_PROXYSTA:
		vdev->proxysta_vdev = val;
		break;
	case CDP_UPDATE_TDLS_FLAGS:
		vdev->tdls_link_connected = val;
		break;
	default:
		break;
	}

	dp_tx_vdev_update_search_flags(vdev);
}

/**
 * dp_peer_set_nawds: set nawds bit in peer
 * @peer_handle: pointer to peer
 * @value: enable/disable nawds
 *
 * return: void
 */
static void dp_peer_set_nawds(void *peer_handle, uint8_t value)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	peer->nawds_enabled = value;
}

/*
 * dp_set_vdev_dscp_tid_map_wifi3(): Update Map ID selected for particular vdev
 * @vdev_handle: DP_VDEV handle
 * @map_id:ID of map that needs to be updated
 *
 * Return: void
 */
static void dp_set_vdev_dscp_tid_map_wifi3(struct cdp_vdev *vdev_handle,
		uint8_t map_id)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->dscp_tid_map_id = map_id;
	return;
}

/**
 * dp_set_pdev_dscp_tid_map_wifi3(): update dscp tid map in pdev
 * @pdev: DP_PDEV handle
 * @map_id: ID of map that needs to be updated
 * @tos: index value in map
 * @tid: tid value passed by the user
 *
 * Return: void
 */
static void dp_set_pdev_dscp_tid_map_wifi3(struct cdp_pdev *pdev_handle,
		uint8_t map_id, uint8_t tos, uint8_t tid)
{
	uint8_t dscp;
	struct dp_pdev *pdev = (struct dp_pdev *) pdev_handle;
	dscp = (tos >> DP_IP_DSCP_SHIFT) & DP_IP_DSCP_MASK;
	pdev->dscp_tid_map[map_id][dscp] = tid;
	hal_tx_update_dscp_tid(pdev->soc->hal_soc, tid,
			map_id, dscp);
	return;
}

/**
 * dp_fw_stats_process(): Process TxRX FW stats request
 * @vdev_handle: DP VDEV handle
 * @val: value passed by user
 *
 * return: int
 */
static int dp_fw_stats_process(struct cdp_vdev *vdev_handle, uint32_t val)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	struct dp_pdev *pdev = NULL;

	if (!vdev) {
		DP_TRACE(NONE, "VDEV not found");
		return 1;
	}

	pdev = vdev->pdev;
	return dp_h2t_ext_stats_msg_send(pdev, val, 0, 0, 0, 0);
}

/*
 * dp_txrx_stats() - function to map to firmware and host stats
 * @vdev: virtual handle
 * @stats: type of statistics requested
 *
 * Return: integer
 */
static int dp_txrx_stats(struct cdp_vdev *vdev, enum cdp_stats stats)
{
	int host_stats;
	int fw_stats;

	if (stats >= CDP_TXRX_MAX_STATS)
		return 0;

	/*
	 * DP_CURR_FW_STATS_AVAIL: no of FW stats currently available
	 *			has to be updated if new FW HTT stats added
	 */
	if (stats > CDP_TXRX_STATS_HTT_MAX)
		stats = stats + DP_CURR_FW_STATS_AVAIL - DP_HTT_DBG_EXT_STATS_MAX;
	fw_stats = dp_stats_mapping_table[stats][STATS_FW];
	host_stats = dp_stats_mapping_table[stats][STATS_HOST];

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		 "stats: %u fw_stats_type: %d host_stats_type: %d",
		  stats, fw_stats, host_stats);

	if (fw_stats != TXRX_FW_STATS_INVALID)
		return dp_fw_stats_process(vdev, fw_stats);

	if ((host_stats != TXRX_HOST_STATS_INVALID) &&
			(host_stats <= TXRX_HOST_STATS_MAX))
		return dp_print_host_stats(vdev, host_stats);
	else
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				"Wrong Input for TxRx Stats");

	return 0;
}

/*
 * dp_print_per_ring_stats(): Packet count per ring
 * @soc - soc handle
 */
static void dp_print_per_ring_stats(struct dp_soc *soc)
{
	uint8_t core, ring;
	uint64_t total_packets;

	DP_TRACE(FATAL, "Reo packets per ring:");
	for (ring = 0; ring < MAX_REO_DEST_RINGS; ring++) {
		total_packets = 0;
		DP_TRACE(FATAL, "Packets on ring %u:", ring);
		for (core = 0; core < NR_CPUS; core++) {
			DP_TRACE(FATAL, "Packets arriving on core %u: %llu",
				core, soc->stats.rx.ring_packets[core][ring]);
			total_packets += soc->stats.rx.ring_packets[core][ring];
		}
		DP_TRACE(FATAL, "Total packets on ring %u: %llu",
			ring, total_packets);
	}
}

/*
 * dp_txrx_path_stats() - Function to display dump stats
 * @soc - soc handle
 *
 * return: none
 */
static void dp_txrx_path_stats(struct dp_soc *soc)
{
	uint8_t error_code;
	uint8_t loop_pdev;
	struct dp_pdev *pdev;
	uint8_t i;

	for (loop_pdev = 0; loop_pdev < soc->pdev_count; loop_pdev++) {

		pdev = soc->pdev_list[loop_pdev];
		dp_aggregate_pdev_stats(pdev);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Tx path Statistics:");

		DP_TRACE(FATAL, "from stack: %u msdus (%u bytes)",
			pdev->stats.tx_i.rcvd.num,
			pdev->stats.tx_i.rcvd.bytes);
		DP_TRACE(FATAL, "processed from host: %u msdus (%u bytes)",
			pdev->stats.tx_i.processed.num,
			pdev->stats.tx_i.processed.bytes);
		DP_TRACE(FATAL, "successfully transmitted: %u msdus (%u bytes)",
			pdev->stats.tx.tx_success.num,
			pdev->stats.tx.tx_success.bytes);

		DP_TRACE(FATAL, "Dropped in host:");
		DP_TRACE(FATAL, "Total packets dropped: %u,",
			pdev->stats.tx_i.dropped.dropped_pkt.num);
		DP_TRACE(FATAL, "Descriptor not available: %u",
			pdev->stats.tx_i.dropped.desc_na);
		DP_TRACE(FATAL, "Ring full: %u",
			pdev->stats.tx_i.dropped.ring_full);
		DP_TRACE(FATAL, "Enqueue fail: %u",
			pdev->stats.tx_i.dropped.enqueue_fail);
		DP_TRACE(FATAL, "DMA Error: %u",
			pdev->stats.tx_i.dropped.dma_error);

		DP_TRACE(FATAL, "Dropped in hardware:");
		DP_TRACE(FATAL, "total packets dropped: %u",
			pdev->stats.tx.tx_failed);
		DP_TRACE(FATAL, "mpdu age out: %u",
			pdev->stats.tx.dropped.mpdu_age_out);
		DP_TRACE(FATAL, "firmware discard reason1: %u",
			pdev->stats.tx.dropped.fw_discard_reason1);
		DP_TRACE(FATAL, "firmware discard reason2: %u",
			pdev->stats.tx.dropped.fw_discard_reason2);
		DP_TRACE(FATAL, "firmware discard reason3: %u",
			pdev->stats.tx.dropped.fw_discard_reason3);
		DP_TRACE(FATAL, "peer_invalid: %u",
			pdev->soc->stats.tx.tx_invalid_peer.num);


		DP_TRACE(FATAL, "Tx packets sent per interrupt:");
		DP_TRACE(FATAL, "Single Packet: %u",
			pdev->stats.tx_comp_histogram.pkts_1);
		DP_TRACE(FATAL, "2-20 Packets:  %u",
			pdev->stats.tx_comp_histogram.pkts_2_20);
		DP_TRACE(FATAL, "21-40 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_21_40);
		DP_TRACE(FATAL, "41-60 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_41_60);
		DP_TRACE(FATAL, "61-80 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_61_80);
		DP_TRACE(FATAL, "81-100 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_81_100);
		DP_TRACE(FATAL, "101-200 Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_101_200);
		DP_TRACE(FATAL, "   201+ Packets: %u",
			pdev->stats.tx_comp_histogram.pkts_201_plus);

		DP_TRACE(FATAL, "Rx path statistics");

		DP_TRACE(FATAL, "delivered %u msdus ( %u bytes),",
			pdev->stats.rx.to_stack.num,
			pdev->stats.rx.to_stack.bytes);
		for (i = 0; i <  CDP_MAX_RX_RINGS; i++)
			DP_TRACE(FATAL, "received on reo[%d] %u msdus ( %u bytes),",
					i, pdev->stats.rx.rcvd_reo[i].num,
					pdev->stats.rx.rcvd_reo[i].bytes);
		DP_TRACE(FATAL, "intra-bss packets %u msdus ( %u bytes),",
			pdev->stats.rx.intra_bss.pkts.num,
			pdev->stats.rx.intra_bss.pkts.bytes);
		DP_TRACE(FATAL, "raw packets %u msdus ( %u bytes),",
			pdev->stats.rx.raw.num,
			pdev->stats.rx.raw.bytes);
		DP_TRACE(FATAL, "dropped: error %u msdus",
			pdev->stats.rx.err.mic_err);
		DP_TRACE(FATAL, "peer invalid %u",
			pdev->soc->stats.rx.err.rx_invalid_peer.num);

		DP_TRACE(FATAL, "Reo Statistics");
		DP_TRACE(FATAL, "rbm error: %u msdus",
			pdev->soc->stats.rx.err.invalid_rbm);
		DP_TRACE(FATAL, "hal ring access fail: %u msdus",
			pdev->soc->stats.rx.err.hal_ring_access_fail);

		DP_TRACE(FATAL, "Reo errors");

		for (error_code = 0; error_code < REO_ERROR_TYPE_MAX;
				error_code++) {
			DP_TRACE(FATAL, "Reo error number (%u): %u msdus",
				error_code,
				pdev->soc->stats.rx.err.reo_error[error_code]);
		}

		for (error_code = 0; error_code < MAX_RXDMA_ERRORS;
				error_code++) {
			DP_TRACE(FATAL, "Rxdma error number (%u): %u msdus",
				error_code,
				pdev->soc->stats.rx.err
				.rxdma_error[error_code]);
		}

		DP_TRACE(FATAL, "Rx packets reaped per interrupt:");
		DP_TRACE(FATAL, "Single Packet: %u",
			 pdev->stats.rx_ind_histogram.pkts_1);
		DP_TRACE(FATAL, "2-20 Packets:  %u",
			 pdev->stats.rx_ind_histogram.pkts_2_20);
		DP_TRACE(FATAL, "21-40 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_21_40);
		DP_TRACE(FATAL, "41-60 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_41_60);
		DP_TRACE(FATAL, "61-80 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_61_80);
		DP_TRACE(FATAL, "81-100 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_81_100);
		DP_TRACE(FATAL, "101-200 Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_101_200);
		DP_TRACE(FATAL, "   201+ Packets: %u",
			 pdev->stats.rx_ind_histogram.pkts_201_plus);
	}
}

/*
 * dp_txrx_dump_stats() -  Dump statistics
 * @value - Statistics option
 */
static QDF_STATUS dp_txrx_dump_stats(void *psoc, uint16_t value)
{
	struct dp_soc *soc =
		(struct dp_soc *)psoc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: soc is NULL", __func__);
		return QDF_STATUS_E_INVAL;
	}

	switch (value) {
	case CDP_TXRX_PATH_STATS:
		dp_txrx_path_stats(soc);
		break;

	case CDP_RX_RING_STATS:
		dp_print_per_ring_stats(soc);
		break;

	case CDP_TXRX_TSO_STATS:
		/* TODO: NOT IMPLEMENTED */
		break;

	case CDP_DUMP_TX_FLOW_POOL_INFO:
		/* TODO: NOT IMPLEMENTED */
		break;

	case CDP_TXRX_DESC_STATS:
		/* TODO: NOT IMPLEMENTED */
		break;

	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;

}

static struct cdp_wds_ops dp_ops_wds = {
	.vdev_set_wds = dp_vdev_set_wds,
};

static struct cdp_cmn_ops dp_ops_cmn = {
	.txrx_soc_attach_target = dp_soc_attach_target_wifi3,
	.txrx_vdev_attach = dp_vdev_attach_wifi3,
	.txrx_vdev_detach = dp_vdev_detach_wifi3,
	.txrx_pdev_attach = dp_pdev_attach_wifi3,
	.txrx_pdev_detach = dp_pdev_detach_wifi3,
	.txrx_peer_create = dp_peer_create_wifi3,
	.txrx_peer_setup = dp_peer_setup_wifi3,
	.txrx_peer_teardown = NULL,
	.txrx_peer_delete = dp_peer_delete_wifi3,
	.txrx_vdev_register = dp_vdev_register_wifi3,
	.txrx_soc_detach = dp_soc_detach_wifi3,
	.txrx_get_vdev_mac_addr = dp_get_vdev_mac_addr_wifi3,
	.txrx_get_vdev_from_vdev_id = dp_get_vdev_from_vdev_id_wifi3,
	.txrx_get_ctrl_pdev_from_vdev = dp_get_ctrl_pdev_from_vdev_wifi3,
	.addba_requestprocess = dp_addba_requestprocess_wifi3,
	.addba_responsesetup = dp_addba_responsesetup_wifi3,
	.delba_process = dp_delba_process_wifi3,
	.get_peer_mac_addr_frm_id = dp_get_peer_mac_addr_frm_id,
	.flush_cache_rx_queue = NULL,
	/* TODO: get API's for dscp-tid need to be added*/
	.set_vdev_dscp_tid_map = dp_set_vdev_dscp_tid_map_wifi3,
	.set_pdev_dscp_tid_map = dp_set_pdev_dscp_tid_map_wifi3,
	.txrx_stats = dp_txrx_stats,
	.txrx_set_monitor_mode = dp_vdev_set_monitor_mode,
	.display_stats = dp_txrx_dump_stats,
	.txrx_soc_set_nss_cfg = dp_soc_set_nss_cfg_wifi3,
	.txrx_soc_get_nss_cfg = dp_soc_get_nss_cfg_wifi3,
	.txrx_intr_attach = dp_soc_interrupt_attach,
	.txrx_intr_detach = dp_soc_interrupt_detach,
	/* TODO: Add other functions */
};

static struct cdp_ctrl_ops dp_ops_ctrl = {
	.txrx_peer_authorize = dp_peer_authorize,
	.txrx_set_vdev_rx_decap_type = dp_set_vdev_rx_decap_type,
	.txrx_set_tx_encap_type = dp_set_vdev_tx_encap_type,
#ifdef MESH_MODE_SUPPORT
	.txrx_set_mesh_mode  = dp_peer_set_mesh_mode,
	.txrx_set_mesh_rx_filter = dp_peer_set_mesh_rx_filter,
#endif
	.txrx_set_vdev_param = dp_set_vdev_param,
	.txrx_peer_set_nawds = dp_peer_set_nawds,
	.txrx_set_pdev_reo_dest = dp_set_pdev_reo_dest,
	.txrx_get_pdev_reo_dest = dp_get_pdev_reo_dest,
	.txrx_set_filter_neighbour_peers = dp_set_filter_neighbour_peers,
	.txrx_update_filter_neighbour_peers =
		dp_update_filter_neighbour_peers,
	/* TODO: Add other functions */
};

static struct cdp_me_ops dp_ops_me = {
#ifdef ATH_SUPPORT_IQUE
	.tx_me_alloc_descriptor = dp_tx_me_alloc_descriptor,
	.tx_me_free_descriptor = dp_tx_me_free_descriptor,
	.tx_me_convert_ucast = dp_tx_me_send_convert_ucast,
#endif
};

static struct cdp_mon_ops dp_ops_mon = {
	.txrx_monitor_set_filter_ucast_data = NULL,
	.txrx_monitor_set_filter_mcast_data = NULL,
	.txrx_monitor_set_filter_non_data = NULL,
	.txrx_monitor_get_filter_ucast_data = NULL,
	.txrx_monitor_get_filter_mcast_data = NULL,
	.txrx_monitor_get_filter_non_data = NULL,
	.txrx_reset_monitor_mode = NULL,
};

static struct cdp_host_stats_ops dp_ops_host_stats = {
	.txrx_per_peer_stats = dp_get_host_peer_stats,
	.get_fw_peer_stats = dp_get_fw_peer_stats,
	/* TODO */
};

static struct cdp_raw_ops dp_ops_raw = {
	/* TODO */
};

#ifdef CONFIG_WIN
static struct cdp_pflow_ops dp_ops_pflow = {
	/* TODO */
};
#endif /* CONFIG_WIN */

#ifndef CONFIG_WIN
static struct cdp_misc_ops dp_ops_misc = {
	.get_opmode = dp_get_opmode,
};

static struct cdp_flowctl_ops dp_ops_flowctl = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_lflowctl_ops dp_ops_l_flowctl = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_ipa_ops dp_ops_ipa = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

/**
 * dp_dummy_bus_suspend() - dummy bus suspend op
 *
 * FIXME - This is a placeholder for the actual logic!
 *
 * Return: QDF_STATUS_SUCCESS
 */
inline QDF_STATUS dp_dummy_bus_suspend(void)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_dummy_bus_resume() - dummy bus resume
 *
 * FIXME - This is a placeholder for the actual logic!
 *
 * Return: QDF_STATUS_SUCCESS
 */
inline QDF_STATUS dp_dummy_bus_resume(void)
{
	return QDF_STATUS_SUCCESS;
}

static struct cdp_bus_ops dp_ops_bus = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
	.bus_suspend = dp_dummy_bus_suspend,
	.bus_resume = dp_dummy_bus_resume
};

static struct cdp_ocb_ops dp_ops_ocb = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};


static struct cdp_throttle_ops dp_ops_throttle = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_mob_stats_ops dp_ops_mob_stats = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_cfg_ops dp_ops_cfg = {
	/* WIFI 3.0 DP NOT IMPLEMENTED YET */
};

static struct cdp_peer_ops dp_ops_peer = {
	.register_peer = dp_register_peer,
	.clear_peer = dp_clear_peer,
	.find_peer_by_addr = dp_find_peer_by_addr,
	.find_peer_by_addr_and_vdev = dp_find_peer_by_addr_and_vdev,
	.local_peer_id = dp_local_peer_id,
	.peer_find_by_local_id = dp_peer_find_by_local_id,
	.peer_state_update = dp_peer_state_update,
	.get_vdevid = dp_get_vdevid,
	.peer_get_peer_mac_addr = dp_peer_get_peer_mac_addr,
	.get_vdev_for_peer = dp_get_vdev_for_peer,
	.get_peer_state = dp_get_peer_state,
	.last_assoc_received = dp_get_last_assoc_received,
	.last_disassoc_received = dp_get_last_disassoc_received,
	.last_deauth_received = dp_get_last_deauth_received,
};
#endif

static struct cdp_ops dp_txrx_ops = {
	.cmn_drv_ops = &dp_ops_cmn,
	.ctrl_ops = &dp_ops_ctrl,
	.me_ops = &dp_ops_me,
	.mon_ops = &dp_ops_mon,
	.host_stats_ops = &dp_ops_host_stats,
	.wds_ops = &dp_ops_wds,
	.raw_ops = &dp_ops_raw,
#ifdef CONFIG_WIN
	.pflow_ops = &dp_ops_pflow,
#endif /* CONFIG_WIN */
#ifndef CONFIG_WIN
	.misc_ops = &dp_ops_misc,
	.cfg_ops = &dp_ops_cfg,
	.flowctl_ops = &dp_ops_flowctl,
	.l_flowctl_ops = &dp_ops_l_flowctl,
	.ipa_ops = &dp_ops_ipa,
	.bus_ops = &dp_ops_bus,
	.ocb_ops = &dp_ops_ocb,
	.peer_ops = &dp_ops_peer,
	.throttle_ops = &dp_ops_throttle,
	.mob_stats_ops = &dp_ops_mob_stats,
#endif
};

/*
 * dp_soc_attach_wifi3() - Attach txrx SOC
 * @osif_soc:		Opaque SOC handle from OSIF/HDD
 * @htc_handle:	Opaque HTC handle
 * @hif_handle:	Opaque HIF handle
 * @qdf_osdev:	QDF device
 *
 * Return: DP SOC handle on success, NULL on failure
 */
/*
 * Local prototype added to temporarily address warning caused by
 * -Wmissing-prototypes. A more correct solution, namely to expose
 * a prototype in an appropriate header file, will come later.
 */
void *dp_soc_attach_wifi3(void *osif_soc, void *hif_handle,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
	struct ol_if_ops *ol_ops, struct wlan_objmgr_psoc *psoc);
void *dp_soc_attach_wifi3(void *osif_soc, void *hif_handle,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
	struct ol_if_ops *ol_ops, struct wlan_objmgr_psoc *psoc)
{
	struct dp_soc *soc = qdf_mem_malloc(sizeof(*soc));

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("DP SOC memory allocation failed"));
		goto fail0;
	}

	soc->cdp_soc.ops = &dp_txrx_ops;
	soc->cdp_soc.ol_ops = ol_ops;
	soc->osif_soc = osif_soc;
	soc->osdev = qdf_osdev;
	soc->hif_handle = hif_handle;
	soc->psoc = psoc;

	soc->hal_soc = hif_get_hal_handle(hif_handle);
	soc->htt_handle = htt_soc_attach(soc, osif_soc, htc_handle,
		soc->hal_soc, qdf_osdev);
	if (!soc->htt_handle) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HTT attach failed"));
		goto fail1;
	}

	soc->wlan_cfg_ctx = wlan_cfg_soc_attach();
	if (!soc->wlan_cfg_ctx) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				FL("wlan_cfg_soc_attach failed"));
		goto fail2;
	}
	qdf_spinlock_create(&soc->peer_ref_mutex);

	qdf_spinlock_create(&soc->reo_desc_freelist_lock);
	qdf_list_create(&soc->reo_desc_freelist, REO_DESC_FREELIST_SIZE);

	return (void *)soc;

fail2:
	htt_soc_detach(soc->htt_handle);
fail1:
	qdf_mem_free(soc);
fail0:
	return NULL;
}

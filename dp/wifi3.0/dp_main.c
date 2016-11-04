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

#include <qdf_types.h>
#include <qdf_lock.h>
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_htt.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_tx.h"
#include "wlan_cfg.h"

/**
 * dp_setup_srng - Internal function to setup SRNG rings used by data path
 */
static int dp_srng_setup(struct dp_soc *soc, struct dp_srng *srng,
	int ring_type, int ring_num, int pdev_id, uint32_t num_entries)
{
	void *hal_soc = soc->hal_soc;
	uint32_t entry_size = hal_srng_get_entrysize(hal_soc, ring_type);
	/* TODO: See if we should get align size from hal */
	uint32_t ring_base_align = 8;
	struct hal_srng_params ring_params;


	srng->hal_srng = NULL;
	srng->alloc_size = (num_entries * entry_size) + ring_base_align - 1;
	srng->base_vaddr_unaligned = qdf_mem_alloc_consistent(
		soc->osdev, NULL, srng->alloc_size,
		&(srng->base_paddr_unaligned));

	if (!srng->base_vaddr_unaligned) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: alloc failed - ring_type: %d, ring_num %d\n",
			__func__, ring_type, ring_num);
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
		pdev_id, &ring_params);
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
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring type: %d, num:%d not setup\n",
			__func__, ring_type, ring_num);
		return;
	}

	hal_srng_cleanup(soc->hal_soc, srng->hal_srng);

	qdf_mem_free_consistent(soc->osdev, NULL,
				srng->alloc_size,
				srng->base_vaddr_unaligned,
				srng->base_paddr_unaligned, 0);
}

/* TODO: Need this interface from HIF */
void *hif_get_hal_handle(void *hif_handle);

/*
 * dp_soc_attach_wifi3() - Attach txrx SOC
 * @osif_soc:		Opaque SOC handle from OSIF/HDD
 * @htc_handle:	Opaque HTC handle
 * @hif_handle:	Opaque HIF handle
 * @qdf_osdev:	QDF device
 *
 * Return: DP SOC handle on success, NULL on failure
 */
void *dp_soc_attach_wifi3(void *osif_soc, void *hif_handle,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
	struct ol_if_ops *ol_ops)
{
	struct dp_soc *soc = qdf_mem_malloc(sizeof(*soc));

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: DP SOC memory allocation failed\n", __func__);
		goto fail0;
	}

	soc->osif_soc = osif_soc;
	soc->osdev = qdf_osdev;
	soc->ol_ops = ol_ops;
	soc->hif_handle = hif_handle;
	soc->hal_soc = hif_get_hal_handle(hif_handle);
	soc->htt_handle = htt_soc_attach(soc, osif_soc, htc_handle,
		soc->hal_soc, qdf_osdev);
	if (soc->htt_handle == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: HTT attach failed\n", __func__);
		goto fail1;
	}

#ifdef notyet
	if (wdi_event_attach(soc)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: WDI event attach failed\n", __func__);
		goto fail2;
	}
#endif

	return (void *)soc;

#ifdef notyet
fail2:
	htt_soc_detach(soc->htt_handle);
#endif
fail1:
	qdf_mem_free(soc);
fail0:
	return NULL;
}

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

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"%s: total_link_descs: %u, link_desc_size: %d\n",
		__func__, total_link_descs, link_desc_size);
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

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"%s: total_mem_size: %d, num_link_desc_banks: %u\n",
		__func__, total_mem_size, num_link_desc_banks);

	for (i = 0; i < num_link_desc_banks; i++) {
		soc->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev, NULL,
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
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Link descriptor memory alloc failed\n",
				__func__);
			goto fail;
		}
	}

	if (last_bank_size) {
		/* Allocate last bank in case total memory required is not exact
		 * multiple of max_alloc_size
		 */
		soc->link_desc_banks[i].base_vaddr_unaligned =
			qdf_mem_alloc_consistent(soc->osdev, NULL,
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
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Link desc idle ring setup failed\n",
				__func__);
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
				qdf_mem_alloc_consistent(soc->osdev, NULL,
				soc->wbm_idle_scatter_buf_size,
				&(soc->wbm_idle_scatter_buf_base_paddr[i]));
			if (soc->wbm_idle_scatter_buf_base_vaddr[i] == NULL) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"%s:Scatter list memory alloc failed\n",
					__func__);
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
			(uint32_t)(scatter_buf_ptr - (unsigned long)(
			soc->wbm_idle_scatter_buf_base_vaddr[
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
			qdf_mem_free_consistent(soc->osdev, NULL,
				soc->wbm_idle_scatter_buf_size,
				soc->wbm_idle_scatter_buf_base_vaddr[i],
				soc->wbm_idle_scatter_buf_base_paddr[i], 0);
		}
	}

	for (i = 0; i < MAX_LINK_DESC_BANKS; i++) {
		if (soc->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, NULL,
				soc->link_desc_banks[i].size,
				soc->link_desc_banks[i].base_vaddr_unaligned,
				soc->link_desc_banks[i].base_paddr_unaligned,
				0);
		}
	}
	return QDF_STATUS_E_FAILURE;
}

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
			qdf_mem_free_consistent(soc->osdev, NULL,
				soc->wbm_idle_scatter_buf_size,
				soc->wbm_idle_scatter_buf_base_vaddr[i],
				soc->wbm_idle_scatter_buf_base_paddr[i], 0);
		}
	}

	for (i = 0; i < MAX_LINK_DESC_BANKS; i++) {
		if (soc->link_desc_banks[i].base_vaddr_unaligned) {
			qdf_mem_free_consistent(soc->osdev, NULL,
				soc->link_desc_banks[i].size,
				soc->link_desc_banks[i].base_vaddr_unaligned,
				soc->link_desc_banks[i].base_paddr_unaligned,
				0);
		}
	}
}

/* TODO: Following should be configurable */
#define WBM_RELEASE_RING_SIZE 64
#define TCL_DATA_RING_SIZE 512
#define TCL_CMD_RING_SIZE 32
#define TCL_STATUS_RING_SIZE 32
#define REO_DST_RING_SIZE 2048
#define REO_REINJECT_RING_SIZE 32
#define RX_RELEASE_RING_SIZE 256
#define REO_EXCEPTION_RING_SIZE 128
#define REO_CMD_RING_SIZE 32
#define REO_STATUS_RING_SIZE 32
#define RXDMA_BUF_RING_SIZE 8192
#define RXDMA_MONITOR_BUF_RING_SIZE 8192
#define RXDMA_MONITOR_DST_RING_SIZE 2048
#define RXDMA_MONITOR_STATUS_RING_SIZE 2048

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

	if (soc->cmn_init_done)
		return 0;

	soc->wlan_cfg_ctx = wlan_cfg_soc_attach();

	if (!soc->wlan_cfg_ctx) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: wlan_cfg_soc_attach failed\n", __func__);
		goto fail0;
	}

	if (dp_peer_find_attach(soc))
		goto fail0;

	if (dp_hw_link_desc_pool_setup(soc))
		goto fail1;

	/* Setup SRNG rings */
	/* Common rings */
	if (dp_srng_setup(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0, 0,
		WBM_RELEASE_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for wbm_desc_rel_ring\n",
			__func__);
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
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"%s: dp_srng_setup failed for tcl_data_ring[%d]\n",
					__func__, i);
				goto fail1;
			}
			if (dp_srng_setup(soc, &soc->tx_comp_ring[i],
				WBM2SW_RELEASE, i, 0, TCL_DATA_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"%s: dp_srng_setup failed for tx_comp_ring[%d]\n",
					__func__, i);
				goto fail1;
			}
		}
	} else {
		/* This will be incremented during per pdev ring setup */
		soc->num_tcl_data_rings = 0;
	}

	if (dp_tx_soc_attach(soc)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: dp_tx_soc_attach failed\n", __func__);
		goto fail1;
	}

	/* TCL command and status rings */
	if (dp_srng_setup(soc, &soc->tcl_cmd_ring, TCL_CMD, 0, 0,
		TCL_CMD_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for tcl_cmd_ring\n",
			__func__);
		goto fail1;
	}

	if (dp_srng_setup(soc, &soc->tcl_status_ring, TCL_STATUS, 0, 0,
		TCL_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for tcl_status_ring\n",
			__func__);
		goto fail1;
	}


	/* TBD: call dp_tx_init to setup Tx SW descriptors and MSDU extension
	 * descriptors
	 */

	/* Rx data rings */
	if (!wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		soc->num_reo_dest_rings =
			wlan_cfg_num_reo_dest_rings(soc->wlan_cfg_ctx);
		for (i = 0; i < soc->num_reo_dest_rings; i++) {
			if (dp_srng_setup(soc, &soc->reo_dest_ring[i], REO_DST,
				i, 0, REO_DST_RING_SIZE)) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					"%s: dp_srng_setup failed for reo_dest_ring[%d]\n",
					__func__, i);
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
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for reo_reinject_ring\n",
			__func__);
		goto fail1;
	}


	/* Rx release ring */
	if (dp_srng_setup(soc, &soc->rx_rel_ring, WBM2SW_RELEASE, 3, 0,
		RX_RELEASE_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for rx_rel_ring\n",
			__func__);
		goto fail1;
	}


	/* Rx exception ring */
	if (dp_srng_setup(soc, &soc->reo_exception_ring, REO_EXCEPTION, 0,
		MAX_REO_DEST_RINGS, REO_EXCEPTION_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for reo_exception_ring\n",
			__func__);
		goto fail1;
	}


	/* REO command and status rings */
	if (dp_srng_setup(soc, &soc->reo_cmd_ring, REO_CMD, 0, 0,
		REO_CMD_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for reo_cmd_ring\n",
			__func__);
		goto fail1;
	}

	if (dp_srng_setup(soc, &soc->reo_status_ring, REO_STATUS, 0, 0,
		REO_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for reo_status_ring\n",
			__func__);
		goto fail1;
	}


	/* Setup HW REO */
	hal_reo_setup(soc->hal_soc);

	soc->cmn_init_done = 1;
	return 0;
fail1:
	/*
	 * Cleanup will be done as part of soc_detach, which will
	 * be called on pdev attach failure
	 */
fail0:
	return QDF_STATUS_E_FAILURE;
}

static void dp_pdev_detach_wifi3(void *txrx_pdev, int force);

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
void *dp_pdev_attach_wifi3(void *txrx_soc, void *ctrl_pdev,
	HTC_HANDLE htc_handle, qdf_device_t qdf_osdev, int pdev_id)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct dp_pdev *pdev = qdf_mem_malloc(sizeof(*pdev));

	if (!pdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: DP PDEV memory allocation failed\n", __func__);
		goto fail0;
	}

	pdev->wlan_cfg_ctx = wlan_cfg_pdev_attach();

	if (!pdev->wlan_cfg_ctx) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: pdev cfg_attach failed\n", __func__);
		goto fail0;
	}

	pdev->soc = soc;
	pdev->osif_pdev = ctrl_pdev;
	pdev->pdev_id = pdev_id;
	soc->pdev_list[pdev_id] = pdev;

	TAILQ_INIT(&pdev->vdev_list);
	pdev->vdev_count = 0;

	if (dp_soc_cmn_setup(soc)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_soc_cmn_setup failed\n", __func__);
		goto fail0;
	}

	/* Setup per PDEV TCL rings if configured */
	if (wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		if (dp_srng_setup(soc, &soc->tcl_data_ring[pdev_id], TCL_DATA,
			pdev_id, pdev_id, TCL_DATA_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: dp_srng_setup failed for tcl_data_ring\n",
				__func__);
			goto fail0;
		}
		if (dp_srng_setup(soc, &soc->tx_comp_ring[pdev_id],
			WBM2SW_RELEASE, pdev_id, pdev_id, TCL_DATA_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: dp_srng_setup failed for tx_comp_ring\n",
				__func__);
			goto fail0;
		}
		soc->num_tcl_data_rings++;
	}

	/* Tx specific init */
	if (dp_tx_pdev_attach(pdev)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_tx_pdev_attach failed\n", __func__);
		goto fail0;
	}

	/* Setup per PDEV REO rings if configured */
	if (wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		if (dp_srng_setup(soc, &soc->reo_dest_ring[pdev_id], REO_DST,
			pdev_id, pdev_id, REO_DST_RING_SIZE)) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: dp_srng_setup failed for reo_dest_ring\n",
				__func__);
			goto fail0;
		}
		soc->num_reo_dest_rings++;

	}

	if (dp_srng_setup(soc, &pdev->rx_refill_buf_ring, RXDMA_BUF, 0, pdev_id,
		RXDMA_BUF_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: dp_srng_setup failed rx refill ring\n", __func__);
		goto fail0;
	}
#ifdef QCA_HOST2FW_RXBUF_RING
	if (dp_srng_setup(soc, &pdev->rx_mac_buf_ring, RXDMA_BUF, 1, pdev_id,
		RXDMA_BUF_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: dp_srng_setup failed rx mac ring\n", __func__);
		goto fail0;
	}
#endif
	/* TODO: RXDMA destination ring is not planned to be used currently.
	 * Setup the ring when required
	 */
	if (dp_srng_setup(soc, &pdev->rxdma_mon_buf_ring, RXDMA_MONITOR_BUF, 0,
		pdev_id, RXDMA_MONITOR_BUF_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for rxdma_mon_buf_ring\n",
			__func__);
		goto fail0;
	}

	if (dp_srng_setup(soc, &pdev->rxdma_mon_dst_ring, RXDMA_MONITOR_DST, 0,
		pdev_id, RXDMA_MONITOR_DST_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for rxdma_mon_dst_ring\n",
			__func__);
		goto fail0;
	}


	if (dp_srng_setup(soc, &pdev->rxdma_mon_status_ring,
		RXDMA_MONITOR_STATUS, 0, pdev_id,
		RXDMA_MONITOR_STATUS_RING_SIZE)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: dp_srng_setup failed for rxdma_mon_status_ring\n",
			__func__);
		goto fail0;
	}


	return (void *)pdev;

fail0:
	dp_pdev_detach_wifi3((void *)pdev, 0);
	return NULL;
}

/*
* dp_pdev_detach_wifi3() - detach txrx pdev
* @txrx_pdev: Datapath PDEV handle
* @force: Force detach
*
*/
static void dp_pdev_detach_wifi3(void *txrx_pdev, int force)
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

	/* Setup per PDEV REO rings if configured */
	if (wlan_cfg_per_pdev_rx_ring(soc->wlan_cfg_ctx)) {
		dp_srng_cleanup(soc, &soc->reo_dest_ring[pdev->pdev_id],
			REO_DST, pdev->pdev_id);
	}

	dp_srng_cleanup(soc, &pdev->rx_refill_buf_ring, RXDMA_BUF, 0);

#ifdef QCA_HOST2FW_RXBUF_RING
	dp_srng_cleanup(soc, &pdev->rx_mac_buf_ring, RXDMA_BUF, 1);
#endif
	dp_srng_cleanup(soc, &pdev->rxdma_mon_buf_ring, RXDMA_MONITOR_BUF, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_dst_ring, RXDMA_MONITOR_DST, 0);

	dp_srng_cleanup(soc, &pdev->rxdma_mon_status_ring,
		RXDMA_MONITOR_STATUS, 0);

	soc->pdev_list[pdev->pdev_id] = NULL;

	qdf_mem_free(pdev);
}

/*
 * dp_soc_detach_wifi3() - Detach txrx SOC
 * @txrx_soc: DP SOC handle
 *
 */
void dp_soc_detach_wifi3(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	struct dp_pdev *pdev = qdf_mem_malloc(sizeof(*pdev));
	int i;

	soc->cmn_init_done = 0;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		if (soc->pdev_list[i])
			dp_pdev_detach_wifi3((void *)pdev, 1);
	}

	dp_peer_find_detach(soc);

	/* TBD: Call Tx and Rx cleanup functions to free buffers and
	 * SW descriptors
	 */

	/* Free the ring memories */
	/* Common rings */
	dp_srng_cleanup(soc, &soc->wbm_desc_rel_ring, SW2WBM_RELEASE, 0);

	/* Tx data rings */
	if (!wlan_cfg_per_pdev_tx_ring(soc->wlan_cfg_ctx)) {
		dp_tx_soc_detach(soc);

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

	htt_soc_detach(soc->htt_handle);
}

/*
 * dp_soc_attach_target_wifi3() - SOC initialization in the target
 * @txrx_soc: Datapath SOC handle
 */
int dp_soc_attach_target_wifi3(void *txrx_soc)
{
	struct dp_soc *soc = (struct dp_soc *)txrx_soc;
	int i;

	htt_soc_attach_target(soc->htt_handle);

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		struct dp_pdev *pdev = soc->pdev_list[i];
		if (pdev) {
			htt_srng_setup(soc->htt_handle, i,
				pdev->rx_refill_buf_ring.hal_srng, RXDMA_BUF);
#ifdef QCA_HOST2FW_RXBUF_RING
			htt_srng_setup(soc->htt_handle, i,
				pdev->rx_mac_buf_ring.hal_srng, RXDMA_BUF);
#endif
#ifdef notyet /* FW doesn't handle monitor rings yet */
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_buf_ring.hal_srng,
				RXDMA_MONITOR_BUF);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_dst_ring.hal_srng,
				RXDMA_MONITOR_DST);
			htt_srng_setup(soc->htt_handle, i,
				pdev->rxdma_mon_status_ring.hal_srng,
				RXDMA_MONITOR_STATUS);
#endif
		}
	}
	return 0;
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
void *dp_vdev_attach_wifi3(void *txrx_pdev,
	uint8_t *vdev_mac_addr, uint8_t vdev_id, enum wlan_op_mode op_mode)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_vdev *vdev = qdf_mem_malloc(sizeof(*vdev));

	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: DP VDEV memory allocation failed\n", __func__);
		goto fail0;
	}

	vdev->pdev = pdev;
	vdev->vdev_id = vdev_id;
	vdev->opmode = op_mode;
	vdev->osdev = soc->osdev;

	vdev->osif_rx = NULL;
	vdev->osif_rx_mon = NULL;
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

	/* TODO: Initialize default HTT meta data that will be used in
	 * TCL descriptors for packets transmitted from this VDEV
	 */

	TAILQ_INIT(&vdev->peer_list);

	/* add this vdev into the pdev's list */
	TAILQ_INSERT_TAIL(&pdev->vdev_list, vdev, vdev_list_elem);
	pdev->vdev_count++;

	dp_tx_vdev_attach(vdev);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		"Created vdev %p (%02x:%02x:%02x:%02x:%02x:%02x)\n", vdev,
		vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
		vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
		vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);

	return (void *)vdev;

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
void dp_vdev_register_wifi3(void *vdev_handle, void *osif_vdev,
	struct ol_txrx_ops *txrx_ops)
{
	struct dp_vdev *vdev = (struct dp_vdev *)vdev_handle;
	vdev->osif_vdev = osif_vdev;
	vdev->osif_rx = txrx_ops->rx.rx;
	vdev->osif_rx_mon = txrx_ops->rx.mon;
#ifdef notyet
#if ATH_SUPPORT_WAPI
	vdev->osif_check_wai = txrx_ops->rx.wai_check;
#endif
#if UMAC_SUPPORT_PROXY_ARP
	vdev->osif_proxy_arp = txrx_ops->proxy_arp;
#endif
#endif
#ifdef notyet
	/* TODO: Enable the following once Tx code is integrated */
	txrx_ops->tx.tx = dp_tx_send;
#endif
}

/*
 * dp_vdev_detach_wifi3() - Detach txrx vdev
 * @txrx_vdev:		Datapath VDEV handle
 * @callback:		Callback OL_IF on completion of detach
 * @cb_context:	Callback context
 *
 */
void dp_vdev_detach_wifi3(void *vdev_handle,
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
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
			"%s: not deleting vdev object %p (%02x:%02x:%02x:%02x:%02x:%02x)"
			"until deletion finishes for all its peers\n",
			__func__, vdev,
			vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
			vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
			vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);
		/* indicate that the vdev needs to be deleted */
		vdev->delete.pending = 1;
		vdev->delete.callback = callback;
		vdev->delete.context = cb_context;
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
		return;
	}
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	dp_tx_vdev_detach(vdev);
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"%s: deleting vdev object %p (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		__func__, vdev,
		vdev->mac_addr.raw[0], vdev->mac_addr.raw[1],
		vdev->mac_addr.raw[2], vdev->mac_addr.raw[3],
		vdev->mac_addr.raw[4], vdev->mac_addr.raw[5]);

	qdf_mem_free(vdev);

	if (callback)
		callback(cb_context);
}

/*
 * dp_peer_attach_wifi3() - attach txrx peer
 * @txrx_vdev: Datapath VDEV handle
 * @peer_mac_addr: Peer MAC address
 *
 * Return: DP peeer handle on success, NULL on failure
 */
void *dp_peer_attach_wifi3(void *vdev_handle, uint8_t *peer_mac_addr)
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

	/* store provided params */
	peer->vdev = vdev;
	qdf_mem_copy(
		&peer->mac_addr.raw[0], peer_mac_addr, OL_TXRX_MAC_ADDR_LEN);

	/* TODO: See of rx_opt_proc is really required */
	peer->rx_opt_proc = soc->rx_opt_proc;

	dp_peer_rx_init(pdev, peer);

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

	if (soc->ol_ops->peer_set_default_routing) {
		/* TODO: Check on the destination ring number to be passed
		 * to FW
		 */
		soc->ol_ops->peer_set_default_routing(soc->osif_soc,
			peer->mac_addr.raw, peer->vdev->vdev_id, 0, 1);
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"vdev %p created peer %p (%02x:%02x:%02x:%02x:%02x:%02x)\n",
		vdev, peer,
		peer->mac_addr.raw[0], peer->mac_addr.raw[1],
		peer->mac_addr.raw[2], peer->mac_addr.raw[3],
		peer->mac_addr.raw[4], peer->mac_addr.raw[5]);
	/*
	 * For every peer MAp message search and set if bss_peer
	 */
	if (memcmp(peer->mac_addr.raw, vdev->mac_addr.raw, 6) == 0) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			"vdev bss_peer!!!!\n");
		peer->bss_peer = 1;
		vdev->vap_bss_peer = peer;
	}

	return (void *)peer;
}

/*
 * dp_peer_authorize() - authorize txrx peer
 * @peer_handle:		Datapath peer handle
 * @authorize
 *
 */
void dp_peer_authorize(void *peer_handle, uint32_t authorize)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;
	struct dp_soc *soc;

	if (peer != NULL) {
		soc = peer->vdev->pdev->soc;

		qdf_spin_lock_bh(&soc->peer_ref_mutex);
		peer->authorize = authorize ? 1 : 0;
#ifdef notyet /* ATH_BAND_STEERING */
		peer->peer_bs_inact_flag = 0;
		peer->peer_bs_inact = soc->pdev_bs_inact_reload;
#endif
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
	struct dp_soc *soc = vdev->pdev->soc;
	struct dp_peer *tmppeer;
	int found = 0;
	uint16_t peer_id;

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
	if (qdf_atomic_dec_and_test(&peer->ref_cnt)) {
		peer_id = peer->peer_ids[0];

		/*
		 * Make sure that the reference to the peer in
		 * peer object map is removed
		 */
		if (peer_id != HTT_INVALID_PEER)
			soc->peer_id_to_obj_map[peer_id] = NULL;

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
			"Deleting peer %p (%02x:%02x:%02x:%02x:%02x:%02x)\n",
			peer, peer->mac_addr.raw[0], peer->mac_addr.raw[1],
			peer->mac_addr.raw[2], peer->mac_addr.raw[3],
			peer->mac_addr.raw[4], peer->mac_addr.raw[5]);

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
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_WARN,
				"WARN peer %p not found in vdev (%p)->peer_list:%p\n",
				peer, vdev, &peer->vdev->peer_list);
		}

		/* cleanup the Rx reorder queues for this peer */
		dp_peer_rx_cleanup(vdev, peer);

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

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_INFO_HIGH,
					"%s: deleting vdev object %p "
					"(%02x:%02x:%02x:%02x:%02x:%02x)"
					" - its last peer is done\n",
					__func__, vdev,
					vdev->mac_addr.raw[0],
					vdev->mac_addr.raw[1],
					vdev->mac_addr.raw[2],
					vdev->mac_addr.raw[3],
					vdev->mac_addr.raw[4],
					vdev->mac_addr.raw[5]);
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
		qdf_mem_free(peer);
#endif

#ifdef notyet /* See why this should be done in DP layer */
		qdf_atomic_inc(&soc->peer_count);
#endif
	} else {
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
	}
}

/*
 * dp_peer_detach_wifi3() – Detach txrx peer
 * @peer_handle:		Datapath peer handle
 *
 */
void dp_peer_detach_wifi3(void *peer_handle)
{
	struct dp_peer *peer = (struct dp_peer *)peer_handle;

	/* redirect the peer's rx delivery function to point to a
	 * discard func
	 */
	peer->rx_opt_proc = dp_rx_discard;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_HIGH,
		"%s:peer %p (%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, peer,
		  peer->mac_addr.raw[0], peer->mac_addr.raw[1],
		  peer->mac_addr.raw[2], peer->mac_addr.raw[3],
		  peer->mac_addr.raw[4], peer->mac_addr.raw[5]);

	/*
	 * Remove the reference added during peer_attach.
	 * The peer will still be left allocated until the
	 * PEER_UNMAP message arrives to remove the other
	 * reference, added by the PEER_MAP message.
	 */
	dp_peer_unref_delete(peer_handle);
}

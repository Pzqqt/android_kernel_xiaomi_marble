/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#include <linux/ipa_wdi3.h>
#include <qdf_types.h>
#include <qdf_lock.h>
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_types.h"
#include "dp_htt.h"
#include "dp_tx.h"
#include "dp_ipa.h"

/* Hard coded config parameters until dp_ops_cfg.cfg_attach implemented */
#define CFG_IPA_UC_TX_BUF_SIZE_DEFAULT            (2048)

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
	uint32_t ring_base_align = 8;
	/*
	 * Uncomment when dp_ops_cfg.cfg_attach is implemented
	 * unsigned int uc_tx_buf_sz =
	 *		dp_cfg_ipa_uc_tx_buf_size(pdev->osif_pdev);
	 */
	unsigned int uc_tx_buf_sz = CFG_IPA_UC_TX_BUF_SIZE_DEFAULT;
	unsigned int alloc_size = uc_tx_buf_sz + ring_base_align - 1;

	for (idx = 0; idx < soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt; idx++) {
		if (soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[idx]) {
			qdf_mem_free_consistent(
				soc->osdev, soc->osdev->dev,
				alloc_size,
				soc->ipa_uc_tx_rsc.
					tx_buf_pool_vaddr_unaligned[idx],
				soc->ipa_uc_tx_rsc.
					tx_buf_pool_paddr_unaligned[idx],
				0);

			soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[idx] =
							(void *)NULL;
			soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned[idx] =
							(qdf_dma_addr_t)NULL;
		}
	}

	qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned);
	soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned = NULL;
	qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned);
	soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned = NULL;
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
}

int dp_ipa_uc_detach(struct dp_soc *soc, struct dp_pdev *pdev)
{
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
	void *buffer_vaddr_unaligned;
	void *buffer_vaddr;
	qdf_dma_addr_t buffer_paddr_unaligned;
	qdf_dma_addr_t buffer_paddr;
	struct hal_srng *wbm_srng =
			soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	struct hal_srng_params srng_params;
	uint32_t paddr_lo;
	uint32_t paddr_hi;
	void *ring_entry;
	int num_entries;
	int retval = QDF_STATUS_SUCCESS;
	/*
	 * Uncomment when dp_ops_cfg.cfg_attach is implemented
	 * unsigned int uc_tx_buf_sz =
	 *		dp_cfg_ipa_uc_tx_buf_size(pdev->osif_pdev);
	 */
	unsigned int uc_tx_buf_sz = CFG_IPA_UC_TX_BUF_SIZE_DEFAULT;
	unsigned int alloc_size = uc_tx_buf_sz + ring_base_align - 1;

	hal_get_srng_params(soc->hal_soc, (void *)wbm_srng, &srng_params);
	num_entries = srng_params.num_entries;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "%s: requested %d buffers to be posted to wbm ring",
		   __func__, num_entries);

	soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned =
		qdf_mem_malloc(num_entries *
		sizeof(*soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned));
	if (!soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: IPA WBM Ring Tx buf pool vaddr alloc fail",
			  __func__);
		return -ENOMEM;
	}

	soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned =
		qdf_mem_malloc(num_entries *
		sizeof(*soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned));
	if (!soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: IPA WBM Ring Tx buf pool paddr alloc fail",
			  __func__);
		qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned);
		return -ENOMEM;
	}

	hal_srng_access_start(soc->hal_soc, (void *)wbm_srng);

	/*
	 * Allocate Tx buffers as many as possible
	 * Populate Tx buffers into WBM2IPA ring
	 * This initial buffer population will simulate H/W as source ring,
	 * and update HP
	 */
	for (tx_buffer_count = 0;
		tx_buffer_count < num_entries - 1; tx_buffer_count++) {
		buffer_vaddr_unaligned = qdf_mem_alloc_consistent(soc->osdev,
			soc->osdev->dev, alloc_size, &buffer_paddr_unaligned);
		if (!buffer_vaddr_unaligned)
			break;

		ring_entry = hal_srng_dst_get_next_hp(soc->hal_soc,
				(void *)wbm_srng);
		if (!ring_entry) {
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
				  "%s: Failed to get WBM ring entry\n",
				  __func__);
			qdf_mem_free_consistent(soc->osdev, soc->osdev->dev,
				alloc_size, buffer_vaddr_unaligned,
				buffer_paddr_unaligned, 0);
			break;
		}

		buffer_vaddr = (void *)qdf_align((unsigned long)
			buffer_vaddr_unaligned, ring_base_align);
		buffer_paddr = buffer_paddr_unaligned +
			((unsigned long)(buffer_vaddr) -
			 (unsigned long)buffer_vaddr_unaligned);

		paddr_lo = ((u64)buffer_paddr & 0x00000000ffffffff);
		paddr_hi = ((u64)buffer_paddr & 0x0000001f00000000) >> 32;
		HAL_WBM_PADDR_LO_SET(ring_entry, paddr_lo);
		HAL_WBM_PADDR_HI_SET(ring_entry, paddr_hi);

		soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned[tx_buffer_count]
			= buffer_vaddr_unaligned;
		soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned[tx_buffer_count]
			= buffer_paddr_unaligned;
	}

	hal_srng_access_end(soc->hal_soc, wbm_srng);

	soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt = tx_buffer_count;

	if (tx_buffer_count) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			  "%s: IPA WDI TX buffer: %d allocated\n",
			  __func__, tx_buffer_count);
	} else {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: No IPA WDI TX buffer allocated\n",
			  __func__);
		qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned);
		soc->ipa_uc_tx_rsc.tx_buf_pool_vaddr_unaligned = NULL;
		qdf_mem_free(soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned);
		soc->ipa_uc_tx_rsc.tx_buf_pool_paddr_unaligned = NULL;
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

	/* TX resource attach */
	error = dp_tx_ipa_uc_attach(soc, pdev);
	if (error) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: DP IPA UC TX attach fail code %d\n",
			  __func__, error);
		return error;
	}

	/* RX resource attach */
	error = dp_rx_ipa_uc_attach(soc, pdev);
	if (error) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "%s: DP IPA UC RX attach fail code %d\n",
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

	/* IPA TCL_DATA Ring - HAL_SRNG_SW2TCL3 */
	hal_srng = soc->tcl_data_ring[IPA_TCL_DATA_RING_IDX].hal_srng;
	hal_get_srng_params(hal_soc, (void *)hal_srng, &srng_params);

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

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"%s: addr_offset=%x, dev_base_paddr=%x, ipa_tcl_hp_paddr=%x",
		__func__, (unsigned int)addr_offset,
		(unsigned int)dev_base_paddr,
		(unsigned int)(soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr));

	/* IPA TX COMP Ring - HAL_SRNG_WBM2SW2_RELEASE */
	hal_srng = soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	hal_get_srng_params(hal_soc, (void *)hal_srng, &srng_params);

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

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"%s: addr_offset=%x, dev_base_paddr=%x, ipa_wbm_tp_paddr=%x",
		__func__, (unsigned int)addr_offset,
		(unsigned int)dev_base_paddr,
		(unsigned int)(soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr));

	/* IPA REO_DEST Ring - HAL_SRNG_REO2SW4 */
	hal_srng = soc->reo_dest_ring[IPA_REO_DEST_RING_IDX].hal_srng;
	hal_get_srng_params(hal_soc, (void *)hal_srng, &srng_params);

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

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"%s: addr_offset=%x, dev_base_paddr=%x, ipa_reo_tp_paddr=%x",
		__func__, (unsigned int)addr_offset,
		(unsigned int)dev_base_paddr,
		(unsigned int)(soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr));

	hal_srng = pdev->rx_refill_buf_ring2.hal_srng;
	hal_get_srng_params(hal_soc, (void *)hal_srng, &srng_params);
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_paddr =
		srng_params.ring_base_paddr;
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_vaddr =
		srng_params.ring_base_vaddr;
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_size =
		(srng_params.num_entries * srng_params.entry_size) << 2;
	hp_addr = hal_srng_get_hp_addr(hal_soc, (void *)hal_srng);
	soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr = hp_addr;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		"%s: ipa_rx_refill_buf_hp_paddr=%x", __func__,
		(unsigned int)(soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr));

	return 0;
}

/**
 * dp_ipa_uc_get_resource() - Client request resource information
 * @ppdev - handle to the device instance
 *
 *  IPA client will request IPA UC related resource information
 *  Resource information will be distributed to IPA module
 *  All of the required resources should be pre-allocated
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_get_resource(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;

	ipa_res->tx_ring_base_paddr =
		soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_paddr;
	ipa_res->tx_ring_size =
		soc->ipa_uc_tx_rsc.ipa_tcl_ring_size;
	ipa_res->tx_num_alloc_buffer =
		(uint32_t)soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt;

	ipa_res->tx_comp_ring_base_paddr =
		soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_paddr;
	ipa_res->tx_comp_ring_size =
		soc->ipa_uc_tx_rsc.ipa_wbm_ring_size;

	ipa_res->rx_rdy_ring_base_paddr =
		soc->ipa_uc_rx_rsc.ipa_reo_ring_base_paddr;
	ipa_res->rx_rdy_ring_size =
		soc->ipa_uc_rx_rsc.ipa_reo_ring_size;

	ipa_res->rx_refill_ring_base_paddr =
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_paddr;
	ipa_res->rx_refill_ring_size =
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_size;

	if ((0 == ipa_res->tx_comp_ring_base_paddr) ||
			(0 == ipa_res->rx_rdy_ring_base_paddr))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_set_doorbell_paddr () - Set doorbell register physical address to SRNG
 * @ppdev - handle to the device instance
 *
 * Set TX_COMP_DOORBELL register physical address to WBM Head_Ptr_MemAddr_LSB
 * Set RX_READ_DOORBELL register physical address to REO Head_Ptr_MemAddr_LSB
 *
 * Return: none
 */
QDF_STATUS dp_ipa_set_doorbell_paddr(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;
	struct hal_srng *wbm_srng =
			soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].hal_srng;
	struct hal_srng *reo_srng =
			soc->reo_dest_ring[IPA_REO_DEST_RING_IDX].hal_srng;

	hal_srng_dst_set_hp_paddr(wbm_srng, ipa_res->tx_comp_doorbell_paddr);
	hal_srng_dst_init_hp(wbm_srng, ipa_res->tx_comp_doorbell_vaddr);
	hal_srng_dst_set_hp_paddr(reo_srng, ipa_res->rx_ready_doorbell_paddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_op_response() - Handle OP command response from firmware
 * @ppdev - handle to the device instance
 * @op_msg: op response message from firmware
 *
 * Return: none
 */
QDF_STATUS dp_ipa_op_response(struct cdp_pdev *ppdev, uint8_t *op_msg)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;

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

/**
 * dp_ipa_register_op_cb() - Register OP handler function
 * @ppdev - handle to the device instance
 * @op_cb: handler function pointer
 *
 * Return: none
 */
QDF_STATUS dp_ipa_register_op_cb(struct cdp_pdev *ppdev,
				 ipa_uc_op_cb_type op_cb,
				 void *usr_ctxt)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;

	pdev->ipa_uc_op_cb = op_cb;
	pdev->usr_ctxt = usr_ctxt;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_get_stat() - Get firmware wdi status
 * @ppdev - handle to the device instance
 *
 * Return: none
 */
QDF_STATUS dp_ipa_get_stat(struct cdp_pdev *ppdev)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_send_ipa_data_frame() - send IPA data frame
 * @vdev: vdev
 * @skb: skb
 *
 * Return: skb/ NULL is for success
 */
qdf_nbuf_t dp_tx_send_ipa_data_frame(struct cdp_vdev *vdev, qdf_nbuf_t skb)
{
	qdf_nbuf_t ret;

	/* Terminate the (single-element) list of tx frames */
	qdf_nbuf_set_next(skb, NULL);
	ret = dp_tx_send((struct dp_vdev_t *)vdev, skb);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to tx", __func__);
		return ret;
	}

	return NULL;
}

/**
 * dp_ipa_enable_autonomy() – Enable autonomy RX path
 * @pdev - handle to the device instance
 *
 * Set all RX packet route to IPA REO ring
 * Program Destination_Ring_Ctrl_IX_0 REO register to point IPA REO ring
 * Return: none
 */
QDF_STATUS dp_ipa_enable_autonomy(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	uint32_t remap_val;

	/* Call HAL API to remap REO rings to REO2IPA ring */
	remap_val = HAL_REO_REMAP_VAL(REO_REMAP_TCL, REO_REMAP_TCL) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW1, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW2, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW3, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW4, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_RELEASE, REO_REMAP_RELEASE) |
		    HAL_REO_REMAP_VAL(REO_REMAP_FW, REO_REMAP_FW) |
		    HAL_REO_REMAP_VAL(REO_REMAP_UNUSED, REO_REMAP_FW);
	hal_reo_remap_IX0(soc->hal_soc, remap_val);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_disable_autonomy() – Disable autonomy RX path
 * @ppdev - handle to the device instance
 *
 * Disable RX packet routing to IPA REO
 * Program Destination_Ring_Ctrl_IX_0 REO register to disable
 * Return: none
 */
QDF_STATUS dp_ipa_disable_autonomy(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	uint32_t remap_val;

	/* Call HAL API to remap REO rings to REO2IPA ring */
	remap_val = HAL_REO_REMAP_VAL(REO_REMAP_TCL, REO_REMAP_TCL) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW1, REO_REMAP_SW1) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW2, REO_REMAP_SW2) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW3, REO_REMAP_SW3) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW4, REO_REMAP_SW2) |
		    HAL_REO_REMAP_VAL(REO_REMAP_RELEASE, REO_REMAP_RELEASE) |
		    HAL_REO_REMAP_VAL(REO_REMAP_FW, REO_REMAP_FW) |
		    HAL_REO_REMAP_VAL(REO_REMAP_UNUSED, REO_REMAP_FW);
	hal_reo_remap_IX0(soc->hal_soc, remap_val);

	return QDF_STATUS_SUCCESS;
}

/* This should be configurable per H/W configuration enable status */
#define L3_HEADER_PADDING	2

/**
 * dp_ipa_setup() - Setup and connect IPA pipes
 * @ppdev - handle to the device instance
 * @ipa_i2w_cb: IPA to WLAN callback
 * @ipa_w2i_cb: WLAN to IPA callback
 * @ipa_wdi_meter_notifier_cb: IPA WDI metering callback
 * @ipa_desc_size: IPA descriptor size
 * @ipa_priv: handle to the HTT instance
 * @is_rm_enabled: Is IPA RM enabled or not
 * @tx_pipe_handle: pointer to Tx pipe handle
 * @rx_pipe_handle: pointer to Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_setup(struct cdp_pdev *ppdev, void *ipa_i2w_cb,
			void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
			uint32_t ipa_desc_size, void *ipa_priv,
			bool is_rm_enabled, uint32_t *tx_pipe_handle,
			uint32_t *rx_pipe_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;
	struct ipa_wdi3_setup_info tx;
	struct ipa_wdi3_setup_info rx;
	struct ipa_wdi3_conn_in_params pipe_in;
	struct ipa_wdi3_conn_out_params pipe_out;
	struct tcl_data_cmd *tcl_desc_ptr;
	uint8_t *desc_addr;
	uint32_t desc_size;
	int ret;

	qdf_mem_zero(&tx, sizeof(struct ipa_wdi3_setup_info));
	qdf_mem_zero(&rx, sizeof(struct ipa_wdi3_setup_info));
	qdf_mem_zero(&pipe_in, sizeof(struct ipa_wdi3_conn_in_params));
	qdf_mem_zero(&pipe_out, sizeof(struct ipa_wdi3_conn_out_params));

	/* TX PIPE */
	/**
	 * Transfer Ring: WBM Ring
	 * Transfer Ring Doorbell PA: WBM Tail Pointer Address
	 * Event Ring: TCL ring
	 * Event Ring Doorbell PA: TCL Head Pointer Address
	 */
	tx.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	tx.ipa_ep_cfg.hdr.hdr_len = DP_IPA_UC_WLAN_TX_HDR_LEN;
	tx.ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 0;
	tx.ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
	tx.ipa_ep_cfg.hdr.hdr_additional_const_len = 0;
	tx.ipa_ep_cfg.mode.mode = IPA_BASIC;
	tx.ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
	tx.client = IPA_CLIENT_WLAN1_CONS;
	tx.transfer_ring_base_pa = ipa_res->tx_comp_ring_base_paddr;
	tx.transfer_ring_size = ipa_res->tx_comp_ring_size;
	tx.transfer_ring_doorbell_pa = /* WBM Tail Pointer Address */
		soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr;
	tx.event_ring_base_pa = ipa_res->tx_ring_base_paddr;
	tx.event_ring_size = ipa_res->tx_ring_size;
	tx.event_ring_doorbell_pa = /* TCL Head Pointer Address */
		soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr;
	tx.num_pkt_buffers = ipa_res->tx_num_alloc_buffer;
	tx.pkt_offset = 0;

	/* Preprogram TCL descriptor */
	desc_addr = (uint8_t *)(tx.desc_format_template);
	desc_size = sizeof(struct tcl_data_cmd);
	HAL_TX_DESC_SET_TLV_HDR(desc_addr, HAL_TX_TCL_DATA_TAG, desc_size);
	tcl_desc_ptr = (struct tcl_data_cmd *)(tx.desc_format_template+1);
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
	rx.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	rx.ipa_ep_cfg.hdr.hdr_len = DP_IPA_UC_WLAN_RX_HDR_LEN;
	rx.ipa_ep_cfg.hdr.hdr_ofst_metadata_valid = 0;
	rx.ipa_ep_cfg.hdr.hdr_metadata_reg_valid = 1;
	rx.ipa_ep_cfg.mode.mode = IPA_BASIC;
	rx.client = IPA_CLIENT_WLAN1_PROD;
	rx.transfer_ring_base_pa = ipa_res->rx_rdy_ring_base_paddr;
	rx.transfer_ring_size = ipa_res->rx_rdy_ring_size;
	rx.transfer_ring_doorbell_pa = /* REO Tail Pointer Address */
		soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr;
	rx.event_ring_base_pa = ipa_res->rx_refill_ring_base_paddr;
	rx.event_ring_size = ipa_res->rx_refill_ring_size;
	rx.event_ring_doorbell_pa = /* FW Head Pointer Address */
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr;
	rx.pkt_offset = RX_PKT_TLVS_LEN + L3_HEADER_PADDING;

	pipe_in.notify = ipa_w2i_cb;
	pipe_in.priv = ipa_priv;
	memcpy(&pipe_in.tx, &tx, sizeof(struct ipa_wdi3_setup_info));
	memcpy(&pipe_in.rx, &rx, sizeof(struct ipa_wdi3_setup_info));

	/* Connect WDI IPA PIPE */
	ret = ipa_wdi3_conn_pipes(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi3_conn_pipes: IPA pipe setup failed: ret=%d",
			  __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* IPA uC Doorbell registers */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Tx DB PA=0x%x, Rx DB PA=0x%x",
		  __func__,
		  (unsigned int)pipe_out.tx_uc_db_pa,
		  (unsigned int)pipe_out.rx_uc_db_pa);

	ipa_res->tx_comp_doorbell_paddr = pipe_out.tx_uc_db_pa;
	ipa_res->tx_comp_doorbell_vaddr = pipe_out.tx_uc_db_va;
	ipa_res->rx_ready_doorbell_paddr = pipe_out.rx_uc_db_pa;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Tx: %s=%pK, %s=%d, %s=%pK, %s=%pK, %s=%d, %s=%pK, %s=%d, %s=%pK",
		  __func__,
		  "transfer_ring_base_pa",
		  (void *)pipe_in.tx.transfer_ring_base_pa,
		  "transfer_ring_size",
		  pipe_in.tx.transfer_ring_size,
		  "transfer_ring_doorbell_pa",
		  (void *)pipe_in.tx.transfer_ring_doorbell_pa,
		  "event_ring_base_pa",
		  (void *)pipe_in.tx.event_ring_base_pa,
		  "event_ring_size",
		  pipe_in.tx.event_ring_size,
		  "event_ring_doorbell_pa",
		  (void *)pipe_in.tx.event_ring_doorbell_pa,
		  "num_pkt_buffers",
		  pipe_in.tx.num_pkt_buffers,
		  "tx_comp_doorbell_paddr",
		  (void *)ipa_res->tx_comp_doorbell_paddr);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Rx: %s=%pK, %s=%d, %s=%pK, %s=%pK, %s=%d, %s=%pK, %s=%d, %s=%pK",
		  __func__,
		  "transfer_ring_base_pa",
		  (void *)pipe_in.rx.transfer_ring_base_pa,
		  "transfer_ring_size",
		  pipe_in.rx.transfer_ring_size,
		  "transfer_ring_doorbell_pa",
		  (void *)pipe_in.rx.transfer_ring_doorbell_pa,
		  "event_ring_base_pa",
		  (void *)pipe_in.rx.event_ring_base_pa,
		  "event_ring_size",
		  pipe_in.rx.event_ring_size,
		  "event_ring_doorbell_pa",
		  (void *)pipe_in.rx.event_ring_doorbell_pa,
		  "num_pkt_buffers",
		  pipe_in.rx.num_pkt_buffers,
		  "tx_comp_doorbell_paddr",
		  (void *)ipa_res->rx_ready_doorbell_paddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_cleanup() - Disconnect IPA pipes
 * @tx_pipe_handle: Tx pipe handle
 * @rx_pipe_handle: Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_cleanup(uint32_t tx_pipe_handle, uint32_t rx_pipe_handle)
{
	int ret;

	ret = ipa_wdi3_disconn_pipes();
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "%s: ipa_wdi3_disconn_pipes: IPA pipe cleanup failed: ret=%d",
		    __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

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
			      enum ipa_client_type prod_client,
			      enum ipa_client_type cons_client,
			      uint8_t session_id, bool is_ipv6_enabled)
{
	struct ipa_wdi3_reg_intf_in_params in;
	struct ipa_wdi3_hdr_info hdr_info;
	struct dp_ipa_uc_tx_hdr uc_tx_hdr;
	int ret = -EINVAL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Add Partial hdr: %s, %pM",
		  __func__, ifname, mac_addr);

	qdf_mem_zero(&hdr_info, sizeof(struct ipa_wdi3_hdr_info));
	qdf_ether_addr_copy(uc_tx_hdr.eth.h_source, mac_addr);

	/* IPV4 header */
	uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IP);

	hdr_info.hdr = (uint8_t *)&uc_tx_hdr;
	hdr_info.hdr_len = DP_IPA_UC_WLAN_TX_HDR_LEN;
	hdr_info.hdr_type = IPA_HDR_L2_ETHERNET_II;
	hdr_info.dst_mac_addr_offset = DP_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	in.netdev_name = ifname;
	memcpy(&(in.hdr_info[0]), &hdr_info, sizeof(struct ipa_wdi3_hdr_info));
	in.is_meta_data_valid = 1;
	in.meta_data = htonl(session_id << 16);
	in.meta_data_mask = htonl(0x00FF0000);

	/* IPV6 header */
	if (is_ipv6_enabled) {
		uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IPV6);
		memcpy(&(in.hdr_info[1]), &hdr_info,
				sizeof(struct ipa_wdi3_hdr_info));
	}

	ret = ipa_wdi3_reg_intf(&in);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "%s: ipa_wdi3_reg_intf: register IPA interface falied: ret=%d",
		    __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
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

	ret = ipa_wdi3_dereg_intf(ifname);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi3_dereg_intf: IPA pipe deregistration failed: ret=%d",
			  __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

 /**
 * dp_ipa_uc_enable_pipes() - Enable and resume traffic on Tx/Rx pipes
 * @ppdev - handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_enable_pipes(struct cdp_pdev *ppdev)
{
	QDF_STATUS result;

	result = ipa_wdi3_enable_pipes();
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Enable WDI PIPE fail, code %d",
			  __func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_uc_disable_pipes() – Suspend traffic and disable Tx/Rx pipes
 * @ppdev - handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_disable_pipes(struct cdp_pdev *ppdev)
{
	QDF_STATUS result;

	result = ipa_wdi3_disable_pipes();
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Disable WDI PIPE fail, code %d",
			  __func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
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
	struct ipa_wdi3_perf_profile profile;
	QDF_STATUS result;

	profile.client = client;
	profile.max_supported_bw_mbps = max_supported_bw_mbps;

	result = ipa_wdi3_set_perf_profile(&profile);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi3_set_perf_profile fail, code %d",
			  __func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

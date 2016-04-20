/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */
 /**
 * @file cdp_txrx_ipa.h
 * @brief Define the host data path IP Acceleraor API functions
 */
#ifndef _CDP_TXRX_IPA_H_
#define _CDP_TXRX_IPA_H_


/**
 * ol_txrx_ipa_resources - Resources needed for IPA
 */
struct ol_txrx_ipa_resources {
	qdf_dma_addr_t ce_sr_base_paddr;
	uint32_t ce_sr_ring_size;
	qdf_dma_addr_t ce_reg_paddr;

	qdf_dma_addr_t tx_comp_ring_base_paddr;
	uint32_t tx_comp_ring_size;
	uint32_t tx_num_alloc_buffer;

	qdf_dma_addr_t rx_rdy_ring_base_paddr;
	uint32_t rx_rdy_ring_size;
	qdf_dma_addr_t rx_proc_done_idx_paddr;
	void *rx_proc_done_idx_vaddr;

	qdf_dma_addr_t rx2_rdy_ring_base_paddr;
	uint32_t rx2_rdy_ring_size;
	qdf_dma_addr_t rx2_proc_done_idx_paddr;
	void *rx2_proc_done_idx_vaddr;
};

#ifdef IPA_OFFLOAD

void
ol_txrx_ipa_uc_get_resource(ol_txrx_pdev_handle pdev,
		 struct ol_txrx_ipa_resources *ipa_res);

void
ol_txrx_ipa_uc_set_doorbell_paddr(ol_txrx_pdev_handle pdev,
		 qdf_dma_addr_t ipa_tx_uc_doorbell_paddr,
		 qdf_dma_addr_t ipa_rx_uc_doorbell_paddr);

void
ol_txrx_ipa_uc_set_active(ol_txrx_pdev_handle pdev,
		 bool uc_active, bool is_tx);

void ol_txrx_ipa_uc_op_response(ol_txrx_pdev_handle pdev, uint8_t *op_msg);

void ol_txrx_ipa_uc_register_op_cb(ol_txrx_pdev_handle pdev,
		 void (*ipa_uc_op_cb_type)(uint8_t *op_msg,
		 void *osif_ctxt),
		 void *osif_dev);

void ol_txrx_ipa_uc_get_stat(ol_txrx_pdev_handle pdev);

qdf_nbuf_t ol_tx_send_ipa_data_frame(void *vdev, qdf_nbuf_t skb);
#else

static inline void
ol_txrx_ipa_uc_get_resource(ol_txrx_pdev_handle pdev,
		 struct ol_txrx_ipa_resources *ipa_res)
{
	return;
}

static inline void
ol_txrx_ipa_uc_set_doorbell_paddr(ol_txrx_pdev_handle pdev,
				  qdf_dma_addr_t ipa_tx_uc_doorbell_paddr,
				  qdf_dma_addr_t ipa_rx_uc_doorbell_paddr)
{
	return;
}

static inline void
ol_txrx_ipa_uc_set_active(ol_txrx_pdev_handle pdev,
	bool uc_active, bool is_tx)
{
	return;
}

static inline void
ol_txrx_ipa_uc_op_response(ol_txrx_pdev_handle pdev, uint8_t *op_msg)
{
	return;
}

static inline void
ol_txrx_ipa_uc_register_op_cb(ol_txrx_pdev_handle pdev,
				   void (*ipa_uc_op_cb_type)(uint8_t *op_msg,
							     void *osif_ctxt),
				   void *osif_dev)
{
	return;
}

static inline void ol_txrx_ipa_uc_get_stat(ol_txrx_pdev_handle pdev)
{
	return;
}
#endif /* IPA_OFFLOAD */

#endif /* _CDP_TXRX_IPA_H_ */


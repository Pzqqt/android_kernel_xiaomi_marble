/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _HAL_LI_API_H_
#define _HAL_LI_API_H_

#include "hal_li_hw_headers.h"
#include "hal_api.h"

/**
 * hal_set_link_desc_addr_li - Setup link descriptor in a buffer_addr_info
 * HW structure
 *
 * @desc: Descriptor entry (from WBM_IDLE_LINK ring)
 * @cookie: SW cookie for the buffer/descriptor
 * @link_desc_paddr: Physical address of link descriptor entry
 * @bm_id: idle link BM id
 */
void hal_set_link_desc_addr_li(void *desc, uint32_t cookie,
			       qdf_dma_addr_t link_desc_paddr,
			       uint8_t bm_id);

/**
 * hal_tx_init_data_ring_li() - Initialize all the TCL Descriptors in SRNG
 * @hal_soc_hdl: Handle to HAL SoC structure
 * @hal_srng: Handle to HAL SRNG structure
 *
 * Return: none
 */
void hal_tx_init_data_ring_li(hal_soc_handle_t hal_soc_hdl,
			      hal_ring_handle_t hal_ring_hdl);

/**
 * hal_hw_txrx_default_ops_attach_li(): Add default ops for Lithium chips
 * @ hal_soc_hdl: hal_soc handle
 *
 * Return: None
 */
void hal_hw_txrx_default_ops_attach_li(struct hal_soc *soc);

uint32_t hal_rx_wbm_err_src_get_li(hal_ring_desc_t ring_desc);
uint8_t hal_rx_ret_buf_manager_get_li(hal_ring_desc_t ring_desc);

/**
 * hal_reo_qdesc_setup - Setup HW REO queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 * @hw_qdesc_vaddr: Virtual address of REO queue descriptor memory
 * @hw_qdesc_paddr: Physical address of REO queue descriptor memory
 * @pn_type: PN type (one of the types defined in 'enum hal_pn_type')
 * @vdev_stats_id: vdev_stats_id
 */
void hal_reo_qdesc_setup_li(hal_soc_handle_t hal_soc_hdl,
			    int tid, uint32_t ba_window_size,
			    uint32_t start_seq, void *hw_qdesc_vaddr,
			    qdf_dma_addr_t hw_qdesc_paddr,
			    int pn_type, uint8_t vdev_stats_id);

#endif /* _HAL_LI_API_H_ */

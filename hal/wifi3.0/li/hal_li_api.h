/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
 * hal_reo_setup_generic_li - Initialize HW REO block
 *
 * @hal_soc: Opaque HAL SOC handle
 * @reo_params: parameters needed by HAL for REO config
 */
void hal_reo_setup_generic_li(struct hal_soc *soc,
			      void *reoparams);

/**
 * hal_set_link_desc_addr_li - Setup link descriptor in a buffer_addr_info
 * HW structure
 *
 * @desc: Descriptor entry (from WBM_IDLE_LINK ring)
 * @cookie: SW cookie for the buffer/descriptor
 * @link_desc_paddr: Physical address of link descriptor entry
 *
 */
void hal_set_link_desc_addr_li(void *desc, uint32_t cookie,
			       qdf_dma_addr_t link_desc_paddr);

/**
 * hal_tx_desc_set_search_type_generic_li - Set the search type value
 * @desc: Handle to Tx Descriptor
 * @search_type: search type
 *		     0 – Normal search
 *		     1 – Index based address search
 *		     2 – Index based flow search
 *
 * Return: void
 */
void hal_tx_desc_set_search_type_generic_li(void *desc,
					    uint8_t search_type);

/**
 * hal_tx_desc_set_search_index_generic_li - Set the search index value
 * @desc: Handle to Tx Descriptor
 * @search_index: The index that will be used for index based address or
 *                flow search. The field is valid when 'search_type' is
 *                1 0r 2
 *
 * Return: void
 */
void hal_tx_desc_set_search_index_generic_li(void *desc,
					     uint32_t search_index);

/**
 * hal_tx_desc_set_cache_set_num_generic_li - Set the cache-set-num value
 * @desc: Handle to Tx Descriptor
 * @cache_num: Cache set number that should be used to cache the index
 *                based search results, for address and flow search.
 *                This value should be equal to LSB four bits of the hash value
 *                of match data, in case of search index points to an entry
 *                which may be used in content based search also. The value can
 *                be anything when the entry pointed by search index will not be
 *                used for content based search.
 *
 * Return: void
 */
void hal_tx_desc_set_cache_set_num_generic_li(void *desc,
					      uint8_t cache_num);

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
 *
 */
void hal_reo_qdesc_setup_li(hal_soc_handle_t hal_soc_hdl,
			    int tid, uint32_t ba_window_size,
			    uint32_t start_seq, void *hw_qdesc_vaddr,
			    qdf_dma_addr_t hw_qdesc_paddr,
			    int pn_type);

#endif /* _HAL_LI_API_H_ */

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

#ifndef _HAL_BE_API_H_
#define _HAL_BE_API_H_

#include "hal_hw_headers.h"
#include "hal_rx.h"

struct hal_hw_cc_config {
	uint32_t lut_base_addr_31_0;
	uint32_t cc_global_en:1,
		 page_4k_align:1,
		 cookie_offset_msb:5,
		 cookie_page_msb:5,
		 lut_base_addr_39_32:8,
		 wbm2sw6_cc_en:1,
		 wbm2sw5_cc_en:1,
		 wbm2sw4_cc_en:1,
		 wbm2sw3_cc_en:1,
		 wbm2sw2_cc_en:1,
		 wbm2sw1_cc_en:1,
		 wbm2sw0_cc_en:1,
		 wbm2fw_cc_en:1,
		 error_path_cookie_conv_en:1,
		 release_path_cookie_conv_en:1,
		 reserved:2;
};

#define HAL_RX_MSDU_EXT_DESC_INFO_GET(msdu_details_ptr) \
	((struct rx_msdu_ext_desc_info *) \
	_OFFSET_TO_BYTE_PTR(msdu_details_ptr, \
RX_MSDU_DETAILS_RX_MSDU_EXT_DESC_INFO_DETAILS_RESERVED_0A_OFFSET))

/**
 * hal_reo_setup_generic_be - Initialize HW REO block
 *
 * @hal_soc: Opaque HAL SOC handle
 * @reo_params: parameters needed by HAL for REO config
 */
void hal_reo_setup_generic_be(struct hal_soc *soc,
			      void *reoparams);

/**
 * hal_rx_msdu_ext_desc_info_get_ptr_be() - Get the msdu extension
 *			descriptor pointer.
 * @msdu_details_ptr: msdu details
 *
 * Return: msdu exntension descriptor pointer.
 */
void *hal_rx_msdu_ext_desc_info_get_ptr_be(void *msdu_details_ptr);

/**
 * hal_set_link_desc_addr_be - Setup link descriptor in a buffer_addr_info
 * HW structure
 *
 * @desc: Descriptor entry (from WBM_IDLE_LINK ring)
 * @cookie: SW cookie for the buffer/descriptor
 * @link_desc_paddr: Physical address of link descriptor entry
 * @bm_id: idle link BM id
 *
 */
void hal_set_link_desc_addr_be(void *desc, uint32_t cookie,
			       qdf_dma_addr_t link_desc_paddr,
			       uint8_t bm_id);

/**
 * hal_hw_txrx_default_ops_attach_be(): Add default ops for BE chips
 * @ hal_soc_hdl: hal_soc handle
 *
 * Return: None
 */
void hal_hw_txrx_default_ops_attach_be(struct hal_soc *soc);

uint32_t hal_tx_comp_get_buffer_source_generic_be(void *hal_desc);
uint8_t hal_rx_ret_buf_manager_get_be(hal_ring_desc_t ring_desc);
void hal_rx_wbm_err_info_get_generic_be(void *wbm_desc, void *wbm_er_info1);

/**
 * hal_reo_qdesc_setup - Setup HW REO queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 * @hw_qdesc_vaddr: Virtual address of REO queue descriptor memory
 * @hw_qdesc_paddr: Physical address of REO queue descriptor memory
 * @pn_type: PN type (one of the types defined in 'enum hal_pn_type')
 * @vdev_stats_id: vdev_stats_id to be programmed in REO Queue Descriptor
 */
void hal_reo_qdesc_setup_be(hal_soc_handle_t hal_soc_hdl,
			    int tid, uint32_t ba_window_size,
			    uint32_t start_seq, void *hw_qdesc_vaddr,
			    qdf_dma_addr_t hw_qdesc_paddr,
			    int pn_type, uint8_t vdev_stats_id);

/**
 * hal_cookie_conversion_reg_cfg_be() - set cookie conversion relevant register
 *					for REO/WBM
 * @soc: HAL soc handle
 * @cc_cfg: structure pointer for HW cookie conversion configuration
 *
 * Return: None
 */
void hal_cookie_conversion_reg_cfg_be(hal_soc_handle_t hal_soc_hdl,
				      struct hal_hw_cc_config *cc_cfg);

/**
 * hal_reo_ix_remap_value_get() - Calculate reo remap register value from
 *				  ring_id_mask which is used for hash based
 *				  reo distribution
 *
 * @hal_soc: Handle to HAL SoC structure
 * @ring_id_mask: mask value indicating the rx rings 0th bit set indicate
 * REO2SW1 is included in hash distribution
 *
 * Return: REO remap value
 */
uint32_t
hal_reo_ix_remap_value_get_be(hal_soc_handle_t hal_soc_hdl,
			      uint8_t rx_ring_mask);

/**
 * hal_reo_ring_remap_value_get_be() - return REO remap value
 *
 * @ring_id: REO2SW ring id
 *
 * Return: REO remap value
 */
uint8_t
hal_reo_ring_remap_value_get_be(uint8_t rx_ring_id);

/**
 * hal_setup_reo_swap() - Set the swap flag for big endian machines
 * @soc: HAL soc handle
 *
 * Return: None
 */
void hal_setup_reo_swap(struct hal_soc *soc);

/**
 * hal_get_idle_link_bm_id_be() - Get idle link BM id from chid_id
 * @chip_id: mlo chip_id
 *
 * Returns: RBM ID
 */
uint8_t hal_get_idle_link_bm_id_be(uint8_t chip_id);
#endif /* _HAL_BE_API_H_ */

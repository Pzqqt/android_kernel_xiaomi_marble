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

#include "wcss_seq_hwiobase.h"
#include "wcss_seq_hwioreg.h"
#include "sw_xml_headers.h"
#include "reo_destination_ring.h"
#include "tcl_data_cmd.h"
#include "tlv_hdr.h"
#include "hal_api.h"

/**
 * hal_setup_link_idle_list - Setup scattered idle list using the
 * buffer list provided
 *
 * @hal_soc: Opaque HAL SOC handle
 * @scatter_bufs_base_paddr: Array of physical base addresses
 * @scatter_bufs_base_vaddr: Array of virtual base addresses
 * @num_scatter_bufs: Number of scatter buffers in the above lists
 * @scatter_buf_size: Size of each scatter buffer
 *
 */
void hal_setup_link_idle_list(void *hal_soc,
	qdf_dma_addr_t scatter_bufs_base_paddr[],
	void *scatter_bufs_base_vaddr[], uint32_t num_scatter_bufs,
	uint32_t scatter_buf_size, uint32_t last_buf_end_offset)
{
	int i;
	uint32_t *prev_buf_link_ptr = NULL;
	struct hal_soc *soc = (struct hal_soc *)hal_soc;

	/* Link the scatter buffers */
	for (i = 0; i < num_scatter_bufs; i++) {
		if (i > 0) {
			prev_buf_link_ptr[0] =
				scatter_bufs_base_paddr[i] & 0xffffffff;
			prev_buf_link_ptr[1] =
				((uint64_t)(scatter_bufs_base_paddr[i]) >> 32) &
				HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_BMSK;
		}
		prev_buf_link_ptr = (uint32_t *)(scatter_bufs_base_vaddr[i] +
			scatter_buf_size - WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE);
	}

	/* TBD: Setup IDLE_LIST_CTRL and IDLE_LIST_SIZE registers - current
	 * definitions in HW headers doesn't match those in WBM MLD document
	 * pending confirmation from HW team
	 */

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		scatter_bufs_base_paddr[0] & 0xffffffff);
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		((uint64_t)(scatter_bufs_base_paddr[0]) >> 32) &
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_BMSK);
	/* ADDRESS_MATCH_TAG field in the above register is expected to match
	 * with the upper bits of link pointer. The above write sets this field
	 * to zero and we are also setting the upper bits of link pointers to
	 * zero while setting up the link list of scatter buffers above
	 */

	/* Setup head and tail pointers for the idle list */
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		scatter_bufs_base_paddr[0] & 0xffffffff);
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1,
		BUFFER_ADDRESS_39_32,
		((uint64_t)(scatter_bufs_base_paddr[0]) >> 32)) |
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1,
		HEAD_POINTER_OFFSET, 0));

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		scatter_bufs_base_paddr[0] & 0xffffffff);

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX0_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		scatter_bufs_base_paddr[num_scatter_bufs - 1] & 0xffffffff);
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_ADDR(
		SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1,
		BUFFER_ADDRESS_39_32,
		((uint64_t)(scatter_bufs_base_paddr[num_scatter_bufs - 1]) >>
		32)) | HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1,
		TAIL_POINTER_OFFSET, last_buf_end_offset << 2));
}


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

#include "hal_be_api.h"
#include "hal_be_hw_headers.h"
#include "hal_be_reo.h"
#include "hal_tx.h"	//HAL_SET_FLD
#include "hal_be_rx.h"	//HAL_RX_BUF_RBM_GET

#if defined(QDF_BIG_ENDIAN_MACHINE)
/**
 * hal_setup_reo_swap() - Set the swap flag for big endian machines
 * @soc: HAL soc handle
 *
 * Return: None
 */
static void hal_setup_reo_swap(struct hal_soc *soc)
{
	uint32_t reg_val;

	reg_val = HAL_REG_READ(soc, HWIO_REO_R0_CACHE_CTL_CONFIG_ADDR(
		REO_REG_REG_BASE));

	reg_val |= HAL_SM(HWIO_REO_R0_CACHE_CTL_CONFIG, WRITE_STRUCT_SWAP, 1);
	reg_val |= HAL_SM(HWIO_REO_R0_CACHE_CTL_CONFIG, READ_STRUCT_SWAP, 1);

	HAL_REG_WRITE(soc, HWIO_REO_R0_CACHE_CTL_CONFIG_ADDR(
		REO_REG_REG_BASE), reg_val);
}
#else
static inline void hal_setup_reo_swap(struct hal_soc *soc)
{
}
#endif

/**
 * hal_tx_init_data_ring_be() - Initialize all the TCL Descriptors in SRNG
 * @hal_soc_hdl: Handle to HAL SoC structure
 * @hal_srng: Handle to HAL SRNG structure
 *
 * Return: none
 */
static void
hal_tx_init_data_ring_be(hal_soc_handle_t hal_soc_hdl,
			 hal_ring_handle_t hal_ring_hdl)
{
}

void hal_reo_setup_generic_be(struct hal_soc *soc, void *reoparams)
{
	uint32_t reg_val;
	struct hal_reo_params *reo_params = (struct hal_reo_params *)reoparams;

	reg_val = HAL_REG_READ(soc, HWIO_REO_R0_GENERAL_ENABLE_ADDR(
		REO_REG_REG_BASE));

	hal_reo_config(soc, reg_val, reo_params);
	/* Other ring enable bits and REO_ENABLE will be set by FW */

	/* TODO: Setup destination ring mapping if enabled */

	/* TODO: Error destination ring setting is left to default.
	 * Default setting is to send all errors to release ring.
	 */

	/* Set the reo descriptor swap bits in case of BIG endian platform */
	hal_setup_reo_swap(soc);

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(REO_REG_REG_BASE),
		      HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000);

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(REO_REG_REG_BASE),
		      (HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(REO_REG_REG_BASE),
		      (HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		      HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(REO_REG_REG_BASE),
		      (HAL_DEFAULT_VO_REO_TIMEOUT_MS * 1000));

	/*
	 * When hash based routing is enabled, routing of the rx packet
	 * is done based on the following value: 1 _ _ _ _ The last 4
	 * bits are based on hash[3:0]. This means the possible values
	 * are 0x10 to 0x1f. This value is used to look-up the
	 * ring ID configured in Destination_Ring_Ctrl_IX_* register.
	 * The Destination_Ring_Ctrl_IX_2 and Destination_Ring_Ctrl_IX_3
	 * registers need to be configured to set-up the 16 entries to
	 * map the hash values to a ring number. There are 3 bits per
	 * hash entry  which are mapped as follows:
	 * 0: TCL, 1:SW1, 2:SW2, * 3:SW3, 4:SW4, 5:Release, 6:FW(WIFI),
	 * 7: NOT_USED.
	 */
	if (reo_params->rx_hash_enabled) {
		HAL_REG_WRITE(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
			REO_REG_REG_BASE),
			reo_params->remap1);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
				       REO_REG_REG_BASE)));

		HAL_REG_WRITE(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
			REO_REG_REG_BASE),
			reo_params->remap2);

		hal_debug("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR 0x%x",
			  HAL_REG_READ(soc,
				       HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
				       REO_REG_REG_BASE)));
	}

	/* TODO: Check if the following registers shoould be setup by host:
	 * AGING_CONTROL
	 * HIGH_MEMORY_THRESHOLD
	 * GLOBAL_LINK_DESC_COUNT_THRESH_IX_0[1,2]
	 * GLOBAL_LINK_DESC_COUNT_CTRL
	 */
}

void hal_set_link_desc_addr_be(void *desc, uint32_t cookie,
			       qdf_dma_addr_t link_desc_paddr)
{
	uint32_t *buf_addr = (uint32_t *)desc;

	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO, BUFFER_ADDR_31_0,
			   link_desc_paddr & 0xffffffff);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO, BUFFER_ADDR_39_32,
			   (uint64_t)link_desc_paddr >> 32);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO, RETURN_BUFFER_MANAGER,
			   WBM_IDLE_DESC_LIST);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO, SW_BUFFER_COOKIE,
			   cookie);
}

void *hal_rx_msdu_ext_desc_info_get_ptr_be(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_EXT_DESC_INFO_GET(msdu_details_ptr);
}

#ifdef TCL_DATA_CMD_SEARCH_INDEX_OFFSET
void hal_tx_desc_set_search_index_generic_be(void *desc, uint32_t search_index)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD, SEARCH_INDEX) |=
		HAL_TX_SM(TCL_DATA_CMD, SEARCH_INDEX, search_index);
}
#else
void hal_tx_desc_set_search_index_generic_be(void *desc, uint32_t search_index)
{
}
#endif

#ifdef TCL_DATA_CMD_CACHE_SET_NUM_OFFSET
void hal_tx_desc_set_cache_set_num_generic_be(void *desc, uint8_t cache_num)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD, CACHE_SET_NUM) |=
		HAL_TX_SM(TCL_DATA_CMD, CACHE_SET_NUM, cache_num);
}
#else
void hal_tx_desc_set_cache_set_num_generic_be(void *desc, uint8_t cache_num)
{
}
#endif

static inline uint32_t
hal_wbm2sw_release_source_get(void *hal_desc, enum hal_be_wbm_release_dir dir)
{
	uint32_t buf_src;

	buf_src = HAL_WBM2SW_RELEASE_SRC_GET(hal_desc);
	switch (buf_src) {
	case HAL_BE_RX_WBM_ERR_SRC_RXDMA:
		return HAL_RX_WBM_ERR_SRC_RXDMA;
	case HAL_BE_RX_WBM_ERR_SRC_REO:
		return HAL_RX_WBM_ERR_SRC_REO;
	case HAL_BE_RX_WBM_ERR_SRC_FW_RX:
		if (dir != HAL_BE_WBM_RELEASE_DIR_RX)
			qdf_assert_always(0);
		return HAL_RX_WBM_ERR_SRC_FW;
	case HAL_BE_RX_WBM_ERR_SRC_SW_RX:
		if (dir != HAL_BE_WBM_RELEASE_DIR_RX)
			qdf_assert_always(0);
		return HAL_RX_WBM_ERR_SRC_SW;
	case HAL_BE_RX_WBM_ERR_SRC_TQM:
		return HAL_RX_WBM_ERR_SRC_TQM;
	case HAL_BE_RX_WBM_ERR_SRC_FW_TX:
		if (dir != HAL_BE_WBM_RELEASE_DIR_TX)
			qdf_assert_always(0);
		return HAL_RX_WBM_ERR_SRC_FW;
	case HAL_BE_RX_WBM_ERR_SRC_SW_TX:
		if (dir != HAL_BE_WBM_RELEASE_DIR_TX)
			qdf_assert_always(0);
		return HAL_RX_WBM_ERR_SRC_SW;
	default:
		qdf_assert_always(0);
	}

	return buf_src;
}

uint32_t hal_tx_comp_get_buffer_source_generic_be(void *hal_desc)
{
	return hal_wbm2sw_release_source_get(hal_desc,
					     HAL_BE_WBM_RELEASE_DIR_TX);
}

/**
 * hal_setup_link_idle_list_generic_be - Setup scattered idle list using the
 * buffer list provided
 *
 * @hal_soc: Opaque HAL SOC handle
 * @scatter_bufs_base_paddr: Array of physical base addresses
 * @scatter_bufs_base_vaddr: Array of virtual base addresses
 * @num_scatter_bufs: Number of scatter buffers in the above lists
 * @scatter_buf_size: Size of each scatter buffer
 * @last_buf_end_offset: Offset to the last entry
 * @num_entries: Total entries of all scatter bufs
 *
 * Return: None
 */
static void
hal_setup_link_idle_list_generic_be(struct hal_soc *soc,
				    qdf_dma_addr_t scatter_bufs_base_paddr[],
				    void *scatter_bufs_base_vaddr[],
				    uint32_t num_scatter_bufs,
				    uint32_t scatter_buf_size,
				    uint32_t last_buf_end_offset,
				    uint32_t num_entries)
{
	int i;
	uint32_t *prev_buf_link_ptr = NULL;
	uint32_t reg_scatter_buf_size, reg_tot_scatter_buf_size;
	uint32_t val;

	/* Link the scatter buffers */
	for (i = 0; i < num_scatter_bufs; i++) {
		if (i > 0) {
			prev_buf_link_ptr[0] =
				scatter_bufs_base_paddr[i] & 0xffffffff;
			prev_buf_link_ptr[1] = HAL_SM(
				HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB,
				BASE_ADDRESS_39_32,
				((uint64_t)(scatter_bufs_base_paddr[i])
				 >> 32)) | HAL_SM(
				HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB,
				ADDRESS_MATCH_TAG,
				ADDRESS_MATCH_TAG_VAL);
		}
		prev_buf_link_ptr = (uint32_t *)(scatter_bufs_base_vaddr[i] +
			scatter_buf_size - WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE);
	}

	/* TBD: Register programming partly based on MLD & the rest based on
	 * inputs from HW team. Not complete yet.
	 */

	reg_scatter_buf_size = (scatter_buf_size -
				WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE) / 64;
	reg_tot_scatter_buf_size = ((scatter_buf_size -
		WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE) * num_scatter_bufs) / 64;

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_IDLE_LIST_CONTROL_ADDR(
		WBM_REG_REG_BASE),
		HAL_SM(HWIO_WBM_R0_IDLE_LIST_CONTROL, SCATTER_BUFFER_SIZE,
		reg_scatter_buf_size) |
		HAL_SM(HWIO_WBM_R0_IDLE_LIST_CONTROL, LINK_DESC_IDLE_LIST_MODE,
		0x1));

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_IDLE_LIST_SIZE_ADDR(
		WBM_REG_REG_BASE),
		HAL_SM(HWIO_WBM_R0_IDLE_LIST_SIZE,
		SCATTER_RING_SIZE_OF_IDLE_LINK_DESC_LIST,
		reg_tot_scatter_buf_size));

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_LSB_ADDR(
		WBM_REG_REG_BASE),
		scatter_bufs_base_paddr[0] & 0xffffffff);

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDR(
		WBM_REG_REG_BASE),
		((uint64_t)(scatter_bufs_base_paddr[0]) >> 32) &
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_BASE_ADDRESS_39_32_BMSK);

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB_ADDR(
		WBM_REG_REG_BASE),
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB,
		BASE_ADDRESS_39_32, ((uint64_t)(scatter_bufs_base_paddr[0])
								>> 32)) |
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_LIST_BASE_MSB,
		ADDRESS_MATCH_TAG, ADDRESS_MATCH_TAG_VAL));

	/* ADDRESS_MATCH_TAG field in the above register is expected to match
	 * with the upper bits of link pointer. The above write sets this field
	 * to zero and we are also setting the upper bits of link pointers to
	 * zero while setting up the link list of scatter buffers above
	 */

	/* Setup head and tail pointers for the idle list */
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(
		WBM_REG_REG_BASE),
		scatter_bufs_base_paddr[num_scatter_bufs - 1] & 0xffffffff);
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1_ADDR(
		WBM_REG_REG_BASE),
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1,
		BUFFER_ADDRESS_39_32,
		((uint64_t)(scatter_bufs_base_paddr[num_scatter_bufs - 1])
								>> 32)) |
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX1,
		HEAD_POINTER_OFFSET, last_buf_end_offset >> 2));

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HEAD_INFO_IX0_ADDR(
		WBM_REG_REG_BASE),
		scatter_bufs_base_paddr[0] & 0xffffffff);

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX0_ADDR(
		WBM_REG_REG_BASE),
		scatter_bufs_base_paddr[0] & 0xffffffff);
	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1_ADDR(
		WBM_REG_REG_BASE),
		HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1,
		BUFFER_ADDRESS_39_32,
		((uint64_t)(scatter_bufs_base_paddr[0]) >>
		32)) | HAL_SM(HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_TAIL_INFO_IX1,
		TAIL_POINTER_OFFSET, 0));

	HAL_REG_WRITE(soc,
		HWIO_WBM_R0_SCATTERED_LINK_DESC_PTR_HP_ADDR(
		WBM_REG_REG_BASE),
		2 * num_entries);

	/* Set RING_ID_DISABLE */
	val = HAL_SM(HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC, RING_ID_DISABLE, 1);

	/*
	 * SRNG_ENABLE bit is not available in HWK v1 (QCA8074v1). Hence
	 * check the presence of the bit before toggling it.
	 */
#ifdef HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_SRNG_ENABLE_BMSK
	val |= HAL_SM(HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC, SRNG_ENABLE, 1);
#endif
	HAL_REG_WRITE(soc,
		      HWIO_WBM_R0_WBM_IDLE_LINK_RING_MISC_ADDR(WBM_REG_REG_BASE),
		      val);
}

/**
 * hal_rx_wbm_err_src_get_be() - Get WBM error source from descriptor
 * @ring_desc: ring descriptor
 *
 * Return: wbm error source
 */
static uint32_t hal_rx_wbm_err_src_get_be(hal_ring_desc_t ring_desc)
{
	return hal_wbm2sw_release_source_get(ring_desc,
					     HAL_BE_WBM_RELEASE_DIR_RX);
}

/**
 * hal_rx_ret_buf_manager_get_be() - Get return buffer manager from ring desc
 * @ring_desc: ring descriptor
 *
 * Return: rbm
 */
uint8_t hal_rx_ret_buf_manager_get_be(hal_ring_desc_t ring_desc)
{
	/*
	 * The following macro takes buf_addr_info as argument,
	 * but since buf_addr_info is the first field in ring_desc
	 * Hence the following call is OK
	 */
	return HAL_RX_BUF_RBM_GET(ring_desc);
}

#define HAL_RX_WBM_REO_PUSH_REASON_GET(wbm_desc) (((*(((uint32_t *)wbm_desc) + \
		(WBM2SW_COMPLETION_RING_RX_REO_PUSH_REASON_OFFSET >> 2))) & \
		WBM2SW_COMPLETION_RING_RX_REO_PUSH_REASON_MASK) >> \
		WBM2SW_COMPLETION_RING_RX_REO_PUSH_REASON_LSB)

#define HAL_RX_WBM_REO_ERROR_CODE_GET(wbm_desc) (((*(((uint32_t *)wbm_desc) + \
		(WBM2SW_COMPLETION_RING_RX_REO_ERROR_CODE_OFFSET >> 2))) & \
		WBM2SW_COMPLETION_RING_RX_REO_ERROR_CODE_MASK) >> \
		WBM2SW_COMPLETION_RING_RX_REO_ERROR_CODE_LSB)

#define HAL_RX_WBM_RXDMA_PUSH_REASON_GET(wbm_desc)	\
	(((*(((uint32_t *)wbm_desc) +			\
	(WBM2SW_COMPLETION_RING_RX_RXDMA_PUSH_REASON_OFFSET >> 2))) & \
	WBM2SW_COMPLETION_RING_RX_RXDMA_PUSH_REASON_MASK) >>	\
	WBM2SW_COMPLETION_RING_RX_RXDMA_PUSH_REASON_LSB)

#define HAL_RX_WBM_RXDMA_ERROR_CODE_GET(wbm_desc)	\
	(((*(((uint32_t *)wbm_desc) +			\
	(WBM2SW_COMPLETION_RING_RX_RXDMA_ERROR_CODE_OFFSET >> 2))) & \
	WBM2SW_COMPLETION_RING_RX_RXDMA_ERROR_CODE_MASK) >>	\
	WBM2SW_COMPLETION_RING_RX_RXDMA_ERROR_CODE_LSB)

/**
 * hal_rx_wbm_err_info_get_generic_be(): Retrieves WBM error code and reason and
 *	save it to hal_wbm_err_desc_info structure passed by caller
 * @wbm_desc: wbm ring descriptor
 * @wbm_er_info1: hal_wbm_err_desc_info structure, output parameter.
 * Return: void
 */
void hal_rx_wbm_err_info_get_generic_be(void *wbm_desc, void *wbm_er_info1)
{
	struct hal_wbm_err_desc_info *wbm_er_info =
		(struct hal_wbm_err_desc_info *)wbm_er_info1;

	wbm_er_info->wbm_err_src = hal_rx_wbm_err_src_get_be(wbm_desc);
	wbm_er_info->reo_psh_rsn = HAL_RX_WBM_REO_PUSH_REASON_GET(wbm_desc);
	wbm_er_info->reo_err_code = HAL_RX_WBM_REO_ERROR_CODE_GET(wbm_desc);
	wbm_er_info->rxdma_psh_rsn = HAL_RX_WBM_RXDMA_PUSH_REASON_GET(wbm_desc);
	wbm_er_info->rxdma_err_code = HAL_RX_WBM_RXDMA_ERROR_CODE_GET(wbm_desc);
}

/**
 * hal_rx_reo_ent_buf_paddr_get_be: Gets the physical address and
 * cookie from the REO entrance ring element
 *
 * @ hal_rx_desc_cookie: Opaque cookie pointer used by HAL to get to
 * the current descriptor
 * @ buf_info: structure to return the buffer information
 * @ msdu_cnt: pointer to msdu count in MPDU
 * Return: void
 */
static
void hal_rx_buf_cookie_rbm_get_be(hal_buff_addrinfo_t buf_addr_info_hdl,
				  hal_buf_info_t buf_info_hdl)
{
	struct hal_buf_info *buf_info =
		(struct hal_buf_info *)buf_info_hdl;
	struct buffer_addr_info *buf_addr_info =
		(struct buffer_addr_info *)buf_addr_info_hdl;

	buf_info->sw_cookie = HAL_RX_BUF_COOKIE_GET(buf_addr_info);
	/*
	 * buffer addr info is the first member of ring desc, so the typecast
	 * can be done.
	 */
	buf_info->rbm = hal_rx_ret_buf_manager_get_be(
						(hal_ring_desc_t)buf_addr_info);
}

/*
 * hal_rxdma_buff_addr_info_set_be() - set the buffer_addr_info of the
 *				    rxdma ring entry.
 * @rxdma_entry: descriptor entry
 * @paddr: physical address of nbuf data pointer.
 * @cookie: SW cookie used as a index to SW rx desc.
 * @manager: who owns the nbuf (host, NSS, etc...).
 *
 */
static inline void
hal_rxdma_buff_addr_info_set_be(void *rxdma_entry,
				qdf_dma_addr_t paddr, uint32_t cookie,
				uint8_t manager)
{
	uint32_t paddr_lo = ((u64)paddr & 0x00000000ffffffff);
	uint32_t paddr_hi = ((u64)paddr & 0xffffffff00000000) >> 32;

	HAL_RXDMA_PADDR_LO_SET(rxdma_entry, paddr_lo);
	HAL_RXDMA_PADDR_HI_SET(rxdma_entry, paddr_hi);
	HAL_RXDMA_COOKIE_SET(rxdma_entry, cookie);
	HAL_RXDMA_MANAGER_SET(rxdma_entry, manager);
}

/**
 * hal_rx_msdu_flags_get_be() - Get msdu flags from ring desc
 * @msdu_desc_info_hdl: msdu desc info handle
 *
 * Return: msdu flags
 */
static uint32_t hal_rx_msdu_flags_get_be(rx_msdu_desc_info_t msdu_desc_info_hdl)
{
	struct rx_msdu_desc_info *msdu_desc_info =
		(struct rx_msdu_desc_info *)msdu_desc_info_hdl;
	uint32_t flags = 0;

// SA_IDX_TIMEOUT and DA_IDX_TIMEOUT is not present in hamilton
// DA_IDX_TIMEOUT and DA_IDX_TIMEOUT is not present in hamilton
	if (HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_GET(msdu_desc_info))
		flags |= HAL_MSDU_F_FIRST_MSDU_IN_MPDU;

	if (HAL_RX_LAST_MSDU_IN_MPDU_FLAG_GET(msdu_desc_info))
		flags |= HAL_MSDU_F_LAST_MSDU_IN_MPDU;

	if (HAL_RX_MSDU_CONTINUATION_FLAG_GET(msdu_desc_info))
		flags |= HAL_MSDU_F_MSDU_CONTINUATION;

	if (HAL_RX_MSDU_SA_IS_VALID_FLAG_GET(msdu_desc_info))
		flags |= HAL_MSDU_F_SA_IS_VALID;

	if (HAL_RX_MSDU_DA_IS_VALID_FLAG_GET(msdu_desc_info))
		flags |= HAL_MSDU_F_DA_IS_VALID;

	if (HAL_RX_MSDU_DA_IS_MCBC_FLAG_GET(msdu_desc_info))
		flags |= HAL_MSDU_F_DA_IS_MCBC;

	return flags;
}

/**
 * hal_rx_get_reo_error_code_be() - Get REO error code from ring desc
 * @rx_desc: rx descriptor
 *
 * Return: REO error code
 */
static uint32_t hal_rx_get_reo_error_code_be(hal_ring_desc_t rx_desc)
{
	struct reo_destination_ring *reo_desc =
			(struct reo_destination_ring *)rx_desc;

	return HAL_RX_REO_ERROR_GET(reo_desc);
}

/**
 * hal_gen_reo_remap_val_generic_be() - Generate the reo map value
 * @ix0_map: mapping values for reo
 *
 * Return: IX0 reo remap register value to be written
 */
static uint32_t
hal_gen_reo_remap_val_generic_be(enum hal_reo_remap_reg remap_reg,
				 uint8_t *ix0_map)
{
	uint32_t ix_val = 0;

	switch (remap_reg) {
	case HAL_REO_REMAP_REG_IX0:
		ix_val = HAL_REO_REMAP_IX0(ix0_map[0], 0) |
			HAL_REO_REMAP_IX0(ix0_map[1], 1) |
			HAL_REO_REMAP_IX0(ix0_map[2], 2) |
			HAL_REO_REMAP_IX0(ix0_map[3], 3) |
			HAL_REO_REMAP_IX0(ix0_map[4], 4) |
			HAL_REO_REMAP_IX0(ix0_map[5], 5) |
			HAL_REO_REMAP_IX0(ix0_map[6], 6) |
			HAL_REO_REMAP_IX0(ix0_map[7], 7);
		break;
	case HAL_REO_REMAP_REG_IX2:
		ix_val = HAL_REO_REMAP_IX2(ix0_map[0], 16) |
			HAL_REO_REMAP_IX2(ix0_map[1], 17) |
			HAL_REO_REMAP_IX2(ix0_map[2], 18) |
			HAL_REO_REMAP_IX2(ix0_map[3], 19) |
			HAL_REO_REMAP_IX2(ix0_map[4], 20) |
			HAL_REO_REMAP_IX2(ix0_map[5], 21) |
			HAL_REO_REMAP_IX2(ix0_map[6], 22) |
			HAL_REO_REMAP_IX2(ix0_map[7], 23);
		break;
	default:
		break;
	}

	return ix_val;
}

void hal_rx_mpdu_desc_info_get_be(void *desc_addr,
				  void *mpdu_desc_info_hdl)
{
	struct reo_destination_ring *reo_dst_ring;
	struct hal_rx_mpdu_desc_info *mpdu_desc_info =
		(struct hal_rx_mpdu_desc_info *)mpdu_desc_info_hdl;
	uint32_t *mpdu_info;

	reo_dst_ring = (struct reo_destination_ring *)desc_addr;

	mpdu_info = (uint32_t *)&reo_dst_ring->rx_mpdu_desc_info_details;

	mpdu_desc_info->msdu_count = HAL_RX_MPDU_MSDU_COUNT_GET(mpdu_info);
// TODO - MPDU Sequence number is not there in hamilton
// mpdu_seq is just used for printing at two places
//	mpdu_desc_info->mpdu_seq = HAL_RX_MPDU_SEQUENCE_NUMBER_GET(mpdu_info);
	mpdu_desc_info->mpdu_flags = hal_rx_get_mpdu_flags(mpdu_info);
	mpdu_desc_info->peer_meta_data =
		HAL_RX_MPDU_DESC_PEER_META_DATA_GET(mpdu_info);
	mpdu_desc_info->bar_frame = HAL_RX_MPDU_BAR_FRAME_GET(mpdu_info);
}

static uint8_t hal_rx_err_status_get_be(hal_ring_desc_t rx_desc)
{
	return HAL_RX_ERROR_STATUS_GET(rx_desc);
}

static uint8_t hal_rx_reo_buf_type_get_be(hal_ring_desc_t rx_desc)
{
	return HAL_RX_REO_BUF_TYPE_GET(rx_desc);
}

/**
 * hal_hw_txrx_default_ops_attach_be() - Attach the default hal ops for
 *		beryllium chipsets.
 * @hal_soc_hdl: HAL soc handle
 *
 * Return: None
 */
void hal_hw_txrx_default_ops_attach_be(struct hal_soc *hal_soc)
{
	hal_soc->ops->hal_set_link_desc_addr = hal_set_link_desc_addr_be;
	hal_soc->ops->hal_tx_init_data_ring = hal_tx_init_data_ring_be;
	hal_soc->ops->hal_get_ba_aging_timeout = hal_get_ba_aging_timeout_be;
	hal_soc->ops->hal_set_ba_aging_timeout = hal_set_ba_aging_timeout_be;
	hal_soc->ops->hal_get_reo_reg_base_offset =
					hal_get_reo_reg_base_offset_be;
	hal_soc->ops->hal_setup_link_idle_list =
				hal_setup_link_idle_list_generic_be;

	hal_soc->ops->hal_rx_buf_cookie_rbm_get = hal_rx_buf_cookie_rbm_get_be;
	hal_soc->ops->hal_rx_ret_buf_manager_get =
						hal_rx_ret_buf_manager_get_be;
	hal_soc->ops->hal_rxdma_buff_addr_info_set =
					hal_rxdma_buff_addr_info_set_be;
	hal_soc->ops->hal_rx_msdu_flags_get = hal_rx_msdu_flags_get_be;
	hal_soc->ops->hal_rx_get_reo_error_code = hal_rx_get_reo_error_code_be;
	hal_soc->ops->hal_gen_reo_remap_val =
				hal_gen_reo_remap_val_generic_be;
	hal_soc->ops->hal_tx_comp_get_buffer_source =
				hal_tx_comp_get_buffer_source_generic_be;
	hal_soc->ops->hal_rx_mpdu_desc_info_get =
				hal_rx_mpdu_desc_info_get_be;
	hal_soc->ops->hal_rx_err_status_get = hal_rx_err_status_get_be;
	hal_soc->ops->hal_rx_reo_buf_type_get = hal_rx_reo_buf_type_get_be;
	hal_soc->ops->hal_rx_wbm_err_src_get = hal_rx_wbm_err_src_get_be;
	hal_soc->ops->hal_reo_send_cmd = hal_reo_send_cmd_be;
	hal_soc->ops->hal_reo_queue_stats_status =
						hal_reo_queue_stats_status_be;
	hal_soc->ops->hal_reo_flush_queue_status =
						hal_reo_flush_queue_status_be;
	hal_soc->ops->hal_reo_flush_cache_status =
						hal_reo_flush_cache_status_be;
	hal_soc->ops->hal_reo_unblock_cache_status =
						hal_reo_unblock_cache_status_be;
	hal_soc->ops->hal_reo_flush_timeout_list_status =
					hal_reo_flush_timeout_list_status_be;
	hal_soc->ops->hal_reo_desc_thres_reached_status =
					hal_reo_desc_thres_reached_status_be;
	hal_soc->ops->hal_reo_rx_update_queue_status =
					hal_reo_rx_update_queue_status_be;
	hal_soc->ops->hal_reo_qdesc_setup = hal_reo_qdesc_setup_be;
}

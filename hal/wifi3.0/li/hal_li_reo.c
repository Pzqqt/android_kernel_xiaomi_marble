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

#include "qdf_module.h"
#include "hal_li_hw_headers.h"
#include "hal_reo.h"
#include "hal_li_reo.h"
#include "hal_li_api.h"

uint32_t hal_get_reo_reg_base_offset_li(void)
{
	return SEQ_WCSS_UMAC_REO_REG_OFFSET;
}

/**
 * hal_reo_qdesc_setup - Setup HW REO queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 * @hw_qdesc_vaddr: Virtual address of REO queue descriptor memory
 * @hw_qdesc_paddr: Physical address of REO queue descriptor memory
 * @tid: TID
 *
 */
void hal_reo_qdesc_setup_li(hal_soc_handle_t hal_soc_hdl, int tid,
			    uint32_t ba_window_size,
			    uint32_t start_seq, void *hw_qdesc_vaddr,
			    qdf_dma_addr_t hw_qdesc_paddr,
			    int pn_type, uint8_t vdev_stats_id)
{
	uint32_t *reo_queue_desc = (uint32_t *)hw_qdesc_vaddr;
	uint32_t *reo_queue_ext_desc;
	uint32_t reg_val;
	uint32_t pn_enable;
	uint32_t pn_size = 0;

	qdf_mem_zero(hw_qdesc_vaddr, sizeof(struct rx_reo_queue));

	hal_uniform_desc_hdr_setup(reo_queue_desc, HAL_DESC_REO_OWNED,
				   HAL_REO_QUEUE_DESC);
	/* Fixed pattern in reserved bits for debugging */
	HAL_DESC_SET_FIELD(reo_queue_desc, UNIFORM_DESCRIPTOR_HEADER_0,
			   RESERVED_0A, 0xDDBEEF);

	/* This a just a SW meta data and will be copied to REO destination
	 * descriptors indicated by hardware.
	 * TODO: Setting TID in this field. See if we should set something else.
	 */
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_1,
			   RECEIVE_QUEUE_NUMBER, tid);
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
			   VLD, 1);
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
			   ASSOCIATED_LINK_DESCRIPTOR_COUNTER,
			   HAL_RX_LINK_DESC_CNTR);

	/*
	 * Fields DISABLE_DUPLICATE_DETECTION and SOFT_REORDER_ENABLE will be 0
	 */

	reg_val = TID_TO_WME_AC(tid);
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, AC, reg_val);

	if (ba_window_size < 1)
		ba_window_size = 1;

	/* WAR to get 2k exception in Non BA case.
	 * Setting window size to 2 to get 2k jump exception
	 * when we receive aggregates in Non BA case
	 */
	ba_window_size = hal_update_non_ba_win_size(tid, ba_window_size);

	/* Set RTY bit for non-BA case. Duplicate detection is currently not
	 * done by HW in non-BA case if RTY bit is not set.
	 * TODO: This is a temporary War and should be removed once HW fix is
	 * made to check and discard duplicates even if RTY bit is not set.
	 */
	if (ba_window_size == 1)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, RTY, 1);

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, BA_WINDOW_SIZE,
			   ba_window_size - 1);

	switch (pn_type) {
	case HAL_PN_WPA:
		pn_enable = 1;
		pn_size = PN_SIZE_48;
		break;
	case HAL_PN_WAPI_EVEN:
	case HAL_PN_WAPI_UNEVEN:
		pn_enable = 1;
		pn_size = PN_SIZE_128;
		break;
	default:
		pn_enable = 0;
		break;
	}

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, PN_CHECK_NEEDED,
			   pn_enable);

	if (pn_type == HAL_PN_WAPI_EVEN)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
				   PN_SHALL_BE_EVEN, 1);
	else if (pn_type == HAL_PN_WAPI_UNEVEN)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
				   PN_SHALL_BE_UNEVEN, 1);

	/*
	 *  TODO: Need to check if PN handling in SW needs to be enabled
	 *  So far this is not a requirement
	 */

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, PN_SIZE,
			   pn_size);

	/* TODO: Check if RX_REO_QUEUE_2_IGNORE_AMPDU_FLAG need to be set
	 * based on BA window size and/or AMPDU capabilities
	 */
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
			   IGNORE_AMPDU_FLAG, 1);

	if (start_seq <= 0xfff)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_3, SSN,
				   start_seq);

	/* TODO: SVLD should be set to 1 if a valid SSN is received in ADDBA,
	 * but REO is not delivering packets if we set it to 1. Need to enable
	 * this once the issue is resolved
	 */
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_3, SVLD, 0);

	/* TODO: Check if we should set start PN for WAPI */

	/* TODO: HW queue descriptors are currently allocated for max BA
	 * window size for all QOS TIDs so that same descriptor can be used
	 * later when ADDBA request is recevied. This should be changed to
	 * allocate HW queue descriptors based on BA window size being
	 * negotiated (0 for non BA cases), and reallocate when BA window
	 * size changes and also send WMI message to FW to change the REO
	 * queue descriptor in Rx peer entry as part of dp_rx_tid_update.
	 */
	if (tid == HAL_NON_QOS_TID)
		return;

	reo_queue_ext_desc = (uint32_t *)
		(((struct rx_reo_queue *)reo_queue_desc) + 1);
	qdf_mem_zero(reo_queue_ext_desc, 3 *
		sizeof(struct rx_reo_queue_ext));
	/* Initialize first reo queue extension descriptor */
	hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
				   HAL_DESC_REO_OWNED,
				   HAL_REO_QUEUE_EXT_DESC);
	/* Fixed pattern in reserved bits for debugging */
	HAL_DESC_SET_FIELD(reo_queue_ext_desc,
			   UNIFORM_DESCRIPTOR_HEADER_0, RESERVED_0A,
			   0xADBEEF);
	/* Initialize second reo queue extension descriptor */
	reo_queue_ext_desc = (uint32_t *)
		(((struct rx_reo_queue_ext *)reo_queue_ext_desc) + 1);
	hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
				   HAL_DESC_REO_OWNED,
				   HAL_REO_QUEUE_EXT_DESC);
	/* Fixed pattern in reserved bits for debugging */
	HAL_DESC_SET_FIELD(reo_queue_ext_desc,
			   UNIFORM_DESCRIPTOR_HEADER_0, RESERVED_0A,
			   0xBDBEEF);
	/* Initialize third reo queue extension descriptor */
	reo_queue_ext_desc = (uint32_t *)
		(((struct rx_reo_queue_ext *)reo_queue_ext_desc) + 1);
	hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
				   HAL_DESC_REO_OWNED,
				   HAL_REO_QUEUE_EXT_DESC);
	/* Fixed pattern in reserved bits for debugging */
	HAL_DESC_SET_FIELD(reo_queue_ext_desc,
			   UNIFORM_DESCRIPTOR_HEADER_0, RESERVED_0A,
				   0xCDBEEF);
}

qdf_export_symbol(hal_reo_qdesc_setup_li);

/**
 * hal_get_ba_aging_timeout_li - Get BA Aging timeout
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ac: Access category
 * @value: window size to get
 */
void hal_get_ba_aging_timeout_li(hal_soc_handle_t hal_soc_hdl, uint8_t ac,
				 uint32_t *value)
{
	struct hal_soc *soc = (struct hal_soc *)hal_soc_hdl;

	switch (ac) {
	case WME_AC_BE:
		*value = HAL_REG_READ(soc,
				      HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(
				      SEQ_WCSS_UMAC_REO_REG_OFFSET)) / 1000;
		break;
	case WME_AC_BK:
		*value = HAL_REG_READ(soc,
				      HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(
				      SEQ_WCSS_UMAC_REO_REG_OFFSET)) / 1000;
		break;
	case WME_AC_VI:
		*value = HAL_REG_READ(soc,
				      HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(
				      SEQ_WCSS_UMAC_REO_REG_OFFSET)) / 1000;
		break;
	case WME_AC_VO:
		*value = HAL_REG_READ(soc,
				      HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(
				      SEQ_WCSS_UMAC_REO_REG_OFFSET)) / 1000;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid AC: %d\n", ac);
	}
}
qdf_export_symbol(hal_get_ba_aging_timeout_li);

/**
 * hal_set_ba_aging_timeout_li - Set BA Aging timeout
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ac: Access category
 * ac: 0 - Background, 1 - Best Effort, 2 - Video, 3 - Voice
 * @value: Input value to set
 */
void hal_set_ba_aging_timeout_li(hal_soc_handle_t hal_soc_hdl, uint8_t ac,
				 uint32_t value)
{
	struct hal_soc *soc = (struct hal_soc *)hal_soc_hdl;

	switch (ac) {
	case WME_AC_BE:
		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(
			      SEQ_WCSS_UMAC_REO_REG_OFFSET),
			      value * 1000);
		break;
	case WME_AC_BK:
		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(
			      SEQ_WCSS_UMAC_REO_REG_OFFSET),
			      value * 1000);
		break;
	case WME_AC_VI:
		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(
			      SEQ_WCSS_UMAC_REO_REG_OFFSET),
			      value * 1000);
		break;
	case WME_AC_VO:
		HAL_REG_WRITE(soc,
			      HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(
			      SEQ_WCSS_UMAC_REO_REG_OFFSET),
			      value * 1000);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid AC: %d\n", ac);
	}
}
qdf_export_symbol(hal_set_ba_aging_timeout_li);

static inline void
hal_reo_cmd_set_descr_addr_li(uint32_t *reo_desc, enum hal_reo_cmd_type type,
			      uint32_t paddr_lo, uint8_t paddr_hi)
{
	switch (type) {
	case CMD_GET_QUEUE_STATS:
		HAL_DESC_SET_FIELD(reo_desc, REO_GET_QUEUE_STATS_1,
				   RX_REO_QUEUE_DESC_ADDR_31_0, paddr_lo);
		HAL_DESC_SET_FIELD(reo_desc, REO_GET_QUEUE_STATS_2,
				   RX_REO_QUEUE_DESC_ADDR_39_32, paddr_hi);
		break;
	case CMD_FLUSH_QUEUE:
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_QUEUE_1,
				   FLUSH_DESC_ADDR_31_0, paddr_lo);
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_QUEUE_2,
				   FLUSH_DESC_ADDR_39_32, paddr_hi);
		break;
	case CMD_FLUSH_CACHE:
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_1,
				   FLUSH_ADDR_31_0, paddr_lo);
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2,
				   FLUSH_ADDR_39_32, paddr_hi);
		break;
	case CMD_UPDATE_RX_REO_QUEUE:
		HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_1,
				   RX_REO_QUEUE_DESC_ADDR_31_0, paddr_lo);
		HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
				   RX_REO_QUEUE_DESC_ADDR_39_32, paddr_hi);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid REO command type", __func__);
		break;
	}
}

static inline int
hal_reo_cmd_queue_stats_li(hal_ring_handle_t  hal_ring_hdl,
			   hal_soc_handle_t hal_soc_hdl,
			   struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_srng_access_start(hal_soc_hdl, hal_ring_hdl);
	reo_desc = hal_srng_src_get_next(hal_soc, hal_ring_hdl);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Out of cmd ring entries", __func__);
		hal_srng_access_end(hal_soc, hal_ring_hdl);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_GET_QUEUE_STATS_E,
			sizeof(struct reo_get_queue_stats));

	/*
	 * Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((reo_desc + NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER),
		     sizeof(struct reo_get_queue_stats) -
		     (NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER << 2));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
			   REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr_li(reo_desc, CMD_GET_QUEUE_STATS,
				      cmd->std.addr_lo,
				      cmd->std.addr_hi);

	HAL_DESC_SET_FIELD(reo_desc, REO_GET_QUEUE_STATS_2, CLEAR_STATS,
			   cmd->u.stats_params.clear);

	if (hif_pm_runtime_get(hal_soc->hif_handle,
			       RTPM_ID_HAL_REO_CMD, false) == 0) {
		hal_srng_access_end(hal_soc_hdl, hal_ring_hdl);
		hif_pm_runtime_put(hal_soc->hif_handle,
				   RTPM_ID_HAL_REO_CMD);
	} else {
		hal_srng_access_end_reap(hal_soc_hdl, hal_ring_hdl);
		hal_srng_set_event(hal_ring_hdl, HAL_SRNG_FLUSH_EVENT);
		hal_srng_inc_flush_cnt(hal_ring_hdl);
	}

	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
			     val);
}

static inline int
hal_reo_cmd_flush_queue_li(hal_ring_handle_t hal_ring_hdl,
			   hal_soc_handle_t hal_soc_hdl,
			   struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_srng_access_start(hal_soc_hdl, hal_ring_hdl);
	reo_desc = hal_srng_src_get_next(hal_soc, hal_ring_hdl);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Out of cmd ring entries", __func__);
		hal_srng_access_end(hal_soc, hal_ring_hdl);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_FLUSH_QUEUE_E,
			sizeof(struct reo_flush_queue));

	/*
	 * Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((reo_desc + NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER),
		     sizeof(struct reo_flush_queue) -
		     (NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER << 2));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
			   REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr_li(reo_desc, CMD_FLUSH_QUEUE,
				      cmd->std.addr_lo, cmd->std.addr_hi);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_QUEUE_2,
			   BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH,
			   cmd->u.fl_queue_params.block_use_after_flush);

	if (cmd->u.fl_queue_params.block_use_after_flush) {
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_QUEUE_2,
				   BLOCK_RESOURCE_INDEX,
				   cmd->u.fl_queue_params.index);
	}

	hal_srng_access_end(hal_soc, hal_ring_hdl);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
			     val);
}

static inline int
hal_reo_cmd_flush_cache_li(hal_ring_handle_t hal_ring_hdl,
			   hal_soc_handle_t hal_soc_hdl,
			   struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;
	struct hal_reo_cmd_flush_cache_params *cp;
	uint8_t index = 0;
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	cp = &cmd->u.fl_cache_params;

	hal_srng_access_start(hal_soc_hdl, hal_ring_hdl);

	/* We need a cache block resource for this operation, and REO HW has
	 * only 4 such blocking resources. These resources are managed using
	 * reo_res_bitmap, and we return failure if none is available.
	 */
	if (cp->block_use_after_flush) {
		index = hal_find_zero_bit(hal_soc->reo_res_bitmap);
		if (index > 3) {
			qdf_print("No blocking resource available!");
			hal_srng_access_end(hal_soc, hal_ring_hdl);
			return -EBUSY;
		}
		hal_soc->index = index;
	}

	reo_desc = hal_srng_src_get_next(hal_soc, hal_ring_hdl);
	if (!reo_desc) {
		hal_srng_access_end(hal_soc, hal_ring_hdl);
		hal_srng_dump(hal_ring_handle_to_hal_srng(hal_ring_hdl));
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_FLUSH_CACHE_E,
			sizeof(struct reo_flush_cache));

	/*
	 * Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((reo_desc + NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER),
		     sizeof(struct reo_flush_cache) -
		     (NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER << 2));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
			   REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr_li(reo_desc, CMD_FLUSH_CACHE,
				      cmd->std.addr_lo, cmd->std.addr_hi);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2,
			   FORWARD_ALL_MPDUS_IN_QUEUE, cp->fwd_mpdus_in_queue);

	/* set it to 0 for now */
	cp->rel_block_index = 0;
	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2,
			   RELEASE_CACHE_BLOCK_INDEX, cp->rel_block_index);

	if (cp->block_use_after_flush) {
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2,
				   CACHE_BLOCK_RESOURCE_INDEX, index);
	}

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2,
			   FLUSH_WITHOUT_INVALIDATE, cp->flush_no_inval);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2,
			   BLOCK_CACHE_USAGE_AFTER_FLUSH,
			   cp->block_use_after_flush);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2, FLUSH_ENTIRE_CACHE,
			   cp->flush_entire_cache);

	if (hif_pm_runtime_get(hal_soc->hif_handle,
			       RTPM_ID_HAL_REO_CMD, false) == 0) {
		hal_srng_access_end(hal_soc_hdl, hal_ring_hdl);
		hif_pm_runtime_put(hal_soc->hif_handle,
				   RTPM_ID_HAL_REO_CMD);
	} else {
		hal_srng_access_end_reap(hal_soc_hdl, hal_ring_hdl);
		hal_srng_set_event(hal_ring_hdl, HAL_SRNG_FLUSH_EVENT);
		hal_srng_inc_flush_cnt(hal_ring_hdl);
	}

	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
			     val);
}

static inline int
hal_reo_cmd_unblock_cache_li(hal_ring_handle_t hal_ring_hdl,
			     hal_soc_handle_t hal_soc_hdl,
			     struct hal_reo_cmd_params *cmd)

{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	uint32_t *reo_desc, val;
	uint8_t index = 0;

	hal_srng_access_start(hal_soc_hdl, hal_ring_hdl);

	if (cmd->u.unblk_cache_params.type == UNBLOCK_RES_INDEX) {
		index = hal_find_one_bit(hal_soc->reo_res_bitmap);
		if (index > 3) {
			hal_srng_access_end(hal_soc, hal_ring_hdl);
			qdf_print("No blocking resource to unblock!");
			return -EBUSY;
		}
	}

	reo_desc = hal_srng_src_get_next(hal_soc, hal_ring_hdl);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Out of cmd ring entries", __func__);
		hal_srng_access_end(hal_soc, hal_ring_hdl);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_UNBLOCK_CACHE_E,
			sizeof(struct reo_unblock_cache));

	/*
	 * Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((reo_desc + NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER),
		     sizeof(struct reo_unblock_cache) -
		     (NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER << 2));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
			   REO_STATUS_REQUIRED, cmd->std.need_status);

	HAL_DESC_SET_FIELD(reo_desc, REO_UNBLOCK_CACHE_1,
			   UNBLOCK_TYPE, cmd->u.unblk_cache_params.type);

	if (cmd->u.unblk_cache_params.type == UNBLOCK_RES_INDEX) {
		HAL_DESC_SET_FIELD(reo_desc, REO_UNBLOCK_CACHE_1,
				   CACHE_BLOCK_RESOURCE_INDEX,
				   cmd->u.unblk_cache_params.index);
	}

	hal_srng_access_end(hal_soc, hal_ring_hdl);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
			     val);
}

static inline int
hal_reo_cmd_flush_timeout_list_li(hal_ring_handle_t hal_ring_hdl,
				  hal_soc_handle_t hal_soc_hdl,
				  struct hal_reo_cmd_params *cmd)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	uint32_t *reo_desc, val;

	hal_srng_access_start(hal_soc_hdl, hal_ring_hdl);
	reo_desc = hal_srng_src_get_next(hal_soc, hal_ring_hdl);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Out of cmd ring entries", __func__);
		hal_srng_access_end(hal_soc, hal_ring_hdl);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_FLUSH_TIMEOUT_LIST_E,
			sizeof(struct reo_flush_timeout_list));

	/*
	 * Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((reo_desc + NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER),
		     sizeof(struct reo_flush_timeout_list) -
		     (NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER << 2));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
			   REO_STATUS_REQUIRED, cmd->std.need_status);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_TIMEOUT_LIST_1, AC_TIMOUT_LIST,
			   cmd->u.fl_tim_list_params.ac_list);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_TIMEOUT_LIST_2,
			   MINIMUM_RELEASE_DESC_COUNT,
			   cmd->u.fl_tim_list_params.min_rel_desc);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_TIMEOUT_LIST_2,
			   MINIMUM_FORWARD_BUF_COUNT,
			   cmd->u.fl_tim_list_params.min_fwd_buf);

	hal_srng_access_end(hal_soc, hal_ring_hdl);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
			     val);
}

static inline int
hal_reo_cmd_update_rx_queue_li(hal_ring_handle_t hal_ring_hdl,
			       hal_soc_handle_t hal_soc_hdl,
			       struct hal_reo_cmd_params *cmd)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	uint32_t *reo_desc, val;
	struct hal_reo_cmd_update_queue_params *p;

	p = &cmd->u.upd_queue_params;

	hal_srng_access_start(hal_soc_hdl, hal_ring_hdl);
	reo_desc = hal_srng_src_get_next(hal_soc, hal_ring_hdl);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: Out of cmd ring entries", __func__);
		hal_srng_access_end(hal_soc, hal_ring_hdl);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_UPDATE_RX_REO_QUEUE_E,
			sizeof(struct reo_update_rx_reo_queue));

	/*
	 * Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((reo_desc + NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER),
		     sizeof(struct reo_update_rx_reo_queue) -
		     (NUM_OF_DWORDS_UNIFORM_REO_CMD_HEADER << 2));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
			   REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr_li(reo_desc, CMD_UPDATE_RX_REO_QUEUE,
				      cmd->std.addr_lo, cmd->std.addr_hi);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_RECEIVE_QUEUE_NUMBER, p->update_rx_queue_num);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2, UPDATE_VLD,
			   p->update_vld);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER,
			   p->update_assoc_link_desc);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_DISABLE_DUPLICATE_DETECTION,
			   p->update_disable_dup_detect);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_DISABLE_DUPLICATE_DETECTION,
			   p->update_disable_dup_detect);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_SOFT_REORDER_ENABLE,
			   p->update_soft_reorder_enab);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_AC, p->update_ac);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_BAR, p->update_bar);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_BAR, p->update_bar);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_RTY, p->update_rty);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_CHK_2K_MODE, p->update_chk_2k_mode);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_OOR_MODE, p->update_oor_mode);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_BA_WINDOW_SIZE, p->update_ba_window_size);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN_CHECK_NEEDED, p->update_pn_check_needed);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN_SHALL_BE_EVEN, p->update_pn_even);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN_SHALL_BE_UNEVEN, p->update_pn_uneven);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN_HANDLING_ENABLE, p->update_pn_hand_enab);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN_SIZE, p->update_pn_size);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_IGNORE_AMPDU_FLAG, p->update_ignore_ampdu);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_SVLD, p->update_svld);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_SSN, p->update_ssn);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_SEQ_2K_ERROR_DETECTED_FLAG,
			   p->update_seq_2k_err_detect);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN_VALID, p->update_pn_valid);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_2,
			   UPDATE_PN, p->update_pn);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   RECEIVE_QUEUE_NUMBER, p->rx_queue_num);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   VLD, p->vld);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   ASSOCIATED_LINK_DESCRIPTOR_COUNTER,
			   p->assoc_link_desc);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   DISABLE_DUPLICATE_DETECTION, p->disable_dup_detect);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   SOFT_REORDER_ENABLE, p->soft_reorder_enab);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3, AC, p->ac);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   BAR, p->bar);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   CHK_2K_MODE, p->chk_2k_mode);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   RTY, p->rty);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   OOR_MODE, p->oor_mode);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   PN_CHECK_NEEDED, p->pn_check_needed);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   PN_SHALL_BE_EVEN, p->pn_even);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   PN_SHALL_BE_UNEVEN, p->pn_uneven);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   PN_HANDLING_ENABLE, p->pn_hand_enab);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_3,
			   IGNORE_AMPDU_FLAG, p->ignore_ampdu);

	if (p->ba_window_size < 1)
		p->ba_window_size = 1;
	/*
	 * WAR to get 2k exception in Non BA case.
	 * Setting window size to 2 to get 2k jump exception
	 * when we receive aggregates in Non BA case
	 */
	if (p->ba_window_size == 1)
		p->ba_window_size++;
	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_4,
			   BA_WINDOW_SIZE, p->ba_window_size - 1);

	if (p->pn_size == 24)
		p->pn_size = PN_SIZE_24;
	else if (p->pn_size == 48)
		p->pn_size = PN_SIZE_48;
	else if (p->pn_size == 128)
		p->pn_size = PN_SIZE_128;

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_4,
			   PN_SIZE, p->pn_size);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_4,
			   SVLD, p->svld);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_4,
			   SSN, p->ssn);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_4,
			   SEQ_2K_ERROR_DETECTED_FLAG, p->seq_2k_err_detect);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_4,
			   PN_ERROR_DETECTED_FLAG, p->pn_err_detect);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_5,
			   PN_31_0, p->pn_31_0);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_6,
			   PN_63_32, p->pn_63_32);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_7,
			   PN_95_64, p->pn_95_64);

	HAL_DESC_SET_FIELD(reo_desc, REO_UPDATE_RX_REO_QUEUE_8,
			   PN_127_96, p->pn_127_96);

	if (hif_pm_runtime_get(hal_soc->hif_handle,
			       RTPM_ID_HAL_REO_CMD, false) == 0) {
		hal_srng_access_end(hal_soc_hdl, hal_ring_hdl);
		hif_pm_runtime_put(hal_soc->hif_handle,
				   RTPM_ID_HAL_REO_CMD);
	} else {
		hal_srng_access_end_reap(hal_soc_hdl, hal_ring_hdl);
		hal_srng_set_event(hal_ring_hdl, HAL_SRNG_FLUSH_EVENT);
		hal_srng_inc_flush_cnt(hal_ring_hdl);
	}

	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
			     val);
}

int hal_reo_send_cmd_li(hal_soc_handle_t hal_soc_hdl,
			hal_ring_handle_t  hal_ring_hdl,
			enum hal_reo_cmd_type cmd,
			void *params)
{
	struct hal_reo_cmd_params *cmd_params =
			(struct hal_reo_cmd_params *)params;
	int num = 0;

	switch (cmd) {
	case CMD_GET_QUEUE_STATS:
		num = hal_reo_cmd_queue_stats_li(hal_ring_hdl,
						 hal_soc_hdl, cmd_params);
		break;
	case CMD_FLUSH_QUEUE:
		num = hal_reo_cmd_flush_queue_li(hal_ring_hdl,
						 hal_soc_hdl, cmd_params);
		break;
	case CMD_FLUSH_CACHE:
		num = hal_reo_cmd_flush_cache_li(hal_ring_hdl,
						 hal_soc_hdl, cmd_params);
		break;
	case CMD_UNBLOCK_CACHE:
		num = hal_reo_cmd_unblock_cache_li(hal_ring_hdl,
						   hal_soc_hdl, cmd_params);
		break;
	case CMD_FLUSH_TIMEOUT_LIST:
		num = hal_reo_cmd_flush_timeout_list_li(hal_ring_hdl,
							hal_soc_hdl,
							cmd_params);
		break;
	case CMD_UPDATE_RX_REO_QUEUE:
		num = hal_reo_cmd_update_rx_queue_li(hal_ring_hdl,
						     hal_soc_hdl, cmd_params);
		break;
	default:
		hal_err("Invalid REO command type: %d", cmd);
		return -EINVAL;
	};

	return num;
}

void
hal_reo_queue_stats_status_li(hal_ring_desc_t ring_desc,
			      void *st_handle,
			      hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_queue_status *st =
		(struct hal_reo_queue_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;
	uint32_t val;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc, HAL_REO_QUEUE_STATS_STATUS_TLV,
				  &(st->header), hal_soc);

	/* SSN */
	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_2, SSN)];
	st->ssn = HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_2, SSN, val);

	/* current index */
	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_2,
					 CURRENT_INDEX)];
	st->curr_idx =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_2,
			      CURRENT_INDEX, val);

	/* PN bits */
	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_3,
					 PN_31_0)];
	st->pn_31_0 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_3,
			      PN_31_0, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_4,
					 PN_63_32)];
	st->pn_63_32 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_4,
			      PN_63_32, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_5,
					 PN_95_64)];
	st->pn_95_64 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_5,
			      PN_95_64, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_6,
					 PN_127_96)];
	st->pn_127_96 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_6,
			      PN_127_96, val);

	/* timestamps */
	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_7,
					 LAST_RX_ENQUEUE_TIMESTAMP)];
	st->last_rx_enq_tstamp =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_7,
			      LAST_RX_ENQUEUE_TIMESTAMP, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_8,
					 LAST_RX_DEQUEUE_TIMESTAMP)];
	st->last_rx_deq_tstamp =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_8,
			      LAST_RX_DEQUEUE_TIMESTAMP, val);

	/* rx bitmap */
	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_9,
					 RX_BITMAP_31_0)];
	st->rx_bitmap_31_0 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_9,
			      RX_BITMAP_31_0, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_10,
					 RX_BITMAP_63_32)];
	st->rx_bitmap_63_32 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_10,
			      RX_BITMAP_63_32, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_11,
					 RX_BITMAP_95_64)];
	st->rx_bitmap_95_64 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_11,
			      RX_BITMAP_95_64, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_12,
					 RX_BITMAP_127_96)];
	st->rx_bitmap_127_96 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_12,
			      RX_BITMAP_127_96, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_13,
					 RX_BITMAP_159_128)];
	st->rx_bitmap_159_128 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_13,
			      RX_BITMAP_159_128, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_14,
					 RX_BITMAP_191_160)];
	st->rx_bitmap_191_160 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_14,
			      RX_BITMAP_191_160, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_15,
					 RX_BITMAP_223_192)];
	st->rx_bitmap_223_192 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_15,
			      RX_BITMAP_223_192, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_16,
					 RX_BITMAP_255_224)];
	st->rx_bitmap_255_224 =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_16,
			      RX_BITMAP_255_224, val);

	/* various counts */
	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_17,
					 CURRENT_MPDU_COUNT)];
	st->curr_mpdu_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_17,
			      CURRENT_MPDU_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_17,
					 CURRENT_MSDU_COUNT)];
	st->curr_msdu_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_17,
			      CURRENT_MSDU_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_18,
					 TIMEOUT_COUNT)];
	st->fwd_timeout_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_18,
			      TIMEOUT_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_18,
					 FORWARD_DUE_TO_BAR_COUNT)];
	st->fwd_bar_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_18,
			      FORWARD_DUE_TO_BAR_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_18,
					 DUPLICATE_COUNT)];
	st->dup_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_18,
			      DUPLICATE_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_19,
					 FRAMES_IN_ORDER_COUNT)];
	st->frms_in_order_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_19,
			      FRAMES_IN_ORDER_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_19,
					 BAR_RECEIVED_COUNT)];
	st->bar_rcvd_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_19,
			      BAR_RECEIVED_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_20,
					 MPDU_FRAMES_PROCESSED_COUNT)];
	st->mpdu_frms_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_20,
			      MPDU_FRAMES_PROCESSED_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_21,
					 MSDU_FRAMES_PROCESSED_COUNT)];
	st->msdu_frms_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_21,
			      MSDU_FRAMES_PROCESSED_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_22,
					 TOTAL_PROCESSED_BYTE_COUNT)];
	st->total_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_22,
			      TOTAL_PROCESSED_BYTE_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_23,
					 LATE_RECEIVE_MPDU_COUNT)];
	st->late_recv_mpdu_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_23,
			      LATE_RECEIVE_MPDU_COUNT, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_23,
					 WINDOW_JUMP_2K)];
	st->win_jump_2k =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_23,
			      WINDOW_JUMP_2K, val);

	val = reo_desc[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_23,
					 HOLE_COUNT)];
	st->hole_cnt =
		HAL_GET_FIELD(REO_GET_QUEUE_STATS_STATUS_23,
			      HOLE_COUNT, val);
}

void
hal_reo_flush_queue_status_li(hal_ring_desc_t ring_desc,
			      void *st_handle,
			      hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_flush_queue_status *st =
			(struct hal_reo_flush_queue_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;
	uint32_t val;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc, HAL_REO_FLUSH_QUEUE_STATUS_TLV,
				  &(st->header), hal_soc);

	/* error bit */
	val = reo_desc[HAL_OFFSET(REO_FLUSH_QUEUE_STATUS_2,
					 ERROR_DETECTED)];
	st->error = HAL_GET_FIELD(REO_FLUSH_QUEUE_STATUS_2, ERROR_DETECTED,
				  val);
}

void
hal_reo_flush_cache_status_li(hal_ring_desc_t ring_desc,
			      void *st_handle,
			      hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_flush_cache_status *st =
			(struct hal_reo_flush_cache_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;
	uint32_t val;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc, HAL_REO_FLUSH_CACHE_STATUS_TLV,
				  &(st->header), hal_soc);

	/* error bit */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_2,
					 ERROR_DETECTED)];
	st->error = HAL_GET_FIELD(REO_FLUSH_QUEUE_STATUS_2, ERROR_DETECTED,
				  val);

	/* block error */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_2,
					 BLOCK_ERROR_DETAILS)];
	st->block_error = HAL_GET_FIELD(REO_FLUSH_CACHE_STATUS_2,
					BLOCK_ERROR_DETAILS,
					val);
	if (!st->block_error)
		qdf_set_bit(hal_soc->index,
			    (unsigned long *)&hal_soc->reo_res_bitmap);

	/* cache flush status */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_2,
				     CACHE_CONTROLLER_FLUSH_STATUS_HIT)];
	st->cache_flush_status = HAL_GET_FIELD(REO_FLUSH_CACHE_STATUS_2,
					CACHE_CONTROLLER_FLUSH_STATUS_HIT,
					val);

	/* cache flush descriptor type */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_2,
				  CACHE_CONTROLLER_FLUSH_STATUS_DESC_TYPE)];
	st->cache_flush_status_desc_type =
		HAL_GET_FIELD(REO_FLUSH_CACHE_STATUS_2,
			      CACHE_CONTROLLER_FLUSH_STATUS_DESC_TYPE,
			      val);

	/* cache flush count */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_2,
				  CACHE_CONTROLLER_FLUSH_COUNT)];
	st->cache_flush_cnt =
		HAL_GET_FIELD(REO_FLUSH_CACHE_STATUS_2,
			      CACHE_CONTROLLER_FLUSH_COUNT,
			      val);
}

void
hal_reo_unblock_cache_status_li(hal_ring_desc_t ring_desc,
				hal_soc_handle_t hal_soc_hdl,
				void *st_handle)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_unblk_cache_status *st =
			(struct hal_reo_unblk_cache_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;
	uint32_t val;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc, HAL_REO_UNBLK_CACHE_STATUS_TLV,
				  &st->header, hal_soc);

	/* error bit */
	val = reo_desc[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_2,
				  ERROR_DETECTED)];
	st->error = HAL_GET_FIELD(REO_UNBLOCK_CACHE_STATUS_2,
				  ERROR_DETECTED,
				  val);

	/* unblock type */
	val = reo_desc[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_2,
				  UNBLOCK_TYPE)];
	st->unblock_type = HAL_GET_FIELD(REO_UNBLOCK_CACHE_STATUS_2,
					 UNBLOCK_TYPE,
					 val);

	if (!st->error && (st->unblock_type == UNBLOCK_RES_INDEX))
		qdf_clear_bit(hal_soc->index,
			      (unsigned long *)&hal_soc->reo_res_bitmap);
}

void hal_reo_flush_timeout_list_status_li(hal_ring_desc_t ring_desc,
					  void *st_handle,
					  hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_flush_timeout_list_status *st =
			(struct hal_reo_flush_timeout_list_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;
	uint32_t val;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc, HAL_REO_TIMOUT_LIST_STATUS_TLV,
				  &(st->header), hal_soc);

	/* error bit */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_2,
					 ERROR_DETECTED)];
	st->error = HAL_GET_FIELD(REO_FLUSH_TIMEOUT_LIST_STATUS_2,
				  ERROR_DETECTED,
				  val);

	/* list empty */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_2,
					 TIMOUT_LIST_EMPTY)];
	st->list_empty = HAL_GET_FIELD(REO_FLUSH_TIMEOUT_LIST_STATUS_2,
				       TIMOUT_LIST_EMPTY,
				       val);

	/* release descriptor count */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_3,
					 RELEASE_DESC_COUNT)];
	st->rel_desc_cnt = HAL_GET_FIELD(REO_FLUSH_TIMEOUT_LIST_STATUS_3,
					 RELEASE_DESC_COUNT,
					 val);

	/* forward buf count */
	val = reo_desc[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_3,
				     FORWARD_BUF_COUNT)];
	st->fwd_buf_cnt = HAL_GET_FIELD(REO_FLUSH_TIMEOUT_LIST_STATUS_3,
					FORWARD_BUF_COUNT,
					val);
}

void hal_reo_desc_thres_reached_status_li(hal_ring_desc_t ring_desc,
					  void *st_handle,
					  hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_desc_thres_reached_status *st =
			(struct hal_reo_desc_thres_reached_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;
	uint32_t val;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc,
				  HAL_REO_DESC_THRES_STATUS_TLV,
				  &(st->header), hal_soc);

	/* threshold index */
	val = reo_desc[HAL_OFFSET_DW(
				 REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2,
				 THRESHOLD_INDEX)];
	st->thres_index = HAL_GET_FIELD(
				REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_2,
				THRESHOLD_INDEX,
				val);

	/* link desc counters */
	val = reo_desc[HAL_OFFSET_DW(
				 REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3,
				 LINK_DESCRIPTOR_COUNTER0)];
	st->link_desc_counter0 = HAL_GET_FIELD(
				REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_3,
				LINK_DESCRIPTOR_COUNTER0,
				val);

	val = reo_desc[HAL_OFFSET_DW(
				 REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4,
				 LINK_DESCRIPTOR_COUNTER1)];
	st->link_desc_counter1 = HAL_GET_FIELD(
				REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_4,
				LINK_DESCRIPTOR_COUNTER1,
				val);

	val = reo_desc[HAL_OFFSET_DW(
				 REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5,
				 LINK_DESCRIPTOR_COUNTER2)];
	st->link_desc_counter2 = HAL_GET_FIELD(
				REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_5,
				LINK_DESCRIPTOR_COUNTER2,
				val);

	val = reo_desc[HAL_OFFSET_DW(
				 REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6,
				 LINK_DESCRIPTOR_COUNTER_SUM)];
	st->link_desc_counter_sum = HAL_GET_FIELD(
				REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_6,
				LINK_DESCRIPTOR_COUNTER_SUM,
				val);
}

void
hal_reo_rx_update_queue_status_li(hal_ring_desc_t ring_desc,
				  void *st_handle,
				  hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct hal_reo_update_rx_queue_status *st =
			(struct hal_reo_update_rx_queue_status *)st_handle;
	uint32_t *reo_desc = (uint32_t *)ring_desc;

	/*
	 * Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	hal_reo_status_get_header(ring_desc,
				  HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV,
				  &(st->header), hal_soc);
}

uint8_t hal_get_tlv_hdr_size_li(void)
{
	return sizeof(struct tlv_32_hdr);
}

uint8_t *hal_rx_get_qdesc_addr_li(uint8_t *dst_ring_desc, uint8_t *buf)
{
	return dst_ring_desc +
		REO_DESTINATION_RING_6_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET;
}

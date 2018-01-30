/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#include "hal_reo.h"
#include "hal_tx.h"
#include "qdf_module.h"

#define BLOCK_RES_MASK		0xF
static inline uint8_t hal_find_one_bit(uint8_t x)
{
	uint8_t y = (x & (~x + 1)) & BLOCK_RES_MASK;
	uint8_t pos;

	for (pos = 0; y; y >>= 1)
		pos++;

	return pos-1;
}

static inline uint8_t hal_find_zero_bit(uint8_t x)
{
	uint8_t y = (~x & (x+1)) & BLOCK_RES_MASK;
	uint8_t pos;

	for (pos = 0; y; y >>= 1)
		pos++;

	return pos-1;
}

inline void hal_reo_cmd_set_descr_addr(uint32_t *reo_desc,
				       enum hal_reo_cmd_type type,
				       uint32_t paddr_lo,
				       uint8_t paddr_hi)
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
			"%s: Invalid REO command type\n", __func__);
		break;
	}
}

inline int hal_reo_cmd_queue_stats(void *reo_ring, struct hal_soc *soc,
				    struct hal_reo_cmd_params *cmd)

{
	uint32_t *reo_desc, val;

	hal_srng_access_start(soc, reo_ring);
	reo_desc = hal_srng_src_get_next(soc, reo_ring);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Out of cmd ring entries\n", __func__);
		hal_srng_access_end(soc, reo_ring);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_GET_QUEUE_STATS_E,
			     sizeof(struct reo_get_queue_stats));

	/* Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((void *)reo_desc, sizeof(struct reo_get_queue_stats));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
		REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr(reo_desc, CMD_GET_QUEUE_STATS,
				   cmd->std.addr_lo,
				   cmd->std.addr_hi);

	HAL_DESC_SET_FIELD(reo_desc, REO_GET_QUEUE_STATS_2, CLEAR_STATS,
			      cmd->u.stats_params.clear);

	hal_srng_access_end(soc, reo_ring);

	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
				     val);
}
qdf_export_symbol(hal_reo_cmd_queue_stats);

inline int hal_reo_cmd_flush_queue(void *reo_ring, struct hal_soc *soc,
				    struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;

	hal_srng_access_start(soc, reo_ring);
	reo_desc = hal_srng_src_get_next(soc, reo_ring);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Out of cmd ring entries\n", __func__);
		hal_srng_access_end(soc, reo_ring);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_FLUSH_QUEUE_E,
			     sizeof(struct reo_flush_queue));

	/* Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((void *)reo_desc, sizeof(struct reo_flush_queue));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
		REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr(reo_desc, CMD_FLUSH_QUEUE, cmd->std.addr_lo,
		cmd->std.addr_hi);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_QUEUE_2,
		BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH,
		cmd->u.fl_queue_params.block_use_after_flush);

	if (cmd->u.fl_queue_params.block_use_after_flush) {
		HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_QUEUE_2,
			BLOCK_RESOURCE_INDEX, cmd->u.fl_queue_params.index);
	}

	hal_srng_access_end(soc, reo_ring);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
				     val);
}
qdf_export_symbol(hal_reo_cmd_flush_queue);

inline int hal_reo_cmd_flush_cache(void *reo_ring, struct hal_soc *soc,
				    struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;
	struct hal_reo_cmd_flush_cache_params *cp;
	uint8_t index = 0;

	cp = &cmd->u.fl_cache_params;

	hal_srng_access_start(soc, reo_ring);

	/* We need a cache block resource for this operation, and REO HW has
	 * only 4 such blocking resources. These resources are managed using
	 * reo_res_bitmap, and we return failure if none is available.
	 */
	if (cp->block_use_after_flush) {
		index = hal_find_zero_bit(soc->reo_res_bitmap);
		if (index > 3) {
			qdf_print("%s, No blocking resource available!\n", __func__);
			hal_srng_access_end(soc, reo_ring);
			return -EBUSY;
		}
		soc->index = index;
	}

	reo_desc = hal_srng_src_get_next(soc, reo_ring);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Out of cmd ring entries\n", __func__);
		hal_srng_access_end(soc, reo_ring);
		hal_srng_dump(reo_ring);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_FLUSH_CACHE_E,
			     sizeof(struct reo_flush_cache));

	/* Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((void *)reo_desc, sizeof(struct reo_flush_cache));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
		REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr(reo_desc, CMD_FLUSH_CACHE, cmd->std.addr_lo,
				   cmd->std.addr_hi);

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
		BLOCK_CACHE_USAGE_AFTER_FLUSH, cp->block_use_after_flush);

	HAL_DESC_SET_FIELD(reo_desc, REO_FLUSH_CACHE_2, FLUSH_ENTIRE_CACHE,
		cp->flush_all);

	hal_srng_access_end(soc, reo_ring);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
				     val);
}
qdf_export_symbol(hal_reo_cmd_flush_cache);

inline int hal_reo_cmd_unblock_cache(void *reo_ring, struct hal_soc *soc,
				      struct hal_reo_cmd_params *cmd)

{
	uint32_t *reo_desc, val;
	uint8_t index = 0;

	hal_srng_access_start(soc, reo_ring);

	if (cmd->u.unblk_cache_params.type == UNBLOCK_RES_INDEX) {
		index = hal_find_one_bit(soc->reo_res_bitmap);
		if (index > 3) {
			hal_srng_access_end(soc, reo_ring);
			qdf_print("%s: No blocking resource to unblock!\n",
				  __func__);
			return -EBUSY;
		}
	}

	reo_desc = hal_srng_src_get_next(soc, reo_ring);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Out of cmd ring entries\n", __func__);
		hal_srng_access_end(soc, reo_ring);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_UNBLOCK_CACHE_E,
			     sizeof(struct reo_unblock_cache));

	/* Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((void *)reo_desc, sizeof(struct reo_unblock_cache));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
		REO_STATUS_REQUIRED, cmd->std.need_status);

	HAL_DESC_SET_FIELD(reo_desc, REO_UNBLOCK_CACHE_1,
		UNBLOCK_TYPE, cmd->u.unblk_cache_params.type);

	if (cmd->u.unblk_cache_params.type == UNBLOCK_RES_INDEX) {
		HAL_DESC_SET_FIELD(reo_desc, REO_UNBLOCK_CACHE_1,
			CACHE_BLOCK_RESOURCE_INDEX,
			cmd->u.unblk_cache_params.index);
	}

	hal_srng_access_end(soc, reo_ring);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
				     val);
}
qdf_export_symbol(hal_reo_cmd_unblock_cache);

inline int hal_reo_cmd_flush_timeout_list(void *reo_ring, struct hal_soc *soc,
					   struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;

	hal_srng_access_start(soc, reo_ring);
	reo_desc = hal_srng_src_get_next(soc, reo_ring);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Out of cmd ring entries\n", __func__);
		hal_srng_access_end(soc, reo_ring);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_FLUSH_TIMEOUT_LIST_E,
			     sizeof(struct reo_flush_timeout_list));

	/* Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((void *)reo_desc, sizeof(struct reo_flush_timeout_list));

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

	hal_srng_access_end(soc, reo_ring);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
				     val);
}
qdf_export_symbol(hal_reo_cmd_flush_timeout_list);

inline int hal_reo_cmd_update_rx_queue(void *reo_ring, struct hal_soc *soc,
					struct hal_reo_cmd_params *cmd)
{
	uint32_t *reo_desc, val;
	struct hal_reo_cmd_update_queue_params *p;

	p = &cmd->u.upd_queue_params;

	hal_srng_access_start(soc, reo_ring);
	reo_desc = hal_srng_src_get_next(soc, reo_ring);
	if (!reo_desc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Out of cmd ring entries\n", __func__);
		hal_srng_access_end(soc, reo_ring);
		return -EBUSY;
	}

	HAL_SET_TLV_HDR(reo_desc, WIFIREO_UPDATE_RX_REO_QUEUE_E,
			     sizeof(struct reo_update_rx_reo_queue));

	/* Offsets of descriptor fields defined in HW headers start from
	 * the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);
	qdf_mem_zero((void *)reo_desc, sizeof(struct reo_update_rx_reo_queue));

	HAL_DESC_SET_FIELD(reo_desc, UNIFORM_REO_CMD_HEADER_0,
		REO_STATUS_REQUIRED, cmd->std.need_status);

	hal_reo_cmd_set_descr_addr(reo_desc, CMD_UPDATE_RX_REO_QUEUE,
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

	hal_srng_access_end(soc, reo_ring);
	val = reo_desc[CMD_HEADER_DW_OFFSET];
	return HAL_GET_FIELD(UNIFORM_REO_CMD_HEADER_0, REO_CMD_NUMBER,
				     val);
}
qdf_export_symbol(hal_reo_cmd_update_rx_queue);

inline void hal_reo_queue_stats_status(uint32_t *reo_desc,
			     struct hal_reo_queue_status *st)
{
	uint32_t val;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc, REO_GET_QUEUE_STATS, st->header);

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
qdf_export_symbol(hal_reo_queue_stats_status);

inline void hal_reo_flush_queue_status(uint32_t *reo_desc,
				    struct hal_reo_flush_queue_status *st)
{
	uint32_t val;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc, REO_FLUSH_QUEUE, st->header);

	/* error bit */
	val = reo_desc[HAL_OFFSET(REO_FLUSH_QUEUE_STATUS_2,
					 ERROR_DETECTED)];
	st->error = HAL_GET_FIELD(REO_FLUSH_QUEUE_STATUS_2, ERROR_DETECTED,
				  val);
}
qdf_export_symbol(hal_reo_flush_queue_status);

inline void hal_reo_flush_cache_status(uint32_t *reo_desc, struct hal_soc *soc,
				    struct hal_reo_flush_cache_status *st)
{
	uint32_t val;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc, REO_FLUSH_CACHE, st->header);

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
		qdf_set_bit(soc->index, (unsigned long *)&soc->reo_res_bitmap);

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
qdf_export_symbol(hal_reo_flush_cache_status);

inline void hal_reo_unblock_cache_status(uint32_t *reo_desc,
					 struct hal_soc *soc,
					 struct hal_reo_unblk_cache_status *st)
{
	uint32_t val;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc, REO_UNBLOCK_CACHE, st->header);

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
		qdf_clear_bit(soc->index,
			     (unsigned long *)&soc->reo_res_bitmap);
}
qdf_export_symbol(hal_reo_unblock_cache_status);

inline void hal_reo_flush_timeout_list_status(
			 uint32_t *reo_desc,
			 struct hal_reo_flush_timeout_list_status *st)

{
	uint32_t val;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc, REO_FLUSH_TIMEOUT_LIST, st->header);

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
qdf_export_symbol(hal_reo_flush_timeout_list_status);

inline void hal_reo_desc_thres_reached_status(
			 uint32_t *reo_desc,
			 struct hal_reo_desc_thres_reached_status *st)
{
	uint32_t val;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc,
			      REO_DESCRIPTOR_THRESHOLD_REACHED, st->header);

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
qdf_export_symbol(hal_reo_desc_thres_reached_status);

inline void hal_reo_rx_update_queue_status(uint32_t *reo_desc,
				      struct hal_reo_update_rx_queue_status *st)
{
	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header */
	reo_desc += (sizeof(struct tlv_32_hdr) >> 2);

	/* header */
	HAL_REO_STATUS_GET_HEADER(reo_desc,
			      REO_UPDATE_RX_REO_QUEUE, st->header);
}
qdf_export_symbol(hal_reo_rx_update_queue_status);

/**
 * hal_reo_init_cmd_ring() - Initialize descriptors of REO command SRNG
 * with command number
 * @hal_soc: Handle to HAL SoC structure
 * @hal_ring: Handle to HAL SRNG structure
 *
 * Return: none
 */
inline void hal_reo_init_cmd_ring(struct hal_soc *soc, void *hal_srng)
{
	int cmd_num;
	uint32_t *desc_addr;
	struct hal_srng_params srng_params;
	uint32_t desc_size;
	uint32_t num_desc;

	hal_get_srng_params(soc, hal_srng, &srng_params);

	desc_addr = (uint32_t *)(srng_params.ring_base_vaddr);
	desc_addr += (sizeof(struct tlv_32_hdr) >> 2);
	desc_size = hal_srng_get_entrysize(soc, REO_CMD) >> 2;
	num_desc = srng_params.num_entries;
	cmd_num = 1;
	while (num_desc) {
		/* Offsets of descriptor fields defined in HW headers start
		 * from the field after TLV header */
		HAL_DESC_SET_FIELD(desc_addr, UNIFORM_REO_CMD_HEADER_0,
			REO_CMD_NUMBER, cmd_num);
		desc_addr += desc_size;
		num_desc--; cmd_num++;
	}

	soc->reo_res_bitmap = 0;
}
qdf_export_symbol(hal_reo_init_cmd_ring);

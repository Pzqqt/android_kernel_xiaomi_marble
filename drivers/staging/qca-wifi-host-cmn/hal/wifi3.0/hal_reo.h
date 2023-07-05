/*
 * Copyright (c) 2017-2019, 2021 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_REO_H_
#define _HAL_REO_H_

#include <qdf_types.h>
/* HW headers */
#include <reo_descriptor_threshold_reached_status.h>
#include <reo_flush_queue.h>
#include <reo_flush_timeout_list_status.h>
#include <reo_unblock_cache.h>
#include <reo_flush_cache.h>
#include <reo_flush_queue_status.h>
#include <reo_get_queue_stats.h>
#include <reo_unblock_cache_status.h>
#include <reo_flush_cache_status.h>
#include <reo_flush_timeout_list.h>
#include <reo_get_queue_stats_status.h>
#include <reo_update_rx_reo_queue.h>
#include <reo_update_rx_reo_queue_status.h>
#include <tlv_tag_def.h>

/* SW headers */
#include "hal_api.h"
#include "hal_rx_hw_defines.h"

/*---------------------------------------------------------------------------
  Preprocessor definitions and constants
  ---------------------------------------------------------------------------*/

/* TLV values */
#define HAL_REO_GET_QUEUE_STATS_TLV	WIFIREO_GET_QUEUE_STATS_E
#define HAL_REO_FLUSH_QUEUE_TLV		WIFIREO_FLUSH_QUEUE_E
#define HAL_REO_FLUSH_CACHE_TLV		WIFIREO_FLUSH_CACHE_E
#define HAL_REO_UNBLOCK_CACHE_TLV	WIFIREO_UNBLOCK_CACHE_E
#define HAL_REO_FLUSH_TIMEOUT_LIST_TLV	WIFIREO_FLUSH_TIMEOUT_LIST_E
#define HAL_REO_RX_UPDATE_QUEUE_TLV     WIFIREO_UPDATE_RX_REO_QUEUE_E

#define HAL_REO_QUEUE_STATS_STATUS_TLV	WIFIREO_GET_QUEUE_STATS_STATUS_E
#define HAL_REO_FLUSH_QUEUE_STATUS_TLV	WIFIREO_FLUSH_QUEUE_STATUS_E
#define HAL_REO_FLUSH_CACHE_STATUS_TLV	WIFIREO_FLUSH_CACHE_STATUS_E
#define HAL_REO_UNBLK_CACHE_STATUS_TLV	WIFIREO_UNBLOCK_CACHE_STATUS_E
#define HAL_REO_TIMOUT_LIST_STATUS_TLV	WIFIREO_FLUSH_TIMEOUT_LIST_STATUS_E
#define HAL_REO_DESC_THRES_STATUS_TLV	\
	WIFIREO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_E
#define HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV WIFIREO_UPDATE_RX_REO_QUEUE_STATUS_E

#define HAL_SET_FIELD(block, field, value) \
	((value << (block ## _ ## field ## _LSB)) &	\
	 (block ## _ ## field ## _MASK))

#define HAL_GET_FIELD(block, field, value)		\
	((value & (block ## _ ## field ## _MASK)) >>	\
	 (block ## _ ## field ## _LSB))

#define HAL_SET_TLV_HDR(desc, tag, len) \
	do {						\
		((struct tlv_32_hdr *) desc)->tlv_tag = tag;	\
		((struct tlv_32_hdr *) desc)->tlv_len = len;	\
	} while (0)

#define HAL_GET_TLV(desc)	(((struct tlv_32_hdr *) desc)->tlv_tag)

#define HAL_OFFSET_DW(_block, _field) (HAL_OFFSET(_block, _field) >> 2)
#define HAL_OFFSET_QW(_block, _field) (HAL_OFFSET(_block, _field) >> 3)
/* dword offsets in REO cmd TLV */
#define CMD_HEADER_DW_OFFSET	0

/* TODO: See if the following definition is available in HW headers */
#define HAL_REO_OWNED 4
#define HAL_REO_QUEUE_DESC 8

/* TODO: Using associated link desc counter 1 for Rx. Check with FW on
 * how these counters are assigned
 */
#define HAL_RX_LINK_DESC_CNTR 1
/* TODO: Following definition should be from HW headers */
#define HAL_DESC_REO_OWNED 4

#ifndef TID_TO_WME_AC
/**
 * enum hal_wme_access_category: Access category enums
 * @WME_AC_BE: best effort
 * @WME_AC_BK: background
 * @WME_AC_VI: video
 * @WME_AC_VO: voice
 */
enum hal_wme_access_category {
	WME_AC_BE,
	WME_AC_BK,
	WME_AC_VI,
	WME_AC_VO
};

#define TID_TO_WME_AC(_tid) ( \
	(((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE : \
	(((_tid) == 1) || ((_tid) == 2)) ? WME_AC_BK : \
	(((_tid) == 4) || ((_tid) == 5)) ? WME_AC_VI : \
	WME_AC_VO)
#endif
#define HAL_NON_QOS_TID 16

/**
 * enum reo_unblock_cache_type: Enum for unblock type in REO unblock command
 * @UNBLOCK_RES_INDEX: Unblock a block resource
 * @UNBLOCK_CACHE: Unblock cache
 */
enum reo_unblock_cache_type {
	UNBLOCK_RES_INDEX	= 0,
	UNBLOCK_CACHE		= 1
};

/**
 * enum reo_thres_index_reg: Enum for reo descriptor usage counter for
 *	which threshold status is being indicated.
 * @reo_desc_counter0_threshold: counter0 reached threshold
 * @reo_desc_counter1_threshold: counter1 reached threshold
 * @reo_desc_counter2_threshold: counter2 reached threshold
 * @reo_desc_counter_sum_threshold: Total count reached threshold
 */
enum reo_thres_index_reg {
	reo_desc_counter0_threshold = 0,
	reo_desc_counter1_threshold = 1,
	reo_desc_counter2_threshold = 2,
	reo_desc_counter_sum_threshold = 3
};

/**
 * enum reo_cmd_exec_status: Enum for execution status of REO command
 *
 * @HAL_REO_CMD_SUCCESS: Command has successfully be executed
 * @HAL_REO_CMD_BLOCKED: Command could not be executed as the queue or cache
 *	was blocked
 * @HAL_REO_CMD_FAILED: Command has encountered problems when executing, like
 *	the queue descriptor not being valid
 */
enum reo_cmd_exec_status {
	HAL_REO_CMD_SUCCESS = 0,
	HAL_REO_CMD_BLOCKED = 1,
	HAL_REO_CMD_FAILED = 2,
	HAL_REO_CMD_RESOURCE_BLOCKED = 3,
	HAL_REO_CMD_DRAIN = 0xff
};

/**
 * struct hal_reo_cmd_params_std: Standard REO command parameters
 * @need_status: Status required for the command
 * @addr_lo: Lower 32 bits of REO queue descriptor address
 * @addr_hi: Upper 8 bits of REO queue descriptor address
 */
struct hal_reo_cmd_params_std {
	bool need_status;
	uint32_t addr_lo;
	uint8_t addr_hi;
};

/**
 * struct hal_reo_cmd_get_queue_stats_params: Parameters to
 *	CMD_GET_QUEUE_STATScommand
 * @clear: Clear stats after retreiving
 */
struct hal_reo_cmd_get_queue_stats_params {
	bool clear;
};

/**
 * struct hal_reo_cmd_flush_queue_params: Parameters to CMD_FLUSH_QUEUE
 * @use_after_flush: Block usage after flush till unblock command
 * @index: Blocking resource to be used
 */
struct hal_reo_cmd_flush_queue_params {
	bool block_use_after_flush;
	uint8_t index;
};

/**
 * struct hal_reo_cmd_flush_cache_params: Parameters to CMD_FLUSH_CACHE
 * @fwd_mpdus_in_queue: Forward MPDUs before flushing descriptor
 * @rel_block_index: Release blocking resource used earlier
 * @cache_block_res_index: Blocking resource to be used
 * @flush_no_inval: Flush without invalidatig descriptor
 * @use_after_flush: Block usage after flush till unblock command
 * @flush_entire_cache: Flush entire REO cache
 */
struct hal_reo_cmd_flush_cache_params {
	bool fwd_mpdus_in_queue;
	bool rel_block_index;
	uint8_t cache_block_res_index;
	bool flush_no_inval;
	bool block_use_after_flush;
	bool flush_entire_cache;
};

/**
 * struct hal_reo_cmd_unblock_cache_params: Parameters to CMD_UNBLOCK_CACHE
 * @type: Unblock type (enum reo_unblock_cache_type)
 * @index: Blocking index to be released
 */
struct hal_reo_cmd_unblock_cache_params {
	enum reo_unblock_cache_type type;
	uint8_t index;
};

/**
 * struct hal_reo_cmd_flush_timeout_list_params: Parameters to
 *		CMD_FLUSH_TIMEOUT_LIST
 * @ac_list: AC timeout list to be flushed
 * @min_rel_desc: Min. number of link descriptors to be release
 * @min_fwd_buf: Min. number of buffers to be forwarded
 */
struct hal_reo_cmd_flush_timeout_list_params {
	uint8_t ac_list;
	uint16_t min_rel_desc;
	uint16_t min_fwd_buf;
};

/**
 * struct hal_reo_cmd_update_queue_params: Parameters to CMD_UPDATE_RX_REO_QUEUE
 * @update_rx_queue_num: Update receive queue number
 * @update_vld: Update valid bit
 * @update_assoc_link_desc: Update associated link descriptor
 * @update_disable_dup_detect: Update duplicate detection
 * @update_soft_reorder_enab: Update soft reorder enable
 * @update_ac: Update access category
 * @update_bar: Update BAR received bit
 * @update_rty: Update retry bit
 * @update_chk_2k_mode: Update chk_2k_mode setting
 * @update_oor_mode: Update OOR mode setting
 * @update_ba_window_size: Update BA window size
 * @update_pn_check_needed: Update pn_check_needed
 * @update_pn_even: Update pn_even
 * @update_pn_uneven: Update pn_uneven
 * @update_pn_hand_enab: Update pn_handling_enable
 * @update_pn_size: Update pn_size
 * @update_ignore_ampdu: Update ignore_ampdu
 * @update_svld: update svld
 * @update_ssn: Update SSN
 * @update_seq_2k_err_detect: Update seq_2k_err_detected flag
 * @update_pn_err_detect: Update pn_err_detected flag
 * @update_pn_valid: Update pn_valid
 * @update_pn: Update PN
 * @rx_queue_num: rx_queue_num to be updated
 * @vld: valid bit to be updated
 * @assoc_link_desc: assoc_link_desc counter
 * @disable_dup_detect: disable_dup_detect to be updated
 * @soft_reorder_enab: soft_reorder_enab to be updated
 * @ac: AC to be updated
 * @bar: BAR flag to be updated
 * @rty: RTY flag to be updated
 * @chk_2k_mode: check_2k_mode setting to be updated
 * @oor_mode: oor_mode to be updated
 * @pn_check_needed: pn_check_needed to be updated
 * @pn_even: pn_even to be updated
 * @pn_uneven: pn_uneven to be updated
 * @pn_hand_enab: pn_handling_enable to be updated
 * @ignore_ampdu: ignore_ampdu to be updated
 * @ba_window_size: BA window size to be updated
 * @pn_size: pn_size to be updated
 * @svld: svld flag to be updated
 * @ssn: SSN to be updated
 * @seq_2k_err_detect: seq_2k_err_detected flag to be updated
 * @pn_err_detect: pn_err_detected flag to be updated
 * @pn_31_0: PN bits 31-0
 * @pn_63_32: PN bits 63-32
 * @pn_95_64: PN bits 95-64
 * @pn_127_96: PN bits 127-96
 */
struct hal_reo_cmd_update_queue_params {
	uint32_t update_rx_queue_num:1,
		update_vld:1,
		update_assoc_link_desc:1,
		update_disable_dup_detect:1,
		update_soft_reorder_enab:1,
		update_ac:1,
		update_bar:1,
		update_rty:1,
		update_chk_2k_mode:1,
		update_oor_mode:1,
		update_ba_window_size:1,
		update_pn_check_needed:1,
		update_pn_even:1,
		update_pn_uneven:1,
		update_pn_hand_enab:1,
		update_pn_size:1,
		update_ignore_ampdu:1,
		update_svld:1,
		update_ssn:1,
		update_seq_2k_err_detect:1,
		update_pn_err_detect:1,
		update_pn_valid:1,
		update_pn:1;
	uint32_t rx_queue_num:16,
		vld:1,
		assoc_link_desc:2,
		disable_dup_detect:1,
		soft_reorder_enab:1,
		ac:2,
		bar:1,
		rty:1,
		chk_2k_mode:1,
		oor_mode:1,
		pn_check_needed:1,
		pn_even:1,
		pn_uneven:1,
		pn_hand_enab:1,
		ignore_ampdu:1;
	uint32_t ba_window_size:9,
		pn_size:8,
		svld:1,
		ssn:12,
		seq_2k_err_detect:1,
		pn_err_detect:1;
	uint32_t pn_31_0:32;
	uint32_t pn_63_32:32;
	uint32_t pn_95_64:32;
	uint32_t pn_127_96:32;
};

/**
 * struct hal_reo_cmd_params: Common structure to pass REO command parameters
 * @hal_reo_cmd_params_std: Standard parameters
 * @u: Union of various REO command parameters
 */
struct hal_reo_cmd_params {
	struct hal_reo_cmd_params_std std;
	union {
		struct hal_reo_cmd_get_queue_stats_params stats_params;
		struct hal_reo_cmd_flush_queue_params fl_queue_params;
		struct hal_reo_cmd_flush_cache_params fl_cache_params;
		struct hal_reo_cmd_unblock_cache_params unblk_cache_params;
		struct hal_reo_cmd_flush_timeout_list_params fl_tim_list_params;
		struct hal_reo_cmd_update_queue_params upd_queue_params;
	} u;
};

/**
 * struct hal_reo_status_header: Common REO status header
 * @cmd_num: Command number
 * @exec_time: execution time
 * @status: command execution status
 * @tstamp: Timestamp of status updated
 */
struct hal_reo_status_header {
	uint16_t cmd_num;
	uint16_t exec_time;
	enum reo_cmd_exec_status status;
	uint32_t tstamp;
};

/**
 * struct hal_reo_queue_status: REO queue status structure
 * @header: Common REO status header
 * @ssn: SSN of current BA window
 * @curr_idx: last forwarded pkt
 * @pn_31_0, pn_63_32, pn_95_64, pn_127_96:
 *	PN number bits extracted from IV field
 * @last_rx_enq_tstamp: Last enqueue timestamp
 * @last_rx_deq_tstamp: Last dequeue timestamp
 * @rx_bitmap_31_0, rx_bitmap_63_32, rx_bitmap_95_64
 * @rx_bitmap_127_96, rx_bitmap_159_128, rx_bitmap_191_160
 * @rx_bitmap_223_192, rx_bitmap_255_224: Each bit corresonds to a frame
 *	held in re-order queue
 * @curr_mpdu_cnt, curr_msdu_cnt: Number of MPDUs and MSDUs in the queue
 * @fwd_timeout_cnt: Frames forwarded due to timeout
 * @fwd_bar_cnt: Frames forwarded BAR frame
 * @dup_cnt: duplicate frames detected
 * @frms_in_order_cnt: Frames received in order
 * @bar_rcvd_cnt: BAR frame count
 * @mpdu_frms_cnt, msdu_frms_cnt, total_cnt: MPDU, MSDU, total frames
	processed by REO
 * @late_recv_mpdu_cnt; received after window had moved on
 * @win_jump_2k: 2K jump count
 * @hole_cnt: sequence hole count
 */
struct hal_reo_queue_status {
	struct hal_reo_status_header header;
	uint16_t ssn;
	uint8_t curr_idx;
	uint32_t pn_31_0, pn_63_32, pn_95_64, pn_127_96;
	uint32_t last_rx_enq_tstamp, last_rx_deq_tstamp;
	uint32_t rx_bitmap_31_0, rx_bitmap_63_32, rx_bitmap_95_64;
	uint32_t rx_bitmap_127_96, rx_bitmap_159_128, rx_bitmap_191_160;
	uint32_t rx_bitmap_223_192, rx_bitmap_255_224;
	uint8_t curr_mpdu_cnt, curr_msdu_cnt;
	uint8_t fwd_timeout_cnt, fwd_bar_cnt;
	uint16_t dup_cnt;
	uint32_t frms_in_order_cnt;
	uint8_t bar_rcvd_cnt;
	uint32_t mpdu_frms_cnt, msdu_frms_cnt, total_cnt;
	uint16_t late_recv_mpdu_cnt;
	uint8_t win_jump_2k;
	uint16_t hole_cnt;
};

/**
 * struct hal_reo_flush_queue_status: FLUSH_QUEUE status structure
 * @header: Common REO status header
 * @error: Error detected
 */
struct hal_reo_flush_queue_status {
	struct hal_reo_status_header header;
	bool error;
};

/**
 * struct hal_reo_flush_cache_status: FLUSH_CACHE status structure
 * @header: Common REO status header
 * @error: Error detected
 * @block_error: Blocking related error
 * @cache_flush_status: Cache hit/miss
 * @cache_flush_status_desc_type: type of descriptor flushed
 * @cache_flush_cnt: number of lines actually flushed
 */
struct hal_reo_flush_cache_status {
	struct hal_reo_status_header header;
	bool error;
	uint8_t block_error;
	bool cache_flush_status;
	uint8_t cache_flush_status_desc_type;
	uint8_t cache_flush_cnt;
};

/**
 * struct hal_reo_unblk_cache_status: UNBLOCK_CACHE status structure
 * @header: Common REO status header
 * @error: error detected
 * unblock_type: resoure or cache
 */
struct hal_reo_unblk_cache_status {
	struct hal_reo_status_header header;
	bool error;
	enum reo_unblock_cache_type unblock_type;
};

/**
 * struct hal_reo_flush_timeout_list_status: FLUSH_TIMEOUT_LIST status structure
 * @header: Common REO status header
 * @error: error detected
 * @list_empty: timeout list empty
 * @rel_desc_cnt: number of link descriptors released
 * @fwd_buf_cnt: number of buffers forwarded to REO destination ring
 */
struct hal_reo_flush_timeout_list_status {
	struct hal_reo_status_header header;
	bool error;
	bool list_empty;
	uint16_t rel_desc_cnt;
	uint16_t fwd_buf_cnt;
};

/**
 * struct hal_reo_desc_thres_reached_status: desc_thres_reached status structure
 * @header: Common REO status header
 * @thres_index: Index of descriptor threshold counter
 * @link_desc_counter0, link_desc_counter1, link_desc_counter2: descriptor
 *	counter values
 * @link_desc_counter_sum: overall descriptor count
 */
struct hal_reo_desc_thres_reached_status {
	struct hal_reo_status_header header;
	enum reo_thres_index_reg thres_index;
	uint32_t link_desc_counter0, link_desc_counter1, link_desc_counter2;
	uint32_t link_desc_counter_sum;
};

/**
 * struct hal_reo_update_rx_queue_status: UPDATE_RX_QUEUE status structure
 * @header: Common REO status header
 */
struct hal_reo_update_rx_queue_status {
	struct hal_reo_status_header header;
};

/**
 * union hal_reo_status: Union to pass REO status to callbacks
 * @queue_status: Refer to struct hal_reo_queue_status
 * @fl_cache_status: Refer to struct hal_reo_flush_cache_status
 * @fl_queue_status: Refer to struct hal_reo_flush_queue_status
 * @fl_timeout_status: Refer to struct hal_reo_flush_timeout_list_status
 * @unblk_cache_status: Refer to struct hal_reo_unblk_cache_status
 * @thres_status: struct hal_reo_desc_thres_reached_status
 * @rx_queue_status: struct hal_reo_update_rx_queue_status
 */
union hal_reo_status {
	struct hal_reo_queue_status queue_status;
	struct hal_reo_flush_cache_status fl_cache_status;
	struct hal_reo_flush_queue_status fl_queue_status;
	struct hal_reo_flush_timeout_list_status fl_timeout_status;
	struct hal_reo_unblk_cache_status unblk_cache_status;
	struct hal_reo_desc_thres_reached_status thres_status;
	struct hal_reo_update_rx_queue_status rx_queue_status;
};

#ifdef HAL_DISABLE_NON_BA_2K_JUMP_ERROR
static inline uint32_t hal_update_non_ba_win_size(int tid,
						  uint32_t ba_window_size)
{
	return ba_window_size;
}
#else
static inline uint32_t hal_update_non_ba_win_size(int tid,
						  uint32_t ba_window_size)
{
	if ((ba_window_size == 1) && (tid != HAL_NON_QOS_TID))
		ba_window_size++;

	return ba_window_size;
}
#endif

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

/* REO command ring routines */

/**
 * hal_uniform_desc_hdr_setup - setup reo_queue_ext descritpro
 * @owner - owner info
 * @buffer_type - buffer type
 */
static inline void
hal_uniform_desc_hdr_setup(uint32_t *desc, uint32_t owner, uint32_t buffer_type)
{
	HAL_DESC_SET_FIELD(desc, HAL_UNIFORM_DESCRIPTOR_HEADER, OWNER,
			   owner);
	HAL_DESC_SET_FIELD(desc, HAL_UNIFORM_DESCRIPTOR_HEADER, BUFFER_TYPE,
			   buffer_type);
}

/**
 * hal_reo_send_cmd() - Send reo cmd using the params provided.
 * @hal_soc_hdl: HAL soc handle
 * @hal_soc_hdl: srng handle
 * @cmd: cmd ID
 * @cmd_params: command params
 *
 * Return: cmd number
 */
static inline int
hal_reo_send_cmd(hal_soc_handle_t hal_soc_hdl,
		 hal_ring_handle_t  hal_ring_hdl,
		 enum hal_reo_cmd_type cmd,
		 struct hal_reo_cmd_params *cmd_params)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return -EINVAL;
	}

	if (hal_soc->ops->hal_reo_send_cmd)
		return hal_soc->ops->hal_reo_send_cmd(hal_soc_hdl, hal_ring_hdl,
						      cmd, cmd_params);

	return -EINVAL;
}

static inline QDF_STATUS
hal_reo_status_update(hal_soc_handle_t hal_soc_hdl,
		      hal_ring_desc_t reo_desc, void *st_handle,
		      uint32_t tlv, int *num_ref)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_reo_send_cmd)
		return hal_soc->ops->hal_reo_status_update(hal_soc_hdl,
							   reo_desc,
							   st_handle,
							   tlv, num_ref);
	return QDF_STATUS_E_FAILURE;
}

/* REO Status ring routines */
static inline void hal_reo_qdesc_setup(hal_soc_handle_t hal_soc_hdl, int tid,
				       uint32_t ba_window_size,
			 uint32_t start_seq, void *hw_qdesc_vaddr,
			 qdf_dma_addr_t hw_qdesc_paddr,
			 int pn_type, uint8_t vdev_stats_id)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return;
	}

	if (hal_soc->ops->hal_reo_qdesc_setup)
		hal_soc->ops->hal_reo_qdesc_setup(hal_soc_hdl, tid,
						  ba_window_size, start_seq,
						  hw_qdesc_vaddr,
						  hw_qdesc_paddr, pn_type,
						  vdev_stats_id);
}

/**
 * hal_get_ba_aging_timeout - Retrieve BA aging timeout
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ac: Access category
 * @value: timeout duration in millisec
 */
static inline void hal_get_ba_aging_timeout(hal_soc_handle_t hal_soc_hdl,
					    uint8_t ac,
					    uint32_t *value)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_get_ba_aging_timeout(hal_soc_hdl, ac, value);
}

/**
 * hal_set_aging_timeout - Set BA aging timeout
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ac: Access category in millisec
 * @value: timeout duration value
 */
static inline void hal_set_ba_aging_timeout(hal_soc_handle_t hal_soc_hdl,
					    uint8_t ac,
					    uint32_t value)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_set_ba_aging_timeout(hal_soc_hdl, ac, value);
}

/**
 * hal_get_reo_reg_base_offset() - Get REO register base offset
 * @hal_soc_hdl: HAL soc handle
 *
 * Return: REO register base
 */
static inline uint32_t hal_get_reo_reg_base_offset(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_get_reo_reg_base_offset();
}

static inline uint32_t
hal_gen_reo_remap_val(hal_soc_handle_t hal_soc_hdl,
		      enum hal_reo_remap_reg remap_reg,
		      uint8_t *ix0_map)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_gen_reo_remap_val)
		return hal_soc->ops->hal_gen_reo_remap_val(remap_reg, ix0_map);

	return 0;
}

static inline uint8_t
hal_get_tlv_hdr_size(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_get_tlv_hdr_size)
		return hal_soc->ops->hal_get_tlv_hdr_size();

	return 0;
}
/* Function Proto-types */

/**
 * hal_reo_init_cmd_ring() - Initialize descriptors of REO command SRNG
 * with command number
 * @hal_soc: Handle to HAL SoC structure
 * @hal_ring: Handle to HAL SRNG structure
 * Return: none
 */
void hal_reo_init_cmd_ring(hal_soc_handle_t hal_soc_hdl,
			   hal_ring_handle_t hal_ring_hdl);
#endif /* _HAL_REO_H */

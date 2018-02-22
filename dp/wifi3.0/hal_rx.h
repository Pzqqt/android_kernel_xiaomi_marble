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

#ifndef _HAL_RX_H_
#define _HAL_RX_H_

#include <hal_internal.h>

/**
 * struct hal_wbm_err_desc_info: structure to hold wbm error codes and reasons
 *
 * @reo_psh_rsn:	REO push reason
 * @reo_err_code:	REO Error code
 * @rxdma_psh_rsn:	RXDMA push reason
 * @rxdma_err_code:	RXDMA Error code
 * @reserved_1:		Reserved bits
 * @wbm_err_src:	WBM error source
 * @pool_id:		pool ID, indicates which rxdma pool
 * @reserved_2:		Reserved bits
 */
struct hal_wbm_err_desc_info {
	uint16_t reo_psh_rsn:2,
		 reo_err_code:5,
		 rxdma_psh_rsn:2,
		 rxdma_err_code:5,
		 reserved_1:2;
	uint8_t wbm_err_src:3,
		pool_id:2,
		reserved_2:3;
};

/**
 * enum hal_reo_error_code: Enum which encapsulates "reo_push_reason"
 *
 * @ HAL_REO_ERROR_DETECTED: Packets arrived because of an error detected
 * @ HAL_REO_ROUTING_INSTRUCTION: Packets arrived because of REO routing
 */
enum hal_reo_error_status {
	HAL_REO_ERROR_DETECTED = 0,
	HAL_REO_ROUTING_INSTRUCTION = 1,
};

/**
 * @msdu_flags: [0] first_msdu_in_mpdu
 *              [1] last_msdu_in_mpdu
 *              [2] msdu_continuation - MSDU spread across buffers
 *             [23] sa_is_valid - SA match in peer table
 *             [24] sa_idx_timeout - Timeout while searching for SA match
 *             [25] da_is_valid - Used to identtify intra-bss forwarding
 *             [26] da_is_MCBC
 *             [27] da_idx_timeout - Timeout while searching for DA match
 *
 */
struct hal_rx_msdu_desc_info {
	uint32_t msdu_flags;
	uint16_t msdu_len; /* 14 bits for length */
};

/**
 * enum hal_rx_msdu_desc_flags: Enum for flags in MSDU_DESC_INFO
 *
 * @ HAL_MSDU_F_FIRST_MSDU_IN_MPDU: First MSDU in MPDU
 * @ HAL_MSDU_F_LAST_MSDU_IN_MPDU: Last MSDU in MPDU
 * @ HAL_MSDU_F_MSDU_CONTINUATION: MSDU continuation
 * @ HAL_MSDU_F_SA_IS_VALID: Found match for SA in AST
 * @ HAL_MSDU_F_SA_IDX_TIMEOUT: AST search for SA timed out
 * @ HAL_MSDU_F_DA_IS_VALID: Found match for DA in AST
 * @ HAL_MSDU_F_DA_IS_MCBC: DA is MC/BC address
 * @ HAL_MSDU_F_DA_IDX_TIMEOUT: AST search for DA timed out
 */
enum hal_rx_msdu_desc_flags {
	HAL_MSDU_F_FIRST_MSDU_IN_MPDU = (0x1 << 0),
	HAL_MSDU_F_LAST_MSDU_IN_MPDU = (0x1 << 1),
	HAL_MSDU_F_MSDU_CONTINUATION = (0x1 << 2),
	HAL_MSDU_F_SA_IS_VALID = (0x1 << 23),
	HAL_MSDU_F_SA_IDX_TIMEOUT = (0x1 << 24),
	HAL_MSDU_F_DA_IS_VALID = (0x1 << 25),
	HAL_MSDU_F_DA_IS_MCBC = (0x1 << 26),
	HAL_MSDU_F_DA_IDX_TIMEOUT = (0x1 << 27)
};

/*
 * @msdu_count:		no. of msdus in the MPDU
 * @mpdu_seq:		MPDU sequence number
 * @mpdu_flags          [0] Fragment flag
 *                      [1] MPDU_retry_bit
 *                      [2] AMPDU flag
 *			[3] raw_ampdu
 * @peer_meta_data:	Upper bits containing peer id, vdev id
 */
struct hal_rx_mpdu_desc_info {
	uint16_t msdu_count;
	uint16_t mpdu_seq; /* 12 bits for length */
	uint32_t mpdu_flags;
	uint32_t peer_meta_data; /* sw progamed meta-data:MAC Id & peer Id */
};

/**
 * enum hal_rx_mpdu_desc_flags: Enum for flags in MPDU_DESC_INFO
 *
 * @ HAL_MPDU_F_FRAGMENT: Fragmented MPDU (802.11 fragemtation)
 * @ HAL_MPDU_F_RETRY_BIT: Retry bit is set in FC of MPDU
 * @ HAL_MPDU_F_AMPDU_FLAG: MPDU received as part of A-MPDU
 * @ HAL_MPDU_F_RAW_AMPDU: MPDU is a Raw MDPU
 */
enum hal_rx_mpdu_desc_flags {
	HAL_MPDU_F_FRAGMENT = (0x1 << 20),
	HAL_MPDU_F_RETRY_BIT = (0x1 << 21),
	HAL_MPDU_F_AMPDU_FLAG = (0x1 << 22),
	HAL_MPDU_F_RAW_AMPDU = (0x1 << 30)
};

/**
 * enum hal_rx_ret_buf_manager: Enum for return_buffer_manager field in
 *				BUFFER_ADDR_INFO structure
 *
 * @ HAL_RX_BUF_RBM_WBM_IDLE_BUF_LIST: Buffer returned to WBM idle buffer list
 * @ HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST: Descriptor returned to WBM idle
 *					descriptor list
 * @ HAL_RX_BUF_RBM_FW_BM: Buffer returned to FW
 * @ HAL_RX_BUF_RBM_SW0_BM: For Tx completion -- returned to host
 * @ HAL_RX_BUF_RBM_SW1_BM: For Tx completion -- returned to host
 * @ HAL_RX_BUF_RBM_SW2_BM: For Tx completion -- returned to host
 * @ HAL_RX_BUF_RBM_SW3_BM: For Rx release -- returned to host
 */
enum hal_rx_ret_buf_manager {
	HAL_RX_BUF_RBM_WBM_IDLE_BUF_LIST = 0,
	HAL_RX_BUF_RBM_WBM_IDLE_DESC_LIST = 1,
	HAL_RX_BUF_RBM_FW_BM = 2,
	HAL_RX_BUF_RBM_SW0_BM = 3,
	HAL_RX_BUF_RBM_SW1_BM = 4,
	HAL_RX_BUF_RBM_SW2_BM = 5,
	HAL_RX_BUF_RBM_SW3_BM = 6,
};

/*
 * Given the offset of a field in bytes, returns uint8_t *
 */
#define _OFFSET_TO_BYTE_PTR(_ptr, _off_in_bytes)	\
	(((uint8_t *)(_ptr)) + (_off_in_bytes))

/*
 * Given the offset of a field in bytes, returns uint32_t *
 */
#define _OFFSET_TO_WORD_PTR(_ptr, _off_in_bytes)	\
	(((uint32_t *)(_ptr)) + ((_off_in_bytes) >> 2))

#define _HAL_MS(_word, _mask, _shift)		\
	(((_word) & (_mask)) >> (_shift))

/*
 * macro to set the LSW of the nbuf data physical address
 * to the rxdma ring entry
 */
#define HAL_RXDMA_PADDR_LO_SET(buff_addr_info, paddr_lo) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_OFFSET >> 2))) = \
		(paddr_lo << BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB) & \
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK)

/*
 * macro to set the LSB of MSW of the nbuf data physical address
 * to the rxdma ring entry
 */
#define HAL_RXDMA_PADDR_HI_SET(buff_addr_info, paddr_hi) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_OFFSET >> 2))) = \
		(paddr_hi << BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB) & \
		BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK)

/*
 * macro to set the cookie into the rxdma ring entry
 */
#define HAL_RXDMA_COOKIE_SET(buff_addr_info, cookie) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) &= \
		~BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK); \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) |= \
		(cookie << BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB) & \
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK)

/*
 * macro to set the LSW of the nbuf data physical address
 * to the WBM ring entry
 */
#define HAL_WBM_PADDR_LO_SET(buff_addr_info, paddr_lo) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_OFFSET >> 2))) = \
		(paddr_lo << BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB) & \
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK)

/*
 * macro to set the LSB of MSW of the nbuf data physical address
 * to the WBM ring entry
 */
#define HAL_WBM_PADDR_HI_SET(buff_addr_info, paddr_hi) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_OFFSET >> 2))) = \
		(paddr_hi << BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB) & \
		BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK)

/*
 * macro to set the manager into the rxdma ring entry
 */
#define HAL_RXDMA_MANAGER_SET(buff_addr_info, manager) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET >> 2))) &= \
		~BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK); \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET >> 2))) |= \
		(manager << BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB) & \
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK)

#define HAL_RX_ERROR_STATUS_GET(reo_desc)			\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(reo_desc,		\
		REO_DESTINATION_RING_7_REO_PUSH_REASON_OFFSET)),\
		REO_DESTINATION_RING_7_REO_PUSH_REASON_MASK,	\
		REO_DESTINATION_RING_7_REO_PUSH_REASON_LSB))

#define HAL_RX_BUF_COOKIE_GET(buff_addr_info)			\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,		\
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET)),	\
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK,	\
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB))

#define HAL_RX_BUFFER_ADDR_39_32_GET(buff_addr_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,		\
		BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_OFFSET)),	\
		BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK,	\
		BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB))

#define HAL_RX_BUFFER_ADDR_31_0_GET(buff_addr_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,		\
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_OFFSET)),	\
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK,	\
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB))

#define HAL_RX_BUF_RBM_GET(buff_addr_info)			\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,		\
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET)),\
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK,	\
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB))

/* TODO: Convert the following structure fields accesseses to offsets */

#define HAL_RX_REO_BUFFER_ADDR_39_32_GET(reo_desc)	\
	(HAL_RX_BUFFER_ADDR_39_32_GET(&			\
	(((struct reo_destination_ring *)		\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_REO_BUFFER_ADDR_31_0_GET(reo_desc)	\
	(HAL_RX_BUFFER_ADDR_31_0_GET(&			\
	(((struct reo_destination_ring *)		\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_REO_BUF_COOKIE_GET(reo_desc)	\
	(HAL_RX_BUF_COOKIE_GET(&		\
	(((struct reo_destination_ring *)	\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_MPDU_SEQUENCE_NUMBER_GET(mpdu_info_ptr)	\
	((mpdu_info_ptr					\
	[RX_MPDU_DESC_INFO_0_MPDU_SEQUENCE_NUMBER_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_MPDU_SEQUENCE_NUMBER_MASK) >> \
	RX_MPDU_DESC_INFO_0_MPDU_SEQUENCE_NUMBER_LSB)

#define HAL_RX_MPDU_DESC_PEER_META_DATA_GET(mpdu_info_ptr)	\
	((mpdu_info_ptr					\
	[RX_MPDU_DESC_INFO_1_PEER_META_DATA_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_1_PEER_META_DATA_MASK) >> \
	RX_MPDU_DESC_INFO_1_PEER_META_DATA_LSB)

#define HAL_RX_MPDU_MSDU_COUNT_GET(mpdu_info_ptr) \
	((mpdu_info_ptr[RX_MPDU_DESC_INFO_0_MSDU_COUNT_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_MSDU_COUNT_MASK) >> \
	RX_MPDU_DESC_INFO_0_MSDU_COUNT_LSB)

#define HAL_RX_MPDU_FRAGMENT_FLAG_GET(mpdu_info_ptr) \
	(mpdu_info_ptr[RX_MPDU_DESC_INFO_0_FRAGMENT_FLAG_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_FRAGMENT_FLAG_MASK)

#define HAL_RX_MPDU_RETRY_BIT_GET(mpdu_info_ptr) \
	(mpdu_info_ptr[RX_MPDU_DESC_INFO_0_MPDU_RETRY_BIT_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_MPDU_RETRY_BIT_MASK)

#define HAL_RX_MPDU_AMPDU_FLAG_GET(mpdu_info_ptr) \
	(mpdu_info_ptr[RX_MPDU_DESC_INFO_0_AMPDU_FLAG_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_AMPDU_FLAG_MASK)

#define HAL_RX_MPDU_RAW_MPDU_GET(mpdu_info_ptr) \
	(mpdu_info_ptr[RX_MPDU_DESC_INFO_0_RAW_MPDU_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_RAW_MPDU_MASK)

#define HAL_RX_MPDU_FLAGS_GET(mpdu_info_ptr) \
	(HAL_RX_MPDU_FRAGMENT_FLAG_GET(mpdu_info_ptr) | \
	HAL_RX_MPDU_RETRY_BIT_GET(mpdu_info_ptr) |	\
	HAL_RX_MPDU_AMPDU_FLAG_GET(mpdu_info_ptr) |	\
	HAL_RX_MPDU_RAW_MPDU_GET(mpdu_info_ptr))


#define HAL_RX_MSDU_PKT_LENGTH_GET(msdu_info_ptr)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,		\
		RX_MSDU_DESC_INFO_0_MSDU_LENGTH_OFFSET)),	\
		RX_MSDU_DESC_INFO_0_MSDU_LENGTH_MASK,		\
		RX_MSDU_DESC_INFO_0_MSDU_LENGTH_LSB))

/*
 * NOTE: None of the following _GET macros need a right
 * shift by the corresponding _LSB. This is because, they are
 * finally taken and "OR'ed" into a single word again.
 */
#define HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_SET(msdu_info_ptr, val)		\
	((*(((uint32_t *)msdu_info_ptr) +				\
		(RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_OFFSET >> 2))) |= \
		(val << RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_LSB) & \
		RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_info_ptr, val)		\
	((*(((uint32_t *)msdu_info_ptr) +				\
		(RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_OFFSET >> 2))) |= \
		(val << RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_LSB) & \
		RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_MSDU_CONTINUATION_FLAG_SET(msdu_info_ptr, val)		\
	((*(((uint32_t *)msdu_info_ptr) +				\
		(RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_OFFSET >> 2))) |= \
		(val << RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_LSB) & \
		RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_MASK)


#define HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr)	\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_OFFSET)) & \
		RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_LAST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr) \
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_OFFSET)) & \
		RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_MSDU_CONTINUATION_FLAG_GET(msdu_info_ptr)	\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_OFFSET)) & \
		RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_MASK)

#define HAL_RX_MSDU_REO_DST_IND_GET(msdu_info_ptr)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,	\
	RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_OFFSET)),	\
	RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_MASK,		\
	RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_LSB))

#define HAL_RX_MSDU_SA_IS_VALID_FLAG_GET(msdu_info_ptr)		\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_SA_IS_VALID_OFFSET)) &	\
		RX_MSDU_DESC_INFO_0_SA_IS_VALID_MASK)

#define HAL_RX_MSDU_SA_IDX_TIMEOUT_FLAG_GET(msdu_info_ptr)	\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_OFFSET)) &	\
		RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_MASK)

#define HAL_RX_MSDU_DA_IS_VALID_FLAG_GET(msdu_info_ptr)		\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_DA_IS_VALID_OFFSET)) &	\
		RX_MSDU_DESC_INFO_0_DA_IS_VALID_MASK)

#define HAL_RX_MSDU_DA_IS_MCBC_FLAG_GET(msdu_info_ptr)		\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_DA_IS_MCBC_OFFSET)) &	\
		RX_MSDU_DESC_INFO_0_DA_IS_MCBC_MASK)

#define HAL_RX_MSDU_DA_IDX_TIMEOUT_FLAG_GET(msdu_info_ptr) \
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_OFFSET)) &	\
		RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_MASK)


#define HAL_RX_MSDU_FLAGS_GET(msdu_info_ptr) \
	(HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_LAST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_CONTINUATION_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_SA_IS_VALID_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_SA_IDX_TIMEOUT_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_DA_IS_VALID_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_DA_IS_MCBC_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_DA_IDX_TIMEOUT_FLAG_GET(msdu_info_ptr))

#define HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr)	\
	((struct rx_msdu_desc_info *)			\
	_OFFSET_TO_BYTE_PTR(msdu_details_ptr,		\
RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET))


#define HAL_RX_MPDU_PN_31_0_GET(_rx_mpdu_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
	RX_MPDU_INFO_4_PN_31_0_OFFSET)),		\
	RX_MPDU_INFO_4_PN_31_0_MASK,			\
	RX_MPDU_INFO_4_PN_31_0_LSB))

#define HAL_RX_MPDU_PN_63_32_GET(_rx_mpdu_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
	RX_MPDU_INFO_5_PN_63_32_OFFSET)),		\
	RX_MPDU_INFO_5_PN_63_32_MASK,			\
	RX_MPDU_INFO_5_PN_63_32_LSB))

#define HAL_RX_MPDU_PN_95_64_GET(_rx_mpdu_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
	RX_MPDU_INFO_6_PN_95_64_OFFSET)),		\
	RX_MPDU_INFO_6_PN_95_64_MASK,			\
	RX_MPDU_INFO_6_PN_95_64_LSB))

#define HAL_RX_MPDU_PN_127_96_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
	RX_MPDU_INFO_7_PN_127_96_OFFSET)),		\
	RX_MPDU_INFO_7_PN_127_96_MASK,			\
	RX_MPDU_INFO_7_PN_127_96_LSB))

#define HAL_RX_MPDU_ENCRYPT_TYPE_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
	RX_MPDU_INFO_3_ENCRYPT_TYPE_OFFSET)),		\
	RX_MPDU_INFO_3_ENCRYPT_TYPE_MASK,		\
	RX_MPDU_INFO_3_ENCRYPT_TYPE_LSB))

#define HAL_RX_MPDU_ENCRYPTION_INFO_VALID(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,		\
	RX_MPDU_INFO_2_FRAME_ENCRYPTION_INFO_VALID_OFFSET)),	\
	RX_MPDU_INFO_2_FRAME_ENCRYPTION_INFO_VALID_MASK,	\
	RX_MPDU_INFO_2_FRAME_ENCRYPTION_INFO_VALID_LSB))

#define HAL_RX_FLD_SET(_ptr, _wrd, _field, _val)		\
	(*(uint32_t *)(((uint8_t *)_ptr) +			\
		_wrd ## _ ## _field ## _OFFSET) |=		\
		((_val << _wrd ## _ ## _field ## _LSB) &	\
		_wrd ## _ ## _field ## _MASK))

#define HAL_RX_UNIFORM_HDR_SET(_rx_msdu_link, _field, _val)		\
	HAL_RX_FLD_SET(_rx_msdu_link, UNIFORM_DESCRIPTOR_HEADER_0,	\
			_field, _val)

#define HAL_RX_MSDU_DESC_INFO_SET(_msdu_info_ptr, _field, _val)		\
	HAL_RX_FLD_SET(_msdu_info_ptr, RX_MSDU_DESC_INFO_0,		\
			_field, _val)

#define HAL_RX_MPDU_DESC_INFO_SET(_mpdu_info_ptr, _field, _val)		\
	HAL_RX_FLD_SET(_mpdu_info_ptr, RX_MPDU_DESC_INFO_0,		\
			_field, _val)

static inline void hal_rx_mpdu_desc_info_get(void *desc_addr,
				struct hal_rx_mpdu_desc_info *mpdu_desc_info)
{
	struct reo_destination_ring *reo_dst_ring;
	uint32_t mpdu_info[NUM_OF_DWORDS_RX_MPDU_DESC_INFO];

	reo_dst_ring = (struct reo_destination_ring *) desc_addr;

	qdf_mem_copy(&mpdu_info,
			(const void *)&reo_dst_ring->rx_mpdu_desc_info_details,
			sizeof(struct rx_mpdu_desc_info));

	mpdu_desc_info->msdu_count = HAL_RX_MPDU_MSDU_COUNT_GET(mpdu_info);
	mpdu_desc_info->mpdu_seq = HAL_RX_MPDU_SEQUENCE_NUMBER_GET(mpdu_info);
	mpdu_desc_info->mpdu_flags = HAL_RX_MPDU_FLAGS_GET(mpdu_info);
	mpdu_desc_info->peer_meta_data =
		HAL_RX_MPDU_DESC_PEER_META_DATA_GET(mpdu_info);
}

/*
 * @ hal_rx_msdu_desc_info_get: Gets the flags related to MSDU desciptor.
 * @				  Specifically flags needed are:
 * @				  first_msdu_in_mpdu, last_msdu_in_mpdu,
 * @				  msdu_continuation, sa_is_valid,
 * @				  sa_idx_timeout, da_is_valid, da_idx_timeout,
 * @				  da_is_MCBC
 *
 * @ hal_rx_desc_cookie: Opaque cookie pointer used by HAL to get to the current
 * @			   descriptor
 * @ msdu_desc_info: Holds MSDU descriptor info from HAL Rx descriptor
 * @ Return: void
 */
static inline void hal_rx_msdu_desc_info_get(void *desc_addr,
			       struct hal_rx_msdu_desc_info *msdu_desc_info)
{
	struct reo_destination_ring *reo_dst_ring;
	uint32_t msdu_info[NUM_OF_DWORDS_RX_MSDU_DESC_INFO];

	reo_dst_ring = (struct reo_destination_ring *) desc_addr;

	qdf_mem_copy(&msdu_info,
			(const void *)&reo_dst_ring->rx_msdu_desc_info_details,
			sizeof(struct rx_msdu_desc_info));

	msdu_desc_info->msdu_flags = HAL_RX_MSDU_FLAGS_GET(msdu_info);
	msdu_desc_info->msdu_len = HAL_RX_MSDU_PKT_LENGTH_GET(msdu_info);
}

/*
 * hal_rxdma_buff_addr_info_set() - set the buffer_addr_info of the
 *				    rxdma ring entry.
 * @rxdma_entry: descriptor entry
 * @paddr: physical address of nbuf data pointer.
 * @cookie: SW cookie used as a index to SW rx desc.
 * @manager: who owns the nbuf (host, NSS, etc...).
 *
 */
static inline void hal_rxdma_buff_addr_info_set(void *rxdma_entry,
			qdf_dma_addr_t paddr, uint32_t cookie, uint8_t manager)
{
	uint32_t paddr_lo = ((u64)paddr & 0x00000000ffffffff);
	uint32_t paddr_hi = ((u64)paddr & 0xffffffff00000000) >> 32;

	HAL_RXDMA_PADDR_LO_SET(rxdma_entry, paddr_lo);
	HAL_RXDMA_PADDR_HI_SET(rxdma_entry, paddr_hi);
	HAL_RXDMA_COOKIE_SET(rxdma_entry, cookie);
	HAL_RXDMA_MANAGER_SET(rxdma_entry, manager);
}

/*
 * Structures & Macros to obtain fields from the TLV's in the Rx packet
 * pre-header.
 */

/*
 * Every Rx packet starts at an offset from the top of the buffer.
 * If the host hasn't subscribed to any specific TLV, there is
 * still space reserved for the following TLV's from the start of
 * the buffer:
 *	-- RX ATTENTION
 *	-- RX MPDU START
 *	-- RX MSDU START
 *	-- RX MSDU END
 *	-- RX MPDU END
 *	-- RX PACKET HEADER (802.11)
 * If the host subscribes to any of the TLV's above, that TLV
 * if populated by the HW
 */

#define NUM_DWORDS_TAG		1

/* By default the packet header TLV is 128 bytes */
#define  NUM_OF_BYTES_RX_802_11_HDR_TLV		128
#define  NUM_OF_DWORDS_RX_802_11_HDR_TLV	\
		(NUM_OF_BYTES_RX_802_11_HDR_TLV >> 2)

#define RX_PKT_OFFSET_WORDS					\
	(							\
	 NUM_OF_DWORDS_RX_ATTENTION + NUM_DWORDS_TAG		\
	 NUM_OF_DWORDS_RX_MPDU_START + NUM_DWORDS_TAG		\
	 NUM_OF_DWORDS_RX_MSDU_START + NUM_DWORDS_TAG		\
	 NUM_OF_DWORDS_RX_MSDU_END + NUM_DWORDS_TAG		\
	 NUM_OF_DWORDS_RX_MPDU_END + NUM_DWORDS_TAG		\
	 NUM_OF_DWORDS_RX_802_11_HDR_TLV + NUM_DWORDS_TAG	\
	)

#define RX_PKT_OFFSET_BYTES			\
	(RX_PKT_OFFSET_WORDS << 2)

#define RX_PKT_HDR_TLV_LEN		120

/*
 * Each RX descriptor TLV is preceded by 1 DWORD "tag"
 */
struct rx_attention_tlv {
	uint32_t tag;
	struct rx_attention rx_attn;
};

struct rx_mpdu_start_tlv {
	uint32_t tag;
	struct rx_mpdu_start rx_mpdu_start;
};

struct rx_msdu_start_tlv {
	uint32_t tag;
	struct rx_msdu_start rx_msdu_start;
};

struct rx_msdu_end_tlv {
	uint32_t tag;
	struct rx_msdu_end rx_msdu_end;
};

struct rx_mpdu_end_tlv {
	uint32_t tag;
	struct rx_mpdu_end rx_mpdu_end;
};

struct rx_pkt_hdr_tlv {
	uint32_t tag;				/* 4 B */
	uint32_t phy_ppdu_id;                   /* 4 B */
	char rx_pkt_hdr[RX_PKT_HDR_TLV_LEN];	/* 120 B */
};


#define RXDMA_OPTIMIZATION

#ifdef RXDMA_OPTIMIZATION
/*
 * The RX_PADDING_BYTES is required so that the TLV's don't
 * spread across the 128 byte boundary
 * RXDMA optimization requires:
 * 1) MSDU_END & ATTENTION TLV's follow in that order
 * 2) TLV's don't span across 128 byte lines
 * 3) Rx Buffer is nicely aligned on the 128 byte boundary
 */
#if defined(WCSS_VERSION) && \
	((defined(CONFIG_WIN) && (WCSS_VERSION >= 96)) || \
	 (defined(CONFIG_MCL) && (WCSS_VERSION >= 72)))
#define RX_PADDING0_BYTES	4
#endif
#define RX_PADDING1_BYTES	16
struct rx_pkt_tlvs {
	struct rx_msdu_end_tlv   msdu_end_tlv;	/*  72 bytes */
	struct rx_attention_tlv  attn_tlv;	/*  16 bytes */
	struct rx_msdu_start_tlv msdu_start_tlv;/*  40 bytes */
#if defined(WCSS_VERSION) && \
	((defined(CONFIG_WIN) && (WCSS_VERSION >= 96)) || \
	 (defined(CONFIG_MCL) && (WCSS_VERSION >= 72)))
	uint8_t rx_padding0[RX_PADDING0_BYTES];	/*   4 bytes */
#endif
	struct rx_mpdu_start_tlv mpdu_start_tlv;/*  96 bytes */
	struct rx_mpdu_end_tlv   mpdu_end_tlv;	/*  12 bytes */
	uint8_t rx_padding1[RX_PADDING1_BYTES];	/*  16 bytes */
	struct rx_pkt_hdr_tlv	 pkt_hdr_tlv;	/* 128 bytes */
};
#else /* RXDMA_OPTIMIZATION */
struct rx_pkt_tlvs {
	struct rx_attention_tlv  attn_tlv;
	struct rx_mpdu_start_tlv mpdu_start_tlv;
	struct rx_msdu_start_tlv msdu_start_tlv;
	struct rx_msdu_end_tlv   msdu_end_tlv;
	struct rx_mpdu_end_tlv   mpdu_end_tlv;
	struct rx_pkt_hdr_tlv	 pkt_hdr_tlv;
};
#endif /* RXDMA_OPTIMIZATION */

#define RX_PKT_TLVS_LEN		(sizeof(struct rx_pkt_tlvs))

static inline uint8_t
*hal_rx_pkt_hdr_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return pkt_tlvs->pkt_hdr_tlv.rx_pkt_hdr;

}

static inline uint8_t
*hal_rx_padding0_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	return pkt_tlvs->rx_padding0;
}

/*
 * @ hal_rx_encryption_info_valid: Returns encryption type.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: encryption type
 */
static inline uint32_t
hal_rx_encryption_info_valid(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &(mpdu_start->rx_mpdu_info_details);
	uint32_t encryption_info = HAL_RX_MPDU_ENCRYPTION_INFO_VALID(mpdu_info);

	return encryption_info;
}

/*
 * @ hal_rx_print_pn: Prints the PN of rx packet.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: void
 */
static inline void
hal_rx_print_pn(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &(mpdu_start->rx_mpdu_info_details);

	uint32_t pn_31_0 = HAL_RX_MPDU_PN_31_0_GET(mpdu_info);
	uint32_t pn_63_32 = HAL_RX_MPDU_PN_63_32_GET(mpdu_info);
	uint32_t pn_95_64 = HAL_RX_MPDU_PN_95_64_GET(mpdu_info);
	uint32_t pn_127_96 = HAL_RX_MPDU_PN_127_96_GET(mpdu_info);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"PN number pn_127_96 0x%x pn_95_64 0x%x pn_63_32 0x%x pn_31_0 0x%x \n",
			pn_127_96, pn_95_64, pn_63_32, pn_31_0);
}

/*
 * Get msdu_done bit from the RX_ATTENTION TLV
 */
#define HAL_RX_ATTN_MSDU_DONE_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_2_MSDU_DONE_OFFSET)),	\
		RX_ATTENTION_2_MSDU_DONE_MASK,		\
		RX_ATTENTION_2_MSDU_DONE_LSB))

static inline uint32_t
hal_rx_attn_msdu_done_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint32_t msdu_done;

	msdu_done = HAL_RX_ATTN_MSDU_DONE_GET(rx_attn);

	return msdu_done;
}

#define HAL_RX_ATTN_FIRST_MPDU_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_1_FIRST_MPDU_OFFSET)),	\
		RX_ATTENTION_1_FIRST_MPDU_MASK,		\
		RX_ATTENTION_1_FIRST_MPDU_LSB))

/*
 * hal_rx_attn_first_mpdu_get(): get fist_mpdu bit from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * reutm: uint32_t(first_msdu)
 */
static inline uint32_t
hal_rx_attn_first_mpdu_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint32_t first_mpdu;

	first_mpdu = HAL_RX_ATTN_FIRST_MPDU_GET(rx_attn);

	return first_mpdu;
}

#define HAL_RX_ATTN_TCP_UDP_CKSUM_FAIL_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,		\
		RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_OFFSET)),	\
		RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_MASK,	\
		RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_LSB))

/*
 * hal_rx_attn_tcp_udp_cksum_fail_get(): get tcp_udp cksum fail bit
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: tcp_udp_cksum_fail
 */
static inline bool
hal_rx_attn_tcp_udp_cksum_fail_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	bool tcp_udp_cksum_fail;

	tcp_udp_cksum_fail = HAL_RX_ATTN_TCP_UDP_CKSUM_FAIL_GET(rx_attn);

	return tcp_udp_cksum_fail;
}

#define HAL_RX_ATTN_IP_CKSUM_FAIL_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_1_IP_CHKSUM_FAIL_OFFSET)),	\
		RX_ATTENTION_1_IP_CHKSUM_FAIL_MASK,	\
		RX_ATTENTION_1_IP_CHKSUM_FAIL_LSB))

/*
 * hal_rx_attn_ip_cksum_fail_get(): get ip cksum fail bit
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: ip_cksum_fail
 */
static inline bool
hal_rx_attn_ip_cksum_fail_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	bool ip_cksum_fail;

	ip_cksum_fail = HAL_RX_ATTN_IP_CKSUM_FAIL_GET(rx_attn);

	return ip_cksum_fail;
}

#define HAL_RX_ATTN_PHY_PPDU_ID_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_0_PHY_PPDU_ID_OFFSET)),	\
		RX_ATTENTION_0_PHY_PPDU_ID_MASK,	\
		RX_ATTENTION_0_PHY_PPDU_ID_LSB))

/*
 * hal_rx_attn_phy_ppdu_id_get(): get phy_ppdu_id value
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: phy_ppdu_id
 */
static inline uint16_t
hal_rx_attn_phy_ppdu_id_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint16_t phy_ppdu_id;

	phy_ppdu_id = HAL_RX_ATTN_PHY_PPDU_ID_GET(rx_attn);

	return phy_ppdu_id;
}

/*
 * Get peer_meta_data from RX_MPDU_INFO within RX_MPDU_START
 */
#define HAL_RX_MPDU_PEER_META_DATA_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_8_PEER_META_DATA_OFFSET)),	\
		RX_MPDU_INFO_8_PEER_META_DATA_MASK,	\
		RX_MPDU_INFO_8_PEER_META_DATA_LSB))

static inline uint32_t
hal_rx_mpdu_peer_meta_data_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	uint32_t peer_meta_data;

	peer_meta_data = HAL_RX_MPDU_PEER_META_DATA_GET(mpdu_info);

	return peer_meta_data;
}

#define HAL_RX_MPDU_INFO_AMPDU_FLAG_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_12_AMPDU_FLAG_OFFSET)),	\
		RX_MPDU_INFO_12_AMPDU_FLAG_MASK,	\
		RX_MPDU_INFO_12_AMPDU_FLAG_LSB))
/**
 * hal_rx_mpdu_info_ampdu_flag_get(): get ampdu flag bit
 * from rx mpdu info
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: ampdu flag
 */
static inline bool
hal_rx_mpdu_info_ampdu_flag_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	bool ampdu_flag;

	ampdu_flag = HAL_RX_MPDU_INFO_AMPDU_FLAG_GET(mpdu_info);

	return ampdu_flag;
}

#define HAL_RX_MPDU_PEER_META_DATA_SET(_rx_mpdu_info, peer_mdata)	\
		((*(((uint32_t *)_rx_mpdu_info) +			\
		(RX_MPDU_INFO_8_PEER_META_DATA_OFFSET >> 2))) =		\
		(peer_mdata << RX_MPDU_INFO_8_PEER_META_DATA_LSB) &	\
		RX_MPDU_INFO_8_PEER_META_DATA_MASK)

/*
 * @ hal_rx_mpdu_peer_meta_data_set: set peer meta data in RX mpdu start tlv
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ peer_mdata: peer meta data to be set.
 * @ Return: void
 */
static inline void
hal_rx_mpdu_peer_meta_data_set(uint8_t *buf, uint32_t peer_mdata)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	HAL_RX_MPDU_PEER_META_DATA_SET(mpdu_info, peer_mdata);
}

#if defined(WCSS_VERSION) && \
	((defined(CONFIG_WIN) && (WCSS_VERSION > 81)) || \
	 (defined(CONFIG_MCL) && (WCSS_VERSION >= 72)))
#define HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_5_L3_HEADER_PADDING_OFFSET)),	\
		RX_MSDU_END_5_L3_HEADER_PADDING_MASK,		\
		RX_MSDU_END_5_L3_HEADER_PADDING_LSB))
#else
#define HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_9_L3_HEADER_PADDING_OFFSET)),	\
		RX_MSDU_END_9_L3_HEADER_PADDING_MASK,		\
		RX_MSDU_END_9_L3_HEADER_PADDING_LSB))
#endif

/**
* LRO information needed from the TLVs
*/
#define HAL_RX_TLV_GET_LRO_ELIGIBLE(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_end_tlv.rx_msdu_end), \
			 RX_MSDU_END_9_LRO_ELIGIBLE_OFFSET)), \
		RX_MSDU_END_9_LRO_ELIGIBLE_MASK, \
		RX_MSDU_END_9_LRO_ELIGIBLE_LSB))

#define HAL_RX_TLV_GET_TCP_CHKSUM(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_end_tlv.rx_msdu_end), \
			 RX_MSDU_END_1_TCP_UDP_CHKSUM_OFFSET)), \
		RX_MSDU_END_1_TCP_UDP_CHKSUM_MASK, \
		RX_MSDU_END_1_TCP_UDP_CHKSUM_LSB))

#define HAL_RX_TLV_GET_TCP_ACK(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_end_tlv.rx_msdu_end), \
			 RX_MSDU_END_8_TCP_ACK_NUMBER_OFFSET)), \
		RX_MSDU_END_8_TCP_ACK_NUMBER_MASK, \
		RX_MSDU_END_8_TCP_ACK_NUMBER_LSB))

#define HAL_RX_TLV_GET_TCP_SEQ(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_end_tlv.rx_msdu_end), \
			 RX_MSDU_END_7_TCP_SEQ_NUMBER_OFFSET)), \
		RX_MSDU_END_7_TCP_SEQ_NUMBER_MASK, \
		RX_MSDU_END_7_TCP_SEQ_NUMBER_LSB))

#define HAL_RX_TLV_GET_TCP_WIN(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_end_tlv.rx_msdu_end), \
			 RX_MSDU_END_9_WINDOW_SIZE_OFFSET)), \
		RX_MSDU_END_9_WINDOW_SIZE_MASK, \
		RX_MSDU_END_9_WINDOW_SIZE_LSB))

#define HAL_RX_TLV_GET_TCP_PURE_ACK(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_2_TCP_ONLY_ACK_OFFSET)), \
		RX_MSDU_START_2_TCP_ONLY_ACK_MASK, \
		RX_MSDU_START_2_TCP_ONLY_ACK_LSB))

#define HAL_RX_TLV_GET_TCP_PROTO(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_2_TCP_PROTO_OFFSET)), \
		RX_MSDU_START_2_TCP_PROTO_MASK, \
		RX_MSDU_START_2_TCP_PROTO_LSB))

#define HAL_RX_TLV_GET_IPV6(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_2_IPV6_PROTO_OFFSET)), \
		RX_MSDU_START_2_IPV6_PROTO_MASK, \
		RX_MSDU_START_2_IPV6_PROTO_LSB))

#define HAL_RX_TLV_GET_IP_OFFSET(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_1_L3_OFFSET_OFFSET)), \
		RX_MSDU_START_1_L3_OFFSET_MASK, \
		RX_MSDU_START_1_L3_OFFSET_LSB))

#define HAL_RX_TLV_GET_TCP_OFFSET(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_1_L4_OFFSET_OFFSET)), \
		RX_MSDU_START_1_L4_OFFSET_MASK, \
		RX_MSDU_START_1_L4_OFFSET_LSB))

#define HAL_RX_TLV_GET_FLOW_ID_TOEPLITZ(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_4_FLOW_ID_TOEPLITZ_OFFSET)), \
		RX_MSDU_START_4_FLOW_ID_TOEPLITZ_MASK, \
		RX_MSDU_START_4_FLOW_ID_TOEPLITZ_LSB))

 /**
 * hal_rx_msdu_end_l3_hdr_padding_get(): API to get the
 * l3_header padding from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: number of l3 header padding bytes
 */
static inline uint32_t
hal_rx_msdu_end_l3_hdr_padding_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t l3_header_padding;

	l3_header_padding = HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(msdu_end);

	return l3_header_padding;
}

#define HAL_RX_MSDU_END_SA_IDX_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_13_SA_IDX_OFFSET)),	\
		RX_MSDU_END_13_SA_IDX_MASK,		\
		RX_MSDU_END_13_SA_IDX_LSB))

 /**
 * hal_rx_msdu_end_sa_idx_get(): API to get the
 * sa_idx from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_idx (SA AST index)
 */
static inline uint16_t
hal_rx_msdu_end_sa_idx_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t sa_idx;

	sa_idx = HAL_RX_MSDU_END_SA_IDX_GET(msdu_end);

	return sa_idx;
}

#define HAL_RX_MSDU_END_SA_IS_VALID_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_SA_IS_VALID_OFFSET)),	\
		RX_MSDU_END_5_SA_IS_VALID_MASK,		\
		RX_MSDU_END_5_SA_IS_VALID_LSB))

 /**
 * hal_rx_msdu_end_sa_is_valid_get(): API to get the
 * sa_is_valid bit from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_is_valid bit
 */
static inline uint8_t
hal_rx_msdu_end_sa_is_valid_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t sa_is_valid;

	sa_is_valid = HAL_RX_MSDU_END_SA_IS_VALID_GET(msdu_end);

	return sa_is_valid;
}

#define HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(_rx_msdu_end)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_16_SA_SW_PEER_ID_OFFSET)),		\
		RX_MSDU_END_16_SA_SW_PEER_ID_MASK,		\
		RX_MSDU_END_16_SA_SW_PEER_ID_LSB))

 /**
 * hal_rx_msdu_end_sa_sw_peer_id_get(): API to get the
 * sa_sw_peer_id from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_sw_peer_id index
 */
static inline uint32_t
hal_rx_msdu_end_sa_sw_peer_id_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t sa_sw_peer_id;

	sa_sw_peer_id = HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(msdu_end);

	return sa_sw_peer_id;
}

#define HAL_RX_MSDU_START_MSDU_LEN_GET(_rx_msdu_start)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,		\
		RX_MSDU_START_1_MSDU_LENGTH_OFFSET)),		\
		RX_MSDU_START_1_MSDU_LENGTH_MASK,		\
		RX_MSDU_START_1_MSDU_LENGTH_LSB))

 /**
 * hal_rx_msdu_start_msdu_len_get(): API to get the MSDU length
 * from rx_msdu_start TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: msdu length
 */
static inline uint32_t
hal_rx_msdu_start_msdu_len_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
			&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t msdu_len;

	msdu_len = HAL_RX_MSDU_START_MSDU_LEN_GET(msdu_start);

	return msdu_len;
}

 /**
 * hal_rx_msdu_start_msdu_len_set(): API to set the MSDU length
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * @len: msdu length
 *
 * Return: none
 */
static inline void
hal_rx_msdu_start_msdu_len_set(uint8_t *buf, uint32_t len)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
			&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	void *wrd1;

	wrd1 = (uint8_t *)msdu_start + RX_MSDU_START_1_MSDU_LENGTH_OFFSET;
	*(uint32_t *)wrd1 &= (~RX_MSDU_START_1_MSDU_LENGTH_MASK);
	*(uint32_t *)wrd1 |= len;
}

#define HAL_RX_MSDU_START_BW_GET(_rx_msdu_start)     \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_RECEIVE_BANDWIDTH_OFFSET)), \
	RX_MSDU_START_5_RECEIVE_BANDWIDTH_MASK,     \
	RX_MSDU_START_5_RECEIVE_BANDWIDTH_LSB))

/*
 * hal_rx_msdu_start_bw_get(): API to get the Bandwidth
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(bw)
 */
static inline uint32_t
hal_rx_msdu_start_bw_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t bw;

	bw = HAL_RX_MSDU_START_BW_GET(msdu_start);

	return bw;
}

#define HAL_RX_MSDU_START_RECEPTION_TYPE_GET(_rx_msdu_start) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),	\
	RX_MSDU_START_5_RECEPTION_TYPE_OFFSET)),	\
	RX_MSDU_START_5_RECEPTION_TYPE_MASK,		\
	RX_MSDU_START_5_RECEPTION_TYPE_LSB))

/*
 * hal_rx_msdu_start_reception_type_get(): API to get the reception type
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(reception_type)
 */
static inline uint32_t
hal_rx_msdu_start_reception_type_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t reception_type;

	reception_type = HAL_RX_MSDU_START_RECEPTION_TYPE_GET(msdu_start);

	return reception_type;
}

#define HAL_RX_MSDU_START_FLOWID_TOEPLITZ_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,		\
		RX_MSDU_START_4_FLOW_ID_TOEPLITZ_OFFSET)),	\
		RX_MSDU_START_4_FLOW_ID_TOEPLITZ_MASK,		\
		RX_MSDU_START_4_FLOW_ID_TOEPLITZ_LSB))

 /**
 * hal_rx_msdu_start_toeplitz_get: API to get the toeplitz hash
 * from rx_msdu_start TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: toeplitz hash
 */
static inline uint32_t
hal_rx_msdu_start_toeplitz_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
			&pkt_tlvs->msdu_start_tlv.rx_msdu_start;

	return HAL_RX_MSDU_START_FLOWID_TOEPLITZ_GET(msdu_start);
}

/*
 * Get qos_control_valid from RX_MPDU_START
 */
#define HAL_RX_MPDU_INFO_QOS_CONTROL_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),		\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_OFFSET)),		\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_MASK,		\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_LSB))

static inline uint32_t
hal_rx_mpdu_start_mpdu_qos_control_valid_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t qos_control_valid;

	qos_control_valid = HAL_RX_MPDU_INFO_QOS_CONTROL_VALID_GET(
		&(mpdu_start->rx_mpdu_info_details));

	return qos_control_valid;
}

/*
 * Get tid from RX_MPDU_START
 */
#define HAL_RX_MPDU_INFO_TID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),	\
		RX_MPDU_INFO_3_TID_OFFSET)),		\
		RX_MPDU_INFO_3_TID_MASK,		\
		RX_MPDU_INFO_3_TID_LSB))

static inline uint32_t
hal_rx_mpdu_start_tid_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t tid;

	tid = HAL_RX_MPDU_INFO_TID_GET(
		&(mpdu_start->rx_mpdu_info_details));

	return tid;
}

/*
 * Get SW peer id from RX_MPDU_START
 */
#define HAL_RX_MPDU_INFO_SW_PEER_ID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),	\
		RX_MPDU_INFO_1_SW_PEER_ID_OFFSET)),	\
		RX_MPDU_INFO_1_SW_PEER_ID_MASK,		\
		RX_MPDU_INFO_1_SW_PEER_ID_LSB))

static inline uint32_t
hal_rx_mpdu_start_sw_peer_id_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t sw_peer_id;

	sw_peer_id = HAL_RX_MPDU_INFO_SW_PEER_ID_GET(
		&(mpdu_start->rx_mpdu_info_details));

	return sw_peer_id;
}

#if defined(WCSS_VERSION) && \
	((defined(CONFIG_WIN) && (WCSS_VERSION > 81)) || \
	 (defined(CONFIG_MCL) && (WCSS_VERSION >= 72)))
#define HAL_RX_MSDU_START_SGI_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
		RX_MSDU_START_5_SGI_OFFSET)),		\
		RX_MSDU_START_5_SGI_MASK,		\
		RX_MSDU_START_5_SGI_LSB))
#else
#define HAL_RX_MSDU_START_SGI_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
		RX_MSDU_START_6_SGI_OFFSET)),		\
		RX_MSDU_START_6_SGI_MASK,		\
		RX_MSDU_START_6_SGI_LSB))
#endif
/**
 * hal_rx_msdu_start_msdu_sgi_get(): API to get the Short Gaurd
 * Interval from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(sgi)
 */
static inline uint32_t
hal_rx_msdu_start_sgi_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t sgi;

	sgi = HAL_RX_MSDU_START_SGI_GET(msdu_start);

	return sgi;
}

#if defined(WCSS_VERSION) && \
	((defined(CONFIG_WIN) && (WCSS_VERSION > 81)) || \
	 (defined(CONFIG_MCL) && (WCSS_VERSION >= 72)))
#define HAL_RX_MSDU_START_RATE_MCS_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
		RX_MSDU_START_5_RATE_MCS_OFFSET)),	\
		RX_MSDU_START_5_RATE_MCS_MASK,		\
		RX_MSDU_START_5_RATE_MCS_LSB))
#else
#define HAL_RX_MSDU_START_RATE_MCS_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
		RX_MSDU_START_6_RATE_MCS_OFFSET)),	\
		RX_MSDU_START_6_RATE_MCS_MASK,		\
		RX_MSDU_START_6_RATE_MCS_LSB))
#endif
/**
 * hal_rx_msdu_start_msdu_rate_mcs_get(): API to get the MCS rate
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(rate_mcs)
 */
static inline uint32_t
hal_rx_msdu_start_rate_mcs_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t rate_mcs;

	rate_mcs = HAL_RX_MSDU_START_RATE_MCS_GET(msdu_start);

	return rate_mcs;
}

#define HAL_RX_ATTN_DECRYPT_STATUS_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,		\
		RX_ATTENTION_2_DECRYPT_STATUS_CODE_OFFSET)),	\
		RX_ATTENTION_2_DECRYPT_STATUS_CODE_MASK,	\
		RX_ATTENTION_2_DECRYPT_STATUS_CODE_LSB))

/*
 * hal_rx_attn_msdu_get_is_decrypted(): API to get the decrypt status of the
 *  packet from rx_attention
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(decryt status)
 */

static inline uint32_t
hal_rx_attn_msdu_get_is_decrypted(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint32_t is_decrypt = 0;
	uint32_t decrypt_status;

	decrypt_status = HAL_RX_ATTN_DECRYPT_STATUS_GET(rx_attn);

	if (!decrypt_status)
		is_decrypt = 1;

	return is_decrypt;
}

/*
 * Get key index from RX_MSDU_END
 */
#define HAL_RX_MSDU_END_KEYID_OCTET_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_2_KEY_ID_OCTET_OFFSET)),	\
		RX_MSDU_END_2_KEY_ID_OCTET_MASK,	\
		RX_MSDU_END_2_KEY_ID_OCTET_LSB))
/*
 * hal_rx_msdu_get_keyid(): API to get the key id if the decrypted packet
 * from rx_msdu_end
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(key id)
 */

static inline uint32_t
hal_rx_msdu_get_keyid(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t keyid_octet;

	keyid_octet = HAL_RX_MSDU_END_KEYID_OCTET_GET(msdu_end);

	return keyid_octet & 0x3;
}

#define HAL_RX_MSDU_START_RSSI_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,  \
		RX_MSDU_START_5_USER_RSSI_OFFSET)),	\
		RX_MSDU_START_5_USER_RSSI_MASK,		\
		RX_MSDU_START_5_USER_RSSI_LSB))
/*
 * hal_rx_msdu_start_get_rssi(): API to get the rssi of received pkt
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(rssi)
 */

static inline uint32_t
hal_rx_msdu_start_get_rssi(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start = &pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t rssi;

	rssi = HAL_RX_MSDU_START_RSSI_GET(msdu_start);

	return rssi;

}

#define HAL_RX_MSDU_START_FREQ_GET(_rx_msdu_start)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,		\
		RX_MSDU_START_7_SW_PHY_META_DATA_OFFSET)),      \
		RX_MSDU_START_7_SW_PHY_META_DATA_MASK,		\
		RX_MSDU_START_7_SW_PHY_META_DATA_LSB))

/*
 * hal_rx_msdu_start_get_freq(): API to get the frequency of operating channel
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(frequency)
 */

static inline uint32_t
hal_rx_msdu_start_get_freq(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t freq;

	freq = HAL_RX_MSDU_START_FREQ_GET(msdu_start);

	return freq;
}


#define HAL_RX_MSDU_START_PKT_TYPE_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,  \
		RX_MSDU_START_5_PKT_TYPE_OFFSET)),      \
		RX_MSDU_START_5_PKT_TYPE_MASK,		\
		RX_MSDU_START_5_PKT_TYPE_LSB))

/*
 * hal_rx_msdu_start_get_pkt_type(): API to get the pkt type
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(pkt type)
 */

static inline uint32_t
hal_rx_msdu_start_get_pkt_type(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start = &pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t pkt_type;

	pkt_type = HAL_RX_MSDU_START_PKT_TYPE_GET(msdu_start);

	return pkt_type;
}

#define HAL_RX_MSDU_START_NSS_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_NSS_OFFSET)),			\
	RX_MSDU_START_5_NSS_MASK,			\
	RX_MSDU_START_5_NSS_LSB))

/*
 * hal_rx_msdu_start_nss_get(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */

#if !defined(QCA_WIFI_QCA6290_11AX)
static inline uint32_t
hal_rx_msdu_start_nss_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t nss;

	nss = HAL_RX_MSDU_START_NSS_GET(msdu_start);
	return nss;
}
#else
#define HAL_RX_MSDU_START_MIMO_SS_BITMAP(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_MIMO_SS_BITMAP_OFFSET)),	\
	RX_MSDU_START_5_MIMO_SS_BITMAP_MASK,		\
	RX_MSDU_START_5_MIMO_SS_BITMAP_LSB))

static inline uint32_t
hal_rx_msdu_start_nss_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint8_t mimo_ss_bitmap;

	mimo_ss_bitmap = HAL_RX_MSDU_START_MIMO_SS_BITMAP(msdu_start);

	return qdf_get_hweight8(mimo_ss_bitmap);
}
#endif

#define HAL_RX_MPDU_GET_TODS(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_TO_DS_OFFSET)),	\
		RX_MPDU_INFO_2_TO_DS_MASK,	\
		RX_MPDU_INFO_2_TO_DS_LSB))

/*
 * hal_rx_mpdu_get_tods(): API to get the tods info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(to_ds)
 */

static inline uint32_t
hal_rx_mpdu_get_to_ds(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	uint32_t to_ds;

	to_ds = HAL_RX_MPDU_GET_TODS(mpdu_info);

	return to_ds;
}

#define HAL_RX_MPDU_GET_FROMDS(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_FR_DS_OFFSET)),	\
		RX_MPDU_INFO_2_FR_DS_MASK,	\
		RX_MPDU_INFO_2_FR_DS_LSB))

/*
 * hal_rx_mpdu_get_fr_ds(): API to get the from ds info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(fr_ds)
 */

static inline uint32_t
hal_rx_mpdu_get_fr_ds(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	uint32_t fr_ds;

	fr_ds = HAL_RX_MPDU_GET_FROMDS(mpdu_info);

	return fr_ds;
}

#define HAL_RX_MPDU_MAC_ADDR_AD1_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD1_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD1_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD1_VALID_LSB))

#define HAL_RX_MPDU_MAC_ADDR_AD2_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD2_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD2_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD2_VALID_LSB))

#define HAL_RX_MPDU_MAC_ADDR_AD3_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD3_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD3_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD3_VALID_LSB))

#define HAL_RX_MPDU_MAC_ADDR_AD4_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_LSB))

#define HAL_RX_MPDU_AD1_31_0_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0_OFFSET)), \
		RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0_MASK,	\
		RX_MPDU_INFO_15_MAC_ADDR_AD1_31_0_LSB))

#define HAL_RX_MPDU_AD1_47_32_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32_OFFSET)), \
		RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32_MASK,	\
		RX_MPDU_INFO_16_MAC_ADDR_AD1_47_32_LSB))

#define HAL_RX_MPDU_AD2_15_0_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0_OFFSET)), \
		RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0_MASK,	\
		RX_MPDU_INFO_16_MAC_ADDR_AD2_15_0_LSB))

#define HAL_RX_MPDU_AD2_47_16_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16_OFFSET)), \
		RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16_MASK,	\
		RX_MPDU_INFO_17_MAC_ADDR_AD2_47_16_LSB))

#define HAL_RX_MPDU_AD3_31_0_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0_OFFSET)), \
		RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0_MASK,	\
		RX_MPDU_INFO_18_MAC_ADDR_AD3_31_0_LSB))

#define HAL_RX_MPDU_AD3_47_32_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32_OFFSET)), \
		RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32_MASK,	\
		RX_MPDU_INFO_19_MAC_ADDR_AD3_47_32_LSB))

#define HAL_RX_MPDU_AD4_31_0_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0_OFFSET)), \
		RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0_MASK,	\
		RX_MPDU_INFO_20_MAC_ADDR_AD4_31_0_LSB))

#define HAL_RX_MPDU_AD4_47_32_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32_OFFSET)), \
		RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32_MASK,	\
		RX_MPDU_INFO_21_MAC_ADDR_AD4_47_32_LSB))

/*
 * hal_rx_mpdu_get_addr1(): API to check get address1 of the mpdu
 *
 * @buf: pointer to the start of RX PKT TLV headera
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr1(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr1 {
		uint32_t ad1_31_0;
		uint16_t ad1_47_32;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr1 *addr = (struct hal_addr1 *)mac_addr;
	uint32_t mac_addr_ad1_valid;

	mac_addr_ad1_valid = HAL_RX_MPDU_MAC_ADDR_AD1_VALID_GET(mpdu_info);

	if (mac_addr_ad1_valid) {
		addr->ad1_31_0 = HAL_RX_MPDU_AD1_31_0_GET(mpdu_info);
		addr->ad1_47_32 = HAL_RX_MPDU_AD1_47_32_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr2(): API to check get address2 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr2(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr2 {
		uint16_t ad2_15_0;
		uint32_t ad2_47_16;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr2 *addr = (struct hal_addr2 *)mac_addr;
	uint32_t mac_addr_ad2_valid;

	mac_addr_ad2_valid = HAL_RX_MPDU_MAC_ADDR_AD2_VALID_GET(mpdu_info);

	if (mac_addr_ad2_valid) {
		addr->ad2_15_0 = HAL_RX_MPDU_AD2_15_0_GET(mpdu_info);
		addr->ad2_47_16 = HAL_RX_MPDU_AD2_47_16_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr3(): API to get address3 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr3(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr3 {
		uint32_t ad3_31_0;
		uint16_t ad3_47_32;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr3 *addr = (struct hal_addr3 *)mac_addr;
	uint32_t mac_addr_ad3_valid;

	mac_addr_ad3_valid = HAL_RX_MPDU_MAC_ADDR_AD3_VALID_GET(mpdu_info);

	if (mac_addr_ad3_valid) {
		addr->ad3_31_0 = HAL_RX_MPDU_AD3_31_0_GET(mpdu_info);
		addr->ad3_47_32 = HAL_RX_MPDU_AD3_47_32_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr4(): API to get address4 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr4(uint8_t *buf, uint8_t *mac_addr)
{
	struct __attribute__((__packed__)) hal_addr4 {
		uint32_t ad4_31_0;
		uint16_t ad4_47_32;
	};

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	struct hal_addr4 *addr = (struct hal_addr4 *)mac_addr;
	uint32_t mac_addr_ad4_valid;

	mac_addr_ad4_valid = HAL_RX_MPDU_MAC_ADDR_AD4_VALID_GET(mpdu_info);

	if (mac_addr_ad4_valid) {
		addr->ad4_31_0 = HAL_RX_MPDU_AD4_31_0_GET(mpdu_info);
		addr->ad4_47_32 = HAL_RX_MPDU_AD4_47_32_GET(mpdu_info);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

#define HAL_RX_MSDU_END_DA_IDX_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_13_DA_IDX_OFFSET)),		\
		RX_MSDU_END_13_DA_IDX_MASK,		\
		RX_MSDU_END_13_DA_IDX_LSB))

 /**
 * hal_rx_msdu_end_da_idx_get: API to get da_idx
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da index
 */
static inline uint16_t
hal_rx_msdu_end_da_idx_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t da_idx;

	da_idx = HAL_RX_MSDU_END_DA_IDX_GET(msdu_end);

	return da_idx;
}

#define HAL_RX_MSDU_END_DA_IS_VALID_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_DA_IS_VALID_OFFSET)),	\
		RX_MSDU_END_5_DA_IS_VALID_MASK,		\
		RX_MSDU_END_5_DA_IS_VALID_LSB))

 /**
 * hal_rx_msdu_end_da_is_valid_get: API to check if da is valid
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_valid
 */
static inline uint8_t
hal_rx_msdu_end_da_is_valid_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t da_is_valid;

	da_is_valid = HAL_RX_MSDU_END_DA_IS_VALID_GET(msdu_end);

	return da_is_valid;
}

#define HAL_RX_MSDU_END_DA_IS_MCBC_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_DA_IS_MCBC_OFFSET)),	\
		RX_MSDU_END_5_DA_IS_MCBC_MASK,		\
		RX_MSDU_END_5_DA_IS_MCBC_LSB))

 /**
 * hal_rx_msdu_end_da_is_mcbc_get: API to check if pkt is MCBC
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_mcbc
 */
static inline uint8_t
hal_rx_msdu_end_da_is_mcbc_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t da_is_mcbc;

	da_is_mcbc = HAL_RX_MSDU_END_DA_IS_MCBC_GET(msdu_end);

	return da_is_mcbc;
}

#define HAL_RX_MSDU_END_FIRST_MSDU_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_FIRST_MSDU_OFFSET)),	\
		RX_MSDU_END_5_FIRST_MSDU_MASK,		\
		RX_MSDU_END_5_FIRST_MSDU_LSB))

 /**
 * hal_rx_msdu_end_first_msdu_get: API to get first msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: first_msdu
 */
static inline uint8_t
hal_rx_msdu_end_first_msdu_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t first_msdu;

	first_msdu = HAL_RX_MSDU_END_FIRST_MSDU_GET(msdu_end);

	return first_msdu;
}

#define HAL_RX_MSDU_END_LAST_MSDU_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_LAST_MSDU_OFFSET)),	\
		RX_MSDU_END_5_LAST_MSDU_MASK,		\
		RX_MSDU_END_5_LAST_MSDU_LSB))

 /**
 * hal_rx_msdu_end_last_msdu_get: API to get last msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: last_msdu
 */
static inline uint8_t
hal_rx_msdu_end_last_msdu_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t last_msdu;

	last_msdu = HAL_RX_MSDU_END_LAST_MSDU_GET(msdu_end);

	return last_msdu;
}
/*******************************************************************************
 * RX ERROR APIS
 ******************************************************************************/

#define HAL_RX_MPDU_END_DECRYPT_ERR_GET(_rx_mpdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_end),\
		RX_MPDU_END_1_RX_IN_TX_DECRYPT_BYP_OFFSET)),	\
		RX_MPDU_END_1_RX_IN_TX_DECRYPT_BYP_MASK,	\
		RX_MPDU_END_1_RX_IN_TX_DECRYPT_BYP_LSB))

/**
 * hal_rx_mpdu_end_decrypt_err_get(): API to get the Decrypt ERR
 * from rx_mpdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(decrypt_err)
 */
static inline uint32_t
hal_rx_mpdu_end_decrypt_err_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_end *mpdu_end =
		&pkt_tlvs->mpdu_end_tlv.rx_mpdu_end;
	uint32_t decrypt_err;

	decrypt_err = HAL_RX_MPDU_END_DECRYPT_ERR_GET(mpdu_end);

	return decrypt_err;
}

#define HAL_RX_MPDU_END_MIC_ERR_GET(_rx_mpdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_end),\
		RX_MPDU_END_1_TKIP_MIC_ERR_OFFSET)),	\
		RX_MPDU_END_1_TKIP_MIC_ERR_MASK,	\
		RX_MPDU_END_1_TKIP_MIC_ERR_LSB))

/**
 * hal_rx_mpdu_end_mic_err_get(): API to get the MIC ERR
 * from rx_mpdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(mic_err)
 */
static inline uint32_t
hal_rx_mpdu_end_mic_err_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_end *mpdu_end =
		&pkt_tlvs->mpdu_end_tlv.rx_mpdu_end;
	uint32_t mic_err;

	mic_err = HAL_RX_MPDU_END_MIC_ERR_GET(mpdu_end);

	return mic_err;
}

/*******************************************************************************
 * RX REO ERROR APIS
 ******************************************************************************/

#define HAL_RX_LINK_DESC_MSDU0_PTR(link_desc)	\
	((struct rx_msdu_details *)		\
		_OFFSET_TO_BYTE_PTR((link_desc),\
		RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET))

#define HAL_RX_NUM_MSDU_DESC 6
#define HAL_RX_MAX_SAVED_RING_DESC 16

/* TODO: rework the structure */
struct hal_rx_msdu_list {
	struct hal_rx_msdu_desc_info msdu_info[HAL_RX_NUM_MSDU_DESC];
	uint32_t sw_cookie[HAL_RX_NUM_MSDU_DESC];
	uint8_t rbm[HAL_RX_NUM_MSDU_DESC];
};

struct hal_buf_info {
	uint64_t paddr;
	uint32_t sw_cookie;
};

/* This special cookie value will be used to indicate FW allocated buffers
 * received through RXDMA2SW ring for RXDMA WARs */
#define HAL_RX_COOKIE_SPECIAL 0x1fffff

/**
 * hal_rx_msdu_link_desc_get(): API to get the MSDU information
 * from the MSDU link descriptor
 *
 * @msdu_link_desc: Opaque pointer used by HAL to get to the
 * MSDU link descriptor (struct rx_msdu_link)
 *
 * @msdu_list: Return the list of MSDUs contained in this link descriptor
 *
 * @num_msdus: Number of MSDUs in the MPDU
 *
 * Return: void
 */
static inline void hal_rx_msdu_list_get(void *msdu_link_desc,
		struct hal_rx_msdu_list *msdu_list, uint16_t *num_msdus)
{
	struct rx_msdu_details *msdu_details;
	struct rx_msdu_desc_info *msdu_desc_info;
	struct rx_msdu_link *msdu_link = (struct rx_msdu_link *)msdu_link_desc;
	int i;

	msdu_details = HAL_RX_LINK_DESC_MSDU0_PTR(msdu_link);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"[%s][%d] msdu_link=%pK msdu_details=%pK\n",
		__func__, __LINE__, msdu_link, msdu_details);

	for (i = 0; i < HAL_RX_NUM_MSDU_DESC; i++) {
		/* num_msdus received in mpdu descriptor may be incorrect
		 * sometimes due to HW issue. Check msdu buffer address also */
		if (HAL_RX_BUFFER_ADDR_31_0_GET(
			&msdu_details[i].buffer_addr_info_details) == 0) {
			/* set the last msdu bit in the prev msdu_desc_info */
			msdu_desc_info =
				HAL_RX_MSDU_DESC_INFO_GET(&msdu_details[i - 1]);
			HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);
			break;
		}
		msdu_desc_info = HAL_RX_MSDU_DESC_INFO_GET(&msdu_details[i]);

		/* set first MSDU bit or the last MSDU bit */
		if (!i)
			HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);
		else if (i == (HAL_RX_NUM_MSDU_DESC - 1))
			HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);

		msdu_list->msdu_info[i].msdu_flags =
			 HAL_RX_MSDU_FLAGS_GET(msdu_desc_info);
		msdu_list->msdu_info[i].msdu_len =
			 HAL_RX_MSDU_PKT_LENGTH_GET(msdu_desc_info);
		msdu_list->sw_cookie[i] =
			 HAL_RX_BUF_COOKIE_GET(
				&msdu_details[i].buffer_addr_info_details);
		 msdu_list->rbm[i] = HAL_RX_BUF_RBM_GET(
				&msdu_details[i].buffer_addr_info_details);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"[%s][%d] i=%d sw_cookie=%d\n",
			__func__, __LINE__, i, msdu_list->sw_cookie[i]);
	}
	*num_msdus = i;
}

/**
 * hal_rx_msdu_reo_dst_ind_get: Gets the REO
 * destination ring ID from the msdu desc info
 *
 * @msdu_link_desc : Opaque cookie pointer used by HAL to get to
 * the current descriptor
 *
 * Return: dst_ind (REO destination ring ID)
 */
static inline uint32_t
hal_rx_msdu_reo_dst_ind_get(void *msdu_link_desc)
{
	struct rx_msdu_details *msdu_details;
	struct rx_msdu_desc_info *msdu_desc_info;
	struct rx_msdu_link *msdu_link = (struct rx_msdu_link *)msdu_link_desc;
	uint32_t dst_ind;

	msdu_details = HAL_RX_LINK_DESC_MSDU0_PTR(msdu_link);

	/* The first msdu in the link should exsist */
	msdu_desc_info = HAL_RX_MSDU_DESC_INFO_GET(&msdu_details[0]);
	dst_ind = HAL_RX_MSDU_REO_DST_IND_GET(msdu_desc_info);
	return dst_ind;
}

/**
 * hal_rx_reo_buf_paddr_get: Gets the physical address and
 * cookie from the REO destination ring element
 *
 * @ hal_rx_desc_cookie: Opaque cookie pointer used by HAL to get to
 * the current descriptor
 * @ buf_info: structure to return the buffer information
 * Return: void
 */
static inline void hal_rx_reo_buf_paddr_get(void *rx_desc,
	 struct hal_buf_info *buf_info)
{
	struct reo_destination_ring *reo_ring =
		 (struct reo_destination_ring *)rx_desc;

	buf_info->paddr =
	 (HAL_RX_REO_BUFFER_ADDR_31_0_GET(reo_ring) |
	  ((uint64_t)(HAL_RX_REO_BUFFER_ADDR_39_32_GET(reo_ring)) << 32));

	buf_info->sw_cookie = HAL_RX_REO_BUF_COOKIE_GET(reo_ring);
}

/**
 * enum hal_reo_error_code: Indicates that type of buffer or descriptor
 *
 * @ HAL_RX_MSDU_BUF_ADDR_TYPE : Reo buffer address points to the MSDU buffer
 * @ HAL_RX_MSDU_LINK_DESC_TYPE: Reo buffer address points to the link
 * descriptor
 */
enum hal_rx_reo_buf_type {
	HAL_RX_REO_MSDU_BUF_ADDR_TYPE = 0,
	HAL_RX_REO_MSDU_LINK_DESC_TYPE,
};

#define HAL_RX_REO_BUF_TYPE_GET(reo_desc) (((*(((uint32_t *) reo_desc)+ \
		(REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_OFFSET >> 2))) & \
		REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_MASK) >> \
		REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_LSB)

/**
 * enum hal_reo_error_code: Error code describing the type of error detected
 *
 * @ HAL_REO_ERR_QUEUE_DESC_ADDR_0 : Reo queue descriptor provided in the
 * REO_ENTRANCE ring is set to 0
 * @ HAL_REO_ERR_QUEUE_DESC_INVALID: Reo queue descriptor valid bit is NOT set
 * @ HAL_REO_ERR_AMPDU_IN_NON_BA : AMPDU frame received without BA session
 * having been setup
 * @ HAL_REO_ERR_NON_BA_DUPLICATE : Non-BA session, SN equal to SSN,
 * Retry bit set: duplicate frame
 * @ HAL_REO_ERR_BA_DUPLICATE : BA session, duplicate frame
 * @ HAL_REO_ERR_REGULAR_FRAME_2K_JUMP : A normal (management/data frame)
 * received with 2K jump in SN
 * @ HAL_REO_ERR_BAR_FRAME_2K_JUMP : A bar received with 2K jump in SSN
 * @ HAL_REO_ERR_REGULAR_FRAME_OOR : A normal (management/data frame) received
 * with SN falling within the OOR window
 * @ HAL_REO_ERR_BAR_FRAME_OOR : A bar received with SSN falling within the
 * OOR window
 * @ HAL_REO_ERR_BAR_FRAME_NO_BA_SESSION : A bar received without a BA session
 * @ HAL_REO_ERR_BAR_FRAME_SN_EQUALS_SSN : A bar received with SSN equal to SN
 * @ HAL_REO_ERR_PN_CHECK_FAILED : PN Check Failed packet
 * @ HAL_REO_ERR_2K_ERROR_HANDLING_FLAG_SET : Frame is forwarded as a result
 * of the Seq_2k_error_detected_flag been set in the REO Queue descriptor
 * @ HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET : Frame is forwarded as a result
 * of the pn_error_detected_flag been set in the REO Queue descriptor
 * @ HAL_REO_ERR_QUEUE_DESC_BLOCKED_SET : Frame is forwarded as a result of
 * the queue descriptor(address) being blocked as SW/FW seems to be currently
 * in the process of making updates to this descriptor
 */
enum hal_reo_error_code {
	HAL_REO_ERR_QUEUE_DESC_ADDR_0 = 0,
	HAL_REO_ERR_QUEUE_DESC_INVALID,
	HAL_REO_ERR_AMPDU_IN_NON_BA,
	HAL_REO_ERR_NON_BA_DUPLICATE,
	HAL_REO_ERR_BA_DUPLICATE,
	HAL_REO_ERR_REGULAR_FRAME_2K_JUMP,
	HAL_REO_ERR_BAR_FRAME_2K_JUMP,
	HAL_REO_ERR_REGULAR_FRAME_OOR,
	HAL_REO_ERR_BAR_FRAME_OOR,
	HAL_REO_ERR_BAR_FRAME_NO_BA_SESSION,
	HAL_REO_ERR_BAR_FRAME_SN_EQUALS_SSN,
	HAL_REO_ERR_PN_CHECK_FAILED,
	HAL_REO_ERR_2K_ERROR_HANDLING_FLAG_SET,
	HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET,
	HAL_REO_ERR_QUEUE_DESC_BLOCKED_SET,
	HAL_REO_ERR_MAX
};

/**
 * enum hal_rxdma_error_code: Code describing the type of RxDMA error detected
 *
 * @HAL_RXDMA_ERR_OVERFLOW: MPDU frame is not complete due to a FIFO overflow
 * @ HAL_RXDMA_ERR_OVERFLOW      : MPDU frame is not complete due to a FIFO
 * 				   overflow
 * @ HAL_RXDMA_ERR_MPDU_LENGTH   : MPDU frame is not complete due to receiving
 * 				   incomplete
 * 		               	   MPDU from the PHY
 * @ HAL_RXDMA_ERR_FCS           : FCS check on the MPDU frame failed
 * @ HAL_RXDMA_ERR_DECRYPT       : Decryption error
 * @ HAL_RXDMA_ERR_TKIP_MIC      : TKIP MIC error
 * @ HAL_RXDMA_ERR_UNENCRYPTED   : Received a frame that was expected to be
 * 			  	   encrypted but wasn’t
 * @ HAL_RXDMA_ERR_MSDU_LEN      : MSDU related length error
 * @ HAL_RXDMA_ERR_MSDU_LIMIT    : Number of MSDUs in the MPDUs exceeded
 * 				   the max allowed
 * @ HAL_RXDMA_ERR_WIFI_PARSE    : wifi parsing error
 * @ HAL_RXDMA_ERR_AMSDU_PARSE   : Amsdu parsing error
 * @ HAL_RXDMA_ERR_SA_TIMEOUT    : Source Address search timeout
 * @ HAL_RXDMA_ERR_DA_TIMEOUT    : Destination Address  search timeout
 * @ HAL_RXDMA_ERR_FLOW_TIMEOUT  : Flow Search Timeout
 * @ HAL_RXDMA_ERR_FLUSH_REQUEST : RxDMA FIFO Flush request
 * @ HAL_RXDMA_ERR_WAR           : RxDMA WAR dummy errors
 */
enum hal_rxdma_error_code {
	HAL_RXDMA_ERR_OVERFLOW = 0,
	HAL_RXDMA_ERR_MPDU_LENGTH,
	HAL_RXDMA_ERR_FCS,
	HAL_RXDMA_ERR_DECRYPT,
	HAL_RXDMA_ERR_TKIP_MIC,
	HAL_RXDMA_ERR_UNENCRYPTED,
	HAL_RXDMA_ERR_MSDU_LEN,
	HAL_RXDMA_ERR_MSDU_LIMIT,
	HAL_RXDMA_ERR_WIFI_PARSE,
	HAL_RXDMA_ERR_AMSDU_PARSE,
	HAL_RXDMA_ERR_SA_TIMEOUT,
	HAL_RXDMA_ERR_DA_TIMEOUT,
	HAL_RXDMA_ERR_FLOW_TIMEOUT,
	HAL_RXDMA_ERR_FLUSH_REQUEST,
	HAL_RXDMA_ERR_WAR = 31,
	HAL_RXDMA_ERR_MAX
};

/**
 * HW BM action settings in WBM release ring
 */
#define HAL_BM_ACTION_PUT_IN_IDLE_LIST 0
#define HAL_BM_ACTION_RELEASE_MSDU_LIST 1

/**
 * enum hal_rx_wbm_error_source: Indicates which module initiated the
 * release of this buffer or descriptor
 *
 * @ HAL_RX_WBM_ERR_SRC_TQM : TQM released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_RXDMA: RXDMA released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_REO: REO released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_FW: FW released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_SW: SW released this buffer or descriptor
 */
enum hal_rx_wbm_error_source {
	HAL_RX_WBM_ERR_SRC_TQM = 0,
	HAL_RX_WBM_ERR_SRC_RXDMA,
	HAL_RX_WBM_ERR_SRC_REO,
	HAL_RX_WBM_ERR_SRC_FW,
	HAL_RX_WBM_ERR_SRC_SW,
};

/**
 * enum hal_rx_wbm_buf_type: Indicates that type of buffer or descriptor
 * released
 *
 * @ HAL_RX_WBM_ERR_SRC_TQM : TQM released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_RXDMA: RXDMA released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_REO: REO released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_FW: FW released this buffer or descriptor
 * @ HAL_RX_WBM_ERR_SRC_SW: SW released this buffer or descriptor
 */
enum hal_rx_wbm_buf_type {
	HAL_RX_WBM_BUF_TYPE_REL_BUF = 0,
	HAL_RX_WBM_BUF_TYPE_MSDU_LINK_DESC,
	HAL_RX_WBM_BUF_TYPE_MPDU_LINK_DESC,
	HAL_RX_WBM_BUF_TYPE_MSDU_EXT_DESC,
	HAL_RX_WBM_BUF_TYPE_Q_EXT_DESC,
};

#define HAL_RX_REO_ERROR_GET(reo_desc) (((*(((uint32_t *) reo_desc)+ \
		(REO_DESTINATION_RING_7_REO_ERROR_CODE_OFFSET >> 2))) & \
		REO_DESTINATION_RING_7_REO_ERROR_CODE_MASK) >> \
		REO_DESTINATION_RING_7_REO_ERROR_CODE_LSB)

/**
 * hal_rx_is_pn_error() - Indicate if this error was caused by a
 * PN check failure
 *
 * @reo_desc: opaque pointer used by HAL to get the REO destination entry
 *
 * Return: true: error caused by PN check, false: other error
 */
static inline bool hal_rx_reo_is_pn_error(void *rx_desc)
{
	struct reo_destination_ring *reo_desc =
			(struct reo_destination_ring *)rx_desc;

	return ((HAL_RX_REO_ERROR_GET(reo_desc) ==
			HAL_REO_ERR_PN_CHECK_FAILED) |
			(HAL_RX_REO_ERROR_GET(reo_desc) ==
			HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET)) ?
			true : false;
}

/**
 * hal_rx_is_2k_jump() - Indicate if this error was caused by a 2K jump in
 * the sequence number
 *
 * @ring_desc: opaque pointer used by HAL to get the REO destination entry
 *
 * Return: true: error caused by 2K jump, false: other error
 */
static inline bool hal_rx_reo_is_2k_jump(void *rx_desc)
{
	struct reo_destination_ring *reo_desc =
			(struct reo_destination_ring *)rx_desc;

	return ((HAL_RX_REO_ERROR_GET(reo_desc) ==
			HAL_REO_ERR_REGULAR_FRAME_2K_JUMP) |
			(HAL_RX_REO_ERROR_GET(reo_desc) ==
			HAL_REO_ERR_2K_ERROR_HANDLING_FLAG_SET)) ?
			true : false;
}

/**
 * hal_rx_msdu_link_desc_set: Retrieves MSDU Link Descriptor to WBM
 *
 * @ soc		: HAL version of the SOC pointer
 * @ src_srng_desc	: void pointer to the WBM Release Ring descriptor
 * @ buf_addr_info	: void pointer to the buffer_addr_info
 * @ bm_action		: put in IDLE list or release to MSDU_LIST
 *
 * Return: void
 */
/* look at implementation at dp_hw_link_desc_pool_setup()*/
static inline void hal_rx_msdu_link_desc_set(struct hal_soc *soc,
			void *src_srng_desc, void *buf_addr_info,
			uint8_t bm_action)
{
	struct wbm_release_ring *wbm_rel_srng =
			(struct wbm_release_ring *)src_srng_desc;

	/* Structure copy !!! */
	wbm_rel_srng->released_buff_or_desc_addr_info =
				*((struct buffer_addr_info *)buf_addr_info);
	HAL_DESC_SET_FIELD(src_srng_desc, WBM_RELEASE_RING_2,
		RELEASE_SOURCE_MODULE, HAL_RX_WBM_ERR_SRC_SW);
	HAL_DESC_SET_FIELD(src_srng_desc, WBM_RELEASE_RING_2, BM_ACTION,
		bm_action);
	HAL_DESC_SET_FIELD(src_srng_desc, WBM_RELEASE_RING_2,
		BUFFER_OR_DESC_TYPE, HAL_RX_WBM_BUF_TYPE_MSDU_LINK_DESC);
}

/*
 * hal_rx_msdu_link_desc_reinject: Re-injects the MSDU link descriptor to
 * REO entrance ring
 *
 * @ soc: HAL version of the SOC pointer
 * @ pa: Physical address of the MSDU Link Descriptor
 * @ cookie: SW cookie to get to the virtual address
 * @ error_enabled_reo_q: Argument to determine whether this needs to go
 * to the error enabled REO queue
 *
 * Return: void
 */
static inline void hal_rx_msdu_link_desc_reinject(struct hal_soc *soc,
	 uint64_t pa, uint32_t cookie, bool error_enabled_reo_q)
{
	/* TODO */
}

/**
 * HAL_RX_BUF_ADDR_INFO_GET: Returns the address of the
 *			     BUFFER_ADDR_INFO, give the RX descriptor
 *			     (Assumption -- BUFFER_ADDR_INFO is the
 *			     first field in the descriptor structure)
 */
#define HAL_RX_BUF_ADDR_INFO_GET(ring_desc)	((void *)(ring_desc))

#define HAL_RX_REO_BUF_ADDR_INFO_GET HAL_RX_BUF_ADDR_INFO_GET

#define HAL_RX_WBM_BUF_ADDR_INFO_GET HAL_RX_BUF_ADDR_INFO_GET

/**
 * hal_rx_ret_buf_manager_get: Returns the "return_buffer_manager"
 *			       from the BUFFER_ADDR_INFO structure
 *			       given a REO destination ring descriptor.
 * @ ring_desc: RX(REO/WBM release) destination ring descriptor
 *
 * Return: uint8_t (value of the return_buffer_manager)
 */
static inline
uint8_t hal_rx_ret_buf_manager_get(void *ring_desc)
{
	/*
	 * The following macro takes buf_addr_info as argument,
	 * but since buf_addr_info is the first field in ring_desc
	 * Hence the following call is OK
	 */
	return HAL_RX_BUF_RBM_GET(ring_desc);
}


/*******************************************************************************
 * RX WBM ERROR APIS
 ******************************************************************************/

#define HAL_RX_WBM_ERR_SRC_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_MASK) >> \
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_LSB)

#define HAL_RX_WBM_BUF_TYPE_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_MASK) >> \
		WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_LSB)

/**
 * enum - hal_rx_wbm_reo_push_reason: Indicates why REO pushed
 * the frame to this release ring
 *
 * @ HAL_RX_WBM_REO_PSH_RSN_ERROR : Reo detected an error and pushed this
 * frame to this queue
 * @ HAL_RX_WBM_REO_PSH_RSN_ROUTE: Reo pushed the frame to this queue per
 * received routing instructions. No error within REO was detected
 */
enum hal_rx_wbm_reo_push_reason {
	HAL_RX_WBM_REO_PSH_RSN_ERROR = 0,
	HAL_RX_WBM_REO_PSH_RSN_ROUTE,
};

#define HAL_RX_WBM_REO_PUSH_REASON_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_REO_PUSH_REASON_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_REO_PUSH_REASON_MASK) >> \
		WBM_RELEASE_RING_2_REO_PUSH_REASON_LSB)

#define HAL_RX_WBM_REO_ERROR_CODE_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_REO_ERROR_CODE_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_REO_ERROR_CODE_MASK) >> \
		WBM_RELEASE_RING_2_REO_ERROR_CODE_LSB)

/**
 * enum hal_rx_wbm_rxdma_push_reason: Indicates why REO pushed the frame to
 * this release ring
 *
 * @ HAL_RX_WBM_RXDMA_PSH_RSN_ERROR : RXDMA detected an error and pushed
 * this frame to this queue
 * @ HAL_RX_WBM_RXDMA_PSH_RSN_ROUTE: RXDMA pushed the frame to this queue
 * per received routing instructions. No error within RXDMA was detected
 */
enum hal_rx_wbm_rxdma_push_reason {
	HAL_RX_WBM_RXDMA_PSH_RSN_ERROR = 0,
	HAL_RX_WBM_RXDMA_PSH_RSN_ROUTE,
};

#define HAL_RX_WBM_RXDMA_PUSH_REASON_GET(wbm_desc)	\
	(((*(((uint32_t *) wbm_desc) +			\
	(WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_OFFSET >> 2))) & \
	WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_MASK) >>	\
	WBM_RELEASE_RING_2_RXDMA_PUSH_REASON_LSB)

#define HAL_RX_WBM_RXDMA_ERROR_CODE_GET(wbm_desc)	\
	(((*(((uint32_t *) wbm_desc) +			\
	(WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_OFFSET >> 2))) & \
	WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_MASK) >>	\
	WBM_RELEASE_RING_2_RXDMA_ERROR_CODE_LSB)

#define HAL_RX_WBM_FIRST_MSDU_GET(wbm_desc)		\
	(((*(((uint32_t *) wbm_desc) +			\
	(WBM_RELEASE_RING_4_FIRST_MSDU_OFFSET >> 2))) & \
	WBM_RELEASE_RING_4_FIRST_MSDU_MASK) >>		\
	WBM_RELEASE_RING_4_FIRST_MSDU_LSB)

#define HAL_RX_WBM_LAST_MSDU_GET(wbm_desc)		\
	(((*(((uint32_t *) wbm_desc) +			\
	(WBM_RELEASE_RING_4_LAST_MSDU_OFFSET >> 2))) & 	\
	WBM_RELEASE_RING_4_LAST_MSDU_MASK) >>		\
	WBM_RELEASE_RING_4_LAST_MSDU_LSB)

#define HAL_RX_WBM_BUF_COOKIE_GET(wbm_desc) \
	HAL_RX_BUF_COOKIE_GET(&((struct wbm_release_ring *) \
	wbm_desc)->released_buff_or_desc_addr_info)

/**
 * hal_rx_dump_rx_attention_tlv: dump RX attention TLV in structured
 *				 humman readable format.
 * @ rx_attn: pointer the rx_attention TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_rx_attention_tlv(struct rx_attention *rx_attn,
							uint8_t dbg_level)
{
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_attention tlv="
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"reserved_0: %d "
			"phy_ppdu_id: %d "
			"first_mpdu : %d "
			"reserved_1a: %d "
			"mcast_bcast: %d "
			"ast_index_not_found: %d "
			"ast_index_timeout: %d "
			"power_mgmt: %d "
			"non_qos: %d "
			"null_data: %d "
			"mgmt_type: %d "
			"ctrl_type: %d "
			"more_data: %d "
			"eosp: %d "
			"a_msdu_error: %d "
			"fragment_flag: %d "
			"order: %d "
			"cce_match: %d "
			"overflow_err: %d "
			"msdu_length_err: %d "
			"tcp_udp_chksum_fail: %d "
			"ip_chksum_fail: %d "
			"sa_idx_invalid: %d "
			"da_idx_invalid: %d "
			"reserved_1b: %d "
			"rx_in_tx_decrypt_byp: %d "
			"encrypt_required: %d "
			"directed: %d "
			"buffer_fragment: %d "
			"mpdu_length_err: %d "
			"tkip_mic_err: %d "
			"decrypt_err: %d "
			"unencrypted_frame_err: %d "
			"fcs_err: %d "
			"flow_idx_timeout: %d "
			"flow_idx_invalid: %d "
			"wifi_parser_error: %d "
			"amsdu_parser_error: %d "
			"sa_idx_timeout: %d "
			"da_idx_timeout: %d "
			"msdu_limit_error: %d "
			"da_is_valid: %d "
			"da_is_mcbc: %d "
			"sa_is_valid: %d "
			"decrypt_status_code: %d "
			"rx_bitmap_not_updated: %d "
			"reserved_2: %d "
			"msdu_done: %d ",
			rx_attn->rxpcu_mpdu_filter_in_category,
			rx_attn->sw_frame_group_id,
			rx_attn->reserved_0,
			rx_attn->phy_ppdu_id,
			rx_attn->first_mpdu,
			rx_attn->reserved_1a,
			rx_attn->mcast_bcast,
			rx_attn->ast_index_not_found,
			rx_attn->ast_index_timeout,
			rx_attn->power_mgmt,
			rx_attn->non_qos,
			rx_attn->null_data,
			rx_attn->mgmt_type,
			rx_attn->ctrl_type,
			rx_attn->more_data,
			rx_attn->eosp,
			rx_attn->a_msdu_error,
			rx_attn->fragment_flag,
			rx_attn->order,
			rx_attn->cce_match,
			rx_attn->overflow_err,
			rx_attn->msdu_length_err,
			rx_attn->tcp_udp_chksum_fail,
			rx_attn->ip_chksum_fail,
			rx_attn->sa_idx_invalid,
			rx_attn->da_idx_invalid,
			rx_attn->reserved_1b,
			rx_attn->rx_in_tx_decrypt_byp,
			rx_attn->encrypt_required,
			rx_attn->directed,
			rx_attn->buffer_fragment,
			rx_attn->mpdu_length_err,
			rx_attn->tkip_mic_err,
			rx_attn->decrypt_err,
			rx_attn->unencrypted_frame_err,
			rx_attn->fcs_err,
			rx_attn->flow_idx_timeout,
			rx_attn->flow_idx_invalid,
			rx_attn->wifi_parser_error,
			rx_attn->amsdu_parser_error,
			rx_attn->sa_idx_timeout,
			rx_attn->da_idx_timeout,
			rx_attn->msdu_limit_error,
			rx_attn->da_is_valid,
			rx_attn->da_is_mcbc,
			rx_attn->sa_is_valid,
			rx_attn->decrypt_status_code,
			rx_attn->rx_bitmap_not_updated,
			rx_attn->reserved_2,
			rx_attn->msdu_done);


}

/**
 * hal_rx_dump_mpdu_start_tlv: dump RX mpdu_start TLV in structured
 *			       human readable format.
 * @ mpdu_start: pointer the rx_attention TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_mpdu_start_tlv(struct rx_mpdu_start *mpdu_start,
uint8_t dbg_level)
{
	struct	rx_mpdu_info *mpdu_info =
		(struct rx_mpdu_info *) &mpdu_start->rx_mpdu_info_details;
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_mpdu_start tlv - "
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"ndp_frame: %d "
			"phy_err: %d "
			"phy_err_during_mpdu_header: %d "
			"protocol_version_err: %d "
			"ast_based_lookup_valid: %d "
			"phy_ppdu_id: %d "
			"ast_index: %d "
			"sw_peer_id: %d "
			"mpdu_frame_control_valid: %d "
			"mpdu_duration_valid: %d "
			"mac_addr_ad1_valid: %d "
			"mac_addr_ad2_valid: %d "
			"mac_addr_ad3_valid: %d "
			"mac_addr_ad4_valid: %d "
			"mpdu_sequence_control_valid: %d "
			"mpdu_qos_control_valid: %d "
			"mpdu_ht_control_valid: %d "
			"frame_encryption_info_valid: %d "
			"fr_ds: %d "
			"to_ds: %d "
			"encrypted: %d "
			"mpdu_retry: %d "
			"mpdu_sequence_number: %d "
			"epd_en: %d "
			"all_frames_shall_be_encrypted: %d "
			"encrypt_type: %d "
			"mesh_sta: %d "
			"bssid_hit: %d "
			"bssid_number: %d "
			"tid: %d "
			"pn_31_0: %d "
			"pn_63_32: %d "
			"pn_95_64: %d "
			"pn_127_96: %d "
			"peer_meta_data: %d "
			"rxpt_classify_info.reo_destination_indication: %d "
			"rxpt_classify_info.use_flow_id_toeplitz_clfy: %d "
			"rx_reo_queue_desc_addr_31_0: %d "
			"rx_reo_queue_desc_addr_39_32: %d "
			"receive_queue_number: %d "
			"pre_delim_err_warning: %d "
			"first_delim_err: %d "
			"key_id_octet: %d "
			"new_peer_entry: %d "
			"decrypt_needed: %d "
			"decap_type: %d "
			"rx_insert_vlan_c_tag_padding: %d "
			"rx_insert_vlan_s_tag_padding: %d "
			"strip_vlan_c_tag_decap: %d "
			"strip_vlan_s_tag_decap: %d "
			"pre_delim_count: %d "
			"ampdu_flag: %d "
			"bar_frame: %d "
			"mpdu_length: %d "
			"first_mpdu: %d "
			"mcast_bcast: %d "
			"ast_index_not_found: %d "
			"ast_index_timeout: %d "
			"power_mgmt: %d "
			"non_qos: %d "
			"null_data: %d "
			"mgmt_type: %d "
			"ctrl_type: %d "
			"more_data: %d "
			"eosp: %d "
			"fragment_flag: %d "
			"order: %d "
			"u_apsd_trigger: %d "
			"encrypt_required: %d "
			"directed: %d "
			"mpdu_frame_control_field: %d "
			"mpdu_duration_field: %d "
			"mac_addr_ad1_31_0: %d "
			"mac_addr_ad1_47_32: %d "
			"mac_addr_ad2_15_0: %d "
			"mac_addr_ad2_47_16: %d "
			"mac_addr_ad3_31_0: %d "
			"mac_addr_ad3_47_32: %d "
			"mpdu_sequence_control_field: %d "
			"mac_addr_ad4_31_0: %d "
			"mac_addr_ad4_47_32: %d "
			"mpdu_qos_control_field: %d "
			"mpdu_ht_control_field: %d ",
			mpdu_info->rxpcu_mpdu_filter_in_category,
			mpdu_info->sw_frame_group_id,
			mpdu_info->ndp_frame,
			mpdu_info->phy_err,
			mpdu_info->phy_err_during_mpdu_header,
			mpdu_info->protocol_version_err,
			mpdu_info->ast_based_lookup_valid,
			mpdu_info->phy_ppdu_id,
			mpdu_info->ast_index,
			mpdu_info->sw_peer_id,
			mpdu_info->mpdu_frame_control_valid,
			mpdu_info->mpdu_duration_valid,
			mpdu_info->mac_addr_ad1_valid,
			mpdu_info->mac_addr_ad2_valid,
			mpdu_info->mac_addr_ad3_valid,
			mpdu_info->mac_addr_ad4_valid,
			mpdu_info->mpdu_sequence_control_valid,
			mpdu_info->mpdu_qos_control_valid,
			mpdu_info->mpdu_ht_control_valid,
			mpdu_info->frame_encryption_info_valid,
			mpdu_info->fr_ds,
			mpdu_info->to_ds,
			mpdu_info->encrypted,
			mpdu_info->mpdu_retry,
			mpdu_info->mpdu_sequence_number,
			mpdu_info->epd_en,
			mpdu_info->all_frames_shall_be_encrypted,
			mpdu_info->encrypt_type,
			mpdu_info->mesh_sta,
			mpdu_info->bssid_hit,
			mpdu_info->bssid_number,
			mpdu_info->tid,
			mpdu_info->pn_31_0,
			mpdu_info->pn_63_32,
			mpdu_info->pn_95_64,
			mpdu_info->pn_127_96,
			mpdu_info->peer_meta_data,
			mpdu_info->rxpt_classify_info_details.reo_destination_indication,
			mpdu_info->rxpt_classify_info_details.use_flow_id_toeplitz_clfy,
			mpdu_info->rx_reo_queue_desc_addr_31_0,
			mpdu_info->rx_reo_queue_desc_addr_39_32,
			mpdu_info->receive_queue_number,
			mpdu_info->pre_delim_err_warning,
			mpdu_info->first_delim_err,
			mpdu_info->key_id_octet,
			mpdu_info->new_peer_entry,
			mpdu_info->decrypt_needed,
			mpdu_info->decap_type,
			mpdu_info->rx_insert_vlan_c_tag_padding,
			mpdu_info->rx_insert_vlan_s_tag_padding,
			mpdu_info->strip_vlan_c_tag_decap,
			mpdu_info->strip_vlan_s_tag_decap,
			mpdu_info->pre_delim_count,
			mpdu_info->ampdu_flag,
			mpdu_info->bar_frame,
			mpdu_info->mpdu_length,
			mpdu_info->first_mpdu,
			mpdu_info->mcast_bcast,
			mpdu_info->ast_index_not_found,
			mpdu_info->ast_index_timeout,
			mpdu_info->power_mgmt,
			mpdu_info->non_qos,
			mpdu_info->null_data,
			mpdu_info->mgmt_type,
			mpdu_info->ctrl_type,
			mpdu_info->more_data,
			mpdu_info->eosp,
			mpdu_info->fragment_flag,
			mpdu_info->order,
			mpdu_info->u_apsd_trigger,
			mpdu_info->encrypt_required,
			mpdu_info->directed,
			mpdu_info->mpdu_frame_control_field,
			mpdu_info->mpdu_duration_field,
			mpdu_info->mac_addr_ad1_31_0,
			mpdu_info->mac_addr_ad1_47_32,
			mpdu_info->mac_addr_ad2_15_0,
			mpdu_info->mac_addr_ad2_47_16,
			mpdu_info->mac_addr_ad3_31_0,
			mpdu_info->mac_addr_ad3_47_32,
			mpdu_info->mpdu_sequence_control_field,
			mpdu_info->mac_addr_ad4_31_0,
			mpdu_info->mac_addr_ad4_47_32,
			mpdu_info->mpdu_qos_control_field,
			mpdu_info->mpdu_ht_control_field);
}

/**
 * hal_rx_dump_msdu_start_tlv: dump RX msdu_start TLV in structured
 *			       human readable format.
 * @ msdu_start: pointer the msdu_start TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_start_tlv(struct rx_msdu_start *msdu_start,
							uint8_t dbg_level)
{
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_msdu_start tlv - "
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"phy_ppdu_id: %d "
			"msdu_length: %d "
			"ipsec_esp: %d "
			"l3_offset: %d "
			"ipsec_ah: %d "
			"l4_offset: %d "
			"msdu_number: %d "
			"decap_format: %d "
			"ipv4_proto: %d "
			"ipv6_proto: %d "
			"tcp_proto: %d "
			"udp_proto: %d "
			"ip_frag: %d "
			"tcp_only_ack: %d "
			"da_is_bcast_mcast: %d "
			"ip4_protocol_ip6_next_header: %d "
			"toeplitz_hash_2_or_4: %d "
			"flow_id_toeplitz: %d "
			"user_rssi: %d "
			"pkt_type: %d "
			"stbc: %d "
			"sgi: %d "
			"rate_mcs: %d "
			"receive_bandwidth: %d "
			"reception_type: %d "
#if !defined(QCA_WIFI_QCA6290_11AX)
			"toeplitz_hash: %d "
			"nss: %d "
#endif
			"ppdu_start_timestamp: %d "
			"sw_phy_meta_data: %d ",
			msdu_start->rxpcu_mpdu_filter_in_category,
			msdu_start->sw_frame_group_id,
			msdu_start->phy_ppdu_id,
			msdu_start->msdu_length,
			msdu_start->ipsec_esp,
			msdu_start->l3_offset,
			msdu_start->ipsec_ah,
			msdu_start->l4_offset,
			msdu_start->msdu_number,
			msdu_start->decap_format,
			msdu_start->ipv4_proto,
			msdu_start->ipv6_proto,
			msdu_start->tcp_proto,
			msdu_start->udp_proto,
			msdu_start->ip_frag,
			msdu_start->tcp_only_ack,
			msdu_start->da_is_bcast_mcast,
			msdu_start->ip4_protocol_ip6_next_header,
			msdu_start->toeplitz_hash_2_or_4,
			msdu_start->flow_id_toeplitz,
			msdu_start->user_rssi,
			msdu_start->pkt_type,
			msdu_start->stbc,
			msdu_start->sgi,
			msdu_start->rate_mcs,
			msdu_start->receive_bandwidth,
			msdu_start->reception_type,
#if !defined(QCA_WIFI_QCA6290_11AX)
			msdu_start->toeplitz_hash,
			msdu_start->nss,
#endif
			msdu_start->ppdu_start_timestamp,
			msdu_start->sw_phy_meta_data);
}

/**
 * hal_rx_dump_msdu_end_tlv: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_msdu_end_tlv(struct rx_msdu_end *msdu_end,
							uint8_t dbg_level)
{
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_msdu_end tlv - "
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"phy_ppdu_id: %d "
			"ip_hdr_chksum: %d "
			"tcp_udp_chksum: %d "
			"key_id_octet: %d "
			"cce_super_rule: %d "
			"cce_classify_not_done_truncat: %d "
			"cce_classify_not_done_cce_dis: %d "
			"ext_wapi_pn_63_48: %d "
			"ext_wapi_pn_95_64: %d "
			"ext_wapi_pn_127_96: %d "
			"reported_mpdu_length: %d "
			"first_msdu: %d "
			"last_msdu: %d "
			"sa_idx_timeout: %d "
			"da_idx_timeout: %d "
			"msdu_limit_error: %d "
			"flow_idx_timeout: %d "
			"flow_idx_invalid: %d "
			"wifi_parser_error: %d "
			"amsdu_parser_error: %d "
			"sa_is_valid: %d "
			"da_is_valid: %d "
			"da_is_mcbc: %d "
			"l3_header_padding: %d "
			"ipv6_options_crc: %d "
			"tcp_seq_number: %d "
			"tcp_ack_number: %d "
			"tcp_flag: %d "
			"lro_eligible: %d "
			"window_size: %d "
			"da_offset: %d "
			"sa_offset: %d "
			"da_offset_valid: %d "
			"sa_offset_valid: %d "
			"rule_indication_31_0: %d "
			"rule_indication_63_32: %d "
			"sa_idx: %d "
			"da_idx: %d "
			"msdu_drop: %d "
			"reo_destination_indication: %d "
			"flow_idx: %d "
			"fse_metadata: %d "
			"cce_metadata: %d "
			"sa_sw_peer_id: %d ",
			msdu_end->rxpcu_mpdu_filter_in_category,
			msdu_end->sw_frame_group_id,
			msdu_end->phy_ppdu_id,
			msdu_end->ip_hdr_chksum,
			msdu_end->tcp_udp_chksum,
			msdu_end->key_id_octet,
			msdu_end->cce_super_rule,
			msdu_end->cce_classify_not_done_truncate,
			msdu_end->cce_classify_not_done_cce_dis,
			msdu_end->ext_wapi_pn_63_48,
			msdu_end->ext_wapi_pn_95_64,
			msdu_end->ext_wapi_pn_127_96,
			msdu_end->reported_mpdu_length,
			msdu_end->first_msdu,
			msdu_end->last_msdu,
			msdu_end->sa_idx_timeout,
			msdu_end->da_idx_timeout,
			msdu_end->msdu_limit_error,
			msdu_end->flow_idx_timeout,
			msdu_end->flow_idx_invalid,
			msdu_end->wifi_parser_error,
			msdu_end->amsdu_parser_error,
			msdu_end->sa_is_valid,
			msdu_end->da_is_valid,
			msdu_end->da_is_mcbc,
			msdu_end->l3_header_padding,
			msdu_end->ipv6_options_crc,
			msdu_end->tcp_seq_number,
			msdu_end->tcp_ack_number,
			msdu_end->tcp_flag,
			msdu_end->lro_eligible,
			msdu_end->window_size,
			msdu_end->da_offset,
			msdu_end->sa_offset,
			msdu_end->da_offset_valid,
			msdu_end->sa_offset_valid,
			msdu_end->rule_indication_31_0,
			msdu_end->rule_indication_63_32,
			msdu_end->sa_idx,
			msdu_end->da_idx,
			msdu_end->msdu_drop,
			msdu_end->reo_destination_indication,
			msdu_end->flow_idx,
			msdu_end->fse_metadata,
			msdu_end->cce_metadata,
			msdu_end->sa_sw_peer_id);
}

/**
 * hal_rx_dump_mpdu_end_tlv: dump RX mpdu_end TLV in structured
 *			     human readable format.
 * @ mpdu_end: pointer the mpdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_mpdu_end_tlv(struct rx_mpdu_end *mpdu_end,
							uint8_t dbg_level)
{
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"rx_mpdu_end tlv - "
			"rxpcu_mpdu_filter_in_category: %d "
			"sw_frame_group_id: %d "
			"phy_ppdu_id: %d "
			"unsup_ktype_short_frame: %d "
			"rx_in_tx_decrypt_byp: %d "
			"overflow_err: %d "
			"mpdu_length_err: %d "
			"tkip_mic_err: %d "
			"decrypt_err: %d "
			"unencrypted_frame_err: %d "
			"pn_fields_contain_valid_info: %d "
			"fcs_err: %d "
			"msdu_length_err: %d "
			"rxdma0_destination_ring: %d "
			"rxdma1_destination_ring: %d "
			"decrypt_status_code: %d "
			"rx_bitmap_not_updated: %d ",
			mpdu_end->rxpcu_mpdu_filter_in_category,
			mpdu_end->sw_frame_group_id,
			mpdu_end->phy_ppdu_id,
			mpdu_end->unsup_ktype_short_frame,
			mpdu_end->rx_in_tx_decrypt_byp,
			mpdu_end->overflow_err,
			mpdu_end->mpdu_length_err,
			mpdu_end->tkip_mic_err,
			mpdu_end->decrypt_err,
			mpdu_end->unencrypted_frame_err,
			mpdu_end->pn_fields_contain_valid_info,
			mpdu_end->fcs_err,
			mpdu_end->msdu_length_err,
			mpdu_end->rxdma0_destination_ring,
			mpdu_end->rxdma1_destination_ring,
			mpdu_end->decrypt_status_code,
			mpdu_end->rx_bitmap_not_updated);
}

/**
 * hal_rx_dump_pkt_hdr_tlv: dump RX pkt header TLV in hex format
 * @ pkt_hdr_tlv: pointer the pkt_hdr_tlv in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_pkt_hdr_tlv(struct rx_pkt_hdr_tlv  *pkt_hdr_tlv,
							uint8_t dbg_level)
{
	QDF_TRACE(QDF_MODULE_ID_DP, dbg_level,
			"\n---------------\n"
			"rx_pkt_hdr_tlv \n"
			"---------------\n"
			"phy_ppdu_id %d \n",
			pkt_hdr_tlv->phy_ppdu_id);

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, dbg_level,
			pkt_hdr_tlv->rx_pkt_hdr, 128);
}

/**
 * hal_rx_dump_pkt_tlvs: API to print all member elements of
 *			 RX TLVs
 * @ buf: pointer the pkt buffer.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_pkt_tlvs(uint8_t *buf, uint8_t dbg_level)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *) buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	struct rx_mpdu_start *mpdu_start =
				&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	struct rx_mpdu_end *mpdu_end = &pkt_tlvs->mpdu_end_tlv.rx_mpdu_end;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	struct rx_pkt_hdr_tlv    *pkt_hdr_tlv = &pkt_tlvs->pkt_hdr_tlv;

	hal_rx_dump_rx_attention_tlv(rx_attn, dbg_level);
	hal_rx_dump_mpdu_start_tlv(mpdu_start, dbg_level);
	hal_rx_dump_msdu_start_tlv(msdu_start, dbg_level);
	hal_rx_dump_mpdu_end_tlv(mpdu_end, dbg_level);
	hal_rx_dump_msdu_end_tlv(msdu_end, dbg_level);
	hal_rx_dump_pkt_hdr_tlv(pkt_hdr_tlv, dbg_level);
}

/**
 * hal_srng_ring_id_get: API to retrieve ring id from hal ring
 *                       structure
 * @hal_ring: pointer to hal_srng structure
 *
 * Return: ring_id
 */
static inline uint8_t hal_srng_ring_id_get(void *hal_ring)
{
	return ((struct hal_srng *)hal_ring)->ring_id;
}

/* Rx MSDU link pointer info */
struct hal_rx_msdu_link_ptr_info {
	struct rx_msdu_link msdu_link;
	struct hal_buf_info msdu_link_buf_info;
};

/**
 * hal_rx_get_pkt_tlvs(): Function to retrieve pkt tlvs from nbuf
 *
 * @nbuf: Pointer to data buffer field
 * Returns: pointer to rx_pkt_tlvs
 */
static inline
struct rx_pkt_tlvs *hal_rx_get_pkt_tlvs(uint8_t *rx_buf_start)
{
	return (struct rx_pkt_tlvs *)rx_buf_start;
}

/**
 * hal_rx_get_mpdu_info(): Function to retrieve mpdu info from pkt tlvs
 *
 * @pkt_tlvs: Pointer to pkt_tlvs
 * Returns: pointer to rx_mpdu_info structure
 */
static inline
struct rx_mpdu_info *hal_rx_get_mpdu_info(struct rx_pkt_tlvs *pkt_tlvs)
{
	return &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;
}

/**
 * hal_rx_get_rx_sequence(): Function to retrieve rx sequence number
 *
 * @nbuf: Network buffer
 * Returns: rx sequence number
 */
#define DOT11_SEQ_FRAG_MASK		0x000f
#define DOT11_FC1_MORE_FRAG_OFFSET	0x04

#define HAL_RX_MPDU_GET_SEQUENCE_NUMBER(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_NUMBER_OFFSET)),	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_NUMBER_MASK,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_NUMBER_LSB))
static inline
uint16_t hal_rx_get_rx_sequence(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint16_t seq_number = 0;

	seq_number =
		HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info) >> 4;

	/* Skip first 4-bits for fragment number */
	return seq_number;
}

/**
 * hal_rx_get_rx_fragment_number(): Function to retrieve rx fragment number
 *
 * @nbuf: Network buffer
 * Returns: rx fragment number
 */
static inline
uint8_t hal_rx_get_rx_fragment_number(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint8_t frag_number = 0;

	frag_number = HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info) &
		DOT11_SEQ_FRAG_MASK;

	/* Return first 4 bits as fragment number */
	return frag_number;
}

#define HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD_OFFSET)),	\
		RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD_MASK,	\
		RX_MPDU_INFO_14_MPDU_FRAME_CONTROL_FIELD_LSB))
/**
 * hal_rx_get_rx_more_frag_bit(): Function to retrieve more fragment bit
 *
 * @nbuf: Network buffer
 * Returns: rx more fragment bit
 */
static inline
uint8_t hal_rx_get_rx_more_frag_bit(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint16_t frame_ctrl = 0;

	frame_ctrl = HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(rx_mpdu_info) >>
		DOT11_FC1_MORE_FRAG_OFFSET;

	/* more fragment bit if at offset bit 4 */
	return frame_ctrl;
}

/**
 * hal_rx_get_frame_ctrl_field(): Function to retrieve frame control field
 *
 * @nbuf: Network buffer
 * Returns: rx more fragment bit
 *
 */
static inline
uint16_t hal_rx_get_frame_ctrl_field(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint16_t frame_ctrl = 0;

	frame_ctrl = HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(rx_mpdu_info);

	return frame_ctrl;
}

/*
 * hal_rx_msdu_is_wlan_mcast(): Check if the buffer is for multicast address
 *
 * @nbuf: Network buffer
 * Returns: flag to indicate whether the nbuf has MC/BC address
 */
static inline
uint32_t hal_rx_msdu_is_wlan_mcast(qdf_nbuf_t nbuf)
{
	uint8 *buf = qdf_nbuf_data(nbuf);

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;

	return rx_attn->mcast_bcast;
}

#define HAL_RX_MPDU_GET_SEQUENCE_CONTROL_VALID(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_OFFSET)),	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_MASK,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_LSB))
/*
 * hal_rx_get_mpdu_sequence_control_valid(): Get mpdu sequence control valid
 *
 * @nbuf: Network buffer
 * Returns: value of sequence control valid field
 */
static inline
uint8_t hal_rx_get_mpdu_sequence_control_valid(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint8_t seq_ctrl_valid = 0;

	seq_ctrl_valid =
		HAL_RX_MPDU_GET_SEQUENCE_CONTROL_VALID(rx_mpdu_info);

	return seq_ctrl_valid;
}

#define HAL_RX_MPDU_GET_FRAME_CONTROL_VALID(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_MPDU_FRAME_CONTROL_VALID_OFFSET)),	\
		RX_MPDU_INFO_2_MPDU_FRAME_CONTROL_VALID_MASK,	\
		RX_MPDU_INFO_2_MPDU_FRAME_CONTROL_VALID_LSB))
/*
 * hal_rx_get_mpdu_frame_control_valid(): Retrieves mpdu frame control valid
 *
 * @nbuf: Network buffer
 * Returns: value of frame control valid field
 */
static inline
uint8_t hal_rx_get_mpdu_frame_control_valid(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint8_t frm_ctrl_valid = 0;

	frm_ctrl_valid =
		HAL_RX_MPDU_GET_FRAME_CONTROL_VALID(rx_mpdu_info);

	return frm_ctrl_valid;
}

#define HAL_RX_MPDU_GET_MAC_AD4_VALID(_rx_mpdu_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,		\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_OFFSET)),	\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_MASK,		\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_LSB))
/*
 * hal_rx_get_mpdu_mac_ad4_valid(): Retrieves if mpdu 4th addr is valid
 *
 * @nbuf: Network buffer
 * Returns: value of mpdu 4th address valid field
 */
static inline
bool hal_rx_get_mpdu_mac_ad4_valid(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	bool ad4_valid = 0;

	ad4_valid = HAL_RX_MPDU_GET_MAC_AD4_VALID(rx_mpdu_info);

	return ad4_valid;
}

/*
 * hal_rx_clear_mpdu_desc_info(): Clears mpdu_desc_info
 *
 * @rx_mpdu_desc_info: HAL view of rx mpdu desc info
 * Returns: None
 */
static inline
void hal_rx_clear_mpdu_desc_info(
		struct hal_rx_mpdu_desc_info *rx_mpdu_desc_info)
{
	qdf_mem_zero(rx_mpdu_desc_info,
		sizeof(*rx_mpdu_desc_info));
}

/*
 * hal_rx_clear_msdu_link_ptr(): Clears msdu_link_ptr
 *
 * @msdu_link_ptr: HAL view of msdu link ptr
 * @size: number of msdu link pointers
 * Returns: None
 */
static inline
void hal_rx_clear_msdu_link_ptr(struct hal_rx_msdu_link_ptr_info *msdu_link_ptr,
				int size)
{
	qdf_mem_zero(msdu_link_ptr,
		(sizeof(*msdu_link_ptr) * size));
}

/*
 * hal_rx_chain_msdu_links() - Chains msdu link pointers
 * @msdu_link_ptr: msdu link pointer
 * @mpdu_desc_info: mpdu descriptor info
 *
 * Build a list of msdus using msdu link pointer. If the
 * number of msdus are more, chain them together
 *
 * Returns: Number of processed msdus
 */
static inline
int hal_rx_chain_msdu_links(qdf_nbuf_t msdu,
	struct hal_rx_msdu_link_ptr_info *msdu_link_ptr_info,
	struct hal_rx_mpdu_desc_info *mpdu_desc_info)
{
	int j;
	struct rx_msdu_link *msdu_link_ptr =
		&msdu_link_ptr_info->msdu_link;
	struct rx_msdu_link *prev_msdu_link_ptr = NULL;
	struct rx_msdu_details *msdu_details =
		HAL_RX_LINK_DESC_MSDU0_PTR(msdu_link_ptr);
	uint8_t num_msdus = mpdu_desc_info->msdu_count;
	struct rx_msdu_desc_info *msdu_desc_info;
	uint8_t fragno, more_frag;
	uint8_t *rx_desc_info;
	struct hal_rx_msdu_list msdu_list;

	for (j = 0; j < num_msdus; j++) {
		msdu_desc_info =
			HAL_RX_MSDU_DESC_INFO_GET(&msdu_details[j]);
		msdu_list.msdu_info[j].msdu_flags =
			HAL_RX_MSDU_FLAGS_GET(msdu_desc_info);
		msdu_list.msdu_info[j].msdu_len =
			HAL_RX_MSDU_PKT_LENGTH_GET(msdu_desc_info);
		msdu_list.sw_cookie[j] = HAL_RX_BUF_COOKIE_GET(
				&msdu_details[j].buffer_addr_info_details);
	}

	/* Chain msdu links together */
	if (prev_msdu_link_ptr) {
		/* 31-0 bits of the physical address */
		prev_msdu_link_ptr->
			next_msdu_link_desc_addr_info.buffer_addr_31_0 =
			msdu_link_ptr_info->msdu_link_buf_info.paddr &
			BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK;
		/* 39-32 bits of the physical address */
		prev_msdu_link_ptr->
			next_msdu_link_desc_addr_info.buffer_addr_39_32
			= ((msdu_link_ptr_info->msdu_link_buf_info.paddr
						>> 32) &&
				BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK);
		prev_msdu_link_ptr->
			next_msdu_link_desc_addr_info.sw_buffer_cookie =
			msdu_link_ptr_info->msdu_link_buf_info.sw_cookie;
	}

	/* There is space for only 6 MSDUs in a MSDU link descriptor */
	if (num_msdus < HAL_RX_NUM_MSDU_DESC) {
		/* mark first and last MSDUs */
		rx_desc_info = qdf_nbuf_data(msdu);
		fragno = hal_rx_get_rx_fragment_number(rx_desc_info);
		more_frag = hal_rx_get_rx_more_frag_bit(rx_desc_info);

		/* TODO: create skb->fragslist[] */

		if (more_frag == 0) {
			msdu_list.msdu_info[num_msdus].msdu_flags |=
				RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_MASK;
		} else if (fragno == 1) {
			msdu_list.msdu_info[num_msdus].msdu_flags |=
			RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_MASK;

			msdu_list.msdu_info[num_msdus].msdu_flags |=
				RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_MASK;
		}

		num_msdus++;

		/* Number of MSDUs per mpdu descriptor is updated */
		mpdu_desc_info->msdu_count += num_msdus;
	} else {
		num_msdus = 0;
		prev_msdu_link_ptr = msdu_link_ptr;
	}

	return num_msdus;
}

/*
 * hal_rx_defrag_update_src_ring_desc(): updates reo src ring desc
 *
 * @ring_desc: HAL view of ring descriptor
 * @mpdu_des_info: saved mpdu desc info
 * @msdu_link_ptr: saved msdu link ptr
 *
 * API used explicitly for rx defrag to update ring desc with
 * mpdu desc info and msdu link ptr before reinjecting the
 * packet back to REO
 *
 * Returns: None
 */
static inline
void hal_rx_defrag_update_src_ring_desc(void *ring_desc,
	void *saved_mpdu_desc_info,
	struct hal_rx_msdu_link_ptr_info *saved_msdu_link_ptr)
{
	struct reo_entrance_ring *reo_ent_ring;
	struct rx_mpdu_desc_info *reo_ring_mpdu_desc_info;
	struct hal_buf_info buf_info;

	reo_ent_ring = (struct reo_entrance_ring *)ring_desc;
	reo_ring_mpdu_desc_info = &reo_ent_ring->
		reo_level_mpdu_frame_info.rx_mpdu_desc_info_details;

	qdf_mem_copy(&reo_ring_mpdu_desc_info, saved_mpdu_desc_info,
		sizeof(*reo_ring_mpdu_desc_info));

	/*
	 * TODO: Check for additional fields that need configuration in
	 * reo_ring_mpdu_desc_info
	 */

	/* Update msdu_link_ptr in the reo entrance ring */
	hal_rx_reo_buf_paddr_get(ring_desc, &buf_info);
	buf_info.paddr = saved_msdu_link_ptr->msdu_link_buf_info.paddr;
	buf_info.sw_cookie =
		saved_msdu_link_ptr->msdu_link_buf_info.sw_cookie;
}

/*
 * hal_rx_defrag_save_info_from_ring_desc(): Saves info from ring desc
 *
 * @msdu_link_desc_va: msdu link descriptor handle
 * @msdu_link_ptr_info: HAL view of msdu link pointer info
 *
 * API used to save msdu link information along with physical
 * address. The API also copues the sw cookie.
 *
 * Returns: None
 */
static inline
void hal_rx_defrag_save_info_from_ring_desc(void *msdu_link_desc_va,
	struct hal_rx_msdu_link_ptr_info *msdu_link_ptr_info,
	struct hal_buf_info *hbi)
{
	struct rx_msdu_link *msdu_link_ptr =
		(struct rx_msdu_link *)msdu_link_desc_va;

	qdf_mem_copy(&msdu_link_ptr_info->msdu_link, msdu_link_ptr,
		sizeof(struct rx_msdu_link));

	msdu_link_ptr_info->msdu_link_buf_info.paddr = hbi->paddr;
	msdu_link_ptr_info->msdu_link_buf_info.sw_cookie = hbi->sw_cookie;
}

/*
 * hal_rx_get_desc_len(): Returns rx descriptor length
 *
 * Returns the size of rx_pkt_tlvs which follows the
 * data in the nbuf
 *
 * Returns: Length of rx descriptor
 */
static inline
uint16_t hal_rx_get_desc_len(void)
{
	return sizeof(struct rx_pkt_tlvs);
}

/*
 * hal_rx_reo_ent_rxdma_push_reason_get(): Retrieves RXDMA push reason from
 *	reo_entrance_ring descriptor
 *
 * @reo_ent_desc: reo_entrance_ring descriptor
 * Returns: value of rxdma_push_reason
 */
static inline
uint8_t hal_rx_reo_ent_rxdma_push_reason_get(void *reo_ent_desc)
{
	return _HAL_MS((*_OFFSET_TO_WORD_PTR(reo_ent_desc,
		REO_ENTRANCE_RING_6_RXDMA_PUSH_REASON_OFFSET)),
		REO_ENTRANCE_RING_6_RXDMA_PUSH_REASON_MASK,
		REO_ENTRANCE_RING_6_RXDMA_PUSH_REASON_LSB);
}

/**
 * hal_rx_reo_ent_rxdma_error_code_get(): Retrieves RXDMA error code from
 *	reo_entrance_ring descriptor
 * @reo_ent_desc: reo_entrance_ring descriptor
 * Return: value of rxdma_error_code
 */
static inline
uint8_t hal_rx_reo_ent_rxdma_error_code_get(void *reo_ent_desc)
{
	return _HAL_MS((*_OFFSET_TO_WORD_PTR(reo_ent_desc,
		REO_ENTRANCE_RING_6_RXDMA_ERROR_CODE_OFFSET)),
		REO_ENTRANCE_RING_6_RXDMA_ERROR_CODE_MASK,
		REO_ENTRANCE_RING_6_RXDMA_ERROR_CODE_LSB);
}

/**
 * hal_rx_wbm_err_info_get(): Retrieves WBM error code and reason and
 *	save it to hal_wbm_err_desc_info structure passed by caller
 * @wbm_desc: wbm ring descriptor
 * @wbm_er_info: hal_wbm_err_desc_info structure, output parameter.
 * Return: void
 */
static inline void hal_rx_wbm_err_info_get(void *wbm_desc,
				struct hal_wbm_err_desc_info *wbm_er_info)
{
	wbm_er_info->wbm_err_src = HAL_RX_WBM_ERR_SRC_GET(wbm_desc);
	wbm_er_info->reo_psh_rsn = HAL_RX_WBM_REO_PUSH_REASON_GET(wbm_desc);
	wbm_er_info->reo_err_code = HAL_RX_WBM_REO_ERROR_CODE_GET(wbm_desc);
	wbm_er_info->rxdma_psh_rsn = HAL_RX_WBM_RXDMA_PUSH_REASON_GET(wbm_desc);
	wbm_er_info->rxdma_err_code = HAL_RX_WBM_RXDMA_ERROR_CODE_GET(wbm_desc);
}

/**
 * hal_rx_wbm_err_info_set_in_tlv(): Save the wbm error codes and reason to
 *	the reserved bytes of rx_tlv_hdr
 * @buf: start of rx_tlv_hdr
 * @wbm_er_info: hal_wbm_err_desc_info structure
 * Return: void
 */
static inline void hal_rx_wbm_err_info_set_in_tlv(uint8_t *buf,
				struct hal_wbm_err_desc_info *wbm_er_info)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	qdf_mem_copy(pkt_tlvs->rx_padding0, wbm_er_info,
		    sizeof(struct hal_wbm_err_desc_info));
}

/**
 * hal_rx_wbm_err_info_get_from_tlv(): retrieve wbm error codes and reason from
 *	the reserved bytes of rx_tlv_hdr.
 * @buf: start of rx_tlv_hdr
 * @wbm_er_info: hal_wbm_err_desc_info structure, output parameter.
 * Return: void
 */
static inline void hal_rx_wbm_err_info_get_from_tlv(uint8_t *buf,
				struct hal_wbm_err_desc_info *wbm_er_info)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	qdf_mem_copy(wbm_er_info, pkt_tlvs->rx_padding0,
		    sizeof(struct hal_wbm_err_desc_info));
}

#endif /* _HAL_RX_H */

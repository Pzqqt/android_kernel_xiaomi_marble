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

#ifndef _HAL_RX_H_
#define _HAL_RX_H_

#include <hal_internal.h>

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
		~((cookie << BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB) & \
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK)); \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) |= \
		(cookie << BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB) & \
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK)

/*
 * macro to set the manager into the rxdma ring entry
 */
#define HAL_RXDMA_MANAGER_SET(buff_addr_info, manager) \
		((*(((unsigned int *) buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET >> 2))) &= \
		~((manager << BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB) & \
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK)); \
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

#define HAL_RX_MPDU_PEER_META_DATA_GET(mpdu_info_ptr)	\
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

static inline void hal_rx_mpdu_info_get(void *desc_addr,
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
		HAL_RX_MPDU_PEER_META_DATA_GET(mpdu_info);
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
			qdf_dma_addr_t paddr, uint16_t cookie, uint8_t manager)
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
#define RX_PADDING_BYTES	16
struct rx_pkt_tlvs {
	struct rx_msdu_end_tlv   msdu_end_tlv;	/*  72 bytes */
	struct rx_attention_tlv  attn_tlv;	/*  16 bytes */
	struct rx_msdu_start_tlv msdu_start_tlv;/*  40 bytes */
	struct rx_mpdu_start_tlv mpdu_start_tlv;/*  96 bytes */
	struct rx_mpdu_end_tlv   mpdu_end_tlv;	/*  12 bytes */
	uint8_t rx_padding[RX_PADDING_BYTES];	/*  20 bytes */
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

/*
 * Get l3_header_padding from RX_MSDU_END
 */
#define HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_9_L3_HEADER_PADDING_OFFSET)),	\
		RX_MSDU_END_9_L3_HEADER_PADDING_MASK,		\
		RX_MSDU_END_9_L3_HEADER_PADDING_LSB))

static inline uint32_t
hal_rx_msdu_end_l3_hdr_padding_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t l3_header_padding;

	l3_header_padding = HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(msdu_end);

	return l3_header_padding;
}
/*******************************************************************************
 * RX ERROR APIS
 ******************************************************************************/

/*******************************************************************************
 * RX REO ERROR APIS
 ******************************************************************************/

#define HAL_RX_LINK_DESC_MSDU0_PTR(link_desc)	\
	((struct rx_msdu_details *)		\
		_OFFSET_TO_BYTE_PTR((link_desc),\
		RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET))

#define HAL_RX_NUM_MSDU_DESC 6

struct hal_rx_msdu_list {
	struct hal_rx_msdu_desc_info msdu_info[HAL_RX_NUM_MSDU_DESC];
	uint32_t sw_cookie[HAL_RX_NUM_MSDU_DESC];
};

struct hal_buf_info {
	uint64_t paddr;
	uint32_t sw_cookie;
};

/**
 * hal_rx_msdu_link_desc_get: API to get the MSDU information
 * from the MSDU link descriptor
 *
 * @ msdu_link_desc: Opaque pointer used by HAL to get to the
 * MSDU link descriptor (struct rx_msdu_link)
 * @ msdu_list: Return the list of MSDUs contained in this link descriptor
 * Return: void
 */
static inline void hal_rx_msdu_list_get(void *msdu_link_desc,
			struct hal_rx_msdu_list *msdu_list, uint8_t *num_msdus)
{
	struct rx_msdu_details *msdu_details;
	struct rx_msdu_desc_info *msdu_desc_info;
	struct rx_msdu_link *msdu_link = (struct rx_msdu_link *)msdu_link_desc;
	int i;

	*num_msdus = 0;
	msdu_details = HAL_RX_LINK_DESC_MSDU0_PTR(msdu_link);

	for (i = 0; i < HAL_RX_NUM_MSDU_DESC; i++) {
		msdu_desc_info = HAL_RX_MSDU_DESC_INFO_GET(&msdu_details[i]);
		msdu_list->msdu_info[i].msdu_flags =
			 HAL_RX_MSDU_FLAGS_GET(msdu_desc_info);
		msdu_list->msdu_info[i].msdu_len =
			 HAL_RX_MSDU_PKT_LENGTH_GET(msdu_desc_info);
		msdu_list->sw_cookie[i] =
			 HAL_RX_BUF_COOKIE_GET(
				&msdu_details[i].buffer_addr_info_details);
	}
	*num_msdus = i;
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
 * of the ‘Seq_2k_error_detected_flag’ been set in the REO Queue descriptor
 * @ HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET : Frame is forwarded as a result
 * of the ‘pn_error_detected_flag’ been set in the REO Queue descriptor
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
	HAL_REO_ERR_QUEUE_DESC_BLOCKED_SET
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
 *
 * Return: void
 */
/* look at implementation at dp_hw_link_desc_pool_setup()*/
static inline void hal_rx_msdu_link_desc_set(struct hal_soc *soc,
			void *src_srng_desc, void *buf_addr_info)
{
	struct wbm_release_ring *wbm_rel_srng =
			(struct wbm_release_ring *)src_srng_desc;

	/* Structure copy !!! */
	wbm_rel_srng->released_buff_or_desc_addr_info =
				*((struct buffer_addr_info *)buf_addr_info);
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

#define HAL_RX_WBM_ERR_SRC_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_OFFSET >> 2))) & \
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_MASK) >> \
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_LSB)

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

#define HAL_RX_WBM_BUF_COOKIE_GET(wbm_desc) \
	HAL_RX_BUF_COOKIE_GET(&((struct wbm_release_ring *) \
	wbm_desc)->released_buff_or_desc_addr_info)

#endif /* _HAL_RX_H */



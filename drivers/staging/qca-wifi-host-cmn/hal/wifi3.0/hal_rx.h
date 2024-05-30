/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <hal_api.h>
#include "hal_rx_hw_defines.h"
#include "hal_hw_headers.h"

/*************************************
 * Ring desc offset/shift/masks
 *************************************/
#define HAL_INVALID_PPDU_ID    0xFFFFFFFF
#define HAL_RX_OFFSET(block, field) block##_##field##_OFFSET
#define HAL_RX_LSB(block, field) block##_##field##_LSB
#define HAL_RX_MASK(block, field) block##_##field##_MASK

#define HAL_RX_GET(_ptr, block, field) \
	(((*((volatile uint32_t *)_ptr + (HAL_RX_OFFSET(block, field)>>2))) & \
	HAL_RX_MASK(block, field)) >> \
	HAL_RX_LSB(block, field))

#define HAL_RX_GET_64(_ptr, block, field) \
	(((*((volatile uint64_t *)(_ptr) + \
	(HAL_RX_OFFSET(block, field) >> 3))) & \
	HAL_RX_MASK(block, field)) >> \
	HAL_RX_LSB(block, field))

#define HAL_RX_FLD_SET(_ptr, _wrd, _field, _val)		\
	(*(uint32_t *)(((uint8_t *)_ptr) +			\
		_wrd ## _ ## _field ## _OFFSET) |=		\
		(((_val) << _wrd ## _ ## _field ## _LSB) &	\
		_wrd ## _ ## _field ## _MASK))

/* BUFFER_SIZE = 1536 data bytes + 384 RX TLV bytes + some spare bytes */
#ifndef RX_DATA_BUFFER_SIZE
#define RX_DATA_BUFFER_SIZE     2048
#endif

#ifndef RX_MONITOR_BUFFER_SIZE
#define RX_MONITOR_BUFFER_SIZE  2048
#endif

#define RXDMA_OPTIMIZATION

/* MONITOR STATUS BUFFER SIZE = 1408 data bytes, buffer allocation of 2k bytes
 * including buffer reservation, buffer alignment and skb shared info size.
 */
#define RX_MON_STATUS_BASE_BUF_SIZE    2048
#define RX_MON_STATUS_BUF_ALIGN  128
#define RX_MON_STATUS_BUF_RESERVATION  128
#define RX_MON_STATUS_BUF_SIZE  (RX_MON_STATUS_BASE_BUF_SIZE - \
				 (RX_MON_STATUS_BUF_RESERVATION + \
				  RX_MON_STATUS_BUF_ALIGN + QDF_SHINFO_SIZE))

#define NUM_OF_DWORDS_BUFFER_ADDR_INFO 2

/* HAL_RX_NON_QOS_TID = NON_QOS_TID which is 16 */
#define HAL_RX_NON_QOS_TID 16

enum {
	HAL_HW_RX_DECAP_FORMAT_RAW = 0,
	HAL_HW_RX_DECAP_FORMAT_NWIFI,
	HAL_HW_RX_DECAP_FORMAT_ETH2,
	HAL_HW_RX_DECAP_FORMAT_8023,
};

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
		msdu_continued:1,
		reserved_2:2;
};

/**
 * hal_rx_mon_dest_buf_info: Structure to hold rx mon dest buffer info
 * @first_buffer: First buffer of MSDU
 * @last_buffer: Last buffer of MSDU
 * @is_decap_raw: Is RAW Frame
 * @reserved_1: Reserved
 *
 * MSDU with continuation:
 *  -----------------------------------------------------------
 * | first_buffer:1   | first_buffer: 0 | ... | first_buffer: 0 |
 * | last_buffer :0   | last_buffer : 0 | ... | last_buffer : 0 |
 * | is_decap_raw:1/0 |      Same as earlier  |  Same as earlier|
 *  -----------------------------------------------------------
 *
 * Single buffer MSDU:
 *  ------------------
 * | first_buffer:1   |
 * | last_buffer :1   |
 * | is_decap_raw:1/0 |
 *  ------------------
 */
struct hal_rx_mon_dest_buf_info {
	uint8_t first_buffer:1,
		last_buffer:1,
		is_decap_raw:1,
		reserved_1:5;
};

/**
 * struct hal_rx_msdu_metadata:Structure to hold rx fast path information.
 *
 * @l3_hdr_pad:	l3 header padding
 * @reserved:	Reserved bits
 * @sa_sw_peer_id: sa sw peer id
 * @sa_idx: sa index
 * @da_idx: da index
 */
struct hal_rx_msdu_metadata {
	uint32_t l3_hdr_pad:16,
		 sa_sw_peer_id:16;
	uint32_t sa_idx:16,
		 da_idx:16;
};

struct hal_proto_params {
	uint8_t tcp_proto;
	uint8_t udp_proto;
	uint8_t ipv6_proto;
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
 * @ HAL_MSDU_F_INTRA_BSS: This is an intrabss packet
 */
enum hal_rx_msdu_desc_flags {
	HAL_MSDU_F_FIRST_MSDU_IN_MPDU = (0x1 << 0),
	HAL_MSDU_F_LAST_MSDU_IN_MPDU = (0x1 << 1),
	HAL_MSDU_F_MSDU_CONTINUATION = (0x1 << 2),
	HAL_MSDU_F_SA_IS_VALID = (0x1 << 23),
	HAL_MSDU_F_SA_IDX_TIMEOUT = (0x1 << 24),
	HAL_MSDU_F_DA_IS_VALID = (0x1 << 25),
	HAL_MSDU_F_DA_IS_MCBC = (0x1 << 26),
	HAL_MSDU_F_DA_IDX_TIMEOUT = (0x1 << 27),
	HAL_MSDU_F_INTRA_BSS = (0x1 << 28),
};

/*
 * @msdu_count:		no. of msdus in the MPDU
 * @mpdu_seq:		MPDU sequence number
 * @mpdu_flags          [0] Fragment flag
 *                      [1] MPDU_retry_bit
 *                      [2] AMPDU flag
 *			[3] raw_ampdu
 * @peer_meta_data:	Upper bits containing peer id, vdev id
 * @bar_frame: indicates if received frame is a bar frame
 * @tid: tid value of received MPDU
 */
struct hal_rx_mpdu_desc_info {
	uint16_t msdu_count;
	uint16_t mpdu_seq; /* 12 bits for length */
	uint32_t mpdu_flags;
	uint32_t peer_meta_data; /* sw progamed meta-data:MAC Id & peer Id */
	uint16_t bar_frame;
	uint8_t tid:4,
		reserved:4;
};

/**
 * enum hal_rx_mpdu_desc_flags: Enum for flags in MPDU_DESC_INFO
 *
 * @ HAL_MPDU_F_FRAGMENT: Fragmented MPDU (802.11 fragemtation)
 * @ HAL_MPDU_F_RETRY_BIT: Retry bit is set in FC of MPDU
 * @ HAL_MPDU_F_AMPDU_FLAG: MPDU received as part of A-MPDU
 * @ HAL_MPDU_F_RAW_AMPDU: MPDU is a Raw MDPU
 * @ HAL_MPDU_F_QOS_CONTROL_VALID: MPDU has a QoS control field
 */
enum hal_rx_mpdu_desc_flags {
	HAL_MPDU_F_FRAGMENT = (0x1 << 20),
	HAL_MPDU_F_RETRY_BIT = (0x1 << 21),
	HAL_MPDU_F_AMPDU_FLAG = (0x1 << 22),
	HAL_MPDU_F_RAW_AMPDU = (0x1 << 30),
	HAL_MPDU_F_QOS_CONTROL_VALID = (0x1 << 31)
};

/* Return Buffer manager ID */
#define HAL_RX_BUF_RBM_WBM_IDLE_BUF_LIST		0
#define HAL_RX_BUF_RBM_WBM_CHIP0_IDLE_DESC_LIST		1
#define HAL_RX_BUF_RBM_WBM_CHIP1_IDLE_DESC_LIST		2
#define HAL_RX_BUF_RBM_WBM_CHIP2_IDLE_DESC_LIST		3
#define HAL_RX_BUF_RBM_SW0_BM(sw0_bm_id)	(sw0_bm_id)
#define HAL_RX_BUF_RBM_SW1_BM(sw0_bm_id)	(sw0_bm_id + 1)
#define HAL_RX_BUF_RBM_SW2_BM(sw0_bm_id)	(sw0_bm_id + 2)
#define HAL_RX_BUF_RBM_SW3_BM(sw0_bm_id)	(sw0_bm_id + 3)
#define HAL_RX_BUF_RBM_SW4_BM(sw0_bm_id)	(sw0_bm_id + 4)
#define HAL_RX_BUF_RBM_SW5_BM(sw0_bm_id)	(sw0_bm_id + 5)
#define HAL_RX_BUF_RBM_SW6_BM(sw0_bm_id)	(sw0_bm_id + 6)

#define HAL_RX_BUF_RBM_SW_BM(sw0_bm_id, wbm2sw_id)	(sw0_bm_id + wbm2sw_id)

#define HAL_REO_DESTINATION_RING_MSDU_COUNT_OFFSET	0x8
#define HAL_REO_DESTINATION_RING_MSDU_COUNT_LSB		0
#define HAL_REO_DESTINATION_RING_MSDU_COUNT_MASK	0x000000ff

#define HAL_RX_REO_DESC_MSDU_COUNT_GET(reo_desc)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(reo_desc,	\
		 HAL_REO_DESTINATION_RING_MSDU_COUNT_OFFSET)),	\
		 HAL_REO_DESTINATION_RING_MSDU_COUNT_MASK,	\
		 HAL_REO_DESTINATION_RING_MSDU_COUNT_LSB))

#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET	0x0
#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB	0
#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK	0xffffffff

#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET	0x4
#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_LSB	0
#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MASK	0x000000ff

/*
 * macro to set the LSW of the nbuf data physical address
 * to the rxdma ring entry
 */
#define HAL_RXDMA_PADDR_LO_SET(buff_addr_info, paddr_lo) \
		((*(((unsigned int *) buff_addr_info) + \
		(HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET >> 2))) = \
		(paddr_lo << HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB) & \
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK)

/*
 * macro to set the LSB of MSW of the nbuf data physical address
 * to the rxdma ring entry
 */
#define HAL_RXDMA_PADDR_HI_SET(buff_addr_info, paddr_hi) \
		((*(((unsigned int *) buff_addr_info) + \
		(HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET >> 2))) = \
		(paddr_hi << HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_LSB) & \
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MASK)

#ifdef DP_RX_DESC_COOKIE_INVALIDATE
#define HAL_RX_COOKIE_INVALID_MASK	0x80000000

/*
 * macro to get the invalid bit for sw cookie
 */
#define HAL_RX_BUF_COOKIE_INVALID_GET(buff_addr_info) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) & \
		 HAL_RX_COOKIE_INVALID_MASK)

/*
 * macro to set the invalid bit for sw cookie
 */
#define HAL_RX_BUF_COOKIE_INVALID_SET(buff_addr_info) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) |= \
		 HAL_RX_COOKIE_INVALID_MASK)

/*
 * macro to reset the invalid bit for sw cookie
 */
#define HAL_RX_BUF_COOKIE_INVALID_RESET(buff_addr_info) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) &= \
		~HAL_RX_COOKIE_INVALID_MASK)

#define HAL_RX_REO_BUF_COOKIE_INVALID_GET(reo_desc)	\
	(HAL_RX_BUF_COOKIE_INVALID_GET(&		\
	(((struct reo_destination_ring *)	\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_REO_BUF_COOKIE_INVALID_SET(reo_desc)	\
	(HAL_RX_BUF_COOKIE_INVALID_SET(&		\
	(((struct reo_destination_ring *)	\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_LINK_COOKIE_INVALID_MASK 0x40000000

#define HAL_RX_BUF_LINK_COOKIE_INVALID_GET(buff_addr_info) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) & \
		HAL_RX_LINK_COOKIE_INVALID_MASK)

#define HAL_RX_BUF_LINK_COOKIE_INVALID_SET(buff_addr_info) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) |= \
		HAL_RX_LINK_COOKIE_INVALID_MASK)

#define HAL_RX_REO_BUF_LINK_COOKIE_INVALID_GET(reo_desc)	\
		(HAL_RX_BUF_LINK_COOKIE_INVALID_GET(&		\
		(((struct reo_destination_ring *)	\
			reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_REO_BUF_LINK_COOKIE_INVALID_SET(reo_desc)	\
		(HAL_RX_BUF_LINK_COOKIE_INVALID_SET(&		\
		(((struct reo_destination_ring *)	\
			reo_desc)->buf_or_link_desc_addr_info)))
#endif

/* TODO: Convert the following structure fields accesseses to offsets */

#define HAL_RX_REO_BUFFER_ADDR_39_32_GET(reo_desc)	\
	(HAL_RX_BUFFER_ADDR_39_32_GET(&			\
	(((struct reo_destination_ring *)		\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_REO_BUFFER_ADDR_31_0_GET(reo_desc)	\
	(HAL_RX_BUFFER_ADDR_31_0_GET(&			\
	(((struct reo_destination_ring *)		\
		reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_REO_BUF_COOKIE_INVALID_RESET(reo_desc)	\
		(HAL_RX_BUF_COOKIE_INVALID_RESET(&		\
		(((struct reo_destination_ring *)	\
			reo_desc)->buf_or_link_desc_addr_info)))

#define HAL_RX_UNIFORM_HDR_SET(_rx_msdu_link, _field, _val)		\
	HAL_RX_FLD_SET(_rx_msdu_link, HAL_UNIFORM_DESCRIPTOR_HEADER,	\
			_field, _val)

#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET	0x0
#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB	0
#define HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK	0xffffffff

#define HAL_RX_BUFFER_ADDR_31_0_GET(buff_addr_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,	\
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET)),	\
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK,	\
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB))

#define HAL_RX_BUFFER_ADDR_39_32_GET(buff_addr_info)			\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,			\
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET)),	\
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MASK,		\
		HAL_BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_LSB))

#define HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_OFFSET	0x0
#define HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_LSB	0
#define HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MASK	0x00000001

#define HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_OFFSET	0x0
#define HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_LSB	1
#define HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MASK	0x00000002

#define HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_SET(msdu_info_ptr, val)		\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_OFFSET)) |= \
		(val << HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_LSB) & \
		HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_info_ptr, val)		\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_OFFSET)) |= \
		(val << HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_LSB) & \
		HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr)	\
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_OFFSET)) & \
		HAL_RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_LAST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr) \
	((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,			\
		HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_OFFSET)) & \
		HAL_RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MASK)

#define HAL_RX_MSDU_DESC_INFO_MSDU_LENGTH_OFFSET	0x0
#define HAL_RX_MSDU_DESC_INFO_MSDU_LENGTH_LSB		3
#define HAL_RX_MSDU_DESC_INFO_MSDU_LENGTH_MASK		0x0001fff8

#define HAL_RX_MSDU_PKT_LENGTH_GET(msdu_info_ptr)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(msdu_info_ptr,		\
		HAL_RX_MSDU_DESC_INFO_MSDU_LENGTH_OFFSET)),	\
		HAL_RX_MSDU_DESC_INFO_MSDU_LENGTH_MASK,		\
		HAL_RX_MSDU_DESC_INFO_MSDU_LENGTH_LSB))

static inline uint32_t
hal_rx_msdu_flags_get(hal_soc_handle_t hal_soc_hdl,
		      rx_msdu_desc_info_t msdu_desc_info_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_flags_get(msdu_desc_info_hdl);
}

/*
 * Structures & Macros to obtain fields from the TLV's in the Rx packet
 * pre-header.
 */

static inline uint8_t *hal_rx_desc_get_80211_hdr(hal_soc_handle_t hal_soc_hdl,
						 void *hw_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_desc_get_80211_hdr(hw_desc_addr);
}

/**
 * hal_rx_mpdu_desc_info_get() - Get MDPU desc info params
 * @hal_soc_hdl: hal soc handle
 * @desc_addr: ring descriptor
 * @mpdu_desc_info: Buffer to fill the mpdu desc info params
 *
 * Return: None
 */
static inline void
hal_rx_mpdu_desc_info_get(hal_soc_handle_t hal_soc_hdl, void *desc_addr,
			  struct hal_rx_mpdu_desc_info *mpdu_desc_info)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_desc_info_get(desc_addr,
						       mpdu_desc_info);
}

#define HAL_RX_NUM_MSDU_DESC 6
#define HAL_RX_MAX_SAVED_RING_DESC 16

/* TODO: rework the structure */
struct hal_rx_msdu_list {
	struct hal_rx_msdu_desc_info msdu_info[HAL_RX_NUM_MSDU_DESC];
	uint32_t sw_cookie[HAL_RX_NUM_MSDU_DESC];
	uint8_t rbm[HAL_RX_NUM_MSDU_DESC];
	/* physical address of the msdu */
	uint64_t paddr[HAL_RX_NUM_MSDU_DESC];
};

struct hal_buf_info {
	uint64_t paddr;
	uint32_t sw_cookie;
	uint8_t rbm;
};

/* This special cookie value will be used to indicate FW allocated buffers
 * received through RXDMA2SW ring for RXDMA WARs
 */
#define HAL_RX_COOKIE_SPECIAL 0x1fffff

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
 *                                 overflow
 * @ HAL_RXDMA_ERR_MPDU_LENGTH   : MPDU frame is not complete due to receiving
 *                                 incomplete
 *                                 MPDU from the PHY
 * @ HAL_RXDMA_ERR_FCS           : FCS check on the MPDU frame failed
 * @ HAL_RXDMA_ERR_DECRYPT       : Decryption error
 * @ HAL_RXDMA_ERR_TKIP_MIC      : TKIP MIC error
 * @ HAL_RXDMA_ERR_UNENCRYPTED   : Received a frame that was expected to be
 *                                 encrypted but wasn’t
 * @ HAL_RXDMA_ERR_MSDU_LEN      : MSDU related length error
 * @ HAL_RXDMA_ERR_MSDU_LIMIT    : Number of MSDUs in the MPDUs exceeded
 *                                 the max allowed
 * @ HAL_RXDMA_ERR_WIFI_PARSE    : wifi parsing error
 * @ HAL_RXDMA_ERR_AMSDU_PARSE   : Amsdu parsing error
 * @ HAL_RXDMA_ERR_SA_TIMEOUT    : Source Address search timeout
 * @ HAL_RXDMA_ERR_DA_TIMEOUT    : Destination Address  search timeout
 * @ HAL_RXDMA_ERR_FLOW_TIMEOUT  : Flow Search Timeout
 * @ HAL_RXDMA_ERR_FLUSH_REQUEST : RxDMA FIFO Flush request
 * @ HAL_RXDMA_AMSDU_FRAGMENT    : Rx PCU reported A-MSDU
 *                                 present as well as a fragmented MPDU
 * @ HAL_RXDMA_MULTICAST_ECHO    : RX OLE reported a multicast echo
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
	HAL_RXDMA_AMSDU_FRAGMENT,
	HAL_RXDMA_MULTICAST_ECHO,
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

#define HAL_WBM_RELEASE_RING_DESC_LEN_DWORDS (NUM_OF_DWORDS_WBM_RELEASE_RING)

//#include "hal_rx_be.h"

/*
 * hal_rx_msdu_is_wlan_mcast(): Check if the buffer is for multicast address
 *
 * @nbuf: Network buffer
 * Returns: flag to indicate whether the nbuf has MC/BC address
 */
static inline uint32_t
hal_rx_msdu_is_wlan_mcast(hal_soc_handle_t hal_soc_hdl,
			  qdf_nbuf_t nbuf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_is_wlan_mcast(nbuf);
}

/**
 * hal_rx_priv_info_set_in_tlv(): Save the private info to
 *				the reserved bytes of rx_tlv_hdr
 * @buf: start of rx_tlv_hdr
 * @wbm_er_info: hal_wbm_err_desc_info structure
 * Return: void
 */
static inline void
hal_rx_priv_info_set_in_tlv(hal_soc_handle_t hal_soc_hdl,
			    uint8_t *buf, uint8_t *priv_data,
			    uint32_t len)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_priv_info_set_in_tlv(buf,
							 priv_data,
							 len);
}

/*
 * hal_rx_reo_ent_rxdma_push_reason_get(): Retrieves RXDMA push reason from
 *	reo_entrance_ring descriptor
 *
 * @reo_ent_desc: reo_entrance_ring descriptor
 * Returns: value of rxdma_push_reason
 */
static inline
uint8_t hal_rx_reo_ent_rxdma_push_reason_get(hal_rxdma_desc_t reo_ent_desc)
{
	return _HAL_MS((*_OFFSET_TO_WORD_PTR(reo_ent_desc,
		HAL_REO_ENTRANCE_RING_RXDMA_PUSH_REASON_OFFSET)),
		HAL_REO_ENTRANCE_RING_RXDMA_PUSH_REASON_MASK,
		HAL_REO_ENTRANCE_RING_RXDMA_PUSH_REASON_LSB);
}

/**
 * hal_rx_reo_ent_rxdma_error_code_get(): Retrieves RXDMA error code from
 *	reo_entrance_ring descriptor
 * @reo_ent_desc: reo_entrance_ring descriptor
 * Return: value of rxdma_error_code
 */
static inline
uint8_t hal_rx_reo_ent_rxdma_error_code_get(hal_rxdma_desc_t reo_ent_desc)
{
	return _HAL_MS((*_OFFSET_TO_WORD_PTR(reo_ent_desc,
		HAL_REO_ENTRANCE_RING_RXDMA_ERROR_CODE_OFFSET)),
		HAL_REO_ENTRANCE_RING_RXDMA_ERROR_CODE_MASK,
		HAL_REO_ENTRANCE_RING_RXDMA_ERROR_CODE_LSB);
}

/**
 * hal_rx_priv_info_get_from_tlv(): retrieve the private data from
 *				the reserved bytes of rx_tlv_hdr.
 * @buf: start of rx_tlv_hdr
 * @wbm_er_info: hal_wbm_err_desc_info structure, output parameter.
 * Return: void
 */
static inline void
hal_rx_priv_info_get_from_tlv(hal_soc_handle_t hal_soc_hdl,
			      uint8_t *buf, uint8_t *wbm_er_info,
			      uint32_t len)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_priv_info_get_from_tlv(buf,
							   wbm_er_info,
							   len);
}

static inline void
hal_rx_get_tlv_size(hal_soc_handle_t hal_soc_hdl, uint16_t *rx_pkt_tlv_size,
		    uint16_t *rx_mon_pkt_tlv_size)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_tlv_size(rx_pkt_tlv_size,
						 rx_mon_pkt_tlv_size);
}

/*
 * hal_rx_encryption_info_valid(): Returns encryption type.
 *
 * @hal_soc_hdl: hal soc handle
 * @buf: rx_tlv_hdr of the received packet
 *
 * Return: encryption type
 */
static inline uint32_t
hal_rx_encryption_info_valid(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_encryption_info_valid(buf);
}

/*
 * hal_rx_print_pn: Prints the PN of rx packet.
 * @hal_soc_hdl: hal soc handle
 * @buf: rx_tlv_hdr of the received packet
 *
 * Return: void
 */
static inline void
hal_rx_print_pn(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_print_pn(buf);
}

/**
 * hal_rx_msdu_end_l3_hdr_padding_get(): API to get the
 * l3_header padding from rx_msdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: number of l3 header padding bytes
 */
static inline uint32_t
hal_rx_msdu_end_l3_hdr_padding_get(hal_soc_handle_t hal_soc_hdl,
				   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_l3_hdr_padding_get(buf);
}

/**
 * hal_rx_msdu_end_sa_idx_get(): API to get the
 * sa_idx from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_idx (SA AST index)
 */
static inline uint16_t
hal_rx_msdu_end_sa_idx_get(hal_soc_handle_t hal_soc_hdl,
			   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_sa_idx_get(buf);
}

 /**
 * hal_rx_msdu_end_sa_is_valid_get(): API to get the
 * sa_is_valid bit from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_is_valid bit
 */
static inline uint8_t
hal_rx_msdu_end_sa_is_valid_get(hal_soc_handle_t hal_soc_hdl,
				uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_sa_is_valid_get(buf);
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
hal_rx_tlv_msdu_len_set(hal_soc_handle_t hal_soc_hdl, uint8_t *buf,
			uint32_t len)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_msdu_len_set(buf, len);
}

/**
 * enum hal_rx_mpdu_info_sw_frame_group_id_type: Enum for group id in MPDU_INFO
 *
 * @ HAL_MPDU_SW_FRAME_GROUP_NDP_FRAME: NDP frame
 * @ HAL_MPDU_SW_FRAME_GROUP_MULTICAST_DATA: multicast data frame
 * @ HAL_MPDU_SW_FRAME_GROUP_UNICAST_DATA: unicast data frame
 * @ HAL_MPDU_SW_FRAME_GROUP_NULL_DATA: NULL data frame
 * @ HAL_MPDU_SW_FRAME_GROUP_MGMT: management frame
 * @ HAL_MPDU_SW_FRAME_GROUP_MGMT_PROBE_REQ: probe req frame
 * @ HAL_MPDU_SW_FRAME_GROUP_CTRL: control frame
 * @ HAL_MPDU_SW_FRAME_GROUP_CTRL_NDPA: NDPA frame
 * @ HAL_MPDU_SW_FRAME_GROUP_CTRL_BAR: BAR frame
 * @ HAL_MPDU_SW_FRAME_GROUP_CTRL_RTS: RTS frame
 * @ HAL_MPDU_SW_FRAME_GROUP_UNSUPPORTED: unsupported
 * @ HAL_MPDU_SW_FRAME_GROUP_MAX: max limit
 */
enum hal_rx_mpdu_info_sw_frame_group_id_type {
	HAL_MPDU_SW_FRAME_GROUP_NDP_FRAME = 0,
	HAL_MPDU_SW_FRAME_GROUP_MULTICAST_DATA,
	HAL_MPDU_SW_FRAME_GROUP_UNICAST_DATA,
	HAL_MPDU_SW_FRAME_GROUP_NULL_DATA,
	HAL_MPDU_SW_FRAME_GROUP_MGMT,
	HAL_MPDU_SW_FRAME_GROUP_MGMT_PROBE_REQ = 8,
	HAL_MPDU_SW_FRAME_GROUP_MGMT_BEACON = 12,
	HAL_MPDU_SW_FRAME_GROUP_CTRL = 20,
	HAL_MPDU_SW_FRAME_GROUP_CTRL_NDPA = 25,
	HAL_MPDU_SW_FRAME_GROUP_CTRL_BAR = 28,
	HAL_MPDU_SW_FRAME_GROUP_CTRL_RTS = 31,
	HAL_MPDU_SW_FRAME_GROUP_UNSUPPORTED = 36,
	HAL_MPDU_SW_FRAME_GROUP_MAX = 37,
};

/**
 * hal_rx_mpdu_start_mpdu_qos_control_valid_get():
 * Retrieve qos control valid bit from the tlv.
 * @hal_soc_hdl: hal_soc handle
 * @buf: pointer to rx pkt TLV.
 *
 * Return: qos control value.
 */
static inline uint32_t
hal_rx_mpdu_start_mpdu_qos_control_valid_get(
				hal_soc_handle_t hal_soc_hdl,
				uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if ((!hal_soc) || (!hal_soc->ops)) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}

	if (hal_soc->ops->hal_rx_mpdu_start_mpdu_qos_control_valid_get)
		return hal_soc->ops->
		       hal_rx_mpdu_start_mpdu_qos_control_valid_get(buf);

	return QDF_STATUS_E_INVAL;
}

/**
 * hal_rx_is_unicast: check packet is unicast frame or not.
 * @hal_soc_hdl: hal_soc handle
 * @buf: pointer to rx pkt TLV.
 *
 * Return: true on unicast.
 */
static inline bool
hal_rx_is_unicast(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_is_unicast(buf);
}

/**
 * hal_rx_tid_get: get tid based on qos control valid.
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to rx pkt TLV.
 *
 * Return: tid
 */
static inline uint32_t
hal_rx_tid_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tid_get(hal_soc_hdl, buf);
}

/**
 * hal_rx_mpdu_start_sw_peer_id_get() - Retrieve sw peer id
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to rx pkt TLV.
 *
 * Return: sw peer_id
 */
static inline uint32_t
hal_rx_mpdu_start_sw_peer_id_get(hal_soc_handle_t hal_soc_hdl,
				 uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_start_sw_peer_id_get(buf);
}

/**
 * hal_rx_mpdu_peer_meta_data_get() - Retrieve PEER_META_DATA
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to rx pkt TLV.
 *
 * Return: peer meta data
 */
static inline uint32_t
hal_rx_mpdu_peer_meta_data_get(hal_soc_handle_t hal_soc_hdl,
			       uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_peer_meta_data_get(buf);
}

/*
 * hal_rx_mpdu_get_tods(): API to get the tods info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(to_ds)
 */

static inline uint32_t
hal_rx_mpdu_get_to_ds(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_get_to_ds(buf);
}


/*
 * hal_rx_mpdu_get_fr_ds(): API to get the from ds info
 * from rx_mpdu_start
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to the start of RX PKT TLV header
 *
 * Return: uint32_t(fr_ds)
 */
static inline uint32_t
hal_rx_mpdu_get_fr_ds(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_get_fr_ds(buf);
}

/*
 * hal_rx_mpdu_get_addr1(): API to check get address1 of the mpdu
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to the start of RX PKT TLV headera
 * @mac_addr: pointer to mac address
 *
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr1(hal_soc_handle_t hal_soc_hdl,
				 uint8_t *buf, uint8_t *mac_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_get_addr1(buf, mac_addr);
}

/*
 * hal_rx_mpdu_get_addr2(): API to check get address2 of the mpdu
 * in the packet
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 *
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr2(hal_soc_handle_t hal_soc_hdl,
				 uint8_t *buf, uint8_t *mac_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_get_addr2(buf, mac_addr);
}

/*
 * hal_rx_mpdu_get_addr3(): API to get address3 of the mpdu
 * in the packet
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 *
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr3(hal_soc_handle_t hal_soc_hdl,
				 uint8_t *buf, uint8_t *mac_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_get_addr3(buf, mac_addr);
}

/*
 * hal_rx_mpdu_get_addr4(): API to get address4 of the mpdu
 * in the packet
 * @hal_soc_hdl: hal_soc handle
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline
QDF_STATUS hal_rx_mpdu_get_addr4(hal_soc_handle_t hal_soc_hdl,
				 uint8_t *buf, uint8_t *mac_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_get_addr4(buf, mac_addr);
}

 /**
 * hal_rx_msdu_end_da_idx_get: API to get da_idx
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da index
 */
static inline uint16_t
hal_rx_msdu_end_da_idx_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_da_idx_get(buf);
}

/**
 * hal_rx_msdu_end_da_is_valid_get: API to check if da is valid
 * from rx_msdu_end TLV
 * @hal_soc_hdl: hal soc handle
 * @ buf: pointer to the start of RX PKT TLV headers
 *
 * Return: da_is_valid
 */
static inline uint8_t
hal_rx_msdu_end_da_is_valid_get(hal_soc_handle_t hal_soc_hdl,
				uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_da_is_valid_get(buf);
}

/**
 * hal_rx_msdu_end_da_is_mcbc_get: API to check if pkt is MCBC
 * from rx_msdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: da_is_mcbc
 */
static inline uint8_t
hal_rx_msdu_end_da_is_mcbc_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_da_is_mcbc_get(buf);
}

/**
 * hal_rx_msdu_end_first_msdu_get: API to get first msdu status
 * from rx_msdu_end TLV
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: first_msdu
 */
static inline uint8_t
hal_rx_msdu_end_first_msdu_get(hal_soc_handle_t hal_soc_hdl,
			       uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_first_msdu_get(buf);
}

/**
 * hal_rx_msdu_end_last_msdu_get: API to get last msdu status
 * from rx_msdu_end TLV
 * @hal_soc_hdl: hal soc handle
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: last_msdu
 */
static inline uint8_t
hal_rx_msdu_end_last_msdu_get(hal_soc_handle_t hal_soc_hdl,
			      uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_last_msdu_get(buf);
}

/**
 * hal_rx_msdu_cce_match_get: API to get CCE match
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: cce_meta_data
 */
static inline bool
hal_rx_msdu_cce_match_get(hal_soc_handle_t hal_soc_hdl,
			  uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_cce_match_get(buf);
}

/**
 * hal_rx_msdu_cce_metadata_get: API to get CCE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: cce_meta_data
 */
static inline uint16_t
hal_rx_msdu_cce_metadata_get(hal_soc_handle_t hal_soc_hdl,
			     uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_cce_metadata_get(buf);
}

/*******************************************************************************
 * RX REO ERROR APIS
 ******************************************************************************/

/**
 * hal_rx_link_desc_msdu0_ptr - Get pointer to rx_msdu details
 * @msdu_link_ptr - msdu link ptr
 * @hal - pointer to hal_soc
 * Return - Pointer to rx_msdu_details structure
 *
 */
static inline
void *hal_rx_link_desc_msdu0_ptr(void *msdu_link_ptr,
				 struct hal_soc *hal_soc)
{
	return hal_soc->ops->hal_rx_link_desc_msdu0_ptr(msdu_link_ptr);
}

/**
 * hal_rx_msdu_desc_info_get_ptr() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 * @hal - pointer to hal_soc
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static inline
void *hal_rx_msdu_desc_info_get_ptr(void *msdu_details_ptr,
				    struct hal_soc *hal_soc)
{
	return hal_soc->ops->hal_rx_msdu_desc_info_get_ptr(msdu_details_ptr);
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
static inline
void hal_rx_reo_buf_paddr_get(hal_soc_handle_t hal_soc_hdl,
			      hal_ring_desc_t rx_desc,
			      struct hal_buf_info *buf_info)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_reo_buf_paddr_get)
		return hal_soc->ops->hal_rx_reo_buf_paddr_get(
					rx_desc,
					buf_info);

}

/**
 * hal_rx_buf_cookie_rbm_get: Gets the physical address and
 * cookie from the REO entrance ring element
 *
 * @ hal_rx_desc_cookie: Opaque cookie pointer used by HAL to get to
 * the current descriptor
 * @ buf_info: structure to return the buffer information
 * @ msdu_cnt: pointer to msdu count in MPDU
 * Return: void
 */
static inline
void hal_rx_buf_cookie_rbm_get(hal_soc_handle_t hal_soc_hdl,
			       uint32_t *buf_addr_info,
			       struct hal_buf_info *buf_info)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_buf_cookie_rbm_get(
					buf_addr_info,
					buf_info);
}

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
static inline void hal_rx_msdu_list_get(hal_soc_handle_t hal_soc_hdl,
					void *msdu_link_desc,
					struct hal_rx_msdu_list *msdu_list,
					uint16_t *num_msdus)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct rx_msdu_details *msdu_details;
	struct rx_msdu_desc_info *msdu_desc_info;
	struct rx_msdu_link *msdu_link = (struct rx_msdu_link *)msdu_link_desc;
	int i;
	struct hal_buf_info buf_info;

	msdu_details = hal_rx_link_desc_msdu0_ptr(msdu_link, hal_soc);

	dp_nofl_debug("[%s][%d] msdu_link=%pK msdu_details=%pK",
		      __func__, __LINE__, msdu_link, msdu_details);

	for (i = 0; i < HAL_RX_NUM_MSDU_DESC; i++) {
		/* num_msdus received in mpdu descriptor may be incorrect
		 * sometimes due to HW issue. Check msdu buffer address also
		 */
		if (!i && (HAL_RX_BUFFER_ADDR_31_0_GET(
			&msdu_details[i].buffer_addr_info_details) == 0))
			break;
		if (HAL_RX_BUFFER_ADDR_31_0_GET(
			&msdu_details[i].buffer_addr_info_details) == 0) {
			/* set the last msdu bit in the prev msdu_desc_info */
			msdu_desc_info =
				hal_rx_msdu_desc_info_get_ptr(&msdu_details[i - 1], hal_soc);
			HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);
			break;
		}
		msdu_desc_info = hal_rx_msdu_desc_info_get_ptr(&msdu_details[i],
							       hal_soc);

		/* set first MSDU bit or the last MSDU bit */
		if (!i)
			HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);
		else if (i == (HAL_RX_NUM_MSDU_DESC - 1))
			HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);

		msdu_list->msdu_info[i].msdu_flags =
			 hal_rx_msdu_flags_get(hal_soc_hdl, msdu_desc_info);
		msdu_list->msdu_info[i].msdu_len =
			 HAL_RX_MSDU_PKT_LENGTH_GET(msdu_desc_info);

		/* addr field in buf_info will not be valid */
		hal_rx_buf_cookie_rbm_get(
			hal_soc_hdl,
			(uint32_t *)&msdu_details[i].buffer_addr_info_details,
			&buf_info);
		msdu_list->sw_cookie[i] = buf_info.sw_cookie;
		msdu_list->rbm[i] = buf_info.rbm;
		msdu_list->paddr[i] = HAL_RX_BUFFER_ADDR_31_0_GET(
			   &msdu_details[i].buffer_addr_info_details) |
			   (uint64_t)HAL_RX_BUFFER_ADDR_39_32_GET(
			   &msdu_details[i].buffer_addr_info_details) << 32;
		dp_nofl_debug("[%s][%d] i=%d sw_cookie=%d",
			      __func__, __LINE__, i, msdu_list->sw_cookie[i]);
	}
	*num_msdus = i;
}

/**
 * hal_rx_is_pn_error() - Indicate if this error was caused by a
 * PN check failure
 *
 * @reo_desc: opaque pointer used by HAL to get the REO destination entry
 *
 * Return: true: error caused by PN check, false: other error
 */
static inline bool hal_rx_reo_is_pn_error(uint32_t error_code)
{

	return ((error_code == HAL_REO_ERR_PN_CHECK_FAILED) ||
		(error_code == HAL_REO_ERR_PN_ERROR_HANDLING_FLAG_SET)) ?
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
static inline bool hal_rx_reo_is_2k_jump(uint32_t error_code)
{
	return ((error_code == HAL_REO_ERR_REGULAR_FRAME_2K_JUMP) ||
		(error_code == HAL_REO_ERR_2K_ERROR_HANDLING_FLAG_SET)) ?
			true : false;
}

/**
 * hal_rx_reo_is_oor_error() - Indicate if this error was caused by OOR
 *
 * @ring_desc: opaque pointer used by HAL to get the REO destination entry
 *
 * Return: true: error caused by OOR, false: other error
 */
static inline bool hal_rx_reo_is_oor_error(uint32_t error_code)
{
	return (error_code == HAL_REO_ERR_REGULAR_FRAME_OOR) ?
			true : false;
}

/**
 * hal_rx_reo_is_bar_oor_2k_jump() - Check if the error is 2k-jump or OOR error
 * @error_code: error code obtained from ring descriptor.
 *
 * Return: true, if the error code is 2k-jump or OOR
 *	false, for other error codes.
 */
static inline bool hal_rx_reo_is_bar_oor_2k_jump(uint32_t error_code)
{
	return ((error_code == HAL_REO_ERR_BAR_FRAME_2K_JUMP) ||
		(error_code == HAL_REO_ERR_BAR_FRAME_OOR)) ?
		true : false;
}

/**
 * hal_dump_wbm_rel_desc() - dump wbm release descriptor
 * @hal_desc: hardware descriptor pointer
 *
 * This function will print wbm release descriptor
 *
 * Return: none
 */
static inline void hal_dump_wbm_rel_desc(void *src_srng_desc)
{
	uint32_t *wbm_comp = (uint32_t *)src_srng_desc;
	uint32_t i;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
		  "Current Rx wbm release descriptor is");

	for (i = 0; i < HAL_WBM_RELEASE_RING_DESC_LEN_DWORDS; i++) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
			  "DWORD[i] = 0x%x", wbm_comp[i]);
	}
}

/**
 * hal_rx_msdu_link_desc_set: Retrieves MSDU Link Descriptor to WBM
 *
 * @ hal_soc_hdl	: HAL version of the SOC pointer
 * @ src_srng_desc	: void pointer to the WBM Release Ring descriptor
 * @ buf_addr_info	: void pointer to the buffer_addr_info
 * @ bm_action		: put in IDLE list or release to MSDU_LIST
 *
 * Return: void
 */
/* look at implementation at dp_hw_link_desc_pool_setup()*/
static inline
void hal_rx_msdu_link_desc_set(hal_soc_handle_t hal_soc_hdl,
			       void *src_srng_desc,
			       hal_buff_addrinfo_t buf_addr_info,
			       uint8_t bm_action)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_msdu_link_desc_set)
		return hal_soc->ops->hal_rx_msdu_link_desc_set(hal_soc_hdl,
					src_srng_desc,
					buf_addr_info,
					bm_action);
}

/**
 * HAL_RX_BUF_ADDR_INFO_GET: Returns the address of the
 *			     BUFFER_ADDR_INFO, give the RX descriptor
 *			     (Assumption -- BUFFER_ADDR_INFO is the
 *			     first field in the descriptor structure)
 */
#define HAL_RX_BUF_ADDR_INFO_GET(ring_desc)	\
			((hal_link_desc_t)(ring_desc))

#define HAL_RX_REO_BUF_ADDR_INFO_GET HAL_RX_BUF_ADDR_INFO_GET

#define HAL_RX_WBM_BUF_ADDR_INFO_GET HAL_RX_BUF_ADDR_INFO_GET

/*******************************************************************************
 * RX WBM ERROR APIS
 ******************************************************************************/

#define HAL_RX_WBM_BUF_TYPE_GET(wbm_desc) (((*(((uint32_t *) wbm_desc)+ \
		(WBM_ERR_RING_BUFFER_OR_DESC_TYPE_OFFSET >> 2))) & \
		WBM_ERR_RING_BUFFER_OR_DESC_TYPE_MASK) >> \
		WBM_ERR_RING_BUFFER_OR_DESC_TYPE_LSB)
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
	HAL_RX_WBM_RXDMA_PSH_RSN_FLUSH,
};

static inline void hal_rx_dump_mpdu_start_tlv(struct rx_mpdu_start *mpdu_start,
					      uint8_t dbg_level,
					      struct hal_soc *hal)
{

	hal->ops->hal_rx_dump_mpdu_start_tlv(mpdu_start, dbg_level);
}

/**
 * hal_rx_dump_msdu_end_tlv: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_msdu_end_tlv(struct hal_soc *hal_soc,
					    struct rx_msdu_end *msdu_end,
					    uint8_t dbg_level)
{
	hal_soc->ops->hal_rx_dump_msdu_end_tlv(msdu_end, dbg_level);
}

/**
 * hal_srng_ring_id_get: API to retrieve ring id from hal ring
 *                       structure
 * @hal_ring: pointer to hal_srng structure
 *
 * Return: ring_id
 */
static inline uint8_t hal_srng_ring_id_get(hal_ring_handle_t hal_ring_hdl)
{
	return ((struct hal_srng *)hal_ring_hdl)->ring_id;
}

#define DOT11_SEQ_FRAG_MASK		0x000f
#define DOT11_FC1_MORE_FRAG_OFFSET	0x04

/**
 * hal_rx_get_rx_fragment_number(): Function to retrieve rx fragment number
 *
 * @nbuf: Network buffer
 * Returns: rx fragment number
 */
static inline
uint8_t hal_rx_get_rx_fragment_number(struct hal_soc *hal_soc,
				      uint8_t *buf)
{
	return hal_soc->ops->hal_rx_get_rx_fragment_number(buf);
}

/*
 * hal_rx_get_mpdu_sequence_control_valid(): Get mpdu sequence control valid
 * @hal_soc_hdl: hal soc handle
 * @nbuf: Network buffer
 *
 * Return: value of sequence control valid field
 */
static inline
uint8_t hal_rx_get_mpdu_sequence_control_valid(hal_soc_handle_t hal_soc_hdl,
					       uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_mpdu_sequence_control_valid(buf);
}

/*
 * hal_rx_get_mpdu_frame_control_valid(): Retrieves mpdu frame control valid
 * @hal_soc_hdl: hal soc handle
 * @nbuf: Network buffer
 *
 * Returns: value of frame control valid field
 */
static inline
uint8_t hal_rx_get_mpdu_frame_control_valid(hal_soc_handle_t hal_soc_hdl,
					    uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_mpdu_frame_control_valid(buf);
}

/**
 * hal_rx_get_mpdu_mac_ad4_valid(): Retrieves if mpdu 4th addr is valid
 * @hal_soc_hdl: hal soc handle
 * @nbuf: Network buffer
 * Returns: value of mpdu 4th address valid field
 */
static inline
bool hal_rx_get_mpdu_mac_ad4_valid(hal_soc_handle_t hal_soc_hdl,
				   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_mpdu_mac_ad4_valid(buf);
}

/*
 * hal_rx_clear_mpdu_desc_info(): Clears mpdu_desc_info
 *
 * @rx_mpdu_desc_info: HAL view of rx mpdu desc info
 * Returns: None
 */
static inline void
hal_rx_clear_mpdu_desc_info(struct hal_rx_mpdu_desc_info *rx_mpdu_desc_info)
{
	qdf_mem_zero(rx_mpdu_desc_info, sizeof(*rx_mpdu_desc_info));
}

/**
 * hal_rx_wbm_err_info_get(): Retrieves WBM error code and reason and
 *	save it to hal_wbm_err_desc_info structure passed by caller
 * @wbm_desc: wbm ring descriptor
 * @wbm_er_info: hal_wbm_err_desc_info structure, output parameter.
 * Return: void
 */
static inline
void hal_rx_wbm_err_info_get(void *wbm_desc,
			     struct hal_wbm_err_desc_info *wbm_er_info,
			     hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_wbm_err_info_get(wbm_desc, (void *)wbm_er_info);
}

/**
 * hal_rx_wbm_err_msdu_continuation_get(): Get wbm msdu continuation
 * bit from wbm release ring descriptor
 * @wbm_desc: wbm ring descriptor
 * Return: uint8_t
 */
static inline
uint8_t hal_rx_wbm_err_msdu_continuation_get(hal_soc_handle_t hal_soc_hdl,
					     void *wbm_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_wbm_err_msdu_continuation_get(wbm_desc);
}

/**
 * hal_rx_mon_hw_desc_get_mpdu_status: Retrieve MPDU status
 *
 * @ hal_soc: HAL version of the SOC pointer
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
static inline
void hal_rx_mon_hw_desc_get_mpdu_status(hal_soc_handle_t hal_soc_hdl,
					void *hw_desc_addr,
					struct mon_rx_status *rs)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_mon_hw_desc_get_mpdu_status(hw_desc_addr, rs);
}

/*
 * hal_rx_get_tlv(): API to get the tlv
 *
 * @hal_soc: HAL version of the SOC pointer
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
static inline uint8_t hal_rx_get_tlv(struct hal_soc *hal_soc, void *rx_tlv)
{
	return hal_soc->ops->hal_rx_get_tlv(rx_tlv);
}

/*
 * hal_rx_msdu_start_nss_get(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @hal_soc: HAL version of the SOC pointer
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */
static inline
uint32_t hal_rx_msdu_start_nss_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_start_nss_get(buf);
}

/**
 * hal_rx_dump_msdu_start_tlv: dump RX msdu_start TLV in structured
 *			       human readable format.
 * @ msdu_start: pointer the msdu_start TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_msdu_start_tlv(struct hal_soc *hal_soc,
					      struct rx_msdu_start *msdu_start,
					      uint8_t dbg_level)
{
	hal_soc->ops->hal_rx_dump_msdu_start_tlv(msdu_start, dbg_level);
}

/**
 * hal_rx_mpdu_start_tid_get - Return tid info from the rx mpdu start
 * info details
 *
 * @ buf - Pointer to buffer containing rx pkt tlvs.
 *
 *
 */
static inline uint32_t hal_rx_mpdu_start_tid_get(hal_soc_handle_t hal_soc_hdl,
						 uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_start_tid_get(buf);
}

/*
 * hal_rx_msdu_start_reception_type_get(): API to get the reception type
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(reception_type)
 */
static inline
uint32_t hal_rx_msdu_start_reception_type_get(hal_soc_handle_t hal_soc_hdl,
					      uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_start_reception_type_get(buf);
}

/**
 * hal_reo_status_get_header_generic - Process reo desc info
 * @d - Pointer to reo descriptior
 * @b - tlv type info
 * @h - Pointer to hal_reo_status_header where info to be stored
 * @hal- pointer to hal_soc structure
 * Return - none.
 *
 */
static inline
void hal_reo_status_get_header(hal_ring_desc_t ring_desc, int b,
			       void *h, struct hal_soc *hal_soc)
{
	hal_soc->ops->hal_reo_status_get_header(ring_desc, b, h);
}

/**
 * hal_rx_desc_is_first_msdu() - Check if first msdu
 *
 * @hal_soc_hdl: hal_soc handle
 * @hw_desc_addr: hardware descriptor address
 *
 * Return: 0 - success/ non-zero failure
 */
static inline
uint32_t hal_rx_desc_is_first_msdu(hal_soc_handle_t hal_soc_hdl,
				   void *hw_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_desc_is_first_msdu(hw_desc_addr);
}

/**
 * hal_rx_tlv_populate_mpdu_desc_info() - Populate mpdu_desc_info fields from
 *					the rx tlv fields.
 * @hal_soc_hdl: HAL SoC handle
 * @buf: rx tlv start address [To be validated by caller]
 * @mpdu_desc_info_hdl: Buffer where the mpdu_desc_info is to be populated.
 *
 * Return: None
 */
static inline void
hal_rx_tlv_populate_mpdu_desc_info(hal_soc_handle_t hal_soc_hdl,
				   uint8_t *buf,
				   void *mpdu_desc_info_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_tlv_populate_mpdu_desc_info)
		return hal_soc->ops->hal_rx_tlv_populate_mpdu_desc_info(buf,
							mpdu_desc_info_hdl);
}

static inline uint32_t
hal_rx_tlv_decap_format_get(hal_soc_handle_t hal_soc_hdl, void *hw_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_decap_format_get(hw_desc_addr);
}

static inline
bool HAL_IS_DECAP_FORMAT_RAW(hal_soc_handle_t hal_soc_hdl,
			     uint8_t *rx_tlv_hdr)
{
	uint8_t decap_format;

	if (hal_rx_desc_is_first_msdu(hal_soc_hdl, rx_tlv_hdr)) {
		decap_format = hal_rx_tlv_decap_format_get(hal_soc_hdl,
							   rx_tlv_hdr);
		if (decap_format == HAL_HW_RX_DECAP_FORMAT_RAW)
			return true;
	}

	return false;
}

/**
 * hal_rx_msdu_fse_metadata_get: API to get FSE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: fse metadata value from MSDU END TLV
 */
static inline uint32_t
hal_rx_msdu_fse_metadata_get(hal_soc_handle_t hal_soc_hdl,
			     uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_fse_metadata_get(buf);
}

/**
 * hal_rx_buffer_addr_info_get_paddr(): get paddr/sw_cookie from
 *					<struct buffer_addr_info> structure
 * @buf_addr_info: pointer to <struct buffer_addr_info> structure
 * @buf_info: structure to return the buffer information including
 *		paddr/cookie
 *
 * return: None
 */
static inline
void hal_rx_buffer_addr_info_get_paddr(void *buf_addr_info,
				       struct hal_buf_info *buf_info)
{
	buf_info->paddr =
	 (HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) |
	  ((uint64_t)(HAL_RX_BUFFER_ADDR_39_32_GET(buf_addr_info)) << 32));
}

/**
 * hal_rx_msdu_flow_idx_get: API to get flow index
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index value from MSDU END TLV
 */
static inline uint32_t
hal_rx_msdu_flow_idx_get(hal_soc_handle_t hal_soc_hdl,
			 uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_flow_idx_get(buf);
}

/**
 * hal_rx_msdu_get_reo_destination_indication: API to get reo
 * destination index from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * @reo_destination_indication: pointer to return value of
 * reo_destination_indication
 *
 * Return: reo_destination_indication value from MSDU END TLV
 */
static inline void
hal_rx_msdu_get_reo_destination_indication(hal_soc_handle_t hal_soc_hdl,
					   uint8_t *buf,
					   uint32_t *reo_destination_indication)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_msdu_get_reo_destination_indication(buf,
						reo_destination_indication);
}

/**
 * hal_rx_msdu_flow_idx_timeout: API to get flow index timeout
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index timeout value from MSDU END TLV
 */
static inline bool
hal_rx_msdu_flow_idx_timeout(hal_soc_handle_t hal_soc_hdl,
			     uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_flow_idx_timeout(buf);
}

/**
 * hal_rx_msdu_flow_idx_invalid: API to get flow index invalid
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index invalid value from MSDU END TLV
 */
static inline bool
hal_rx_msdu_flow_idx_invalid(hal_soc_handle_t hal_soc_hdl,
			     uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_flow_idx_invalid(buf);
}

/**
 * hal_rx_hw_desc_get_ppduid_get() - Retrieve ppdu id
 * @hal_soc_hdl: hal_soc handle
 * @rx_tlv_hdr: Rx_tlv_hdr
 * @rxdma_dst_ring_desc: Rx HW descriptor
 *
 * Return: ppdu id
 */
static inline
uint32_t hal_rx_hw_desc_get_ppduid_get(hal_soc_handle_t hal_soc_hdl,
				       void *rx_tlv_hdr,
				       void *rxdma_dst_ring_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_hw_desc_get_ppduid_get(rx_tlv_hdr,
							   rxdma_dst_ring_desc);
}

/**
 * hal_rx_msdu_end_sa_sw_peer_id_get() - get sw peer id
 * @hal_soc_hdl: hal_soc handle
 * @buf: rx tlv address
 *
 * Return: sw peer id
 */
static inline
uint32_t hal_rx_msdu_end_sa_sw_peer_id_get(hal_soc_handle_t hal_soc_hdl,
					   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_end_sa_sw_peer_id_get(buf);
}

static inline
void *hal_rx_msdu0_buffer_addr_lsb(hal_soc_handle_t hal_soc_hdl,
				   void *link_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu0_buffer_addr_lsb(link_desc_addr);
}

static inline
void *hal_rx_msdu_desc_info_ptr_get(hal_soc_handle_t hal_soc_hdl,
				    void *msdu_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_desc_info_ptr_get(msdu_addr);
}

static inline
void *hal_ent_mpdu_desc_info(hal_soc_handle_t hal_soc_hdl,
			     void *hw_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_ent_mpdu_desc_info(hw_addr);
}

static inline
void *hal_dst_mpdu_desc_info(hal_soc_handle_t hal_soc_hdl,
			     void *hw_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_dst_mpdu_desc_info(hw_addr);
}

static inline
uint8_t hal_rx_get_fc_valid(hal_soc_handle_t hal_soc_hdl,
			    uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_fc_valid(buf);
}

static inline
uint8_t hal_rx_get_to_ds_flag(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_to_ds_flag(buf);
}

static inline
uint8_t hal_rx_get_mac_addr2_valid(hal_soc_handle_t hal_soc_hdl,
				   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_mac_addr2_valid(buf);
}

static inline
uint8_t hal_rx_get_filter_category(hal_soc_handle_t hal_soc_hdl,
				   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_filter_category(buf);
}

static inline
uint32_t hal_rx_get_ppdu_id(hal_soc_handle_t hal_soc_hdl,
			    uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_ppdu_id(buf);
}

/**
 * hal_reo_config(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static inline
void hal_reo_config(struct hal_soc *hal_soc,
		    uint32_t reg_val,
		    struct hal_reo_params *reo_params)
{
	hal_soc->ops->hal_reo_config(hal_soc,
				     reg_val,
				     reo_params);
}

/**
 * hal_rx_msdu_get_flow_params: API to get flow index,
 * flow index invalid and flow index timeout from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * @flow_invalid: pointer to return value of flow_idx_valid
 * @flow_timeout: pointer to return value of flow_idx_timeout
 * @flow_index: pointer to return value of flow_idx
 *
 * Return: none
 */
static inline void
hal_rx_msdu_get_flow_params(hal_soc_handle_t hal_soc_hdl,
			    uint8_t *buf,
			    bool *flow_invalid,
			    bool *flow_timeout,
			    uint32_t *flow_index)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_msdu_get_flow_params(buf,
						  flow_invalid,
						  flow_timeout,
						  flow_index);
}

static inline
uint16_t hal_rx_tlv_get_tcp_chksum(hal_soc_handle_t hal_soc_hdl,
				   uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_get_tcp_chksum(buf);
}

static inline
uint16_t hal_rx_get_rx_sequence(hal_soc_handle_t hal_soc_hdl,
				uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_rx_sequence(buf);
}

static inline void
hal_rx_get_bb_info(hal_soc_handle_t hal_soc_hdl,
		   void *rx_tlv,
		   void *ppdu_info)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_get_bb_info)
		hal_soc->ops->hal_rx_get_bb_info(rx_tlv, ppdu_info);
}

static inline void
hal_rx_get_rtt_info(hal_soc_handle_t hal_soc_hdl,
		    void *rx_tlv,
		    void *ppdu_info)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_get_rtt_info)
		hal_soc->ops->hal_rx_get_rtt_info(rx_tlv, ppdu_info);
}

/**
 * hal_rx_msdu_metadata_get(): API to get the
 * fast path information from rx_msdu_end TLV
 *
 * @ hal_soc_hdl: DP soc handle
 * @ buf: pointer to the start of RX PKT TLV headers
 * @ msdu_metadata: Structure to hold msdu end information
 * Return: none
 */
static inline void
hal_rx_msdu_metadata_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf,
			 struct hal_rx_msdu_metadata *msdu_md)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_msdu_packet_metadata_get(buf, msdu_md);
}

/**
 * hal_rx_get_fisa_cumulative_l4_checksum: API to get cumulative_l4_checksum
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: cumulative_l4_checksum
 */
static inline uint16_t
hal_rx_get_fisa_cumulative_l4_checksum(hal_soc_handle_t hal_soc_hdl,
				       uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (!hal_soc->ops->hal_rx_get_fisa_cumulative_l4_checksum)
		return 0;

	return hal_soc->ops->hal_rx_get_fisa_cumulative_l4_checksum(buf);
}

/**
 * hal_rx_get_fisa_cumulative_ip_length: API to get cumulative_ip_length
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: cumulative_ip_length
 */
static inline uint16_t
hal_rx_get_fisa_cumulative_ip_length(hal_soc_handle_t hal_soc_hdl,
				     uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_get_fisa_cumulative_ip_length)
		return hal_soc->ops->hal_rx_get_fisa_cumulative_ip_length(buf);

	return 0;
}

/**
 * hal_rx_get_udp_proto: API to get UDP proto field
 * from rx_msdu_start TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: UDP proto field value
 */
static inline bool
hal_rx_get_udp_proto(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_get_udp_proto)
		return hal_soc->ops->hal_rx_get_udp_proto(buf);

	return 0;
}

/**
 * hal_rx_get_fisa_flow_agg_continuation: API to get fisa flow_agg_continuation
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow_agg_continuation bit field value
 */
static inline bool
hal_rx_get_fisa_flow_agg_continuation(hal_soc_handle_t hal_soc_hdl,
				      uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_get_fisa_flow_agg_continuation)
		return hal_soc->ops->hal_rx_get_fisa_flow_agg_continuation(buf);

	return 0;
}

/**
 * hal_rx_get_fisa_flow_agg_count: API to get fisa flow_agg count from
 * rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow_agg count value
 */
static inline uint8_t
hal_rx_get_fisa_flow_agg_count(hal_soc_handle_t hal_soc_hdl,
			       uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_get_fisa_flow_agg_count)
		return hal_soc->ops->hal_rx_get_fisa_flow_agg_count(buf);

	return 0;
}

/**
 * hal_rx_get_fisa_timeout: API to get fisa time out from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: fisa flow_agg timeout bit value
 */
static inline bool
hal_rx_get_fisa_timeout(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_get_fisa_timeout)
		return hal_soc->ops->hal_rx_get_fisa_timeout(buf);

	return 0;
}

/**
 * hal_rx_mpdu_start_tlv_tag_valid - API to check if RX_MPDU_START tlv
 * tag is valid
 *
 * @hal_soc_hdl: HAL SOC handle
 * @rx_tlv_hdr: start address of rx_pkt_tlvs
 *
 * Return: true if RX_MPDU_START tlv tag is valid, else false
 */

static inline uint8_t
hal_rx_mpdu_start_tlv_tag_valid(hal_soc_handle_t hal_soc_hdl,
				void *rx_tlv_hdr)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc_hdl;

	if (hal->ops->hal_rx_mpdu_start_tlv_tag_valid)
		return hal->ops->hal_rx_mpdu_start_tlv_tag_valid(rx_tlv_hdr);

	return 0;
}

/**
 * hal_rx_get_next_msdu_link_desc_buf_addr_info(): get next msdu link desc
 *						   buffer addr info
 * @link_desc_va: pointer to current msdu link Desc
 * @next_addr_info: buffer to save next msdu link Desc buffer addr info
 *
 * return: None
 */
static inline void hal_rx_get_next_msdu_link_desc_buf_addr_info(
		void *link_desc_va,
		struct buffer_addr_info *next_addr_info)
{
	struct rx_msdu_link *msdu_link = link_desc_va;

	if (!msdu_link) {
		qdf_mem_zero(next_addr_info, sizeof(struct buffer_addr_info));
		return;
	}

	*next_addr_info = msdu_link->next_msdu_link_desc_addr_info;
}

/**
 * hal_rx_clear_next_msdu_link_desc_buf_addr_info(): clear next msdu link desc
 *						     buffer addr info
 * @link_desc_va: pointer to current msdu link Desc
 *
 * return: None
 */
static inline
void hal_rx_clear_next_msdu_link_desc_buf_addr_info(void *link_desc_va)
{
	struct rx_msdu_link *msdu_link = link_desc_va;

	if (msdu_link)
		qdf_mem_zero(&msdu_link->next_msdu_link_desc_addr_info,
			     sizeof(msdu_link->next_msdu_link_desc_addr_info));
}

/**
 * hal_rx_is_buf_addr_info_valid(): check is the buf_addr_info valid
 *
 * @buf_addr_info: pointer to buf_addr_info structure
 *
 * return: true: has valid paddr, false: not.
 */
static inline
bool hal_rx_is_buf_addr_info_valid(struct buffer_addr_info *buf_addr_info)
{
	return (HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) == 0) ?
						false : true;
}

/**
 * hal_rx_msdu_end_offset_get(): Get the MSDU end offset from
 * rx_pkt_tlvs structure
 *
 * @hal_soc_hdl: HAL SOC handle
 * return: msdu_end_tlv offset value
 */
static inline
uint32_t hal_rx_msdu_end_offset_get(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	return hal_soc->ops->hal_rx_msdu_end_offset_get();
}

/**
 * hal_rx_msdu_start_offset_get(): Get the MSDU start offset from
 * rx_pkt_tlvs structure
 *
 * @hal_soc_hdl: HAL SOC handle
 * return: msdu_start_tlv offset value
 */
static inline
uint32_t hal_rx_msdu_start_offset_get(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	return hal_soc->ops->hal_rx_msdu_start_offset_get();
}

/**
 * hal_rx_mpdu_start_offset_get(): Get the MPDU start offset from
 * rx_pkt_tlvs structure
 *
 * @hal_soc_hdl: HAL SOC handle
 * return: mpdu_start_tlv offset value
 */
static inline
uint32_t hal_rx_mpdu_start_offset_get(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	return hal_soc->ops->hal_rx_mpdu_start_offset_get();
}

static inline
uint32_t hal_rx_pkt_tlv_offset_get(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	return hal_soc->ops->hal_rx_pkt_tlv_offset_get();
}

/**
 * hal_rx_mpdu_end_offset_get(): Get the MPDU end offset from
 * rx_pkt_tlvs structure
 *
 * @hal_soc_hdl: HAL SOC handle
 * return: mpdu_end_tlv offset value
 */
static inline
uint32_t hal_rx_mpdu_end_offset_get(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	return hal_soc->ops->hal_rx_mpdu_end_offset_get();
}

/**
 * hal_rx_attn_offset_get(): Get the ATTENTION offset from
 * rx_pkt_tlvs structure
 *
 * @hal_soc_hdl: HAL SOC handle
 * return: attn_tlv offset value
 */
static inline
uint32_t hal_rx_attn_offset_get(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	return hal_soc->ops->hal_rx_attn_offset_get();
}

/**
 * hal_rx_msdu_desc_info_get_ptr() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 * @hal - pointer to hal_soc
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static inline
void *hal_rx_msdu_ext_desc_info_get_ptr(void *msdu_details_ptr,
					struct hal_soc *hal_soc)
{
	return hal_soc->ops->hal_rx_msdu_ext_desc_info_get_ptr(
						msdu_details_ptr);
}

static inline void
hal_rx_dump_pkt_tlvs(hal_soc_handle_t hal_soc_hdl,
		     uint8_t *buf, uint8_t dbg_level)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_dump_pkt_tlvs(hal_soc_hdl, buf, dbg_level);
}

//TODO - Change the names to not include tlv names
static inline uint16_t
hal_rx_attn_phy_ppdu_id_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_phy_ppdu_id_get(buf);
}

static inline uint32_t
hal_rx_attn_msdu_done_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_msdu_done_get(buf);
}

static inline uint32_t
hal_rx_msdu_start_msdu_len_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_msdu_len_get(buf);
}

static inline uint16_t
hal_rx_get_frame_ctrl_field(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_frame_ctrl_field(buf);
}

static inline int
hal_rx_tlv_get_offload_info(hal_soc_handle_t hal_soc_hdl,
			    uint8_t *rx_pkt_tlv,
			    struct hal_offload_info *offload_info)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_get_offload_info(rx_pkt_tlv,
							 offload_info);

}

static inline int
hal_rx_get_proto_params(hal_soc_handle_t hal_soc_hdl, uint8_t *buf,
			void *proto_params)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_proto_params(buf, proto_params);
}

static inline int
hal_rx_get_l3_l4_offsets(hal_soc_handle_t hal_soc_hdl, uint8_t *buf,
			 uint32_t *l3_hdr_offset, uint32_t *l4_hdr_offset)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_l3_l4_offsets(buf,
						      l3_hdr_offset,
						      l4_hdr_offset);
}

static inline uint32_t
hal_rx_tlv_mic_err_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_mic_err_get(buf);
}

/*
 * hal_rx_tlv_get_pkt_type(): API to get the pkt type
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(pkt type)
 */

static inline uint32_t
hal_rx_tlv_get_pkt_type(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_get_pkt_type(buf);
}

static inline void
hal_rx_tlv_get_pn_num(hal_soc_handle_t hal_soc_hdl,
		      uint8_t *buf, uint64_t *pn_num)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_rx_tlv_get_pn_num(buf, pn_num);
}

static inline uint8_t *
hal_get_reo_ent_desc_qdesc_addr(hal_soc_handle_t hal_soc_hdl, uint8_t *desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_get_reo_ent_desc_qdesc_addr)
		return hal_soc->ops->hal_get_reo_ent_desc_qdesc_addr(desc);

	return NULL;
}

static inline uint8_t *
hal_rx_get_qdesc_addr(hal_soc_handle_t hal_soc_hdl, uint8_t *dst_ring_desc,
		      uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_get_qdesc_addr)
		return hal_soc->ops->hal_rx_get_qdesc_addr(dst_ring_desc, buf);

	return NULL;
}

static inline void
hal_set_reo_ent_desc_reo_dest_ind(hal_soc_handle_t hal_soc_hdl,
				  uint8_t *desc, uint32_t dst_ind)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_set_reo_ent_desc_reo_dest_ind)
		hal_soc->ops->hal_set_reo_ent_desc_reo_dest_ind(desc, dst_ind);
}

static inline uint32_t
hal_rx_tlv_get_is_decrypted(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_tlv_get_is_decrypted)
		return hal_soc->ops->hal_rx_tlv_get_is_decrypted(buf);

	return 0;
}

static inline uint8_t *
hal_rx_pkt_hdr_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_pkt_hdr_get(buf);
}

static inline uint8_t
hal_rx_msdu_get_keyid(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_msdu_get_keyid)
		return hal_soc->ops->hal_rx_msdu_get_keyid(buf);

	return 0;
}

static inline uint32_t
hal_rx_tlv_get_freq(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_tlv_get_freq)
		return hal_soc->ops->hal_rx_tlv_get_freq(buf);

	return 0;
}

static inline void hal_mpdu_desc_info_set(hal_soc_handle_t hal_soc_hdl,
					  void *mpdu_desc_info, uint32_t val)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_mpdu_desc_info_set)
		return hal_soc->ops->hal_mpdu_desc_info_set(
				hal_soc_hdl, mpdu_desc_info, val);
}

static inline void hal_msdu_desc_info_set(hal_soc_handle_t hal_soc_hdl,
					  void *msdu_desc_info,
					  uint32_t val, uint32_t nbuf_len)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_msdu_desc_info_set)
		return hal_soc->ops->hal_msdu_desc_info_set(
				hal_soc_hdl, msdu_desc_info, val, nbuf_len);
}

static inline uint32_t
hal_rx_msdu_reo_dst_ind_get(hal_soc_handle_t hal_soc_hdl, void *msdu_link_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_msdu_reo_dst_ind_get)
		return hal_soc->ops->hal_rx_msdu_reo_dst_ind_get(
				hal_soc_hdl, msdu_link_desc);

	return 0;
}

static inline uint32_t
hal_rx_tlv_sgi_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_sgi_get(buf);
}

static inline uint32_t
hal_rx_tlv_rate_mcs_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_rate_mcs_get(buf);
}

static inline uint32_t
hal_rx_tlv_decrypt_err_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_decrypt_err_get(buf);
}

static inline uint32_t
hal_rx_tlv_first_mpdu_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_first_mpdu_get(buf);
}

static inline uint32_t
hal_rx_tlv_bw_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_bw_get(buf);
}

static inline uint32_t
hal_rx_wbm_err_src_get(hal_soc_handle_t hal_soc_hdl,
		       hal_ring_desc_t ring_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_wbm_err_src_get(ring_desc);
}

/**
 * hal_rx_ret_buf_manager_get: Returns the "return_buffer_manager"
 *			       from the BUFFER_ADDR_INFO structure
 *			       given a REO destination ring descriptor.
 * @ ring_desc: RX(REO/WBM release) destination ring descriptor
 *
 * Return: uint8_t (value of the return_buffer_manager)
 */
static inline uint8_t
hal_rx_ret_buf_manager_get(hal_soc_handle_t hal_soc_hdl,
			   hal_ring_desc_t ring_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_ret_buf_manager_get(ring_desc);
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
static inline void hal_rxdma_buff_addr_info_set(hal_soc_handle_t hal_soc_hdl,
						void *rxdma_entry,
						qdf_dma_addr_t paddr,
						uint32_t cookie,
						uint8_t manager)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rxdma_buff_addr_info_set(rxdma_entry,
							  paddr,
							  cookie,
							  manager);
}

static inline uint32_t
hal_rx_get_reo_error_code(hal_soc_handle_t hal_soc_hdl, hal_ring_desc_t rx_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_get_reo_error_code(rx_desc);
}

static inline void
hal_rx_tlv_csum_err_get(hal_soc_handle_t hal_soc_hdl, uint8_t *rx_tlv_hdr,
			uint32_t *ip_csum_err, uint32_t *tcp_udp_csum_err)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_csum_err_get(rx_tlv_hdr,
						     ip_csum_err,
						     tcp_udp_csum_err);
}

static inline void
hal_rx_tlv_get_pkt_capture_flags(hal_soc_handle_t hal_soc_hdl,
				 uint8_t *rx_tlv_hdr,
				 struct hal_rx_pkt_capture_flags *flags)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_tlv_get_pkt_capture_flags(rx_tlv_hdr,
							      flags);
}

static inline uint8_t
hal_rx_err_status_get(hal_soc_handle_t hal_soc_hdl, hal_ring_desc_t rx_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_err_status_get(rx_desc);
}

static inline uint8_t
hal_rx_reo_buf_type_get(hal_soc_handle_t hal_soc_hdl, hal_ring_desc_t rx_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_reo_buf_type_get(rx_desc);
}

/**
 * hal_rx_reo_prev_pn_get() - Get the previous pn from ring descriptor.
 * @hal_soc_hdl: HAL SoC handle
 * @ring_desc: REO ring descriptor
 * @prev_pn: Buffer to populate the previos PN
 *
 * Return: None
 */
static inline void
hal_rx_reo_prev_pn_get(hal_soc_handle_t hal_soc_hdl, hal_ring_desc_t ring_desc,
		       uint64_t *prev_pn)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_reo_prev_pn_get)
		return hal_soc->ops->hal_rx_reo_prev_pn_get(ring_desc, prev_pn);
}

/**
 * hal_rx_mpdu_info_ampdu_flag_get(): get ampdu flag bit
 * from rx mpdu info
 * @buf: pointer to rx_pkt_tlvs
 *
 * No input validdataion, since this function is supposed to be
 * called from fastpath.
 *
 * Return: ampdu flag
 */
static inline bool
hal_rx_mpdu_info_ampdu_flag_get(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_mpdu_info_ampdu_flag_get(buf);
}

#ifdef WLAN_FEATURE_MARK_FIRST_WAKEUP_PACKET
static inline uint8_t
hal_get_first_wow_wakeup_packet(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_get_first_wow_wakeup_packet(buf);
}
#endif

#endif /* _HAL_RX_H */

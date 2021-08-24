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

#ifndef _HAL_LI_RX_H_
#define _HAL_LI_RX_H_

#include <hal_rx.h>

/*
 * macro to set the cookie into the rxdma ring entry
 */
#define HAL_RXDMA_COOKIE_SET(buff_addr_info, cookie) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) &= \
		~BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK); \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET >> 2))) |= \
		((cookie) << BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB) & \
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK)

/*
 * macro to set the manager into the rxdma ring entry
 */
#define HAL_RXDMA_MANAGER_SET(buff_addr_info, manager) \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET >> 2))) &= \
		~BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK); \
		((*(((unsigned int *)buff_addr_info) + \
		(BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET >> 2))) |= \
		((manager) << BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB) & \
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

#define HAL_RX_BUF_RBM_GET(buff_addr_info)			\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,		\
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_OFFSET)),\
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK,	\
		BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB))

/* TODO: Convert the following structure fields accesseses to offsets */

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

#define HAL_RX_MPDU_BAR_FRAME_GET(mpdu_info_ptr) \
	((mpdu_info_ptr[RX_MPDU_DESC_INFO_0_BAR_FRAME_OFFSET >> 2] & \
	RX_MPDU_DESC_INFO_0_BAR_FRAME_MASK) >> \
	RX_MPDU_DESC_INFO_0_BAR_FRAME_LSB)

/*
 * NOTE: None of the following _GET macros need a right
 * shift by the corresponding _LSB. This is because, they are
 * finally taken and "OR'ed" into a single word again.
 */
#define HAL_RX_MSDU_CONTINUATION_FLAG_SET(msdu_info_ptr, val)		\
	((*(((uint32_t *)msdu_info_ptr) +				\
		(RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_OFFSET >> 2))) |= \
		((val) << RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_LSB) & \
		RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_MASK)

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

#define HAL_RX_REO_MSDU_REO_DST_IND_GET(reo_desc)	\
	(HAL_RX_MSDU_REO_DST_IND_GET(&		\
	(((struct reo_destination_ring *)	\
	   reo_desc)->rx_msdu_desc_info_details)))

#define HAL_RX_MPDU_ENCRYPT_TYPE_GET(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
	RX_MPDU_INFO_3_ENCRYPT_TYPE_OFFSET)),		\
	RX_MPDU_INFO_3_ENCRYPT_TYPE_MASK,		\
	RX_MPDU_INFO_3_ENCRYPT_TYPE_LSB))

#define HAL_RX_MSDU_DESC_INFO_SET(_msdu_info_ptr, _field, _val)		\
	HAL_RX_FLD_SET(_msdu_info_ptr, RX_MSDU_DESC_INFO_0,		\
			_field, _val)

#define HAL_RX_MPDU_DESC_INFO_SET(_mpdu_info_ptr, _field, _val)		\
	HAL_RX_FLD_SET(_mpdu_info_ptr, RX_MPDU_DESC_INFO_0,		\
			_field, _val)

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

/* rx_pkt_tlvs structure should be used to process Data buffers, monitor status
 * buffers, monitor destination buffers and monitor descriptor buffers.
 */
#ifdef RXDMA_OPTIMIZATION
/*
 * The RX_PADDING_BYTES is required so that the TLV's don't
 * spread across the 128 byte boundary
 * RXDMA optimization requires:
 * 1) MSDU_END & ATTENTION TLV's follow in that order
 * 2) TLV's don't span across 128 byte lines
 * 3) Rx Buffer is nicely aligned on the 128 byte boundary
 */
#define RX_PADDING0_BYTES	4
#define RX_PADDING1_BYTES	16
struct rx_pkt_tlvs {
	struct rx_msdu_end_tlv   msdu_end_tlv;	/*  72 bytes */
	struct rx_attention_tlv  attn_tlv;	/*  16 bytes */
	struct rx_msdu_start_tlv msdu_start_tlv;/*  40 bytes */
	uint8_t rx_padding0[RX_PADDING0_BYTES];	/*   4 bytes */
	struct rx_mpdu_start_tlv mpdu_start_tlv;/*  96 bytes */
	struct rx_mpdu_end_tlv   mpdu_end_tlv;	/*  12 bytes */
	uint8_t rx_padding1[RX_PADDING1_BYTES];	/*  16 bytes */
#ifndef NO_RX_PKT_HDR_TLV
	struct rx_pkt_hdr_tlv	 pkt_hdr_tlv;	/* 128 bytes */
#endif
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

/* rx_mon_pkt_tlvs structure should be used to process monitor data buffers */
#ifdef RXDMA_OPTIMIZATION
struct rx_mon_pkt_tlvs {
	struct rx_msdu_end_tlv   msdu_end_tlv;	/*  72 bytes */
	struct rx_attention_tlv  attn_tlv;	/*  16 bytes */
	struct rx_msdu_start_tlv msdu_start_tlv;/*  40 bytes */
	uint8_t rx_padding0[RX_PADDING0_BYTES];	/*   4 bytes */
	struct rx_mpdu_start_tlv mpdu_start_tlv;/*  96 bytes */
	struct rx_mpdu_end_tlv   mpdu_end_tlv;	/*  12 bytes */
	uint8_t rx_padding1[RX_PADDING1_BYTES];	/*  16 bytes */
	struct rx_pkt_hdr_tlv	 pkt_hdr_tlv;	/* 128 bytes */
};
#else /* RXDMA_OPTIMIZATION */
struct rx_mon_pkt_tlvs {
	struct rx_attention_tlv  attn_tlv;
	struct rx_mpdu_start_tlv mpdu_start_tlv;
	struct rx_msdu_start_tlv msdu_start_tlv;
	struct rx_msdu_end_tlv   msdu_end_tlv;
	struct rx_mpdu_end_tlv   mpdu_end_tlv;
	struct rx_pkt_hdr_tlv	 pkt_hdr_tlv;
};
#endif

#define SIZE_OF_MONITOR_TLV sizeof(struct rx_mon_pkt_tlvs)
#define SIZE_OF_DATA_RX_TLV sizeof(struct rx_pkt_tlvs)

#define RX_PKT_TLVS_LEN		SIZE_OF_DATA_RX_TLV

#define RX_PKT_TLV_OFFSET(field) qdf_offsetof(struct rx_pkt_tlvs, field)

#define HAL_RX_PKT_TLV_MPDU_START_OFFSET(hal_soc) \
					RX_PKT_TLV_OFFSET(mpdu_start_tlv)
#define HAL_RX_PKT_TLV_MPDU_END_OFFSET(hal_soc) RX_PKT_TLV_OFFSET(mpdu_end_tlv)
#define HAL_RX_PKT_TLV_MSDU_START_OFFSET(hal_soc) \
					RX_PKT_TLV_OFFSET(msdu_start_tlv)
#define HAL_RX_PKT_TLV_MSDU_END_OFFSET(hal_soc) RX_PKT_TLV_OFFSET(msdu_end_tlv)
#define HAL_RX_PKT_TLV_ATTN_OFFSET(hal_soc) RX_PKT_TLV_OFFSET(attn_tlv)
#define HAL_RX_PKT_TLV_PKT_HDR_OFFSET(hal_soc) RX_PKT_TLV_OFFSET(pkt_hdr_tlv)

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
 * hal_rx_mon_dest_get_buffer_info_from_tlv(): Retrieve mon dest frame info
 * from the reserved bytes of rx_tlv_hdr.
 * @buf: start of rx_tlv_hdr
 * @buf_info: hal_rx_mon_dest_buf_info structure
 *
 * Return: void
 */
static inline void hal_rx_mon_dest_get_buffer_info_from_tlv(
				uint8_t *buf,
				struct hal_rx_mon_dest_buf_info *buf_info)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	qdf_mem_copy(buf_info, pkt_tlvs->rx_padding0,
		     sizeof(struct hal_rx_mon_dest_buf_info));
}

/*
 * Get msdu_done bit from the RX_ATTENTION TLV
 */
#define HAL_RX_ATTN_MSDU_DONE_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_2_MSDU_DONE_OFFSET)),	\
		RX_ATTENTION_2_MSDU_DONE_MASK,		\
		RX_ATTENTION_2_MSDU_DONE_LSB))

#define HAL_RX_ATTN_FIRST_MPDU_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_1_FIRST_MPDU_OFFSET)),	\
		RX_ATTENTION_1_FIRST_MPDU_MASK,		\
		RX_ATTENTION_1_FIRST_MPDU_LSB))

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
	uint8_t tcp_udp_cksum_fail;

	tcp_udp_cksum_fail = HAL_RX_ATTN_TCP_UDP_CKSUM_FAIL_GET(rx_attn);

	return !!tcp_udp_cksum_fail;
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
	uint8_t	 ip_cksum_fail;

	ip_cksum_fail = HAL_RX_ATTN_IP_CKSUM_FAIL_GET(rx_attn);

	return !!ip_cksum_fail;
}

#define HAL_RX_ATTN_PHY_PPDU_ID_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_0_PHY_PPDU_ID_OFFSET)),	\
		RX_ATTENTION_0_PHY_PPDU_ID_MASK,	\
		RX_ATTENTION_0_PHY_PPDU_ID_LSB))

#define HAL_RX_ATTN_CCE_MATCH_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_1_CCE_MATCH_OFFSET)),		\
		RX_ATTENTION_1_CCE_MATCH_MASK,			\
		RX_ATTENTION_1_CCE_MATCH_LSB))

/*
 * hal_rx_msdu_cce_match_get(): get CCE match bit
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 * Return: CCE match value
 */
static inline bool
hal_rx_msdu_cce_match_get(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint8_t cce_match_val;

	cce_match_val = HAL_RX_ATTN_CCE_MATCH_GET(rx_attn);
	return !!cce_match_val;
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

#define HAL_RX_MPDU_PEER_META_DATA_SET(_rx_mpdu_info, peer_mdata)	\
		((*(((uint32_t *)_rx_mpdu_info) +			\
		(RX_MPDU_INFO_8_PEER_META_DATA_OFFSET >> 2))) =		\
		((peer_mdata) << RX_MPDU_INFO_8_PEER_META_DATA_LSB) &	\
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

#define HAL_RX_TLV_GET_UDP_PROTO(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_start_tlv.rx_msdu_start), \
			 RX_MSDU_START_2_UDP_PROTO_OFFSET)), \
		RX_MSDU_START_2_UDP_PROTO_MASK, \
		RX_MSDU_START_2_UDP_PROTO_LSB))

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

#define HAL_RX_MSDU_START_MSDU_LEN_GET(_rx_msdu_start)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,		\
		RX_MSDU_START_1_MSDU_LENGTH_OFFSET)),		\
		RX_MSDU_START_1_MSDU_LENGTH_MASK,		\
		RX_MSDU_START_1_MSDU_LENGTH_LSB))

#define HAL_RX_MSDU_START_BW_GET(_rx_msdu_start)     \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_RECEIVE_BANDWIDTH_OFFSET)), \
	RX_MSDU_START_5_RECEIVE_BANDWIDTH_MASK,     \
	RX_MSDU_START_5_RECEIVE_BANDWIDTH_LSB))

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

#define HAL_RX_MSDU_START_SGI_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
		RX_MSDU_START_5_SGI_OFFSET)),		\
		RX_MSDU_START_5_SGI_MASK,		\
		RX_MSDU_START_5_SGI_LSB))

#define HAL_RX_MSDU_START_RATE_MCS_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
		RX_MSDU_START_5_RATE_MCS_OFFSET)),	\
		RX_MSDU_START_5_RATE_MCS_MASK,		\
		RX_MSDU_START_5_RATE_MCS_LSB))

#define HAL_RX_ATTN_DECRYPT_STATUS_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,		\
		RX_ATTENTION_2_DECRYPT_STATUS_CODE_OFFSET)),	\
		RX_ATTENTION_2_DECRYPT_STATUS_CODE_MASK,	\
		RX_ATTENTION_2_DECRYPT_STATUS_CODE_LSB))

/*
 * Get key index from RX_MSDU_END
 */
#define HAL_RX_MSDU_END_KEYID_OCTET_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_2_KEY_ID_OCTET_OFFSET)),	\
		RX_MSDU_END_2_KEY_ID_OCTET_MASK,	\
		RX_MSDU_END_2_KEY_ID_OCTET_LSB))

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
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t rssi;

	rssi = HAL_RX_MSDU_START_RSSI_GET(msdu_start);

	return rssi;
}

#define HAL_RX_MSDU_START_FREQ_GET(_rx_msdu_start)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,		\
		RX_MSDU_START_7_SW_PHY_META_DATA_OFFSET)),      \
		RX_MSDU_START_7_SW_PHY_META_DATA_MASK,		\
		RX_MSDU_START_7_SW_PHY_META_DATA_LSB))

#define HAL_RX_MSDU_START_PKT_TYPE_GET(_rx_msdu_start)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_start,  \
		RX_MSDU_START_5_PKT_TYPE_OFFSET)),      \
		RX_MSDU_START_5_PKT_TYPE_MASK,		\
		RX_MSDU_START_5_PKT_TYPE_LSB))

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

/*******************************************************************************
 * RX ERROR APIS
 ******************************************************************************/

#define HAL_RX_MPDU_END_DECRYPT_ERR_GET(_rx_mpdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_end),\
		RX_MPDU_END_1_RX_IN_TX_DECRYPT_BYP_OFFSET)),	\
		RX_MPDU_END_1_RX_IN_TX_DECRYPT_BYP_MASK,	\
		RX_MPDU_END_1_RX_IN_TX_DECRYPT_BYP_LSB))

#define HAL_RX_MPDU_END_MIC_ERR_GET(_rx_mpdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_end),\
		RX_MPDU_END_1_TKIP_MIC_ERR_OFFSET)),	\
		RX_MPDU_END_1_TKIP_MIC_ERR_MASK,	\
		RX_MPDU_END_1_TKIP_MIC_ERR_LSB))


/*******************************************************************************
 * RX REO ERROR APIS
 ******************************************************************************/


#define HAL_RX_REO_BUF_TYPE_GET(reo_desc) (((*(((uint32_t *)reo_desc) + \
		(REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_OFFSET >> 2))) & \
		REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_MASK) >> \
		REO_DESTINATION_RING_7_REO_DEST_BUFFER_TYPE_LSB)

#define HAL_RX_REO_QUEUE_NUMBER_GET(reo_desc) (((*(((uint32_t *)reo_desc) + \
		(REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER_OFFSET >> 2))) & \
		REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER_MASK) >> \
		REO_DESTINATION_RING_7_RECEIVE_QUEUE_NUMBER_LSB)

#define HAL_RX_REO_ERROR_GET(reo_desc) (((*(((uint32_t *)reo_desc) + \
		(REO_DESTINATION_RING_7_REO_ERROR_CODE_OFFSET >> 2))) & \
		REO_DESTINATION_RING_7_REO_ERROR_CODE_MASK) >> \
		REO_DESTINATION_RING_7_REO_ERROR_CODE_LSB)

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
static inline
void hal_rx_msdu_link_desc_reinject(struct hal_soc *soc,
				    uint64_t pa,
				    uint32_t cookie,
				    bool error_enabled_reo_q)
{
	/* TODO */
}

#define HAL_RX_WBM_FIRST_MSDU_GET(wbm_desc)		\
	(((*(((uint32_t *)wbm_desc) +			\
	(WBM_RELEASE_RING_4_FIRST_MSDU_OFFSET >> 2))) & \
	WBM_RELEASE_RING_4_FIRST_MSDU_MASK) >>		\
	WBM_RELEASE_RING_4_FIRST_MSDU_LSB)

#define HAL_RX_WBM_LAST_MSDU_GET(wbm_desc)		\
	(((*(((uint32_t *)wbm_desc) +			\
	(WBM_RELEASE_RING_4_LAST_MSDU_OFFSET >> 2))) &  \
	WBM_RELEASE_RING_4_LAST_MSDU_MASK) >>		\
	WBM_RELEASE_RING_4_LAST_MSDU_LSB)

#define HAL_RX_WBM_BUF_COOKIE_GET(wbm_desc) \
	HAL_RX_BUF_COOKIE_GET(&((struct wbm_release_ring *) \
	wbm_desc)->released_buff_or_desc_addr_info)

static inline
uint32_t
HAL_RX_DESC_GET_DECAP_FORMAT(void *hw_desc_addr) {
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_msdu_start = &rx_desc->msdu_start_tlv.rx_msdu_start;

	return HAL_RX_GET(rx_msdu_start, RX_MSDU_START_2, DECAP_FORMAT);
}

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
	hal_verbose_debug("rx_attention tlv (1/2) - "
			  "rxpcu_mpdu_filter_in_category: %x "
			  "sw_frame_group_id: %x "
			  "reserved_0: %x "
			  "phy_ppdu_id: %x "
			  "first_mpdu : %x "
			  "reserved_1a: %x "
			  "mcast_bcast: %x "
			  "ast_index_not_found: %x "
			  "ast_index_timeout: %x "
			  "power_mgmt: %x "
			  "non_qos: %x "
			  "null_data: %x "
			  "mgmt_type: %x "
			  "ctrl_type: %x "
			  "more_data: %x "
			  "eosp: %x "
			  "a_msdu_error: %x "
			  "fragment_flag: %x "
			  "order: %x "
			  "cce_match: %x "
			  "overflow_err: %x "
			  "msdu_length_err: %x "
			  "tcp_udp_chksum_fail: %x "
			  "ip_chksum_fail: %x "
			  "sa_idx_invalid: %x "
			  "da_idx_invalid: %x "
			  "reserved_1b: %x "
			  "rx_in_tx_decrypt_byp: %x ",
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
			  rx_attn->rx_in_tx_decrypt_byp);

	hal_verbose_debug("rx_attention tlv (2/2) - "
			  "encrypt_required: %x "
			  "directed: %x "
			  "buffer_fragment: %x "
			  "mpdu_length_err: %x "
			  "tkip_mic_err: %x "
			  "decrypt_err: %x "
			  "unencrypted_frame_err: %x "
			  "fcs_err: %x "
			  "flow_idx_timeout: %x "
			  "flow_idx_invalid: %x "
			  "wifi_parser_error: %x "
			  "amsdu_parser_error: %x "
			  "sa_idx_timeout: %x "
			  "da_idx_timeout: %x "
			  "msdu_limit_error: %x "
			  "da_is_valid: %x "
			  "da_is_mcbc: %x "
			  "sa_is_valid: %x "
			  "decrypt_status_code: %x "
			  "rx_bitmap_not_updated: %x "
			  "reserved_2: %x "
			  "msdu_done: %x ",
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
	hal_verbose_debug("rx_mpdu_end tlv - "
			  "rxpcu_mpdu_filter_in_category: %x "
			  "sw_frame_group_id: %x "
			  "phy_ppdu_id: %x "
			  "unsup_ktype_short_frame: %x "
			  "rx_in_tx_decrypt_byp: %x "
			  "overflow_err: %x "
			  "mpdu_length_err: %x "
			  "tkip_mic_err: %x "
			  "decrypt_err: %x "
			  "unencrypted_frame_err: %x "
			  "pn_fields_contain_valid_info: %x "
			  "fcs_err: %x "
			  "msdu_length_err: %x "
			  "rxdma0_destination_ring: %x "
			  "rxdma1_destination_ring: %x "
			  "decrypt_status_code: %x "
			  "rx_bitmap_not_updated: %x ",
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

#ifdef NO_RX_PKT_HDR_TLV
static inline void hal_rx_dump_pkt_hdr_tlv(struct rx_pkt_tlvs *pkt_tlvs,
					   uint8_t dbg_level)
{
}
#else
/**
 * hal_rx_dump_pkt_hdr_tlv: dump RX pkt header TLV in hex format
 * @ pkt_hdr_tlv: pointer the pkt_hdr_tlv in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static inline void hal_rx_dump_pkt_hdr_tlv(struct rx_pkt_tlvs *pkt_tlvs,
					   uint8_t dbg_level)
{
	struct rx_pkt_hdr_tlv *pkt_hdr_tlv = &pkt_tlvs->pkt_hdr_tlv;

	hal_verbose_debug("\n---------------\nrx_pkt_hdr_tlv"
			  "\n---------------\nphy_ppdu_id %d ",
			  pkt_hdr_tlv->phy_ppdu_id);
	hal_verbose_hex_dump(pkt_hdr_tlv->rx_pkt_hdr, 128);
}
#endif

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

static inline
void hal_rx_mpdu_desc_info_get_li(void *desc_addr,
				  void *mpdu_desc_info_hdl)
{
	struct reo_destination_ring *reo_dst_ring;
	struct hal_rx_mpdu_desc_info *mpdu_desc_info =
		(struct hal_rx_mpdu_desc_info *)mpdu_desc_info_hdl;
	uint32_t *mpdu_info;

	reo_dst_ring = (struct reo_destination_ring *)desc_addr;

	mpdu_info = (uint32_t *)&reo_dst_ring->rx_mpdu_desc_info_details;

	mpdu_desc_info->msdu_count = HAL_RX_MPDU_MSDU_COUNT_GET(mpdu_info);
	mpdu_desc_info->mpdu_seq = HAL_RX_MPDU_SEQUENCE_NUMBER_GET(mpdu_info);
	mpdu_desc_info->mpdu_flags = HAL_RX_MPDU_FLAGS_GET(mpdu_info);
	mpdu_desc_info->peer_meta_data =
		HAL_RX_MPDU_DESC_PEER_META_DATA_GET(mpdu_info);
	mpdu_desc_info->bar_frame = HAL_RX_MPDU_BAR_FRAME_GET(mpdu_info);
}

/**
 * hal_rx_attn_msdu_done_get_li() - Get msdi done flag from RX TLV
 * @buf: RX tlv address
 *
 * Return: msdu done flag
 */
static inline uint32_t hal_rx_attn_msdu_done_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint32_t msdu_done;

	msdu_done = HAL_RX_ATTN_MSDU_DONE_GET(rx_attn);

	return msdu_done;
}

#define HAL_RX_MSDU_FLAGS_GET(msdu_info_ptr) \
	(HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_LAST_MSDU_IN_MPDU_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_CONTINUATION_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_SA_IS_VALID_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_SA_IDX_TIMEOUT_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_DA_IS_VALID_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_DA_IS_MCBC_FLAG_GET(msdu_info_ptr) | \
	HAL_RX_MSDU_DA_IDX_TIMEOUT_FLAG_GET(msdu_info_ptr))

/**
 * hal_rx_msdu_flags_get_li() - Get msdu flags from ring desc
 * @msdu_desc_info_hdl: msdu desc info handle
 *
 * Return: msdu flags
 */
static inline
uint32_t hal_rx_msdu_flags_get_li(rx_msdu_desc_info_t msdu_desc_info_hdl)
{
	struct rx_msdu_desc_info *msdu_desc_info =
		(struct rx_msdu_desc_info *)msdu_desc_info_hdl;

	return HAL_RX_MSDU_FLAGS_GET(msdu_desc_info);
}

/*
 *hal_rx_msdu_desc_info_get_li: Gets the flags related to MSDU descriptor.
 *@desc_addr: REO ring descriptor addr
 *@msdu_desc_info: Holds MSDU descriptor info from HAL Rx descriptor
 *
 * Specifically flags needed are: first_msdu_in_mpdu,
 * last_msdu_in_mpdu, msdu_continuation, sa_is_valid,
 * sa_idx_timeout, da_is_valid, da_idx_timeout, da_is_MCBC
 *

 *Return: void
 */
static inline void
hal_rx_msdu_desc_info_get_li(void *desc_addr,
			     struct hal_rx_msdu_desc_info *msdu_desc_info)
{
	struct reo_destination_ring *reo_dst_ring;
	uint32_t *msdu_info;

	reo_dst_ring = (struct reo_destination_ring *)desc_addr;

	msdu_info = (uint32_t *)&reo_dst_ring->rx_msdu_desc_info_details;
	msdu_desc_info->msdu_flags =
		hal_rx_msdu_flags_get_li((struct rx_msdu_desc_info *)msdu_info);
	msdu_desc_info->msdu_len = HAL_RX_MSDU_PKT_LENGTH_GET(msdu_info);
}

#define HAL_RX_MSDU_START_NSS_GET(_rx_msdu_start)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),	\
	RX_MSDU_START_5_NSS_OFFSET)),				\
	RX_MSDU_START_5_NSS_MASK,				\
	RX_MSDU_START_5_NSS_LSB))

#define HAL_RX_ATTN_MSDU_LEN_ERR_GET(_rx_attn)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_attn,	\
		RX_ATTENTION_1_MSDU_LENGTH_ERR_OFFSET)),	\
		RX_ATTENTION_1_MSDU_LENGTH_ERR_MASK,		\
		RX_ATTENTION_1_MSDU_LENGTH_ERR_LSB))

/**
 * hal_rx_attn_msdu_len_err_get_li(): Get msdu_len_err value from
 *  rx attention tlvs
 * @buf: pointer to rx pkt tlvs hdr
 *
 * Return: msdu_len_err value
 */
static inline uint32_t
hal_rx_attn_msdu_len_err_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;

	return HAL_RX_ATTN_MSDU_LEN_ERR_GET(rx_attn);
}
#endif /* _HAL_LI_RX_H_ */

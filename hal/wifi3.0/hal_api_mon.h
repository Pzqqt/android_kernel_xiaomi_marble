/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_API_MON_H_
#define _HAL_API_MON_H_

#include "qdf_types.h"
#include "hal_internal.h"
#include <target_type.h>

#define HAL_RX_PHY_DATA_RADAR 0x01
#define HAL_SU_MU_CODING_LDPC 0x01

#define HAL_RX_FCS_LEN (4)
#define KEY_EXTIV 0x20

#define HAL_RX_USER_TLV32_TYPE_OFFSET		0x00000000
#define HAL_RX_USER_TLV32_TYPE_LSB		1
#define HAL_RX_USER_TLV32_TYPE_MASK		0x000003FE

#define HAL_RX_USER_TLV32_LEN_OFFSET		0x00000000
#define HAL_RX_USER_TLV32_LEN_LSB		10
#define HAL_RX_USER_TLV32_LEN_MASK		0x003FFC00

#define HAL_RX_USER_TLV32_USERID_OFFSET		0x00000000
#define HAL_RX_USER_TLV32_USERID_LSB		26
#define HAL_RX_USER_TLV32_USERID_MASK		0xFC000000

#define HAL_ALIGN(x, a)				HAL_ALIGN_MASK(x, (a)-1)
#define HAL_ALIGN_MASK(x, mask)	(typeof(x))(((uint32)(x) + (mask)) & ~(mask))

#define HAL_RX_TLV32_HDR_SIZE			4

#define HAL_RX_GET_USER_TLV32_TYPE(rx_status_tlv_ptr) \
		((*((uint32_t *)(rx_status_tlv_ptr)) & \
		HAL_RX_USER_TLV32_TYPE_MASK) >> \
		HAL_RX_USER_TLV32_TYPE_LSB)

#define HAL_RX_GET_USER_TLV32_LEN(rx_status_tlv_ptr) \
		((*((uint32_t *)(rx_status_tlv_ptr)) & \
		HAL_RX_USER_TLV32_LEN_MASK) >> \
		HAL_RX_USER_TLV32_LEN_LSB)

#define HAL_RX_GET_USER_TLV32_USERID(rx_status_tlv_ptr) \
		((*((uint32_t *)(rx_status_tlv_ptr)) & \
		HAL_RX_USER_TLV32_USERID_MASK) >> \
		HAL_RX_USER_TLV32_USERID_LSB)

#define HAL_TLV_STATUS_PPDU_NOT_DONE		0
#define HAL_TLV_STATUS_PPDU_DONE		1
#define HAL_TLV_STATUS_BUF_DONE			2
#define HAL_TLV_STATUS_PPDU_NON_STD_DONE	3


#define HAL_MAX_UL_MU_USERS			8

#define HAL_RX_PKT_TYPE_11A	0
#define HAL_RX_PKT_TYPE_11B	1
#define HAL_RX_PKT_TYPE_11N	2
#define HAL_RX_PKT_TYPE_11AC	3
#define HAL_RX_PKT_TYPE_11AX	4

#define HAL_RX_RECEPTION_TYPE_SU	0
#define HAL_RX_RECEPTION_TYPE_MU_MIMO	1
#define HAL_RX_RECEPTION_TYPE_OFDMA	2
#define HAL_RX_RECEPTION_TYPE_MU_OFDMA	3

/* Multiply rate by 2 to avoid float point
 * and get rate in units of 500kbps
 */
#define HAL_11B_RATE_0MCS	11*2
#define HAL_11B_RATE_1MCS	5.5*2
#define HAL_11B_RATE_2MCS	2*2
#define HAL_11B_RATE_3MCS	1*2
#define HAL_11B_RATE_4MCS	11*2
#define HAL_11B_RATE_5MCS	5.5*2
#define HAL_11B_RATE_6MCS	2*2

#define HAL_11A_RATE_0MCS	48*2
#define HAL_11A_RATE_1MCS	24*2
#define HAL_11A_RATE_2MCS	12*2
#define HAL_11A_RATE_3MCS	6*2
#define HAL_11A_RATE_4MCS	54*2
#define HAL_11A_RATE_5MCS	36*2
#define HAL_11A_RATE_6MCS	18*2
#define HAL_11A_RATE_7MCS	9*2

#define HAL_LEGACY_MCS0  0
#define HAL_LEGACY_MCS1  1
#define HAL_LEGACY_MCS2  2
#define HAL_LEGACY_MCS3  3
#define HAL_LEGACY_MCS4  4
#define HAL_LEGACY_MCS5  5
#define HAL_LEGACY_MCS6  6
#define HAL_LEGACY_MCS7  7

#define HE_GI_0_8 0
#define HE_GI_0_4 1
#define HE_GI_1_6 2
#define HE_GI_3_2 3

#define HT_SGI_PRESENT 0x80

#define HE_LTF_1_X 0
#define HE_LTF_2_X 1
#define HE_LTF_4_X 2
#define HE_LTF_UNKNOWN 3
#define VHT_SIG_SU_NSS_MASK	0x7
#define HT_SIG_SU_NSS_SHIFT	0x3

#define HAL_TID_INVALID 31
#define HAL_AST_IDX_INVALID 0xFFFF

#ifdef GET_MSDU_AGGREGATION
#define HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs)\
{\
	struct rx_msdu_end *rx_msdu_end;\
	bool first_msdu, last_msdu; \
	rx_msdu_end = &rx_desc->msdu_end_tlv.rx_msdu_end;\
	first_msdu = HAL_RX_GET(rx_msdu_end, RX_MSDU_END_5, FIRST_MSDU);\
	last_msdu = HAL_RX_GET(rx_msdu_end, RX_MSDU_END_5, LAST_MSDU);\
	if (first_msdu && last_msdu)\
		rs->rs_flags &= (~IEEE80211_AMSDU_FLAG);\
	else\
		rs->rs_flags |= (IEEE80211_AMSDU_FLAG); \
} \

#else
#define HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs)
#endif

enum {
	DP_PPDU_STATUS_START,
	DP_PPDU_STATUS_DONE,
};

static inline
uint32_t HAL_RX_MON_HW_RX_DESC_SIZE(void)
{
	/* return the HW_RX_DESC size */
	return sizeof(struct rx_pkt_tlvs);
}

static inline
uint8_t *HAL_RX_MON_DEST_GET_DESC(uint8_t *data)
{
	return data;
}

static inline
uint32_t HAL_RX_DESC_GET_MPDU_LENGTH_ERR(void *hw_desc_addr)
{
	struct rx_attention *rx_attn;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_attn = &rx_desc->attn_tlv.rx_attn;

	return HAL_RX_GET(rx_attn, RX_ATTENTION_1, MPDU_LENGTH_ERR);
}

static inline
uint32_t HAL_RX_DESC_GET_MPDU_FCS_ERR(void *hw_desc_addr)
{
	struct rx_attention *rx_attn;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_attn = &rx_desc->attn_tlv.rx_attn;

	return HAL_RX_GET(rx_attn, RX_ATTENTION_1, FCS_ERR);
}

/*
 * HAL_RX_HW_DESC_MPDU_VALID() - check MPDU start TLV tag in MPDU
 *			start TLV of Hardware TLV descriptor
 * @hw_desc_addr: Hardware desciptor address
 *
 * Return: bool: if TLV tag match
 */
static inline
bool HAL_RX_HW_DESC_MPDU_VALID(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;
	uint32_t tlv_tag;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(
		&rx_desc->mpdu_start_tlv);

	return tlv_tag == WIFIRX_MPDU_START_E ? true : false;
}

static inline
uint32_t HAL_RX_HW_DESC_GET_PPDUID_GET(void *hw_desc_addr)
{
	struct rx_mpdu_info *rx_mpdu_info;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_mpdu_info =
		&rx_desc->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;

	return HAL_RX_GET(rx_mpdu_info, RX_MPDU_INFO_0, PHY_PPDU_ID);
}

/* TODO: Move all Rx descriptor functions to hal_rx.h to avoid duplication */

#define HAL_RX_BUFFER_ADDR_31_0_GET(buff_addr_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(buff_addr_info,		\
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_OFFSET)),	\
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK,	\
		BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB))

#define HAL_RX_REO_ENT_BUFFER_ADDR_39_32_GET(reo_ent_desc)	\
	(HAL_RX_BUFFER_ADDR_39_32_GET(&				\
		(((struct reo_entrance_ring *)reo_ent_desc)	\
			->reo_level_mpdu_frame_info.msdu_link_desc_addr_info)))

#define HAL_RX_REO_ENT_BUFFER_ADDR_31_0_GET(reo_ent_desc)	\
	(HAL_RX_BUFFER_ADDR_31_0_GET(&				\
		(((struct reo_entrance_ring *)reo_ent_desc)	\
			->reo_level_mpdu_frame_info.msdu_link_desc_addr_info)))

#define HAL_RX_REO_ENT_BUF_COOKIE_GET(reo_ent_desc)		\
	(HAL_RX_BUF_COOKIE_GET(&					\
		(((struct reo_entrance_ring *)reo_ent_desc)	\
			->reo_level_mpdu_frame_info.msdu_link_desc_addr_info)))

/**
 * hal_rx_reo_ent_buf_paddr_get: Gets the physical address and
 * cookie from the REO entrance ring element
 *
 * @ hal_rx_desc_cookie: Opaque cookie pointer used by HAL to get to
 * the current descriptor
 * @ buf_info: structure to return the buffer information
 * @ msdu_cnt: pointer to msdu count in MPDU
 * Return: void
 */
static inline
void hal_rx_reo_ent_buf_paddr_get(void *rx_desc,
	struct hal_buf_info *buf_info,
	void **pp_buf_addr_info,
	uint32_t *msdu_cnt
)
{
	struct reo_entrance_ring *reo_ent_ring =
		(struct reo_entrance_ring *)rx_desc;
	struct buffer_addr_info *buf_addr_info;
	struct rx_mpdu_desc_info *rx_mpdu_desc_info_details;
	uint32_t loop_cnt;

	rx_mpdu_desc_info_details =
	&reo_ent_ring->reo_level_mpdu_frame_info.rx_mpdu_desc_info_details;

	*msdu_cnt = HAL_RX_GET(rx_mpdu_desc_info_details,
				RX_MPDU_DESC_INFO_0, MSDU_COUNT);

	loop_cnt = HAL_RX_GET(reo_ent_ring, REO_ENTRANCE_RING_7, LOOPING_COUNT);

	buf_addr_info =
	&reo_ent_ring->reo_level_mpdu_frame_info.msdu_link_desc_addr_info;

	buf_info->paddr =
		(HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) |
		((uint64_t)
		(HAL_RX_BUFFER_ADDR_39_32_GET(buf_addr_info)) << 32));

	buf_info->sw_cookie = HAL_RX_BUF_COOKIE_GET(buf_addr_info);

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"[%s][%d] ReoAddr=%pK, addrInfo=%pK, paddr=0x%llx, loopcnt=%d",
		__func__, __LINE__, reo_ent_ring, buf_addr_info,
	(unsigned long long)buf_info->paddr, loop_cnt);

	*pp_buf_addr_info = (void *)buf_addr_info;
}

static inline
void hal_rx_mon_next_link_desc_get(void *rx_msdu_link_desc,
	struct hal_buf_info *buf_info, void **pp_buf_addr_info)
{
	struct rx_msdu_link *msdu_link =
		(struct rx_msdu_link *)rx_msdu_link_desc;
	struct buffer_addr_info *buf_addr_info;

	buf_addr_info = &msdu_link->next_msdu_link_desc_addr_info;

	buf_info->paddr =
		(HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) |
		((uint64_t)
		(HAL_RX_BUFFER_ADDR_39_32_GET(buf_addr_info)) << 32));

	buf_info->sw_cookie = HAL_RX_BUF_COOKIE_GET(buf_addr_info);

	*pp_buf_addr_info = (void *)buf_addr_info;
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

static inline void hal_rx_mon_msdu_link_desc_set(struct hal_soc *soc,
			void *src_srng_desc, void *buf_addr_info)
{
	struct buffer_addr_info *wbm_srng_buffer_addr_info =
			(struct buffer_addr_info *)src_srng_desc;
	uint64_t paddr;
	struct buffer_addr_info *p_buffer_addr_info =
			(struct buffer_addr_info *)buf_addr_info;

	paddr =
		(HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) |
		((uint64_t)
		(HAL_RX_BUFFER_ADDR_39_32_GET(buf_addr_info)) << 32));

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		"[%s][%d] src_srng_desc=%pK, buf_addr=0x%llx, cookie=0x%llx",
		__func__, __LINE__, src_srng_desc, (unsigned long long)paddr,
		(unsigned long long)p_buffer_addr_info->sw_buffer_cookie);

	/* Structure copy !!! */
	*wbm_srng_buffer_addr_info =
		*((struct buffer_addr_info *)buf_addr_info);
}

static inline
uint32 hal_get_rx_msdu_link_desc_size(void)
{
	return sizeof(struct rx_msdu_link);
}

enum {
	HAL_PKT_TYPE_OFDM = 0,
	HAL_PKT_TYPE_CCK,
	HAL_PKT_TYPE_HT,
	HAL_PKT_TYPE_VHT,
	HAL_PKT_TYPE_HE,
};

enum {
	HAL_SGI_0_8_US,
	HAL_SGI_0_4_US,
	HAL_SGI_1_6_US,
	HAL_SGI_3_2_US,
};

enum {
	HAL_FULL_RX_BW_20,
	HAL_FULL_RX_BW_40,
	HAL_FULL_RX_BW_80,
	HAL_FULL_RX_BW_160,
};

enum {
	HAL_RX_TYPE_SU,
	HAL_RX_TYPE_MU_MIMO,
	HAL_RX_TYPE_MU_OFDMA,
	HAL_RX_TYPE_MU_OFDMA_MIMO,
};

/**
 * enum
 * @HAL_RX_MON_PPDU_START: PPDU start TLV is decoded in HAL
 * @HAL_RX_MON_PPDU_END: PPDU end TLV is decided in HAL
 */
enum {
	HAL_RX_MON_PPDU_START = 0,
	HAL_RX_MON_PPDU_END,
};

struct hal_rx_ppdu_user_info {

};

struct hal_rx_ppdu_common_info {
	uint32_t ppdu_id;
	uint32_t ppdu_timestamp;
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
};

struct hal_rx_msdu_payload_info {
	uint8_t *first_msdu_payload;
	uint32_t payload_len;
};

/**
 * struct hal_rx_nac_info - struct for neighbour info
 * @fc_valid: flag indicate if it has valid frame control information
 * @to_ds_flag: flag indicate to_ds bit
 * @mac_addr2_valid: flag indicate if mac_addr2 is valid
 * @mac_addr2: mac address2 in wh
 */
struct hal_rx_nac_info {
	uint8_t fc_valid;
	uint8_t to_ds_flag;
	uint8_t mac_addr2_valid;
	uint8_t mac_addr2[QDF_MAC_ADDR_SIZE];
};

struct hal_rx_ppdu_info {
	struct hal_rx_ppdu_common_info com_info;
	struct hal_rx_ppdu_user_info user_info[HAL_MAX_UL_MU_USERS];
	struct mon_rx_status rx_status;
	struct hal_rx_msdu_payload_info msdu_info;
	struct hal_rx_nac_info nac_info;
	/* status ring PPDU start and end state */
	uint32_t rx_state;
};

static inline uint32_t
hal_get_rx_status_buf_size(void) {
	/* RX status buffer size is hard coded for now */
	return 2048;
}

static inline uint8_t*
hal_rx_status_get_next_tlv(uint8_t *rx_tlv) {
	uint32_t tlv_len, tlv_tag;

	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv);
	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv);

	/* The actual length of PPDU_END is the combined length of many PHY
	 * TLVs that follow. Skip the TLV header and
	 * rx_rxpcu_classification_overview that follows the header to get to
	 * next TLV.
	 */
	if (tlv_tag == WIFIRX_PPDU_END_E)
		tlv_len = sizeof(struct rx_rxpcu_classification_overview);

	return (uint8_t *)(((unsigned long)(rx_tlv + tlv_len +
			HAL_RX_TLV32_HDR_SIZE + 3)) & (~((unsigned long)3)));
}

/**
 * hal_rx_proc_phyrx_other_receive_info_tlv()
 *				    - process other receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: None
 */
static inline void hal_rx_proc_phyrx_other_receive_info_tlv(struct hal_soc *hal_soc,
						     void *rx_tlv_hdr,
						     struct hal_rx_ppdu_info
						     *ppdu_info)
{
	hal_soc->ops->hal_rx_proc_phyrx_other_receive_info_tlv(rx_tlv_hdr,
							(void *)ppdu_info);
}

/**
 * hal_rx_status_get_tlv_info() - process receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: HAL_TLV_STATUS_PPDU_NOT_DONE or HAL_TLV_STATUS_PPDU_DONE from tlv
 */
static inline uint32_t
hal_rx_status_get_tlv_info(void *rx_tlv_hdr, void *ppdu_info,
			   struct hal_soc *hal_soc)
{
	return hal_soc->ops->hal_rx_status_get_tlv_info(rx_tlv_hdr,
							ppdu_info, hal_soc);
}

static inline
uint32_t hal_get_rx_status_done_tlv_size(void *hal_soc)
{
	return HAL_RX_TLV32_HDR_SIZE;
}

static inline QDF_STATUS
hal_get_rx_status_done(uint8_t *rx_tlv)
{
	uint32_t tlv_tag;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv);

	if (tlv_tag == WIFIRX_STATUS_BUFFER_DONE_E)
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_EMPTY;
}

static inline QDF_STATUS
hal_clear_rx_status_done(uint8_t *rx_tlv)
{
	*(uint32_t *)rx_tlv = 0;
	return QDF_STATUS_SUCCESS;
}

#endif

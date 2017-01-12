/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#define HAL_RX_OFFSET(block, field) block##_##field##_OFFSET
#define HAL_RX_LSB(block, field) block##_##field##_LSB
#define HAL_RX_MASk(block, field) block##_##field##_MASK

#define HAL_RX_GET(_ptr, block, field) \
	(((*((volatile uint32_t *)_ptr + (HAL_RX_OFFSET(block, field)>>2))) & \
	HAL_RX_MASk(block, field)) >> \
	HAL_RX_LSB(block, field))

#define HAL_RX_PHY_DATA_RADAR 0x01

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
#define HAL_TLV_STATUS_DUMMY			2

#define HAL_MAX_UL_MU_USERS			8

enum {
	HAL_HW_RX_DECAP_FORMAT_RAW = 0,
	HAL_HW_RX_DECAP_FORMAT_NWIFI,
	HAL_HW_RX_DECAP_FORMAT_ETH2,
	HAL_HW_RX_DECAP_FORMAT_8023,
};

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

static inline
uint32_t
HAL_RX_DESC_GET_DECAP_FORMAT(void *hw_desc_addr) {
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_msdu_start = &rx_desc->msdu_start_tlv.rx_msdu_start;

	return HAL_RX_GET(rx_msdu_start, RX_MSDU_START_2, DECAP_FORMAT);
}

static inline
uint8_t *
HAL_RX_DESC_GET_80211_HDR(void *hw_desc_addr) {
	uint8_t *rx_pkt_hdr;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_pkt_hdr = &rx_desc->pkt_hdr_tlv.rx_pkt_hdr[0];

	return rx_pkt_hdr;
}

static inline
uint32_t HAL_RX_MON_HW_DESC_GET_PPDUID_GET(void *hw_desc_addr)
{
	struct rx_attention *rx_attn;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_attn = &rx_desc->attn_tlv.rx_attn;

	return HAL_RX_GET(rx_attn, RX_ATTENTION_0, PHY_PPDU_ID);
}

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
		"[%s][%d] ReoAddr=%p, addrInfo=%p, paddr=0x%llx, loopcnt=%d\n",
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
		"[%s][%d] src_srng_desc=%p, buf_addr=0x%llx, cookie=0x%llx\n",
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
	HAL_CDP_PKT_TYPE_CCK,
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

static inline
void HAL_RX_MON_HW_DESC_GET_PPDU_START_STATUS(void *hw_desc_addr,
		struct cdp_mon_status *rs)
{
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;
	uint32_t rx_pream_type;
	uint32_t rx_sgi;
	uint32_t rx_type;
	uint32_t rx_bw;
static  uint32_t pkt_type_hw_to_cdp[] = {
			CDP_PKT_TYPE_OFDM,
			CDP_PKT_TYPE_CCK,
			CDP_PKT_TYPE_HT,
			CDP_PKT_TYPE_VHT,
			CDP_PKT_TYPE_HE,
		};

static  uint32_t sgi_hw_to_cdp[] = {
			CDP_SGI_0_8_US,
			CDP_SGI_0_4_US,
			CDP_SGI_1_6_US,
			CDP_SGI_3_2_US,
		};

static uint32_t rx_type_hw_to_cdp[] = {
			CDP_RX_TYPE_SU,
			CDP_RX_TYPE_MU_MIMO,
			CDP_RX_TYPE_MU_OFDMA,
			CDP_RX_TYPE_MU_OFDMA_MIMO,
		};

static uint32_t rx_bw_hw_to_cdp[] = {
			CDP_FULL_RX_BW_20,
			CDP_FULL_RX_BW_40,
			CDP_FULL_RX_BW_80,
			CDP_FULL_RX_BW_160,
		};

	rx_msdu_start = &rx_desc->msdu_start_tlv.rx_msdu_start;

	rs->cdp_rs_tstamp.cdp_tsf = rx_msdu_start->ppdu_start_timestamp;

	rx_pream_type = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, PKT_TYPE);
	rs->cdp_rs_pream_type = pkt_type_hw_to_cdp[rx_pream_type];

	rs->cdp_rs_user_rssi = HAL_RX_GET(rx_msdu_start,
					RX_MSDU_START_5, USER_RSSI);

	rs->cdp_rs_stbc = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, STBC);

	rx_sgi = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, SGI);
	rs->cdp_rs_sgi  = sgi_hw_to_cdp[rx_sgi];

	rs->cdf_rs_rate_mcs = HAL_RX_GET(rx_msdu_start,
					RX_MSDU_START_5, RATE_MCS);

	rx_type = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEPTION_TYPE);

	rs->cdp_rs_reception_type = rx_type_hw_to_cdp[rx_type];

	rx_bw = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEIVE_BANDWIDTH);

	rs->cdp_rs_bw = rx_bw_hw_to_cdp[rx_bw];

	rs->cdp_rs_nss = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, NSS);

}

struct hal_rx_ppdu_user_info {

};

struct hal_rx_ppdu_common_info {
	uint32_t ppdu_id;
	uint32_t ppdu_timestamp;
};

struct hal_rx_ppdu_info {
	struct hal_rx_ppdu_common_info com_info;
	struct hal_rx_ppdu_user_info user_info[HAL_MAX_UL_MU_USERS];
};

static inline uint32_t
hal_get_rx_status_buf_size(void) {
	/* RX status buffer size is hard coded for now */
	return 2048;
}

static inline uint8_t*
hal_rx_status_get_next_tlv(uint8_t *rx_tlv) {
	uint32_t tlv_len;

	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv);

	return (uint8_t *)(((unsigned long)(rx_tlv + tlv_len +
			HAL_RX_TLV32_HDR_SIZE + 3)) & (~((unsigned long)3)));
}

static inline uint32_t
hal_rx_status_get_tlv_info(void *rx_tlv, struct hal_rx_ppdu_info *ppdu_info)
{
	uint32_t tlv_tag, user_id, tlv_len;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv);
	user_id = HAL_RX_GET_USER_TLV32_USERID(rx_tlv);
	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv);

	rx_tlv = (uint8_t *) rx_tlv + HAL_RX_TLV32_HDR_SIZE;

	switch (tlv_tag) {

	case WIFIRX_PPDU_START_E:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"[%s][%d] ppdu_start_e len=%d\n",
					__func__, __LINE__, tlv_len);
		ppdu_info->com_info.ppdu_id =
			HAL_RX_GET(rx_tlv, RX_PPDU_START_0,
				PHY_PPDU_ID);
		ppdu_info->com_info.ppdu_timestamp =
			HAL_RX_GET(rx_tlv, RX_PPDU_START_2,
				PPDU_START_TIMESTAMP);
		break;

	case WIFIRX_PPDU_START_USER_INFO_E:
		break;

	case WIFIRX_PPDU_END_E:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			"[%s][%d] ppdu_end_e len=%d\n",
				__func__, __LINE__, tlv_len);
		break;

	case WIFIRXPCU_PPDU_END_INFO_E:
		break;

	case WIFIRX_PPDU_END_USER_STATS_E:
		break;

	case WIFIRX_PPDU_END_USER_STATS_EXT_E:
		break;

	case WIFIRX_PPDU_END_STATUS_DONE_E:
		return HAL_TLV_STATUS_PPDU_DONE;

	case WIFIDUMMY_E:
		return HAL_TLV_STATUS_DUMMY;

	case 0:
		return HAL_TLV_STATUS_PPDU_DONE;

	default:
		break;
	}

	return HAL_TLV_STATUS_PPDU_NOT_DONE;
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

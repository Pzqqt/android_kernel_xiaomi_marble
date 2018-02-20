/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#define HE_GI_0_8 0
#define HE_GI_1_6 1
#define HE_GI_3_2 2

#define HT_SGI_PRESENT 0x80

#define HE_LTF_1_X 0
#define HE_LTF_2_X 1
#define HE_LTF_4_X 2
#define VHT_SIG_SU_NSS_MASK	0x7

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
	struct rx_mpdu_info *rx_mpdu_info;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_mpdu_info =
		&rx_desc->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;

	return HAL_RX_GET(rx_mpdu_info, RX_MPDU_INFO_0, PHY_PPDU_ID);
}

/* TODO: Move all Rx descriptor functions to hal_rx.h to avoid duplication */
static inline
uint32_t hal_rx_desc_is_first_msdu(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_tlvs = (struct rx_pkt_tlvs *)hw_desc_addr;
	struct rx_msdu_end *msdu_end = &rx_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_GET(msdu_end, RX_MSDU_END_5, FIRST_MSDU);
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
		"[%s][%d] ReoAddr=%pK, addrInfo=%pK, paddr=0x%llx, loopcnt=%d\n",
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
		"[%s][%d] src_srng_desc=%pK, buf_addr=0x%llx, cookie=0x%llx\n",
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
 * hal_rx_mon_hw_desc_get_mpdu_status: Retrieve MPDU status
 *
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
static inline
void hal_rx_mon_hw_desc_get_mpdu_status(void *hw_desc_addr,
		struct mon_rx_status *rs)
{
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;
	uint32_t reg_value;
	static uint32_t sgi_hw_to_cdp[] = {
		CDP_SGI_0_8_US,
		CDP_SGI_0_4_US,
		CDP_SGI_1_6_US,
		CDP_SGI_3_2_US,
	};

	rx_msdu_start = &rx_desc->msdu_start_tlv.rx_msdu_start;
	HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs);

	rs->ant_signal_db = HAL_RX_GET(rx_msdu_start,
					RX_MSDU_START_5, USER_RSSI);
	rs->is_stbc = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, STBC);

	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, SGI);
	rs->sgi = sgi_hw_to_cdp[reg_value];
#if !defined(QCA_WIFI_QCA6290_11AX)
	rs->nr_ant = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, NSS);
#endif

	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEPTION_TYPE);
	rs->beamformed = (reg_value == HAL_RX_RECEPTION_TYPE_MU_MIMO) ? 1 : 0;
	/* TODO: rs->beamformed should be set for SU beamforming also */
}

struct hal_rx_ppdu_user_info {

};

struct hal_rx_ppdu_common_info {
	uint32_t ppdu_id;
	uint32_t last_ppdu_id;
	uint32_t ppdu_timestamp;
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
};

struct hal_rx_ppdu_info {
	struct hal_rx_ppdu_common_info com_info;
	struct hal_rx_ppdu_user_info user_info[HAL_MAX_UL_MU_USERS];
	struct mon_rx_status rx_status;
	uint8_t *first_msdu_payload;
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

	/* The actual length of PPDU_END is the combined lenght of many PHY
	 * TLVs that follow. Skip the TLV header and
	 * rx_rxpcu_classification_overview that follows the header to get to
	 * next TLV.
	 */
	if (tlv_tag == WIFIRX_PPDU_END_E)
		tlv_len = sizeof(struct rx_rxpcu_classification_overview);

	return (uint8_t *)(((unsigned long)(rx_tlv + tlv_len +
			HAL_RX_TLV32_HDR_SIZE + 3)) & (~((unsigned long)3)));
}

static inline uint32_t
hal_rx_status_get_tlv_info(void *rx_tlv, struct hal_rx_ppdu_info *ppdu_info)
{
	uint32_t tlv_tag, user_id, tlv_len, value;
	uint8_t group_id = 0;
	uint8_t he_dcm = 0;
	uint8_t he_stbc = 0;
	uint16_t he_gi = 0;
	uint16_t he_ltf = 0;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv);
	user_id = HAL_RX_GET_USER_TLV32_USERID(rx_tlv);
	tlv_len = HAL_RX_GET_USER_TLV32_LEN(rx_tlv);

	rx_tlv = (uint8_t *) rx_tlv + HAL_RX_TLV32_HDR_SIZE;
	switch (tlv_tag) {

	case WIFIRX_PPDU_START_E:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
				  "[%s][%d] ppdu_start_e len=%d",
					__func__, __LINE__, tlv_len);
		ppdu_info->com_info.ppdu_id =
			HAL_RX_GET(rx_tlv, RX_PPDU_START_0,
				PHY_PPDU_ID);
		/* channel number is set in PHY meta data */
		ppdu_info->rx_status.chan_num =
			HAL_RX_GET(rx_tlv, RX_PPDU_START_1,
				SW_PHY_META_DATA);
		ppdu_info->com_info.ppdu_timestamp =
			HAL_RX_GET(rx_tlv, RX_PPDU_START_2,
				PPDU_START_TIMESTAMP);
		break;

	case WIFIRX_PPDU_START_USER_INFO_E:
		break;

	case WIFIRX_PPDU_END_E:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"[%s][%d] ppdu_end_e len=%d",
				__func__, __LINE__, tlv_len);
		/* This is followed by sub-TLVs of PPDU_END */
		break;

	case WIFIRXPCU_PPDU_END_INFO_E:
		ppdu_info->rx_status.tsft =
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_1,
				WB_TIMESTAMP_UPPER_32);
		ppdu_info->rx_status.tsft = (ppdu_info->rx_status.tsft << 32) |
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_0,
				WB_TIMESTAMP_LOWER_32);
		ppdu_info->rx_status.duration =
			HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_8,
				RX_PPDU_DURATION);
		break;

	case WIFIRX_PPDU_END_USER_STATS_E:
	{
		unsigned long tid = 0;
		uint16_t seq = 0;

		ppdu_info->rx_status.ast_index =
				HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_4,
						AST_INDEX);
		ppdu_info->rx_status.mcs =
			 HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_1, MCS);

		tid = HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_12,
				RECEIVED_QOS_DATA_TID_BITMAP);
		ppdu_info->rx_status.tid = qdf_find_first_bit(&tid, sizeof(tid)*8);

		if (ppdu_info->rx_status.tid == (sizeof(tid) * 8))
			ppdu_info->rx_status.tid = HAL_TID_INVALID;

		ppdu_info->rx_status.tcp_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_9,
					TCP_MSDU_COUNT) +
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_10,
					TCP_ACK_MSDU_COUNT);
		ppdu_info->rx_status.udp_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_9,
						UDP_MSDU_COUNT);
		ppdu_info->rx_status.other_msdu_count =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_10,
					OTHER_MSDU_COUNT);

		ppdu_info->rx_status.frame_control_info_valid =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
					DATA_SEQUENCE_CONTROL_INFO_VALID);

		seq = HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_5,
					FIRST_DATA_SEQ_CTRL);
		if (ppdu_info->rx_status.frame_control_info_valid)
			ppdu_info->rx_status.first_data_seq_ctrl = seq;

		ppdu_info->rx_status.preamble_type =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
						HT_CONTROL_FIELD_PKT_TYPE);
		switch (ppdu_info->rx_status.preamble_type) {
		case HAL_RX_PKT_TYPE_11N:
			ppdu_info->rx_status.ht_flags = 1;
			ppdu_info->rx_status.rtap_flags |= HT_SGI_PRESENT;
			break;
		case HAL_RX_PKT_TYPE_11AC:
			ppdu_info->rx_status.vht_flags = 1;
			break;
		case HAL_RX_PKT_TYPE_11AX:
			ppdu_info->rx_status.he_flags = 1;
			break;
		default:
			break;
		}

		ppdu_info->com_info.mpdu_cnt_fcs_ok =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_3,
					MPDU_CNT_FCS_OK);
		ppdu_info->com_info.mpdu_cnt_fcs_err =
			HAL_RX_GET(rx_tlv, RX_PPDU_END_USER_STATS_2,
					MPDU_CNT_FCS_ERR);
		if ((ppdu_info->com_info.mpdu_cnt_fcs_ok |
			ppdu_info->com_info.mpdu_cnt_fcs_err) > 1)
			ppdu_info->rx_status.rs_flags |= IEEE80211_AMPDU_FLAG;
		else
			ppdu_info->rx_status.rs_flags &=
				(~IEEE80211_AMPDU_FLAG);
		break;
	}

	case WIFIRX_PPDU_END_USER_STATS_EXT_E:
		break;

	case WIFIRX_PPDU_END_STATUS_DONE_E:
		return HAL_TLV_STATUS_PPDU_DONE;

	case WIFIDUMMY_E:
		return HAL_TLV_STATUS_BUF_DONE;

	case WIFIPHYRX_HT_SIG_E:
	{
		uint8_t *ht_sig_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(PHYRX_HT_SIG_0,
				HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS);
		value = HAL_RX_GET(ht_sig_info, HT_SIG_INFO_1,
				FEC_CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		ppdu_info->rx_status.mcs = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO_0, MCS);
		ppdu_info->rx_status.bw = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO_0, CBW);
		ppdu_info->rx_status.sgi = HAL_RX_GET(ht_sig_info,
				HT_SIG_INFO_1, SHORT_GI);
		break;
	}

	case WIFIPHYRX_L_SIG_B_E:
	{
		uint8_t *l_sig_b_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(PHYRX_L_SIG_B_0,
				L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS);

		value = HAL_RX_GET(l_sig_b_info, L_SIG_B_INFO_0, RATE);
		switch (value) {
		case 1:
			ppdu_info->rx_status.rate = HAL_11B_RATE_3MCS;
			break;
		case 2:
			ppdu_info->rx_status.rate = HAL_11B_RATE_2MCS;
			break;
		case 3:
			ppdu_info->rx_status.rate = HAL_11B_RATE_1MCS;
			break;
		case 4:
			ppdu_info->rx_status.rate = HAL_11B_RATE_0MCS;
			break;
		case 5:
			ppdu_info->rx_status.rate = HAL_11B_RATE_6MCS;
			break;
		case 6:
			ppdu_info->rx_status.rate = HAL_11B_RATE_5MCS;
			break;
		case 7:
			ppdu_info->rx_status.rate = HAL_11B_RATE_4MCS;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.cck_flag = 1;
	break;
	}

	case WIFIPHYRX_L_SIG_A_E:
	{
		uint8_t *l_sig_a_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(PHYRX_L_SIG_A_0,
				L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS);

		value = HAL_RX_GET(l_sig_a_info, L_SIG_A_INFO_0, RATE);
		switch (value) {
		case 8:
			ppdu_info->rx_status.rate = HAL_11A_RATE_0MCS;
			break;
		case 9:
			ppdu_info->rx_status.rate = HAL_11A_RATE_1MCS;
			break;
		case 10:
			ppdu_info->rx_status.rate = HAL_11A_RATE_2MCS;
			break;
		case 11:
			ppdu_info->rx_status.rate = HAL_11A_RATE_3MCS;
			break;
		case 12:
			ppdu_info->rx_status.rate = HAL_11A_RATE_4MCS;
			break;
		case 13:
			ppdu_info->rx_status.rate = HAL_11A_RATE_5MCS;
			break;
		case 14:
			ppdu_info->rx_status.rate = HAL_11A_RATE_6MCS;
			break;
		case 15:
			ppdu_info->rx_status.rate = HAL_11A_RATE_7MCS;
			break;
		default:
			break;
		}
		ppdu_info->rx_status.ofdm_flag = 1;
	break;
	}

	case WIFIPHYRX_VHT_SIG_A_E:
	{
		uint8_t *vht_sig_a_info = (uint8_t *)rx_tlv +
				HAL_RX_OFFSET(PHYRX_VHT_SIG_A_0,
				VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS);

		value = HAL_RX_GET(vht_sig_a_info, VHT_SIG_A_INFO_1,
				SU_MU_CODING);
		ppdu_info->rx_status.ldpc = (value == HAL_SU_MU_CODING_LDPC) ?
			1 : 0;
		group_id = HAL_RX_GET(vht_sig_a_info, VHT_SIG_A_INFO_0, GROUP_ID);
		ppdu_info->rx_status.vht_flag_values5 = group_id;
		ppdu_info->rx_status.mcs = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_1, MCS);
		ppdu_info->rx_status.sgi = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_1, GI_SETTING);
#if !defined(QCA_WIFI_QCA6290_11AX)
		value =  HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_0, N_STS);
		ppdu_info->rx_status.nss = ((value & VHT_SIG_SU_NSS_MASK) + 1);
#else
		ppdu_info->rx_status.nss = 0;
#endif
		ppdu_info->rx_status.vht_flag_values3[0] =
				(((ppdu_info->rx_status.mcs) << 4)
				| ppdu_info->rx_status.nss);
		ppdu_info->rx_status.bw = HAL_RX_GET(vht_sig_a_info,
				VHT_SIG_A_INFO_0, BANDWIDTH);
		ppdu_info->rx_status.vht_flag_values2 =
			ppdu_info->rx_status.bw;
		ppdu_info->rx_status.vht_flag_values4 =
			HAL_RX_GET(vht_sig_a_info,
				  VHT_SIG_A_INFO_1, SU_MU_CODING);
		break;
	}
	case WIFIPHYRX_HE_SIG_A_SU_E:
	{
		uint8_t *he_sig_a_su_info = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_A_SU_0,
			HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS);
		ppdu_info->rx_status.he_flags = 1;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0,
			FORMAT_INDICATION);
		if (value == 0) {
			ppdu_info->rx_status.he_data1 =
				QDF_MON_STATUS_HE_TRIG_FORMAT_TYPE;
		} else {
			 ppdu_info->rx_status.he_data1 =
				 QDF_MON_STATUS_HE_SU_OR_EXT_SU_FORMAT_TYPE;
		}

		/*data1*/
		ppdu_info->rx_status.he_data1 |=
			QDF_MON_STATUS_HE_BSS_COLOR_KNOWN |
			QDF_MON_STATUS_HE_BEAM_CHANGE_KNOWN |
			QDF_MON_STATUS_HE_DL_UL_KNOWN |
			QDF_MON_STATUS_HE_MCS_KNOWN |
			QDF_MON_STATUS_HE_DCM_KNOWN |
			QDF_MON_STATUS_HE_CODING_KNOWN |
			QDF_MON_STATUS_HE_LDPC_EXTRA_SYMBOL_KNOWN |
			QDF_MON_STATUS_HE_STBC_KNOWN |
			QDF_MON_STATUS_HE_DATA_BW_RU_KNOWN |
			QDF_MON_STATUS_HE_DOPPLER_KNOWN;

		/*data2*/
		ppdu_info->rx_status.he_data2 =
			QDF_MON_STATUS_HE_GI_KNOWN;
		ppdu_info->rx_status.he_data2 |=
			QDF_MON_STATUS_TXBF_KNOWN |
			QDF_MON_STATUS_PE_DISAMBIGUITY_KNOWN |
			QDF_MON_STATUS_TXOP_KNOWN |
			QDF_MON_STATUS_LTF_SYMBOLS_KNOWN |
			QDF_MON_STATUS_PRE_FEC_PADDING_KNOWN |
			QDF_MON_STATUS_MIDABLE_PERIODICITY_KNOWN;

		/*data3*/
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, BSS_COLOR_ID);
		ppdu_info->rx_status.he_data3 = value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, BEAM_CHANGE);
		value = value << QDF_MON_STATUS_BEAM_CHANGE_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, DL_UL_FLAG);
		value = value << QDF_MON_STATUS_DL_UL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, TRANSMIT_MCS);
		ppdu_info->rx_status.mcs = value;
		value = value << QDF_MON_STATUS_TRANSMIT_MCS_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, DCM);
		he_dcm = value;
		value = value << QDF_MON_STATUS_DCM_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_1, CODING);
		value = value << QDF_MON_STATUS_CODING_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_1,
				LDPC_EXTRA_SYMBOL);
		value = value << QDF_MON_STATUS_LDPC_EXTRA_SYMBOL_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_1, STBC);
		he_stbc = value;
		value = value << QDF_MON_STATUS_STBC_SHIFT;
		ppdu_info->rx_status.he_data3 |= value;

		/*data4*/
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0,
							SPATIAL_REUSE);
		ppdu_info->rx_status.he_data4 = value;

		/*data5*/
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, TRANSMIT_BW);
		ppdu_info->rx_status.he_data5 = value;
		ppdu_info->rx_status.bw = value;
		value = HAL_RX_GET(he_sig_a_su_info,
				HE_SIG_A_SU_INFO_0, CP_LTF_SIZE);
		switch (value) {
		case 0:
				he_gi = HE_GI_0_8;
				he_ltf = HE_LTF_1_X;
				break;
		case 1:
				he_gi = HE_GI_0_8;
				he_ltf = HE_LTF_2_X;
				break;
		case 2:
				he_gi = HE_GI_1_6;
				he_ltf = HE_LTF_2_X;
				break;
		case 3:
				if (he_dcm && he_stbc) {
					he_gi = HE_GI_0_8;
					he_ltf = HE_LTF_4_X;
				} else {
					he_gi = HE_GI_3_2;
					he_ltf = HE_LTF_4_X;
				}
				break;
		}
		ppdu_info->rx_status.sgi = he_gi;
		value = he_gi << QDF_MON_STATUS_GI_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = he_ltf << QDF_MON_STATUS_HE_LTF_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
							PACKET_EXTENSION_A_FACTOR);
		value = value << QDF_MON_STATUS_PRE_FEC_PAD_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1, TXBF);
		value = value << QDF_MON_STATUS_TXBF_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
							PACKET_EXTENSION_PE_DISAMBIGUITY);
		value = value << QDF_MON_STATUS_PE_DISAMBIGUITY_SHIFT;
		ppdu_info->rx_status.he_data5 |= value;

		/*data6*/
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_0, NSTS);
		value++;
		ppdu_info->rx_status.nss = value;
		ppdu_info->rx_status.he_data6 = value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
							DOPPLER_INDICATION);
		value = value << QDF_MON_STATUS_DOPPLER_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;
		value = HAL_RX_GET(he_sig_a_su_info, HE_SIG_A_SU_INFO_1,
							TXOP_DURATION);
		value = value << QDF_MON_STATUS_TXOP_SHIFT;
		ppdu_info->rx_status.he_data6 |= value;

		break;
	}
	case WIFIPHYRX_HE_SIG_A_MU_DL_E:
		ppdu_info->rx_status.he_sig_A1 =
			*((uint32_t *)((uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_A_MU_DL_0,
			HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS)));
		ppdu_info->rx_status.he_sig_A1 |=
			QDF_MON_STATUS_HE_SIG_A1_HE_FORMAT_MU;
		ppdu_info->rx_status.he_sig_A1_known =
			QDF_MON_STATUS_HE_SIG_A1_MU_KNOWN_ALL;

		ppdu_info->rx_status.he_sig_A2 =
			*((uint32_t *)((uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_A_MU_DL_1,
			HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS)));
		ppdu_info->rx_status.he_sig_A2_known =
			QDF_MON_STATUS_HE_SIG_A2_MU_KNOWN_ALL;
		break;
	case WIFIPHYRX_HE_SIG_B1_MU_E:
	{
		uint8_t *he_sig_b1_mu_info = (uint8_t *)rx_tlv +
			*((uint32_t *)((uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_B1_MU_0,
			HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS)));

		ppdu_info->rx_status.he_sig_b_common_RU[0] =
			HAL_RX_GET(he_sig_b1_mu_info, HE_SIG_B1_MU_INFO_0,
				RU_ALLOCATION);

		ppdu_info->rx_status.he_sig_b_common_known =
			QDF_MON_STATUS_HE_SIG_B_COMMON_KNOWN_RU0;
		/* TODO: Check on the availability of other fields in
		 * sig_b_common
		 */
		break;
	}
	case WIFIPHYRX_HE_SIG_B2_MU_E:
		ppdu_info->rx_status.he_sig_b_user =
			*((uint32_t *)((uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_B2_MU_0,
			HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS)));
		ppdu_info->rx_status.he_sig_b_user_known =
			QDF_MON_STATUS_HE_SIG_B_USER_KNOWN_SIG_B_ALL;
		break;
	case WIFIPHYRX_HE_SIG_B2_OFDMA_E:
		ppdu_info->rx_status.he_sig_b_user =
			*((uint32_t *)((uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_HE_SIG_B2_OFDMA_0,
			HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS)));
		ppdu_info->rx_status.he_sig_b_user_known =
			QDF_MON_STATUS_HE_SIG_B_USER_KNOWN_SIG_B_ALL;
		break;
	case WIFIPHYRX_RSSI_LEGACY_E:
	{
		uint8_t *rssi_info_tlv = (uint8_t *)rx_tlv +
			HAL_RX_OFFSET(PHYRX_RSSI_LEGACY_3,
			RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS);

		ppdu_info->rx_status.rssi_comb = HAL_RX_GET(rx_tlv,
			PHYRX_RSSI_LEGACY_35, RSSI_COMB);
		ppdu_info->rx_status.bw = HAL_RX_GET(rx_tlv,
#if !defined(QCA_WIFI_QCA6290_11AX)
			PHYRX_RSSI_LEGACY_35, RECEIVE_BANDWIDTH);
#else
			PHYRX_RSSI_LEGACY_0, RECEIVE_BANDWIDTH);
#endif
		ppdu_info->rx_status.he_re = 0;

		ppdu_info->rx_status.reception_type = HAL_RX_GET(rx_tlv,
				PHYRX_RSSI_LEGACY_0, RECEPTION_TYPE);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_0, RSSI_PRI20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_PRI20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_0, RSSI_EXT20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_0, RSSI_EXT40_LOW20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT40_LOW20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_0, RSSI_EXT40_HIGH20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT40_HIGH20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_1, RSSI_EXT80_LOW20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT80_LOW20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_1, RSSI_EXT80_LOW_HIGH20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT80_LOW_HIGH20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_1, RSSI_EXT80_HIGH_LOW20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT80_HIGH_LOW20_CHAIN0: %d\n", value);

		value = HAL_RX_GET(rssi_info_tlv,
			RECEIVE_RSSI_INFO_1, RSSI_EXT80_HIGH20_CHAIN0);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"RSSI_EXT80_HIGH20_CHAIN0: %d\n", value);
		break;
	}
	case WIFIRX_HEADER_E:
		ppdu_info->first_msdu_payload = rx_tlv;
		break;
	case 0:
		return HAL_TLV_STATUS_PPDU_DONE;

	default:
		break;
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s TLV type: %d, TLV len:%d",
			  __func__, tlv_tag, tlv_len);

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

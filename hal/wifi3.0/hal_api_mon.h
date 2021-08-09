/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
#include "hal_hw_headers.h"
#include <target_type.h>

#define HAL_RX_PHY_DATA_RADAR 0x01
#define HAL_SU_MU_CODING_LDPC 0x01

#define HAL_RX_FCS_LEN (4)
#define KEY_EXTIV 0x20

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

#define HAL_TLV_STATUS_PPDU_NOT_DONE 0
#define HAL_TLV_STATUS_PPDU_DONE 1
#define HAL_TLV_STATUS_BUF_DONE 2
#define HAL_TLV_STATUS_PPDU_NON_STD_DONE 3
#define HAL_TLV_STATUS_PPDU_START 4
#define HAL_TLV_STATUS_HEADER 5
#define HAL_TLV_STATUS_MPDU_END 6
#define HAL_TLV_STATUS_MSDU_START 7
#define HAL_TLV_STATUS_MSDU_END 8

#define HAL_MAX_UL_MU_USERS	37

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

#define HE_GI_RADIOTAP_0_8 0
#define HE_GI_RADIOTAP_1_6 1
#define HE_GI_RADIOTAP_3_2 2
#define HE_GI_RADIOTAP_RESERVED 3

#define HE_LTF_RADIOTAP_UNKNOWN 0
#define HE_LTF_RADIOTAP_1_X 1
#define HE_LTF_RADIOTAP_2_X 2
#define HE_LTF_RADIOTAP_4_X 3

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

#define HAL_RX_SET_MSDU_AGGREGATION((rs_mpdu), (rs_ppdu))\
{\
	if (rs_mpdu->rs_flags & IEEE80211_AMSDU_FLAG)\
		rs_ppdu->rs_flags |= IEEE80211_AMSDU_FLAG;\
} \

#else
#define HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs)
#define HAL_RX_SET_MSDU_AGGREGATION(rs_mpdu, rs_ppdu)
#endif

/* Max MPDUs per status buffer */
#define HAL_RX_MAX_MPDU 256
#define HAL_RX_NUM_WORDS_PER_PPDU_BITMAP (HAL_RX_MAX_MPDU >> 5)
#define HAL_RX_MAX_MPDU_H_PER_STATUS_BUFFER 16

/* Max pilot count */
#define HAL_RX_MAX_SU_EVM_COUNT 32

#define HAL_RX_FRAMECTRL_TYPE_MASK 0x0C
#define HAL_RX_GET_FRAME_CTRL_TYPE(fc)\
		(((fc) & HAL_RX_FRAMECTRL_TYPE_MASK) >> 2)
#define HAL_RX_FRAME_CTRL_TYPE_MGMT 0x0
#define HAL_RX_FRAME_CTRL_TYPE_CTRL 0x1
#define HAL_RX_FRAME_CTRL_TYPE_DATA 0x2

/**
 * struct hal_rx_mon_desc_info () - HAL Rx Monitor descriptor info
 *
 * @ppdu_id:                 PHY ppdu id
 * @status_ppdu_id:          status PHY ppdu id
 * @status_buf_count:        number of status buffer count
 * @rxdma_push_reason:       rxdma push reason
 * @rxdma_error_code:        rxdma error code
 * @msdu_cnt:                msdu count
 * @end_of_ppdu:             end of ppdu
 * @link_desc:               msdu link descriptor address
 * @status_buf:              for a PPDU, status buffers can span acrosss
 *                           multiple buffers, status_buf points to first
 *                           status buffer address of PPDU
 * @drop_ppdu:               flag to indicate current destination
 *                           ring ppdu drop
 */
struct hal_rx_mon_desc_info {
	uint16_t ppdu_id;
	uint16_t status_ppdu_id;
	uint8_t status_buf_count;
	uint8_t rxdma_push_reason;
	uint8_t rxdma_error_code;
	uint8_t msdu_count;
	uint8_t end_of_ppdu;
	struct hal_buf_info link_desc;
	struct hal_buf_info status_buf;
	bool drop_ppdu;
};

/*
 * Struct hal_rx_su_evm_info - SU evm info
 * @number_of_symbols: number of symbols
 * @nss_count:         nss count
 * @pilot_count:       pilot count
 * @pilot_evm:         Array of pilot evm values
 */
struct hal_rx_su_evm_info {
	uint32_t number_of_symbols;
	uint8_t  nss_count;
	uint8_t  pilot_count;
	uint32_t pilot_evm[HAL_RX_MAX_SU_EVM_COUNT];
};

enum {
	DP_PPDU_STATUS_START,
	DP_PPDU_STATUS_DONE,
};

/**
 * hal_rx_reo_ent_buf_paddr_get: Gets the physical address and
 *			cookie from the REO entrance ring element
 * @hal_rx_desc_cookie: Opaque cookie pointer used by HAL to get to
 * the current descriptor
 * @ buf_info: structure to return the buffer information
 * @ msdu_cnt: pointer to msdu count in MPDU
 *
 * CAUTION: This API calls a hal_soc ops, so be careful before calling this in
 * per packet path
 *
 * Return: void
 */
static inline
void hal_rx_reo_ent_buf_paddr_get(hal_soc_handle_t hal_soc_hdl,
				  hal_rxdma_desc_t rx_desc,
				  struct hal_buf_info *buf_info,
				  uint32_t *msdu_cnt)
{
	struct reo_entrance_ring *reo_ent_ring =
		(struct reo_entrance_ring *)rx_desc;
	struct buffer_addr_info *buf_addr_info;
	struct rx_mpdu_desc_info *rx_mpdu_desc_info_details;
	uint32_t loop_cnt;

	rx_mpdu_desc_info_details =
	&reo_ent_ring->reo_level_mpdu_frame_info.rx_mpdu_desc_info_details;

	*msdu_cnt = HAL_RX_GET(rx_mpdu_desc_info_details,
				HAL_RX_MPDU_DESC_INFO, MSDU_COUNT);

	loop_cnt = HAL_RX_GET(reo_ent_ring, HAL_REO_ENTRANCE_RING,
			      LOOPING_COUNT);

	buf_addr_info =
	&reo_ent_ring->reo_level_mpdu_frame_info.msdu_link_desc_addr_info;

	hal_rx_buf_cookie_rbm_get(hal_soc_hdl, (uint32_t *)buf_addr_info,
				  buf_info);
	buf_info->paddr =
		(HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) |
		((uint64_t)
		(HAL_RX_BUFFER_ADDR_39_32_GET(buf_addr_info)) << 32));

	dp_nofl_debug("[%s][%d] ReoAddr=%pK, addrInfo=%pK, paddr=0x%llx, loopcnt=%d",
		      __func__, __LINE__, reo_ent_ring, buf_addr_info,
	(unsigned long long)buf_info->paddr, loop_cnt);
}

static inline
void hal_rx_mon_next_link_desc_get(hal_soc_handle_t hal_soc_hdl,
				   void *rx_msdu_link_desc,
				   struct hal_buf_info *buf_info)
{
	struct rx_msdu_link *msdu_link =
		(struct rx_msdu_link *)rx_msdu_link_desc;
	struct buffer_addr_info *buf_addr_info;

	buf_addr_info = &msdu_link->next_msdu_link_desc_addr_info;

	hal_rx_buf_cookie_rbm_get(hal_soc_hdl, (uint32_t *)buf_addr_info,
				  buf_info);

	buf_info->paddr =
		(HAL_RX_BUFFER_ADDR_31_0_GET(buf_addr_info) |
		((uint64_t)
		(HAL_RX_BUFFER_ADDR_39_32_GET(buf_addr_info)) << 32));
}

static inline
uint8_t *HAL_RX_MON_DEST_GET_DESC(uint8_t *data)
{
	return data;
}

static inline uint32_t
hal_rx_tlv_mpdu_len_err_get(hal_soc_handle_t hal_soc_hdl, void *hw_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_tlv_mpdu_len_err_get)
		return hal_soc->ops->hal_rx_tlv_mpdu_len_err_get(hw_desc_addr);

	return 0;
}

static inline uint32_t
hal_rx_tlv_mpdu_fcs_err_get(hal_soc_handle_t hal_soc_hdl, void *hw_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_tlv_mpdu_fcs_err_get)
		return hal_soc->ops->hal_rx_tlv_mpdu_fcs_err_get(hw_desc_addr);

	return 0;
}

#ifdef notyet
/*
 * HAL_RX_HW_DESC_MPDU_VALID() - check MPDU start TLV tag in MPDU
 *			start TLV of Hardware TLV descriptor
 * @hw_desc_addr: Hardware descriptor address
 *
 * Return: bool: if TLV tag match
 */
static inline
bool HAL_RX_HW_DESC_MPDU_VALID(void *hw_desc_addr)
{
	struct rx_mon_pkt_tlvs *rx_desc =
		(struct rx_mon_pkt_tlvs *)hw_desc_addr;
	uint32_t tlv_tag;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(&rx_desc->mpdu_start_tlv);

	return tlv_tag == WIFIRX_MPDU_START_E ? true : false;
}
#endif

/*
 * HAL_RX_HW_DESC_MPDU_VALID() - check MPDU start TLV user id in MPDU
 *			start TLV of Hardware TLV descriptor
 * @hw_desc_addr: Hardware descriptor address
 *
 * Return: unit32_t: user id
 */
static inline uint32_t
hal_rx_hw_desc_mpdu_user_id(hal_soc_handle_t hal_soc_hdl,
			    void *hw_desc_addr)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		hal_err("hal handle is NULL");
		QDF_BUG(0);
		return 0;
	}

	if (hal_soc->ops->hal_rx_hw_desc_mpdu_user_id)
		return hal_soc->ops->hal_rx_hw_desc_mpdu_user_id(hw_desc_addr);

	return 0;
}

/* TODO: Move all Rx descriptor functions to hal_rx.h to avoid duplication */

/**
 * hal_rx_msdu_link_desc_set: Retrieves MSDU Link Descriptor to WBM
 *
 * @ soc		: HAL version of the SOC pointer
 * @ src_srng_desc	: void pointer to the WBM Release Ring descriptor
 * @ buf_addr_info	: void pointer to the buffer_addr_info
 *
 * Return: void
 */
static inline
void hal_rx_mon_msdu_link_desc_set(hal_soc_handle_t hal_soc_hdl,
				   void *src_srng_desc,
				   hal_buff_addrinfo_t buf_addr_info)
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

	dp_nofl_debug("[%s][%d] src_srng_desc=%pK, buf_addr=0x%llx, cookie=0x%llx",
		      __func__, __LINE__, src_srng_desc, (unsigned long long)paddr,
		      (unsigned long long)p_buffer_addr_info->sw_buffer_cookie);

	/* Structure copy !!! */
	*wbm_srng_buffer_addr_info =
		*((struct buffer_addr_info *)buf_addr_info);
}

/**
 * hal_get_rx_msdu_link_desc_size() - Get msdu link descriptor size
 *
 * Return: size of rx_msdu_link
 */
static inline
uint32_t hal_get_rx_msdu_link_desc_size(void)
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
 * @HAL_RX_MON_PPDU_END: PPDU end TLV is decoded in HAL
 * @HAL_RX_MON_PPDU_RESET: Not PPDU start and end TLV
 */
enum {
	HAL_RX_MON_PPDU_START = 0,
	HAL_RX_MON_PPDU_END,
	HAL_RX_MON_PPDU_RESET,
};

/* struct hal_rx_ppdu_common_info  - common ppdu info
 * @ppdu_id - ppdu id number
 * @ppdu_timestamp - timestamp at ppdu received
 * @mpdu_cnt_fcs_ok - mpdu count in ppdu with fcs ok
 * @mpdu_cnt_fcs_err - mpdu count in ppdu with fcs err
 * @mpdu_fcs_ok_bitmap - fcs ok mpdu count in ppdu bitmap
 * @last_ppdu_id - last received ppdu id
 * @mpdu_cnt - total mpdu count
 * @num_users - num users
 */
struct hal_rx_ppdu_common_info {
	uint32_t ppdu_id;
	uint32_t ppdu_timestamp;
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
	uint32_t mpdu_fcs_ok_bitmap[HAL_RX_NUM_WORDS_PER_PPDU_BITMAP];
	uint32_t last_ppdu_id;
	uint32_t mpdu_cnt;
	uint8_t num_users;
};

/**
 * struct hal_rx_msdu_payload_info - msdu payload info
 * @first_msdu_payload: pointer to first msdu payload
 * @payload_len: payload len
 */
struct hal_rx_msdu_payload_info {
	uint8_t *first_msdu_payload;
	uint32_t payload_len;
};

/**
 * struct hal_rx_nac_info - struct for neighbour info
 * @fc_valid: flag indicate if it has valid frame control information
 * @frame_control: frame control from each MPDU
 * @to_ds_flag: flag indicate to_ds bit
 * @mac_addr2_valid: flag indicate if mac_addr2 is valid
 * @mac_addr2: mac address2 in wh
 * @mcast_bcast: multicast/broadcast
 */
struct hal_rx_nac_info {
	uint8_t fc_valid;
	uint16_t frame_control;
	uint8_t to_ds_flag;
	uint8_t mac_addr2_valid;
	uint8_t mac_addr2[QDF_MAC_ADDR_SIZE];
	uint8_t mcast_bcast;
};

/**
 * struct hal_rx_ppdu_msdu_info - struct for msdu info from HW TLVs
 * @cce_metadata: cached CCE metadata value received in the MSDU_END TLV
 * @is_flow_idx_timeout: flag to indicate if flow search timeout occurred
 * @is_flow_idx_invalid: flag to indicate if flow idx is valid or not
 * @fse_metadata: cached FSE metadata value received in the MSDU END TLV
 * @flow_idx: flow idx matched in FSE received in the MSDU END TLV
 */
struct hal_rx_ppdu_msdu_info {
	uint16_t cce_metadata;
	bool is_flow_idx_timeout;
	bool is_flow_idx_invalid;
	uint32_t fse_metadata;
	uint32_t flow_idx;
};

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
/**
 * struct hal_rx_ppdu_cfr_user_info - struct for storing peer info extracted
 * from HW TLVs, this will be used for correlating CFR data with multiple peers
 * in MU PPDUs
 *
 * @peer_macaddr: macaddr of the peer
 * @ast_index: AST index of the peer
 */
struct hal_rx_ppdu_cfr_user_info {
	uint8_t peer_macaddr[QDF_MAC_ADDR_SIZE];
	uint32_t ast_index;
};

/**
 * struct hal_rx_ppdu_cfr_info - struct for storing ppdu info extracted from HW
 * TLVs, this will be used for CFR correlation
 *
 * @bb_captured_channel : Set by RXPCU when MACRX_FREEZE_CAPTURE_CHANNEL TLV is
 * sent to PHY, SW checks it to correlate current PPDU TLVs with uploaded
 * channel information.
 *
 * @bb_captured_timeout : Set by RxPCU to indicate channel capture condition is
 * met, but MACRX_FREEZE_CAPTURE_CHANNEL is not sent to PHY due to AST delay,
 * which means the rx_frame_falling edge to FREEZE TLV ready time exceeds
 * the threshold time defined by RXPCU register FREEZE_TLV_DELAY_CNT_THRESH.
 * Bb_captured_reason is still valid in this case.
 *
 * @rx_location_info_valid: Indicates whether CFR DMA address in the PPDU TLV
 * is valid
 * <enum 0 rx_location_info_is_not_valid>
 * <enum 1 rx_location_info_is_valid>
 * <legal all>
 *
 * @bb_captured_reason : Copy capture_reason of MACRX_FREEZE_CAPTURE_CHANNEL
 * TLV to here for FW usage. Valid when bb_captured_channel or
 * bb_captured_timeout is set.
 * <enum 0 freeze_reason_TM>
 * <enum 1 freeze_reason_FTM>
 * <enum 2 freeze_reason_ACK_resp_to_TM_FTM>
 * <enum 3 freeze_reason_TA_RA_TYPE_FILTER>
 * <enum 4 freeze_reason_NDPA_NDP>
 * <enum 5 freeze_reason_ALL_PACKET>
 * <legal 0-5>
 *
 * @rtt_che_buffer_pointer_low32 : The low 32 bits of the 40 bits pointer to
 * external RTT channel information buffer
 *
 * @rtt_che_buffer_pointer_high8 : The high 8 bits of the 40 bits pointer to
 * external RTT channel information buffer
 *
 * @chan_capture_status : capture status reported by ucode
 * a. CAPTURE_IDLE: FW has disabled "REPETITIVE_CHE_CAPTURE_CTRL"
 * b. CAPTURE_BUSY: previous PPDU’s channel capture upload DMA ongoing. (Note
 * that this upload is triggered after receiving freeze_channel_capture TLV
 * after last PPDU is rx)
 * c. CAPTURE_ACTIVE: channel capture is enabled and no previous channel
 * capture ongoing
 * d. CAPTURE_NO_BUFFER: next buffer in IPC ring not available
 *
 * @cfr_user_info: Peer mac for upto 4 MU users
 *
 * @rtt_cfo_measurement : raw cfo data extracted from hardware, which is 14 bit
 * signed number. The first bit used for sign representation and 13 bits for
 * fractional part.
 *
 * @agc_gain_info0: Chain 0 & chain 1 agc gain information reported by PHY
 *
 * @agc_gain_info1: Chain 2 & chain 3 agc gain information reported by PHY
 *
 * @agc_gain_info2: Chain 4 & chain 5 agc gain information reported by PHY
 *
 * @agc_gain_info3: Chain 6 & chain 7 agc gain information reported by PHY
 *
 * @rx_start_ts: Rx packet timestamp, the time the first L-STF ADC sample
 * arrived at Rx antenna.
 *
 * @mcs_rate: Indicates the mcs/rate in which packet is received.
 * If HT,
 *    0-7: MCS0-MCS7
 * If VHT,
 *    0-9: MCS0 to MCS9
 * If HE,
 *    0-11: MCS0 to MCS11,
 *    12-13: 4096QAM,
 *    14-15: reserved
 * If Legacy,
 *    0: 48 Mbps
 *    1: 24 Mbps
 *    2: 12 Mbps
 *    3: 6 Mbps
 *    4: 54 Mbps
 *    5: 36 Mbps
 *    6: 18 Mbps
 *    7: 9 Mbps
 *
 * @gi_type: Indicates the gaurd interval.
 *    0: 0.8 us
 *    1: 0.4 us
 *    2: 1.6 us
 *    3: 3.2 us
 */
struct hal_rx_ppdu_cfr_info {
	bool bb_captured_channel;
	bool bb_captured_timeout;
	uint8_t bb_captured_reason;
	bool rx_location_info_valid;
	uint8_t chan_capture_status;
	uint8_t rtt_che_buffer_pointer_high8;
	uint32_t rtt_che_buffer_pointer_low32;
	struct hal_rx_ppdu_cfr_user_info cfr_user_info[HAL_MAX_UL_MU_USERS];
	int16_t rtt_cfo_measurement;
	uint32_t agc_gain_info0;
	uint32_t agc_gain_info1;
	uint32_t agc_gain_info2;
	uint32_t agc_gain_info3;
	uint32_t rx_start_ts;
	uint32_t mcs_rate;
	uint32_t gi_type;
};
#else
struct hal_rx_ppdu_cfr_info {};
#endif

struct mon_rx_info {
	uint8_t  qos_control_info_valid;
	uint16_t qos_control;
	uint8_t mac_addr1_valid;
	uint8_t mac_addr1[QDF_MAC_ADDR_SIZE];
	uint32_t user_id;
};

struct mon_rx_user_info {
	uint16_t qos_control;
	uint8_t qos_control_info_valid;
};

#ifdef QCA_SUPPORT_SCAN_SPCL_VAP_STATS
struct hal_rx_frm_type_info {
	uint32_t rx_mgmt_cnt;
	uint32_t rx_ctrl_cnt;
	uint32_t rx_data_cnt;
};
#else
struct hal_rx_frm_type_info {};
#endif

struct hal_rx_ppdu_info {
	struct hal_rx_ppdu_common_info com_info;
	struct mon_rx_status rx_status;
	struct mon_rx_user_status rx_user_status[HAL_MAX_UL_MU_USERS];
	struct mon_rx_info rx_info;
	struct mon_rx_user_info rx_user_info[HAL_MAX_UL_MU_USERS];
	struct hal_rx_msdu_payload_info msdu_info;
	struct hal_rx_msdu_payload_info fcs_ok_msdu_info;
	struct hal_rx_nac_info nac_info;
	/* status ring PPDU start and end state */
	uint32_t rx_state;
	/* MU user id for status ring TLV */
	uint32_t user_id;
	/* MPDU/MSDU truncated to 128 bytes header start addr in status skb */
	unsigned char *data;
	/* MPDU/MSDU truncated to 128 bytes header real length */
	uint32_t hdr_len;
	/* MPDU FCS error */
	bool fcs_err;
	/* Id to indicate how to process mpdu */
	uint8_t sw_frame_group_id;
	struct hal_rx_ppdu_msdu_info rx_msdu_info[HAL_MAX_UL_MU_USERS];
	/* fcs passed mpdu count in rx monitor status buffer */
	uint8_t fcs_ok_cnt;
	/* fcs error mpdu count in rx monitor status buffer */
	uint8_t fcs_err_cnt;
	/* MPDU FCS passed */
	bool is_fcs_passed;
	/* first msdu payload for all mpdus in rx monitor status buffer */
	struct hal_rx_msdu_payload_info ppdu_msdu_info[HAL_RX_MAX_MPDU_H_PER_STATUS_BUFFER];
	/* evm info */
	struct hal_rx_su_evm_info evm_info;
	/**
	 * Will be used to store ppdu info extracted from HW TLVs,
	 * and for CFR correlation as well
	 */
	struct hal_rx_ppdu_cfr_info cfr_info;
	/* per frame type counts */
	struct hal_rx_frm_type_info frm_type_info;
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
 * @hal_soc: HAL soc handle
 * @nbuf: PPDU status netowrk buffer
 *
 * Return: HAL_TLV_STATUS_PPDU_NOT_DONE or HAL_TLV_STATUS_PPDU_DONE from tlv
 */
static inline uint32_t
hal_rx_status_get_tlv_info(void *rx_tlv_hdr, void *ppdu_info,
			   hal_soc_handle_t hal_soc_hdl,
			   qdf_nbuf_t nbuf)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_rx_status_get_tlv_info(
						rx_tlv_hdr,
						ppdu_info,
						hal_soc_hdl,
						nbuf);
}

static inline
uint32_t hal_get_rx_status_done_tlv_size(hal_soc_handle_t hal_soc_hdl)
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

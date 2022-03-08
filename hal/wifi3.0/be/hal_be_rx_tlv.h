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

#ifndef _HAL_BE_RX_TLV_H_
#define _HAL_BE_RX_TLV_H_

#include "hal_api_mon.h"

/*
 * Structures & Macros to obtain fields from the TLV's in the Rx packet
 * pre-header.
 */

#define HAL_RX_BE_PKT_HDR_TLV_LEN		112

#ifndef CONFIG_WORD_BASED_TLV
typedef struct rx_mpdu_start hal_rx_mpdu_start_t;
typedef struct rx_msdu_end hal_rx_msdu_end_t;
#endif

/*
 * Each RX descriptor TLV is preceded by 1 QWORD "tag"
 */

struct rx_mpdu_start_tlv {
	uint64_t tag;
	hal_rx_mpdu_start_t rx_mpdu_start;
};

struct rx_msdu_end_tlv {
	uint64_t tag;
	hal_rx_msdu_end_t rx_msdu_end;
};

struct rx_pkt_hdr_tlv {
	uint64_t tag;					/* 8 B */
	uint64_t phy_ppdu_id;				/* 8 B */
	char rx_pkt_hdr[HAL_RX_BE_PKT_HDR_TLV_LEN];		/* 112 B */
};

#define RX_BE_PADDING0_BYTES 8
#define RX_BE_PADDING1_BYTES 8

struct rx_pkt_tlvs {
	struct rx_msdu_end_tlv   msdu_end_tlv;	/*  120 bytes */
	uint8_t rx_padding0[RX_BE_PADDING0_BYTES];	/*  8 bytes */
	struct rx_mpdu_start_tlv mpdu_start_tlv;	/*  120 bytes */
	struct rx_pkt_hdr_tlv	 pkt_hdr_tlv;		/* 128 bytes */
};

#define SIZE_OF_DATA_RX_TLV sizeof(struct rx_pkt_tlvs)

#define RX_PKT_TLVS_LEN		SIZE_OF_DATA_RX_TLV

#define RX_PKT_TLV_OFFSET(field) qdf_offsetof(struct rx_pkt_tlvs, field)

#ifndef CONFIG_WORD_BASED_TLV
#define HAL_RX_MSDU_END(_rx_pkt_tlv)		\
	(((struct rx_pkt_tlvs *)_rx_pkt_tlv)->msdu_end_tlv.rx_msdu_end)

#define HAL_RX_MPDU_START(_rx_pkt_tlv)	\
	(((struct rx_pkt_tlvs *)_rx_pkt_tlv)->mpdu_start_tlv.	\
	   rx_mpdu_start.rx_mpdu_info_details)

#else /* CONFIG_WORD_BASED_TLV */
#define HAL_RX_MSDU_END(_rx_pkt_tlv)		\
	(((struct rx_pkt_tlvs *)_rx_pkt_tlv)->msdu_end_tlv.rx_msdu_end)

#define HAL_RX_MPDU_START(_rx_pkt_tlv)	\
	(((struct rx_pkt_tlvs *)_rx_pkt_tlv)->mpdu_start_tlv.	\
			rx_mpdu_start)
#endif	/* CONFIG_WORD_BASED_TLV */

#define HAL_RX_TLV_MSDU_DONE_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).msdu_done

#define HAL_RX_TLV_DECAP_FORMAT_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).decap_format

#ifdef RECEIVE_OFFLOAD
#define HAL_RX_TLV_GET_TCP_PURE_ACK(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).tcp_only_ack

#define HAL_RX_TLV_GET_TCP_PROTO(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).tcp_proto

#define HAL_RX_TLV_GET_UDP_PROTO(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).udp_proto

#define HAL_RX_TLV_GET_IPV6(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).ipv6_proto

#define HAL_RX_TLV_GET_IP_OFFSET(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).l3_offset

#define HAL_RX_TLV_GET_TCP_OFFSET(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).l4_offset
#endif /* RECEIVE_OFFLOAD */

#define HAL_RX_TLV_GET_FLOW_ID_TOEPLITZ(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).flow_id_toeplitz

#define HAL_RX_TLV_MSDU_LEN_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).msdu_length

#define HAL_RX_TLV_CCE_MATCH_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).cce_match

#define HAL_RX_TLV_PHY_PPDU_ID_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).phy_ppdu_id

#define HAL_RX_TLV_BW_GET(_rx_pkt_tlv)     \
	HAL_RX_MSDU_END(_rx_pkt_tlv).receive_bandwidth

#define HAL_RX_TLV_FLOWID_TOEPLITZ_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).flow_id_toeplitz

#define HAL_RX_TLV_SGI_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).sgi

#define HAL_RX_TLV_RATE_MCS_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).rate_mcs

#define HAL_RX_TLV_DECRYPT_STATUS_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).decrypt_status_code

#define HAL_RX_TLV_RSSI_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).user_rssi

#define HAL_RX_TLV_FREQ_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).sw_phy_meta_data

#define HAL_RX_TLV_PKT_TYPE_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).pkt_type

#define HAL_RX_TLV_DECRYPT_ERR_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).decrypt_err

#define HAL_RX_TLV_MIC_ERR_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).tkip_mic_err

#define HAL_RX_TLV_MIMO_SS_BITMAP(_rx_pkt_tlv)\
	HAL_RX_MSDU_END(_rx_pkt_tlv).mimo_ss_bitmap

#define HAL_RX_TLV_ANT_SIGNAL_DB_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).user_rssi

#define HAL_RX_TLV_STBC_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).stbc

#define HAL_RX_TLV_RECEPTION_TYPE_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).reception_type

#define HAL_RX_TLV_IP_CSUM_FAIL_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).ip_chksum_fail

#define HAL_RX_TLV_TCP_UDP_CSUM_FAIL_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).tcp_udp_chksum_fail

#define HAL_RX_TLV_MPDU_LEN_ERR_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).mpdu_length_err

#define HAL_RX_TLV_MPDU_FCS_ERR_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).fcs_err

#define HAL_RX_TLV_IS_MCAST_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).mcast_bcast

#ifndef CONFIG_WORD_BASED_TLV
#define HAL_RX_TLV_FIRST_MPDU_GET(_rx_pkt_tlv)		\
	HAL_RX_MPDU_START(_rx_pkt_tlv).first_mpdu
#else
#define HAL_RX_TLV_FIRST_MPDU_GET(_rx_pkt_tlv)          \
	HAL_RX_MSDU_END(_rx_pkt_tlv).first_mpdu
#endif

#ifdef RECEIVE_OFFLOAD
/**
* LRO information needed from the TLVs
*/
#define HAL_RX_TLV_GET_LRO_ELIGIBLE(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).lro_eligible

#define HAL_RX_TLV_GET_TCP_ACK(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).tcp_ack_number

#define HAL_RX_TLV_GET_TCP_SEQ(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).tcp_seq_number

#define HAL_RX_TLV_GET_TCP_WIN(_rx_pkt_tlv) \
	HAL_RX_MSDU_END(_rx_pkt_tlv).window_size
#endif

#define HAL_RX_GET_FILTER_CATEGORY(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).rxpcu_mpdu_filter_in_category

#define HAL_RX_GET_PPDU_ID(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).phy_ppdu_id

#define HAL_RX_TLV_PEER_META_DATA_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).peer_meta_data

#define HAL_RX_TLV_AMPDU_FLAG_GET(_rx_pkt_tlv)		\
	HAL_RX_MPDU_START(_rx_pkt_tlv).ampdu_flag

#define HAL_RX_TLV_SW_FRAME_GROUP_ID_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).sw_frame_group_id

#define HAL_RX_TLV_KEYID_OCTET_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).key_id_octet

#define HAL_RX_MPDU_SEQUENCE_NUMBER_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_sequence_number

#define HAL_RX_TLV_SA_SW_PEER_ID_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).sa_sw_peer_id

#define HAL_RX_TLV_L3_HEADER_PADDING_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).l3_header_padding

#define HAL_RX_TLV_SA_IDX_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).sa_idx

#define HAL_RX_TLV_DA_IDX_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).da_idx_or_sw_peer_id

#define HAL_RX_TLV_FIRST_MSDU_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).first_msdu

#define HAL_RX_TLV_LAST_MSDU_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).last_msdu

/*
 * Get tid from RX_MPDU_START
 */
#define HAL_RX_TLV_TID_GET(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).tid

#define HAL_RX_TLV_DA_IS_MCBC_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).da_is_mcbc

#define HAL_RX_TLV_SA_IS_VALID_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).sa_is_valid

#define HAL_RX_TLV_MPDU_ENCRYPTION_INFO_VALID(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).frame_encryption_info_valid

#define HAL_RX_TLV_MPDU_PN_31_0_GET(_rx_pkt_tlv)		\
	HAL_RX_MPDU_START(_rx_pkt_tlv).pn_31_0

#define HAL_RX_TLV_MPDU_PN_63_32_GET(_rx_pkt_tlv)		\
	HAL_RX_MPDU_START(_rx_pkt_tlv).pn_63_32

#define HAL_RX_TLV_MPDU_PN_95_64_GET(_rx_pkt_tlv)		\
	HAL_RX_MPDU_START(_rx_pkt_tlv).pn_95_64

#define HAL_RX_TLV_MPDU_PN_127_96_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).pn_127_96

#define HAL_RX_TLV_DA_IS_VALID_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).da_is_valid

#define HAL_RX_TLV_MPDU_MAC_ADDR_AD4_VALID_GET(_rx_pkt_tlv)		\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad4_valid

#define HAL_RX_TLV_SW_PEER_ID_GET(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).sw_peer_id

#define HAL_RX_TLV_MPDU_GET_TODS(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).to_ds

#define HAL_RX_TLV_MPDU_GET_FROMDS(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).fr_ds

#define HAL_RX_TLV_MPDU_GET_FRAME_CONTROL_VALID(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_frame_control_valid

#define HAL_RX_TLV_MPDU_MAC_ADDR_AD1_VALID_GET(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad1_valid

#define HAL_RX_TLV_MPDU_AD1_31_0_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad1_31_0

#define HAL_RX_TLV_MPDU_AD1_47_32_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad1_47_32

#define HAL_RX_TLV_MPDU_MAC_ADDR_AD2_VALID_GET(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad2_valid

#define HAL_RX_TLV_MPDU_AD2_15_0_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad2_15_0

#define HAL_RX_TLV_MPDU_AD2_47_16_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad2_47_16

#define HAL_RX_TLV_MPDU_MAC_ADDR_AD3_VALID_GET(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad3_valid

#define HAL_RX_TLV_MPDU_AD3_31_0_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad3_31_0

#define HAL_RX_TLV_MPDU_AD3_47_32_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad3_47_32

#define HAL_RX_TLV_MPDU_AD4_31_0_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad4_31_0

#define HAL_RX_TLV_MPDU_AD4_47_32_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mac_addr_ad4_47_32

#define HAL_RX_TLV_MPDU_GET_SEQUENCE_CONTROL_VALID(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_sequence_control_valid

#define HAL_RX_TLV_MPDU_QOS_CONTROL_VALID_GET(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_qos_control_valid

#define HAL_RX_TLV_GET_FC_VALID(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_frame_control_valid

#define HAL_RX_TLV_GET_TO_DS_FLAG(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).to_ds

#define HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(_rx_pkt_tlv)	\
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_frame_control_field

#define HAL_RX_TLV_FLOW_IDX_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).flow_idx

#define HAL_RX_TLV_REO_DEST_IND_GET(_rx_pkt_tlv)		\
	HAL_RX_MSDU_END(_rx_pkt_tlv).reo_destination_indication

#define HAL_RX_TLV_FLOW_IDX_INVALID_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).flow_idx_invalid

#define HAL_RX_TLV_FLOW_IDX_TIMEOUT_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).flow_idx_timeout

#define HAL_RX_TLV_FSE_METADATA_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).fse_metadata

#define HAL_RX_TLV_CCE_METADATA_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).cce_metadata

#define HAL_RX_TLV_DECRYPT_STATUS_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).decrypt_status_code

#define HAL_RX_TLV_GET_TCP_CHKSUM(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).tcp_udp_chksum

#define HAL_RX_TLV_GET_FLOW_AGGR_CONT(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).flow_aggregation_continuation

#define HAL_RX_TLV_GET_FLOW_AGGR_COUNT(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).aggregation_count

#define HAL_RX_TLV_GET_FISA_TIMEOUT(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).fisa_timeout

#define HAL_RX_TLV_GET_FISA_CUMULATIVE_L4_CHECKSUM(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).cumulative_l4_checksum

#define HAL_RX_TLV_GET_FISA_CUMULATIVE_IP_LENGTH(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).cumulative_ip_length

#define HAL_RX_MPDU_INFO_QOS_CONTROL_VALID_GET(_rx_pkt_tlv) \
	HAL_RX_MPDU_START(_rx_pkt_tlv).mpdu_qos_control_valid

#define HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(_rx_pkt_tlv)	\
	HAL_RX_MSDU_END(_rx_pkt_tlv).sa_sw_peer_id

/* used by monitor mode for parsing from full TLV */
#define HAL_RX_MON_GET_FC_VALID(_rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO, MPDU_FRAME_CONTROL_VALID)

#define HAL_RX_MON_GET_TO_DS_FLAG(_rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO, TO_DS)

#define HAL_RX_MON_GET_MAC_ADDR2_VALID(_rx_mpdu_start) \
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO, MAC_ADDR_AD2_VALID)

static inline
uint32_t hal_rx_tlv_decap_format_get_be(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs =
			(struct rx_pkt_tlvs *)hw_desc_addr;

	return HAL_RX_TLV_DECAP_FORMAT_GET(rx_pkt_tlvs);
}

static inline uint32_t hal_rx_tlv_msdu_done_get_be(uint8_t *buf)
{
	return HAL_RX_TLV_MSDU_DONE_GET(buf);
}

/*
 * hal_rx_attn_first_mpdu_get(): get fist_mpdu bit from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * return: uint32_t(first_msdu)
 */
static inline uint32_t hal_rx_tlv_first_mpdu_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_FIRST_MPDU_GET(rx_pkt_tlvs);
}

/*
 * hal_rx_msdu_cce_match_get(): get CCE match bit
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 * Return: CCE match value
 */
static inline bool hal_rx_msdu_cce_match_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_CCE_MATCH_GET(rx_pkt_tlvs);
}

/*
 * hal_rx_attn_phy_ppdu_id_get(): get phy_ppdu_id value
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: phy_ppdu_id
 */
static inline uint16_t hal_rx_attn_phy_ppdu_id_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint16_t phy_ppdu_id;

	phy_ppdu_id = HAL_RX_TLV_PHY_PPDU_ID_GET(rx_pkt_tlvs);

	return phy_ppdu_id;
}

/*
 * hal_rx_tlv_phy_ppdu_id_get(): get phy_ppdu_id value
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: phy_ppdu_id
 */
static inline uint16_t hal_rx_tlv_phy_ppdu_id_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_PHY_PPDU_ID_GET(rx_pkt_tlvs);
}

/*
 * hal_rx_mpdu_peer_meta_data_set: set peer meta data in RX mpdu start tlv
 *
 * @buf: rx_tlv_hdr of the received packet
 * @peer_mdata: peer meta data to be set.
 * Return: void
 */
static inline void
hal_rx_mpdu_peer_meta_data_set_be(uint8_t *buf, uint32_t peer_mdata)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	HAL_RX_TLV_PEER_META_DATA_GET(rx_pkt_tlvs) = peer_mdata;
}

/*
 * Get peer_meta_data from RX_MPDU_INFO within RX_MPDU_START
 */

static inline uint32_t hal_rx_mpdu_peer_meta_data_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_PEER_META_DATA_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_mpdu_info_ampdu_flag_get_be(): get ampdu flag bit
 * from rx mpdu info
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: ampdu flag
 */
static inline bool hal_rx_mpdu_info_ampdu_flag_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return !!HAL_RX_TLV_AMPDU_FLAG_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_tlv_msdu_len_get(): API to get the MSDU length
 * from rx_msdu_start TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: msdu length
 */
static inline uint32_t hal_rx_tlv_msdu_len_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_MSDU_LEN_GET(rx_pkt_tlvs);
}

 /**
 * hal_rx_tlv_msdu_len_set(): API to set the MSDU length
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * @len: msdu length
 *
 * Return: none
 */
static inline void hal_rx_tlv_msdu_len_set_be(uint8_t *buf, uint32_t len)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	HAL_RX_TLV_MSDU_LEN_GET(rx_pkt_tlvs) = len;
}

/*
 * hal_rx_tlv_bw_get(): API to get the Bandwidth
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(bw)
 */
static inline uint32_t hal_rx_tlv_bw_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_BW_GET(rx_pkt_tlvs);
}

 /**
 * hal_rx_tlv_toeplitz_get: API to get the toeplitz hash
 * from rx_msdu_start TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: toeplitz hash
 */
static inline uint32_t hal_rx_tlv_toeplitz_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_FLOWID_TOEPLITZ_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_tlv_msdu_sgi_get(): API to get the Short Gaurd
 * Interval from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(sgi)
 */
static inline uint32_t hal_rx_tlv_sgi_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_SGI_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_tlv_msdu_rate_mcs_get(): API to get the MCS rate
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(rate_mcs)
 */
static inline uint32_t hal_rx_tlv_rate_mcs_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t rate_mcs;

	rate_mcs = HAL_RX_TLV_RATE_MCS_GET(rx_pkt_tlvs);

	return rate_mcs;
}

/*
 * Get key index from RX_MSDU_END
 */
/*
 * hal_rx_msdu_get_keyid(): API to get the key id if the decrypted packet
 * from rx_msdu_end
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(key id)
 */

static inline uint8_t hal_rx_msdu_get_keyid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t keyid_octet;

	keyid_octet = HAL_RX_TLV_KEYID_OCTET_GET(rx_pkt_tlvs);

	return keyid_octet & 0x3;
}

/*
 * hal_rx_tlv_get_rssi(): API to get the rssi of received pkt
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(rssi)
 */

static inline uint32_t hal_rx_tlv_get_rssi_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t rssi;

	rssi = HAL_RX_TLV_RSSI_GET(rx_pkt_tlvs);

	return rssi;
}

/*
 * hal_rx_tlv_get_freq(): API to get the frequency of operating channel
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(frequency)
 */

static inline uint32_t hal_rx_tlv_get_freq_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t freq;

	freq = HAL_RX_TLV_FREQ_GET(rx_pkt_tlvs);

	return freq;
}


/*
 * hal_rx_tlv_get_pkt_type(): API to get the pkt type
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(pkt type)
 */

static inline uint32_t hal_rx_tlv_get_pkt_type_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t pkt_type;

	pkt_type = HAL_RX_TLV_PKT_TYPE_GET(rx_pkt_tlvs);

	return pkt_type;
}

/*******************************************************************************
 * RX ERROR APIS
 ******************************************************************************/

/**
 * hal_rx_tlv_decrypt_err_get(): API to get the Decrypt ERR
 * from rx_mpdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(decrypt_err)
 */
static inline uint32_t hal_rx_tlv_decrypt_err_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t decrypt_err;

	decrypt_err = HAL_RX_TLV_DECRYPT_ERR_GET(rx_pkt_tlvs);

	return decrypt_err;
}

/**
 * hal_rx_tlv_mic_err_get(): API to get the MIC ERR
 * from rx_tlv TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(mic_err)
 */
static inline uint32_t hal_rx_tlv_mic_err_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t mic_err;

	mic_err = HAL_RX_TLV_MIC_ERR_GET(rx_pkt_tlvs);

	return mic_err;
}

/**
 * hal_get_reo_ent_desc_qdesc_addr_be(): API to get qdesc address of reo
 * entrance ring desc
 *
 * @desc: reo entrance ring descriptor
 * Return: qdesc adrress
 */
static inline uint8_t *hal_get_reo_ent_desc_qdesc_addr_be(uint8_t *desc)
{
	return desc + REO_ENTRANCE_RING_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET;
}

/**
 * hal_rx_get_qdesc_addr_be(): API to get qdesc address of reo
 * entrance ring desc
 *
 * @dst_ring_desc: reo dest ring descriptor (used for Lithium DP)
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: qdesc adrress in reo destination ring buffer
 */
static inline uint8_t *hal_rx_get_qdesc_addr_be(uint8_t *dst_ring_desc,
						uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return (uint8_t *)(&HAL_RX_MPDU_START(rx_pkt_tlvs) +
			RX_MPDU_INFO_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET);
}

/**
 * hal_set_reo_ent_desc_reo_dest_ind_be(): API to set reo destination
 * indication of reo entrance ring desc
 *
 * @desc: reo ent ring descriptor
 * @dst_ind: reo destination indication value
 * Return: None
 */
static inline void
hal_set_reo_ent_desc_reo_dest_ind_be(uint8_t *desc, uint32_t dst_ind)
{
	HAL_RX_FLD_SET(desc, REO_ENTRANCE_RING,
		       REO_DESTINATION_INDICATION, dst_ind);
}

/**
 * hal_rx_mpdu_sequence_number_get() - Get mpdu sequence number
 * @buf: pointer to packet buffer
 *
 * Return: mpdu sequence
 */
static inline int hal_rx_mpdu_sequence_number_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_MPDU_SEQUENCE_NUMBER_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_msdu_packet_metadata_get(): API to get the
 * msdu information from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * @ hal_rx_msdu_metadata: pointer to the msdu info structure
 */
static inline void
hal_rx_msdu_packet_metadata_get_generic_be(uint8_t *buf,
					   void *pkt_msdu_metadata)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct hal_rx_msdu_metadata *msdu_metadata =
			(struct hal_rx_msdu_metadata *)pkt_msdu_metadata;

	msdu_metadata->l3_hdr_pad =
		HAL_RX_TLV_L3_HEADER_PADDING_GET(rx_pkt_tlvs);
	msdu_metadata->sa_idx = HAL_RX_TLV_SA_IDX_GET(rx_pkt_tlvs);
	msdu_metadata->da_idx = HAL_RX_TLV_DA_IDX_GET(rx_pkt_tlvs);
	msdu_metadata->sa_sw_peer_id =
		HAL_RX_TLV_SA_SW_PEER_ID_GET(rx_pkt_tlvs);
}

/*
 * hal_rx_msdu_start_nss_get_kiwi(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */
static inline uint32_t hal_rx_tlv_nss_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint8_t mimo_ss_bitmap;

	mimo_ss_bitmap = HAL_RX_TLV_MIMO_SS_BITMAP(rx_pkt_tlvs);

	return qdf_get_hweight8(mimo_ss_bitmap);
}

#ifdef GET_MSDU_AGGREGATION
#define HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs)\
{\
	bool first_msdu, last_msdu; \
	first_msdu = HAL_RX_TLV_FIRST_MSDU_GET(rx_desc);\
	last_msdu = HAL_RX_TLV_LAST_MSDU_GET(rx_desc);\
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

/**
 * hal_rx_mon_hw_desc_get_mpdu_status_be(): Retrieve MPDU status
 *
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
static inline void
hal_rx_mon_hw_desc_get_mpdu_status_be(void *hw_desc_addr,
				      struct mon_rx_status *rs)
{
	uint32_t reg_value;
	struct rx_pkt_tlvs *rx_desc =
				(struct rx_pkt_tlvs *)hw_desc_addr;
	const uint32_t sgi_hw_to_cdp[] = {
		CDP_SGI_0_8_US,
		CDP_SGI_0_4_US,
		CDP_SGI_1_6_US,
		CDP_SGI_3_2_US,
	};

	HAL_RX_GET_MSDU_AGGREGATION(rx_desc, rs);

	rs->ant_signal_db = HAL_RX_TLV_ANT_SIGNAL_DB_GET(rx_desc);
	rs->is_stbc = HAL_RX_TLV_STBC_GET(rx_desc);

	reg_value = HAL_RX_TLV_SGI_GET(rx_desc);
	rs->sgi = sgi_hw_to_cdp[reg_value];

	reg_value = HAL_RX_TLV_RECEPTION_TYPE_GET(rx_desc);
	rs->beamformed = (reg_value == HAL_RX_RECEPTION_TYPE_MU_MIMO) ? 1 : 0;
	/* TODO: rs->beamformed should be set for SU beamforming also */
}

static inline uint32_t hal_rx_tlv_tid_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t tid;

	tid = HAL_RX_TLV_TID_GET(rx_pkt_tlvs);

	return tid;
}

/*
 * hal_rx_tlv_reception_type_get(): API to get the reception type
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(reception_type)
 */
static inline
uint32_t hal_rx_tlv_reception_type_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t reception_type;

	reception_type = HAL_RX_TLV_RECEPTION_TYPE_GET(rx_pkt_tlvs);

	return reception_type;
}

/**
 * hal_rx_msdu_end_da_idx_get_be: API to get da_idx
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da index
 */
static inline uint16_t hal_rx_msdu_end_da_idx_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint16_t da_idx;

	da_idx = HAL_RX_TLV_DA_IDX_GET(rx_pkt_tlvs);

	return da_idx;
}

/**
 * hal_rx_get_rx_fragment_number_be(): Function to retrieve rx fragment number
 *
 * @nbuf: Network buffer
 * Returns: rx fragment number
 */
static inline
uint8_t hal_rx_get_rx_fragment_number_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	/* Return first 4 bits as fragment number */
	return (HAL_RX_MPDU_SEQUENCE_NUMBER_GET(rx_pkt_tlvs) &
		DOT11_SEQ_FRAG_MASK);
}

/**
 * hal_rx_msdu_end_da_is_mcbc_get_be(): API to check if pkt is MCBC
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_mcbc
 */
static inline uint8_t
hal_rx_tlv_da_is_mcbc_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_DA_IS_MCBC_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_tlvd_sa_is_valid_get_be(): API to get the sa_is_valid bit from
 *					rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_is_valid bit
 */
static inline uint8_t
hal_rx_tlv_sa_is_valid_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint8_t sa_is_valid;

	sa_is_valid = HAL_RX_TLV_SA_IS_VALID_GET(rx_pkt_tlvs);

	return sa_is_valid;
}

/**
 * hal_rx_tlv_sa_idx_get_be(): API to get the sa_idx from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_idx (SA AST index)
 */
static inline
uint16_t hal_rx_tlv_sa_idx_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint16_t sa_idx;

	sa_idx = HAL_RX_TLV_SA_IDX_GET(rx_pkt_tlvs);

	return sa_idx;
}

/**
 * hal_rx_desc_is_first_msdu_be() - Check if first msdu
 *
 * @hal_soc_hdl: hal_soc handle
 * @hw_desc_addr: hardware descriptor address
 *
 * Return: 0 - success/ non-zero failure
 */
static inline uint32_t hal_rx_desc_is_first_msdu_be(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs =
				(struct rx_pkt_tlvs *)hw_desc_addr;

	return HAL_RX_TLV_FIRST_MSDU_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_tlv_l3_hdr_padding_get_be(): API to get the l3_header padding
 *					from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: number of l3 header padding bytes
 */
static inline uint32_t hal_rx_tlv_l3_hdr_padding_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t l3_header_padding;

	l3_header_padding = HAL_RX_TLV_L3_HEADER_PADDING_GET(rx_pkt_tlvs);

	return l3_header_padding;
}

/*
 * @ hal_rx_encryption_info_valid_be: Returns encryption type.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: encryption type
 */
static inline uint32_t hal_rx_encryption_info_valid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t encryption_info =
			HAL_RX_TLV_MPDU_ENCRYPTION_INFO_VALID(rx_pkt_tlvs);

	return encryption_info;
}

/*
 * @ hal_rx_print_pn_be: Prints the PN of rx packet.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: void
 */
static inline void hal_rx_print_pn_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	uint32_t pn_31_0 = HAL_RX_TLV_MPDU_PN_31_0_GET(rx_pkt_tlvs);
	uint32_t pn_63_32 = HAL_RX_TLV_MPDU_PN_63_32_GET(rx_pkt_tlvs);
	uint32_t pn_95_64 = HAL_RX_TLV_MPDU_PN_95_64_GET(rx_pkt_tlvs);
	uint32_t pn_127_96 = HAL_RX_TLV_MPDU_PN_127_96_GET(rx_pkt_tlvs);

	hal_debug("PN number pn_127_96 0x%x pn_95_64 0x%x pn_63_32 0x%x pn_31_0 0x%x ",
		  pn_127_96, pn_95_64, pn_63_32, pn_31_0);
}

static inline void hal_rx_tlv_get_pn_num_be(uint8_t *buf, uint64_t *pn_num)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	pn_num[0] = HAL_RX_TLV_MPDU_PN_31_0_GET(rx_pkt_tlvs);
	pn_num[0] |= ((uint64_t)HAL_RX_TLV_MPDU_PN_63_32_GET(rx_pkt_tlvs) << 32);

	pn_num[1] = HAL_RX_TLV_MPDU_PN_95_64_GET(rx_pkt_tlvs);
	pn_num[1] |= ((uint64_t)HAL_RX_TLV_MPDU_PN_127_96_GET(rx_pkt_tlvs) << 32);
}

/**
 * hal_rx_tlv_first_msdu_get_be: API to get first msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: first_msdu
 */
static inline uint8_t hal_rx_tlv_first_msdu_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint8_t first_msdu;

	first_msdu = HAL_RX_TLV_FIRST_MSDU_GET(rx_pkt_tlvs);

	return first_msdu;
}

/**
 * hal_rx_tlv_da_is_valid_get_be: API to check if da is valid
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_valid
 */
static inline uint8_t hal_rx_tlv_da_is_valid_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint8_t da_is_valid;

	da_is_valid = HAL_RX_TLV_DA_IS_VALID_GET(rx_pkt_tlvs);

	return da_is_valid;
}

/**
 * hal_rx_tlv_last_msdu_get_be: API to get last msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: last_msdu
 */
static inline uint8_t hal_rx_tlv_last_msdu_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint8_t last_msdu;

	last_msdu = HAL_RX_TLV_LAST_MSDU_GET(rx_pkt_tlvs);

	return last_msdu;
}

/*
 * hal_rx_get_mpdu_mac_ad4_valid_be(): Retrieves if mpdu 4th addr is valid
 *
 * @nbuf: Network buffer
 * Returns: value of mpdu 4th address valid field
 */
static inline bool hal_rx_get_mpdu_mac_ad4_valid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	bool ad4_valid = 0;

	ad4_valid = HAL_RX_TLV_MPDU_MAC_ADDR_AD4_VALID_GET(rx_pkt_tlvs);

	return ad4_valid;
}

/**
 * hal_rx_mpdu_start_sw_peer_id_get_be: Retrieve sw peer_id
 * @buf: network buffer
 *
 * Return: sw peer_id
 */
static inline uint32_t hal_rx_mpdu_start_sw_peer_id_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_SW_PEER_ID_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_mpdu_get_to_ds_be(): API to get the tods info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(to_ds)
 */
static inline uint32_t hal_rx_mpdu_get_to_ds_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_MPDU_GET_TODS(rx_pkt_tlvs);
}

/*
 * hal_rx_mpdu_get_fr_ds_be(): API to get the from ds info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(fr_ds)
 */
static inline uint32_t hal_rx_mpdu_get_fr_ds_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_MPDU_GET_FROMDS(rx_pkt_tlvs);
}

/*
 * hal_rx_get_mpdu_frame_control_valid_be(): Retrieves mpdu
 * frame control valid
 *
 * @nbuf: Network buffer
 * Returns: value of frame control valid field
 */
static inline uint8_t hal_rx_get_mpdu_frame_control_valid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_MPDU_GET_FRAME_CONTROL_VALID(rx_pkt_tlvs);
}

/*
 * hal_rx_mpdu_get_addr1_be(): API to check get address1 of the mpdu
 *
 * @buf: pointer to the start of RX PKT TLV headera
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline QDF_STATUS hal_rx_mpdu_get_addr1_be(uint8_t *buf,
						  uint8_t *mac_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct __attribute__((__packed__)) hal_addr1 {
		uint32_t ad1_31_0;
		uint16_t ad1_47_32;
	};
	struct hal_addr1 *addr = (struct hal_addr1 *)mac_addr;
	uint32_t mac_addr_ad1_valid;

	mac_addr_ad1_valid = HAL_RX_TLV_MPDU_MAC_ADDR_AD1_VALID_GET(rx_pkt_tlvs);

	if (mac_addr_ad1_valid) {
		addr->ad1_31_0 = HAL_RX_TLV_MPDU_AD1_31_0_GET(rx_pkt_tlvs);
		addr->ad1_47_32 = HAL_RX_TLV_MPDU_AD1_47_32_GET(rx_pkt_tlvs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr2_be(): API to check get address2 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline QDF_STATUS hal_rx_mpdu_get_addr2_be(uint8_t *buf,
						  uint8_t *mac_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct __attribute__((__packed__)) hal_addr2 {
		uint16_t ad2_15_0;
		uint32_t ad2_47_16;
	};
	struct hal_addr2 *addr = (struct hal_addr2 *)mac_addr;
	uint32_t mac_addr_ad2_valid;

	mac_addr_ad2_valid = HAL_RX_TLV_MPDU_MAC_ADDR_AD2_VALID_GET(rx_pkt_tlvs);

	if (mac_addr_ad2_valid) {
		addr->ad2_15_0 = HAL_RX_TLV_MPDU_AD2_15_0_GET(rx_pkt_tlvs);
		addr->ad2_47_16 = HAL_RX_TLV_MPDU_AD2_47_16_GET(rx_pkt_tlvs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr3_be(): API to get address3 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline QDF_STATUS hal_rx_mpdu_get_addr3_be(uint8_t *buf,
						  uint8_t *mac_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct __attribute__((__packed__)) hal_addr3 {
		uint32_t ad3_31_0;
		uint16_t ad3_47_32;
	};
	struct hal_addr3 *addr = (struct hal_addr3 *)mac_addr;
	uint32_t mac_addr_ad3_valid;

	mac_addr_ad3_valid = HAL_RX_TLV_MPDU_MAC_ADDR_AD3_VALID_GET(rx_pkt_tlvs);

	if (mac_addr_ad3_valid) {
		addr->ad3_31_0 = HAL_RX_TLV_MPDU_AD3_31_0_GET(rx_pkt_tlvs);
		addr->ad3_47_32 = HAL_RX_TLV_MPDU_AD3_47_32_GET(rx_pkt_tlvs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_mpdu_get_addr4_be(): API to get address4 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static inline QDF_STATUS hal_rx_mpdu_get_addr4_be(uint8_t *buf,
						  uint8_t *mac_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct __attribute__((__packed__)) hal_addr4 {
		uint32_t ad4_31_0;
		uint16_t ad4_47_32;
	};
	struct hal_addr4 *addr = (struct hal_addr4 *)mac_addr;
	uint32_t mac_addr_ad4_valid;

	mac_addr_ad4_valid = HAL_RX_TLV_MPDU_MAC_ADDR_AD4_VALID_GET(rx_pkt_tlvs);

	if (mac_addr_ad4_valid) {
		addr->ad4_31_0 = HAL_RX_TLV_MPDU_AD4_31_0_GET(rx_pkt_tlvs);
		addr->ad4_47_32 = HAL_RX_TLV_MPDU_AD4_47_32_GET(rx_pkt_tlvs);
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

/*
 * hal_rx_get_mpdu_sequence_control_valid_be(): Get mpdu
 * sequence control valid
 *
 * @nbuf: Network buffer
 * Returns: value of sequence control valid field
 */
static inline uint8_t hal_rx_get_mpdu_sequence_control_valid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_MPDU_GET_SEQUENCE_CONTROL_VALID(rx_pkt_tlvs);
}

/**
 * hal_rx_tid_get_be: get tid based on qos control valid.
 * @hal_soc_hdl: hal_soc handle
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: tid
 */
static inline uint32_t hal_rx_tid_get_be(hal_soc_handle_t hal_soc_hdl,
					 uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint8_t qos_control_valid =
			HAL_RX_TLV_MPDU_QOS_CONTROL_VALID_GET(rx_pkt_tlvs);

	if (qos_control_valid)
		return hal_rx_tlv_tid_get_be(buf);

	return HAL_RX_NON_QOS_TID;
}

static inline
uint8_t hal_rx_get_fc_valid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_FC_VALID(rx_pkt_tlvs);
}

static inline uint8_t hal_rx_get_to_ds_flag_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_TO_DS_FLAG(rx_pkt_tlvs);
}

static inline uint8_t hal_rx_get_mac_addr2_valid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_MPDU_MAC_ADDR_AD2_VALID_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_is_unicast_be: check packet is unicast frame or not.
 *
 * @ buf: pointer to rx pkt TLV.
 * Return: true on unicast.
 */
static inline bool hal_rx_is_unicast_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t grp_id;

	grp_id = HAL_RX_TLV_SW_FRAME_GROUP_ID_GET(rx_pkt_tlvs);

	return (HAL_MPDU_SW_FRAME_GROUP_UNICAST_DATA == grp_id) ? true : false;
}

static inline uint8_t hal_rx_get_filter_category_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_GET_FILTER_CATEGORY(rx_pkt_tlvs);
}

/**
 * hal_rx_hw_desc_get_ppduid_get_be(): retrieve ppdu id
 * @rx_tlv_hdr: start address of rx_pkt_tlvs
 * @rxdma_dst_ring_desc: Rx HW descriptor
 *
 * Return: ppdu id
 */
static inline uint32_t
hal_rx_hw_desc_get_ppduid_get_be(void *rx_tlv_hdr, void *rxdma_dst_ring_desc)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs =
					(struct rx_pkt_tlvs *)rx_tlv_hdr;

	return HAL_RX_TLV_PHY_PPDU_ID_GET(rx_pkt_tlvs);
}

static inline uint32_t
hal_rx_get_ppdu_id_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_GET_PPDU_ID(rx_pkt_tlvs);
}

/**
 * hal_rx_msdu_flow_idx_get_be: API to get flow index
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index value from MSDU END TLV
 */
static inline uint32_t hal_rx_msdu_flow_idx_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_FLOW_IDX_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_get_reo_destination_indication_be: API to get
 * reo_destination_indication from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * @reo_destination_indication: pointer to return value of
 *				reo_destination_indication
 *
 * Return: none
 */
static inline void
hal_rx_msdu_get_reo_destination_indication_be(uint8_t *buf,
						uint32_t *reo_destination_indication)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	*reo_destination_indication = HAL_RX_TLV_REO_DEST_IND_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_flow_idx_invalid_be: API to get flow index invalid
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index invalid value from MSDU END TLV
 */
static inline bool hal_rx_msdu_flow_idx_invalid_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_FLOW_IDX_INVALID_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_flow_idx_timeout_be: API to get flow index timeout
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index timeout value from MSDU END TLV
 */
static inline bool hal_rx_msdu_flow_idx_timeout_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_FLOW_IDX_TIMEOUT_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_fse_metadata_get_be: API to get FSE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: fse metadata value from MSDU END TLV
 */
static inline uint32_t hal_rx_msdu_fse_metadata_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_FSE_METADATA_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_cce_metadata_get_be: API to get CCE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: cce_metadata
 */
static inline uint16_t
hal_rx_msdu_cce_metadata_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_CCE_METADATA_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_get_flow_params_be: API to get flow index, flow index invalid
 * and flow index timeout from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * @flow_invalid: pointer to return value of flow_idx_valid
 * @flow_timeout: pointer to return value of flow_idx_timeout
 * @flow_index: pointer to return value of flow_idx
 *
 * Return: none
 */
static inline void
hal_rx_msdu_get_flow_params_be(uint8_t *buf,
				 bool *flow_invalid,
				 bool *flow_timeout,
				 uint32_t *flow_index)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	*flow_invalid = HAL_RX_TLV_FLOW_IDX_INVALID_GET(pkt_tlvs);
	*flow_timeout = HAL_RX_TLV_FLOW_IDX_TIMEOUT_GET(pkt_tlvs);
	*flow_index = HAL_RX_TLV_FLOW_IDX_GET(pkt_tlvs);
}

/**
 * hal_rx_tlv_get_tcp_chksum_be() - API to get tcp checksum
 * @buf: rx_tlv_hdr
 *
 * Return: tcp checksum
 */
static inline uint16_t
hal_rx_tlv_get_tcp_chksum_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_TCP_CHKSUM(rx_pkt_tlvs);
}

/**
 * hal_rx_get_rx_sequence_be(): Function to retrieve rx sequence number
 *
 * @nbuf: Network buffer
 * Returns: rx sequence number
 */
static inline
uint16_t hal_rx_get_rx_sequence_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_MPDU_SEQUENCE_NUMBER_GET(rx_pkt_tlvs);
}

#ifdef RECEIVE_OFFLOAD
#ifdef QCA_WIFI_KIWI_V2
static inline
uint16_t hal_rx_get_fisa_cumulative_l4_checksum_be(uint8_t *buf)
{
	/*
	 * cumulative l4 checksum is not supported in V2 and
	 * cumulative_l4_checksum field is not present
	 */
	return 0;
}
#else
/**
 * hal_rx_get_fisa_cumulative_l4_checksum_be() - Retrieve cumulative
 *                                                 checksum
 * @buf: buffer pointer
 *
 * Return: cumulative checksum
 */
static inline
uint16_t hal_rx_get_fisa_cumulative_l4_checksum_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_FISA_CUMULATIVE_L4_CHECKSUM(rx_pkt_tlvs);
}
#endif

/**
 * hal_rx_get_fisa_cumulative_ip_length_be() - Retrieve cumulative
 *                                               ip length
 * @buf: buffer pointer
 *
 * Return: cumulative length
 */
static inline
uint16_t hal_rx_get_fisa_cumulative_ip_length_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_FISA_CUMULATIVE_IP_LENGTH(rx_pkt_tlvs);
}

/**
 * hal_rx_get_udp_proto_be() - Retrieve udp proto value
 * @buf: buffer
 *
 * Return: udp proto bit
 */
static inline
bool hal_rx_get_udp_proto_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_UDP_PROTO(rx_pkt_tlvs);
}
#endif

/**
 * hal_rx_get_flow_agg_continuation_be() - retrieve flow agg
 *                                           continuation
 * @buf: buffer
 *
 * Return: flow agg
 */
static inline
bool hal_rx_get_flow_agg_continuation_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_FLOW_AGGR_CONT(rx_pkt_tlvs);
}

/**
 * hal_rx_get_flow_agg_count_be()- Retrieve flow agg count
 * @buf: buffer
 *
 * Return: flow agg count
 */
static inline
uint8_t hal_rx_get_flow_agg_count_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_FLOW_AGGR_COUNT(rx_pkt_tlvs);
}

/**
 * hal_rx_get_fisa_timeout_be() - Retrieve fisa timeout
 * @buf: buffer
 *
 * Return: fisa timeout
 */
static inline
bool hal_rx_get_fisa_timeout_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_TLV_GET_FISA_TIMEOUT(rx_pkt_tlvs);
}

/**
 * hal_rx_mpdu_start_tlv_tag_valid_be () - API to check if RX_MPDU_START
 * tlv tag is valid
 *
 *@rx_tlv_hdr: start address of rx_pkt_tlvs
 *
 * Return: true if RX_MPDU_START is valied, else false.
 */
static inline uint8_t hal_rx_mpdu_start_tlv_tag_valid_be(void *rx_tlv_hdr)
{
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)rx_tlv_hdr;
	uint32_t tlv_tag;

	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(&rx_desc->mpdu_start_tlv);

	return tlv_tag == WIFIRX_MPDU_START_E ? true : false;
}

/**
 * hal_rx_msdu_end_offset_get_generic(): API to get the
 * msdu_end structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of msdu_end TLV from structure
 * rx_pkt_tlvs
 */
static inline uint32_t hal_rx_msdu_end_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(msdu_end_tlv);
}

/**
 * hal_rx_mpdu_start_offset_get_generic(): API to get the
 * mpdu_start structure offset rx_pkt_tlv structure
 *
 * NOTE: API returns offset of attn TLV from structure
 * rx_pkt_tlvs
 */
static inline uint32_t hal_rx_mpdu_start_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(mpdu_start_tlv);
}

static inline  uint32_t hal_rx_pkt_tlv_offset_get_generic(void)
{
	return RX_PKT_TLV_OFFSET(pkt_hdr_tlv);
}

#ifdef RECEIVE_OFFLOAD
static inline int
hal_rx_tlv_get_offload_info_be(uint8_t *rx_tlv,
			       struct hal_offload_info *offload_info)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)rx_tlv;

	offload_info->lro_eligible = HAL_RX_TLV_GET_LRO_ELIGIBLE(rx_pkt_tlvs);
	offload_info->flow_id = HAL_RX_TLV_GET_FLOW_ID_TOEPLITZ(rx_pkt_tlvs);
	offload_info->ipv6_proto = HAL_RX_TLV_GET_IPV6(rx_pkt_tlvs);
	offload_info->tcp_proto = HAL_RX_TLV_GET_TCP_PROTO(rx_pkt_tlvs);

	if (offload_info->tcp_proto) {
		offload_info->tcp_pure_ack =
				HAL_RX_TLV_GET_TCP_PURE_ACK(rx_pkt_tlvs);
		offload_info->tcp_offset =
				HAL_RX_TLV_GET_TCP_OFFSET(rx_pkt_tlvs);
		offload_info->tcp_win = HAL_RX_TLV_GET_TCP_WIN(rx_pkt_tlvs);
		offload_info->tcp_seq_num = HAL_RX_TLV_GET_TCP_SEQ(rx_pkt_tlvs);
		offload_info->tcp_ack_num = HAL_RX_TLV_GET_TCP_ACK(rx_pkt_tlvs);
	}
	return 0;
}

static inline int hal_rx_get_proto_params_be(uint8_t *buf, void *proto_params)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct hal_proto_params *param =
				(struct hal_proto_params *)proto_params;

	param->tcp_proto = HAL_RX_TLV_GET_TCP_PROTO(rx_pkt_tlvs);
	param->udp_proto = HAL_RX_TLV_GET_UDP_PROTO(rx_pkt_tlvs);
	param->ipv6_proto = HAL_RX_TLV_GET_IPV6(rx_pkt_tlvs);

	return 0;
}

static inline int hal_rx_get_l3_l4_offsets_be(uint8_t *buf,
					      uint32_t *l3_hdr_offset,
					      uint32_t *l4_hdr_offset)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	*l3_hdr_offset = HAL_RX_TLV_GET_IP_OFFSET(rx_pkt_tlvs);
	*l4_hdr_offset = HAL_RX_TLV_GET_TCP_OFFSET(rx_pkt_tlvs);

	return 0;
}
#endif

/**
* hal_rx_msdu_start_msdu_len_get(): API to get the MSDU length
* from rx_msdu_start TLV
*
* @ buf: pointer to the start of RX PKT TLV headers
* Return: msdu length
*/
static inline uint32_t hal_rx_msdu_start_msdu_len_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t msdu_len;

	msdu_len = HAL_RX_TLV_MSDU_LEN_GET(rx_pkt_tlvs);

	return msdu_len;
}

/**
 * hal_rx_get_frame_ctrl_field(): Function to retrieve frame control field
 *
 * @nbuf: Network buffer
 * Returns: rx more fragment bit
 *
 */
static inline uint16_t hal_rx_get_frame_ctrl_field_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint16_t frame_ctrl = 0;

	frame_ctrl = HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(rx_pkt_tlvs);

	return frame_ctrl;
}

/*
 * hal_rx_tlv_get_is_decrypted_be(): API to get the decrypt status of the
 *  packet from msdu_end
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(decryt status)
 */
static inline uint32_t hal_rx_tlv_get_is_decrypted_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t is_decrypt = 0;
	uint32_t decrypt_status;

	decrypt_status = HAL_RX_TLV_DECRYPT_STATUS_GET(rx_pkt_tlvs);

	if (!decrypt_status)
		is_decrypt = 1;

	return is_decrypt;
}

//TODO -  Currently going with NO-PKT-HDR, need to add pkt hdr tlv and check
static inline uint8_t *hal_rx_pkt_hdr_get_be(uint8_t *buf)
{
	return buf + RX_PKT_TLVS_LEN;
}

/**
 * hal_rx_priv_info_set_in_tlv_be(): Save the private info to
 *				the reserved bytes of rx_tlv_hdr
 * @buf: start of rx_tlv_hdr
 * @priv_data: hal_wbm_err_desc_info structure
 * @len: length of the private data
 * Return: void
 */
static inline void hal_rx_priv_info_set_in_tlv_be(uint8_t *buf,
						  uint8_t *priv_data,
						  uint32_t len)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t copy_len = (len > RX_BE_PADDING0_BYTES) ?
			    RX_BE_PADDING0_BYTES : len;

	qdf_mem_copy(pkt_tlvs->rx_padding0, priv_data, copy_len);
}

/**
 * hal_rx_priv_info_get_from_tlv_be(): retrieve the private data from
 *				the reserved bytes of rx_tlv_hdr.
 * @buf: start of rx_tlv_hdr
 * @priv_data: Handle to get the private data, output parameter.
 * @len: length of the private data
 * Return: void
 */
static inline void hal_rx_priv_info_get_from_tlv_be(uint8_t *buf,
						    uint8_t *priv_data,
						    uint32_t len)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t copy_len = (len > RX_BE_PADDING0_BYTES) ?
			    RX_BE_PADDING0_BYTES : len;

	qdf_mem_copy(priv_data, pkt_tlvs->rx_padding0, copy_len);
}

/**
 * hal_rx_tlv_csum_err_get_be() - Get IP and tcp-udp checksum fail flag
 * @rx_tlv_hdr: start address of rx_tlv_hdr
 * @ip_csum_err: buffer to return ip_csum_fail flag
 * @tcp_udp_csum_fail: placeholder to return tcp-udp checksum fail flag
 *
 * Return: None
 */
static inline void
hal_rx_tlv_csum_err_get_be(uint8_t *rx_tlv_hdr, uint32_t *ip_csum_err,
			   uint32_t *tcp_udp_csum_err)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs =
					(struct rx_pkt_tlvs *)rx_tlv_hdr;

	*ip_csum_err = HAL_RX_TLV_IP_CSUM_FAIL_GET(rx_pkt_tlvs);
	*tcp_udp_csum_err = HAL_RX_TLV_TCP_UDP_CSUM_FAIL_GET(rx_pkt_tlvs);
}

static inline
uint32_t hal_rx_tlv_mpdu_len_err_get_be(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs =
				(struct rx_pkt_tlvs *)hw_desc_addr;

	return HAL_RX_TLV_MPDU_LEN_ERR_GET(rx_pkt_tlvs);
}

static inline
uint32_t hal_rx_tlv_mpdu_fcs_err_get_be(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_pkt_tlvs =
				(struct rx_pkt_tlvs *)hw_desc_addr;

	return HAL_RX_TLV_MPDU_FCS_ERR_GET(rx_pkt_tlvs);
}

/**
 * hal_rx_get_rx_more_frag_bit(): Function to retrieve more fragment bit
 *
 * @nbuf: Network buffer
 * Returns: rx more fragment bit
 */
static inline
uint8_t hal_rx_get_rx_more_frag_bit(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint16_t frame_ctrl = 0;

	frame_ctrl = HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(pkt_tlvs) >>
		DOT11_FC1_MORE_FRAG_OFFSET;

	/* more fragment bit if at offset bit 4 */
	return frame_ctrl;
}

/*
 * hal_rx_msdu_is_wlan_mcast_generic_be(): Check if the buffer is for multicast
 *					address
 * @nbuf: Network buffer
 *
 * Returns: flag to indicate whether the nbuf has MC/BC address
 */
static inline uint32_t hal_rx_msdu_is_wlan_mcast_generic_be(qdf_nbuf_t nbuf)
{
	uint8_t *buf = qdf_nbuf_data(nbuf);

	return HAL_RX_TLV_IS_MCAST_GET(buf);;
}

/**
 * hal_rx_msdu_start_msdu_len_set_be(): API to set the MSDU length
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * @len: msdu length
 *
 * Return: none
 */
static inline void
hal_rx_msdu_start_msdu_len_set_be(uint8_t *buf, uint32_t len)
{
	HAL_RX_TLV_MSDU_LEN_GET(buf) = len;
}

/**
 * hal_rx_mpdu_start_mpdu_qos_control_valid_get_be():
 * Retrieve qos control valid bit from the tlv.
 * @buf: pointer to rx pkt TLV.
 *
 * Return: qos control value.
 */
static inline uint32_t
hal_rx_mpdu_start_mpdu_qos_control_valid_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return HAL_RX_MPDU_INFO_QOS_CONTROL_VALID_GET(pkt_tlvs);
}

/**
 * hal_rx_msdu_end_sa_sw_peer_id_get_be(): API to get the
 * sa_sw_peer_id from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: sa_sw_peer_id index
 */
static inline uint32_t
hal_rx_msdu_end_sa_sw_peer_id_get_be(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	hal_rx_msdu_end_t *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(msdu_end);
}

#endif /* _HAL_BE_RX_TLV_H_ */

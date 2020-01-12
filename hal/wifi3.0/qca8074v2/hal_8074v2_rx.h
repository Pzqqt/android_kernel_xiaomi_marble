/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "cdp_txrx_mon_struct.h"
#include "qdf_trace.h"
#include "hal_rx.h"
#include "hal_tx.h"
#include "dp_types.h"
#include "hal_api_mon.h"
#ifndef QCA_WIFI_QCA6018
#include "phyrx_other_receive_info_su_evm_details.h"
#endif

#define HAL_RX_MPDU_GET_SEQUENCE_NUMBER(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_NUMBER_OFFSET)),	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_NUMBER_MASK,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_NUMBER_LSB))

#define HAL_RX_MSDU_END_DA_IS_MCBC_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_DA_IS_MCBC_OFFSET)),	\
		RX_MSDU_END_5_DA_IS_MCBC_MASK,		\
		RX_MSDU_END_5_DA_IS_MCBC_LSB))

#define HAL_RX_MSDU_END_SA_IS_VALID_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_SA_IS_VALID_OFFSET)),	\
		RX_MSDU_END_5_SA_IS_VALID_MASK,		\
		RX_MSDU_END_5_SA_IS_VALID_LSB))

#define HAL_RX_MSDU_END_SA_IDX_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_13_SA_IDX_OFFSET)),	\
		RX_MSDU_END_13_SA_IDX_MASK,		\
		RX_MSDU_END_13_SA_IDX_LSB))

#define HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_5_L3_HEADER_PADDING_OFFSET)),	\
		RX_MSDU_END_5_L3_HEADER_PADDING_MASK,		\
		RX_MSDU_END_5_L3_HEADER_PADDING_LSB))

#define HAL_RX_MPDU_ENCRYPTION_INFO_VALID(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,		\
	RX_MPDU_INFO_2_FRAME_ENCRYPTION_INFO_VALID_OFFSET)),	\
	RX_MPDU_INFO_2_FRAME_ENCRYPTION_INFO_VALID_MASK,	\
	RX_MPDU_INFO_2_FRAME_ENCRYPTION_INFO_VALID_LSB))

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

#define HAL_RX_MSDU_END_FIRST_MSDU_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_FIRST_MSDU_OFFSET)),	\
		RX_MSDU_END_5_FIRST_MSDU_MASK,		\
		RX_MSDU_END_5_FIRST_MSDU_LSB))

#define HAL_RX_MSDU_START_MIMO_SS_BITMAP(_rx_msdu_start)\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_msdu_start),\
	RX_MSDU_START_5_MIMO_SS_BITMAP_OFFSET)),	\
	RX_MSDU_START_5_MIMO_SS_BITMAP_MASK,		\
	RX_MSDU_START_5_MIMO_SS_BITMAP_LSB))

#define HAL_RX_MSDU_END_DA_IS_VALID_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_DA_IS_VALID_OFFSET)),	\
		RX_MSDU_END_5_DA_IS_VALID_MASK,		\
		RX_MSDU_END_5_DA_IS_VALID_LSB))

#define HAL_RX_MSDU_END_LAST_MSDU_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_5_LAST_MSDU_OFFSET)),	\
		RX_MSDU_END_5_LAST_MSDU_MASK,		\
		RX_MSDU_END_5_LAST_MSDU_LSB))

#define HAL_RX_MPDU_GET_MAC_AD4_VALID(_rx_mpdu_info)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,		\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_OFFSET)),	\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_MASK,		\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_LSB))

#define HAL_RX_MPDU_INFO_SW_PEER_ID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),	\
		RX_MPDU_INFO_1_SW_PEER_ID_OFFSET)),	\
		RX_MPDU_INFO_1_SW_PEER_ID_MASK,		\
		RX_MPDU_INFO_1_SW_PEER_ID_LSB))

#define HAL_RX_MPDU_GET_TODS(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_TO_DS_OFFSET)),	\
		RX_MPDU_INFO_2_TO_DS_MASK,	\
		RX_MPDU_INFO_2_TO_DS_LSB))

#define HAL_RX_MPDU_GET_FROMDS(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_FR_DS_OFFSET)),	\
		RX_MPDU_INFO_2_FR_DS_MASK,	\
		RX_MPDU_INFO_2_FR_DS_LSB))

#define HAL_RX_MPDU_GET_FRAME_CONTROL_VALID(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_MPDU_FRAME_CONTROL_VALID_OFFSET)),	\
		RX_MPDU_INFO_2_MPDU_FRAME_CONTROL_VALID_MASK,	\
		RX_MPDU_INFO_2_MPDU_FRAME_CONTROL_VALID_LSB))

#define HAL_RX_MPDU_MAC_ADDR_AD1_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD1_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD1_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD1_VALID_LSB))

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

#define HAL_RX_MPDU_MAC_ADDR_AD2_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD2_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD2_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD2_VALID_LSB))

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

#define HAL_RX_MPDU_MAC_ADDR_AD3_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD3_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD3_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD3_VALID_LSB))

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

#define HAL_RX_MPDU_MAC_ADDR_AD4_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info, \
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_OFFSET)), \
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_MASK,	\
		RX_MPDU_INFO_2_MAC_ADDR_AD4_VALID_LSB))

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

#define HAL_RX_MPDU_GET_SEQUENCE_CONTROL_VALID(_rx_mpdu_info)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_mpdu_info,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_OFFSET)),	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_MASK,	\
		RX_MPDU_INFO_2_MPDU_SEQUENCE_CONTROL_VALID_LSB))

#define HAL_RX_MPDU_INFO_QOS_CONTROL_VALID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),		\
		RX_MPDU_INFO_2_MPDU_QOS_CONTROL_VALID_OFFSET)),		\
		RX_MPDU_INFO_2_MPDU_QOS_CONTROL_VALID_MASK,		\
		RX_MPDU_INFO_2_MPDU_QOS_CONTROL_VALID_LSB))

#define HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(_rx_msdu_end)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_16_SA_SW_PEER_ID_OFFSET)),		\
		RX_MSDU_END_16_SA_SW_PEER_ID_MASK,		\
		RX_MSDU_END_16_SA_SW_PEER_ID_LSB))

#define HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va)      \
	(uint8_t *)(link_desc_va) +			\
	RX_MSDU_LINK_8_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET

#define HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0)			\
	(uint8_t *)(msdu0) +				\
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET

#define HAL_ENT_MPDU_DESC_INFO(ent_ring_desc)		\
	(uint8_t *)(ent_ring_desc) +			\
	RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET

#define HAL_DST_MPDU_DESC_INFO(dst_ring_desc)		\
	(uint8_t *)(dst_ring_desc) +			\
	REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET

#define HAL_RX_GET_FC_VALID(rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_2, MPDU_FRAME_CONTROL_VALID)

#define HAL_RX_GET_TO_DS_FLAG(rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_2, TO_DS)

#define HAL_RX_GET_MAC_ADDR2_VALID(rx_mpdu_start) \
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_2, MAC_ADDR_AD2_VALID)

#define HAL_RX_GET_FILTER_CATEGORY(rx_mpdu_start) \
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_0, RXPCU_MPDU_FILTER_IN_CATEGORY)

#define HAL_RX_GET_PPDU_ID(rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_0, PHY_PPDU_ID)

#define HAL_RX_GET_SW_FRAME_GROUP_ID(rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO_0, SW_FRAME_GROUP_ID)

#define HAL_REO_R0_CONFIG(soc, reg_val, reo_params)		\
	do { \
		reg_val &= \
			~(HWIO_REO_R0_GENERAL_ENABLE_FRAGMENT_DEST_RING_BMSK |\
			HWIO_REO_R0_GENERAL_ENABLE_AGING_LIST_ENABLE_BMSK | \
			HWIO_REO_R0_GENERAL_ENABLE_AGING_FLUSH_ENABLE_BMSK); \
		reg_val |= \
			HAL_SM(HWIO_REO_R0_GENERAL_ENABLE, \
			       FRAGMENT_DEST_RING, \
			       (reo_params)->frag_dst_ring) |	\
			HAL_SM(HWIO_REO_R0_GENERAL_ENABLE, \
			       AGING_LIST_ENABLE, 1) |\
			HAL_SM(HWIO_REO_R0_GENERAL_ENABLE, \
			       AGING_FLUSH_ENABLE, 1);\
		HAL_REG_WRITE((soc), \
			      HWIO_REO_R0_GENERAL_ENABLE_ADDR( \
			      SEQ_WCSS_UMAC_REO_REG_OFFSET), \
			      (reg_val)); \
	} while (0)

#define HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr) \
	((struct rx_msdu_desc_info *) \
	_OFFSET_TO_BYTE_PTR((msdu_details_ptr), \
UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET))

#define HAL_RX_LINK_DESC_MSDU0_PTR(link_desc)   \
	((struct rx_msdu_details *) \
	 _OFFSET_TO_BYTE_PTR((link_desc),\
	UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET))

#define HAL_RX_MSDU_END_FLOW_IDX_GET(_rx_msdu_end)  \
		(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,  \
		RX_MSDU_END_14_FLOW_IDX_OFFSET)),  \
		RX_MSDU_END_14_FLOW_IDX_MASK,    \
		RX_MSDU_END_14_FLOW_IDX_LSB))

#define HAL_RX_MSDU_END_FLOW_IDX_INVALID_GET(_rx_msdu_end)  \
		(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,  \
		RX_MSDU_END_5_FLOW_IDX_INVALID_OFFSET)),  \
		RX_MSDU_END_5_FLOW_IDX_INVALID_MASK,    \
		RX_MSDU_END_5_FLOW_IDX_INVALID_LSB))

#define HAL_RX_MSDU_END_FLOW_IDX_TIMEOUT_GET(_rx_msdu_end)  \
		(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,  \
		RX_MSDU_END_5_FLOW_IDX_TIMEOUT_OFFSET)),  \
		RX_MSDU_END_5_FLOW_IDX_TIMEOUT_MASK,    \
		RX_MSDU_END_5_FLOW_IDX_TIMEOUT_LSB))

#define HAL_RX_MSDU_END_FSE_METADATA_GET(_rx_msdu_end)  \
		(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,  \
		RX_MSDU_END_15_FSE_METADATA_OFFSET)),  \
		RX_MSDU_END_15_FSE_METADATA_MASK,    \
		RX_MSDU_END_15_FSE_METADATA_LSB))

#define HAL_RX_MSDU_END_CCE_METADATA_GET(_rx_msdu_end)	\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,	\
		RX_MSDU_END_16_CCE_METADATA_OFFSET)),	\
		RX_MSDU_END_16_CCE_METADATA_MASK,	\
		RX_MSDU_END_16_CCE_METADATA_LSB))

#define HAL_RX_TLV_GET_TCP_CHKSUM(buf) \
	(_HAL_MS( \
		 (*_OFFSET_TO_WORD_PTR(&(((struct rx_pkt_tlvs *)(buf))->\
			 msdu_end_tlv.rx_msdu_end), \
			 RX_MSDU_END_1_TCP_UDP_CHKSUM_OFFSET)), \
		RX_MSDU_END_1_TCP_UDP_CHKSUM_MASK, \
		RX_MSDU_END_1_TCP_UDP_CHKSUM_LSB))

/*
 * hal_rx_msdu_start_nss_get_8074v2(): API to get the NSS
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(nss)
 */
static uint32_t hal_rx_msdu_start_nss_get_8074v2(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint8_t mimo_ss_bitmap;

	mimo_ss_bitmap = HAL_RX_MSDU_START_MIMO_SS_BITMAP(msdu_start);

	return qdf_get_hweight8(mimo_ss_bitmap);
}

/**
 * hal_rx_mon_hw_desc_get_mpdu_status_8074v2(): Retrieve MPDU status
 *
 * @ hw_desc_addr: Start address of Rx HW TLVs
 * @ rs: Status for monitor mode
 *
 * Return: void
 */
static void hal_rx_mon_hw_desc_get_mpdu_status_8074v2(void *hw_desc_addr,
						    struct mon_rx_status *rs)
{
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;
	uint32_t reg_value;
	const uint32_t sgi_hw_to_cdp[] = {
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
	reg_value = HAL_RX_GET(rx_msdu_start, RX_MSDU_START_5, RECEPTION_TYPE);
	rs->beamformed = (reg_value == HAL_RX_RECEPTION_TYPE_MU_MIMO) ? 1 : 0;
	/* TODO: rs->beamformed should be set for SU beamforming also */
}

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)
static uint32_t hal_get_link_desc_size_8074v2(void)
{
	return LINK_DESC_SIZE;
}

/*
 * hal_rx_get_tlv_8074v2(): API to get the tlv
 *
 * @rx_tlv: TLV data extracted from the rx packet
 * Return: uint8_t
 */
static uint8_t hal_rx_get_tlv_8074v2(void *rx_tlv)
{
	return HAL_RX_GET(rx_tlv, PHYRX_RSSI_LEGACY_0, RECEIVE_BANDWIDTH);
}

#ifndef QCA_WIFI_QCA6018
#define HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, evm, pilot) \
	(ppdu_info)->evm_info.pilot_evm[pilot] = HAL_RX_GET(rx_tlv, \
				PHYRX_OTHER_RECEIVE_INFO, \
				SU_EVM_DETAILS_##evm##_PILOT_##pilot##_EVM)

static inline void
hal_rx_update_su_evm_info(void *rx_tlv,
			  void *ppdu_info_hdl)
{
	struct hal_rx_ppdu_info *ppdu_info =
			(struct hal_rx_ppdu_info *)ppdu_info_hdl;

	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 1, 0);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 2, 1);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 3, 2);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 4, 3);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 5, 4);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 6, 5);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 7, 6);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 8, 7);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 9, 8);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 10, 9);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 11, 10);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 12, 11);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 13, 12);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 14, 13);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 15, 14);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 16, 15);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 17, 16);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 18, 17);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 19, 18);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 20, 19);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 21, 20);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 22, 21);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 23, 22);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 24, 23);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 25, 24);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 26, 25);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 27, 26);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 28, 27);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 29, 28);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 30, 29);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 31, 30);
	HAL_RX_UPDATE_SU_EVM_INFO(rx_tlv, ppdu_info, 32, 31);
}
/**
 * hal_rx_proc_phyrx_other_receive_info_tlv_8074v2()
 *				      -process other receive info TLV
 * @rx_tlv_hdr: pointer to TLV header
 * @ppdu_info: pointer to ppdu_info
 *
 * Return: None
 */
static
void hal_rx_proc_phyrx_other_receive_info_tlv_8074v2(void *rx_tlv_hdr,
						     void *ppdu_info_hdl)
{
	uint16_t tlv_tag;
	void *rx_tlv;
	struct hal_rx_ppdu_info *ppdu_info  = ppdu_info_hdl;

	/* Skip TLV_HDR for OTHER_RECEIVE_INFO and follows the
	 * embedded TLVs inside
	 */
	rx_tlv = (uint8_t *)rx_tlv_hdr + HAL_RX_TLV32_HDR_SIZE;
	tlv_tag = HAL_RX_GET_USER_TLV32_TYPE(rx_tlv);

	switch (tlv_tag) {
	case WIFIPHYRX_OTHER_RECEIVE_INFO_SU_EVM_DETAILS_E:

		/* Skip TLV length to get TLV content */
		rx_tlv = (uint8_t *)rx_tlv + HAL_RX_TLV32_HDR_SIZE;

		ppdu_info->evm_info.number_of_symbols = HAL_RX_GET(rx_tlv,
				PHYRX_OTHER_RECEIVE_INFO,
				SU_EVM_DETAILS_0_NUMBER_OF_SYMBOLS);
		ppdu_info->evm_info.pilot_count = HAL_RX_GET(rx_tlv,
				PHYRX_OTHER_RECEIVE_INFO,
				SU_EVM_DETAILS_0_PILOT_COUNT);
		ppdu_info->evm_info.nss_count = HAL_RX_GET(rx_tlv,
				PHYRX_OTHER_RECEIVE_INFO,
				SU_EVM_DETAILS_0_NSS_COUNT);
		hal_rx_update_su_evm_info(rx_tlv, ppdu_info_hdl);
	break;
	}
}
#else
static inline
void hal_rx_proc_phyrx_other_receive_info_tlv_8074v2(void *rx_tlv_hdr,
						     void *ppdu_info_hdl)
{
}
#endif

#if defined(QCA_WIFI_QCA6018) && defined(WLAN_CFR_ENABLE) && \
	defined(WLAN_ENH_CFR_ENABLE)
static inline
void hal_rx_get_bb_info_8074v2(void *rx_tlv,
			       void *ppdu_info_hdl)
{
	struct hal_rx_ppdu_info *ppdu_info  = ppdu_info_hdl;

	ppdu_info->cfr_info.bb_captured_channel =
	  HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_3, BB_CAPTURED_CHANNEL);

	ppdu_info->cfr_info.bb_captured_timeout =
	  HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_3, BB_CAPTURED_TIMEOUT);

	ppdu_info->cfr_info.bb_captured_reason =
	  HAL_RX_GET(rx_tlv, RXPCU_PPDU_END_INFO_3, BB_CAPTURED_REASON);
}

static inline
void hal_rx_get_rtt_info_8074v2(void *rx_tlv,
				void *ppdu_info_hdl)
{
	struct hal_rx_ppdu_info *ppdu_info  = ppdu_info_hdl;

	ppdu_info->cfr_info.rx_location_info_valid =
		HAL_RX_GET(rx_tlv, PHYRX_PKT_END_13_RX_PKT_END_DETAILS,
			   RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID);

	ppdu_info->cfr_info.rtt_che_buffer_pointer_low32 =
	HAL_RX_GET(rx_tlv,
		   PHYRX_PKT_END_12_RX_PKT_END_DETAILS_RX_LOCATION_INFO_DETAILS,
		   RTT_CHE_BUFFER_POINTER_LOW32);

	ppdu_info->cfr_info.rtt_che_buffer_pointer_high8 =
	HAL_RX_GET(rx_tlv,
		   PHYRX_PKT_END_11_RX_PKT_END_DETAILS_RX_LOCATION_INFO_DETAILS,
		   RTT_CHE_BUFFER_POINTER_HIGH8);

	ppdu_info->cfr_info.chan_capture_status =
	HAL_RX_GET(rx_tlv,
		   PHYRX_PKT_END_13_RX_PKT_END_DETAILS_RX_LOCATION_INFO_DETAILS,
		   RESERVED_8);
}
#endif

/**
 * hal_rx_dump_msdu_start_tlv_8074v2() : dump RX msdu_start TLV in structured
 *			     human readable format.
 * @ msdu_start: pointer the msdu_start TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_start_tlv_8074v2(void *msdustart,
					    uint8_t dbg_level)
{
	struct rx_msdu_start *msdu_start = (struct rx_msdu_start *)msdustart;

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
			msdu_start->ppdu_start_timestamp,
			msdu_start->sw_phy_meta_data);
}

/**
 * hal_rx_dump_msdu_end_tlv_8074v2: dump RX msdu_end TLV in structured
 *			     human readable format.
 * @ msdu_end: pointer the msdu_end TLV in pkt.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_msdu_end_tlv_8074v2(void *msduend,
					  uint8_t dbg_level)
{
	struct rx_msdu_end *msdu_end = (struct rx_msdu_end *)msduend;

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
			msdu_end->msdu_drop,
			msdu_end->reo_destination_indication,
			msdu_end->flow_idx,
			msdu_end->fse_metadata,
			msdu_end->cce_metadata,
			msdu_end->sa_sw_peer_id);
}


/*
 * Get tid from RX_MPDU_START
 */
#define HAL_RX_MPDU_INFO_TID_GET(_rx_mpdu_info) \
	(_HAL_MS((*_OFFSET_TO_WORD_PTR((_rx_mpdu_info),	\
		RX_MPDU_INFO_3_TID_OFFSET)),		\
		RX_MPDU_INFO_3_TID_MASK,		\
		RX_MPDU_INFO_3_TID_LSB))

static uint32_t hal_rx_mpdu_start_tid_get_8074v2(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t tid;

	tid = HAL_RX_MPDU_INFO_TID_GET(&mpdu_start->rx_mpdu_info_details);

	return tid;
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
static uint32_t hal_rx_msdu_start_reception_type_get_8074v2(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t reception_type;

	reception_type = HAL_RX_MSDU_START_RECEPTION_TYPE_GET(msdu_start);

	return reception_type;
}

/* RX_MSDU_END_13_DA_IDX_OR_SW_PEER_ID_OFFSET */
#define HAL_RX_MSDU_END_DA_IDX_GET(_rx_msdu_end)		\
	(_HAL_MS((*_OFFSET_TO_WORD_PTR(_rx_msdu_end,		\
		RX_MSDU_END_13_DA_IDX_OR_SW_PEER_ID_OFFSET)),	\
		RX_MSDU_END_13_DA_IDX_OR_SW_PEER_ID_MASK,	\
		RX_MSDU_END_13_DA_IDX_OR_SW_PEER_ID_LSB))
 /**
 * hal_rx_msdu_end_da_idx_get_8074v2: API to get da_idx
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da index
 */
static uint16_t hal_rx_msdu_end_da_idx_get_8074v2(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t da_idx;

	da_idx = HAL_RX_MSDU_END_DA_IDX_GET(msdu_end);

	return da_idx;
}

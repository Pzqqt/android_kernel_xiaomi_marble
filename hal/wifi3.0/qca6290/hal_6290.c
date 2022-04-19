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
#include "qdf_types.h"
#include "qdf_util.h"
#include "qdf_types.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "hal_li_hw_headers.h"
#include "hal_internal.h"
#include "hal_api.h"
#include "target_type.h"
#include "wcss_version.h"
#include "qdf_module.h"

#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_OFFSET \
	RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_OFFSET
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_MASK \
	RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_MASK
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_LSB \
	RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_LSB
#define UNIFIED_PHYRX_HT_SIG_0_HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS_OFFSET \
	PHYRX_HT_SIG_0_HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_L_SIG_B_0_L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_0_L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_L_SIG_A_0_L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_A_0_L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_VHT_SIG_A_0_VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_VHT_SIG_A_0_VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_SU_0_HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_SU_0_HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_MU_DL_0_HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_MU_DL_0_HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B1_MU_0_HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B1_MU_0_HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_MU_0_HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_MU_0_HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_OFDMA_0_HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_OFDMA_0_HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_RSSI_LEGACY_19_RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_19_RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MPDU_START_0_RX_MPDU_INFO_RX_MPDU_INFO_DETAILS_OFFSET \
	RX_MPDU_START_0_RX_MPDU_INFO_RX_MPDU_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET
#define UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC \
	UNIFORM_REO_STATUS_HEADER_STATUS_HEADER
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET
#define UNIFIED_TCL_DATA_CMD_0_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_0_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET
#define UNIFIED_TCL_DATA_CMD_1_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_1_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET \
	TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB \
	BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK \
	BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB \
	BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK \
	BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB \
	BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK \
	BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB \
	BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK \
	BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB \
	TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK \
	TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK \
	WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET \
	WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB \
	WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB

#include "hal_6290_tx.h"
#include "hal_6290_rx.h"
#include <hal_generic_api.h>
#include "hal_li_rx.h"
#include "hal_li_api.h"
#include "hal_li_generic_api.h"

/**
 * hal_rx_get_rx_fragment_number_6290(): Function to retrieve rx fragment number
 *
 * @nbuf: Network buffer
 * Returns: rx fragment number
 */
static
uint8_t hal_rx_get_rx_fragment_number_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	/* Return first 4 bits as fragment number */
	return (HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info) &
		DOT11_SEQ_FRAG_MASK);
}

/**
 * hal_rx_msdu_end_da_is_mcbc_get: API to check if pkt is MCBC
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_mcbc
 */
static inline uint8_t
hal_rx_msdu_end_da_is_mcbc_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_DA_IS_MCBC_GET(msdu_end);
}

/**
 * hal_rx_msdu_end_sa_is_valid_get_6290(): API to get_6290 the
 * sa_is_valid bit from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_is_valid bit
 */
static uint8_t
hal_rx_msdu_end_sa_is_valid_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t sa_is_valid;

	sa_is_valid = HAL_RX_MSDU_END_SA_IS_VALID_GET(msdu_end);

	return sa_is_valid;
}

/**
 * hal_rx_msdu_end_sa_idx_get_6290(): API to get_6290 the
 * sa_idx from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_idx (SA AST index)
 */
static
uint16_t hal_rx_msdu_end_sa_idx_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t sa_idx;

	sa_idx = HAL_RX_MSDU_END_SA_IDX_GET(msdu_end);

	return sa_idx;
}

/**
 * hal_rx_desc_is_first_msdu_6290() - Check if first msdu
 *
 * @hal_soc_hdl: hal_soc handle
 * @hw_desc_addr: hardware descriptor address
 *
 * Return: 0 - success/ non-zero failure
 */
static uint32_t hal_rx_desc_is_first_msdu_6290(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_tlvs = (struct rx_pkt_tlvs *)hw_desc_addr;
	struct rx_msdu_end *msdu_end = &rx_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_GET(msdu_end, RX_MSDU_END_5, FIRST_MSDU);
}

/**
 * hal_rx_msdu_end_l3_hdr_padding_get_6290(): API to get_6290 the
 * l3_header padding from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: number of l3 header padding bytes
 */
static uint32_t hal_rx_msdu_end_l3_hdr_padding_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t l3_header_padding;

	l3_header_padding = HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(msdu_end);

	return l3_header_padding;
}

/*
 * @ hal_rx_encryption_info_valid_6290: Returns encryption type.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: encryption type
 */
static uint32_t hal_rx_encryption_info_valid_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	uint32_t encryption_info = HAL_RX_MPDU_ENCRYPTION_INFO_VALID(mpdu_info);

	return encryption_info;
}

/*
 * hal_rx_print_pn_6290: Prints the PN of rx packet.
 * @buf: rx_tlv_hdr of the received packet
 *
 * Return: void
 */
static void hal_rx_print_pn_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	uint32_t pn_31_0 = HAL_RX_MPDU_PN_31_0_GET(mpdu_info);
	uint32_t pn_63_32 = HAL_RX_MPDU_PN_63_32_GET(mpdu_info);
	uint32_t pn_95_64 = HAL_RX_MPDU_PN_95_64_GET(mpdu_info);
	uint32_t pn_127_96 = HAL_RX_MPDU_PN_127_96_GET(mpdu_info);

	hal_debug("PN number pn_127_96 0x%x pn_95_64 0x%x pn_63_32 0x%x pn_31_0 0x%x ",
		  pn_127_96, pn_95_64, pn_63_32, pn_31_0);
}

/**
 * hal_rx_msdu_end_first_msdu_get_6290: API to get first msdu status
 * from rx_msdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: first_msdu
 */
static uint8_t
hal_rx_msdu_end_first_msdu_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t first_msdu;

	first_msdu = HAL_RX_MSDU_END_FIRST_MSDU_GET(msdu_end);

	return first_msdu;
}

/**
 * hal_rx_msdu_end_da_is_valid_get_6290: API to check if da is valid
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_valid
 */
static uint8_t hal_rx_msdu_end_da_is_valid_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t da_is_valid;

	da_is_valid = HAL_RX_MSDU_END_DA_IS_VALID_GET(msdu_end);

	return da_is_valid;
}

/**
 * hal_rx_msdu_end_last_msdu_get_6290: API to get last msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: last_msdu
 */
static uint8_t hal_rx_msdu_end_last_msdu_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t last_msdu;

	last_msdu = HAL_RX_MSDU_END_LAST_MSDU_GET(msdu_end);

	return last_msdu;
}

/*
 * hal_rx_get_mpdu_mac_ad4_valid_6290(): Retrieves if mpdu 4th addr is valid
 *
 * @nbuf: Network buffer
 * Returns: value of mpdu 4th address valid field
 */
static bool hal_rx_get_mpdu_mac_ad4_valid_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	bool ad4_valid = 0;

	ad4_valid = HAL_RX_MPDU_GET_MAC_AD4_VALID(rx_mpdu_info);

	return ad4_valid;
}

/**
 * hal_rx_mpdu_start_sw_peer_id_get_6290: Retrieve sw peer_id
 * @buf: network buffer
 *
 * Return: sw peer_id:
 */
static uint32_t hal_rx_mpdu_start_sw_peer_id_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	return HAL_RX_MPDU_INFO_SW_PEER_ID_GET(
		&mpdu_start->rx_mpdu_info_details);
}

/*
 * hal_rx_mpdu_get_to_ds_6290(): API to get the tods info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(to_ds)
 */

static uint32_t hal_rx_mpdu_get_to_ds_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	return HAL_RX_MPDU_GET_TODS(mpdu_info);
}

/*
 * hal_rx_mpdu_get_fr_ds_6290(): API to get the from ds info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(fr_ds)
 */
static uint32_t hal_rx_mpdu_get_fr_ds_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	return HAL_RX_MPDU_GET_FROMDS(mpdu_info);
}

/*
 * hal_rx_get_mpdu_frame_control_valid_6290(): Retrieves mpdu frame
 * control valid
 *
 * @nbuf: Network buffer
 * Returns: value of frame control valid field
 */
static uint8_t hal_rx_get_mpdu_frame_control_valid_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_FRAME_CONTROL_VALID(rx_mpdu_info);
}

/*
 * hal_rx_mpdu_get_addr1_6290(): API to check get address1 of the mpdu
 *
 * @buf: pointer to the start of RX PKT TLV headera
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr1_6290(uint8_t *buf, uint8_t *mac_addr)
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
 * hal_rx_mpdu_get_addr2_6290(): API to check get address2 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr2_6290(uint8_t *buf,
					     uint8_t *mac_addr)
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
 * hal_rx_mpdu_get_addr3_6290(): API to get address3 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr3_6290(uint8_t *buf, uint8_t *mac_addr)
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
 * hal_rx_mpdu_get_addr4_6290(): API to get address4 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr4_6290(uint8_t *buf, uint8_t *mac_addr)
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

/*
 * hal_rx_get_mpdu_sequence_control_valid_6290(): Get mpdu
 * sequence control valid
 *
 * @nbuf: Network buffer
 * Returns: value of sequence control valid field
 */
static uint8_t hal_rx_get_mpdu_sequence_control_valid_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_SEQUENCE_CONTROL_VALID(rx_mpdu_info);
}

/**
 * hal_rx_is_unicast_6290: check packet is unicast frame or not.
 *
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: true on unicast.
 */
static bool hal_rx_is_unicast_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
		&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t grp_id;
	uint8_t *rx_mpdu_info = (uint8_t *)&mpdu_start->rx_mpdu_info_details;

	grp_id = (_HAL_MS((*_OFFSET_TO_WORD_PTR((rx_mpdu_info),
			   RX_MPDU_INFO_0_SW_FRAME_GROUP_ID_OFFSET)),
			  RX_MPDU_INFO_0_SW_FRAME_GROUP_ID_MASK,
			  RX_MPDU_INFO_0_SW_FRAME_GROUP_ID_LSB));

	return (HAL_MPDU_SW_FRAME_GROUP_UNICAST_DATA == grp_id) ? true : false;
}

/**
 * hal_rx_tid_get_6290: get tid based on qos control valid.
 * @hal_soc_hdl: hal soc handle
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: tid
 */
static uint32_t hal_rx_tid_get_6290(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
	&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint8_t *rx_mpdu_info = (uint8_t *)&mpdu_start->rx_mpdu_info_details;
	uint8_t qos_control_valid =
		(_HAL_MS((*_OFFSET_TO_WORD_PTR((rx_mpdu_info),
			  RX_MPDU_INFO_2_MPDU_QOS_CONTROL_VALID_OFFSET)),
			 RX_MPDU_INFO_2_MPDU_QOS_CONTROL_VALID_MASK,
			 RX_MPDU_INFO_2_MPDU_QOS_CONTROL_VALID_LSB));

	if (qos_control_valid)
		return hal_rx_mpdu_start_tid_get_6290(buf);

	return HAL_RX_NON_QOS_TID;
}

/**
 * hal_rx_hw_desc_get_ppduid_get_6290(): retrieve ppdu id
 * @rx_tlv_hdr: start address of rx_pkt_tlvs
 * @rxdma_dst_ring_desc: Rx HW descriptor
 *
 * Return: ppdu id
 */
static uint32_t hal_rx_hw_desc_get_ppduid_get_6290(void *rx_tlv_hdr,
						   void *rxdma_dst_ring_desc)
{
	struct rx_mpdu_info *rx_mpdu_info;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)rx_tlv_hdr;

	rx_mpdu_info =
		&rx_desc->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;

	return HAL_RX_GET(rx_mpdu_info, RX_MPDU_INFO_0, PHY_PPDU_ID);
}

/**
 * hal_reo_status_get_header_6290 - Process reo desc info
 * @ring_desc: REO status ring descriptor
 * @b - tlv type info
 * @h1 - Pointer to hal_reo_status_header where info to be stored
 *
 * Return - none.
 *
 */
static void hal_reo_status_get_header_6290(hal_ring_desc_t ring_desc, int b,
					   void *h1)
{
	uint32_t *d = (uint32_t *)ring_desc;
	uint32_t val1 = 0;
	struct hal_reo_status_header *h =
			(struct hal_reo_status_header *)h1;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	d += HAL_GET_NUM_DWORDS(sizeof(struct tlv_32_hdr));

	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_0,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_QUEUE_STATUS_0,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_0,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_0,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_0,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_DW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_0,
		  UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UPDATE_RX_REO_QUEUE_STATUS_0,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->cmd_num =
		HAL_GET_FIELD(
			      UNIFORM_REO_STATUS_HEADER_0, REO_STATUS_NUMBER,
			      val1);
	h->exec_time =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_0,
			      CMD_EXECUTION_TIME, val1);
	h->status =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_0,
			      REO_CMD_EXECUTION_STATUS, val1);
	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_1,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_QUEUE_STATUS_1,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_1,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_1,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_1,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_DW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_1,
		  UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UPDATE_RX_REO_QUEUE_STATUS_1,
			UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->tstamp =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_1, TIMESTAMP, val1);
}

/**
 * hal_rx_mpdu_start_mpdu_qos_control_valid_get_6290():
 * Retrieve qos control valid bit from the tlv.
 * @buf: pointer to rx pkt TLV.
 *
 * Return: qos control value.
 */
static inline uint32_t
hal_rx_mpdu_start_mpdu_qos_control_valid_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	return HAL_RX_MPDU_INFO_QOS_CONTROL_VALID_GET(
		&mpdu_start->rx_mpdu_info_details);
}

/**
 * hal_rx_msdu_end_sa_sw_peer_id_get_6290(): API to get the
 * sa_sw_peer_id from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: sa_sw_peer_id index
 */
static inline uint32_t
hal_rx_msdu_end_sa_sw_peer_id_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_SA_SW_PEER_ID_GET(msdu_end);
}

/**
 * hal_tx_desc_set_mesh_en_6290 - Set mesh_enable flag in Tx descriptor
 * @desc: Handle to Tx Descriptor
 * @en:   For raw WiFi frames, this indicates transmission to a mesh STA,
 *        enabling the interpretation of the 'Mesh Control Present' bit
 *        (bit 8) of QoS Control (otherwise this bit is ignored),
 *        For native WiFi frames, this indicates that a 'Mesh Control' field
 *        is present between the header and the LLC.
 *
 * Return: void
 */
static inline
void hal_tx_desc_set_mesh_en_6290(void *desc, uint8_t en)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_4, MESH_ENABLE) |=
		HAL_TX_SM(TCL_DATA_CMD_4, MESH_ENABLE, en);
}

static
void *hal_rx_msdu0_buffer_addr_lsb_6290(void *link_desc_va)
{
	return (void *)HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va);
}

static
void *hal_rx_msdu_desc_info_ptr_get_6290(void *msdu0)
{
	return (void *)HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0);
}

static
void *hal_ent_mpdu_desc_info_6290(void *ent_ring_desc)
{
	return (void *)HAL_ENT_MPDU_DESC_INFO(ent_ring_desc);
}

static
void *hal_dst_mpdu_desc_info_6290(void *dst_ring_desc)
{
	return (void *)HAL_DST_MPDU_DESC_INFO(dst_ring_desc);
}

static
uint8_t hal_rx_get_fc_valid_6290(uint8_t *buf)
{
	return HAL_RX_GET_FC_VALID(buf);
}

static uint8_t hal_rx_get_to_ds_flag_6290(uint8_t *buf)
{
	return HAL_RX_GET_TO_DS_FLAG(buf);
}

static uint8_t hal_rx_get_mac_addr2_valid_6290(uint8_t *buf)
{
	return HAL_RX_GET_MAC_ADDR2_VALID(buf);
}

static uint8_t hal_rx_get_filter_category_6290(uint8_t *buf)
{
	return HAL_RX_GET_FILTER_CATEGORY(buf);
}

static uint32_t
hal_rx_get_ppdu_id_6290(uint8_t *buf)
{
	return HAL_RX_GET_PPDU_ID(buf);
}

/**
 * hal_reo_config_6290(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static
void hal_reo_config_6290(struct hal_soc *soc,
			 uint32_t reg_val,
			 struct hal_reo_params *reo_params)
{
	HAL_REO_R0_CONFIG(soc, reg_val, reo_params);
}

/**
 * hal_rx_msdu_desc_info_get_ptr_6290() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 *
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static void *hal_rx_msdu_desc_info_get_ptr_6290(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr);
}

/**
 * hal_rx_link_desc_msdu0_ptr_6290 - Get pointer to rx_msdu details
 * @link_desc - Pointer to link desc
 *
 * Return - Pointer to rx_msdu_details structure
 *
 */
static void *hal_rx_link_desc_msdu0_ptr_6290(void *link_desc)
{
	return HAL_RX_LINK_DESC_MSDU0_PTR(link_desc);
}

/**
 * hal_rx_msdu_flow_idx_get_6290: API to get flow index
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index value from MSDU END TLV
 */
static inline uint32_t hal_rx_msdu_flow_idx_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_GET(msdu_end);
}

/**
 * hal_rx_msdu_flow_idx_invalid_6290: API to get flow index invalid
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index invalid value from MSDU END TLV
 */
static bool hal_rx_msdu_flow_idx_invalid_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_INVALID_GET(msdu_end);
}

/**
 * hal_rx_msdu_flow_idx_timeout_6290: API to get flow index timeout
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index timeout value from MSDU END TLV
 */
static bool hal_rx_msdu_flow_idx_timeout_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_TIMEOUT_GET(msdu_end);
}

/**
 * hal_rx_msdu_fse_metadata_get_6290: API to get FSE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: fse metadata value from MSDU END TLV
 */
static uint32_t hal_rx_msdu_fse_metadata_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FSE_METADATA_GET(msdu_end);
}

/**
 * hal_rx_msdu_cce_metadata_get_6290: API to get CCE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: cce_metadata
 */
static uint16_t
hal_rx_msdu_cce_metadata_get_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_CCE_METADATA_GET(msdu_end);
}

/**
 * hal_rx_msdu_get_flow_params_6290: API to get flow index, flow index invalid
 * and flow index timeout from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 * @flow_invalid: pointer to return value of flow_idx_valid
 * @flow_timeout: pointer to return value of flow_idx_timeout
 * @flow_index: pointer to return value of flow_idx
 *
 * Return: none
 */
static inline void
hal_rx_msdu_get_flow_params_6290(uint8_t *buf,
				 bool *flow_invalid,
				 bool *flow_timeout,
				 uint32_t *flow_index)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	*flow_invalid = HAL_RX_MSDU_END_FLOW_IDX_INVALID_GET(msdu_end);
	*flow_timeout = HAL_RX_MSDU_END_FLOW_IDX_TIMEOUT_GET(msdu_end);
	*flow_index = HAL_RX_MSDU_END_FLOW_IDX_GET(msdu_end);
}

/**
 * hal_rx_tlv_get_tcp_chksum_6290() - API to get tcp checksum
 * @buf: rx_tlv_hdr
 *
 * Return: tcp checksum
 */
static uint16_t
hal_rx_tlv_get_tcp_chksum_6290(uint8_t *buf)
{
	return HAL_RX_TLV_GET_TCP_CHKSUM(buf);
}

/**
 * hal_rx_get_rx_sequence_6290(): Function to retrieve rx sequence number
 * @nbuf: Network buffer
 *
 * Return: rx sequence number
 */
static
uint16_t hal_rx_get_rx_sequence_6290(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info);
}

/**
 * hal_get_window_address_6290(): Function to get hp/tp address
 * @hal_soc: Pointer to hal_soc
 * @addr: address offset of register
 *
 * Return: modified address offset of register
 */
static inline qdf_iomem_t hal_get_window_address_6290(struct hal_soc *hal_soc,
							qdf_iomem_t addr)
{
	return addr;
}

static
void hal_compute_reo_remap_ix2_ix3_6290(uint32_t *ring, uint32_t num_rings,
					uint32_t *remap1, uint32_t *remap2)
{
	switch (num_rings) {
	case 3:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[1], 17) |
				HAL_REO_REMAP_IX2(ring[2], 18) |
				HAL_REO_REMAP_IX2(ring[0], 19) |
				HAL_REO_REMAP_IX2(ring[1], 20) |
				HAL_REO_REMAP_IX2(ring[2], 21) |
				HAL_REO_REMAP_IX2(ring[0], 22) |
				HAL_REO_REMAP_IX2(ring[1], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[2], 24) |
				HAL_REO_REMAP_IX3(ring[0], 25) |
				HAL_REO_REMAP_IX3(ring[1], 26) |
				HAL_REO_REMAP_IX3(ring[2], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[1], 29) |
				HAL_REO_REMAP_IX3(ring[2], 30) |
				HAL_REO_REMAP_IX3(ring[0], 31);
		break;
	case 4:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[1], 17) |
				HAL_REO_REMAP_IX2(ring[2], 18) |
				HAL_REO_REMAP_IX2(ring[3], 19) |
				HAL_REO_REMAP_IX2(ring[0], 20) |
				HAL_REO_REMAP_IX2(ring[1], 21) |
				HAL_REO_REMAP_IX2(ring[2], 22) |
				HAL_REO_REMAP_IX2(ring[3], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[0], 24) |
				HAL_REO_REMAP_IX3(ring[1], 25) |
				HAL_REO_REMAP_IX3(ring[2], 26) |
				HAL_REO_REMAP_IX3(ring[3], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[1], 29) |
				HAL_REO_REMAP_IX3(ring[2], 30) |
				HAL_REO_REMAP_IX3(ring[3], 31);
		break;
	}
}

static void hal_hw_txrx_ops_attach_6290(struct hal_soc *hal_soc)
{
	/* init and setup */
	hal_soc->ops->hal_srng_dst_hw_init = hal_srng_dst_hw_init_generic;
	hal_soc->ops->hal_srng_src_hw_init = hal_srng_src_hw_init_generic;
	hal_soc->ops->hal_get_hw_hptp = hal_get_hw_hptp_generic;
	hal_soc->ops->hal_reo_setup = hal_reo_setup_generic_li;
	hal_soc->ops->hal_get_window_address = hal_get_window_address_6290;

	/* tx */
	hal_soc->ops->hal_tx_desc_set_dscp_tid_table_id =
	hal_tx_desc_set_dscp_tid_table_id_6290;
	hal_soc->ops->hal_tx_set_dscp_tid_map = hal_tx_set_dscp_tid_map_6290;
	hal_soc->ops->hal_tx_update_dscp_tid = hal_tx_update_dscp_tid_6290;
	hal_soc->ops->hal_tx_desc_set_lmac_id = hal_tx_desc_set_lmac_id_6290;
	hal_soc->ops->hal_tx_desc_set_buf_addr =
					hal_tx_desc_set_buf_addr_generic_li;
	hal_soc->ops->hal_tx_desc_set_search_type =
					hal_tx_desc_set_search_type_generic_li;
	hal_soc->ops->hal_tx_desc_set_search_index =
					hal_tx_desc_set_search_index_generic_li;
	hal_soc->ops->hal_tx_desc_set_cache_set_num =
				hal_tx_desc_set_cache_set_num_generic_li;
	hal_soc->ops->hal_tx_comp_get_status =
					hal_tx_comp_get_status_generic_li;
	hal_soc->ops->hal_tx_comp_get_release_reason =
		hal_tx_comp_get_release_reason_generic_li;
	hal_soc->ops->hal_get_wbm_internal_error =
					hal_get_wbm_internal_error_generic_li;
	hal_soc->ops->hal_tx_desc_set_mesh_en = hal_tx_desc_set_mesh_en_6290;
	hal_soc->ops->hal_tx_init_cmd_credit_ring =
					hal_tx_init_cmd_credit_ring_6290;

	/* rx */
	hal_soc->ops->hal_rx_msdu_start_nss_get =
					hal_rx_msdu_start_nss_get_6290;
	hal_soc->ops->hal_rx_mon_hw_desc_get_mpdu_status =
		hal_rx_mon_hw_desc_get_mpdu_status_6290;
	hal_soc->ops->hal_rx_get_tlv = hal_rx_get_tlv_6290;
	hal_soc->ops->hal_rx_proc_phyrx_other_receive_info_tlv =
		hal_rx_proc_phyrx_other_receive_info_tlv_6290;
	hal_soc->ops->hal_rx_dump_msdu_start_tlv =
					hal_rx_dump_msdu_start_tlv_6290;
	hal_soc->ops->hal_rx_dump_msdu_end_tlv = hal_rx_dump_msdu_end_tlv_6290;
	hal_soc->ops->hal_get_link_desc_size = hal_get_link_desc_size_6290;
	hal_soc->ops->hal_rx_mpdu_start_tid_get =
					hal_rx_mpdu_start_tid_get_6290;
	hal_soc->ops->hal_rx_msdu_start_reception_type_get =
		hal_rx_msdu_start_reception_type_get_6290;
	hal_soc->ops->hal_rx_msdu_end_da_idx_get =
					hal_rx_msdu_end_da_idx_get_6290;
	hal_soc->ops->hal_rx_msdu_desc_info_get_ptr =
					hal_rx_msdu_desc_info_get_ptr_6290;
	hal_soc->ops->hal_rx_link_desc_msdu0_ptr =
					hal_rx_link_desc_msdu0_ptr_6290;
	hal_soc->ops->hal_reo_status_get_header =
					hal_reo_status_get_header_6290;
	hal_soc->ops->hal_rx_status_get_tlv_info =
					hal_rx_status_get_tlv_info_generic_li;
	hal_soc->ops->hal_rx_wbm_err_info_get =
					hal_rx_wbm_err_info_get_generic_li;
	hal_soc->ops->hal_rx_dump_mpdu_start_tlv =
					hal_rx_dump_mpdu_start_tlv_generic_li;

	hal_soc->ops->hal_tx_set_pcp_tid_map =
					hal_tx_set_pcp_tid_map_generic_li;
	hal_soc->ops->hal_tx_update_pcp_tid_map =
					hal_tx_update_pcp_tid_generic_li;
	hal_soc->ops->hal_tx_set_tidmap_prty =
					hal_tx_update_tidmap_prty_generic_li;
	hal_soc->ops->hal_rx_get_rx_fragment_number =
					hal_rx_get_rx_fragment_number_6290;
	hal_soc->ops->hal_rx_msdu_end_da_is_mcbc_get =
					hal_rx_msdu_end_da_is_mcbc_get_6290;
	hal_soc->ops->hal_rx_msdu_end_sa_is_valid_get =
					hal_rx_msdu_end_sa_is_valid_get_6290;
	hal_soc->ops->hal_rx_msdu_end_sa_idx_get =
					hal_rx_msdu_end_sa_idx_get_6290;
	hal_soc->ops->hal_rx_desc_is_first_msdu =
					hal_rx_desc_is_first_msdu_6290;
	hal_soc->ops->hal_rx_msdu_end_l3_hdr_padding_get =
				hal_rx_msdu_end_l3_hdr_padding_get_6290;
	hal_soc->ops->hal_rx_encryption_info_valid =
					hal_rx_encryption_info_valid_6290;
	hal_soc->ops->hal_rx_print_pn = hal_rx_print_pn_6290;
	hal_soc->ops->hal_rx_msdu_end_first_msdu_get =
					hal_rx_msdu_end_first_msdu_get_6290;
	hal_soc->ops->hal_rx_msdu_end_da_is_valid_get =
					hal_rx_msdu_end_da_is_valid_get_6290;
	hal_soc->ops->hal_rx_msdu_end_last_msdu_get =
					hal_rx_msdu_end_last_msdu_get_6290;
	hal_soc->ops->hal_rx_get_mpdu_mac_ad4_valid =
					hal_rx_get_mpdu_mac_ad4_valid_6290;
	hal_soc->ops->hal_rx_mpdu_start_sw_peer_id_get =
		hal_rx_mpdu_start_sw_peer_id_get_6290;
	hal_soc->ops->hal_rx_mpdu_peer_meta_data_get =
		hal_rx_mpdu_peer_meta_data_get_li;
	hal_soc->ops->hal_rx_mpdu_get_to_ds = hal_rx_mpdu_get_to_ds_6290;
	hal_soc->ops->hal_rx_mpdu_get_fr_ds = hal_rx_mpdu_get_fr_ds_6290;
	hal_soc->ops->hal_rx_get_mpdu_frame_control_valid =
				hal_rx_get_mpdu_frame_control_valid_6290;
	hal_soc->ops->hal_rx_mpdu_get_addr1 = hal_rx_mpdu_get_addr1_6290;
	hal_soc->ops->hal_rx_mpdu_get_addr2 = hal_rx_mpdu_get_addr2_6290;
	hal_soc->ops->hal_rx_mpdu_get_addr3 = hal_rx_mpdu_get_addr3_6290;
	hal_soc->ops->hal_rx_mpdu_get_addr4 = hal_rx_mpdu_get_addr4_6290;
	hal_soc->ops->hal_rx_get_mpdu_sequence_control_valid =
				hal_rx_get_mpdu_sequence_control_valid_6290;
	hal_soc->ops->hal_rx_is_unicast = hal_rx_is_unicast_6290;
	hal_soc->ops->hal_rx_tid_get = hal_rx_tid_get_6290;
	hal_soc->ops->hal_rx_hw_desc_get_ppduid_get =
					hal_rx_hw_desc_get_ppduid_get_6290;
	hal_soc->ops->hal_rx_mpdu_start_mpdu_qos_control_valid_get =
		hal_rx_mpdu_start_mpdu_qos_control_valid_get_6290;
	hal_soc->ops->hal_rx_msdu_end_sa_sw_peer_id_get =
		hal_rx_msdu_end_sa_sw_peer_id_get_6290;
	hal_soc->ops->hal_rx_msdu0_buffer_addr_lsb =
					hal_rx_msdu0_buffer_addr_lsb_6290;
	hal_soc->ops->hal_rx_msdu_desc_info_ptr_get =
					hal_rx_msdu_desc_info_ptr_get_6290;
	hal_soc->ops->hal_ent_mpdu_desc_info = hal_ent_mpdu_desc_info_6290;
	hal_soc->ops->hal_dst_mpdu_desc_info = hal_dst_mpdu_desc_info_6290;
	hal_soc->ops->hal_rx_get_fc_valid = hal_rx_get_fc_valid_6290;
	hal_soc->ops->hal_rx_get_to_ds_flag = hal_rx_get_to_ds_flag_6290;
	hal_soc->ops->hal_rx_get_mac_addr2_valid =
					hal_rx_get_mac_addr2_valid_6290;
	hal_soc->ops->hal_rx_get_filter_category =
					hal_rx_get_filter_category_6290;
	hal_soc->ops->hal_rx_get_ppdu_id = hal_rx_get_ppdu_id_6290;
	hal_soc->ops->hal_reo_config = hal_reo_config_6290;
	hal_soc->ops->hal_rx_msdu_flow_idx_get = hal_rx_msdu_flow_idx_get_6290;
	hal_soc->ops->hal_rx_msdu_flow_idx_invalid =
					hal_rx_msdu_flow_idx_invalid_6290;
	hal_soc->ops->hal_rx_msdu_flow_idx_timeout =
					hal_rx_msdu_flow_idx_timeout_6290;
	hal_soc->ops->hal_rx_msdu_fse_metadata_get =
					hal_rx_msdu_fse_metadata_get_6290;
	hal_soc->ops->hal_rx_msdu_cce_match_get =
					hal_rx_msdu_cce_match_get_li;
	hal_soc->ops->hal_rx_msdu_cce_metadata_get =
					hal_rx_msdu_cce_metadata_get_6290;
	hal_soc->ops->hal_rx_msdu_get_flow_params =
					hal_rx_msdu_get_flow_params_6290;
	hal_soc->ops->hal_rx_tlv_get_tcp_chksum =
					hal_rx_tlv_get_tcp_chksum_6290;
	hal_soc->ops->hal_rx_get_rx_sequence = hal_rx_get_rx_sequence_6290;
	/* rx - msdu end fast path info fields */
	hal_soc->ops->hal_rx_msdu_packet_metadata_get =
		hal_rx_msdu_packet_metadata_get_generic_li;
	/* rx - TLV struct offsets */
	hal_soc->ops->hal_rx_msdu_end_offset_get =
					hal_rx_msdu_end_offset_get_generic;
	hal_soc->ops->hal_rx_attn_offset_get =
					hal_rx_attn_offset_get_generic;
	hal_soc->ops->hal_rx_msdu_start_offset_get =
					hal_rx_msdu_start_offset_get_generic;
	hal_soc->ops->hal_rx_mpdu_start_offset_get =
					hal_rx_mpdu_start_offset_get_generic;
	hal_soc->ops->hal_rx_mpdu_end_offset_get =
					hal_rx_mpdu_end_offset_get_generic;
#ifndef NO_RX_PKT_HDR_TLV
	hal_soc->ops->hal_rx_pkt_tlv_offset_get =
					hal_rx_pkt_tlv_offset_get_generic;
#endif
	hal_soc->ops->hal_compute_reo_remap_ix2_ix3 =
					hal_compute_reo_remap_ix2_ix3_6290;
	hal_soc->ops->hal_setup_link_idle_list =
				hal_setup_link_idle_list_generic_li;
	hal_soc->ops->hal_compute_reo_remap_ix0 = NULL;
};

struct hal_hw_srng_config hw_srng_table_6290[] = {
	/* TODO: max_rings can populated by querying HW capabilities */
	{ /* REO_DST */
		.start_ring_id = HAL_SRNG_REO2SW1,
		.max_rings = 4,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO2SW1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
		},
		.reg_size = {
			HWIO_REO_R0_REO2SW2_RING_BASE_LSB_ADDR(0) -
				HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(0),
			HWIO_REO_R2_REO2SW2_RING_HP_ADDR(0) -
				HWIO_REO_R2_REO2SW1_RING_HP_ADDR(0),
		},
		.max_size = HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_EXCEPTION */
		/* Designating REO2TCL ring as exception ring. This ring is
		 * similar to other REO2SW rings though it is named as REO2TCL.
		 * Any of theREO2SW rings can be used as exception ring.
		 */
		.start_ring_id = HAL_SRNG_REO2TCL,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2TCL_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO2TCL_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_REO2TCL_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2TCL_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_REINJECT */
		.start_ring_id = HAL_SRNG_SW2REO,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_SW2REO_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_BMSK >>
				HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_CMD */
		.start_ring_id = HAL_SRNG_REO_CMD,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct reo_get_queue_stats)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_REO_CMD_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO_CMD_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_STATUS */
		.start_ring_id = HAL_SRNG_REO_STATUS,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct reo_get_queue_stats_status)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO_STATUS_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_DATA */
		.start_ring_id = HAL_SRNG_SW2TCL1,
		.max_rings = 3,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_data_cmd)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
		},
		.reg_size = {
			HWIO_TCL_R0_SW2TCL2_RING_BASE_LSB_ADDR(0) -
				HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(0),
			HWIO_TCL_R2_SW2TCL2_RING_HP_ADDR(0) -
				HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(0),
		},
		.max_size = HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_CMD */
		.start_ring_id = HAL_SRNG_SW2TCL_CMD,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_gse_cmd)) >> 2,
		.lmac_ring =  FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL_CMD_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_SW2TCL_CMD_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_TCL_R0_SW2TCL_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL_CMD_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_STATUS */
		.start_ring_id = HAL_SRNG_TCL_STATUS,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_status_ring)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_SRC */
		.start_ring_id = HAL_SRNG_CE_0_SRC,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_src_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET),
		},
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_SRC_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_SRC_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST */
		.start_ring_id = HAL_SRNG_CE_0_DST,
		.max_rings = 12,
		.entry_size = 8 >> 2,
		/*TODO: entry_size above should actually be
		 * sizeof(struct ce_dst_desc) >> 2, but couldn't find definition
		 * of struct ce_dst_desc in HW header files
		 */
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		},
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST_STATUS */
		.start_ring_id = HAL_SRNG_CE_0_DST_STATUS,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_stat_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_STATUS_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		},
			/* TODO: check destination status ring registers */
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM_IDLE_LINK */
		.start_ring_id = HAL_SRNG_WBM_IDLE_LINK,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_link_descriptor_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* SW2WBM_RELEASE */
		.start_ring_id = HAL_SRNG_WBM_SW_RELEASE,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM2SW_RELEASE */
		.start_ring_id = HAL_SRNG_WBM2SW0_RELEASE,
		.max_rings = 4,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		},
		.reg_size = {
			HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET) -
				HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
			HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET) -
				HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		},
		.max_size =
			HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
				HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* RXDMA_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA0_BUF0,
#ifdef IPA_OFFLOAD
		.max_rings = 3,
#else
		.max_rings = 2,
#endif
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_DST */
		.start_ring_id = HAL_SRNG_WMAC1_RXDMA2SW0,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring =  TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA2_BUF,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_STATUS */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_STATBUF,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_DST */
		.start_ring_id = HAL_SRNG_WMAC1_RXDMA2SW1,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_DESC */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_DESC,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* DIR_BUF_RX_DMA_SRC */
		.start_ring_id = HAL_SRNG_DIR_BUF_RX_SRC_DMA_RING,
		.max_rings = 1,
		.entry_size = 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
#ifdef WLAN_FEATURE_CIF_CFR
	{ /* WIFI_POS_SRC */
		.start_ring_id = HAL_SRNG_WIFI_POS_SRC_DMA_RING,
		.max_rings = 1,
		.entry_size = sizeof(wmi_oem_dma_buf_release_entry)  >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
#endif
	{ /* REO2PPE */ 0},
	{ /* PPE2TCL */ 0},
	{ /* PPE_RELEASE */ 0},
	{ /* TX_MONITOR_BUF */ 0},
	{ /* TX_MONITOR_DST */ 0},
	{ /* SW2RXDMA_NEW */ 0},
};

/**
 * hal_qca6290_attach() - Attach 6290 target specific hal_soc ops,
 *			  offset and srng table
 */
void hal_qca6290_attach(struct hal_soc *hal_soc)
{
	hal_soc->hw_srng_table = hw_srng_table_6290;
	hal_srng_hw_reg_offset_init_generic(hal_soc);

	hal_hw_txrx_default_ops_attach_li(hal_soc);
	hal_hw_txrx_ops_attach_6290(hal_soc);
}

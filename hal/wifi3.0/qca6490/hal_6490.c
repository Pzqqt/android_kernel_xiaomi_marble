/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

/**
 * hal_rx_get_rx_fragment_number_6490(): Function to retrieve rx fragment number
 *
 * @nbuf: Network buffer
 * Returns: rx fragment number
 */
static
uint8_t hal_rx_get_rx_fragment_number_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	/* Return first 4 bits as fragment number */
	return (HAL_RX_MPDU_GET_SEQUENCE_NUMBER(rx_mpdu_info) &
		DOT11_SEQ_FRAG_MASK);
}

/**
 * hal_rx_msdu_end_da_is_mcbc_get_6490(): API to check if pkt is MCBC
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_mcbc
 */
static uint8_t
hal_rx_msdu_end_da_is_mcbc_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_DA_IS_MCBC_GET(msdu_end);
}

/**
 * hal_rx_msdu_end_sa_is_valid_get_6490(): API to get_6490 the
 * sa_is_valid bit from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_is_valid bit
 */
static uint8_t
hal_rx_msdu_end_sa_is_valid_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t sa_is_valid;

	sa_is_valid = HAL_RX_MSDU_END_SA_IS_VALID_GET(msdu_end);

	return sa_is_valid;
}

/**
 * hal_rx_msdu_end_sa_idx_get_6490(): API to get_6490 the
 * sa_idx from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: sa_idx (SA AST index)
 */
static
uint16_t hal_rx_msdu_end_sa_idx_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint16_t sa_idx;

	sa_idx = HAL_RX_MSDU_END_SA_IDX_GET(msdu_end);

	return sa_idx;
}

/**
 * hal_rx_desc_is_first_msdu_6490() - Check if first msdu
 *
 * @hal_soc_hdl: hal_soc handle
 * @hw_desc_addr: hardware descriptor address
 *
 * Return: 0 - success/ non-zero failure
 */
static uint32_t hal_rx_desc_is_first_msdu_6490(void *hw_desc_addr)
{
	struct rx_pkt_tlvs *rx_tlvs = (struct rx_pkt_tlvs *)hw_desc_addr;
	struct rx_msdu_end *msdu_end = &rx_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_GET(msdu_end, RX_MSDU_END_10, FIRST_MSDU);
}

/**
 * hal_rx_msdu_end_l3_hdr_padding_get_6490(): API to get_6490 the
 * l3_header padding from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: number of l3 header padding bytes
 */
static uint32_t hal_rx_msdu_end_l3_hdr_padding_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t l3_header_padding;

	l3_header_padding = HAL_RX_MSDU_END_L3_HEADER_PADDING_GET(msdu_end);

	return l3_header_padding;
}

/*
 * @ hal_rx_encryption_info_valid_6490: Returns encryption type.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: encryption type
 */
static uint32_t hal_rx_encryption_info_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	uint32_t encryption_info = HAL_RX_MPDU_ENCRYPTION_INFO_VALID(mpdu_info);

	return encryption_info;
}

/*
 * @ hal_rx_print_pn_6490: Prints the PN of rx packet.
 *
 * @ buf: rx_tlv_hdr of the received packet
 * @ Return: void
 */
static void hal_rx_print_pn_6490(uint8_t *buf)
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
 * hal_rx_msdu_end_first_msdu_get_6490: API to get first msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: first_msdu
 */
static uint8_t hal_rx_msdu_end_first_msdu_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t first_msdu;

	first_msdu = HAL_RX_MSDU_END_FIRST_MSDU_GET(msdu_end);

	return first_msdu;
}

/**
 * hal_rx_msdu_end_da_is_valid_get_6490: API to check if da is valid
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: da_is_valid
 */
static uint8_t hal_rx_msdu_end_da_is_valid_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t da_is_valid;

	da_is_valid = HAL_RX_MSDU_END_DA_IS_VALID_GET(msdu_end);

	return da_is_valid;
}

/**
 * hal_rx_msdu_end_last_msdu_get_6490: API to get last msdu status
 * from rx_msdu_end TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: last_msdu
 */
static uint8_t hal_rx_msdu_end_last_msdu_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint8_t last_msdu;

	last_msdu = HAL_RX_MSDU_END_LAST_MSDU_GET(msdu_end);

	return last_msdu;
}

/*
 * hal_rx_get_mpdu_mac_ad4_valid_6490(): Retrieves if mpdu 4th addr is valid
 *
 * @nbuf: Network buffer
 * Returns: value of mpdu 4th address valid field
 */
static bool hal_rx_get_mpdu_mac_ad4_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	bool ad4_valid = 0;

	ad4_valid = HAL_RX_MPDU_GET_MAC_AD4_VALID(rx_mpdu_info);

	return ad4_valid;
}

/**
 * hal_rx_mpdu_start_sw_peer_id_get_6490: Retrieve sw peer_id
 * @buf: network buffer
 *
 * Return: sw peer_id
 */
static uint32_t hal_rx_mpdu_start_sw_peer_id_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
			&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	return HAL_RX_MPDU_INFO_SW_PEER_ID_GET(
		&mpdu_start->rx_mpdu_info_details);
}

/**
 * hal_rx_mpdu_get_to_ds_6490(): API to get the tods info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(to_ds)
 */
static uint32_t hal_rx_mpdu_get_to_ds_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	return HAL_RX_MPDU_GET_TODS(mpdu_info);
}

/*
 * hal_rx_mpdu_get_fr_ds_6490(): API to get the from ds info
 * from rx_mpdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(fr_ds)
 */
static uint32_t hal_rx_mpdu_get_fr_ds_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;

	return HAL_RX_MPDU_GET_FROMDS(mpdu_info);
}

/*
 * hal_rx_get_mpdu_frame_control_valid_6490(): Retrieves mpdu
 * frame control valid
 *
 * @nbuf: Network buffer
 * Returns: value of frame control valid field
 */
static uint8_t hal_rx_get_mpdu_frame_control_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_FRAME_CONTROL_VALID(rx_mpdu_info);
}

/*
 * hal_rx_mpdu_get_addr1_6490(): API to check get address1 of the mpdu
 *
 * @buf: pointer to the start of RX PKT TLV headera
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr1_6490(uint8_t *buf, uint8_t *mac_addr)
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
 * hal_rx_mpdu_get_addr2_6490(): API to check get address2 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr2_6490(uint8_t *buf,
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
 * hal_rx_mpdu_get_addr3_6490(): API to get address3 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr3_6490(uint8_t *buf, uint8_t *mac_addr)
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
 * hal_rx_mpdu_get_addr4_6490(): API to get address4 of the mpdu
 * in the packet
 *
 * @buf: pointer to the start of RX PKT TLV header
 * @mac_addr: pointer to mac address
 * Return: success/failure
 */
static QDF_STATUS hal_rx_mpdu_get_addr4_6490(uint8_t *buf, uint8_t *mac_addr)
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
 * hal_rx_get_mpdu_sequence_control_valid_6490(): Get mpdu
 * sequence control valid
 *
 * @nbuf: Network buffer
 * Returns: value of sequence control valid field
 */
static uint8_t hal_rx_get_mpdu_sequence_control_valid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);

	return HAL_RX_MPDU_GET_SEQUENCE_CONTROL_VALID(rx_mpdu_info);
}

/**
 * hal_rx_is_unicast_6490: check packet is unicast frame or not.
 *
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: true on unicast.
 */
static bool hal_rx_is_unicast_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
		&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint32_t grp_id;
	uint8_t *rx_mpdu_info = (uint8_t *)&mpdu_start->rx_mpdu_info_details;

	grp_id = (_HAL_MS((*_OFFSET_TO_WORD_PTR((rx_mpdu_info),
			   RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_OFFSET)),
			  RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_MASK,
			  RX_MPDU_INFO_9_SW_FRAME_GROUP_ID_LSB));

	return (HAL_MPDU_SW_FRAME_GROUP_UNICAST_DATA == grp_id) ? true : false;
}

/**
 * hal_rx_tid_get_6490: get tid based on qos control valid.
 * @hal_soc_hdl: hal_soc handle
 * @ buf: pointer to rx pkt TLV.
 *
 * Return: tid
 */
static uint32_t hal_rx_tid_get_6490(hal_soc_handle_t hal_soc_hdl, uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
	&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	uint8_t *rx_mpdu_info = (uint8_t *)&mpdu_start->rx_mpdu_info_details;
	uint8_t qos_control_valid =
		(_HAL_MS((*_OFFSET_TO_WORD_PTR((rx_mpdu_info),
			  RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_OFFSET)),
			 RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_MASK,
			 RX_MPDU_INFO_11_MPDU_QOS_CONTROL_VALID_LSB));

	if (qos_control_valid)
		return hal_rx_mpdu_start_tid_get_6490(buf);

	return HAL_RX_NON_QOS_TID;
}

/**
 * hal_rx_hw_desc_get_ppduid_get_6490(): retrieve ppdu id
 * @hw_desc_addr: hw addr
 *
 * Return: ppdu id
 */
static uint32_t hal_rx_hw_desc_get_ppduid_get_6490(void *hw_desc_addr)
{
	struct rx_mpdu_info *rx_mpdu_info;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_mpdu_info =
		&rx_desc->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;

	return HAL_RX_GET(rx_mpdu_info, RX_MPDU_INFO_9, PHY_PPDU_ID);
}

/**
 * hal_reo_status_get_header_6490 - Process reo desc info
 * @d - Pointer to reo descriptior
 * @b - tlv type info
 * @h1 - Pointer to hal_reo_status_header where info to be stored
 *
 * Return - none.
 *
 */
static void hal_reo_status_get_header_6490(uint32_t *d, int b, void *h1)
{
	uint32_t val1 = 0;
	struct hal_reo_status_header *h =
			(struct hal_reo_status_header *)h1;

	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_GET_QUEUE_STATS_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_QUEUE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_DW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_0,
		  STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UPDATE_RX_REO_QUEUE_STATUS_0,
			STATUS_HEADER_REO_STATUS_NUMBER)];
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
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_QUEUE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_CACHE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UNBLOCK_CACHE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_FLUSH_TIMEOUT_LIST_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_DW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_1,
		  STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_DW(REO_UPDATE_RX_REO_QUEUE_STATUS_1,
			STATUS_HEADER_TIMESTAMP)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->tstamp =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER_1, TIMESTAMP, val1);
}

/**
 * hal_tx_desc_set_mesh_en_6490 - Set mesh_enable flag in Tx descriptor
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
void hal_tx_desc_set_mesh_en_6490(void *desc, uint8_t en)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_5, MESH_ENABLE) |=
		HAL_TX_SM(TCL_DATA_CMD_5, MESH_ENABLE, en);
}

static
void *hal_rx_msdu0_buffer_addr_lsb_6490(void *link_desc_va)
{
	return (void *)HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va);
}

static
void *hal_rx_msdu_desc_info_ptr_get_6490(void *msdu0)
{
	return (void *)HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0);
}

static
void *hal_ent_mpdu_desc_info_6490(void *ent_ring_desc)
{
	return (void *)HAL_ENT_MPDU_DESC_INFO(ent_ring_desc);
}

static
void *hal_dst_mpdu_desc_info_6490(void *dst_ring_desc)
{
	return (void *)HAL_DST_MPDU_DESC_INFO(dst_ring_desc);
}

static
uint8_t hal_rx_get_fc_valid_6490(uint8_t *buf)
{
	return HAL_RX_GET_FC_VALID(buf);
}

static uint8_t hal_rx_get_to_ds_flag_6490(uint8_t *buf)
{
	return HAL_RX_GET_TO_DS_FLAG(buf);
}

static uint8_t hal_rx_get_mac_addr2_valid_6490(uint8_t *buf)
{
	return HAL_RX_GET_MAC_ADDR2_VALID(buf);
}

static uint8_t hal_rx_get_filter_category_6490(uint8_t *buf)
{
	return HAL_RX_GET_FILTER_CATEGORY(buf);
}

static uint32_t
hal_rx_get_ppdu_id_6490(uint8_t *buf)
{
	return HAL_RX_GET_PPDU_ID(buf);
}

/**
 * hal_reo_config_6490(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static
void hal_reo_config_6490(struct hal_soc *soc,
			 uint32_t reg_val,
			 struct hal_reo_params *reo_params)
{
	HAL_REO_R0_CONFIG(soc, reg_val, reo_params);
}

/**
 * hal_rx_msdu_desc_info_get_ptr_6490() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 *
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static void *hal_rx_msdu_desc_info_get_ptr_6490(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr);
}

/**
 * hal_rx_link_desc_msdu0_ptr_6490 - Get pointer to rx_msdu details
 * @link_desc - Pointer to link desc
 *
 * Return - Pointer to rx_msdu_details structure
 *
 */
static void *hal_rx_link_desc_msdu0_ptr_6490(void *link_desc)
{
	return HAL_RX_LINK_DESC_MSDU0_PTR(link_desc);
}

/**
 * hal_rx_msdu_flow_idx_get_6490: API to get flow index
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index value from MSDU END TLV
 */
static inline uint32_t hal_rx_msdu_flow_idx_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_GET(msdu_end);
}

/**
 * hal_rx_msdu_flow_idx_invalid_6490: API to get flow index invalid
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index invalid value from MSDU END TLV
 */
static bool hal_rx_msdu_flow_idx_invalid_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_INVALID_GET(msdu_end);
}

/**
 * hal_rx_msdu_flow_idx_timeout_6490: API to get flow index timeout
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: flow index timeout value from MSDU END TLV
 */
static bool hal_rx_msdu_flow_idx_timeout_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FLOW_IDX_TIMEOUT_GET(msdu_end);
}

/**
 * hal_rx_msdu_fse_metadata_get_6490: API to get FSE metadata
 * from rx_msdu_end TLV
 * @buf: pointer to the start of RX PKT TLV headers
 *
 * Return: fse metadata value from MSDU END TLV
 */
static uint32_t hal_rx_msdu_fse_metadata_get_6490(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;

	return HAL_RX_MSDU_END_FSE_METADATA_GET(msdu_end);
}

struct hal_hw_txrx_ops qca6490_hal_hw_txrx_ops = {
	/* tx */
	hal_tx_desc_set_mesh_en_6490,
	/* rx */
	hal_rx_get_rx_fragment_number_6490,
	hal_rx_msdu_end_da_is_mcbc_get_6490,
	hal_rx_msdu_end_sa_is_valid_get_6490,
	hal_rx_desc_is_first_msdu_6490,
	hal_rx_msdu_end_l3_hdr_padding_get_6490,
	hal_rx_encryption_info_valid_6490,
	hal_rx_print_pn_6490,
	hal_rx_msdu_end_first_msdu_get_6490,
	hal_rx_msdu_end_da_is_valid_get_6490,
	hal_rx_msdu_end_last_msdu_get_6490,
	hal_rx_get_mpdu_mac_ad4_valid_6490,
	hal_rx_mpdu_start_sw_peer_id_get_6490,
	hal_rx_mpdu_get_to_ds_6490,
	hal_rx_mpdu_get_fr_ds_6490,
	hal_rx_get_mpdu_frame_control_valid_6490,
	hal_rx_mpdu_get_addr1_6490,
	hal_rx_mpdu_get_addr2_6490,
	hal_rx_mpdu_get_addr3_6490,
	hal_rx_mpdu_get_addr4_6490,
	hal_rx_get_mpdu_sequence_control_valid_6490,
	hal_rx_is_unicast_6490,
	hal_rx_tid_get_6490,
	hal_rx_hw_desc_get_ppduid_get_6490,
	NULL,
	NULL,
	hal_rx_msdu0_buffer_addr_lsb_6490,
	hal_rx_msdu_desc_info_ptr_get_6490,
	hal_ent_mpdu_desc_info_6490,
	hal_dst_mpdu_desc_info_6490,
	hal_rx_get_fc_valid_6490,
	hal_rx_get_to_ds_flag_6490,
	hal_rx_get_mac_addr2_valid_6490,
	hal_rx_get_filter_category_6490,
	hal_rx_get_ppdu_id_6490,
	hal_reo_config_6490,
	hal_rx_msdu_flow_idx_get_6490,
	hal_rx_msdu_flow_idx_invalid_6490,
	hal_rx_msdu_flow_idx_timeout_6490,
	hal_rx_msdu_fse_metadata_get_6490,
};

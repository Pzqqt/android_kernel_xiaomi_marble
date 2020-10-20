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

#if !defined(HAL_TX_H)
#define HAL_TX_H

/*---------------------------------------------------------------------------
  Include files
  ---------------------------------------------------------------------------*/
#include "hal_api.h"
#include "wcss_version.h"

#define WBM_RELEASE_RING_5_TX_RATE_STATS_OFFSET   0x00000014
#define WBM_RELEASE_RING_5_TX_RATE_STATS_LSB      0
#define WBM_RELEASE_RING_5_TX_RATE_STATS_MASK     0xffffffff

#define HAL_WBM_RELEASE_RING_2_BUFFER_TYPE    0
#define HAL_WBM_RELEASE_RING_2_DESC_TYPE      1

#define HAL_TX_DESC_TLV_TAG_OFFSET 1
#define HAL_TX_DESC_TLV_LEN_OFFSET 10

/*---------------------------------------------------------------------------
  Preprocessor definitions and constants
  ---------------------------------------------------------------------------*/
#define HAL_OFFSET(block, field) block ## _ ## field ## _OFFSET

#define HAL_SET_FLD(desc, block , field) \
	(*(uint32_t *) ((uint8_t *) desc + HAL_OFFSET(block, field)))

#define HAL_SET_FLD_OFFSET(desc, block , field, offset) \
	(*(uint32_t *) ((uint8_t *) desc + HAL_OFFSET(block, field) + (offset)))

#define HAL_TX_DESC_SET_TLV_HDR(desc, tag, len) \
do {                                            \
	uint32_t temp = 0; \
	temp |= (tag << HAL_TX_DESC_TLV_TAG_OFFSET); \
	temp |= (len << HAL_TX_DESC_TLV_LEN_OFFSET); \
	(*(uint32_t *)desc) = temp; \
} while (0)

#define HAL_TX_TCL_DATA_TAG WIFITCL_DATA_CMD_E
#define HAL_TX_TCL_CMD_TAG WIFITCL_GSE_CMD_E

#define HAL_TX_SM(block, field, value) \
	((value << (block ## _ ## field ## _LSB)) & \
	 (block ## _ ## field ## _MASK))

#define HAL_TX_MS(block, field, value) \
	(((value) & (block ## _ ## field ## _MASK)) >> \
	 (block ## _ ## field ## _LSB))

#define HAL_TX_DESC_GET(desc, block, field) \
	HAL_TX_MS(block, field, HAL_SET_FLD(desc, block, field))

#define HAL_TX_DESC_SUBBLOCK_GET(desc, block, sub, field) \
	HAL_TX_MS(sub, field, HAL_SET_FLD(desc, block, sub))

#define HAL_TX_BUF_TYPE_BUFFER 0
#define HAL_TX_BUF_TYPE_EXT_DESC 1

#define HAL_TX_DESC_LEN_DWORDS (NUM_OF_DWORDS_TCL_DATA_CMD)
#define HAL_TX_DESC_LEN_BYTES  (NUM_OF_DWORDS_TCL_DATA_CMD * 4)
#define HAL_TX_EXTENSION_DESC_LEN_DWORDS (NUM_OF_DWORDS_TX_MSDU_EXTENSION)
#define HAL_TX_EXTENSION_DESC_LEN_BYTES (NUM_OF_DWORDS_TX_MSDU_EXTENSION * 4)

#define HAL_TX_COMPLETION_DESC_LEN_DWORDS (NUM_OF_DWORDS_WBM_RELEASE_RING)
#define HAL_TX_COMPLETION_DESC_LEN_BYTES (NUM_OF_DWORDS_WBM_RELEASE_RING*4)
#define HAL_TX_BITS_PER_TID 3
#define HAL_TX_TID_BITS_MASK ((1 << HAL_TX_BITS_PER_TID) - 1)
#define HAL_TX_NUM_DSCP_PER_REGISTER 10
#define HAL_MAX_HW_DSCP_TID_MAPS 2
#define HAL_MAX_HW_DSCP_TID_MAPS_11AX 32

#define HAL_MAX_HW_DSCP_TID_V2_MAPS 48
#define HTT_META_HEADER_LEN_BYTES 64
#define HAL_TX_EXT_DESC_WITH_META_DATA \
	(HTT_META_HEADER_LEN_BYTES + HAL_TX_EXTENSION_DESC_LEN_BYTES)

#define HAL_TX_NUM_PCP_PER_REGISTER 8

/* Length of WBM release ring without the status words */
#define HAL_TX_COMPLETION_DESC_BASE_LEN 12

#define HAL_TX_COMP_RELEASE_SOURCE_TQM 0
#define HAL_TX_COMP_RELEASE_SOURCE_REO 2
#define HAL_TX_COMP_RELEASE_SOURCE_FW 3

/* Define a place-holder release reason for FW */
#define HAL_TX_COMP_RELEASE_REASON_FW 99

/*
 * Offset of HTT Tx Descriptor in WBM Completion
 * HTT Tx Desc structure is passed from firmware to host overlayed
 * on wbm_release_ring DWORDs 2,3 ,4 and 5for software based completions
 * (Exception frames and TQM bypass frames)
 */
#define HAL_TX_COMP_HTT_STATUS_OFFSET 8
#define HAL_TX_COMP_HTT_STATUS_LEN 16

#define HAL_TX_BUF_TYPE_BUFFER 0
#define HAL_TX_BUF_TYPE_EXT_DESC 1

#define HAL_TX_EXT_DESC_BUF_OFFSET TX_MSDU_EXTENSION_6_BUF0_PTR_31_0_OFFSET
#define HAL_TX_EXT_BUF_LOW_MASK TX_MSDU_EXTENSION_6_BUF0_PTR_31_0_MASK
#define HAL_TX_EXT_BUF_HI_MASK TX_MSDU_EXTENSION_7_BUF0_PTR_39_32_MASK
#define HAL_TX_EXT_BUF_LEN_MASK TX_MSDU_EXTENSION_7_BUF0_LEN_MASK
#define HAL_TX_EXT_BUF_LEN_LSB  TX_MSDU_EXTENSION_7_BUF0_LEN_LSB
#define HAL_TX_EXT_BUF_WD_SIZE  2

#define HAL_TX_DESC_ADDRX_EN 0x1
#define HAL_TX_DESC_ADDRY_EN 0x2
#define HAL_TX_DESC_DEFAULT_LMAC_ID 0x3

#define HAL_TX_ADDR_SEARCH_DEFAULT 0x0
#define HAL_TX_ADDR_INDEX_SEARCH 0x1
#define HAL_TX_FLOW_INDEX_SEARCH 0x2

enum hal_tx_ret_buf_manager {
	HAL_WBM_SW0_BM_ID = 3,
	HAL_WBM_SW1_BM_ID = 4,
	HAL_WBM_SW2_BM_ID = 5,
	HAL_WBM_SW3_BM_ID = 6,
};

/*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/
/**
 * struct hal_tx_completion_status - HAL Tx completion descriptor contents
 * @status: frame acked/failed
 * @release_src: release source = TQM/FW
 * @ack_frame_rssi: RSSI of the received ACK or BA frame
 * @first_msdu: Indicates this MSDU is the first MSDU in AMSDU
 * @last_msdu: Indicates this MSDU is the last MSDU in AMSDU
 * @msdu_part_of_amsdu : Indicates this MSDU was part of an A-MSDU in MPDU
 * @bw: Indicates the BW of the upcoming transmission -
 *       <enum 0 transmit_bw_20_MHz>
 *       <enum 1 transmit_bw_40_MHz>
 *       <enum 2 transmit_bw_80_MHz>
 *       <enum 3 transmit_bw_160_MHz>
 * @pkt_type: Transmit Packet Type
 * @stbc: When set, STBC transmission rate was used
 * @ldpc: When set, use LDPC transmission rates
 * @sgi: <enum 0     0_8_us_sgi > Legacy normal GI
 *       <enum 1     0_4_us_sgi > Legacy short GI
 *       <enum 2     1_6_us_sgi > HE related GI
 *       <enum 3     3_2_us_sgi > HE
 * @mcs: Transmit MCS Rate
 * @ofdma: Set when the transmission was an OFDMA transmission
 * @tones_in_ru: The number of tones in the RU used.
 * @tsf: Lower 32 bits of the TSF
 * @ppdu_id: TSF, snapshot of this value when transmission of the
 *           PPDU containing the frame finished.
 * @transmit_cnt: Number of times this frame has been transmitted
 * @tid: TID of the flow or MPDU queue
 * @peer_id: Peer ID of the flow or MPDU queue
 */
struct hal_tx_completion_status {
	uint8_t status;
	uint8_t release_src;
	uint8_t ack_frame_rssi;
	uint8_t first_msdu:1,
		last_msdu:1,
		msdu_part_of_amsdu:1;
	uint32_t bw:2,
		 pkt_type:4,
		 stbc:1,
		 ldpc:1,
		 sgi:2,
		 mcs:4,
		 ofdma:1,
		 tones_in_ru:12,
		 valid:1;
	uint32_t tsf;
	uint32_t ppdu_id;
	uint8_t transmit_cnt;
	uint8_t tid;
	uint16_t peer_id;
};

/**
 * struct hal_tx_desc_comp_s - hal tx completion descriptor contents
 * @desc: Transmit status information from descriptor
 */
struct hal_tx_desc_comp_s {
	uint32_t desc[HAL_TX_COMPLETION_DESC_LEN_DWORDS];
};

/*
 * enum hal_tx_encrypt_type - Type of decrypt cipher used (valid only for RAW)
 * @HAL_TX_ENCRYPT_TYPE_WEP_40: WEP 40-bit
 * @HAL_TX_ENCRYPT_TYPE_WEP_10: WEP 10-bit
 * @HAL_TX_ENCRYPT_TYPE_TKIP_NO_MIC: TKIP without MIC
 * @HAL_TX_ENCRYPT_TYPE_WEP_128: WEP_128
 * @HAL_TX_ENCRYPT_TYPE_TKIP_WITH_MIC: TKIP_WITH_MIC
 * @HAL_TX_ENCRYPT_TYPE_WAPI: WAPI
 * @HAL_TX_ENCRYPT_TYPE_AES_CCMP_128: AES_CCMP_128
 * @HAL_TX_ENCRYPT_TYPE_NO_CIPHER: NO CIPHER
 * @HAL_TX_ENCRYPT_TYPE_AES_CCMP_256: AES_CCMP_256
 * @HAL_TX_ENCRYPT_TYPE_AES_GCMP_128: AES_GCMP_128
 * @HAL_TX_ENCRYPT_TYPE_AES_GCMP_256: AES_GCMP_256
 * @HAL_TX_ENCRYPT_TYPE_WAPI_GCM_SM4: WAPI GCM SM4
 */
enum hal_tx_encrypt_type {
	HAL_TX_ENCRYPT_TYPE_WEP_40 = 0,
	HAL_TX_ENCRYPT_TYPE_WEP_104 = 1 ,
	HAL_TX_ENCRYPT_TYPE_TKIP_NO_MIC = 2,
	HAL_TX_ENCRYPT_TYPE_WEP_128 = 3,
	HAL_TX_ENCRYPT_TYPE_TKIP_WITH_MIC = 4,
	HAL_TX_ENCRYPT_TYPE_WAPI = 5,
	HAL_TX_ENCRYPT_TYPE_AES_CCMP_128 = 6,
	HAL_TX_ENCRYPT_TYPE_NO_CIPHER = 7,
	HAL_TX_ENCRYPT_TYPE_AES_CCMP_256 = 8,
	HAL_TX_ENCRYPT_TYPE_AES_GCMP_128 = 9,
	HAL_TX_ENCRYPT_TYPE_AES_GCMP_256 = 10,
	HAL_TX_ENCRYPT_TYPE_WAPI_GCM_SM4 = 11,
};

/*
 * enum hal_tx_encap_type - Encapsulation type that HW will perform
 * @HAL_TX_ENCAP_TYPE_RAW: Raw Packet Type
 * @HAL_TX_ENCAP_TYPE_NWIFI: Native WiFi Type
 * @HAL_TX_ENCAP_TYPE_ETHERNET: Ethernet
 * @HAL_TX_ENCAP_TYPE_802_3: 802.3 Frame
 */
enum hal_tx_encap_type {
	HAL_TX_ENCAP_TYPE_RAW = 0,
	HAL_TX_ENCAP_TYPE_NWIFI = 1,
	HAL_TX_ENCAP_TYPE_ETHERNET = 2,
	HAL_TX_ENCAP_TYPE_802_3 = 3,
};

/**
 * enum hal_tx_tqm_release_reason - TQM Release reason codes
 *
 * @HAL_TX_TQM_RR_FRAME_ACKED : ACK of BA for it was received
 * @HAL_TX_TQM_RR_REM_CMD_REM : Remove cmd of type “Remove_mpdus” initiated
 *				by SW
 * @HAL_TX_TQM_RR_REM_CMD_TX  : Remove command of type Remove_transmitted_mpdus
 *				initiated by SW
 * @HAL_TX_TQM_RR_REM_CMD_NOTX : Remove cmd of type Remove_untransmitted_mpdus
 *				initiated by SW
 * @HAL_TX_TQM_RR_REM_CMD_AGED : Remove command of type “Remove_aged_mpdus” or
 *				“Remove_aged_msdus” initiated by SW
 * @HAL_TX_TQM_RR_FW_REASON1 : Remove command where fw indicated that
 *				remove reason is fw_reason1
 * @HAL_TX_TQM_RR_FW_REASON2 : Remove command where fw indicated that
 *				remove reason is fw_reason2
 * @HAL_TX_TQM_RR_FW_REASON3 : Remove command where fw indicated that
 *				remove reason is fw_reason3
 * @HAL_TX_TQM_RR_REM_CMD_DISABLE_QUEUE : Remove command where fw indicated that
 *				remove reason is remove disable queue
 */
enum hal_tx_tqm_release_reason {
	HAL_TX_TQM_RR_FRAME_ACKED,
	HAL_TX_TQM_RR_REM_CMD_REM,
	HAL_TX_TQM_RR_REM_CMD_TX,
	HAL_TX_TQM_RR_REM_CMD_NOTX,
	HAL_TX_TQM_RR_REM_CMD_AGED,
	HAL_TX_TQM_RR_FW_REASON1,
	HAL_TX_TQM_RR_FW_REASON2,
	HAL_TX_TQM_RR_FW_REASON3,
	HAL_TX_TQM_RR_REM_CMD_DISABLE_QUEUE,
};

/* enum - Table IDs for 2 DSCP-TID mapping Tables that TCL H/W supports
 * @HAL_TX_DSCP_TID_MAP_TABLE_DEFAULT: Default DSCP-TID mapping table
 * @HAL_TX_DSCP_TID_MAP_TABLE_OVERRIDE: DSCP-TID map override table
 */
enum hal_tx_dscp_tid_table_id {
	HAL_TX_DSCP_TID_MAP_TABLE_DEFAULT,
	HAL_TX_DSCP_TID_MAP_TABLE_OVERRIDE,
};

/*---------------------------------------------------------------------------
  Function declarations and documentation
  ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
  TCL Descriptor accessor APIs
  ---------------------------------------------------------------------------*/

/**
 * hal_tx_desc_set_buf_length - Set Data length in bytes in Tx Descriptor
 * @desc: Handle to Tx Descriptor
 * @data_length: MSDU length in case of direct descriptor.
 *              Length of link extension descriptor in case of Link extension
 *              descriptor.Includes the length of Metadata
 * Return: None
 */
static inline void  hal_tx_desc_set_buf_length(void *desc,
					       uint16_t data_length)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3, DATA_LENGTH) |=
		HAL_TX_SM(TCL_DATA_CMD_3, DATA_LENGTH, data_length);
}

/**
 * hal_tx_desc_set_buf_offset - Sets Packet Offset field in Tx descriptor
 * @desc: Handle to Tx Descriptor
 * @offset: Packet offset from Metadata in case of direct buffer descriptor.
 *
 * Return: void
 */
static inline void hal_tx_desc_set_buf_offset(void *desc,
					      uint8_t offset)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3, PACKET_OFFSET) |=
		HAL_TX_SM(TCL_DATA_CMD_3, PACKET_OFFSET, offset);
}

/**
 * hal_tx_desc_set_encap_type - Set encapsulation type in Tx Descriptor
 * @desc: Handle to Tx Descriptor
 * @encap_type: Encapsulation that HW will perform
 *
 * Return: void
 *
 */
static inline void hal_tx_desc_set_encap_type(void *desc,
					      enum hal_tx_encap_type encap_type)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_2, ENCAP_TYPE) |=
		HAL_TX_SM(TCL_DATA_CMD_2, ENCAP_TYPE, encap_type);
}

/**
 * hal_tx_desc_set_encrypt_type - Sets the Encrypt Type in Tx Descriptor
 * @desc: Handle to Tx Descriptor
 * @type: Encrypt Type
 *
 * Return: void
 */
static inline void hal_tx_desc_set_encrypt_type(void *desc,
						enum hal_tx_encrypt_type type)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_2, ENCRYPT_TYPE) |=
		HAL_TX_SM(TCL_DATA_CMD_2, ENCRYPT_TYPE, type);
}

/**
 * hal_tx_desc_set_addr_search_flags - Enable AddrX and AddrY search flags
 * @desc: Handle to Tx Descriptor
 * @flags: Bit 0 - AddrY search enable, Bit 1 - AddrX search enable
 *
 * Return: void
 */
static inline void hal_tx_desc_set_addr_search_flags(void *desc,
						     uint8_t flags)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_2, ADDRX_EN) |=
		HAL_TX_SM(TCL_DATA_CMD_2, ADDRX_EN, (flags & 0x1));

	HAL_SET_FLD(desc, TCL_DATA_CMD_2, ADDRY_EN) |=
		HAL_TX_SM(TCL_DATA_CMD_2, ADDRY_EN, (flags >> 1));
}

/**
 * hal_tx_desc_set_l4_checksum_en -  Set TCP/IP checksum enable flags
 * Tx Descriptor for MSDU_buffer type
 * @desc: Handle to Tx Descriptor
 * @en: UDP/TCP over ipv4/ipv6 checksum enable flags (5 bits)
 *
 * Return: void
 */
static inline void hal_tx_desc_set_l4_checksum_en(void *desc,
						  uint8_t en)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3, IPV4_CHECKSUM_EN) |=
		(HAL_TX_SM(TCL_DATA_CMD_3, UDP_OVER_IPV4_CHECKSUM_EN, en) |
		 HAL_TX_SM(TCL_DATA_CMD_3, UDP_OVER_IPV6_CHECKSUM_EN, en) |
		 HAL_TX_SM(TCL_DATA_CMD_3, TCP_OVER_IPV4_CHECKSUM_EN, en) |
		 HAL_TX_SM(TCL_DATA_CMD_3, TCP_OVER_IPV6_CHECKSUM_EN, en));
}

/**
 * hal_tx_desc_set_l3_checksum_en -  Set IPv4 checksum enable flag in
 * Tx Descriptor for MSDU_buffer type
 * @desc: Handle to Tx Descriptor
 * @checksum_en_flags: ipv4 checksum enable flags
 *
 * Return: void
 */
static inline void hal_tx_desc_set_l3_checksum_en(void *desc,
						  uint8_t en)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3, IPV4_CHECKSUM_EN) |=
		HAL_TX_SM(TCL_DATA_CMD_3, IPV4_CHECKSUM_EN, en);
}

/**
 * hal_tx_desc_set_fw_metadata- Sets the metadata that is part of TCL descriptor
 * @desc:Handle to Tx Descriptor
 * @metadata: Metadata to be sent to Firmware
 *
 * Return: void
 */
static inline void hal_tx_desc_set_fw_metadata(void *desc,
				       uint16_t metadata)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_2, TCL_CMD_NUMBER) |=
		HAL_TX_SM(TCL_DATA_CMD_2, TCL_CMD_NUMBER, metadata);
}

/**
 * hal_tx_desc_set_to_fw - Set To_FW bit in Tx Descriptor.
 * @desc:Handle to Tx Descriptor
 * @to_fw: if set, Forward packet to FW along with classification result
 *
 * Return: void
 */
static inline void hal_tx_desc_set_to_fw(void *desc, uint8_t to_fw)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3, TO_FW) |=
		HAL_TX_SM(TCL_DATA_CMD_3, TO_FW, to_fw);
}

/**
 * hal_tx_desc_set_mesh_en - Set mesh_enable flag in Tx descriptor
 * @hal_soc_hdl: hal soc handle
 * @desc: Handle to Tx Descriptor
 * @en:   For raw WiFi frames, this indicates transmission to a mesh STA,
 *        enabling the interpretation of the 'Mesh Control Present' bit
 *        (bit 8) of QoS Control (otherwise this bit is ignored),
 *        For native WiFi frames, this indicates that a 'Mesh Control' field
 *        is present between the header and the LLC.
 *
 * Return: void
 */
static inline void hal_tx_desc_set_mesh_en(hal_soc_handle_t hal_soc_hdl,
					   void *desc, uint8_t en)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_mesh_en(desc, en);
}

/**
 * hal_tx_desc_set_hlos_tid - Set the TID value (override DSCP/PCP fields in
 * frame) to be used for Tx Frame
 * @desc: Handle to Tx Descriptor
 * @hlos_tid: HLOS TID
 *
 * Return: void
 */
static inline void hal_tx_desc_set_hlos_tid(void *desc,
					    uint8_t hlos_tid)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_4, HLOS_TID) |=
		HAL_TX_SM(TCL_DATA_CMD_4, HLOS_TID, hlos_tid);

	HAL_SET_FLD(desc, TCL_DATA_CMD_4, HLOS_TID_OVERWRITE) |=
	   HAL_TX_SM(TCL_DATA_CMD_4, HLOS_TID_OVERWRITE, 1);
}
/**
 * hal_tx_desc_sync - Commit the descriptor to Hardware
 * @hal_tx_des_cached: Cached descriptor that software maintains
 * @hw_desc: Hardware descriptor to be updated
 */
static inline void hal_tx_desc_sync(void *hal_tx_desc_cached,
				    void *hw_desc)
{
	qdf_mem_copy((hw_desc + sizeof(struct tlv_32_hdr)),
			hal_tx_desc_cached, HAL_TX_DESC_LEN_BYTES);
}

/*---------------------------------------------------------------------------
  Tx MSDU Extension Descriptor accessor APIs
  ---------------------------------------------------------------------------*/
/**
 * hal_tx_ext_desc_set_tso_enable() - Set TSO Enable Flag
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @tso_en: bool value set to true if TSO is enabled
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_tso_enable(void *desc,
		uint8_t tso_en)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_0, TSO_ENABLE) |=
		HAL_TX_SM(TX_MSDU_EXTENSION_0, TSO_ENABLE, tso_en);
}

/**
 * hal_tx_ext_desc_set_tso_flags() - Set TSO Flags
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @falgs: 32-bit word with all TSO flags consolidated
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_tso_flags(void *desc,
		uint32_t tso_flags)
{
	HAL_SET_FLD_OFFSET(desc, TX_MSDU_EXTENSION_0, TSO_ENABLE, 0) =
		tso_flags;
}

/**
 * hal_tx_ext_desc_set_tcp_flags() - Enable HW Checksum offload
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @tcp_flags: TCP flags {NS,CWR,ECE,URG,ACK,PSH, RST ,SYN,FIN}
 * @mask: TCP flag mask. Tcp_flag is inserted into the header
 *        based on the mask, if tso is enabled
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_tcp_flags(void *desc,
						 uint16_t tcp_flags,
						 uint16_t mask)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_0, TCP_FLAG) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_0, TCP_FLAG, tcp_flags)) |
		 (HAL_TX_SM(TX_MSDU_EXTENSION_0, TCP_FLAG_MASK, mask)));
}

/**
 * hal_tx_ext_desc_set_msdu_length() - Set L2 and IP Lengths
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @l2_len: L2 length for the msdu, if tso is enabled
 * @ip_len: IP length for the msdu, if tso is enabled
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_msdu_length(void *desc,
						   uint16_t l2_len,
						   uint16_t ip_len)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_1, L2_LENGTH) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_1, L2_LENGTH, l2_len)) |
		 (HAL_TX_SM(TX_MSDU_EXTENSION_1, IP_LENGTH, ip_len)));
}

/**
 * hal_tx_ext_desc_set_tcp_seq() - Set TCP Sequence number
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @seq_num: Tcp_seq_number for the msdu, if tso is enabled
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_tcp_seq(void *desc,
					       uint32_t seq_num)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_2, TCP_SEQ_NUMBER) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_2, TCP_SEQ_NUMBER, seq_num)));
}


/**
 * hal_tx_ext_desc_set_ip_id() - Set IP Identification field
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @id: IP Id field for the msdu, if tso is enabled
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_ip_id(void *desc,
					       uint16_t id)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_3, IP_IDENTIFICATION) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_3, IP_IDENTIFICATION, id)));
}
/**
 * hal_tx_ext_desc_set_buffer() - Set Buffer Pointer and Length for a fragment
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @frag_num: Fragment number (value can be 0 to 5)
 * @paddr_lo: Lower 32-bit of Buffer Physical address
 * @paddr_hi: Upper 32-bit of Buffer Physical address
 * @length: Buffer Length
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_buffer(void *desc,
					      uint8_t frag_num,
					      uint32_t paddr_lo,
					      uint16_t paddr_hi,
					      uint16_t length)
{
	HAL_SET_FLD_OFFSET(desc, TX_MSDU_EXTENSION_6, BUF0_PTR_31_0,
				(frag_num << 3)) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_6, BUF0_PTR_31_0, paddr_lo)));

	HAL_SET_FLD_OFFSET(desc, TX_MSDU_EXTENSION_7, BUF0_PTR_39_32,
				(frag_num << 3)) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_7, BUF0_PTR_39_32,
			 (paddr_hi))));

	HAL_SET_FLD_OFFSET(desc, TX_MSDU_EXTENSION_7, BUF0_LEN,
				(frag_num << 3)) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_7, BUF0_LEN, length)));
}

/**
 * hal_tx_ext_desc_set_buffer0_param() - Set Buffer 0 Pointer and Length
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @paddr_lo: Lower 32-bit of Buffer Physical address
 * @paddr_hi: Upper 32-bit of Buffer Physical address
 * @length: Buffer 0 Length
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_buffer0_param(void *desc,
						     uint32_t paddr_lo,
						     uint16_t paddr_hi,
						     uint16_t length)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_6, BUF0_PTR_31_0) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_6, BUF0_PTR_31_0, paddr_lo)));

	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_7, BUF0_PTR_39_32) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_7,
			 BUF0_PTR_39_32, paddr_hi)));

	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_7, BUF0_LEN) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_7, BUF0_LEN, length)));
}

/**
 * hal_tx_ext_desc_set_buffer1_param() - Set Buffer 1 Pointer and Length
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @paddr_lo: Lower 32-bit of Buffer Physical address
 * @paddr_hi: Upper 32-bit of Buffer Physical address
 * @length: Buffer 1 Length
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_buffer1_param(void *desc,
						     uint32_t paddr_lo,
						     uint16_t paddr_hi,
						     uint16_t length)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_8, BUF1_PTR_31_0) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_8, BUF1_PTR_31_0, paddr_lo)));

	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_9, BUF1_PTR_39_32) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_9,
			 BUF1_PTR_39_32, paddr_hi)));

	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_9, BUF1_LEN) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_9, BUF1_LEN, length)));
}

/**
 * hal_tx_ext_desc_set_buffer2_param() - Set Buffer 2 Pointer and Length
 * @desc: Handle to Tx MSDU Extension Descriptor
 * @paddr_lo: Lower 32-bit of Buffer Physical address
 * @paddr_hi: Upper 32-bit of Buffer Physical address
 * @length: Buffer 2 Length
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_set_buffer2_param(void *desc,
						     uint32_t paddr_lo,
						     uint16_t paddr_hi,
						     uint16_t length)
{
	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_10, BUF2_PTR_31_0) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_10, BUF2_PTR_31_0,
			 paddr_lo)));

	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_11, BUF2_PTR_39_32) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_11, BUF2_PTR_39_32,
			 paddr_hi)));

	HAL_SET_FLD(desc, TX_MSDU_EXTENSION_11, BUF2_LEN) |=
		((HAL_TX_SM(TX_MSDU_EXTENSION_11, BUF2_LEN, length)));
}

/**
 * hal_tx_ext_desc_sync - Commit the descriptor to Hardware
 * @desc_cached: Cached descriptor that software maintains
 * @hw_desc: Hardware descriptor to be updated
 *
 * Return: none
 */
static inline void hal_tx_ext_desc_sync(uint8_t *desc_cached,
					uint8_t *hw_desc)
{
	qdf_mem_copy(&hw_desc[0], &desc_cached[0],
			HAL_TX_EXT_DESC_WITH_META_DATA);
}

/**
 * hal_tx_ext_desc_get_tso_enable() - Set TSO Enable Flag
 * @hal_tx_ext_desc: Handle to Tx MSDU Extension Descriptor
 *
 * Return: tso_enable value in the descriptor
 */
static inline uint32_t hal_tx_ext_desc_get_tso_enable(void *hal_tx_ext_desc)
{
	uint32_t *desc = (uint32_t *) hal_tx_ext_desc;
	return (*desc & TX_MSDU_EXTENSION_0_TSO_ENABLE_MASK) >>
		TX_MSDU_EXTENSION_0_TSO_ENABLE_LSB;
}

/*---------------------------------------------------------------------------
  WBM Descriptor accessor APIs for Tx completions
  ---------------------------------------------------------------------------*/
/**
 * hal_tx_comp_get_desc_id() - Get TX descriptor id within comp descriptor
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will tx descriptor id, cookie, within hardware completion
 * descriptor
 *
 * Return: cookie
 */
static inline uint32_t hal_tx_comp_get_desc_id(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *) (((uint8_t *) hal_desc) +
			       BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_OFFSET);

	/* Cookie is placed on 2nd word */
	return (comp_desc & BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK) >>
		BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB;
}

/**
 * hal_tx_comp_get_paddr() - Get paddr within comp descriptor
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will get buffer physical address within hardware completion
 * descriptor
 *
 * Return: Buffer physical address
 */
static inline qdf_dma_addr_t hal_tx_comp_get_paddr(void *hal_desc)
{
	uint32_t paddr_lo;
	uint32_t paddr_hi;

	paddr_lo = *(uint32_t *) (((uint8_t *) hal_desc) +
			BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_OFFSET);

	paddr_hi = *(uint32_t *) (((uint8_t *) hal_desc) +
			BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_OFFSET);

	paddr_hi = (paddr_hi & BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK) >>
		BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB;

	return (qdf_dma_addr_t) (paddr_lo | (((uint64_t) paddr_hi) << 32));
}

/**
 * hal_tx_comp_get_buffer_source() - Get buffer release source value
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will get buffer release source from Tx completion descriptor
 *
 * Return: buffer release source
 */
static inline uint32_t hal_tx_comp_get_buffer_source(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *) (((uint8_t *) hal_desc) +
			       WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_OFFSET);

	return (comp_desc & WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_MASK) >>
		WBM_RELEASE_RING_2_RELEASE_SOURCE_MODULE_LSB;
}

/**
 * hal_tx_comp_get_buffer_type() - Buffer or Descriptor type
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will return the type of pointer - buffer or descriptor
 *
 * Return: buffer type
 */
static inline uint32_t hal_tx_comp_get_buffer_type(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *) (((uint8_t *) hal_desc) +
			       WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_OFFSET);

	return (comp_desc & WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_MASK) >>
		WBM_RELEASE_RING_2_BUFFER_OR_DESC_TYPE_LSB;
}

/**
 * hal_tx_comp_get_release_reason() - TQM Release reason
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will return the type of pointer - buffer or descriptor
 *
 * Return: buffer type
 */
static inline
uint8_t hal_tx_comp_get_release_reason(void *hal_desc,
				       hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_tx_comp_get_release_reason(hal_desc);
}

/**
 * hal_tx_comp_get_peer_id() - Get peer_id value()
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will get peer_id value from Tx completion descriptor
 *
 * Return: buffer release source
 */
static inline uint16_t hal_tx_comp_get_peer_id(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *)(((uint8_t *)hal_desc) +
			       WBM_RELEASE_RING_7_SW_PEER_ID_OFFSET);

	return (comp_desc & WBM_RELEASE_RING_7_SW_PEER_ID_MASK) >>
		WBM_RELEASE_RING_7_SW_PEER_ID_LSB;
}

/**
 * hal_tx_comp_get_tx_status() - Get tx transmission status()
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will get transmit status value from Tx completion descriptor
 *
 * Return: buffer release source
 */
static inline uint8_t hal_tx_comp_get_tx_status(void *hal_desc)
{
	uint32_t comp_desc =
		*(uint32_t *)(((uint8_t *)hal_desc) +
			       WBM_RELEASE_RING_2_TQM_RELEASE_REASON_OFFSET);

	return (comp_desc & WBM_RELEASE_RING_2_TQM_RELEASE_REASON_MASK) >>
		WBM_RELEASE_RING_2_TQM_RELEASE_REASON_LSB;
}

/**
 * hal_tx_comp_desc_sync() - collect hardware descriptor contents
 * @hal_desc: hardware descriptor pointer
 * @comp: software descriptor pointer
 * @read_status: 0 - Do not read status words from descriptors
 *		 1 - Enable reading of status words from descriptor
 *
 * This function will collect hardware release ring element contents and
 * translate to software descriptor content
 *
 * Return: none
 */

static inline void hal_tx_comp_desc_sync(void *hw_desc,
					 struct hal_tx_desc_comp_s *comp,
					 bool read_status)
{
	if (!read_status)
		qdf_mem_copy(comp, hw_desc, HAL_TX_COMPLETION_DESC_BASE_LEN);
	else
		qdf_mem_copy(comp, hw_desc, HAL_TX_COMPLETION_DESC_LEN_BYTES);
}

/**
 * hal_dump_comp_desc() - dump tx completion descriptor
 * @hal_desc: hardware descriptor pointer
 *
 * This function will print tx completion descriptor
 *
 * Return: none
 */
static inline void hal_dump_comp_desc(void *hw_desc)
{
	struct hal_tx_desc_comp_s *comp =
				(struct hal_tx_desc_comp_s *)hw_desc;
	uint32_t i;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
		  "Current tx completion descriptor is");

	for (i = 0; i < HAL_TX_COMPLETION_DESC_LEN_DWORDS; i++) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_FATAL,
			  "DWORD[i] = 0x%x", comp->desc[i]);
	}
}

/**
 * hal_tx_comp_get_htt_desc() - Read the HTT portion of WBM Descriptor
 * @hal_desc: Hardware (WBM) descriptor pointer
 * @htt_desc: Software HTT descriptor pointer
 *
 * This function will read the HTT structure overlaid on WBM descriptor
 * into a cached software descriptor
 *
 */
static inline void hal_tx_comp_get_htt_desc(void *hw_desc, uint8_t *htt_desc)
{
	uint8_t *desc = hw_desc + HAL_TX_COMP_HTT_STATUS_OFFSET;

	qdf_mem_copy(htt_desc, desc, HAL_TX_COMP_HTT_STATUS_LEN);
}

/**
 * hal_tx_init_data_ring() - Initialize all the TCL Descriptors in SRNG
 * @hal_soc_hdl: Handle to HAL SoC structure
 * @hal_srng: Handle to HAL SRNG structure
 *
 * Return: none
 */
static inline void hal_tx_init_data_ring(hal_soc_handle_t hal_soc_hdl,
					 hal_ring_handle_t hal_ring_hdl)
{
	uint8_t *desc_addr;
	struct hal_srng_params srng_params;
	uint32_t desc_size;
	uint32_t num_desc;

	hal_get_srng_params(hal_soc_hdl, hal_ring_hdl, &srng_params);

	desc_addr = (uint8_t *)srng_params.ring_base_vaddr;
	desc_size = sizeof(struct tcl_data_cmd);
	num_desc = srng_params.num_entries;

	while (num_desc) {
		HAL_TX_DESC_SET_TLV_HDR(desc_addr, HAL_TX_TCL_DATA_TAG,
					desc_size);
		desc_addr += (desc_size + sizeof(struct tlv_32_hdr));
		num_desc--;
	}
}

/**
 * hal_tx_desc_set_dscp_tid_table_id() - Sets DSCP to TID conversion table ID
 * @hal_soc: Handle to HAL SoC structure
 * @desc: Handle to Tx Descriptor
 * @id: DSCP to tid conversion table to be used for this frame
 *
 * Return: void
 */
static inline
void hal_tx_desc_set_dscp_tid_table_id(hal_soc_handle_t hal_soc_hdl,
				       void *desc, uint8_t id)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_dscp_tid_table_id(desc, id);
}

/**
 * hal_tx_set_dscp_tid_map_default() - Configure default DSCP to TID map table
 *
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id: mapping table ID - 0,1
 *
 * Return: void
 */
static inline void hal_tx_set_dscp_tid_map(hal_soc_handle_t hal_soc_hdl,
					   uint8_t *map, uint8_t id)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_set_dscp_tid_map(hal_soc, map, id);
}

/**
 * hal_tx_update_dscp_tid() - Update the dscp tid map table as updated by user
 *
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id : MAP ID
 * @dscp: DSCP_TID map index
 *
 * Return: void
 */
static inline
void hal_tx_update_dscp_tid(hal_soc_handle_t hal_soc_hdl, uint8_t tid,
			    uint8_t id, uint8_t dscp)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_update_dscp_tid(hal_soc, tid, id, dscp);
}

/**
 * hal_tx_desc_set_lmac_id - Set the lmac_id value
 * @desc: Handle to Tx Descriptor
 * @lmac_id: mac Id to ast matching
 *                     b00 – mac 0
 *                     b01 – mac 1
 *                     b10 – mac 2
 *                     b11 – all macs (legacy HK way)
 *
 * Return: void
 */
static inline void hal_tx_desc_set_lmac_id(hal_soc_handle_t hal_soc_hdl,
					   void *desc, uint8_t lmac_id)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_lmac_id(desc, lmac_id);
}

/**
 * hal_tx_desc_set_search_type - Set the search type value
 * @desc: Handle to Tx Descriptor
 * @search_type: search type
 *		     0 – Normal search
 *		     1 – Index based address search
 *		     2 – Index based flow search
 *
 * Return: void
 */
static inline void hal_tx_desc_set_search_type(hal_soc_handle_t hal_soc_hdl,
					       void *desc, uint8_t search_type)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_search_type(desc, search_type);
}

/**
 * hal_tx_desc_set_search_index - Set the search index value
 * @desc: Handle to Tx Descriptor
 * @search_index: The index that will be used for index based address or
 *                flow search. The field is valid when 'search_type' is
 *                1 0r 2
 *
 * Return: void
 */
static inline void hal_tx_desc_set_search_index(hal_soc_handle_t hal_soc_hdl,
						void *desc,
						uint32_t search_index)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_search_index(desc, search_index);
}

/**
 * hal_tx_desc_set_cache_set_num - Set the cache-set-num value
 * @desc: Handle to Tx Descriptor
 * @cache_num: Cache set number that should be used to cache the index
 *                based search results, for address and flow search.
 *                This value should be equal to LSB four bits of the hash value
 *                of match data, in case of search index points to an entry
 *                which may be used in content based search also. The value can
 *                be anything when the entry pointed by search index will not be
 *                used for content based search.
 *
 * Return: void
 */
static inline void hal_tx_desc_set_cache_set_num(hal_soc_handle_t hal_soc_hdl,
						 void *desc,
						 uint8_t cache_num)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_cache_set_num(desc, cache_num);
}

/**
 * hal_tx_comp_get_status() - TQM Release reason
 * @hal_desc: completion ring Tx status
 *
 * This function will parse the WBM completion descriptor and populate in
 * HAL structure
 *
 * Return: none
 */
static inline void hal_tx_comp_get_status(void *desc, void *ts,
					  hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_comp_get_status(desc, ts, hal_soc);
}


/**
 * hal_tx_desc_set_buf_addr - Fill Buffer Address information in Tx Descriptor
 * @desc: Handle to Tx Descriptor
 * @paddr: Physical Address
 * @pool_id: Return Buffer Manager ID
 * @desc_id: Descriptor ID
 * @type: 0 - Address points to a MSDU buffer
 *		1 - Address points to MSDU extension descriptor
 *
 * Return: void
 */
static inline
void hal_tx_desc_set_buf_addr(hal_soc_handle_t hal_soc_hdl, void *desc,
			      dma_addr_t paddr,
			      uint8_t pool_id, uint32_t desc_id,
			      uint8_t type)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_desc_set_buf_addr(desc, paddr, pool_id,
						desc_id, type);

}

/**
 * hal_tx_set_pcp_tid_map_default() - Configure default PCP to TID map table
 *
 * @soc: HAL SoC context
 * @map: PCP-TID mapping table
 *
 * Return: void
 */
static inline void hal_tx_set_pcp_tid_map_default(hal_soc_handle_t hal_soc_hdl,
						  uint8_t *map)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_set_pcp_tid_map(hal_soc, map);
}

/**
 * hal_tx_update_pcp_tid_map() - Update PCP to TID map table
 *
 * @soc: HAL SoC context
 * @pcp: pcp value
 * @tid: tid no
 *
 * Return: void
 */
static inline void hal_tx_update_pcp_tid_map(hal_soc_handle_t hal_soc_hdl,
					     uint8_t pcp, uint8_t tid)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_update_pcp_tid_map(hal_soc, tid, tid);
}

/**
 * hal_tx_set_tidmap_prty() - Configure TIDmap priority
 *
 * @soc: HAL SoC context
 * @val: priority value
 *
 * Return: void
 */
static inline
void hal_tx_set_tidmap_prty(hal_soc_handle_t hal_soc_hdl, uint8_t val)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_tx_set_tidmap_prty(hal_soc, val);
}

/**
 * hal_get_wbm_internal_error() - wbm internal error
 * @hal_desc: completion ring descriptor pointer
 *
 * This function will return the type of pointer - buffer or descriptor
 *
 * Return: buffer type
 */
static inline
uint8_t hal_get_wbm_internal_error(hal_soc_handle_t hal_soc_hdl, void *hal_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	return hal_soc->ops->hal_get_wbm_internal_error(hal_desc);
}

#endif /* HAL_TX_H */

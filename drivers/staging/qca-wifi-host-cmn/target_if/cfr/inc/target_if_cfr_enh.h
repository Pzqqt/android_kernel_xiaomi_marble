/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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

#ifndef _TARGET_IF_CFR_ENH_H_
#define _TARGET_IF_CFR_ENH_H_

#ifdef WLAN_ENH_CFR_ENABLE
/*
 * Memory requirements :
 *
 *  1. DMA header :
 *
 * Legacy DMA header(QCA8074V2) : 2 words (length = 8 bytes)
 * Enhanced DMA header(QCA6018) : Upto 16 words depending on no. of MU users
 *                       in UL-MU-PPDU (Max length = 64 bytes)
 *
 * Fixed 4 words for whal_cfir_enhanced_hdr + freeze TLV
 *                                          + uplink_user_info TLV (MAX 4)
 *
 * mu_rx_num_users -> No. of words in CFR DMA header
 * 0 -> 12  =  4 + 7(freeze TLV) + 1(for 64-bit alignment)
 * 1 -> 12  =  4 + 7(freeze TLV) + 1(user1)
 * 2 -> 14  =  4 + 7(freeze TLV) + 2(users 1,2) + 1(for 64-bit alignment)
 * 3 -> 14  =  4 + 7(freeze TLV) + 3(users 1,2,3)
 * 4 -> 16  =  4 + 7(freeze TLV) + 4(users 1,2,3,4) + 1(for 64-bit alignment)
 *
 *
 * 2. CFR data size for max BW/Nss/Nrx
 *
 *	Cypress : Max BW = 80 MHz
 *			 NSS = 2
 *			 Nrx = 2
 *			 Size of one tone = 4 bytes
 *
 *		a. RTT-H - 2048 bytes
 *
 *		b. Debug-H (MIMO CFR) - 16016 bytes
 *
 *		c. RTT-H + CIR - 10240 bytes = 2048(RTT-H) + 8192(CIR)
 */

/* Max 4 users in MU case */
#define CYP_CFR_MU_USERS 4

#define CYP_MAX_HEADER_LENGTH_WORDS 16

/* payload_len = Max(2048, 16016, 10240) = 16064 (64-bit alignment) */
#define CYP_MAX_DATA_LENGTH_BYTES 16064

/* in ms */
#define LUT_AGE_TIMER 3000
#define LUT_AGE_THRESHOLD 3000

/* Max size :
 * sizeof(csi_cfr_header) + 64 bytes(cfr header) + 16064 bytes(cfr payload)
 */
#define STREAMFS_MAX_SUBBUF_CYP \
	(sizeof(struct csi_cfr_header) + \
	 (CYP_MAX_HEADER_LENGTH_WORDS * 4) + \
	 CYP_MAX_DATA_LENGTH_BYTES)

#define STREAMFS_NUM_SUBBUF_CYP 255

/* Max 37 users in MU case for Pine */
#define PINE_CFR_MU_USERS 37

#define PINE_MAX_HEADER_LENGTH_WORDS 50

#define PINE_MAX_DATA_LENGTH_BYTES 16384

/* Max size :
 * sizeof(csi_cfr_header) + 200 bytes(cfr header) + 16384 bytes(cfr payload)
 */
#define STREAMFS_MAX_SUBBUF_PINE \
	(sizeof(struct csi_cfr_header) + \
	 (PINE_MAX_HEADER_LENGTH_WORDS * 4) + \
	 PINE_MAX_DATA_LENGTH_BYTES)

#define STREAMFS_NUM_SUBBUF_PINE 255

/* Max 37 users in MU case for Waikiki */
#define WAIKIKI_CFR_MU_USERS 37

#define WAIKIKI_MAX_HEADER_LENGTH_WORDS 87

#define WAIKIKI_MAX_DATA_LENGTH_BYTES 64128

/* Max size :
 * sizeof(csi_cfr_header) + 348 bytes(cfr header) + 64128 bytes(cfr payload)
 * where cfr_header size = rtt upload header len + freeze_tlv len +
 *                         uplink user setup info + alignment bytes
 *                       = 16bytes + 32bytes + (8bytes * 37users) + 4bytes
 */
#define STREAMFS_MAX_SUBBUF_WAIKIKI \
	(sizeof(struct csi_cfr_header) + \
	 (WAIKIKI_MAX_HEADER_LENGTH_WORDS * 4) + \
	 WAIKIKI_MAX_DATA_LENGTH_BYTES)

#define STREAMFS_NUM_SUBBUF_WAIKIKI  127

/* Max 4 users in MU case for Spruce */
#define SPRUCE_CFR_MU_USERS 4

#define SPRUCE_MAX_HEADER_LENGTH_WORDS 16

#define SPRUCE_MAX_DATA_LENGTH_BYTES 16384

/* Max size :
 * sizeof(csi_cfr_header) + 200 bytes(cfr header) + 16384 bytes(cfr payload)
 */
#define STREAMFS_MAX_SUBBUF_SPRUCE \
	(sizeof(struct csi_cfr_header) + \
	 (SPRUCE_MAX_HEADER_LENGTH_WORDS * 4) + \
	 SPRUCE_MAX_DATA_LENGTH_BYTES)

#define STREAMFS_NUM_SUBBUF_SPRUCE 255

/* enum macrx_freeze_tlv_version: Reported by uCode in enh_dma_header
 * MACRX_FREEZE_TLV_VERSION_1: Single MU UL user info reported by MAC.
 * This is used in Cypress/HastingsPrime chips. Corresponding structures are
 * macrx_freeze_capture_channel and 1 uplink_user_setup_info.
 *
 * MACRX_FREEZE_TLV_VERSION_2: Upto 4 MU UL user info reported by MAC.
 * This is used in Maple/Spruce/Moselle chips. Corresponding structures are
 * macrx_freeze_capture_channel and 2 uplink_user_setup_info.
 *
 * MACRX_FREEZE_TLV_VERSION_3: Upto 37 MU UL user info reported by MAC.
 * This is used in Pine chip. The corresponding structures are
 * macrx_freeze_capture_channel_v3 and 37 uplink_user_setup_info.
 *
 * MACRX_FREEZE_TLV_VERSION_4: Upto 37 MU UL user info reported by MAC.
 * This is used in Hamilton 1/2.
 *
 * MACRX_FREEZE_TLV_VERSION_5: Upto 37 MU UL user info reported by MAC.
 * This is used in Waikiki chipsets.
 */
enum macrx_freeze_tlv_version {
	MACRX_FREEZE_TLV_VERSION_1 = 1,
	MACRX_FREEZE_TLV_VERSION_2 = 2,
	MACRX_FREEZE_TLV_VERSION_3 = 3,
	MACRX_FREEZE_TLV_VERSION_4 = 4,
	MACRX_FREEZE_TLV_VERSION_5 = 5,
	MACRX_FREEZE_TLV_VERSION_MAX
};

/* Max 4 users in MU case for Maple */
#define MAPLE_CFR_MU_USERS 4

#define MAPLE_MAX_HEADER_LENGTH_WORDS 16

#define MAPLE_MAX_DATA_LENGTH_BYTES 4096

/* Max size :
 * sizeof(csi_cfr_header) + 64 bytes(cfr uCode header) +
 * 4096 bytes(cfr payload)
 */
#define STREAMFS_MAX_SUBBUF_MAPLE \
	(sizeof(struct csi_cfr_header) + \
	 (MAPLE_MAX_HEADER_LENGTH_WORDS * 4) + \
	 MAPLE_MAX_DATA_LENGTH_BYTES)

/*
 * RelayFS memory required:
 * Max sub buffer size * Number of sub buffers
 *
 * Cascade: (2200B  * 1100) ~= 2MB
 * Dakota:  (1100B  * 2200) ~= 2MB
 * Hawkeye: (8200B  * 255 ) ~= 2MB
 * Cypress: (16438B * 255 ) ~= 4MB
 * Pine   : (16894B * 255 ) ~= 4MB
 * Maple  : (4470B *  255 ) ~= 1MB
 *
 */
#define STREAMFS_NUM_SUBBUF_MAPLE 255

enum UCODE_UPLOAD_HEADER_VERSION {
	UPLOAD_HEADER_VERSION_1 = 1,
	UPLOAD_HEADER_VERSION_2 = 2,
	UPLOAD_HEADER_VERSION_3 = 3,
	UPLOAD_HEADER_VERSION_4 = 4,
	UPLOAD_HEADER_VERSION_8 = 8,
	UPLOAD_HEADER_VERSION_9 = 9,
	UPLOAD_HEADER_VERSION_MAX
};

/*
 * @tag: ucode fills this with 0xBA
 *
 * @length: length of CFR header in words (32-bit)
 *
 * @upload_done: ucode sets this to 1 to indicate DMA completion
 *
 * @capture_type:
 *
 *			0 - None
 *			1 - RTT-H (Nss = 1, Nrx)
 *			2 - Debug-H (Nss, Nrx)
 *			3 - Reserved
 *			5 - RTT-H + CIR(Nss, Nrx)
 *
 * @preamble_type:
 *
 *			0 - Legacy
 *			1 - HT
 *			2 - VHT
 *			3 - HE
 *
 * @nss:
 *
 *			0 - 1-stream
 *			1 - 2-stream
 *			..	..
 *			7 - 8-stream
 *
 *@num_chains:
 *
 *			0 - 1-chain
 *			1 - 2-chain
 *			..  ..
 *			7 - 8-chain
 *
 *@upload_bw_pkt:
 *
 *			0 - 20 MHz
 *			1 - 40 MHz
 *			2 - 80 MHz
 *			3 - 160 MHz
 *
 * @sw_peer_id_valid: Indicates whether sw_peer_id field is valid or not,
 * sent from MAC to PHY via the MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * @sw_peer_id: Indicates peer id based on AST search, sent from MAC to PHY
 * via the MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * @phy_ppdu_id: sent from PHY to MAC, copied to MACRX_FREEZE_CAPTURE_CHANNEL
 * TLV
 *
 * @total_bytes: Total size of CFR payload (FFT bins)
 *
 * @header_version:
 *
 *			1 - HKV2/Hastings
 *			2 - Cypress
 *			3 - Hasting Prime
 *			4 - Pine
 *			8 - Hamilton
 *			9 - Waikiki
 *
 * @target_id:
 *
 *			1 - Hastings
 *			2 - Cypress
 *			3 - Hastings Prime
 *			4 - Pine
 *
 * @cfr_fmt:
 *
 *			0 - raw (32-bit format)
 *			1 - compressed (24-bit format)
 *
 * @mu_rx_data_incl: Indicates whether CFR header contains UL-MU-MIMO info
 *
 * @freeze_data_incl: Indicates whether CFR header contains
 * MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * @freeze_tlv_version: Indicates the version of freeze_tlv
 *			1 - HSP, Cypress
 *			2 - Maple/Spruce/Moselle
 *			3 - Pine
 *
 * @decimation_factor: FFT bins decimation
 * @mu_rx_num_users: Number of users in UL-MU-PPDU
 */
struct whal_cfir_enhanced_hdr {
	uint16_t tag              :  8,
		 length           :  6,
		 rsvd1            :  2;

	uint16_t upload_done        :  1,
		 capture_type       :  3,
		 preamble_type      :  2,
		 nss                :  3,
		 num_chains         :  3,
		 upload_pkt_bw      :  3,
		 sw_peer_id_valid   :  1;

	uint16_t sw_peer_id         : 16;

	uint16_t phy_ppdu_id        : 16;

	uint16_t total_bytes;

	uint16_t header_version     :4,
		 target_id          :4,
		 cfr_fmt            :1,
		 rsvd2              :1,
		 mu_rx_data_incl    :1,
		 freeze_data_incl   :1,
		 freeze_tlv_version :4;

	uint16_t mu_rx_num_users   :8,
		 decimation_factor :4,
		 rsvd3             :4;

	uint16_t rsvd4;
};

/*
 * freeze_tlv v1/v2 used by Hastings/Cypress/Maple/Spruce/Moselle supports upto
 * 4 UL MU users
 *
 * @freeze:
 *		0: Allow channel capture
 *		1: Freeze channel capture
 *
 * @capture_reason: Field only valid when the freeze field is 1. Indicates why
 * the MAC asked to capture the channel
 *		0: freeze_reason_TM
 *		1: freeze_reason_FTM
 *		2: freeze_reason_ACK_resp_to_TM_FTM
 *		3: freeze_reason_TA_RA_TYPE_FILTER
 *		4: freeze readon NDP_NDP
 *		5: freeze_reason_ALL_PACKET
 *
 * @packet_type: Packet type of captured packets.
 *		0: Management
 *		1: Control
 *		2: Data
 *		3: Extension
 *
 * @packet_sub_type: packet subtype of the captured packets.
 * @sw_peer_id_valid: It is valid only when the freeze field is set to 1.
 *		0: no TA address search on the received frame has been
 *		   performed. This is due to the frame not having a TA address
 *		   (like ACK frame), or the received frame being from an other
 *		   AP to which this device is not associated.
 *		1: field sw_peer_id will contain valid information.
 *		   This implies that a (successful) address search has been
 *		   performed on the TA address of the received frame.
 *
 * @sw_peer_id: Valid only when sw_peer_id_valid field is set. It is an
 * identifier that allows SW to double check that the CSI info stored belongs
 * to the device with this SW identifier.
 *
 * @phy_ppdu_id: ppdu_id of ppdu which has channel capture performed. Field
 * only valid when the freeze field is set to 1.
 *
 * @packet_ta_lower_16: Packet’s lower 16bits transmit address in MAC header.
 *
 * @packet_ta_mid_16: Packet’s middle 16bits transmit address in MAC header.
 *
 * @packet_ta_upper_16: Packet’s upper 16bits transmit address in MAC header.
 *
 * @packet_ra_lower_16: Packet’s lower 16bits receive address in MAC header.
 *
 * @packet_ra_mid_16: Packet’s middle 16bits receive address in MAC header.
 *
 * @packet_ra_upper_16: Packet’s upper 16bits receive address in MAC header.
 *
 * @tsf_timestamp_15_0: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [15:0].
 *
 * @tsf_timestamp_31_16: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [31:16].
 *
 * @tsf_timestamp_47_32: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [47:32].
 *
 * @tsf_timestamp_63_48: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [63:48].
 *
 * @user_index_or_user_mask_5_0: When freeze_tlv_version is 1, this field from
 * MAC, indicate to PHY which user's channel information need to be uploaded.
 * When freeze_tlv_version is 2 & MU_SUPPORT_IN_TLV is 1, this field indicates
 * bitmap of users upto 4 to which channel capture need to be uploaded. And if
 * freeze_tlv_version is 2 & MU_SUPPORT_IN_TLV is 0, this field indicate to PHY
 * which user's channel information need to be uploaded
 *
 * @directed: Indicate the frame is directed to us or not when NDPA/NDP capture
 * or FTM/TM/ACK capture. 1=directed. For other modes, it is 0.
 */
struct macrx_freeze_capture_channel {
	uint16_t freeze                          :  1, //[0]
		 capture_reason                  :  3, //[3:1]
		 packet_type                     :  2, //[5:4]
		 packet_sub_type                 :  4, //[9:6]
		 reserved                        :  5, //[14:10]
		 sw_peer_id_valid                :  1; //[15]
	uint16_t sw_peer_id                      : 16; //[15:0]
	uint16_t phy_ppdu_id                     : 16; //[15:0]
	uint16_t packet_ta_lower_16              : 16; //[15:0]
	uint16_t packet_ta_mid_16                : 16; //[15:0]
	uint16_t packet_ta_upper_16              : 16; //[15:0]
	uint16_t packet_ra_lower_16              : 16; //[15:0]
	uint16_t packet_ra_mid_16                : 16; //[15:0]
	uint16_t packet_ra_upper_16              : 16; //[15:0]
	uint16_t tsf_timestamp_15_0              : 16; //[15:0]
	uint16_t tsf_timestamp_31_16             : 16; //[15:0]
	uint16_t tsf_timestamp_47_32             : 16; //[15:0]
	uint16_t tsf_timestamp_63_48             : 16; //[15:0]
	uint16_t user_index_or_user_mask_5_0     :  6, //[5:0]
		 directed                        :  1, //[6]
		 reserved_13                     :  9; //[15:7]
};

/*
 * freeze_tlv v3 used by Pine
 *
 * @freeze:
 *		0: Allow channel capture
 *		1: Freeze channel capture
 *
 * @capture_reason: Field only valid when the freeze field is 1. Indicates why
 * the MAC asked to capture the channel
 *		0: freeze_reason_TM
 *		1: freeze_reason_FTM
 *		2: freeze_reason_ACK_resp_to_TM_FTM
 *		3: freeze_reason_TA_RA_TYPE_FILTER
 *		4: freeze readon NDP_NDP
 *		5: freeze_reason_ALL_PACKET
 *
 * @packet_type: Packet type of captured packets.
 *		0: Management
 *		1: Control
 *		2: Data
 *		3: Extension
 *
 * @packet_sub_type: packet subtype of the captured packets.
 *
 * @directed: Indicate the frame is directed to us or not when NDPA/NDP capture
 * or FTM/TM/ACK capture. 1=directed. For other modes, it is 0.
 *
 * @sw_peer_id_valid: It is valid only when the freeze field is set to 1.
 *		0: no TA address search on the received frame has been
 *		   performed. This is due to the frame not having a TA address
 *		   (like ACK frame), or the received frame being from an other
 *		   AP to which this device is not associated.
 *		1: field sw_peer_id will contain valid information.
 *		   This implies that a (successful) address search has been
 *		   performed on the TA address of the received frame.
 *
 * @sw_peer_id: Valid only when sw_peer_id_valid field is set. It is an
 * identifier that allows SW to double check that the CSI info stored belongs
 * to the device with this SW identifier.
 *
 * @phy_ppdu_id: ppdu_id of ppdu which has channel capture performed. Field
 * only valid when the freeze field is set to 1.
 *
 * @packet_ta_lower_16: Packet’s lower 16bits transmit address in MAC header.
 *
 * @packet_ta_mid_16: Packet’s middle 16bits transmit address in MAC header.
 *
 * @packet_ta_upper_16: Packet’s upper 16bits transmit address in MAC header.
 *
 * @packet_ra_lower_16: Packet’s lower 16bits receive address in MAC header.
 *
 * @packet_ra_mid_16: Packet’s middle 16bits receive address in MAC header.
 *
 * @packet_ra_upper_16: Packet’s upper 16bits receive address in MAC header.
 *
 * @tsf_timestamp_15_0: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [15:0].
 *
 * @tsf_timestamp_31_16: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [31:16].
 *
 * @tsf_timestamp_47_32: MAC side 64bit TSF timestamp when this TLV is sent to
 * PHY. Bits [47:32].
 *
 * @tsf_63_48_or_user_mask_36_32: Indicates to PHY which user's channel info
 * need to be uploaded. Only valid in UL MU case with MU_SUPPORT_IN_TLV = 1.
 * Otherwise this indicates to PHY MSBs 63:48 of the MAC side 64bit TSF
 * timestamp when this TLV is sent to PHY.
 *
 * @user_index_or_user_mask_15_0: Indicate to PHY which user's channel info
 * need to be uploaded in UL MU case with MU_SUPPORT_IN_TLV = 1. Otherwise it
 * indicate PHY which user’s channel information need to be uploaded.
 *
 * @user_mask_31_16: Indicate to PHY which user’s channel information need to
 * be uploaded. Only valid in UL MU case with MU_SUPPORT_IN_TLV = 1.
 */
struct macrx_freeze_capture_channel_v3 {
	uint16_t freeze                          :  1, //[0]
		 capture_reason                  :  3, //[3:1]
		 packet_type                     :  2, //[5:4]
		 packet_sub_type                 :  4, //[9:6]
		 directed                        :  1, //[10]
		 reserved                        :  4, //[14:11]
		 sw_peer_id_valid                :  1; //[15]
	uint16_t sw_peer_id                      : 16; //[15:0]
	uint16_t phy_ppdu_id                     : 16; //[15:0]
	uint16_t packet_ta_lower_16              : 16; //[15:0]
	uint16_t packet_ta_mid_16                : 16; //[15:0]
	uint16_t packet_ta_upper_16              : 16; //[15:0]
	uint16_t packet_ra_lower_16              : 16; //[15:0]
	uint16_t packet_ra_mid_16                : 16; //[15:0]
	uint16_t packet_ra_upper_16              : 16; //[15:0]
	uint16_t tsf_timestamp_15_0              : 16; //[15:0]
	uint16_t tsf_timestamp_31_16             : 16; //[15:0]
	uint16_t tsf_timestamp_47_32             : 16; //[15:0]
	uint16_t tsf_63_48_or_user_mask_36_32    : 16; //[15:0]
	uint16_t user_index_or_user_mask_15_0    : 16; //[15:0]
	uint16_t user_mask_31_16                 : 16; //[15:0]
};

struct uplink_user_setup_info {
	uint32_t bw_info_valid                   :  1, //[0]
		 uplink_receive_type             :  2, //[2:1]
		 reserved_0a                     :  1, //[3]
		 uplink_11ax_mcs                 :  4, //[7:4]
		 ru_width                        :  7, //[14:8]
		 reserved_0b                     :  1, //[15]
		 nss                             :  3, //[18:16]
		 stream_offset                   :  3, //[21:19]
		 sta_dcm                         :  1, //[22]
		 sta_coding                      :  1, //[23]
		 ru_start_index                  :  7, //[30:24]
		 reserved_0c                     :  1; //[31]
};

struct macrx_freeze_capture_channel_v5 {
	uint16_t freeze                          :  1, //[0]
		 capture_reason                  :  3, //[3:1]
		 packet_type                     :  2, //[5:4]
		 packet_sub_type                 :  4, //[9:6]
		 reserved                        :  5, //[14:10]
		 sw_peer_id_valid                :  1; //[15]
	uint16_t sw_peer_id                      : 16; //[15:0]
	uint16_t phy_ppdu_id                     : 16; //[15:0]
	uint16_t packet_ta_lower_16              : 16; //[15:0]
	uint16_t packet_ta_mid_16                : 16; //[15:0]
	uint16_t packet_ta_upper_16              : 16; //[15:0]
	uint16_t packet_ra_lower_16              : 16; //[15:0]
	uint16_t packet_ra_mid_16                : 16; //[15:0]
	uint16_t packet_ra_upper_16              : 16; //[15:0]
	uint16_t tsf_timestamp_15_0              : 16; //[15:0]
	uint16_t tsf_timestamp_31_16             : 16; //[15:0]
	uint16_t tsf_timestamp_47_32             : 16; //[15:0]
	uint16_t tsf_timestamp_63_48             : 16; //[15:0]
	uint16_t user_index_or_user_mask_5_0     :  6, //[5:0]
		 directed                        :  1, //[6]
		 reserved_13                     :  9; //[15:7]
	uint16_t user_mask_21_6                  : 16; //[15:0]
	uint16_t user_mask_36_22                 : 15, //[14:0]
		 reserved_15a                    :  1; //[15]
};

struct uplink_user_setup_info_v2 {
	uint32_t bw_info_valid                   :  1, //[0]
		 uplink_receive_type             :  2, //[2:1]
		 reserved_0a                     :  1, //[3]
		 uplink_11ax_mcs                 :  4, //[7:4]
		 nss                             :  3, //[10:8]
		 stream_offset                   :  3, //[13:11]
		 sta_dcm                         :  1, //[14]
		 sta_coding                      :  1, //[15]
		 ru_type_80_0                    :  4, //[19:16]
		 ru_type_80_1                    :  4, //[23:20]
		 ru_type_80_2                    :  4, //[27:24]
		 ru_type_80_3                    :  4; //[31:28]
	uint32_t ru_start_index_80_0             :  6, //[5:0]
		 reserved_1a                     :  2, //[7:6]
		 ru_start_index_80_1             :  6, //[13:8]
		 reserved_1b                     :  2, //[15:14]
		 ru_start_index_80_2             :  6, //[21:16]
		 reserved_1c                     :  2, //[23:22]
		 ru_start_index_80_3             :  6, //[29:24]
		 reserved_1d                     :  2; //[31-30]
};

/**
 * cfr_enh_init_pdev() - Inits cfr pdev and registers necessary handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: Registration status for necessary handlers
 */
QDF_STATUS cfr_enh_init_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev);

/**
 * cfr_enh_deinit_pdev() - De-inits corresponding pdev and handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: De-registration status for necessary handlers
 */
QDF_STATUS cfr_enh_deinit_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_start_lut_age_timer() - Start timer to flush aged-out LUT
 * entries
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_start_lut_age_timer(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_stop_lut_age_timer() - Stop timer to flush aged-out LUT
 * entries
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_stop_lut_age_timer(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_dump_lut_enh() - Dump all valid LUT entries
 * @pdev: objmgr PDEV
 *
 * Return: none
 */
void target_if_cfr_dump_lut_enh(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_config_rcc() - Start repetitive channel capture
 * @pdev: pointer to pdev object
 * @rcc_param: rcc configurations
 *
 * Return: Success/Failure status
 */
QDF_STATUS target_if_cfr_config_rcc(struct wlan_objmgr_pdev *pdev,
				    struct cfr_rcc_param *rcc_param);

/**
 *  target_if_cfr_default_ta_ra_config() - Configure default values to all
 *  params(BW/NSS/TA/RA) in TA_RA mode
 * @rcc_param: rcc configurations
 * @allvalid: Indicates whether all TA_RA params are valid or not.
 *            It could be either 0 or 1.
 *            1: should be sent to FW during CFR initialization
 *            0: should be set, after a successful commit session.
 * @reset_cfg: This bitmap is being used to determine which groups'
 *            parameters are needed to be reset to its default state.
 */
void target_if_cfr_default_ta_ra_config(struct cfr_rcc_param *rcc_param,
					bool allvalid, uint16_t reset_cfg);

/**
 * target_if_cfr_rx_tlv_process() - Process PPDU status TLVs and store info in
 * lookup table
 * @pdev: PDEV object
 * @nbuf: ppdu info
 *
 * Return: none
 */
void target_if_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf);

/**
 * target_if_cfr_update_global_cfg() - Update global config after a successful
 * commit
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_update_global_cfg(struct wlan_objmgr_pdev *pdev);
#else
static inline QDF_STATUS cfr_enh_init_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS cfr_enh_deinit_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif

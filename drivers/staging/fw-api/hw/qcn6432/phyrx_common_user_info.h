/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _PHYRX_COMMON_USER_INFO_H_
#define _PHYRX_COMMON_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_COMMON_USER_INFO 4

#define NUM_OF_QWORDS_PHYRX_COMMON_USER_INFO 2


struct phyrx_common_user_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t receive_duration                                        : 16, // [15:0]
                      reserved_0a                                             : 16; // [31:16]
             uint32_t u_sig_puncture_pattern_encoding                         :  6, // [5:0]
                      reserved_1a                                             : 26; // [31:6]
             uint32_t eht_ppdu_type                                           :  2, // [1:0]
                      bss_color_id                                            :  6, // [7:2]
                      dl_ul_flag                                              :  1, // [8:8]
                      txop_duration                                           :  7, // [15:9]
                      cp_setting                                              :  2, // [17:16]
                      ltf_size                                                :  2, // [19:18]
                      spatial_reuse                                           :  4, // [23:20]
                      rx_ndp                                                  :  1, // [24:24]
                      dot11be_su_extended                                     :  1, // [25:25]
                      reserved_2a                                             :  6; // [31:26]
             uint32_t eht_duplicate                                           :  2, // [1:0]
                      eht_sig_cmn_field_type                                  :  2, // [3:2]
                      doppler_indication                                      :  1, // [4:4]
                      sta_id                                                  : 11, // [15:5]
                      puncture_bitmap                                         : 16; // [31:16]
#else
             uint32_t reserved_0a                                             : 16, // [31:16]
                      receive_duration                                        : 16; // [15:0]
             uint32_t reserved_1a                                             : 26, // [31:6]
                      u_sig_puncture_pattern_encoding                         :  6; // [5:0]
             uint32_t reserved_2a                                             :  6, // [31:26]
                      dot11be_su_extended                                     :  1, // [25:25]
                      rx_ndp                                                  :  1, // [24:24]
                      spatial_reuse                                           :  4, // [23:20]
                      ltf_size                                                :  2, // [19:18]
                      cp_setting                                              :  2, // [17:16]
                      txop_duration                                           :  7, // [15:9]
                      dl_ul_flag                                              :  1, // [8:8]
                      bss_color_id                                            :  6, // [7:2]
                      eht_ppdu_type                                           :  2; // [1:0]
             uint32_t puncture_bitmap                                         : 16, // [31:16]
                      sta_id                                                  : 11, // [15:5]
                      doppler_indication                                      :  1, // [4:4]
                      eht_sig_cmn_field_type                                  :  2, // [3:2]
                      eht_duplicate                                           :  2; // [1:0]
#endif
};


/* Description		RECEIVE_DURATION

			The remaining receive duration of this PPDU in the medium
			 (in us). 
			The timing reference point is the assertion of 'rx_frame' 
			by PHY for the PPDU reception. The value shall be accurate
			 to within 2us.
			RXPCU shall subtract the time elapsed between 'rx_frame' 
			assertion and reception of this TLV to find the actual remaining
			 receive duration.
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_OFFSET                              0x0000000000000000
#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_LSB                                 0
#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_MSB                                 15
#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_MASK                                0x000000000000ffff


/* Description		RESERVED_0A

			<legal 0>
*/

#define PHYRX_COMMON_USER_INFO_RESERVED_0A_OFFSET                                   0x0000000000000000
#define PHYRX_COMMON_USER_INFO_RESERVED_0A_LSB                                      16
#define PHYRX_COMMON_USER_INFO_RESERVED_0A_MSB                                      31
#define PHYRX_COMMON_USER_INFO_RESERVED_0A_MASK                                     0x00000000ffff0000


/* Description		U_SIG_PUNCTURE_PATTERN_ENCODING

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			The 6-bit value used in U-SIG and/or EHT-SIG Common field
			 for the puncture pattern
			<legal 0-29>
*/

#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET               0x0000000000000000
#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                  32
#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                  37
#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                 0x0000003f00000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define PHYRX_COMMON_USER_INFO_RESERVED_1A_OFFSET                                   0x0000000000000000
#define PHYRX_COMMON_USER_INFO_RESERVED_1A_LSB                                      38
#define PHYRX_COMMON_USER_INFO_RESERVED_1A_MSB                                      63
#define PHYRX_COMMON_USER_INFO_RESERVED_1A_MASK                                     0xffffffc000000000


/* Description		EHT_PPDU_TYPE

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			<enum 0 EHT_PPDU_rsvd> DO NOT USE
			<enum 1 EHT_PPDU_TB>
			<enum 2 EHT_PPDU_MU> Need to look at both EHT-SIG content
			 channels
			<enum 3 EHT_PPDU_SU> Need to look at only one EHT-SIG content
			 channel
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_LSB                                    0
#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_MSB                                    1
#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_MASK                                   0x0000000000000003


/* Description		BSS_COLOR_ID

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			BSS color ID
			
			Field used by MAC HW
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_OFFSET                                  0x0000000000000008
#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_LSB                                     2
#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_MSB                                     7
#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_MASK                                    0x00000000000000fc


/* Description		DL_UL_FLAG

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			Differentiates between DL and UL transmission
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_OFFSET                                    0x0000000000000008
#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_LSB                                       8
#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_MSB                                       8
#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_MASK                                      0x0000000000000100


/* Description		TXOP_DURATION

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			Indicates the remaining time in the current TXOP
			
			Field used by MAC HW
			 <legal all>
*/

#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_LSB                                    9
#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_MSB                                    15
#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_MASK                                   0x000000000000fe00


/* Description		CP_SETTING

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			<enum 0     gi_0_8_us > Legacy normal GI
			<enum 1     gi_0_4_us > Legacy short GI
			<enum 2     gi_1_6_us > HE related GI
			<enum 3     gi_3_2_us > HE related GI
			<legal 0 - 3>
*/

#define PHYRX_COMMON_USER_INFO_CP_SETTING_OFFSET                                    0x0000000000000008
#define PHYRX_COMMON_USER_INFO_CP_SETTING_LSB                                       16
#define PHYRX_COMMON_USER_INFO_CP_SETTING_MSB                                       17
#define PHYRX_COMMON_USER_INFO_CP_SETTING_MASK                                      0x0000000000030000


/* Description		LTF_SIZE

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			Ltf size
			
			<enum 0     ltf_1x > 
			<enum 1     ltf_2x > 
			<enum 2     ltf_4x > 
			<legal 0 - 2>
*/

#define PHYRX_COMMON_USER_INFO_LTF_SIZE_OFFSET                                      0x0000000000000008
#define PHYRX_COMMON_USER_INFO_LTF_SIZE_LSB                                         18
#define PHYRX_COMMON_USER_INFO_LTF_SIZE_MSB                                         19
#define PHYRX_COMMON_USER_INFO_LTF_SIZE_MASK                                        0x00000000000c0000


/* Description		SPATIAL_REUSE

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			TODO: Placeholder
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_LSB                                    20
#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_MSB                                    23
#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_MASK                                   0x0000000000f00000


/* Description		RX_NDP

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			On RX side, looked at by MAC HW
			
			When set, PHY has received an (expected) NDP frame
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_RX_NDP_OFFSET                                        0x0000000000000008
#define PHYRX_COMMON_USER_INFO_RX_NDP_LSB                                           24
#define PHYRX_COMMON_USER_INFO_RX_NDP_MSB                                           24
#define PHYRX_COMMON_USER_INFO_RX_NDP_MASK                                          0x0000000001000000


/* Description		DOT11BE_SU_EXTENDED

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			On RX side, evaluated by MAC HW
			
			This is the only way for MAC RX to know that this was a 
			U_SIG_EHT_SU received in extended range format.
			
			When set, the 11be frame is of the extended range format.
			
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_OFFSET                           0x0000000000000008
#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_LSB                              25
#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_MSB                              25
#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_MASK                             0x0000000002000000


/* Description		RESERVED_2A

			<legal 0>
*/

#define PHYRX_COMMON_USER_INFO_RESERVED_2A_OFFSET                                   0x0000000000000008
#define PHYRX_COMMON_USER_INFO_RESERVED_2A_LSB                                      26
#define PHYRX_COMMON_USER_INFO_RESERVED_2A_MSB                                      31
#define PHYRX_COMMON_USER_INFO_RESERVED_2A_MASK                                     0x00000000fc000000


/* Description		EHT_DUPLICATE

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			Indicates EHT duplicate modulation
			
			<enum 0 eht_no_duplicate>
			<enum 1 eht_2x_duplicate>
			<enum 2 eht_4x_duplicate>
			
			<legal 0-2>
*/

#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_LSB                                    32
#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_MSB                                    33
#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_MASK                                   0x0000000300000000


/* Description		EHT_SIG_CMN_FIELD_TYPE

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			Indicates the type of EHT-SIG Common field
			
			<enum 0 EHT_SIG_CMN_NO_PUNC> Non-OFDMA, EHT-SIG Common field
			 does not contain puncturing information
			<enum 1 EHT_SIG_CMN_PUNC> Non-OFDMA, EHT-SIG Common field
			 contains puncturing information
			<enum 2 EHT_SIG_OFDMA>
			OFDMA, EHT-SIG Common field contains RU structure
			<legal 0-2>
*/

#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_OFFSET                        0x0000000000000008
#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_LSB                           34
#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_MSB                           35
#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_MASK                          0x0000000c00000000


/* Description		DOPPLER_INDICATION

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV.
			
			
			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_OFFSET                            0x0000000000000008
#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_LSB                               36
#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_MSB                               36
#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_MASK                              0x0000001000000000


/* Description		STA_ID

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV
			 and EHT_PPDU_type is EHT_PPDU_MU (MU-MIMO or OFDMA).
			
			Identifies the STA that is addressed. Details of STA ID 
			are TBD
*/

#define PHYRX_COMMON_USER_INFO_STA_ID_OFFSET                                        0x0000000000000008
#define PHYRX_COMMON_USER_INFO_STA_ID_LSB                                           37
#define PHYRX_COMMON_USER_INFO_STA_ID_MSB                                           47
#define PHYRX_COMMON_USER_INFO_STA_ID_MASK                                          0x0000ffe000000000


/* Description		PUNCTURE_BITMAP

			Field only valid if PHY has sent 'PHYRX_GENERIC_U_SIG' TLV
			 and EHT_SIG_CMN_field_type is EHT_SIG_CMN_PUNC.
			
			Indicates which 20 MHz sub-bands will be modulated vs punctured
			 (bits [15:0]) in CCA order (primary/secondary)
			
			Bit 0: primary 20MHz sub-band
			Bit 1: secondary 20 MHz sub-band
			Bit 2: first 20 MHz sub-band in secondary 40 MHz
			Bit 3: second 20 MHz sub-band in secondary 40 MHz
			... 
			Bit 15: last 20MHz sub-band in secondary 160 MHz
			A value of 0 means the band is punctured
			A value of 1 means the band is modulated
			
			If the PPDU BW is less than 320 MHz, the MSB bits are reserved
			 and set to 0.
*/

#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_OFFSET                               0x0000000000000008
#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_LSB                                  48
#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_MSB                                  63
#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_MASK                                 0xffff000000000000



#endif   // PHYRX_COMMON_USER_INFO

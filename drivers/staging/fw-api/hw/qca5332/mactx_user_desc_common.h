
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _MACTX_USER_DESC_COMMON_H_
#define _MACTX_USER_DESC_COMMON_H_
#if !defined(__ASSEMBLER__)
#endif

#include "unallocated_ru_160_info.h"
#include "ru_allocation_160_info.h"
#define NUM_OF_DWORDS_MACTX_USER_DESC_COMMON 16

#define NUM_OF_QWORDS_MACTX_USER_DESC_COMMON 8


struct mactx_user_desc_common {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t num_users                                               :  6, // [5:0]
                      reserved_0b                                             :  5, // [10:6]
                      ltf_size                                                :  2, // [12:11]
                      reserved_0c                                             :  3, // [15:13]
                      he_stf_long                                             :  1, // [16:16]
                      reserved_0d                                             :  7, // [23:17]
                      num_users_he_sigb_band0                                 :  8; // [31:24]
             uint32_t num_ltf_symbols                                         :  3, // [2:0]
                      reserved_1a                                             :  5, // [7:3]
                      num_users_he_sigb_band1                                 :  8, // [15:8]
                      reserved_1b                                             : 16; // [31:16]
             uint32_t packet_extension_a_factor                               :  2, // [1:0]
                      packet_extension_pe_disambiguity                        :  1, // [2:2]
                      packet_extension                                        :  3, // [5:3]
                      reserved                                                :  2, // [7:6]
                      he_sigb_dcm                                             :  1, // [8:8]
                      reserved_2b                                             :  7, // [15:9]
                      he_sigb_compression                                     :  1, // [16:16]
                      reserved_2c                                             : 15; // [31:17]
             uint32_t he_sigb_0_mcs                                           :  3, // [2:0]
                      reserved_3a                                             : 13, // [15:3]
                      num_he_sigb_sym                                         :  5, // [20:16]
                      center_ru_0                                             :  1, // [21:21]
                      center_ru_1                                             :  1, // [22:22]
                      reserved_3b                                             :  1, // [23:23]
                      ftm_en                                                  :  1, // [24:24]
                      pe_nss                                                  :  3, // [27:25]
                      pe_ltf_size                                             :  2, // [29:28]
                      pe_content                                              :  1, // [30:30]
                      pe_chain_csd_en                                         :  1; // [31:31]
             struct   ru_allocation_160_info                                    ru_allocation_0123_details;
             struct   ru_allocation_160_info                                    ru_allocation_4567_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_0_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_1_details;
             uint32_t num_data_symbols                                        : 16, // [15:0]
                      ndp_ru_tone_set_index                                   :  7, // [22:16]
                      ndp_feedback_status                                     :  1, // [23:23]
                      doppler_indication                                      :  1, // [24:24]
                      reserved_14a                                            :  7; // [31:25]
             uint32_t spatial_reuse                                           : 16, // [15:0]
                      reserved_15a                                            : 16; // [31:16]
#else
             uint32_t num_users_he_sigb_band0                                 :  8, // [31:24]
                      reserved_0d                                             :  7, // [23:17]
                      he_stf_long                                             :  1, // [16:16]
                      reserved_0c                                             :  3, // [15:13]
                      ltf_size                                                :  2, // [12:11]
                      reserved_0b                                             :  5, // [10:6]
                      num_users                                               :  6; // [5:0]
             uint32_t reserved_1b                                             : 16, // [31:16]
                      num_users_he_sigb_band1                                 :  8, // [15:8]
                      reserved_1a                                             :  5, // [7:3]
                      num_ltf_symbols                                         :  3; // [2:0]
             uint32_t reserved_2c                                             : 15, // [31:17]
                      he_sigb_compression                                     :  1, // [16:16]
                      reserved_2b                                             :  7, // [15:9]
                      he_sigb_dcm                                             :  1, // [8:8]
                      reserved                                                :  2, // [7:6]
                      packet_extension                                        :  3, // [5:3]
                      packet_extension_pe_disambiguity                        :  1, // [2:2]
                      packet_extension_a_factor                               :  2; // [1:0]
             uint32_t pe_chain_csd_en                                         :  1, // [31:31]
                      pe_content                                              :  1, // [30:30]
                      pe_ltf_size                                             :  2, // [29:28]
                      pe_nss                                                  :  3, // [27:25]
                      ftm_en                                                  :  1, // [24:24]
                      reserved_3b                                             :  1, // [23:23]
                      center_ru_1                                             :  1, // [22:22]
                      center_ru_0                                             :  1, // [21:21]
                      num_he_sigb_sym                                         :  5, // [20:16]
                      reserved_3a                                             : 13, // [15:3]
                      he_sigb_0_mcs                                           :  3; // [2:0]
             struct   ru_allocation_160_info                                    ru_allocation_0123_details;
             struct   ru_allocation_160_info                                    ru_allocation_4567_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_0_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_1_details;
             uint32_t reserved_14a                                            :  7, // [31:25]
                      doppler_indication                                      :  1, // [24:24]
                      ndp_feedback_status                                     :  1, // [23:23]
                      ndp_ru_tone_set_index                                   :  7, // [22:16]
                      num_data_symbols                                        : 16; // [15:0]
             uint32_t reserved_15a                                            : 16, // [31:16]
                      spatial_reuse                                           : 16; // [15:0]
#endif
};


/* Description		NUM_USERS

			The number of users in this transmission
			
			Use this same field for HE-ranging NDP as well.
			<legal 1-37>
*/

#define MACTX_USER_DESC_COMMON_NUM_USERS_OFFSET                                     0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_USERS_LSB                                        0
#define MACTX_USER_DESC_COMMON_NUM_USERS_MSB                                        5
#define MACTX_USER_DESC_COMMON_NUM_USERS_MASK                                       0x000000000000003f


/* Description		RESERVED_0B

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_0B_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_0B_LSB                                      6
#define MACTX_USER_DESC_COMMON_RESERVED_0B_MSB                                      10
#define MACTX_USER_DESC_COMMON_RESERVED_0B_MASK                                     0x00000000000007c0


/* Description		LTF_SIZE

			Ltf size
			
			Specify the right LTF-size for HE-Ranging NDPs (11az)/Short-NDP.
			
			
			<enum 0     ltf_1x > 
			<enum 1     ltf_2x > 
			<enum 2     ltf_4x > 
			<legal 0 - 2>
*/

#define MACTX_USER_DESC_COMMON_LTF_SIZE_OFFSET                                      0x0000000000000000
#define MACTX_USER_DESC_COMMON_LTF_SIZE_LSB                                         11
#define MACTX_USER_DESC_COMMON_LTF_SIZE_MSB                                         12
#define MACTX_USER_DESC_COMMON_LTF_SIZE_MASK                                        0x0000000000001800


/* Description		RESERVED_0C

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_0C_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_0C_LSB                                      13
#define MACTX_USER_DESC_COMMON_RESERVED_0C_MSB                                      15
#define MACTX_USER_DESC_COMMON_RESERVED_0C_MASK                                     0x000000000000e000


/* Description		HE_STF_LONG

			0: Normal HE STF. 
			1: Long HE STF
			
			Specify the right STF-size for HE-Ranging NDPs (11az)/Short-NDP.
			
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_HE_STF_LONG_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_HE_STF_LONG_LSB                                      16
#define MACTX_USER_DESC_COMMON_HE_STF_LONG_MSB                                      16
#define MACTX_USER_DESC_COMMON_HE_STF_LONG_MASK                                     0x0000000000010000


/* Description		RESERVED_0D

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_0D_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_0D_LSB                                      17
#define MACTX_USER_DESC_COMMON_RESERVED_0D_MSB                                      23
#define MACTX_USER_DESC_COMMON_RESERVED_0D_MASK                                     0x0000000000fe0000


/* Description		NUM_USERS_HE_SIGB_BAND0

			number of users in HE_SIGB_0 or EHT_SIG_0
			
			Note for MAC:
			directly from pdg_fes_setup, based on BW
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_OFFSET                       0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_LSB                          24
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_MSB                          31
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_MASK                         0x00000000ff000000


/* Description		NUM_LTF_SYMBOLS

			Indicates the number of HE-LTF symbols
			
			0: 1 symbol
			1: 2 symbols
			2: 3 symbols
			3: 4 symbols
			4: 5 symbols
			5: 6 symbols
			6: 7 symbols
			7: 8 symbols
			
			NOTE that this encoding is different from what is in "Num_LTF_symbols" 
			in the HE_SIG_A_MU_DL.
			
			NOTE 2: Not used for HE-Ranging NDPs (11az)
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_OFFSET                               0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_LSB                                  32
#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_MSB                                  34
#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_MASK                                 0x0000000700000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_1A_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_1A_LSB                                      35
#define MACTX_USER_DESC_COMMON_RESERVED_1A_MSB                                      39
#define MACTX_USER_DESC_COMMON_RESERVED_1A_MASK                                     0x000000f800000000


/* Description		NUM_USERS_HE_SIGB_BAND1

			number of users in HE_SIGB_1 or EHT_SIG_1
			
			Note for MAC:
			directly from pdg_fes_setup, based on BW
			For 20Mhz transmission, this is set to 0
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_OFFSET                       0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_LSB                          40
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_MSB                          47
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_MASK                         0x0000ff0000000000


/* Description		RESERVED_1B

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_1B_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_1B_LSB                                      48
#define MACTX_USER_DESC_COMMON_RESERVED_1B_MSB                                      63
#define MACTX_USER_DESC_COMMON_RESERVED_1B_MASK                                     0xffff000000000000


/* Description		PACKET_EXTENSION_A_FACTOR

			the packet extension duration of the trigger-based PPDU 
			response with these two bits indicating the "a-factor" 
			
			<enum 0 a_factor_4>
			<enum 1 a_factor_1>
			<enum 2 a_factor_2>
			<enum 3 a_factor_3>
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_OFFSET                     0x0000000000000008
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_LSB                        0
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_MSB                        1
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_MASK                       0x0000000000000003


/* Description		PACKET_EXTENSION_PE_DISAMBIGUITY

			the packet extension duration of the trigger-based PPDU 
			response with this bit indicating the PE-Disambiguity 
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET              0x0000000000000008
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                 2
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                 2
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                0x0000000000000004


/* Description		PACKET_EXTENSION

			Packet extension size
			
			Specify the right packet extension size for HE-Ranging NDPs
			 (11az)/Short-NDP.
			<enum 0     packet_ext_0>
			<enum 1     packet_ext_4>
			<enum 2     packet_ext_8>
			<enum 3     packet_ext_12>
			<enum 4     packet_ext_16>
			<enum 5     packet_ext_20>
			<legal 0 - 5>
*/

#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_OFFSET                              0x0000000000000008
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_LSB                                 3
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_MSB                                 5
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_MASK                                0x0000000000000038


/* Description		RESERVED

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_OFFSET                                      0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_LSB                                         6
#define MACTX_USER_DESC_COMMON_RESERVED_MSB                                         7
#define MACTX_USER_DESC_COMMON_RESERVED_MASK                                        0x00000000000000c0


/* Description		HE_SIGB_DCM

			Indicates whether dual sub-carrier modulation is applied
			 to HE-SIG-B or EHT-SIG
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_LSB                                      8
#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_MSB                                      8
#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_MASK                                     0x0000000000000100


/* Description		RESERVED_2B

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_2B_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_2B_LSB                                      9
#define MACTX_USER_DESC_COMMON_RESERVED_2B_MSB                                      15
#define MACTX_USER_DESC_COMMON_RESERVED_2B_MASK                                     0x000000000000fe00


/* Description		HE_SIGB_COMPRESSION

			Indicates the compression mode of HE-SIG-B or EHT-SIG
			<legal all> 
*/

#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_OFFSET                           0x0000000000000008
#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_LSB                              16
#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_MSB                              16
#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_MASK                             0x0000000000010000


/* Description		RESERVED_2C

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_2C_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_2C_LSB                                      17
#define MACTX_USER_DESC_COMMON_RESERVED_2C_MSB                                      31
#define MACTX_USER_DESC_COMMON_RESERVED_2C_MASK                                     0x00000000fffe0000


/* Description		HE_SIGB_0_MCS

			Note: stbc setting is indicated in the MACTX_PHY_DESC.
			
			Indicates the MCS of HE-SIG-B or EHT-SIG.
			
			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_OFFSET                                 0x0000000000000008
#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_LSB                                    32
#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_MSB                                    34
#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_MASK                                   0x0000000700000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_3A_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_3A_LSB                                      35
#define MACTX_USER_DESC_COMMON_RESERVED_3A_MSB                                      47
#define MACTX_USER_DESC_COMMON_RESERVED_3A_MASK                                     0x0000fff800000000


/* Description		NUM_HE_SIGB_SYM

			This field is valid only when (MACTX_PHY_DESC.pkt_type==11ax
			 or MACTX_PHY_DESC.pkt_type == 11be)
			
			Indicates the number of HE-SIG-B or EHT-SIG symbols
			
			This field is 0-based with 0 indicating that 1 he_sigb/eht_sig
			 symbol needs to be transmitted
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_OFFSET                               0x0000000000000008
#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_LSB                                  48
#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_MSB                                  52
#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_MASK                                 0x001f000000000000


/* Description		CENTER_RU_0

			Field only valid for 11ax transmission with a BW of 80Mhz
			 or 160 Mhz
			
			Indicates whether the Center RU is occupied in the lower
			 80 MHz band. This is part of HE_SIGB content channel 1
			
			0: center RU is NOT used
			1: center RU is used
			
			NOTE: EHT is not expected to use the center RU.
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_CENTER_RU_0_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_CENTER_RU_0_LSB                                      53
#define MACTX_USER_DESC_COMMON_CENTER_RU_0_MSB                                      53
#define MACTX_USER_DESC_COMMON_CENTER_RU_0_MASK                                     0x0020000000000000


/* Description		CENTER_RU_1

			Field only valid for 11ax transmission with a BW of 160 
			Mhz (or 80 + 80)
			
			Indicates whether the Center RU is occupied in the upper
			 80 MHz band. This is part of HE_SIGB content channel 1
			
			0: center RU is NOT used
			1: center RU is used
			
			NOTE: EHT is not expected to use the center RU.
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_CENTER_RU_1_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_CENTER_RU_1_LSB                                      54
#define MACTX_USER_DESC_COMMON_CENTER_RU_1_MSB                                      54
#define MACTX_USER_DESC_COMMON_CENTER_RU_1_MASK                                     0x0040000000000000


/* Description		RESERVED_3B

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_3B_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_3B_LSB                                      55
#define MACTX_USER_DESC_COMMON_RESERVED_3B_MSB                                      55
#define MACTX_USER_DESC_COMMON_RESERVED_3B_MASK                                     0x0080000000000000


/* Description		FTM_EN

			This field is set to 1 if the present packet is either an
			 FTM_1 or an FTM_2 packet or an HE-Ranging NDP (11az).
			
			0: non-FTM frame
			1: FTM or HE-Ranging NDP Frame
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_FTM_EN_OFFSET                                        0x0000000000000008
#define MACTX_USER_DESC_COMMON_FTM_EN_LSB                                           56
#define MACTX_USER_DESC_COMMON_FTM_EN_MSB                                           56
#define MACTX_USER_DESC_COMMON_FTM_EN_MASK                                          0x0100000000000000


/* Description		PE_NSS

			Number of active spatial streams during packet extension.
			
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define MACTX_USER_DESC_COMMON_PE_NSS_OFFSET                                        0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_NSS_LSB                                           57
#define MACTX_USER_DESC_COMMON_PE_NSS_MSB                                           59
#define MACTX_USER_DESC_COMMON_PE_NSS_MASK                                          0x0e00000000000000


/* Description		PE_LTF_SIZE

			LTF size to be used during packet extention. . This field
			 is valid for both FTM and non-FTM packets.
			0-1x
			1-2x (unsupported un HWK-1)
			2-4x (unsupported un HWK-1) 
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_LSB                                      60
#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_MSB                                      61
#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_MASK                                     0x3000000000000000


/* Description		PE_CONTENT

			Content of packet extension. Valid for all 11ax packets 
			having packet extension
			
			0-he_ltf, 1-last_data_symbol
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_PE_CONTENT_OFFSET                                    0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_CONTENT_LSB                                       62
#define MACTX_USER_DESC_COMMON_PE_CONTENT_MSB                                       62
#define MACTX_USER_DESC_COMMON_PE_CONTENT_MASK                                      0x4000000000000000


/* Description		PE_CHAIN_CSD_EN

			This field denotes whether to apply CSD on the packet extension
			 portion of the packet. This field is valid for all 11ax
			 packets.
			0: disable per-chain csd
			1: enable per-chain csd
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_OFFSET                               0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_LSB                                  63
#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_MSB                                  63
#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_MASK                                 0x8000000000000000


/* Description		RU_ALLOCATION_0123_DETAILS

			See detailed description of the STRUCT.
*/


/* Description		RU_ALLOCATION_BAND0_0

			Field not used for MIMO
			
			Indicates RU arrangement in frequency domain. RU allocated
			 for MU-MIMO, and number of users in the MU-MIMO.
			0 - valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320
			1 - valid for HE_80/HE_160 (80+80)/ EHT_80/EHT_160/EHT_240/EHT_320
			
			2 - valid for HE_160 (80+80)/ EHT_160/EHT_240/EHT_320
			3 - valid for HE_160 (80+80)/ EHT_160/EHT_240/EHT_320
			
			The four bands are for HE_SIGB0 & B1 respectively or for
			 EHT_SIG0, EHT_SIG1, EHT_SIG2 & EHT_SIG3 respectively.
			
			valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320 packets and denotes RU-map of the first 
			20MHz band of HE_SIGB0 or EHT_SIG0
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_MASK 0x00000000000001ff


/* Description		RU_ALLOCATION_BAND0_1

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_80/HE_160/ EHT_80/EHT_160/EHT_240/EHT_320 packets
			 and denotes RU-map of the second 20MHz band of HE_SIGB0
			 or EHT_SIG0
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_MASK 0x000000000003fe00


/* Description		RESERVED_0A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_OFFSET        0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_MSB           23
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_MASK          0x0000000000fc0000


/* Description		RU_ALLOCATIONS_01_SUBBAND80_MASK

			Field not used for HE
			
			Indicates what 80 MHz subbands 'ru_allocation_band{0, 1}_{0, 
			1}' are valid for
			Bit 0: lowest 80 MHz
			Bit 1: 2nd lowest 80 MHz
			Bit 2: 2nd highest 80 MHz
			Bit 3: highest 80 MHz
			
			In other 80 MHz subbands PHY microcode should override these
			 with 'zero-user RU996.'
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_LSB 24
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MSB 27
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MASK 0x000000000f000000


/* Description		RU_ALLOCATIONS_23_SUBBAND80_MASK

			Field not used for HE
			
			Indicates what 80 MHz subbands 'ru_allocation_band{0, 1}_{2, 
			3}' are valid for
			Bit 0: lowest 80 MHz
			Bit 1: 2nd lowest 80 MHz
			Bit 2: 2nd highest 80 MHz
			Bit 3: highest 80 MHz
			
			In other 80 MHz subbands PHY microcode should override these
			 with 'zero-user RU996.'
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_LSB 28
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MSB 31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MASK 0x00000000f0000000


/* Description		RU_ALLOCATION_BAND0_2

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the third 20MHz band of HE_SIGB0 or EHT_SIG0
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_MASK 0x000001ff00000000


/* Description		RU_ALLOCATION_BAND0_3

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the fourth 20MHz band of HE_SIGB0 or EHT_SIG0
			
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_MASK 0x0003fe0000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_OFFSET        0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_MASK          0xfffc000000000000


/* Description		RU_ALLOCATION_BAND1_0

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_40/HE_80/HE_160/ EHT_40/EHT_80/EHT_160/ EHT_240/EHT_320
			 packets and denotes RU-map of the first 20MHz band of HE_SIGB1
			 or EHT_SIG1
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_MASK 0x00000000000001ff


/* Description		RU_ALLOCATION_BAND1_1

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_80/HE_160/ EHT_80/EHT_160/EHT_240/EHT_320 packets
			 and denotes RU-map of the second 20MHz band of HE_SIGB1
			 or EHT_SIG1
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_MASK 0x000000000003fe00


/* Description		RESERVED_2A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_OFFSET        0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_MSB           31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_MASK          0x00000000fffc0000


/* Description		RU_ALLOCATION_BAND1_2

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the third 20MHz band of HE_SIGB1 or EHT_SIG1
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_MASK 0x000001ff00000000


/* Description		RU_ALLOCATION_BAND1_3

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the fourth 20MHz band of HE_SIGB1 or EHT_SIG1
			
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_MASK 0x0003fe0000000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_OFFSET        0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_MASK          0xfffc000000000000


/* Description		RU_ALLOCATION_4567_DETAILS

			See detailed description of the STRUCT.
			
			Valid for EHT_240/EHT_320 packets and denotes RU-map of 
			the fifth/sixth/sevent/eighth 20MHz bands of EHT_SIG0/EHT_SIG1
			
*/


/* Description		RU_ALLOCATION_BAND0_0

			Field not used for MIMO
			
			Indicates RU arrangement in frequency domain. RU allocated
			 for MU-MIMO, and number of users in the MU-MIMO.
			0 - valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320
			1 - valid for HE_80/HE_160 (80+80)/ EHT_80/EHT_160/EHT_240/EHT_320
			
			2 - valid for HE_160 (80+80)/ EHT_160/EHT_240/EHT_320
			3 - valid for HE_160 (80+80)/ EHT_160/EHT_240/EHT_320
			
			The four bands are for HE_SIGB0 & B1 respectively or for
			 EHT_SIG0, EHT_SIG1, EHT_SIG2 & EHT_SIG3 respectively.
			
			valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320 packets and denotes RU-map of the first 
			20MHz band of HE_SIGB0 or EHT_SIG0
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_MASK 0x00000000000001ff


/* Description		RU_ALLOCATION_BAND0_1

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_80/HE_160/ EHT_80/EHT_160/EHT_240/EHT_320 packets
			 and denotes RU-map of the second 20MHz band of HE_SIGB0
			 or EHT_SIG0
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_MASK 0x000000000003fe00


/* Description		RESERVED_0A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_OFFSET        0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_MSB           23
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_MASK          0x0000000000fc0000


/* Description		RU_ALLOCATIONS_01_SUBBAND80_MASK

			Field not used for HE
			
			Indicates what 80 MHz subbands 'ru_allocation_band{0, 1}_{0, 
			1}' are valid for
			Bit 0: lowest 80 MHz
			Bit 1: 2nd lowest 80 MHz
			Bit 2: 2nd highest 80 MHz
			Bit 3: highest 80 MHz
			
			In other 80 MHz subbands PHY microcode should override these
			 with 'zero-user RU996.'
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_LSB 24
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MSB 27
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MASK 0x000000000f000000


/* Description		RU_ALLOCATIONS_23_SUBBAND80_MASK

			Field not used for HE
			
			Indicates what 80 MHz subbands 'ru_allocation_band{0, 1}_{2, 
			3}' are valid for
			Bit 0: lowest 80 MHz
			Bit 1: 2nd lowest 80 MHz
			Bit 2: 2nd highest 80 MHz
			Bit 3: highest 80 MHz
			
			In other 80 MHz subbands PHY microcode should override these
			 with 'zero-user RU996.'
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_LSB 28
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MSB 31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MASK 0x00000000f0000000


/* Description		RU_ALLOCATION_BAND0_2

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the third 20MHz band of HE_SIGB0 or EHT_SIG0
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_MASK 0x000001ff00000000


/* Description		RU_ALLOCATION_BAND0_3

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the fourth 20MHz band of HE_SIGB0 or EHT_SIG0
			
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_MASK 0x0003fe0000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_OFFSET        0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_MASK          0xfffc000000000000


/* Description		RU_ALLOCATION_BAND1_0

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_40/HE_80/HE_160/ EHT_40/EHT_80/EHT_160/ EHT_240/EHT_320
			 packets and denotes RU-map of the first 20MHz band of HE_SIGB1
			 or EHT_SIG1
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_MASK 0x00000000000001ff


/* Description		RU_ALLOCATION_BAND1_1

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_80/HE_160/ EHT_80/EHT_160/EHT_240/EHT_320 packets
			 and denotes RU-map of the second 20MHz band of HE_SIGB1
			 or EHT_SIG1
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_MASK 0x000000000003fe00


/* Description		RESERVED_2A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_OFFSET        0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_MSB           31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_MASK          0x00000000fffc0000


/* Description		RU_ALLOCATION_BAND1_2

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the third 20MHz band of HE_SIGB1 or EHT_SIG1
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_MASK 0x000001ff00000000


/* Description		RU_ALLOCATION_BAND1_3

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the fourth 20MHz band of HE_SIGB1 or EHT_SIG1
			
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_MASK 0x0003fe0000000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_OFFSET        0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_MASK          0xfffc000000000000


/* Description		RU_ALLOCATION_160_0_DETAILS

			See detailed description of the STRUCT.
*/


/* Description		SUBBAND80_0_CC0

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 0 (HE_SIGB0 or EHT_SIG0) 
			for the lower 80 MHz
			
			Valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_LSB      0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_MSB      7
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_MASK     0x00000000000000ff


/* Description		SUBBAND80_0_CC1

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 1 (HE_SIGB1 or EHT_SIG1) 
			for the lower 80 MHz
			
			Valid for HE_40/HE_80/HE_160 (80+80)/ EHT_40/EHT_80/EHT_160/EHT_240/EHT_320
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_LSB      8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_MSB      15
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_MASK     0x000000000000ff00


/* Description		SUBBAND80_1_CC0

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 0 (HE_SIGB0 or EHT_SIG0) 
			for the higher 80 MHz
			
			Valid for EHT_160/EHT_240/EHT_320
			All 80 MHz subbands are identical for HE_160 (80+80).
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_LSB      16
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_MSB      23
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_MASK     0x0000000000ff0000


/* Description		SUBBAND80_1_CC1

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 1 (HE_SIGB1 or EHT_SIG1) 
			for the higher 80 MHz
			
			Valid for EHT_160/EHT_240/EHT_320
			All 80 MHz subbands are identical for HE_160 (80+80).
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_LSB      24
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_MSB      31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_MASK     0x00000000ff000000


/* Description		RU_ALLOCATION_160_1_DETAILS

			See detailed description of the STRUCT.
			
			Valid for EHT_240/EHT_320
*/


/* Description		SUBBAND80_0_CC0

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 0 (HE_SIGB0 or EHT_SIG0) 
			for the lower 80 MHz
			
			Valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_LSB      32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_MSB      39
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_MASK     0x000000ff00000000


/* Description		SUBBAND80_0_CC1

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 1 (HE_SIGB1 or EHT_SIG1) 
			for the lower 80 MHz
			
			Valid for HE_40/HE_80/HE_160 (80+80)/ EHT_40/EHT_80/EHT_160/EHT_240/EHT_320
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_LSB      40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_MSB      47
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_MASK     0x0000ff0000000000


/* Description		SUBBAND80_1_CC0

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 0 (HE_SIGB0 or EHT_SIG0) 
			for the higher 80 MHz
			
			Valid for EHT_160/EHT_240/EHT_320
			All 80 MHz subbands are identical for HE_160 (80+80).
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_LSB      48
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_MSB      55
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_MASK     0x00ff000000000000


/* Description		SUBBAND80_1_CC1

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 1 (HE_SIGB1 or EHT_SIG1) 
			for the higher 80 MHz
			
			Valid for EHT_160/EHT_240/EHT_320
			All 80 MHz subbands are identical for HE_160 (80+80).
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_LSB      56
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_MSB      63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_MASK     0xff00000000000000


/* Description		NUM_DATA_SYMBOLS

			The number of data symbols in the upcoming transmission.
			
			
			This does not include PE_LTF. Also for STBC packets this
			 has to be an even number.
			
			NOTE: Napier and Hastings80 MAC might not handle LDPC extra
			 symbol and/or padding for STBC to make this an even number.
			
*/

#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_OFFSET                              0x0000000000000038
#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_LSB                                 0
#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_MSB                                 15
#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_MASK                                0x000000000000ffff


/* Description		NDP_RU_TONE_SET_INDEX

			Determines the RU tone set (1 - 72) to use for Short-NDP
			 feedback
			
			Can be set to 0 for frames other than Short-NDP
			<legal 0-72>
*/

#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_OFFSET                         0x0000000000000038
#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_LSB                            16
#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_MSB                            22
#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_MASK                           0x00000000007f0000


/* Description		NDP_FEEDBACK_STATUS

			Determines the feedback value for Short-NDP
			<legal 0-1>
*/

#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_OFFSET                           0x0000000000000038
#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_LSB                              23
#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_MSB                              23
#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_MASK                             0x0000000000800000


/* Description		DOPPLER_INDICATION

			This field is valid only when (MACTX_PHY_DESC.pkt_type==11ax
			 or MACTX_PHY_DESC.pkt_type == 11be).
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_OFFSET                            0x0000000000000038
#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_LSB                               24
#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_MSB                               24
#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_MASK                              0x0000000001000000


/* Description		RESERVED_14A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_14A_OFFSET                                  0x0000000000000038
#define MACTX_USER_DESC_COMMON_RESERVED_14A_LSB                                     25
#define MACTX_USER_DESC_COMMON_RESERVED_14A_MSB                                     31
#define MACTX_USER_DESC_COMMON_RESERVED_14A_MASK                                    0x00000000fe000000


/* Description		SPATIAL_REUSE

			This field is valid only when (MACTX_PHY_DESC.pkt_type==11ax
			 or MACTX_PHY_DESC.pkt_type == 11be)
			
			For an HE TB PPDU all 16 bits are valid.
			For an EHT TB PPDU LSB 8 bits are valid.
			For any other HE/EHT PPDU LSB 4 bits are valid.
			
			<legal all>
*/

#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_OFFSET                                 0x0000000000000038
#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_LSB                                    32
#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_MSB                                    47
#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_MASK                                   0x0000ffff00000000


/* Description		RESERVED_15A

			<legal 0>
*/

#define MACTX_USER_DESC_COMMON_RESERVED_15A_OFFSET                                  0x0000000000000038
#define MACTX_USER_DESC_COMMON_RESERVED_15A_LSB                                     48
#define MACTX_USER_DESC_COMMON_RESERVED_15A_MSB                                     63
#define MACTX_USER_DESC_COMMON_RESERVED_15A_MASK                                    0xffff000000000000



#endif   // MACTX_USER_DESC_COMMON


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
             uint32_t num_users                                               :  6,
                      reserved_0b                                             :  5,
                      ltf_size                                                :  2,
                      reserved_0c                                             :  3,
                      he_stf_long                                             :  1,
                      reserved_0d                                             :  7,
                      num_users_he_sigb_band0                                 :  8;
             uint32_t num_ltf_symbols                                         :  3,
                      reserved_1a                                             :  5,
                      num_users_he_sigb_band1                                 :  8,
                      reserved_1b                                             : 16;
             uint32_t packet_extension_a_factor                               :  2,
                      packet_extension_pe_disambiguity                        :  1,
                      packet_extension                                        :  3,
                      reserved                                                :  2,
                      he_sigb_dcm                                             :  1,
                      reserved_2b                                             :  7,
                      he_sigb_compression                                     :  1,
                      reserved_2c                                             : 15;
             uint32_t he_sigb_0_mcs                                           :  3,
                      reserved_3a                                             : 13,
                      num_he_sigb_sym                                         :  5,
                      center_ru_0                                             :  1,
                      center_ru_1                                             :  1,
                      reserved_3b                                             :  1,
                      ftm_en                                                  :  1,
                      pe_nss                                                  :  3,
                      pe_ltf_size                                             :  2,
                      pe_content                                              :  1,
                      pe_chain_csd_en                                         :  1;
             struct   ru_allocation_160_info                                    ru_allocation_0123_details;
             struct   ru_allocation_160_info                                    ru_allocation_4567_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_0_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_1_details;
             uint32_t num_data_symbols                                        : 16,
                      ndp_ru_tone_set_index                                   :  7,
                      ndp_feedback_status                                     :  1,
                      doppler_indication                                      :  1,
                      reserved_14a                                            :  7;
             uint32_t spatial_reuse                                           : 16,
                      reserved_15a                                            : 16;
#else
             uint32_t num_users_he_sigb_band0                                 :  8,
                      reserved_0d                                             :  7,
                      he_stf_long                                             :  1,
                      reserved_0c                                             :  3,
                      ltf_size                                                :  2,
                      reserved_0b                                             :  5,
                      num_users                                               :  6;
             uint32_t reserved_1b                                             : 16,
                      num_users_he_sigb_band1                                 :  8,
                      reserved_1a                                             :  5,
                      num_ltf_symbols                                         :  3;
             uint32_t reserved_2c                                             : 15,
                      he_sigb_compression                                     :  1,
                      reserved_2b                                             :  7,
                      he_sigb_dcm                                             :  1,
                      reserved                                                :  2,
                      packet_extension                                        :  3,
                      packet_extension_pe_disambiguity                        :  1,
                      packet_extension_a_factor                               :  2;
             uint32_t pe_chain_csd_en                                         :  1,
                      pe_content                                              :  1,
                      pe_ltf_size                                             :  2,
                      pe_nss                                                  :  3,
                      ftm_en                                                  :  1,
                      reserved_3b                                             :  1,
                      center_ru_1                                             :  1,
                      center_ru_0                                             :  1,
                      num_he_sigb_sym                                         :  5,
                      reserved_3a                                             : 13,
                      he_sigb_0_mcs                                           :  3;
             struct   ru_allocation_160_info                                    ru_allocation_0123_details;
             struct   ru_allocation_160_info                                    ru_allocation_4567_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_0_details;
             struct   unallocated_ru_160_info                                   ru_allocation_160_1_details;
             uint32_t reserved_14a                                            :  7,
                      doppler_indication                                      :  1,
                      ndp_feedback_status                                     :  1,
                      ndp_ru_tone_set_index                                   :  7,
                      num_data_symbols                                        : 16;
             uint32_t reserved_15a                                            : 16,
                      spatial_reuse                                           : 16;
#endif
};

#define MACTX_USER_DESC_COMMON_NUM_USERS_OFFSET                                     0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_USERS_LSB                                        0
#define MACTX_USER_DESC_COMMON_NUM_USERS_MSB                                        5
#define MACTX_USER_DESC_COMMON_NUM_USERS_MASK                                       0x000000000000003f

#define MACTX_USER_DESC_COMMON_RESERVED_0B_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_0B_LSB                                      6
#define MACTX_USER_DESC_COMMON_RESERVED_0B_MSB                                      10
#define MACTX_USER_DESC_COMMON_RESERVED_0B_MASK                                     0x00000000000007c0

#define MACTX_USER_DESC_COMMON_LTF_SIZE_OFFSET                                      0x0000000000000000
#define MACTX_USER_DESC_COMMON_LTF_SIZE_LSB                                         11
#define MACTX_USER_DESC_COMMON_LTF_SIZE_MSB                                         12
#define MACTX_USER_DESC_COMMON_LTF_SIZE_MASK                                        0x0000000000001800

#define MACTX_USER_DESC_COMMON_RESERVED_0C_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_0C_LSB                                      13
#define MACTX_USER_DESC_COMMON_RESERVED_0C_MSB                                      15
#define MACTX_USER_DESC_COMMON_RESERVED_0C_MASK                                     0x000000000000e000

#define MACTX_USER_DESC_COMMON_HE_STF_LONG_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_HE_STF_LONG_LSB                                      16
#define MACTX_USER_DESC_COMMON_HE_STF_LONG_MSB                                      16
#define MACTX_USER_DESC_COMMON_HE_STF_LONG_MASK                                     0x0000000000010000

#define MACTX_USER_DESC_COMMON_RESERVED_0D_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_0D_LSB                                      17
#define MACTX_USER_DESC_COMMON_RESERVED_0D_MSB                                      23
#define MACTX_USER_DESC_COMMON_RESERVED_0D_MASK                                     0x0000000000fe0000

#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_OFFSET                       0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_LSB                          24
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_MSB                          31
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND0_MASK                         0x00000000ff000000

#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_OFFSET                               0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_LSB                                  32
#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_MSB                                  34
#define MACTX_USER_DESC_COMMON_NUM_LTF_SYMBOLS_MASK                                 0x0000000700000000

#define MACTX_USER_DESC_COMMON_RESERVED_1A_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_1A_LSB                                      35
#define MACTX_USER_DESC_COMMON_RESERVED_1A_MSB                                      39
#define MACTX_USER_DESC_COMMON_RESERVED_1A_MASK                                     0x000000f800000000

#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_OFFSET                       0x0000000000000000
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_LSB                          40
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_MSB                          47
#define MACTX_USER_DESC_COMMON_NUM_USERS_HE_SIGB_BAND1_MASK                         0x0000ff0000000000

#define MACTX_USER_DESC_COMMON_RESERVED_1B_OFFSET                                   0x0000000000000000
#define MACTX_USER_DESC_COMMON_RESERVED_1B_LSB                                      48
#define MACTX_USER_DESC_COMMON_RESERVED_1B_MSB                                      63
#define MACTX_USER_DESC_COMMON_RESERVED_1B_MASK                                     0xffff000000000000

#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_OFFSET                     0x0000000000000008
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_LSB                        0
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_MSB                        1
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_A_FACTOR_MASK                       0x0000000000000003

#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET              0x0000000000000008
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                 2
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                 2
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                0x0000000000000004

#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_OFFSET                              0x0000000000000008
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_LSB                                 3
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_MSB                                 5
#define MACTX_USER_DESC_COMMON_PACKET_EXTENSION_MASK                                0x0000000000000038

#define MACTX_USER_DESC_COMMON_RESERVED_OFFSET                                      0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_LSB                                         6
#define MACTX_USER_DESC_COMMON_RESERVED_MSB                                         7
#define MACTX_USER_DESC_COMMON_RESERVED_MASK                                        0x00000000000000c0

#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_LSB                                      8
#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_MSB                                      8
#define MACTX_USER_DESC_COMMON_HE_SIGB_DCM_MASK                                     0x0000000000000100

#define MACTX_USER_DESC_COMMON_RESERVED_2B_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_2B_LSB                                      9
#define MACTX_USER_DESC_COMMON_RESERVED_2B_MSB                                      15
#define MACTX_USER_DESC_COMMON_RESERVED_2B_MASK                                     0x000000000000fe00

#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_OFFSET                           0x0000000000000008
#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_LSB                              16
#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_MSB                              16
#define MACTX_USER_DESC_COMMON_HE_SIGB_COMPRESSION_MASK                             0x0000000000010000

#define MACTX_USER_DESC_COMMON_RESERVED_2C_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_2C_LSB                                      17
#define MACTX_USER_DESC_COMMON_RESERVED_2C_MSB                                      31
#define MACTX_USER_DESC_COMMON_RESERVED_2C_MASK                                     0x00000000fffe0000

#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_OFFSET                                 0x0000000000000008
#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_LSB                                    32
#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_MSB                                    34
#define MACTX_USER_DESC_COMMON_HE_SIGB_0_MCS_MASK                                   0x0000000700000000

#define MACTX_USER_DESC_COMMON_RESERVED_3A_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_3A_LSB                                      35
#define MACTX_USER_DESC_COMMON_RESERVED_3A_MSB                                      47
#define MACTX_USER_DESC_COMMON_RESERVED_3A_MASK                                     0x0000fff800000000

#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_OFFSET                               0x0000000000000008
#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_LSB                                  48
#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_MSB                                  52
#define MACTX_USER_DESC_COMMON_NUM_HE_SIGB_SYM_MASK                                 0x001f000000000000

#define MACTX_USER_DESC_COMMON_CENTER_RU_0_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_CENTER_RU_0_LSB                                      53
#define MACTX_USER_DESC_COMMON_CENTER_RU_0_MSB                                      53
#define MACTX_USER_DESC_COMMON_CENTER_RU_0_MASK                                     0x0020000000000000

#define MACTX_USER_DESC_COMMON_CENTER_RU_1_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_CENTER_RU_1_LSB                                      54
#define MACTX_USER_DESC_COMMON_CENTER_RU_1_MSB                                      54
#define MACTX_USER_DESC_COMMON_CENTER_RU_1_MASK                                     0x0040000000000000

#define MACTX_USER_DESC_COMMON_RESERVED_3B_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_RESERVED_3B_LSB                                      55
#define MACTX_USER_DESC_COMMON_RESERVED_3B_MSB                                      55
#define MACTX_USER_DESC_COMMON_RESERVED_3B_MASK                                     0x0080000000000000

#define MACTX_USER_DESC_COMMON_FTM_EN_OFFSET                                        0x0000000000000008
#define MACTX_USER_DESC_COMMON_FTM_EN_LSB                                           56
#define MACTX_USER_DESC_COMMON_FTM_EN_MSB                                           56
#define MACTX_USER_DESC_COMMON_FTM_EN_MASK                                          0x0100000000000000

#define MACTX_USER_DESC_COMMON_PE_NSS_OFFSET                                        0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_NSS_LSB                                           57
#define MACTX_USER_DESC_COMMON_PE_NSS_MSB                                           59
#define MACTX_USER_DESC_COMMON_PE_NSS_MASK                                          0x0e00000000000000

#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_OFFSET                                   0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_LSB                                      60
#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_MSB                                      61
#define MACTX_USER_DESC_COMMON_PE_LTF_SIZE_MASK                                     0x3000000000000000

#define MACTX_USER_DESC_COMMON_PE_CONTENT_OFFSET                                    0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_CONTENT_LSB                                       62
#define MACTX_USER_DESC_COMMON_PE_CONTENT_MSB                                       62
#define MACTX_USER_DESC_COMMON_PE_CONTENT_MASK                                      0x4000000000000000

#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_OFFSET                               0x0000000000000008
#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_LSB                                  63
#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_MSB                                  63
#define MACTX_USER_DESC_COMMON_PE_CHAIN_CSD_EN_MASK                                 0x8000000000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_0_MASK 0x00000000000001ff

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_1_MASK 0x000000000003fe00

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_OFFSET        0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_MSB           23
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_0A_MASK          0x0000000000fc0000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_LSB 24
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MSB 27
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MASK 0x000000000f000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_LSB 28
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MSB 31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MASK 0x00000000f0000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_2_MASK 0x000001ff00000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_OFFSET 0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND0_3_MASK 0x0003fe0000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_OFFSET        0x0000000000000010
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_1A_MASK          0xfffc000000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_0_MASK 0x00000000000001ff

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_1_MASK 0x000000000003fe00

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_OFFSET        0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_MSB           31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_2A_MASK          0x00000000fffc0000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_2_MASK 0x000001ff00000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_OFFSET 0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RU_ALLOCATION_BAND1_3_MASK 0x0003fe0000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_OFFSET        0x0000000000000018
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_0123_DETAILS_RESERVED_3A_MASK          0xfffc000000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_0_MASK 0x00000000000001ff

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_1_MASK 0x000000000003fe00

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_OFFSET        0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_MSB           23
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_0A_MASK          0x0000000000fc0000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_LSB 24
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MSB 27
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_01_SUBBAND80_MASK_MASK 0x000000000f000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_LSB 28
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MSB 31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATIONS_23_SUBBAND80_MASK_MASK 0x00000000f0000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_2_MASK 0x000001ff00000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_OFFSET 0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND0_3_MASK 0x0003fe0000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_OFFSET        0x0000000000000020
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_1A_MASK          0xfffc000000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_LSB 0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_MSB 8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_0_MASK 0x00000000000001ff

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_LSB 9
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_MSB 17
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_1_MASK 0x000000000003fe00

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_OFFSET        0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_LSB           18
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_MSB           31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_2A_MASK          0x00000000fffc0000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_LSB 32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_MSB 40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_2_MASK 0x000001ff00000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_OFFSET 0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_LSB 41
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_MSB 49
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RU_ALLOCATION_BAND1_3_MASK 0x0003fe0000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_OFFSET        0x0000000000000028
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_LSB           50
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_MSB           63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_4567_DETAILS_RESERVED_3A_MASK          0xfffc000000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_LSB      0
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_MSB      7
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC0_MASK     0x00000000000000ff

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_LSB      8
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_MSB      15
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_0_CC1_MASK     0x000000000000ff00

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_LSB      16
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_MSB      23
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC0_MASK     0x0000000000ff0000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_LSB      24
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_MSB      31
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_0_DETAILS_SUBBAND80_1_CC1_MASK     0x00000000ff000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_LSB      32
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_MSB      39
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC0_MASK     0x000000ff00000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_LSB      40
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_MSB      47
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_0_CC1_MASK     0x0000ff0000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_LSB      48
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_MSB      55
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC0_MASK     0x00ff000000000000

#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_OFFSET   0x0000000000000030
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_LSB      56
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_MSB      63
#define MACTX_USER_DESC_COMMON_RU_ALLOCATION_160_1_DETAILS_SUBBAND80_1_CC1_MASK     0xff00000000000000

#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_OFFSET                              0x0000000000000038
#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_LSB                                 0
#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_MSB                                 15
#define MACTX_USER_DESC_COMMON_NUM_DATA_SYMBOLS_MASK                                0x000000000000ffff

#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_OFFSET                         0x0000000000000038
#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_LSB                            16
#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_MSB                            22
#define MACTX_USER_DESC_COMMON_NDP_RU_TONE_SET_INDEX_MASK                           0x00000000007f0000

#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_OFFSET                           0x0000000000000038
#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_LSB                              23
#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_MSB                              23
#define MACTX_USER_DESC_COMMON_NDP_FEEDBACK_STATUS_MASK                             0x0000000000800000

#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_OFFSET                            0x0000000000000038
#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_LSB                               24
#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_MSB                               24
#define MACTX_USER_DESC_COMMON_DOPPLER_INDICATION_MASK                              0x0000000001000000

#define MACTX_USER_DESC_COMMON_RESERVED_14A_OFFSET                                  0x0000000000000038
#define MACTX_USER_DESC_COMMON_RESERVED_14A_LSB                                     25
#define MACTX_USER_DESC_COMMON_RESERVED_14A_MSB                                     31
#define MACTX_USER_DESC_COMMON_RESERVED_14A_MASK                                    0x00000000fe000000

#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_OFFSET                                 0x0000000000000038
#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_LSB                                    32
#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_MSB                                    47
#define MACTX_USER_DESC_COMMON_SPATIAL_REUSE_MASK                                   0x0000ffff00000000

#define MACTX_USER_DESC_COMMON_RESERVED_15A_OFFSET                                  0x0000000000000038
#define MACTX_USER_DESC_COMMON_RESERVED_15A_LSB                                     48
#define MACTX_USER_DESC_COMMON_RESERVED_15A_MSB                                     63
#define MACTX_USER_DESC_COMMON_RESERVED_15A_MASK                                    0xffff000000000000

#endif

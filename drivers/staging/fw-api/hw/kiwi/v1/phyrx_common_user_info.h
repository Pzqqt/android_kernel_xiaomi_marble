
/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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











#ifndef _PHYRX_COMMON_USER_INFO_H_
#define _PHYRX_COMMON_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_COMMON_USER_INFO 4

#define NUM_OF_QWORDS_PHYRX_COMMON_USER_INFO 2


struct phyrx_common_user_info {
	     uint32_t receive_duration                                        : 16,
		      reserved_0a                                             : 16;
	     uint32_t u_sig_puncture_pattern_encoding                         :  6,
		      reserved_1a                                             : 26;
	     uint32_t eht_ppdu_type                                           :  2,
		      bss_color_id                                            :  6,
		      dl_ul_flag                                              :  1,
		      txop_duration                                           :  7,
		      cp_setting                                              :  2,
		      ltf_size                                                :  2,
		      spatial_reuse                                           :  4,
		      rx_ndp                                                  :  1,
		      dot11be_su_extended                                     :  1,
		      reserved_2a                                             :  6;
	     uint32_t eht_duplicate                                           :  2,
		      eht_sig_cmn_field_type                                  :  2,
		      doppler_indication                                      :  1,
		      sta_id                                                  : 11,
		      puncture_bitmap                                         : 16;
};




#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_OFFSET                              0x0000000000000000
#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_LSB                                 0
#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_MSB                                 15
#define PHYRX_COMMON_USER_INFO_RECEIVE_DURATION_MASK                                0x000000000000ffff




#define PHYRX_COMMON_USER_INFO_RESERVED_0A_OFFSET                                   0x0000000000000000
#define PHYRX_COMMON_USER_INFO_RESERVED_0A_LSB                                      16
#define PHYRX_COMMON_USER_INFO_RESERVED_0A_MSB                                      31
#define PHYRX_COMMON_USER_INFO_RESERVED_0A_MASK                                     0x00000000ffff0000




#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_OFFSET               0x0000000000000000
#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_LSB                  32
#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MSB                  37
#define PHYRX_COMMON_USER_INFO_U_SIG_PUNCTURE_PATTERN_ENCODING_MASK                 0x0000003f00000000




#define PHYRX_COMMON_USER_INFO_RESERVED_1A_OFFSET                                   0x0000000000000000
#define PHYRX_COMMON_USER_INFO_RESERVED_1A_LSB                                      38
#define PHYRX_COMMON_USER_INFO_RESERVED_1A_MSB                                      63
#define PHYRX_COMMON_USER_INFO_RESERVED_1A_MASK                                     0xffffffc000000000




#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_LSB                                    0
#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_MSB                                    1
#define PHYRX_COMMON_USER_INFO_EHT_PPDU_TYPE_MASK                                   0x0000000000000003




#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_OFFSET                                  0x0000000000000008
#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_LSB                                     2
#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_MSB                                     7
#define PHYRX_COMMON_USER_INFO_BSS_COLOR_ID_MASK                                    0x00000000000000fc




#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_OFFSET                                    0x0000000000000008
#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_LSB                                       8
#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_MSB                                       8
#define PHYRX_COMMON_USER_INFO_DL_UL_FLAG_MASK                                      0x0000000000000100




#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_LSB                                    9
#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_MSB                                    15
#define PHYRX_COMMON_USER_INFO_TXOP_DURATION_MASK                                   0x000000000000fe00




#define PHYRX_COMMON_USER_INFO_CP_SETTING_OFFSET                                    0x0000000000000008
#define PHYRX_COMMON_USER_INFO_CP_SETTING_LSB                                       16
#define PHYRX_COMMON_USER_INFO_CP_SETTING_MSB                                       17
#define PHYRX_COMMON_USER_INFO_CP_SETTING_MASK                                      0x0000000000030000




#define PHYRX_COMMON_USER_INFO_LTF_SIZE_OFFSET                                      0x0000000000000008
#define PHYRX_COMMON_USER_INFO_LTF_SIZE_LSB                                         18
#define PHYRX_COMMON_USER_INFO_LTF_SIZE_MSB                                         19
#define PHYRX_COMMON_USER_INFO_LTF_SIZE_MASK                                        0x00000000000c0000




#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_LSB                                    20
#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_MSB                                    23
#define PHYRX_COMMON_USER_INFO_SPATIAL_REUSE_MASK                                   0x0000000000f00000




#define PHYRX_COMMON_USER_INFO_RX_NDP_OFFSET                                        0x0000000000000008
#define PHYRX_COMMON_USER_INFO_RX_NDP_LSB                                           24
#define PHYRX_COMMON_USER_INFO_RX_NDP_MSB                                           24
#define PHYRX_COMMON_USER_INFO_RX_NDP_MASK                                          0x0000000001000000




#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_OFFSET                           0x0000000000000008
#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_LSB                              25
#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_MSB                              25
#define PHYRX_COMMON_USER_INFO_DOT11BE_SU_EXTENDED_MASK                             0x0000000002000000




#define PHYRX_COMMON_USER_INFO_RESERVED_2A_OFFSET                                   0x0000000000000008
#define PHYRX_COMMON_USER_INFO_RESERVED_2A_LSB                                      26
#define PHYRX_COMMON_USER_INFO_RESERVED_2A_MSB                                      31
#define PHYRX_COMMON_USER_INFO_RESERVED_2A_MASK                                     0x00000000fc000000




#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_OFFSET                                 0x0000000000000008
#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_LSB                                    32
#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_MSB                                    33
#define PHYRX_COMMON_USER_INFO_EHT_DUPLICATE_MASK                                   0x0000000300000000




#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_OFFSET                        0x0000000000000008
#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_LSB                           34
#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_MSB                           35
#define PHYRX_COMMON_USER_INFO_EHT_SIG_CMN_FIELD_TYPE_MASK                          0x0000000c00000000




#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_OFFSET                            0x0000000000000008
#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_LSB                               36
#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_MSB                               36
#define PHYRX_COMMON_USER_INFO_DOPPLER_INDICATION_MASK                              0x0000001000000000




#define PHYRX_COMMON_USER_INFO_STA_ID_OFFSET                                        0x0000000000000008
#define PHYRX_COMMON_USER_INFO_STA_ID_LSB                                           37
#define PHYRX_COMMON_USER_INFO_STA_ID_MSB                                           47
#define PHYRX_COMMON_USER_INFO_STA_ID_MASK                                          0x0000ffe000000000




#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_OFFSET                               0x0000000000000008
#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_LSB                                  48
#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_MSB                                  63
#define PHYRX_COMMON_USER_INFO_PUNCTURE_BITMAP_MASK                                 0xffff000000000000



#endif

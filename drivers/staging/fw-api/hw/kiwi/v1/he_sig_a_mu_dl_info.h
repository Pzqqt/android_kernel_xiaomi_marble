
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











#ifndef _HE_SIG_A_MU_DL_INFO_H_
#define _HE_SIG_A_MU_DL_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HE_SIG_A_MU_DL_INFO 2


struct he_sig_a_mu_dl_info {
	     uint32_t dl_ul_flag                                              :  1,
		      mcs_of_sig_b                                            :  3,
		      dcm_of_sig_b                                            :  1,
		      bss_color_id                                            :  6,
		      spatial_reuse                                           :  4,
		      transmit_bw                                             :  3,
		      num_sig_b_symbols                                       :  4,
		      comp_mode_sig_b                                         :  1,
		      cp_ltf_size                                             :  2,
		      doppler_indication                                      :  1,
		      reserved_0a                                             :  6;
	     uint32_t txop_duration                                           :  7,
		      reserved_1a                                             :  1,
		      num_ltf_symbols                                         :  3,
		      ldpc_extra_symbol                                       :  1,
		      stbc                                                    :  1,
		      packet_extension_a_factor                               :  2,
		      packet_extension_pe_disambiguity                        :  1,
		      crc                                                     :  4,
		      tail                                                    :  6,
		      reserved_1b                                             :  5,
		      rx_integrity_check_passed                               :  1;
};




#define HE_SIG_A_MU_DL_INFO_DL_UL_FLAG_OFFSET                                       0x00000000
#define HE_SIG_A_MU_DL_INFO_DL_UL_FLAG_LSB                                          0
#define HE_SIG_A_MU_DL_INFO_DL_UL_FLAG_MSB                                          0
#define HE_SIG_A_MU_DL_INFO_DL_UL_FLAG_MASK                                         0x00000001




#define HE_SIG_A_MU_DL_INFO_MCS_OF_SIG_B_OFFSET                                     0x00000000
#define HE_SIG_A_MU_DL_INFO_MCS_OF_SIG_B_LSB                                        1
#define HE_SIG_A_MU_DL_INFO_MCS_OF_SIG_B_MSB                                        3
#define HE_SIG_A_MU_DL_INFO_MCS_OF_SIG_B_MASK                                       0x0000000e




#define HE_SIG_A_MU_DL_INFO_DCM_OF_SIG_B_OFFSET                                     0x00000000
#define HE_SIG_A_MU_DL_INFO_DCM_OF_SIG_B_LSB                                        4
#define HE_SIG_A_MU_DL_INFO_DCM_OF_SIG_B_MSB                                        4
#define HE_SIG_A_MU_DL_INFO_DCM_OF_SIG_B_MASK                                       0x00000010




#define HE_SIG_A_MU_DL_INFO_BSS_COLOR_ID_OFFSET                                     0x00000000
#define HE_SIG_A_MU_DL_INFO_BSS_COLOR_ID_LSB                                        5
#define HE_SIG_A_MU_DL_INFO_BSS_COLOR_ID_MSB                                        10
#define HE_SIG_A_MU_DL_INFO_BSS_COLOR_ID_MASK                                       0x000007e0




#define HE_SIG_A_MU_DL_INFO_SPATIAL_REUSE_OFFSET                                    0x00000000
#define HE_SIG_A_MU_DL_INFO_SPATIAL_REUSE_LSB                                       11
#define HE_SIG_A_MU_DL_INFO_SPATIAL_REUSE_MSB                                       14
#define HE_SIG_A_MU_DL_INFO_SPATIAL_REUSE_MASK                                      0x00007800




#define HE_SIG_A_MU_DL_INFO_TRANSMIT_BW_OFFSET                                      0x00000000
#define HE_SIG_A_MU_DL_INFO_TRANSMIT_BW_LSB                                         15
#define HE_SIG_A_MU_DL_INFO_TRANSMIT_BW_MSB                                         17
#define HE_SIG_A_MU_DL_INFO_TRANSMIT_BW_MASK                                        0x00038000




#define HE_SIG_A_MU_DL_INFO_NUM_SIG_B_SYMBOLS_OFFSET                                0x00000000
#define HE_SIG_A_MU_DL_INFO_NUM_SIG_B_SYMBOLS_LSB                                   18
#define HE_SIG_A_MU_DL_INFO_NUM_SIG_B_SYMBOLS_MSB                                   21
#define HE_SIG_A_MU_DL_INFO_NUM_SIG_B_SYMBOLS_MASK                                  0x003c0000




#define HE_SIG_A_MU_DL_INFO_COMP_MODE_SIG_B_OFFSET                                  0x00000000
#define HE_SIG_A_MU_DL_INFO_COMP_MODE_SIG_B_LSB                                     22
#define HE_SIG_A_MU_DL_INFO_COMP_MODE_SIG_B_MSB                                     22
#define HE_SIG_A_MU_DL_INFO_COMP_MODE_SIG_B_MASK                                    0x00400000




#define HE_SIG_A_MU_DL_INFO_CP_LTF_SIZE_OFFSET                                      0x00000000
#define HE_SIG_A_MU_DL_INFO_CP_LTF_SIZE_LSB                                         23
#define HE_SIG_A_MU_DL_INFO_CP_LTF_SIZE_MSB                                         24
#define HE_SIG_A_MU_DL_INFO_CP_LTF_SIZE_MASK                                        0x01800000




#define HE_SIG_A_MU_DL_INFO_DOPPLER_INDICATION_OFFSET                               0x00000000
#define HE_SIG_A_MU_DL_INFO_DOPPLER_INDICATION_LSB                                  25
#define HE_SIG_A_MU_DL_INFO_DOPPLER_INDICATION_MSB                                  25
#define HE_SIG_A_MU_DL_INFO_DOPPLER_INDICATION_MASK                                 0x02000000




#define HE_SIG_A_MU_DL_INFO_RESERVED_0A_OFFSET                                      0x00000000
#define HE_SIG_A_MU_DL_INFO_RESERVED_0A_LSB                                         26
#define HE_SIG_A_MU_DL_INFO_RESERVED_0A_MSB                                         31
#define HE_SIG_A_MU_DL_INFO_RESERVED_0A_MASK                                        0xfc000000




#define HE_SIG_A_MU_DL_INFO_TXOP_DURATION_OFFSET                                    0x00000004
#define HE_SIG_A_MU_DL_INFO_TXOP_DURATION_LSB                                       0
#define HE_SIG_A_MU_DL_INFO_TXOP_DURATION_MSB                                       6
#define HE_SIG_A_MU_DL_INFO_TXOP_DURATION_MASK                                      0x0000007f




#define HE_SIG_A_MU_DL_INFO_RESERVED_1A_OFFSET                                      0x00000004
#define HE_SIG_A_MU_DL_INFO_RESERVED_1A_LSB                                         7
#define HE_SIG_A_MU_DL_INFO_RESERVED_1A_MSB                                         7
#define HE_SIG_A_MU_DL_INFO_RESERVED_1A_MASK                                        0x00000080




#define HE_SIG_A_MU_DL_INFO_NUM_LTF_SYMBOLS_OFFSET                                  0x00000004
#define HE_SIG_A_MU_DL_INFO_NUM_LTF_SYMBOLS_LSB                                     8
#define HE_SIG_A_MU_DL_INFO_NUM_LTF_SYMBOLS_MSB                                     10
#define HE_SIG_A_MU_DL_INFO_NUM_LTF_SYMBOLS_MASK                                    0x00000700




#define HE_SIG_A_MU_DL_INFO_LDPC_EXTRA_SYMBOL_OFFSET                                0x00000004
#define HE_SIG_A_MU_DL_INFO_LDPC_EXTRA_SYMBOL_LSB                                   11
#define HE_SIG_A_MU_DL_INFO_LDPC_EXTRA_SYMBOL_MSB                                   11
#define HE_SIG_A_MU_DL_INFO_LDPC_EXTRA_SYMBOL_MASK                                  0x00000800




#define HE_SIG_A_MU_DL_INFO_STBC_OFFSET                                             0x00000004
#define HE_SIG_A_MU_DL_INFO_STBC_LSB                                                12
#define HE_SIG_A_MU_DL_INFO_STBC_MSB                                                12
#define HE_SIG_A_MU_DL_INFO_STBC_MASK                                               0x00001000




#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_A_FACTOR_OFFSET                        0x00000004
#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_A_FACTOR_LSB                           13
#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_A_FACTOR_MSB                           14
#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_A_FACTOR_MASK                          0x00006000




#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET                 0x00000004
#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB                    15
#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB                    15
#define HE_SIG_A_MU_DL_INFO_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK                   0x00008000




#define HE_SIG_A_MU_DL_INFO_CRC_OFFSET                                              0x00000004
#define HE_SIG_A_MU_DL_INFO_CRC_LSB                                                 16
#define HE_SIG_A_MU_DL_INFO_CRC_MSB                                                 19
#define HE_SIG_A_MU_DL_INFO_CRC_MASK                                                0x000f0000




#define HE_SIG_A_MU_DL_INFO_TAIL_OFFSET                                             0x00000004
#define HE_SIG_A_MU_DL_INFO_TAIL_LSB                                                20
#define HE_SIG_A_MU_DL_INFO_TAIL_MSB                                                25
#define HE_SIG_A_MU_DL_INFO_TAIL_MASK                                               0x03f00000




#define HE_SIG_A_MU_DL_INFO_RESERVED_1B_OFFSET                                      0x00000004
#define HE_SIG_A_MU_DL_INFO_RESERVED_1B_LSB                                         26
#define HE_SIG_A_MU_DL_INFO_RESERVED_1B_MSB                                         30
#define HE_SIG_A_MU_DL_INFO_RESERVED_1B_MASK                                        0x7c000000




#define HE_SIG_A_MU_DL_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                        0x00000004
#define HE_SIG_A_MU_DL_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                           31
#define HE_SIG_A_MU_DL_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                           31
#define HE_SIG_A_MU_DL_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                          0x80000000



#endif

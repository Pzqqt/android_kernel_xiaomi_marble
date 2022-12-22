
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef _HE_SIG_A_SU_INFO_H_
#define _HE_SIG_A_SU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HE_SIG_A_SU_INFO 2

struct he_sig_a_su_info {
             uint32_t format_indication               :  1,
                      beam_change                     :  1,
                      dl_ul_flag                      :  1,
                      transmit_mcs                    :  4,
                      dcm                             :  1,
                      bss_color_id                    :  6,
                      reserved_0a                     :  1,
                      spatial_reuse                   :  4,
                      transmit_bw                     :  2,
                      cp_ltf_size                     :  2,
                      nsts                            :  3,
                      reserved_0b                     :  6;
             uint32_t txop_duration                   :  7,
                      coding                          :  1,
                      ldpc_extra_symbol               :  1,
                      stbc                            :  1,
                      txbf                            :  1,
                      packet_extension_a_factor       :  2,
                      packet_extension_pe_disambiguity:  1,
                      reserved_1a                     :  1,
                      doppler_indication              :  1,
                      crc                             :  4,
                      tail                            :  6,
                      dot11ax_su_extended             :  1,
                      dot11ax_ext_ru_size             :  4,
                      rx_ndp                          :  1;
};

#define HE_SIG_A_SU_INFO_0_FORMAT_INDICATION_OFFSET                  0x00000000
#define HE_SIG_A_SU_INFO_0_FORMAT_INDICATION_LSB                     0
#define HE_SIG_A_SU_INFO_0_FORMAT_INDICATION_MASK                    0x00000001

#define HE_SIG_A_SU_INFO_0_BEAM_CHANGE_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_BEAM_CHANGE_LSB                           1
#define HE_SIG_A_SU_INFO_0_BEAM_CHANGE_MASK                          0x00000002

#define HE_SIG_A_SU_INFO_0_DL_UL_FLAG_OFFSET                         0x00000000
#define HE_SIG_A_SU_INFO_0_DL_UL_FLAG_LSB                            2
#define HE_SIG_A_SU_INFO_0_DL_UL_FLAG_MASK                           0x00000004

#define HE_SIG_A_SU_INFO_0_TRANSMIT_MCS_OFFSET                       0x00000000
#define HE_SIG_A_SU_INFO_0_TRANSMIT_MCS_LSB                          3
#define HE_SIG_A_SU_INFO_0_TRANSMIT_MCS_MASK                         0x00000078

#define HE_SIG_A_SU_INFO_0_DCM_OFFSET                                0x00000000
#define HE_SIG_A_SU_INFO_0_DCM_LSB                                   7
#define HE_SIG_A_SU_INFO_0_DCM_MASK                                  0x00000080

#define HE_SIG_A_SU_INFO_0_BSS_COLOR_ID_OFFSET                       0x00000000
#define HE_SIG_A_SU_INFO_0_BSS_COLOR_ID_LSB                          8
#define HE_SIG_A_SU_INFO_0_BSS_COLOR_ID_MASK                         0x00003f00

#define HE_SIG_A_SU_INFO_0_RESERVED_0A_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_RESERVED_0A_LSB                           14
#define HE_SIG_A_SU_INFO_0_RESERVED_0A_MASK                          0x00004000

#define HE_SIG_A_SU_INFO_0_SPATIAL_REUSE_OFFSET                      0x00000000
#define HE_SIG_A_SU_INFO_0_SPATIAL_REUSE_LSB                         15
#define HE_SIG_A_SU_INFO_0_SPATIAL_REUSE_MASK                        0x00078000

#define HE_SIG_A_SU_INFO_0_TRANSMIT_BW_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_TRANSMIT_BW_LSB                           19
#define HE_SIG_A_SU_INFO_0_TRANSMIT_BW_MASK                          0x00180000

#define HE_SIG_A_SU_INFO_0_CP_LTF_SIZE_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_CP_LTF_SIZE_LSB                           21
#define HE_SIG_A_SU_INFO_0_CP_LTF_SIZE_MASK                          0x00600000

#define HE_SIG_A_SU_INFO_0_NSTS_OFFSET                               0x00000000
#define HE_SIG_A_SU_INFO_0_NSTS_LSB                                  23
#define HE_SIG_A_SU_INFO_0_NSTS_MASK                                 0x03800000

#define HE_SIG_A_SU_INFO_0_RESERVED_0B_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_RESERVED_0B_LSB                           26
#define HE_SIG_A_SU_INFO_0_RESERVED_0B_MASK                          0xfc000000

#define HE_SIG_A_SU_INFO_1_TXOP_DURATION_OFFSET                      0x00000004
#define HE_SIG_A_SU_INFO_1_TXOP_DURATION_LSB                         0
#define HE_SIG_A_SU_INFO_1_TXOP_DURATION_MASK                        0x0000007f

#define HE_SIG_A_SU_INFO_1_CODING_OFFSET                             0x00000004
#define HE_SIG_A_SU_INFO_1_CODING_LSB                                7
#define HE_SIG_A_SU_INFO_1_CODING_MASK                               0x00000080

#define HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL_OFFSET                  0x00000004
#define HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL_LSB                     8
#define HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL_MASK                    0x00000100

#define HE_SIG_A_SU_INFO_1_STBC_OFFSET                               0x00000004
#define HE_SIG_A_SU_INFO_1_STBC_LSB                                  9
#define HE_SIG_A_SU_INFO_1_STBC_MASK                                 0x00000200

#define HE_SIG_A_SU_INFO_1_TXBF_OFFSET                               0x00000004
#define HE_SIG_A_SU_INFO_1_TXBF_LSB                                  10
#define HE_SIG_A_SU_INFO_1_TXBF_MASK                                 0x00000400

#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR_OFFSET          0x00000004
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR_LSB             11
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR_MASK            0x00001800

#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET   0x00000004
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB      13
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK     0x00002000

#define HE_SIG_A_SU_INFO_1_RESERVED_1A_OFFSET                        0x00000004
#define HE_SIG_A_SU_INFO_1_RESERVED_1A_LSB                           14
#define HE_SIG_A_SU_INFO_1_RESERVED_1A_MASK                          0x00004000

#define HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION_OFFSET                 0x00000004
#define HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION_LSB                    15
#define HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION_MASK                   0x00008000

#define HE_SIG_A_SU_INFO_1_CRC_OFFSET                                0x00000004
#define HE_SIG_A_SU_INFO_1_CRC_LSB                                   16
#define HE_SIG_A_SU_INFO_1_CRC_MASK                                  0x000f0000

#define HE_SIG_A_SU_INFO_1_TAIL_OFFSET                               0x00000004
#define HE_SIG_A_SU_INFO_1_TAIL_LSB                                  20
#define HE_SIG_A_SU_INFO_1_TAIL_MASK                                 0x03f00000

#define HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED_OFFSET                0x00000004
#define HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED_LSB                   26
#define HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED_MASK                  0x04000000

#define HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE_OFFSET                0x00000004
#define HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE_LSB                   27
#define HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE_MASK                  0x78000000

#define HE_SIG_A_SU_INFO_1_RX_NDP_OFFSET                             0x00000004
#define HE_SIG_A_SU_INFO_1_RX_NDP_LSB                                31
#define HE_SIG_A_SU_INFO_1_RX_NDP_MASK                               0x80000000

#endif

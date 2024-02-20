/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _VHT_SIG_B_SU160_INFO_H_
#define _VHT_SIG_B_SU160_INFO_H_

#define NUM_OF_DWORDS_VHT_SIG_B_SU160_INFO 8

struct vht_sig_b_su160_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 21,
                      vhtb_reserved                                           :  2,
                      tail                                                    :  6,
                      reserved_0                                              :  2,
                      rx_ndp                                                  :  1;
             uint32_t length_copy_a                                           : 21,
                      vhtb_reserved_copy_a                                    :  2,
                      tail_copy_a                                             :  6,
                      reserved_1                                              :  2,
                      rx_ndp_copy_a                                           :  1;
             uint32_t length_copy_b                                           : 21,
                      vhtb_reserved_copy_b                                    :  2,
                      tail_copy_b                                             :  6,
                      reserved_2                                              :  2,
                      rx_ndp_copy_b                                           :  1;
             uint32_t length_copy_c                                           : 21,
                      vhtb_reserved_copy_c                                    :  2,
                      tail_copy_c                                             :  6,
                      reserved_3                                              :  2,
                      rx_ndp_copy_c                                           :  1;
             uint32_t length_copy_d                                           : 21,
                      vhtb_reserved_copy_d                                    :  2,
                      tail_copy_d                                             :  6,
                      reserved_4                                              :  2,
                      rx_ndp_copy_d                                           :  1;
             uint32_t length_copy_e                                           : 21,
                      vhtb_reserved_copy_e                                    :  2,
                      tail_copy_e                                             :  6,
                      reserved_5                                              :  2,
                      rx_ndp_copy_e                                           :  1;
             uint32_t length_copy_f                                           : 21,
                      vhtb_reserved_copy_f                                    :  2,
                      tail_copy_f                                             :  6,
                      reserved_6                                              :  2,
                      rx_ndp_copy_f                                           :  1;
             uint32_t length_copy_g                                           : 21,
                      vhtb_reserved_copy_g                                    :  2,
                      tail_copy_g                                             :  6,
                      reserved_7                                              :  2,
                      rx_ndp_copy_g                                           :  1;
#else
             uint32_t rx_ndp                                                  :  1,
                      reserved_0                                              :  2,
                      tail                                                    :  6,
                      vhtb_reserved                                           :  2,
                      length                                                  : 21;
             uint32_t rx_ndp_copy_a                                           :  1,
                      reserved_1                                              :  2,
                      tail_copy_a                                             :  6,
                      vhtb_reserved_copy_a                                    :  2,
                      length_copy_a                                           : 21;
             uint32_t rx_ndp_copy_b                                           :  1,
                      reserved_2                                              :  2,
                      tail_copy_b                                             :  6,
                      vhtb_reserved_copy_b                                    :  2,
                      length_copy_b                                           : 21;
             uint32_t rx_ndp_copy_c                                           :  1,
                      reserved_3                                              :  2,
                      tail_copy_c                                             :  6,
                      vhtb_reserved_copy_c                                    :  2,
                      length_copy_c                                           : 21;
             uint32_t rx_ndp_copy_d                                           :  1,
                      reserved_4                                              :  2,
                      tail_copy_d                                             :  6,
                      vhtb_reserved_copy_d                                    :  2,
                      length_copy_d                                           : 21;
             uint32_t rx_ndp_copy_e                                           :  1,
                      reserved_5                                              :  2,
                      tail_copy_e                                             :  6,
                      vhtb_reserved_copy_e                                    :  2,
                      length_copy_e                                           : 21;
             uint32_t rx_ndp_copy_f                                           :  1,
                      reserved_6                                              :  2,
                      tail_copy_f                                             :  6,
                      vhtb_reserved_copy_f                                    :  2,
                      length_copy_f                                           : 21;
             uint32_t rx_ndp_copy_g                                           :  1,
                      reserved_7                                              :  2,
                      tail_copy_g                                             :  6,
                      vhtb_reserved_copy_g                                    :  2,
                      length_copy_g                                           : 21;
#endif
};

#define VHT_SIG_B_SU160_INFO_LENGTH_OFFSET                                          0x00000000
#define VHT_SIG_B_SU160_INFO_LENGTH_LSB                                             0
#define VHT_SIG_B_SU160_INFO_LENGTH_MSB                                             20
#define VHT_SIG_B_SU160_INFO_LENGTH_MASK                                            0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_OFFSET                                   0x00000000
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_LSB                                      21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_MSB                                      22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_MASK                                     0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_OFFSET                                            0x00000000
#define VHT_SIG_B_SU160_INFO_TAIL_LSB                                               23
#define VHT_SIG_B_SU160_INFO_TAIL_MSB                                               28
#define VHT_SIG_B_SU160_INFO_TAIL_MASK                                              0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_0_OFFSET                                      0x00000000
#define VHT_SIG_B_SU160_INFO_RESERVED_0_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_0_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_0_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_OFFSET                                          0x00000000
#define VHT_SIG_B_SU160_INFO_RX_NDP_LSB                                             31
#define VHT_SIG_B_SU160_INFO_RX_NDP_MSB                                             31
#define VHT_SIG_B_SU160_INFO_RX_NDP_MASK                                            0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_A_OFFSET                                   0x00000004
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_A_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_A_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_A_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_A_OFFSET                            0x00000004
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_A_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_A_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_A_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_A_OFFSET                                     0x00000004
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_A_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_A_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_A_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_1_OFFSET                                      0x00000004
#define VHT_SIG_B_SU160_INFO_RESERVED_1_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_1_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_1_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_A_OFFSET                                   0x00000004
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_A_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_A_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_A_MASK                                     0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_B_OFFSET                                   0x00000008
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_B_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_B_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_B_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_B_OFFSET                            0x00000008
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_B_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_B_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_B_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_B_OFFSET                                     0x00000008
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_B_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_B_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_B_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_2_OFFSET                                      0x00000008
#define VHT_SIG_B_SU160_INFO_RESERVED_2_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_2_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_2_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_B_OFFSET                                   0x00000008
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_B_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_B_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_B_MASK                                     0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_C_OFFSET                                   0x0000000c
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_C_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_C_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_C_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_C_OFFSET                            0x0000000c
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_C_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_C_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_C_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_C_OFFSET                                     0x0000000c
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_C_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_C_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_C_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_3_OFFSET                                      0x0000000c
#define VHT_SIG_B_SU160_INFO_RESERVED_3_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_3_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_3_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_C_OFFSET                                   0x0000000c
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_C_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_C_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_C_MASK                                     0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_D_OFFSET                                   0x00000010
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_D_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_D_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_D_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_D_OFFSET                            0x00000010
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_D_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_D_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_D_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_D_OFFSET                                     0x00000010
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_D_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_D_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_D_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_4_OFFSET                                      0x00000010
#define VHT_SIG_B_SU160_INFO_RESERVED_4_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_4_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_4_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_D_OFFSET                                   0x00000010
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_D_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_D_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_D_MASK                                     0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_E_OFFSET                                   0x00000014
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_E_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_E_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_E_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_E_OFFSET                            0x00000014
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_E_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_E_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_E_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_E_OFFSET                                     0x00000014
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_E_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_E_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_E_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_5_OFFSET                                      0x00000014
#define VHT_SIG_B_SU160_INFO_RESERVED_5_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_5_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_5_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_E_OFFSET                                   0x00000014
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_E_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_E_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_E_MASK                                     0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_F_OFFSET                                   0x00000018
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_F_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_F_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_F_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_F_OFFSET                            0x00000018
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_F_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_F_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_F_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_F_OFFSET                                     0x00000018
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_F_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_F_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_F_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_6_OFFSET                                      0x00000018
#define VHT_SIG_B_SU160_INFO_RESERVED_6_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_6_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_6_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_F_OFFSET                                   0x00000018
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_F_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_F_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_F_MASK                                     0x80000000

#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_G_OFFSET                                   0x0000001c
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_G_LSB                                      0
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_G_MSB                                      20
#define VHT_SIG_B_SU160_INFO_LENGTH_COPY_G_MASK                                     0x001fffff

#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_G_OFFSET                            0x0000001c
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_G_LSB                               21
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_G_MSB                               22
#define VHT_SIG_B_SU160_INFO_VHTB_RESERVED_COPY_G_MASK                              0x00600000

#define VHT_SIG_B_SU160_INFO_TAIL_COPY_G_OFFSET                                     0x0000001c
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_G_LSB                                        23
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_G_MSB                                        28
#define VHT_SIG_B_SU160_INFO_TAIL_COPY_G_MASK                                       0x1f800000

#define VHT_SIG_B_SU160_INFO_RESERVED_7_OFFSET                                      0x0000001c
#define VHT_SIG_B_SU160_INFO_RESERVED_7_LSB                                         29
#define VHT_SIG_B_SU160_INFO_RESERVED_7_MSB                                         30
#define VHT_SIG_B_SU160_INFO_RESERVED_7_MASK                                        0x60000000

#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_G_OFFSET                                   0x0000001c
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_G_LSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_G_MSB                                      31
#define VHT_SIG_B_SU160_INFO_RX_NDP_COPY_G_MASK                                     0x80000000

#endif

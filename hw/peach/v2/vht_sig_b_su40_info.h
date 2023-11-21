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


#ifndef _VHT_SIG_B_SU40_INFO_H_
#define _VHT_SIG_B_SU40_INFO_H_

#define NUM_OF_DWORDS_VHT_SIG_B_SU40_INFO 2

struct vht_sig_b_su40_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 19,
                      vhtb_reserved                                           :  2,
                      tail                                                    :  6,
                      reserved                                                :  4,
                      rx_ndp                                                  :  1;
             uint32_t length_copy                                             : 19,
                      vhtb_reserved_copy                                      :  2,
                      tail_copy                                               :  6,
                      reserved_copy                                           :  4,
                      rx_ndp_copy                                             :  1;
#else
             uint32_t rx_ndp                                                  :  1,
                      reserved                                                :  4,
                      tail                                                    :  6,
                      vhtb_reserved                                           :  2,
                      length                                                  : 19;
             uint32_t rx_ndp_copy                                             :  1,
                      reserved_copy                                           :  4,
                      tail_copy                                               :  6,
                      vhtb_reserved_copy                                      :  2,
                      length_copy                                             : 19;
#endif
};

#define VHT_SIG_B_SU40_INFO_LENGTH_OFFSET                                           0x00000000
#define VHT_SIG_B_SU40_INFO_LENGTH_LSB                                              0
#define VHT_SIG_B_SU40_INFO_LENGTH_MSB                                              18
#define VHT_SIG_B_SU40_INFO_LENGTH_MASK                                             0x0007ffff

#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_OFFSET                                    0x00000000
#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_LSB                                       19
#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_MSB                                       20
#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_MASK                                      0x00180000

#define VHT_SIG_B_SU40_INFO_TAIL_OFFSET                                             0x00000000
#define VHT_SIG_B_SU40_INFO_TAIL_LSB                                                21
#define VHT_SIG_B_SU40_INFO_TAIL_MSB                                                26
#define VHT_SIG_B_SU40_INFO_TAIL_MASK                                               0x07e00000

#define VHT_SIG_B_SU40_INFO_RESERVED_OFFSET                                         0x00000000
#define VHT_SIG_B_SU40_INFO_RESERVED_LSB                                            27
#define VHT_SIG_B_SU40_INFO_RESERVED_MSB                                            30
#define VHT_SIG_B_SU40_INFO_RESERVED_MASK                                           0x78000000

#define VHT_SIG_B_SU40_INFO_RX_NDP_OFFSET                                           0x00000000
#define VHT_SIG_B_SU40_INFO_RX_NDP_LSB                                              31
#define VHT_SIG_B_SU40_INFO_RX_NDP_MSB                                              31
#define VHT_SIG_B_SU40_INFO_RX_NDP_MASK                                             0x80000000

#define VHT_SIG_B_SU40_INFO_LENGTH_COPY_OFFSET                                      0x00000004
#define VHT_SIG_B_SU40_INFO_LENGTH_COPY_LSB                                         0
#define VHT_SIG_B_SU40_INFO_LENGTH_COPY_MSB                                         18
#define VHT_SIG_B_SU40_INFO_LENGTH_COPY_MASK                                        0x0007ffff

#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_COPY_OFFSET                               0x00000004
#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_COPY_LSB                                  19
#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_COPY_MSB                                  20
#define VHT_SIG_B_SU40_INFO_VHTB_RESERVED_COPY_MASK                                 0x00180000

#define VHT_SIG_B_SU40_INFO_TAIL_COPY_OFFSET                                        0x00000004
#define VHT_SIG_B_SU40_INFO_TAIL_COPY_LSB                                           21
#define VHT_SIG_B_SU40_INFO_TAIL_COPY_MSB                                           26
#define VHT_SIG_B_SU40_INFO_TAIL_COPY_MASK                                          0x07e00000

#define VHT_SIG_B_SU40_INFO_RESERVED_COPY_OFFSET                                    0x00000004
#define VHT_SIG_B_SU40_INFO_RESERVED_COPY_LSB                                       27
#define VHT_SIG_B_SU40_INFO_RESERVED_COPY_MSB                                       30
#define VHT_SIG_B_SU40_INFO_RESERVED_COPY_MASK                                      0x78000000

#define VHT_SIG_B_SU40_INFO_RX_NDP_COPY_OFFSET                                      0x00000004
#define VHT_SIG_B_SU40_INFO_RX_NDP_COPY_LSB                                         31
#define VHT_SIG_B_SU40_INFO_RX_NDP_COPY_MSB                                         31
#define VHT_SIG_B_SU40_INFO_RX_NDP_COPY_MASK                                        0x80000000

#endif

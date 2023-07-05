
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



#ifndef _HT_SIG_INFO_H_
#define _HT_SIG_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HT_SIG_INFO 2

struct ht_sig_info {
             uint32_t mcs                             :  7,
                      cbw                             :  1,
                      length                          : 16,
                      reserved_0                      :  8;
             uint32_t smoothing                       :  1,
                      not_sounding                    :  1,
                      ht_reserved                     :  1,
                      aggregation                     :  1,
                      stbc                            :  2,
                      fec_coding                      :  1,
                      short_gi                        :  1,
                      num_ext_sp_str                  :  2,
                      crc                             :  8,
                      signal_tail                     :  6,
                      reserved_1                      :  8;
};

#define HT_SIG_INFO_0_MCS_OFFSET                                     0x00000000
#define HT_SIG_INFO_0_MCS_LSB                                        0
#define HT_SIG_INFO_0_MCS_MASK                                       0x0000007f

#define HT_SIG_INFO_0_CBW_OFFSET                                     0x00000000
#define HT_SIG_INFO_0_CBW_LSB                                        7
#define HT_SIG_INFO_0_CBW_MASK                                       0x00000080

#define HT_SIG_INFO_0_LENGTH_OFFSET                                  0x00000000
#define HT_SIG_INFO_0_LENGTH_LSB                                     8
#define HT_SIG_INFO_0_LENGTH_MASK                                    0x00ffff00

#define HT_SIG_INFO_0_RESERVED_0_OFFSET                              0x00000000
#define HT_SIG_INFO_0_RESERVED_0_LSB                                 24
#define HT_SIG_INFO_0_RESERVED_0_MASK                                0xff000000

#define HT_SIG_INFO_1_SMOOTHING_OFFSET                               0x00000004
#define HT_SIG_INFO_1_SMOOTHING_LSB                                  0
#define HT_SIG_INFO_1_SMOOTHING_MASK                                 0x00000001

#define HT_SIG_INFO_1_NOT_SOUNDING_OFFSET                            0x00000004
#define HT_SIG_INFO_1_NOT_SOUNDING_LSB                               1
#define HT_SIG_INFO_1_NOT_SOUNDING_MASK                              0x00000002

#define HT_SIG_INFO_1_HT_RESERVED_OFFSET                             0x00000004
#define HT_SIG_INFO_1_HT_RESERVED_LSB                                2
#define HT_SIG_INFO_1_HT_RESERVED_MASK                               0x00000004

#define HT_SIG_INFO_1_AGGREGATION_OFFSET                             0x00000004
#define HT_SIG_INFO_1_AGGREGATION_LSB                                3
#define HT_SIG_INFO_1_AGGREGATION_MASK                               0x00000008

#define HT_SIG_INFO_1_STBC_OFFSET                                    0x00000004
#define HT_SIG_INFO_1_STBC_LSB                                       4
#define HT_SIG_INFO_1_STBC_MASK                                      0x00000030

#define HT_SIG_INFO_1_FEC_CODING_OFFSET                              0x00000004
#define HT_SIG_INFO_1_FEC_CODING_LSB                                 6
#define HT_SIG_INFO_1_FEC_CODING_MASK                                0x00000040

#define HT_SIG_INFO_1_SHORT_GI_OFFSET                                0x00000004
#define HT_SIG_INFO_1_SHORT_GI_LSB                                   7
#define HT_SIG_INFO_1_SHORT_GI_MASK                                  0x00000080

#define HT_SIG_INFO_1_NUM_EXT_SP_STR_OFFSET                          0x00000004
#define HT_SIG_INFO_1_NUM_EXT_SP_STR_LSB                             8
#define HT_SIG_INFO_1_NUM_EXT_SP_STR_MASK                            0x00000300

#define HT_SIG_INFO_1_CRC_OFFSET                                     0x00000004
#define HT_SIG_INFO_1_CRC_LSB                                        10
#define HT_SIG_INFO_1_CRC_MASK                                       0x0003fc00

#define HT_SIG_INFO_1_SIGNAL_TAIL_OFFSET                             0x00000004
#define HT_SIG_INFO_1_SIGNAL_TAIL_LSB                                18
#define HT_SIG_INFO_1_SIGNAL_TAIL_MASK                               0x00fc0000

#define HT_SIG_INFO_1_RESERVED_1_OFFSET                              0x00000004
#define HT_SIG_INFO_1_RESERVED_1_LSB                                 24
#define HT_SIG_INFO_1_RESERVED_1_MASK                                0xff000000

#endif

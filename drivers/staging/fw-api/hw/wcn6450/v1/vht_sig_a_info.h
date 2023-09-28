
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



#ifndef _VHT_SIG_A_INFO_H_
#define _VHT_SIG_A_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_A_INFO 2

struct vht_sig_a_info {
             uint32_t bandwidth                       :  2,
                      vhta_reserved_0                 :  1,
                      stbc                            :  1,
                      group_id                        :  6,
                      n_sts                           : 12,
                      txop_ps_not_allowed             :  1,
                      vhta_reserved_0b                :  1,
                      reserved_0                      :  8;
             uint32_t gi_setting                      :  2,
                      su_mu_coding                    :  1,
                      ldpc_extra_symbol               :  1,
                      mcs                             :  4,
                      beamformed                      :  1,
                      vhta_reserved_1                 :  1,
                      crc                             :  8,
                      tail                            :  6,
                      reserved_1                      :  8;
};

#define VHT_SIG_A_INFO_0_BANDWIDTH_OFFSET                            0x00000000
#define VHT_SIG_A_INFO_0_BANDWIDTH_LSB                               0
#define VHT_SIG_A_INFO_0_BANDWIDTH_MASK                              0x00000003

#define VHT_SIG_A_INFO_0_VHTA_RESERVED_0_OFFSET                      0x00000000
#define VHT_SIG_A_INFO_0_VHTA_RESERVED_0_LSB                         2
#define VHT_SIG_A_INFO_0_VHTA_RESERVED_0_MASK                        0x00000004

#define VHT_SIG_A_INFO_0_STBC_OFFSET                                 0x00000000
#define VHT_SIG_A_INFO_0_STBC_LSB                                    3
#define VHT_SIG_A_INFO_0_STBC_MASK                                   0x00000008

#define VHT_SIG_A_INFO_0_GROUP_ID_OFFSET                             0x00000000
#define VHT_SIG_A_INFO_0_GROUP_ID_LSB                                4
#define VHT_SIG_A_INFO_0_GROUP_ID_MASK                               0x000003f0

#define VHT_SIG_A_INFO_0_N_STS_OFFSET                                0x00000000
#define VHT_SIG_A_INFO_0_N_STS_LSB                                   10
#define VHT_SIG_A_INFO_0_N_STS_MASK                                  0x003ffc00

#define VHT_SIG_A_INFO_0_TXOP_PS_NOT_ALLOWED_OFFSET                  0x00000000
#define VHT_SIG_A_INFO_0_TXOP_PS_NOT_ALLOWED_LSB                     22
#define VHT_SIG_A_INFO_0_TXOP_PS_NOT_ALLOWED_MASK                    0x00400000

#define VHT_SIG_A_INFO_0_VHTA_RESERVED_0B_OFFSET                     0x00000000
#define VHT_SIG_A_INFO_0_VHTA_RESERVED_0B_LSB                        23
#define VHT_SIG_A_INFO_0_VHTA_RESERVED_0B_MASK                       0x00800000

#define VHT_SIG_A_INFO_0_RESERVED_0_OFFSET                           0x00000000
#define VHT_SIG_A_INFO_0_RESERVED_0_LSB                              24
#define VHT_SIG_A_INFO_0_RESERVED_0_MASK                             0xff000000

#define VHT_SIG_A_INFO_1_GI_SETTING_OFFSET                           0x00000004
#define VHT_SIG_A_INFO_1_GI_SETTING_LSB                              0
#define VHT_SIG_A_INFO_1_GI_SETTING_MASK                             0x00000003

#define VHT_SIG_A_INFO_1_SU_MU_CODING_OFFSET                         0x00000004
#define VHT_SIG_A_INFO_1_SU_MU_CODING_LSB                            2
#define VHT_SIG_A_INFO_1_SU_MU_CODING_MASK                           0x00000004

#define VHT_SIG_A_INFO_1_LDPC_EXTRA_SYMBOL_OFFSET                    0x00000004
#define VHT_SIG_A_INFO_1_LDPC_EXTRA_SYMBOL_LSB                       3
#define VHT_SIG_A_INFO_1_LDPC_EXTRA_SYMBOL_MASK                      0x00000008

#define VHT_SIG_A_INFO_1_MCS_OFFSET                                  0x00000004
#define VHT_SIG_A_INFO_1_MCS_LSB                                     4
#define VHT_SIG_A_INFO_1_MCS_MASK                                    0x000000f0

#define VHT_SIG_A_INFO_1_BEAMFORMED_OFFSET                           0x00000004
#define VHT_SIG_A_INFO_1_BEAMFORMED_LSB                              8
#define VHT_SIG_A_INFO_1_BEAMFORMED_MASK                             0x00000100

#define VHT_SIG_A_INFO_1_VHTA_RESERVED_1_OFFSET                      0x00000004
#define VHT_SIG_A_INFO_1_VHTA_RESERVED_1_LSB                         9
#define VHT_SIG_A_INFO_1_VHTA_RESERVED_1_MASK                        0x00000200

#define VHT_SIG_A_INFO_1_CRC_OFFSET                                  0x00000004
#define VHT_SIG_A_INFO_1_CRC_LSB                                     10
#define VHT_SIG_A_INFO_1_CRC_MASK                                    0x0003fc00

#define VHT_SIG_A_INFO_1_TAIL_OFFSET                                 0x00000004
#define VHT_SIG_A_INFO_1_TAIL_LSB                                    18
#define VHT_SIG_A_INFO_1_TAIL_MASK                                   0x00fc0000

#define VHT_SIG_A_INFO_1_RESERVED_1_OFFSET                           0x00000004
#define VHT_SIG_A_INFO_1_RESERVED_1_LSB                              24
#define VHT_SIG_A_INFO_1_RESERVED_1_MASK                             0xff000000

#endif

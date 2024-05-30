
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



#ifndef _RECEIVE_RSSI_INFO_H_
#define _RECEIVE_RSSI_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RECEIVE_RSSI_INFO 16

struct receive_rssi_info {
             uint32_t rssi_pri20_chain0               :  8,
                      rssi_ext20_chain0               :  8,
                      rssi_ext40_low20_chain0         :  8,
                      rssi_ext40_high20_chain0        :  8;
             uint32_t rssi_ext80_low20_chain0         :  8,
                      rssi_ext80_low_high20_chain0    :  8,
                      rssi_ext80_high_low20_chain0    :  8,
                      rssi_ext80_high20_chain0        :  8;
             uint32_t rssi_pri20_chain1               :  8,
                      rssi_ext20_chain1               :  8,
                      rssi_ext40_low20_chain1         :  8,
                      rssi_ext40_high20_chain1        :  8;
             uint32_t rssi_ext80_low20_chain1         :  8,
                      rssi_ext80_low_high20_chain1    :  8,
                      rssi_ext80_high_low20_chain1    :  8,
                      rssi_ext80_high20_chain1        :  8;
             uint32_t rssi_pri20_chain2               :  8,
                      rssi_ext20_chain2               :  8,
                      rssi_ext40_low20_chain2         :  8,
                      rssi_ext40_high20_chain2        :  8;
             uint32_t rssi_ext80_low20_chain2         :  8,
                      rssi_ext80_low_high20_chain2    :  8,
                      rssi_ext80_high_low20_chain2    :  8,
                      rssi_ext80_high20_chain2        :  8;
             uint32_t rssi_pri20_chain3               :  8,
                      rssi_ext20_chain3               :  8,
                      rssi_ext40_low20_chain3         :  8,
                      rssi_ext40_high20_chain3        :  8;
             uint32_t rssi_ext80_low20_chain3         :  8,
                      rssi_ext80_low_high20_chain3    :  8,
                      rssi_ext80_high_low20_chain3    :  8,
                      rssi_ext80_high20_chain3        :  8;
             uint32_t rssi_pri20_chain4               :  8,
                      rssi_ext20_chain4               :  8,
                      rssi_ext40_low20_chain4         :  8,
                      rssi_ext40_high20_chain4        :  8;
             uint32_t rssi_ext80_low20_chain4         :  8,
                      rssi_ext80_low_high20_chain4    :  8,
                      rssi_ext80_high_low20_chain4    :  8,
                      rssi_ext80_high20_chain4        :  8;
             uint32_t rssi_pri20_chain5               :  8,
                      rssi_ext20_chain5               :  8,
                      rssi_ext40_low20_chain5         :  8,
                      rssi_ext40_high20_chain5        :  8;
             uint32_t rssi_ext80_low20_chain5         :  8,
                      rssi_ext80_low_high20_chain5    :  8,
                      rssi_ext80_high_low20_chain5    :  8,
                      rssi_ext80_high20_chain5        :  8;
             uint32_t rssi_pri20_chain6               :  8,
                      rssi_ext20_chain6               :  8,
                      rssi_ext40_low20_chain6         :  8,
                      rssi_ext40_high20_chain6        :  8;
             uint32_t rssi_ext80_low20_chain6         :  8,
                      rssi_ext80_low_high20_chain6    :  8,
                      rssi_ext80_high_low20_chain6    :  8,
                      rssi_ext80_high20_chain6        :  8;
             uint32_t rssi_pri20_chain7               :  8,
                      rssi_ext20_chain7               :  8,
                      rssi_ext40_low20_chain7         :  8,
                      rssi_ext40_high20_chain7        :  8;
             uint32_t rssi_ext80_low20_chain7         :  8,
                      rssi_ext80_low_high20_chain7    :  8,
                      rssi_ext80_high_low20_chain7    :  8,
                      rssi_ext80_high20_chain7        :  8;
};

#define RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0_OFFSET                 0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0_LSB                    0
#define RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0_MASK                   0x000000ff

#define RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0_OFFSET                 0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0_LSB                    8
#define RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0_MASK                   0x0000ff00

#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0_OFFSET           0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0_LSB              16
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0_MASK             0x00ff0000

#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0_OFFSET          0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0_LSB             24
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0_MASK            0xff000000

#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0_OFFSET           0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0_LSB              0
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0_MASK             0x000000ff

#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET      0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB         8
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK        0x0000ff00

#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET      0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB         16
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK        0x00ff0000

#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0_OFFSET          0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0_LSB             24
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0_MASK            0xff000000

#define RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1_OFFSET                 0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1_LSB                    0
#define RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1_MASK                   0x000000ff

#define RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1_OFFSET                 0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1_LSB                    8
#define RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1_MASK                   0x0000ff00

#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1_OFFSET           0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1_LSB              16
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1_MASK             0x00ff0000

#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1_OFFSET          0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1_LSB             24
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1_MASK            0xff000000

#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1_OFFSET           0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1_LSB              0
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1_MASK             0x000000ff

#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET      0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB         8
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK        0x0000ff00

#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET      0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB         16
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK        0x00ff0000

#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1_OFFSET          0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1_LSB             24
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1_MASK            0xff000000

#define RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2_OFFSET                 0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2_LSB                    0
#define RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2_MASK                   0x000000ff

#define RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2_OFFSET                 0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2_LSB                    8
#define RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2_MASK                   0x0000ff00

#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2_OFFSET           0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2_LSB              16
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2_MASK             0x00ff0000

#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2_OFFSET          0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2_LSB             24
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2_MASK            0xff000000

#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2_OFFSET           0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2_LSB              0
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2_MASK             0x000000ff

#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET      0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB         8
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK        0x0000ff00

#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET      0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB         16
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK        0x00ff0000

#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2_OFFSET          0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2_LSB             24
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2_MASK            0xff000000

#define RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3_OFFSET                 0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3_LSB                    0
#define RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3_MASK                   0x000000ff

#define RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3_OFFSET                 0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3_LSB                    8
#define RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3_MASK                   0x0000ff00

#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3_OFFSET           0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3_LSB              16
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3_MASK             0x00ff0000

#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3_OFFSET          0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3_LSB             24
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3_MASK            0xff000000

#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3_OFFSET           0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3_LSB              0
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3_MASK             0x000000ff

#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET      0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB         8
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK        0x0000ff00

#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET      0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB         16
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK        0x00ff0000

#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3_OFFSET          0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3_LSB             24
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3_MASK            0xff000000

#define RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4_OFFSET                 0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4_LSB                    0
#define RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4_MASK                   0x000000ff

#define RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4_OFFSET                 0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4_LSB                    8
#define RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4_MASK                   0x0000ff00

#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4_OFFSET           0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4_LSB              16
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4_MASK             0x00ff0000

#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4_OFFSET          0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4_LSB             24
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4_MASK            0xff000000

#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4_OFFSET           0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4_LSB              0
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4_MASK             0x000000ff

#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4_OFFSET      0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4_LSB         8
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4_MASK        0x0000ff00

#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4_OFFSET      0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4_LSB         16
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4_MASK        0x00ff0000

#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4_OFFSET          0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4_LSB             24
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4_MASK            0xff000000

#define RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5_OFFSET                0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5_LSB                   0
#define RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5_MASK                  0x000000ff

#define RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5_OFFSET                0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5_LSB                   8
#define RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5_MASK                  0x0000ff00

#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5_OFFSET          0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5_LSB             16
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5_MASK            0x00ff0000

#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5_OFFSET         0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5_LSB            24
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5_MASK           0xff000000

#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5_OFFSET          0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5_LSB             0
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5_MASK            0x000000ff

#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5_OFFSET     0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5_LSB        8
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5_MASK       0x0000ff00

#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5_OFFSET     0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5_LSB        16
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5_MASK       0x00ff0000

#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5_OFFSET         0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5_LSB            24
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5_MASK           0xff000000

#define RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6_OFFSET                0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6_LSB                   0
#define RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6_MASK                  0x000000ff

#define RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6_OFFSET                0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6_LSB                   8
#define RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6_MASK                  0x0000ff00

#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6_OFFSET          0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6_LSB             16
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6_MASK            0x00ff0000

#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6_OFFSET         0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6_LSB            24
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6_MASK           0xff000000

#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6_OFFSET          0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6_LSB             0
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6_MASK            0x000000ff

#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6_OFFSET     0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6_LSB        8
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6_MASK       0x0000ff00

#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6_OFFSET     0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6_LSB        16
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6_MASK       0x00ff0000

#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6_OFFSET         0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6_LSB            24
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6_MASK           0xff000000

#define RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7_OFFSET                0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7_LSB                   0
#define RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7_MASK                  0x000000ff

#define RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7_OFFSET                0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7_LSB                   8
#define RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7_MASK                  0x0000ff00

#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7_OFFSET          0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7_LSB             16
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7_MASK            0x00ff0000

#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7_OFFSET         0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7_LSB            24
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7_MASK           0xff000000

#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7_OFFSET          0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7_LSB             0
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7_MASK            0x000000ff

#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7_OFFSET     0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7_LSB        8
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7_MASK       0x0000ff00

#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7_OFFSET     0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7_LSB        16
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7_MASK       0x00ff0000

#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7_OFFSET         0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7_LSB            24
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7_MASK           0xff000000

#endif


/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rssi_pri20_chain0                                       :  8,  
                      rssi_ext20_chain0                                       :  8,  
                      rssi_ext40_low20_chain0                                 :  8,  
                      rssi_ext40_high20_chain0                                :  8;  
             uint32_t rssi_ext80_low20_chain0                                 :  8,  
                      rssi_ext80_low_high20_chain0                            :  8,  
                      rssi_ext80_high_low20_chain0                            :  8,  
                      rssi_ext80_high20_chain0                                :  8;  
             uint32_t rssi_ext160_0_chain0                                    :  8,  
                      rssi_ext160_1_chain0                                    :  8,  
                      rssi_ext160_2_chain0                                    :  8,  
                      rssi_ext160_3_chain0                                    :  8;  
             uint32_t rssi_ext160_4_chain0                                    :  8,  
                      rssi_ext160_5_chain0                                    :  8,  
                      rssi_ext160_6_chain0                                    :  8,  
                      rssi_ext160_7_chain0                                    :  8;  
             uint32_t rssi_pri20_chain1                                       :  8,  
                      rssi_ext20_chain1                                       :  8,  
                      rssi_ext40_low20_chain1                                 :  8,  
                      rssi_ext40_high20_chain1                                :  8;  
             uint32_t rssi_ext80_low20_chain1                                 :  8,  
                      rssi_ext80_low_high20_chain1                            :  8,  
                      rssi_ext80_high_low20_chain1                            :  8,  
                      rssi_ext80_high20_chain1                                :  8;  
             uint32_t rssi_ext160_0_chain1                                    :  8,  
                      rssi_ext160_1_chain1                                    :  8,  
                      rssi_ext160_2_chain1                                    :  8,  
                      rssi_ext160_3_chain1                                    :  8;  
             uint32_t rssi_ext160_4_chain1                                    :  8,  
                      rssi_ext160_5_chain1                                    :  8,  
                      rssi_ext160_6_chain1                                    :  8,  
                      rssi_ext160_7_chain1                                    :  8;  
             uint32_t rssi_pri20_chain2                                       :  8,  
                      rssi_ext20_chain2                                       :  8,  
                      rssi_ext40_low20_chain2                                 :  8,  
                      rssi_ext40_high20_chain2                                :  8;  
             uint32_t rssi_ext80_low20_chain2                                 :  8,  
                      rssi_ext80_low_high20_chain2                            :  8,  
                      rssi_ext80_high_low20_chain2                            :  8,  
                      rssi_ext80_high20_chain2                                :  8;  
             uint32_t rssi_ext160_0_chain2                                    :  8,  
                      rssi_ext160_1_chain2                                    :  8,  
                      rssi_ext160_2_chain2                                    :  8,  
                      rssi_ext160_3_chain2                                    :  8;  
             uint32_t rssi_ext160_4_chain2                                    :  8,  
                      rssi_ext160_5_chain2                                    :  8,  
                      rssi_ext160_6_chain2                                    :  8,  
                      rssi_ext160_7_chain2                                    :  8;  
             uint32_t rssi_pri20_chain3                                       :  8,  
                      rssi_ext20_chain3                                       :  8,  
                      rssi_ext40_low20_chain3                                 :  8,  
                      rssi_ext40_high20_chain3                                :  8;  
             uint32_t rssi_ext80_low20_chain3                                 :  8,  
                      rssi_ext80_low_high20_chain3                            :  8,  
                      rssi_ext80_high_low20_chain3                            :  8,  
                      rssi_ext80_high20_chain3                                :  8;  
             uint32_t rssi_ext160_0_chain3                                    :  8,  
                      rssi_ext160_1_chain3                                    :  8,  
                      rssi_ext160_2_chain3                                    :  8,  
                      rssi_ext160_3_chain3                                    :  8;  
             uint32_t rssi_ext160_4_chain3                                    :  8,  
                      rssi_ext160_5_chain3                                    :  8,  
                      rssi_ext160_6_chain3                                    :  8,  
                      rssi_ext160_7_chain3                                    :  8;  
#else
             uint32_t rssi_ext40_high20_chain0                                :  8,  
                      rssi_ext40_low20_chain0                                 :  8,  
                      rssi_ext20_chain0                                       :  8,  
                      rssi_pri20_chain0                                       :  8;  
             uint32_t rssi_ext80_high20_chain0                                :  8,  
                      rssi_ext80_high_low20_chain0                            :  8,  
                      rssi_ext80_low_high20_chain0                            :  8,  
                      rssi_ext80_low20_chain0                                 :  8;  
             uint32_t rssi_ext160_3_chain0                                    :  8,  
                      rssi_ext160_2_chain0                                    :  8,  
                      rssi_ext160_1_chain0                                    :  8,  
                      rssi_ext160_0_chain0                                    :  8;  
             uint32_t rssi_ext160_7_chain0                                    :  8,  
                      rssi_ext160_6_chain0                                    :  8,  
                      rssi_ext160_5_chain0                                    :  8,  
                      rssi_ext160_4_chain0                                    :  8;  
             uint32_t rssi_ext40_high20_chain1                                :  8,  
                      rssi_ext40_low20_chain1                                 :  8,  
                      rssi_ext20_chain1                                       :  8,  
                      rssi_pri20_chain1                                       :  8;  
             uint32_t rssi_ext80_high20_chain1                                :  8,  
                      rssi_ext80_high_low20_chain1                            :  8,  
                      rssi_ext80_low_high20_chain1                            :  8,  
                      rssi_ext80_low20_chain1                                 :  8;  
             uint32_t rssi_ext160_3_chain1                                    :  8,  
                      rssi_ext160_2_chain1                                    :  8,  
                      rssi_ext160_1_chain1                                    :  8,  
                      rssi_ext160_0_chain1                                    :  8;  
             uint32_t rssi_ext160_7_chain1                                    :  8,  
                      rssi_ext160_6_chain1                                    :  8,  
                      rssi_ext160_5_chain1                                    :  8,  
                      rssi_ext160_4_chain1                                    :  8;  
             uint32_t rssi_ext40_high20_chain2                                :  8,  
                      rssi_ext40_low20_chain2                                 :  8,  
                      rssi_ext20_chain2                                       :  8,  
                      rssi_pri20_chain2                                       :  8;  
             uint32_t rssi_ext80_high20_chain2                                :  8,  
                      rssi_ext80_high_low20_chain2                            :  8,  
                      rssi_ext80_low_high20_chain2                            :  8,  
                      rssi_ext80_low20_chain2                                 :  8;  
             uint32_t rssi_ext160_3_chain2                                    :  8,  
                      rssi_ext160_2_chain2                                    :  8,  
                      rssi_ext160_1_chain2                                    :  8,  
                      rssi_ext160_0_chain2                                    :  8;  
             uint32_t rssi_ext160_7_chain2                                    :  8,  
                      rssi_ext160_6_chain2                                    :  8,  
                      rssi_ext160_5_chain2                                    :  8,  
                      rssi_ext160_4_chain2                                    :  8;  
             uint32_t rssi_ext40_high20_chain3                                :  8,  
                      rssi_ext40_low20_chain3                                 :  8,  
                      rssi_ext20_chain3                                       :  8,  
                      rssi_pri20_chain3                                       :  8;  
             uint32_t rssi_ext80_high20_chain3                                :  8,  
                      rssi_ext80_high_low20_chain3                            :  8,  
                      rssi_ext80_low_high20_chain3                            :  8,  
                      rssi_ext80_low20_chain3                                 :  8;  
             uint32_t rssi_ext160_3_chain3                                    :  8,  
                      rssi_ext160_2_chain3                                    :  8,  
                      rssi_ext160_1_chain3                                    :  8,  
                      rssi_ext160_0_chain3                                    :  8;  
             uint32_t rssi_ext160_7_chain3                                    :  8,  
                      rssi_ext160_6_chain3                                    :  8,  
                      rssi_ext160_5_chain3                                    :  8,  
                      rssi_ext160_4_chain3                                    :  8;  
#endif
};

#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN0_OFFSET                                  0x00000000
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN0_LSB                                     0
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN0_MSB                                     7
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN0_MASK                                    0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN0_OFFSET                                  0x00000000
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN0_LSB                                     8
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN0_MSB                                     15
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN0_MASK                                    0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN0_OFFSET                            0x00000000
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN0_LSB                               16
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN0_MSB                               23
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN0_MASK                              0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN0_OFFSET                           0x00000000
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN0_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN0_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN0_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN0_OFFSET                            0x00000004
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN0_LSB                               0
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN0_MSB                               7
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN0_MASK                              0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET                       0x00000004
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB                          8
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN0_MSB                          15
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK                         0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET                       0x00000004
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB                          16
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN0_MSB                          23
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK                         0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN0_OFFSET                           0x00000004
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN0_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN0_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN0_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN0_OFFSET                               0x00000008
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN0_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN0_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN0_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN0_OFFSET                               0x00000008
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN0_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN0_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN0_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN0_OFFSET                               0x00000008
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN0_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN0_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN0_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN0_OFFSET                               0x00000008
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN0_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN0_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN0_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN0_OFFSET                               0x0000000c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN0_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN0_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN0_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN0_OFFSET                               0x0000000c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN0_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN0_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN0_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN0_OFFSET                               0x0000000c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN0_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN0_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN0_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN0_OFFSET                               0x0000000c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN0_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN0_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN0_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN1_OFFSET                                  0x00000010
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN1_LSB                                     0
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN1_MSB                                     7
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN1_MASK                                    0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN1_OFFSET                                  0x00000010
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN1_LSB                                     8
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN1_MSB                                     15
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN1_MASK                                    0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN1_OFFSET                            0x00000010
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN1_LSB                               16
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN1_MSB                               23
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN1_MASK                              0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN1_OFFSET                           0x00000010
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN1_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN1_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN1_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN1_OFFSET                            0x00000014
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN1_LSB                               0
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN1_MSB                               7
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN1_MASK                              0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET                       0x00000014
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB                          8
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN1_MSB                          15
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK                         0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET                       0x00000014
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB                          16
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN1_MSB                          23
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK                         0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN1_OFFSET                           0x00000014
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN1_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN1_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN1_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN1_OFFSET                               0x00000018
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN1_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN1_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN1_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN1_OFFSET                               0x00000018
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN1_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN1_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN1_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN1_OFFSET                               0x00000018
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN1_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN1_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN1_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN1_OFFSET                               0x00000018
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN1_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN1_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN1_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN1_OFFSET                               0x0000001c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN1_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN1_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN1_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN1_OFFSET                               0x0000001c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN1_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN1_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN1_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN1_OFFSET                               0x0000001c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN1_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN1_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN1_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN1_OFFSET                               0x0000001c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN1_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN1_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN1_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN2_OFFSET                                  0x00000020
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN2_LSB                                     0
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN2_MSB                                     7
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN2_MASK                                    0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN2_OFFSET                                  0x00000020
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN2_LSB                                     8
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN2_MSB                                     15
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN2_MASK                                    0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN2_OFFSET                            0x00000020
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN2_LSB                               16
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN2_MSB                               23
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN2_MASK                              0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN2_OFFSET                           0x00000020
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN2_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN2_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN2_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN2_OFFSET                            0x00000024
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN2_LSB                               0
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN2_MSB                               7
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN2_MASK                              0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET                       0x00000024
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB                          8
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN2_MSB                          15
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK                         0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET                       0x00000024
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB                          16
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN2_MSB                          23
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK                         0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN2_OFFSET                           0x00000024
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN2_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN2_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN2_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN2_OFFSET                               0x00000028
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN2_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN2_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN2_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN2_OFFSET                               0x00000028
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN2_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN2_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN2_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN2_OFFSET                               0x00000028
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN2_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN2_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN2_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN2_OFFSET                               0x00000028
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN2_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN2_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN2_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN2_OFFSET                               0x0000002c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN2_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN2_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN2_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN2_OFFSET                               0x0000002c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN2_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN2_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN2_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN2_OFFSET                               0x0000002c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN2_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN2_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN2_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN2_OFFSET                               0x0000002c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN2_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN2_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN2_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN3_OFFSET                                  0x00000030
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN3_LSB                                     0
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN3_MSB                                     7
#define RECEIVE_RSSI_INFO_RSSI_PRI20_CHAIN3_MASK                                    0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN3_OFFSET                                  0x00000030
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN3_LSB                                     8
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN3_MSB                                     15
#define RECEIVE_RSSI_INFO_RSSI_EXT20_CHAIN3_MASK                                    0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN3_OFFSET                            0x00000030
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN3_LSB                               16
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN3_MSB                               23
#define RECEIVE_RSSI_INFO_RSSI_EXT40_LOW20_CHAIN3_MASK                              0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN3_OFFSET                           0x00000030
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN3_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN3_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT40_HIGH20_CHAIN3_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN3_OFFSET                            0x00000034
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN3_LSB                               0
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN3_MSB                               7
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW20_CHAIN3_MASK                              0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET                       0x00000034
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB                          8
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN3_MSB                          15
#define RECEIVE_RSSI_INFO_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK                         0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET                       0x00000034
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB                          16
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN3_MSB                          23
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK                         0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN3_OFFSET                           0x00000034
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN3_LSB                              24
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN3_MSB                              31
#define RECEIVE_RSSI_INFO_RSSI_EXT80_HIGH20_CHAIN3_MASK                             0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN3_OFFSET                               0x00000038
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN3_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN3_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_0_CHAIN3_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN3_OFFSET                               0x00000038
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN3_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN3_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_1_CHAIN3_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN3_OFFSET                               0x00000038
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN3_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN3_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_2_CHAIN3_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN3_OFFSET                               0x00000038
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN3_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN3_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_3_CHAIN3_MASK                                 0xff000000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN3_OFFSET                               0x0000003c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN3_LSB                                  0
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN3_MSB                                  7
#define RECEIVE_RSSI_INFO_RSSI_EXT160_4_CHAIN3_MASK                                 0x000000ff

#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN3_OFFSET                               0x0000003c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN3_LSB                                  8
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN3_MSB                                  15
#define RECEIVE_RSSI_INFO_RSSI_EXT160_5_CHAIN3_MASK                                 0x0000ff00

#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN3_OFFSET                               0x0000003c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN3_LSB                                  16
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN3_MSB                                  23
#define RECEIVE_RSSI_INFO_RSSI_EXT160_6_CHAIN3_MASK                                 0x00ff0000

#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN3_OFFSET                               0x0000003c
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN3_LSB                                  24
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN3_MSB                                  31
#define RECEIVE_RSSI_INFO_RSSI_EXT160_7_CHAIN3_MASK                                 0xff000000

#endif

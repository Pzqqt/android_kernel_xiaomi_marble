/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _PHYTX_PKT_END_H_
#define _PHYTX_PKT_END_H_

#define NUM_OF_WORDS_PHYTX_PKT_END 26

#define NUM_OF_DWORDS_PHYTX_PKT_END 13

struct phytx_pkt_end {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t start_of_frame_timestamp_15_0                           : 16;
             uint16_t start_of_frame_timestamp_31_16                          : 16;
             uint16_t end_of_frame_timestamp_15_0                             : 16;
             uint16_t end_of_frame_timestamp_31_16                            : 16;
             uint16_t tx_group_delay                                          : 12,
                      timing_status                                           :  2,
                      phyrx_entered_nap_state                                 :  1,
                      dpdtrain_done                                           :  1;
             uint16_t transmit_delay                                          : 16;
             uint16_t tpc_dbg_info_cmn_15_0                                   : 16;
             uint16_t tpc_dbg_info_cmn_31_16                                  : 16;
             uint16_t tpc_dbg_info_cmn_47_32                                  : 16;
             uint16_t tpc_dbg_info_chn1_15_0                                  : 16;
             uint16_t tpc_dbg_info_chn1_31_16                                 : 16;
             uint16_t tpc_dbg_info_chn1_47_32                                 : 16;
             uint16_t tpc_dbg_info_chn1_63_48                                 : 16;
             uint16_t tpc_dbg_info_chn1_79_64                                 : 16;
             uint16_t tpc_dbg_info_chn2_15_0                                  : 16;
             uint16_t tpc_dbg_info_chn2_31_16                                 : 16;
             uint16_t tpc_dbg_info_chn2_47_32                                 : 16;
             uint16_t tpc_dbg_info_chn2_63_48                                 : 16;
             uint16_t tpc_dbg_info_chn2_79_64                                 : 16;
             uint16_t phytx_tx_end_sw_info_15_0                               : 16;
             uint16_t phytx_tx_end_sw_info_31_16                              : 16;
             uint16_t phytx_tx_end_sw_info_47_32                              : 16;
             uint16_t phytx_tx_end_sw_info_63_48                              : 16;
             uint16_t beamform_masked_user_bitmap_15_0                        : 16;
             uint16_t beamform_masked_user_bitmap_31_16                       : 16;
             uint16_t beamform_masked_user_bitmap_36_32                       :  5,
                      reserved_23                                             : 11;
#else
             uint16_t start_of_frame_timestamp_15_0                           : 16;
             uint16_t start_of_frame_timestamp_31_16                          : 16;
             uint16_t end_of_frame_timestamp_15_0                             : 16;
             uint16_t end_of_frame_timestamp_31_16                            : 16;
             uint16_t dpdtrain_done                                           :  1,
                      phyrx_entered_nap_state                                 :  1,
                      timing_status                                           :  2,
                      tx_group_delay                                          : 12;
             uint16_t transmit_delay                                          : 16;
             uint16_t tpc_dbg_info_cmn_15_0                                   : 16;
             uint16_t tpc_dbg_info_cmn_31_16                                  : 16;
             uint16_t tpc_dbg_info_cmn_47_32                                  : 16;
             uint16_t tpc_dbg_info_chn1_15_0                                  : 16;
             uint16_t tpc_dbg_info_chn1_31_16                                 : 16;
             uint16_t tpc_dbg_info_chn1_47_32                                 : 16;
             uint16_t tpc_dbg_info_chn1_63_48                                 : 16;
             uint16_t tpc_dbg_info_chn1_79_64                                 : 16;
             uint16_t tpc_dbg_info_chn2_15_0                                  : 16;
             uint16_t tpc_dbg_info_chn2_31_16                                 : 16;
             uint16_t tpc_dbg_info_chn2_47_32                                 : 16;
             uint16_t tpc_dbg_info_chn2_63_48                                 : 16;
             uint16_t tpc_dbg_info_chn2_79_64                                 : 16;
             uint16_t phytx_tx_end_sw_info_15_0                               : 16;
             uint16_t phytx_tx_end_sw_info_31_16                              : 16;
             uint16_t phytx_tx_end_sw_info_47_32                              : 16;
             uint16_t phytx_tx_end_sw_info_63_48                              : 16;
             uint16_t beamform_masked_user_bitmap_15_0                        : 16;
             uint16_t beamform_masked_user_bitmap_31_16                       : 16;
             uint16_t reserved_23                                             : 11,
                      beamform_masked_user_bitmap_36_32                       :  5;
#endif
};

#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_15_0_OFFSET                          0x00000000
#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_15_0_LSB                             0
#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_15_0_MSB                             15
#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_15_0_MASK                            0x0000ffff

#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_31_16_OFFSET                         0x00000002
#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_31_16_LSB                            0
#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_31_16_MSB                            15
#define PHYTX_PKT_END_START_OF_FRAME_TIMESTAMP_31_16_MASK                           0x0000ffff

#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_15_0_OFFSET                            0x00000004
#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_15_0_LSB                               0
#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_15_0_MSB                               15
#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_15_0_MASK                              0x0000ffff

#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_31_16_OFFSET                           0x00000006
#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_31_16_LSB                              0
#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_31_16_MSB                              15
#define PHYTX_PKT_END_END_OF_FRAME_TIMESTAMP_31_16_MASK                             0x0000ffff

#define PHYTX_PKT_END_TX_GROUP_DELAY_OFFSET                                         0x00000008
#define PHYTX_PKT_END_TX_GROUP_DELAY_LSB                                            0
#define PHYTX_PKT_END_TX_GROUP_DELAY_MSB                                            11
#define PHYTX_PKT_END_TX_GROUP_DELAY_MASK                                           0x00000fff

#define PHYTX_PKT_END_TIMING_STATUS_OFFSET                                          0x00000008
#define PHYTX_PKT_END_TIMING_STATUS_LSB                                             12
#define PHYTX_PKT_END_TIMING_STATUS_MSB                                             13
#define PHYTX_PKT_END_TIMING_STATUS_MASK                                            0x00003000

#define PHYTX_PKT_END_PHYRX_ENTERED_NAP_STATE_OFFSET                                0x00000008
#define PHYTX_PKT_END_PHYRX_ENTERED_NAP_STATE_LSB                                   14
#define PHYTX_PKT_END_PHYRX_ENTERED_NAP_STATE_MSB                                   14
#define PHYTX_PKT_END_PHYRX_ENTERED_NAP_STATE_MASK                                  0x00004000

#define PHYTX_PKT_END_DPDTRAIN_DONE_OFFSET                                          0x00000008
#define PHYTX_PKT_END_DPDTRAIN_DONE_LSB                                             15
#define PHYTX_PKT_END_DPDTRAIN_DONE_MSB                                             15
#define PHYTX_PKT_END_DPDTRAIN_DONE_MASK                                            0x00008000

#define PHYTX_PKT_END_TRANSMIT_DELAY_OFFSET                                         0x0000000a
#define PHYTX_PKT_END_TRANSMIT_DELAY_LSB                                            0
#define PHYTX_PKT_END_TRANSMIT_DELAY_MSB                                            15
#define PHYTX_PKT_END_TRANSMIT_DELAY_MASK                                           0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_15_0_OFFSET                                  0x0000000c
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_15_0_LSB                                     0
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_15_0_MSB                                     15
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_15_0_MASK                                    0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_31_16_OFFSET                                 0x0000000e
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_31_16_LSB                                    0
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_31_16_MSB                                    15
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_31_16_MASK                                   0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_47_32_OFFSET                                 0x00000010
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_47_32_LSB                                    0
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_47_32_MSB                                    15
#define PHYTX_PKT_END_TPC_DBG_INFO_CMN_47_32_MASK                                   0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_15_0_OFFSET                                 0x00000012
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_15_0_LSB                                    0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_15_0_MSB                                    15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_15_0_MASK                                   0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_31_16_OFFSET                                0x00000014
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_31_16_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_31_16_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_31_16_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_47_32_OFFSET                                0x00000016
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_47_32_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_47_32_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_47_32_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_63_48_OFFSET                                0x00000018
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_63_48_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_63_48_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_63_48_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_79_64_OFFSET                                0x0000001a
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_79_64_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_79_64_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN1_79_64_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_15_0_OFFSET                                 0x0000001c
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_15_0_LSB                                    0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_15_0_MSB                                    15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_15_0_MASK                                   0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_31_16_OFFSET                                0x0000001e
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_31_16_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_31_16_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_31_16_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_47_32_OFFSET                                0x00000020
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_47_32_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_47_32_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_47_32_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_63_48_OFFSET                                0x00000022
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_63_48_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_63_48_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_63_48_MASK                                  0x0000ffff

#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_79_64_OFFSET                                0x00000024
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_79_64_LSB                                   0
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_79_64_MSB                                   15
#define PHYTX_PKT_END_TPC_DBG_INFO_CHN2_79_64_MASK                                  0x0000ffff

#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_15_0_OFFSET                              0x00000026
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_15_0_LSB                                 0
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_15_0_MSB                                 15
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_15_0_MASK                                0x0000ffff

#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_31_16_OFFSET                             0x00000028
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_31_16_LSB                                0
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_31_16_MSB                                15
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_31_16_MASK                               0x0000ffff

#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_47_32_OFFSET                             0x0000002a
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_47_32_LSB                                0
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_47_32_MSB                                15
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_47_32_MASK                               0x0000ffff

#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_63_48_OFFSET                             0x0000002c
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_63_48_LSB                                0
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_63_48_MSB                                15
#define PHYTX_PKT_END_PHYTX_TX_END_SW_INFO_63_48_MASK                               0x0000ffff

#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_15_0_OFFSET                       0x0000002e
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_15_0_LSB                          0
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MSB                          15
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_15_0_MASK                         0x0000ffff

#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_31_16_OFFSET                      0x00000030
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_31_16_LSB                         0
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MSB                         15
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_31_16_MASK                        0x0000ffff

#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_36_32_OFFSET                      0x00000032
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_36_32_LSB                         0
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MSB                         4
#define PHYTX_PKT_END_BEAMFORM_MASKED_USER_BITMAP_36_32_MASK                        0x0000001f

#define PHYTX_PKT_END_RESERVED_23_OFFSET                                            0x00000032
#define PHYTX_PKT_END_RESERVED_23_LSB                                               5
#define PHYTX_PKT_END_RESERVED_23_MSB                                               15
#define PHYTX_PKT_END_RESERVED_23_MASK                                              0x0000ffe0

#endif

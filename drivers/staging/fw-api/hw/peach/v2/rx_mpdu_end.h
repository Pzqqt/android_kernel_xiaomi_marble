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


#ifndef _RX_MPDU_END_H_
#define _RX_MPDU_END_H_

#define NUM_OF_DWORDS_RX_MPDU_END 4

struct rx_mpdu_end {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rxpcu_mpdu_filter_in_category                           :  2,
                      sw_frame_group_id                                       :  7,
                      reserved_0                                              :  7,
                      phy_ppdu_id                                             : 16;
             uint32_t reserved_1a                                             : 11,
                      unsup_ktype_short_frame                                 :  1,
                      rx_in_tx_decrypt_byp                                    :  1,
                      overflow_err                                            :  1,
                      mpdu_length_err                                         :  1,
                      tkip_mic_err                                            :  1,
                      decrypt_err                                             :  1,
                      unencrypted_frame_err                                   :  1,
                      pn_fields_contain_valid_info                            :  1,
                      fcs_err                                                 :  1,
                      msdu_length_err                                         :  1,
                      rxdma0_destination_ring                                 :  3,
                      rxdma1_destination_ring                                 :  3,
                      decrypt_status_code                                     :  3,
                      rx_bitmap_not_updated                                   :  1,
                      reserved_1b                                             :  1;
             uint32_t reserved_2a                                             : 15,
                      rxpcu_mgmt_sequence_nr_valid                            :  1,
                      rxpcu_mgmt_sequence_nr                                  : 16;
             uint32_t __reserved_g_0002                                       : 32;
#else
             uint32_t phy_ppdu_id                                             : 16,
                      reserved_0                                              :  7,
                      sw_frame_group_id                                       :  7,
                      rxpcu_mpdu_filter_in_category                           :  2;
             uint32_t reserved_1b                                             :  1,
                      rx_bitmap_not_updated                                   :  1,
                      decrypt_status_code                                     :  3,
                      rxdma1_destination_ring                                 :  3,
                      rxdma0_destination_ring                                 :  3,
                      msdu_length_err                                         :  1,
                      fcs_err                                                 :  1,
                      pn_fields_contain_valid_info                            :  1,
                      unencrypted_frame_err                                   :  1,
                      decrypt_err                                             :  1,
                      tkip_mic_err                                            :  1,
                      mpdu_length_err                                         :  1,
                      overflow_err                                            :  1,
                      rx_in_tx_decrypt_byp                                    :  1,
                      unsup_ktype_short_frame                                 :  1,
                      reserved_1a                                             : 11;
             uint32_t rxpcu_mgmt_sequence_nr                                  : 16,
                      rxpcu_mgmt_sequence_nr_valid                            :  1,
                      reserved_2a                                             : 15;
             uint32_t __reserved_g_0002                                       : 32;
#endif
};

#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                            0x00000000
#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                               0
#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                               1
#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                              0x00000003

#define RX_MPDU_END_SW_FRAME_GROUP_ID_OFFSET                                        0x00000000
#define RX_MPDU_END_SW_FRAME_GROUP_ID_LSB                                           2
#define RX_MPDU_END_SW_FRAME_GROUP_ID_MSB                                           8
#define RX_MPDU_END_SW_FRAME_GROUP_ID_MASK                                          0x000001fc

#define RX_MPDU_END_RESERVED_0_OFFSET                                               0x00000000
#define RX_MPDU_END_RESERVED_0_LSB                                                  9
#define RX_MPDU_END_RESERVED_0_MSB                                                  15
#define RX_MPDU_END_RESERVED_0_MASK                                                 0x0000fe00

#define RX_MPDU_END_PHY_PPDU_ID_OFFSET                                              0x00000000
#define RX_MPDU_END_PHY_PPDU_ID_LSB                                                 16
#define RX_MPDU_END_PHY_PPDU_ID_MSB                                                 31
#define RX_MPDU_END_PHY_PPDU_ID_MASK                                                0xffff0000

#define RX_MPDU_END_RESERVED_1A_OFFSET                                              0x00000004
#define RX_MPDU_END_RESERVED_1A_LSB                                                 0
#define RX_MPDU_END_RESERVED_1A_MSB                                                 10
#define RX_MPDU_END_RESERVED_1A_MASK                                                0x000007ff

#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_OFFSET                                  0x00000004
#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_LSB                                     11
#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_MSB                                     11
#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_MASK                                    0x00000800

#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_OFFSET                                     0x00000004
#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_LSB                                        12
#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_MSB                                        12
#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_MASK                                       0x00001000

#define RX_MPDU_END_OVERFLOW_ERR_OFFSET                                             0x00000004
#define RX_MPDU_END_OVERFLOW_ERR_LSB                                                13
#define RX_MPDU_END_OVERFLOW_ERR_MSB                                                13
#define RX_MPDU_END_OVERFLOW_ERR_MASK                                               0x00002000

#define RX_MPDU_END_MPDU_LENGTH_ERR_OFFSET                                          0x00000004
#define RX_MPDU_END_MPDU_LENGTH_ERR_LSB                                             14
#define RX_MPDU_END_MPDU_LENGTH_ERR_MSB                                             14
#define RX_MPDU_END_MPDU_LENGTH_ERR_MASK                                            0x00004000

#define RX_MPDU_END_TKIP_MIC_ERR_OFFSET                                             0x00000004
#define RX_MPDU_END_TKIP_MIC_ERR_LSB                                                15
#define RX_MPDU_END_TKIP_MIC_ERR_MSB                                                15
#define RX_MPDU_END_TKIP_MIC_ERR_MASK                                               0x00008000

#define RX_MPDU_END_DECRYPT_ERR_OFFSET                                              0x00000004
#define RX_MPDU_END_DECRYPT_ERR_LSB                                                 16
#define RX_MPDU_END_DECRYPT_ERR_MSB                                                 16
#define RX_MPDU_END_DECRYPT_ERR_MASK                                                0x00010000

#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_OFFSET                                    0x00000004
#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_LSB                                       17
#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_MSB                                       17
#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_MASK                                      0x00020000

#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET                             0x00000004
#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_LSB                                18
#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_MSB                                18
#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_MASK                               0x00040000

#define RX_MPDU_END_FCS_ERR_OFFSET                                                  0x00000004
#define RX_MPDU_END_FCS_ERR_LSB                                                     19
#define RX_MPDU_END_FCS_ERR_MSB                                                     19
#define RX_MPDU_END_FCS_ERR_MASK                                                    0x00080000

#define RX_MPDU_END_MSDU_LENGTH_ERR_OFFSET                                          0x00000004
#define RX_MPDU_END_MSDU_LENGTH_ERR_LSB                                             20
#define RX_MPDU_END_MSDU_LENGTH_ERR_MSB                                             20
#define RX_MPDU_END_MSDU_LENGTH_ERR_MASK                                            0x00100000

#define RX_MPDU_END_RXDMA0_DESTINATION_RING_OFFSET                                  0x00000004
#define RX_MPDU_END_RXDMA0_DESTINATION_RING_LSB                                     21
#define RX_MPDU_END_RXDMA0_DESTINATION_RING_MSB                                     23
#define RX_MPDU_END_RXDMA0_DESTINATION_RING_MASK                                    0x00e00000

#define RX_MPDU_END_RXDMA1_DESTINATION_RING_OFFSET                                  0x00000004
#define RX_MPDU_END_RXDMA1_DESTINATION_RING_LSB                                     24
#define RX_MPDU_END_RXDMA1_DESTINATION_RING_MSB                                     26
#define RX_MPDU_END_RXDMA1_DESTINATION_RING_MASK                                    0x07000000

#define RX_MPDU_END_DECRYPT_STATUS_CODE_OFFSET                                      0x00000004
#define RX_MPDU_END_DECRYPT_STATUS_CODE_LSB                                         27
#define RX_MPDU_END_DECRYPT_STATUS_CODE_MSB                                         29
#define RX_MPDU_END_DECRYPT_STATUS_CODE_MASK                                        0x38000000

#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_OFFSET                                    0x00000004
#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_LSB                                       30
#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_MSB                                       30
#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_MASK                                      0x40000000

#define RX_MPDU_END_RESERVED_1B_OFFSET                                              0x00000004
#define RX_MPDU_END_RESERVED_1B_LSB                                                 31
#define RX_MPDU_END_RESERVED_1B_MSB                                                 31
#define RX_MPDU_END_RESERVED_1B_MASK                                                0x80000000

#define RX_MPDU_END_RESERVED_2A_OFFSET                                              0x00000008
#define RX_MPDU_END_RESERVED_2A_LSB                                                 0
#define RX_MPDU_END_RESERVED_2A_MSB                                                 14
#define RX_MPDU_END_RESERVED_2A_MASK                                                0x00007fff

#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_OFFSET                             0x00000008
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_LSB                                15
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_MSB                                15
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_MASK                               0x00008000

#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_OFFSET                                   0x00000008
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_LSB                                      16
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_MSB                                      31
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_MASK                                     0xffff0000

#endif

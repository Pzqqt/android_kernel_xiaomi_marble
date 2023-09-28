
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _TX_MPDU_START_H_
#define _TX_MPDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_MPDU_START 10

#define NUM_OF_QWORDS_TX_MPDU_START 5


struct tx_mpdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mpdu_length                                             : 14,  
                      frame_not_from_tqm                                      :  1,  
                      vht_control_present                                     :  1,  
                      mpdu_header_length                                      :  8,  
                      retry_count                                             :  7,  
                      wds                                                     :  1;  
             uint32_t pn_31_0                                                 : 32;  
             uint32_t pn_47_32                                                : 16,  
                      mpdu_sequence_number                                    : 12,  
                      raw_already_encrypted                                   :  1,  
                      frame_type                                              :  2,  
                      txdma_dropped_mpdu_warning                              :  1;  
             uint32_t iv_byte_0                                               :  8,  
                      iv_byte_1                                               :  8,  
                      iv_byte_2                                               :  8,  
                      iv_byte_3                                               :  8;  
             uint32_t iv_byte_4                                               :  8,  
                      iv_byte_5                                               :  8,  
                      iv_byte_6                                               :  8,  
                      iv_byte_7                                               :  8;  
             uint32_t iv_byte_8                                               :  8,  
                      iv_byte_9                                               :  8,  
                      iv_byte_10                                              :  8,  
                      iv_byte_11                                              :  8;  
             uint32_t iv_byte_12                                              :  8,  
                      iv_byte_13                                              :  8,  
                      iv_byte_14                                              :  8,  
                      iv_byte_15                                              :  8;  
             uint32_t iv_byte_16                                              :  8,  
                      iv_byte_17                                              :  8,  
                      iv_len                                                  :  5,  
                      icv_len                                                 :  5,  
                      vht_control_offset                                      :  6;  
             uint32_t mpdu_type                                               :  1,  
                      transmit_bw_restriction                                 :  1,  
                      allowed_transmit_bw                                     :  4,  
                      tx_notify_frame                                         :  3,  
                      reserved_8a                                             : 23;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t wds                                                     :  1,  
                      retry_count                                             :  7,  
                      mpdu_header_length                                      :  8,  
                      vht_control_present                                     :  1,  
                      frame_not_from_tqm                                      :  1,  
                      mpdu_length                                             : 14;  
             uint32_t pn_31_0                                                 : 32;  
             uint32_t txdma_dropped_mpdu_warning                              :  1,  
                      frame_type                                              :  2,  
                      raw_already_encrypted                                   :  1,  
                      mpdu_sequence_number                                    : 12,  
                      pn_47_32                                                : 16;  
             uint32_t iv_byte_3                                               :  8,  
                      iv_byte_2                                               :  8,  
                      iv_byte_1                                               :  8,  
                      iv_byte_0                                               :  8;  
             uint32_t iv_byte_7                                               :  8,  
                      iv_byte_6                                               :  8,  
                      iv_byte_5                                               :  8,  
                      iv_byte_4                                               :  8;  
             uint32_t iv_byte_11                                              :  8,  
                      iv_byte_10                                              :  8,  
                      iv_byte_9                                               :  8,  
                      iv_byte_8                                               :  8;  
             uint32_t iv_byte_15                                              :  8,  
                      iv_byte_14                                              :  8,  
                      iv_byte_13                                              :  8,  
                      iv_byte_12                                              :  8;  
             uint32_t vht_control_offset                                      :  6,  
                      icv_len                                                 :  5,  
                      iv_len                                                  :  5,  
                      iv_byte_17                                              :  8,  
                      iv_byte_16                                              :  8;  
             uint32_t reserved_8a                                             : 23,  
                      tx_notify_frame                                         :  3,  
                      allowed_transmit_bw                                     :  4,  
                      transmit_bw_restriction                                 :  1,  
                      mpdu_type                                               :  1;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define TX_MPDU_START_MPDU_LENGTH_OFFSET                                            0x0000000000000000
#define TX_MPDU_START_MPDU_LENGTH_LSB                                               0
#define TX_MPDU_START_MPDU_LENGTH_MSB                                               13
#define TX_MPDU_START_MPDU_LENGTH_MASK                                              0x0000000000003fff


 

#define TX_MPDU_START_FRAME_NOT_FROM_TQM_OFFSET                                     0x0000000000000000
#define TX_MPDU_START_FRAME_NOT_FROM_TQM_LSB                                        14
#define TX_MPDU_START_FRAME_NOT_FROM_TQM_MSB                                        14
#define TX_MPDU_START_FRAME_NOT_FROM_TQM_MASK                                       0x0000000000004000


 

#define TX_MPDU_START_VHT_CONTROL_PRESENT_OFFSET                                    0x0000000000000000
#define TX_MPDU_START_VHT_CONTROL_PRESENT_LSB                                       15
#define TX_MPDU_START_VHT_CONTROL_PRESENT_MSB                                       15
#define TX_MPDU_START_VHT_CONTROL_PRESENT_MASK                                      0x0000000000008000


 

#define TX_MPDU_START_MPDU_HEADER_LENGTH_OFFSET                                     0x0000000000000000
#define TX_MPDU_START_MPDU_HEADER_LENGTH_LSB                                        16
#define TX_MPDU_START_MPDU_HEADER_LENGTH_MSB                                        23
#define TX_MPDU_START_MPDU_HEADER_LENGTH_MASK                                       0x0000000000ff0000


 

#define TX_MPDU_START_RETRY_COUNT_OFFSET                                            0x0000000000000000
#define TX_MPDU_START_RETRY_COUNT_LSB                                               24
#define TX_MPDU_START_RETRY_COUNT_MSB                                               30
#define TX_MPDU_START_RETRY_COUNT_MASK                                              0x000000007f000000


 

#define TX_MPDU_START_WDS_OFFSET                                                    0x0000000000000000
#define TX_MPDU_START_WDS_LSB                                                       31
#define TX_MPDU_START_WDS_MSB                                                       31
#define TX_MPDU_START_WDS_MASK                                                      0x0000000080000000


 

#define TX_MPDU_START_PN_31_0_OFFSET                                                0x0000000000000000
#define TX_MPDU_START_PN_31_0_LSB                                                   32
#define TX_MPDU_START_PN_31_0_MSB                                                   63
#define TX_MPDU_START_PN_31_0_MASK                                                  0xffffffff00000000


 

#define TX_MPDU_START_PN_47_32_OFFSET                                               0x0000000000000008
#define TX_MPDU_START_PN_47_32_LSB                                                  0
#define TX_MPDU_START_PN_47_32_MSB                                                  15
#define TX_MPDU_START_PN_47_32_MASK                                                 0x000000000000ffff


 

#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_OFFSET                                   0x0000000000000008
#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_LSB                                      16
#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_MSB                                      27
#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_MASK                                     0x000000000fff0000


 

#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_OFFSET                                  0x0000000000000008
#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_LSB                                     28
#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_MSB                                     28
#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_MASK                                    0x0000000010000000


 

#define TX_MPDU_START_FRAME_TYPE_OFFSET                                             0x0000000000000008
#define TX_MPDU_START_FRAME_TYPE_LSB                                                29
#define TX_MPDU_START_FRAME_TYPE_MSB                                                30
#define TX_MPDU_START_FRAME_TYPE_MASK                                               0x0000000060000000


 

#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_OFFSET                             0x0000000000000008
#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_LSB                                31
#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_MSB                                31
#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_MASK                               0x0000000080000000


 

#define TX_MPDU_START_IV_BYTE_0_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_0_LSB                                                 32
#define TX_MPDU_START_IV_BYTE_0_MSB                                                 39
#define TX_MPDU_START_IV_BYTE_0_MASK                                                0x000000ff00000000


 

#define TX_MPDU_START_IV_BYTE_1_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_1_LSB                                                 40
#define TX_MPDU_START_IV_BYTE_1_MSB                                                 47
#define TX_MPDU_START_IV_BYTE_1_MASK                                                0x0000ff0000000000


 

#define TX_MPDU_START_IV_BYTE_2_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_2_LSB                                                 48
#define TX_MPDU_START_IV_BYTE_2_MSB                                                 55
#define TX_MPDU_START_IV_BYTE_2_MASK                                                0x00ff000000000000


 

#define TX_MPDU_START_IV_BYTE_3_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_3_LSB                                                 56
#define TX_MPDU_START_IV_BYTE_3_MSB                                                 63
#define TX_MPDU_START_IV_BYTE_3_MASK                                                0xff00000000000000


 

#define TX_MPDU_START_IV_BYTE_4_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_4_LSB                                                 0
#define TX_MPDU_START_IV_BYTE_4_MSB                                                 7
#define TX_MPDU_START_IV_BYTE_4_MASK                                                0x00000000000000ff


 

#define TX_MPDU_START_IV_BYTE_5_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_5_LSB                                                 8
#define TX_MPDU_START_IV_BYTE_5_MSB                                                 15
#define TX_MPDU_START_IV_BYTE_5_MASK                                                0x000000000000ff00


 

#define TX_MPDU_START_IV_BYTE_6_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_6_LSB                                                 16
#define TX_MPDU_START_IV_BYTE_6_MSB                                                 23
#define TX_MPDU_START_IV_BYTE_6_MASK                                                0x0000000000ff0000


 

#define TX_MPDU_START_IV_BYTE_7_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_7_LSB                                                 24
#define TX_MPDU_START_IV_BYTE_7_MSB                                                 31
#define TX_MPDU_START_IV_BYTE_7_MASK                                                0x00000000ff000000


 

#define TX_MPDU_START_IV_BYTE_8_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_8_LSB                                                 32
#define TX_MPDU_START_IV_BYTE_8_MSB                                                 39
#define TX_MPDU_START_IV_BYTE_8_MASK                                                0x000000ff00000000


 

#define TX_MPDU_START_IV_BYTE_9_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_9_LSB                                                 40
#define TX_MPDU_START_IV_BYTE_9_MSB                                                 47
#define TX_MPDU_START_IV_BYTE_9_MASK                                                0x0000ff0000000000


 

#define TX_MPDU_START_IV_BYTE_10_OFFSET                                             0x0000000000000010
#define TX_MPDU_START_IV_BYTE_10_LSB                                                48
#define TX_MPDU_START_IV_BYTE_10_MSB                                                55
#define TX_MPDU_START_IV_BYTE_10_MASK                                               0x00ff000000000000


 

#define TX_MPDU_START_IV_BYTE_11_OFFSET                                             0x0000000000000010
#define TX_MPDU_START_IV_BYTE_11_LSB                                                56
#define TX_MPDU_START_IV_BYTE_11_MSB                                                63
#define TX_MPDU_START_IV_BYTE_11_MASK                                               0xff00000000000000


 

#define TX_MPDU_START_IV_BYTE_12_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_12_LSB                                                0
#define TX_MPDU_START_IV_BYTE_12_MSB                                                7
#define TX_MPDU_START_IV_BYTE_12_MASK                                               0x00000000000000ff


 

#define TX_MPDU_START_IV_BYTE_13_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_13_LSB                                                8
#define TX_MPDU_START_IV_BYTE_13_MSB                                                15
#define TX_MPDU_START_IV_BYTE_13_MASK                                               0x000000000000ff00


 

#define TX_MPDU_START_IV_BYTE_14_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_14_LSB                                                16
#define TX_MPDU_START_IV_BYTE_14_MSB                                                23
#define TX_MPDU_START_IV_BYTE_14_MASK                                               0x0000000000ff0000


 

#define TX_MPDU_START_IV_BYTE_15_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_15_LSB                                                24
#define TX_MPDU_START_IV_BYTE_15_MSB                                                31
#define TX_MPDU_START_IV_BYTE_15_MASK                                               0x00000000ff000000


 

#define TX_MPDU_START_IV_BYTE_16_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_16_LSB                                                32
#define TX_MPDU_START_IV_BYTE_16_MSB                                                39
#define TX_MPDU_START_IV_BYTE_16_MASK                                               0x000000ff00000000


 

#define TX_MPDU_START_IV_BYTE_17_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_17_LSB                                                40
#define TX_MPDU_START_IV_BYTE_17_MSB                                                47
#define TX_MPDU_START_IV_BYTE_17_MASK                                               0x0000ff0000000000


 

#define TX_MPDU_START_IV_LEN_OFFSET                                                 0x0000000000000018
#define TX_MPDU_START_IV_LEN_LSB                                                    48
#define TX_MPDU_START_IV_LEN_MSB                                                    52
#define TX_MPDU_START_IV_LEN_MASK                                                   0x001f000000000000


 

#define TX_MPDU_START_ICV_LEN_OFFSET                                                0x0000000000000018
#define TX_MPDU_START_ICV_LEN_LSB                                                   53
#define TX_MPDU_START_ICV_LEN_MSB                                                   57
#define TX_MPDU_START_ICV_LEN_MASK                                                  0x03e0000000000000


 

#define TX_MPDU_START_VHT_CONTROL_OFFSET_OFFSET                                     0x0000000000000018
#define TX_MPDU_START_VHT_CONTROL_OFFSET_LSB                                        58
#define TX_MPDU_START_VHT_CONTROL_OFFSET_MSB                                        63
#define TX_MPDU_START_VHT_CONTROL_OFFSET_MASK                                       0xfc00000000000000


 

#define TX_MPDU_START_MPDU_TYPE_OFFSET                                              0x0000000000000020
#define TX_MPDU_START_MPDU_TYPE_LSB                                                 0
#define TX_MPDU_START_MPDU_TYPE_MSB                                                 0
#define TX_MPDU_START_MPDU_TYPE_MASK                                                0x0000000000000001


 

#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_OFFSET                                0x0000000000000020
#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_LSB                                   1
#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_MSB                                   1
#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_MASK                                  0x0000000000000002


 

#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_OFFSET                                    0x0000000000000020
#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_LSB                                       2
#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_MSB                                       5
#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_MASK                                      0x000000000000003c


 

#define TX_MPDU_START_TX_NOTIFY_FRAME_OFFSET                                        0x0000000000000020
#define TX_MPDU_START_TX_NOTIFY_FRAME_LSB                                           6
#define TX_MPDU_START_TX_NOTIFY_FRAME_MSB                                           8
#define TX_MPDU_START_TX_NOTIFY_FRAME_MASK                                          0x00000000000001c0


 

#define TX_MPDU_START_RESERVED_8A_OFFSET                                            0x0000000000000020
#define TX_MPDU_START_RESERVED_8A_LSB                                               9
#define TX_MPDU_START_RESERVED_8A_MSB                                               31
#define TX_MPDU_START_RESERVED_8A_MASK                                              0x00000000fffffe00


 

#define TX_MPDU_START_TLV64_PADDING_OFFSET                                          0x0000000000000020
#define TX_MPDU_START_TLV64_PADDING_LSB                                             32
#define TX_MPDU_START_TLV64_PADDING_MSB                                             63
#define TX_MPDU_START_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif    

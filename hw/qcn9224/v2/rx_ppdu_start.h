
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

 
 
 
 
 
 
 


#ifndef _RX_PPDU_START_H_
#define _RX_PPDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_PPDU_START 6

#define NUM_OF_QWORDS_RX_PPDU_START 3


struct rx_ppdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_ppdu_id                                             : 16,  
                      preamble_time_to_rxframe                                :  8,  
                      reserved_0a                                             :  8;  
             uint32_t sw_phy_meta_data                                        : 32;  
             uint32_t ppdu_start_timestamp_31_0                               : 32;  
             uint32_t ppdu_start_timestamp_63_32                              : 32;  
             uint32_t rxframe_assert_timestamp                                : 32;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t reserved_0a                                             :  8,  
                      preamble_time_to_rxframe                                :  8,  
                      phy_ppdu_id                                             : 16;  
             uint32_t sw_phy_meta_data                                        : 32;  
             uint32_t ppdu_start_timestamp_31_0                               : 32;  
             uint32_t ppdu_start_timestamp_63_32                              : 32;  
             uint32_t rxframe_assert_timestamp                                : 32;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define RX_PPDU_START_PHY_PPDU_ID_OFFSET                                            0x0000000000000000
#define RX_PPDU_START_PHY_PPDU_ID_LSB                                               0
#define RX_PPDU_START_PHY_PPDU_ID_MSB                                               15
#define RX_PPDU_START_PHY_PPDU_ID_MASK                                              0x000000000000ffff


 

#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_OFFSET                               0x0000000000000000
#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_LSB                                  16
#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_MSB                                  23
#define RX_PPDU_START_PREAMBLE_TIME_TO_RXFRAME_MASK                                 0x0000000000ff0000


 

#define RX_PPDU_START_RESERVED_0A_OFFSET                                            0x0000000000000000
#define RX_PPDU_START_RESERVED_0A_LSB                                               24
#define RX_PPDU_START_RESERVED_0A_MSB                                               31
#define RX_PPDU_START_RESERVED_0A_MASK                                              0x00000000ff000000


 

#define RX_PPDU_START_SW_PHY_META_DATA_OFFSET                                       0x0000000000000000
#define RX_PPDU_START_SW_PHY_META_DATA_LSB                                          32
#define RX_PPDU_START_SW_PHY_META_DATA_MSB                                          63
#define RX_PPDU_START_SW_PHY_META_DATA_MASK                                         0xffffffff00000000


 

#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_OFFSET                              0x0000000000000008
#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_LSB                                 0
#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_MSB                                 31
#define RX_PPDU_START_PPDU_START_TIMESTAMP_31_0_MASK                                0x00000000ffffffff


 

#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_OFFSET                             0x0000000000000008
#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_LSB                                32
#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_MSB                                63
#define RX_PPDU_START_PPDU_START_TIMESTAMP_63_32_MASK                               0xffffffff00000000


 

#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_OFFSET                               0x0000000000000010
#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_LSB                                  0
#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_MSB                                  31
#define RX_PPDU_START_RXFRAME_ASSERT_TIMESTAMP_MASK                                 0x00000000ffffffff


 

#define RX_PPDU_START_TLV64_PADDING_OFFSET                                          0x0000000000000010
#define RX_PPDU_START_TLV64_PADDING_LSB                                             32
#define RX_PPDU_START_TLV64_PADDING_MSB                                             63
#define RX_PPDU_START_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif    

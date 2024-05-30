
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

 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_ACK_OR_BA_H_
#define _TX_FES_STATUS_ACK_OR_BA_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_ACK_OR_BA 10

#define NUM_OF_QWORDS_TX_FES_STATUS_ACK_OR_BA 5


struct tx_fes_status_ack_or_ba {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ack_ba_status_type                                      :  1,  
                      ba_type                                                 :  1,  
                      ba_tid                                                  :  4,  
                      unexpected_ack_or_ba                                    :  1,  
                      response_timeout                                        :  1,  
                      ack_frame_rssi                                          :  8,  
                      ssn                                                     : 12,  
                      reserved_0b                                             :  4;  
             uint32_t sw_peer_id                                              : 16,  
                      reserved_1a                                             : 16;  
             uint32_t ba_bitmap_31_0                                          : 32;  
             uint32_t ba_bitmap_63_32                                         : 32;  
             uint32_t ba_bitmap_95_64                                         : 32;  
             uint32_t ba_bitmap_127_96                                        : 32;  
             uint32_t ba_bitmap_159_128                                       : 32;  
             uint32_t ba_bitmap_191_160                                       : 32;  
             uint32_t ba_bitmap_223_192                                       : 32;  
             uint32_t ba_bitmap_255_224                                       : 32;  
#else
             uint32_t reserved_0b                                             :  4,  
                      ssn                                                     : 12,  
                      ack_frame_rssi                                          :  8,  
                      response_timeout                                        :  1,  
                      unexpected_ack_or_ba                                    :  1,  
                      ba_tid                                                  :  4,  
                      ba_type                                                 :  1,  
                      ack_ba_status_type                                      :  1;  
             uint32_t reserved_1a                                             : 16,  
                      sw_peer_id                                              : 16;  
             uint32_t ba_bitmap_31_0                                          : 32;  
             uint32_t ba_bitmap_63_32                                         : 32;  
             uint32_t ba_bitmap_95_64                                         : 32;  
             uint32_t ba_bitmap_127_96                                        : 32;  
             uint32_t ba_bitmap_159_128                                       : 32;  
             uint32_t ba_bitmap_191_160                                       : 32;  
             uint32_t ba_bitmap_223_192                                       : 32;  
             uint32_t ba_bitmap_255_224                                       : 32;  
#endif
};


 

#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_LSB                              0
#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_MSB                              0
#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_MASK                             0x0000000000000001


 

#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_LSB                                         1
#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_MSB                                         1
#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_MASK                                        0x0000000000000002


 

#define TX_FES_STATUS_ACK_OR_BA_BA_TID_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_BA_TID_LSB                                          2
#define TX_FES_STATUS_ACK_OR_BA_BA_TID_MSB                                          5
#define TX_FES_STATUS_ACK_OR_BA_BA_TID_MASK                                         0x000000000000003c


 

#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_LSB                            6
#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_MSB                            6
#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_MASK                           0x0000000000000040


 

#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_OFFSET                             0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_LSB                                7
#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_MSB                                7
#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_MASK                               0x0000000000000080


 

#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_LSB                                  8
#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_MSB                                  15
#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_MASK                                 0x000000000000ff00


 

#define TX_FES_STATUS_ACK_OR_BA_SSN_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_SSN_LSB                                             16
#define TX_FES_STATUS_ACK_OR_BA_SSN_MSB                                             27
#define TX_FES_STATUS_ACK_OR_BA_SSN_MASK                                            0x000000000fff0000


 

#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_OFFSET                                  0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_LSB                                     28
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_MSB                                     31
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_MASK                                    0x00000000f0000000


 

#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_OFFSET                                   0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_LSB                                      32
#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_MSB                                      47
#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_MASK                                     0x0000ffff00000000


 

#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_OFFSET                                  0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_LSB                                     48
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_MSB                                     63
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_MASK                                    0xffff000000000000


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_OFFSET                               0x0000000000000008
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_LSB                                  0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_MSB                                  31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_MASK                                 0x00000000ffffffff


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_OFFSET                              0x0000000000000008
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_LSB                                 32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_MSB                                 63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_MASK                                0xffffffff00000000


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_OFFSET                              0x0000000000000010
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_LSB                                 0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_MSB                                 31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_MASK                                0x00000000ffffffff


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_OFFSET                             0x0000000000000010
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_LSB                                32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_MSB                                63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_MASK                               0xffffffff00000000


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_LSB                               0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_MSB                               31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_MASK                              0x00000000ffffffff


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_LSB                               32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_MSB                               63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_MASK                              0xffffffff00000000


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_LSB                               0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_MSB                               31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_MASK                              0x00000000ffffffff


 

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_LSB                               32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_MSB                               63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_MASK                              0xffffffff00000000



#endif    

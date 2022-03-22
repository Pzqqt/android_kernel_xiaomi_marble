
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

 
 
 
 
 
 
 


#ifndef _RX_FRAME_BITMAP_REQ_H_
#define _RX_FRAME_BITMAP_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_FRAME_BITMAP_REQ 2

#define NUM_OF_QWORDS_RX_FRAME_BITMAP_REQ 1


struct rx_frame_bitmap_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t explicit_user_request                                   :  1,  
                      user_request_type                                       :  1,  
                      user_number                                             :  6,  
                      sw_peer_id                                              : 16,  
                      tid_specific_request                                    :  1,  
                      requested_tid                                           :  4,  
                      reserved_0                                              :  3;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t reserved_0                                              :  3,  
                      requested_tid                                           :  4,  
                      tid_specific_request                                    :  1,  
                      sw_peer_id                                              : 16,  
                      user_number                                             :  6,  
                      user_request_type                                       :  1,  
                      explicit_user_request                                   :  1;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_OFFSET                            0x0000000000000000
#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_LSB                               0
#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_MSB                               0
#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_MASK                              0x0000000000000001


 

#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_OFFSET                                0x0000000000000000
#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_LSB                                   1
#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_MSB                                   1
#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_MASK                                  0x0000000000000002


 

#define RX_FRAME_BITMAP_REQ_USER_NUMBER_OFFSET                                      0x0000000000000000
#define RX_FRAME_BITMAP_REQ_USER_NUMBER_LSB                                         2
#define RX_FRAME_BITMAP_REQ_USER_NUMBER_MSB                                         7
#define RX_FRAME_BITMAP_REQ_USER_NUMBER_MASK                                        0x00000000000000fc


 

#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_OFFSET                                       0x0000000000000000
#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_LSB                                          8
#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_MSB                                          23
#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_MASK                                         0x0000000000ffff00


 

#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_OFFSET                             0x0000000000000000
#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_LSB                                24
#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_MSB                                24
#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_MASK                               0x0000000001000000


 

#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_OFFSET                                    0x0000000000000000
#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_LSB                                       25
#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_MSB                                       28
#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_MASK                                      0x000000001e000000


 

#define RX_FRAME_BITMAP_REQ_RESERVED_0_OFFSET                                       0x0000000000000000
#define RX_FRAME_BITMAP_REQ_RESERVED_0_LSB                                          29
#define RX_FRAME_BITMAP_REQ_RESERVED_0_MSB                                          31
#define RX_FRAME_BITMAP_REQ_RESERVED_0_MASK                                         0x00000000e0000000


 

#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_OFFSET                                    0x0000000000000000
#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_LSB                                       32
#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_MSB                                       63
#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_MASK                                      0xffffffff00000000



#endif    

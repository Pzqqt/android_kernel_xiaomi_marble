
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

 
 
 
 
 
 
 


#ifndef _PHYRX_ABORT_REQUEST_INFO_H_
#define _PHYRX_ABORT_REQUEST_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_ABORT_REQUEST_INFO 1


struct phyrx_abort_request_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phyrx_abort_reason                                      :  8,  
                      phy_enters_nap_state                                    :  1,  
                      phy_enters_defer_state                                  :  1,  
                      reserved_0                                              :  6,  
                      receive_duration                                        : 16;  
#else
             uint32_t receive_duration                                        : 16,  
                      reserved_0                                              :  6,  
                      phy_enters_defer_state                                  :  1,  
                      phy_enters_nap_state                                    :  1,  
                      phyrx_abort_reason                                      :  8;  
#endif
};


 

#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_OFFSET                          0x00000000
#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_LSB                             0
#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_MSB                             7
#define PHYRX_ABORT_REQUEST_INFO_PHYRX_ABORT_REASON_MASK                            0x000000ff


 

#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_OFFSET                        0x00000000
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_LSB                           8
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_MSB                           8
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_NAP_STATE_MASK                          0x00000100


 

#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_OFFSET                      0x00000000
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_LSB                         9
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_MSB                         9
#define PHYRX_ABORT_REQUEST_INFO_PHY_ENTERS_DEFER_STATE_MASK                        0x00000200


 

#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_OFFSET                                  0x00000000
#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_LSB                                     10
#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_MSB                                     15
#define PHYRX_ABORT_REQUEST_INFO_RESERVED_0_MASK                                    0x0000fc00


 

#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_OFFSET                            0x00000000
#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_LSB                               16
#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_MSB                               31
#define PHYRX_ABORT_REQUEST_INFO_RECEIVE_DURATION_MASK                              0xffff0000



#endif    

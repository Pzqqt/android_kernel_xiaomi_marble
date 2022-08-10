
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

 
 
 
 
 
 
 


#ifndef _RX_RXPCU_CLASSIFICATION_OVERVIEW_H_
#define _RX_RXPCU_CLASSIFICATION_OVERVIEW_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_RXPCU_CLASSIFICATION_OVERVIEW 1


struct rx_rxpcu_classification_overview {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t filter_pass_mpdus                                       :  1,  
                      filter_pass_mpdus_fcs_ok                                :  1,  
                      monitor_direct_mpdus                                    :  1,  
                      monitor_direct_mpdus_fcs_ok                             :  1,  
                      monitor_other_mpdus                                     :  1,  
                      monitor_other_mpdus_fcs_ok                              :  1,  
                      phyrx_abort_received                                    :  1,  
                      filter_pass_monitor_ovrd_mpdus                          :  1,  
                      filter_pass_monitor_ovrd_mpdus_fcs_ok                   :  1,  
                      reserved_0                                              :  7,  
                      phy_ppdu_id                                             : 16;  
#else
             uint32_t phy_ppdu_id                                             : 16,  
                      reserved_0                                              :  7,  
                      filter_pass_monitor_ovrd_mpdus_fcs_ok                   :  1,  
                      filter_pass_monitor_ovrd_mpdus                          :  1,  
                      phyrx_abort_received                                    :  1,  
                      monitor_other_mpdus_fcs_ok                              :  1,  
                      monitor_other_mpdus                                     :  1,  
                      monitor_direct_mpdus_fcs_ok                             :  1,  
                      monitor_direct_mpdus                                    :  1,  
                      filter_pass_mpdus_fcs_ok                                :  1,  
                      filter_pass_mpdus                                       :  1;  
#endif
};


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_OFFSET                   0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_LSB                      0
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_MSB                      0
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_MASK                     0x00000001


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_FCS_OK_OFFSET            0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_FCS_OK_LSB               1
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_FCS_OK_MSB               1
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MPDUS_FCS_OK_MASK              0x00000002


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_OFFSET                0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_LSB                   2
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_MSB                   2
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_MASK                  0x00000004


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_FCS_OK_OFFSET         0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_FCS_OK_LSB            3
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_FCS_OK_MSB            3
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_DIRECT_MPDUS_FCS_OK_MASK           0x00000008


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_OFFSET                 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_LSB                    4
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_MSB                    4
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_MASK                   0x00000010


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_FCS_OK_OFFSET          0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_FCS_OK_LSB             5
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_FCS_OK_MSB             5
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_MONITOR_OTHER_MPDUS_FCS_OK_MASK            0x00000020


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHYRX_ABORT_RECEIVED_OFFSET                0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHYRX_ABORT_RECEIVED_LSB                   6
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHYRX_ABORT_RECEIVED_MSB                   6
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHYRX_ABORT_RECEIVED_MASK                  0x00000040


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_OFFSET      0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_LSB         7
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_MSB         7
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_MASK        0x00000080


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_LSB  8
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MSB  8
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_FILTER_PASS_MONITOR_OVRD_MPDUS_FCS_OK_MASK 0x00000100


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_RESERVED_0_OFFSET                          0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_RESERVED_0_LSB                             9
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_RESERVED_0_MSB                             15
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_RESERVED_0_MASK                            0x0000fe00


 

#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHY_PPDU_ID_OFFSET                         0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHY_PPDU_ID_LSB                            16
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHY_PPDU_ID_MSB                            31
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_PHY_PPDU_ID_MASK                           0xffff0000



#endif    

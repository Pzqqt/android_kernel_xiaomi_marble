
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RESPONSE_START_STATUS_H_
#define _RESPONSE_START_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RESPONSE_START_STATUS 2

#define NUM_OF_QWORDS_RESPONSE_START_STATUS 1


struct response_start_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t generated_response                                      :  3,  
                      ftm_tm                                                  :  2,  
                      trig_response_related                                   :  1,  
                      response_sta_count                                      :  7,  
                      reserved                                                : 19;  
             uint32_t phy_ppdu_id                                             : 16,  
                      sw_peer_id                                              : 16;  
#else
             uint32_t reserved                                                : 19,  
                      response_sta_count                                      :  7,  
                      trig_response_related                                   :  1,  
                      ftm_tm                                                  :  2,  
                      generated_response                                      :  3;  
             uint32_t sw_peer_id                                              : 16,  
                      phy_ppdu_id                                             : 16;  
#endif
};


 

#define RESPONSE_START_STATUS_GENERATED_RESPONSE_OFFSET                             0x0000000000000000
#define RESPONSE_START_STATUS_GENERATED_RESPONSE_LSB                                0
#define RESPONSE_START_STATUS_GENERATED_RESPONSE_MSB                                2
#define RESPONSE_START_STATUS_GENERATED_RESPONSE_MASK                               0x0000000000000007


 

#define RESPONSE_START_STATUS_FTM_TM_OFFSET                                         0x0000000000000000
#define RESPONSE_START_STATUS_FTM_TM_LSB                                            3
#define RESPONSE_START_STATUS_FTM_TM_MSB                                            4
#define RESPONSE_START_STATUS_FTM_TM_MASK                                           0x0000000000000018


 

#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_OFFSET                          0x0000000000000000
#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_LSB                             5
#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_MSB                             5
#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_MASK                            0x0000000000000020


 

#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_OFFSET                             0x0000000000000000
#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_LSB                                6
#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_MSB                                12
#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_MASK                               0x0000000000001fc0


 

#define RESPONSE_START_STATUS_RESERVED_OFFSET                                       0x0000000000000000
#define RESPONSE_START_STATUS_RESERVED_LSB                                          13
#define RESPONSE_START_STATUS_RESERVED_MSB                                          31
#define RESPONSE_START_STATUS_RESERVED_MASK                                         0x00000000ffffe000


 

#define RESPONSE_START_STATUS_PHY_PPDU_ID_OFFSET                                    0x0000000000000000
#define RESPONSE_START_STATUS_PHY_PPDU_ID_LSB                                       32
#define RESPONSE_START_STATUS_PHY_PPDU_ID_MSB                                       47
#define RESPONSE_START_STATUS_PHY_PPDU_ID_MASK                                      0x0000ffff00000000


 

#define RESPONSE_START_STATUS_SW_PEER_ID_OFFSET                                     0x0000000000000000
#define RESPONSE_START_STATUS_SW_PEER_ID_LSB                                        48
#define RESPONSE_START_STATUS_SW_PEER_ID_MSB                                        63
#define RESPONSE_START_STATUS_SW_PEER_ID_MASK                                       0xffff000000000000



#endif    

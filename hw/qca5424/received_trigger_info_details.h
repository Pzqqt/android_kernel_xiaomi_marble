
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RECEIVED_TRIGGER_INFO_DETAILS_H_
#define _RECEIVED_TRIGGER_INFO_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RECEIVED_TRIGGER_INFO_DETAILS 5


struct received_trigger_info_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t trigger_type                                            :  4,  
                      ax_trigger_source                                       :  1,  
                      ax_trigger_type                                         :  4,  
                      trigger_source_sta_full_aid                             : 13,  
                      frame_control_valid                                     :  1,  
                      qos_control_valid                                       :  1,  
                      he_control_info_valid                                   :  1,  
                      ranging_trigger_subtype                                 :  4,  
                      reserved_0b                                             :  3;  
             uint32_t phy_ppdu_id                                             : 16,  
                      lsig_response_length                                    : 12,  
                      reserved_1a                                             :  4;  
             uint32_t frame_control                                           : 16,  
                      qos_control                                             : 16;  
             uint32_t sw_peer_id                                              : 16,  
                      reserved_3a                                             : 16;  
             uint32_t he_control                                              : 32;  
#else
             uint32_t reserved_0b                                             :  3,  
                      ranging_trigger_subtype                                 :  4,  
                      he_control_info_valid                                   :  1,  
                      qos_control_valid                                       :  1,  
                      frame_control_valid                                     :  1,  
                      trigger_source_sta_full_aid                             : 13,  
                      ax_trigger_type                                         :  4,  
                      ax_trigger_source                                       :  1,  
                      trigger_type                                            :  4;  
             uint32_t reserved_1a                                             :  4,  
                      lsig_response_length                                    : 12,  
                      phy_ppdu_id                                             : 16;  
             uint32_t qos_control                                             : 16,  
                      frame_control                                           : 16;  
             uint32_t reserved_3a                                             : 16,  
                      sw_peer_id                                              : 16;  
             uint32_t he_control                                              : 32;  
#endif
};


 

#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_TYPE_OFFSET                           0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_TYPE_LSB                              0
#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_TYPE_MSB                              3
#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_TYPE_MASK                             0x0000000f


 

#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_SOURCE_OFFSET                      0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_SOURCE_LSB                         4
#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_SOURCE_MSB                         4
#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_SOURCE_MASK                        0x00000010


 

#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_TYPE_OFFSET                        0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_TYPE_LSB                           5
#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_TYPE_MSB                           8
#define RECEIVED_TRIGGER_INFO_DETAILS_AX_TRIGGER_TYPE_MASK                          0x000001e0


 

#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_OFFSET            0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_LSB               9
#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_MSB               21
#define RECEIVED_TRIGGER_INFO_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_MASK              0x003ffe00


 

#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_VALID_OFFSET                    0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_VALID_LSB                       22
#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_VALID_MSB                       22
#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_VALID_MASK                      0x00400000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_VALID_OFFSET                      0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_VALID_LSB                         23
#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_VALID_MSB                         23
#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_VALID_MASK                        0x00800000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_INFO_VALID_OFFSET                  0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_INFO_VALID_LSB                     24
#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_INFO_VALID_MSB                     24
#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_INFO_VALID_MASK                    0x01000000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_RANGING_TRIGGER_SUBTYPE_OFFSET                0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_RANGING_TRIGGER_SUBTYPE_LSB                   25
#define RECEIVED_TRIGGER_INFO_DETAILS_RANGING_TRIGGER_SUBTYPE_MSB                   28
#define RECEIVED_TRIGGER_INFO_DETAILS_RANGING_TRIGGER_SUBTYPE_MASK                  0x1e000000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_0B_OFFSET                            0x00000000
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_0B_LSB                               29
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_0B_MSB                               31
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_0B_MASK                              0xe0000000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_PHY_PPDU_ID_OFFSET                            0x00000004
#define RECEIVED_TRIGGER_INFO_DETAILS_PHY_PPDU_ID_LSB                               0
#define RECEIVED_TRIGGER_INFO_DETAILS_PHY_PPDU_ID_MSB                               15
#define RECEIVED_TRIGGER_INFO_DETAILS_PHY_PPDU_ID_MASK                              0x0000ffff


 

#define RECEIVED_TRIGGER_INFO_DETAILS_LSIG_RESPONSE_LENGTH_OFFSET                   0x00000004
#define RECEIVED_TRIGGER_INFO_DETAILS_LSIG_RESPONSE_LENGTH_LSB                      16
#define RECEIVED_TRIGGER_INFO_DETAILS_LSIG_RESPONSE_LENGTH_MSB                      27
#define RECEIVED_TRIGGER_INFO_DETAILS_LSIG_RESPONSE_LENGTH_MASK                     0x0fff0000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_1A_OFFSET                            0x00000004
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_1A_LSB                               28
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_1A_MSB                               31
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_1A_MASK                              0xf0000000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_OFFSET                          0x00000008
#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_LSB                             0
#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_MSB                             15
#define RECEIVED_TRIGGER_INFO_DETAILS_FRAME_CONTROL_MASK                            0x0000ffff


 

#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_OFFSET                            0x00000008
#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_LSB                               16
#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_MSB                               31
#define RECEIVED_TRIGGER_INFO_DETAILS_QOS_CONTROL_MASK                              0xffff0000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_SW_PEER_ID_OFFSET                             0x0000000c
#define RECEIVED_TRIGGER_INFO_DETAILS_SW_PEER_ID_LSB                                0
#define RECEIVED_TRIGGER_INFO_DETAILS_SW_PEER_ID_MSB                                15
#define RECEIVED_TRIGGER_INFO_DETAILS_SW_PEER_ID_MASK                               0x0000ffff


 

#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_3A_OFFSET                            0x0000000c
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_3A_LSB                               16
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_3A_MSB                               31
#define RECEIVED_TRIGGER_INFO_DETAILS_RESERVED_3A_MASK                              0xffff0000


 

#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_OFFSET                             0x00000010
#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_LSB                                0
#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_MSB                                31
#define RECEIVED_TRIGGER_INFO_DETAILS_HE_CONTROL_MASK                               0xffffffff



#endif    

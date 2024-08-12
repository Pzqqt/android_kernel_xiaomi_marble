
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _MON_DESTINATION_RING_WITH_DROP_H_
#define _MON_DESTINATION_RING_WITH_DROP_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MON_DESTINATION_RING_WITH_DROP 4


struct mon_destination_ring_with_drop {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ppdu_drop_cnt                                           : 10,  
                      mpdu_drop_cnt                                           : 10,  
                      tlv_drop_cnt                                            : 10,  
                      end_of_ppdu_seen                                        :  1,  
                      reserved_0a                                             :  1;  
             uint32_t reserved_1a                                             : 32;  
             uint32_t ppdu_id                                                 : 32;  
             uint32_t reserved_3a                                             : 18,  
                      initiator                                               :  1,  
                      empty_descriptor                                        :  1,  
                      ring_id                                                 :  8,  
                      looping_count                                           :  4;  
#else
             uint32_t reserved_0a                                             :  1,  
                      end_of_ppdu_seen                                        :  1,  
                      tlv_drop_cnt                                            : 10,  
                      mpdu_drop_cnt                                           : 10,  
                      ppdu_drop_cnt                                           : 10;  
             uint32_t reserved_1a                                             : 32;  
             uint32_t ppdu_id                                                 : 32;  
             uint32_t looping_count                                           :  4,  
                      ring_id                                                 :  8,  
                      empty_descriptor                                        :  1,  
                      initiator                                               :  1,  
                      reserved_3a                                             : 18;  
#endif
};


 

#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_OFFSET                         0x00000000
#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_LSB                            0
#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_MSB                            9
#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_MASK                           0x000003ff


 

#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_OFFSET                         0x00000000
#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_LSB                            10
#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_MSB                            19
#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_MASK                           0x000ffc00


 

#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_OFFSET                          0x00000000
#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_LSB                             20
#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_MSB                             29
#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_MASK                            0x3ff00000


 

#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_OFFSET                      0x00000000
#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_LSB                         30
#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_MSB                         30
#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_MASK                        0x40000000


 

#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_OFFSET                           0x00000000
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_LSB                              31
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_MSB                              31
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_MASK                             0x80000000


 

#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_OFFSET                           0x00000004
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_LSB                              0
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_MSB                              31
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_MASK                             0xffffffff


 

#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_OFFSET                               0x00000008
#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_LSB                                  0
#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_MSB                                  31
#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_MASK                                 0xffffffff


 

#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_OFFSET                           0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_LSB                              0
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_MSB                              17
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_MASK                             0x0003ffff


 

#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_OFFSET                             0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_LSB                                18
#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_MSB                                18
#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_MASK                               0x00040000


 

#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_OFFSET                      0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_LSB                         19
#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_MSB                         19
#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_MASK                        0x00080000


 

#define MON_DESTINATION_RING_WITH_DROP_RING_ID_OFFSET                               0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_RING_ID_LSB                                  20
#define MON_DESTINATION_RING_WITH_DROP_RING_ID_MSB                                  27
#define MON_DESTINATION_RING_WITH_DROP_RING_ID_MASK                                 0x0ff00000


 

#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_OFFSET                         0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_LSB                            28
#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_MSB                            31
#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_MASK                           0xf0000000



#endif    

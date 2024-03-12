
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_TRIG_INFO_H_
#define _RX_TRIG_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_TRIG_INFO 2

#define NUM_OF_QWORDS_RX_TRIG_INFO 1


struct rx_trig_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_trigger_frame_type                                   :  2,  
                      trigger_resp_type                                       :  3,  
                      reserved_0                                              : 27;  
             uint32_t ppdu_duration                                           : 16,  
                      unique_destination_id                                   : 16;  
#else
             uint32_t reserved_0                                              : 27,  
                      trigger_resp_type                                       :  3,  
                      rx_trigger_frame_type                                   :  2;  
             uint32_t unique_destination_id                                   : 16,  
                      ppdu_duration                                           : 16;  
#endif
};


 

#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_OFFSET                                   0x0000000000000000
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_LSB                                      0
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_MSB                                      1
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_MASK                                     0x0000000000000003


 

#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_OFFSET                                       0x0000000000000000
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_LSB                                          2
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_MSB                                          4
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_MASK                                         0x000000000000001c


 

#define RX_TRIG_INFO_RESERVED_0_OFFSET                                              0x0000000000000000
#define RX_TRIG_INFO_RESERVED_0_LSB                                                 5
#define RX_TRIG_INFO_RESERVED_0_MSB                                                 31
#define RX_TRIG_INFO_RESERVED_0_MASK                                                0x00000000ffffffe0


 

#define RX_TRIG_INFO_PPDU_DURATION_OFFSET                                           0x0000000000000000
#define RX_TRIG_INFO_PPDU_DURATION_LSB                                              32
#define RX_TRIG_INFO_PPDU_DURATION_MSB                                              47
#define RX_TRIG_INFO_PPDU_DURATION_MASK                                             0x0000ffff00000000


 

#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_OFFSET                                   0x0000000000000000
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_LSB                                      48
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_MSB                                      63
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_MASK                                     0xffff000000000000



#endif    

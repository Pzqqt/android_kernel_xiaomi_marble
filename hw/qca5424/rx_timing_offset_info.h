
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_TIMING_OFFSET_INFO_H_
#define _RX_TIMING_OFFSET_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_TIMING_OFFSET_INFO 1


struct rx_timing_offset_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t residual_phase_offset                                   : 12,  
                      reserved                                                : 20;  
#else
             uint32_t reserved                                                : 20,  
                      residual_phase_offset                                   : 12;  
#endif
};


 

#define RX_TIMING_OFFSET_INFO_RESIDUAL_PHASE_OFFSET_OFFSET                          0x00000000
#define RX_TIMING_OFFSET_INFO_RESIDUAL_PHASE_OFFSET_LSB                             0
#define RX_TIMING_OFFSET_INFO_RESIDUAL_PHASE_OFFSET_MSB                             11
#define RX_TIMING_OFFSET_INFO_RESIDUAL_PHASE_OFFSET_MASK                            0x00000fff


 

#define RX_TIMING_OFFSET_INFO_RESERVED_OFFSET                                       0x00000000
#define RX_TIMING_OFFSET_INFO_RESERVED_LSB                                          12
#define RX_TIMING_OFFSET_INFO_RESERVED_MSB                                          31
#define RX_TIMING_OFFSET_INFO_RESERVED_MASK                                         0xfffff000



#endif    

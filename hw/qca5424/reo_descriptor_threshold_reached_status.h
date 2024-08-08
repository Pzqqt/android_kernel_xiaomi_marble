
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_H_
#define _REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"
#define NUM_OF_DWORDS_REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS 26

#define NUM_OF_QWORDS_REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS 13


struct reo_descriptor_threshold_reached_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_status_header                                 status_header;
             uint32_t threshold_index                                         :  2,  
                      reserved_2                                              : 30;  
             uint32_t link_descriptor_counter0                                : 24,  
                      reserved_3                                              :  8;  
             uint32_t link_descriptor_counter1                                : 24,  
                      reserved_4                                              :  8;  
             uint32_t link_descriptor_counter2                                : 24,  
                      reserved_5                                              :  8;  
             uint32_t link_descriptor_counter_sum                             : 26,  
                      reserved_6                                              :  6;  
             uint32_t reserved_7                                              : 32;  
             uint32_t reserved_8                                              : 32;  
             uint32_t reserved_9a                                             : 32;  
             uint32_t reserved_10a                                            : 32;  
             uint32_t reserved_11a                                            : 32;  
             uint32_t reserved_12a                                            : 32;  
             uint32_t reserved_13a                                            : 32;  
             uint32_t reserved_14a                                            : 32;  
             uint32_t reserved_15a                                            : 32;  
             uint32_t reserved_16a                                            : 32;  
             uint32_t reserved_17a                                            : 32;  
             uint32_t reserved_18a                                            : 32;  
             uint32_t reserved_19a                                            : 32;  
             uint32_t reserved_20a                                            : 32;  
             uint32_t reserved_21a                                            : 32;  
             uint32_t reserved_22a                                            : 32;  
             uint32_t reserved_23a                                            : 32;  
             uint32_t reserved_24a                                            : 32;  
             uint32_t reserved_25a                                            : 28,  
                      looping_count                                           :  4;  
#else
             struct   uniform_reo_status_header                                 status_header;
             uint32_t reserved_2                                              : 30,  
                      threshold_index                                         :  2;  
             uint32_t reserved_3                                              :  8,  
                      link_descriptor_counter0                                : 24;  
             uint32_t reserved_4                                              :  8,  
                      link_descriptor_counter1                                : 24;  
             uint32_t reserved_5                                              :  8,  
                      link_descriptor_counter2                                : 24;  
             uint32_t reserved_6                                              :  6,  
                      link_descriptor_counter_sum                             : 26;  
             uint32_t reserved_7                                              : 32;  
             uint32_t reserved_8                                              : 32;  
             uint32_t reserved_9a                                             : 32;  
             uint32_t reserved_10a                                            : 32;  
             uint32_t reserved_11a                                            : 32;  
             uint32_t reserved_12a                                            : 32;  
             uint32_t reserved_13a                                            : 32;  
             uint32_t reserved_14a                                            : 32;  
             uint32_t reserved_15a                                            : 32;  
             uint32_t reserved_16a                                            : 32;  
             uint32_t reserved_17a                                            : 32;  
             uint32_t reserved_18a                                            : 32;  
             uint32_t reserved_19a                                            : 32;  
             uint32_t reserved_20a                                            : 32;  
             uint32_t reserved_21a                                            : 32;  
             uint32_t reserved_22a                                            : 32;  
             uint32_t reserved_23a                                            : 32;  
             uint32_t reserved_24a                                            : 32;  
             uint32_t looping_count                                           :  4,  
                      reserved_25a                                            : 28;  
#endif
};


 


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET 0x0000000000000000
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_LSB 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MSB 15
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MASK 0x000000000000ffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET 0x0000000000000000
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_LSB 16
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MSB 25
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MASK 0x0000000003ff0000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET 0x0000000000000000
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB 26
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MSB 27
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK 0x000000000c000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_RESERVED_0A_OFFSET    0x0000000000000000
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_RESERVED_0A_LSB       28
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_RESERVED_0A_MSB       31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_RESERVED_0A_MASK      0x00000000f0000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_TIMESTAMP_OFFSET      0x0000000000000000
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_TIMESTAMP_LSB         32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_TIMESTAMP_MSB         63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_STATUS_HEADER_TIMESTAMP_MASK        0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_THRESHOLD_INDEX_OFFSET              0x0000000000000008
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_THRESHOLD_INDEX_LSB                 0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_THRESHOLD_INDEX_MSB                 1
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_THRESHOLD_INDEX_MASK                0x0000000000000003


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_2_OFFSET                   0x0000000000000008
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_2_LSB                      2
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_2_MSB                      31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_2_MASK                     0x00000000fffffffc


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER0_OFFSET     0x0000000000000008
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER0_LSB        32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER0_MSB        55
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER0_MASK       0x00ffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_3_OFFSET                   0x0000000000000008
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_3_LSB                      56
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_3_MSB                      63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_3_MASK                     0xff00000000000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER1_OFFSET     0x0000000000000010
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER1_LSB        0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER1_MSB        23
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER1_MASK       0x0000000000ffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_4_OFFSET                   0x0000000000000010
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_4_LSB                      24
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_4_MSB                      31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_4_MASK                     0x00000000ff000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER2_OFFSET     0x0000000000000010
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER2_LSB        32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER2_MSB        55
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER2_MASK       0x00ffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_5_OFFSET                   0x0000000000000010
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_5_LSB                      56
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_5_MSB                      63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_5_MASK                     0xff00000000000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER_SUM_OFFSET  0x0000000000000018
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER_SUM_LSB     0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER_SUM_MSB     25
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LINK_DESCRIPTOR_COUNTER_SUM_MASK    0x0000000003ffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_6_OFFSET                   0x0000000000000018
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_6_LSB                      26
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_6_MSB                      31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_6_MASK                     0x00000000fc000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_7_OFFSET                   0x0000000000000018
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_7_LSB                      32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_7_MSB                      63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_7_MASK                     0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_8_OFFSET                   0x0000000000000020
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_8_LSB                      0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_8_MSB                      31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_8_MASK                     0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_9A_OFFSET                  0x0000000000000020
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_9A_LSB                     32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_9A_MSB                     63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_9A_MASK                    0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_10A_OFFSET                 0x0000000000000028
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_10A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_10A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_10A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_11A_OFFSET                 0x0000000000000028
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_11A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_11A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_11A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_12A_OFFSET                 0x0000000000000030
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_12A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_12A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_12A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_13A_OFFSET                 0x0000000000000030
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_13A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_13A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_13A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_14A_OFFSET                 0x0000000000000038
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_14A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_14A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_14A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_15A_OFFSET                 0x0000000000000038
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_15A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_15A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_15A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_16A_OFFSET                 0x0000000000000040
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_16A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_16A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_16A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_17A_OFFSET                 0x0000000000000040
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_17A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_17A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_17A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_18A_OFFSET                 0x0000000000000048
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_18A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_18A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_18A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_19A_OFFSET                 0x0000000000000048
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_19A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_19A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_19A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_20A_OFFSET                 0x0000000000000050
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_20A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_20A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_20A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_21A_OFFSET                 0x0000000000000050
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_21A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_21A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_21A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_22A_OFFSET                 0x0000000000000058
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_22A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_22A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_22A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_23A_OFFSET                 0x0000000000000058
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_23A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_23A_MSB                    63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_23A_MASK                   0xffffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_24A_OFFSET                 0x0000000000000060
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_24A_LSB                    0
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_24A_MSB                    31
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_24A_MASK                   0x00000000ffffffff


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_25A_OFFSET                 0x0000000000000060
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_25A_LSB                    32
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_25A_MSB                    59
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_RESERVED_25A_MASK                   0x0fffffff00000000


 

#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LOOPING_COUNT_OFFSET                0x0000000000000060
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LOOPING_COUNT_LSB                   60
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LOOPING_COUNT_MSB                   63
#define REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS_LOOPING_COUNT_MASK                  0xf000000000000000



#endif    

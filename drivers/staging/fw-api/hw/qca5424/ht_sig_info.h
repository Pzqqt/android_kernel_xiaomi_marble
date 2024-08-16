
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _HT_SIG_INFO_H_
#define _HT_SIG_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HT_SIG_INFO 2


struct ht_sig_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mcs                                                     :  7,  
                      cbw                                                     :  1,  
                      length                                                  : 16,  
                      reserved_0                                              :  8;  
             uint32_t smoothing                                               :  1,  
                      not_sounding                                            :  1,  
                      ht_reserved                                             :  1,  
                      aggregation                                             :  1,  
                      stbc                                                    :  2,  
                      fec_coding                                              :  1,  
                      short_gi                                                :  1,  
                      num_ext_sp_str                                          :  2,  
                      crc                                                     :  8,  
                      signal_tail                                             :  6,  
                      reserved_1                                              :  7,  
                      rx_integrity_check_passed                               :  1;  
#else
             uint32_t reserved_0                                              :  8,  
                      length                                                  : 16,  
                      cbw                                                     :  1,  
                      mcs                                                     :  7;  
             uint32_t rx_integrity_check_passed                               :  1,  
                      reserved_1                                              :  7,  
                      signal_tail                                             :  6,  
                      crc                                                     :  8,  
                      num_ext_sp_str                                          :  2,  
                      short_gi                                                :  1,  
                      fec_coding                                              :  1,  
                      stbc                                                    :  2,  
                      aggregation                                             :  1,  
                      ht_reserved                                             :  1,  
                      not_sounding                                            :  1,  
                      smoothing                                               :  1;  
#endif
};


 

#define HT_SIG_INFO_MCS_OFFSET                                                      0x00000000
#define HT_SIG_INFO_MCS_LSB                                                         0
#define HT_SIG_INFO_MCS_MSB                                                         6
#define HT_SIG_INFO_MCS_MASK                                                        0x0000007f


 

#define HT_SIG_INFO_CBW_OFFSET                                                      0x00000000
#define HT_SIG_INFO_CBW_LSB                                                         7
#define HT_SIG_INFO_CBW_MSB                                                         7
#define HT_SIG_INFO_CBW_MASK                                                        0x00000080


 

#define HT_SIG_INFO_LENGTH_OFFSET                                                   0x00000000
#define HT_SIG_INFO_LENGTH_LSB                                                      8
#define HT_SIG_INFO_LENGTH_MSB                                                      23
#define HT_SIG_INFO_LENGTH_MASK                                                     0x00ffff00


 

#define HT_SIG_INFO_RESERVED_0_OFFSET                                               0x00000000
#define HT_SIG_INFO_RESERVED_0_LSB                                                  24
#define HT_SIG_INFO_RESERVED_0_MSB                                                  31
#define HT_SIG_INFO_RESERVED_0_MASK                                                 0xff000000


 

#define HT_SIG_INFO_SMOOTHING_OFFSET                                                0x00000004
#define HT_SIG_INFO_SMOOTHING_LSB                                                   0
#define HT_SIG_INFO_SMOOTHING_MSB                                                   0
#define HT_SIG_INFO_SMOOTHING_MASK                                                  0x00000001


 

#define HT_SIG_INFO_NOT_SOUNDING_OFFSET                                             0x00000004
#define HT_SIG_INFO_NOT_SOUNDING_LSB                                                1
#define HT_SIG_INFO_NOT_SOUNDING_MSB                                                1
#define HT_SIG_INFO_NOT_SOUNDING_MASK                                               0x00000002


 

#define HT_SIG_INFO_HT_RESERVED_OFFSET                                              0x00000004
#define HT_SIG_INFO_HT_RESERVED_LSB                                                 2
#define HT_SIG_INFO_HT_RESERVED_MSB                                                 2
#define HT_SIG_INFO_HT_RESERVED_MASK                                                0x00000004


 

#define HT_SIG_INFO_AGGREGATION_OFFSET                                              0x00000004
#define HT_SIG_INFO_AGGREGATION_LSB                                                 3
#define HT_SIG_INFO_AGGREGATION_MSB                                                 3
#define HT_SIG_INFO_AGGREGATION_MASK                                                0x00000008


 

#define HT_SIG_INFO_STBC_OFFSET                                                     0x00000004
#define HT_SIG_INFO_STBC_LSB                                                        4
#define HT_SIG_INFO_STBC_MSB                                                        5
#define HT_SIG_INFO_STBC_MASK                                                       0x00000030


 

#define HT_SIG_INFO_FEC_CODING_OFFSET                                               0x00000004
#define HT_SIG_INFO_FEC_CODING_LSB                                                  6
#define HT_SIG_INFO_FEC_CODING_MSB                                                  6
#define HT_SIG_INFO_FEC_CODING_MASK                                                 0x00000040


 

#define HT_SIG_INFO_SHORT_GI_OFFSET                                                 0x00000004
#define HT_SIG_INFO_SHORT_GI_LSB                                                    7
#define HT_SIG_INFO_SHORT_GI_MSB                                                    7
#define HT_SIG_INFO_SHORT_GI_MASK                                                   0x00000080


 

#define HT_SIG_INFO_NUM_EXT_SP_STR_OFFSET                                           0x00000004
#define HT_SIG_INFO_NUM_EXT_SP_STR_LSB                                              8
#define HT_SIG_INFO_NUM_EXT_SP_STR_MSB                                              9
#define HT_SIG_INFO_NUM_EXT_SP_STR_MASK                                             0x00000300


 

#define HT_SIG_INFO_CRC_OFFSET                                                      0x00000004
#define HT_SIG_INFO_CRC_LSB                                                         10
#define HT_SIG_INFO_CRC_MSB                                                         17
#define HT_SIG_INFO_CRC_MASK                                                        0x0003fc00


 

#define HT_SIG_INFO_SIGNAL_TAIL_OFFSET                                              0x00000004
#define HT_SIG_INFO_SIGNAL_TAIL_LSB                                                 18
#define HT_SIG_INFO_SIGNAL_TAIL_MSB                                                 23
#define HT_SIG_INFO_SIGNAL_TAIL_MASK                                                0x00fc0000


 

#define HT_SIG_INFO_RESERVED_1_OFFSET                                               0x00000004
#define HT_SIG_INFO_RESERVED_1_LSB                                                  24
#define HT_SIG_INFO_RESERVED_1_MSB                                                  30
#define HT_SIG_INFO_RESERVED_1_MASK                                                 0x7f000000


 

#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                                0x00000004
#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                                   31
#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                                   31
#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                                  0x80000000



#endif    

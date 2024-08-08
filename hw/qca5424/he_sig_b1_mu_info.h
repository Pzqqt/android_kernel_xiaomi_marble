
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _HE_SIG_B1_MU_INFO_H_
#define _HE_SIG_B1_MU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HE_SIG_B1_MU_INFO 1


struct he_sig_b1_mu_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ru_allocation                                           :  8,  
                      reserved_0                                              : 23,  
                      rx_integrity_check_passed                               :  1;  
#else
             uint32_t rx_integrity_check_passed                               :  1,  
                      reserved_0                                              : 23,  
                      ru_allocation                                           :  8;  
#endif
};


 

#define HE_SIG_B1_MU_INFO_RU_ALLOCATION_OFFSET                                      0x00000000
#define HE_SIG_B1_MU_INFO_RU_ALLOCATION_LSB                                         0
#define HE_SIG_B1_MU_INFO_RU_ALLOCATION_MSB                                         7
#define HE_SIG_B1_MU_INFO_RU_ALLOCATION_MASK                                        0x000000ff


 

#define HE_SIG_B1_MU_INFO_RESERVED_0_OFFSET                                         0x00000000
#define HE_SIG_B1_MU_INFO_RESERVED_0_LSB                                            8
#define HE_SIG_B1_MU_INFO_RESERVED_0_MSB                                            30
#define HE_SIG_B1_MU_INFO_RESERVED_0_MASK                                           0x7fffff00


 

#define HE_SIG_B1_MU_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                          0x00000000
#define HE_SIG_B1_MU_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                             31
#define HE_SIG_B1_MU_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                             31
#define HE_SIG_B1_MU_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                            0x80000000



#endif    

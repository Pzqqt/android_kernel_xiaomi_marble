
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _VHT_SIG_B_MU40_INFO_H_
#define _VHT_SIG_B_MU40_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_B_MU40_INFO 2


struct vht_sig_b_mu40_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 17,  
                      mcs                                                     :  4,  
                      tail                                                    :  6,  
                      reserved_0                                              :  2,  
                      mu_user_number                                          :  3;  
             uint32_t length_copy                                             : 17,  
                      mcs_copy                                                :  4,  
                      tail_copy                                               :  6,  
                      reserved_1                                              :  5;  
#else
             uint32_t mu_user_number                                          :  3,  
                      reserved_0                                              :  2,  
                      tail                                                    :  6,  
                      mcs                                                     :  4,  
                      length                                                  : 17;  
             uint32_t reserved_1                                              :  5,  
                      tail_copy                                               :  6,  
                      mcs_copy                                                :  4,  
                      length_copy                                             : 17;  
#endif
};


 

#define VHT_SIG_B_MU40_INFO_LENGTH_OFFSET                                           0x00000000
#define VHT_SIG_B_MU40_INFO_LENGTH_LSB                                              0
#define VHT_SIG_B_MU40_INFO_LENGTH_MSB                                              16
#define VHT_SIG_B_MU40_INFO_LENGTH_MASK                                             0x0001ffff


 

#define VHT_SIG_B_MU40_INFO_MCS_OFFSET                                              0x00000000
#define VHT_SIG_B_MU40_INFO_MCS_LSB                                                 17
#define VHT_SIG_B_MU40_INFO_MCS_MSB                                                 20
#define VHT_SIG_B_MU40_INFO_MCS_MASK                                                0x001e0000


 

#define VHT_SIG_B_MU40_INFO_TAIL_OFFSET                                             0x00000000
#define VHT_SIG_B_MU40_INFO_TAIL_LSB                                                21
#define VHT_SIG_B_MU40_INFO_TAIL_MSB                                                26
#define VHT_SIG_B_MU40_INFO_TAIL_MASK                                               0x07e00000


 

#define VHT_SIG_B_MU40_INFO_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_B_MU40_INFO_RESERVED_0_LSB                                          27
#define VHT_SIG_B_MU40_INFO_RESERVED_0_MSB                                          28
#define VHT_SIG_B_MU40_INFO_RESERVED_0_MASK                                         0x18000000


 

#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_OFFSET                                   0x00000000
#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_LSB                                      29
#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_MSB                                      31
#define VHT_SIG_B_MU40_INFO_MU_USER_NUMBER_MASK                                     0xe0000000


 

#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_OFFSET                                      0x00000004
#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_LSB                                         0
#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_MSB                                         16
#define VHT_SIG_B_MU40_INFO_LENGTH_COPY_MASK                                        0x0001ffff


 

#define VHT_SIG_B_MU40_INFO_MCS_COPY_OFFSET                                         0x00000004
#define VHT_SIG_B_MU40_INFO_MCS_COPY_LSB                                            17
#define VHT_SIG_B_MU40_INFO_MCS_COPY_MSB                                            20
#define VHT_SIG_B_MU40_INFO_MCS_COPY_MASK                                           0x001e0000


 

#define VHT_SIG_B_MU40_INFO_TAIL_COPY_OFFSET                                        0x00000004
#define VHT_SIG_B_MU40_INFO_TAIL_COPY_LSB                                           21
#define VHT_SIG_B_MU40_INFO_TAIL_COPY_MSB                                           26
#define VHT_SIG_B_MU40_INFO_TAIL_COPY_MASK                                          0x07e00000


 

#define VHT_SIG_B_MU40_INFO_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_B_MU40_INFO_RESERVED_1_LSB                                          27
#define VHT_SIG_B_MU40_INFO_RESERVED_1_MSB                                          31
#define VHT_SIG_B_MU40_INFO_RESERVED_1_MASK                                         0xf8000000



#endif    

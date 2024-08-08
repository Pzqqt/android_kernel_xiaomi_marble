
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _VHT_SIG_B_MU80_INFO_H_
#define _VHT_SIG_B_MU80_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_B_MU80_INFO 4


struct vht_sig_b_mu80_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 19,  
                      mcs                                                     :  4,  
                      tail                                                    :  6,  
                      reserved_0                                              :  3;  
             uint32_t length_copy_a                                           : 19,  
                      mcs_copy_a                                              :  4,  
                      tail_copy_a                                             :  6,  
                      reserved_1                                              :  3;  
             uint32_t length_copy_b                                           : 19,  
                      mcs_copy_b                                              :  4,  
                      tail_copy_b                                             :  6,  
                      mu_user_number                                          :  3;  
             uint32_t length_copy_c                                           : 19,  
                      mcs_copy_c                                              :  4,  
                      tail_copy_c                                             :  6,  
                      reserved_3                                              :  3;  
#else
             uint32_t reserved_0                                              :  3,  
                      tail                                                    :  6,  
                      mcs                                                     :  4,  
                      length                                                  : 19;  
             uint32_t reserved_1                                              :  3,  
                      tail_copy_a                                             :  6,  
                      mcs_copy_a                                              :  4,  
                      length_copy_a                                           : 19;  
             uint32_t mu_user_number                                          :  3,  
                      tail_copy_b                                             :  6,  
                      mcs_copy_b                                              :  4,  
                      length_copy_b                                           : 19;  
             uint32_t reserved_3                                              :  3,  
                      tail_copy_c                                             :  6,  
                      mcs_copy_c                                              :  4,  
                      length_copy_c                                           : 19;  
#endif
};


 

#define VHT_SIG_B_MU80_INFO_LENGTH_OFFSET                                           0x00000000
#define VHT_SIG_B_MU80_INFO_LENGTH_LSB                                              0
#define VHT_SIG_B_MU80_INFO_LENGTH_MSB                                              18
#define VHT_SIG_B_MU80_INFO_LENGTH_MASK                                             0x0007ffff


 

#define VHT_SIG_B_MU80_INFO_MCS_OFFSET                                              0x00000000
#define VHT_SIG_B_MU80_INFO_MCS_LSB                                                 19
#define VHT_SIG_B_MU80_INFO_MCS_MSB                                                 22
#define VHT_SIG_B_MU80_INFO_MCS_MASK                                                0x00780000


 

#define VHT_SIG_B_MU80_INFO_TAIL_OFFSET                                             0x00000000
#define VHT_SIG_B_MU80_INFO_TAIL_LSB                                                23
#define VHT_SIG_B_MU80_INFO_TAIL_MSB                                                28
#define VHT_SIG_B_MU80_INFO_TAIL_MASK                                               0x1f800000


 

#define VHT_SIG_B_MU80_INFO_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_B_MU80_INFO_RESERVED_0_LSB                                          29
#define VHT_SIG_B_MU80_INFO_RESERVED_0_MSB                                          31
#define VHT_SIG_B_MU80_INFO_RESERVED_0_MASK                                         0xe0000000


 

#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_A_OFFSET                                    0x00000004
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_A_LSB                                       0
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_A_MSB                                       18
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_A_MASK                                      0x0007ffff


 

#define VHT_SIG_B_MU80_INFO_MCS_COPY_A_OFFSET                                       0x00000004
#define VHT_SIG_B_MU80_INFO_MCS_COPY_A_LSB                                          19
#define VHT_SIG_B_MU80_INFO_MCS_COPY_A_MSB                                          22
#define VHT_SIG_B_MU80_INFO_MCS_COPY_A_MASK                                         0x00780000


 

#define VHT_SIG_B_MU80_INFO_TAIL_COPY_A_OFFSET                                      0x00000004
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_A_LSB                                         23
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_A_MSB                                         28
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_A_MASK                                        0x1f800000


 

#define VHT_SIG_B_MU80_INFO_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_B_MU80_INFO_RESERVED_1_LSB                                          29
#define VHT_SIG_B_MU80_INFO_RESERVED_1_MSB                                          31
#define VHT_SIG_B_MU80_INFO_RESERVED_1_MASK                                         0xe0000000


 

#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_B_OFFSET                                    0x00000008
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_B_LSB                                       0
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_B_MSB                                       18
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_B_MASK                                      0x0007ffff


 

#define VHT_SIG_B_MU80_INFO_MCS_COPY_B_OFFSET                                       0x00000008
#define VHT_SIG_B_MU80_INFO_MCS_COPY_B_LSB                                          19
#define VHT_SIG_B_MU80_INFO_MCS_COPY_B_MSB                                          22
#define VHT_SIG_B_MU80_INFO_MCS_COPY_B_MASK                                         0x00780000


 

#define VHT_SIG_B_MU80_INFO_TAIL_COPY_B_OFFSET                                      0x00000008
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_B_LSB                                         23
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_B_MSB                                         28
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_B_MASK                                        0x1f800000


 

#define VHT_SIG_B_MU80_INFO_MU_USER_NUMBER_OFFSET                                   0x00000008
#define VHT_SIG_B_MU80_INFO_MU_USER_NUMBER_LSB                                      29
#define VHT_SIG_B_MU80_INFO_MU_USER_NUMBER_MSB                                      31
#define VHT_SIG_B_MU80_INFO_MU_USER_NUMBER_MASK                                     0xe0000000


 

#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_C_OFFSET                                    0x0000000c
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_C_LSB                                       0
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_C_MSB                                       18
#define VHT_SIG_B_MU80_INFO_LENGTH_COPY_C_MASK                                      0x0007ffff


 

#define VHT_SIG_B_MU80_INFO_MCS_COPY_C_OFFSET                                       0x0000000c
#define VHT_SIG_B_MU80_INFO_MCS_COPY_C_LSB                                          19
#define VHT_SIG_B_MU80_INFO_MCS_COPY_C_MSB                                          22
#define VHT_SIG_B_MU80_INFO_MCS_COPY_C_MASK                                         0x00780000


 

#define VHT_SIG_B_MU80_INFO_TAIL_COPY_C_OFFSET                                      0x0000000c
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_C_LSB                                         23
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_C_MSB                                         28
#define VHT_SIG_B_MU80_INFO_TAIL_COPY_C_MASK                                        0x1f800000


 

#define VHT_SIG_B_MU80_INFO_RESERVED_3_OFFSET                                       0x0000000c
#define VHT_SIG_B_MU80_INFO_RESERVED_3_LSB                                          29
#define VHT_SIG_B_MU80_INFO_RESERVED_3_MSB                                          31
#define VHT_SIG_B_MU80_INFO_RESERVED_3_MASK                                         0xe0000000



#endif    


/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _VHT_SIG_A_INFO_H_
#define _VHT_SIG_A_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_A_INFO 2


struct vht_sig_a_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t bandwidth                                               :  2,  
                      vhta_reserved_0                                         :  1,  
                      stbc                                                    :  1,  
                      group_id                                                :  6,  
                      n_sts                                                   : 12,  
                      txop_ps_not_allowed                                     :  1,  
                      vhta_reserved_0b                                        :  1,  
                      reserved_0                                              :  8;  
             uint32_t gi_setting                                              :  2,  
                      su_mu_coding                                            :  1,  
                      ldpc_extra_symbol                                       :  1,  
                      mcs                                                     :  4,  
                      beamformed                                              :  1,  
                      vhta_reserved_1                                         :  1,  
                      crc                                                     :  8,  
                      tail                                                    :  6,  
                      reserved_1                                              :  7,  
                      rx_integrity_check_passed                               :  1;  
#else
             uint32_t reserved_0                                              :  8,  
                      vhta_reserved_0b                                        :  1,  
                      txop_ps_not_allowed                                     :  1,  
                      n_sts                                                   : 12,  
                      group_id                                                :  6,  
                      stbc                                                    :  1,  
                      vhta_reserved_0                                         :  1,  
                      bandwidth                                               :  2;  
             uint32_t rx_integrity_check_passed                               :  1,  
                      reserved_1                                              :  7,  
                      tail                                                    :  6,  
                      crc                                                     :  8,  
                      vhta_reserved_1                                         :  1,  
                      beamformed                                              :  1,  
                      mcs                                                     :  4,  
                      ldpc_extra_symbol                                       :  1,  
                      su_mu_coding                                            :  1,  
                      gi_setting                                              :  2;  
#endif
};


 

#define VHT_SIG_A_INFO_BANDWIDTH_OFFSET                                             0x00000000
#define VHT_SIG_A_INFO_BANDWIDTH_LSB                                                0
#define VHT_SIG_A_INFO_BANDWIDTH_MSB                                                1
#define VHT_SIG_A_INFO_BANDWIDTH_MASK                                               0x00000003


 

#define VHT_SIG_A_INFO_VHTA_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_A_INFO_VHTA_RESERVED_0_LSB                                          2
#define VHT_SIG_A_INFO_VHTA_RESERVED_0_MSB                                          2
#define VHT_SIG_A_INFO_VHTA_RESERVED_0_MASK                                         0x00000004


 

#define VHT_SIG_A_INFO_STBC_OFFSET                                                  0x00000000
#define VHT_SIG_A_INFO_STBC_LSB                                                     3
#define VHT_SIG_A_INFO_STBC_MSB                                                     3
#define VHT_SIG_A_INFO_STBC_MASK                                                    0x00000008


 

#define VHT_SIG_A_INFO_GROUP_ID_OFFSET                                              0x00000000
#define VHT_SIG_A_INFO_GROUP_ID_LSB                                                 4
#define VHT_SIG_A_INFO_GROUP_ID_MSB                                                 9
#define VHT_SIG_A_INFO_GROUP_ID_MASK                                                0x000003f0


 

#define VHT_SIG_A_INFO_N_STS_OFFSET                                                 0x00000000
#define VHT_SIG_A_INFO_N_STS_LSB                                                    10
#define VHT_SIG_A_INFO_N_STS_MSB                                                    21
#define VHT_SIG_A_INFO_N_STS_MASK                                                   0x003ffc00


 

#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_OFFSET                                   0x00000000
#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_LSB                                      22
#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_MSB                                      22
#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_MASK                                     0x00400000


 

#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_OFFSET                                      0x00000000
#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_LSB                                         23
#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_MSB                                         23
#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_MASK                                        0x00800000


 

#define VHT_SIG_A_INFO_RESERVED_0_OFFSET                                            0x00000000
#define VHT_SIG_A_INFO_RESERVED_0_LSB                                               24
#define VHT_SIG_A_INFO_RESERVED_0_MSB                                               31
#define VHT_SIG_A_INFO_RESERVED_0_MASK                                              0xff000000


 

#define VHT_SIG_A_INFO_GI_SETTING_OFFSET                                            0x00000004
#define VHT_SIG_A_INFO_GI_SETTING_LSB                                               0
#define VHT_SIG_A_INFO_GI_SETTING_MSB                                               1
#define VHT_SIG_A_INFO_GI_SETTING_MASK                                              0x00000003


 

#define VHT_SIG_A_INFO_SU_MU_CODING_OFFSET                                          0x00000004
#define VHT_SIG_A_INFO_SU_MU_CODING_LSB                                             2
#define VHT_SIG_A_INFO_SU_MU_CODING_MSB                                             2
#define VHT_SIG_A_INFO_SU_MU_CODING_MASK                                            0x00000004


 

#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_OFFSET                                     0x00000004
#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_LSB                                        3
#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_MSB                                        3
#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_MASK                                       0x00000008


 

#define VHT_SIG_A_INFO_MCS_OFFSET                                                   0x00000004
#define VHT_SIG_A_INFO_MCS_LSB                                                      4
#define VHT_SIG_A_INFO_MCS_MSB                                                      7
#define VHT_SIG_A_INFO_MCS_MASK                                                     0x000000f0


 

#define VHT_SIG_A_INFO_BEAMFORMED_OFFSET                                            0x00000004
#define VHT_SIG_A_INFO_BEAMFORMED_LSB                                               8
#define VHT_SIG_A_INFO_BEAMFORMED_MSB                                               8
#define VHT_SIG_A_INFO_BEAMFORMED_MASK                                              0x00000100


 

#define VHT_SIG_A_INFO_VHTA_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_A_INFO_VHTA_RESERVED_1_LSB                                          9
#define VHT_SIG_A_INFO_VHTA_RESERVED_1_MSB                                          9
#define VHT_SIG_A_INFO_VHTA_RESERVED_1_MASK                                         0x00000200


 

#define VHT_SIG_A_INFO_CRC_OFFSET                                                   0x00000004
#define VHT_SIG_A_INFO_CRC_LSB                                                      10
#define VHT_SIG_A_INFO_CRC_MSB                                                      17
#define VHT_SIG_A_INFO_CRC_MASK                                                     0x0003fc00


 

#define VHT_SIG_A_INFO_TAIL_OFFSET                                                  0x00000004
#define VHT_SIG_A_INFO_TAIL_LSB                                                     18
#define VHT_SIG_A_INFO_TAIL_MSB                                                     23
#define VHT_SIG_A_INFO_TAIL_MASK                                                    0x00fc0000


 

#define VHT_SIG_A_INFO_RESERVED_1_OFFSET                                            0x00000004
#define VHT_SIG_A_INFO_RESERVED_1_LSB                                               24
#define VHT_SIG_A_INFO_RESERVED_1_MSB                                               30
#define VHT_SIG_A_INFO_RESERVED_1_MASK                                              0x7f000000


 

#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                             0x00000004
#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                                31
#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                                31
#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                               0x80000000



#endif    

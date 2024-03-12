
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _EHT_SIG_USR_SU_INFO_H_
#define _EHT_SIG_USR_SU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_EHT_SIG_USR_SU_INFO 1


struct eht_sig_usr_su_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t sta_id                                                  : 11,  
                      sta_mcs                                                 :  4,  
                      validate_0a                                             :  1,  
                      nss                                                     :  4,  
                      txbf                                                    :  1,  
                      sta_coding                                              :  1,  
                      reserved_0b                                             :  9,  
                      rx_integrity_check_passed                               :  1;  
#else
             uint32_t rx_integrity_check_passed                               :  1,  
                      reserved_0b                                             :  9,  
                      sta_coding                                              :  1,  
                      txbf                                                    :  1,  
                      nss                                                     :  4,  
                      validate_0a                                             :  1,  
                      sta_mcs                                                 :  4,  
                      sta_id                                                  : 11;  
#endif
};


 

#define EHT_SIG_USR_SU_INFO_STA_ID_OFFSET                                           0x00000000
#define EHT_SIG_USR_SU_INFO_STA_ID_LSB                                              0
#define EHT_SIG_USR_SU_INFO_STA_ID_MSB                                              10
#define EHT_SIG_USR_SU_INFO_STA_ID_MASK                                             0x000007ff


 

#define EHT_SIG_USR_SU_INFO_STA_MCS_OFFSET                                          0x00000000
#define EHT_SIG_USR_SU_INFO_STA_MCS_LSB                                             11
#define EHT_SIG_USR_SU_INFO_STA_MCS_MSB                                             14
#define EHT_SIG_USR_SU_INFO_STA_MCS_MASK                                            0x00007800


 

#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_OFFSET                                      0x00000000
#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_LSB                                         15
#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_MSB                                         15
#define EHT_SIG_USR_SU_INFO_VALIDATE_0A_MASK                                        0x00008000


 

#define EHT_SIG_USR_SU_INFO_NSS_OFFSET                                              0x00000000
#define EHT_SIG_USR_SU_INFO_NSS_LSB                                                 16
#define EHT_SIG_USR_SU_INFO_NSS_MSB                                                 19
#define EHT_SIG_USR_SU_INFO_NSS_MASK                                                0x000f0000


 

#define EHT_SIG_USR_SU_INFO_TXBF_OFFSET                                             0x00000000
#define EHT_SIG_USR_SU_INFO_TXBF_LSB                                                20
#define EHT_SIG_USR_SU_INFO_TXBF_MSB                                                20
#define EHT_SIG_USR_SU_INFO_TXBF_MASK                                               0x00100000


 

#define EHT_SIG_USR_SU_INFO_STA_CODING_OFFSET                                       0x00000000
#define EHT_SIG_USR_SU_INFO_STA_CODING_LSB                                          21
#define EHT_SIG_USR_SU_INFO_STA_CODING_MSB                                          21
#define EHT_SIG_USR_SU_INFO_STA_CODING_MASK                                         0x00200000


 

#define EHT_SIG_USR_SU_INFO_RESERVED_0B_OFFSET                                      0x00000000
#define EHT_SIG_USR_SU_INFO_RESERVED_0B_LSB                                         22
#define EHT_SIG_USR_SU_INFO_RESERVED_0B_MSB                                         30
#define EHT_SIG_USR_SU_INFO_RESERVED_0B_MASK                                        0x7fc00000


 

#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                        0x00000000
#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                           31
#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                           31
#define EHT_SIG_USR_SU_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                          0x80000000



#endif    

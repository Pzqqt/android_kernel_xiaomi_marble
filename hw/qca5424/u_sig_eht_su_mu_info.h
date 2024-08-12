
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _U_SIG_EHT_SU_MU_INFO_H_
#define _U_SIG_EHT_SU_MU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_U_SIG_EHT_SU_MU_INFO 2


struct u_sig_eht_su_mu_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_version                                             :  3,  
                      transmit_bw                                             :  3,  
                      dl_ul_flag                                              :  1,  
                      bss_color_id                                            :  6,  
                      txop_duration                                           :  7,  
                      disregard_0a                                            :  5,  
                      validate_0b                                             :  1,  
                      reserved_0c                                             :  6;  
             uint32_t eht_ppdu_sig_cmn_type                                   :  2,  
                      validate_1a                                             :  1,  
                      punctured_channel_information                           :  5,  
                      validate_1b                                             :  1,  
                      mcs_of_eht_sig                                          :  2,  
                      num_eht_sig_symbols                                     :  5,  
                      crc                                                     :  4,  
                      tail                                                    :  6,  
                      dot11ax_su_extended                                     :  1,  
                      reserved_1d                                             :  3,  
                      rx_ndp                                                  :  1,  
                      rx_integrity_check_passed                               :  1;  
#else
             uint32_t reserved_0c                                             :  6,  
                      validate_0b                                             :  1,  
                      disregard_0a                                            :  5,  
                      txop_duration                                           :  7,  
                      bss_color_id                                            :  6,  
                      dl_ul_flag                                              :  1,  
                      transmit_bw                                             :  3,  
                      phy_version                                             :  3;  
             uint32_t rx_integrity_check_passed                               :  1,  
                      rx_ndp                                                  :  1,  
                      reserved_1d                                             :  3,  
                      dot11ax_su_extended                                     :  1,  
                      tail                                                    :  6,  
                      crc                                                     :  4,  
                      num_eht_sig_symbols                                     :  5,  
                      mcs_of_eht_sig                                          :  2,  
                      validate_1b                                             :  1,  
                      punctured_channel_information                           :  5,  
                      validate_1a                                             :  1,  
                      eht_ppdu_sig_cmn_type                                   :  2;  
#endif
};


 

#define U_SIG_EHT_SU_MU_INFO_PHY_VERSION_OFFSET                                     0x00000000
#define U_SIG_EHT_SU_MU_INFO_PHY_VERSION_LSB                                        0
#define U_SIG_EHT_SU_MU_INFO_PHY_VERSION_MSB                                        2
#define U_SIG_EHT_SU_MU_INFO_PHY_VERSION_MASK                                       0x00000007


 

#define U_SIG_EHT_SU_MU_INFO_TRANSMIT_BW_OFFSET                                     0x00000000
#define U_SIG_EHT_SU_MU_INFO_TRANSMIT_BW_LSB                                        3
#define U_SIG_EHT_SU_MU_INFO_TRANSMIT_BW_MSB                                        5
#define U_SIG_EHT_SU_MU_INFO_TRANSMIT_BW_MASK                                       0x00000038


 

#define U_SIG_EHT_SU_MU_INFO_DL_UL_FLAG_OFFSET                                      0x00000000
#define U_SIG_EHT_SU_MU_INFO_DL_UL_FLAG_LSB                                         6
#define U_SIG_EHT_SU_MU_INFO_DL_UL_FLAG_MSB                                         6
#define U_SIG_EHT_SU_MU_INFO_DL_UL_FLAG_MASK                                        0x00000040


 

#define U_SIG_EHT_SU_MU_INFO_BSS_COLOR_ID_OFFSET                                    0x00000000
#define U_SIG_EHT_SU_MU_INFO_BSS_COLOR_ID_LSB                                       7
#define U_SIG_EHT_SU_MU_INFO_BSS_COLOR_ID_MSB                                       12
#define U_SIG_EHT_SU_MU_INFO_BSS_COLOR_ID_MASK                                      0x00001f80


 

#define U_SIG_EHT_SU_MU_INFO_TXOP_DURATION_OFFSET                                   0x00000000
#define U_SIG_EHT_SU_MU_INFO_TXOP_DURATION_LSB                                      13
#define U_SIG_EHT_SU_MU_INFO_TXOP_DURATION_MSB                                      19
#define U_SIG_EHT_SU_MU_INFO_TXOP_DURATION_MASK                                     0x000fe000


 

#define U_SIG_EHT_SU_MU_INFO_DISREGARD_0A_OFFSET                                    0x00000000
#define U_SIG_EHT_SU_MU_INFO_DISREGARD_0A_LSB                                       20
#define U_SIG_EHT_SU_MU_INFO_DISREGARD_0A_MSB                                       24
#define U_SIG_EHT_SU_MU_INFO_DISREGARD_0A_MASK                                      0x01f00000


 

#define U_SIG_EHT_SU_MU_INFO_VALIDATE_0B_OFFSET                                     0x00000000
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_0B_LSB                                        25
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_0B_MSB                                        25
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_0B_MASK                                       0x02000000


 

#define U_SIG_EHT_SU_MU_INFO_RESERVED_0C_OFFSET                                     0x00000000
#define U_SIG_EHT_SU_MU_INFO_RESERVED_0C_LSB                                        26
#define U_SIG_EHT_SU_MU_INFO_RESERVED_0C_MSB                                        31
#define U_SIG_EHT_SU_MU_INFO_RESERVED_0C_MASK                                       0xfc000000


 

#define U_SIG_EHT_SU_MU_INFO_EHT_PPDU_SIG_CMN_TYPE_OFFSET                           0x00000004
#define U_SIG_EHT_SU_MU_INFO_EHT_PPDU_SIG_CMN_TYPE_LSB                              0
#define U_SIG_EHT_SU_MU_INFO_EHT_PPDU_SIG_CMN_TYPE_MSB                              1
#define U_SIG_EHT_SU_MU_INFO_EHT_PPDU_SIG_CMN_TYPE_MASK                             0x00000003


 

#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1A_OFFSET                                     0x00000004
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1A_LSB                                        2
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1A_MSB                                        2
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1A_MASK                                       0x00000004


 

#define U_SIG_EHT_SU_MU_INFO_PUNCTURED_CHANNEL_INFORMATION_OFFSET                   0x00000004
#define U_SIG_EHT_SU_MU_INFO_PUNCTURED_CHANNEL_INFORMATION_LSB                      3
#define U_SIG_EHT_SU_MU_INFO_PUNCTURED_CHANNEL_INFORMATION_MSB                      7
#define U_SIG_EHT_SU_MU_INFO_PUNCTURED_CHANNEL_INFORMATION_MASK                     0x000000f8


 

#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1B_OFFSET                                     0x00000004
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1B_LSB                                        8
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1B_MSB                                        8
#define U_SIG_EHT_SU_MU_INFO_VALIDATE_1B_MASK                                       0x00000100


 

#define U_SIG_EHT_SU_MU_INFO_MCS_OF_EHT_SIG_OFFSET                                  0x00000004
#define U_SIG_EHT_SU_MU_INFO_MCS_OF_EHT_SIG_LSB                                     9
#define U_SIG_EHT_SU_MU_INFO_MCS_OF_EHT_SIG_MSB                                     10
#define U_SIG_EHT_SU_MU_INFO_MCS_OF_EHT_SIG_MASK                                    0x00000600


 

#define U_SIG_EHT_SU_MU_INFO_NUM_EHT_SIG_SYMBOLS_OFFSET                             0x00000004
#define U_SIG_EHT_SU_MU_INFO_NUM_EHT_SIG_SYMBOLS_LSB                                11
#define U_SIG_EHT_SU_MU_INFO_NUM_EHT_SIG_SYMBOLS_MSB                                15
#define U_SIG_EHT_SU_MU_INFO_NUM_EHT_SIG_SYMBOLS_MASK                               0x0000f800


 

#define U_SIG_EHT_SU_MU_INFO_CRC_OFFSET                                             0x00000004
#define U_SIG_EHT_SU_MU_INFO_CRC_LSB                                                16
#define U_SIG_EHT_SU_MU_INFO_CRC_MSB                                                19
#define U_SIG_EHT_SU_MU_INFO_CRC_MASK                                               0x000f0000


 

#define U_SIG_EHT_SU_MU_INFO_TAIL_OFFSET                                            0x00000004
#define U_SIG_EHT_SU_MU_INFO_TAIL_LSB                                               20
#define U_SIG_EHT_SU_MU_INFO_TAIL_MSB                                               25
#define U_SIG_EHT_SU_MU_INFO_TAIL_MASK                                              0x03f00000


 

#define U_SIG_EHT_SU_MU_INFO_DOT11AX_SU_EXTENDED_OFFSET                             0x00000004
#define U_SIG_EHT_SU_MU_INFO_DOT11AX_SU_EXTENDED_LSB                                26
#define U_SIG_EHT_SU_MU_INFO_DOT11AX_SU_EXTENDED_MSB                                26
#define U_SIG_EHT_SU_MU_INFO_DOT11AX_SU_EXTENDED_MASK                               0x04000000


 

#define U_SIG_EHT_SU_MU_INFO_RESERVED_1D_OFFSET                                     0x00000004
#define U_SIG_EHT_SU_MU_INFO_RESERVED_1D_LSB                                        27
#define U_SIG_EHT_SU_MU_INFO_RESERVED_1D_MSB                                        29
#define U_SIG_EHT_SU_MU_INFO_RESERVED_1D_MASK                                       0x38000000


 

#define U_SIG_EHT_SU_MU_INFO_RX_NDP_OFFSET                                          0x00000004
#define U_SIG_EHT_SU_MU_INFO_RX_NDP_LSB                                             30
#define U_SIG_EHT_SU_MU_INFO_RX_NDP_MSB                                             30
#define U_SIG_EHT_SU_MU_INFO_RX_NDP_MASK                                            0x40000000


 

#define U_SIG_EHT_SU_MU_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                       0x00000004
#define U_SIG_EHT_SU_MU_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                          31
#define U_SIG_EHT_SU_MU_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                          31
#define U_SIG_EHT_SU_MU_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                         0x80000000



#endif    

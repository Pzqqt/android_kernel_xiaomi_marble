
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _VHT_SIG_B_SU80_INFO_H_
#define _VHT_SIG_B_SU80_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_B_SU80_INFO 4


struct vht_sig_b_su80_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t length                                                  : 21,  
                      vhtb_reserved                                           :  2,  
                      tail                                                    :  6,  
                      reserved_0                                              :  2,  
                      rx_ndp                                                  :  1;  
             uint32_t length_copy_a                                           : 21,  
                      vhtb_reserved_copy_a                                    :  2,  
                      tail_copy_a                                             :  6,  
                      reserved_1                                              :  2,  
                      rx_ndp_copy_a                                           :  1;  
             uint32_t length_copy_b                                           : 21,  
                      vhtb_reserved_copy_b                                    :  2,  
                      tail_copy_b                                             :  6,  
                      reserved_2                                              :  2,  
                      rx_ndp_copy_b                                           :  1;  
             uint32_t length_copy_c                                           : 21,  
                      vhtb_reserved_copy_c                                    :  2,  
                      tail_copy_c                                             :  6,  
                      reserved_3                                              :  2,  
                      rx_ndp_copy_c                                           :  1;  
#else
             uint32_t rx_ndp                                                  :  1,  
                      reserved_0                                              :  2,  
                      tail                                                    :  6,  
                      vhtb_reserved                                           :  2,  
                      length                                                  : 21;  
             uint32_t rx_ndp_copy_a                                           :  1,  
                      reserved_1                                              :  2,  
                      tail_copy_a                                             :  6,  
                      vhtb_reserved_copy_a                                    :  2,  
                      length_copy_a                                           : 21;  
             uint32_t rx_ndp_copy_b                                           :  1,  
                      reserved_2                                              :  2,  
                      tail_copy_b                                             :  6,  
                      vhtb_reserved_copy_b                                    :  2,  
                      length_copy_b                                           : 21;  
             uint32_t rx_ndp_copy_c                                           :  1,  
                      reserved_3                                              :  2,  
                      tail_copy_c                                             :  6,  
                      vhtb_reserved_copy_c                                    :  2,  
                      length_copy_c                                           : 21;  
#endif
};


 

#define VHT_SIG_B_SU80_INFO_LENGTH_OFFSET                                           0x00000000
#define VHT_SIG_B_SU80_INFO_LENGTH_LSB                                              0
#define VHT_SIG_B_SU80_INFO_LENGTH_MSB                                              20
#define VHT_SIG_B_SU80_INFO_LENGTH_MASK                                             0x001fffff


 

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_OFFSET                                    0x00000000
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_LSB                                       21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_MSB                                       22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_MASK                                      0x00600000


 

#define VHT_SIG_B_SU80_INFO_TAIL_OFFSET                                             0x00000000
#define VHT_SIG_B_SU80_INFO_TAIL_LSB                                                23
#define VHT_SIG_B_SU80_INFO_TAIL_MSB                                                28
#define VHT_SIG_B_SU80_INFO_TAIL_MASK                                               0x1f800000


 

#define VHT_SIG_B_SU80_INFO_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_B_SU80_INFO_RESERVED_0_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_0_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_0_MASK                                         0x60000000


 

#define VHT_SIG_B_SU80_INFO_RX_NDP_OFFSET                                           0x00000000
#define VHT_SIG_B_SU80_INFO_RX_NDP_LSB                                              31
#define VHT_SIG_B_SU80_INFO_RX_NDP_MSB                                              31
#define VHT_SIG_B_SU80_INFO_RX_NDP_MASK                                             0x80000000


 

#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_OFFSET                                    0x00000004
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_LSB                                       0
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_MSB                                       20
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_A_MASK                                      0x001fffff


 

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_OFFSET                             0x00000004
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_LSB                                21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_MSB                                22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_A_MASK                               0x00600000


 

#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_OFFSET                                      0x00000004
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_LSB                                         23
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_MSB                                         28
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_A_MASK                                        0x1f800000


 

#define VHT_SIG_B_SU80_INFO_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_B_SU80_INFO_RESERVED_1_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_1_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_1_MASK                                         0x60000000


 

#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_OFFSET                                    0x00000004
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_LSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_MSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_A_MASK                                      0x80000000


 

#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_OFFSET                                    0x00000008
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_LSB                                       0
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_MSB                                       20
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_B_MASK                                      0x001fffff


 

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_OFFSET                             0x00000008
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_LSB                                21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_MSB                                22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_B_MASK                               0x00600000


 

#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_OFFSET                                      0x00000008
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_LSB                                         23
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_MSB                                         28
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_B_MASK                                        0x1f800000


 

#define VHT_SIG_B_SU80_INFO_RESERVED_2_OFFSET                                       0x00000008
#define VHT_SIG_B_SU80_INFO_RESERVED_2_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_2_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_2_MASK                                         0x60000000


 

#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_OFFSET                                    0x00000008
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_LSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_MSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_B_MASK                                      0x80000000


 

#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_OFFSET                                    0x0000000c
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_LSB                                       0
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_MSB                                       20
#define VHT_SIG_B_SU80_INFO_LENGTH_COPY_C_MASK                                      0x001fffff


 

#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_OFFSET                             0x0000000c
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_LSB                                21
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_MSB                                22
#define VHT_SIG_B_SU80_INFO_VHTB_RESERVED_COPY_C_MASK                               0x00600000


 

#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_OFFSET                                      0x0000000c
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_LSB                                         23
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_MSB                                         28
#define VHT_SIG_B_SU80_INFO_TAIL_COPY_C_MASK                                        0x1f800000


 

#define VHT_SIG_B_SU80_INFO_RESERVED_3_OFFSET                                       0x0000000c
#define VHT_SIG_B_SU80_INFO_RESERVED_3_LSB                                          29
#define VHT_SIG_B_SU80_INFO_RESERVED_3_MSB                                          30
#define VHT_SIG_B_SU80_INFO_RESERVED_3_MASK                                         0x60000000


 

#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_OFFSET                                    0x0000000c
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_LSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_MSB                                       31
#define VHT_SIG_B_SU80_INFO_RX_NDP_COPY_C_MASK                                      0x80000000



#endif    

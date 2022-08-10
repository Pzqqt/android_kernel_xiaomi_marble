
/* Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

 
 
 
 
 
 
 


#ifndef _REO_UPDATE_RX_REO_QUEUE_H_
#define _REO_UPDATE_RX_REO_QUEUE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"
#define NUM_OF_DWORDS_REO_UPDATE_RX_REO_QUEUE 10

#define NUM_OF_QWORDS_REO_UPDATE_RX_REO_QUEUE 5


struct reo_update_rx_reo_queue {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32;  
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8,  
                      update_receive_queue_number                             :  1,  
                      update_vld                                              :  1,  
                      update_associated_link_descriptor_counter               :  1,  
                      update_disable_duplicate_detection                      :  1,  
                      update_soft_reorder_enable                              :  1,  
                      update_ac                                               :  1,  
                      update_bar                                              :  1,  
                      update_rty                                              :  1,  
                      update_chk_2k_mode                                      :  1,  
                      update_oor_mode                                         :  1,  
                      update_ba_window_size                                   :  1,  
                      update_pn_check_needed                                  :  1,  
                      update_pn_shall_be_even                                 :  1,  
                      update_pn_shall_be_uneven                               :  1,  
                      update_pn_handling_enable                               :  1,  
                      update_pn_size                                          :  1,  
                      update_ignore_ampdu_flag                                :  1,  
                      update_svld                                             :  1,  
                      update_ssn                                              :  1,  
                      update_seq_2k_error_detected_flag                       :  1,  
                      update_pn_error_detected_flag                           :  1,  
                      update_pn_valid                                         :  1,  
                      update_pn                                               :  1,  
                      clear_stat_counters                                     :  1;  
             uint32_t receive_queue_number                                    : 16,  
                      vld                                                     :  1,  
                      associated_link_descriptor_counter                      :  2,  
                      disable_duplicate_detection                             :  1,  
                      soft_reorder_enable                                     :  1,  
                      ac                                                      :  2,  
                      bar                                                     :  1,  
                      rty                                                     :  1,  
                      chk_2k_mode                                             :  1,  
                      oor_mode                                                :  1,  
                      pn_check_needed                                         :  1,  
                      pn_shall_be_even                                        :  1,  
                      pn_shall_be_uneven                                      :  1,  
                      pn_handling_enable                                      :  1,  
                      ignore_ampdu_flag                                       :  1;  
             uint32_t ba_window_size                                          : 10,  
                      pn_size                                                 :  2,  
                      svld                                                    :  1,  
                      ssn                                                     : 12,  
                      seq_2k_error_detected_flag                              :  1,  
                      pn_error_detected_flag                                  :  1,  
                      pn_valid                                                :  1,  
                      flush_from_cache                                        :  1,  
                      reserved_4a                                             :  3;  
             uint32_t pn_31_0                                                 : 32;  
             uint32_t pn_63_32                                                : 32;  
             uint32_t pn_95_64                                                : 32;  
             uint32_t pn_127_96                                               : 32;  
             uint32_t tlv64_padding                                           : 32;  
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32;  
             uint32_t clear_stat_counters                                     :  1,  
                      update_pn                                               :  1,  
                      update_pn_valid                                         :  1,  
                      update_pn_error_detected_flag                           :  1,  
                      update_seq_2k_error_detected_flag                       :  1,  
                      update_ssn                                              :  1,  
                      update_svld                                             :  1,  
                      update_ignore_ampdu_flag                                :  1,  
                      update_pn_size                                          :  1,  
                      update_pn_handling_enable                               :  1,  
                      update_pn_shall_be_uneven                               :  1,  
                      update_pn_shall_be_even                                 :  1,  
                      update_pn_check_needed                                  :  1,  
                      update_ba_window_size                                   :  1,  
                      update_oor_mode                                         :  1,  
                      update_chk_2k_mode                                      :  1,  
                      update_rty                                              :  1,  
                      update_bar                                              :  1,  
                      update_ac                                               :  1,  
                      update_soft_reorder_enable                              :  1,  
                      update_disable_duplicate_detection                      :  1,  
                      update_associated_link_descriptor_counter               :  1,  
                      update_vld                                              :  1,  
                      update_receive_queue_number                             :  1,  
                      rx_reo_queue_desc_addr_39_32                            :  8;  
             uint32_t ignore_ampdu_flag                                       :  1,  
                      pn_handling_enable                                      :  1,  
                      pn_shall_be_uneven                                      :  1,  
                      pn_shall_be_even                                        :  1,  
                      pn_check_needed                                         :  1,  
                      oor_mode                                                :  1,  
                      chk_2k_mode                                             :  1,  
                      rty                                                     :  1,  
                      bar                                                     :  1,  
                      ac                                                      :  2,  
                      soft_reorder_enable                                     :  1,  
                      disable_duplicate_detection                             :  1,  
                      associated_link_descriptor_counter                      :  2,  
                      vld                                                     :  1,  
                      receive_queue_number                                    : 16;  
             uint32_t reserved_4a                                             :  3,  
                      flush_from_cache                                        :  1,  
                      pn_valid                                                :  1,  
                      pn_error_detected_flag                                  :  1,  
                      seq_2k_error_detected_flag                              :  1,  
                      ssn                                                     : 12,  
                      svld                                                    :  1,  
                      pn_size                                                 :  2,  
                      ba_window_size                                          : 10;  
             uint32_t pn_31_0                                                 : 32;  
             uint32_t pn_63_32                                                : 32;  
             uint32_t pn_95_64                                                : 32;  
             uint32_t pn_127_96                                               : 32;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 


 

#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_OFFSET                    0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_LSB                       0
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MSB                       15
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MASK                      0x000000000000ffff


 

#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET               0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_LSB                  16
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MSB                  16
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MASK                 0x0000000000010000


 

#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_OFFSET                       0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_LSB                          17
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_MSB                          31
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_MASK                         0x00000000fffe0000


 

#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                  0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                     32
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                     63
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                    0xffffffff00000000


 

#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                 0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                    0
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                    7
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                   0x00000000000000ff


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_OFFSET                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_LSB                     8
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_MSB                     8
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_MASK                    0x0000000000000100


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_LSB                                      9
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_MSB                                      9
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_MASK                                     0x0000000000000200


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB       10
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MSB       10
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK      0x0000000000000400


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_OFFSET           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_LSB              11
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_MSB              11
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_MASK             0x0000000000000800


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_OFFSET                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_LSB                      12
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_MSB                      12
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_MASK                     0x0000000000001000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_OFFSET                                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_LSB                                       13
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_MSB                                       13
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_MASK                                      0x0000000000002000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_LSB                                      14
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_MSB                                      14
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_MASK                                     0x0000000000004000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_LSB                                      15
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_MSB                                      15
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_MASK                                     0x0000000000008000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_OFFSET                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_LSB                              16
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_MSB                              16
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_MASK                             0x0000000000010000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_OFFSET                              0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_LSB                                 17
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_MSB                                 17
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_MASK                                0x0000000000020000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_OFFSET                        0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_LSB                           18
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_MSB                           18
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_MASK                          0x0000000000040000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_OFFSET                       0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_LSB                          19
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_MSB                          19
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_MASK                         0x0000000000080000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_OFFSET                      0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_LSB                         20
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_MSB                         20
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_MASK                        0x0000000000100000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_OFFSET                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_LSB                       21
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_MSB                       21
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_MASK                      0x0000000000200000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_OFFSET                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_LSB                       22
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_MSB                       22
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_MASK                      0x0000000000400000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_OFFSET                               0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_LSB                                  23
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_MSB                                  23
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_MASK                                 0x0000000000800000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_OFFSET                     0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_LSB                        24
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_MSB                        24
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_MASK                       0x0000000001000000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_OFFSET                                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_LSB                                     25
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_MSB                                     25
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_MASK                                    0x0000000002000000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_LSB                                      26
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_MSB                                      26
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_MASK                                     0x0000000004000000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET            0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_LSB               27
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_MSB               27
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_MASK              0x0000000008000000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_OFFSET                0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_LSB                   28
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_MSB                   28
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_MASK                  0x0000000010000000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_OFFSET                              0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_LSB                                 29
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_MSB                                 29
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_MASK                                0x0000000020000000


 

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_OFFSET                                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_LSB                                       30
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_MSB                                       30
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_MASK                                      0x0000000040000000


 

#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_OFFSET                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_LSB                             31
#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_MSB                             31
#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_MASK                            0x0000000080000000


 

#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_OFFSET                         0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_LSB                            32
#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_MSB                            47
#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_MASK                           0x0000ffff00000000


 

#define REO_UPDATE_RX_REO_QUEUE_VLD_OFFSET                                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_VLD_LSB                                             48
#define REO_UPDATE_RX_REO_QUEUE_VLD_MSB                                             48
#define REO_UPDATE_RX_REO_QUEUE_VLD_MASK                                            0x0001000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB              49
#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MSB              50
#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK             0x0006000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_OFFSET                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_LSB                     51
#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_MSB                     51
#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_MASK                    0x0008000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_OFFSET                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_LSB                             52
#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_MSB                             52
#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_MASK                            0x0010000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_AC_OFFSET                                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_AC_LSB                                              53
#define REO_UPDATE_RX_REO_QUEUE_AC_MSB                                              54
#define REO_UPDATE_RX_REO_QUEUE_AC_MASK                                             0x0060000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_BAR_OFFSET                                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_BAR_LSB                                             55
#define REO_UPDATE_RX_REO_QUEUE_BAR_MSB                                             55
#define REO_UPDATE_RX_REO_QUEUE_BAR_MASK                                            0x0080000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_RTY_OFFSET                                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_RTY_LSB                                             56
#define REO_UPDATE_RX_REO_QUEUE_RTY_MSB                                             56
#define REO_UPDATE_RX_REO_QUEUE_RTY_MASK                                            0x0100000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_OFFSET                                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_LSB                                     57
#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_MSB                                     57
#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_MASK                                    0x0200000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_OFFSET                                     0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_LSB                                        58
#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_MSB                                        58
#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_MASK                                       0x0400000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_OFFSET                              0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_LSB                                 59
#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_MSB                                 59
#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_MASK                                0x0800000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_OFFSET                             0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_LSB                                60
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_MSB                                60
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_MASK                               0x1000000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_OFFSET                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_LSB                              61
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_MSB                              61
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_MASK                             0x2000000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_OFFSET                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_LSB                              62
#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_MSB                              62
#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_MASK                             0x4000000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_OFFSET                            0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_LSB                               63
#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_MSB                               63
#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_MASK                              0x8000000000000000


 

#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_OFFSET                               0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_LSB                                  0
#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_MSB                                  9
#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_MASK                                 0x00000000000003ff


 

#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_OFFSET                                      0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_LSB                                         10
#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_MSB                                         11
#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_MASK                                        0x0000000000000c00


 

#define REO_UPDATE_RX_REO_QUEUE_SVLD_OFFSET                                         0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_SVLD_LSB                                            12
#define REO_UPDATE_RX_REO_QUEUE_SVLD_MSB                                            12
#define REO_UPDATE_RX_REO_QUEUE_SVLD_MASK                                           0x0000000000001000


 

#define REO_UPDATE_RX_REO_QUEUE_SSN_OFFSET                                          0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_SSN_LSB                                             13
#define REO_UPDATE_RX_REO_QUEUE_SSN_MSB                                             24
#define REO_UPDATE_RX_REO_QUEUE_SSN_MASK                                            0x0000000001ffe000


 

#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET                   0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_LSB                      25
#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_MSB                      25
#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_MASK                     0x0000000002000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_OFFSET                       0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_LSB                          26
#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_MSB                          26
#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_MASK                         0x0000000004000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_OFFSET                                     0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_LSB                                        27
#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_MSB                                        27
#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_MASK                                       0x0000000008000000


 

#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_OFFSET                             0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_LSB                                28
#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_MSB                                28
#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_MASK                               0x0000000010000000


 

#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_OFFSET                                  0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_LSB                                     29
#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_MSB                                     31
#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_MASK                                    0x00000000e0000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_OFFSET                                      0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_LSB                                         32
#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_MSB                                         63
#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_MASK                                        0xffffffff00000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_OFFSET                                     0x0000000000000018
#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_LSB                                        0
#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_MSB                                        31
#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_MASK                                       0x00000000ffffffff


 

#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_OFFSET                                     0x0000000000000018
#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_LSB                                        32
#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_MSB                                        63
#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_MASK                                       0xffffffff00000000


 

#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_OFFSET                                    0x0000000000000020
#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_LSB                                       0
#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_MSB                                       31
#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_MASK                                      0x00000000ffffffff


 

#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_OFFSET                                0x0000000000000020
#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_LSB                                   32
#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_MSB                                   63
#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_MASK                                  0xffffffff00000000



#endif    

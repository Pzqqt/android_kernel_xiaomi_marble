
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */



#ifndef _REO_UPDATE_RX_REO_QUEUE_H_
#define _REO_UPDATE_RX_REO_QUEUE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"

#define NUM_OF_DWORDS_REO_UPDATE_RX_REO_QUEUE 9

struct reo_update_rx_reo_queue {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0     : 32;
             uint32_t rx_reo_queue_desc_addr_39_32    :  8,
                      update_receive_queue_number     :  1,
                      update_vld                      :  1,
                      update_associated_link_descriptor_counter:  1,
                      update_disable_duplicate_detection:  1,
                      update_soft_reorder_enable      :  1,
                      update_ac                       :  1,
                      update_bar                      :  1,
                      update_rty                      :  1,
                      update_chk_2k_mode              :  1,
                      update_oor_mode                 :  1,
                      update_ba_window_size           :  1,
                      update_pn_check_needed          :  1,
                      update_pn_shall_be_even         :  1,
                      update_pn_shall_be_uneven       :  1,
                      update_pn_handling_enable       :  1,
                      update_pn_size                  :  1,
                      update_ignore_ampdu_flag        :  1,
                      update_svld                     :  1,
                      update_ssn                      :  1,
                      update_seq_2k_error_detected_flag:  1,
                      update_pn_error_detected_flag   :  1,
                      update_pn_valid                 :  1,
                      update_pn                       :  1,
                      clear_stat_counters             :  1;
             uint32_t receive_queue_number            : 16,
                      vld                             :  1,
                      associated_link_descriptor_counter:  2,
                      disable_duplicate_detection     :  1,
                      soft_reorder_enable             :  1,
                      ac                              :  2,
                      bar                             :  1,
                      rty                             :  1,
                      chk_2k_mode                     :  1,
                      oor_mode                        :  1,
                      pn_check_needed                 :  1,
                      pn_shall_be_even                :  1,
                      pn_shall_be_uneven              :  1,
                      pn_handling_enable              :  1,
                      ignore_ampdu_flag               :  1;
             uint32_t ba_window_size                  :  8,
                      pn_size                         :  2,
                      svld                            :  1,
                      ssn                             : 12,
                      seq_2k_error_detected_flag      :  1,
                      pn_error_detected_flag          :  1,
                      pn_valid                        :  1,
                      flush_from_cache                :  1,
                      reserved_4a                     :  5;
             uint32_t pn_31_0                         : 32;
             uint32_t pn_63_32                        : 32;
             uint32_t pn_95_64                        : 32;
             uint32_t pn_127_96                       : 32;
};

#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER_OFFSET   0x00000000
#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER_LSB      0
#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER_MASK     0x0000ffff

#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET 0x00000000
#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED_LSB 16
#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED_MASK 0x00010000

#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_RESERVED_0A_OFFSET      0x00000000
#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_RESERVED_0A_LSB         17
#define REO_UPDATE_RX_REO_QUEUE_0_CMD_HEADER_RESERVED_0A_MASK        0xfffe0000

#define REO_UPDATE_RX_REO_QUEUE_1_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET 0x00000004
#define REO_UPDATE_RX_REO_QUEUE_1_RX_REO_QUEUE_DESC_ADDR_31_0_LSB    0
#define REO_UPDATE_RX_REO_QUEUE_1_RX_REO_QUEUE_DESC_ADDR_31_0_MASK   0xffffffff

#define REO_UPDATE_RX_REO_QUEUE_2_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_RX_REO_QUEUE_DESC_ADDR_39_32_LSB   0
#define REO_UPDATE_RX_REO_QUEUE_2_RX_REO_QUEUE_DESC_ADDR_39_32_MASK  0x000000ff

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_RECEIVE_QUEUE_NUMBER_OFFSET 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_RECEIVE_QUEUE_NUMBER_LSB    8
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_RECEIVE_QUEUE_NUMBER_MASK   0x00000100

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_VLD_OFFSET                  0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_VLD_LSB                     9
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_VLD_MASK                    0x00000200

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB 10
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK 0x00000400

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_DISABLE_DUPLICATE_DETECTION_OFFSET 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_DISABLE_DUPLICATE_DETECTION_LSB 11
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_DISABLE_DUPLICATE_DETECTION_MASK 0x00000800

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SOFT_REORDER_ENABLE_OFFSET  0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SOFT_REORDER_ENABLE_LSB     12
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SOFT_REORDER_ENABLE_MASK    0x00001000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_AC_OFFSET                   0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_AC_LSB                      13
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_AC_MASK                     0x00002000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_BAR_OFFSET                  0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_BAR_LSB                     14
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_BAR_MASK                    0x00004000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_RTY_OFFSET                  0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_RTY_LSB                     15
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_RTY_MASK                    0x00008000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_CHK_2K_MODE_OFFSET          0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_CHK_2K_MODE_LSB             16
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_CHK_2K_MODE_MASK            0x00010000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_OOR_MODE_OFFSET             0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_OOR_MODE_LSB                17
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_OOR_MODE_MASK               0x00020000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_BA_WINDOW_SIZE_OFFSET       0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_BA_WINDOW_SIZE_LSB          18
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_BA_WINDOW_SIZE_MASK         0x00040000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_CHECK_NEEDED_OFFSET      0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_CHECK_NEEDED_LSB         19
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_CHECK_NEEDED_MASK        0x00080000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SHALL_BE_EVEN_OFFSET     0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SHALL_BE_EVEN_LSB        20
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SHALL_BE_EVEN_MASK       0x00100000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SHALL_BE_UNEVEN_OFFSET   0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SHALL_BE_UNEVEN_LSB      21
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SHALL_BE_UNEVEN_MASK     0x00200000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_HANDLING_ENABLE_OFFSET   0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_HANDLING_ENABLE_LSB      22
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_HANDLING_ENABLE_MASK     0x00400000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SIZE_OFFSET              0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SIZE_LSB                 23
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_SIZE_MASK                0x00800000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_IGNORE_AMPDU_FLAG_OFFSET    0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_IGNORE_AMPDU_FLAG_LSB       24
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_IGNORE_AMPDU_FLAG_MASK      0x01000000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SVLD_OFFSET                 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SVLD_LSB                    25
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SVLD_MASK                   0x02000000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SSN_OFFSET                  0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SSN_LSB                     26
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SSN_MASK                    0x04000000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_LSB 27
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_MASK 0x08000000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_ERROR_DETECTED_FLAG_OFFSET 0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_ERROR_DETECTED_FLAG_LSB  28
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_ERROR_DETECTED_FLAG_MASK 0x10000000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_VALID_OFFSET             0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_VALID_LSB                29
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_VALID_MASK               0x20000000

#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_OFFSET                   0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_LSB                      30
#define REO_UPDATE_RX_REO_QUEUE_2_UPDATE_PN_MASK                     0x40000000

#define REO_UPDATE_RX_REO_QUEUE_2_CLEAR_STAT_COUNTERS_OFFSET         0x00000008
#define REO_UPDATE_RX_REO_QUEUE_2_CLEAR_STAT_COUNTERS_LSB            31
#define REO_UPDATE_RX_REO_QUEUE_2_CLEAR_STAT_COUNTERS_MASK           0x80000000

#define REO_UPDATE_RX_REO_QUEUE_3_RECEIVE_QUEUE_NUMBER_OFFSET        0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_RECEIVE_QUEUE_NUMBER_LSB           0
#define REO_UPDATE_RX_REO_QUEUE_3_RECEIVE_QUEUE_NUMBER_MASK          0x0000ffff

#define REO_UPDATE_RX_REO_QUEUE_3_VLD_OFFSET                         0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_VLD_LSB                            16
#define REO_UPDATE_RX_REO_QUEUE_3_VLD_MASK                           0x00010000

#define REO_UPDATE_RX_REO_QUEUE_3_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET 0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB 17
#define REO_UPDATE_RX_REO_QUEUE_3_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK 0x00060000

#define REO_UPDATE_RX_REO_QUEUE_3_DISABLE_DUPLICATE_DETECTION_OFFSET 0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_DISABLE_DUPLICATE_DETECTION_LSB    19
#define REO_UPDATE_RX_REO_QUEUE_3_DISABLE_DUPLICATE_DETECTION_MASK   0x00080000

#define REO_UPDATE_RX_REO_QUEUE_3_SOFT_REORDER_ENABLE_OFFSET         0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_SOFT_REORDER_ENABLE_LSB            20
#define REO_UPDATE_RX_REO_QUEUE_3_SOFT_REORDER_ENABLE_MASK           0x00100000

#define REO_UPDATE_RX_REO_QUEUE_3_AC_OFFSET                          0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_AC_LSB                             21
#define REO_UPDATE_RX_REO_QUEUE_3_AC_MASK                            0x00600000

#define REO_UPDATE_RX_REO_QUEUE_3_BAR_OFFSET                         0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_BAR_LSB                            23
#define REO_UPDATE_RX_REO_QUEUE_3_BAR_MASK                           0x00800000

#define REO_UPDATE_RX_REO_QUEUE_3_RTY_OFFSET                         0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_RTY_LSB                            24
#define REO_UPDATE_RX_REO_QUEUE_3_RTY_MASK                           0x01000000

#define REO_UPDATE_RX_REO_QUEUE_3_CHK_2K_MODE_OFFSET                 0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_CHK_2K_MODE_LSB                    25
#define REO_UPDATE_RX_REO_QUEUE_3_CHK_2K_MODE_MASK                   0x02000000

#define REO_UPDATE_RX_REO_QUEUE_3_OOR_MODE_OFFSET                    0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_OOR_MODE_LSB                       26
#define REO_UPDATE_RX_REO_QUEUE_3_OOR_MODE_MASK                      0x04000000

#define REO_UPDATE_RX_REO_QUEUE_3_PN_CHECK_NEEDED_OFFSET             0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_PN_CHECK_NEEDED_LSB                27
#define REO_UPDATE_RX_REO_QUEUE_3_PN_CHECK_NEEDED_MASK               0x08000000

#define REO_UPDATE_RX_REO_QUEUE_3_PN_SHALL_BE_EVEN_OFFSET            0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_PN_SHALL_BE_EVEN_LSB               28
#define REO_UPDATE_RX_REO_QUEUE_3_PN_SHALL_BE_EVEN_MASK              0x10000000

#define REO_UPDATE_RX_REO_QUEUE_3_PN_SHALL_BE_UNEVEN_OFFSET          0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_PN_SHALL_BE_UNEVEN_LSB             29
#define REO_UPDATE_RX_REO_QUEUE_3_PN_SHALL_BE_UNEVEN_MASK            0x20000000

#define REO_UPDATE_RX_REO_QUEUE_3_PN_HANDLING_ENABLE_OFFSET          0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_PN_HANDLING_ENABLE_LSB             30
#define REO_UPDATE_RX_REO_QUEUE_3_PN_HANDLING_ENABLE_MASK            0x40000000

#define REO_UPDATE_RX_REO_QUEUE_3_IGNORE_AMPDU_FLAG_OFFSET           0x0000000c
#define REO_UPDATE_RX_REO_QUEUE_3_IGNORE_AMPDU_FLAG_LSB              31
#define REO_UPDATE_RX_REO_QUEUE_3_IGNORE_AMPDU_FLAG_MASK             0x80000000

#define REO_UPDATE_RX_REO_QUEUE_4_BA_WINDOW_SIZE_OFFSET              0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_BA_WINDOW_SIZE_LSB                 0
#define REO_UPDATE_RX_REO_QUEUE_4_BA_WINDOW_SIZE_MASK                0x000000ff

#define REO_UPDATE_RX_REO_QUEUE_4_PN_SIZE_OFFSET                     0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_PN_SIZE_LSB                        8
#define REO_UPDATE_RX_REO_QUEUE_4_PN_SIZE_MASK                       0x00000300

#define REO_UPDATE_RX_REO_QUEUE_4_SVLD_OFFSET                        0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_SVLD_LSB                           10
#define REO_UPDATE_RX_REO_QUEUE_4_SVLD_MASK                          0x00000400

#define REO_UPDATE_RX_REO_QUEUE_4_SSN_OFFSET                         0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_SSN_LSB                            11
#define REO_UPDATE_RX_REO_QUEUE_4_SSN_MASK                           0x007ff800

#define REO_UPDATE_RX_REO_QUEUE_4_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET  0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_SEQ_2K_ERROR_DETECTED_FLAG_LSB     23
#define REO_UPDATE_RX_REO_QUEUE_4_SEQ_2K_ERROR_DETECTED_FLAG_MASK    0x00800000

#define REO_UPDATE_RX_REO_QUEUE_4_PN_ERROR_DETECTED_FLAG_OFFSET      0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_PN_ERROR_DETECTED_FLAG_LSB         24
#define REO_UPDATE_RX_REO_QUEUE_4_PN_ERROR_DETECTED_FLAG_MASK        0x01000000

#define REO_UPDATE_RX_REO_QUEUE_4_PN_VALID_OFFSET                    0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_PN_VALID_LSB                       25
#define REO_UPDATE_RX_REO_QUEUE_4_PN_VALID_MASK                      0x02000000

#define REO_UPDATE_RX_REO_QUEUE_4_FLUSH_FROM_CACHE_OFFSET            0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_FLUSH_FROM_CACHE_LSB               26
#define REO_UPDATE_RX_REO_QUEUE_4_FLUSH_FROM_CACHE_MASK              0x04000000

#define REO_UPDATE_RX_REO_QUEUE_4_RESERVED_4A_OFFSET                 0x00000010
#define REO_UPDATE_RX_REO_QUEUE_4_RESERVED_4A_LSB                    27
#define REO_UPDATE_RX_REO_QUEUE_4_RESERVED_4A_MASK                   0xf8000000

#define REO_UPDATE_RX_REO_QUEUE_5_PN_31_0_OFFSET                     0x00000014
#define REO_UPDATE_RX_REO_QUEUE_5_PN_31_0_LSB                        0
#define REO_UPDATE_RX_REO_QUEUE_5_PN_31_0_MASK                       0xffffffff

#define REO_UPDATE_RX_REO_QUEUE_6_PN_63_32_OFFSET                    0x00000018
#define REO_UPDATE_RX_REO_QUEUE_6_PN_63_32_LSB                       0
#define REO_UPDATE_RX_REO_QUEUE_6_PN_63_32_MASK                      0xffffffff

#define REO_UPDATE_RX_REO_QUEUE_7_PN_95_64_OFFSET                    0x0000001c
#define REO_UPDATE_RX_REO_QUEUE_7_PN_95_64_LSB                       0
#define REO_UPDATE_RX_REO_QUEUE_7_PN_95_64_MASK                      0xffffffff

#define REO_UPDATE_RX_REO_QUEUE_8_PN_127_96_OFFSET                   0x00000020
#define REO_UPDATE_RX_REO_QUEUE_8_PN_127_96_LSB                      0
#define REO_UPDATE_RX_REO_QUEUE_8_PN_127_96_MASK                     0xffffffff

#endif

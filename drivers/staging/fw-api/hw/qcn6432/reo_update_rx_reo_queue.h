/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t rx_reo_queue_desc_addr_39_32                            :  8, // [7:0]
                      update_receive_queue_number                             :  1, // [8:8]
                      update_vld                                              :  1, // [9:9]
                      update_associated_link_descriptor_counter               :  1, // [10:10]
                      update_disable_duplicate_detection                      :  1, // [11:11]
                      update_soft_reorder_enable                              :  1, // [12:12]
                      update_ac                                               :  1, // [13:13]
                      update_bar                                              :  1, // [14:14]
                      update_rty                                              :  1, // [15:15]
                      update_chk_2k_mode                                      :  1, // [16:16]
                      update_oor_mode                                         :  1, // [17:17]
                      update_ba_window_size                                   :  1, // [18:18]
                      update_pn_check_needed                                  :  1, // [19:19]
                      update_pn_shall_be_even                                 :  1, // [20:20]
                      update_pn_shall_be_uneven                               :  1, // [21:21]
                      update_pn_handling_enable                               :  1, // [22:22]
                      update_pn_size                                          :  1, // [23:23]
                      update_ignore_ampdu_flag                                :  1, // [24:24]
                      update_svld                                             :  1, // [25:25]
                      update_ssn                                              :  1, // [26:26]
                      update_seq_2k_error_detected_flag                       :  1, // [27:27]
                      update_pn_error_detected_flag                           :  1, // [28:28]
                      update_pn_valid                                         :  1, // [29:29]
                      update_pn                                               :  1, // [30:30]
                      clear_stat_counters                                     :  1; // [31:31]
             uint32_t receive_queue_number                                    : 16, // [15:0]
                      vld                                                     :  1, // [16:16]
                      associated_link_descriptor_counter                      :  2, // [18:17]
                      disable_duplicate_detection                             :  1, // [19:19]
                      soft_reorder_enable                                     :  1, // [20:20]
                      ac                                                      :  2, // [22:21]
                      bar                                                     :  1, // [23:23]
                      rty                                                     :  1, // [24:24]
                      chk_2k_mode                                             :  1, // [25:25]
                      oor_mode                                                :  1, // [26:26]
                      pn_check_needed                                         :  1, // [27:27]
                      pn_shall_be_even                                        :  1, // [28:28]
                      pn_shall_be_uneven                                      :  1, // [29:29]
                      pn_handling_enable                                      :  1, // [30:30]
                      ignore_ampdu_flag                                       :  1; // [31:31]
             uint32_t ba_window_size                                          : 10, // [9:0]
                      pn_size                                                 :  2, // [11:10]
                      svld                                                    :  1, // [12:12]
                      ssn                                                     : 12, // [24:13]
                      seq_2k_error_detected_flag                              :  1, // [25:25]
                      pn_error_detected_flag                                  :  1, // [26:26]
                      pn_valid                                                :  1, // [27:27]
                      flush_from_cache                                        :  1, // [28:28]
                      reserved_4a                                             :  3; // [31:29]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   uniform_reo_cmd_header                                    cmd_header;
             uint32_t rx_reo_queue_desc_addr_31_0                             : 32; // [31:0]
             uint32_t clear_stat_counters                                     :  1, // [31:31]
                      update_pn                                               :  1, // [30:30]
                      update_pn_valid                                         :  1, // [29:29]
                      update_pn_error_detected_flag                           :  1, // [28:28]
                      update_seq_2k_error_detected_flag                       :  1, // [27:27]
                      update_ssn                                              :  1, // [26:26]
                      update_svld                                             :  1, // [25:25]
                      update_ignore_ampdu_flag                                :  1, // [24:24]
                      update_pn_size                                          :  1, // [23:23]
                      update_pn_handling_enable                               :  1, // [22:22]
                      update_pn_shall_be_uneven                               :  1, // [21:21]
                      update_pn_shall_be_even                                 :  1, // [20:20]
                      update_pn_check_needed                                  :  1, // [19:19]
                      update_ba_window_size                                   :  1, // [18:18]
                      update_oor_mode                                         :  1, // [17:17]
                      update_chk_2k_mode                                      :  1, // [16:16]
                      update_rty                                              :  1, // [15:15]
                      update_bar                                              :  1, // [14:14]
                      update_ac                                               :  1, // [13:13]
                      update_soft_reorder_enable                              :  1, // [12:12]
                      update_disable_duplicate_detection                      :  1, // [11:11]
                      update_associated_link_descriptor_counter               :  1, // [10:10]
                      update_vld                                              :  1, // [9:9]
                      update_receive_queue_number                             :  1, // [8:8]
                      rx_reo_queue_desc_addr_39_32                            :  8; // [7:0]
             uint32_t ignore_ampdu_flag                                       :  1, // [31:31]
                      pn_handling_enable                                      :  1, // [30:30]
                      pn_shall_be_uneven                                      :  1, // [29:29]
                      pn_shall_be_even                                        :  1, // [28:28]
                      pn_check_needed                                         :  1, // [27:27]
                      oor_mode                                                :  1, // [26:26]
                      chk_2k_mode                                             :  1, // [25:25]
                      rty                                                     :  1, // [24:24]
                      bar                                                     :  1, // [23:23]
                      ac                                                      :  2, // [22:21]
                      soft_reorder_enable                                     :  1, // [20:20]
                      disable_duplicate_detection                             :  1, // [19:19]
                      associated_link_descriptor_counter                      :  2, // [18:17]
                      vld                                                     :  1, // [16:16]
                      receive_queue_number                                    : 16; // [15:0]
             uint32_t reserved_4a                                             :  3, // [31:29]
                      flush_from_cache                                        :  1, // [28:28]
                      pn_valid                                                :  1, // [27:27]
                      pn_error_detected_flag                                  :  1, // [26:26]
                      seq_2k_error_detected_flag                              :  1, // [25:25]
                      ssn                                                     : 12, // [24:13]
                      svld                                                    :  1, // [12:12]
                      pn_size                                                 :  2, // [11:10]
                      ba_window_size                                          : 10; // [9:0]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		CMD_HEADER

			Consumer: REO
			Producer: SW
			
			Details for command execution tracking purposes.
*/


/* Description		REO_CMD_NUMBER

			Consumer: REO/SW/DEBUG
			Producer: SW 
			
			This number can be used by SW to track, identify and link
			 the created commands with the command statusses
			
			
			<legal all> 
*/

#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_OFFSET                    0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_LSB                       0
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MSB                       15
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_CMD_NUMBER_MASK                      0x000000000000ffff


/* Description		REO_STATUS_REQUIRED

			Consumer: REO
			Producer: SW 
			
			<enum 0 NoStatus> REO does not need to generate a status
			 TLV for the execution of this command
			<enum 1 StatusRequired> REO shall generate a status TLV 
			for the execution of this command
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET               0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_LSB                  16
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MSB                  16
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_REO_STATUS_REQUIRED_MASK                 0x0000000000010000


/* Description		RESERVED_0A

			<legal 0>
*/

#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_OFFSET                       0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_LSB                          17
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_MSB                          31
#define REO_UPDATE_RX_REO_QUEUE_CMD_HEADER_RESERVED_0A_MASK                         0x00000000fffe0000


/* Description		RX_REO_QUEUE_DESC_ADDR_31_0

			Consumer: REO
			Producer: SW
			
			Address (lower 32 bits) of the REO queue descriptor
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_OFFSET                  0x0000000000000000
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_LSB                     32
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_MSB                     63
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_31_0_MASK                    0xffffffff00000000


/* Description		RX_REO_QUEUE_DESC_ADDR_39_32

			Consumer: REO
			Producer: SW
			
			Address (upper 8 bits) of the REO queue descriptor
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_OFFSET                 0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_LSB                    0
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_MSB                    7
#define REO_UPDATE_RX_REO_QUEUE_RX_REO_QUEUE_DESC_ADDR_39_32_MASK                   0x00000000000000ff


/* Description		UPDATE_RECEIVE_QUEUE_NUMBER

			Consumer: REO
			Producer: SW
			When set, receive_queue_number from this command will be
			 updated in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_OFFSET                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_LSB                     8
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_MSB                     8
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RECEIVE_QUEUE_NUMBER_MASK                    0x0000000000000100


/* Description		UPDATE_VLD

			Consumer: REO
			Producer: SW
			
			When clear, REO will NOT update the VLD bit setting. For
			 this setting, SW MUST set the Flush_from_cache bit in this
			 command.
			
			When set, VLD from this command will be updated in the descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_LSB                                      9
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_MSB                                      9
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_VLD_MASK                                     0x0000000000000200


/* Description		UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER

			Consumer: REO
			Producer: SW
			When set, Associated_link_descriptor_counter from this command
			 will be updated in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB       10
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MSB       10
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK      0x0000000000000400


/* Description		UPDATE_DISABLE_DUPLICATE_DETECTION

			Consumer: REO
			Producer: SW
			When set, Disable_duplicate_detection from this command 
			will be updated in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_OFFSET           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_LSB              11
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_MSB              11
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_DISABLE_DUPLICATE_DETECTION_MASK             0x0000000000000800


/* Description		UPDATE_SOFT_REORDER_ENABLE

			Consumer: REO
			Producer: SW
			When set, Soft_reorder_enable from this command will be 
			updated in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_OFFSET                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_LSB                      12
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_MSB                      12
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SOFT_REORDER_ENABLE_MASK                     0x0000000000001000


/* Description		UPDATE_AC

			Consumer: REO
			Producer: SW
			When set, AC from this command will be updated in the descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_OFFSET                                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_LSB                                       13
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_MSB                                       13
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_AC_MASK                                      0x0000000000002000


/* Description		UPDATE_BAR

			Consumer: REO
			Producer: SW
			When set, BAR from this command will be updated in the descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_LSB                                      14
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_MSB                                      14
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BAR_MASK                                     0x0000000000004000


/* Description		UPDATE_RTY

			Consumer: REO
			Producer: SW
			When set, RTY from this command will be updated in the descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_LSB                                      15
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_MSB                                      15
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_RTY_MASK                                     0x0000000000008000


/* Description		UPDATE_CHK_2K_MODE

			Consumer: REO
			Producer: SW
			When set, Chk_2k_mode from this command will be updated 
			in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_OFFSET                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_LSB                              16
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_MSB                              16
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_CHK_2K_MODE_MASK                             0x0000000000010000


/* Description		UPDATE_OOR_MODE

			Consumer: REO
			Producer: SW
			When set, OOR_Mode from this command will be updated in 
			the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_OFFSET                              0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_LSB                                 17
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_MSB                                 17
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_OOR_MODE_MASK                                0x0000000000020000


/* Description		UPDATE_BA_WINDOW_SIZE

			Consumer: REO
			Producer: SW
			When set, BA_window_size from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_OFFSET                        0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_LSB                           18
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_MSB                           18
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_BA_WINDOW_SIZE_MASK                          0x0000000000040000


/* Description		UPDATE_PN_CHECK_NEEDED

			Consumer: REO
			Producer: SW
			When set, Pn_check_needed from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_OFFSET                       0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_LSB                          19
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_MSB                          19
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_CHECK_NEEDED_MASK                         0x0000000000080000


/* Description		UPDATE_PN_SHALL_BE_EVEN

			Consumer: REO
			Producer: SW
			When set, Pn_shall_be_even from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_OFFSET                      0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_LSB                         20
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_MSB                         20
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_EVEN_MASK                        0x0000000000100000


/* Description		UPDATE_PN_SHALL_BE_UNEVEN

			Consumer: REO
			Producer: SW
			When set, Pn_shall_be_uneven from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_OFFSET                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_LSB                       21
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_MSB                       21
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SHALL_BE_UNEVEN_MASK                      0x0000000000200000


/* Description		UPDATE_PN_HANDLING_ENABLE

			Consumer: REO
			Producer: SW
			When set, Pn_handling_enable from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_OFFSET                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_LSB                       22
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_MSB                       22
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_HANDLING_ENABLE_MASK                      0x0000000000400000


/* Description		UPDATE_PN_SIZE

			Consumer: REO
			Producer: SW
			When set, Pn_size from this command will be updated in the
			 descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_OFFSET                               0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_LSB                                  23
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_MSB                                  23
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_SIZE_MASK                                 0x0000000000800000


/* Description		UPDATE_IGNORE_AMPDU_FLAG

			Consumer: REO
			Producer: SW
			When set, Ignore_ampdu_flag from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_OFFSET                     0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_LSB                        24
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_MSB                        24
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_IGNORE_AMPDU_FLAG_MASK                       0x0000000001000000


/* Description		UPDATE_SVLD

			Consumer: REO
			Producer: SW
			When set, Svld from this command will be updated in the 
			descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_OFFSET                                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_LSB                                     25
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_MSB                                     25
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SVLD_MASK                                    0x0000000002000000


/* Description		UPDATE_SSN

			Consumer: REO
			Producer: SW
			When set, SSN from this command will be updated in the descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_OFFSET                                   0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_LSB                                      26
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_MSB                                      26
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SSN_MASK                                     0x0000000004000000


/* Description		UPDATE_SEQ_2K_ERROR_DETECTED_FLAG

			Consumer: REO
			Producer: SW
			When set, Seq_2k_error_detected_flag from this command will
			 be updated in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET            0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_LSB               27
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_MSB               27
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_SEQ_2K_ERROR_DETECTED_FLAG_MASK              0x0000000008000000


/* Description		UPDATE_PN_ERROR_DETECTED_FLAG

			Consumer: REO
			Producer: SW
			When set, pn_error_detected_flag from this command will 
			be updated in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_OFFSET                0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_LSB                   28
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_MSB                   28
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_ERROR_DETECTED_FLAG_MASK                  0x0000000010000000


/* Description		UPDATE_PN_VALID

			Consumer: REO
			Producer: SW
			When set, pn_valid from this command will be updated in 
			the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_OFFSET                              0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_LSB                                 29
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_MSB                                 29
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_VALID_MASK                                0x0000000020000000


/* Description		UPDATE_PN

			Consumer: REO
			Producer: SW
			When set, all pn_... fields from this command will be updated
			 in the descriptor.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_OFFSET                                    0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_LSB                                       30
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_MSB                                       30
#define REO_UPDATE_RX_REO_QUEUE_UPDATE_PN_MASK                                      0x0000000040000000


/* Description		CLEAR_STAT_COUNTERS

			Consumer: REO
			Producer: SW
			When set, REO will clear (=> set to 0) the following stat
			 counters in the REO_QUEUE_STRUCT
			
			Last_rx_enqueue_TimeStamp
			Last_rx_dequeue_Timestamp
			Rx_bitmap (not a counter, but bitmap is cleared)
			Timeout_count
			Forward_due_to_bar_count
			Duplicate_count
			Frames_in_order_count
			BAR_received_count
			MPDU_Frames_processed_count
			MSDU_Frames_processed_count
			Total_processed_byte_count
			Late_receive_MPDU_count
			window_jump_2k
			Hole_count
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_OFFSET                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_LSB                             31
#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_MSB                             31
#define REO_UPDATE_RX_REO_QUEUE_CLEAR_STAT_COUNTERS_MASK                            0x0000000080000000


/* Description		RECEIVE_QUEUE_NUMBER

			Field only valid when Update_receive_queue_number is set
			
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_OFFSET                         0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_LSB                            32
#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_MSB                            47
#define REO_UPDATE_RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_MASK                           0x0000ffff00000000


/* Description		VLD

			Field only valid when Update_VLD is set
			
			For Update_VLD set and VLD clear, SW MUST set the Flush_from_cache
			 bit in this command.
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_VLD_OFFSET                                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_VLD_LSB                                             48
#define REO_UPDATE_RX_REO_QUEUE_VLD_MSB                                             48
#define REO_UPDATE_RX_REO_QUEUE_VLD_MASK                                            0x0001000000000000


/* Description		ASSOCIATED_LINK_DESCRIPTOR_COUNTER

			Field only valid when Update_Associated_link_descriptor_counter
			 is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB              49
#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MSB              50
#define REO_UPDATE_RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK             0x0006000000000000


/* Description		DISABLE_DUPLICATE_DETECTION

			Field only valid when Update_Disable_duplicate_detection
			 is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_OFFSET                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_LSB                     51
#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_MSB                     51
#define REO_UPDATE_RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_MASK                    0x0008000000000000


/* Description		SOFT_REORDER_ENABLE

			Field only valid when Update_Soft_reorder_enable is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_OFFSET                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_LSB                             52
#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_MSB                             52
#define REO_UPDATE_RX_REO_QUEUE_SOFT_REORDER_ENABLE_MASK                            0x0010000000000000


/* Description		AC

			Field only valid when Update_AC is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_AC_OFFSET                                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_AC_LSB                                              53
#define REO_UPDATE_RX_REO_QUEUE_AC_MSB                                              54
#define REO_UPDATE_RX_REO_QUEUE_AC_MASK                                             0x0060000000000000


/* Description		BAR

			Field only valid when Update_BAR is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_BAR_OFFSET                                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_BAR_LSB                                             55
#define REO_UPDATE_RX_REO_QUEUE_BAR_MSB                                             55
#define REO_UPDATE_RX_REO_QUEUE_BAR_MASK                                            0x0080000000000000


/* Description		RTY

			Field only valid when Update_RTY is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_RTY_OFFSET                                          0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_RTY_LSB                                             56
#define REO_UPDATE_RX_REO_QUEUE_RTY_MSB                                             56
#define REO_UPDATE_RX_REO_QUEUE_RTY_MASK                                            0x0100000000000000


/* Description		CHK_2K_MODE

			Field only valid when Update_Chk_2k_Mode is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_OFFSET                                  0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_LSB                                     57
#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_MSB                                     57
#define REO_UPDATE_RX_REO_QUEUE_CHK_2K_MODE_MASK                                    0x0200000000000000


/* Description		OOR_MODE

			Field only valid when Update_OOR_Mode is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_OFFSET                                     0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_LSB                                        58
#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_MSB                                        58
#define REO_UPDATE_RX_REO_QUEUE_OOR_MODE_MASK                                       0x0400000000000000


/* Description		PN_CHECK_NEEDED

			Field only valid when Update_Pn_check_needed is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_OFFSET                              0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_LSB                                 59
#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_MSB                                 59
#define REO_UPDATE_RX_REO_QUEUE_PN_CHECK_NEEDED_MASK                                0x0800000000000000


/* Description		PN_SHALL_BE_EVEN

			Field only valid when Update_Pn_shall_be_even is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_OFFSET                             0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_LSB                                60
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_MSB                                60
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_EVEN_MASK                               0x1000000000000000


/* Description		PN_SHALL_BE_UNEVEN

			Field only valid when Update_Pn_shall_be_uneven is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_OFFSET                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_LSB                              61
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_MSB                              61
#define REO_UPDATE_RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_MASK                             0x2000000000000000


/* Description		PN_HANDLING_ENABLE

			Field only valid when Update_Pn_handling_enable is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_OFFSET                           0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_LSB                              62
#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_MSB                              62
#define REO_UPDATE_RX_REO_QUEUE_PN_HANDLING_ENABLE_MASK                             0x4000000000000000


/* Description		IGNORE_AMPDU_FLAG

			Field only valid when Update_Ignore_ampdu_flag is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_OFFSET                            0x0000000000000008
#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_LSB                               63
#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_MSB                               63
#define REO_UPDATE_RX_REO_QUEUE_IGNORE_AMPDU_FLAG_MASK                              0x8000000000000000


/* Description		BA_WINDOW_SIZE

			Field only valid when Update_BA_window_size is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_OFFSET                               0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_LSB                                  0
#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_MSB                                  9
#define REO_UPDATE_RX_REO_QUEUE_BA_WINDOW_SIZE_MASK                                 0x00000000000003ff


/* Description		PN_SIZE

			Field only valid when Update_Pn_size is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			
			<enum 0     pn_size_24>
			<enum 1     pn_size_48>
			<enum 2     pn_size_128>
			
			<legal 0-2>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_OFFSET                                      0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_LSB                                         10
#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_MSB                                         11
#define REO_UPDATE_RX_REO_QUEUE_PN_SIZE_MASK                                        0x0000000000000c00


/* Description		SVLD

			Field only valid when Update_Svld is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_SVLD_OFFSET                                         0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_SVLD_LSB                                            12
#define REO_UPDATE_RX_REO_QUEUE_SVLD_MSB                                            12
#define REO_UPDATE_RX_REO_QUEUE_SVLD_MASK                                           0x0000000000001000


/* Description		SSN

			Field only valid when Update_SSN is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_SSN_OFFSET                                          0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_SSN_LSB                                             13
#define REO_UPDATE_RX_REO_QUEUE_SSN_MSB                                             24
#define REO_UPDATE_RX_REO_QUEUE_SSN_MASK                                            0x0000000001ffe000


/* Description		SEQ_2K_ERROR_DETECTED_FLAG

			Field only valid when Update_Seq_2k_error_detected_flag 
			is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET                   0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_LSB                      25
#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_MSB                      25
#define REO_UPDATE_RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_MASK                     0x0000000002000000


/* Description		PN_ERROR_DETECTED_FLAG

			Field only valid when Update_pn_error_detected_flag is set
			
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_OFFSET                       0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_LSB                          26
#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_MSB                          26
#define REO_UPDATE_RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_MASK                         0x0000000004000000


/* Description		PN_VALID

			Field only valid when Update_pn_valid is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_OFFSET                                     0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_LSB                                        27
#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_MSB                                        27
#define REO_UPDATE_RX_REO_QUEUE_PN_VALID_MASK                                       0x0000000008000000


/* Description		FLUSH_FROM_CACHE

			When set, REO shall, after finishing the execution of this
			 command, flush the related descriptor from the cache.
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_OFFSET                             0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_LSB                                28
#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_MSB                                28
#define REO_UPDATE_RX_REO_QUEUE_FLUSH_FROM_CACHE_MASK                               0x0000000010000000


/* Description		RESERVED_4A

			<legal 0>
*/

#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_OFFSET                                  0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_LSB                                     29
#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_MSB                                     31
#define REO_UPDATE_RX_REO_QUEUE_RESERVED_4A_MASK                                    0x00000000e0000000


/* Description		PN_31_0

			Field only valid when Update_Pn is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_OFFSET                                      0x0000000000000010
#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_LSB                                         32
#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_MSB                                         63
#define REO_UPDATE_RX_REO_QUEUE_PN_31_0_MASK                                        0xffffffff00000000


/* Description		PN_63_32

			Field only valid when Update_pn is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_OFFSET                                     0x0000000000000018
#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_LSB                                        0
#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_MSB                                        31
#define REO_UPDATE_RX_REO_QUEUE_PN_63_32_MASK                                       0x00000000ffffffff


/* Description		PN_95_64

			Field only valid when Update_pn is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_OFFSET                                     0x0000000000000018
#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_LSB                                        32
#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_MSB                                        63
#define REO_UPDATE_RX_REO_QUEUE_PN_95_64_MASK                                       0xffffffff00000000


/* Description		PN_127_96

			Field only valid when Update_pn is set
			
			Field value to be copied over into the RX_REO_QUEUE descriptor.
			
			<legal all>
*/

#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_OFFSET                                    0x0000000000000020
#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_LSB                                       0
#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_MSB                                       31
#define REO_UPDATE_RX_REO_QUEUE_PN_127_96_MASK                                      0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_OFFSET                                0x0000000000000020
#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_LSB                                   32
#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_MSB                                   63
#define REO_UPDATE_RX_REO_QUEUE_TLV64_PADDING_MASK                                  0xffffffff00000000



#endif   // REO_UPDATE_RX_REO_QUEUE


/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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











#ifndef _WBM2SW_COMPLETION_RING_TX_H_
#define _WBM2SW_COMPLETION_RING_TX_H_
#if !defined(__ASSEMBLER__)
#endif

#include "tx_rate_stats_info.h"
#define NUM_OF_DWORDS_WBM2SW_COMPLETION_RING_TX 8


struct wbm2sw_completion_ring_tx {
	     uint32_t buffer_virt_addr_31_0                                   : 32;
	     uint32_t buffer_virt_addr_63_32                                  : 32;
	     uint32_t release_source_module                                   :  3,
		      reserved_2a                                             :  3,
		      buffer_or_desc_type                                     :  3,
		      return_buffer_manager                                   :  4,
		      tqm_release_reason                                      :  4,
		      rbm_override_valid                                      :  1,
		      sw_buffer_cookie_11_0                                   : 12,
		      reserved_2b                                             :  1,
		      wbm_internal_error                                      :  1;
	     uint32_t tqm_status_number                                       : 24,
		      transmit_count                                          :  7,
		      sw_release_details_valid                                :  1;
	     uint32_t ack_frame_rssi                                          :  8,
		      first_msdu                                              :  1,
		      last_msdu                                               :  1,
		      fw_tx_notify_frame                                      :  3,
		      buffer_timestamp                                        : 19;
	     struct   tx_rate_stats_info                                        tx_rate_stats;
	     uint32_t sw_peer_id                                              : 16,
		      tid                                                     :  4,
		      sw_buffer_cookie_19_12                                  :  8,
		      looping_count                                           :  4;
};




#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_31_0_OFFSET                      0x00000000
#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_31_0_LSB                         0
#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_31_0_MSB                         31
#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_31_0_MASK                        0xffffffff




#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_63_32_OFFSET                     0x00000004
#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_63_32_LSB                        0
#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_63_32_MSB                        31
#define WBM2SW_COMPLETION_RING_TX_BUFFER_VIRT_ADDR_63_32_MASK                       0xffffffff




#define WBM2SW_COMPLETION_RING_TX_RELEASE_SOURCE_MODULE_OFFSET                      0x00000008
#define WBM2SW_COMPLETION_RING_TX_RELEASE_SOURCE_MODULE_LSB                         0
#define WBM2SW_COMPLETION_RING_TX_RELEASE_SOURCE_MODULE_MSB                         2
#define WBM2SW_COMPLETION_RING_TX_RELEASE_SOURCE_MODULE_MASK                        0x00000007




#define WBM2SW_COMPLETION_RING_TX_RESERVED_2A_OFFSET                                0x00000008
#define WBM2SW_COMPLETION_RING_TX_RESERVED_2A_LSB                                   3
#define WBM2SW_COMPLETION_RING_TX_RESERVED_2A_MSB                                   5
#define WBM2SW_COMPLETION_RING_TX_RESERVED_2A_MASK                                  0x00000038




#define WBM2SW_COMPLETION_RING_TX_BUFFER_OR_DESC_TYPE_OFFSET                        0x00000008
#define WBM2SW_COMPLETION_RING_TX_BUFFER_OR_DESC_TYPE_LSB                           6
#define WBM2SW_COMPLETION_RING_TX_BUFFER_OR_DESC_TYPE_MSB                           8
#define WBM2SW_COMPLETION_RING_TX_BUFFER_OR_DESC_TYPE_MASK                          0x000001c0




#define WBM2SW_COMPLETION_RING_TX_RETURN_BUFFER_MANAGER_OFFSET                      0x00000008
#define WBM2SW_COMPLETION_RING_TX_RETURN_BUFFER_MANAGER_LSB                         9
#define WBM2SW_COMPLETION_RING_TX_RETURN_BUFFER_MANAGER_MSB                         12
#define WBM2SW_COMPLETION_RING_TX_RETURN_BUFFER_MANAGER_MASK                        0x00001e00




#define WBM2SW_COMPLETION_RING_TX_TQM_RELEASE_REASON_OFFSET                         0x00000008
#define WBM2SW_COMPLETION_RING_TX_TQM_RELEASE_REASON_LSB                            13
#define WBM2SW_COMPLETION_RING_TX_TQM_RELEASE_REASON_MSB                            16
#define WBM2SW_COMPLETION_RING_TX_TQM_RELEASE_REASON_MASK                           0x0001e000




#define WBM2SW_COMPLETION_RING_TX_RBM_OVERRIDE_VALID_OFFSET                         0x00000008
#define WBM2SW_COMPLETION_RING_TX_RBM_OVERRIDE_VALID_LSB                            17
#define WBM2SW_COMPLETION_RING_TX_RBM_OVERRIDE_VALID_MSB                            17
#define WBM2SW_COMPLETION_RING_TX_RBM_OVERRIDE_VALID_MASK                           0x00020000




#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_11_0_OFFSET                      0x00000008
#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_11_0_LSB                         18
#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_11_0_MSB                         29
#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_11_0_MASK                        0x3ffc0000




#define WBM2SW_COMPLETION_RING_TX_RESERVED_2B_OFFSET                                0x00000008
#define WBM2SW_COMPLETION_RING_TX_RESERVED_2B_LSB                                   30
#define WBM2SW_COMPLETION_RING_TX_RESERVED_2B_MSB                                   30
#define WBM2SW_COMPLETION_RING_TX_RESERVED_2B_MASK                                  0x40000000




#define WBM2SW_COMPLETION_RING_TX_WBM_INTERNAL_ERROR_OFFSET                         0x00000008
#define WBM2SW_COMPLETION_RING_TX_WBM_INTERNAL_ERROR_LSB                            31
#define WBM2SW_COMPLETION_RING_TX_WBM_INTERNAL_ERROR_MSB                            31
#define WBM2SW_COMPLETION_RING_TX_WBM_INTERNAL_ERROR_MASK                           0x80000000




#define WBM2SW_COMPLETION_RING_TX_TQM_STATUS_NUMBER_OFFSET                          0x0000000c
#define WBM2SW_COMPLETION_RING_TX_TQM_STATUS_NUMBER_LSB                             0
#define WBM2SW_COMPLETION_RING_TX_TQM_STATUS_NUMBER_MSB                             23
#define WBM2SW_COMPLETION_RING_TX_TQM_STATUS_NUMBER_MASK                            0x00ffffff




#define WBM2SW_COMPLETION_RING_TX_TRANSMIT_COUNT_OFFSET                             0x0000000c
#define WBM2SW_COMPLETION_RING_TX_TRANSMIT_COUNT_LSB                                24
#define WBM2SW_COMPLETION_RING_TX_TRANSMIT_COUNT_MSB                                30
#define WBM2SW_COMPLETION_RING_TX_TRANSMIT_COUNT_MASK                               0x7f000000




#define WBM2SW_COMPLETION_RING_TX_SW_RELEASE_DETAILS_VALID_OFFSET                   0x0000000c
#define WBM2SW_COMPLETION_RING_TX_SW_RELEASE_DETAILS_VALID_LSB                      31
#define WBM2SW_COMPLETION_RING_TX_SW_RELEASE_DETAILS_VALID_MSB                      31
#define WBM2SW_COMPLETION_RING_TX_SW_RELEASE_DETAILS_VALID_MASK                     0x80000000




#define WBM2SW_COMPLETION_RING_TX_ACK_FRAME_RSSI_OFFSET                             0x00000010
#define WBM2SW_COMPLETION_RING_TX_ACK_FRAME_RSSI_LSB                                0
#define WBM2SW_COMPLETION_RING_TX_ACK_FRAME_RSSI_MSB                                7
#define WBM2SW_COMPLETION_RING_TX_ACK_FRAME_RSSI_MASK                               0x000000ff




#define WBM2SW_COMPLETION_RING_TX_FIRST_MSDU_OFFSET                                 0x00000010
#define WBM2SW_COMPLETION_RING_TX_FIRST_MSDU_LSB                                    8
#define WBM2SW_COMPLETION_RING_TX_FIRST_MSDU_MSB                                    8
#define WBM2SW_COMPLETION_RING_TX_FIRST_MSDU_MASK                                   0x00000100




#define WBM2SW_COMPLETION_RING_TX_LAST_MSDU_OFFSET                                  0x00000010
#define WBM2SW_COMPLETION_RING_TX_LAST_MSDU_LSB                                     9
#define WBM2SW_COMPLETION_RING_TX_LAST_MSDU_MSB                                     9
#define WBM2SW_COMPLETION_RING_TX_LAST_MSDU_MASK                                    0x00000200




#define WBM2SW_COMPLETION_RING_TX_FW_TX_NOTIFY_FRAME_OFFSET                         0x00000010
#define WBM2SW_COMPLETION_RING_TX_FW_TX_NOTIFY_FRAME_LSB                            10
#define WBM2SW_COMPLETION_RING_TX_FW_TX_NOTIFY_FRAME_MSB                            12
#define WBM2SW_COMPLETION_RING_TX_FW_TX_NOTIFY_FRAME_MASK                           0x00001c00




#define WBM2SW_COMPLETION_RING_TX_BUFFER_TIMESTAMP_OFFSET                           0x00000010
#define WBM2SW_COMPLETION_RING_TX_BUFFER_TIMESTAMP_LSB                              13
#define WBM2SW_COMPLETION_RING_TX_BUFFER_TIMESTAMP_MSB                              31
#define WBM2SW_COMPLETION_RING_TX_BUFFER_TIMESTAMP_MASK                             0xffffe000







#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_OFFSET     0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_LSB        0
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_MSB        0
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TX_RATE_STATS_INFO_VALID_MASK       0x00000001




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_BW_OFFSET                  0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_BW_LSB                     1
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_BW_MSB                     3
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_BW_MASK                    0x0000000e




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_OFFSET            0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_LSB               4
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_MSB               7
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_PKT_TYPE_MASK              0x000000f0




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_OFFSET                0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_LSB                   8
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_MSB                   8
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_STBC_MASK                  0x00000100




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_OFFSET                0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_LSB                   9
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_MSB                   9
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_LDPC_MASK                  0x00000200




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_OFFSET                 0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_LSB                    10
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_MSB                    11
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_SGI_MASK                   0x00000c00




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_OFFSET                 0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_LSB                    12
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_MSB                    15
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TRANSMIT_MCS_MASK                   0x0000f000




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_OFFSET           0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_LSB              16
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_MSB              16
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_OFDMA_TRANSMISSION_MASK             0x00010000




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TONES_IN_RU_OFFSET                  0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TONES_IN_RU_LSB                     17
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TONES_IN_RU_MSB                     28
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_TONES_IN_RU_MASK                    0x1ffe0000




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_RESERVED_0A_OFFSET                  0x00000014
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_RESERVED_0A_LSB                     29
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_RESERVED_0A_MSB                     31
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_RESERVED_0A_MASK                    0xe0000000




#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET        0x00000018
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB           0
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MSB           31
#define WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK          0xffffffff




#define WBM2SW_COMPLETION_RING_TX_SW_PEER_ID_OFFSET                                 0x0000001c
#define WBM2SW_COMPLETION_RING_TX_SW_PEER_ID_LSB                                    0
#define WBM2SW_COMPLETION_RING_TX_SW_PEER_ID_MSB                                    15
#define WBM2SW_COMPLETION_RING_TX_SW_PEER_ID_MASK                                   0x0000ffff




#define WBM2SW_COMPLETION_RING_TX_TID_OFFSET                                        0x0000001c
#define WBM2SW_COMPLETION_RING_TX_TID_LSB                                           16
#define WBM2SW_COMPLETION_RING_TX_TID_MSB                                           19
#define WBM2SW_COMPLETION_RING_TX_TID_MASK                                          0x000f0000




#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_19_12_OFFSET                     0x0000001c
#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_19_12_LSB                        20
#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_19_12_MSB                        27
#define WBM2SW_COMPLETION_RING_TX_SW_BUFFER_COOKIE_19_12_MASK                       0x0ff00000




#define WBM2SW_COMPLETION_RING_TX_LOOPING_COUNT_OFFSET                              0x0000001c
#define WBM2SW_COMPLETION_RING_TX_LOOPING_COUNT_LSB                                 28
#define WBM2SW_COMPLETION_RING_TX_LOOPING_COUNT_MSB                                 31
#define WBM2SW_COMPLETION_RING_TX_LOOPING_COUNT_MASK                                0xf0000000



#endif


/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

 

#ifndef _WBM_RELEASE_RING_RX_H_
#define _WBM_RELEASE_RING_RX_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_msdu_desc_info.h"
#include "rx_mpdu_desc_info.h"
#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_WBM_RELEASE_RING_RX 8

struct wbm_release_ring_rx {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t release_source_module                                   :  3,  
                      bm_action                                               :  3,  
                      buffer_or_desc_type                                     :  3,  
                      first_msdu_index                                        :  4,  
                      reserved_2a                                             :  2,  
                      cache_id                                                :  1,  
                      cookie_conversion_status                                :  1,  
                      rxdma_push_reason                                       :  2,  
                      rxdma_error_code                                        :  5,  
                      reo_push_reason                                         :  2,  
                      reo_error_code                                          :  5,  
                      wbm_internal_error                                      :  1;  
             struct   rx_mpdu_desc_info                                         rx_mpdu_desc_info_details;
             struct   rx_msdu_desc_info                                         rx_msdu_desc_info_details;
             uint32_t reserved_6a                                             : 32;  
             uint32_t reserved_7a                                             : 20,  
                      ring_id                                                 :  8,  
                      looping_count                                           :  4;  
#else
             struct   buffer_addr_info                                          released_buff_or_desc_addr_info;
             uint32_t wbm_internal_error                                      :  1,  
                      reo_error_code                                          :  5,  
                      reo_push_reason                                         :  2,  
                      rxdma_error_code                                        :  5,  
                      rxdma_push_reason                                       :  2,  
                      cookie_conversion_status                                :  1,  
                      cache_id                                                :  1,  
                      reserved_2a                                             :  2,  
                      first_msdu_index                                        :  4,  
                      buffer_or_desc_type                                     :  3,  
                      bm_action                                               :  3,  
                      release_source_module                                   :  3;  
             struct   rx_mpdu_desc_info                                         rx_mpdu_desc_info_details;
             struct   rx_msdu_desc_info                                         rx_msdu_desc_info_details;
             uint32_t reserved_6a                                             : 32;  
             uint32_t looping_count                                           :  4,  
                      ring_id                                                 :  8,  
                      reserved_7a                                             : 20;  
#endif
};

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET 0x00000000
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_LSB    0
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MSB    31
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_31_0_MASK   0xffffffff

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET 0x00000004
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_LSB   0
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MSB   7
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_BUFFER_ADDR_39_32_MASK  0x000000ff

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET 0x00000004
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB 8
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB 11
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK 0x00000f00

#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET 0x00000004
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_LSB    12
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MSB    31
#define WBM_RELEASE_RING_RX_RELEASED_BUFF_OR_DESC_ADDR_INFO_SW_BUFFER_COOKIE_MASK   0xfffff000

#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_OFFSET                            0x00000008
#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_LSB                               0
#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_MSB                               2
#define WBM_RELEASE_RING_RX_RELEASE_SOURCE_MODULE_MASK                              0x00000007

#define WBM_RELEASE_RING_RX_BM_ACTION_OFFSET                                        0x00000008
#define WBM_RELEASE_RING_RX_BM_ACTION_LSB                                           3
#define WBM_RELEASE_RING_RX_BM_ACTION_MSB                                           5
#define WBM_RELEASE_RING_RX_BM_ACTION_MASK                                          0x00000038

#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_OFFSET                              0x00000008
#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_LSB                                 6
#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_MSB                                 8
#define WBM_RELEASE_RING_RX_BUFFER_OR_DESC_TYPE_MASK                                0x000001c0

#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_LSB                                    9
#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_MSB                                    12
#define WBM_RELEASE_RING_RX_FIRST_MSDU_INDEX_MASK                                   0x00001e00

#define WBM_RELEASE_RING_RX_RESERVED_2A_OFFSET                                      0x00000008
#define WBM_RELEASE_RING_RX_RESERVED_2A_LSB                                         13
#define WBM_RELEASE_RING_RX_RESERVED_2A_MSB                                         14
#define WBM_RELEASE_RING_RX_RESERVED_2A_MASK                                        0x00006000

#define WBM_RELEASE_RING_RX_CACHE_ID_OFFSET                                         0x00000008
#define WBM_RELEASE_RING_RX_CACHE_ID_LSB                                            15
#define WBM_RELEASE_RING_RX_CACHE_ID_MSB                                            15
#define WBM_RELEASE_RING_RX_CACHE_ID_MASK                                           0x00008000

#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_OFFSET                         0x00000008
#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_LSB                            16
#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_MSB                            16
#define WBM_RELEASE_RING_RX_COOKIE_CONVERSION_STATUS_MASK                           0x00010000

#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_OFFSET                                0x00000008
#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_LSB                                   17
#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_MSB                                   18
#define WBM_RELEASE_RING_RX_RXDMA_PUSH_REASON_MASK                                  0x00060000

#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_OFFSET                                 0x00000008
#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_LSB                                    19
#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_MSB                                    23
#define WBM_RELEASE_RING_RX_RXDMA_ERROR_CODE_MASK                                   0x00f80000

#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_OFFSET                                  0x00000008
#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_LSB                                     24
#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_MSB                                     25
#define WBM_RELEASE_RING_RX_REO_PUSH_REASON_MASK                                    0x03000000

#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_OFFSET                                   0x00000008
#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_LSB                                      26
#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_MSB                                      30
#define WBM_RELEASE_RING_RX_REO_ERROR_CODE_MASK                                     0x7c000000

#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_OFFSET                               0x00000008
#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_LSB                                  31
#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_MSB                                  31
#define WBM_RELEASE_RING_RX_WBM_INTERNAL_ERROR_MASK                                 0x80000000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET             0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_LSB                0
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MSB                7
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_MASK               0x000000ff

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_OFFSET          0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_LSB             8
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MSB             8
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_FRAGMENT_FLAG_MASK            0x00000100

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_OFFSET         0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_LSB            9
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MSB            9
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_RETRY_BIT_MASK           0x00000200

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_OFFSET             0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_LSB                10
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MSB                10
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_AMPDU_FLAG_MASK               0x00000400

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_OFFSET              0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_LSB                 11
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MSB                 11
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_BAR_FRAME_MASK                0x00000800

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET 0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_LSB 12
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MSB 12
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PN_FIELDS_CONTAIN_VALID_INFO_MASK 0x00001000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_OFFSET               0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_LSB                  13
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MSB                  13
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_RAW_MPDU_MASK                 0x00002000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_OFFSET     0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_LSB        14
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MSB        14
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MORE_FRAGMENT_FLAG_MASK       0x00004000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_OFFSET               0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_LSB                  15
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MSB                  26
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_SRC_INFO_MASK                 0x07ff8000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_OFFSET 0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_LSB    27
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MSB    27
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_MPDU_QOS_CONTROL_VALID_MASK   0x08000000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_OFFSET                    0x0000000c
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_LSB                       28
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_MSB                       31
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_TID_MASK                      0xf0000000

#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_OFFSET         0x00000010
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_LSB            0
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MSB            31
#define WBM_RELEASE_RING_RX_RX_MPDU_DESC_INFO_DETAILS_PEER_META_DATA_MASK           0xffffffff

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_LSB   0
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MSB   0
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_MASK  0x00000001

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_OFFSET 0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_LSB    1
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MSB    1
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_LAST_MSDU_IN_MPDU_FLAG_MASK   0x00000002

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_OFFSET      0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_LSB         2
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MSB         2
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_CONTINUATION_MASK        0x00000004

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_LSB               3
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MSB               16
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_LENGTH_MASK              0x0001fff8

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_OFFSET              0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_LSB                 17
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MSB                 17
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_MSDU_DROP_MASK                0x00020000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_LSB               18
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MSB               18
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_SA_IS_VALID_MASK              0x00040000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_LSB               19
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MSB               19
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_VALID_MASK              0x00080000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_OFFSET             0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_LSB                20
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MSB                20
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DA_IS_MCBC_MASK               0x00100000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_OFFSET  0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_LSB     21
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MSB     21
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_L3_HEADER_PADDING_MSB_MASK    0x00200000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_OFFSET    0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_LSB       22
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MSB       22
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TCP_UDP_CHKSUM_FAIL_MASK      0x00400000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_OFFSET         0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_LSB            23
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MSB            23
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_IP_CHKSUM_FAIL_MASK           0x00800000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_OFFSET                  0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_LSB                     24
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MSB                     24
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_FR_DS_MASK                    0x01000000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_OFFSET                  0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_LSB                     25
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MSB                     25
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_TO_DS_MASK                    0x02000000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_OFFSET              0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_LSB                 26
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MSB                 26
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_INTRA_BSS_MASK                0x04000000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_OFFSET           0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_LSB              27
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MSB              28
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DEST_CHIP_ID_MASK             0x18000000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_OFFSET           0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_LSB              29
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MSB              30
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_DECAP_FORMAT_MASK             0x60000000

#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_OFFSET            0x00000014
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_LSB               31
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_MSB               31
#define WBM_RELEASE_RING_RX_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_MASK              0x80000000

#define WBM_RELEASE_RING_RX_RESERVED_6A_OFFSET                                      0x00000018
#define WBM_RELEASE_RING_RX_RESERVED_6A_LSB                                         0
#define WBM_RELEASE_RING_RX_RESERVED_6A_MSB                                         31
#define WBM_RELEASE_RING_RX_RESERVED_6A_MASK                                        0xffffffff

#define WBM_RELEASE_RING_RX_RESERVED_7A_OFFSET                                      0x0000001c
#define WBM_RELEASE_RING_RX_RESERVED_7A_LSB                                         0
#define WBM_RELEASE_RING_RX_RESERVED_7A_MSB                                         19
#define WBM_RELEASE_RING_RX_RESERVED_7A_MASK                                        0x000fffff

#define WBM_RELEASE_RING_RX_RING_ID_OFFSET                                          0x0000001c
#define WBM_RELEASE_RING_RX_RING_ID_LSB                                             20
#define WBM_RELEASE_RING_RX_RING_ID_MSB                                             27
#define WBM_RELEASE_RING_RX_RING_ID_MASK                                            0x0ff00000

#define WBM_RELEASE_RING_RX_LOOPING_COUNT_OFFSET                                    0x0000001c
#define WBM_RELEASE_RING_RX_LOOPING_COUNT_LSB                                       28
#define WBM_RELEASE_RING_RX_LOOPING_COUNT_MSB                                       31
#define WBM_RELEASE_RING_RX_LOOPING_COUNT_MASK                                      0xf0000000

#endif


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

 

#ifndef _REO_FLUSH_CACHE_STATUS_H_
#define _REO_FLUSH_CACHE_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"
#define NUM_OF_DWORDS_REO_FLUSH_CACHE_STATUS 26

#define NUM_OF_QWORDS_REO_FLUSH_CACHE_STATUS 13

struct reo_flush_cache_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_reo_status_header                                 status_header;
             uint32_t error_detected                                          :  1,  
                      block_error_details                                     :  2,  
                      reserved_2a                                             :  5,  
                      cache_controller_flush_status_hit                       :  1,  
                      cache_controller_flush_status_desc_type                 :  3,  
                      cache_controller_flush_status_client_id                 :  4,  
                      cache_controller_flush_status_error                     :  2,  
                      cache_controller_flush_count                            :  8,  
                      flush_queue_1k_desc                                     :  1,  
                      reserved_2b                                             :  5;  
             uint32_t reserved_3a                                             : 32;  
             uint32_t reserved_4a                                             : 32;  
             uint32_t reserved_5a                                             : 32;  
             uint32_t reserved_6a                                             : 32;  
             uint32_t reserved_7a                                             : 32;  
             uint32_t reserved_8a                                             : 32;  
             uint32_t reserved_9a                                             : 32;  
             uint32_t reserved_10a                                            : 32;  
             uint32_t reserved_11a                                            : 32;  
             uint32_t reserved_12a                                            : 32;  
             uint32_t reserved_13a                                            : 32;  
             uint32_t reserved_14a                                            : 32;  
             uint32_t reserved_15a                                            : 32;  
             uint32_t reserved_16a                                            : 32;  
             uint32_t reserved_17a                                            : 32;  
             uint32_t reserved_18a                                            : 32;  
             uint32_t reserved_19a                                            : 32;  
             uint32_t reserved_20a                                            : 32;  
             uint32_t reserved_21a                                            : 32;  
             uint32_t reserved_22a                                            : 32;  
             uint32_t reserved_23a                                            : 32;  
             uint32_t reserved_24a                                            : 32;  
             uint32_t reserved_25a                                            : 28,  
                      looping_count                                           :  4;  
#else
             struct   uniform_reo_status_header                                 status_header;
             uint32_t reserved_2b                                             :  5,  
                      flush_queue_1k_desc                                     :  1,  
                      cache_controller_flush_count                            :  8,  
                      cache_controller_flush_status_error                     :  2,  
                      cache_controller_flush_status_client_id                 :  4,  
                      cache_controller_flush_status_desc_type                 :  3,  
                      cache_controller_flush_status_hit                       :  1,  
                      reserved_2a                                             :  5,  
                      block_error_details                                     :  2,  
                      error_detected                                          :  1;  
             uint32_t reserved_3a                                             : 32;  
             uint32_t reserved_4a                                             : 32;  
             uint32_t reserved_5a                                             : 32;  
             uint32_t reserved_6a                                             : 32;  
             uint32_t reserved_7a                                             : 32;  
             uint32_t reserved_8a                                             : 32;  
             uint32_t reserved_9a                                             : 32;  
             uint32_t reserved_10a                                            : 32;  
             uint32_t reserved_11a                                            : 32;  
             uint32_t reserved_12a                                            : 32;  
             uint32_t reserved_13a                                            : 32;  
             uint32_t reserved_14a                                            : 32;  
             uint32_t reserved_15a                                            : 32;  
             uint32_t reserved_16a                                            : 32;  
             uint32_t reserved_17a                                            : 32;  
             uint32_t reserved_18a                                            : 32;  
             uint32_t reserved_19a                                            : 32;  
             uint32_t reserved_20a                                            : 32;  
             uint32_t reserved_21a                                            : 32;  
             uint32_t reserved_22a                                            : 32;  
             uint32_t reserved_23a                                            : 32;  
             uint32_t reserved_24a                                            : 32;  
             uint32_t looping_count                                           :  4,  
                      reserved_25a                                            : 28;  
#endif
};

#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET               0x0000000000000000
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_LSB                  0
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MSB                  15
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_STATUS_NUMBER_MASK                 0x000000000000ffff

#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET              0x0000000000000000
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_LSB                 16
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MSB                 25
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_CMD_EXECUTION_TIME_MASK                0x0000000003ff0000

#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET        0x0000000000000000
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB           26
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MSB           27
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK          0x000000000c000000

#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_RESERVED_0A_OFFSET                     0x0000000000000000
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_RESERVED_0A_LSB                        28
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_RESERVED_0A_MSB                        31
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_RESERVED_0A_MASK                       0x00000000f0000000

#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_TIMESTAMP_OFFSET                       0x0000000000000000
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_TIMESTAMP_LSB                          32
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_TIMESTAMP_MSB                          63
#define REO_FLUSH_CACHE_STATUS_STATUS_HEADER_TIMESTAMP_MASK                         0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_ERROR_DETECTED_OFFSET                                0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_ERROR_DETECTED_LSB                                   0
#define REO_FLUSH_CACHE_STATUS_ERROR_DETECTED_MSB                                   0
#define REO_FLUSH_CACHE_STATUS_ERROR_DETECTED_MASK                                  0x0000000000000001

#define REO_FLUSH_CACHE_STATUS_BLOCK_ERROR_DETAILS_OFFSET                           0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_BLOCK_ERROR_DETAILS_LSB                              1
#define REO_FLUSH_CACHE_STATUS_BLOCK_ERROR_DETAILS_MSB                              2
#define REO_FLUSH_CACHE_STATUS_BLOCK_ERROR_DETAILS_MASK                             0x0000000000000006

#define REO_FLUSH_CACHE_STATUS_RESERVED_2A_OFFSET                                   0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_RESERVED_2A_LSB                                      3
#define REO_FLUSH_CACHE_STATUS_RESERVED_2A_MSB                                      7
#define REO_FLUSH_CACHE_STATUS_RESERVED_2A_MASK                                     0x00000000000000f8

#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_HIT_OFFSET             0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_HIT_LSB                8
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_HIT_MSB                8
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_HIT_MASK               0x0000000000000100

#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_DESC_TYPE_OFFSET       0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_DESC_TYPE_LSB          9
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_DESC_TYPE_MSB          11
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_DESC_TYPE_MASK         0x0000000000000e00

#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_CLIENT_ID_OFFSET       0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_CLIENT_ID_LSB          12
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_CLIENT_ID_MSB          15
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_CLIENT_ID_MASK         0x000000000000f000

#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_ERROR_OFFSET           0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_ERROR_LSB              16
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_ERROR_MSB              17
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_STATUS_ERROR_MASK             0x0000000000030000

#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_COUNT_OFFSET                  0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_COUNT_LSB                     18
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_COUNT_MSB                     25
#define REO_FLUSH_CACHE_STATUS_CACHE_CONTROLLER_FLUSH_COUNT_MASK                    0x0000000003fc0000

#define REO_FLUSH_CACHE_STATUS_FLUSH_QUEUE_1K_DESC_OFFSET                           0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_FLUSH_QUEUE_1K_DESC_LSB                              26
#define REO_FLUSH_CACHE_STATUS_FLUSH_QUEUE_1K_DESC_MSB                              26
#define REO_FLUSH_CACHE_STATUS_FLUSH_QUEUE_1K_DESC_MASK                             0x0000000004000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_2B_OFFSET                                   0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_RESERVED_2B_LSB                                      27
#define REO_FLUSH_CACHE_STATUS_RESERVED_2B_MSB                                      31
#define REO_FLUSH_CACHE_STATUS_RESERVED_2B_MASK                                     0x00000000f8000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_3A_OFFSET                                   0x0000000000000008
#define REO_FLUSH_CACHE_STATUS_RESERVED_3A_LSB                                      32
#define REO_FLUSH_CACHE_STATUS_RESERVED_3A_MSB                                      63
#define REO_FLUSH_CACHE_STATUS_RESERVED_3A_MASK                                     0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_4A_OFFSET                                   0x0000000000000010
#define REO_FLUSH_CACHE_STATUS_RESERVED_4A_LSB                                      0
#define REO_FLUSH_CACHE_STATUS_RESERVED_4A_MSB                                      31
#define REO_FLUSH_CACHE_STATUS_RESERVED_4A_MASK                                     0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_5A_OFFSET                                   0x0000000000000010
#define REO_FLUSH_CACHE_STATUS_RESERVED_5A_LSB                                      32
#define REO_FLUSH_CACHE_STATUS_RESERVED_5A_MSB                                      63
#define REO_FLUSH_CACHE_STATUS_RESERVED_5A_MASK                                     0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_6A_OFFSET                                   0x0000000000000018
#define REO_FLUSH_CACHE_STATUS_RESERVED_6A_LSB                                      0
#define REO_FLUSH_CACHE_STATUS_RESERVED_6A_MSB                                      31
#define REO_FLUSH_CACHE_STATUS_RESERVED_6A_MASK                                     0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_7A_OFFSET                                   0x0000000000000018
#define REO_FLUSH_CACHE_STATUS_RESERVED_7A_LSB                                      32
#define REO_FLUSH_CACHE_STATUS_RESERVED_7A_MSB                                      63
#define REO_FLUSH_CACHE_STATUS_RESERVED_7A_MASK                                     0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_8A_OFFSET                                   0x0000000000000020
#define REO_FLUSH_CACHE_STATUS_RESERVED_8A_LSB                                      0
#define REO_FLUSH_CACHE_STATUS_RESERVED_8A_MSB                                      31
#define REO_FLUSH_CACHE_STATUS_RESERVED_8A_MASK                                     0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_9A_OFFSET                                   0x0000000000000020
#define REO_FLUSH_CACHE_STATUS_RESERVED_9A_LSB                                      32
#define REO_FLUSH_CACHE_STATUS_RESERVED_9A_MSB                                      63
#define REO_FLUSH_CACHE_STATUS_RESERVED_9A_MASK                                     0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_10A_OFFSET                                  0x0000000000000028
#define REO_FLUSH_CACHE_STATUS_RESERVED_10A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_10A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_10A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_11A_OFFSET                                  0x0000000000000028
#define REO_FLUSH_CACHE_STATUS_RESERVED_11A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_11A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_11A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_12A_OFFSET                                  0x0000000000000030
#define REO_FLUSH_CACHE_STATUS_RESERVED_12A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_12A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_12A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_13A_OFFSET                                  0x0000000000000030
#define REO_FLUSH_CACHE_STATUS_RESERVED_13A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_13A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_13A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_14A_OFFSET                                  0x0000000000000038
#define REO_FLUSH_CACHE_STATUS_RESERVED_14A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_14A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_14A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_15A_OFFSET                                  0x0000000000000038
#define REO_FLUSH_CACHE_STATUS_RESERVED_15A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_15A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_15A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_16A_OFFSET                                  0x0000000000000040
#define REO_FLUSH_CACHE_STATUS_RESERVED_16A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_16A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_16A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_17A_OFFSET                                  0x0000000000000040
#define REO_FLUSH_CACHE_STATUS_RESERVED_17A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_17A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_17A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_18A_OFFSET                                  0x0000000000000048
#define REO_FLUSH_CACHE_STATUS_RESERVED_18A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_18A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_18A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_19A_OFFSET                                  0x0000000000000048
#define REO_FLUSH_CACHE_STATUS_RESERVED_19A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_19A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_19A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_20A_OFFSET                                  0x0000000000000050
#define REO_FLUSH_CACHE_STATUS_RESERVED_20A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_20A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_20A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_21A_OFFSET                                  0x0000000000000050
#define REO_FLUSH_CACHE_STATUS_RESERVED_21A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_21A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_21A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_22A_OFFSET                                  0x0000000000000058
#define REO_FLUSH_CACHE_STATUS_RESERVED_22A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_22A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_22A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_23A_OFFSET                                  0x0000000000000058
#define REO_FLUSH_CACHE_STATUS_RESERVED_23A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_23A_MSB                                     63
#define REO_FLUSH_CACHE_STATUS_RESERVED_23A_MASK                                    0xffffffff00000000

#define REO_FLUSH_CACHE_STATUS_RESERVED_24A_OFFSET                                  0x0000000000000060
#define REO_FLUSH_CACHE_STATUS_RESERVED_24A_LSB                                     0
#define REO_FLUSH_CACHE_STATUS_RESERVED_24A_MSB                                     31
#define REO_FLUSH_CACHE_STATUS_RESERVED_24A_MASK                                    0x00000000ffffffff

#define REO_FLUSH_CACHE_STATUS_RESERVED_25A_OFFSET                                  0x0000000000000060
#define REO_FLUSH_CACHE_STATUS_RESERVED_25A_LSB                                     32
#define REO_FLUSH_CACHE_STATUS_RESERVED_25A_MSB                                     59
#define REO_FLUSH_CACHE_STATUS_RESERVED_25A_MASK                                    0x0fffffff00000000

#define REO_FLUSH_CACHE_STATUS_LOOPING_COUNT_OFFSET                                 0x0000000000000060
#define REO_FLUSH_CACHE_STATUS_LOOPING_COUNT_LSB                                    60
#define REO_FLUSH_CACHE_STATUS_LOOPING_COUNT_MSB                                    63
#define REO_FLUSH_CACHE_STATUS_LOOPING_COUNT_MASK                                   0xf000000000000000

#endif

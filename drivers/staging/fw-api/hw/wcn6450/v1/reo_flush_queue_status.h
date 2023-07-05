
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



#ifndef _REO_FLUSH_QUEUE_STATUS_H_
#define _REO_FLUSH_QUEUE_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_status_header.h"

#define NUM_OF_DWORDS_REO_FLUSH_QUEUE_STATUS 25

struct reo_flush_queue_status {
    struct            uniform_reo_status_header                       status_header;
             uint32_t error_detected                  :  1,
                      reserved_2a                     : 31;
             uint32_t reserved_3a                     : 32;
             uint32_t reserved_4a                     : 32;
             uint32_t reserved_5a                     : 32;
             uint32_t reserved_6a                     : 32;
             uint32_t reserved_7a                     : 32;
             uint32_t reserved_8a                     : 32;
             uint32_t reserved_9a                     : 32;
             uint32_t reserved_10a                    : 32;
             uint32_t reserved_11a                    : 32;
             uint32_t reserved_12a                    : 32;
             uint32_t reserved_13a                    : 32;
             uint32_t reserved_14a                    : 32;
             uint32_t reserved_15a                    : 32;
             uint32_t reserved_16a                    : 32;
             uint32_t reserved_17a                    : 32;
             uint32_t reserved_18a                    : 32;
             uint32_t reserved_19a                    : 32;
             uint32_t reserved_20a                    : 32;
             uint32_t reserved_21a                    : 32;
             uint32_t reserved_22a                    : 32;
             uint32_t reserved_23a                    : 32;
             uint32_t reserved_24a                    : 28,
                      looping_count                   :  4;
};

#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER_OFFSET 0x00000000
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER_LSB 0
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_REO_STATUS_NUMBER_MASK 0x0000ffff

#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME_OFFSET 0x00000000
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME_LSB 16
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_CMD_EXECUTION_TIME_MASK 0x03ff0000

#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_OFFSET 0x00000000
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_LSB 26
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_REO_CMD_EXECUTION_STATUS_MASK 0x0c000000

#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_RESERVED_0A_OFFSET    0x00000000
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_RESERVED_0A_LSB       28
#define REO_FLUSH_QUEUE_STATUS_0_STATUS_HEADER_RESERVED_0A_MASK      0xf0000000

#define REO_FLUSH_QUEUE_STATUS_1_STATUS_HEADER_TIMESTAMP_OFFSET      0x00000004
#define REO_FLUSH_QUEUE_STATUS_1_STATUS_HEADER_TIMESTAMP_LSB         0
#define REO_FLUSH_QUEUE_STATUS_1_STATUS_HEADER_TIMESTAMP_MASK        0xffffffff

#define REO_FLUSH_QUEUE_STATUS_2_ERROR_DETECTED_OFFSET               0x00000008
#define REO_FLUSH_QUEUE_STATUS_2_ERROR_DETECTED_LSB                  0
#define REO_FLUSH_QUEUE_STATUS_2_ERROR_DETECTED_MASK                 0x00000001

#define REO_FLUSH_QUEUE_STATUS_2_RESERVED_2A_OFFSET                  0x00000008
#define REO_FLUSH_QUEUE_STATUS_2_RESERVED_2A_LSB                     1
#define REO_FLUSH_QUEUE_STATUS_2_RESERVED_2A_MASK                    0xfffffffe

#define REO_FLUSH_QUEUE_STATUS_3_RESERVED_3A_OFFSET                  0x0000000c
#define REO_FLUSH_QUEUE_STATUS_3_RESERVED_3A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_3_RESERVED_3A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_4_RESERVED_4A_OFFSET                  0x00000010
#define REO_FLUSH_QUEUE_STATUS_4_RESERVED_4A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_4_RESERVED_4A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_5_RESERVED_5A_OFFSET                  0x00000014
#define REO_FLUSH_QUEUE_STATUS_5_RESERVED_5A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_5_RESERVED_5A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_6_RESERVED_6A_OFFSET                  0x00000018
#define REO_FLUSH_QUEUE_STATUS_6_RESERVED_6A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_6_RESERVED_6A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_7_RESERVED_7A_OFFSET                  0x0000001c
#define REO_FLUSH_QUEUE_STATUS_7_RESERVED_7A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_7_RESERVED_7A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_8_RESERVED_8A_OFFSET                  0x00000020
#define REO_FLUSH_QUEUE_STATUS_8_RESERVED_8A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_8_RESERVED_8A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_9_RESERVED_9A_OFFSET                  0x00000024
#define REO_FLUSH_QUEUE_STATUS_9_RESERVED_9A_LSB                     0
#define REO_FLUSH_QUEUE_STATUS_9_RESERVED_9A_MASK                    0xffffffff

#define REO_FLUSH_QUEUE_STATUS_10_RESERVED_10A_OFFSET                0x00000028
#define REO_FLUSH_QUEUE_STATUS_10_RESERVED_10A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_10_RESERVED_10A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_11_RESERVED_11A_OFFSET                0x0000002c
#define REO_FLUSH_QUEUE_STATUS_11_RESERVED_11A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_11_RESERVED_11A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_12_RESERVED_12A_OFFSET                0x00000030
#define REO_FLUSH_QUEUE_STATUS_12_RESERVED_12A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_12_RESERVED_12A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_13_RESERVED_13A_OFFSET                0x00000034
#define REO_FLUSH_QUEUE_STATUS_13_RESERVED_13A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_13_RESERVED_13A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_14_RESERVED_14A_OFFSET                0x00000038
#define REO_FLUSH_QUEUE_STATUS_14_RESERVED_14A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_14_RESERVED_14A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_15_RESERVED_15A_OFFSET                0x0000003c
#define REO_FLUSH_QUEUE_STATUS_15_RESERVED_15A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_15_RESERVED_15A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_16_RESERVED_16A_OFFSET                0x00000040
#define REO_FLUSH_QUEUE_STATUS_16_RESERVED_16A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_16_RESERVED_16A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_17_RESERVED_17A_OFFSET                0x00000044
#define REO_FLUSH_QUEUE_STATUS_17_RESERVED_17A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_17_RESERVED_17A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_18_RESERVED_18A_OFFSET                0x00000048
#define REO_FLUSH_QUEUE_STATUS_18_RESERVED_18A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_18_RESERVED_18A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_19_RESERVED_19A_OFFSET                0x0000004c
#define REO_FLUSH_QUEUE_STATUS_19_RESERVED_19A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_19_RESERVED_19A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_20_RESERVED_20A_OFFSET                0x00000050
#define REO_FLUSH_QUEUE_STATUS_20_RESERVED_20A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_20_RESERVED_20A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_21_RESERVED_21A_OFFSET                0x00000054
#define REO_FLUSH_QUEUE_STATUS_21_RESERVED_21A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_21_RESERVED_21A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_22_RESERVED_22A_OFFSET                0x00000058
#define REO_FLUSH_QUEUE_STATUS_22_RESERVED_22A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_22_RESERVED_22A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_23_RESERVED_23A_OFFSET                0x0000005c
#define REO_FLUSH_QUEUE_STATUS_23_RESERVED_23A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_23_RESERVED_23A_MASK                  0xffffffff

#define REO_FLUSH_QUEUE_STATUS_24_RESERVED_24A_OFFSET                0x00000060
#define REO_FLUSH_QUEUE_STATUS_24_RESERVED_24A_LSB                   0
#define REO_FLUSH_QUEUE_STATUS_24_RESERVED_24A_MASK                  0x0fffffff

#define REO_FLUSH_QUEUE_STATUS_24_LOOPING_COUNT_OFFSET               0x00000060
#define REO_FLUSH_QUEUE_STATUS_24_LOOPING_COUNT_LSB                  28
#define REO_FLUSH_QUEUE_STATUS_24_LOOPING_COUNT_MASK                 0xf0000000

#endif

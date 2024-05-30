
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



#ifndef _RX_FLOW_SEARCH_ENTRY_H_
#define _RX_FLOW_SEARCH_ENTRY_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_FLOW_SEARCH_ENTRY 16

struct rx_flow_search_entry {
             uint32_t src_ip_127_96                   : 32;
             uint32_t src_ip_95_64                    : 32;
             uint32_t src_ip_63_32                    : 32;
             uint32_t src_ip_31_0                     : 32;
             uint32_t dest_ip_127_96                  : 32;
             uint32_t dest_ip_95_64                   : 32;
             uint32_t dest_ip_63_32                   : 32;
             uint32_t dest_ip_31_0                    : 32;
             uint32_t src_port                        : 16,
                      dest_port                       : 16;
             uint32_t l4_protocol                     :  8,
                      valid                           :  1,
                      reserved_9                      : 15,
                      reo_destination_indication      :  5,
                      msdu_drop                       :  1,
                      reo_destination_handler         :  2;
             uint32_t metadata                        : 32;
             uint32_t aggregation_count               :  7,
                      lro_eligible                    :  1,
                      msdu_count                      : 24;
             uint32_t msdu_byte_count                 : 32;
             uint32_t timestamp                       : 32;
             uint32_t cumulative_l4_checksum          : 16,
                      cumulative_ip_length            : 16;
             uint32_t tcp_sequence_number             : 32;
};

#define RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96_OFFSET                  0x00000000
#define RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96_LSB                     0
#define RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96_MASK                    0xffffffff

#define RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64_OFFSET                   0x00000004
#define RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64_LSB                      0
#define RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64_MASK                     0xffffffff

#define RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32_OFFSET                   0x00000008
#define RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32_LSB                      0
#define RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32_MASK                     0xffffffff

#define RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0_OFFSET                    0x0000000c
#define RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0_LSB                       0
#define RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0_MASK                      0xffffffff

#define RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96_OFFSET                 0x00000010
#define RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96_LSB                    0
#define RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96_MASK                   0xffffffff

#define RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64_OFFSET                  0x00000014
#define RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64_LSB                     0
#define RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64_MASK                    0xffffffff

#define RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32_OFFSET                  0x00000018
#define RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32_LSB                     0
#define RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32_MASK                    0xffffffff

#define RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0_OFFSET                   0x0000001c
#define RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0_LSB                      0
#define RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0_MASK                     0xffffffff

#define RX_FLOW_SEARCH_ENTRY_8_SRC_PORT_OFFSET                       0x00000020
#define RX_FLOW_SEARCH_ENTRY_8_SRC_PORT_LSB                          0
#define RX_FLOW_SEARCH_ENTRY_8_SRC_PORT_MASK                         0x0000ffff

#define RX_FLOW_SEARCH_ENTRY_8_DEST_PORT_OFFSET                      0x00000020
#define RX_FLOW_SEARCH_ENTRY_8_DEST_PORT_LSB                         16
#define RX_FLOW_SEARCH_ENTRY_8_DEST_PORT_MASK                        0xffff0000

#define RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL_OFFSET                    0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL_LSB                       0
#define RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL_MASK                      0x000000ff

#define RX_FLOW_SEARCH_ENTRY_9_VALID_OFFSET                          0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_VALID_LSB                             8
#define RX_FLOW_SEARCH_ENTRY_9_VALID_MASK                            0x00000100

#define RX_FLOW_SEARCH_ENTRY_9_RESERVED_9_OFFSET                     0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_RESERVED_9_LSB                        9
#define RX_FLOW_SEARCH_ENTRY_9_RESERVED_9_MASK                       0x00fffe00

#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION_OFFSET     0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION_LSB        24
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION_MASK       0x1f000000

#define RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP_OFFSET                      0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP_LSB                         29
#define RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP_MASK                        0x20000000

#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER_OFFSET        0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER_LSB           30
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER_MASK          0xc0000000

#define RX_FLOW_SEARCH_ENTRY_10_METADATA_OFFSET                      0x00000028
#define RX_FLOW_SEARCH_ENTRY_10_METADATA_LSB                         0
#define RX_FLOW_SEARCH_ENTRY_10_METADATA_MASK                        0xffffffff

#define RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT_OFFSET             0x0000002c
#define RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT_LSB                0
#define RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT_MASK               0x0000007f

#define RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE_OFFSET                  0x0000002c
#define RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE_LSB                     7
#define RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE_MASK                    0x00000080

#define RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT_OFFSET                    0x0000002c
#define RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT_LSB                       8
#define RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT_MASK                      0xffffff00

#define RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT_OFFSET               0x00000030
#define RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT_LSB                  0
#define RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT_MASK                 0xffffffff

#define RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP_OFFSET                     0x00000034
#define RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP_LSB                        0
#define RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP_MASK                       0xffffffff

#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM_OFFSET        0x00000038
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM_LSB           0
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM_MASK          0x0000ffff

#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH_OFFSET          0x00000038
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH_LSB             16
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH_MASK            0xffff0000

#define RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER_OFFSET           0x0000003c
#define RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER_LSB              0
#define RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER_MASK             0xffffffff

#endif

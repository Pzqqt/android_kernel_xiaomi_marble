
/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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










#ifndef _NO_ACK_REPORT_H_
#define _NO_ACK_REPORT_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_NO_ACK_REPORT 4


struct no_ack_report {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t no_ack_transmit_reason                                  :  4, // [3:0]
                      macrx_abort_reason                                      :  4, // [7:4]
                      phyrx_abort_reason                                      :  8, // [15:8]
                      frame_control                                           : 16; // [31:16]
             uint32_t rx_ppdu_duration                                        : 24, // [23:0]
                      sr_ppdu_during_obss                                     :  1, // [24:24]
                      selfgen_response_reason_to_sr_ppdu                      :  4, // [28:25]
                      reserved_1                                              :  3; // [31:29]
             uint32_t pre_bt_broadcast_status_details                         : 12, // [11:0]
                      first_bt_broadcast_status_details                       : 12, // [23:12]
                      reserved_2                                              :  8; // [31:24]
             uint32_t second_bt_broadcast_status_details                      : 12, // [11:0]
                      reserved_3                                              : 20; // [31:12]
#else
             uint32_t frame_control                                           : 16, // [31:16]
                      phyrx_abort_reason                                      :  8, // [15:8]
                      macrx_abort_reason                                      :  4, // [7:4]
                      no_ack_transmit_reason                                  :  4; // [3:0]
             uint32_t reserved_1                                              :  3, // [31:29]
                      selfgen_response_reason_to_sr_ppdu                      :  4, // [28:25]
                      sr_ppdu_during_obss                                     :  1, // [24:24]
                      rx_ppdu_duration                                        : 24; // [23:0]
             uint32_t reserved_2                                              :  8, // [31:24]
                      first_bt_broadcast_status_details                       : 12, // [23:12]
                      pre_bt_broadcast_status_details                         : 12; // [11:0]
             uint32_t reserved_3                                              : 20, // [31:12]
                      second_bt_broadcast_status_details                      : 12; // [11:0]
#endif
};




#define NO_ACK_REPORT_NO_ACK_TRANSMIT_REASON_OFFSET                                 0x00000000
#define NO_ACK_REPORT_NO_ACK_TRANSMIT_REASON_LSB                                    0
#define NO_ACK_REPORT_NO_ACK_TRANSMIT_REASON_MSB                                    3
#define NO_ACK_REPORT_NO_ACK_TRANSMIT_REASON_MASK                                   0x0000000f




#define NO_ACK_REPORT_MACRX_ABORT_REASON_OFFSET                                     0x00000000
#define NO_ACK_REPORT_MACRX_ABORT_REASON_LSB                                        4
#define NO_ACK_REPORT_MACRX_ABORT_REASON_MSB                                        7
#define NO_ACK_REPORT_MACRX_ABORT_REASON_MASK                                       0x000000f0




#define NO_ACK_REPORT_PHYRX_ABORT_REASON_OFFSET                                     0x00000000
#define NO_ACK_REPORT_PHYRX_ABORT_REASON_LSB                                        8
#define NO_ACK_REPORT_PHYRX_ABORT_REASON_MSB                                        15
#define NO_ACK_REPORT_PHYRX_ABORT_REASON_MASK                                       0x0000ff00




#define NO_ACK_REPORT_FRAME_CONTROL_OFFSET                                          0x00000000
#define NO_ACK_REPORT_FRAME_CONTROL_LSB                                             16
#define NO_ACK_REPORT_FRAME_CONTROL_MSB                                             31
#define NO_ACK_REPORT_FRAME_CONTROL_MASK                                            0xffff0000




#define NO_ACK_REPORT_RX_PPDU_DURATION_OFFSET                                       0x00000004
#define NO_ACK_REPORT_RX_PPDU_DURATION_LSB                                          0
#define NO_ACK_REPORT_RX_PPDU_DURATION_MSB                                          23
#define NO_ACK_REPORT_RX_PPDU_DURATION_MASK                                         0x00ffffff




#define NO_ACK_REPORT_SR_PPDU_DURING_OBSS_OFFSET                                    0x00000004
#define NO_ACK_REPORT_SR_PPDU_DURING_OBSS_LSB                                       24
#define NO_ACK_REPORT_SR_PPDU_DURING_OBSS_MSB                                       24
#define NO_ACK_REPORT_SR_PPDU_DURING_OBSS_MASK                                      0x01000000




#define NO_ACK_REPORT_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_OFFSET                     0x00000004
#define NO_ACK_REPORT_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_LSB                        25
#define NO_ACK_REPORT_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_MSB                        28
#define NO_ACK_REPORT_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_MASK                       0x1e000000




#define NO_ACK_REPORT_RESERVED_1_OFFSET                                             0x00000004
#define NO_ACK_REPORT_RESERVED_1_LSB                                                29
#define NO_ACK_REPORT_RESERVED_1_MSB                                                31
#define NO_ACK_REPORT_RESERVED_1_MASK                                               0xe0000000




#define NO_ACK_REPORT_PRE_BT_BROADCAST_STATUS_DETAILS_OFFSET                        0x00000008
#define NO_ACK_REPORT_PRE_BT_BROADCAST_STATUS_DETAILS_LSB                           0
#define NO_ACK_REPORT_PRE_BT_BROADCAST_STATUS_DETAILS_MSB                           11
#define NO_ACK_REPORT_PRE_BT_BROADCAST_STATUS_DETAILS_MASK                          0x00000fff




#define NO_ACK_REPORT_FIRST_BT_BROADCAST_STATUS_DETAILS_OFFSET                      0x00000008
#define NO_ACK_REPORT_FIRST_BT_BROADCAST_STATUS_DETAILS_LSB                         12
#define NO_ACK_REPORT_FIRST_BT_BROADCAST_STATUS_DETAILS_MSB                         23
#define NO_ACK_REPORT_FIRST_BT_BROADCAST_STATUS_DETAILS_MASK                        0x00fff000




#define NO_ACK_REPORT_RESERVED_2_OFFSET                                             0x00000008
#define NO_ACK_REPORT_RESERVED_2_LSB                                                24
#define NO_ACK_REPORT_RESERVED_2_MSB                                                31
#define NO_ACK_REPORT_RESERVED_2_MASK                                               0xff000000




#define NO_ACK_REPORT_SECOND_BT_BROADCAST_STATUS_DETAILS_OFFSET                     0x0000000c
#define NO_ACK_REPORT_SECOND_BT_BROADCAST_STATUS_DETAILS_LSB                        0
#define NO_ACK_REPORT_SECOND_BT_BROADCAST_STATUS_DETAILS_MSB                        11
#define NO_ACK_REPORT_SECOND_BT_BROADCAST_STATUS_DETAILS_MASK                       0x00000fff




#define NO_ACK_REPORT_RESERVED_3_OFFSET                                             0x0000000c
#define NO_ACK_REPORT_RESERVED_3_LSB                                                12
#define NO_ACK_REPORT_RESERVED_3_MSB                                                31
#define NO_ACK_REPORT_RESERVED_3_MASK                                               0xfffff000




#endif

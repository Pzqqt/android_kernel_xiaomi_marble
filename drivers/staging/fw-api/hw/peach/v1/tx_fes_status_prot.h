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


#ifndef _TX_FES_STATUS_PROT_H_
#define _TX_FES_STATUS_PROT_H_

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_TX_FES_STATUS_PROT 3

struct tx_fes_status_prot {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t success                                                 :  1,
                      phytx_pkt_end_info_valid                                :  1,
                      phytx_abort_request_info_valid                          :  1,
                      reserved_0                                              : 20,
                      pkt_type                                                :  4,
                      dot11ax_su_extended                                     :  1,
                      rate_mcs                                                :  4;
             uint32_t frame_type                                              :  2,
                      frame_subtype                                           :  4,
                      rx_pwr_mgmt                                             :  1,
                      status                                                  :  1,
                      duration_field                                          : 16,
                      reserved_1a                                             :  2,
                      agc_cbw                                                 :  3,
                      service_cbw                                             :  3;
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t reserved_2a                                             : 16;
#else
             uint32_t rate_mcs                                                :  4,
                      dot11ax_su_extended                                     :  1,
                      pkt_type                                                :  4,
                      reserved_0                                              : 20,
                      phytx_abort_request_info_valid                          :  1,
                      phytx_pkt_end_info_valid                                :  1,
                      success                                                 :  1;
             uint32_t service_cbw                                             :  3,
                      agc_cbw                                                 :  3,
                      reserved_1a                                             :  2,
                      duration_field                                          : 16,
                      status                                                  :  1,
                      rx_pwr_mgmt                                             :  1,
                      frame_subtype                                           :  4,
                      frame_type                                              :  2;
             uint32_t reserved_2a                                             : 16;
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
#endif
};

#define TX_FES_STATUS_PROT_SUCCESS_OFFSET                                           0x00000000
#define TX_FES_STATUS_PROT_SUCCESS_LSB                                              0
#define TX_FES_STATUS_PROT_SUCCESS_MSB                                              0
#define TX_FES_STATUS_PROT_SUCCESS_MASK                                             0x00000001

#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_OFFSET                          0x00000000
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_LSB                             1
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_MSB                             1
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_MASK                            0x00000002

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                    0x00000000
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                       2
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                       2
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                      0x00000004

#define TX_FES_STATUS_PROT_RESERVED_0_OFFSET                                        0x00000000
#define TX_FES_STATUS_PROT_RESERVED_0_LSB                                           3
#define TX_FES_STATUS_PROT_RESERVED_0_MSB                                           22
#define TX_FES_STATUS_PROT_RESERVED_0_MASK                                          0x007ffff8

#define TX_FES_STATUS_PROT_PKT_TYPE_OFFSET                                          0x00000000
#define TX_FES_STATUS_PROT_PKT_TYPE_LSB                                             23
#define TX_FES_STATUS_PROT_PKT_TYPE_MSB                                             26
#define TX_FES_STATUS_PROT_PKT_TYPE_MASK                                            0x07800000

#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_OFFSET                               0x00000000
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_LSB                                  27
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_MSB                                  27
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_MASK                                 0x08000000

#define TX_FES_STATUS_PROT_RATE_MCS_OFFSET                                          0x00000000
#define TX_FES_STATUS_PROT_RATE_MCS_LSB                                             28
#define TX_FES_STATUS_PROT_RATE_MCS_MSB                                             31
#define TX_FES_STATUS_PROT_RATE_MCS_MASK                                            0xf0000000

#define TX_FES_STATUS_PROT_FRAME_TYPE_OFFSET                                        0x00000004
#define TX_FES_STATUS_PROT_FRAME_TYPE_LSB                                           0
#define TX_FES_STATUS_PROT_FRAME_TYPE_MSB                                           1
#define TX_FES_STATUS_PROT_FRAME_TYPE_MASK                                          0x00000003

#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_OFFSET                                     0x00000004
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_LSB                                        2
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_MSB                                        5
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_MASK                                       0x0000003c

#define TX_FES_STATUS_PROT_RX_PWR_MGMT_OFFSET                                       0x00000004
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_LSB                                          6
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_MSB                                          6
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_MASK                                         0x00000040

#define TX_FES_STATUS_PROT_STATUS_OFFSET                                            0x00000004
#define TX_FES_STATUS_PROT_STATUS_LSB                                               7
#define TX_FES_STATUS_PROT_STATUS_MSB                                               7
#define TX_FES_STATUS_PROT_STATUS_MASK                                              0x00000080

#define TX_FES_STATUS_PROT_DURATION_FIELD_OFFSET                                    0x00000004
#define TX_FES_STATUS_PROT_DURATION_FIELD_LSB                                       8
#define TX_FES_STATUS_PROT_DURATION_FIELD_MSB                                       23
#define TX_FES_STATUS_PROT_DURATION_FIELD_MASK                                      0x00ffff00

#define TX_FES_STATUS_PROT_RESERVED_1A_OFFSET                                       0x00000004
#define TX_FES_STATUS_PROT_RESERVED_1A_LSB                                          24
#define TX_FES_STATUS_PROT_RESERVED_1A_MSB                                          25
#define TX_FES_STATUS_PROT_RESERVED_1A_MASK                                         0x03000000

#define TX_FES_STATUS_PROT_AGC_CBW_OFFSET                                           0x00000004
#define TX_FES_STATUS_PROT_AGC_CBW_LSB                                              26
#define TX_FES_STATUS_PROT_AGC_CBW_MSB                                              28
#define TX_FES_STATUS_PROT_AGC_CBW_MASK                                             0x1c000000

#define TX_FES_STATUS_PROT_SERVICE_CBW_OFFSET                                       0x00000004
#define TX_FES_STATUS_PROT_SERVICE_CBW_LSB                                          29
#define TX_FES_STATUS_PROT_SERVICE_CBW_MSB                                          31
#define TX_FES_STATUS_PROT_SERVICE_CBW_MASK                                         0xe0000000

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x00000008
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB  0
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB  7
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK 0x000000ff

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET      0x00000008
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB         8
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB         13
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK        0x00003f00

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET         0x00000008
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB            14
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB            15
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK           0x0000c000

#define TX_FES_STATUS_PROT_RESERVED_2A_OFFSET                                       0x00000008
#define TX_FES_STATUS_PROT_RESERVED_2A_LSB                                          16
#define TX_FES_STATUS_PROT_RESERVED_2A_MSB                                          31
#define TX_FES_STATUS_PROT_RESERVED_2A_MASK                                         0xffff0000

#endif

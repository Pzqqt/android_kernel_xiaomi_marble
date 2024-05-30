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


#ifndef _TX_FLUSH_REQ_H_
#define _TX_FLUSH_REQ_H_

#define NUM_OF_DWORDS_TX_FLUSH_REQ 1

struct tx_flush_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t flush_req_reason                                        :  8,
                      phytx_abort_reason                                      :  8,
                      flush_req_user_number_or_link_id                        :  6,
                      mlo_abort_reason                                        :  5,
                      reserved_0a                                             :  5;
#else
             uint32_t reserved_0a                                             :  5,
                      mlo_abort_reason                                        :  5,
                      flush_req_user_number_or_link_id                        :  6,
                      phytx_abort_reason                                      :  8,
                      flush_req_reason                                        :  8;
#endif
};

#define TX_FLUSH_REQ_FLUSH_REQ_REASON_OFFSET                                        0x00000000
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_LSB                                           0
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_MSB                                           7
#define TX_FLUSH_REQ_FLUSH_REQ_REASON_MASK                                          0x000000ff

#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_OFFSET                                      0x00000000
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_LSB                                         8
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_MSB                                         15
#define TX_FLUSH_REQ_PHYTX_ABORT_REASON_MASK                                        0x0000ff00

#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_OFFSET                        0x00000000
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_LSB                           16
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_MSB                           21
#define TX_FLUSH_REQ_FLUSH_REQ_USER_NUMBER_OR_LINK_ID_MASK                          0x003f0000

#define TX_FLUSH_REQ_MLO_ABORT_REASON_OFFSET                                        0x00000000
#define TX_FLUSH_REQ_MLO_ABORT_REASON_LSB                                           22
#define TX_FLUSH_REQ_MLO_ABORT_REASON_MSB                                           26
#define TX_FLUSH_REQ_MLO_ABORT_REASON_MASK                                          0x07c00000

#define TX_FLUSH_REQ_RESERVED_0A_OFFSET                                             0x00000000
#define TX_FLUSH_REQ_RESERVED_0A_LSB                                                27
#define TX_FLUSH_REQ_RESERVED_0A_MSB                                                31
#define TX_FLUSH_REQ_RESERVED_0A_MASK                                               0xf8000000

#endif

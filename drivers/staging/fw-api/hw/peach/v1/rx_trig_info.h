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


#ifndef _RX_TRIG_INFO_H_
#define _RX_TRIG_INFO_H_

#define NUM_OF_DWORDS_RX_TRIG_INFO 2

struct rx_trig_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_trigger_frame_type                                   :  2,
                      trigger_resp_type                                       :  3,
                      reserved_0                                              : 27;
             uint32_t ppdu_duration                                           : 16,
                      unique_destination_id                                   : 16;
#else
             uint32_t reserved_0                                              : 27,
                      trigger_resp_type                                       :  3,
                      rx_trigger_frame_type                                   :  2;
             uint32_t unique_destination_id                                   : 16,
                      ppdu_duration                                           : 16;
#endif
};

#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_OFFSET                                   0x00000000
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_LSB                                      0
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_MSB                                      1
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_MASK                                     0x00000003

#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_OFFSET                                       0x00000000
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_LSB                                          2
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_MSB                                          4
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_MASK                                         0x0000001c

#define RX_TRIG_INFO_RESERVED_0_OFFSET                                              0x00000000
#define RX_TRIG_INFO_RESERVED_0_LSB                                                 5
#define RX_TRIG_INFO_RESERVED_0_MSB                                                 31
#define RX_TRIG_INFO_RESERVED_0_MASK                                                0xffffffe0

#define RX_TRIG_INFO_PPDU_DURATION_OFFSET                                           0x00000004
#define RX_TRIG_INFO_PPDU_DURATION_LSB                                              0
#define RX_TRIG_INFO_PPDU_DURATION_MSB                                              15
#define RX_TRIG_INFO_PPDU_DURATION_MASK                                             0x0000ffff

#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_OFFSET                                   0x00000004
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_LSB                                      16
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_MSB                                      31
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_MASK                                     0xffff0000

#endif

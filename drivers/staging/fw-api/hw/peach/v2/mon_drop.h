/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _MON_DROP_H_
#define _MON_DROP_H_

#define NUM_OF_DWORDS_MON_DROP 2

struct mon_drop {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ppdu_id                                                 : 32;
             uint32_t ppdu_drop_cnt                                           : 10,
                      mpdu_drop_cnt                                           : 10,
                      tlv_drop_cnt                                            : 10,
                      end_of_ppdu_seen                                        :  1,
                      reserved_1a                                             :  1;
#else
             uint32_t ppdu_id                                                 : 32;
             uint32_t reserved_1a                                             :  1,
                      end_of_ppdu_seen                                        :  1,
                      tlv_drop_cnt                                            : 10,
                      mpdu_drop_cnt                                           : 10,
                      ppdu_drop_cnt                                           : 10;
#endif
};

#define MON_DROP_PPDU_ID_OFFSET                                                     0x00000000
#define MON_DROP_PPDU_ID_LSB                                                        0
#define MON_DROP_PPDU_ID_MSB                                                        31
#define MON_DROP_PPDU_ID_MASK                                                       0xffffffff

#define MON_DROP_PPDU_DROP_CNT_OFFSET                                               0x00000004
#define MON_DROP_PPDU_DROP_CNT_LSB                                                  0
#define MON_DROP_PPDU_DROP_CNT_MSB                                                  9
#define MON_DROP_PPDU_DROP_CNT_MASK                                                 0x000003ff

#define MON_DROP_MPDU_DROP_CNT_OFFSET                                               0x00000004
#define MON_DROP_MPDU_DROP_CNT_LSB                                                  10
#define MON_DROP_MPDU_DROP_CNT_MSB                                                  19
#define MON_DROP_MPDU_DROP_CNT_MASK                                                 0x000ffc00

#define MON_DROP_TLV_DROP_CNT_OFFSET                                                0x00000004
#define MON_DROP_TLV_DROP_CNT_LSB                                                   20
#define MON_DROP_TLV_DROP_CNT_MSB                                                   29
#define MON_DROP_TLV_DROP_CNT_MASK                                                  0x3ff00000

#define MON_DROP_END_OF_PPDU_SEEN_OFFSET                                            0x00000004
#define MON_DROP_END_OF_PPDU_SEEN_LSB                                               30
#define MON_DROP_END_OF_PPDU_SEEN_MSB                                               30
#define MON_DROP_END_OF_PPDU_SEEN_MASK                                              0x40000000

#define MON_DROP_RESERVED_1A_OFFSET                                                 0x00000004
#define MON_DROP_RESERVED_1A_LSB                                                    31
#define MON_DROP_RESERVED_1A_MSB                                                    31
#define MON_DROP_RESERVED_1A_MASK                                                   0x80000000

#endif

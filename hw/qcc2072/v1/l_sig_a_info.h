/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _L_SIG_A_INFO_H_
#define _L_SIG_A_INFO_H_

#define NUM_OF_DWORDS_L_SIG_A_INFO 1

struct l_sig_a_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rate                                                    :  4,
                      lsig_reserved                                           :  1,
                      length                                                  : 12,
                      parity                                                  :  1,
                      tail                                                    :  6,
                      pkt_type                                                :  4,
                      captured_implicit_sounding                              :  1,
                      reserved                                                :  2,
                      rx_integrity_check_passed                               :  1;
#else
             uint32_t rx_integrity_check_passed                               :  1,
                      reserved                                                :  2,
                      captured_implicit_sounding                              :  1,
                      pkt_type                                                :  4,
                      tail                                                    :  6,
                      parity                                                  :  1,
                      length                                                  : 12,
                      lsig_reserved                                           :  1,
                      rate                                                    :  4;
#endif
};

#define L_SIG_A_INFO_RATE_OFFSET                                                    0x00000000
#define L_SIG_A_INFO_RATE_LSB                                                       0
#define L_SIG_A_INFO_RATE_MSB                                                       3
#define L_SIG_A_INFO_RATE_MASK                                                      0x0000000f

#define L_SIG_A_INFO_LSIG_RESERVED_OFFSET                                           0x00000000
#define L_SIG_A_INFO_LSIG_RESERVED_LSB                                              4
#define L_SIG_A_INFO_LSIG_RESERVED_MSB                                              4
#define L_SIG_A_INFO_LSIG_RESERVED_MASK                                             0x00000010

#define L_SIG_A_INFO_LENGTH_OFFSET                                                  0x00000000
#define L_SIG_A_INFO_LENGTH_LSB                                                     5
#define L_SIG_A_INFO_LENGTH_MSB                                                     16
#define L_SIG_A_INFO_LENGTH_MASK                                                    0x0001ffe0

#define L_SIG_A_INFO_PARITY_OFFSET                                                  0x00000000
#define L_SIG_A_INFO_PARITY_LSB                                                     17
#define L_SIG_A_INFO_PARITY_MSB                                                     17
#define L_SIG_A_INFO_PARITY_MASK                                                    0x00020000

#define L_SIG_A_INFO_TAIL_OFFSET                                                    0x00000000
#define L_SIG_A_INFO_TAIL_LSB                                                       18
#define L_SIG_A_INFO_TAIL_MSB                                                       23
#define L_SIG_A_INFO_TAIL_MASK                                                      0x00fc0000

#define L_SIG_A_INFO_PKT_TYPE_OFFSET                                                0x00000000
#define L_SIG_A_INFO_PKT_TYPE_LSB                                                   24
#define L_SIG_A_INFO_PKT_TYPE_MSB                                                   27
#define L_SIG_A_INFO_PKT_TYPE_MASK                                                  0x0f000000

#define L_SIG_A_INFO_CAPTURED_IMPLICIT_SOUNDING_OFFSET                              0x00000000
#define L_SIG_A_INFO_CAPTURED_IMPLICIT_SOUNDING_LSB                                 28
#define L_SIG_A_INFO_CAPTURED_IMPLICIT_SOUNDING_MSB                                 28
#define L_SIG_A_INFO_CAPTURED_IMPLICIT_SOUNDING_MASK                                0x10000000

#define L_SIG_A_INFO_RESERVED_OFFSET                                                0x00000000
#define L_SIG_A_INFO_RESERVED_LSB                                                   29
#define L_SIG_A_INFO_RESERVED_MSB                                                   30
#define L_SIG_A_INFO_RESERVED_MASK                                                  0x60000000

#define L_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                               0x00000000
#define L_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                                  31
#define L_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                                  31
#define L_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                                 0x80000000

#endif

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


#ifndef _MACTX_L_SIG_B_H_
#define _MACTX_L_SIG_B_H_

#include "l_sig_b_info.h"
#define NUM_OF_DWORDS_MACTX_L_SIG_B 1

struct mactx_l_sig_b {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   l_sig_b_info                                              mactx_l_sig_b_info_details;
#else
             struct   l_sig_b_info                                              mactx_l_sig_b_info_details;
#endif
};

#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RATE_OFFSET                        0x00000000
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RATE_LSB                           0
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RATE_MSB                           3
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RATE_MASK                          0x0000000f

#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_LENGTH_OFFSET                      0x00000000
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_LENGTH_LSB                         4
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_LENGTH_MSB                         15
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_LENGTH_MASK                        0x0000fff0

#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RESERVED_OFFSET                    0x00000000
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RESERVED_LSB                       16
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RESERVED_MSB                       30
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RESERVED_MASK                      0x7fff0000

#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_OFFSET   0x00000000
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_LSB      31
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MSB      31
#define MACTX_L_SIG_B_MACTX_L_SIG_B_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MASK     0x80000000

#endif

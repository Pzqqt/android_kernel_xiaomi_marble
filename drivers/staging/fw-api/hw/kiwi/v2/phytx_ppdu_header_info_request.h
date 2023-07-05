
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



#ifndef _PHYTX_PPDU_HEADER_INFO_REQUEST_H_
#define _PHYTX_PPDU_HEADER_INFO_REQUEST_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_WORDS_PHYTX_PPDU_HEADER_INFO_REQUEST 2

#define NUM_OF_DWORDS_PHYTX_PPDU_HEADER_INFO_REQUEST 1

struct phytx_ppdu_header_info_request {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t request_type                                            :  5,
                      reserved                                                : 11;
             uint16_t tlv32_padding                                           : 16;
#else
             uint16_t reserved                                                : 11,
                      request_type                                            :  5;
             uint16_t tlv32_padding                                           : 16;
#endif
};

#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_OFFSET                          0x00000000
#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_LSB                             0
#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_MSB                             4
#define PHYTX_PPDU_HEADER_INFO_REQUEST_REQUEST_TYPE_MASK                            0x0000001f

#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_OFFSET                              0x00000000
#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_LSB                                 5
#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_MSB                                 15
#define PHYTX_PPDU_HEADER_INFO_REQUEST_RESERVED_MASK                                0x0000ffe0

#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_OFFSET                         0x00000002
#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_LSB                            0
#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_MSB                            15
#define PHYTX_PPDU_HEADER_INFO_REQUEST_TLV32_PADDING_MASK                           0x0000ffff

#endif

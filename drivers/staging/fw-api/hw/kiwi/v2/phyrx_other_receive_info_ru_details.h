
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

 

#ifndef _PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_H_
#define _PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS 4

#define NUM_OF_QWORDS_PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS 2

struct phyrx_other_receive_info_ru_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ru_details_channel_0                                    : 32;  
             uint32_t ru_details_channel_1                                    : 32;  
             uint32_t spare                                                   : 32;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t ru_details_channel_0                                    : 32;  
             uint32_t ru_details_channel_1                                    : 32;  
             uint32_t spare                                                   : 32;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};

#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_0_OFFSET             0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_0_LSB                0
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_0_MSB                31
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_0_MASK               0x00000000ffffffff

#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_1_OFFSET             0x0000000000000000
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_1_LSB                32
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_1_MSB                63
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_RU_DETAILS_CHANNEL_1_MASK               0xffffffff00000000

#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_SPARE_OFFSET                            0x0000000000000008
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_SPARE_LSB                               0
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_SPARE_MSB                               31
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_SPARE_MASK                              0x00000000ffffffff

#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_TLV64_PADDING_OFFSET                    0x0000000000000008
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_TLV64_PADDING_LSB                       32
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_TLV64_PADDING_MSB                       63
#define PHYRX_OTHER_RECEIVE_INFO_RU_DETAILS_TLV64_PADDING_MASK                      0xffffffff00000000

#endif

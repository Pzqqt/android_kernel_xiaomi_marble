
/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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











#ifndef _RX_MSDU_EXT_DESC_INFO_H_
#define _RX_MSDU_EXT_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_EXT_DESC_INFO 1


struct rx_msdu_ext_desc_info {
	     uint32_t reo_destination_indication                              :  5,
		      service_code                                            :  9,
		      priority_valid                                          :  1,
		      data_offset                                             : 12,
		      reserved_0a                                             :  5;
};




#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_OFFSET                     0x00000000
#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_LSB                        0
#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_MSB                        4
#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_MASK                       0x0000001f




#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_OFFSET                                   0x00000000
#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_LSB                                      5
#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_MSB                                      13
#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_MASK                                     0x00003fe0




#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_OFFSET                                 0x00000000
#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_LSB                                    14
#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_MSB                                    14
#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_MASK                                   0x00004000




#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_OFFSET                                    0x00000000
#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_LSB                                       15
#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_MSB                                       26
#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_MASK                                      0x07ff8000




#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_OFFSET                                    0x00000000
#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_LSB                                       27
#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_MSB                                       31
#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_MASK                                      0xf8000000



#endif

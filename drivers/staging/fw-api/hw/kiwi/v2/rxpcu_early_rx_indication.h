
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



#ifndef _RXPCU_EARLY_RX_INDICATION_H_
#define _RXPCU_EARLY_RX_INDICATION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RXPCU_EARLY_RX_INDICATION 2

#define NUM_OF_QWORDS_RXPCU_EARLY_RX_INDICATION 1

struct rxpcu_early_rx_indication {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t pkt_type                                                :  4,
                      dot11ax_su_extended                                     :  1,
                      rate_mcs                                                :  4,
                      dot11ax_received_ext_ru_size                            :  4,
                      reserved_0a                                             : 19;
             uint32_t tlv64_padding                                           : 32;
#else
             uint32_t reserved_0a                                             : 19,
                      dot11ax_received_ext_ru_size                            :  4,
                      rate_mcs                                                :  4,
                      dot11ax_su_extended                                     :  1,
                      pkt_type                                                :  4;
             uint32_t tlv64_padding                                           : 32;
#endif
};

#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_OFFSET                                   0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_LSB                                      0
#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_MSB                                      3
#define RXPCU_EARLY_RX_INDICATION_PKT_TYPE_MASK                                     0x000000000000000f

#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_OFFSET                        0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_LSB                           4
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_MSB                           4
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_SU_EXTENDED_MASK                          0x0000000000000010

#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_OFFSET                                   0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_LSB                                      5
#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_MSB                                      8
#define RXPCU_EARLY_RX_INDICATION_RATE_MCS_MASK                                     0x00000000000001e0

#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_OFFSET               0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_LSB                  9
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_MSB                  12
#define RXPCU_EARLY_RX_INDICATION_DOT11AX_RECEIVED_EXT_RU_SIZE_MASK                 0x0000000000001e00

#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_OFFSET                                0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_LSB                                   13
#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_MSB                                   31
#define RXPCU_EARLY_RX_INDICATION_RESERVED_0A_MASK                                  0x00000000ffffe000

#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_OFFSET                              0x0000000000000000
#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_LSB                                 32
#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_MSB                                 63
#define RXPCU_EARLY_RX_INDICATION_TLV64_PADDING_MASK                                0xffffffff00000000

#endif

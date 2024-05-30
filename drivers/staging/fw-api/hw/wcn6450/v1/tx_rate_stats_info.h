
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef _TX_RATE_STATS_INFO_H_
#define _TX_RATE_STATS_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_RATE_STATS_INFO 2

struct tx_rate_stats_info {
             uint32_t tx_rate_stats_info_valid        :  1,
                      transmit_bw                     :  2,
                      transmit_pkt_type               :  4,
                      transmit_stbc                   :  1,
                      transmit_ldpc                   :  1,
                      transmit_sgi                    :  2,
                      transmit_mcs                    :  4,
                      ofdma_transmission              :  1,
                      tones_in_ru                     : 12,
                      reserved_0a                     :  4;
             uint32_t ppdu_transmission_tsf           : 32;
};

#define TX_RATE_STATS_INFO_0_TX_RATE_STATS_INFO_VALID_OFFSET         0x00000000
#define TX_RATE_STATS_INFO_0_TX_RATE_STATS_INFO_VALID_LSB            0
#define TX_RATE_STATS_INFO_0_TX_RATE_STATS_INFO_VALID_MASK           0x00000001

#define TX_RATE_STATS_INFO_0_TRANSMIT_BW_OFFSET                      0x00000000
#define TX_RATE_STATS_INFO_0_TRANSMIT_BW_LSB                         1
#define TX_RATE_STATS_INFO_0_TRANSMIT_BW_MASK                        0x00000006

#define TX_RATE_STATS_INFO_0_TRANSMIT_PKT_TYPE_OFFSET                0x00000000
#define TX_RATE_STATS_INFO_0_TRANSMIT_PKT_TYPE_LSB                   3
#define TX_RATE_STATS_INFO_0_TRANSMIT_PKT_TYPE_MASK                  0x00000078

#define TX_RATE_STATS_INFO_0_TRANSMIT_STBC_OFFSET                    0x00000000
#define TX_RATE_STATS_INFO_0_TRANSMIT_STBC_LSB                       7
#define TX_RATE_STATS_INFO_0_TRANSMIT_STBC_MASK                      0x00000080

#define TX_RATE_STATS_INFO_0_TRANSMIT_LDPC_OFFSET                    0x00000000
#define TX_RATE_STATS_INFO_0_TRANSMIT_LDPC_LSB                       8
#define TX_RATE_STATS_INFO_0_TRANSMIT_LDPC_MASK                      0x00000100

#define TX_RATE_STATS_INFO_0_TRANSMIT_SGI_OFFSET                     0x00000000
#define TX_RATE_STATS_INFO_0_TRANSMIT_SGI_LSB                        9
#define TX_RATE_STATS_INFO_0_TRANSMIT_SGI_MASK                       0x00000600

#define TX_RATE_STATS_INFO_0_TRANSMIT_MCS_OFFSET                     0x00000000
#define TX_RATE_STATS_INFO_0_TRANSMIT_MCS_LSB                        11
#define TX_RATE_STATS_INFO_0_TRANSMIT_MCS_MASK                       0x00007800

#define TX_RATE_STATS_INFO_0_OFDMA_TRANSMISSION_OFFSET               0x00000000
#define TX_RATE_STATS_INFO_0_OFDMA_TRANSMISSION_LSB                  15
#define TX_RATE_STATS_INFO_0_OFDMA_TRANSMISSION_MASK                 0x00008000

#define TX_RATE_STATS_INFO_0_TONES_IN_RU_OFFSET                      0x00000000
#define TX_RATE_STATS_INFO_0_TONES_IN_RU_LSB                         16
#define TX_RATE_STATS_INFO_0_TONES_IN_RU_MASK                        0x0fff0000

#define TX_RATE_STATS_INFO_0_RESERVED_0A_OFFSET                      0x00000000
#define TX_RATE_STATS_INFO_0_RESERVED_0A_LSB                         28
#define TX_RATE_STATS_INFO_0_RESERVED_0A_MASK                        0xf0000000

#define TX_RATE_STATS_INFO_1_PPDU_TRANSMISSION_TSF_OFFSET            0x00000004
#define TX_RATE_STATS_INFO_1_PPDU_TRANSMISSION_TSF_LSB               0
#define TX_RATE_STATS_INFO_1_PPDU_TRANSMISSION_TSF_MASK              0xffffffff

#endif

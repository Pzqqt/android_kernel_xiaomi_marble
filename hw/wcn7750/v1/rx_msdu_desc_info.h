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


#ifndef _RX_MSDU_DESC_INFO_H_
#define _RX_MSDU_DESC_INFO_H_

#define NUM_OF_DWORDS_RX_MSDU_DESC_INFO 1

struct rx_msdu_desc_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t first_msdu_in_mpdu_flag                                 :  1,
                      last_msdu_in_mpdu_flag                                  :  1,
                      msdu_continuation                                       :  1,
                      msdu_length                                             : 14,
                      msdu_drop                                               :  1,
                      sa_is_valid                                             :  1,
                      da_is_valid                                             :  1,
                      da_is_mcbc                                              :  1,
                      l3_header_padding_msb                                   :  1,
                      tcp_udp_chksum_fail                                     :  1,
                      ip_chksum_fail                                          :  1,
                      fr_ds                                                   :  1,
                      to_ds                                                   :  1,
                      intra_bss                                               :  1,
                      dest_chip_id                                            :  2,
                      decap_format                                            :  2,
                      reserved_0a                                             :  1;
#else
             uint32_t reserved_0a                                             :  1,
                      decap_format                                            :  2,
                      dest_chip_id                                            :  2,
                      intra_bss                                               :  1,
                      to_ds                                                   :  1,
                      fr_ds                                                   :  1,
                      ip_chksum_fail                                          :  1,
                      tcp_udp_chksum_fail                                     :  1,
                      l3_header_padding_msb                                   :  1,
                      da_is_mcbc                                              :  1,
                      da_is_valid                                             :  1,
                      sa_is_valid                                             :  1,
                      msdu_drop                                               :  1,
                      msdu_length                                             : 14,
                      msdu_continuation                                       :  1,
                      last_msdu_in_mpdu_flag                                  :  1,
                      first_msdu_in_mpdu_flag                                 :  1;
#endif
};

#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_OFFSET                            0x00000000
#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_LSB                               0
#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MSB                               0
#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MASK                              0x00000001

#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_OFFSET                             0x00000000
#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_LSB                                1
#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MSB                                1
#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MASK                               0x00000002

#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_OFFSET                                  0x00000000
#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_LSB                                     2
#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_MSB                                     2
#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_MASK                                    0x00000004

#define RX_MSDU_DESC_INFO_MSDU_LENGTH_OFFSET                                        0x00000000
#define RX_MSDU_DESC_INFO_MSDU_LENGTH_LSB                                           3
#define RX_MSDU_DESC_INFO_MSDU_LENGTH_MSB                                           16
#define RX_MSDU_DESC_INFO_MSDU_LENGTH_MASK                                          0x0001fff8

#define RX_MSDU_DESC_INFO_MSDU_DROP_OFFSET                                          0x00000000
#define RX_MSDU_DESC_INFO_MSDU_DROP_LSB                                             17
#define RX_MSDU_DESC_INFO_MSDU_DROP_MSB                                             17
#define RX_MSDU_DESC_INFO_MSDU_DROP_MASK                                            0x00020000

#define RX_MSDU_DESC_INFO_SA_IS_VALID_OFFSET                                        0x00000000
#define RX_MSDU_DESC_INFO_SA_IS_VALID_LSB                                           18
#define RX_MSDU_DESC_INFO_SA_IS_VALID_MSB                                           18
#define RX_MSDU_DESC_INFO_SA_IS_VALID_MASK                                          0x00040000

#define RX_MSDU_DESC_INFO_DA_IS_VALID_OFFSET                                        0x00000000
#define RX_MSDU_DESC_INFO_DA_IS_VALID_LSB                                           19
#define RX_MSDU_DESC_INFO_DA_IS_VALID_MSB                                           19
#define RX_MSDU_DESC_INFO_DA_IS_VALID_MASK                                          0x00080000

#define RX_MSDU_DESC_INFO_DA_IS_MCBC_OFFSET                                         0x00000000
#define RX_MSDU_DESC_INFO_DA_IS_MCBC_LSB                                            20
#define RX_MSDU_DESC_INFO_DA_IS_MCBC_MSB                                            20
#define RX_MSDU_DESC_INFO_DA_IS_MCBC_MASK                                           0x00100000

#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_OFFSET                              0x00000000
#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_LSB                                 21
#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_MSB                                 21
#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_MASK                                0x00200000

#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_OFFSET                                0x00000000
#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_LSB                                   22
#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_MSB                                   22
#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_MASK                                  0x00400000

#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_OFFSET                                     0x00000000
#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_LSB                                        23
#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_MSB                                        23
#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_MASK                                       0x00800000

#define RX_MSDU_DESC_INFO_FR_DS_OFFSET                                              0x00000000
#define RX_MSDU_DESC_INFO_FR_DS_LSB                                                 24
#define RX_MSDU_DESC_INFO_FR_DS_MSB                                                 24
#define RX_MSDU_DESC_INFO_FR_DS_MASK                                                0x01000000

#define RX_MSDU_DESC_INFO_TO_DS_OFFSET                                              0x00000000
#define RX_MSDU_DESC_INFO_TO_DS_LSB                                                 25
#define RX_MSDU_DESC_INFO_TO_DS_MSB                                                 25
#define RX_MSDU_DESC_INFO_TO_DS_MASK                                                0x02000000

#define RX_MSDU_DESC_INFO_INTRA_BSS_OFFSET                                          0x00000000
#define RX_MSDU_DESC_INFO_INTRA_BSS_LSB                                             26
#define RX_MSDU_DESC_INFO_INTRA_BSS_MSB                                             26
#define RX_MSDU_DESC_INFO_INTRA_BSS_MASK                                            0x04000000

#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_OFFSET                                       0x00000000
#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_LSB                                          27
#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_MSB                                          28
#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_MASK                                         0x18000000

#define RX_MSDU_DESC_INFO_DECAP_FORMAT_OFFSET                                       0x00000000
#define RX_MSDU_DESC_INFO_DECAP_FORMAT_LSB                                          29
#define RX_MSDU_DESC_INFO_DECAP_FORMAT_MSB                                          30
#define RX_MSDU_DESC_INFO_DECAP_FORMAT_MASK                                         0x60000000

#endif

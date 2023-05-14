
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



#ifndef _PDG_TX_REQ_H_
#define _PDG_TX_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PDG_TX_REQ 2

#define NUM_OF_QWORDS_PDG_TX_REQ 1

struct pdg_tx_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tx_reason                                               :  2,
                      use_puncture_pattern                                    :  2,
                      req_bw                                                  :  3,
                      puncture_pattern_number                                 :  6,
                      reserved_0b                                             :  1,
                      req_paprd                                               :  1,
                      duration_field_boundary_valid                           :  1,
                      duration_field_boundary                                 : 16;
             uint32_t puncture_subband_mask                                   : 16,
                      reserved_0c                                             : 16;
#else
             uint32_t duration_field_boundary                                 : 16,
                      duration_field_boundary_valid                           :  1,
                      req_paprd                                               :  1,
                      reserved_0b                                             :  1,
                      puncture_pattern_number                                 :  6,
                      req_bw                                                  :  3,
                      use_puncture_pattern                                    :  2,
                      tx_reason                                               :  2;
             uint32_t reserved_0c                                             : 16,
                      puncture_subband_mask                                   : 16;
#endif
};

#define PDG_TX_REQ_TX_REASON_OFFSET                                                 0x0000000000000000
#define PDG_TX_REQ_TX_REASON_LSB                                                    0
#define PDG_TX_REQ_TX_REASON_MSB                                                    1
#define PDG_TX_REQ_TX_REASON_MASK                                                   0x0000000000000003

#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_OFFSET                                      0x0000000000000000
#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_LSB                                         2
#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_MSB                                         3
#define PDG_TX_REQ_USE_PUNCTURE_PATTERN_MASK                                        0x000000000000000c

#define PDG_TX_REQ_REQ_BW_OFFSET                                                    0x0000000000000000
#define PDG_TX_REQ_REQ_BW_LSB                                                       4
#define PDG_TX_REQ_REQ_BW_MSB                                                       6
#define PDG_TX_REQ_REQ_BW_MASK                                                      0x0000000000000070

#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_OFFSET                                   0x0000000000000000
#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_LSB                                      7
#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_MSB                                      12
#define PDG_TX_REQ_PUNCTURE_PATTERN_NUMBER_MASK                                     0x0000000000001f80

#define PDG_TX_REQ_RESERVED_0B_OFFSET                                               0x0000000000000000
#define PDG_TX_REQ_RESERVED_0B_LSB                                                  13
#define PDG_TX_REQ_RESERVED_0B_MSB                                                  13
#define PDG_TX_REQ_RESERVED_0B_MASK                                                 0x0000000000002000

#define PDG_TX_REQ_REQ_PAPRD_OFFSET                                                 0x0000000000000000
#define PDG_TX_REQ_REQ_PAPRD_LSB                                                    14
#define PDG_TX_REQ_REQ_PAPRD_MSB                                                    14
#define PDG_TX_REQ_REQ_PAPRD_MASK                                                   0x0000000000004000

#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_OFFSET                             0x0000000000000000
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_LSB                                15
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_MSB                                15
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_VALID_MASK                               0x0000000000008000

#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_OFFSET                                   0x0000000000000000
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_LSB                                      16
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_MSB                                      31
#define PDG_TX_REQ_DURATION_FIELD_BOUNDARY_MASK                                     0x00000000ffff0000

#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_OFFSET                                     0x0000000000000000
#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_LSB                                        32
#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_MSB                                        47
#define PDG_TX_REQ_PUNCTURE_SUBBAND_MASK_MASK                                       0x0000ffff00000000

#define PDG_TX_REQ_RESERVED_0C_OFFSET                                               0x0000000000000000
#define PDG_TX_REQ_RESERVED_0C_LSB                                                  48
#define PDG_TX_REQ_RESERVED_0C_MSB                                                  63
#define PDG_TX_REQ_RESERVED_0C_MASK                                                 0xffff000000000000

#endif


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



#ifndef _RXPT_CLASSIFY_INFO_H_
#define _RXPT_CLASSIFY_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RXPT_CLASSIFY_INFO 1

struct rxpt_classify_info {
             uint32_t reo_destination_indication      :  5,
                      lmac_peer_id_msb                :  2,
                      use_flow_id_toeplitz_clfy       :  1,
                      pkt_selection_fp_ucast_data     :  1,
                      pkt_selection_fp_mcast_data     :  1,
                      pkt_selection_fp_1000           :  1,
                      rxdma0_source_ring_selection    :  2,
                      rxdma0_destination_ring_selection:  2,
                      reserved_0b                     : 17;
};

#define RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION_OFFSET       0x00000000
#define RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION_LSB          0
#define RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION_MASK         0x0000001f

#define RXPT_CLASSIFY_INFO_0_LMAC_PEER_ID_MSB_OFFSET                 0x00000000
#define RXPT_CLASSIFY_INFO_0_LMAC_PEER_ID_MSB_LSB                    5
#define RXPT_CLASSIFY_INFO_0_LMAC_PEER_ID_MSB_MASK                   0x00000060

#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET        0x00000000
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_LSB           7
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_MASK          0x00000080

#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA_OFFSET      0x00000000
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA_LSB         8
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA_MASK        0x00000100

#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA_OFFSET      0x00000000
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA_LSB         9
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA_MASK        0x00000200

#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000_OFFSET            0x00000000
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000_LSB               10
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000_MASK              0x00000400

#define RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION_OFFSET     0x00000000
#define RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION_LSB        11
#define RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION_MASK       0x00001800

#define RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION_OFFSET 0x00000000
#define RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION_LSB   13
#define RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION_MASK  0x00006000

#define RXPT_CLASSIFY_INFO_0_RESERVED_0B_OFFSET                      0x00000000
#define RXPT_CLASSIFY_INFO_0_RESERVED_0B_LSB                         15
#define RXPT_CLASSIFY_INFO_0_RESERVED_0B_MASK                        0xffff8000

#endif

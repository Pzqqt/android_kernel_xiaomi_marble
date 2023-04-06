
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



#ifndef _RX_LOCATION_INFO_H_
#define _RX_LOCATION_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_LOCATION_INFO 9

struct rx_location_info {
             uint32_t rtt_fac_legacy                  : 16,
                      rtt_fac_legacy_ext80            : 16;
             uint32_t rtt_fac_vht                     : 16,
                      rtt_fac_vht_ext80               : 16;
             uint32_t rtt_fac_legacy_status           :  1,
                      rtt_fac_legacy_ext80_status     :  1,
                      rtt_fac_vht_status              :  1,
                      rtt_fac_vht_ext80_status        :  1,
                      rtt_fac_sifs                    : 12,
                      rtt_fac_sifs_status             :  2,
                      rtt_cfr_status                  :  1,
                      rtt_cir_status                  :  1,
                      rtt_channel_dump_size           : 11,
                      rtt_hw_ifft_mode                :  1;
             uint32_t rtt_btcf_status                 :  1,
                      rtt_preamble_type               :  5,
                      rtt_pkt_bw_leg                  :  2,
                      rtt_pkt_bw_vht                  :  2,
                      rtt_gi_type                     :  2,
                      rtt_mcs_rate                    :  5,
                      rtt_strongest_chain             :  3,
                      rtt_strongest_chain_ext80       :  3,
                      rtt_rx_chain_mask               :  8,
                      reserved_3                      :  1;
             uint32_t rx_start_ts                     : 32;
             uint32_t rx_end_ts                       : 32;
             uint32_t sfo_phase_pkt_start             : 12,
                      sfo_phase_pkt_end               : 12,
                      rtt_che_buffer_pointer_high8    :  8;
             uint32_t rtt_che_buffer_pointer_low32    : 32;
             uint32_t rtt_cfo_measurement             : 14,
                      rtt_chan_spread                 :  8,
                      rtt_timing_backoff_sel          :  2,
                      reserved_8                      :  7,
                      rx_location_info_valid          :  1;
};

#define RX_LOCATION_INFO_0_RTT_FAC_LEGACY_OFFSET                     0x00000000
#define RX_LOCATION_INFO_0_RTT_FAC_LEGACY_LSB                        0
#define RX_LOCATION_INFO_0_RTT_FAC_LEGACY_MASK                       0x0000ffff

#define RX_LOCATION_INFO_0_RTT_FAC_LEGACY_EXT80_OFFSET               0x00000000
#define RX_LOCATION_INFO_0_RTT_FAC_LEGACY_EXT80_LSB                  16
#define RX_LOCATION_INFO_0_RTT_FAC_LEGACY_EXT80_MASK                 0xffff0000

#define RX_LOCATION_INFO_1_RTT_FAC_VHT_OFFSET                        0x00000004
#define RX_LOCATION_INFO_1_RTT_FAC_VHT_LSB                           0
#define RX_LOCATION_INFO_1_RTT_FAC_VHT_MASK                          0x0000ffff

#define RX_LOCATION_INFO_1_RTT_FAC_VHT_EXT80_OFFSET                  0x00000004
#define RX_LOCATION_INFO_1_RTT_FAC_VHT_EXT80_LSB                     16
#define RX_LOCATION_INFO_1_RTT_FAC_VHT_EXT80_MASK                    0xffff0000

#define RX_LOCATION_INFO_2_RTT_FAC_LEGACY_STATUS_OFFSET              0x00000008
#define RX_LOCATION_INFO_2_RTT_FAC_LEGACY_STATUS_LSB                 0
#define RX_LOCATION_INFO_2_RTT_FAC_LEGACY_STATUS_MASK                0x00000001

#define RX_LOCATION_INFO_2_RTT_FAC_LEGACY_EXT80_STATUS_OFFSET        0x00000008
#define RX_LOCATION_INFO_2_RTT_FAC_LEGACY_EXT80_STATUS_LSB           1
#define RX_LOCATION_INFO_2_RTT_FAC_LEGACY_EXT80_STATUS_MASK          0x00000002

#define RX_LOCATION_INFO_2_RTT_FAC_VHT_STATUS_OFFSET                 0x00000008
#define RX_LOCATION_INFO_2_RTT_FAC_VHT_STATUS_LSB                    2
#define RX_LOCATION_INFO_2_RTT_FAC_VHT_STATUS_MASK                   0x00000004

#define RX_LOCATION_INFO_2_RTT_FAC_VHT_EXT80_STATUS_OFFSET           0x00000008
#define RX_LOCATION_INFO_2_RTT_FAC_VHT_EXT80_STATUS_LSB              3
#define RX_LOCATION_INFO_2_RTT_FAC_VHT_EXT80_STATUS_MASK             0x00000008

#define RX_LOCATION_INFO_2_RTT_FAC_SIFS_OFFSET                       0x00000008
#define RX_LOCATION_INFO_2_RTT_FAC_SIFS_LSB                          4
#define RX_LOCATION_INFO_2_RTT_FAC_SIFS_MASK                         0x0000fff0

#define RX_LOCATION_INFO_2_RTT_FAC_SIFS_STATUS_OFFSET                0x00000008
#define RX_LOCATION_INFO_2_RTT_FAC_SIFS_STATUS_LSB                   16
#define RX_LOCATION_INFO_2_RTT_FAC_SIFS_STATUS_MASK                  0x00030000

#define RX_LOCATION_INFO_2_RTT_CFR_STATUS_OFFSET                     0x00000008
#define RX_LOCATION_INFO_2_RTT_CFR_STATUS_LSB                        18
#define RX_LOCATION_INFO_2_RTT_CFR_STATUS_MASK                       0x00040000

#define RX_LOCATION_INFO_2_RTT_CIR_STATUS_OFFSET                     0x00000008
#define RX_LOCATION_INFO_2_RTT_CIR_STATUS_LSB                        19
#define RX_LOCATION_INFO_2_RTT_CIR_STATUS_MASK                       0x00080000

#define RX_LOCATION_INFO_2_RTT_CHANNEL_DUMP_SIZE_OFFSET              0x00000008
#define RX_LOCATION_INFO_2_RTT_CHANNEL_DUMP_SIZE_LSB                 20
#define RX_LOCATION_INFO_2_RTT_CHANNEL_DUMP_SIZE_MASK                0x7ff00000

#define RX_LOCATION_INFO_2_RTT_HW_IFFT_MODE_OFFSET                   0x00000008
#define RX_LOCATION_INFO_2_RTT_HW_IFFT_MODE_LSB                      31
#define RX_LOCATION_INFO_2_RTT_HW_IFFT_MODE_MASK                     0x80000000

#define RX_LOCATION_INFO_3_RTT_BTCF_STATUS_OFFSET                    0x0000000c
#define RX_LOCATION_INFO_3_RTT_BTCF_STATUS_LSB                       0
#define RX_LOCATION_INFO_3_RTT_BTCF_STATUS_MASK                      0x00000001

#define RX_LOCATION_INFO_3_RTT_PREAMBLE_TYPE_OFFSET                  0x0000000c
#define RX_LOCATION_INFO_3_RTT_PREAMBLE_TYPE_LSB                     1
#define RX_LOCATION_INFO_3_RTT_PREAMBLE_TYPE_MASK                    0x0000003e

#define RX_LOCATION_INFO_3_RTT_PKT_BW_LEG_OFFSET                     0x0000000c
#define RX_LOCATION_INFO_3_RTT_PKT_BW_LEG_LSB                        6
#define RX_LOCATION_INFO_3_RTT_PKT_BW_LEG_MASK                       0x000000c0

#define RX_LOCATION_INFO_3_RTT_PKT_BW_VHT_OFFSET                     0x0000000c
#define RX_LOCATION_INFO_3_RTT_PKT_BW_VHT_LSB                        8
#define RX_LOCATION_INFO_3_RTT_PKT_BW_VHT_MASK                       0x00000300

#define RX_LOCATION_INFO_3_RTT_GI_TYPE_OFFSET                        0x0000000c
#define RX_LOCATION_INFO_3_RTT_GI_TYPE_LSB                           10
#define RX_LOCATION_INFO_3_RTT_GI_TYPE_MASK                          0x00000c00

#define RX_LOCATION_INFO_3_RTT_MCS_RATE_OFFSET                       0x0000000c
#define RX_LOCATION_INFO_3_RTT_MCS_RATE_LSB                          12
#define RX_LOCATION_INFO_3_RTT_MCS_RATE_MASK                         0x0001f000

#define RX_LOCATION_INFO_3_RTT_STRONGEST_CHAIN_OFFSET                0x0000000c
#define RX_LOCATION_INFO_3_RTT_STRONGEST_CHAIN_LSB                   17
#define RX_LOCATION_INFO_3_RTT_STRONGEST_CHAIN_MASK                  0x000e0000

#define RX_LOCATION_INFO_3_RTT_STRONGEST_CHAIN_EXT80_OFFSET          0x0000000c
#define RX_LOCATION_INFO_3_RTT_STRONGEST_CHAIN_EXT80_LSB             20
#define RX_LOCATION_INFO_3_RTT_STRONGEST_CHAIN_EXT80_MASK            0x00700000

#define RX_LOCATION_INFO_3_RTT_RX_CHAIN_MASK_OFFSET                  0x0000000c
#define RX_LOCATION_INFO_3_RTT_RX_CHAIN_MASK_LSB                     23
#define RX_LOCATION_INFO_3_RTT_RX_CHAIN_MASK_MASK                    0x7f800000

#define RX_LOCATION_INFO_3_RESERVED_3_OFFSET                         0x0000000c
#define RX_LOCATION_INFO_3_RESERVED_3_LSB                            31
#define RX_LOCATION_INFO_3_RESERVED_3_MASK                           0x80000000

#define RX_LOCATION_INFO_4_RX_START_TS_OFFSET                        0x00000010
#define RX_LOCATION_INFO_4_RX_START_TS_LSB                           0
#define RX_LOCATION_INFO_4_RX_START_TS_MASK                          0xffffffff

#define RX_LOCATION_INFO_5_RX_END_TS_OFFSET                          0x00000014
#define RX_LOCATION_INFO_5_RX_END_TS_LSB                             0
#define RX_LOCATION_INFO_5_RX_END_TS_MASK                            0xffffffff

#define RX_LOCATION_INFO_6_SFO_PHASE_PKT_START_OFFSET                0x00000018
#define RX_LOCATION_INFO_6_SFO_PHASE_PKT_START_LSB                   0
#define RX_LOCATION_INFO_6_SFO_PHASE_PKT_START_MASK                  0x00000fff

#define RX_LOCATION_INFO_6_SFO_PHASE_PKT_END_OFFSET                  0x00000018
#define RX_LOCATION_INFO_6_SFO_PHASE_PKT_END_LSB                     12
#define RX_LOCATION_INFO_6_SFO_PHASE_PKT_END_MASK                    0x00fff000

#define RX_LOCATION_INFO_6_RTT_CHE_BUFFER_POINTER_HIGH8_OFFSET       0x00000018
#define RX_LOCATION_INFO_6_RTT_CHE_BUFFER_POINTER_HIGH8_LSB          24
#define RX_LOCATION_INFO_6_RTT_CHE_BUFFER_POINTER_HIGH8_MASK         0xff000000

#define RX_LOCATION_INFO_7_RTT_CHE_BUFFER_POINTER_LOW32_OFFSET       0x0000001c
#define RX_LOCATION_INFO_7_RTT_CHE_BUFFER_POINTER_LOW32_LSB          0
#define RX_LOCATION_INFO_7_RTT_CHE_BUFFER_POINTER_LOW32_MASK         0xffffffff

#define RX_LOCATION_INFO_8_RTT_CFO_MEASUREMENT_OFFSET                0x00000020
#define RX_LOCATION_INFO_8_RTT_CFO_MEASUREMENT_LSB                   0
#define RX_LOCATION_INFO_8_RTT_CFO_MEASUREMENT_MASK                  0x00003fff

#define RX_LOCATION_INFO_8_RTT_CHAN_SPREAD_OFFSET                    0x00000020
#define RX_LOCATION_INFO_8_RTT_CHAN_SPREAD_LSB                       14
#define RX_LOCATION_INFO_8_RTT_CHAN_SPREAD_MASK                      0x003fc000

#define RX_LOCATION_INFO_8_RTT_TIMING_BACKOFF_SEL_OFFSET             0x00000020
#define RX_LOCATION_INFO_8_RTT_TIMING_BACKOFF_SEL_LSB                22
#define RX_LOCATION_INFO_8_RTT_TIMING_BACKOFF_SEL_MASK               0x00c00000

#define RX_LOCATION_INFO_8_RESERVED_8_OFFSET                         0x00000020
#define RX_LOCATION_INFO_8_RESERVED_8_LSB                            24
#define RX_LOCATION_INFO_8_RESERVED_8_MASK                           0x7f000000

#define RX_LOCATION_INFO_8_RX_LOCATION_INFO_VALID_OFFSET             0x00000020
#define RX_LOCATION_INFO_8_RX_LOCATION_INFO_VALID_LSB                31
#define RX_LOCATION_INFO_8_RX_LOCATION_INFO_VALID_MASK               0x80000000

#endif

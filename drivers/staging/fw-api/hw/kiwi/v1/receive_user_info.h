
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











#ifndef _RECEIVE_USER_INFO_H_
#define _RECEIVE_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RECEIVE_USER_INFO 12


struct receive_user_info {
	     uint32_t phy_ppdu_id                                             : 16,
		      user_rssi                                               :  8,
		      pkt_type                                                :  4,
		      stbc                                                    :  1,
		      reception_type                                          :  3;
	     uint32_t rate_mcs                                                :  4,
		      sgi                                                     :  2,
		      reserved_1a                                             :  2,
		      mimo_ss_bitmap                                          :  8,
		      receive_bandwidth                                       :  3,
		      reserved_1b                                             :  5,
		      dl_ofdma_user_index                                     :  8;
	     uint32_t dl_ofdma_content_channel                                :  1,
		      reserved_2a                                             :  7,
		      nss                                                     :  3,
		      stream_offset                                           :  3,
		      sta_dcm                                                 :  1,
		      ldpc                                                    :  1,
		      ru_type_80_0                                            :  4,
		      ru_type_80_1                                            :  4,
		      ru_type_80_2                                            :  4,
		      ru_type_80_3                                            :  4;
	     uint32_t ru_start_index_80_0                                     :  6,
		      reserved_3a                                             :  2,
		      ru_start_index_80_1                                     :  6,
		      reserved_3b                                             :  2,
		      ru_start_index_80_2                                     :  6,
		      reserved_3c                                             :  2,
		      ru_start_index_80_3                                     :  6,
		      reserved_3d                                             :  2;
	     uint32_t user_fd_rssi_ss0                                        : 32;
	     uint32_t user_fd_rssi_ss1                                        : 32;
	     uint32_t user_fd_rssi_ss2                                        : 32;
	     uint32_t user_fd_rssi_ss3                                        : 32;
	     uint32_t user_fd_rssi_ss4                                        : 32;
	     uint32_t user_fd_rssi_ss5                                        : 32;
	     uint32_t user_fd_rssi_ss6                                        : 32;
	     uint32_t user_fd_rssi_ss7                                        : 32;
};




#define RECEIVE_USER_INFO_PHY_PPDU_ID_OFFSET                                        0x00000000
#define RECEIVE_USER_INFO_PHY_PPDU_ID_LSB                                           0
#define RECEIVE_USER_INFO_PHY_PPDU_ID_MSB                                           15
#define RECEIVE_USER_INFO_PHY_PPDU_ID_MASK                                          0x0000ffff




#define RECEIVE_USER_INFO_USER_RSSI_OFFSET                                          0x00000000
#define RECEIVE_USER_INFO_USER_RSSI_LSB                                             16
#define RECEIVE_USER_INFO_USER_RSSI_MSB                                             23
#define RECEIVE_USER_INFO_USER_RSSI_MASK                                            0x00ff0000




#define RECEIVE_USER_INFO_PKT_TYPE_OFFSET                                           0x00000000
#define RECEIVE_USER_INFO_PKT_TYPE_LSB                                              24
#define RECEIVE_USER_INFO_PKT_TYPE_MSB                                              27
#define RECEIVE_USER_INFO_PKT_TYPE_MASK                                             0x0f000000




#define RECEIVE_USER_INFO_STBC_OFFSET                                               0x00000000
#define RECEIVE_USER_INFO_STBC_LSB                                                  28
#define RECEIVE_USER_INFO_STBC_MSB                                                  28
#define RECEIVE_USER_INFO_STBC_MASK                                                 0x10000000




#define RECEIVE_USER_INFO_RECEPTION_TYPE_OFFSET                                     0x00000000
#define RECEIVE_USER_INFO_RECEPTION_TYPE_LSB                                        29
#define RECEIVE_USER_INFO_RECEPTION_TYPE_MSB                                        31
#define RECEIVE_USER_INFO_RECEPTION_TYPE_MASK                                       0xe0000000




#define RECEIVE_USER_INFO_RATE_MCS_OFFSET                                           0x00000004
#define RECEIVE_USER_INFO_RATE_MCS_LSB                                              0
#define RECEIVE_USER_INFO_RATE_MCS_MSB                                              3
#define RECEIVE_USER_INFO_RATE_MCS_MASK                                             0x0000000f




#define RECEIVE_USER_INFO_SGI_OFFSET                                                0x00000004
#define RECEIVE_USER_INFO_SGI_LSB                                                   4
#define RECEIVE_USER_INFO_SGI_MSB                                                   5
#define RECEIVE_USER_INFO_SGI_MASK                                                  0x00000030




#define RECEIVE_USER_INFO_RESERVED_1A_OFFSET                                        0x00000004
#define RECEIVE_USER_INFO_RESERVED_1A_LSB                                           6
#define RECEIVE_USER_INFO_RESERVED_1A_MSB                                           7
#define RECEIVE_USER_INFO_RESERVED_1A_MASK                                          0x000000c0




#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_OFFSET                                     0x00000004
#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_LSB                                        8
#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_MSB                                        15
#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_MASK                                       0x0000ff00




#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_OFFSET                                  0x00000004
#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_LSB                                     16
#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_MSB                                     18
#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_MASK                                    0x00070000




#define RECEIVE_USER_INFO_RESERVED_1B_OFFSET                                        0x00000004
#define RECEIVE_USER_INFO_RESERVED_1B_LSB                                           19
#define RECEIVE_USER_INFO_RESERVED_1B_MSB                                           23
#define RECEIVE_USER_INFO_RESERVED_1B_MASK                                          0x00f80000




#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_OFFSET                                0x00000004
#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_LSB                                   24
#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_MSB                                   31
#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_MASK                                  0xff000000




#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_OFFSET                           0x00000008
#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_LSB                              0
#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_MSB                              0
#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_MASK                             0x00000001




#define RECEIVE_USER_INFO_RESERVED_2A_OFFSET                                        0x00000008
#define RECEIVE_USER_INFO_RESERVED_2A_LSB                                           1
#define RECEIVE_USER_INFO_RESERVED_2A_MSB                                           7
#define RECEIVE_USER_INFO_RESERVED_2A_MASK                                          0x000000fe




#define RECEIVE_USER_INFO_NSS_OFFSET                                                0x00000008
#define RECEIVE_USER_INFO_NSS_LSB                                                   8
#define RECEIVE_USER_INFO_NSS_MSB                                                   10
#define RECEIVE_USER_INFO_NSS_MASK                                                  0x00000700




#define RECEIVE_USER_INFO_STREAM_OFFSET_OFFSET                                      0x00000008
#define RECEIVE_USER_INFO_STREAM_OFFSET_LSB                                         11
#define RECEIVE_USER_INFO_STREAM_OFFSET_MSB                                         13
#define RECEIVE_USER_INFO_STREAM_OFFSET_MASK                                        0x00003800




#define RECEIVE_USER_INFO_STA_DCM_OFFSET                                            0x00000008
#define RECEIVE_USER_INFO_STA_DCM_LSB                                               14
#define RECEIVE_USER_INFO_STA_DCM_MSB                                               14
#define RECEIVE_USER_INFO_STA_DCM_MASK                                              0x00004000




#define RECEIVE_USER_INFO_LDPC_OFFSET                                               0x00000008
#define RECEIVE_USER_INFO_LDPC_LSB                                                  15
#define RECEIVE_USER_INFO_LDPC_MSB                                                  15
#define RECEIVE_USER_INFO_LDPC_MASK                                                 0x00008000




#define RECEIVE_USER_INFO_RU_TYPE_80_0_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_0_LSB                                          16
#define RECEIVE_USER_INFO_RU_TYPE_80_0_MSB                                          19
#define RECEIVE_USER_INFO_RU_TYPE_80_0_MASK                                         0x000f0000




#define RECEIVE_USER_INFO_RU_TYPE_80_1_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_1_LSB                                          20
#define RECEIVE_USER_INFO_RU_TYPE_80_1_MSB                                          23
#define RECEIVE_USER_INFO_RU_TYPE_80_1_MASK                                         0x00f00000




#define RECEIVE_USER_INFO_RU_TYPE_80_2_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_2_LSB                                          24
#define RECEIVE_USER_INFO_RU_TYPE_80_2_MSB                                          27
#define RECEIVE_USER_INFO_RU_TYPE_80_2_MASK                                         0x0f000000




#define RECEIVE_USER_INFO_RU_TYPE_80_3_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_3_LSB                                          28
#define RECEIVE_USER_INFO_RU_TYPE_80_3_MSB                                          31
#define RECEIVE_USER_INFO_RU_TYPE_80_3_MASK                                         0xf0000000




#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_LSB                                   0
#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_MSB                                   5
#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_MASK                                  0x0000003f




#define RECEIVE_USER_INFO_RESERVED_3A_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3A_LSB                                           6
#define RECEIVE_USER_INFO_RESERVED_3A_MSB                                           7
#define RECEIVE_USER_INFO_RESERVED_3A_MASK                                          0x000000c0




#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_LSB                                   8
#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_MSB                                   13
#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_MASK                                  0x00003f00




#define RECEIVE_USER_INFO_RESERVED_3B_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3B_LSB                                           14
#define RECEIVE_USER_INFO_RESERVED_3B_MSB                                           15
#define RECEIVE_USER_INFO_RESERVED_3B_MASK                                          0x0000c000




#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_LSB                                   16
#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_MSB                                   21
#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_MASK                                  0x003f0000




#define RECEIVE_USER_INFO_RESERVED_3C_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3C_LSB                                           22
#define RECEIVE_USER_INFO_RESERVED_3C_MSB                                           23
#define RECEIVE_USER_INFO_RESERVED_3C_MASK                                          0x00c00000




#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_LSB                                   24
#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_MSB                                   29
#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_MASK                                  0x3f000000




#define RECEIVE_USER_INFO_RESERVED_3D_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3D_LSB                                           30
#define RECEIVE_USER_INFO_RESERVED_3D_MSB                                           31
#define RECEIVE_USER_INFO_RESERVED_3D_MASK                                          0xc0000000




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS0_OFFSET                                   0x00000010
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS0_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS0_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS0_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS1_OFFSET                                   0x00000014
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS1_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS1_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS1_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS2_OFFSET                                   0x00000018
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS2_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS2_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS2_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS3_OFFSET                                   0x0000001c
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS3_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS3_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS3_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS4_OFFSET                                   0x00000020
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS4_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS4_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS4_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS5_OFFSET                                   0x00000024
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS5_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS5_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS5_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS6_OFFSET                                   0x00000028
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS6_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS6_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS6_MASK                                     0xffffffff




#define RECEIVE_USER_INFO_USER_FD_RSSI_SS7_OFFSET                                   0x0000002c
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS7_LSB                                      0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS7_MSB                                      31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SS7_MASK                                     0xffffffff



#endif

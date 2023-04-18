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

#ifndef _RECEIVE_USER_INFO_H_
#define _RECEIVE_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RECEIVE_USER_INFO 8


struct receive_user_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t phy_ppdu_id                                             : 16, // [15:0]
                      user_rssi                                               :  8, // [23:16]
                      pkt_type                                                :  4, // [27:24]
                      stbc                                                    :  1, // [28:28]
                      reception_type                                          :  3; // [31:29]
             uint32_t rate_mcs                                                :  4, // [3:0]
                      sgi                                                     :  2, // [5:4]
                      he_ranging_ndp                                          :  1, // [6:6]
                      reserved_1a                                             :  1, // [7:7]
                      mimo_ss_bitmap                                          :  8, // [15:8]
                      receive_bandwidth                                       :  3, // [18:16]
                      reserved_1b                                             :  5, // [23:19]
                      dl_ofdma_user_index                                     :  8; // [31:24]
             uint32_t dl_ofdma_content_channel                                :  1, // [0:0]
                      reserved_2a                                             :  7, // [7:1]
                      nss                                                     :  3, // [10:8]
                      stream_offset                                           :  3, // [13:11]
                      sta_dcm                                                 :  1, // [14:14]
                      ldpc                                                    :  1, // [15:15]
                      ru_type_80_0                                            :  4, // [19:16]
                      ru_type_80_1                                            :  4, // [23:20]
                      ru_type_80_2                                            :  4, // [27:24]
                      ru_type_80_3                                            :  4; // [31:28]
             uint32_t ru_start_index_80_0                                     :  6, // [5:0]
                      reserved_3a                                             :  2, // [7:6]
                      ru_start_index_80_1                                     :  6, // [13:8]
                      reserved_3b                                             :  2, // [15:14]
                      ru_start_index_80_2                                     :  6, // [21:16]
                      reserved_3c                                             :  2, // [23:22]
                      ru_start_index_80_3                                     :  6, // [29:24]
                      reserved_3d                                             :  2; // [31:30]
             uint32_t user_fd_rssi_seg0                                       : 32; // [31:0]
             uint32_t user_fd_rssi_seg1                                       : 32; // [31:0]
             uint32_t user_fd_rssi_seg2                                       : 32; // [31:0]
             uint32_t user_fd_rssi_seg3                                       : 32; // [31:0]
#else
             uint32_t reception_type                                          :  3, // [31:29]
                      stbc                                                    :  1, // [28:28]
                      pkt_type                                                :  4, // [27:24]
                      user_rssi                                               :  8, // [23:16]
                      phy_ppdu_id                                             : 16; // [15:0]
             uint32_t dl_ofdma_user_index                                     :  8, // [31:24]
                      reserved_1b                                             :  5, // [23:19]
                      receive_bandwidth                                       :  3, // [18:16]
                      mimo_ss_bitmap                                          :  8, // [15:8]
                      reserved_1a                                             :  1, // [7:7]
                      he_ranging_ndp                                          :  1, // [6:6]
                      sgi                                                     :  2, // [5:4]
                      rate_mcs                                                :  4; // [3:0]
             uint32_t ru_type_80_3                                            :  4, // [31:28]
                      ru_type_80_2                                            :  4, // [27:24]
                      ru_type_80_1                                            :  4, // [23:20]
                      ru_type_80_0                                            :  4, // [19:16]
                      ldpc                                                    :  1, // [15:15]
                      sta_dcm                                                 :  1, // [14:14]
                      stream_offset                                           :  3, // [13:11]
                      nss                                                     :  3, // [10:8]
                      reserved_2a                                             :  7, // [7:1]
                      dl_ofdma_content_channel                                :  1; // [0:0]
             uint32_t reserved_3d                                             :  2, // [31:30]
                      ru_start_index_80_3                                     :  6, // [29:24]
                      reserved_3c                                             :  2, // [23:22]
                      ru_start_index_80_2                                     :  6, // [21:16]
                      reserved_3b                                             :  2, // [15:14]
                      ru_start_index_80_1                                     :  6, // [13:8]
                      reserved_3a                                             :  2, // [7:6]
                      ru_start_index_80_0                                     :  6; // [5:0]
             uint32_t user_fd_rssi_seg0                                       : 32; // [31:0]
             uint32_t user_fd_rssi_seg1                                       : 32; // [31:0]
             uint32_t user_fd_rssi_seg2                                       : 32; // [31:0]
             uint32_t user_fd_rssi_seg3                                       : 32; // [31:0]
#endif
};


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RECEIVE_USER_INFO_PHY_PPDU_ID_OFFSET                                        0x00000000
#define RECEIVE_USER_INFO_PHY_PPDU_ID_LSB                                           0
#define RECEIVE_USER_INFO_PHY_PPDU_ID_MSB                                           15
#define RECEIVE_USER_INFO_PHY_PPDU_ID_MASK                                          0x0000ffff


/* Description		USER_RSSI

			RSSI for this user
			Frequency domain RSSI measurement for this user. Based on
			 the channel estimate.  
			
			<legal all>
*/

#define RECEIVE_USER_INFO_USER_RSSI_OFFSET                                          0x00000000
#define RECEIVE_USER_INFO_USER_RSSI_LSB                                             16
#define RECEIVE_USER_INFO_USER_RSSI_MSB                                             23
#define RECEIVE_USER_INFO_USER_RSSI_MASK                                            0x00ff0000


/* Description		PKT_TYPE

			Packet type:
			
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define RECEIVE_USER_INFO_PKT_TYPE_OFFSET                                           0x00000000
#define RECEIVE_USER_INFO_PKT_TYPE_LSB                                              24
#define RECEIVE_USER_INFO_PKT_TYPE_MSB                                              27
#define RECEIVE_USER_INFO_PKT_TYPE_MASK                                             0x0f000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define RECEIVE_USER_INFO_STBC_OFFSET                                               0x00000000
#define RECEIVE_USER_INFO_STBC_LSB                                                  28
#define RECEIVE_USER_INFO_STBC_MSB                                                  28
#define RECEIVE_USER_INFO_STBC_MASK                                                 0x10000000


/* Description		RECEPTION_TYPE

			Indicates what type of reception this is.
			<enum 0     reception_type_SU > Basic SU reception (not 
			part of OFDMA or MU-MIMO)
			<enum 1     reception_type_MU_MIMO > This is related to 
			DL type of reception
			<enum 2     reception_type_MU_OFDMA >  This is related to
			 DL type of reception
			<enum 3     reception_type_MU_OFDMA_MIMO >  This is related
			 to DL type of reception
			<enum 4     reception_type_UL_MU_MIMO > This is related 
			to UL type of reception
			<enum 5     reception_type_UL_MU_OFDMA >  This is related
			 to UL type of reception
			<enum 6     reception_type_UL_MU_OFDMA_MIMO >  This is related
			 to UL type of reception
			
			<legal 0-6>
*/

#define RECEIVE_USER_INFO_RECEPTION_TYPE_OFFSET                                     0x00000000
#define RECEIVE_USER_INFO_RECEPTION_TYPE_LSB                                        29
#define RECEIVE_USER_INFO_RECEPTION_TYPE_MSB                                        31
#define RECEIVE_USER_INFO_RECEPTION_TYPE_MASK                                       0xe0000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define RECEIVE_USER_INFO_RATE_MCS_OFFSET                                           0x00000004
#define RECEIVE_USER_INFO_RATE_MCS_LSB                                              0
#define RECEIVE_USER_INFO_RATE_MCS_MSB                                              3
#define RECEIVE_USER_INFO_RATE_MCS_MASK                                             0x0000000f


/* Description		SGI

			Field only valid when pkt type is HT, VHT or HE.
			
			<enum 0     gi_0_8_us > Legacy normal GI.  Can also be used
			 for HE
			<enum 1     gi_0_4_us > Legacy short GI.  Can also be used
			 for HE
			<enum 2     gi_1_6_us > HE related GI
			<enum 3     gi_3_2_us > HE related GI
			<legal 0 - 3>
*/

#define RECEIVE_USER_INFO_SGI_OFFSET                                                0x00000004
#define RECEIVE_USER_INFO_SGI_LSB                                                   4
#define RECEIVE_USER_INFO_SGI_MSB                                                   5
#define RECEIVE_USER_INFO_SGI_MASK                                                  0x00000030


/* Description		HE_RANGING_NDP

			Set to 1 for expected HE TB ranging NDP Rx in response to
			 sounding/secure sounding ranging Trigger Tx
			
			<legal all>
*/

#define RECEIVE_USER_INFO_HE_RANGING_NDP_OFFSET                                     0x00000004
#define RECEIVE_USER_INFO_HE_RANGING_NDP_LSB                                        6
#define RECEIVE_USER_INFO_HE_RANGING_NDP_MSB                                        6
#define RECEIVE_USER_INFO_HE_RANGING_NDP_MASK                                       0x00000040


/* Description		RESERVED_1A

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_1A_OFFSET                                        0x00000004
#define RECEIVE_USER_INFO_RESERVED_1A_LSB                                           7
#define RECEIVE_USER_INFO_RESERVED_1A_MSB                                           7
#define RECEIVE_USER_INFO_RESERVED_1A_MASK                                          0x00000080


/* Description		MIMO_SS_BITMAP

			Bitmap, with each bit indicating if the related spatial 
			stream is used for this STA
			LSB related to SS 0
			
			0: spatial stream not used for this reception
			1: spatial stream used for this reception
			
			<legal all>
*/

#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_OFFSET                                     0x00000004
#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_LSB                                        8
#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_MSB                                        15
#define RECEIVE_USER_INFO_MIMO_SS_BITMAP_MASK                                       0x0000ff00


/* Description		RECEIVE_BANDWIDTH

			Full receive Bandwidth
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_OFFSET                                  0x00000004
#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_LSB                                     16
#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_MSB                                     18
#define RECEIVE_USER_INFO_RECEIVE_BANDWIDTH_MASK                                    0x00070000


/* Description		RESERVED_1B

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_1B_OFFSET                                        0x00000004
#define RECEIVE_USER_INFO_RESERVED_1B_LSB                                           19
#define RECEIVE_USER_INFO_RESERVED_1B_MSB                                           23
#define RECEIVE_USER_INFO_RESERVED_1B_MASK                                          0x00f80000


/* Description		DL_OFDMA_USER_INDEX

			Field only valid in the of DL MU OFDMA reception
			
			The user number within the RU_allocation.
			
			This is needed for SW to determine the exact RU position
			 within the reception.
			<legal all>
*/

#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_OFFSET                                0x00000004
#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_LSB                                   24
#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_MSB                                   31
#define RECEIVE_USER_INFO_DL_OFDMA_USER_INDEX_MASK                                  0xff000000


/* Description		DL_OFDMA_CONTENT_CHANNEL

			Field only valid in the of DL MU OFDMA/MIMO reception
			
			In case of DL MU reception, this field indicates the content
			 channel number where PHY found the RU information for this
			 user
			
			This is needed for SW to determine the exact RU position
			 within the reception.
			
			<enum 0      content_channel_1>
			<enum 1      content_channel_2> 
			
			<legal all>
*/

#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_OFFSET                           0x00000008
#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_LSB                              0
#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_MSB                              0
#define RECEIVE_USER_INFO_DL_OFDMA_CONTENT_CHANNEL_MASK                             0x00000001


/* Description		RESERVED_2A

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_2A_OFFSET                                        0x00000008
#define RECEIVE_USER_INFO_RESERVED_2A_LSB                                           1
#define RECEIVE_USER_INFO_RESERVED_2A_MSB                                           7
#define RECEIVE_USER_INFO_RESERVED_2A_MASK                                          0x000000fe


/* Description		NSS

			Field only valid in case of Uplink_receive_type == mimo_only
			 OR ofdma_mimo
			
			Number of Spatial Streams occupied by the User
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define RECEIVE_USER_INFO_NSS_OFFSET                                                0x00000008
#define RECEIVE_USER_INFO_NSS_LSB                                                   8
#define RECEIVE_USER_INFO_NSS_MSB                                                   10
#define RECEIVE_USER_INFO_NSS_MASK                                                  0x00000700


/* Description		STREAM_OFFSET

			Field only valid in case of Uplink_receive_type == mimo_only
			 OR ofdma_mimo
			
			Stream Offset from which the User occupies the Streams
			
			Note MAC:
			directly from pdg_fes_setup, based on BW
*/

#define RECEIVE_USER_INFO_STREAM_OFFSET_OFFSET                                      0x00000008
#define RECEIVE_USER_INFO_STREAM_OFFSET_LSB                                         11
#define RECEIVE_USER_INFO_STREAM_OFFSET_MSB                                         13
#define RECEIVE_USER_INFO_STREAM_OFFSET_MASK                                        0x00003800


/* Description		STA_DCM

			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define RECEIVE_USER_INFO_STA_DCM_OFFSET                                            0x00000008
#define RECEIVE_USER_INFO_STA_DCM_LSB                                               14
#define RECEIVE_USER_INFO_STA_DCM_MSB                                               14
#define RECEIVE_USER_INFO_STA_DCM_MASK                                              0x00004000


/* Description		LDPC

			When set, use LDPC transmission rates were used.
			<legal all>
*/

#define RECEIVE_USER_INFO_LDPC_OFFSET                                               0x00000008
#define RECEIVE_USER_INFO_LDPC_LSB                                                  15
#define RECEIVE_USER_INFO_LDPC_MSB                                                  15
#define RECEIVE_USER_INFO_LDPC_MASK                                                 0x00008000


/* Description		RU_TYPE_80_0

			Indicates the size of the RU in the first 80 MHz sub-band
			
			<enum 0 RU_26_per80>
			<enum 1 RU_52_per80>
			<enum 2 RU_78_per80>
			<enum 3 RU_106_per80>
			<enum 4 RU_132_per80>
			<enum 5 RU_242_per80>
			<enum 6 RU_484_per80>
			<enum 7 RU_726_per80>
			<enum 8 RU_996_per80>
			<enum 9 RU_996x2>
			<enum 10 RU_996x3>
			<enum 11 RU_996x4>
			<enum 12 RU_rsvd0> DO NOT USE
			<enum 13 RU_rsvd1> DO NOT USE
			<enum 14 RU_rsvd2> DO NOT USE
			<enum 15 RU_NONE> No RUs in this 80 MHz
			<legal 0-15>
*/

#define RECEIVE_USER_INFO_RU_TYPE_80_0_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_0_LSB                                          16
#define RECEIVE_USER_INFO_RU_TYPE_80_0_MSB                                          19
#define RECEIVE_USER_INFO_RU_TYPE_80_0_MASK                                         0x000f0000


/* Description		RU_TYPE_80_1

			Indicates the size of the RU in the second 80 MHz sub-band
			
			<enum 0 RU_26_per80>
			<enum 1 RU_52_per80>
			<enum 2 RU_78_per80>
			<enum 3 RU_106_per80>
			<enum 4 RU_132_per80>
			<enum 5 RU_242_per80>
			<enum 6 RU_484_per80>
			<enum 7 RU_726_per80>
			<enum 8 RU_996_per80>
			<enum 9 RU_996x2>
			<enum 10 RU_996x3>
			<enum 11 RU_996x4>
			<enum 12 RU_rsvd0> DO NOT USE
			<enum 13 RU_rsvd1> DO NOT USE
			<enum 14 RU_rsvd2> DO NOT USE
			<enum 15 RU_NONE> No RUs in this 80 MHz
			<legal 0-15>
*/

#define RECEIVE_USER_INFO_RU_TYPE_80_1_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_1_LSB                                          20
#define RECEIVE_USER_INFO_RU_TYPE_80_1_MSB                                          23
#define RECEIVE_USER_INFO_RU_TYPE_80_1_MASK                                         0x00f00000


/* Description		RU_TYPE_80_2

			Indicates the size of the RU in the third 80 MHz sub-band
			
			<enum 0 RU_26_per80>
			<enum 1 RU_52_per80>
			<enum 2 RU_78_per80>
			<enum 3 RU_106_per80>
			<enum 4 RU_132_per80>
			<enum 5 RU_242_per80>
			<enum 6 RU_484_per80>
			<enum 7 RU_726_per80>
			<enum 8 RU_996_per80>
			<enum 9 RU_996x2>
			<enum 10 RU_996x3>
			<enum 11 RU_996x4>
			<enum 12 RU_rsvd0> DO NOT USE
			<enum 13 RU_rsvd1> DO NOT USE
			<enum 14 RU_rsvd2> DO NOT USE
			<enum 15 RU_NONE> No RUs in this 80 MHz
			<legal 0-15>
*/

#define RECEIVE_USER_INFO_RU_TYPE_80_2_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_2_LSB                                          24
#define RECEIVE_USER_INFO_RU_TYPE_80_2_MSB                                          27
#define RECEIVE_USER_INFO_RU_TYPE_80_2_MASK                                         0x0f000000


/* Description		RU_TYPE_80_3

			Indicates the size of the RU in the fourth 80 MHz sub-band
			
			<enum 0 RU_26_per80>
			<enum 1 RU_52_per80>
			<enum 2 RU_78_per80>
			<enum 3 RU_106_per80>
			<enum 4 RU_132_per80>
			<enum 5 RU_242_per80>
			<enum 6 RU_484_per80>
			<enum 7 RU_726_per80>
			<enum 8 RU_996_per80>
			<enum 9 RU_996x2>
			<enum 10 RU_996x3>
			<enum 11 RU_996x4>
			<enum 12 RU_rsvd0> DO NOT USE
			<enum 13 RU_rsvd1> DO NOT USE
			<enum 14 RU_rsvd2> DO NOT USE
			<enum 15 RU_NONE> No RUs in this 80 MHz
			<legal 0-15>
*/

#define RECEIVE_USER_INFO_RU_TYPE_80_3_OFFSET                                       0x00000008
#define RECEIVE_USER_INFO_RU_TYPE_80_3_LSB                                          28
#define RECEIVE_USER_INFO_RU_TYPE_80_3_MSB                                          31
#define RECEIVE_USER_INFO_RU_TYPE_80_3_MASK                                         0xf0000000


/* Description		RU_START_INDEX_80_0

			RU index number to which User is assigned in the first 80
			 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_LSB                                   0
#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_MSB                                   5
#define RECEIVE_USER_INFO_RU_START_INDEX_80_0_MASK                                  0x0000003f


/* Description		RESERVED_3A

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_3A_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3A_LSB                                           6
#define RECEIVE_USER_INFO_RESERVED_3A_MSB                                           7
#define RECEIVE_USER_INFO_RESERVED_3A_MASK                                          0x000000c0


/* Description		RU_START_INDEX_80_1

			RU index number to which User is assigned in the second 
			80 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_LSB                                   8
#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_MSB                                   13
#define RECEIVE_USER_INFO_RU_START_INDEX_80_1_MASK                                  0x00003f00


/* Description		RESERVED_3B

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_3B_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3B_LSB                                           14
#define RECEIVE_USER_INFO_RESERVED_3B_MSB                                           15
#define RECEIVE_USER_INFO_RESERVED_3B_MASK                                          0x0000c000


/* Description		RU_START_INDEX_80_2

			RU index number to which User is assigned in the third 80
			 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_LSB                                   16
#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_MSB                                   21
#define RECEIVE_USER_INFO_RU_START_INDEX_80_2_MASK                                  0x003f0000


/* Description		RESERVED_3C

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_3C_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3C_LSB                                           22
#define RECEIVE_USER_INFO_RESERVED_3C_MSB                                           23
#define RECEIVE_USER_INFO_RESERVED_3C_MASK                                          0x00c00000


/* Description		RU_START_INDEX_80_3

			RU index number to which User is assigned in the fourth 
			80 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_OFFSET                                0x0000000c
#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_LSB                                   24
#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_MSB                                   29
#define RECEIVE_USER_INFO_RU_START_INDEX_80_3_MASK                                  0x3f000000


/* Description		RESERVED_3D

			<legal 0>
*/

#define RECEIVE_USER_INFO_RESERVED_3D_OFFSET                                        0x0000000c
#define RECEIVE_USER_INFO_RESERVED_3D_LSB                                           30
#define RECEIVE_USER_INFO_RESERVED_3D_MSB                                           31
#define RECEIVE_USER_INFO_RESERVED_3D_MASK                                          0xc0000000


/* Description		USER_FD_RSSI_SEG0

			Frequency domain RSSI measurement for the lowest 80 MHz 
			subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG0_OFFSET                                  0x00000010
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG0_LSB                                     0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG0_MSB                                     31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG0_MASK                                    0xffffffff


/* Description		USER_FD_RSSI_SEG1

			Frequency domain RSSI measurement for the second lowest 
			80 MHz subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG1_OFFSET                                  0x00000014
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG1_LSB                                     0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG1_MSB                                     31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG1_MASK                                    0xffffffff


/* Description		USER_FD_RSSI_SEG2

			Frequency domain RSSI measurement for the third lowest 80
			 MHz subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG2_OFFSET                                  0x00000018
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG2_LSB                                     0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG2_MSB                                     31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG2_MASK                                    0xffffffff


/* Description		USER_FD_RSSI_SEG3

			Frequency domain RSSI measurement for the highest 80 MHz
			 subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG3_OFFSET                                  0x0000001c
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG3_LSB                                     0
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG3_MSB                                     31
#define RECEIVE_USER_INFO_USER_FD_RSSI_SEG3_MASK                                    0xffffffff



#endif   // RECEIVE_USER_INFO

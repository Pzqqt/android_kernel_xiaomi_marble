
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _RX_PPDU_START_USER_INFO_H_
#define _RX_PPDU_START_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "receive_user_info.h"
#define NUM_OF_DWORDS_RX_PPDU_START_USER_INFO 8

#define NUM_OF_QWORDS_RX_PPDU_START_USER_INFO 4


struct rx_ppdu_start_user_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   receive_user_info                                         receive_user_info_details;
#else
             struct   receive_user_info                                         receive_user_info_details;
#endif
};


/* Description		RECEIVE_USER_INFO_DETAILS

			Overview of receive parameters that the MAC needs to prepend
			 to every received MSDU/MPDU.
*/


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PHY_PPDU_ID_OFFSET        0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PHY_PPDU_ID_LSB           0
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PHY_PPDU_ID_MSB           15
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PHY_PPDU_ID_MASK          0x000000000000ffff


/* Description		USER_RSSI

			RSSI for this user
			Frequency domain RSSI measurement for this user. Based on
			 the channel estimate.  
			
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_RSSI_OFFSET          0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_RSSI_LSB             16
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_RSSI_MSB             23
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_RSSI_MASK            0x0000000000ff0000


/* Description		PKT_TYPE

			Packet type:
			
			<enum_type PKT_TYPE_ENUM>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PKT_TYPE_OFFSET           0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PKT_TYPE_LSB              24
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PKT_TYPE_MSB              27
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_PKT_TYPE_MASK             0x000000000f000000


/* Description		STBC

			When set, use STBC transmission rates
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STBC_OFFSET               0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STBC_LSB                  28
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STBC_MSB                  28
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STBC_MASK                 0x0000000010000000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEPTION_TYPE_OFFSET     0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEPTION_TYPE_LSB        29
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEPTION_TYPE_MSB        31
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEPTION_TYPE_MASK       0x00000000e0000000


/* Description		RATE_MCS

			For details, refer to  MCS_TYPE description
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RATE_MCS_OFFSET           0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RATE_MCS_LSB              32
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RATE_MCS_MSB              35
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RATE_MCS_MASK             0x0000000f00000000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_SGI_OFFSET                0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_SGI_LSB                   36
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_SGI_MSB                   37
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_SGI_MASK                  0x0000003000000000


/* Description		HE_RANGING_NDP

			Set to 1 for expected HE TB ranging NDP Rx in response to
			 sounding/secure sounding ranging Trigger Tx
			
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_HE_RANGING_NDP_OFFSET     0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_HE_RANGING_NDP_LSB        38
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_HE_RANGING_NDP_MSB        38
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_HE_RANGING_NDP_MASK       0x0000004000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1A_OFFSET        0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1A_LSB           39
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1A_MSB           39
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1A_MASK          0x0000008000000000


/* Description		MIMO_SS_BITMAP

			Bitmap, with each bit indicating if the related spatial 
			stream is used for this STA
			LSB related to SS 0
			
			0: spatial stream not used for this reception
			1: spatial stream used for this reception
			
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_MIMO_SS_BITMAP_OFFSET     0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_MIMO_SS_BITMAP_LSB        40
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_MIMO_SS_BITMAP_MSB        47
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_MIMO_SS_BITMAP_MASK       0x0000ff0000000000


/* Description		RECEIVE_BANDWIDTH

			Full receive Bandwidth
			
			<enum_type BW_ENUM>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEIVE_BANDWIDTH_OFFSET  0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEIVE_BANDWIDTH_LSB     48
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEIVE_BANDWIDTH_MSB     50
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RECEIVE_BANDWIDTH_MASK    0x0007000000000000


/* Description		RESERVED_1B

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1B_OFFSET        0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1B_LSB           51
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1B_MSB           55
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_1B_MASK          0x00f8000000000000


/* Description		DL_OFDMA_USER_INDEX

			Field only valid in the of DL MU OFDMA reception
			
			The user number within the RU_allocation.
			
			This is needed for SW to determine the exact RU position
			 within the reception.
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_USER_INDEX_OFFSET 0x0000000000000000
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_USER_INDEX_LSB   56
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_USER_INDEX_MSB   63
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_USER_INDEX_MASK  0xff00000000000000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_CONTENT_CHANNEL_OFFSET 0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_CONTENT_CHANNEL_LSB 0
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_CONTENT_CHANNEL_MSB 0
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_DL_OFDMA_CONTENT_CHANNEL_MASK 0x0000000000000001


/* Description		RESERVED_2A

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_2A_OFFSET        0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_2A_LSB           1
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_2A_MSB           7
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_2A_MASK          0x00000000000000fe


/* Description		NSS

			Field only valid in case of Uplink_receive_type == mimo_only
			 OR ofdma_mimo
			
			Number of Spatial Streams occupied by the User
			
			<enum_type SS_COUNT_ENUM>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_NSS_OFFSET                0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_NSS_LSB                   8
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_NSS_MSB                   10
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_NSS_MASK                  0x0000000000000700


/* Description		STREAM_OFFSET

			Field only valid in case of Uplink_receive_type == mimo_only
			 OR ofdma_mimo
			
			Stream Offset from which the User occupies the Streams
			
			Note MAC:
			directly from pdg_fes_setup, based on BW
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STREAM_OFFSET_OFFSET      0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STREAM_OFFSET_LSB         11
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STREAM_OFFSET_MSB         13
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STREAM_OFFSET_MASK        0x0000000000003800


/* Description		STA_DCM

			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STA_DCM_OFFSET            0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STA_DCM_LSB               14
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STA_DCM_MSB               14
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_STA_DCM_MASK              0x0000000000004000


/* Description		LDPC

			When set, use LDPC transmission rates were used.
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_LDPC_OFFSET               0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_LDPC_LSB                  15
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_LDPC_MSB                  15
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_LDPC_MASK                 0x0000000000008000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_0_OFFSET       0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_0_LSB          16
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_0_MSB          19
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_0_MASK         0x00000000000f0000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_1_OFFSET       0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_1_LSB          20
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_1_MSB          23
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_1_MASK         0x0000000000f00000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_2_OFFSET       0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_2_LSB          24
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_2_MSB          27
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_2_MASK         0x000000000f000000


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

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_3_OFFSET       0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_3_LSB          28
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_3_MSB          31
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_TYPE_80_3_MASK         0x00000000f0000000


/* Description		RU_START_INDEX_80_0

			RU index number to which User is assigned in the first 80
			 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_0_OFFSET 0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_0_LSB   32
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_0_MSB   37
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_0_MASK  0x0000003f00000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3A_OFFSET        0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3A_LSB           38
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3A_MSB           39
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3A_MASK          0x000000c000000000


/* Description		RU_START_INDEX_80_1

			RU index number to which User is assigned in the second 
			80 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_1_OFFSET 0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_1_LSB   40
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_1_MSB   45
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_1_MASK  0x00003f0000000000


/* Description		RESERVED_3B

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3B_OFFSET        0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3B_LSB           46
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3B_MSB           47
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3B_MASK          0x0000c00000000000


/* Description		RU_START_INDEX_80_2

			RU index number to which User is assigned in the third 80
			 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_2_OFFSET 0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_2_LSB   48
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_2_MSB   53
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_2_MASK  0x003f000000000000


/* Description		RESERVED_3C

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3C_OFFSET        0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3C_LSB           54
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3C_MSB           55
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3C_MASK          0x00c0000000000000


/* Description		RU_START_INDEX_80_3

			RU index number to which User is assigned in the fourth 
			80 MHz
			RU numbering is over the entire BW, starting from 0 and 
			in increasing frequency order and not primary-secondary 
			order
			<legal 0-36>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_3_OFFSET 0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_3_LSB   56
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_3_MSB   61
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RU_START_INDEX_80_3_MASK  0x3f00000000000000


/* Description		RESERVED_3D

			<legal 0>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3D_OFFSET        0x0000000000000008
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3D_LSB           62
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3D_MSB           63
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_RESERVED_3D_MASK          0xc000000000000000


/* Description		USER_FD_RSSI_SEG0

			Frequency domain RSSI measurement for the lowest 80 MHz 
			subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG0_OFFSET  0x0000000000000010
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG0_LSB     0
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG0_MSB     31
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG0_MASK    0x00000000ffffffff


/* Description		USER_FD_RSSI_SEG1

			Frequency domain RSSI measurement for the second lowest 
			80 MHz subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG1_OFFSET  0x0000000000000010
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG1_LSB     32
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG1_MSB     63
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG1_MASK    0xffffffff00000000


/* Description		USER_FD_RSSI_SEG2

			Frequency domain RSSI measurement for the third lowest 80
			 MHz subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG2_OFFSET  0x0000000000000018
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG2_LSB     0
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG2_MSB     31
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG2_MASK    0x00000000ffffffff


/* Description		USER_FD_RSSI_SEG3

			Frequency domain RSSI measurement for the highest 80 MHz
			 subband of this user, per spatial stream
			[7:0]: first spatial stream
			...
			[31:24]: fourth spatial stream
			
			In Hamilton v1 this structure had 4 more (32-bit) words 
			after this field.
			<legal all>
*/

#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG3_OFFSET  0x0000000000000018
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG3_LSB     32
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG3_MSB     63
#define RX_PPDU_START_USER_INFO_RECEIVE_USER_INFO_DETAILS_USER_FD_RSSI_SEG3_MASK    0xffffffff00000000



#endif   // RX_PPDU_START_USER_INFO

/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	phy_ppdu_id[15:0], user_rssi[23:16], pkt_type[27:24], stbc[28], sgi[30:29], reserved_0[31]
//	1	rate_mcs[3:0], reception_type[5:4], receive_bandwidth[7:6], mimo_ss_bitmap[15:8], user_ru_allocation[23:16], nss[26:24], reserved_1b[31:27]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RECEIVE_USER_INFO 2

struct receive_user_info {
             uint32_t phy_ppdu_id                     : 16, //[15:0]
                      user_rssi                       :  8, //[23:16]
                      pkt_type                        :  4, //[27:24]
                      stbc                            :  1, //[28]
                      sgi                             :  2, //[30:29]
                      reserved_0                      :  1; //[31]
             uint32_t rate_mcs                        :  4, //[3:0]
                      reception_type                  :  2, //[5:4]
                      receive_bandwidth               :  2, //[7:6]
                      mimo_ss_bitmap                  :  8, //[15:8]
                      user_ru_allocation              :  8, //[23:16]
                      nss                             :  3, //[26:24]
                      reserved_1b                     :  5; //[31:27]
};

/*

phy_ppdu_id
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>

user_rssi
			
			RSSI for this user
			
			Frequency domain RSSI measurement for this user. Based
			on the channel estimate.  
			
			
			
			If PHY implementation runs into difficulties, the backup
			is to reuse the previously notified RSSI values.
			
			TODO PHY: request to update the text here based on the
			latest design insights
			
			
			
			<legal all>

pkt_type
			
			Packet type:
			
			
			
			<enum 0 dot11a>802.11a PPDU type
			
			<enum 1 dot11b>802.11b PPDU type
			
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			
			<enum 3 dot11ac>802.11ac PPDU type
			
			<enum 4 dot11ax>802.11ax PPDU type

stbc
			
			When set, use STBC transmission rates

sgi
			
			Field only valid when pkt type is HT, VHT or HE.
			
			
			
			<enum 0     gi_0_8_us > Legacy normal GI
			
			<enum 1     gi_0_4_us > Legacy short GI
			
			<enum 2     gi_1_6_us > HE related GI
			
			<enum 3     gi_3_2_us > HE related GI
			
			<legal 0 - 3>

reserved_0
			
			<legal 0>

rate_mcs
			
			For details, refer to  MCS_TYPE description
			
			<legal all>

reception_type
			
			Indicates what type of reception this is.
			
			<enum 0     reception_type_SU > 
			
			<enum 1     reception_type_MU_MIMO > 
			
			<enum 2     reception_type_MU_OFDMA > 
			
			<enum 3     reception_type_MU_OFDMA_MIMO > 
			
			<legal all>

receive_bandwidth
			
			Full receive Bandwidth
			
			
			
			<enum 0     full_rx_bw_20_mhz>
			
			<enum 1      full_rx_bw_40_mhz>
			
			<enum 2      full_rx_bw_80_mhz>
			
			<enum 3      full_rx_bw_160_mhz> 
			
			
			
			<legal 0-3>

mimo_ss_bitmap
			
			Field only valid in case of MIMO type reception
			
			
			
			Bitmap, with each bit indicating if the related spatial
			stream is used for this STA
			
			LSB related to SS 0
			
			
			
			0: spatial stream not used for this reception
			
			1: spatial stream used for this reception
			
			
			
			<legal all>

user_ru_allocation
			
			Field only valid in case of OFDMA uplink type reception
			
			
			
			Indicates the RU number for this user
			
			<legal 0-73>

nss
			
			Number of Spatial Streams occupied by the User
			
			<enum 0 1_spatial_stream>Single spatial stream
			
			<enum 1 2_spatial_streams>2 spatial streams
			
			<enum 2 3_spatial_streams>3 spatial streams
			
			<enum 3 4_spatial_streams>4 spatial streams
			
			<enum 4 5_spatial_streams>5 spatial streams
			
			<enum 5 6_spatial_streams>6 spatial streams
			
			<enum 6 7_spatial_streams>7 spatial streams
			
			<enum 7 8_spatial_streams>8 spatial streams

reserved_1b
			
			<legal 0>
*/


/* Description		RECEIVE_USER_INFO_0_PHY_PPDU_ID
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/
#define RECEIVE_USER_INFO_0_PHY_PPDU_ID_OFFSET                       0x00000000
#define RECEIVE_USER_INFO_0_PHY_PPDU_ID_LSB                          0
#define RECEIVE_USER_INFO_0_PHY_PPDU_ID_MASK                         0x0000ffff

/* Description		RECEIVE_USER_INFO_0_USER_RSSI
			
			RSSI for this user
			
			Frequency domain RSSI measurement for this user. Based
			on the channel estimate.  
			
			
			
			If PHY implementation runs into difficulties, the backup
			is to reuse the previously notified RSSI values.
			
			TODO PHY: request to update the text here based on the
			latest design insights
			
			
			
			<legal all>
*/
#define RECEIVE_USER_INFO_0_USER_RSSI_OFFSET                         0x00000000
#define RECEIVE_USER_INFO_0_USER_RSSI_LSB                            16
#define RECEIVE_USER_INFO_0_USER_RSSI_MASK                           0x00ff0000

/* Description		RECEIVE_USER_INFO_0_PKT_TYPE
			
			Packet type:
			
			
			
			<enum 0 dot11a>802.11a PPDU type
			
			<enum 1 dot11b>802.11b PPDU type
			
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			
			<enum 3 dot11ac>802.11ac PPDU type
			
			<enum 4 dot11ax>802.11ax PPDU type
*/
#define RECEIVE_USER_INFO_0_PKT_TYPE_OFFSET                          0x00000000
#define RECEIVE_USER_INFO_0_PKT_TYPE_LSB                             24
#define RECEIVE_USER_INFO_0_PKT_TYPE_MASK                            0x0f000000

/* Description		RECEIVE_USER_INFO_0_STBC
			
			When set, use STBC transmission rates
*/
#define RECEIVE_USER_INFO_0_STBC_OFFSET                              0x00000000
#define RECEIVE_USER_INFO_0_STBC_LSB                                 28
#define RECEIVE_USER_INFO_0_STBC_MASK                                0x10000000

/* Description		RECEIVE_USER_INFO_0_SGI
			
			Field only valid when pkt type is HT, VHT or HE.
			
			
			
			<enum 0     gi_0_8_us > Legacy normal GI
			
			<enum 1     gi_0_4_us > Legacy short GI
			
			<enum 2     gi_1_6_us > HE related GI
			
			<enum 3     gi_3_2_us > HE related GI
			
			<legal 0 - 3>
*/
#define RECEIVE_USER_INFO_0_SGI_OFFSET                               0x00000000
#define RECEIVE_USER_INFO_0_SGI_LSB                                  29
#define RECEIVE_USER_INFO_0_SGI_MASK                                 0x60000000

/* Description		RECEIVE_USER_INFO_0_RESERVED_0
			
			<legal 0>
*/
#define RECEIVE_USER_INFO_0_RESERVED_0_OFFSET                        0x00000000
#define RECEIVE_USER_INFO_0_RESERVED_0_LSB                           31
#define RECEIVE_USER_INFO_0_RESERVED_0_MASK                          0x80000000

/* Description		RECEIVE_USER_INFO_1_RATE_MCS
			
			For details, refer to  MCS_TYPE description
			
			<legal all>
*/
#define RECEIVE_USER_INFO_1_RATE_MCS_OFFSET                          0x00000004
#define RECEIVE_USER_INFO_1_RATE_MCS_LSB                             0
#define RECEIVE_USER_INFO_1_RATE_MCS_MASK                            0x0000000f

/* Description		RECEIVE_USER_INFO_1_RECEPTION_TYPE
			
			Indicates what type of reception this is.
			
			<enum 0     reception_type_SU > 
			
			<enum 1     reception_type_MU_MIMO > 
			
			<enum 2     reception_type_MU_OFDMA > 
			
			<enum 3     reception_type_MU_OFDMA_MIMO > 
			
			<legal all>
*/
#define RECEIVE_USER_INFO_1_RECEPTION_TYPE_OFFSET                    0x00000004
#define RECEIVE_USER_INFO_1_RECEPTION_TYPE_LSB                       4
#define RECEIVE_USER_INFO_1_RECEPTION_TYPE_MASK                      0x00000030

/* Description		RECEIVE_USER_INFO_1_RECEIVE_BANDWIDTH
			
			Full receive Bandwidth
			
			
			
			<enum 0     full_rx_bw_20_mhz>
			
			<enum 1      full_rx_bw_40_mhz>
			
			<enum 2      full_rx_bw_80_mhz>
			
			<enum 3      full_rx_bw_160_mhz> 
			
			
			
			<legal 0-3>
*/
#define RECEIVE_USER_INFO_1_RECEIVE_BANDWIDTH_OFFSET                 0x00000004
#define RECEIVE_USER_INFO_1_RECEIVE_BANDWIDTH_LSB                    6
#define RECEIVE_USER_INFO_1_RECEIVE_BANDWIDTH_MASK                   0x000000c0

/* Description		RECEIVE_USER_INFO_1_MIMO_SS_BITMAP
			
			Field only valid in case of MIMO type reception
			
			
			
			Bitmap, with each bit indicating if the related spatial
			stream is used for this STA
			
			LSB related to SS 0
			
			
			
			0: spatial stream not used for this reception
			
			1: spatial stream used for this reception
			
			
			
			<legal all>
*/
#define RECEIVE_USER_INFO_1_MIMO_SS_BITMAP_OFFSET                    0x00000004
#define RECEIVE_USER_INFO_1_MIMO_SS_BITMAP_LSB                       8
#define RECEIVE_USER_INFO_1_MIMO_SS_BITMAP_MASK                      0x0000ff00

/* Description		RECEIVE_USER_INFO_1_USER_RU_ALLOCATION
			
			Field only valid in case of OFDMA uplink type reception
			
			
			
			Indicates the RU number for this user
			
			<legal 0-73>
*/
#define RECEIVE_USER_INFO_1_USER_RU_ALLOCATION_OFFSET                0x00000004
#define RECEIVE_USER_INFO_1_USER_RU_ALLOCATION_LSB                   16
#define RECEIVE_USER_INFO_1_USER_RU_ALLOCATION_MASK                  0x00ff0000

/* Description		RECEIVE_USER_INFO_1_NSS
			
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
#define RECEIVE_USER_INFO_1_NSS_OFFSET                               0x00000004
#define RECEIVE_USER_INFO_1_NSS_LSB                                  24
#define RECEIVE_USER_INFO_1_NSS_MASK                                 0x07000000

/* Description		RECEIVE_USER_INFO_1_RESERVED_1B
			
			<legal 0>
*/
#define RECEIVE_USER_INFO_1_RESERVED_1B_OFFSET                       0x00000004
#define RECEIVE_USER_INFO_1_RESERVED_1B_LSB                          27
#define RECEIVE_USER_INFO_1_RESERVED_1B_MASK                         0xf8000000


#endif // _RECEIVE_USER_INFO_H_

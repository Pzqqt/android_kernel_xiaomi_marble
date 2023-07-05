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

#ifndef _PHYRX_RSSI_LEGACY_H_
#define _PHYRX_RSSI_LEGACY_H_
#if !defined(__ASSEMBLER__)
#endif

#include "receive_rssi_info.h"
#define NUM_OF_DWORDS_PHYRX_RSSI_LEGACY 42

#define NUM_OF_QWORDS_PHYRX_RSSI_LEGACY 21


struct phyrx_rssi_legacy {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reception_type                                          :  4, // [3:0]
                      rx_chain_mask_type                                      :  1, // [4:4]
                      receive_bandwidth                                       :  3, // [7:5]
                      rx_chain_mask                                           :  8, // [15:8]
                      phy_ppdu_id                                             : 16; // [31:16]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t preamble_time_to_rxframe                                :  8, // [7:0]
                      standalone_snifer_mode                                  :  1, // [8:8]
                      reserved_5a                                             : 23; // [31:9]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             struct   receive_rssi_info                                         pre_rssi_info_details;
             struct   receive_rssi_info                                         preamble_rssi_info_details;
             uint32_t pre_rssi_comb                                           :  8, // [7:0]
                      rssi_comb                                               :  8, // [15:8]
                      normalized_pre_rssi_comb                                :  8, // [23:16]
                      normalized_rssi_comb                                    :  8; // [31:24]
             uint32_t rssi_comb_ppdu                                          :  8, // [7:0]
                      rssi_db_to_dbm_offset                                   :  8, // [15:8]
                      rssi_for_spatial_reuse                                  :  8, // [23:16]
                      rssi_for_trigger_resp                                   :  8; // [31:24]
#else
             uint32_t phy_ppdu_id                                             : 16, // [31:16]
                      rx_chain_mask                                           :  8, // [15:8]
                      receive_bandwidth                                       :  3, // [7:5]
                      rx_chain_mask_type                                      :  1, // [4:4]
                      reception_type                                          :  4; // [3:0]
             uint32_t sw_phy_meta_data                                        : 32; // [31:0]
             uint32_t ppdu_start_timestamp_31_0                               : 32; // [31:0]
             uint32_t ppdu_start_timestamp_63_32                              : 32; // [31:0]
             uint32_t reserved_4a                                             : 32; // [31:0]
             uint32_t reserved_5a                                             : 23, // [31:9]
                      standalone_snifer_mode                                  :  1, // [8:8]
                      preamble_time_to_rxframe                                :  8; // [7:0]
             uint32_t reserved_6a                                             : 32; // [31:0]
             uint32_t reserved_7a                                             : 32; // [31:0]
             struct   receive_rssi_info                                         pre_rssi_info_details;
             struct   receive_rssi_info                                         preamble_rssi_info_details;
             uint32_t normalized_rssi_comb                                    :  8, // [31:24]
                      normalized_pre_rssi_comb                                :  8, // [23:16]
                      rssi_comb                                               :  8, // [15:8]
                      pre_rssi_comb                                           :  8; // [7:0]
             uint32_t rssi_for_trigger_resp                                   :  8, // [31:24]
                      rssi_for_spatial_reuse                                  :  8, // [23:16]
                      rssi_db_to_dbm_offset                                   :  8, // [15:8]
                      rssi_comb_ppdu                                          :  8; // [7:0]
#endif
};


/* Description		RECEPTION_TYPE

			This field helps MAC SW determine which field in this (and
			 following TLVs) will contain valid information. For example
			 some RSSI info not valid in case of uplink_ofdma.. 
			
			In case of UL MU OFDMA or UL MU-MIMO reception pre-announced
			 by MAC during trigger Tx, e-nums 0 or 1 should be used.
			
			
			In case of UL MU OFDMA+MIMO reception, or in case of UL 
			MU reception when PHY has not been pre-informed, e-num 2
			 should be used.
			If this happens, the UL MU frame in the medium is by definition
			 not for this device.
			
			<enum 0 reception_is_uplink_ofdma>
			<enum 1 reception_is_uplink_mimo>
			<enum 2 reception_is_other>
			<enum 3 reception_is_frameless> PHY RX has been instructed
			 in advance that the upcoming reception is frameless. This
			 implieas that in advance it is known that all frames will
			 collide in the medium, and nothing can be properly decoded... 
			This can happen during the CTS reception in response to 
			the triggered MU-RTS transmission.
			MAC takes no action when seeing this e_num. For the frameless
			 reception the indication in pkt_end is the final one evaluated
			 by the MAC
			
			For the relationship between pkt_type and this field, see
			 the table at the end of this TLV description.
			<legal 0-3>
*/

#define PHYRX_RSSI_LEGACY_RECEPTION_TYPE_OFFSET                                     0x0000000000000000
#define PHYRX_RSSI_LEGACY_RECEPTION_TYPE_LSB                                        0
#define PHYRX_RSSI_LEGACY_RECEPTION_TYPE_MSB                                        3
#define PHYRX_RSSI_LEGACY_RECEPTION_TYPE_MASK                                       0x000000000000000f


/* Description		RX_CHAIN_MASK_TYPE

			Indicates if the field rx_chain_mask represents the mask
			 at start of reception (on which the Rssi_comb value is 
			based), or the setting used during the remainder of the 
			reception
			
			1'b0: rxtd.listen_pri80_mask 
			1'b1: Final receive mask
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_TYPE_OFFSET                                 0x0000000000000000
#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_TYPE_LSB                                    4
#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_TYPE_MSB                                    4
#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_TYPE_MASK                                   0x0000000000000010


/* Description		RECEIVE_BANDWIDTH

			Full receive Bandwidth
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define PHYRX_RSSI_LEGACY_RECEIVE_BANDWIDTH_OFFSET                                  0x0000000000000000
#define PHYRX_RSSI_LEGACY_RECEIVE_BANDWIDTH_LSB                                     5
#define PHYRX_RSSI_LEGACY_RECEIVE_BANDWIDTH_MSB                                     7
#define PHYRX_RSSI_LEGACY_RECEIVE_BANDWIDTH_MASK                                    0x00000000000000e0


/* Description		RX_CHAIN_MASK

			The chain mask at the start of the reception of this frame.
			
			
			each bit is one antenna
			0: the chain is NOT used
			1: the chain is used
			
			Supports up to 8 chains
			
			Used in 11ax TPC calculations for UL OFDMA/MIMO and has 
			to be in sync with the rssi_comb value as this is also used
			 by the MAC for the TPC calculations.
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_OFFSET                                      0x0000000000000000
#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_LSB                                         8
#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_MSB                                         15
#define PHYRX_RSSI_LEGACY_RX_CHAIN_MASK_MASK                                        0x000000000000ff00


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_PHY_PPDU_ID_OFFSET                                        0x0000000000000000
#define PHYRX_RSSI_LEGACY_PHY_PPDU_ID_LSB                                           16
#define PHYRX_RSSI_LEGACY_PHY_PPDU_ID_MSB                                           31
#define PHYRX_RSSI_LEGACY_PHY_PPDU_ID_MASK                                          0x00000000ffff0000


/* Description		SW_PHY_META_DATA

			32 bit Meta data that SW can program in a 32 bit PHY register
			 and PHY will insert the value in every RX_RSSI_LEGACY TLV
			 that it generates. 
			SW uses this field to embed among other things some SW channel
			 info.
*/

#define PHYRX_RSSI_LEGACY_SW_PHY_META_DATA_OFFSET                                   0x0000000000000000
#define PHYRX_RSSI_LEGACY_SW_PHY_META_DATA_LSB                                      32
#define PHYRX_RSSI_LEGACY_SW_PHY_META_DATA_MSB                                      63
#define PHYRX_RSSI_LEGACY_SW_PHY_META_DATA_MASK                                     0xffffffff00000000


/* Description		PPDU_START_TIMESTAMP_31_0

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, lower 32 bits
			
			Note that PHY will detect the start later, and will have
			 to derive out of the preamble info when the frame actually
			 appeared on the medium.
*/

#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_31_0_OFFSET                          0x0000000000000008
#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_31_0_LSB                             0
#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_31_0_MSB                             31
#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_31_0_MASK                            0x00000000ffffffff


/* Description		PPDU_START_TIMESTAMP_63_32

			Timestamp that indicates when the PPDU that contained this
			 MPDU started on the medium, upper 32 bits
			
			Note that PHY will detect the start later, and will have
			 to derive out of the preamble info when the frame actually
			 appeared on the medium.
*/

#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_63_32_OFFSET                         0x0000000000000008
#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_63_32_LSB                            32
#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_63_32_MSB                            63
#define PHYRX_RSSI_LEGACY_PPDU_START_TIMESTAMP_63_32_MASK                           0xffffffff00000000


/* Description		RESERVED_4A

			NOTE: DO not assign a field... Internally used in RXPCU 
			to store 'RX_PPDU_START::Rxframe_assert_timestamp.'
			<legal 0>
*/

#define PHYRX_RSSI_LEGACY_RESERVED_4A_OFFSET                                        0x0000000000000010
#define PHYRX_RSSI_LEGACY_RESERVED_4A_LSB                                           0
#define PHYRX_RSSI_LEGACY_RESERVED_4A_MSB                                           31
#define PHYRX_RSSI_LEGACY_RESERVED_4A_MASK                                          0x00000000ffffffff


/* Description		PREAMBLE_TIME_TO_RXFRAME

			The time taken (in us) from the frame starting on the medium
			 and PHY raising 'rx_frame'
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_TIME_TO_RXFRAME_OFFSET                           0x0000000000000010
#define PHYRX_RSSI_LEGACY_PREAMBLE_TIME_TO_RXFRAME_LSB                              32
#define PHYRX_RSSI_LEGACY_PREAMBLE_TIME_TO_RXFRAME_MSB                              39
#define PHYRX_RSSI_LEGACY_PREAMBLE_TIME_TO_RXFRAME_MASK                             0x000000ff00000000


/* Description		STANDALONE_SNIFER_MODE

			When set to 1, PHY has been configured to operate in the
			 stand alone sniffer mode.
			When 0, PHY is operating in the "normal" mission mode.
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_STANDALONE_SNIFER_MODE_OFFSET                             0x0000000000000010
#define PHYRX_RSSI_LEGACY_STANDALONE_SNIFER_MODE_LSB                                40
#define PHYRX_RSSI_LEGACY_STANDALONE_SNIFER_MODE_MSB                                40
#define PHYRX_RSSI_LEGACY_STANDALONE_SNIFER_MODE_MASK                               0x0000010000000000


/* Description		RESERVED_5A

			<legal 0>
*/

#define PHYRX_RSSI_LEGACY_RESERVED_5A_OFFSET                                        0x0000000000000010
#define PHYRX_RSSI_LEGACY_RESERVED_5A_LSB                                           41
#define PHYRX_RSSI_LEGACY_RESERVED_5A_MSB                                           63
#define PHYRX_RSSI_LEGACY_RESERVED_5A_MASK                                          0xfffffe0000000000


/* Description		RESERVED_6A

			NOTE: DO not assign a field... Internally used in RXPCU 
			to construct 'RX_PPDU_START.'
			<legal 0>
*/

#define PHYRX_RSSI_LEGACY_RESERVED_6A_OFFSET                                        0x0000000000000018
#define PHYRX_RSSI_LEGACY_RESERVED_6A_LSB                                           0
#define PHYRX_RSSI_LEGACY_RESERVED_6A_MSB                                           31
#define PHYRX_RSSI_LEGACY_RESERVED_6A_MASK                                          0x00000000ffffffff


/* Description		RESERVED_7A

			NOTE: DO not assign a field... Internally used in RXPCU 
			to construct 'RX_PPDU_START.'
			<legal 0>
*/

#define PHYRX_RSSI_LEGACY_RESERVED_7A_OFFSET                                        0x0000000000000018
#define PHYRX_RSSI_LEGACY_RESERVED_7A_LSB                                           32
#define PHYRX_RSSI_LEGACY_RESERVED_7A_MSB                                           63
#define PHYRX_RSSI_LEGACY_RESERVED_7A_MASK                                          0xffffffff00000000


/* Description		PRE_RSSI_INFO_DETAILS

			This field is not valid when reception_is_uplink_ofdma
			
			Overview of the pre-RSSI values. That is RSSI values measured
			 on the medium before this reception started.
*/


/* Description		RSSI_PRI20_CHAIN0

			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET            0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_LSB               0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MSB               7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MASK              0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_OFFSET            0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_LSB               8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MSB               15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MASK              0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_OFFSET      0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_LSB         16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MSB         23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MASK        0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_OFFSET     0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_LSB        24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MSB        31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MASK       0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_OFFSET      0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_LSB         32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MSB         39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MASK        0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET 0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB    40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MSB    47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK   0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET 0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB    48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MSB    55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK   0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_OFFSET     0x0000000000000020
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_LSB        56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MSB        63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MASK       0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_LSB            0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_MSB            7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_MASK           0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_LSB            8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_MSB            15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_MASK           0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_LSB            16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_MSB            23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_MASK           0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_LSB            24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_MSB            31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_MASK           0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_LSB            32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_MSB            39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_MASK           0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_LSB            40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_MSB            47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_MASK           0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_LSB            48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_MSB            55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_MASK           0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, highest 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_OFFSET         0x0000000000000028
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_LSB            56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_MSB            63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_MASK           0xff00000000000000


/* Description		RSSI_PRI20_CHAIN1

			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_OFFSET            0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_LSB               0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MSB               7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MASK              0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_OFFSET            0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_LSB               8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MSB               15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MASK              0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_OFFSET      0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_LSB         16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MSB         23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MASK        0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_OFFSET     0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_LSB        24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MSB        31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MASK       0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_OFFSET      0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_LSB         32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MSB         39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MASK        0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET 0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB    40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MSB    47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK   0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET 0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB    48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MSB    55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK   0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_OFFSET     0x0000000000000030
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_LSB        56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MSB        63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MASK       0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_LSB            0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_MSB            7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_MASK           0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_LSB            8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_MSB            15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_MASK           0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_LSB            16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_MSB            23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_MASK           0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_LSB            24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_MSB            31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_MASK           0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_LSB            32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_MSB            39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_MASK           0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_LSB            40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_MSB            47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_MASK           0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_LSB            48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_MSB            55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_MASK           0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, highest 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_OFFSET         0x0000000000000038
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_LSB            56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_MSB            63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_MASK           0xff00000000000000


/* Description		RSSI_PRI20_CHAIN2

			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_OFFSET            0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_LSB               0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MSB               7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MASK              0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_OFFSET            0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_LSB               8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MSB               15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MASK              0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_OFFSET      0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_LSB         16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MSB         23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MASK        0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_OFFSET     0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_LSB        24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MSB        31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MASK       0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_OFFSET      0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_LSB         32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MSB         39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MASK        0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET 0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB    40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MSB    47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK   0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET 0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB    48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MSB    55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK   0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_OFFSET     0x0000000000000040
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_LSB        56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MSB        63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MASK       0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_LSB            0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_MSB            7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_MASK           0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_LSB            8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_MSB            15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_MASK           0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_LSB            16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_MSB            23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_MASK           0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_LSB            24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_MSB            31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_MASK           0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_LSB            32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_MSB            39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_MASK           0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_LSB            40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_MSB            47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_MASK           0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_LSB            48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_MSB            55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_MASK           0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, highest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_OFFSET         0x0000000000000048
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_LSB            56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_MSB            63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_MASK           0xff00000000000000


/* Description		RSSI_PRI20_CHAIN3

			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_OFFSET            0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_LSB               0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MSB               7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MASK              0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_OFFSET            0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_LSB               8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MSB               15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MASK              0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_OFFSET      0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_LSB         16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MSB         23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MASK        0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_OFFSET     0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_LSB        24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MSB        31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MASK       0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_OFFSET      0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_LSB         32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MSB         39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MASK        0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET 0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB    40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MSB    47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK   0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET 0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB    48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MSB    55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK   0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_OFFSET     0x0000000000000050
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_LSB        56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MSB        63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MASK       0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_LSB            0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_MSB            7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_MASK           0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_LSB            8
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_MSB            15
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_MASK           0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_LSB            16
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_MSB            23
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_MASK           0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_LSB            24
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_MSB            31
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_MASK           0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_LSB            32
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_MSB            39
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_MASK           0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_LSB            40
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_MSB            47
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_MASK           0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_LSB            48
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_MSB            55
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_MASK           0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, highest 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_OFFSET         0x0000000000000058
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_LSB            56
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_MSB            63
#define PHYRX_RSSI_LEGACY_PRE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_MASK           0xff00000000000000


/* Description		PREAMBLE_RSSI_INFO_DETAILS

			This field is not valid when reception_is_uplink_ofdma
			
			Overview of the RSSI values measured during the pre-amble
			 phase of this reception
*/


/* Description		RSSI_PRI20_CHAIN0

			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET       0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_LSB          0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MSB          7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MASK         0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_OFFSET       0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_LSB          8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MSB          15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MASK         0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_OFFSET 0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_LSB    16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MSB    23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MASK   0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_OFFSET 0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_LSB   24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MSB   31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MASK  0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_OFFSET 0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_LSB    32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MSB    39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MASK   0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET 0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB 40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MSB 47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK 0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET 0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB 48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MSB 55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK 0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_OFFSET 0x0000000000000060
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_LSB   56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MSB   63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MASK  0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_LSB       0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_MSB       7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN0_MASK      0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_LSB       8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_MSB       15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN0_MASK      0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_LSB       16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_MSB       23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN0_MASK      0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_LSB       24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_MSB       31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN0_MASK      0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_LSB       32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_MSB       39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN0_MASK      0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_LSB       40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_MSB       47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN0_MASK      0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_LSB       48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_MSB       55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN0_MASK      0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN0

			RSSI of RX PPDU on chain 0 of extension 160, highest 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_OFFSET    0x0000000000000068
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_LSB       56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_MSB       63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN0_MASK      0xff00000000000000


/* Description		RSSI_PRI20_CHAIN1

			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_OFFSET       0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_LSB          0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MSB          7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MASK         0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_OFFSET       0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_LSB          8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MSB          15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MASK         0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_OFFSET 0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_LSB    16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MSB    23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MASK   0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_OFFSET 0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_LSB   24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MSB   31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MASK  0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_OFFSET 0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_LSB    32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MSB    39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MASK   0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET 0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB 40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MSB 47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK 0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET 0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB 48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MSB 55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK 0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_OFFSET 0x0000000000000070
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_LSB   56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MSB   63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MASK  0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_LSB       0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_MSB       7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN1_MASK      0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_LSB       8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_MSB       15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN1_MASK      0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_LSB       16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_MSB       23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN1_MASK      0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_LSB       24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_MSB       31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN1_MASK      0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_LSB       32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_MSB       39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN1_MASK      0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_LSB       40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_MSB       47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN1_MASK      0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_LSB       48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_MSB       55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN1_MASK      0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN1

			RSSI of RX PPDU on chain 1 of extension 160, highest 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_OFFSET    0x0000000000000078
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_LSB       56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_MSB       63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN1_MASK      0xff00000000000000


/* Description		RSSI_PRI20_CHAIN2

			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_OFFSET       0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_LSB          0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MSB          7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MASK         0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_OFFSET       0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_LSB          8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MSB          15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MASK         0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_OFFSET 0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_LSB    16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MSB    23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MASK   0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_OFFSET 0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_LSB   24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MSB   31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MASK  0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_OFFSET 0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_LSB    32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MSB    39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MASK   0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET 0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB 40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MSB 47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK 0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET 0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB 48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MSB 55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK 0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_OFFSET 0x0000000000000080
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_LSB   56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MSB   63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MASK  0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_LSB       0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_MSB       7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN2_MASK      0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_LSB       8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_MSB       15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN2_MASK      0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_LSB       16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_MSB       23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN2_MASK      0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_LSB       24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_MSB       31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN2_MASK      0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_LSB       32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_MSB       39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN2_MASK      0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_LSB       40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_MSB       47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN2_MASK      0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_LSB       48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_MSB       55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN2_MASK      0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN2

			RSSI of RX PPDU on chain 2 of extension 80, highest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_OFFSET    0x0000000000000088
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_LSB       56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_MSB       63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN2_MASK      0xff00000000000000


/* Description		RSSI_PRI20_CHAIN3

			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_OFFSET       0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_LSB          0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MSB          7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MASK         0x00000000000000ff


/* Description		RSSI_EXT20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_OFFSET       0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_LSB          8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MSB          15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MASK         0x000000000000ff00


/* Description		RSSI_EXT40_LOW20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_OFFSET 0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_LSB    16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MSB    23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MASK   0x0000000000ff0000


/* Description		RSSI_EXT40_HIGH20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_OFFSET 0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_LSB   24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MSB   31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MASK  0x00000000ff000000


/* Description		RSSI_EXT80_LOW20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz bandwidth. 
			 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_OFFSET 0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_LSB    32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MSB    39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MASK   0x000000ff00000000


/* Description		RSSI_EXT80_LOW_HIGH20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, low-high 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET 0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB 40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MSB 47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK 0x0000ff0000000000


/* Description		RSSI_EXT80_HIGH_LOW20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, high-low 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET 0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB 48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MSB 55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK 0x00ff000000000000


/* Description		RSSI_EXT80_HIGH20_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz 
			bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_OFFSET 0x0000000000000090
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_LSB   56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MSB   63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MASK  0xff00000000000000


/* Description		RSSI_EXT160_0_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, lowest 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_LSB       0
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_MSB       7
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_0_CHAIN3_MASK      0x00000000000000ff


/* Description		RSSI_EXT160_1_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth. 
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_LSB       8
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_MSB       15
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_1_CHAIN3_MASK      0x000000000000ff00


/* Description		RSSI_EXT160_2_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_LSB       16
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_MSB       23
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_2_CHAIN3_MASK      0x0000000000ff0000


/* Description		RSSI_EXT160_3_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_LSB       24
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_MSB       31
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_3_CHAIN3_MASK      0x00000000ff000000


/* Description		RSSI_EXT160_4_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_LSB       32
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_MSB       39
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_4_CHAIN3_MASK      0x000000ff00000000


/* Description		RSSI_EXT160_5_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_LSB       40
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_MSB       47
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_5_CHAIN3_MASK      0x0000ff0000000000


/* Description		RSSI_EXT160_6_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, next 20 MHz
			 bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_LSB       48
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_MSB       55
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_6_CHAIN3_MASK      0x00ff000000000000


/* Description		RSSI_EXT160_7_CHAIN3

			RSSI of RX PPDU on chain 3 of extension 160, highest 20 
			MHz bandwidth.  
			Value of 0x80 indicates invalid.
*/

#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_OFFSET    0x0000000000000098
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_LSB       56
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_MSB       63
#define PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT160_7_CHAIN3_MASK      0xff00000000000000


/* Description		PRE_RSSI_COMB

			Combined pre_rssi of all chains. Based on primary channel
			 RSSI.
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_PRE_RSSI_COMB_OFFSET                                      0x00000000000000a0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_COMB_LSB                                         0
#define PHYRX_RSSI_LEGACY_PRE_RSSI_COMB_MSB                                         7
#define PHYRX_RSSI_LEGACY_PRE_RSSI_COMB_MASK                                        0x00000000000000ff


/* Description		RSSI_COMB

			Combined rssi of all chains. Based on primary channel RSSI.
			
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RSSI_COMB_OFFSET                                          0x00000000000000a0
#define PHYRX_RSSI_LEGACY_RSSI_COMB_LSB                                             8
#define PHYRX_RSSI_LEGACY_RSSI_COMB_MSB                                             15
#define PHYRX_RSSI_LEGACY_RSSI_COMB_MASK                                            0x000000000000ff00


/* Description		NORMALIZED_PRE_RSSI_COMB

			Combined pre_rssi of all chains, but "normalized" back to
			 a single chain. This avoids PDG from having to evaluate
			 this in combination with receive chain mask and perform
			 all kinds of pre-processing algorithms.
			
			Based on primary channel RSSI.
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_NORMALIZED_PRE_RSSI_COMB_OFFSET                           0x00000000000000a0
#define PHYRX_RSSI_LEGACY_NORMALIZED_PRE_RSSI_COMB_LSB                              16
#define PHYRX_RSSI_LEGACY_NORMALIZED_PRE_RSSI_COMB_MSB                              23
#define PHYRX_RSSI_LEGACY_NORMALIZED_PRE_RSSI_COMB_MASK                             0x0000000000ff0000


/* Description		NORMALIZED_RSSI_COMB

			Combined rssi of all chains, but "normalized" back to a 
			single chain. This avoids PDG from having to evaluate this
			 in combination with receive chain mask and perform all 
			kinds of pre-processing algorithms.
			
			Based on primary channel RSSI.
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_NORMALIZED_RSSI_COMB_OFFSET                               0x00000000000000a0
#define PHYRX_RSSI_LEGACY_NORMALIZED_RSSI_COMB_LSB                                  24
#define PHYRX_RSSI_LEGACY_NORMALIZED_RSSI_COMB_MSB                                  31
#define PHYRX_RSSI_LEGACY_NORMALIZED_RSSI_COMB_MASK                                 0x00000000ff000000


/* Description		RSSI_COMB_PPDU

			Combined rssi of all chains, based on active RUs/subchannels, 
			a.k.a. rssi_pkt_bw_mac
			
			RSSI is reported as 8b signed values. Nominally value is
			 in dB units above or below the noisefloor(minCCApwr). 
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			When packet BW is 20 MHz,
			rssi_comb_ppdu = rssi_comb.
			
			When packet BW > 20 MHz,
			rssi_comb < rssi_comb_ppdu because rssi_comb only includes
			 power of primary 20 MHz while rssi_comb_ppdu includes power
			 of active RUs/subchannels.
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RSSI_COMB_PPDU_OFFSET                                     0x00000000000000a0
#define PHYRX_RSSI_LEGACY_RSSI_COMB_PPDU_LSB                                        32
#define PHYRX_RSSI_LEGACY_RSSI_COMB_PPDU_MSB                                        39
#define PHYRX_RSSI_LEGACY_RSSI_COMB_PPDU_MASK                                       0x000000ff00000000


/* Description		RSSI_DB_TO_DBM_OFFSET

			Offset between 'dB' and 'dBm' values. SW can use this value
			 to convert RSSI 'dBm' values back to 'dB,' and report both
			 the values.
			
			When rssi_db_to_dbm_offset = 0,
			all rssi_xxx fields are defined in dB.
			
			When rssi_db_to_dbm_offset is a large negative value, all
			 rssi_xxx fields are defined in dBm.
			
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RSSI_DB_TO_DBM_OFFSET_OFFSET                              0x00000000000000a0
#define PHYRX_RSSI_LEGACY_RSSI_DB_TO_DBM_OFFSET_LSB                                 40
#define PHYRX_RSSI_LEGACY_RSSI_DB_TO_DBM_OFFSET_MSB                                 47
#define PHYRX_RSSI_LEGACY_RSSI_DB_TO_DBM_OFFSET_MASK                                0x0000ff0000000000


/* Description		RSSI_FOR_SPATIAL_REUSE

			RSSI to be used by HWSCH for transmit (power) selection 
			during an SR opportunity, reported as an 8-bit signed value
			
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			As per 802.11ax draft 3.3 subsubclauses 27.10.2.2/3, for
			 OBSS PD spatial reuse, the received signal strength level
			 should be measured from the L-STF or L-LTF (but not L-SIG), 
			just as measured to indicate CCA.
			
			Also, as per 802.11ax draft 3.3, for OBSS PD spatial reuse, 
			MAC should compare this value with its programmed OBSS_PDlevel
			 scaled from 20 MHz to the Rx PPDU bandwidth. Since MAC 
			does not do this scaling, PHY is instead expected to normalize
			 the reported RSSI to 20 MHz.
			
			Also as per 802.11ax draft 3.3 subsubclause 27.10.3.2, for
			 SRP spatial reuse, the received power level should be measured
			 from the L-STF or L-LTF (but not L-SIG) and normalized 
			to 20 MHz.
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RSSI_FOR_SPATIAL_REUSE_OFFSET                             0x00000000000000a0
#define PHYRX_RSSI_LEGACY_RSSI_FOR_SPATIAL_REUSE_LSB                                48
#define PHYRX_RSSI_LEGACY_RSSI_FOR_SPATIAL_REUSE_MSB                                55
#define PHYRX_RSSI_LEGACY_RSSI_FOR_SPATIAL_REUSE_MASK                               0x00ff000000000000


/* Description		RSSI_FOR_TRIGGER_RESP

			RSSI to be used by PDG for transmit (power) selection during
			 trigger response, reported as an 8-bit signed value
			
			The resolution can be: 
			1dB or 0.5dB. This is statically configured within the PHY
			 and MAC
			
			In case of 1dB, the Range is:
			 -128dB to 127dB
			
			In case of 0.5dB, the Range is:
			 -64dB to 63.5dB
			
			As per 802.11ax draft 3.3 subsubclauses 28.3.14.2, for trigger
			 response, the received power should be measured from the
			 non-HE portion of the preamble of the PPDU containing the
			 trigger, normalized to 20 MHz, averaged over the antennas
			 over which the average pathloss is being computed.
			<legal all>
*/

#define PHYRX_RSSI_LEGACY_RSSI_FOR_TRIGGER_RESP_OFFSET                              0x00000000000000a0
#define PHYRX_RSSI_LEGACY_RSSI_FOR_TRIGGER_RESP_LSB                                 56
#define PHYRX_RSSI_LEGACY_RSSI_FOR_TRIGGER_RESP_MSB                                 63
#define PHYRX_RSSI_LEGACY_RSSI_FOR_TRIGGER_RESP_MASK                                0xff00000000000000



#endif   // PHYRX_RSSI_LEGACY

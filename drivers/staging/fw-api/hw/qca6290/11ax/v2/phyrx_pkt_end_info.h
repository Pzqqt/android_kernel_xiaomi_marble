/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _PHYRX_PKT_END_INFO_H_
#define _PHYRX_PKT_END_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_location_info.h"
#include "rx_timing_offset_info.h"
#include "receive_rssi_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	phy_internal_nap[0], location_info_valid[1], timing_info_valid[2], rssi_info_valid[3], rx_frame_correction_needed[4], frameless_frame_received[5], reserved_0a[11:6], dl_ofdma_info_valid[12], dl_ofdma_ru_start_index[19:13], dl_ofdma_ru_width[26:20], reserved_0b[31:27]
//	1	phy_timestamp_1_lower_32[31:0]
//	2	phy_timestamp_1_upper_32[31:0]
//	3	phy_timestamp_2_lower_32[31:0]
//	4	phy_timestamp_2_upper_32[31:0]
//	5-13	struct rx_location_info rx_location_info_details;
//	14	struct rx_timing_offset_info rx_timing_offset_info_details;
//	15-30	struct receive_rssi_info post_rssi_info_details;
//	31	phy_sw_status_31_0[31:0]
//	32	phy_sw_status_63_32[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_PKT_END_INFO 33

struct phyrx_pkt_end_info {
             uint32_t phy_internal_nap                :  1, //[0]
                      location_info_valid             :  1, //[1]
                      timing_info_valid               :  1, //[2]
                      rssi_info_valid                 :  1, //[3]
                      rx_frame_correction_needed      :  1, //[4]
                      frameless_frame_received        :  1, //[5]
                      reserved_0a                     :  6, //[11:6]
                      dl_ofdma_info_valid             :  1, //[12]
                      dl_ofdma_ru_start_index         :  7, //[19:13]
                      dl_ofdma_ru_width               :  7, //[26:20]
                      reserved_0b                     :  5; //[31:27]
             uint32_t phy_timestamp_1_lower_32        : 32; //[31:0]
             uint32_t phy_timestamp_1_upper_32        : 32; //[31:0]
             uint32_t phy_timestamp_2_lower_32        : 32; //[31:0]
             uint32_t phy_timestamp_2_upper_32        : 32; //[31:0]
    struct            rx_location_info                       rx_location_info_details;
    struct            rx_timing_offset_info                       rx_timing_offset_info_details;
    struct            receive_rssi_info                       post_rssi_info_details;
             uint32_t phy_sw_status_31_0              : 32; //[31:0]
             uint32_t phy_sw_status_63_32             : 32; //[31:0]
};

/*

phy_internal_nap
			
			When set, PHY RX entered an internal NAP state, as PHY
			determined that this reception was not destined to this
			device

location_info_valid
			
			Indicates that the RX_LOCATION_INFO structure later on
			in the TLV contains valid info

timing_info_valid
			
			Indicates that the RX_TIMING_OFFSET_INFO structure later
			on in the TLV contains valid info

rssi_info_valid
			
			Indicates that the RECEIVE_RSSI_INFO structure later on
			in the TLV contains valid info

rx_frame_correction_needed
			
			When clear, no action is needed in the MAC.
			
			
			
			When set, the falling edge of the rx_frame happened 4us
			too late. MAC will need to compensate for this delay in
			order to maintain proper SIFS timing and/or not to get
			de-slotted.
			
			
			
			PHY uses this for very short 11a frames. 
			
			
			
			When set, PHY will have passed this TLV to the MAC up to
			8 us into the 'real SIFS' time, and thus within 4us from the
			falling edge of the rx_frame.
			
			
			
			<legal all>

frameless_frame_received
			
			When set, PHY has received the 'frameless frame' . Can
			be used in the 'MU-RTS -CTS exchange where CTS reception can
			be problematic.
			
			<legal all>

reserved_0a
			
			<legal 0>

dl_ofdma_info_valid
			
			When set, the following DL_ofdma_... fields are valid.
			
			It provides the MAC insight into which RU was allocated
			to this device. 
			
			<legal all>

dl_ofdma_ru_start_index
			
			RU index number to which User is assigned
			
			RU numbering is over the entire BW, starting from 0
			
			<legal 0-73>

dl_ofdma_ru_width
			
			The size of the RU for this user.
			
			In units of 1 (26 tone) RU
			
			<legal 1-74>

reserved_0b
			
			<legal 0>

phy_timestamp_1_lower_32
			
			TODO PHY: cleanup descriptionThe PHY timestamp in the
			AMPI of the first rising edge of rx_clear_pri after
			TX_PHY_DESC. .  This field should set to 0 by the PHY and
			should be updated by the AMPI before being forwarded to the
			rest of the MAC. This field indicates the lower 32 bits of
			the timestamp

phy_timestamp_1_upper_32
			
			TODO PHY: cleanup description 
			
			The PHY timestamp in the AMPI of the first rising edge
			of rx_clear_pri after TX_PHY_DESC.  This field should set to
			0 by the PHY and should be updated by the AMPI before being
			forwarded to the rest of the MAC. This field indicates the
			upper 32 bits of the timestamp

phy_timestamp_2_lower_32
			
			TODO PHY: cleanup description 
			
			The PHY timestamp in the AMPI of the rising edge of
			rx_clear_pri after RX_RSSI_LEGACY.  This field should set to
			0 by the PHY and should be updated by the AMPI before being
			forwarded to the rest of the MAC. This field indicates the
			lower 32 bits of the timestamp

phy_timestamp_2_upper_32
			
			TODO PHY: cleanup description 
			
			The PHY timestamp in the AMPI of the rising edge of
			rx_clear_pri after RX_RSSI_LEGACY.  This field should set to
			0 by the PHY and should be updated by the AMPI before being
			forwarded to the rest of the MAC. This field indicates the
			upper 32 bits of the timestamp

struct rx_location_info rx_location_info_details
			
			Overview of location related info 

struct rx_timing_offset_info rx_timing_offset_info_details
			
			Overview of timing offset related info

struct receive_rssi_info post_rssi_info_details
			
			Overview of the post-RSSI values. 

phy_sw_status_31_0
			
			Some PHY micro code status that can be put in here.
			Details of definition within SW specification
			
			This field can be used for debugging, FW - SW message
			exchange, etc.
			
			It could for example be a pointer to a DDR memory
			location where PHY FW put some debug info.
			
			<legal all>

phy_sw_status_63_32
			
			Some PHY micro code status that can be put in here.
			Details of definition within SW specification
			
			This field can be used for debugging, FW - SW message
			exchange, etc.
			
			It could for example be a pointer to a DDR memory
			location where PHY FW put some debug info.
			
			<legal all>
*/


/* Description		PHYRX_PKT_END_INFO_0_PHY_INTERNAL_NAP
			
			When set, PHY RX entered an internal NAP state, as PHY
			determined that this reception was not destined to this
			device
*/
#define PHYRX_PKT_END_INFO_0_PHY_INTERNAL_NAP_OFFSET                 0x00000000
#define PHYRX_PKT_END_INFO_0_PHY_INTERNAL_NAP_LSB                    0
#define PHYRX_PKT_END_INFO_0_PHY_INTERNAL_NAP_MASK                   0x00000001

/* Description		PHYRX_PKT_END_INFO_0_LOCATION_INFO_VALID
			
			Indicates that the RX_LOCATION_INFO structure later on
			in the TLV contains valid info
*/
#define PHYRX_PKT_END_INFO_0_LOCATION_INFO_VALID_OFFSET              0x00000000
#define PHYRX_PKT_END_INFO_0_LOCATION_INFO_VALID_LSB                 1
#define PHYRX_PKT_END_INFO_0_LOCATION_INFO_VALID_MASK                0x00000002

/* Description		PHYRX_PKT_END_INFO_0_TIMING_INFO_VALID
			
			Indicates that the RX_TIMING_OFFSET_INFO structure later
			on in the TLV contains valid info
*/
#define PHYRX_PKT_END_INFO_0_TIMING_INFO_VALID_OFFSET                0x00000000
#define PHYRX_PKT_END_INFO_0_TIMING_INFO_VALID_LSB                   2
#define PHYRX_PKT_END_INFO_0_TIMING_INFO_VALID_MASK                  0x00000004

/* Description		PHYRX_PKT_END_INFO_0_RSSI_INFO_VALID
			
			Indicates that the RECEIVE_RSSI_INFO structure later on
			in the TLV contains valid info
*/
#define PHYRX_PKT_END_INFO_0_RSSI_INFO_VALID_OFFSET                  0x00000000
#define PHYRX_PKT_END_INFO_0_RSSI_INFO_VALID_LSB                     3
#define PHYRX_PKT_END_INFO_0_RSSI_INFO_VALID_MASK                    0x00000008

/* Description		PHYRX_PKT_END_INFO_0_RX_FRAME_CORRECTION_NEEDED
			
			When clear, no action is needed in the MAC.
			
			
			
			When set, the falling edge of the rx_frame happened 4us
			too late. MAC will need to compensate for this delay in
			order to maintain proper SIFS timing and/or not to get
			de-slotted.
			
			
			
			PHY uses this for very short 11a frames. 
			
			
			
			When set, PHY will have passed this TLV to the MAC up to
			8 us into the 'real SIFS' time, and thus within 4us from the
			falling edge of the rx_frame.
			
			
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_0_RX_FRAME_CORRECTION_NEEDED_OFFSET       0x00000000
#define PHYRX_PKT_END_INFO_0_RX_FRAME_CORRECTION_NEEDED_LSB          4
#define PHYRX_PKT_END_INFO_0_RX_FRAME_CORRECTION_NEEDED_MASK         0x00000010

/* Description		PHYRX_PKT_END_INFO_0_FRAMELESS_FRAME_RECEIVED
			
			When set, PHY has received the 'frameless frame' . Can
			be used in the 'MU-RTS -CTS exchange where CTS reception can
			be problematic.
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_0_FRAMELESS_FRAME_RECEIVED_OFFSET         0x00000000
#define PHYRX_PKT_END_INFO_0_FRAMELESS_FRAME_RECEIVED_LSB            5
#define PHYRX_PKT_END_INFO_0_FRAMELESS_FRAME_RECEIVED_MASK           0x00000020

/* Description		PHYRX_PKT_END_INFO_0_RESERVED_0A
			
			<legal 0>
*/
#define PHYRX_PKT_END_INFO_0_RESERVED_0A_OFFSET                      0x00000000
#define PHYRX_PKT_END_INFO_0_RESERVED_0A_LSB                         6
#define PHYRX_PKT_END_INFO_0_RESERVED_0A_MASK                        0x00000fc0

/* Description		PHYRX_PKT_END_INFO_0_DL_OFDMA_INFO_VALID
			
			When set, the following DL_ofdma_... fields are valid.
			
			It provides the MAC insight into which RU was allocated
			to this device. 
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_INFO_VALID_OFFSET              0x00000000
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_INFO_VALID_LSB                 12
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_INFO_VALID_MASK                0x00001000

/* Description		PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_START_INDEX
			
			RU index number to which User is assigned
			
			RU numbering is over the entire BW, starting from 0
			
			<legal 0-73>
*/
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_START_INDEX_OFFSET          0x00000000
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_START_INDEX_LSB             13
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_START_INDEX_MASK            0x000fe000

/* Description		PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_WIDTH
			
			The size of the RU for this user.
			
			In units of 1 (26 tone) RU
			
			<legal 1-74>
*/
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_WIDTH_OFFSET                0x00000000
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_WIDTH_LSB                   20
#define PHYRX_PKT_END_INFO_0_DL_OFDMA_RU_WIDTH_MASK                  0x07f00000

/* Description		PHYRX_PKT_END_INFO_0_RESERVED_0B
			
			<legal 0>
*/
#define PHYRX_PKT_END_INFO_0_RESERVED_0B_OFFSET                      0x00000000
#define PHYRX_PKT_END_INFO_0_RESERVED_0B_LSB                         27
#define PHYRX_PKT_END_INFO_0_RESERVED_0B_MASK                        0xf8000000

/* Description		PHYRX_PKT_END_INFO_1_PHY_TIMESTAMP_1_LOWER_32
			
			TODO PHY: cleanup descriptionThe PHY timestamp in the
			AMPI of the first rising edge of rx_clear_pri after
			TX_PHY_DESC. .  This field should set to 0 by the PHY and
			should be updated by the AMPI before being forwarded to the
			rest of the MAC. This field indicates the lower 32 bits of
			the timestamp
*/
#define PHYRX_PKT_END_INFO_1_PHY_TIMESTAMP_1_LOWER_32_OFFSET         0x00000004
#define PHYRX_PKT_END_INFO_1_PHY_TIMESTAMP_1_LOWER_32_LSB            0
#define PHYRX_PKT_END_INFO_1_PHY_TIMESTAMP_1_LOWER_32_MASK           0xffffffff

/* Description		PHYRX_PKT_END_INFO_2_PHY_TIMESTAMP_1_UPPER_32
			
			TODO PHY: cleanup description 
			
			The PHY timestamp in the AMPI of the first rising edge
			of rx_clear_pri after TX_PHY_DESC.  This field should set to
			0 by the PHY and should be updated by the AMPI before being
			forwarded to the rest of the MAC. This field indicates the
			upper 32 bits of the timestamp
*/
#define PHYRX_PKT_END_INFO_2_PHY_TIMESTAMP_1_UPPER_32_OFFSET         0x00000008
#define PHYRX_PKT_END_INFO_2_PHY_TIMESTAMP_1_UPPER_32_LSB            0
#define PHYRX_PKT_END_INFO_2_PHY_TIMESTAMP_1_UPPER_32_MASK           0xffffffff

/* Description		PHYRX_PKT_END_INFO_3_PHY_TIMESTAMP_2_LOWER_32
			
			TODO PHY: cleanup description 
			
			The PHY timestamp in the AMPI of the rising edge of
			rx_clear_pri after RX_RSSI_LEGACY.  This field should set to
			0 by the PHY and should be updated by the AMPI before being
			forwarded to the rest of the MAC. This field indicates the
			lower 32 bits of the timestamp
*/
#define PHYRX_PKT_END_INFO_3_PHY_TIMESTAMP_2_LOWER_32_OFFSET         0x0000000c
#define PHYRX_PKT_END_INFO_3_PHY_TIMESTAMP_2_LOWER_32_LSB            0
#define PHYRX_PKT_END_INFO_3_PHY_TIMESTAMP_2_LOWER_32_MASK           0xffffffff

/* Description		PHYRX_PKT_END_INFO_4_PHY_TIMESTAMP_2_UPPER_32
			
			TODO PHY: cleanup description 
			
			The PHY timestamp in the AMPI of the rising edge of
			rx_clear_pri after RX_RSSI_LEGACY.  This field should set to
			0 by the PHY and should be updated by the AMPI before being
			forwarded to the rest of the MAC. This field indicates the
			upper 32 bits of the timestamp
*/
#define PHYRX_PKT_END_INFO_4_PHY_TIMESTAMP_2_UPPER_32_OFFSET         0x00000010
#define PHYRX_PKT_END_INFO_4_PHY_TIMESTAMP_2_UPPER_32_LSB            0
#define PHYRX_PKT_END_INFO_4_PHY_TIMESTAMP_2_UPPER_32_MASK           0xffffffff
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000014
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000018
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000024
#define PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000028
#define PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x0000002c
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000030
#define PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_OFFSET 0x00000034
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_RX_LOCATION_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_RX_TIMING_OFFSET_INFO_DETAILS_OFFSET 0x00000038
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_RX_TIMING_OFFSET_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_RX_TIMING_OFFSET_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_15_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x0000003c
#define PHYRX_PKT_END_INFO_15_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_15_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_16_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000040
#define PHYRX_PKT_END_INFO_16_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_16_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_17_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000044
#define PHYRX_PKT_END_INFO_17_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_17_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_18_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000048
#define PHYRX_PKT_END_INFO_18_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_18_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_19_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x0000004c
#define PHYRX_PKT_END_INFO_19_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_19_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_20_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000050
#define PHYRX_PKT_END_INFO_20_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_20_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_21_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000054
#define PHYRX_PKT_END_INFO_21_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_21_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_22_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000058
#define PHYRX_PKT_END_INFO_22_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_22_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_23_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x0000005c
#define PHYRX_PKT_END_INFO_23_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_23_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_24_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000060
#define PHYRX_PKT_END_INFO_24_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_24_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_25_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000064
#define PHYRX_PKT_END_INFO_25_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_25_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_26_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000068
#define PHYRX_PKT_END_INFO_26_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_26_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_27_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x0000006c
#define PHYRX_PKT_END_INFO_27_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_27_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_28_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000070
#define PHYRX_PKT_END_INFO_28_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_28_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_29_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000074
#define PHYRX_PKT_END_INFO_29_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_29_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff
#define PHYRX_PKT_END_INFO_30_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_OFFSET 0x00000078
#define PHYRX_PKT_END_INFO_30_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_LSB 0
#define PHYRX_PKT_END_INFO_30_RECEIVE_RSSI_INFO_POST_RSSI_INFO_DETAILS_MASK 0xffffffff

/* Description		PHYRX_PKT_END_INFO_31_PHY_SW_STATUS_31_0
			
			Some PHY micro code status that can be put in here.
			Details of definition within SW specification
			
			This field can be used for debugging, FW - SW message
			exchange, etc.
			
			It could for example be a pointer to a DDR memory
			location where PHY FW put some debug info.
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_31_PHY_SW_STATUS_31_0_OFFSET              0x0000007c
#define PHYRX_PKT_END_INFO_31_PHY_SW_STATUS_31_0_LSB                 0
#define PHYRX_PKT_END_INFO_31_PHY_SW_STATUS_31_0_MASK                0xffffffff

/* Description		PHYRX_PKT_END_INFO_32_PHY_SW_STATUS_63_32
			
			Some PHY micro code status that can be put in here.
			Details of definition within SW specification
			
			This field can be used for debugging, FW - SW message
			exchange, etc.
			
			It could for example be a pointer to a DDR memory
			location where PHY FW put some debug info.
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_32_PHY_SW_STATUS_63_32_OFFSET             0x00000080
#define PHYRX_PKT_END_INFO_32_PHY_SW_STATUS_63_32_LSB                0
#define PHYRX_PKT_END_INFO_32_PHY_SW_STATUS_63_32_MASK               0xffffffff


#endif // _PHYRX_PKT_END_INFO_H_

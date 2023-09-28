/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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
			
			RU numbering is over the entire BW, starting from 0 and
			in increasing frequency order and not primary-secondary
			order
			
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
			
			RU numbering is over the entire BW, starting from 0 and
			in increasing frequency order and not primary-secondary
			order
			
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

 /* EXTERNAL REFERENCE : struct rx_location_info rx_location_info_details */ 


/* Description		PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY
			
			For 20/40/80, this field shows the RTT first arrival
			correction value computed from L-LTF on the first selected
			Rx chain
			
			
			
			For 80+80, this field shows the RTT first arrival
			correction value computed from L-LTF on pri80 on the
			selected pri80 Rx chain
			
			
			
			16 bits, signed 12.4. 12 bits integer to cover -6.4us to
			6.4us, and 4 bits fraction to cover pri80 and 32x FAC
			interpolation
			
			
			
			clock unit is 320MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_OFFSET 0x00000014
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_LSB 0
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_MASK 0x0000ffff

/* Description		PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80
			
			For 20/40/80, this field shows the RTT first arrival
			correction value computed from L-LTF on the second selected
			Rx chain
			
			
			
			For 80+80, this field shows the RTT first arrival
			correction value computed from L-LTF on ext80 on the
			selected ext80 Rx chain
			
			
			
			16 bits, signed 12.4. 12 bits integer to cover -6.4us to
			6.4us, and 4 bits fraction to cover ext80 and 32x FAC
			interpolation
			
			
			
			clock unit is 320MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_OFFSET 0x00000014
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_LSB 16
#define PHYRX_PKT_END_INFO_5_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_MASK 0xffff0000

/* Description		PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT
			
			For 20/40/80, this field shows the RTT first arrival
			correction value computed from (V)HT/HE-LTF on the first
			selected Rx chain
			
			
			
			For 80+80, this field shows the RTT first arrival
			correction value computed from (V)HT/HE-LTF on pri80 on the
			selected pri80 Rx chain
			
			
			
			16 bits, signed 12.4. 12 bits integer to cover -6.4us to
			6.4us, and 4 bits fraction to cover pri80 and 32x FAC
			interpolation
			
			
			
			clock unit is 320MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_OFFSET 0x00000018
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_LSB 0
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_MASK 0x0000ffff

/* Description		PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80
			
			For 20/40/80, this field shows the RTT first arrival
			correction value computed from (V)HT/HE-LTF on the second
			selected Rx chain
			
			
			
			For 80+80, this field shows the RTT first arrival
			correction value computed from (V)HT/HE-LTF on ext80 on the
			selected ext80 Rx chain
			
			
			
			16 bits, signed 12.4. 12 bits integer to cover -6.4us to
			6.4us, and 4 bits fraction to cover ext80 and 32x FAC
			interpolation
			
			
			
			clock unit is 320MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_OFFSET 0x00000018
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_LSB 16
#define PHYRX_PKT_END_INFO_6_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_MASK 0xffff0000

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_STATUS
			
			Status of rtt_fac_legacy
			
			
			
			<enum 0 location_fac_legacy_status_not_valid>
			
			<enum 1 location_fac_legacy_status_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_STATUS_LSB 0
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_STATUS_MASK 0x00000001

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_STATUS
			
			Status of rtt_fac_legacy_ext80
			
			
			
			<enum 0 location_fac_legacy_ext80_status_not_valid>
			
			<enum 1 location_fac_legacy_ext80_status_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_STATUS_LSB 1
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_LEGACY_EXT80_STATUS_MASK 0x00000002

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_STATUS
			
			Status of rtt_fac_vht
			
			
			
			<enum 0 location_fac_vht_status_not_valid>
			
			<enum 1 location_fac_vht_status_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_STATUS_LSB 2
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_STATUS_MASK 0x00000004

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_STATUS
			
			Status of rtt_fac_vht_ext80
			
			
			
			<enum 0 location_fac_vht_ext80_status_not_valid>
			
			<enum 1 location_fac_vht_ext80_status_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_STATUS_LSB 3
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_VHT_EXT80_STATUS_MASK 0x00000008

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS
			
			To support fine SIFS adjustment, need to provide FAC
			value @ integer number of 320 MHz clock cycles to MAC.  It
			is from L-LTF if it is a Legacy packet and from (V)HT/HE-LTF
			if it is a (V)HT/HE packet
			
			
			
			12 bits, signed, no fractional part
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_LSB 4
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_MASK 0x0000fff0

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_STATUS
			
			Status of rtt_fac_sifs
			
			0: not valid
			
			1: valid and from L-LTF
			
			2: valid and from (V)HT/HE-LTF
			
			3: reserved
			
			<legal 0-2>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_STATUS_LSB 16
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_FAC_SIFS_STATUS_MASK 0x00030000

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS
			
			Status of channel frequency response dump
			
			
			
			<enum 0 location_CFR_dump_not_valid>
			
			<enum 1 location_CFR_dump_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_LSB 18
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_MASK 0x00040000

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS
			
			Status of channel impulse response dump
			
			
			
			<enum 0 location_CIR_dump_not_valid>
			
			<enum 1 location_CIR_dump_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_LSB 19
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_MASK 0x00080000

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CHANNEL_DUMP_SIZE
			
			Channel dump size.  It shows how many tones in CFR in
			one chain, for example, it will show 52 for Legacy20 and 484
			for VHT160
			
			
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CHANNEL_DUMP_SIZE_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CHANNEL_DUMP_SIZE_LSB 20
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_CHANNEL_DUMP_SIZE_MASK 0x7ff00000

/* Description		PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE
			
			Indicator showing if HW IFFT mode or SW IFFT mode
			
			
			
			<enum 0 location_sw_ifft_mode>
			
			<enum 1 location_hw_ifft_mode>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_OFFSET 0x0000001c
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_LSB 31
#define PHYRX_PKT_END_INFO_7_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_MASK 0x80000000

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_BTCF_STATUS
			
			Indicate if BTCF is used to capture the timestamps
			
			
			
			<enum 0 location_not_BTCF_based_ts>
			
			<enum 1 location_BTCF_based_ts>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_BTCF_STATUS_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_BTCF_STATUS_LSB 0
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_BTCF_STATUS_MASK 0x00000001

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE
			
			Indicate preamble type
			
			
			
			<enum 0 location_preamble_type_legacy>
			
			<enum 1 location_preamble_type_ht>
			
			<enum 2 location_preamble_type_vht>
			
			<enum 3 location_preamble_type_he_su_4xltf>
			
			<enum 4 location_preamble_type_he_su_2xltf>
			
			<enum 5 location_preamble_type_he_su_1xltf>
			
			<enum 6
			location_preamble_type_he_trigger_based_ul_4xltf>
			
			<enum 7
			location_preamble_type_he_trigger_based_ul_2xltf>
			
			<enum 8
			location_preamble_type_he_trigger_based_ul_1xltf>
			
			<enum 9 location_preamble_type_he_mu_4xltf>
			
			<enum 10 location_preamble_type_he_mu_2xltf>
			
			<enum 11 location_preamble_type_he_mu_1xltf>
			
			<enum 12
			location_preamble_type_he_extended_range_su_4xltf>
			
			<enum 13
			location_preamble_type_he_extended_range_su_2xltf>
			
			<enum 14
			location_preamble_type_he_extended_range_su_1xltf>
			
			<legal 0-14>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_LSB 1
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_MASK 0x0000003e

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG
			
			Indicate the bandwidth of L-LTF
			
			
			
			<enum 0 location_pkt_bw_20MHz>
			
			<enum 1 location_pkt_bw_40MHz>
			
			<enum 2 location_pkt_bw_80MHz>
			
			<enum 3 location_pkt_bw_160MHz>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_LSB 6
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_MASK 0x000000c0

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT
			
			Indicate the bandwidth of (V)HT/HE-LTF
			
			
			
			<enum 0 location_pkt_bw_20MHz>
			
			<enum 1 location_pkt_bw_40MHz>
			
			<enum 2 location_pkt_bw_80MHz>
			
			<enum 3 location_pkt_bw_160MHz>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_LSB 8
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_MASK 0x00000300

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE
			
			Indicate GI (guard interval) type
			
			
			
			<enum 0     gi_0_8_us > HE related GI. Can also be used
			for HE
			
			<enum 1     gi_0_4_us > HE related GI. Can also be used
			for HE
			
			<enum 2     gi_1_6_us > HE related GI
			
			<enum 3     gi_3_2_us > HE related GI
			
			<legal 0 - 3>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_LSB 10
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_MASK 0x00000c00

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE
			
			Bits 0~4 indicate MCS rate, if Legacy, 
			
			0: 48 Mbps,
			
			1: 24 Mbps,
			
			2: 12 Mbps,
			
			3: 6 Mbps,
			
			4: 54 Mbps,
			
			5: 36 Mbps,
			
			6: 18 Mbps,
			
			7: 9 Mbps,
			
			
			
			if HT, 0-7: MCS0-MCS7, 
			
			if VHT, 0-9: MCS0-MCS9, 
			
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_LSB 12
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_MASK 0x0001f000

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN
			
			For 20/40/80, this field shows the first selected Rx
			chain that is used in HW IFFT mode
			
			
			
			For 80+80, this field shows the selected pri80 Rx chain
			that is used in HW IFFT mode
			
			
			
			<enum 0 location_strongest_chain_is_0>
			
			<enum 1 location_strongest_chain_is_1>
			
			<enum 2 location_strongest_chain_is_2>
			
			<enum 3 location_strongest_chain_is_3>
			
			<enum 4 location_strongest_chain_is_4>
			
			<enum 5 location_strongest_chain_is_5>
			
			<enum 6 location_strongest_chain_is_6>
			
			<enum 7 location_strongest_chain_is_7>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_LSB 17
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_MASK 0x000e0000

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_EXT80
			
			For 20/40/80, this field shows the second selected Rx
			chain that is used in HW IFFT mode
			
			
			
			For 80+80, this field shows the selected ext80 Rx chain
			that is used in HW IFFT mode
			
			
			
			<enum 0 location_strongest_chain_is_0>
			
			<enum 1 location_strongest_chain_is_1>
			
			<enum 2 location_strongest_chain_is_2>
			
			<enum 3 location_strongest_chain_is_3>
			
			<enum 4 location_strongest_chain_is_4>
			
			<enum 5 location_strongest_chain_is_5>
			
			<enum 6 location_strongest_chain_is_6>
			
			<enum 7 location_strongest_chain_is_7>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_EXT80_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_EXT80_LSB 20
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_STRONGEST_CHAIN_EXT80_MASK 0x00700000

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK
			
			Rx chain mask, each bit is a Rx chain
			
			0: the Rx chain is not used
			
			1: the Rx chain is used
			
			Support up to 8 Rx chains
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_LSB 23
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_MASK 0x7f800000

/* Description		PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RESERVED_3
			
			<legal 0>
*/
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RESERVED_3_OFFSET 0x00000020
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RESERVED_3_LSB 31
#define PHYRX_PKT_END_INFO_8_RX_LOCATION_INFO_DETAILS_RESERVED_3_MASK 0x80000000

/* Description		PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_DETAILS_RX_START_TS
			
			RX packet start timestamp
			
			
			
			It reports the time the first L-STF ADC sample arrived
			at RX antenna
			
			
			
			clock unit is 480MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_DETAILS_RX_START_TS_OFFSET 0x00000024
#define PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_DETAILS_RX_START_TS_LSB 0
#define PHYRX_PKT_END_INFO_9_RX_LOCATION_INFO_DETAILS_RX_START_TS_MASK 0xffffffff

/* Description		PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_DETAILS_RX_END_TS
			
			RX packet end timestamp
			
			
			
			It reports the time the last symbol's last ADC sample
			arrived at RX antenna
			
			
			
			clock unit is 480MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_DETAILS_RX_END_TS_OFFSET 0x00000028
#define PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_DETAILS_RX_END_TS_LSB 0
#define PHYRX_PKT_END_INFO_10_RX_LOCATION_INFO_DETAILS_RX_END_TS_MASK 0xffffffff

/* Description		PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_START
			
			The phase of the SFO of the first symbol's first FFT
			input sample
			
			
			
			12 bits, signed 6.6. 6 bits integer to cover -66.7ns to
			66.7ns, and 6 bits fraction to provide a resolution of
			0.03ns
			
			
			
			clock unit is 480MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_START_OFFSET 0x0000002c
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_START_LSB 0
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_START_MASK 0x00000fff

/* Description		PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_END
			
			The phase of the SFO of the last symbol's last FFT input
			sample
			
			
			
			12 bits, signed 6.6. 6 bits integer to cover -66.7ns to
			66.7ns, and 6 bits fraction to provide a resolution of
			0.03ns
			
			
			
			clock unit is 480MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_END_OFFSET 0x0000002c
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_END_LSB 12
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_SFO_PHASE_PKT_END_MASK 0x00fff000

/* Description		PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8
			
			The high 8 bits of the 40 bits pointer pointed to the
			external RTT channel information buffer
			
			
			
			8 bits
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_OFFSET 0x0000002c
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_LSB 24
#define PHYRX_PKT_END_INFO_11_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32
			
			The low 32 bits of the 40 bits pointer pointed to the
			external RTT channel information buffer
			
			
			
			32 bits
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_OFFSET 0x00000030
#define PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_LSB 0
#define PHYRX_PKT_END_INFO_12_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_MASK 0xffffffff

/* Description		PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT
			
			CFO measurement. Needed for passive locationing
			
			
			
			14 bits, signed 1.13. 13 bits fraction to provide a
			resolution of 153 Hz
			
			
			
			In units of cycles/800 ns
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_OFFSET 0x00000034
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_LSB 0
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_MASK 0x00003fff

/* Description		PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CHAN_SPREAD
			
			Channel delay spread measurement. Needed for selecting
			GI length
			
			
			
			8 bits, unsigned. At 25 ns step. Can represent up to
			6375 ns
			
			
			
			In units of cycles @ 40 MHz
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CHAN_SPREAD_OFFSET 0x00000034
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CHAN_SPREAD_LSB 14
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_CHAN_SPREAD_MASK 0x003fc000

/* Description		PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL
			
			Indicate which timing backoff value is used
			
			
			
			<enum 0 timing_backoff_low_rssi>
			
			<enum 1 timing_backoff_mid_rssi>
			
			<enum 2 timing_backoff_high_rssi>
			
			<enum 3 reserved>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_OFFSET 0x00000034
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_LSB 22
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_MASK 0x00c00000

/* Description		PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RESERVED_8
			
			<legal 0>
*/
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RESERVED_8_OFFSET 0x00000034
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RESERVED_8_LSB 24
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RESERVED_8_MASK 0x7f000000

/* Description		PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID
			
			<enum 0 rx_location_info_is_not_valid>
			
			<enum 1 rx_location_info_is_valid>
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_OFFSET 0x00000034
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_LSB 31
#define PHYRX_PKT_END_INFO_13_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_MASK 0x80000000

 /* EXTERNAL REFERENCE : struct rx_timing_offset_info rx_timing_offset_info_details */ 


/* Description		PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESIDUAL_PHASE_OFFSET
			
			Cumulative reference frequency error at end of RX
			
			<legal all>
*/
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESIDUAL_PHASE_OFFSET_OFFSET 0x00000038
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESIDUAL_PHASE_OFFSET_LSB 0
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESIDUAL_PHASE_OFFSET_MASK 0x00000fff

/* Description		PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESERVED
			
			<legal 0>
*/
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESERVED_OFFSET 0x00000038
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESERVED_LSB 12
#define PHYRX_PKT_END_INFO_14_RX_TIMING_OFFSET_INFO_DETAILS_RESERVED_MASK 0xfffff000

 /* EXTERNAL REFERENCE : struct receive_rssi_info post_rssi_info_details */ 


/* Description		PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET 0x0000003c
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_LSB 0
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_OFFSET 0x0000003c
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_LSB 8
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_OFFSET 0x0000003c
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_LSB 16
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_OFFSET 0x0000003c
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_LSB 24
#define PHYRX_PKT_END_INFO_15_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_OFFSET 0x00000040
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_LSB 0
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET 0x00000040
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB 8
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET 0x00000040
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB 16
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_OFFSET 0x00000040
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_LSB 24
#define PHYRX_PKT_END_INFO_16_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_OFFSET 0x00000044
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_LSB 0
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_OFFSET 0x00000044
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_LSB 8
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_OFFSET 0x00000044
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_LSB 16
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_OFFSET 0x00000044
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_LSB 24
#define PHYRX_PKT_END_INFO_17_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_OFFSET 0x00000048
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_LSB 0
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET 0x00000048
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB 8
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET 0x00000048
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB 16
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_OFFSET 0x00000048
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_LSB 24
#define PHYRX_PKT_END_INFO_18_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_OFFSET 0x0000004c
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_LSB 0
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_OFFSET 0x0000004c
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_LSB 8
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_OFFSET 0x0000004c
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_LSB 16
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_OFFSET 0x0000004c
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_LSB 24
#define PHYRX_PKT_END_INFO_19_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_OFFSET 0x00000050
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_LSB 0
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET 0x00000050
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB 8
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET 0x00000050
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB 16
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_OFFSET 0x00000050
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_LSB 24
#define PHYRX_PKT_END_INFO_20_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_OFFSET 0x00000054
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_LSB 0
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_OFFSET 0x00000054
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_LSB 8
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_OFFSET 0x00000054
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_LSB 16
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_OFFSET 0x00000054
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_LSB 24
#define PHYRX_PKT_END_INFO_21_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_OFFSET 0x00000058
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_LSB 0
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET 0x00000058
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB 8
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET 0x00000058
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB 16
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_OFFSET 0x00000058
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_LSB 24
#define PHYRX_PKT_END_INFO_22_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_OFFSET 0x0000005c
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_LSB 0
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_OFFSET 0x0000005c
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_LSB 8
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_OFFSET 0x0000005c
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_LSB 16
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_OFFSET 0x0000005c
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_LSB 24
#define PHYRX_PKT_END_INFO_23_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_OFFSET 0x00000060
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_LSB 0
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_OFFSET 0x00000060
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_LSB 8
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_OFFSET 0x00000060
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_LSB 16
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_OFFSET 0x00000060
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_LSB 24
#define PHYRX_PKT_END_INFO_24_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_OFFSET 0x00000064
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_LSB 0
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_OFFSET 0x00000064
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_LSB 8
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_OFFSET 0x00000064
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_LSB 16
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_OFFSET 0x00000064
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_LSB 24
#define PHYRX_PKT_END_INFO_25_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_OFFSET 0x00000068
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_LSB 0
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_OFFSET 0x00000068
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_LSB 8
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_OFFSET 0x00000068
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_LSB 16
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_OFFSET 0x00000068
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_LSB 24
#define PHYRX_PKT_END_INFO_26_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_OFFSET 0x0000006c
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_LSB 0
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_OFFSET 0x0000006c
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_LSB 8
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_OFFSET 0x0000006c
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_LSB 16
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_OFFSET 0x0000006c
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_LSB 24
#define PHYRX_PKT_END_INFO_27_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_OFFSET 0x00000070
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_LSB 0
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_OFFSET 0x00000070
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_LSB 8
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_OFFSET 0x00000070
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_LSB 16
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_OFFSET 0x00000070
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_LSB 24
#define PHYRX_PKT_END_INFO_28_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_OFFSET 0x00000074
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_LSB 0
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_OFFSET 0x00000074
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_LSB 8
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_OFFSET 0x00000074
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_LSB 16
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_OFFSET 0x00000074
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_LSB 24
#define PHYRX_PKT_END_INFO_29_POST_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_MASK 0xff000000

/* Description		PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_OFFSET 0x00000078
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_LSB 0
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_MASK 0x000000ff

/* Description		PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_OFFSET 0x00000078
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_LSB 8
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_MASK 0x0000ff00

/* Description		PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_OFFSET 0x00000078
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_LSB 16
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_MASK 0x00ff0000

/* Description		PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_OFFSET 0x00000078
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_LSB 24
#define PHYRX_PKT_END_INFO_30_POST_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_MASK 0xff000000

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

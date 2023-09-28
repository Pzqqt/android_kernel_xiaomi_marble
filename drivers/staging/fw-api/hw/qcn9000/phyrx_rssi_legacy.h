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

#ifndef _PHYRX_RSSI_LEGACY_H_
#define _PHYRX_RSSI_LEGACY_H_
#if !defined(__ASSEMBLER__)
#endif

#include "receive_rssi_info.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	reception_type[3:0], rx_chain_mask_type[4], reserved_0[5], receive_bandwidth[7:6], rx_chain_mask[15:8], phy_ppdu_id[31:16]
//	1	sw_phy_meta_data[31:0]
//	2	ppdu_start_timestamp[31:0]
//	3-18	struct receive_rssi_info pre_rssi_info_details;
//	19-34	struct receive_rssi_info preamble_rssi_info_details;
//	35	pre_rssi_comb[7:0], rssi_comb[15:8], normalized_pre_rssi_comb[23:16], normalized_rssi_comb[31:24]
//	36	rssi_comb_ppdu[7:0], rssi_db_to_dbm_offset[15:8], rssi_for_spatial_reuse[23:16], rssi_for_trigger_resp[31:24]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_PHYRX_RSSI_LEGACY 37

struct phyrx_rssi_legacy {
             uint32_t reception_type                  :  4, //[3:0]
                      rx_chain_mask_type              :  1, //[4]
                      reserved_0                      :  1, //[5]
                      receive_bandwidth               :  2, //[7:6]
                      rx_chain_mask                   :  8, //[15:8]
                      phy_ppdu_id                     : 16; //[31:16]
             uint32_t sw_phy_meta_data                : 32; //[31:0]
             uint32_t ppdu_start_timestamp            : 32; //[31:0]
    struct            receive_rssi_info                       pre_rssi_info_details;
    struct            receive_rssi_info                       preamble_rssi_info_details;
             uint32_t pre_rssi_comb                   :  8, //[7:0]
                      rssi_comb                       :  8, //[15:8]
                      normalized_pre_rssi_comb        :  8, //[23:16]
                      normalized_rssi_comb            :  8; //[31:24]
             uint32_t rssi_comb_ppdu                  :  8, //[7:0]
                      rssi_db_to_dbm_offset           :  8, //[15:8]
                      rssi_for_spatial_reuse          :  8, //[23:16]
                      rssi_for_trigger_resp           :  8; //[31:24]
};

/*

reception_type
			
			This field helps MAC SW determine which field in this
			(and following TLVs) will contain valid information. For
			example some RSSI info not valid in case of uplink_ofdma.. 
			
			
			
			In case of UL MU OFDMA or UL MU-MIMO reception
			pre-announced by MAC during trigger Tx, e-nums 0 or 1 should
			be used.
			
			
			
			In case of UL MU OFDMA+MIMO reception, or in case of UL
			MU reception when PHY has not been pre-informed, e-num 2
			should be used.
			
			If this happens, the UL MU frame in the medium is by
			definition not for this device.
			
			As reference, see doc:
			
			Lithium_mac_phy_interface_hld.docx
			
			Chapter: 7.15.1: 11ax UL MU Reception TLV sequences when
			this device is not targeted.
			
			
			
			<enum 0 reception_is_uplink_ofdma>
			
			<enum 1 reception_is_uplink_mimo>
			
			<enum 2 reception_is_other>
			
			<enum 3 reception_is_frameless> PHY RX has been
			instructed in advance that the upcoming reception is
			frameless. This implieas that in advance it is known that
			all frames will collide in the medium, and nothing can be
			properly decoded... This can happen during the CTS reception
			in response to the triggered MU-RTS transmission.
			
			MAC takes no action when seeing this e_num. For the
			frameless reception the indication in pkt_end is the final
			one evaluated by the MAC
			
			
			
			For the relationship between pkt_type and this field,
			see the table at the end of this TLV description.
			
			<legal 0-3>

rx_chain_mask_type
			
			Indicates if the field rx_chain_mask represents the mask
			at start of reception (on which the Rssi_comb value is
			based), or the setting used during the remainder of the
			reception
			
			
			
			1'b0: rxtd.listen_pri80_mask 
			
			1'b1: Final receive mask
			
			
			
			<legal all>

reserved_0
			
			<legal 0>

receive_bandwidth
			
			Full receive Bandwidth
			
			
			
			<enum 0     full_rx_bw_20_mhz>
			
			<enum 1      full_rx_bw_40_mhz>
			
			<enum 2      full_rx_bw_80_mhz>
			
			<enum 3      full_rx_bw_160_mhz> 
			
			
			
			<legal 0-3>

rx_chain_mask
			
			The chain mask at the start of the reception of this
			frame.
			
			
			
			each bit is one antenna
			
			0: the chain is NOT used
			
			1: the chain is used
			
			
			
			Supports up to 8 chains
			
			
			
			Used in 11ax TPC calculations for UL OFDMA/MIMO and has
			to be in sync with the rssi_comb value as this is also used
			by the MAC for the TPC calculations.
			
			<legal all>

phy_ppdu_id
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>

sw_phy_meta_data
			
			32 bit Meta data that SW can program in a 32 bit PHY
			register and PHY will insert the value in every
			RX_RSSI_LEGACY TLV that it generates. 
			
			SW uses this field to embed among other things some SW
			channel info.

ppdu_start_timestamp
			
			Timestamp that indicates when the PPDU that contained
			this MPDU started on the medium.
			
			
			
			Note that PHY will detect the start later, and will have
			to derive out of the preamble info when the frame actually
			appeared on the medium
			
			<legal 0- 10>

struct receive_rssi_info pre_rssi_info_details
			
			This field is not valid when reception_is_uplink_ofdma
			
			
			
			Overview of the pre-RSSI values. That is RSSI values
			measured on the medium before this reception started.

struct receive_rssi_info preamble_rssi_info_details
			
			This field is not valid when reception_is_uplink_ofdma
			
			
			
			Overview of the RSSI values measured during the
			pre-amble phase of this reception

pre_rssi_comb
			
			Combined pre_rssi of all chains. Based on primary
			channel RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>

rssi_comb
			
			Combined rssi of all chains. Based on primary channel
			RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>

normalized_pre_rssi_comb
			
			Combined pre_rssi of all chains, but normalized back to
			a single chain. This avoids PDG from having to evaluate this
			in combination with receive chain mask and perform all kinds
			of pre-processing algorithms.
			
			
			
			Based on primary channel RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>

normalized_rssi_comb
			
			Combined rssi of all chains, but normalized back to a
			single chain. This avoids PDG from having to evaluate this
			in combination with receive chain mask and perform all kinds
			of pre-processing algorithms.
			
			
			
			Based on primary channel RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>

rssi_comb_ppdu
			
			Combined rssi of all chains, based on active
			RUs/subchannels, a.k.a. rssi_pkt_bw_mac
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			When packet BW is 20 MHz,
			
			rssi_comb_ppdu = rssi_comb.
			
			
			
			When packet BW > 20 MHz,
			
			rssi_comb < rssi_comb_ppdu because rssi_comb only
			includes power of primary 20 MHz while rssi_comb_ppdu
			includes power of active RUs/subchannels.
			
			
			
			<legal all>

rssi_db_to_dbm_offset
			
			Offset between 'dB' and 'dBm' values. SW can use this
			value to convert RSSI 'dBm' values back to 'dB,' and report
			both the values.
			
			
			
			When rssi_db_to_dbm_offset = 0,
			
			all rssi_xxx fields are defined in dB.
			
			
			
			When rssi_db_to_dbm_offset is a large negative value,
			all rssi_xxx fields are defined in dBm.
			
			
			
			<legal all>

rssi_for_spatial_reuse
			
			<legal all>

rssi_for_trigger_resp
			
			RSSI to be used by PDG for transmit (power) selection
			during trigger response, reported as an 8-bit signed value
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			As per 802.11ax draft 3.3 subsubclauses 28.3.14.2, for
			trigger response, the received power should be measured from
			the non-HE portion of the preamble of the PPDU containing
			the trigger, normalized to 20 MHz, averaged over the
			antennas over which the average pathloss is being computed.
			
			<legal all>
*/


/* Description		PHYRX_RSSI_LEGACY_0_RECEPTION_TYPE
			
			This field helps MAC SW determine which field in this
			(and following TLVs) will contain valid information. For
			example some RSSI info not valid in case of uplink_ofdma.. 
			
			
			
			In case of UL MU OFDMA or UL MU-MIMO reception
			pre-announced by MAC during trigger Tx, e-nums 0 or 1 should
			be used.
			
			
			
			In case of UL MU OFDMA+MIMO reception, or in case of UL
			MU reception when PHY has not been pre-informed, e-num 2
			should be used.
			
			If this happens, the UL MU frame in the medium is by
			definition not for this device.
			
			As reference, see doc:
			
			Lithium_mac_phy_interface_hld.docx
			
			Chapter: 7.15.1: 11ax UL MU Reception TLV sequences when
			this device is not targeted.
			
			
			
			<enum 0 reception_is_uplink_ofdma>
			
			<enum 1 reception_is_uplink_mimo>
			
			<enum 2 reception_is_other>
			
			<enum 3 reception_is_frameless> PHY RX has been
			instructed in advance that the upcoming reception is
			frameless. This implieas that in advance it is known that
			all frames will collide in the medium, and nothing can be
			properly decoded... This can happen during the CTS reception
			in response to the triggered MU-RTS transmission.
			
			MAC takes no action when seeing this e_num. For the
			frameless reception the indication in pkt_end is the final
			one evaluated by the MAC
			
			
			
			For the relationship between pkt_type and this field,
			see the table at the end of this TLV description.
			
			<legal 0-3>
*/
#define PHYRX_RSSI_LEGACY_0_RECEPTION_TYPE_OFFSET                    0x00000000
#define PHYRX_RSSI_LEGACY_0_RECEPTION_TYPE_LSB                       0
#define PHYRX_RSSI_LEGACY_0_RECEPTION_TYPE_MASK                      0x0000000f

/* Description		PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_TYPE
			
			Indicates if the field rx_chain_mask represents the mask
			at start of reception (on which the Rssi_comb value is
			based), or the setting used during the remainder of the
			reception
			
			
			
			1'b0: rxtd.listen_pri80_mask 
			
			1'b1: Final receive mask
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_TYPE_OFFSET                0x00000000
#define PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_TYPE_LSB                   4
#define PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_TYPE_MASK                  0x00000010

/* Description		PHYRX_RSSI_LEGACY_0_RESERVED_0
			
			<legal 0>
*/
#define PHYRX_RSSI_LEGACY_0_RESERVED_0_OFFSET                        0x00000000
#define PHYRX_RSSI_LEGACY_0_RESERVED_0_LSB                           5
#define PHYRX_RSSI_LEGACY_0_RESERVED_0_MASK                          0x00000020

/* Description		PHYRX_RSSI_LEGACY_0_RECEIVE_BANDWIDTH
			
			Full receive Bandwidth
			
			
			
			<enum 0     full_rx_bw_20_mhz>
			
			<enum 1      full_rx_bw_40_mhz>
			
			<enum 2      full_rx_bw_80_mhz>
			
			<enum 3      full_rx_bw_160_mhz> 
			
			
			
			<legal 0-3>
*/
#define PHYRX_RSSI_LEGACY_0_RECEIVE_BANDWIDTH_OFFSET                 0x00000000
#define PHYRX_RSSI_LEGACY_0_RECEIVE_BANDWIDTH_LSB                    6
#define PHYRX_RSSI_LEGACY_0_RECEIVE_BANDWIDTH_MASK                   0x000000c0

/* Description		PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK
			
			The chain mask at the start of the reception of this
			frame.
			
			
			
			each bit is one antenna
			
			0: the chain is NOT used
			
			1: the chain is used
			
			
			
			Supports up to 8 chains
			
			
			
			Used in 11ax TPC calculations for UL OFDMA/MIMO and has
			to be in sync with the rssi_comb value as this is also used
			by the MAC for the TPC calculations.
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_OFFSET                     0x00000000
#define PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_LSB                        8
#define PHYRX_RSSI_LEGACY_0_RX_CHAIN_MASK_MASK                       0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_0_PHY_PPDU_ID
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_0_PHY_PPDU_ID_OFFSET                       0x00000000
#define PHYRX_RSSI_LEGACY_0_PHY_PPDU_ID_LSB                          16
#define PHYRX_RSSI_LEGACY_0_PHY_PPDU_ID_MASK                         0xffff0000

/* Description		PHYRX_RSSI_LEGACY_1_SW_PHY_META_DATA
			
			32 bit Meta data that SW can program in a 32 bit PHY
			register and PHY will insert the value in every
			RX_RSSI_LEGACY TLV that it generates. 
			
			SW uses this field to embed among other things some SW
			channel info.
*/
#define PHYRX_RSSI_LEGACY_1_SW_PHY_META_DATA_OFFSET                  0x00000004
#define PHYRX_RSSI_LEGACY_1_SW_PHY_META_DATA_LSB                     0
#define PHYRX_RSSI_LEGACY_1_SW_PHY_META_DATA_MASK                    0xffffffff

/* Description		PHYRX_RSSI_LEGACY_2_PPDU_START_TIMESTAMP
			
			Timestamp that indicates when the PPDU that contained
			this MPDU started on the medium.
			
			
			
			Note that PHY will detect the start later, and will have
			to derive out of the preamble info when the frame actually
			appeared on the medium
			
			<legal 0- 10>
*/
#define PHYRX_RSSI_LEGACY_2_PPDU_START_TIMESTAMP_OFFSET              0x00000008
#define PHYRX_RSSI_LEGACY_2_PPDU_START_TIMESTAMP_LSB                 0
#define PHYRX_RSSI_LEGACY_2_PPDU_START_TIMESTAMP_MASK                0xffffffff

 /* EXTERNAL REFERENCE : struct receive_rssi_info pre_rssi_info_details */ 


/* Description		PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET 0x0000000c
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_LSB 0
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_OFFSET 0x0000000c
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_LSB 8
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_OFFSET 0x0000000c
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_LSB 16
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_OFFSET 0x0000000c
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_LSB 24
#define PHYRX_RSSI_LEGACY_3_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_OFFSET 0x00000010
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_LSB 0
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET 0x00000010
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB 8
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET 0x00000010
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB 16
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_OFFSET 0x00000010
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_LSB 24
#define PHYRX_RSSI_LEGACY_4_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_OFFSET 0x00000014
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_LSB 0
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_OFFSET 0x00000014
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_LSB 8
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_OFFSET 0x00000014
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_LSB 16
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_OFFSET 0x00000014
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_LSB 24
#define PHYRX_RSSI_LEGACY_5_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_OFFSET 0x00000018
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_LSB 0
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET 0x00000018
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB 8
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET 0x00000018
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB 16
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_OFFSET 0x00000018
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_LSB 24
#define PHYRX_RSSI_LEGACY_6_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_OFFSET 0x0000001c
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_LSB 0
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_OFFSET 0x0000001c
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_LSB 8
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_OFFSET 0x0000001c
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_LSB 16
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_OFFSET 0x0000001c
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_LSB 24
#define PHYRX_RSSI_LEGACY_7_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_OFFSET 0x00000020
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_LSB 0
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET 0x00000020
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB 8
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET 0x00000020
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB 16
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_OFFSET 0x00000020
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_LSB 24
#define PHYRX_RSSI_LEGACY_8_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_OFFSET 0x00000024
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_LSB 0
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_OFFSET 0x00000024
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_LSB 8
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_OFFSET 0x00000024
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_LSB 16
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_OFFSET 0x00000024
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_LSB 24
#define PHYRX_RSSI_LEGACY_9_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_OFFSET 0x00000028
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_LSB 0
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET 0x00000028
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB 8
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET 0x00000028
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB 16
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_OFFSET 0x00000028
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_LSB 24
#define PHYRX_RSSI_LEGACY_10_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_OFFSET 0x0000002c
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_LSB 0
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_OFFSET 0x0000002c
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_LSB 8
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_OFFSET 0x0000002c
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_LSB 16
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_OFFSET 0x0000002c
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_LSB 24
#define PHYRX_RSSI_LEGACY_11_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_OFFSET 0x00000030
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_LSB 0
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_OFFSET 0x00000030
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_LSB 8
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_OFFSET 0x00000030
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_LSB 16
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_OFFSET 0x00000030
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_LSB 24
#define PHYRX_RSSI_LEGACY_12_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_OFFSET 0x00000034
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_LSB 0
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_OFFSET 0x00000034
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_LSB 8
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_OFFSET 0x00000034
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_LSB 16
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_OFFSET 0x00000034
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_LSB 24
#define PHYRX_RSSI_LEGACY_13_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_OFFSET 0x00000038
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_LSB 0
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_OFFSET 0x00000038
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_LSB 8
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_OFFSET 0x00000038
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_LSB 16
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_OFFSET 0x00000038
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_LSB 24
#define PHYRX_RSSI_LEGACY_14_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_OFFSET 0x0000003c
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_LSB 0
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_OFFSET 0x0000003c
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_LSB 8
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_OFFSET 0x0000003c
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_LSB 16
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_OFFSET 0x0000003c
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_LSB 24
#define PHYRX_RSSI_LEGACY_15_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_OFFSET 0x00000040
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_LSB 0
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_OFFSET 0x00000040
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_LSB 8
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_OFFSET 0x00000040
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_LSB 16
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_OFFSET 0x00000040
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_LSB 24
#define PHYRX_RSSI_LEGACY_16_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_OFFSET 0x00000044
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_LSB 0
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_OFFSET 0x00000044
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_LSB 8
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_OFFSET 0x00000044
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_LSB 16
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_OFFSET 0x00000044
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_LSB 24
#define PHYRX_RSSI_LEGACY_17_PRE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_OFFSET 0x00000048
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_LSB 0
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_OFFSET 0x00000048
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_LSB 8
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_OFFSET 0x00000048
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_LSB 16
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_OFFSET 0x00000048
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_LSB 24
#define PHYRX_RSSI_LEGACY_18_PRE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_MASK 0xff000000

 /* EXTERNAL REFERENCE : struct receive_rssi_info preamble_rssi_info_details */ 


/* Description		PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET 0x0000004c
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_LSB 0
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_OFFSET 0x0000004c
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_LSB 8
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN0_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_OFFSET 0x0000004c
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_LSB 16
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN0_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_OFFSET 0x0000004c
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_LSB 24
#define PHYRX_RSSI_LEGACY_19_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN0_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_OFFSET 0x00000050
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_LSB 0
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN0_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET 0x00000050
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB 8
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET 0x00000050
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB 16
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_OFFSET 0x00000050
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_LSB 24
#define PHYRX_RSSI_LEGACY_20_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN0_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_OFFSET 0x00000054
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_LSB 0
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN1_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_OFFSET 0x00000054
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_LSB 8
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN1_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_OFFSET 0x00000054
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_LSB 16
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN1_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_OFFSET 0x00000054
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_LSB 24
#define PHYRX_RSSI_LEGACY_21_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN1_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_OFFSET 0x00000058
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_LSB 0
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN1_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET 0x00000058
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB 8
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET 0x00000058
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB 16
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_OFFSET 0x00000058
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_LSB 24
#define PHYRX_RSSI_LEGACY_22_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN1_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_OFFSET 0x0000005c
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_LSB 0
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN2_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_OFFSET 0x0000005c
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_LSB 8
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN2_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_OFFSET 0x0000005c
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_LSB 16
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN2_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_OFFSET 0x0000005c
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_LSB 24
#define PHYRX_RSSI_LEGACY_23_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN2_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_OFFSET 0x00000060
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_LSB 0
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN2_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET 0x00000060
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB 8
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET 0x00000060
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB 16
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_OFFSET 0x00000060
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_LSB 24
#define PHYRX_RSSI_LEGACY_24_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN2_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_OFFSET 0x00000064
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_LSB 0
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN3_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_OFFSET 0x00000064
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_LSB 8
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN3_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_OFFSET 0x00000064
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_LSB 16
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN3_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_OFFSET 0x00000064
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_LSB 24
#define PHYRX_RSSI_LEGACY_25_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN3_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_OFFSET 0x00000068
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_LSB 0
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN3_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET 0x00000068
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB 8
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET 0x00000068
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB 16
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_OFFSET 0x00000068
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_LSB 24
#define PHYRX_RSSI_LEGACY_26_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN3_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_OFFSET 0x0000006c
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_LSB 0
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN4_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_OFFSET 0x0000006c
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_LSB 8
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN4_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_OFFSET 0x0000006c
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_LSB 16
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN4_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_OFFSET 0x0000006c
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_LSB 24
#define PHYRX_RSSI_LEGACY_27_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN4_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_OFFSET 0x00000070
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_LSB 0
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN4_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_OFFSET 0x00000070
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_LSB 8
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN4_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_OFFSET 0x00000070
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_LSB 16
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN4_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_OFFSET 0x00000070
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_LSB 24
#define PHYRX_RSSI_LEGACY_28_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN4_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_OFFSET 0x00000074
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_LSB 0
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN5_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_OFFSET 0x00000074
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_LSB 8
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN5_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_OFFSET 0x00000074
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_LSB 16
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN5_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_OFFSET 0x00000074
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_LSB 24
#define PHYRX_RSSI_LEGACY_29_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN5_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_OFFSET 0x00000078
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_LSB 0
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN5_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_OFFSET 0x00000078
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_LSB 8
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN5_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_OFFSET 0x00000078
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_LSB 16
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN5_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_OFFSET 0x00000078
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_LSB 24
#define PHYRX_RSSI_LEGACY_30_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN5_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_OFFSET 0x0000007c
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_LSB 0
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN6_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_OFFSET 0x0000007c
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_LSB 8
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN6_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_OFFSET 0x0000007c
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_LSB 16
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN6_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_OFFSET 0x0000007c
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_LSB 24
#define PHYRX_RSSI_LEGACY_31_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN6_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_OFFSET 0x00000080
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_LSB 0
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN6_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_OFFSET 0x00000080
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_LSB 8
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN6_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_OFFSET 0x00000080
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_LSB 16
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN6_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_OFFSET 0x00000080
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_LSB 24
#define PHYRX_RSSI_LEGACY_32_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN6_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_OFFSET 0x00000084
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_LSB 0
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN7_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_OFFSET 0x00000084
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_LSB 8
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT20_CHAIN7_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_OFFSET 0x00000084
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_LSB 16
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_LOW20_CHAIN7_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_OFFSET 0x00000084
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_LSB 24
#define PHYRX_RSSI_LEGACY_33_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT40_HIGH20_CHAIN7_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_OFFSET 0x00000088
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_LSB 0
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW20_CHAIN7_MASK 0x000000ff

/* Description		PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_OFFSET 0x00000088
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_LSB 8
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_LOW_HIGH20_CHAIN7_MASK 0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_OFFSET 0x00000088
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_LSB 16
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH_LOW20_CHAIN7_MASK 0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_OFFSET 0x00000088
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_LSB 24
#define PHYRX_RSSI_LEGACY_34_PREAMBLE_RSSI_INFO_DETAILS_RSSI_EXT80_HIGH20_CHAIN7_MASK 0xff000000

/* Description		PHYRX_RSSI_LEGACY_35_PRE_RSSI_COMB
			
			Combined pre_rssi of all chains. Based on primary
			channel RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_35_PRE_RSSI_COMB_OFFSET                    0x0000008c
#define PHYRX_RSSI_LEGACY_35_PRE_RSSI_COMB_LSB                       0
#define PHYRX_RSSI_LEGACY_35_PRE_RSSI_COMB_MASK                      0x000000ff

/* Description		PHYRX_RSSI_LEGACY_35_RSSI_COMB
			
			Combined rssi of all chains. Based on primary channel
			RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_35_RSSI_COMB_OFFSET                        0x0000008c
#define PHYRX_RSSI_LEGACY_35_RSSI_COMB_LSB                           8
#define PHYRX_RSSI_LEGACY_35_RSSI_COMB_MASK                          0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_35_NORMALIZED_PRE_RSSI_COMB
			
			Combined pre_rssi of all chains, but normalized back to
			a single chain. This avoids PDG from having to evaluate this
			in combination with receive chain mask and perform all kinds
			of pre-processing algorithms.
			
			
			
			Based on primary channel RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_35_NORMALIZED_PRE_RSSI_COMB_OFFSET         0x0000008c
#define PHYRX_RSSI_LEGACY_35_NORMALIZED_PRE_RSSI_COMB_LSB            16
#define PHYRX_RSSI_LEGACY_35_NORMALIZED_PRE_RSSI_COMB_MASK           0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_35_NORMALIZED_RSSI_COMB
			
			Combined rssi of all chains, but normalized back to a
			single chain. This avoids PDG from having to evaluate this
			in combination with receive chain mask and perform all kinds
			of pre-processing algorithms.
			
			
			
			Based on primary channel RSSI.
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_35_NORMALIZED_RSSI_COMB_OFFSET             0x0000008c
#define PHYRX_RSSI_LEGACY_35_NORMALIZED_RSSI_COMB_LSB                24
#define PHYRX_RSSI_LEGACY_35_NORMALIZED_RSSI_COMB_MASK               0xff000000

/* Description		PHYRX_RSSI_LEGACY_36_RSSI_COMB_PPDU
			
			Combined rssi of all chains, based on active
			RUs/subchannels, a.k.a. rssi_pkt_bw_mac
			
			
			
			RSSI is reported as 8b signed values. Nominally value is
			in dB units above or below the noisefloor(minCCApwr). 
			
			
			
			The resolution can be: 
			
			1dB or 0.5dB. This is statically configured within the
			PHY and MAC
			
			
			
			In case of 1dB, the Range is:
			
			 -128dB to 127dB
			
			
			
			In case of 0.5dB, the Range is:
			
			 -64dB to 63.5dB
			
			
			
			When packet BW is 20 MHz,
			
			rssi_comb_ppdu = rssi_comb.
			
			
			
			When packet BW > 20 MHz,
			
			rssi_comb < rssi_comb_ppdu because rssi_comb only
			includes power of primary 20 MHz while rssi_comb_ppdu
			includes power of active RUs/subchannels.
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_36_RSSI_COMB_PPDU_OFFSET                   0x00000090
#define PHYRX_RSSI_LEGACY_36_RSSI_COMB_PPDU_LSB                      0
#define PHYRX_RSSI_LEGACY_36_RSSI_COMB_PPDU_MASK                     0x000000ff

/* Description		PHYRX_RSSI_LEGACY_36_RSSI_DB_TO_DBM_OFFSET
			
			Offset between 'dB' and 'dBm' values. SW can use this
			value to convert RSSI 'dBm' values back to 'dB,' and report
			both the values.
			
			
			
			When rssi_db_to_dbm_offset = 0,
			
			all rssi_xxx fields are defined in dB.
			
			
			
			When rssi_db_to_dbm_offset is a large negative value,
			all rssi_xxx fields are defined in dBm.
			
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_36_RSSI_DB_TO_DBM_OFFSET_OFFSET            0x00000090
#define PHYRX_RSSI_LEGACY_36_RSSI_DB_TO_DBM_OFFSET_LSB               8
#define PHYRX_RSSI_LEGACY_36_RSSI_DB_TO_DBM_OFFSET_MASK              0x0000ff00

/* Description		PHYRX_RSSI_LEGACY_36_RSSI_FOR_SPATIAL_REUSE
			
			RSSI to be used by HWSCH for transmit (power) selection
			during an SR opportunity, reported as an 8-bit signed value
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_36_RSSI_FOR_SPATIAL_REUSE_OFFSET           0x00000090
#define PHYRX_RSSI_LEGACY_36_RSSI_FOR_SPATIAL_REUSE_LSB              16
#define PHYRX_RSSI_LEGACY_36_RSSI_FOR_SPATIAL_REUSE_MASK             0x00ff0000

/* Description		PHYRX_RSSI_LEGACY_36_RSSI_FOR_TRIGGER_RESP
			
			RSSI to be used by PDG for transmit (power) selection
			during trigger response, reported as an 8-bit signed value
			
			
			<legal all>
*/
#define PHYRX_RSSI_LEGACY_36_RSSI_FOR_TRIGGER_RESP_OFFSET            0x00000090
#define PHYRX_RSSI_LEGACY_36_RSSI_FOR_TRIGGER_RESP_LSB               24
#define PHYRX_RSSI_LEGACY_36_RSSI_FOR_TRIGGER_RESP_MASK              0xff000000


#endif // _PHYRX_RSSI_LEGACY_H_

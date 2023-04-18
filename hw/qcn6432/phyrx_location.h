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

#ifndef _PHYRX_LOCATION_H_
#define _PHYRX_LOCATION_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_location_info.h"
#define NUM_OF_DWORDS_PHYRX_LOCATION 28

#define NUM_OF_QWORDS_PHYRX_LOCATION 14


struct phyrx_location {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   rx_location_info                                          rx_location_info_details;
#else
             struct   rx_location_info                                          rx_location_info_details;
#endif
};


/* Description		RX_LOCATION_INFO_DETAILS

			Overview of location related info 
*/


/* Description		RX_LOCATION_INFO_VALID

			<enum 0 rx_location_info_is_not_valid>
			<enum 1 rx_location_info_is_valid>
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_OFFSET       0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_LSB          0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_MSB          0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_LOCATION_INFO_VALID_MASK         0x0000000000000001


/* Description		RTT_HW_IFFT_MODE

			Indicator showing if HW IFFT mode or SW IFFT mode
			
			<enum 0 location_sw_ifft_mode>
			<enum 1 location_hw_ifft_mode>
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_OFFSET             0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_LSB                1
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_MSB                1
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_HW_IFFT_MODE_MASK               0x0000000000000002


/* Description		RTT_11AZ_MODE

			Indicator showing RTT5/.11mc or .11az mode for debug
			
			<enum 0 location_rtt5_mode> legacy RTT5/.11mc mode
			<enum 1 location_11az_ISTA> .11az ISTA location info. sent
			 on Rx path after receiving R2I LMR
			<enum 2 location_RSVD>
			<enum 3 location_11az_RSTA> .11az RSTA location info. sent
			 on Tx path after transmitting R2I LMR
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_11AZ_MODE_OFFSET                0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_11AZ_MODE_LSB                   2
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_11AZ_MODE_MSB                   3
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_11AZ_MODE_MASK                  0x000000000000000c


/* Description		RESERVED_0

			<legal 0>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_0_OFFSET                   0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_0_LSB                      4
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_0_MSB                      7
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_0_MASK                     0x00000000000000f0


/* Description		RTT_NUM_FAC

			Number of valid first arrival correction (FAC) values (in
			 fields rtt_fac_0 - rtt_fac_31)
			<legal 0-32>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_FAC_OFFSET                  0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_FAC_LSB                     8
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_FAC_MSB                     15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_FAC_MASK                    0x000000000000ff00


/* Description		RTT_RX_CHAIN_MASK

			Rx chain mask, each bit is a Rx chain
			0: the Rx chain is not used
			1: the Rx chain is used
			
			Up to 4 Rx chains are supported.
			
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_OFFSET            0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_LSB               16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_MSB               23
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_RX_CHAIN_MASK_MASK              0x0000000000ff0000


/* Description		RTT_NUM_STREAMS

			Number of streams used
			
			Up to 8 streams are supported.
			
			<legal 0-8>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_STREAMS_OFFSET              0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_STREAMS_LSB                 24
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_STREAMS_MSB                 31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_NUM_STREAMS_MASK                0x00000000ff000000


/* Description		RTT_FIRST_SELECTED_CHAIN

			For legacy RTT5/.11mc mode, this field shows the first selected
			 Rx chain that is used for FAC calculations, when forced
			 by a virtual register.
			
			<enum 0 location_selected_chain_is_0>
			<enum 1 location_selected_chain_is_1>
			<enum 2 location_selected_chain_is_2>
			<enum 3 location_selected_chain_is_3>
			<legal 0-3>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FIRST_SELECTED_CHAIN_OFFSET     0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FIRST_SELECTED_CHAIN_LSB        32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FIRST_SELECTED_CHAIN_MSB        39
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FIRST_SELECTED_CHAIN_MASK       0x000000ff00000000


/* Description		RTT_SECOND_SELECTED_CHAIN

			For legacy RTT5/.11mc mode, this field shows the second 
			selected Rx chain that is used for FAC calculations, when
			 forced by a virtual register.
			
			<enum 0 location_selected_chain_is_0>
			<enum 1 location_selected_chain_is_1>
			<enum 2 location_selected_chain_is_2>
			<enum 3 location_selected_chain_is_3>
			<legal 0-3>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_SECOND_SELECTED_CHAIN_OFFSET    0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_SECOND_SELECTED_CHAIN_LSB       40
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_SECOND_SELECTED_CHAIN_MSB       47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_SECOND_SELECTED_CHAIN_MASK      0x0000ff0000000000


/* Description		RTT_CFR_STATUS

			Status of channel frequency response dump
			
			<enum 0 location_CFR_dump_not_valid>
			<enum 1 location_CFR_dump_valid>
			<legal 0-1>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_OFFSET               0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_LSB                  48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_MSB                  55
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFR_STATUS_MASK                 0x00ff000000000000


/* Description		RTT_CIR_STATUS

			Status of channel impulse response dump
			
			<enum 0 location_CIR_dump_not_valid>
			<enum 1 location_CIR_dump_valid>
			<legal 0-1>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_OFFSET               0x0000000000000000
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_LSB                  56
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_MSB                  63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CIR_STATUS_MASK                 0xff00000000000000


/* Description		RTT_CHE_BUFFER_POINTER_LOW32

			The low 32 bits of the 40 bits pointer pointed to the external
			 RTT channel information buffer
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_OFFSET 0x0000000000000008
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_LSB    0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_MSB    31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_LOW32_MASK   0x00000000ffffffff


/* Description		RTT_CHE_BUFFER_POINTER_HIGH8

			The high 8 bits of the 40 bits pointer pointed to the external
			 RTT channel information buffer
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_OFFSET 0x0000000000000008
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_LSB    32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_MSB    39
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CHE_BUFFER_POINTER_HIGH8_MASK   0x000000ff00000000


/* Description		RESERVED_3

			<legal 0>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_3_OFFSET                   0x0000000000000008
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_3_LSB                      40
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_3_MSB                      47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_3_MASK                     0x0000ff0000000000


/* Description		RTT_PKT_BW_VHT

			Indicate the bandwidth of (V)HT/HE-LTF
			
			<enum 0 location_pkt_bw_20MHz>
			<enum 1 location_pkt_bw_40MHz>
			<enum 2 location_pkt_bw_80MHz>
			<enum 3 location_pkt_bw_160MHz>
			<enum 4 location_pkt_bw_240MHz> Only valid for CFR, FAC 
			calculations are not PoR for 240 MHz.
			<enum 5 location_pkt_bw_320MHz> Only valid for CFR, FAC 
			calculations are not PoR for 320 MHz.
			<legal 0-5>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_OFFSET               0x0000000000000008
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_LSB                  48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_MSB                  51
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_VHT_MASK                 0x000f000000000000


/* Description		RTT_PKT_BW_LEG

			Indicate the bandwidth of L-LTF
			
			<enum 0 location_pkt_bw_20MHz>
			<enum 1 location_pkt_bw_40MHz>
			<enum 2 location_pkt_bw_80MHz>
			<enum 3 location_pkt_bw_160MHz>
			<enum 4 location_pkt_bw_240MHz> Only valid for CFR, FAC 
			calculations are not PoR for 240 MHz.
			<enum 5 location_pkt_bw_320MHz> Only valid for CFR, FAC 
			calculations are not PoR for 320 MHz.
			<legal 0-5>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_OFFSET               0x0000000000000008
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_LSB                  52
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_MSB                  55
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PKT_BW_LEG_MASK                 0x00f0000000000000


/* Description		RTT_MCS_RATE

			Bits 0~4 indicate MCS rate, if Legacy, 
			0: 48 Mbps,
			1: 24 Mbps,
			2: 12 Mbps,
			3: 6 Mbps,
			4: 54 Mbps,
			5: 36 Mbps,
			6: 18 Mbps,
			7: 9 Mbps,
			8-15: reserved
			
			if HT, 0-7: MCS0-MCS7, 8-15: reserved,
			if VHT, 0-9: MCS0-MCS9, 10-15: reserved,
			if HE or EHT, 0-11: MCS0-MCS11, 12-13: 4096QAM, 14-15: reserved
			
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_OFFSET                 0x0000000000000008
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_LSB                    56
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_MSB                    63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_MCS_RATE_MASK                   0xff00000000000000


/* Description		RTT_CFO_MEASUREMENT

			CFO measurement. Needed for passive locationing
			
			14 bits, signed 1.13. 13 bits fraction to provide a resolution
			 of 153 Hz
			
			In units of cycles/800 ns
			<legal 0-16383>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_OFFSET          0x0000000000000010
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_LSB             0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_MSB             15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_CFO_MEASUREMENT_MASK            0x000000000000ffff


/* Description		RTT_PREAMBLE_TYPE

			Indicate preamble type
			
			<enum 0 location_preamble_type_legacy>
			<enum 1 location_preamble_type_ht>
			<enum 2 location_preamble_type_vht>
			<enum 3 location_preamble_type_he_su_4xltf>
			<enum 4 location_preamble_type_he_su_2xltf>
			<enum 5 location_preamble_type_he_su_1xltf>
			<enum 6 location_preamble_type_he_trigger_based_ul_4xltf>
			
			<enum 7 location_preamble_type_he_trigger_based_ul_2xltf>
			
			<enum 8 location_preamble_type_he_trigger_based_ul_1xltf>
			
			<enum 9 location_preamble_type_he_mu_4xltf>
			<enum 10 location_preamble_type_he_mu_2xltf>
			<enum 11 location_preamble_type_he_mu_1xltf>
			<enum 12 location_preamble_type_he_extended_range_su_4xltf>
			
			<enum 13 location_preamble_type_he_extended_range_su_2xltf>
			
			<enum 14 location_preamble_type_he_extended_range_su_1xltf>
			
			<legal 0-14>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_OFFSET            0x0000000000000010
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_LSB               16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_MSB               23
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_PREAMBLE_TYPE_MASK              0x0000000000ff0000


/* Description		RTT_GI_TYPE

			Indicate GI (guard interval) type
			
			<enum 0 location_gi_0_8_us > HE related GI. Can also be 
			used for HE
			<enum 1 location_gi_0_4_us > HE related GI. Can also be 
			used for HE
			<enum 2 location_gi_1_6_us > HE related GI
			<enum 3 location_gi_3_2_us > HE related GI
			<legal 0 - 3>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_OFFSET                  0x0000000000000010
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_LSB                     24
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_MSB                     31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_GI_TYPE_MASK                    0x00000000ff000000


/* Description		RX_START_TS

			RX packet start timestamp lower 32 bits
			
			It reports the time the first L-STF ADC sample arrived at
			 RX antenna.
			
			The clock unit is 960MHz.
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_OFFSET                  0x0000000000000010
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_LSB                     32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_MSB                     63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_MASK                    0xffffffff00000000


/* Description		RX_START_TS_UPPER

			RX packet start timestamp upper 32 bits
			
			It reports the time the first L-STF ADC sample arrived at
			 RX antenna.
			
			The clock unit is 960MHz.
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_UPPER_OFFSET            0x0000000000000018
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_UPPER_LSB               0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_UPPER_MSB               31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_START_TS_UPPER_MASK              0x00000000ffffffff


/* Description		RX_END_TS

			RX packet end timestamp lower 32 bits
			
			It reports the time the last symbol's last ADC sample arrived
			 at RX antenna.
			
			The clock unit is 960MHz. Only 32 bits are reported.
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_END_TS_OFFSET                    0x0000000000000018
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_END_TS_LSB                       32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_END_TS_MSB                       63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RX_END_TS_MASK                      0xffffffff00000000


/* Description		GAIN_CHAIN0

			Reports the total gain in dB and the gain table index to
			 support angle of arrival for chain0
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN0_OFFSET                  0x0000000000000020
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN0_LSB                     0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN0_MSB                     15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN0_MASK                    0x000000000000ffff


/* Description		GAIN_CHAIN1

			Reports the total gain in dB and the gain table index to
			 support angle of arrival for chain1
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN1_OFFSET                  0x0000000000000020
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN1_LSB                     16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN1_MSB                     31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN1_MASK                    0x00000000ffff0000


/* Description		GAIN_CHAIN2

			Reports the total gain in dB and the gain table index to
			 support angle of arrival for chain2
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN2_OFFSET                  0x0000000000000020
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN2_LSB                     32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN2_MSB                     47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN2_MASK                    0x0000ffff00000000


/* Description		GAIN_CHAIN3

			Reports the total gain in dB and the gain table index to
			 support angle of arrival for chain3
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN3_OFFSET                  0x0000000000000020
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN3_LSB                     48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN3_MSB                     63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_CHAIN3_MASK                    0xffff000000000000


/* Description		GAIN_REPORT_STATUS

			Number of valid gain reports (in fields gain_chain0 - gain_chain_3)
			
			<legal 0-4>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_REPORT_STATUS_OFFSET           0x0000000000000028
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_REPORT_STATUS_LSB              0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_REPORT_STATUS_MSB              7
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_GAIN_REPORT_STATUS_MASK             0x00000000000000ff


/* Description		RTT_TIMING_BACKOFF_SEL

			Indicate which timing backoff value is used
			
			<enum 0 timing_backoff_low_rssi>
			<enum 1 timing_backoff_mid_rssi>
			<enum 2 timing_backoff_high_rssi>
			<enum 3 reserved>
			<legal 0-3>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_OFFSET       0x0000000000000028
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_LSB          8
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_MSB          15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_TIMING_BACKOFF_SEL_MASK         0x000000000000ff00


/* Description		RTT_FAC_COMBINED

			Final adjusted and combined first arrival correction value
			
			<legal all>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_COMBINED_OFFSET             0x0000000000000028
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_COMBINED_LSB                16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_COMBINED_MSB                31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_COMBINED_MASK               0x00000000ffff0000


/* Description		RTT_FAC_0

			The fields 'rtt_fac_0' - 'rtt_fac_31' show the RTT first
			 arrival correction (FAC) value computed from the LTFs on
			 the selected Rx chains.
			
			16 bits, signed 11.5. 11 integer bits to cover -3.2us to
			 3.2us, and 5 fraction bits to cover 160 MHz with 32x FAC
			 interpolation.
			
			The clock unit is 320MHz.
			
			For .11az/MIMO, the FACs will be stored in spatial stream
			 order with multiple chains reported together for each stream. [ss0-ch0, 
			ss0-ch1, ..., ss1-ch0, ss1-ch1, ...]
			
			For legacy RTT5/.11mc, the FACs will be stored in preamble
			 order with multiple chains reported together for each LTF. [legacy-ch0, 
			legacy-ch1, ..., (v)ht/he-ch0, (v)ht/he-ch1, ...]
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_0_OFFSET                    0x0000000000000028
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_0_LSB                       32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_0_MSB                       47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_0_MASK                      0x0000ffff00000000


/* Description		RTT_FAC_1

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_1_OFFSET                    0x0000000000000028
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_1_LSB                       48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_1_MSB                       63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_1_MASK                      0xffff000000000000


/* Description		RTT_FAC_2

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_2_OFFSET                    0x0000000000000030
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_2_LSB                       0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_2_MSB                       15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_2_MASK                      0x000000000000ffff


/* Description		RTT_FAC_3

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_3_OFFSET                    0x0000000000000030
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_3_LSB                       16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_3_MSB                       31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_3_MASK                      0x00000000ffff0000


/* Description		RTT_FAC_4

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_4_OFFSET                    0x0000000000000030
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_4_LSB                       32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_4_MSB                       47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_4_MASK                      0x0000ffff00000000


/* Description		RTT_FAC_5

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_5_OFFSET                    0x0000000000000030
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_5_LSB                       48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_5_MSB                       63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_5_MASK                      0xffff000000000000


/* Description		RTT_FAC_6

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_6_OFFSET                    0x0000000000000038
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_6_LSB                       0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_6_MSB                       15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_6_MASK                      0x000000000000ffff


/* Description		RTT_FAC_7

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_7_OFFSET                    0x0000000000000038
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_7_LSB                       16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_7_MSB                       31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_7_MASK                      0x00000000ffff0000


/* Description		RTT_FAC_8

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_8_OFFSET                    0x0000000000000038
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_8_LSB                       32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_8_MSB                       47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_8_MASK                      0x0000ffff00000000


/* Description		RTT_FAC_9

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_9_OFFSET                    0x0000000000000038
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_9_LSB                       48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_9_MSB                       63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_9_MASK                      0xffff000000000000


/* Description		RTT_FAC_10

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_10_OFFSET                   0x0000000000000040
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_10_LSB                      0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_10_MSB                      15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_10_MASK                     0x000000000000ffff


/* Description		RTT_FAC_11

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_11_OFFSET                   0x0000000000000040
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_11_LSB                      16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_11_MSB                      31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_11_MASK                     0x00000000ffff0000


/* Description		RTT_FAC_12

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_12_OFFSET                   0x0000000000000040
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_12_LSB                      32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_12_MSB                      47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_12_MASK                     0x0000ffff00000000


/* Description		RTT_FAC_13

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_13_OFFSET                   0x0000000000000040
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_13_LSB                      48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_13_MSB                      63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_13_MASK                     0xffff000000000000


/* Description		RTT_FAC_14

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_14_OFFSET                   0x0000000000000048
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_14_LSB                      0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_14_MSB                      15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_14_MASK                     0x000000000000ffff


/* Description		RTT_FAC_15

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_15_OFFSET                   0x0000000000000048
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_15_LSB                      16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_15_MSB                      31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_15_MASK                     0x00000000ffff0000


/* Description		RTT_FAC_16

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_16_OFFSET                   0x0000000000000048
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_16_LSB                      32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_16_MSB                      47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_16_MASK                     0x0000ffff00000000


/* Description		RTT_FAC_17

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_17_OFFSET                   0x0000000000000048
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_17_LSB                      48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_17_MSB                      63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_17_MASK                     0xffff000000000000


/* Description		RTT_FAC_18

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_18_OFFSET                   0x0000000000000050
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_18_LSB                      0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_18_MSB                      15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_18_MASK                     0x000000000000ffff


/* Description		RTT_FAC_19

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_19_OFFSET                   0x0000000000000050
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_19_LSB                      16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_19_MSB                      31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_19_MASK                     0x00000000ffff0000


/* Description		RTT_FAC_20

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_20_OFFSET                   0x0000000000000050
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_20_LSB                      32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_20_MSB                      47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_20_MASK                     0x0000ffff00000000


/* Description		RTT_FAC_21

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_21_OFFSET                   0x0000000000000050
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_21_LSB                      48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_21_MSB                      63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_21_MASK                     0xffff000000000000


/* Description		RTT_FAC_22

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_22_OFFSET                   0x0000000000000058
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_22_LSB                      0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_22_MSB                      15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_22_MASK                     0x000000000000ffff


/* Description		RTT_FAC_23

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_23_OFFSET                   0x0000000000000058
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_23_LSB                      16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_23_MSB                      31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_23_MASK                     0x00000000ffff0000


/* Description		RTT_FAC_24

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_24_OFFSET                   0x0000000000000058
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_24_LSB                      32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_24_MSB                      47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_24_MASK                     0x0000ffff00000000


/* Description		RTT_FAC_25

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_25_OFFSET                   0x0000000000000058
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_25_LSB                      48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_25_MSB                      63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_25_MASK                     0xffff000000000000


/* Description		RTT_FAC_26

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_26_OFFSET                   0x0000000000000060
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_26_LSB                      0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_26_MSB                      15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_26_MASK                     0x000000000000ffff


/* Description		RTT_FAC_27

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_27_OFFSET                   0x0000000000000060
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_27_LSB                      16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_27_MSB                      31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_27_MASK                     0x00000000ffff0000


/* Description		RTT_FAC_28

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_28_OFFSET                   0x0000000000000060
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_28_LSB                      32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_28_MSB                      47
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_28_MASK                     0x0000ffff00000000


/* Description		RTT_FAC_29

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_29_OFFSET                   0x0000000000000060
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_29_LSB                      48
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_29_MSB                      63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_29_MASK                     0xffff000000000000


/* Description		RTT_FAC_30

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_30_OFFSET                   0x0000000000000068
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_30_LSB                      0
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_30_MSB                      15
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_30_MASK                     0x000000000000ffff


/* Description		RTT_FAC_31

			See 'rtt_fac_0' description
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_31_OFFSET                   0x0000000000000068
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_31_LSB                      16
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_31_MSB                      31
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RTT_FAC_31_MASK                     0x00000000ffff0000


/* Description		RESERVED_27A

			<legal 0>
*/

#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_27A_OFFSET                 0x0000000000000068
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_27A_LSB                    32
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_27A_MSB                    63
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_RESERVED_27A_MASK                   0xffffffff00000000



#endif   // PHYRX_LOCATION

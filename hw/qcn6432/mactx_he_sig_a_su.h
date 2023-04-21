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

#ifndef _MACTX_HE_SIG_A_SU_H_
#define _MACTX_HE_SIG_A_SU_H_
#if !defined(__ASSEMBLER__)
#endif

#include "he_sig_a_su_info.h"
#define NUM_OF_DWORDS_MACTX_HE_SIG_A_SU 2

#define NUM_OF_QWORDS_MACTX_HE_SIG_A_SU 1


struct mactx_he_sig_a_su {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   he_sig_a_su_info                                          mactx_he_sig_a_su_info_details;
#else
             struct   he_sig_a_su_info                                          mactx_he_sig_a_su_info_details;
#endif
};


/* Description		MACTX_HE_SIG_A_SU_INFO_DETAILS

			See detailed description of the STRUCT
*/


/* Description		FORMAT_INDICATION

			<enum 0 HE_SIGA_FORMAT_HE_TRIG>
			<enum 1 HE_SIGA_FORMAT_SU_OR_EXT_SU>
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_OFFSET   0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_LSB      0
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_MSB      0
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_MASK     0x0000000000000001


/* Description		BEAM_CHANGE

			Indicates whether spatial mapping is changed between legacy
			 and HE portion of preamble. If not, channel estimation 
			can include legacy preamble to improve accuracy
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BEAM_CHANGE_OFFSET         0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BEAM_CHANGE_LSB            1
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BEAM_CHANGE_MSB            1
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BEAM_CHANGE_MASK           0x0000000000000002


/* Description		DL_UL_FLAG

			Differentiates between DL and UL transmission 
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DL_UL_FLAG_OFFSET          0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DL_UL_FLAG_LSB             2
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DL_UL_FLAG_MSB             2
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DL_UL_FLAG_MASK            0x0000000000000004


/* Description		TRANSMIT_MCS

			Indicates the data MCS
			
			Field Used by MAC HW
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_MCS_OFFSET        0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_MCS_LSB           3
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_MCS_MSB           6
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_MCS_MASK          0x0000000000000078


/* Description		DCM

			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DCM_OFFSET                 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DCM_LSB                    7
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DCM_MSB                    7
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DCM_MASK                   0x0000000000000080


/* Description		BSS_COLOR_ID

			BSS color ID 
			
			Field Used by MAC HW
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BSS_COLOR_ID_OFFSET        0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BSS_COLOR_ID_LSB           8
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BSS_COLOR_ID_MSB           13
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_BSS_COLOR_ID_MASK          0x0000000000003f00


/* Description		RESERVED_0A

			Note: spec indicates this shall be set to 1
			<legal 1>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0A_OFFSET         0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0A_LSB            14
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0A_MSB            14
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0A_MASK           0x0000000000004000


/* Description		SPATIAL_REUSE

			Spatial reuse
			
			For 20MHz one SR field corresponding to entire 20MHz (other
			 3 fields indicate identical values)
			For 40MHz two SR fields for each 20MHz (other 2 fields indicate
			 identical values)
			For 80MHz four SR fields for each 20MHz
			For 160MHz four SR fields for each 40MHz
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_SPATIAL_REUSE_OFFSET       0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_SPATIAL_REUSE_LSB          15
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_SPATIAL_REUSE_MSB          18
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_SPATIAL_REUSE_MASK         0x0000000000078000


/* Description		TRANSMIT_BW

			Bandwidth of the PPDU.
			
			For HE SU PPDU                                          
			                                                        
			                                                        
			                                 
			<enum 0 HE_SIG_A_BW20> 20 Mhz 
			<enum 1 HE_SIG_A_BW40> 40 Mhz 
			<enum 2 HE_SIG_A_BW80> 80 Mhz 
			<enum 3 HE_SIG_A_BW160> 160 MHz or 80+80 MHz
			
			For HE Extended Range SU PPDU
			Set to 0 for 242-tone RU                                
			                                                        
			                  Set to 1 for right 106-tone RU within 
			the primary 20 MHz  
			
			On RX side, Field Used by MAC HW
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_BW_OFFSET         0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_BW_LSB            19
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_BW_MSB            20
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TRANSMIT_BW_MASK           0x0000000000180000


/* Description		CP_LTF_SIZE

			Indicates the CP and HE-LTF type 
			
			<enum 0 OneX_LTF_0_8CP> 1xLTF + 0.8 us CP
			<enum 1 TwoX_LTF_0_8CP> 2x LTF + 0.8 µs CP
			<enum 2 TwoX_LTF_1_6CP> 2x LTF + 1.6 µs CP
			
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note: 
			In this scenario, Neither DCM nor STBC is applied to HE 
			data field.
			
			NOTE:
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			0      = 1xLTF + 0.4 usec
			1      = 2xLTF + 0.4 usec
			2~3 = Reserved
			
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CP_LTF_SIZE_OFFSET         0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CP_LTF_SIZE_LSB            21
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CP_LTF_SIZE_MSB            22
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CP_LTF_SIZE_MASK           0x0000000000600000


/* Description		NSTS

			Indicates number of streams used for the SU transmission
			
			
			For HE SU PPDU                                          
			                                                        
			                                                        
			                      Set to n for n+1 space time stream, 
			where n = 0, 1, 2,.....,7.                              
			                                                        
			                                                        
			            
			
			For HE Extended Range PPDU                              
			                                                        
			                                                        
			                            Set to 0 for 1 space time stream. 
			 Value 1 is TBD                                         
			                                                        
			                                                        
			                                                        
			   Values 2 - 7 are reserved
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_NSTS_OFFSET                0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_NSTS_LSB                   23
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_NSTS_MSB                   25
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_NSTS_MASK                  0x0000000003800000


/* Description		RESERVED_0B

			<legal 0>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0B_OFFSET         0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0B_LSB            26
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0B_MSB            31
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_0B_MASK           0x00000000fc000000


/* Description		TXOP_DURATION

			Indicates the remaining time in the current TXOP
			
			Field Used by MAC HW
			 <legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXOP_DURATION_OFFSET       0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXOP_DURATION_LSB          32
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXOP_DURATION_MSB          38
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXOP_DURATION_MASK         0x0000007f00000000


/* Description		CODING

			Distinguishes between BCC and LDPC coding. 
			
			0: BCC
			1: LDPC
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CODING_OFFSET              0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CODING_LSB                 39
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CODING_MSB                 39
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CODING_MASK                0x0000008000000000


/* Description		LDPC_EXTRA_SYMBOL

			If LDPC, 
			  0: LDPC extra symbol not present
			  1: LDPC extra symbol present
			Else 
			  Set to 1
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_LDPC_EXTRA_SYMBOL_OFFSET   0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_LDPC_EXTRA_SYMBOL_LSB      40
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_LDPC_EXTRA_SYMBOL_MSB      40
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_LDPC_EXTRA_SYMBOL_MASK     0x0000010000000000


/* Description		STBC

			Indicates whether STBC is applied
			0: No STBC
			1: STBC
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_STBC_OFFSET                0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_STBC_LSB                   41
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_STBC_MSB                   41
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_STBC_MASK                  0x0000020000000000


/* Description		TXBF

			Indicates whether beamforming is applied
			0: No beamforming
			1: beamforming
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXBF_OFFSET                0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXBF_LSB                   42
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXBF_MSB                   42
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TXBF_MASK                  0x0000040000000000


/* Description		PACKET_EXTENSION_A_FACTOR

			Common trigger info
			
			the packet extension duration of the trigger-based PPDU 
			response with these two bits indicating the "a-factor" 
			
			<enum 0 a_factor_4>
			<enum 1 a_factor_1>
			<enum 2 a_factor_2>
			<enum 3 a_factor_3>
			
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_A_FACTOR_OFFSET 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_A_FACTOR_LSB 43
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_A_FACTOR_MSB 44
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_A_FACTOR_MASK 0x0000180000000000


/* Description		PACKET_EXTENSION_PE_DISAMBIGUITY

			Common trigger info
			
			the packet extension duration of the trigger-based PPDU 
			response with this bit indicating the PE-Disambiguity 
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB 45
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_MSB 45
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK 0x0000200000000000


/* Description		RESERVED_1A

			Note: per standard, set to 1
			<legal 1>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_1A_OFFSET         0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_1A_LSB            46
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_1A_MSB            46
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RESERVED_1A_MASK           0x0000400000000000


/* Description		DOPPLER_INDICATION

			0: No Doppler support
			1: Doppler support
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOPPLER_INDICATION_OFFSET  0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOPPLER_INDICATION_LSB     47
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOPPLER_INDICATION_MSB     47
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOPPLER_INDICATION_MASK    0x0000800000000000


/* Description		CRC

			CRC for HE-SIG-A contents.
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CRC_OFFSET                 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CRC_LSB                    48
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CRC_MSB                    51
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_CRC_MASK                   0x000f000000000000


/* Description		TAIL

			<legal 0>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TAIL_OFFSET                0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TAIL_LSB                   52
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TAIL_MSB                   57
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_TAIL_MASK                  0x03f0000000000000


/* Description		DOT11AX_SU_EXTENDED

			TX side:
			Set to 0
			
			RX side:
			On RX side, evaluated by MAC HW. This is the only way for
			 MAC RX to know that this was an HE_SIG_A_SU received in
			 'extended' format
			
			When set, the 11ax frame is of the extended range format
			
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_SU_EXTENDED_OFFSET 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_SU_EXTENDED_LSB    58
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_SU_EXTENDED_MSB    58
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_SU_EXTENDED_MASK   0x0400000000000000


/* Description		DOT11AX_EXT_RU_SIZE

			TX side:
			Set to 0
			
			RX side:
			Field only contains valid info when dot11ax_su_extended 
			is set.
			
			On RX side, evaluated by MAC HW. This is the only way for
			 MAC RX to know what the number of based RUs was in this
			 extended range reception. It is used by the MAC to determine
			 the RU size for the response...
			
			<enum 0 EXT_RU_26> 
			<enum 1 EXT_RU_52>
			<enum 2 EXT_RU_106>
			<enum 3 EXT_RU_242><legal 0-3>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_EXT_RU_SIZE_OFFSET 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_EXT_RU_SIZE_LSB    59
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_EXT_RU_SIZE_MSB    61
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_DOT11AX_EXT_RU_SIZE_MASK   0x3800000000000000


/* Description		RX_NDP

			TX side:
			Set to 0
			
			RX side:Valid on RX side only, and looked at by MAC HW
			
			When set, PHY has received (expected) NDP frame
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_NDP_OFFSET              0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_NDP_LSB                 62
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_NDP_MSB                 62
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_NDP_MASK                0x4000000000000000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the HE-SIG-A CRC check
			 has passed, else set to 0
			
			<legal all>
*/

#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_OFFSET 0x0000000000000000
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_LSB 63
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MSB 63
#define MACTX_HE_SIG_A_SU_MACTX_HE_SIG_A_SU_INFO_DETAILS_RX_INTEGRITY_CHECK_PASSED_MASK 0x8000000000000000



#endif   // MACTX_HE_SIG_A_SU

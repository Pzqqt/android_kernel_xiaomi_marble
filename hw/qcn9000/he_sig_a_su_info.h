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

#ifndef _HE_SIG_A_SU_INFO_H_
#define _HE_SIG_A_SU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	format_indication[0], beam_change[1], dl_ul_flag[2], transmit_mcs[6:3], dcm[7], bss_color_id[13:8], reserved_0a[14], spatial_reuse[18:15], transmit_bw[20:19], cp_ltf_size[22:21], nsts[25:23], reserved_0b[31:26]
//	1	txop_duration[6:0], coding[7], ldpc_extra_symbol[8], stbc[9], txbf[10], packet_extension_a_factor[12:11], packet_extension_pe_disambiguity[13], reserved_1a[14], doppler_indication[15], crc[19:16], tail[25:20], dot11ax_su_extended[26], dot11ax_ext_ru_size[30:27], rx_ndp[31]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_HE_SIG_A_SU_INFO 2

struct he_sig_a_su_info {
             uint32_t format_indication               :  1, //[0]
                      beam_change                     :  1, //[1]
                      dl_ul_flag                      :  1, //[2]
                      transmit_mcs                    :  4, //[6:3]
                      dcm                             :  1, //[7]
                      bss_color_id                    :  6, //[13:8]
                      reserved_0a                     :  1, //[14]
                      spatial_reuse                   :  4, //[18:15]
                      transmit_bw                     :  2, //[20:19]
                      cp_ltf_size                     :  2, //[22:21]
                      nsts                            :  3, //[25:23]
                      reserved_0b                     :  6; //[31:26]
             uint32_t txop_duration                   :  7, //[6:0]
                      coding                          :  1, //[7]
                      ldpc_extra_symbol               :  1, //[8]
                      stbc                            :  1, //[9]
                      txbf                            :  1, //[10]
                      packet_extension_a_factor       :  2, //[12:11]
                      packet_extension_pe_disambiguity:  1, //[13]
                      reserved_1a                     :  1, //[14]
                      doppler_indication              :  1, //[15]
                      crc                             :  4, //[19:16]
                      tail                            :  6, //[25:20]
                      dot11ax_su_extended             :  1, //[26]
                      dot11ax_ext_ru_size             :  4, //[30:27]
                      rx_ndp                          :  1; //[31]
};

/*

format_indication
			
			<enum 0 HE_SIGA_FORMAT_HE_TRIG>
			
			<enum 1 HE_SIGA_FORMAT_SU_OR_EXT_SU>
			
			<legal all>

beam_change
			
			Indicates whether spatial mapping is changed between
			legacy and HE portion of preamble. If not, channel
			estimation can include legacy preamble to improve accuracy
			
			<legal all>

dl_ul_flag
			
			Differentiates between DL and UL transmission 
			
			
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>

transmit_mcs
			
			Indicates the data MCS
			
			
			
			Field Used by MAC HW
			
			<legal all>

dcm
			
			
			0: No DCM
			
			1:DCM
			
			<legal all>

bss_color_id
			
			BSS color ID 
			
			
			
			Field Used by MAC HW
			
			<legal all>

reserved_0a
			
			Note: spec indicates this shall be set to 1
			
			<legal 1>

spatial_reuse
			
			Spatial reuse
			
			
			
			For 20MHz one SR field corresponding to entire 20MHz
			(other 3 fields indicate identical values)
			
			For 40MHz two SR fields for each 20MHz (other 2 fields
			indicate identical values)
			
			For 80MHz four SR fields for each 20MHz
			
			For 160MHz four SR fields for each 40MHz
			
			<legal all>

transmit_bw
			
			Bandwidth of the PPDU.
			
			
			
			For HE SU PPDU                                          
			                                                           
			
			<enum 0 HE_SIG_A_BW20> 20 Mhz 
			
			<enum 1 HE_SIG_A_BW40> 40 Mhz 
			
			<enum 2 HE_SIG_A_BW80> 80 Mhz 
			
			<enum 3 HE_SIG_A_BW160> 160 MHz or 80+80 MHz
			
			
			
			For HE Extended Range SU PPDU
			
			Set to 0 for 242-tone RU                                
			 Set to 1 for right 106-tone RU within the primary 20 MHz  
			
			
			
			On RX side, Field Used by MAC HW
			
			<legal all>

cp_ltf_size
			
			Indicates the CP and HE-LTF type 
			
			
			
			
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note:
			In this scenario, Neither DCM nor STBC is applied to HE data
			field.
			
			
			NOTE:
			
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			
			0      = 1xLTF + 0.4 usec
			
			1      = 2xLTF + 0.4 usec
			
			2~3 = Reserved
			
			
			
			<legal all>

nsts
			
			
			
			
			For HE SU PPDU                                          
			
			
			
			For HE Extended Range PPDU                              
			
			<legal all>

reserved_0b
			
			<legal 0>

txop_duration
			
			Indicates the remaining time in the current TXOP
			
			
			
			Field Used by MAC HW
			
			 <legal all>

coding
			
			Distinguishes between BCC and LDPC coding. 
			
			
			
			0: BCC
			
			1: LDPC
			
			<legal all>

ldpc_extra_symbol
			
			If LDPC, 
			
			  0: LDPC extra symbol not present
			
			  1: LDPC extra symbol present
			
			Else 
			
			  Set to 1
			
			<legal all>

stbc
			
			Indicates whether STBC is applied
			
			0: No STBC
			
			1: STBC
			
			<legal all>

txbf
			
			Indicates whether beamforming is applied
			
			0: No beamforming
			
			1: beamforming
			
			<legal all>

packet_extension_a_factor
			
			Common trigger info
			
			
			
			the packet extension duration of the trigger-based PPDU
			response with these two bits indicating the a-factor 
			
			
			
			<enum 0 a_factor_4>
			
			<enum 1 a_factor_1>
			
			<enum 2 a_factor_2>
			
			<enum 3 a_factor_3>
			
			
			
			<legal all>

packet_extension_pe_disambiguity
			
			Common trigger info
			
			
			
			the packet extension duration of the trigger-based PPDU
			response with this bit indicating the PE-Disambiguity 
			
			<legal all>

reserved_1a
			
			Note: per standard, set to 1
			
			<legal 1>

doppler_indication
			
			0: No Doppler support
			
			1: Doppler support
			
			<legal all>

crc
			
			CRC for HE-SIG-A contents.
			
			<legal all>

tail
			
			<legal 0>

dot11ax_su_extended
			
			TX side:
			
			Set to 0
			
			
			
			RX side:
			
			On RX side, evaluated by MAC HW. This is the only way
			for MAC RX to know that this was an HE_SIG_A_SU received in
			'extended' format
			
			
			
			
			<legal all>

dot11ax_ext_ru_size
			
			TX side:
			
			Set to 0
			
			
			
			RX side:
			
			Field only contains valid info when dot11ax_su_extended
			is set.
			
			
			
			On RX side, evaluated by MAC HW. This is the only way
			for MAC RX to know what the number of based RUs was in this
			extended range reception. It is used by the MAC to determine
			the RU size for the response...
			
			<legal all>

rx_ndp
			
			TX side:
			
			Set to 0
			
			
			
			RX side:Valid on RX side only, and looked at by MAC HW
			
			
			
			When set, PHY has received (expected) NDP frame
			
			<legal all>
*/


/* Description		HE_SIG_A_SU_INFO_0_FORMAT_INDICATION
			
			<enum 0 HE_SIGA_FORMAT_HE_TRIG>
			
			<enum 1 HE_SIGA_FORMAT_SU_OR_EXT_SU>
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_FORMAT_INDICATION_OFFSET                  0x00000000
#define HE_SIG_A_SU_INFO_0_FORMAT_INDICATION_LSB                     0
#define HE_SIG_A_SU_INFO_0_FORMAT_INDICATION_MASK                    0x00000001

/* Description		HE_SIG_A_SU_INFO_0_BEAM_CHANGE
			
			Indicates whether spatial mapping is changed between
			legacy and HE portion of preamble. If not, channel
			estimation can include legacy preamble to improve accuracy
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_BEAM_CHANGE_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_BEAM_CHANGE_LSB                           1
#define HE_SIG_A_SU_INFO_0_BEAM_CHANGE_MASK                          0x00000002

/* Description		HE_SIG_A_SU_INFO_0_DL_UL_FLAG
			
			Differentiates between DL and UL transmission 
			
			
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			
			<enum 1 DL_UL_FLAG_IS_UL>
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_DL_UL_FLAG_OFFSET                         0x00000000
#define HE_SIG_A_SU_INFO_0_DL_UL_FLAG_LSB                            2
#define HE_SIG_A_SU_INFO_0_DL_UL_FLAG_MASK                           0x00000004

/* Description		HE_SIG_A_SU_INFO_0_TRANSMIT_MCS
			
			Indicates the data MCS
			
			
			
			Field Used by MAC HW
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_TRANSMIT_MCS_OFFSET                       0x00000000
#define HE_SIG_A_SU_INFO_0_TRANSMIT_MCS_LSB                          3
#define HE_SIG_A_SU_INFO_0_TRANSMIT_MCS_MASK                         0x00000078

/* Description		HE_SIG_A_SU_INFO_0_DCM
			
			
			0: No DCM
			
			1:DCM
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_DCM_OFFSET                                0x00000000
#define HE_SIG_A_SU_INFO_0_DCM_LSB                                   7
#define HE_SIG_A_SU_INFO_0_DCM_MASK                                  0x00000080

/* Description		HE_SIG_A_SU_INFO_0_BSS_COLOR_ID
			
			BSS color ID 
			
			
			
			Field Used by MAC HW
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_BSS_COLOR_ID_OFFSET                       0x00000000
#define HE_SIG_A_SU_INFO_0_BSS_COLOR_ID_LSB                          8
#define HE_SIG_A_SU_INFO_0_BSS_COLOR_ID_MASK                         0x00003f00

/* Description		HE_SIG_A_SU_INFO_0_RESERVED_0A
			
			Note: spec indicates this shall be set to 1
			
			<legal 1>
*/
#define HE_SIG_A_SU_INFO_0_RESERVED_0A_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_RESERVED_0A_LSB                           14
#define HE_SIG_A_SU_INFO_0_RESERVED_0A_MASK                          0x00004000

/* Description		HE_SIG_A_SU_INFO_0_SPATIAL_REUSE
			
			Spatial reuse
			
			
			
			For 20MHz one SR field corresponding to entire 20MHz
			(other 3 fields indicate identical values)
			
			For 40MHz two SR fields for each 20MHz (other 2 fields
			indicate identical values)
			
			For 80MHz four SR fields for each 20MHz
			
			For 160MHz four SR fields for each 40MHz
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_SPATIAL_REUSE_OFFSET                      0x00000000
#define HE_SIG_A_SU_INFO_0_SPATIAL_REUSE_LSB                         15
#define HE_SIG_A_SU_INFO_0_SPATIAL_REUSE_MASK                        0x00078000

/* Description		HE_SIG_A_SU_INFO_0_TRANSMIT_BW
			
			Bandwidth of the PPDU.
			
			
			
			For HE SU PPDU                                          
			                                                           
			
			<enum 0 HE_SIG_A_BW20> 20 Mhz 
			
			<enum 1 HE_SIG_A_BW40> 40 Mhz 
			
			<enum 2 HE_SIG_A_BW80> 80 Mhz 
			
			<enum 3 HE_SIG_A_BW160> 160 MHz or 80+80 MHz
			
			
			
			For HE Extended Range SU PPDU
			
			Set to 0 for 242-tone RU                                
			 Set to 1 for right 106-tone RU within the primary 20 MHz  
			
			
			
			On RX side, Field Used by MAC HW
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_TRANSMIT_BW_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_TRANSMIT_BW_LSB                           19
#define HE_SIG_A_SU_INFO_0_TRANSMIT_BW_MASK                          0x00180000

/* Description		HE_SIG_A_SU_INFO_0_CP_LTF_SIZE
			
			Indicates the CP and HE-LTF type 
			
			<enum 3 FourX_LTF_0_8CP_3_2CP>
			
			When DCM == 0 OR STBC == 0: 4x LTF + 3.2 µs CP 
			
			When DCM == 1 AND STBC == 1: 4x LTF + 0.8 µs CP. Note:
			In this scenario, Neither DCM nor STBC is applied to HE data
			field.
			
			
			NOTE:
			
			If ( DCM == 1 ) and ( MCS > 0 ) and (STBC == 0) 
			
			0      = 1xLTF + 0.4 usec
			
			1      = 2xLTF + 0.4 usec
			
			2~3 = Reserved
			
			
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_CP_LTF_SIZE_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_CP_LTF_SIZE_LSB                           21
#define HE_SIG_A_SU_INFO_0_CP_LTF_SIZE_MASK                          0x00600000

/* Description		HE_SIG_A_SU_INFO_0_NSTS
			
			
			
			
			For HE SU PPDU                                          
			
			
			
			For HE Extended Range PPDU                              
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_0_NSTS_OFFSET                               0x00000000
#define HE_SIG_A_SU_INFO_0_NSTS_LSB                                  23
#define HE_SIG_A_SU_INFO_0_NSTS_MASK                                 0x03800000

/* Description		HE_SIG_A_SU_INFO_0_RESERVED_0B
			
			<legal 0>
*/
#define HE_SIG_A_SU_INFO_0_RESERVED_0B_OFFSET                        0x00000000
#define HE_SIG_A_SU_INFO_0_RESERVED_0B_LSB                           26
#define HE_SIG_A_SU_INFO_0_RESERVED_0B_MASK                          0xfc000000

/* Description		HE_SIG_A_SU_INFO_1_TXOP_DURATION
			
			Indicates the remaining time in the current TXOP
			
			
			
			Field Used by MAC HW
			
			 <legal all>
*/
#define HE_SIG_A_SU_INFO_1_TXOP_DURATION_OFFSET                      0x00000004
#define HE_SIG_A_SU_INFO_1_TXOP_DURATION_LSB                         0
#define HE_SIG_A_SU_INFO_1_TXOP_DURATION_MASK                        0x0000007f

/* Description		HE_SIG_A_SU_INFO_1_CODING
			
			Distinguishes between BCC and LDPC coding. 
			
			
			
			0: BCC
			
			1: LDPC
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_CODING_OFFSET                             0x00000004
#define HE_SIG_A_SU_INFO_1_CODING_LSB                                7
#define HE_SIG_A_SU_INFO_1_CODING_MASK                               0x00000080

/* Description		HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL
			
			If LDPC, 
			
			  0: LDPC extra symbol not present
			
			  1: LDPC extra symbol present
			
			Else 
			
			  Set to 1
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL_OFFSET                  0x00000004
#define HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL_LSB                     8
#define HE_SIG_A_SU_INFO_1_LDPC_EXTRA_SYMBOL_MASK                    0x00000100

/* Description		HE_SIG_A_SU_INFO_1_STBC
			
			Indicates whether STBC is applied
			
			0: No STBC
			
			1: STBC
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_STBC_OFFSET                               0x00000004
#define HE_SIG_A_SU_INFO_1_STBC_LSB                                  9
#define HE_SIG_A_SU_INFO_1_STBC_MASK                                 0x00000200

/* Description		HE_SIG_A_SU_INFO_1_TXBF
			
			Indicates whether beamforming is applied
			
			0: No beamforming
			
			1: beamforming
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_TXBF_OFFSET                               0x00000004
#define HE_SIG_A_SU_INFO_1_TXBF_LSB                                  10
#define HE_SIG_A_SU_INFO_1_TXBF_MASK                                 0x00000400

/* Description		HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR
			
			Common trigger info
			
			
			
			the packet extension duration of the trigger-based PPDU
			response with these two bits indicating the a-factor 
			
			
			
			<enum 0 a_factor_4>
			
			<enum 1 a_factor_1>
			
			<enum 2 a_factor_2>
			
			<enum 3 a_factor_3>
			
			
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR_OFFSET          0x00000004
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR_LSB             11
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_A_FACTOR_MASK            0x00001800

/* Description		HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY
			
			Common trigger info
			
			
			
			the packet extension duration of the trigger-based PPDU
			response with this bit indicating the PE-Disambiguity 
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY_OFFSET   0x00000004
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY_LSB      13
#define HE_SIG_A_SU_INFO_1_PACKET_EXTENSION_PE_DISAMBIGUITY_MASK     0x00002000

/* Description		HE_SIG_A_SU_INFO_1_RESERVED_1A
			
			Note: per standard, set to 1
			
			<legal 1>
*/
#define HE_SIG_A_SU_INFO_1_RESERVED_1A_OFFSET                        0x00000004
#define HE_SIG_A_SU_INFO_1_RESERVED_1A_LSB                           14
#define HE_SIG_A_SU_INFO_1_RESERVED_1A_MASK                          0x00004000

/* Description		HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION
			
			0: No Doppler support
			
			1: Doppler support
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION_OFFSET                 0x00000004
#define HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION_LSB                    15
#define HE_SIG_A_SU_INFO_1_DOPPLER_INDICATION_MASK                   0x00008000

/* Description		HE_SIG_A_SU_INFO_1_CRC
			
			CRC for HE-SIG-A contents.
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_CRC_OFFSET                                0x00000004
#define HE_SIG_A_SU_INFO_1_CRC_LSB                                   16
#define HE_SIG_A_SU_INFO_1_CRC_MASK                                  0x000f0000

/* Description		HE_SIG_A_SU_INFO_1_TAIL
			
			<legal 0>
*/
#define HE_SIG_A_SU_INFO_1_TAIL_OFFSET                               0x00000004
#define HE_SIG_A_SU_INFO_1_TAIL_LSB                                  20
#define HE_SIG_A_SU_INFO_1_TAIL_MASK                                 0x03f00000

/* Description		HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED
			
			TX side:
			
			Set to 0
			
			
			
			RX side:
			
			On RX side, evaluated by MAC HW. This is the only way
			for MAC RX to know that this was an HE_SIG_A_SU received in
			'extended' format
			
			
			
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED_OFFSET                0x00000004
#define HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED_LSB                   26
#define HE_SIG_A_SU_INFO_1_DOT11AX_SU_EXTENDED_MASK                  0x04000000

/* Description		HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE
			
			TX side:
			
			Set to 0
			
			
			
			RX side:
			
			Field only contains valid info when dot11ax_su_extended
			is set.
			
			
			
			On RX side, evaluated by MAC HW. This is the only way
			for MAC RX to know what the number of based RUs was in this
			extended range reception. It is used by the MAC to determine
			the RU size for the response...
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE_OFFSET                0x00000004
#define HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE_LSB                   27
#define HE_SIG_A_SU_INFO_1_DOT11AX_EXT_RU_SIZE_MASK                  0x78000000

/* Description		HE_SIG_A_SU_INFO_1_RX_NDP
			
			TX side:
			
			Set to 0
			
			
			
			RX side:Valid on RX side only, and looked at by MAC HW
			
			
			
			When set, PHY has received (expected) NDP frame
			
			<legal all>
*/
#define HE_SIG_A_SU_INFO_1_RX_NDP_OFFSET                             0x00000004
#define HE_SIG_A_SU_INFO_1_RX_NDP_LSB                                31
#define HE_SIG_A_SU_INFO_1_RX_NDP_MASK                               0x80000000


#endif // _HE_SIG_A_SU_INFO_H_


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

 
 
 
 
 
 
 


#ifndef _MACTX_USER_DESC_PER_USER_H_
#define _MACTX_USER_DESC_PER_USER_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MACTX_USER_DESC_PER_USER 4

#define NUM_OF_QWORDS_MACTX_USER_DESC_PER_USER 2


struct mactx_user_desc_per_user {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t psdu_length                                             : 24, // [23:0]
                      reserved_0a                                             :  8; // [31:24]
             uint32_t ru_start_index                                          :  8, // [7:0]
                      ru_size                                                 :  4, // [11:8]
                      reserved_1b                                             :  4, // [15:12]
                      ofdma_mu_mimo_enabled                                   :  1, // [16:16]
                      nss                                                     :  3, // [19:17]
                      stream_offset                                           :  3, // [22:20]
                      reserved_1c                                             :  1, // [23:23]
                      mcs                                                     :  4, // [27:24]
                      dcm                                                     :  1, // [28:28]
                      reserved_1d                                             :  3; // [31:29]
             uint32_t fec_type                                                :  1, // [0:0]
                      reserved_2a                                             :  7, // [7:1]
                      user_bf_type                                            :  2, // [9:8]
                      reserved_2b                                             :  6, // [15:10]
                      drop_user_cbf                                           :  1, // [16:16]
                      reserved_2c                                             :  7, // [23:17]
                      ldpc_extra_symbol                                       :  1, // [24:24]
                      force_extra_symbol                                      :  1, // [25:25]
                      reserved_2d                                             :  6; // [31:26]
             uint32_t sw_peer_id                                              : 16, // [15:0]
                      per_user_subband_mask                                   : 16; // [31:16]
#else
             uint32_t reserved_0a                                             :  8, // [31:24]
                      psdu_length                                             : 24; // [23:0]
             uint32_t reserved_1d                                             :  3, // [31:29]
                      dcm                                                     :  1, // [28:28]
                      mcs                                                     :  4, // [27:24]
                      reserved_1c                                             :  1, // [23:23]
                      stream_offset                                           :  3, // [22:20]
                      nss                                                     :  3, // [19:17]
                      ofdma_mu_mimo_enabled                                   :  1, // [16:16]
                      reserved_1b                                             :  4, // [15:12]
                      ru_size                                                 :  4, // [11:8]
                      ru_start_index                                          :  8; // [7:0]
             uint32_t reserved_2d                                             :  6, // [31:26]
                      force_extra_symbol                                      :  1, // [25:25]
                      ldpc_extra_symbol                                       :  1, // [24:24]
                      reserved_2c                                             :  7, // [23:17]
                      drop_user_cbf                                           :  1, // [16:16]
                      reserved_2b                                             :  6, // [15:10]
                      user_bf_type                                            :  2, // [9:8]
                      reserved_2a                                             :  7, // [7:1]
                      fec_type                                                :  1; // [0:0]
             uint32_t per_user_subband_mask                                   : 16, // [31:16]
                      sw_peer_id                                              : 16; // [15:0]
#endif
};


/* Description		PSDU_LENGTH

			PSDU Length for the User in octets
			NOTE: This also holds good for .11ba packets
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_OFFSET                                 0x0000000000000000
#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_LSB                                    0
#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_MSB                                    23
#define MACTX_USER_DESC_PER_USER_PSDU_LENGTH_MASK                                   0x0000000000ffffff


/* Description		RESERVED_0A

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_0A_OFFSET                                 0x0000000000000000
#define MACTX_USER_DESC_PER_USER_RESERVED_0A_LSB                                    24
#define MACTX_USER_DESC_PER_USER_RESERVED_0A_MSB                                    31
#define MACTX_USER_DESC_PER_USER_RESERVED_0A_MASK                                   0x00000000ff000000


/* Description		RU_START_INDEX

			Field only valid in case of .11ax or .11be OFDMA transmission
			  (=> from MACTX_PHY_DESC, field MU_type == OFDMA)
			OR 
			11ax SU "Narrow band" transmission.
			
			RU Number to which User is assigned
			RU numbering is over the entire BW, starting from 0 and 
			for the different users in increasing frequency order and
			 not primary-secondary order.
			
			For DL OFDMA transmissions, PDG shall fill this as instructed
			 by SW.
			
			For UL OFDMA transmissions, the RU number within 80 MHz 
			is available from the RU allocation information in the trigger. 
			For 160 MHz UL OFDMA transmissions, the trigger RU allocation
			 only mentions primary/secondary 80 MHz. PDG needs to convert
			 this to lower/higher 80 MHz.
			
			If in 'PCU_PPDU_SETUP_START'/'MACTX_PRE_PHY_DESC,' CCA_Subband_channel_bonding_mask
			 bit 0 is mapped to any of bits 4 - 7 of Freq_Subband_channel_bonding_mask, 
			then the primary 80 MHz is the higher 80 MHz and the secondary
			 80 MHz is the lower one.
			Otherwise (if CCA_Subband_channel_bonding_mask bit 0 is 
			mapped to any of bits 0 - 3 of Freq_Subband_channel_bonding_mask, 
			then the primary 80 MHz is the lower 80 MHz and the secondary
			 80 MHz is the higher one.
			
			<legal 0-147>
*/

#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_OFFSET                              0x0000000000000000
#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_LSB                                 32
#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_MSB                                 39
#define MACTX_USER_DESC_PER_USER_RU_START_INDEX_MASK                                0x000000ff00000000


/* Description		RU_SIZE

			The size of the RU for this user
			
			In case of HE extended range transmission, e-num 2 (10MHz) 
			or e-num 7 (20MHz) are used.
			
			In case of trig transmission or OFDMA single user or MU-MIMO
			 single user transmission, if the RU allocated to the user
			 is the full BW (with respect to AP_bw) then the e-num 7
			 is used.
			For all other cases, e-nums corresponding to the RU size
			 allocated to the user is used.
			
			In case of EHT duplicate transmissions, this field indicates
			 the width of the actual content before duplication, e.g. 
			a 40 MHz PPDU duplicated to 160 MHz will have the bandwidth
			 fields indicating 160 MHz and this field set to e-num 4
			 (RU_484).
			
			<enum 0 RU_26>
			<enum 1 RU_52>
			<enum 2 RU_106>
			<enum 3 RU_242>
			<enum 4 RU_484>
			<enum 5 RU_996>
			<enum 6 RU_1992>
			<enum 7 RU_FULLBW> Set when the RU occupies the full packet
			 bandwidth
			<enum 8 RU_FULLBW_240> Set when the RU occupies the full
			 packet bandwidth
			<enum 9 RU_FULLBW_320> Set when the RU occupies the full
			 packet bandwidth
			<enum 10 RU_MULTI_LARGE> HW will use per-user sub-band-mask
			 to infer the actual RU-size for Multi-large-RU/SU-Puncturing
			
			<enum 11 RU_78> multi small RU
			<enum 12 RU_132> multi small RU<legal 0-12>
*/

#define MACTX_USER_DESC_PER_USER_RU_SIZE_OFFSET                                     0x0000000000000000
#define MACTX_USER_DESC_PER_USER_RU_SIZE_LSB                                        40
#define MACTX_USER_DESC_PER_USER_RU_SIZE_MSB                                        43
#define MACTX_USER_DESC_PER_USER_RU_SIZE_MASK                                       0x00000f0000000000


/* Description		RESERVED_1B

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_1B_OFFSET                                 0x0000000000000000
#define MACTX_USER_DESC_PER_USER_RESERVED_1B_LSB                                    44
#define MACTX_USER_DESC_PER_USER_RESERVED_1B_MSB                                    47
#define MACTX_USER_DESC_PER_USER_RESERVED_1B_MASK                                   0x0000f00000000000


/* Description		OFDMA_MU_MIMO_ENABLED

			Field only valid in case of .11ax or .11be OFDMA transmission
			  (=> from MACTX_PHY_DESC, field MU_type == OFDMA)
			
			When set, for this user there is MIMO transmission within
			 the RU
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_OFFSET                       0x0000000000000000
#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_LSB                          48
#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_MSB                          48
#define MACTX_USER_DESC_PER_USER_OFDMA_MU_MIMO_ENABLED_MASK                         0x0001000000000000


/* Description		NSS

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

#define MACTX_USER_DESC_PER_USER_NSS_OFFSET                                         0x0000000000000000
#define MACTX_USER_DESC_PER_USER_NSS_LSB                                            49
#define MACTX_USER_DESC_PER_USER_NSS_MSB                                            51
#define MACTX_USER_DESC_PER_USER_NSS_MASK                                           0x000e000000000000


/* Description		STREAM_OFFSET

			Field only valid in case of MU-MIMO transmission  (=> from
			 MACTX_PHY_DESC, field MU_type == MU-MIMO) 
			OR
			when field Ofdma_mu_mimo_enabled is set
			
			Stream Offset from which the User occupies the Streams
			
			Note MAC:
			directly from pdg_fes_setup, based on BW
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_OFFSET                               0x0000000000000000
#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_LSB                                  52
#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_MSB                                  54
#define MACTX_USER_DESC_PER_USER_STREAM_OFFSET_MASK                                 0x0070000000000000


/* Description		RESERVED_1C

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_1C_OFFSET                                 0x0000000000000000
#define MACTX_USER_DESC_PER_USER_RESERVED_1C_LSB                                    55
#define MACTX_USER_DESC_PER_USER_RESERVED_1C_MSB                                    55
#define MACTX_USER_DESC_PER_USER_RESERVED_1C_MASK                                   0x0080000000000000


/* Description		MCS

			Modulation Coding Scheme for the User
			
			The MCS to be used for the upcoming transmission. It must
			 match the 4-bit MCS value that is sent in the appropriate
			 signal field for the given packet type, except that EHT
			 BPSK with DCM and/or duplicate is encoded as '0.'
			
			
			For details, refer to  the SIG field, related to this pkt_type.
			
			(Note that this is slightly different then what is on the
			 MAC side defined as "MCS_TYPE". For this reason, the 'legal
			 values' here are NOT defined as MCS_TYPE)
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_MCS_OFFSET                                         0x0000000000000000
#define MACTX_USER_DESC_PER_USER_MCS_LSB                                            56
#define MACTX_USER_DESC_PER_USER_MCS_MSB                                            59
#define MACTX_USER_DESC_PER_USER_MCS_MASK                                           0x0f00000000000000


/* Description		DCM

			Field only valid in case of 11ax transmission
			
			Indicates whether dual sub-carrier modulation is applied
			
			0: No DCM
			1:DCM
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_DCM_OFFSET                                         0x0000000000000000
#define MACTX_USER_DESC_PER_USER_DCM_LSB                                            60
#define MACTX_USER_DESC_PER_USER_DCM_MSB                                            60
#define MACTX_USER_DESC_PER_USER_DCM_MASK                                           0x1000000000000000


/* Description		RESERVED_1D

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_1D_OFFSET                                 0x0000000000000000
#define MACTX_USER_DESC_PER_USER_RESERVED_1D_LSB                                    61
#define MACTX_USER_DESC_PER_USER_RESERVED_1D_MSB                                    63
#define MACTX_USER_DESC_PER_USER_RESERVED_1D_MASK                                   0xe000000000000000


/* Description		FEC_TYPE

			0: BCC
			1: LDPC
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_FEC_TYPE_OFFSET                                    0x0000000000000008
#define MACTX_USER_DESC_PER_USER_FEC_TYPE_LSB                                       0
#define MACTX_USER_DESC_PER_USER_FEC_TYPE_MSB                                       0
#define MACTX_USER_DESC_PER_USER_FEC_TYPE_MASK                                      0x0000000000000001


/* Description		RESERVED_2A

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_2A_OFFSET                                 0x0000000000000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2A_LSB                                    1
#define MACTX_USER_DESC_PER_USER_RESERVED_2A_MSB                                    7
#define MACTX_USER_DESC_PER_USER_RESERVED_2A_MASK                                   0x00000000000000fe


/* Description		USER_BF_TYPE

			This field is valid for all packets using multiple antennas
			 because it defines whether the user's tones will be beamformed, 
			spatially spread, both or none of the above.
			
			<enum 0     USER_NO_BF> Direct mapping from Stream to Chain
			
			<enum 1     USER_WALSH_ONLY>  Enable Walsh mapping only
			<enum 2     USER_BF_ONLY> Enable Beamforming only
			<enum 3     USER_WALSH_AND_BF> Enable Walsh and Beamforming
			
			
			NOTE: USER_NO_BF and USER_BF_ONLY are not allowed if the
			 number of spatial streams (NSS) < the number of Tx chains
			 (NTx).
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_OFFSET                                0x0000000000000008
#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_LSB                                   8
#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_MSB                                   9
#define MACTX_USER_DESC_PER_USER_USER_BF_TYPE_MASK                                  0x0000000000000300


/* Description		RESERVED_2B

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_2B_OFFSET                                 0x0000000000000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2B_LSB                                    10
#define MACTX_USER_DESC_PER_USER_RESERVED_2B_MSB                                    15
#define MACTX_USER_DESC_PER_USER_RESERVED_2B_MASK                                   0x000000000000fc00


/* Description		DROP_USER_CBF

			This user shall be dropped because of CBF FCS failure or
			 no CBF reception.
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_OFFSET                               0x0000000000000008
#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_LSB                                  16
#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_MSB                                  16
#define MACTX_USER_DESC_PER_USER_DROP_USER_CBF_MASK                                 0x0000000000010000


/* Description		RESERVED_2C

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_2C_OFFSET                                 0x0000000000000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2C_LSB                                    17
#define MACTX_USER_DESC_PER_USER_RESERVED_2C_MSB                                    23
#define MACTX_USER_DESC_PER_USER_RESERVED_2C_MASK                                   0x0000000000fe0000


/* Description		LDPC_EXTRA_SYMBOL

			Set to 1 if the LDPC PPDU encoding process (if an SU PPDU), 
			or at least one LDPC user's PPDU encoding process (if an
			 MU PPDU), results in an extra OFDM symbol (or symbols) 
			as described in 22.3.10.5.4 (LDPC coding) and 22.3.10.5.5
			 (Encoding process for MU PPDUs). Set to 0 otherwise.
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_OFFSET                           0x0000000000000008
#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_LSB                              24
#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_MSB                              24
#define MACTX_USER_DESC_PER_USER_LDPC_EXTRA_SYMBOL_MASK                             0x0000000001000000


/* Description		FORCE_EXTRA_SYMBOL

			Set to 1 to force an extra OFDM symbol (or symbols) even
			 if none of the users' PPDU encoding process resuls in an
			 extra OFDM symbol (or symbols).
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_OFFSET                          0x0000000000000008
#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_LSB                             25
#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_MSB                             25
#define MACTX_USER_DESC_PER_USER_FORCE_EXTRA_SYMBOL_MASK                            0x0000000002000000


/* Description		RESERVED_2D

			<legal 0>
*/

#define MACTX_USER_DESC_PER_USER_RESERVED_2D_OFFSET                                 0x0000000000000008
#define MACTX_USER_DESC_PER_USER_RESERVED_2D_LSB                                    26
#define MACTX_USER_DESC_PER_USER_RESERVED_2D_MSB                                    31
#define MACTX_USER_DESC_PER_USER_RESERVED_2D_MASK                                   0x00000000fc000000


/* Description		SW_PEER_ID

			When set to 0, SW did not populate this field.
			
			The SW peer ID for this user
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_OFFSET                                  0x0000000000000008
#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_LSB                                     32
#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_MSB                                     47
#define MACTX_USER_DESC_PER_USER_SW_PEER_ID_MASK                                    0x0000ffff00000000


/* Description		PER_USER_SUBBAND_MASK

			This specifies a per-20 MHz subband mask per-user to be 
			used in case of either multi-large-RU or preamble puncturing.
			
			<legal all>
*/

#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_OFFSET                       0x0000000000000008
#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_LSB                          48
#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_MSB                          63
#define MACTX_USER_DESC_PER_USER_PER_USER_SUBBAND_MASK_MASK                         0xffff000000000000



#endif   // MACTX_USER_DESC_PER_USER

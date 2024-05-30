
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

 
 
 
 
 
 
 


#ifndef _VHT_SIG_A_INFO_H_
#define _VHT_SIG_A_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_VHT_SIG_A_INFO 2


struct vht_sig_a_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t bandwidth                                               :  2, // [1:0]
                      vhta_reserved_0                                         :  1, // [2:2]
                      stbc                                                    :  1, // [3:3]
                      group_id                                                :  6, // [9:4]
                      n_sts                                                   : 12, // [21:10]
                      txop_ps_not_allowed                                     :  1, // [22:22]
                      vhta_reserved_0b                                        :  1, // [23:23]
                      reserved_0                                              :  8; // [31:24]
             uint32_t gi_setting                                              :  2, // [1:0]
                      su_mu_coding                                            :  1, // [2:2]
                      ldpc_extra_symbol                                       :  1, // [3:3]
                      mcs                                                     :  4, // [7:4]
                      beamformed                                              :  1, // [8:8]
                      vhta_reserved_1                                         :  1, // [9:9]
                      crc                                                     :  8, // [17:10]
                      tail                                                    :  6, // [23:18]
                      reserved_1                                              :  7, // [30:24]
                      rx_integrity_check_passed                               :  1; // [31:31]
#else
             uint32_t reserved_0                                              :  8, // [31:24]
                      vhta_reserved_0b                                        :  1, // [23:23]
                      txop_ps_not_allowed                                     :  1, // [22:22]
                      n_sts                                                   : 12, // [21:10]
                      group_id                                                :  6, // [9:4]
                      stbc                                                    :  1, // [3:3]
                      vhta_reserved_0                                         :  1, // [2:2]
                      bandwidth                                               :  2; // [1:0]
             uint32_t rx_integrity_check_passed                               :  1, // [31:31]
                      reserved_1                                              :  7, // [30:24]
                      tail                                                    :  6, // [23:18]
                      crc                                                     :  8, // [17:10]
                      vhta_reserved_1                                         :  1, // [9:9]
                      beamformed                                              :  1, // [8:8]
                      mcs                                                     :  4, // [7:4]
                      ldpc_extra_symbol                                       :  1, // [3:3]
                      su_mu_coding                                            :  1, // [2:2]
                      gi_setting                                              :  2; // [1:0]
#endif
};


/* Description		BANDWIDTH

			Packet bandwidth
			
			<enum 0    20_MHZ_11AC>
			<enum 1    40_MHZ_11AC>
			<enum 2    80_MHZ_11AC>
			<enum 3    160_MHZ_11AC>
			
			<legal 0-3>
*/

#define VHT_SIG_A_INFO_BANDWIDTH_OFFSET                                             0x00000000
#define VHT_SIG_A_INFO_BANDWIDTH_LSB                                                0
#define VHT_SIG_A_INFO_BANDWIDTH_MSB                                                1
#define VHT_SIG_A_INFO_BANDWIDTH_MASK                                               0x00000003


/* Description		VHTA_RESERVED_0

			Reserved.  Set to 1 by MAC, PHY should ignore
			<legal 1>
*/

#define VHT_SIG_A_INFO_VHTA_RESERVED_0_OFFSET                                       0x00000000
#define VHT_SIG_A_INFO_VHTA_RESERVED_0_LSB                                          2
#define VHT_SIG_A_INFO_VHTA_RESERVED_0_MSB                                          2
#define VHT_SIG_A_INFO_VHTA_RESERVED_0_MASK                                         0x00000004


/* Description		STBC

			Space time block coding:
			<enum 0     stbc_disabled>  Indicates STBC is disabled
			<enum 1     stbc_enabled>  Indicates STBC is enabled on 
			all streams
			<legal 0-1>
*/

#define VHT_SIG_A_INFO_STBC_OFFSET                                                  0x00000000
#define VHT_SIG_A_INFO_STBC_LSB                                                     3
#define VHT_SIG_A_INFO_STBC_MSB                                                     3
#define VHT_SIG_A_INFO_STBC_MASK                                                    0x00000008


/* Description		GROUP_ID

			In a SU VHT PPDU, if the PPDU carries MPDU(s) addressed 
			to an AP or to a mesh STA, the Group ID field is set to 
			0, otherwise it is set to 63.  In an NDP PPDU the Group 
			ID is set according to IEEE 802.11ac_D1.0 Section 9.30.6
			 (Transmission of a VHT NDP). For a MU-MIMO PPDU the Group
			 ID is set as in 802.11ac_D1.0 Section 22.3.11.3 (Group 
			ID).  <legal all>
*/

#define VHT_SIG_A_INFO_GROUP_ID_OFFSET                                              0x00000000
#define VHT_SIG_A_INFO_GROUP_ID_LSB                                                 4
#define VHT_SIG_A_INFO_GROUP_ID_MSB                                                 9
#define VHT_SIG_A_INFO_GROUP_ID_MASK                                                0x000003f0


/* Description		N_STS

			For MU: 
			3 bits/user with maximum of 4 users (user u uses
			vht_sig_a[0][10+3u] - vht_sig_a[0][12+3u]), u = 0, 1, 2, 
			3) 
			Set to 0 for 0 space time streams
			Set to 1 for 1 space time stream
			Set to 2 for 2 space time streams
			Set to 3 for 3 space time streams
			Set to 4 for 4 space time streams (not supported in Wifi
			 3.0)
			Values 5-7 are reserved
			In this field, references to user "u" should be interpreted
			 as MU user "u". As described in the previous chapter in
			 this document (see chapter on User number), the MU user
			 value for a given client is defined for each MU group that
			 the client participates in. The MU user number is not related
			 to the internal user number that is used within the BFer. 
			
			
			
			For SU:
			vht_sig_a[0][12:10]
			Set to 0 for 1 space time stream
			Set to 1 for 2 space time streams
			Set to 2 for 3 space time streams
			Set to 3 for 4 space time streams 
			Set to 4 for 5 space time streams 
			Set to 5 for 6 space time streams
			Set to 6 for 7 space time streams
			Set to 7 for 8 space time streams
			
			vht_sig_a[0][21:13]
			Partial AID: 
			Set to the value of the TXVECTOR parameter PARTIAL_AID. 
			Partial AID provides an abbreviated indication of the intended
			 recipient(s) of the frame (see IEEE802.11ac_D1.0 Section
			 9.17a (Partial AID in VHT PPDUs)).
			<legal all>
*/

#define VHT_SIG_A_INFO_N_STS_OFFSET                                                 0x00000000
#define VHT_SIG_A_INFO_N_STS_LSB                                                    10
#define VHT_SIG_A_INFO_N_STS_MSB                                                    21
#define VHT_SIG_A_INFO_N_STS_MASK                                                   0x003ffc00


/* Description		TXOP_PS_NOT_ALLOWED

			E_num 0     txop_ps_allowed  Not supported: If set to by
			 VHT AP if it allows non-AP VHT STAs in TXOP power save 
			mode to enter Doze state during a TXOP
			<enum 1     no_txop_ps_allowed> Otherwise
			<legal 1>
*/

#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_OFFSET                                   0x00000000
#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_LSB                                      22
#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_MSB                                      22
#define VHT_SIG_A_INFO_TXOP_PS_NOT_ALLOWED_MASK                                     0x00400000


/* Description		VHTA_RESERVED_0B

			Reserved: Should be set to 1 by the MAC and ignored by the
			 PHY  <legal 1>
*/

#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_OFFSET                                      0x00000000
#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_LSB                                         23
#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_MSB                                         23
#define VHT_SIG_A_INFO_VHTA_RESERVED_0B_MASK                                        0x00800000


/* Description		RESERVED_0

			This field is not part of HT-SIG:
			Reserved: Should be set to 0 by the MAC and ignored by the
			 PHY <legal 0>
*/

#define VHT_SIG_A_INFO_RESERVED_0_OFFSET                                            0x00000000
#define VHT_SIG_A_INFO_RESERVED_0_LSB                                               24
#define VHT_SIG_A_INFO_RESERVED_0_MSB                                               31
#define VHT_SIG_A_INFO_RESERVED_0_MASK                                              0xff000000


/* Description		GI_SETTING

			<enum 0     normal_gi>  Indicates short guard interval is
			 not used in the data field
			<enum 1     short_gi>  Indicates short guard interval is
			 used in the data field
			<enum 3     short_gi_ambiguity>  Indicates short guard interval
			 is used in the data field and NSYM mod 10 = 9
			NSYM is defined in IEEE802.11ac_D1.0 Section 22.4.3 (TXTIME
			 and PSDU_LENGTH calculation).
			<legal 0,1,3>
*/

#define VHT_SIG_A_INFO_GI_SETTING_OFFSET                                            0x00000004
#define VHT_SIG_A_INFO_GI_SETTING_LSB                                               0
#define VHT_SIG_A_INFO_GI_SETTING_MSB                                               1
#define VHT_SIG_A_INFO_GI_SETTING_MASK                                              0x00000003


/* Description		SU_MU_CODING

			For an SU PPDU, B2 is set to 0 for BCC, 1 for LDPC For an
			 MU PPDU, if the MU[0] NSTS field is nonzero(#6773), then
			 B2 indicates the coding used for user 0; set to 0 for BCC
			 and 1 for LDPC. If the MU[0] NSTS field is 0, then this
			 field is reserved and set to 1
*/

#define VHT_SIG_A_INFO_SU_MU_CODING_OFFSET                                          0x00000004
#define VHT_SIG_A_INFO_SU_MU_CODING_LSB                                             2
#define VHT_SIG_A_INFO_SU_MU_CODING_MSB                                             2
#define VHT_SIG_A_INFO_SU_MU_CODING_MASK                                            0x00000004


/* Description		LDPC_EXTRA_SYMBOL

			Set to 1 if the LDPC PPDU encoding process (if an SU PPDU), 
			or at least one LDPC user's PPDU encoding process (if an
			 MU PPDU), results in an extra OFDM symbol (or symbols) 
			as described in 22.3.10.5.4 (LDPC coding) and 22.3.10.5.5
			 (Encoding process for MU PPDUs). Set to 0 otherwise.
*/

#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_OFFSET                                     0x00000004
#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_LSB                                        3
#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_MSB                                        3
#define VHT_SIG_A_INFO_LDPC_EXTRA_SYMBOL_MASK                                       0x00000008


/* Description		MCS

			For SU:
			Set to 0 for BPSK 1/2
			Set to 1 for QPSK 1/2
			Set to 2 for QPSK 3/4
			Set to 3 for 16-QAM 1/2
			Set to 4 for 16-QAM 3/4
			Set to 5 for 64-QAM 2/3
			Set to 6 for 64-QAM 3/4
			Set to 7 for 64-QAM 5/6
			Set to 8 for 256-QAM 3/4
			Set to 9 for 256-QAM 5/6
			For MU:
			If NSTS for user 1 is non-zero, then vht_sig_a[1][4] indicates
			 coding for user 1: set to 0 for BCC, 1 for LDPC.
			If NSTS for user 1 is set to 0, then vht_sig_a[1][4] is 
			reserved and set to 1.
			If NSTS for user 2 is non-zero, then vht_sig_a[1][5] indicates
			 coding for user 2: set to 0 for BCC, 1 for LDPC.
			If NSTS for user 2 is set to 0, then vht_sig_a[1][5] is 
			reserved and set to 1.
			If NSTS for user 3 is non-zero, then vht_sig_a[1][6] indicates
			 coding for user 3: set to 0 for BCC, 1 for LDPC.
			If NSTS for user 3 is set to 0, then vht_sig_a[1][6] is 
			reserved and set to 1.
			vht_sig_a[1][7] is reserved and set to 1
			<legal 0-15>
*/

#define VHT_SIG_A_INFO_MCS_OFFSET                                                   0x00000004
#define VHT_SIG_A_INFO_MCS_LSB                                                      4
#define VHT_SIG_A_INFO_MCS_MSB                                                      7
#define VHT_SIG_A_INFO_MCS_MASK                                                     0x000000f0


/* Description		BEAMFORMED

			For SU:
			Set to 1 if a Beamforming steering matrix is applied to 
			the waveform in an SU transmission as described in IEEE802.11ac_D1.0
			 Section 19.3.11.11.2 (Spatial mapping), set to 0 otherwise.
			
			For MU:
			Reserved and set to 1
			<legal 0-1>
*/

#define VHT_SIG_A_INFO_BEAMFORMED_OFFSET                                            0x00000004
#define VHT_SIG_A_INFO_BEAMFORMED_LSB                                               8
#define VHT_SIG_A_INFO_BEAMFORMED_MSB                                               8
#define VHT_SIG_A_INFO_BEAMFORMED_MASK                                              0x00000100


/* Description		VHTA_RESERVED_1

			Reserved and set to 1.  <legal 1>
*/

#define VHT_SIG_A_INFO_VHTA_RESERVED_1_OFFSET                                       0x00000004
#define VHT_SIG_A_INFO_VHTA_RESERVED_1_LSB                                          9
#define VHT_SIG_A_INFO_VHTA_RESERVED_1_MSB                                          9
#define VHT_SIG_A_INFO_VHTA_RESERVED_1_MASK                                         0x00000200


/* Description		CRC

			CRC calculated as in IEEE802.11ac_D1.0 Section 19.3.9.4.4
			 (CRC calculation for HTSIG) with C7 in vht_sig_a[1][10], 
			etc.  <legal all>
*/

#define VHT_SIG_A_INFO_CRC_OFFSET                                                   0x00000004
#define VHT_SIG_A_INFO_CRC_LSB                                                      10
#define VHT_SIG_A_INFO_CRC_MSB                                                      17
#define VHT_SIG_A_INFO_CRC_MASK                                                     0x0003fc00


/* Description		TAIL

			Used to terminate the trellis of the convolutional decoder. 
			 Set to 0.  <legal 0>
*/

#define VHT_SIG_A_INFO_TAIL_OFFSET                                                  0x00000004
#define VHT_SIG_A_INFO_TAIL_LSB                                                     18
#define VHT_SIG_A_INFO_TAIL_MSB                                                     23
#define VHT_SIG_A_INFO_TAIL_MASK                                                    0x00fc0000


/* Description		RESERVED_1

			This field is not part of HT-SIG:
			Reserved: Should be set to 0 by the MAC and ignored by the
			 PHY <legal 0>
*/

#define VHT_SIG_A_INFO_RESERVED_1_OFFSET                                            0x00000004
#define VHT_SIG_A_INFO_RESERVED_1_LSB                                               24
#define VHT_SIG_A_INFO_RESERVED_1_MSB                                               30
#define VHT_SIG_A_INFO_RESERVED_1_MASK                                              0x7f000000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the VHT-SIG-A CRC check
			 has passed, else set to 0
			
			<legal all>
*/

#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                             0x00000004
#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                                31
#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                                31
#define VHT_SIG_A_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                               0x80000000



#endif   // VHT_SIG_A_INFO

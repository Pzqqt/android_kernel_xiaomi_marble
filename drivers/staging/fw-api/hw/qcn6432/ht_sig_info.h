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

#ifndef _HT_SIG_INFO_H_
#define _HT_SIG_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_HT_SIG_INFO 2


struct ht_sig_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mcs                                                     :  7, // [6:0]
                      cbw                                                     :  1, // [7:7]
                      length                                                  : 16, // [23:8]
                      reserved_0                                              :  8; // [31:24]
             uint32_t smoothing                                               :  1, // [0:0]
                      not_sounding                                            :  1, // [1:1]
                      ht_reserved                                             :  1, // [2:2]
                      aggregation                                             :  1, // [3:3]
                      stbc                                                    :  2, // [5:4]
                      fec_coding                                              :  1, // [6:6]
                      short_gi                                                :  1, // [7:7]
                      num_ext_sp_str                                          :  2, // [9:8]
                      crc                                                     :  8, // [17:10]
                      signal_tail                                             :  6, // [23:18]
                      reserved_1                                              :  7, // [30:24]
                      rx_integrity_check_passed                               :  1; // [31:31]
#else
             uint32_t reserved_0                                              :  8, // [31:24]
                      length                                                  : 16, // [23:8]
                      cbw                                                     :  1, // [7:7]
                      mcs                                                     :  7; // [6:0]
             uint32_t rx_integrity_check_passed                               :  1, // [31:31]
                      reserved_1                                              :  7, // [30:24]
                      signal_tail                                             :  6, // [23:18]
                      crc                                                     :  8, // [17:10]
                      num_ext_sp_str                                          :  2, // [9:8]
                      short_gi                                                :  1, // [7:7]
                      fec_coding                                              :  1, // [6:6]
                      stbc                                                    :  2, // [5:4]
                      aggregation                                             :  1, // [3:3]
                      ht_reserved                                             :  1, // [2:2]
                      not_sounding                                            :  1, // [1:1]
                      smoothing                                               :  1; // [0:0]
#endif
};


/* Description		MCS

			Modulation Coding Scheme:
			0-7 are used for single stream
			8-15 are used for 2 streams
			16-23 are used for 3 streams
			24-31 are used for 4 streams
			32 is used for duplicate HT20 (unsupported)
			33-76 is used for unequal modulation (unsupported)
			77-127 is reserved.
			<legal 0-31>
*/

#define HT_SIG_INFO_MCS_OFFSET                                                      0x00000000
#define HT_SIG_INFO_MCS_LSB                                                         0
#define HT_SIG_INFO_MCS_MSB                                                         6
#define HT_SIG_INFO_MCS_MASK                                                        0x0000007f


/* Description		CBW

			Packet bandwidth:
			<enum 0     ht_20_mhz>
			<enum 1     ht_40_mhz>
			<legal 0-1>
*/

#define HT_SIG_INFO_CBW_OFFSET                                                      0x00000000
#define HT_SIG_INFO_CBW_LSB                                                         7
#define HT_SIG_INFO_CBW_MSB                                                         7
#define HT_SIG_INFO_CBW_MASK                                                        0x00000080


/* Description		LENGTH

			This is the MPDU or A-MPDU length in octets of the PPDU
			<legal all>
*/

#define HT_SIG_INFO_LENGTH_OFFSET                                                   0x00000000
#define HT_SIG_INFO_LENGTH_LSB                                                      8
#define HT_SIG_INFO_LENGTH_MSB                                                      23
#define HT_SIG_INFO_LENGTH_MASK                                                     0x00ffff00


/* Description		RESERVED_0

			This field is not part of HT-SIG
			Reserved: Should be set to 0 by the MAC and ignored by the
			 PHY <legal 0>
*/

#define HT_SIG_INFO_RESERVED_0_OFFSET                                               0x00000000
#define HT_SIG_INFO_RESERVED_0_LSB                                                  24
#define HT_SIG_INFO_RESERVED_0_MSB                                                  31
#define HT_SIG_INFO_RESERVED_0_MASK                                                 0xff000000


/* Description		SMOOTHING

			Field indicates if smoothing is needed
			E_num 0     do_smoothing Unsupported setting: indicates 
			smoothing is often used for beamforming 
			<enum 1     no_smoothing> Indicates no smoothing is used
			
			<legal 1>
*/

#define HT_SIG_INFO_SMOOTHING_OFFSET                                                0x00000004
#define HT_SIG_INFO_SMOOTHING_LSB                                                   0
#define HT_SIG_INFO_SMOOTHING_MSB                                                   0
#define HT_SIG_INFO_SMOOTHING_MASK                                                  0x00000001


/* Description		NOT_SOUNDING

			E_num 0     sounding Unsupported setting: indicates sounding
			 is used
			<enum 1     no_sounding>  Indicates no sounding is used
			<legal 1>
*/

#define HT_SIG_INFO_NOT_SOUNDING_OFFSET                                             0x00000004
#define HT_SIG_INFO_NOT_SOUNDING_LSB                                                1
#define HT_SIG_INFO_NOT_SOUNDING_MSB                                                1
#define HT_SIG_INFO_NOT_SOUNDING_MASK                                               0x00000002


/* Description		HT_RESERVED

			Reserved: Should be set to 1 by the MAC and ignored by the
			 PHY 
			<legal 1>
*/

#define HT_SIG_INFO_HT_RESERVED_OFFSET                                              0x00000004
#define HT_SIG_INFO_HT_RESERVED_LSB                                                 2
#define HT_SIG_INFO_HT_RESERVED_MSB                                                 2
#define HT_SIG_INFO_HT_RESERVED_MASK                                                0x00000004


/* Description		AGGREGATION

			<enum 0     mpdu> Indicates MPDU format
			<enum 1     a_mpdu> Indicates A-MPDU format
			<legal 0-1>
*/

#define HT_SIG_INFO_AGGREGATION_OFFSET                                              0x00000004
#define HT_SIG_INFO_AGGREGATION_LSB                                                 3
#define HT_SIG_INFO_AGGREGATION_MSB                                                 3
#define HT_SIG_INFO_AGGREGATION_MASK                                                0x00000008


/* Description		STBC

			<enum 0     no_stbc> Indicates no STBC
			<enum 1     1_str_stbc> Indicates 1 stream STBC
			E_num 2     2_str_stbc Indicates 2 stream STBC (Unsupported)
			
			<legal 0-1>
*/

#define HT_SIG_INFO_STBC_OFFSET                                                     0x00000004
#define HT_SIG_INFO_STBC_LSB                                                        4
#define HT_SIG_INFO_STBC_MSB                                                        5
#define HT_SIG_INFO_STBC_MASK                                                       0x00000030


/* Description		FEC_CODING

			<enum 0     ht_bcc>  Indicates BCC coding
			<enum 1     ht_ldpc>  Indicates LDPC coding
			<legal 0-1>
*/

#define HT_SIG_INFO_FEC_CODING_OFFSET                                               0x00000004
#define HT_SIG_INFO_FEC_CODING_LSB                                                  6
#define HT_SIG_INFO_FEC_CODING_MSB                                                  6
#define HT_SIG_INFO_FEC_CODING_MASK                                                 0x00000040


/* Description		SHORT_GI

			<enum 0     ht_normal_gi>  Indicates normal guard interval
			
			<enum 1     ht_short_gi>  Indicates short guard interval
			
			<legal 0-1>
*/

#define HT_SIG_INFO_SHORT_GI_OFFSET                                                 0x00000004
#define HT_SIG_INFO_SHORT_GI_LSB                                                    7
#define HT_SIG_INFO_SHORT_GI_MSB                                                    7
#define HT_SIG_INFO_SHORT_GI_MASK                                                   0x00000080


/* Description		NUM_EXT_SP_STR

			Number of extension spatial streams: (Used for TxBF)
			<enum 0     0_ext_sp_str>  No extension spatial streams
			E_num 1     1_ext_sp_str  Not supported: 1 extension spatial
			 streams
			E_num 2     2_ext_sp_str  Not supported:  2 extension spatial
			 streams
			<legal 0>
*/

#define HT_SIG_INFO_NUM_EXT_SP_STR_OFFSET                                           0x00000004
#define HT_SIG_INFO_NUM_EXT_SP_STR_LSB                                              8
#define HT_SIG_INFO_NUM_EXT_SP_STR_MSB                                              9
#define HT_SIG_INFO_NUM_EXT_SP_STR_MASK                                             0x00000300


/* Description		CRC

			The CRC protects the HT-SIG (HT-SIG[0][23:0] and HT-SIG[1][9:0]. 
			The generator polynomial is G(D) = D8 + D2 + D + 1.  <legal
			 all>
*/

#define HT_SIG_INFO_CRC_OFFSET                                                      0x00000004
#define HT_SIG_INFO_CRC_LSB                                                         10
#define HT_SIG_INFO_CRC_MSB                                                         17
#define HT_SIG_INFO_CRC_MASK                                                        0x0003fc00


/* Description		SIGNAL_TAIL

			The 6 bits of tail is always set to 0 is used to flush the
			 BCC encoder and decoder.  <legal 0>
*/

#define HT_SIG_INFO_SIGNAL_TAIL_OFFSET                                              0x00000004
#define HT_SIG_INFO_SIGNAL_TAIL_LSB                                                 18
#define HT_SIG_INFO_SIGNAL_TAIL_MSB                                                 23
#define HT_SIG_INFO_SIGNAL_TAIL_MASK                                                0x00fc0000


/* Description		RESERVED_1

			This field is not part of HT-SIG:
			Reserved: Should be set to 0 by the MAC and ignored by the
			 PHY.  <legal 0>
*/

#define HT_SIG_INFO_RESERVED_1_OFFSET                                               0x00000004
#define HT_SIG_INFO_RESERVED_1_LSB                                                  24
#define HT_SIG_INFO_RESERVED_1_MSB                                                  30
#define HT_SIG_INFO_RESERVED_1_MASK                                                 0x7f000000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the HT-SIG CRC check
			 has passed, else set to 0
			
			<legal all>
*/

#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                                0x00000004
#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                                   31
#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                                   31
#define HT_SIG_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                                  0x80000000



#endif   // HT_SIG_INFO

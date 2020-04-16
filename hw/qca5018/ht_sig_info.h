/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	mcs[6:0], cbw[7], length[23:8], reserved_0[31:24]
//	1	smoothing[0], not_sounding[1], ht_reserved[2], aggregation[3], stbc[5:4], fec_coding[6], short_gi[7], num_ext_sp_str[9:8], crc[17:10], signal_tail[23:18], reserved_1[31:24]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_HT_SIG_INFO 2

struct ht_sig_info {
             uint32_t mcs                             :  7, //[6:0]
                      cbw                             :  1, //[7]
                      length                          : 16, //[23:8]
                      reserved_0                      :  8; //[31:24]
             uint32_t smoothing                       :  1, //[0]
                      not_sounding                    :  1, //[1]
                      ht_reserved                     :  1, //[2]
                      aggregation                     :  1, //[3]
                      stbc                            :  2, //[5:4]
                      fec_coding                      :  1, //[6]
                      short_gi                        :  1, //[7]
                      num_ext_sp_str                  :  2, //[9:8]
                      crc                             :  8, //[17:10]
                      signal_tail                     :  6, //[23:18]
                      reserved_1                      :  8; //[31:24]
};

/*

mcs
			
			Modulation Coding Scheme:
			
			0-7 are used for single stream
			
			8-15 are used for 2 streams
			
			16-23 are used for 3 streams
			
			24-31 are used for 4 streams
			
			32 is used for duplicate HT20 (unsupported)
			
			33-76 is used for unequal modulation (unsupported)
			
			77-127 is reserved.
			
			<legal 0-31>

cbw
			
			Packet bandwidth:
			
			<enum 0     ht_20_mhz>
			
			<enum 1     ht_40_mhz>
			
			<legal 0-1>

length
			
			This is the MPDU or A-MPDU length in octets of the PPDU
			
			<legal all>

reserved_0
			
			This field is not part of HT-SIG
			
			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY <legal 0>

smoothing
			
			Field indicates if smoothing is needed
			
			E_num 0     do_smoothing Unsupported setting: indicates
			smoothing is often used for beamforming 
			
			
			<legal 1>

not_sounding
			
			E_num 0     sounding Unsupported setting: indicates
			sounding is used
			
			<enum 1     no_sounding>  Indicates no sounding is used
			
			<legal 1>

ht_reserved
			
			Reserved: Should be set to 1 by the MAC and ignored by
			the PHY 
			
			<legal 1>

aggregation
			
			<enum 0     mpdu> Indicates MPDU format
			
			<enum 1     a_mpdu> Indicates A-MPDU format
			
			<legal 0-1>

stbc
			
			<enum 0     no_stbc> Indicates no STBC
			
			<enum 1     1_str_stbc> Indicates 1 stream STBC
			
			E_num 2     2_str_stbc Indicates 2 stream STBC
			(Unsupported)
			
			<legal 0-1>

fec_coding
			
			<enum 0     ht_bcc>  Indicates BCC coding
			
			<enum 1     ht_ldpc>  Indicates LDPC coding
			
			<legal 0-1>

short_gi
			
			<enum 0     ht_normal_gi>  Indicates normal guard
			interval
			
			
			<legal 0-1>

num_ext_sp_str
			
			Number of extension spatial streams: (Used for TxBF)
			
			<enum 0     0_ext_sp_str>  No extension spatial streams
			
			E_num 1     1_ext_sp_str  Not supported: 1 extension
			spatial streams
			
			E_num 2     2_ext_sp_str  Not supported:  2 extension
			spatial streams
			
			<legal 0>

crc
			
			The CRC protects the HT-SIG (HT-SIG[0][23:0] and
			HT-SIG[1][9:0]. The generator polynomial is G(D) = D8 + D2 +
			D + 1.  <legal all>

signal_tail
			
			The 6 bits of tail is always set to 0 is used to flush
			the BCC encoder and decoder.  <legal 0>

reserved_1
			
			This field is not part of HT-SIG:
			
			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY.  <legal 0>
*/


/* Description		HT_SIG_INFO_0_MCS
			
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
#define HT_SIG_INFO_0_MCS_OFFSET                                     0x00000000
#define HT_SIG_INFO_0_MCS_LSB                                        0
#define HT_SIG_INFO_0_MCS_MASK                                       0x0000007f

/* Description		HT_SIG_INFO_0_CBW
			
			Packet bandwidth:
			
			<enum 0     ht_20_mhz>
			
			<enum 1     ht_40_mhz>
			
			<legal 0-1>
*/
#define HT_SIG_INFO_0_CBW_OFFSET                                     0x00000000
#define HT_SIG_INFO_0_CBW_LSB                                        7
#define HT_SIG_INFO_0_CBW_MASK                                       0x00000080

/* Description		HT_SIG_INFO_0_LENGTH
			
			This is the MPDU or A-MPDU length in octets of the PPDU
			
			<legal all>
*/
#define HT_SIG_INFO_0_LENGTH_OFFSET                                  0x00000000
#define HT_SIG_INFO_0_LENGTH_LSB                                     8
#define HT_SIG_INFO_0_LENGTH_MASK                                    0x00ffff00

/* Description		HT_SIG_INFO_0_RESERVED_0
			
			This field is not part of HT-SIG
			
			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY <legal 0>
*/
#define HT_SIG_INFO_0_RESERVED_0_OFFSET                              0x00000000
#define HT_SIG_INFO_0_RESERVED_0_LSB                                 24
#define HT_SIG_INFO_0_RESERVED_0_MASK                                0xff000000

/* Description		HT_SIG_INFO_1_SMOOTHING
			
			Field indicates if smoothing is needed
			
			E_num 0     do_smoothing Unsupported setting: indicates
			smoothing is often used for beamforming 
			
			
			<legal 1>
*/
#define HT_SIG_INFO_1_SMOOTHING_OFFSET                               0x00000004
#define HT_SIG_INFO_1_SMOOTHING_LSB                                  0
#define HT_SIG_INFO_1_SMOOTHING_MASK                                 0x00000001

/* Description		HT_SIG_INFO_1_NOT_SOUNDING
			
			E_num 0     sounding Unsupported setting: indicates
			sounding is used
			
			<enum 1     no_sounding>  Indicates no sounding is used
			
			<legal 1>
*/
#define HT_SIG_INFO_1_NOT_SOUNDING_OFFSET                            0x00000004
#define HT_SIG_INFO_1_NOT_SOUNDING_LSB                               1
#define HT_SIG_INFO_1_NOT_SOUNDING_MASK                              0x00000002

/* Description		HT_SIG_INFO_1_HT_RESERVED
			
			Reserved: Should be set to 1 by the MAC and ignored by
			the PHY 
			
			<legal 1>
*/
#define HT_SIG_INFO_1_HT_RESERVED_OFFSET                             0x00000004
#define HT_SIG_INFO_1_HT_RESERVED_LSB                                2
#define HT_SIG_INFO_1_HT_RESERVED_MASK                               0x00000004

/* Description		HT_SIG_INFO_1_AGGREGATION
			
			<enum 0     mpdu> Indicates MPDU format
			
			<enum 1     a_mpdu> Indicates A-MPDU format
			
			<legal 0-1>
*/
#define HT_SIG_INFO_1_AGGREGATION_OFFSET                             0x00000004
#define HT_SIG_INFO_1_AGGREGATION_LSB                                3
#define HT_SIG_INFO_1_AGGREGATION_MASK                               0x00000008

/* Description		HT_SIG_INFO_1_STBC
			
			<enum 0     no_stbc> Indicates no STBC
			
			<enum 1     1_str_stbc> Indicates 1 stream STBC
			
			E_num 2     2_str_stbc Indicates 2 stream STBC
			(Unsupported)
			
			<legal 0-1>
*/
#define HT_SIG_INFO_1_STBC_OFFSET                                    0x00000004
#define HT_SIG_INFO_1_STBC_LSB                                       4
#define HT_SIG_INFO_1_STBC_MASK                                      0x00000030

/* Description		HT_SIG_INFO_1_FEC_CODING
			
			<enum 0     ht_bcc>  Indicates BCC coding
			
			<enum 1     ht_ldpc>  Indicates LDPC coding
			
			<legal 0-1>
*/
#define HT_SIG_INFO_1_FEC_CODING_OFFSET                              0x00000004
#define HT_SIG_INFO_1_FEC_CODING_LSB                                 6
#define HT_SIG_INFO_1_FEC_CODING_MASK                                0x00000040

/* Description		HT_SIG_INFO_1_SHORT_GI
			
			<enum 0     ht_normal_gi>  Indicates normal guard
			interval
			
			
			<legal 0-1>
*/
#define HT_SIG_INFO_1_SHORT_GI_OFFSET                                0x00000004
#define HT_SIG_INFO_1_SHORT_GI_LSB                                   7
#define HT_SIG_INFO_1_SHORT_GI_MASK                                  0x00000080

/* Description		HT_SIG_INFO_1_NUM_EXT_SP_STR
			
			Number of extension spatial streams: (Used for TxBF)
			
			<enum 0     0_ext_sp_str>  No extension spatial streams
			
			E_num 1     1_ext_sp_str  Not supported: 1 extension
			spatial streams
			
			E_num 2     2_ext_sp_str  Not supported:  2 extension
			spatial streams
			
			<legal 0>
*/
#define HT_SIG_INFO_1_NUM_EXT_SP_STR_OFFSET                          0x00000004
#define HT_SIG_INFO_1_NUM_EXT_SP_STR_LSB                             8
#define HT_SIG_INFO_1_NUM_EXT_SP_STR_MASK                            0x00000300

/* Description		HT_SIG_INFO_1_CRC
			
			The CRC protects the HT-SIG (HT-SIG[0][23:0] and
			HT-SIG[1][9:0]. The generator polynomial is G(D) = D8 + D2 +
			D + 1.  <legal all>
*/
#define HT_SIG_INFO_1_CRC_OFFSET                                     0x00000004
#define HT_SIG_INFO_1_CRC_LSB                                        10
#define HT_SIG_INFO_1_CRC_MASK                                       0x0003fc00

/* Description		HT_SIG_INFO_1_SIGNAL_TAIL
			
			The 6 bits of tail is always set to 0 is used to flush
			the BCC encoder and decoder.  <legal 0>
*/
#define HT_SIG_INFO_1_SIGNAL_TAIL_OFFSET                             0x00000004
#define HT_SIG_INFO_1_SIGNAL_TAIL_LSB                                18
#define HT_SIG_INFO_1_SIGNAL_TAIL_MASK                               0x00fc0000

/* Description		HT_SIG_INFO_1_RESERVED_1
			
			This field is not part of HT-SIG:
			
			Reserved: Should be set to 0 by the MAC and ignored by
			the PHY.  <legal 0>
*/
#define HT_SIG_INFO_1_RESERVED_1_OFFSET                              0x00000004
#define HT_SIG_INFO_1_RESERVED_1_LSB                                 24
#define HT_SIG_INFO_1_RESERVED_1_MASK                                0xff000000


#endif // _HT_SIG_INFO_H_

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

#ifndef _L_SIG_B_INFO_H_
#define _L_SIG_B_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	rate[3:0], length[15:4], reserved[31:16]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_L_SIG_B_INFO 1

struct l_sig_b_info {
             uint32_t rate                            :  4, //[3:0]
                      length                          : 12, //[15:4]
                      reserved                        : 16; //[31:16]
};

/*

rate
			
			<enum 1    dsss_1_mpbs_long> DSSS 1 Mbps long
			
			<enum 2    dsss_2_mbps_long> DSSS 2 Mbps long
			
			<enum 3    cck_5_5_mbps_long> CCK 5.5 Mbps long
			
			<enum 4    cck_11_mbps_long> CCK 11 Mbps long
			
			<enum 5    dsss_2_mbps_short> DSSS 2 Mbps short
			
			<enum 6    cck_5_5_mbps_short> CCK 5.5 Mbps short
			
			<enum 7    cck_11_mbps_short> CCK 11 Mbps short
			
			<legal 1-7>

length
			
			The length indicates the number of octets in this MPDU.
			
			<legal all>

reserved
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>
*/


/* Description		L_SIG_B_INFO_0_RATE
			
			<enum 1    dsss_1_mpbs_long> DSSS 1 Mbps long
			
			<enum 2    dsss_2_mbps_long> DSSS 2 Mbps long
			
			<enum 3    cck_5_5_mbps_long> CCK 5.5 Mbps long
			
			<enum 4    cck_11_mbps_long> CCK 11 Mbps long
			
			<enum 5    dsss_2_mbps_short> DSSS 2 Mbps short
			
			<enum 6    cck_5_5_mbps_short> CCK 5.5 Mbps short
			
			<enum 7    cck_11_mbps_short> CCK 11 Mbps short
			
			<legal 1-7>
*/
#define L_SIG_B_INFO_0_RATE_OFFSET                                   0x00000000
#define L_SIG_B_INFO_0_RATE_LSB                                      0
#define L_SIG_B_INFO_0_RATE_MASK                                     0x0000000f

/* Description		L_SIG_B_INFO_0_LENGTH
			
			The length indicates the number of octets in this MPDU.
			
			<legal all>
*/
#define L_SIG_B_INFO_0_LENGTH_OFFSET                                 0x00000000
#define L_SIG_B_INFO_0_LENGTH_LSB                                    4
#define L_SIG_B_INFO_0_LENGTH_MASK                                   0x0000fff0

/* Description		L_SIG_B_INFO_0_RESERVED
			
			Reserved: Should be set to 0 by the transmitting MAC and
			ignored by the PHY <legal 0>
*/
#define L_SIG_B_INFO_0_RESERVED_OFFSET                               0x00000000
#define L_SIG_B_INFO_0_RESERVED_LSB                                  16
#define L_SIG_B_INFO_0_RESERVED_MASK                                 0xffff0000


#endif // _L_SIG_B_INFO_H_

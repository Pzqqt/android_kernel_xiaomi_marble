
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

 
 
 
 
 
 
 


#ifndef _L_SIG_B_INFO_H_
#define _L_SIG_B_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_L_SIG_B_INFO 1


struct l_sig_b_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rate                                                    :  4, // [3:0]
                      length                                                  : 12, // [15:4]
                      reserved                                                : 15, // [30:16]
                      rx_integrity_check_passed                               :  1; // [31:31]
#else
             uint32_t rx_integrity_check_passed                               :  1, // [31:31]
                      reserved                                                : 15, // [30:16]
                      length                                                  : 12, // [15:4]
                      rate                                                    :  4; // [3:0]
#endif
};


/* Description		RATE

			<enum 1    dsss_1_mpbs_long> DSSS 1 Mbps long
			<enum 2    dsss_2_mbps_long> DSSS 2 Mbps long
			<enum 3    cck_5_5_mbps_long> CCK 5.5 Mbps long
			<enum 4    cck_11_mbps_long> CCK 11 Mbps long
			<enum 5    dsss_2_mbps_short> DSSS 2 Mbps short
			<enum 6    cck_5_5_mbps_short> CCK 5.5 Mbps short
			<enum 7    cck_11_mbps_short> CCK 11 Mbps short
			<legal 1-7>
*/

#define L_SIG_B_INFO_RATE_OFFSET                                                    0x00000000
#define L_SIG_B_INFO_RATE_LSB                                                       0
#define L_SIG_B_INFO_RATE_MSB                                                       3
#define L_SIG_B_INFO_RATE_MASK                                                      0x0000000f


/* Description		LENGTH

			The length indicates the number of octets in this MPDU.
			<legal all>
*/

#define L_SIG_B_INFO_LENGTH_OFFSET                                                  0x00000000
#define L_SIG_B_INFO_LENGTH_LSB                                                     4
#define L_SIG_B_INFO_LENGTH_MSB                                                     15
#define L_SIG_B_INFO_LENGTH_MASK                                                    0x0000fff0


/* Description		RESERVED

			Reserved: Should be set to 0 by the transmitting MAC and
			 ignored by the PHY <legal 0>
*/

#define L_SIG_B_INFO_RESERVED_OFFSET                                                0x00000000
#define L_SIG_B_INFO_RESERVED_LSB                                                   16
#define L_SIG_B_INFO_RESERVED_MSB                                                   30
#define L_SIG_B_INFO_RESERVED_MASK                                                  0x7fff0000


/* Description		RX_INTEGRITY_CHECK_PASSED

			TX side: Set to 0
			RX side: Set to 1 if PHY determines the .11b PHY header 
			CRC check has passed, else set to 0
			
			<legal all>
*/

#define L_SIG_B_INFO_RX_INTEGRITY_CHECK_PASSED_OFFSET                               0x00000000
#define L_SIG_B_INFO_RX_INTEGRITY_CHECK_PASSED_LSB                                  31
#define L_SIG_B_INFO_RX_INTEGRITY_CHECK_PASSED_MSB                                  31
#define L_SIG_B_INFO_RX_INTEGRITY_CHECK_PASSED_MASK                                 0x80000000



#endif   // L_SIG_B_INFO

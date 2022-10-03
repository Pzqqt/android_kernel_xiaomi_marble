
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

 
 
 
 
 
 
 


#ifndef _MON_DROP_H_
#define _MON_DROP_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MON_DROP 2

#define NUM_OF_QWORDS_MON_DROP 1


struct mon_drop {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ppdu_id                                                 : 32; // [31:0]
             uint32_t ppdu_drop_cnt                                           : 10, // [9:0]
                      mpdu_drop_cnt                                           : 10, // [19:10]
                      tlv_drop_cnt                                            : 10, // [29:20]
                      end_of_ppdu_seen                                        :  1, // [30:30]
                      reserved_1a                                             :  1; // [31:31]
#else
             uint32_t ppdu_id                                                 : 32; // [31:0]
             uint32_t reserved_1a                                             :  1, // [31:31]
                      end_of_ppdu_seen                                        :  1, // [30:30]
                      tlv_drop_cnt                                            : 10, // [29:20]
                      mpdu_drop_cnt                                           : 10, // [19:10]
                      ppdu_drop_cnt                                           : 10; // [9:0]
#endif
};


/* Description		PPDU_ID

			The ID of the last PPDU which saw the back-pressure on AXI
			
			
			TXMON fills this with the schedule_id from 'TX_FES_SETUP' 
			in case of a TX FES (TXOP initiator).
			TXMON fills this with the Phy_ppdu_id from 'RX_RESPONSE_REQUIRED_INFO' 
			in case of a response TX (TXOP responder).
			RXMON fills this with the Phy_ppdu_id from 'RX_PPDU_START.'
			
			<legal all>
*/

#define MON_DROP_PPDU_ID_OFFSET                                                     0x0000000000000000
#define MON_DROP_PPDU_ID_LSB                                                        0
#define MON_DROP_PPDU_ID_MSB                                                        31
#define MON_DROP_PPDU_ID_MASK                                                       0x00000000ffffffff


/* Description		PPDU_DROP_CNT

			The number of PPDUs dropped due to the back-pressure
			
			Set to 1023 if >1023 PPDUs got dropped
			<legal all>
*/

#define MON_DROP_PPDU_DROP_CNT_OFFSET                                               0x0000000000000000
#define MON_DROP_PPDU_DROP_CNT_LSB                                                  32
#define MON_DROP_PPDU_DROP_CNT_MSB                                                  41
#define MON_DROP_PPDU_DROP_CNT_MASK                                                 0x000003ff00000000


/* Description		MPDU_DROP_CNT

			The number of MPDUs dropped within the first PPDU due to
			 the back-pressure
			
			Set to 1023 if >1023 MPDUs got dropped
			<legal all>
*/

#define MON_DROP_MPDU_DROP_CNT_OFFSET                                               0x0000000000000000
#define MON_DROP_MPDU_DROP_CNT_LSB                                                  42
#define MON_DROP_MPDU_DROP_CNT_MSB                                                  51
#define MON_DROP_MPDU_DROP_CNT_MASK                                                 0x000ffc0000000000


/* Description		TLV_DROP_CNT

			The number of PPDU-level (global or per-user) TLVs dropped
			 within the first PPDU due to the back-pressure
*/

#define MON_DROP_TLV_DROP_CNT_OFFSET                                                0x0000000000000000
#define MON_DROP_TLV_DROP_CNT_LSB                                                   52
#define MON_DROP_TLV_DROP_CNT_MSB                                                   61
#define MON_DROP_TLV_DROP_CNT_MASK                                                  0x3ff0000000000000


/* Description		END_OF_PPDU_SEEN

			Field valid only if mpdu_drop_cnt > 0 or tlv_drop_cnt > 
			0
			
			Set by TXMON if 'TX_FES_STATUS_END' is received but dropped
			 in case of a TX FES (TXOP initiator).
			Set by TXMON if 'RESPONSE_END_STATUS' is received but dropped
			 in case of a response TX (TXOP responder).
			Set by RXMON if 'RX_PPDU_END' is received but dropped
*/

#define MON_DROP_END_OF_PPDU_SEEN_OFFSET                                            0x0000000000000000
#define MON_DROP_END_OF_PPDU_SEEN_LSB                                               62
#define MON_DROP_END_OF_PPDU_SEEN_MSB                                               62
#define MON_DROP_END_OF_PPDU_SEEN_MASK                                              0x4000000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define MON_DROP_RESERVED_1A_OFFSET                                                 0x0000000000000000
#define MON_DROP_RESERVED_1A_LSB                                                    63
#define MON_DROP_RESERVED_1A_MSB                                                    63
#define MON_DROP_RESERVED_1A_MASK                                                   0x8000000000000000



#endif   // MON_DROP

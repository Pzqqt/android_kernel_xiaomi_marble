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

#ifndef _RESPONSE_START_STATUS_H_
#define _RESPONSE_START_STATUS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RESPONSE_START_STATUS 2

#define NUM_OF_QWORDS_RESPONSE_START_STATUS 1


struct response_start_status {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t generated_response                                      :  3, // [2:0]
                      ftm_tm                                                  :  2, // [4:3]
                      trig_response_related                                   :  1, // [5:5]
                      response_sta_count                                      :  7, // [12:6]
                      reserved                                                : 19; // [31:13]
             uint32_t phy_ppdu_id                                             : 16, // [15:0]
                      sw_peer_id                                              : 16; // [31:16]
#else
             uint32_t reserved                                                : 19, // [31:13]
                      response_sta_count                                      :  7, // [12:6]
                      trig_response_related                                   :  1, // [5:5]
                      ftm_tm                                                  :  2, // [4:3]
                      generated_response                                      :  3; // [2:0]
             uint32_t sw_peer_id                                              : 16, // [31:16]
                      phy_ppdu_id                                             : 16; // [15:0]
#endif
};


/* Description		GENERATED_RESPONSE

			The generated response frame
			
			<enum 0 selfgen_ACK> TXPCU generated an ACK response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 1 selfgen_CTS> TXPCU generated an CTS response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 2 selfgen_BA> TXPCU generated a BA response. Note 
			that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 3 selfgen_MBA> TXPCU generated an M BA response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 4 selfgen_CBF> TXPCU generated a CBF response. Note
			 that this can be part of a trigger response. In that case
			 bit trig_response_related will be set as well.
			
			<enum 5 selfgen_other_trig_response>
			TXPCU generated a trigger related response of a type not
			 specified above. Note that in this case bit trig_response_related
			 will be set as well. 
			
			This e-num will also be used when TXPCU has been programmed
			 to overwrite it's own self gen response generation, and
			 wait for the response to come from SCH..
			Also applicable for basic trigger response. 
			
			<enum 6 selfgen_NDP_LMR> TXPCU generated a self-gen NDP 
			followed by a self-gen LMR for the ranging NDPA followed
			 by NDP received by RXPCU.
			
			<legal 0-6>
*/

#define RESPONSE_START_STATUS_GENERATED_RESPONSE_OFFSET                             0x0000000000000000
#define RESPONSE_START_STATUS_GENERATED_RESPONSE_LSB                                0
#define RESPONSE_START_STATUS_GENERATED_RESPONSE_MSB                                2
#define RESPONSE_START_STATUS_GENERATED_RESPONSE_MASK                               0x0000000000000007


/* Description		FTM_TM

			This field Indicates if the response is related to receiving
			 a TM or FTM frame
			
			0: no TM and no FTM frame => there is NO measurement done
			
			1: FTM frame
			2: TM frame
			3: reserved
*/

#define RESPONSE_START_STATUS_FTM_TM_OFFSET                                         0x0000000000000000
#define RESPONSE_START_STATUS_FTM_TM_LSB                                            3
#define RESPONSE_START_STATUS_FTM_TM_MSB                                            4
#define RESPONSE_START_STATUS_FTM_TM_MASK                                           0x0000000000000018


/* Description		TRIG_RESPONSE_RELATED

			When set, this TLV is generated by TXPCU in the context 
			of a response transmission to a received trigger frame.
			
			<legal all>
*/

#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_OFFSET                          0x0000000000000000
#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_LSB                             5
#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_MSB                             5
#define RESPONSE_START_STATUS_TRIG_RESPONSE_RELATED_MASK                            0x0000000000000020


/* Description		RESPONSE_STA_COUNT

			The number of STAs to which the responses need to be sent.
			
			
			In case of multiple ACKs/BAs to be send, TXPCU uses this
			 field to determine what address formatting to use for the
			 response frame: This could be broadcast or unicast.
			
			<legal all>
*/

#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_OFFSET                             0x0000000000000000
#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_LSB                                6
#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_MSB                                12
#define RESPONSE_START_STATUS_RESPONSE_STA_COUNT_MASK                               0x0000000000001fc0


/* Description		RESERVED

			<legal 0>
*/

#define RESPONSE_START_STATUS_RESERVED_OFFSET                                       0x0000000000000000
#define RESPONSE_START_STATUS_RESERVED_LSB                                          13
#define RESPONSE_START_STATUS_RESERVED_MSB                                          31
#define RESPONSE_START_STATUS_RESERVED_MASK                                         0x00000000ffffe000


/* Description		PHY_PPDU_ID

			The PHY_PPDU_ID of the received PPDU for which this response
			 is generated.
*/

#define RESPONSE_START_STATUS_PHY_PPDU_ID_OFFSET                                    0x0000000000000000
#define RESPONSE_START_STATUS_PHY_PPDU_ID_LSB                                       32
#define RESPONSE_START_STATUS_PHY_PPDU_ID_MSB                                       47
#define RESPONSE_START_STATUS_PHY_PPDU_ID_MASK                                      0x0000ffff00000000


/* Description		SW_PEER_ID

			This field is only valid when Response_STA_count  is set
			 to 1
			
			An identifier indicating for which device this response 
			is needed.
			<legal all>
*/

#define RESPONSE_START_STATUS_SW_PEER_ID_OFFSET                                     0x0000000000000000
#define RESPONSE_START_STATUS_SW_PEER_ID_LSB                                        48
#define RESPONSE_START_STATUS_SW_PEER_ID_MSB                                        63
#define RESPONSE_START_STATUS_SW_PEER_ID_MASK                                       0xffff000000000000



#endif   // RESPONSE_START_STATUS

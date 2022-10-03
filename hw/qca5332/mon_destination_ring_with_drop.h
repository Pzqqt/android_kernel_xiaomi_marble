
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

 
 
 
 
 
 
 


#ifndef _MON_DESTINATION_RING_WITH_DROP_H_
#define _MON_DESTINATION_RING_WITH_DROP_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MON_DESTINATION_RING_WITH_DROP 4


struct mon_destination_ring_with_drop {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ppdu_drop_cnt                                           : 10, // [9:0]
                      mpdu_drop_cnt                                           : 10, // [19:10]
                      tlv_drop_cnt                                            : 10, // [29:20]
                      end_of_ppdu_seen                                        :  1, // [30:30]
                      reserved_0a                                             :  1; // [31:31]
             uint32_t reserved_1a                                             : 32; // [31:0]
             uint32_t ppdu_id                                                 : 32; // [31:0]
             uint32_t reserved_3a                                             : 18, // [17:0]
                      initiator                                               :  1, // [18:18]
                      empty_descriptor                                        :  1, // [19:19]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             uint32_t reserved_0a                                             :  1, // [31:31]
                      end_of_ppdu_seen                                        :  1, // [30:30]
                      tlv_drop_cnt                                            : 10, // [29:20]
                      mpdu_drop_cnt                                           : 10, // [19:10]
                      ppdu_drop_cnt                                           : 10; // [9:0]
             uint32_t reserved_1a                                             : 32; // [31:0]
             uint32_t ppdu_id                                                 : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      empty_descriptor                                        :  1, // [19:19]
                      initiator                                               :  1, // [18:18]
                      reserved_3a                                             : 18; // [17:0]
#endif
};


/* Description		PPDU_DROP_CNT

			The number of PPDUs dropped due to the back-pressure
			
			Set to 1023 if >1023 PPDUs got dropped
			<legal all>
*/

#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_OFFSET                         0x00000000
#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_LSB                            0
#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_MSB                            9
#define MON_DESTINATION_RING_WITH_DROP_PPDU_DROP_CNT_MASK                           0x000003ff


/* Description		MPDU_DROP_CNT

			The number of MPDUs dropped within the first PPDU due to
			 the back-pressure
			
			Set to 1023 if >1023 MPDUs got dropped
			<legal all>
*/

#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_OFFSET                         0x00000000
#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_LSB                            10
#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_MSB                            19
#define MON_DESTINATION_RING_WITH_DROP_MPDU_DROP_CNT_MASK                           0x000ffc00


/* Description		TLV_DROP_CNT

			The number of PPDU-level (global or per-user) TLVs dropped
			 within the first PPDU due to the back-pressure
*/

#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_OFFSET                          0x00000000
#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_LSB                             20
#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_MSB                             29
#define MON_DESTINATION_RING_WITH_DROP_TLV_DROP_CNT_MASK                            0x3ff00000


/* Description		END_OF_PPDU_SEEN

			Field valid only if mpdu_drop_cnt > 0 or tlv_drop_cnt > 
			0
			
			Set by TXMON if 'TX_FES_STATUS_END' is received for a partially
			 dropped PPDU when Initiator = 1.
			Set by TXMON if 'RESPONSE_END_STATUS' is received for a 
			partially dropped PPDU when Initiator = 0.
			Set by RXMON if 'RX_PPDU_END_STATUS_DONE' is received for
			 a partially dropped PPDU.
*/

#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_OFFSET                      0x00000000
#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_LSB                         30
#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_MSB                         30
#define MON_DESTINATION_RING_WITH_DROP_END_OF_PPDU_SEEN_MASK                        0x40000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_OFFSET                           0x00000000
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_LSB                              31
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_MSB                              31
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_0A_MASK                             0x80000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_OFFSET                           0x00000004
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_LSB                              0
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_MSB                              31
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_1A_MASK                             0xffffffff


/* Description		PPDU_ID

			The ID of the last PPDU which saw the back-pressure on AXI
			
			
			TXMON fills this with the schedule_id from 'TX_FES_SETUP' 
			when Initiator = 1.
			TXMON fills this with the Phy_ppdu_id from 'RX_RESPONSE_REQUIRED_INFO' 
			when Initiator = 0.
			RXMON fills this with the Phy_ppdu_id from 'RX_PPDU_START.'
			
			<legal all>
*/

#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_OFFSET                               0x00000008
#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_LSB                                  0
#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_MSB                                  31
#define MON_DESTINATION_RING_WITH_DROP_PPDU_ID_MASK                                 0xffffffff


/* Description		RESERVED_3A

			<legal 0>
*/

#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_OFFSET                           0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_LSB                              0
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_MSB                              17
#define MON_DESTINATION_RING_WITH_DROP_RESERVED_3A_MASK                             0x0003ffff


/* Description		INITIATOR

			1: This descriptor belongs to a TX FES (TXOP initiator)
			0: This descriptor belongs to a response TX (TXOP responder)
			
			<legal all>
*/

#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_OFFSET                             0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_LSB                                18
#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_MSB                                18
#define MON_DESTINATION_RING_WITH_DROP_INITIATOR_MASK                               0x00040000


/* Description		EMPTY_DESCRIPTOR

			0: This descriptor is written on a flush or the end of a
			 PPDU or the end of status buffer (see 'MON_DESTINATION_RING' 
			structure)
			1: This descriptor is written to indicate drop information
			
			<legal 1>
*/

#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_OFFSET                      0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_LSB                         19
#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_MSB                         19
#define MON_DESTINATION_RING_WITH_DROP_EMPTY_DESCRIPTOR_MASK                        0x00080000


/* Description		RING_ID

			Consumer: SW/REO/DEBUG
			Producer: SRNG (of TXMON/RXMON)
			
			For debugging. 
			This field is filled in by the SRNG module.
			It help to identify the ring that is being looked
			<legal 0>
*/

#define MON_DESTINATION_RING_WITH_DROP_RING_ID_OFFSET                               0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_RING_ID_LSB                                  20
#define MON_DESTINATION_RING_WITH_DROP_RING_ID_MSB                                  27
#define MON_DESTINATION_RING_WITH_DROP_RING_ID_MASK                                 0x0ff00000


/* Description		LOOPING_COUNT

			Consumer: SW/DEBUG
			Producer: SRNG (of TXMON/RXMON)
			
			For debugging. 
			This field is filled in by the SRNG module.
			
			A count value that indicates the number of times the producer
			 of entries into this Ring has looped around the ring.
			At initialization time, this value is set to 0. On the first
			 loop, this value is set to 1. After the max value is reached
			 allowed by the number of bits for this field, the count
			 value continues with 0 again.
			
			In case SW is the consumer of the ring entries, it can use
			 this field to figure out up to where the producer of entries
			 has created new entries. This eliminates the need to check
			 where the "head pointer' of the ring is located once the
			 SW starts processing an interrupt indicating that new entries
			 have been put into this ring...
			
			Also note that SW if it wants only needs to look at the 
			LSB bit of this count value.
			<legal all>
*/

#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_OFFSET                         0x0000000c
#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_LSB                            28
#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_MSB                            31
#define MON_DESTINATION_RING_WITH_DROP_LOOPING_COUNT_MASK                           0xf0000000



#endif   // MON_DESTINATION_RING_WITH_DROP

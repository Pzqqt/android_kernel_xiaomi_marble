
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

 
 
 
 
 
 
 


#ifndef _MON_DESTINATION_RING_H_
#define _MON_DESTINATION_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MON_DESTINATION_RING 4


struct mon_destination_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t stat_buf_virt_addr_31_0                                 : 32; // [31:0]
             uint32_t stat_buf_virt_addr_63_32                                : 32; // [31:0]
             uint32_t ppdu_id                                                 : 32; // [31:0]
             uint32_t end_offset                                              : 12, // [11:0]
                      reserved_3a                                             :  4, // [15:12]
                      end_reason                                              :  2, // [17:16]
                      initiator                                               :  1, // [18:18]
                      empty_descriptor                                        :  1, // [19:19]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             uint32_t stat_buf_virt_addr_31_0                                 : 32; // [31:0]
             uint32_t stat_buf_virt_addr_63_32                                : 32; // [31:0]
             uint32_t ppdu_id                                                 : 32; // [31:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      empty_descriptor                                        :  1, // [19:19]
                      initiator                                               :  1, // [18:18]
                      end_reason                                              :  2, // [17:16]
                      reserved_3a                                             :  4, // [15:12]
                      end_offset                                              : 12; // [11:0]
#endif
};


/* Description		STAT_BUF_VIRT_ADDR_31_0

			Lower 32 bits of the 64-bit virtual address of the status
			 buffer
			<legal all>
*/

#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_31_0_OFFSET                         0x00000000
#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_31_0_LSB                            0
#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_31_0_MSB                            31
#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_31_0_MASK                           0xffffffff


/* Description		STAT_BUF_VIRT_ADDR_63_32

			Upper 32 bits of the 64-bit virtual address of the status
			 buffer
			<legal all>
*/

#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_63_32_OFFSET                        0x00000004
#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_63_32_LSB                           0
#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_63_32_MSB                           31
#define MON_DESTINATION_RING_STAT_BUF_VIRT_ADDR_63_32_MASK                          0xffffffff


/* Description		PPDU_ID

			TXMON fills this with the schedule_id from 'TX_FES_SETUP' 
			when Initiator = 1.
			TXMON fills this with the Phy_ppdu_id from 'RX_RESPONSE_REQUIRED_INFO' 
			when Initiator = 0.
			RXMON fills this with the Phy_ppdu_id from 'RX_PPDU_START.'
			
			<legal all>
*/

#define MON_DESTINATION_RING_PPDU_ID_OFFSET                                         0x00000008
#define MON_DESTINATION_RING_PPDU_ID_LSB                                            0
#define MON_DESTINATION_RING_PPDU_ID_MSB                                            31
#define MON_DESTINATION_RING_PPDU_ID_MASK                                           0xffffffff


/* Description		END_OFFSET

			The offset (in units of 4 bytes) into the status buffer 
			where DMA ended, i.e. offset to the last TLV + last TLV 
			size MINUS 1.
			
			In case of a 'TX_FLUSH' or 'RX_FLUSH,' this reflects the
			 offset at which flush occurred.
			<legal all>
*/

#define MON_DESTINATION_RING_END_OFFSET_OFFSET                                      0x0000000c
#define MON_DESTINATION_RING_END_OFFSET_LSB                                         0
#define MON_DESTINATION_RING_END_OFFSET_MSB                                         11
#define MON_DESTINATION_RING_END_OFFSET_MASK                                        0x00000fff


/* Description		RESERVED_3A

			<legal 0>
*/

#define MON_DESTINATION_RING_RESERVED_3A_OFFSET                                     0x0000000c
#define MON_DESTINATION_RING_RESERVED_3A_LSB                                        12
#define MON_DESTINATION_RING_RESERVED_3A_MSB                                        15
#define MON_DESTINATION_RING_RESERVED_3A_MASK                                       0x0000f000


/* Description		END_REASON

			<enum 0 MON_status_buffer_full> The status buffer was fully
			 written.
			<enum 1 MON_flush_detected> A 'TX_FLUSH' or 'RX_FLUSH' was
			 received. This is implicitly the end of the Tx FES or Rx
			 PPDU. The status buffer data can be discarded by SW.
			<enum 2 MON_end_of_PPDU> A 'TX_FES_STATUS_END' or 'RX_PPDU_END' 
			was received indicating the end of the Tx FES or Rx PPDU.
			
			<enum 3 MON_PPDU_truncated> The PPDU got truncated due to
			 a system-level error.
			<legal all>
*/

#define MON_DESTINATION_RING_END_REASON_OFFSET                                      0x0000000c
#define MON_DESTINATION_RING_END_REASON_LSB                                         16
#define MON_DESTINATION_RING_END_REASON_MSB                                         17
#define MON_DESTINATION_RING_END_REASON_MASK                                        0x00030000


/* Description		INITIATOR

			1: This descriptor belongs to a TX FES (TXOP initiator)
			0: This descriptor belongs to a response TX (TXOP responder)
			
			<legal all>
*/

#define MON_DESTINATION_RING_INITIATOR_OFFSET                                       0x0000000c
#define MON_DESTINATION_RING_INITIATOR_LSB                                          18
#define MON_DESTINATION_RING_INITIATOR_MSB                                          18
#define MON_DESTINATION_RING_INITIATOR_MASK                                         0x00040000


/* Description		EMPTY_DESCRIPTOR

			0: This descriptor is written on a flush or the end of a
			 PPDU or the end of status buffer
			1: This descriptor is written to indicate drop information
			 (see 'MON_DESTINATION_RING_WITH_DROP' structure)
			<legal 0>
*/

#define MON_DESTINATION_RING_EMPTY_DESCRIPTOR_OFFSET                                0x0000000c
#define MON_DESTINATION_RING_EMPTY_DESCRIPTOR_LSB                                   19
#define MON_DESTINATION_RING_EMPTY_DESCRIPTOR_MSB                                   19
#define MON_DESTINATION_RING_EMPTY_DESCRIPTOR_MASK                                  0x00080000


/* Description		RING_ID

			Consumer: SW/REO/DEBUG
			Producer: SRNG (of TXMON/RXMON)
			
			For debugging. 
			This field is filled in by the SRNG module.
			It help to identify the ring that is being looked
			<legal 0>
*/

#define MON_DESTINATION_RING_RING_ID_OFFSET                                         0x0000000c
#define MON_DESTINATION_RING_RING_ID_LSB                                            20
#define MON_DESTINATION_RING_RING_ID_MSB                                            27
#define MON_DESTINATION_RING_RING_ID_MASK                                           0x0ff00000


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

#define MON_DESTINATION_RING_LOOPING_COUNT_OFFSET                                   0x0000000c
#define MON_DESTINATION_RING_LOOPING_COUNT_LSB                                      28
#define MON_DESTINATION_RING_LOOPING_COUNT_MSB                                      31
#define MON_DESTINATION_RING_LOOPING_COUNT_MASK                                     0xf0000000



#endif   // MON_DESTINATION_RING

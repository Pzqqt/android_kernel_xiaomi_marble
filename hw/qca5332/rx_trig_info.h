
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

 
 
 
 
 
 
 


#ifndef _RX_TRIG_INFO_H_
#define _RX_TRIG_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_TRIG_INFO 2

#define NUM_OF_QWORDS_RX_TRIG_INFO 1


struct rx_trig_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rx_trigger_frame_type                                   :  2, // [1:0]
                      trigger_resp_type                                       :  3, // [4:2]
                      reserved_0                                              : 27; // [31:5]
             uint32_t ppdu_duration                                           : 16, // [15:0]
                      unique_destination_id                                   : 16; // [31:16]
#else
             uint32_t reserved_0                                              : 27, // [31:5]
                      trigger_resp_type                                       :  3, // [4:2]
                      rx_trigger_frame_type                                   :  2; // [1:0]
             uint32_t unique_destination_id                                   : 16, // [31:16]
                      ppdu_duration                                           : 16; // [15:0]
#endif
};


/* Description		RX_TRIGGER_FRAME_TYPE

			Trigger frame type.
			
			Field not really needed by PDG, but is there for debugging
			 purposes to be put in event.
			
			<enum 0 dot11ax_direct_trigger_frame>
			<enum 1 dot11ax_wildcard_trigger_frame>
			<enum 2 dot11ax_usassoc_wildcard_trigger_frame> 
			
			<legal 0-2>
*/

#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_OFFSET                                   0x0000000000000000
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_LSB                                      0
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_MSB                                      1
#define RX_TRIG_INFO_RX_TRIGGER_FRAME_TYPE_MASK                                     0x0000000000000003


/* Description		TRIGGER_RESP_TYPE

			Indicates what kind of response is required to the received
			 OFDMA trigger...
			
			Field not really needed by PDG, but is there for debugging
			 purposes to be put in event.
			<enum 0 OFDMA_ACK_frame> OFDMA trigger indicates an OFDMA
			 based transmission, where the contents shall be and ACK
			 frame.
			<enum 1 OFDMA_BA_frames> OFDMA trigger indicates an OFDMA
			 based transmission, where the contents shall be a BA frame.
			
			<enum 2 OFDMA_DATA_frames> OFDMA trigger indicates an OFDMA
			 based transmission, where the contents shall be only data.
			
			<enum 3 OFDMA_BA_DATA_frames> OFDMA trigger indicates an
			 OFDMA based transmission, where the contents shall be a
			 BA frame and data.
			
			<legal 0-3>
*/

#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_OFFSET                                       0x0000000000000000
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_LSB                                          2
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_MSB                                          4
#define RX_TRIG_INFO_TRIGGER_RESP_TYPE_MASK                                         0x000000000000001c


/* Description		RESERVED_0

			Reserved and unused by HW
			<legal 0>
*/

#define RX_TRIG_INFO_RESERVED_0_OFFSET                                              0x0000000000000000
#define RX_TRIG_INFO_RESERVED_0_LSB                                                 5
#define RX_TRIG_INFO_RESERVED_0_MSB                                                 31
#define RX_TRIG_INFO_RESERVED_0_MASK                                                0x00000000ffffffe0


/* Description		PPDU_DURATION

			11ax
			This field is valid only when rx_trig_frame is dot11ax_direct_trigger_frame
			 or dot11ax_wildcard_trigger_frame or dot11ax_usassoc_wildcard_trigger_frame
			
			
			The PPDU duration populated in trigger frame. This is the
			 duration that station is allowed to use to transmit the
			 packet
*/

#define RX_TRIG_INFO_PPDU_DURATION_OFFSET                                           0x0000000000000000
#define RX_TRIG_INFO_PPDU_DURATION_LSB                                              32
#define RX_TRIG_INFO_PPDU_DURATION_MSB                                              47
#define RX_TRIG_INFO_PPDU_DURATION_MASK                                             0x0000ffff00000000


/* Description		UNIQUE_DESTINATION_ID

			11ax
			This field is valid only when rx_trig_frame is dot11ax_direct_trigger_frame
			 or dot11ax_wildcard_trigger_frame or dot11ax_usassoc_wildcard_trigger_frame
			
			
			Unique destination identification number used by HWSCH to
			 compare with the station ID in the command
*/

#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_OFFSET                                   0x0000000000000000
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_LSB                                      48
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_MSB                                      63
#define RX_TRIG_INFO_UNIQUE_DESTINATION_ID_MASK                                     0xffff000000000000



#endif   // RX_TRIG_INFO

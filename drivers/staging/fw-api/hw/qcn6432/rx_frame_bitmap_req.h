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

#ifndef _RX_FRAME_BITMAP_REQ_H_
#define _RX_FRAME_BITMAP_REQ_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_FRAME_BITMAP_REQ 2

#define NUM_OF_QWORDS_RX_FRAME_BITMAP_REQ 1


struct rx_frame_bitmap_req {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t explicit_user_request                                   :  1, // [0:0]
                      user_request_type                                       :  1, // [1:1]
                      user_number                                             :  6, // [7:2]
                      sw_peer_id                                              : 16, // [23:8]
                      tid_specific_request                                    :  1, // [24:24]
                      requested_tid                                           :  4, // [28:25]
                      reserved_0                                              :  3; // [31:29]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0                                              :  3, // [31:29]
                      requested_tid                                           :  4, // [28:25]
                      tid_specific_request                                    :  1, // [24:24]
                      sw_peer_id                                              : 16, // [23:8]
                      user_number                                             :  6, // [7:2]
                      user_request_type                                       :  1, // [1:1]
                      explicit_user_request                                   :  1; // [0:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		EXPLICIT_USER_REQUEST

			Note: TXCPU is allowed to interleave requests of the two
			 different types
			
			Also, for either request, RXPCU shall clear the internal
			 flag that linked the bitmap to the just received frame 
			
			
			When set, TXPCU is asking for the bitmap for an explicit
			 user. This is typically only to be used after an MU OFDMA
			 or MU MIMO  reception. Note that this request can be used
			 to retrieve bitmaps that do not necessarily belong to the
			 just received PPDU, but might have been generated a while
			 ago.
			
			When not set, it is up to RXPCU to decide which bitmap it
			 wants to give to TXPCU based on what is available (and 
			has not been passed on the TXPCU in a previous request, 
			which might have included a request in the 'Explicit_user_request' 
			format). This type of request is typically (but not required
			 to be) used  in case of a non OFDMA reception, where a 
			BA needs to be send back as response.
			It is mode is typically (but not required to be) used by
			 TXPCU in case of sending a Multi STA BA
			Note that this request can only be used to retrieve bitmaps
			 that are generated as result of the just received PPDU, 
			and can not be used to retrieve bitmaps of earlier received
			 PPDUs.
			
			
			<legal all>
*/

#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_OFFSET                            0x0000000000000000
#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_LSB                               0
#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_MSB                               0
#define RX_FRAME_BITMAP_REQ_EXPLICIT_USER_REQUEST_MASK                              0x0000000000000001


/* Description		USER_REQUEST_TYPE

			Field only valid when Explicit_user_request is set
			
			<enum 0 bitmap_req_user_number_based> The request is based
			 on a user_number. This method is typically used in case
			 of SIFS response for Multi User BA
			
			<enum 1 bitmap_req_sw_peer_id_based> The request is based
			 on the sw_peer_id.  This method is typically used in the
			 response to response scenario where TXPCU got a new scheduling
			 command for the response to response part, and SW now explicitly
			 indicates for which STAs a BA shall be requested.
			<legal all>
*/

#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_OFFSET                                0x0000000000000000
#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_LSB                                   1
#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_MSB                                   1
#define RX_FRAME_BITMAP_REQ_USER_REQUEST_TYPE_MASK                                  0x0000000000000002


/* Description		USER_NUMBER

			Field only valid when Explicit_user_request is set 
			and User_request_type is set to bitmap_req_user_number_based
			
			
			The user number for which the bitmap is requested.
			<legal all>
*/

#define RX_FRAME_BITMAP_REQ_USER_NUMBER_OFFSET                                      0x0000000000000000
#define RX_FRAME_BITMAP_REQ_USER_NUMBER_LSB                                         2
#define RX_FRAME_BITMAP_REQ_USER_NUMBER_MSB                                         7
#define RX_FRAME_BITMAP_REQ_USER_NUMBER_MASK                                        0x00000000000000fc


/* Description		SW_PEER_ID

			Field only valid when Explicit_user_request is set 
			and User_request_type is set to bitmap_req_sw_peer_id_based
			
			
			The sw_peer_id for which the bitmap is requested. 
			<legal all>
*/

#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_OFFSET                                       0x0000000000000000
#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_LSB                                          8
#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_MSB                                          23
#define RX_FRAME_BITMAP_REQ_SW_PEER_ID_MASK                                         0x0000000000ffff00


/* Description		TID_SPECIFIC_REQUEST

			Field only valid when Explicit_user_request is set
			
			When set, the request is going out for a specific TID, indicated
			 in field TID
			
			When clear, it is up to RXPCU to determine in which order
			 it wants to return bitmaps to TXPCU. Note that these bitmaps
			 do need to all belong the the requested user, as Explicit_user_request
			 has also been set. 
			<legal all>
*/

#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_OFFSET                             0x0000000000000000
#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_LSB                                24
#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_MSB                                24
#define RX_FRAME_BITMAP_REQ_TID_SPECIFIC_REQUEST_MASK                               0x0000000001000000


/* Description		REQUESTED_TID

			Field only valid when Explicit_user_request is set 
			and User_request_type is set to bitmap_req_sw_peer_id_based
			
			and Tid_specific_request is set
			
			The TID for which a BA bitmap is requested
			<legal all>
*/

#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_OFFSET                                    0x0000000000000000
#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_LSB                                       25
#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_MSB                                       28
#define RX_FRAME_BITMAP_REQ_REQUESTED_TID_MASK                                      0x000000001e000000


/* Description		RESERVED_0

			<legal 0>
*/

#define RX_FRAME_BITMAP_REQ_RESERVED_0_OFFSET                                       0x0000000000000000
#define RX_FRAME_BITMAP_REQ_RESERVED_0_LSB                                          29
#define RX_FRAME_BITMAP_REQ_RESERVED_0_MSB                                          31
#define RX_FRAME_BITMAP_REQ_RESERVED_0_MASK                                         0x00000000e0000000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_OFFSET                                    0x0000000000000000
#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_LSB                                       32
#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_MSB                                       63
#define RX_FRAME_BITMAP_REQ_TLV64_PADDING_MASK                                      0xffffffff00000000



#endif   // RX_FRAME_BITMAP_REQ

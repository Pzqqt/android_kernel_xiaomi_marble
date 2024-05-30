
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

 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_ACK_OR_BA_H_
#define _TX_FES_STATUS_ACK_OR_BA_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_ACK_OR_BA 10

#define NUM_OF_QWORDS_TX_FES_STATUS_ACK_OR_BA 5


struct tx_fes_status_ack_or_ba {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ack_ba_status_type                                      :  1, // [0:0]
                      ba_type                                                 :  1, // [1:1]
                      ba_tid                                                  :  4, // [5:2]
                      unexpected_ack_or_ba                                    :  1, // [6:6]
                      response_timeout                                        :  1, // [7:7]
                      ack_frame_rssi                                          :  8, // [15:8]
                      ssn                                                     : 12, // [27:16]
                      reserved_0b                                             :  4; // [31:28]
             uint32_t sw_peer_id                                              : 16, // [15:0]
                      reserved_1a                                             : 16; // [31:16]
             uint32_t ba_bitmap_31_0                                          : 32; // [31:0]
             uint32_t ba_bitmap_63_32                                         : 32; // [31:0]
             uint32_t ba_bitmap_95_64                                         : 32; // [31:0]
             uint32_t ba_bitmap_127_96                                        : 32; // [31:0]
             uint32_t ba_bitmap_159_128                                       : 32; // [31:0]
             uint32_t ba_bitmap_191_160                                       : 32; // [31:0]
             uint32_t ba_bitmap_223_192                                       : 32; // [31:0]
             uint32_t ba_bitmap_255_224                                       : 32; // [31:0]
#else
             uint32_t reserved_0b                                             :  4, // [31:28]
                      ssn                                                     : 12, // [27:16]
                      ack_frame_rssi                                          :  8, // [15:8]
                      response_timeout                                        :  1, // [7:7]
                      unexpected_ack_or_ba                                    :  1, // [6:6]
                      ba_tid                                                  :  4, // [5:2]
                      ba_type                                                 :  1, // [1:1]
                      ack_ba_status_type                                      :  1; // [0:0]
             uint32_t reserved_1a                                             : 16, // [31:16]
                      sw_peer_id                                              : 16; // [15:0]
             uint32_t ba_bitmap_31_0                                          : 32; // [31:0]
             uint32_t ba_bitmap_63_32                                         : 32; // [31:0]
             uint32_t ba_bitmap_95_64                                         : 32; // [31:0]
             uint32_t ba_bitmap_127_96                                        : 32; // [31:0]
             uint32_t ba_bitmap_159_128                                       : 32; // [31:0]
             uint32_t ba_bitmap_191_160                                       : 32; // [31:0]
             uint32_t ba_bitmap_223_192                                       : 32; // [31:0]
             uint32_t ba_bitmap_255_224                                       : 32; // [31:0]
#endif
};


/* Description		ACK_BA_STATUS_TYPE

			Consumer: SW
			Producer: RXPCU
			
			<enum 0 Ack_type> This TLV represents an ACK reception.
			
			<enum 1 BA_type>  This TLV represents an BA reception.
			 <legal 0-1>
*/

#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_LSB                              0
#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_MSB                              0
#define TX_FES_STATUS_ACK_OR_BA_ACK_BA_STATUS_TYPE_MASK                             0x0000000000000001


/* Description		BA_TYPE

			Field only valid when  Ack_ba_status_type ==  BA_type
			
			<enum 0 BA_TYPE_ACK>
			<enum 1 BA_TYPE_bitmap>
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_OFFSET                                      0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_LSB                                         1
#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_MSB                                         1
#define TX_FES_STATUS_ACK_OR_BA_BA_TYPE_MASK                                        0x0000000000000002


/* Description		BA_TID

			Field only valid when  Ack_ba_status_type ==  BA_type
			
			The TID field copied from the BA frame
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_TID_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_BA_TID_LSB                                          2
#define TX_FES_STATUS_ACK_OR_BA_BA_TID_MSB                                          5
#define TX_FES_STATUS_ACK_OR_BA_BA_TID_MASK                                         0x000000000000003c


/* Description		UNEXPECTED_ACK_OR_BA

			Set when RXPCU received a BA for which there was no " RXPCU_USER_SETUP_EXT
			 TLV' received.
			This can happen when a BA for unexpected TID is received.
			
			
			This message enables SW to still pass this BA information
			 on to the right TQM queue.
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_LSB                            6
#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_MSB                            6
#define TX_FES_STATUS_ACK_OR_BA_UNEXPECTED_ACK_OR_BA_MASK                           0x0000000000000040


/* Description		RESPONSE_TIMEOUT

			When set, there was delay in RXPCU (likely due to AST fetch
			 delay) that resulted in TXPCU not being able to send the
			 RX_RESPONSE_REQUIRED_INFO TLV within a certain timeout 
			from the falling edge of the frame. This status TLV is still
			 generated but RXPCU will NOT have generated the RX_RESPONSE_REQUIRED
			 TLV.
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_OFFSET                             0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_LSB                                7
#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_MSB                                7
#define TX_FES_STATUS_ACK_OR_BA_RESPONSE_TIMEOUT_MASK                               0x0000000000000080


/* Description		ACK_FRAME_RSSI

			RSSI of the received ACK, BA or M-BA frame. 
			
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_LSB                                  8
#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_MSB                                  15
#define TX_FES_STATUS_ACK_OR_BA_ACK_FRAME_RSSI_MASK                                 0x000000000000ff00


/* Description		SSN

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Ack_ba_status_type indicating: 
			BA_type 
			
			The starting Sequence number of the (B)ACK bitmap <legal
			 all>
*/

#define TX_FES_STATUS_ACK_OR_BA_SSN_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_SSN_LSB                                             16
#define TX_FES_STATUS_ACK_OR_BA_SSN_MSB                                             27
#define TX_FES_STATUS_ACK_OR_BA_SSN_MASK                                            0x000000000fff0000


/* Description		RESERVED_0B

			<legal 0>
*/

#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_OFFSET                                  0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_LSB                                     28
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_MSB                                     31
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_0B_MASK                                    0x00000000f0000000


/* Description		SW_PEER_ID

			The sw_peer_id for which the bitmap is requested. 
			
			SW could use this info to link this TLV back to the right
			 TQM queue (if needed)
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_OFFSET                                   0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_LSB                                      32
#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_MSB                                      47
#define TX_FES_STATUS_ACK_OR_BA_SW_PEER_ID_MASK                                     0x0000ffff00000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_OFFSET                                  0x0000000000000000
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_LSB                                     48
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_MSB                                     63
#define TX_FES_STATUS_ACK_OR_BA_RESERVED_1A_MASK                                    0xffff000000000000


/* Description		BA_BITMAP_31_0

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Ack_ba_status_type indicating: 
			BA_type
			
			Ba_bitmap_31_0
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_OFFSET                               0x0000000000000008
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_LSB                                  0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_MSB                                  31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_31_0_MASK                                 0x00000000ffffffff


/* Description		BA_BITMAP_63_32

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Ack_ba_status_type indicating: 
			BA_type
			
			Ba_bitmap_63_32
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_OFFSET                              0x0000000000000008
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_LSB                                 32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_MSB                                 63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_63_32_MASK                                0xffffffff00000000


/* Description		BA_BITMAP_95_64

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Remove_acked_cmd_type  
			indicating:
			remove_Block_Acked_mpdus 
			
			Ba_bitmap_95_64
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_OFFSET                              0x0000000000000010
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_LSB                                 0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_MSB                                 31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_95_64_MASK                                0x00000000ffffffff


/* Description		BA_BITMAP_127_96

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Remove_acked_cmd_type  
			indicating:
			remove_Block_Acked_mpdus 
			
			Ba_bitmap_127_96
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_OFFSET                             0x0000000000000010
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_LSB                                32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_MSB                                63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_127_96_MASK                               0xffffffff00000000


/* Description		BA_BITMAP_159_128

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Remove_acked_cmd_type  
			indicating:
			remove_Block_Acked_mpdus 
			
			Ba_bitmap_159_128
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_LSB                               0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_MSB                               31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_159_128_MASK                              0x00000000ffffffff


/* Description		BA_BITMAP_191_160

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Remove_acked_cmd_type  
			indicating:
			remove_Block_Acked_mpdus 
			
			Ba_bitmap_191_160
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_LSB                               32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_MSB                               63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_191_160_MASK                              0xffffffff00000000


/* Description		BA_BITMAP_223_192

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Remove_acked_cmd_type  
			indicating:
			remove_Block_Acked_mpdus 
			
			Ba_bitmap_223_192
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_LSB                               0
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_MSB                               31
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_223_192_MASK                              0x00000000ffffffff


/* Description		BA_BITMAP_255_224

			Consumer: TQM/FW
			Producer: SW/RXPCU
			
			Field only valid in case of the  Remove_acked_cmd_type  
			indicating:
			remove_Block_Acked_mpdus 
			
			Ba_bitmap_255_224
			<legal all>
*/

#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_LSB                               32
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_MSB                               63
#define TX_FES_STATUS_ACK_OR_BA_BA_BITMAP_255_224_MASK                              0xffffffff00000000



#endif   // TX_FES_STATUS_ACK_OR_BA

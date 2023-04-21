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

#ifndef _RX_FRAME_BITMAP_ACK_H_
#define _RX_FRAME_BITMAP_ACK_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_FRAME_BITMAP_ACK 14

#define NUM_OF_QWORDS_RX_FRAME_BITMAP_ACK 7


struct rx_frame_bitmap_ack {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t no_bitmap_available                                     :  1, // [0:0]
                      explicit_ack                                            :  1, // [1:1]
                      explict_ack_type                                        :  3, // [4:2]
                      ba_bitmap_size                                          :  2, // [6:5]
                      reserved_0a                                             :  3, // [9:7]
                      ba_tid                                                  :  4, // [13:10]
                      sta_full_aid                                            : 13, // [26:14]
                      reserved_0b                                             :  5; // [31:27]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr1_47_32                                             : 16, // [15:0]
                      addr2_15_0                                              : 16; // [31:16]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t ba_ts_ctrl                                              : 16, // [15:0]
                      ba_ts_seq                                               : 16; // [31:16]
             uint32_t ba_ts_bitmap_31_0                                       : 32; // [31:0]
             uint32_t ba_ts_bitmap_63_32                                      : 32; // [31:0]
             uint32_t ba_ts_bitmap_95_64                                      : 32; // [31:0]
             uint32_t ba_ts_bitmap_127_96                                     : 32; // [31:0]
             uint32_t ba_ts_bitmap_159_128                                    : 32; // [31:0]
             uint32_t ba_ts_bitmap_191_160                                    : 32; // [31:0]
             uint32_t ba_ts_bitmap_223_192                                    : 32; // [31:0]
             uint32_t ba_ts_bitmap_255_224                                    : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0b                                             :  5, // [31:27]
                      sta_full_aid                                            : 13, // [26:14]
                      ba_tid                                                  :  4, // [13:10]
                      reserved_0a                                             :  3, // [9:7]
                      ba_bitmap_size                                          :  2, // [6:5]
                      explict_ack_type                                        :  3, // [4:2]
                      explicit_ack                                            :  1, // [1:1]
                      no_bitmap_available                                     :  1; // [0:0]
             uint32_t addr1_31_0                                              : 32; // [31:0]
             uint32_t addr2_15_0                                              : 16, // [31:16]
                      addr1_47_32                                             : 16; // [15:0]
             uint32_t addr2_47_16                                             : 32; // [31:0]
             uint32_t ba_ts_seq                                               : 16, // [31:16]
                      ba_ts_ctrl                                              : 16; // [15:0]
             uint32_t ba_ts_bitmap_31_0                                       : 32; // [31:0]
             uint32_t ba_ts_bitmap_63_32                                      : 32; // [31:0]
             uint32_t ba_ts_bitmap_95_64                                      : 32; // [31:0]
             uint32_t ba_ts_bitmap_127_96                                     : 32; // [31:0]
             uint32_t ba_ts_bitmap_159_128                                    : 32; // [31:0]
             uint32_t ba_ts_bitmap_191_160                                    : 32; // [31:0]
             uint32_t ba_ts_bitmap_223_192                                    : 32; // [31:0]
             uint32_t ba_ts_bitmap_255_224                                    : 32; // [31:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		NO_BITMAP_AVAILABLE

			When set, RXPCU does not have any info available for the
			 requested user. 
			
			RXPCU will set the TA/RA, addresses with the devices OWN
			 address.
			All other fields are set to 0
			
			TXPCU will just blindly follow RXPCUs info.
			(only for status reporting is TXPCU using this).
			
			Note that this field and field "Explicit_ack" can not be
			 simultaneously set.
			<legal all>
*/

#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_OFFSET                              0x0000000000000000
#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_LSB                                 0
#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_MSB                                 0
#define RX_FRAME_BITMAP_ACK_NO_BITMAP_AVAILABLE_MASK                                0x0000000000000001


/* Description		EXPLICIT_ACK

			When set, no BA is needed for this STA. Instead just a single
			 ACK indication 
			
			Note that this field and field "No_bitmap_available" can
			 not be simultaneously set.
			
			Also note that RXPCU might not know if the response that
			 TXPCU is generating is a single ACK or M(sta) BA.
			For that reason, RXPCU shall also properly fill in all the
			 BA related fields. TXPCU will based on the explicit ack
			 type and in case of BA type response, blindely copy the
			 required BA related fields and not change their contents:
			
			The related fields are:
			Ba_tid 
			ba_ts_ctrl 
			ba_ts_seq
			ba_ts_bitmap_...
			
			<legal all>
*/

#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_OFFSET                                     0x0000000000000000
#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_LSB                                        1
#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_MSB                                        1
#define RX_FRAME_BITMAP_ACK_EXPLICIT_ACK_MASK                                       0x0000000000000002


/* Description		EXPLICT_ACK_TYPE

			Field only valid when Explicit_ack is set
			
			Note that TXPCU only needs to evaluate this field in case
			 of generating a multi (STA) BA
			
			<enum 0 ack_for_single_data_frame> set when only a single
			 data frame was received that indicated explicitly a 'normal' 
			ack (no BA) to be sent.
			<enum 1 ack_for_management> set when a management frame 
			was received
			<enum 2 ack_for_PSPOLL> set when a PS_POLL frame was received
			
			<enum 3 ack_for_assoc_request> set when an association request
			 was received from an unassociated STA.
			<enum 4 ack_for_all_frames> set when RXPCU determined that
			 all frames have been properly received.
			<legal 0-4>
*/

#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_OFFSET                                 0x0000000000000000
#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_LSB                                    2
#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_MSB                                    4
#define RX_FRAME_BITMAP_ACK_EXPLICT_ACK_TYPE_MASK                                   0x000000000000001c


/* Description		BA_BITMAP_SIZE

			Field not valid when "No_bitmap_available" or "Explicit_ack" 
			is set.
			
			
			<enum 0 BA_bitmap_32 > Bitmap size set to window of 32
			<enum 1 BA_bitmap_64 > Bitmap size set to window of 64
			<enum 2 BA_bitmap_128 > Bitmap size set to window of 128
			
			<enum 3 BA_bitmap_256 > Bitmap size set to window of 256
			
			
			<legal 0-3>
*/

#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_OFFSET                                   0x0000000000000000
#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_LSB                                      5
#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_MSB                                      6
#define RX_FRAME_BITMAP_ACK_BA_BITMAP_SIZE_MASK                                     0x0000000000000060


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_FRAME_BITMAP_ACK_RESERVED_0A_OFFSET                                      0x0000000000000000
#define RX_FRAME_BITMAP_ACK_RESERVED_0A_LSB                                         7
#define RX_FRAME_BITMAP_ACK_RESERVED_0A_MSB                                         9
#define RX_FRAME_BITMAP_ACK_RESERVED_0A_MASK                                        0x0000000000000380


/* Description		BA_TID

			The tid for the BA
*/

#define RX_FRAME_BITMAP_ACK_BA_TID_OFFSET                                           0x0000000000000000
#define RX_FRAME_BITMAP_ACK_BA_TID_LSB                                              10
#define RX_FRAME_BITMAP_ACK_BA_TID_MSB                                              13
#define RX_FRAME_BITMAP_ACK_BA_TID_MASK                                             0x0000000000003c00


/* Description		STA_FULL_AID

			The full AID of this station. 
*/

#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_OFFSET                                     0x0000000000000000
#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_LSB                                        14
#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_MSB                                        26
#define RX_FRAME_BITMAP_ACK_STA_FULL_AID_MASK                                       0x0000000007ffc000


/* Description		RESERVED_0B

			<legal 0>
*/

#define RX_FRAME_BITMAP_ACK_RESERVED_0B_OFFSET                                      0x0000000000000000
#define RX_FRAME_BITMAP_ACK_RESERVED_0B_LSB                                         27
#define RX_FRAME_BITMAP_ACK_RESERVED_0B_MSB                                         31
#define RX_FRAME_BITMAP_ACK_RESERVED_0B_MASK                                        0x00000000f8000000


/* Description		ADDR1_31_0

			lower 32 bits of addr1 of the received frame
*/

#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_OFFSET                                       0x0000000000000000
#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_LSB                                          32
#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_MSB                                          63
#define RX_FRAME_BITMAP_ACK_ADDR1_31_0_MASK                                         0xffffffff00000000


/* Description		ADDR1_47_32

			upper 16 bits of addr1 of the received frame
*/

#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_OFFSET                                      0x0000000000000008
#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_LSB                                         0
#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_MSB                                         15
#define RX_FRAME_BITMAP_ACK_ADDR1_47_32_MASK                                        0x000000000000ffff


/* Description		ADDR2_15_0

			lower 16 bits of addr2 of the received frame
*/

#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_OFFSET                                       0x0000000000000008
#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_LSB                                          16
#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_MSB                                          31
#define RX_FRAME_BITMAP_ACK_ADDR2_15_0_MASK                                         0x00000000ffff0000


/* Description		ADDR2_47_16

			upper 32 bits of addr2 of the received frame
*/

#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_OFFSET                                      0x0000000000000008
#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_LSB                                         32
#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_MSB                                         63
#define RX_FRAME_BITMAP_ACK_ADDR2_47_16_MASK                                        0xffffffff00000000


/* Description		BA_TS_CTRL

			Transmit BA control
			RXPCU assumes the C-BA format, NOT M-BA format.
			In case TXPCU is responding with M-BA, TXPCU will ignore
			 this field. TXPCU will generate it 
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_OFFSET                                       0x0000000000000010
#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_LSB                                          0
#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_MSB                                          15
#define RX_FRAME_BITMAP_ACK_BA_TS_CTRL_MASK                                         0x000000000000ffff


/* Description		BA_TS_SEQ

			Transmit BA sequence number. 
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_OFFSET                                        0x0000000000000010
#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_LSB                                           16
#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_MSB                                           31
#define RX_FRAME_BITMAP_ACK_BA_TS_SEQ_MASK                                          0x00000000ffff0000


/* Description		BA_TS_BITMAP_31_0

			Transmit BA bitmap[31:0]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_OFFSET                                0x0000000000000010
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_LSB                                   32
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_MSB                                   63
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_31_0_MASK                                  0xffffffff00000000


/* Description		BA_TS_BITMAP_63_32

			Transmit BA bitmap[63:32]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_OFFSET                               0x0000000000000018
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_LSB                                  0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_MSB                                  31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_63_32_MASK                                 0x00000000ffffffff


/* Description		BA_TS_BITMAP_95_64

			Transmit BA bitmap[95:64]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_OFFSET                               0x0000000000000018
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_LSB                                  32
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_MSB                                  63
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_95_64_MASK                                 0xffffffff00000000


/* Description		BA_TS_BITMAP_127_96

			Transmit BA bitmap[127:96]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_OFFSET                              0x0000000000000020
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_LSB                                 0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_MSB                                 31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_127_96_MASK                                0x00000000ffffffff


/* Description		BA_TS_BITMAP_159_128

			Transmit BA bitmap[159:128]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_OFFSET                             0x0000000000000020
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_LSB                                32
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_MSB                                63
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_159_128_MASK                               0xffffffff00000000


/* Description		BA_TS_BITMAP_191_160

			Transmit BA bitmap[191:160]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_OFFSET                             0x0000000000000028
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_LSB                                0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_MSB                                31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_191_160_MASK                               0x00000000ffffffff


/* Description		BA_TS_BITMAP_223_192

			Transmit BA bitmap[223:192]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_OFFSET                             0x0000000000000028
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_LSB                                32
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_MSB                                63
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_223_192_MASK                               0xffffffff00000000


/* Description		BA_TS_BITMAP_255_224

			Transmit BA bitmap[255:224]
*/

#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_OFFSET                             0x0000000000000030
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_LSB                                0
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_MSB                                31
#define RX_FRAME_BITMAP_ACK_BA_TS_BITMAP_255_224_MASK                               0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RX_FRAME_BITMAP_ACK_TLV64_PADDING_OFFSET                                    0x0000000000000030
#define RX_FRAME_BITMAP_ACK_TLV64_PADDING_LSB                                       32
#define RX_FRAME_BITMAP_ACK_TLV64_PADDING_MSB                                       63
#define RX_FRAME_BITMAP_ACK_TLV64_PADDING_MASK                                      0xffffffff00000000



#endif   // RX_FRAME_BITMAP_ACK

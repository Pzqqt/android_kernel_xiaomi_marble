
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

 
 
 
 
 
 
 


#ifndef _TX_MPDU_START_H_
#define _TX_MPDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_MPDU_START 10

#define NUM_OF_QWORDS_TX_MPDU_START 5


struct tx_mpdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mpdu_length                                             : 14, // [13:0]
                      frame_not_from_tqm                                      :  1, // [14:14]
                      vht_control_present                                     :  1, // [15:15]
                      mpdu_header_length                                      :  8, // [23:16]
                      retry_count                                             :  7, // [30:24]
                      wds                                                     :  1; // [31:31]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_47_32                                                : 16, // [15:0]
                      mpdu_sequence_number                                    : 12, // [27:16]
                      raw_already_encrypted                                   :  1, // [28:28]
                      frame_type                                              :  2, // [30:29]
                      txdma_dropped_mpdu_warning                              :  1; // [31:31]
             uint32_t iv_byte_0                                               :  8, // [7:0]
                      iv_byte_1                                               :  8, // [15:8]
                      iv_byte_2                                               :  8, // [23:16]
                      iv_byte_3                                               :  8; // [31:24]
             uint32_t iv_byte_4                                               :  8, // [7:0]
                      iv_byte_5                                               :  8, // [15:8]
                      iv_byte_6                                               :  8, // [23:16]
                      iv_byte_7                                               :  8; // [31:24]
             uint32_t iv_byte_8                                               :  8, // [7:0]
                      iv_byte_9                                               :  8, // [15:8]
                      iv_byte_10                                              :  8, // [23:16]
                      iv_byte_11                                              :  8; // [31:24]
             uint32_t iv_byte_12                                              :  8, // [7:0]
                      iv_byte_13                                              :  8, // [15:8]
                      iv_byte_14                                              :  8, // [23:16]
                      iv_byte_15                                              :  8; // [31:24]
             uint32_t iv_byte_16                                              :  8, // [7:0]
                      iv_byte_17                                              :  8, // [15:8]
                      iv_len                                                  :  5, // [20:16]
                      icv_len                                                 :  5, // [25:21]
                      vht_control_offset                                      :  6; // [31:26]
             uint32_t mpdu_type                                               :  1, // [0:0]
                      transmit_bw_restriction                                 :  1, // [1:1]
                      allowed_transmit_bw                                     :  4, // [5:2]
                      tx_notify_frame                                         :  3, // [8:6]
                      reserved_8a                                             : 23; // [31:9]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t wds                                                     :  1, // [31:31]
                      retry_count                                             :  7, // [30:24]
                      mpdu_header_length                                      :  8, // [23:16]
                      vht_control_present                                     :  1, // [15:15]
                      frame_not_from_tqm                                      :  1, // [14:14]
                      mpdu_length                                             : 14; // [13:0]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t txdma_dropped_mpdu_warning                              :  1, // [31:31]
                      frame_type                                              :  2, // [30:29]
                      raw_already_encrypted                                   :  1, // [28:28]
                      mpdu_sequence_number                                    : 12, // [27:16]
                      pn_47_32                                                : 16; // [15:0]
             uint32_t iv_byte_3                                               :  8, // [31:24]
                      iv_byte_2                                               :  8, // [23:16]
                      iv_byte_1                                               :  8, // [15:8]
                      iv_byte_0                                               :  8; // [7:0]
             uint32_t iv_byte_7                                               :  8, // [31:24]
                      iv_byte_6                                               :  8, // [23:16]
                      iv_byte_5                                               :  8, // [15:8]
                      iv_byte_4                                               :  8; // [7:0]
             uint32_t iv_byte_11                                              :  8, // [31:24]
                      iv_byte_10                                              :  8, // [23:16]
                      iv_byte_9                                               :  8, // [15:8]
                      iv_byte_8                                               :  8; // [7:0]
             uint32_t iv_byte_15                                              :  8, // [31:24]
                      iv_byte_14                                              :  8, // [23:16]
                      iv_byte_13                                              :  8, // [15:8]
                      iv_byte_12                                              :  8; // [7:0]
             uint32_t vht_control_offset                                      :  6, // [31:26]
                      icv_len                                                 :  5, // [25:21]
                      iv_len                                                  :  5, // [20:16]
                      iv_byte_17                                              :  8, // [15:8]
                      iv_byte_16                                              :  8; // [7:0]
             uint32_t reserved_8a                                             : 23, // [31:9]
                      tx_notify_frame                                         :  3, // [8:6]
                      allowed_transmit_bw                                     :  4, // [5:2]
                      transmit_bw_restriction                                 :  1, // [1:1]
                      mpdu_type                                               :  1; // [0:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		MPDU_LENGTH

			Consumer: TXOLE/CRYPTO/TXPCU
			Producer: TXDMA
			
			Expected Length of the entire MPDU, which includes all MSDUs
			 within the MPDU and all OLE and Crypto processing. This
			 length includes the FCS field.
*/

#define TX_MPDU_START_MPDU_LENGTH_OFFSET                                            0x0000000000000000
#define TX_MPDU_START_MPDU_LENGTH_LSB                                               0
#define TX_MPDU_START_MPDU_LENGTH_MSB                                               13
#define TX_MPDU_START_MPDU_LENGTH_MASK                                              0x0000000000003fff


/* Description		FRAME_NOT_FROM_TQM

			When set, TXPCU shall not take this frame into account for
			 indicating to TQM how many frames from it's queue got transmitted.
			
			
			TXDMA gets this field from the TX_MSDU_DETAILS STRUCT (of
			 the first MSDU in the MPDU) in the MSDU link descriptor.
			
			
			SW sets this bit (in TX_MSDU_DETAILS STRUCT) when it generates
			 a frame outside of the TQM path and that frame can be intermingled
			 with the other frames from the TQM. For example a trigger
			 frame embedded or put in front of data frames from TQM 
			within the same A-MPDU. For this SW generated frame, TXPCU
			 shall not include this frame in the transmit frame count
			 that is reported to TQM as that would result in incorrect
			 reporting to TQM.
			
			<legal all>
*/

#define TX_MPDU_START_FRAME_NOT_FROM_TQM_OFFSET                                     0x0000000000000000
#define TX_MPDU_START_FRAME_NOT_FROM_TQM_LSB                                        14
#define TX_MPDU_START_FRAME_NOT_FROM_TQM_MSB                                        14
#define TX_MPDU_START_FRAME_NOT_FROM_TQM_MASK                                       0x0000000000004000


/* Description		VHT_CONTROL_PRESENT

			TXOLE sets this bit when it added 4 placeholder bytes for
			 VHT-CONTROL field in the MPDU header.
			
			For RAW frames, OLE will set this bit and compute  vht_control_offset
			 when the order bit and QoS bit in frame_control field are
			 set to 1. For RAW management frame, this bit will be set
			 if order bit is set to 1.
			
			Used by TXPCU, to find out if it needs to overwrite the 
			HE-CONTROL field.
			<legal all>
*/

#define TX_MPDU_START_VHT_CONTROL_PRESENT_OFFSET                                    0x0000000000000000
#define TX_MPDU_START_VHT_CONTROL_PRESENT_LSB                                       15
#define TX_MPDU_START_VHT_CONTROL_PRESENT_MSB                                       15
#define TX_MPDU_START_VHT_CONTROL_PRESENT_MASK                                      0x0000000000008000


/* Description		MPDU_HEADER_LENGTH

			This field is filled in by the OLE
			Used by PCU, This prevents PCU from having to do this again
			 (in the same way))
*/

#define TX_MPDU_START_MPDU_HEADER_LENGTH_OFFSET                                     0x0000000000000000
#define TX_MPDU_START_MPDU_HEADER_LENGTH_LSB                                        16
#define TX_MPDU_START_MPDU_HEADER_LENGTH_MSB                                        23
#define TX_MPDU_START_MPDU_HEADER_LENGTH_MASK                                       0x0000000000ff0000


/* Description		RETRY_COUNT

			Consumer: TXOLE/TXPCU
			Producer: TXDMA
			
			The number of times the frame is transmitted
			<legal all>
*/

#define TX_MPDU_START_RETRY_COUNT_OFFSET                                            0x0000000000000000
#define TX_MPDU_START_RETRY_COUNT_LSB                                               24
#define TX_MPDU_START_RETRY_COUNT_MSB                                               30
#define TX_MPDU_START_RETRY_COUNT_MASK                                              0x000000007f000000


/* Description		WDS

			If set the current packet is 4-address frame.  
			
			Required because an aggregate can include some frames with
			 3 address format and other frames with 4 address format. 
			 Used by the OLE during encapsulation.  
			
			TXDMA sets this when wds in the extension descriptor is 
			set.
			
			If no extension descriptor is used for this MPDU, TXDMA 
			gets the setting for this bit from a control register in
			 TXDMA
			<legal all>
*/

#define TX_MPDU_START_WDS_OFFSET                                                    0x0000000000000000
#define TX_MPDU_START_WDS_LSB                                                       31
#define TX_MPDU_START_WDS_MSB                                                       31
#define TX_MPDU_START_WDS_MASK                                                      0x0000000080000000


/* Description		PN_31_0

			Consumer: TXOLE
			Producer: TXDMA
			
			Bits 31 - 0 for the Packet Number used by encryption
			<legal all>
*/

#define TX_MPDU_START_PN_31_0_OFFSET                                                0x0000000000000000
#define TX_MPDU_START_PN_31_0_LSB                                                   32
#define TX_MPDU_START_PN_31_0_MSB                                                   63
#define TX_MPDU_START_PN_31_0_MASK                                                  0xffffffff00000000


/* Description		PN_47_32

			Consumer: TXOLE
			Producer: TXDMA
			
			Bits 47 - 32 for the Packet Number used by encryption
			<legal all>
*/

#define TX_MPDU_START_PN_47_32_OFFSET                                               0x0000000000000008
#define TX_MPDU_START_PN_47_32_LSB                                                  0
#define TX_MPDU_START_PN_47_32_MSB                                                  15
#define TX_MPDU_START_PN_47_32_MASK                                                 0x000000000000ffff


/* Description		MPDU_SEQUENCE_NUMBER

			Consumer: TXOLE
			Producer: TXDMA
			
			Sequence number assigned to this MPDU
			<legal all>
*/

#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_OFFSET                                   0x0000000000000008
#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_LSB                                      16
#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_MSB                                      27
#define TX_MPDU_START_MPDU_SEQUENCE_NUMBER_MASK                                     0x000000000fff0000


/* Description		RAW_ALREADY_ENCRYPTED

			Consumer: CRYPTO
			Producer: TXDMA
			
			If set it indicates that the RAW MPDU has already been encrypted
			 and does not require HW encryption.  If clear and if the
			 frame control indicates that this is a "protected" MPDU
			 and the peer key type indicates a cipher type then the 
			HW is expected to encrypt this packet.
			<legal all> 
*/

#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_OFFSET                                  0x0000000000000008
#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_LSB                                     28
#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_MSB                                     28
#define TX_MPDU_START_RAW_ALREADY_ENCRYPTED_MASK                                    0x0000000010000000


/* Description		FRAME_TYPE

			Consumer: TXMON
			Producer: TXOLE
			
			802.11 frame type field
			
			TXDMA fills this as zero and TXOLE overwrites it.
			
			<legal all>
*/

#define TX_MPDU_START_FRAME_TYPE_OFFSET                                             0x0000000000000008
#define TX_MPDU_START_FRAME_TYPE_LSB                                                29
#define TX_MPDU_START_FRAME_TYPE_MSB                                                30
#define TX_MPDU_START_FRAME_TYPE_MASK                                               0x0000000060000000


/* Description		TXDMA_DROPPED_MPDU_WARNING

			Consumer: FW
			Producer: TXDMA
			
			Indication to TXPCU to indicate to FW a warning that Tx 
			DMA has dropped MPDUs due to SFM FIFO full condition
			<legal all> 
*/

#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_OFFSET                             0x0000000000000008
#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_LSB                                31
#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_MSB                                31
#define TX_MPDU_START_TXDMA_DROPPED_MPDU_WARNING_MASK                               0x0000000080000000


/* Description		IV_BYTE_0

			Byte 0 of the IV field of the MPDU
			Based on the Encryption type the iv_byte_0 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			 
*/

#define TX_MPDU_START_IV_BYTE_0_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_0_LSB                                                 32
#define TX_MPDU_START_IV_BYTE_0_MSB                                                 39
#define TX_MPDU_START_IV_BYTE_0_MASK                                                0x000000ff00000000


/* Description		IV_BYTE_1

			Byte 1 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_1 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_1_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_1_LSB                                                 40
#define TX_MPDU_START_IV_BYTE_1_MSB                                                 47
#define TX_MPDU_START_IV_BYTE_1_MASK                                                0x0000ff0000000000


/* Description		IV_BYTE_2

			Byte 2 of the IV field of the MDPU 
			Based on the Encryption type the iv_byte_2 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_2_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_2_LSB                                                 48
#define TX_MPDU_START_IV_BYTE_2_MSB                                                 55
#define TX_MPDU_START_IV_BYTE_2_MASK                                                0x00ff000000000000


/* Description		IV_BYTE_3

			Byte 3 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_3 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_3_OFFSET                                              0x0000000000000008
#define TX_MPDU_START_IV_BYTE_3_LSB                                                 56
#define TX_MPDU_START_IV_BYTE_3_MSB                                                 63
#define TX_MPDU_START_IV_BYTE_3_MASK                                                0xff00000000000000


/* Description		IV_BYTE_4

			Byte 4 of the IV field of the MPDU
			Based on the Encryption type the iv_byte_4 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_4_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_4_LSB                                                 0
#define TX_MPDU_START_IV_BYTE_4_MSB                                                 7
#define TX_MPDU_START_IV_BYTE_4_MASK                                                0x00000000000000ff


/* Description		IV_BYTE_5

			Byte 5 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_5 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_5_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_5_LSB                                                 8
#define TX_MPDU_START_IV_BYTE_5_MSB                                                 15
#define TX_MPDU_START_IV_BYTE_5_MASK                                                0x000000000000ff00


/* Description		IV_BYTE_6

			Byte 6 of the IV field of the MDPU 
			Based on the Encryption type the iv_byte_6 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_6_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_6_LSB                                                 16
#define TX_MPDU_START_IV_BYTE_6_MSB                                                 23
#define TX_MPDU_START_IV_BYTE_6_MASK                                                0x0000000000ff0000


/* Description		IV_BYTE_7

			Byte 7 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_7 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_7_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_7_LSB                                                 24
#define TX_MPDU_START_IV_BYTE_7_MSB                                                 31
#define TX_MPDU_START_IV_BYTE_7_MASK                                                0x00000000ff000000


/* Description		IV_BYTE_8

			Byte 8 of the IV field of the MPDU
			Based on the Encryption type the iv_byte_8 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_8_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_8_LSB                                                 32
#define TX_MPDU_START_IV_BYTE_8_MSB                                                 39
#define TX_MPDU_START_IV_BYTE_8_MASK                                                0x000000ff00000000


/* Description		IV_BYTE_9

			Byte 9 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_9 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_9_OFFSET                                              0x0000000000000010
#define TX_MPDU_START_IV_BYTE_9_LSB                                                 40
#define TX_MPDU_START_IV_BYTE_9_MSB                                                 47
#define TX_MPDU_START_IV_BYTE_9_MASK                                                0x0000ff0000000000


/* Description		IV_BYTE_10

			Byte 10 of the IV field of the MDPU 
			Based on the Encryption type the iv_byte_10 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_10_OFFSET                                             0x0000000000000010
#define TX_MPDU_START_IV_BYTE_10_LSB                                                48
#define TX_MPDU_START_IV_BYTE_10_MSB                                                55
#define TX_MPDU_START_IV_BYTE_10_MASK                                               0x00ff000000000000


/* Description		IV_BYTE_11

			Byte 11 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_11 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_11_OFFSET                                             0x0000000000000010
#define TX_MPDU_START_IV_BYTE_11_LSB                                                56
#define TX_MPDU_START_IV_BYTE_11_MSB                                                63
#define TX_MPDU_START_IV_BYTE_11_MASK                                               0xff00000000000000


/* Description		IV_BYTE_12

			Byte 8 of the IV field of the MPDU
			Based on the Encryption type the iv_byte_12 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_12_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_12_LSB                                                0
#define TX_MPDU_START_IV_BYTE_12_MSB                                                7
#define TX_MPDU_START_IV_BYTE_12_MASK                                               0x00000000000000ff


/* Description		IV_BYTE_13

			Byte 9 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_13 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_13_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_13_LSB                                                8
#define TX_MPDU_START_IV_BYTE_13_MSB                                                15
#define TX_MPDU_START_IV_BYTE_13_MASK                                               0x000000000000ff00


/* Description		IV_BYTE_14

			Byte 10 of the IV field of the MDPU 
			Based on the Encryption type the iv_byte_14 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_14_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_14_LSB                                                16
#define TX_MPDU_START_IV_BYTE_14_MSB                                                23
#define TX_MPDU_START_IV_BYTE_14_MASK                                               0x0000000000ff0000


/* Description		IV_BYTE_15

			Byte 11 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_15 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_15_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_15_LSB                                                24
#define TX_MPDU_START_IV_BYTE_15_MSB                                                31
#define TX_MPDU_START_IV_BYTE_15_MASK                                               0x00000000ff000000


/* Description		IV_BYTE_16

			Byte 8 of the IV field of the MPDU
			Based on the Encryption type the iv_byte_16 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_16_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_16_LSB                                                32
#define TX_MPDU_START_IV_BYTE_16_MSB                                                39
#define TX_MPDU_START_IV_BYTE_16_MASK                                               0x000000ff00000000


/* Description		IV_BYTE_17

			Byte 9 of the IV field of the MPDU 
			Based on the Encryption type the iv_byte_17 takes the appropriate
			 meaning. For IV formats,  refer to the crypto MLDR document
			
*/

#define TX_MPDU_START_IV_BYTE_17_OFFSET                                             0x0000000000000018
#define TX_MPDU_START_IV_BYTE_17_LSB                                                40
#define TX_MPDU_START_IV_BYTE_17_MSB                                                47
#define TX_MPDU_START_IV_BYTE_17_MASK                                               0x0000ff0000000000


/* Description		IV_LEN

			Length of the IV field generated by Tx OLE
*/

#define TX_MPDU_START_IV_LEN_OFFSET                                                 0x0000000000000018
#define TX_MPDU_START_IV_LEN_LSB                                                    48
#define TX_MPDU_START_IV_LEN_MSB                                                    52
#define TX_MPDU_START_IV_LEN_MASK                                                   0x001f000000000000


/* Description		ICV_LEN

			Length of the ICV field generated by Tx OLE. OLE will insert
			 zeros in the ICV field when it pushes a frame
*/

#define TX_MPDU_START_ICV_LEN_OFFSET                                                0x0000000000000018
#define TX_MPDU_START_ICV_LEN_LSB                                                   53
#define TX_MPDU_START_ICV_LEN_MSB                                                   57
#define TX_MPDU_START_ICV_LEN_MASK                                                  0x03e0000000000000


/* Description		VHT_CONTROL_OFFSET

			Field only valid when vht_control_present is set.
			
			Field filled in by TXOLE, used by TXPCU
			
			The starting byte number of the VHT control field in the
			 header
			<legal all>
*/

#define TX_MPDU_START_VHT_CONTROL_OFFSET_OFFSET                                     0x0000000000000018
#define TX_MPDU_START_VHT_CONTROL_OFFSET_LSB                                        58
#define TX_MPDU_START_VHT_CONTROL_OFFSET_MSB                                        63
#define TX_MPDU_START_VHT_CONTROL_OFFSET_MASK                                       0xfc00000000000000


/* Description		MPDU_TYPE

			Indicates the type of MPDU that OLE will generate:
			
			<enum 0    mpdu_type_basic> This MPDU is not in the A-MSDU
			 format (meaning there is no A-MSDU delimeter present) if
			 there is only 1 MSDU in the MPDU. When there are multiple
			 MSDUs in the MPDU, there is no choice, and the MSDUs within
			 the MPDU shall all have A-MSDU delimiters in front of them.
			
			<enum 1    mpdu_type_amsdu> The MSDUs within the MPDU will
			 all have to be in the A-MSDU format, even if there is just
			 a single MSDU embedded in the MPDU. In other words, there
			 is always an A-MSDU delimiter in front of the MSDU(s) in
			 the MPDU.
			This is not supported in Hastings80 and HastingsPrime.
			<legal all>
*/

#define TX_MPDU_START_MPDU_TYPE_OFFSET                                              0x0000000000000020
#define TX_MPDU_START_MPDU_TYPE_LSB                                                 0
#define TX_MPDU_START_MPDU_TYPE_MSB                                                 0
#define TX_MPDU_START_MPDU_TYPE_MASK                                                0x0000000000000001


/* Description		TRANSMIT_BW_RESTRICTION

			Consumer: TXPCU
			Producer: TXDMA
			
			1'b0: This is a normal frame and there are no restrictions
			 on the BW that this frame can be transmitted on.
			
			1'b1: This MPDU is only allowed to be transmitted at certain
			 BWs. The one and only allowed BW is indicated in field 
			allowed_transmit_bw
			When TXPCU has made a BW selection and then encounters this
			 frame, the frame will be dropped and TXPCU will continue
			 transmitting the next frame (assuming there is no BW restriction
			 on that one)
			<legal all>
*/

#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_OFFSET                                0x0000000000000020
#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_LSB                                   1
#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_MSB                                   1
#define TX_MPDU_START_TRANSMIT_BW_RESTRICTION_MASK                                  0x0000000000000002


/* Description		ALLOWED_TRANSMIT_BW

			Consumer: TXPCU
			Producer: TXDMA
			
			Field only valid when transmit_bw_restriction is set
			
			TXDMA gets this from the three or four upper bits of the
			 "Sw_buffer_cookie" field from the TX_MPDU_DETAILS STRUCT
			
			
			In case of NON punctured transmission:
			allowed_transmit_bw[2:0] = 3'b000: 20 MHz TX only
			allowed_transmit_bw[2:0] = 3'b001: 40 MHz TX only
			allowed_transmit_bw[2:0] = 3'b010: 80 MHz TX only
			allowed_transmit_bw[2:0] = 3'b011: 160 MHz TX only
			allowed_transmit_bw[2:0] = 3'b100: 240 MHz TX only
			allowed_transmit_bw[2:0] = 3'b101: 320 MHz TX only
			allowed_transmit_bw[2:1] = 2'b11: reserved
			
			In case of punctured transmission:
			allowed_transmit_bw[3:0] = 4'b0000: pattern 0 only
			allowed_transmit_bw[3:0] = 4'b0001: pattern 1 only
			allowed_transmit_bw[3:0] = 4'b0010: pattern 2 only
			allowed_transmit_bw[3:0] = 4'b0011: pattern 3 only
			allowed_transmit_bw[3:0] = 4'b0100: pattern 4 only
			allowed_transmit_bw[3:0] = 4'b0101: pattern 5 only
			allowed_transmit_bw[3:0] = 4'b0110: pattern 6 only
			allowed_transmit_bw[3:0] = 4'b0111: pattern 7 only
			allowed_transmit_bw[3:0] = 4'b1000: pattern 8 only
			allowed_transmit_bw[3:0] = 4'b1001: pattern 9 only
			allowed_transmit_bw[3:0] = 4'b1010: pattern 10 only
			allowed_transmit_bw[3:0] = 4'b1011: pattern 11 only
			allowed_transmit_bw[3:2] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal 0-11>
*/

#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_OFFSET                                    0x0000000000000020
#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_LSB                                       2
#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_MSB                                       5
#define TX_MPDU_START_ALLOWED_TRANSMIT_BW_MASK                                      0x000000000000003c


/* Description		TX_NOTIFY_FRAME

			Consumer: TQM/PDG/TXOLE
			Producer: FW/SW
			
			When clear, this frame does not require any special handling.
			
			
			When set, this MPDU contains an MSDU with the 'FW_tx_notify_frame' 
			field set.
			This means this MPDU is a special frame that requires special
			 handling in TQM.
			
			Note that FW/SW shall always set the amsdu_not_allowed bit
			 in 'TX_MSDU_DETAILS' for any notify frame.
			
			<enum 0 NO_TX_NOTIFY> Not a notify frame
			<enum 1 TX_HARD_NOTIFY>
			<enum 2 TX_SOFT_NOTIFY>
			<enum 3 TX_SEMI_HARD_NOTIFY>
			<enum 4 TX_SEMI_HARD_NOTIFY_CURR_RATE> Rate cannot be overridden
			 by PDG
			<legal 0-4>
*/

#define TX_MPDU_START_TX_NOTIFY_FRAME_OFFSET                                        0x0000000000000020
#define TX_MPDU_START_TX_NOTIFY_FRAME_LSB                                           6
#define TX_MPDU_START_TX_NOTIFY_FRAME_MSB                                           8
#define TX_MPDU_START_TX_NOTIFY_FRAME_MASK                                          0x00000000000001c0


/* Description		RESERVED_8A

			Bit 9: self_gen:
			
			Field only used in the MAC-flexibility feature in TXPCU 
			and PHY microcode
			
			0: Indicates a normal data MPDU
			1: Indicates a self-gen MPDU
			
			Not supported in Hamilton/Waikiki v1
			<legal 0-1>
*/

#define TX_MPDU_START_RESERVED_8A_OFFSET                                            0x0000000000000020
#define TX_MPDU_START_RESERVED_8A_LSB                                               9
#define TX_MPDU_START_RESERVED_8A_MSB                                               31
#define TX_MPDU_START_RESERVED_8A_MASK                                              0x00000000fffffe00


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define TX_MPDU_START_TLV64_PADDING_OFFSET                                          0x0000000000000020
#define TX_MPDU_START_TLV64_PADDING_LSB                                             32
#define TX_MPDU_START_TLV64_PADDING_MSB                                             63
#define TX_MPDU_START_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif   // TX_MPDU_START

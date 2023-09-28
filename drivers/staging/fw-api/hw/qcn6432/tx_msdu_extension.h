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

#ifndef _TX_MSDU_EXTENSION_H_
#define _TX_MSDU_EXTENSION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_MSDU_EXTENSION 18


struct tx_msdu_extension {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t tso_enable                                              :  1, // [0:0]
                      reserved_0a                                             :  6, // [6:1]
                      tcp_flag                                                :  9, // [15:7]
                      tcp_flag_mask                                           :  9, // [24:16]
                      reserved_0b                                             :  7; // [31:25]
             uint32_t l2_length                                               : 16, // [15:0]
                      ip_length                                               : 16; // [31:16]
             uint32_t tcp_seq_number                                          : 32; // [31:0]
             uint32_t ip_identification                                       : 16, // [15:0]
                      udp_length                                              : 16; // [31:16]
             uint32_t checksum_offset                                         : 14, // [13:0]
                      partial_checksum_en                                     :  1, // [14:14]
                      reserved_4a                                             :  1, // [15:15]
                      payload_start_offset                                    : 14, // [29:16]
                      reserved_4b                                             :  2; // [31:30]
             uint32_t payload_end_offset                                      : 14, // [13:0]
                      reserved_5a                                             :  2, // [15:14]
                      wds                                                     :  1, // [16:16]
                      reserved_5b                                             : 15; // [31:17]
             uint32_t buf0_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf0_ptr_39_32                                          :  8, // [7:0]
                      extn_override                                           :  1, // [8:8]
                      encap_type                                              :  2, // [10:9]
                      encrypt_type                                            :  4, // [14:11]
                      tqm_no_drop                                             :  1, // [15:15]
                      buf0_len                                                : 16; // [31:16]
             uint32_t buf1_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf1_ptr_39_32                                          :  8, // [7:0]
                      epd                                                     :  1, // [8:8]
                      mesh_enable                                             :  2, // [10:9]
                      reserved_9a                                             :  5, // [15:11]
                      buf1_len                                                : 16; // [31:16]
             uint32_t buf2_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf2_ptr_39_32                                          :  8, // [7:0]
                      dscp_tid_table_num                                      :  6, // [13:8]
                      reserved_11a                                            :  2, // [15:14]
                      buf2_len                                                : 16; // [31:16]
             uint32_t buf3_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf3_ptr_39_32                                          :  8, // [7:0]
                      reserved_13a                                            :  8, // [15:8]
                      buf3_len                                                : 16; // [31:16]
             uint32_t buf4_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf4_ptr_39_32                                          :  8, // [7:0]
                      reserved_15a                                            :  8, // [15:8]
                      buf4_len                                                : 16; // [31:16]
             uint32_t buf5_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf5_ptr_39_32                                          :  8, // [7:0]
                      reserved_17a                                            :  8, // [15:8]
                      buf5_len                                                : 16; // [31:16]
#else
             uint32_t reserved_0b                                             :  7, // [31:25]
                      tcp_flag_mask                                           :  9, // [24:16]
                      tcp_flag                                                :  9, // [15:7]
                      reserved_0a                                             :  6, // [6:1]
                      tso_enable                                              :  1; // [0:0]
             uint32_t ip_length                                               : 16, // [31:16]
                      l2_length                                               : 16; // [15:0]
             uint32_t tcp_seq_number                                          : 32; // [31:0]
             uint32_t udp_length                                              : 16, // [31:16]
                      ip_identification                                       : 16; // [15:0]
             uint32_t reserved_4b                                             :  2, // [31:30]
                      payload_start_offset                                    : 14, // [29:16]
                      reserved_4a                                             :  1, // [15:15]
                      partial_checksum_en                                     :  1, // [14:14]
                      checksum_offset                                         : 14; // [13:0]
             uint32_t reserved_5b                                             : 15, // [31:17]
                      wds                                                     :  1, // [16:16]
                      reserved_5a                                             :  2, // [15:14]
                      payload_end_offset                                      : 14; // [13:0]
             uint32_t buf0_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf0_len                                                : 16, // [31:16]
                      tqm_no_drop                                             :  1, // [15:15]
                      encrypt_type                                            :  4, // [14:11]
                      encap_type                                              :  2, // [10:9]
                      extn_override                                           :  1, // [8:8]
                      buf0_ptr_39_32                                          :  8; // [7:0]
             uint32_t buf1_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf1_len                                                : 16, // [31:16]
                      reserved_9a                                             :  5, // [15:11]
                      mesh_enable                                             :  2, // [10:9]
                      epd                                                     :  1, // [8:8]
                      buf1_ptr_39_32                                          :  8; // [7:0]
             uint32_t buf2_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf2_len                                                : 16, // [31:16]
                      reserved_11a                                            :  2, // [15:14]
                      dscp_tid_table_num                                      :  6, // [13:8]
                      buf2_ptr_39_32                                          :  8; // [7:0]
             uint32_t buf3_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf3_len                                                : 16, // [31:16]
                      reserved_13a                                            :  8, // [15:8]
                      buf3_ptr_39_32                                          :  8; // [7:0]
             uint32_t buf4_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf4_len                                                : 16, // [31:16]
                      reserved_15a                                            :  8, // [15:8]
                      buf4_ptr_39_32                                          :  8; // [7:0]
             uint32_t buf5_ptr_31_0                                           : 32; // [31:0]
             uint32_t buf5_len                                                : 16, // [31:16]
                      reserved_17a                                            :  8, // [15:8]
                      buf5_ptr_39_32                                          :  8; // [7:0]
#endif
};


/* Description		TSO_ENABLE

			Enable transmit segmentation offload <legal all>
*/

#define TX_MSDU_EXTENSION_TSO_ENABLE_OFFSET                                         0x00000000
#define TX_MSDU_EXTENSION_TSO_ENABLE_LSB                                            0
#define TX_MSDU_EXTENSION_TSO_ENABLE_MSB                                            0
#define TX_MSDU_EXTENSION_TSO_ENABLE_MASK                                           0x00000001


/* Description		RESERVED_0A

			FW will set to 0, MAC will ignore.  <legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_0A_OFFSET                                        0x00000000
#define TX_MSDU_EXTENSION_RESERVED_0A_LSB                                           1
#define TX_MSDU_EXTENSION_RESERVED_0A_MSB                                           6
#define TX_MSDU_EXTENSION_RESERVED_0A_MASK                                          0x0000007e


/* Description		TCP_FLAG

			TCP flags
			{NS,CWR,ECE,URG,ACK,PSH, RST ,SYN,FIN}<legal all>
*/

#define TX_MSDU_EXTENSION_TCP_FLAG_OFFSET                                           0x00000000
#define TX_MSDU_EXTENSION_TCP_FLAG_LSB                                              7
#define TX_MSDU_EXTENSION_TCP_FLAG_MSB                                              15
#define TX_MSDU_EXTENSION_TCP_FLAG_MASK                                             0x0000ff80


/* Description		TCP_FLAG_MASK

			TCP flag mask. Tcp_flag is inserted into the header based
			 on the mask, if TSO is enabled
*/

#define TX_MSDU_EXTENSION_TCP_FLAG_MASK_OFFSET                                      0x00000000
#define TX_MSDU_EXTENSION_TCP_FLAG_MASK_LSB                                         16
#define TX_MSDU_EXTENSION_TCP_FLAG_MASK_MSB                                         24
#define TX_MSDU_EXTENSION_TCP_FLAG_MASK_MASK                                        0x01ff0000


/* Description		RESERVED_0B

			FW will set to 0, MAC will ignore.  <legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_0B_OFFSET                                        0x00000000
#define TX_MSDU_EXTENSION_RESERVED_0B_LSB                                           25
#define TX_MSDU_EXTENSION_RESERVED_0B_MSB                                           31
#define TX_MSDU_EXTENSION_RESERVED_0B_MASK                                          0xfe000000


/* Description		L2_LENGTH

			L2 length for the msdu, if TSO is enabled <legal all>
*/

#define TX_MSDU_EXTENSION_L2_LENGTH_OFFSET                                          0x00000004
#define TX_MSDU_EXTENSION_L2_LENGTH_LSB                                             0
#define TX_MSDU_EXTENSION_L2_LENGTH_MSB                                             15
#define TX_MSDU_EXTENSION_L2_LENGTH_MASK                                            0x0000ffff


/* Description		IP_LENGTH

			IP length for the msdu, if TSO is enabled <legal all>
*/

#define TX_MSDU_EXTENSION_IP_LENGTH_OFFSET                                          0x00000004
#define TX_MSDU_EXTENSION_IP_LENGTH_LSB                                             16
#define TX_MSDU_EXTENSION_IP_LENGTH_MSB                                             31
#define TX_MSDU_EXTENSION_IP_LENGTH_MASK                                            0xffff0000


/* Description		TCP_SEQ_NUMBER

			Tcp_seq_number for the msdu, if TSO is enabled <legal all>
			
*/

#define TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_OFFSET                                     0x00000008
#define TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_LSB                                        0
#define TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_MSB                                        31
#define TX_MSDU_EXTENSION_TCP_SEQ_NUMBER_MASK                                       0xffffffff


/* Description		IP_IDENTIFICATION

			IP_identification for the msdu, if TSO is enabled <legal
			 all>
*/

#define TX_MSDU_EXTENSION_IP_IDENTIFICATION_OFFSET                                  0x0000000c
#define TX_MSDU_EXTENSION_IP_IDENTIFICATION_LSB                                     0
#define TX_MSDU_EXTENSION_IP_IDENTIFICATION_MSB                                     15
#define TX_MSDU_EXTENSION_IP_IDENTIFICATION_MASK                                    0x0000ffff


/* Description		UDP_LENGTH

			TXDMA is copies this field into MSDU START TLV
*/

#define TX_MSDU_EXTENSION_UDP_LENGTH_OFFSET                                         0x0000000c
#define TX_MSDU_EXTENSION_UDP_LENGTH_LSB                                            16
#define TX_MSDU_EXTENSION_UDP_LENGTH_MSB                                            31
#define TX_MSDU_EXTENSION_UDP_LENGTH_MASK                                           0xffff0000


/* Description		CHECKSUM_OFFSET

			The calculated checksum from start offset to end offset 
			will be added to the checksum at the offset given by this
			 field<legal all>
*/

#define TX_MSDU_EXTENSION_CHECKSUM_OFFSET_OFFSET                                    0x00000010
#define TX_MSDU_EXTENSION_CHECKSUM_OFFSET_LSB                                       0
#define TX_MSDU_EXTENSION_CHECKSUM_OFFSET_MSB                                       13
#define TX_MSDU_EXTENSION_CHECKSUM_OFFSET_MASK                                      0x00003fff


/* Description		PARTIAL_CHECKSUM_EN

			Partial Checksum Enable Bit.
			<legal 0-1>
*/

#define TX_MSDU_EXTENSION_PARTIAL_CHECKSUM_EN_OFFSET                                0x00000010
#define TX_MSDU_EXTENSION_PARTIAL_CHECKSUM_EN_LSB                                   14
#define TX_MSDU_EXTENSION_PARTIAL_CHECKSUM_EN_MSB                                   14
#define TX_MSDU_EXTENSION_PARTIAL_CHECKSUM_EN_MASK                                  0x00004000


/* Description		RESERVED_4A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_4A_OFFSET                                        0x00000010
#define TX_MSDU_EXTENSION_RESERVED_4A_LSB                                           15
#define TX_MSDU_EXTENSION_RESERVED_4A_MSB                                           15
#define TX_MSDU_EXTENSION_RESERVED_4A_MASK                                          0x00008000


/* Description		PAYLOAD_START_OFFSET

			L4 checksum calculations will start fromt this offset
			<Legal all>
*/

#define TX_MSDU_EXTENSION_PAYLOAD_START_OFFSET_OFFSET                               0x00000010
#define TX_MSDU_EXTENSION_PAYLOAD_START_OFFSET_LSB                                  16
#define TX_MSDU_EXTENSION_PAYLOAD_START_OFFSET_MSB                                  29
#define TX_MSDU_EXTENSION_PAYLOAD_START_OFFSET_MASK                                 0x3fff0000


/* Description		RESERVED_4B

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_4B_OFFSET                                        0x00000010
#define TX_MSDU_EXTENSION_RESERVED_4B_LSB                                           30
#define TX_MSDU_EXTENSION_RESERVED_4B_MSB                                           31
#define TX_MSDU_EXTENSION_RESERVED_4B_MASK                                          0xc0000000


/* Description		PAYLOAD_END_OFFSET

			L4 checksum calculations will end at this offset. 
			<Legal all>
*/

#define TX_MSDU_EXTENSION_PAYLOAD_END_OFFSET_OFFSET                                 0x00000014
#define TX_MSDU_EXTENSION_PAYLOAD_END_OFFSET_LSB                                    0
#define TX_MSDU_EXTENSION_PAYLOAD_END_OFFSET_MSB                                    13
#define TX_MSDU_EXTENSION_PAYLOAD_END_OFFSET_MASK                                   0x00003fff


/* Description		RESERVED_5A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_5A_OFFSET                                        0x00000014
#define TX_MSDU_EXTENSION_RESERVED_5A_LSB                                           14
#define TX_MSDU_EXTENSION_RESERVED_5A_MSB                                           15
#define TX_MSDU_EXTENSION_RESERVED_5A_MASK                                          0x0000c000


/* Description		WDS

			If set the current packet is 4-address frame.  Required 
			because an aggregate can include some frames with 3 address
			 format and other frames with 4 address format.  Used by
			 the OLE during encapsulation.  
			Note: there is also global wds tx control in the TX_PEER_ENTRY
			
			<legal all>
*/

#define TX_MSDU_EXTENSION_WDS_OFFSET                                                0x00000014
#define TX_MSDU_EXTENSION_WDS_LSB                                                   16
#define TX_MSDU_EXTENSION_WDS_MSB                                                   16
#define TX_MSDU_EXTENSION_WDS_MASK                                                  0x00010000


/* Description		RESERVED_5B

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_5B_OFFSET                                        0x00000014
#define TX_MSDU_EXTENSION_RESERVED_5B_LSB                                           17
#define TX_MSDU_EXTENSION_RESERVED_5B_MSB                                           31
#define TX_MSDU_EXTENSION_RESERVED_5B_MASK                                          0xfffe0000


/* Description		BUF0_PTR_31_0

			Lower 32 bits of the first buffer pointer 
			
			NOTE: SW/FW manages the 'cookie' info related to this buffer
			 together with the 'cookie' info for this MSDU_EXTENSION
			 descriptor
			<legal all>
*/

#define TX_MSDU_EXTENSION_BUF0_PTR_31_0_OFFSET                                      0x00000018
#define TX_MSDU_EXTENSION_BUF0_PTR_31_0_LSB                                         0
#define TX_MSDU_EXTENSION_BUF0_PTR_31_0_MSB                                         31
#define TX_MSDU_EXTENSION_BUF0_PTR_31_0_MASK                                        0xffffffff


/* Description		BUF0_PTR_39_32

			Upper 8 bits of the first buffer pointer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF0_PTR_39_32_OFFSET                                     0x0000001c
#define TX_MSDU_EXTENSION_BUF0_PTR_39_32_LSB                                        0
#define TX_MSDU_EXTENSION_BUF0_PTR_39_32_MSB                                        7
#define TX_MSDU_EXTENSION_BUF0_PTR_39_32_MASK                                       0x000000ff


/* Description		EXTN_OVERRIDE

			Field only used by TCL
			
			When set, the fields encap_type, Encrypt_type, TQM_NO_DROP, 
			EPD and mesh_enable are valid and override any TCL per-bank
			 registers specifying these values (except TQM_NO_DROP).
			
			
			When clear, the values for encap_type, Encrypt_type, EPD, 
			mesh_enable and DSCP_TID_TABLE_NUM are taken from per-bank
			 registers in TCL and TQM_NO_DROP is not being requested
			 by SW.
			
			<legal all>
*/

#define TX_MSDU_EXTENSION_EXTN_OVERRIDE_OFFSET                                      0x0000001c
#define TX_MSDU_EXTENSION_EXTN_OVERRIDE_LSB                                         8
#define TX_MSDU_EXTENSION_EXTN_OVERRIDE_MSB                                         8
#define TX_MSDU_EXTENSION_EXTN_OVERRIDE_MASK                                        0x00000100


/* Description		ENCAP_TYPE

			Field only used by TCL, only valid if Extn_override is set.
			
			
			Indicates the encapsulation that HW will perform:
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> DO NOT USE. Indicate Ethernet
			
			Used by the OLE during encapsulation.
			<legal all>
*/

#define TX_MSDU_EXTENSION_ENCAP_TYPE_OFFSET                                         0x0000001c
#define TX_MSDU_EXTENSION_ENCAP_TYPE_LSB                                            9
#define TX_MSDU_EXTENSION_ENCAP_TYPE_MSB                                            10
#define TX_MSDU_EXTENSION_ENCAP_TYPE_MASK                                           0x00000600


/* Description		ENCRYPT_TYPE

			Field only used by TCL, only valid if Extn_override is set
			 and encap_type = RAW
			
			Indicates type of decrypt cipher used (as defined in the
			 peer entry)
			<enum 0 wep_40> WEP 40-bit
			<enum 1 wep_104> WEP 104-bit
			<enum 2 tkip_no_mic> TKIP without MIC
			<enum 3 wep_128> WEP 128-bit
			<enum 4 tkip_with_mic> TKIP with MIC
			<enum 5 wapi> WAPI
			<enum 6 aes_ccmp_128> AES CCMP 128
			<enum 7 no_cipher> No crypto
			<enum 8 aes_ccmp_256> AES CCMP 256
			<enum 9 aes_gcmp_128> AES CCMP 128
			<enum 10 aes_gcmp_256> AES CCMP 256
			<enum 11 wapi_gcm_sm4> WAPI GCM SM4
			
			<enum 12 wep_varied_width> DO not use... Only for higher
			 layer modules..
			<legal 0-12>
*/

#define TX_MSDU_EXTENSION_ENCRYPT_TYPE_OFFSET                                       0x0000001c
#define TX_MSDU_EXTENSION_ENCRYPT_TYPE_LSB                                          11
#define TX_MSDU_EXTENSION_ENCRYPT_TYPE_MSB                                          14
#define TX_MSDU_EXTENSION_ENCRYPT_TYPE_MASK                                         0x00007800


/* Description		TQM_NO_DROP

			Field only used by TCL, only valid if Extn_override is set.
			
			
			This bit is used to stop TQM from dropping MSDUs while adding
			 them to MSDU flows1'b1: Do not drop MSDU when any of the
			 threshold value is met while adding MSDU in a flow1'b1: 
			Drop MSDU when any of the threshold value is met while adding
			 MSDU in a flow
			Note: TCL can also have CCE/LCE rules to set 'TQM_NO_DROP' 
			which will be OR'd to this value.
			<legal all>
*/

#define TX_MSDU_EXTENSION_TQM_NO_DROP_OFFSET                                        0x0000001c
#define TX_MSDU_EXTENSION_TQM_NO_DROP_LSB                                           15
#define TX_MSDU_EXTENSION_TQM_NO_DROP_MSB                                           15
#define TX_MSDU_EXTENSION_TQM_NO_DROP_MASK                                          0x00008000


/* Description		BUF0_LEN

			Length of the first buffer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF0_LEN_OFFSET                                           0x0000001c
#define TX_MSDU_EXTENSION_BUF0_LEN_LSB                                              16
#define TX_MSDU_EXTENSION_BUF0_LEN_MSB                                              31
#define TX_MSDU_EXTENSION_BUF0_LEN_MASK                                             0xffff0000


/* Description		BUF1_PTR_31_0

			Lower 32 bits of the second buffer pointer 
			
			NOTE: SW/FW manages the 'cookie' info related to this buffer
			 together with the 'cookie' info for this MSDU_EXTENSION
			 descriptor
			<legal all>
*/

#define TX_MSDU_EXTENSION_BUF1_PTR_31_0_OFFSET                                      0x00000020
#define TX_MSDU_EXTENSION_BUF1_PTR_31_0_LSB                                         0
#define TX_MSDU_EXTENSION_BUF1_PTR_31_0_MSB                                         31
#define TX_MSDU_EXTENSION_BUF1_PTR_31_0_MASK                                        0xffffffff


/* Description		BUF1_PTR_39_32

			Upper 8 bits of the second buffer pointer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF1_PTR_39_32_OFFSET                                     0x00000024
#define TX_MSDU_EXTENSION_BUF1_PTR_39_32_LSB                                        0
#define TX_MSDU_EXTENSION_BUF1_PTR_39_32_MSB                                        7
#define TX_MSDU_EXTENSION_BUF1_PTR_39_32_MASK                                       0x000000ff


/* Description		EPD

			Field only used by TCL, only valid if Extn_override is set.
			
			
			When this bit is set then input packet is an EPD type
			<legal all>
*/

#define TX_MSDU_EXTENSION_EPD_OFFSET                                                0x00000024
#define TX_MSDU_EXTENSION_EPD_LSB                                                   8
#define TX_MSDU_EXTENSION_EPD_MSB                                                   8
#define TX_MSDU_EXTENSION_EPD_MASK                                                  0x00000100


/* Description		MESH_ENABLE

			Field only used by TCL, only valid if Extn_override is set.
			
			
			If set to a non-zero value:
			* For raw WiFi frames, this indicates transmission to a 
			mesh STA, enabling the interpretation of the 'Mesh Control
			 Present' bit (bit 8) of QoS Control (otherwise this bit
			 is ignored). The interpretation of the A-MSDU 'Length' 
			field is decided by the e-numerations below.
			* For native WiFi frames, this indicates that a 'Mesh Control' 
			field is present between the header and the LLC. The three
			 non-zero values are interchangeable.
			
			<enum 0 MESH_DISABLE>
			<enum 1 MESH_Q2Q> A-MSDU 'Length' is big endian and includes
			 the length of Mesh Control.
			<enum 2 MESH_11S_BE> A-MSDU 'Length' is big endian and excludes
			 the length of Mesh Control.
			<enum 3 MESH_11S_LE> A-MSDU 'Length' is little endian and
			 excludes the length of Mesh Control. This is 802.11s-compliant.
			
			<legal 0-3>
*/

#define TX_MSDU_EXTENSION_MESH_ENABLE_OFFSET                                        0x00000024
#define TX_MSDU_EXTENSION_MESH_ENABLE_LSB                                           9
#define TX_MSDU_EXTENSION_MESH_ENABLE_MSB                                           10
#define TX_MSDU_EXTENSION_MESH_ENABLE_MASK                                          0x00000600


/* Description		RESERVED_9A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_9A_OFFSET                                        0x00000024
#define TX_MSDU_EXTENSION_RESERVED_9A_LSB                                           11
#define TX_MSDU_EXTENSION_RESERVED_9A_MSB                                           15
#define TX_MSDU_EXTENSION_RESERVED_9A_MASK                                          0x0000f800


/* Description		BUF1_LEN

			Length of the second buffer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF1_LEN_OFFSET                                           0x00000024
#define TX_MSDU_EXTENSION_BUF1_LEN_LSB                                              16
#define TX_MSDU_EXTENSION_BUF1_LEN_MSB                                              31
#define TX_MSDU_EXTENSION_BUF1_LEN_MASK                                             0xffff0000


/* Description		BUF2_PTR_31_0

			Lower 32 bits of the third buffer pointer 
			NOTE: SW/FW manages the 'cookie' info related to this buffer
			 together with the 'cookie' info for this MSDU_EXTENSION
			 descriptor
			<legal all>
*/

#define TX_MSDU_EXTENSION_BUF2_PTR_31_0_OFFSET                                      0x00000028
#define TX_MSDU_EXTENSION_BUF2_PTR_31_0_LSB                                         0
#define TX_MSDU_EXTENSION_BUF2_PTR_31_0_MSB                                         31
#define TX_MSDU_EXTENSION_BUF2_PTR_31_0_MASK                                        0xffffffff


/* Description		BUF2_PTR_39_32

			Upper 8 bits of the third buffer pointer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF2_PTR_39_32_OFFSET                                     0x0000002c
#define TX_MSDU_EXTENSION_BUF2_PTR_39_32_LSB                                        0
#define TX_MSDU_EXTENSION_BUF2_PTR_39_32_MSB                                        7
#define TX_MSDU_EXTENSION_BUF2_PTR_39_32_MASK                                       0x000000ff


/* Description		DSCP_TID_TABLE_NUM

			Field only used by TCL, only valid if Extn_override is set.
			
			
			This specifies the DSCP to TID mapping table to be used 
			for the MSDU
			<legal all>
*/

#define TX_MSDU_EXTENSION_DSCP_TID_TABLE_NUM_OFFSET                                 0x0000002c
#define TX_MSDU_EXTENSION_DSCP_TID_TABLE_NUM_LSB                                    8
#define TX_MSDU_EXTENSION_DSCP_TID_TABLE_NUM_MSB                                    13
#define TX_MSDU_EXTENSION_DSCP_TID_TABLE_NUM_MASK                                   0x00003f00


/* Description		RESERVED_11A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_11A_OFFSET                                       0x0000002c
#define TX_MSDU_EXTENSION_RESERVED_11A_LSB                                          14
#define TX_MSDU_EXTENSION_RESERVED_11A_MSB                                          15
#define TX_MSDU_EXTENSION_RESERVED_11A_MASK                                         0x0000c000


/* Description		BUF2_LEN

			Length of the third buffer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF2_LEN_OFFSET                                           0x0000002c
#define TX_MSDU_EXTENSION_BUF2_LEN_LSB                                              16
#define TX_MSDU_EXTENSION_BUF2_LEN_MSB                                              31
#define TX_MSDU_EXTENSION_BUF2_LEN_MASK                                             0xffff0000


/* Description		BUF3_PTR_31_0

			Lower 32 bits of the fourth buffer pointer
			
			NOTE: SW/FW manages the 'cookie' info related to this buffer
			 together with the 'cookie' info for this MSDU_EXTENSION
			 descriptor
			 <legal all>
*/

#define TX_MSDU_EXTENSION_BUF3_PTR_31_0_OFFSET                                      0x00000030
#define TX_MSDU_EXTENSION_BUF3_PTR_31_0_LSB                                         0
#define TX_MSDU_EXTENSION_BUF3_PTR_31_0_MSB                                         31
#define TX_MSDU_EXTENSION_BUF3_PTR_31_0_MASK                                        0xffffffff


/* Description		BUF3_PTR_39_32

			Upper 8 bits of the fourth buffer pointer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF3_PTR_39_32_OFFSET                                     0x00000034
#define TX_MSDU_EXTENSION_BUF3_PTR_39_32_LSB                                        0
#define TX_MSDU_EXTENSION_BUF3_PTR_39_32_MSB                                        7
#define TX_MSDU_EXTENSION_BUF3_PTR_39_32_MASK                                       0x000000ff


/* Description		RESERVED_13A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_13A_OFFSET                                       0x00000034
#define TX_MSDU_EXTENSION_RESERVED_13A_LSB                                          8
#define TX_MSDU_EXTENSION_RESERVED_13A_MSB                                          15
#define TX_MSDU_EXTENSION_RESERVED_13A_MASK                                         0x0000ff00


/* Description		BUF3_LEN

			Length of the fourth buffer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF3_LEN_OFFSET                                           0x00000034
#define TX_MSDU_EXTENSION_BUF3_LEN_LSB                                              16
#define TX_MSDU_EXTENSION_BUF3_LEN_MSB                                              31
#define TX_MSDU_EXTENSION_BUF3_LEN_MASK                                             0xffff0000


/* Description		BUF4_PTR_31_0

			Lower 32 bits of the fifth buffer pointer 
			
			NOTE: SW/FW manages the 'cookie' info related to this buffer
			 together with the 'cookie' info for this MSDU_EXTENSION
			 descriptor
			<legal all>
*/

#define TX_MSDU_EXTENSION_BUF4_PTR_31_0_OFFSET                                      0x00000038
#define TX_MSDU_EXTENSION_BUF4_PTR_31_0_LSB                                         0
#define TX_MSDU_EXTENSION_BUF4_PTR_31_0_MSB                                         31
#define TX_MSDU_EXTENSION_BUF4_PTR_31_0_MASK                                        0xffffffff


/* Description		BUF4_PTR_39_32

			Upper 8 bits of the fifth buffer pointer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF4_PTR_39_32_OFFSET                                     0x0000003c
#define TX_MSDU_EXTENSION_BUF4_PTR_39_32_LSB                                        0
#define TX_MSDU_EXTENSION_BUF4_PTR_39_32_MSB                                        7
#define TX_MSDU_EXTENSION_BUF4_PTR_39_32_MASK                                       0x000000ff


/* Description		RESERVED_15A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_15A_OFFSET                                       0x0000003c
#define TX_MSDU_EXTENSION_RESERVED_15A_LSB                                          8
#define TX_MSDU_EXTENSION_RESERVED_15A_MSB                                          15
#define TX_MSDU_EXTENSION_RESERVED_15A_MASK                                         0x0000ff00


/* Description		BUF4_LEN

			Length of the fifth buffer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF4_LEN_OFFSET                                           0x0000003c
#define TX_MSDU_EXTENSION_BUF4_LEN_LSB                                              16
#define TX_MSDU_EXTENSION_BUF4_LEN_MSB                                              31
#define TX_MSDU_EXTENSION_BUF4_LEN_MASK                                             0xffff0000


/* Description		BUF5_PTR_31_0

			Lower 32 bits of the sixth buffer pointer
			
			NOTE: SW/FW manages the 'cookie' info related to this buffer
			 together with the 'cookie' info for this MSDU_EXTENSION
			 descriptor
			 <legal all>
*/

#define TX_MSDU_EXTENSION_BUF5_PTR_31_0_OFFSET                                      0x00000040
#define TX_MSDU_EXTENSION_BUF5_PTR_31_0_LSB                                         0
#define TX_MSDU_EXTENSION_BUF5_PTR_31_0_MSB                                         31
#define TX_MSDU_EXTENSION_BUF5_PTR_31_0_MASK                                        0xffffffff


/* Description		BUF5_PTR_39_32

			Upper 8 bits of the sixth buffer pointer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF5_PTR_39_32_OFFSET                                     0x00000044
#define TX_MSDU_EXTENSION_BUF5_PTR_39_32_LSB                                        0
#define TX_MSDU_EXTENSION_BUF5_PTR_39_32_MSB                                        7
#define TX_MSDU_EXTENSION_BUF5_PTR_39_32_MASK                                       0x000000ff


/* Description		RESERVED_17A

			<Legal 0>
*/

#define TX_MSDU_EXTENSION_RESERVED_17A_OFFSET                                       0x00000044
#define TX_MSDU_EXTENSION_RESERVED_17A_LSB                                          8
#define TX_MSDU_EXTENSION_RESERVED_17A_MSB                                          15
#define TX_MSDU_EXTENSION_RESERVED_17A_MASK                                         0x0000ff00


/* Description		BUF5_LEN

			Length of the sixth buffer <legal all>
*/

#define TX_MSDU_EXTENSION_BUF5_LEN_OFFSET                                           0x00000044
#define TX_MSDU_EXTENSION_BUF5_LEN_LSB                                              16
#define TX_MSDU_EXTENSION_BUF5_LEN_MSB                                              31
#define TX_MSDU_EXTENSION_BUF5_LEN_MASK                                             0xffff0000



#endif   // TX_MSDU_EXTENSION

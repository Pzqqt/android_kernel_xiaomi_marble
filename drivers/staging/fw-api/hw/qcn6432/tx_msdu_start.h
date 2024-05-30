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

#ifndef _TX_MSDU_START_H_
#define _TX_MSDU_START_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_MSDU_START 8

#define NUM_OF_QWORDS_TX_MSDU_START 4


struct tx_msdu_start {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t msdu_len                                                : 14, // [13:0]
                      first_msdu                                              :  1, // [14:14]
                      last_msdu                                               :  1, // [15:15]
                      encap_type                                              :  2, // [17:16]
                      epd_en                                                  :  1, // [18:18]
                      da_sa_present                                           :  2, // [20:19]
                      ipv4_checksum_en                                        :  1, // [21:21]
                      udp_over_ipv4_checksum_en                               :  1, // [22:22]
                      udp_over_ipv6_checksum_en                               :  1, // [23:23]
                      tcp_over_ipv4_checksum_en                               :  1, // [24:24]
                      tcp_over_ipv6_checksum_en                               :  1, // [25:25]
                      dummy_msdu_delimitation                                 :  1, // [26:26]
                      reserved_0a                                             :  5; // [31:27]
             uint32_t tso_enable                                              :  1, // [0:0]
                      reserved_1a                                             :  6, // [6:1]
                      tcp_flag                                                :  9, // [15:7]
                      tcp_flag_mask                                           :  9, // [24:16]
                      mesh_enable                                             :  1, // [25:25]
                      reserved_1b                                             :  6; // [31:26]
             uint32_t l2_length                                               : 16, // [15:0]
                      ip_length                                               : 16; // [31:16]
             uint32_t tcp_seq_number                                          : 32; // [31:0]
             uint32_t ip_identification                                       : 16, // [15:0]
                      checksum_offset                                         : 13, // [28:16]
                      partial_checksum_en                                     :  1, // [29:29]
                      reserved_4                                              :  2; // [31:30]
             uint32_t payload_start_offset                                    : 14, // [13:0]
                      reserved_5a                                             :  2, // [15:14]
                      payload_end_offset                                      : 14, // [29:16]
                      reserved_5b                                             :  2; // [31:30]
             uint32_t udp_length                                              : 16, // [15:0]
                      reserved_6                                              : 16; // [31:16]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             uint32_t reserved_0a                                             :  5, // [31:27]
                      dummy_msdu_delimitation                                 :  1, // [26:26]
                      tcp_over_ipv6_checksum_en                               :  1, // [25:25]
                      tcp_over_ipv4_checksum_en                               :  1, // [24:24]
                      udp_over_ipv6_checksum_en                               :  1, // [23:23]
                      udp_over_ipv4_checksum_en                               :  1, // [22:22]
                      ipv4_checksum_en                                        :  1, // [21:21]
                      da_sa_present                                           :  2, // [20:19]
                      epd_en                                                  :  1, // [18:18]
                      encap_type                                              :  2, // [17:16]
                      last_msdu                                               :  1, // [15:15]
                      first_msdu                                              :  1, // [14:14]
                      msdu_len                                                : 14; // [13:0]
             uint32_t reserved_1b                                             :  6, // [31:26]
                      mesh_enable                                             :  1, // [25:25]
                      tcp_flag_mask                                           :  9, // [24:16]
                      tcp_flag                                                :  9, // [15:7]
                      reserved_1a                                             :  6, // [6:1]
                      tso_enable                                              :  1; // [0:0]
             uint32_t ip_length                                               : 16, // [31:16]
                      l2_length                                               : 16; // [15:0]
             uint32_t tcp_seq_number                                          : 32; // [31:0]
             uint32_t reserved_4                                              :  2, // [31:30]
                      partial_checksum_en                                     :  1, // [29:29]
                      checksum_offset                                         : 13, // [28:16]
                      ip_identification                                       : 16; // [15:0]
             uint32_t reserved_5b                                             :  2, // [31:30]
                      payload_end_offset                                      : 14, // [29:16]
                      reserved_5a                                             :  2, // [15:14]
                      payload_start_offset                                    : 14; // [13:0]
             uint32_t reserved_6                                              : 16, // [31:16]
                      udp_length                                              : 16; // [15:0]
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		MSDU_LEN

			MSDU length before encapsulation. It is the same value as
			 the length in the MSDU packet TLV
*/

#define TX_MSDU_START_MSDU_LEN_OFFSET                                               0x0000000000000000
#define TX_MSDU_START_MSDU_LEN_LSB                                                  0
#define TX_MSDU_START_MSDU_LEN_MSB                                                  13
#define TX_MSDU_START_MSDU_LEN_MASK                                                 0x0000000000003fff


/* Description		FIRST_MSDU

			If set the current MSDU is the first MSDU in MPDU.  Used
			 by the OLE during encapsulation.
*/

#define TX_MSDU_START_FIRST_MSDU_OFFSET                                             0x0000000000000000
#define TX_MSDU_START_FIRST_MSDU_LSB                                                14
#define TX_MSDU_START_FIRST_MSDU_MSB                                                14
#define TX_MSDU_START_FIRST_MSDU_MASK                                               0x0000000000004000


/* Description		LAST_MSDU

			If set the current MSDU is the last MSDU in MPDU.  Used 
			by the OLE during encapsulation.
*/

#define TX_MSDU_START_LAST_MSDU_OFFSET                                              0x0000000000000000
#define TX_MSDU_START_LAST_MSDU_LSB                                                 15
#define TX_MSDU_START_LAST_MSDU_MSB                                                 15
#define TX_MSDU_START_LAST_MSDU_MASK                                                0x0000000000008000


/* Description		ENCAP_TYPE

			Indicates the encapsulation that HW will perform:
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX) or 802.3 (uses SNAP/LLC) 
			
			<enum 3 802_3> DO NOT USE. Indicate Ethernet
			Used by the OLE during encapsulation.
			<legal all>
*/

#define TX_MSDU_START_ENCAP_TYPE_OFFSET                                             0x0000000000000000
#define TX_MSDU_START_ENCAP_TYPE_LSB                                                16
#define TX_MSDU_START_ENCAP_TYPE_MSB                                                17
#define TX_MSDU_START_ENCAP_TYPE_MASK                                               0x0000000000030000


/* Description		EPD_EN

			Consumer: TXOLE
			Producer: SW/TCL
			
			If set to one use EPD instead of LPD
			<legal all>
*/

#define TX_MSDU_START_EPD_EN_OFFSET                                                 0x0000000000000000
#define TX_MSDU_START_EPD_EN_LSB                                                    18
#define TX_MSDU_START_EPD_EN_MSB                                                    18
#define TX_MSDU_START_EPD_EN_MASK                                                   0x0000000000040000


/* Description		DA_SA_PRESENT

			Used for 11ah
			
			Indicates the encapsulation that HW will perform:
			<enum 0 DA_SA_IS_ABSENT> DA and SA absent
			<enum 1 DA_IS_PRESENT>  DA Present, SA Absent
			<enum 2 SA_IS_PRESENT>  
			<enum 3 DA_SA_IS_PRESENT>  Both DA and SA are present 
			Used by the OLE during encapsulation.
			
			TXDMA gets this configuration from a sw configuration register.
			
			
			<legal all>
*/

#define TX_MSDU_START_DA_SA_PRESENT_OFFSET                                          0x0000000000000000
#define TX_MSDU_START_DA_SA_PRESENT_LSB                                             19
#define TX_MSDU_START_DA_SA_PRESENT_MSB                                             20
#define TX_MSDU_START_DA_SA_PRESENT_MASK                                            0x0000000000180000


/* Description		IPV4_CHECKSUM_EN

			Enable IPv4 checksum replacement
*/

#define TX_MSDU_START_IPV4_CHECKSUM_EN_OFFSET                                       0x0000000000000000
#define TX_MSDU_START_IPV4_CHECKSUM_EN_LSB                                          21
#define TX_MSDU_START_IPV4_CHECKSUM_EN_MSB                                          21
#define TX_MSDU_START_IPV4_CHECKSUM_EN_MASK                                         0x0000000000200000


/* Description		UDP_OVER_IPV4_CHECKSUM_EN

			Enable UDP over IPv4 checksum replacement.  UDP checksum
			 over IPv4 is optional for TCP/IP stacks.
*/

#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_LSB                                 22
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_MSB                                 22
#define TX_MSDU_START_UDP_OVER_IPV4_CHECKSUM_EN_MASK                                0x0000000000400000


/* Description		UDP_OVER_IPV6_CHECKSUM_EN

			Enable UDP over IPv6 checksum replacement.  UDP checksum
			 over IPv6 is mandatory for TCP/IP stacks.
*/

#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_LSB                                 23
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_MSB                                 23
#define TX_MSDU_START_UDP_OVER_IPV6_CHECKSUM_EN_MASK                                0x0000000000800000


/* Description		TCP_OVER_IPV4_CHECKSUM_EN

			Enable TCP checksum over IPv4 replacement
*/

#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_LSB                                 24
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_MSB                                 24
#define TX_MSDU_START_TCP_OVER_IPV4_CHECKSUM_EN_MASK                                0x0000000001000000


/* Description		TCP_OVER_IPV6_CHECKSUM_EN

			Enable TCP checksum over IPv6 eplacement
*/

#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET                              0x0000000000000000
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_LSB                                 25
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_MSB                                 25
#define TX_MSDU_START_TCP_OVER_IPV6_CHECKSUM_EN_MASK                                0x0000000002000000


/* Description		DUMMY_MSDU_DELIMITATION

			This bit is mainly for debug.
			
			TXDMA sets this bit when sending a dummy 'TX_MSDU_END' + 'TX_MSDU_START' 
			sequence for a user to delimit user arbitration where it
			 could switch to packet data from other users before continuing
			 this MSDU.
			
			This is done mainly for long raw Wi-Fi packets where TXDMA
			 needs to switch users in the midst of the packet but other
			 blocks assume TXDMA switch only at MSDU boundaries.
			<legal all>
*/

#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_OFFSET                                0x0000000000000000
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_LSB                                   26
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_MSB                                   26
#define TX_MSDU_START_DUMMY_MSDU_DELIMITATION_MASK                                  0x0000000004000000


/* Description		RESERVED_0A

			FW will set to 0, MAC will ignore.  <legal 0>
*/

#define TX_MSDU_START_RESERVED_0A_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_RESERVED_0A_LSB                                               27
#define TX_MSDU_START_RESERVED_0A_MSB                                               31
#define TX_MSDU_START_RESERVED_0A_MASK                                              0x00000000f8000000


/* Description		TSO_ENABLE

			Enable transmit segmentation offload.
			
			In case MSDU_EXTENSION is used, TXDMA gets the setting for
			 this bit from that descriptor.
			In case MSDU_EXTENSION is NOT use, TXDMA gets the setting
			 for this bit from an internal SW programmable register.
			
			 <legal all>
*/

#define TX_MSDU_START_TSO_ENABLE_OFFSET                                             0x0000000000000000
#define TX_MSDU_START_TSO_ENABLE_LSB                                                32
#define TX_MSDU_START_TSO_ENABLE_MSB                                                32
#define TX_MSDU_START_TSO_ENABLE_MASK                                               0x0000000100000000


/* Description		RESERVED_1A

			FW will set to 0, MAC will ignore.  <legal 0>
*/

#define TX_MSDU_START_RESERVED_1A_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_RESERVED_1A_LSB                                               33
#define TX_MSDU_START_RESERVED_1A_MSB                                               38
#define TX_MSDU_START_RESERVED_1A_MASK                                              0x0000007e00000000


/* Description		TCP_FLAG

			TCP flags
			{NS,CWR,ECE,URG,ACK,PSH, RST ,SYN,FIN}<legal all>
*/

#define TX_MSDU_START_TCP_FLAG_OFFSET                                               0x0000000000000000
#define TX_MSDU_START_TCP_FLAG_LSB                                                  39
#define TX_MSDU_START_TCP_FLAG_MSB                                                  47
#define TX_MSDU_START_TCP_FLAG_MASK                                                 0x0000ff8000000000


/* Description		TCP_FLAG_MASK

			TCP flag mask. Tcp_flag is inserted into the header based
			 on the mask, if TSO is enabled
*/

#define TX_MSDU_START_TCP_FLAG_MASK_OFFSET                                          0x0000000000000000
#define TX_MSDU_START_TCP_FLAG_MASK_LSB                                             48
#define TX_MSDU_START_TCP_FLAG_MASK_MSB                                             56
#define TX_MSDU_START_TCP_FLAG_MASK_MASK                                            0x01ff000000000000


/* Description		MESH_ENABLE

			If set to 1:
			
			* For raw WiFi frames, this indicates transmission to a 
			mesh STA but is ignored by HW
			
			* For native WiFi frames, this is used to indicate to TX
			 OLE that a 'Mesh Control' field is present between the 
			header and the LLC
*/

#define TX_MSDU_START_MESH_ENABLE_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_MESH_ENABLE_LSB                                               57
#define TX_MSDU_START_MESH_ENABLE_MSB                                               57
#define TX_MSDU_START_MESH_ENABLE_MASK                                              0x0200000000000000


/* Description		RESERVED_1B

			FW will set to 0, MAC will ignore.  <legal 0>
*/

#define TX_MSDU_START_RESERVED_1B_OFFSET                                            0x0000000000000000
#define TX_MSDU_START_RESERVED_1B_LSB                                               58
#define TX_MSDU_START_RESERVED_1B_MSB                                               63
#define TX_MSDU_START_RESERVED_1B_MASK                                              0xfc00000000000000


/* Description		L2_LENGTH

			L2 length for the msdu, if TSO is enabled <legal all>
*/

#define TX_MSDU_START_L2_LENGTH_OFFSET                                              0x0000000000000008
#define TX_MSDU_START_L2_LENGTH_LSB                                                 0
#define TX_MSDU_START_L2_LENGTH_MSB                                                 15
#define TX_MSDU_START_L2_LENGTH_MASK                                                0x000000000000ffff


/* Description		IP_LENGTH

			IP length for the msdu, if TSO is enabled <legal all>
*/

#define TX_MSDU_START_IP_LENGTH_OFFSET                                              0x0000000000000008
#define TX_MSDU_START_IP_LENGTH_LSB                                                 16
#define TX_MSDU_START_IP_LENGTH_MSB                                                 31
#define TX_MSDU_START_IP_LENGTH_MASK                                                0x00000000ffff0000


/* Description		TCP_SEQ_NUMBER

			Tcp_seq_number for the msdu, if TSO is enabled <legal all>
			
*/

#define TX_MSDU_START_TCP_SEQ_NUMBER_OFFSET                                         0x0000000000000008
#define TX_MSDU_START_TCP_SEQ_NUMBER_LSB                                            32
#define TX_MSDU_START_TCP_SEQ_NUMBER_MSB                                            63
#define TX_MSDU_START_TCP_SEQ_NUMBER_MASK                                           0xffffffff00000000


/* Description		IP_IDENTIFICATION

			IP_identification for the msdu, if TSO is enabled <legal
			 all>
*/

#define TX_MSDU_START_IP_IDENTIFICATION_OFFSET                                      0x0000000000000010
#define TX_MSDU_START_IP_IDENTIFICATION_LSB                                         0
#define TX_MSDU_START_IP_IDENTIFICATION_MSB                                         15
#define TX_MSDU_START_IP_IDENTIFICATION_MASK                                        0x000000000000ffff


/* Description		CHECKSUM_OFFSET

			The calculated checksum from start offset to end offset 
			will be added to the checksum at the offset given by this
			 field<legal all>
*/

#define TX_MSDU_START_CHECKSUM_OFFSET_OFFSET                                        0x0000000000000010
#define TX_MSDU_START_CHECKSUM_OFFSET_LSB                                           16
#define TX_MSDU_START_CHECKSUM_OFFSET_MSB                                           28
#define TX_MSDU_START_CHECKSUM_OFFSET_MASK                                          0x000000001fff0000


/* Description		PARTIAL_CHECKSUM_EN

			Enable Partial Checksum, MAV feature
*/

#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_OFFSET                                    0x0000000000000010
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_LSB                                       29
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_MSB                                       29
#define TX_MSDU_START_PARTIAL_CHECKSUM_EN_MASK                                      0x0000000020000000


/* Description		RESERVED_4

			<legal 0>
*/

#define TX_MSDU_START_RESERVED_4_OFFSET                                             0x0000000000000010
#define TX_MSDU_START_RESERVED_4_LSB                                                30
#define TX_MSDU_START_RESERVED_4_MSB                                                31
#define TX_MSDU_START_RESERVED_4_MASK                                               0x00000000c0000000


/* Description		PAYLOAD_START_OFFSET

			L4 checksum calculations will start fromt this offset
			<legal all>
*/

#define TX_MSDU_START_PAYLOAD_START_OFFSET_OFFSET                                   0x0000000000000010
#define TX_MSDU_START_PAYLOAD_START_OFFSET_LSB                                      32
#define TX_MSDU_START_PAYLOAD_START_OFFSET_MSB                                      45
#define TX_MSDU_START_PAYLOAD_START_OFFSET_MASK                                     0x00003fff00000000


/* Description		RESERVED_5A

			<legal 0>
*/

#define TX_MSDU_START_RESERVED_5A_OFFSET                                            0x0000000000000010
#define TX_MSDU_START_RESERVED_5A_LSB                                               46
#define TX_MSDU_START_RESERVED_5A_MSB                                               47
#define TX_MSDU_START_RESERVED_5A_MASK                                              0x0000c00000000000


/* Description		PAYLOAD_END_OFFSET

			L4 checksum calculations will end at this offset. 
			<legal all>
*/

#define TX_MSDU_START_PAYLOAD_END_OFFSET_OFFSET                                     0x0000000000000010
#define TX_MSDU_START_PAYLOAD_END_OFFSET_LSB                                        48
#define TX_MSDU_START_PAYLOAD_END_OFFSET_MSB                                        61
#define TX_MSDU_START_PAYLOAD_END_OFFSET_MASK                                       0x3fff000000000000


/* Description		RESERVED_5B

			<legal 0>
*/

#define TX_MSDU_START_RESERVED_5B_OFFSET                                            0x0000000000000010
#define TX_MSDU_START_RESERVED_5B_LSB                                               62
#define TX_MSDU_START_RESERVED_5B_MSB                                               63
#define TX_MSDU_START_RESERVED_5B_MASK                                              0xc000000000000000


/* Description		UDP_LENGTH

			This field indicates UDP length/UDP lite checksum coverage
			 field to be used by L4 checksum engine in case TSO is enabled
			 for UDP/UDP lite respectively 
			<legal all>
*/

#define TX_MSDU_START_UDP_LENGTH_OFFSET                                             0x0000000000000018
#define TX_MSDU_START_UDP_LENGTH_LSB                                                0
#define TX_MSDU_START_UDP_LENGTH_MSB                                                15
#define TX_MSDU_START_UDP_LENGTH_MASK                                               0x000000000000ffff


/* Description		RESERVED_6

			<legal 0>
*/

#define TX_MSDU_START_RESERVED_6_OFFSET                                             0x0000000000000018
#define TX_MSDU_START_RESERVED_6_LSB                                                16
#define TX_MSDU_START_RESERVED_6_MSB                                                31
#define TX_MSDU_START_RESERVED_6_MASK                                               0x00000000ffff0000


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define TX_MSDU_START_TLV64_PADDING_OFFSET                                          0x0000000000000018
#define TX_MSDU_START_TLV64_PADDING_LSB                                             32
#define TX_MSDU_START_TLV64_PADDING_MSB                                             63
#define TX_MSDU_START_TLV64_PADDING_MASK                                            0xffffffff00000000



#endif   // TX_MSDU_START

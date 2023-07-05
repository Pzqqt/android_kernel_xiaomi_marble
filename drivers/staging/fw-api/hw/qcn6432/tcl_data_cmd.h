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

#ifndef _TCL_DATA_CMD_H_
#define _TCL_DATA_CMD_H_
#if !defined(__ASSEMBLER__)
#endif

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_TCL_DATA_CMD 8


struct tcl_data_cmd {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          buf_addr_info;
             uint32_t tcl_cmd_type                                            :  1, // [0:0]
                      buf_or_ext_desc_type                                    :  1, // [1:1]
                      bank_id                                                 :  6, // [7:2]
                      tx_notify_frame                                         :  3, // [10:8]
                      header_length_read_sel                                  :  1, // [11:11]
                      buffer_timestamp                                        : 19, // [30:12]
                      buffer_timestamp_valid                                  :  1; // [31:31]
             uint32_t reserved_3a                                             : 16, // [15:0]
                      tcl_cmd_number                                          : 16; // [31:16]
             uint32_t data_length                                             : 16, // [15:0]
                      ipv4_checksum_en                                        :  1, // [16:16]
                      udp_over_ipv4_checksum_en                               :  1, // [17:17]
                      udp_over_ipv6_checksum_en                               :  1, // [18:18]
                      tcp_over_ipv4_checksum_en                               :  1, // [19:19]
                      tcp_over_ipv6_checksum_en                               :  1, // [20:20]
                      to_fw                                                   :  1, // [21:21]
                      reserved_4a                                             :  1, // [22:22]
                      packet_offset                                           :  9; // [31:23]
             uint32_t hlos_tid_overwrite                                      :  1, // [0:0]
                      flow_override_enable                                    :  1, // [1:1]
                      who_classify_info_sel                                   :  2, // [3:2]
                      hlos_tid                                                :  4, // [7:4]
                      flow_override                                           :  1, // [8:8]
                      pmac_id                                                 :  2, // [10:9]
                      msdu_color                                              :  2, // [12:11]
                      reserved_5a                                             : 11, // [23:13]
                      vdev_id                                                 :  8; // [31:24]
             uint32_t search_index                                            : 20, // [19:0]
                      cache_set_num                                           :  4, // [23:20]
                      index_lookup_override                                   :  1, // [24:24]
                      reserved_6a                                             :  7; // [31:25]
             uint32_t reserved_7a                                             : 20, // [19:0]
                      ring_id                                                 :  8, // [27:20]
                      looping_count                                           :  4; // [31:28]
#else
             struct   buffer_addr_info                                          buf_addr_info;
             uint32_t buffer_timestamp_valid                                  :  1, // [31:31]
                      buffer_timestamp                                        : 19, // [30:12]
                      header_length_read_sel                                  :  1, // [11:11]
                      tx_notify_frame                                         :  3, // [10:8]
                      bank_id                                                 :  6, // [7:2]
                      buf_or_ext_desc_type                                    :  1, // [1:1]
                      tcl_cmd_type                                            :  1; // [0:0]
             uint32_t tcl_cmd_number                                          : 16, // [31:16]
                      reserved_3a                                             : 16; // [15:0]
             uint32_t packet_offset                                           :  9, // [31:23]
                      reserved_4a                                             :  1, // [22:22]
                      to_fw                                                   :  1, // [21:21]
                      tcp_over_ipv6_checksum_en                               :  1, // [20:20]
                      tcp_over_ipv4_checksum_en                               :  1, // [19:19]
                      udp_over_ipv6_checksum_en                               :  1, // [18:18]
                      udp_over_ipv4_checksum_en                               :  1, // [17:17]
                      ipv4_checksum_en                                        :  1, // [16:16]
                      data_length                                             : 16; // [15:0]
             uint32_t vdev_id                                                 :  8, // [31:24]
                      reserved_5a                                             : 11, // [23:13]
                      msdu_color                                              :  2, // [12:11]
                      pmac_id                                                 :  2, // [10:9]
                      flow_override                                           :  1, // [8:8]
                      hlos_tid                                                :  4, // [7:4]
                      who_classify_info_sel                                   :  2, // [3:2]
                      flow_override_enable                                    :  1, // [1:1]
                      hlos_tid_overwrite                                      :  1; // [0:0]
             uint32_t reserved_6a                                             :  7, // [31:25]
                      index_lookup_override                                   :  1, // [24:24]
                      cache_set_num                                           :  4, // [23:20]
                      search_index                                            : 20; // [19:0]
             uint32_t looping_count                                           :  4, // [31:28]
                      ring_id                                                 :  8, // [27:20]
                      reserved_7a                                             : 20; // [19:0]
#endif
};


/* Description		BUF_ADDR_INFO

			Details of the physical address for a single buffer containing
			 the entire MSDU or an MSDU extension descriptor. 
			It also contains return ownership info as well as some meta
			 data for SW related to this buffer.
			
			In case of Buf_or_ext_desc_type indicating 'MSDU_buffer', 
			this address indicates the start of the meta data that is
			 preceding the actual packet data.
			The start of the actual packet data is provided by field: 
			Packet_offset
*/


/* Description		BUFFER_ADDR_31_0

			Address (lower 32 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET                          0x00000000
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_LSB                             0
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MSB                             31
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MASK                            0xffffffff


/* Description		BUFFER_ADDR_39_32

			Address (upper 8 bits) of the MSDU buffer OR MSDU_EXTENSION
			 descriptor OR Link Descriptor
			
			In case of 'NULL' pointer, this field is set to 0
			<legal all>
*/

#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET                         0x00000004
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_LSB                            0
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MSB                            7
#define TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MASK                           0x000000ff


/* Description		RETURN_BUFFER_MANAGER

			Consumer: WBM
			Producer: SW/FW
			
			In case of 'NULL' pointer, this field is set to 0
			
			Indicates to which buffer manager the buffer OR MSDU_EXTENSION
			 descriptor OR link descriptor that is being pointed to 
			shall be returned after the frame has been processed. It
			 is used by WBM for routing purposes.
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			 to the WMB buffer idle list
			<enum 1 WBM_CHIP0_IDLE_DESC_LIST> This buffer shall be returned
			 to the WBM idle link descriptor idle list, where the chip
			 0 WBM is chosen in case of a multi-chip config
			<enum 2 WBM_CHIP1_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 1 WBM idle link descriptor idle list
			<enum 3 WBM_CHIP2_IDLE_DESC_LIST> This buffer shall be returned
			 to the chip 2 WBM idle link descriptor idle list
			<enum 12 WBM_CHIP3_IDLE_DESC_LIST> This buffer shall be 
			returned to chip 3 WBM idle link descriptor idle list
			<enum 4 FW_BM> This buffer shall be returned to the FW
			<enum 5 SW0_BM> This buffer shall be returned to the SW, 
			ring 0
			<enum 6 SW1_BM> This buffer shall be returned to the SW, 
			ring 1
			<enum 7 SW2_BM> This buffer shall be returned to the SW, 
			ring 2
			<enum 8 SW3_BM> This buffer shall be returned to the SW, 
			ring 3
			<enum 9 SW4_BM> This buffer shall be returned to the SW, 
			ring 4
			<enum 10 SW5_BM> This buffer shall be returned to the SW, 
			ring 5
			<enum 11 SW6_BM> This buffer shall be returned to the SW, 
			ring 6
			
			<legal 0-12>
*/

#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET                     0x00000004
#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB                        8
#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MSB                        11
#define TCL_DATA_CMD_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK                       0x00000f00


/* Description		SW_BUFFER_COOKIE

			Cookie field exclusively used by SW. 
			
			In case of 'NULL' pointer, this field is set to 0
			
			HW ignores the contents, accept that it passes the programmed
			 value on to other descriptors together with the physical
			 address 
			
			Field can be used by SW to for example associate the buffers
			 physical address with the virtual address
			The bit definitions as used by SW are within SW HLD specification
			
			
			NOTE1:
			The three most significant bits can have a special meaning
			 in case this struct is embedded in a TX_MPDU_DETAILS STRUCT, 
			and field transmit_bw_restriction is set
			
			In case of NON punctured transmission:
			Sw_buffer_cookie[19:17] = 3'b000: 20 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b001: 40 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b010: 80 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b011: 160 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b101: 240 MHz TX only
			Sw_buffer_cookie[19:17] = 3'b100: 320 MHz TX only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			In case of punctured transmission:
			Sw_buffer_cookie[19:16] = 4'b0000: pattern 0 only
			Sw_buffer_cookie[19:16] = 4'b0001: pattern 1 only
			Sw_buffer_cookie[19:16] = 4'b0010: pattern 2 only
			Sw_buffer_cookie[19:16] = 4'b0011: pattern 3 only
			Sw_buffer_cookie[19:16] = 4'b0100: pattern 4 only
			Sw_buffer_cookie[19:16] = 4'b0101: pattern 5 only
			Sw_buffer_cookie[19:16] = 4'b0110: pattern 6 only
			Sw_buffer_cookie[19:16] = 4'b0111: pattern 7 only
			Sw_buffer_cookie[19:16] = 4'b1000: pattern 8 only
			Sw_buffer_cookie[19:16] = 4'b1001: pattern 9 only
			Sw_buffer_cookie[19:16] = 4'b1010: pattern 10 only
			Sw_buffer_cookie[19:16] = 4'b1011: pattern 11 only
			Sw_buffer_cookie[19:18] = 2'b11: reserved
			
			Note: a punctured transmission is indicated by the presence
			 of TLV TX_PUNCTURE_SETUP embedded in the scheduler TLV
			
			<legal all>
*/

#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET                          0x00000004
#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_LSB                             12
#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MSB                             31
#define TCL_DATA_CMD_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MASK                            0xfffff000


/* Description		TCL_CMD_TYPE

			This field is used to select the type of TCL Command decriptor
			 that is queued by SW/FW. For 'TCL_DATA_CMD' this has to
			 be 0.
			<legal 0>
*/

#define TCL_DATA_CMD_TCL_CMD_TYPE_OFFSET                                            0x00000008
#define TCL_DATA_CMD_TCL_CMD_TYPE_LSB                                               0
#define TCL_DATA_CMD_TCL_CMD_TYPE_MSB                                               0
#define TCL_DATA_CMD_TCL_CMD_TYPE_MASK                                              0x00000001


/* Description		BUF_OR_EXT_DESC_TYPE

			<enum 0 MSDU_buffer> The address points to an MSDU buffer. 
			
			<enum 1 extension_descriptor> The address points to an MSDU
			 link extension descriptor
			< legal all>
*/

#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_OFFSET                                    0x00000008
#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_LSB                                       1
#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_MSB                                       1
#define TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_MASK                                      0x00000002


/* Description		BANK_ID

			This is used to select one of the TCL register banks for
			 fields removed from 'TCL_DATA_CMD' that do not change often
			 within one virtual device or a set of virtual devices:
			EPD
			encap_type
			Encrypt_type
			src_buffer_swap
			Link_meta_swap
			Search_type
			AddrX_en
			AddrY_en
			DSCP_TID_TABLE_NUM
			mesh_enable
*/

#define TCL_DATA_CMD_BANK_ID_OFFSET                                                 0x00000008
#define TCL_DATA_CMD_BANK_ID_LSB                                                    2
#define TCL_DATA_CMD_BANK_ID_MSB                                                    7
#define TCL_DATA_CMD_BANK_ID_MASK                                                   0x000000fc


/* Description		TX_NOTIFY_FRAME

			TCL copies this value to 'TQM_ENTRANCE_RING' field FW_tx_notify_frame.
			
			Note: TCL can also have CCE/LCE rules to set 'Tx_notify_frame.' 
			TCL shall have a register to choose the notify type in case
			 of a conflict between the two settings.
*/

#define TCL_DATA_CMD_TX_NOTIFY_FRAME_OFFSET                                         0x00000008
#define TCL_DATA_CMD_TX_NOTIFY_FRAME_LSB                                            8
#define TCL_DATA_CMD_TX_NOTIFY_FRAME_MSB                                            10
#define TCL_DATA_CMD_TX_NOTIFY_FRAME_MASK                                           0x00000700


/* Description		HEADER_LENGTH_READ_SEL

			This field is used to select the per 'encap_type' register
			 set for MSDU header read length.
			0: set 0 header read length register
			1: set 1 header read length register
			<legal all>
*/

#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_OFFSET                                  0x00000008
#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_LSB                                     11
#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_MSB                                     11
#define TCL_DATA_CMD_HEADER_LENGTH_READ_SEL_MASK                                    0x00000800


/* Description		BUFFER_TIMESTAMP

			Field only valid when 'Buffer_timestamp_valid ' is set.
			
			Frame system entrance timestamp. The timestamp is related
			 to the global system timer
			
			Generally the first module (SW, TCL or TQM). that sees this
			 frame and this timestamp field is not valid, shall fill
			 in this field.
			
			Timestamp in units determined by the UMCMN 'TX_TIMESTAMP_RESOLUTION_SELECT' 
			register
			
*/

#define TCL_DATA_CMD_BUFFER_TIMESTAMP_OFFSET                                        0x00000008
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_LSB                                           12
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_MSB                                           30
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_MASK                                          0x7ffff000


/* Description		BUFFER_TIMESTAMP_VALID

			When set, the Buffer_timestamp field contains valid info.
			
*/

#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_OFFSET                                  0x00000008
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_LSB                                     31
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_MSB                                     31
#define TCL_DATA_CMD_BUFFER_TIMESTAMP_VALID_MASK                                    0x80000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define TCL_DATA_CMD_RESERVED_3A_OFFSET                                             0x0000000c
#define TCL_DATA_CMD_RESERVED_3A_LSB                                                0
#define TCL_DATA_CMD_RESERVED_3A_MSB                                                15
#define TCL_DATA_CMD_RESERVED_3A_MASK                                               0x0000ffff


/* Description		TCL_CMD_NUMBER

			This number can be used by SW to track, identify and link
			 the created commands with the command statuses
			
			Is set to the value 'TCL_CMD_Number' of the related TCL_DATA
			 command
			<legal all> 
*/

#define TCL_DATA_CMD_TCL_CMD_NUMBER_OFFSET                                          0x0000000c
#define TCL_DATA_CMD_TCL_CMD_NUMBER_LSB                                             16
#define TCL_DATA_CMD_TCL_CMD_NUMBER_MSB                                             31
#define TCL_DATA_CMD_TCL_CMD_NUMBER_MASK                                            0xffff0000


/* Description		DATA_LENGTH

			Valid Data length in bytes. 
			
			MSDU length in case of direct descriptor.
			Length of link extension descriptor in case of Link extension
			 descriptor. This is used to know the size of Metadata.
			<legal all>
*/

#define TCL_DATA_CMD_DATA_LENGTH_OFFSET                                             0x00000010
#define TCL_DATA_CMD_DATA_LENGTH_LSB                                                0
#define TCL_DATA_CMD_DATA_LENGTH_MSB                                                15
#define TCL_DATA_CMD_DATA_LENGTH_MASK                                               0x0000ffff


/* Description		IPV4_CHECKSUM_EN

			OLE related control
			Enable IPv4 checksum replacement
*/

#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_OFFSET                                        0x00000010
#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_LSB                                           16
#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_MSB                                           16
#define TCL_DATA_CMD_IPV4_CHECKSUM_EN_MASK                                          0x00010000


/* Description		UDP_OVER_IPV4_CHECKSUM_EN

			OLE related control
			Enable UDP over IPv4 checksum replacement.  UDP checksum
			 over IPv4 is optional for TCP/IP stacks.
*/

#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_LSB                                  17
#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_MSB                                  17
#define TCL_DATA_CMD_UDP_OVER_IPV4_CHECKSUM_EN_MASK                                 0x00020000


/* Description		UDP_OVER_IPV6_CHECKSUM_EN

			OLE related control
			Enable UDP over IPv6 checksum replacement.  UDP checksum
			 over IPv6 is mandatory for TCP/IP stacks.
*/

#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_LSB                                  18
#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_MSB                                  18
#define TCL_DATA_CMD_UDP_OVER_IPV6_CHECKSUM_EN_MASK                                 0x00040000


/* Description		TCP_OVER_IPV4_CHECKSUM_EN

			OLE related control
			Enable TCP checksum over IPv4 replacement
*/

#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_LSB                                  19
#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_MSB                                  19
#define TCL_DATA_CMD_TCP_OVER_IPV4_CHECKSUM_EN_MASK                                 0x00080000


/* Description		TCP_OVER_IPV6_CHECKSUM_EN

			OLE related control
			Enable TCP checksum over IPv6 replacement
*/

#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET                               0x00000010
#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_LSB                                  20
#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_MSB                                  20
#define TCL_DATA_CMD_TCP_OVER_IPV6_CHECKSUM_EN_MASK                                 0x00100000


/* Description		TO_FW

			Forward packet to FW along with classification result. The
			 packet will not be forward to TQM when this bit is set
			
			1'b0: Use classification result to forward the packet.
			1'b1: Override classification result and forward packet 
			only to FW.
			<legal all>
*/

#define TCL_DATA_CMD_TO_FW_OFFSET                                                   0x00000010
#define TCL_DATA_CMD_TO_FW_LSB                                                      21
#define TCL_DATA_CMD_TO_FW_MSB                                                      21
#define TCL_DATA_CMD_TO_FW_MASK                                                     0x00200000


/* Description		RESERVED_4A

			<legal 0>
*/

#define TCL_DATA_CMD_RESERVED_4A_OFFSET                                             0x00000010
#define TCL_DATA_CMD_RESERVED_4A_LSB                                                22
#define TCL_DATA_CMD_RESERVED_4A_MSB                                                22
#define TCL_DATA_CMD_RESERVED_4A_MASK                                               0x00400000


/* Description		PACKET_OFFSET

			Packet offset from Metadata in case of direct buffer descriptor. 
			This field is valid when Buf_or_ext_desc_type is reset(= 
			0).
			<legal all>
*/

#define TCL_DATA_CMD_PACKET_OFFSET_OFFSET                                           0x00000010
#define TCL_DATA_CMD_PACKET_OFFSET_LSB                                              23
#define TCL_DATA_CMD_PACKET_OFFSET_MSB                                              31
#define TCL_DATA_CMD_PACKET_OFFSET_MASK                                             0xff800000


/* Description		HLOS_TID_OVERWRITE

			When set, TCL shall ignore the IP DSCP and VLAN PCP fields
			 and use HLOS_TID as the final TID. Otherwise TCL shall 
			consider the DSCP and PCP fields as well as HLOS_TID and
			 choose a final TID based on the configured priority 
			<legal all>
*/

#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_OFFSET                                      0x00000014
#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_LSB                                         0
#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_MSB                                         0
#define TCL_DATA_CMD_HLOS_TID_OVERWRITE_MASK                                        0x00000001


/* Description		FLOW_OVERRIDE_ENABLE

			TCL uses this to select the flow pointer from the peer table, 
			which can be overridden by SW for pre-encrypted raw WiFi
			 packets that cannot be parsed for UDP or for other MLO 
			or enterprise use cases:
			<enum 0 FP_PARSE_IP> Use the flow-pointer based on parsing
			 the IPv4 or IPv6 header.
			<enum 1 FP_USE_OVERRIDE> Use the who_classify_info_sel and
			 flow_override fields to select the flow-pointer.
			<legal all>
*/

#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_OFFSET                                    0x00000014
#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_LSB                                       1
#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_MSB                                       1
#define TCL_DATA_CMD_FLOW_OVERRIDE_ENABLE_MASK                                      0x00000002


/* Description		WHO_CLASSIFY_INFO_SEL

			Field only valid when flow_override_enable is set to FP_USE_OVERRIDE.
			
			 
			This field is used to select  one of the 'WHO_CLASSIFY_INFO's
			 in the peer table in case more than 2 flows are mapped 
			to a single TID.
			0: To choose Flow 0 and 1 of any TID use this value.
			1: To choose Flow 2 and 3 of any TID use this value.
			2: To choose Flow 4 and 5 of any TID use this value.
			3: To choose Flow 6 and 7 of any TID use this value.
			
			If who_classify_info sel is not in sync with the num_tx_classify_info
			 field from address search, then TCL will set 'who_classify_info_sel' 
			to 0 use flows 0 and 1. 
			<legal all>
*/

#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_OFFSET                                   0x00000014
#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_LSB                                      2
#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_MSB                                      3
#define TCL_DATA_CMD_WHO_CLASSIFY_INFO_SEL_MASK                                     0x0000000c


/* Description		HLOS_TID

			HLOS MSDU priority
			
			Field is used when HLOS_TID_overwrite is set or flow_override_enable
			 is set to FP_USE_OVERRIDE.
			
			Field is also used when HLOS_TID_overwrite is not set and
			 DSCP/PCP is not available in the packet.
			<legal all>
*/

#define TCL_DATA_CMD_HLOS_TID_OFFSET                                                0x00000014
#define TCL_DATA_CMD_HLOS_TID_LSB                                                   4
#define TCL_DATA_CMD_HLOS_TID_MSB                                                   7
#define TCL_DATA_CMD_HLOS_TID_MASK                                                  0x000000f0


/* Description		FLOW_OVERRIDE

			Field only valid when flow_override_enable is set to FP_USE_OVERRIDE.
			
			
			TCL uses this to select the flow pointer from the peer table, 
			which can be overridden by SW for pre-encrypted raw WiFi
			 packets that cannot be parsed for UDP or for other MLO 
			or enterprise use cases:
			<enum 0 FP_USE_NON_UDP> Use the non-UDP flow pointer (flow
			 0)
			<enum 1 FP_USE_UDP> Use the UDP flow pointer (flow 1)
			
			<legal all>
*/

#define TCL_DATA_CMD_FLOW_OVERRIDE_OFFSET                                           0x00000014
#define TCL_DATA_CMD_FLOW_OVERRIDE_LSB                                              8
#define TCL_DATA_CMD_FLOW_OVERRIDE_MSB                                              8
#define TCL_DATA_CMD_FLOW_OVERRIDE_MASK                                             0x00000100


/* Description		PMAC_ID

			TCL uses this PMAC_ID in address search, i.e, while finding
			 matching entry for the packet in AST corresponding to given
			 PMAC_ID
			If PMAC ID is all 1s (=> value 3), it indicates wildcard
			 match for any PMAC
			<legal 0-3>
*/

#define TCL_DATA_CMD_PMAC_ID_OFFSET                                                 0x00000014
#define TCL_DATA_CMD_PMAC_ID_LSB                                                    9
#define TCL_DATA_CMD_PMAC_ID_MSB                                                    10
#define TCL_DATA_CMD_PMAC_ID_MASK                                                   0x00000600


/* Description		MSDU_COLOR

			Consumer: TQM
			Producer: SW
			
			TCL copies this value to 'TQM_ENTRANCE_RING' in the structure
			 'TX_MSDU_DETAILS' field msdu_color.
			
			When set, TQM will check the color and choose the color 
			based threshold with which it will decide if the MSDU has
			 to be dropped.
			
			<enum 0 MSDU_COLORLESS> MSDUs which have no color and TQM
			 uses legacy drop thresholds for these MSDUs.
			<enum 1 MSDU_COLOR_GREEN>
			<enum 2 MSDU_COLOR_YELLOW>
			<enum 3 MSDU_COLOR_RED>
			<legal 0-3>
*/

#define TCL_DATA_CMD_MSDU_COLOR_OFFSET                                              0x00000014
#define TCL_DATA_CMD_MSDU_COLOR_LSB                                                 11
#define TCL_DATA_CMD_MSDU_COLOR_MSB                                                 12
#define TCL_DATA_CMD_MSDU_COLOR_MASK                                                0x00001800


/* Description		RESERVED_5A

			<legal 0>
*/

#define TCL_DATA_CMD_RESERVED_5A_OFFSET                                             0x00000014
#define TCL_DATA_CMD_RESERVED_5A_LSB                                                13
#define TCL_DATA_CMD_RESERVED_5A_MSB                                                23
#define TCL_DATA_CMD_RESERVED_5A_MASK                                               0x00ffe000


/* Description		VDEV_ID

			Virtual device ID to check against the address search entry
			 to avoid security issues from transmitting packets from
			 an incorrect virtual device
			<legal all>
*/

#define TCL_DATA_CMD_VDEV_ID_OFFSET                                                 0x00000014
#define TCL_DATA_CMD_VDEV_ID_LSB                                                    24
#define TCL_DATA_CMD_VDEV_ID_MSB                                                    31
#define TCL_DATA_CMD_VDEV_ID_MASK                                                   0xff000000


/* Description		SEARCH_INDEX

			The index that will be used for index based address or flow
			 search. The field is valid when 'search_type' is  1 or 
			2. 
			<legal all>
*/

#define TCL_DATA_CMD_SEARCH_INDEX_OFFSET                                            0x00000018
#define TCL_DATA_CMD_SEARCH_INDEX_LSB                                               0
#define TCL_DATA_CMD_SEARCH_INDEX_MSB                                               19
#define TCL_DATA_CMD_SEARCH_INDEX_MASK                                              0x000fffff


/* Description		CACHE_SET_NUM

			Cache set number that should be used to cache the index 
			based search results, for address and flow search. This 
			value should be equal to LSB four bits of the hash value
			 of match data, in case of search index points to an entry
			 which may be used in content based search also. The value
			 can be anything when the entry pointed by search index 
			will not be used for content based search. 
			<legal all>
*/

#define TCL_DATA_CMD_CACHE_SET_NUM_OFFSET                                           0x00000018
#define TCL_DATA_CMD_CACHE_SET_NUM_LSB                                              20
#define TCL_DATA_CMD_CACHE_SET_NUM_MSB                                              23
#define TCL_DATA_CMD_CACHE_SET_NUM_MASK                                             0x00f00000


/* Description		INDEX_LOOKUP_OVERRIDE

			When set, address search and packet routing is forced to
			 use 'search_index' instead of following the register configuration
			 seleced by Bank_id.
			<legal all>
*/

#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_OFFSET                                   0x00000018
#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_LSB                                      24
#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_MSB                                      24
#define TCL_DATA_CMD_INDEX_LOOKUP_OVERRIDE_MASK                                     0x01000000


/* Description		RESERVED_6A

			<legal 0>
*/

#define TCL_DATA_CMD_RESERVED_6A_OFFSET                                             0x00000018
#define TCL_DATA_CMD_RESERVED_6A_LSB                                                25
#define TCL_DATA_CMD_RESERVED_6A_MSB                                                31
#define TCL_DATA_CMD_RESERVED_6A_MASK                                               0xfe000000


/* Description		RESERVED_7A

			<legal 0>
*/

#define TCL_DATA_CMD_RESERVED_7A_OFFSET                                             0x0000001c
#define TCL_DATA_CMD_RESERVED_7A_LSB                                                0
#define TCL_DATA_CMD_RESERVED_7A_MSB                                                19
#define TCL_DATA_CMD_RESERVED_7A_MASK                                               0x000fffff


/* Description		RING_ID

			The buffer pointer ring ID.
			0 refers to the IDLE ring
			1 - N refers to other rings
			
			Helps with debugging when dumping ring contents.
			<legal all>
*/

#define TCL_DATA_CMD_RING_ID_OFFSET                                                 0x0000001c
#define TCL_DATA_CMD_RING_ID_LSB                                                    20
#define TCL_DATA_CMD_RING_ID_MSB                                                    27
#define TCL_DATA_CMD_RING_ID_MASK                                                   0x0ff00000


/* Description		LOOPING_COUNT

			A count value that indicates the number of times the producer
			 of entries into the Ring has looped around the ring.
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

#define TCL_DATA_CMD_LOOPING_COUNT_OFFSET                                           0x0000001c
#define TCL_DATA_CMD_LOOPING_COUNT_LSB                                              28
#define TCL_DATA_CMD_LOOPING_COUNT_MSB                                              31
#define TCL_DATA_CMD_LOOPING_COUNT_MASK                                             0xf0000000



#endif   // TCL_DATA_CMD

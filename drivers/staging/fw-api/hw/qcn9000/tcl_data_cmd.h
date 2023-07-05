/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0-1	struct buffer_addr_info buf_addr_info;
//	2	buf_or_ext_desc_type[0], epd[1], encap_type[3:2], encrypt_type[7:4], src_buffer_swap[8], link_meta_swap[9], tqm_no_drop[10], reserved_2a[11], search_type[13:12], addrx_en[14], addry_en[15], tcl_cmd_number[31:16]
//	3	data_length[15:0], ipv4_checksum_en[16], udp_over_ipv4_checksum_en[17], udp_over_ipv6_checksum_en[18], tcp_over_ipv4_checksum_en[19], tcp_over_ipv6_checksum_en[20], to_fw[21], reserved_3a[22], packet_offset[31:23]
//	4	buffer_timestamp[18:0], buffer_timestamp_valid[19], reserved_4a[20], hlos_tid_overwrite[21], hlos_tid[25:22], lmac_id[27:26], reserved_4b[31:28]
//	5	dscp_tid_table_num[5:0], search_index[25:6], cache_set_num[29:26], mesh_enable[31:30]
//	6	reserved_6a[19:0], ring_id[27:20], looping_count[31:28]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_TCL_DATA_CMD 7

struct tcl_data_cmd {
    struct            buffer_addr_info                       buf_addr_info;
             uint32_t buf_or_ext_desc_type            :  1, //[0]
                      epd                             :  1, //[1]
                      encap_type                      :  2, //[3:2]
                      encrypt_type                    :  4, //[7:4]
                      src_buffer_swap                 :  1, //[8]
                      link_meta_swap                  :  1, //[9]
                      tqm_no_drop                     :  1, //[10]
                      reserved_2a                     :  1, //[11]
                      search_type                     :  2, //[13:12]
                      addrx_en                        :  1, //[14]
                      addry_en                        :  1, //[15]
                      tcl_cmd_number                  : 16; //[31:16]
             uint32_t data_length                     : 16, //[15:0]
                      ipv4_checksum_en                :  1, //[16]
                      udp_over_ipv4_checksum_en       :  1, //[17]
                      udp_over_ipv6_checksum_en       :  1, //[18]
                      tcp_over_ipv4_checksum_en       :  1, //[19]
                      tcp_over_ipv6_checksum_en       :  1, //[20]
                      to_fw                           :  1, //[21]
                      reserved_3a                     :  1, //[22]
                      packet_offset                   :  9; //[31:23]
             uint32_t buffer_timestamp                : 19, //[18:0]
                      buffer_timestamp_valid          :  1, //[19]
                      reserved_4a                     :  1, //[20]
                      hlos_tid_overwrite              :  1, //[21]
                      hlos_tid                        :  4, //[25:22]
                      lmac_id                         :  2, //[27:26]
                      reserved_4b                     :  4; //[31:28]
             uint32_t dscp_tid_table_num              :  6, //[5:0]
                      search_index                    : 20, //[25:6]
                      cache_set_num                   :  4, //[29:26]
                      mesh_enable                     :  2; //[31:30]
             uint32_t reserved_6a                     : 20, //[19:0]
                      ring_id                         :  8, //[27:20]
                      looping_count                   :  4; //[31:28]
};

/*

struct buffer_addr_info buf_addr_info
			
			Details of the physical address for a single buffer
			
			It also contains return ownership info as well as some
			meta data for SW related to this buffer.
			
			
			
			In case of Buf_or_ext_desc_type indicating
			'MSDU_buffer', this address indicates the start of the meta
			data that is preceding the actual packet data.
			
			The start of the actual packet data is provided by
			field: Packet_offset

buf_or_ext_desc_type
			
			<enum 0 MSDU_buffer> The address points to an MSDU
			buffer. 
			
			<enum 1 extension_descriptor> The address points to an
			MSDU link extension descriptor
			
			< legal all>

epd
			
			When this bit is set then input packet is an EPD type
			
			<legal all>

encap_type
			
			Indicates the encapsulation that HW will perform:
			
			<enum 0 RAW> No encapsulation
			
			<enum 1 Native_WiFi>
			
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses
			SNAP/LLC)
			
			<enum 3 802_3> DO NOT USE. Indicate Ethernet
			
			
			
			Used by the OLE during encapsulation.
			
			<legal all>

encrypt_type
			
			Field only valid for encap_type: RAW 
			
			
			
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

src_buffer_swap
			
			Treats source memory (packet buffer) organization as
			big-endian. The packets are read and byte swapped.
			
			1'b0: Source memory is little endian
			
			1'b1: Source memory is big endian
			
			<legal all>

link_meta_swap
			
			Treats link descriptor and Metadata as big-endian. The
			link descriptor/Metadata is read and byte swapped.
			
			1'b0: Memory is little endian
			
			1'b1: Memory is big endian
			
			<legal all>

tqm_no_drop
			
			This bit is used to stop TQM from dropping MSDUs while
			adding them to MSDU flows1'b1: Do not drop MSDU when any of
			the threshold value is met while adding MSDU in a flow1'b1:
			Drop MSDU when any of the threshold value is met while
			adding MSDU in a flow
			
			<legal all>

reserved_2a
			
			<legal 0>

search_type
			
			Search type select 
			
			
			
			<enum 0 Normal_search> Address and flow search will use
			packet contents
			
			<enum 1 Index_based_address_search> Address search will
			
			<enum 2 Index_based_flow_search> Flow search will use
			'search_index', and address search will use packet contents
			
			<legal 0-2>

addrx_en
			
			Address X search enable in ASE
			
			1'b0: Search disable
			
			1'b1: Search Enable
			
			<legal all>

addry_en
			
			Address Y search enable in ASE
			
			1'b0: Search disable
			
			1'b1: Search Enable
			
			<legal all>

tcl_cmd_number
			
			This number can be used by SW to track, identify and
			link the created commands with the command statuses
			
			
			
			Is set to the value 'TCL_CMD_Number' of the related
			TCL_DATA command
			
			<legal all> 

data_length
			
			Valid Data length in bytes. 
			
			
			
			MSDU length in case of direct descriptor.
			
			Length of link extension descriptor in case of Link
			extension descriptor. This is used to know the size of
			Metadata.
			
			<legal all>

ipv4_checksum_en
			
			OLE related control
			
			Enable IPv4 checksum replacement

udp_over_ipv4_checksum_en
			
			OLE related control
			
			Enable UDP over IPv4 checksum replacement.  UDP checksum
			over IPv4 is optional for TCP/IP stacks.

udp_over_ipv6_checksum_en
			
			OLE related control
			
			Enable UDP over IPv6 checksum replacement.  UDP checksum
			over IPv6 is mandatory for TCP/IP stacks.

tcp_over_ipv4_checksum_en
			
			OLE related control
			
			Enable TCP checksum over IPv4 replacement

tcp_over_ipv6_checksum_en
			
			OLE related control
			
			Enable TCP checksum over IPv6 replacement

to_fw
			
			Forward packet to FW along with classification result.
			The packet will not be forward to TQM when this bit is set
			
			
			
			1'b0: Use classification result to forward the packet.
			
			1'b1: Override classification result and forward packet
			only to FW.
			
			<legal all>

reserved_3a
			
			<legal 0>

packet_offset
			
			Packet offset from Metadata in case of direct buffer
			descriptor. This field is valid when Buf_or_ext_desc_type is
			reset(= 0).
			
			<legal all>

buffer_timestamp
			
			Field only valid when 'Buffer_timestamp_valid ' is set.
			
			
			
			Frame system entrance timestamp. The timestamp is
			related to the global system timer
			
			
			
			Generally the first module (SW, TCL or TQM). that sees
			this frame and this timestamp field is not valid, shall fill
			in this field.
			
			
			
			Timestamp in units of 1024 us

buffer_timestamp_valid
			
			When set, the Buffer_timestamp field contains valid
			info.

reserved_4a
			
			<legal 0>

hlos_tid_overwrite
			
			When set, TCL shall ignore the IP DSCP and VLAN PCP
			fields and use HLOS_TID as the final TID. Otherwise TCL
			shall consider the DSCP and PCP fields as well as HLOS_TID
			and choose a final TID based on the configured priority 
			
			<legal all>

hlos_tid
			
			HLOS MSDU priority
			
			
			
			Field is used when HLOS_TID_overwrite is set.
			
			
			
			Field is also used when HLOS_TID_overwrite is not set
			and DSCP/PCP is not available in the packet
			
			<legal all>

lmac_id
			
			TCL uses this LMAC_ID in address search, i.e, while
			finding matching entry for the packet in AST corresponding
			to given LMAC_ID
			
			If LMAC ID is all 1s (=> value 3), it indicates wildcard
			match for any MAC
			
			<legal 0-3>

reserved_4b
			
			<legal 0>

dscp_tid_table_num
			
			DSCP to TID mapping table number that need to be used
			for the MSDU, should be specified using this field
			
			<legal all>

search_index
			
			The index that will be used for index based address or
			flow search. The field is valid when 'search_type' is  1 or
			2. 
			
			<legal all>

cache_set_num
			
			Cache set number that should be used to cache the index
			based search results, for address and flow search. This
			value should be equal to LSB four bits of the hash value of
			match data, in case of search index points to an entry which
			may be used in content based search also. The value can be
			anything when the entry pointed by search index will not be
			used for content based search. 
			
			<legal all>

mesh_enable
			
			If set to a non-zero value:
			
			* For raw WiFi frames, this indicates transmission to a
			mesh STA, enabling the interpretation of the 'Mesh Control
			Present' bit (bit 8) of QoS Control (otherwise this bit is
			ignored). The interpretation of the A-MSDU 'Length' field is
			decided by the e-numerations below.
			
			* For native WiFi frames, this indicates that a 'Mesh
			Control' field is present between the header and the LLC.
			The three non-zero values are interchangeable.
			
			
			
			<enum 0 MESH_DISABLE>
			
			<enum 1 MESH_Q2Q> A-MSDU 'Length' is big endian and
			includes the length of Mesh Control.
			
			<enum 2 MESH_11S_BE> A-MSDU 'Length' is big endian and
			excludes the length of Mesh Control.
			
			<enum 3 MESH_11S_LE> A-MSDU 'Length' is little endian
			and excludes the length of Mesh Control. This is
			802.11s-compliant.

reserved_6a
			
			<legal 0>

ring_id
			
			The buffer pointer ring ID.
			
			0 refers to the IDLE ring
			
			1 - N refers to other rings
			
			
			
			Helps with debugging when dumping ring contents.
			
			<legal all>

looping_count
			
			A count value that indicates the number of times the
			producer of entries into the Ring has looped around the
			ring.
			
			At initialization time, this value is set to 0. On the
			first loop, this value is set to 1. After the max value is
			reached allowed by the number of bits for this field, the
			count value continues with 0 again.
			
			
			
			In case SW is the consumer of the ring entries, it can
			use this field to figure out up to where the producer of
			entries has created new entries. This eliminates the need to
			check where the head pointer' of the ring is located once
			the SW starts processing an interrupt indicating that new
			entries have been put into this ring...
			
			
			
			Also note that SW if it wants only needs to look at the
			LSB bit of this count value.
			
			<legal all>
*/


 /* EXTERNAL REFERENCE : struct buffer_addr_info buf_addr_info */ 


/* Description		TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0
			
			Address (lower 32 bits) of the MSDU buffer OR
			MSDU_EXTENSION descriptor OR Link Descriptor
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			<legal all>
*/
#define TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET         0x00000000
#define TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_LSB            0
#define TCL_DATA_CMD_0_BUF_ADDR_INFO_BUFFER_ADDR_31_0_MASK           0xffffffff

/* Description		TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32
			
			Address (upper 8 bits) of the MSDU buffer OR
			MSDU_EXTENSION descriptor OR Link Descriptor
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			<legal all>
*/
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET        0x00000004
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_LSB           0
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_BUFFER_ADDR_39_32_MASK          0x000000ff

/* Description		TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER
			
			Consumer: WBM
			
			Producer: SW/FW
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			
			
			Indicates to which buffer manager the buffer OR
			MSDU_EXTENSION descriptor OR link descriptor that is being
			pointed to shall be returned after the frame has been
			processed. It is used by WBM for routing purposes.
			
			
			
			<enum 0 WBM_IDLE_BUF_LIST> This buffer shall be returned
			to the WMB buffer idle list
			
			<enum 1 WBM_IDLE_DESC_LIST> This buffer shall be
			returned to the WMB idle link descriptor idle list
			
			<enum 2 FW_BM> This buffer shall be returned to the FW
			
			<enum 3 SW0_BM> This buffer shall be returned to the SW,
			ring 0
			
			<enum 4 SW1_BM> This buffer shall be returned to the SW,
			ring 1
			
			<enum 5 SW2_BM> This buffer shall be returned to the SW,
			ring 2
			
			<enum 6 SW3_BM> This buffer shall be returned to the SW,
			ring 3
			
			<enum 7 SW4_BM> This buffer shall be returned to the SW,
			ring 4
			
			
			
			<legal all>
*/
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_OFFSET    0x00000004
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB       8
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK      0x00000700

/* Description		TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE
			
			Cookie field exclusively used by SW. 
			
			
			
			In case of 'NULL' pointer, this field is set to 0
			
			
			
			HW ignores the contents, accept that it passes the
			programmed value on to other descriptors together with the
			physical address 
			
			
			
			Field can be used by SW to for example associate the
			buffers physical address with the virtual address
			
			The bit definitions as used by SW are within SW HLD
			specification
			
			
			
			NOTE:
			
			The three most significant bits can have a special
			meaning in case this struct is embedded in a TX_MPDU_DETAILS
			STRUCT, and field transmit_bw_restriction is set
			
			
			
			In case of NON punctured transmission:
			
			Sw_buffer_cookie[20:19] = 2'b00: 20 MHz TX only
			
			Sw_buffer_cookie[20:19] = 2'b01: 40 MHz TX only
			
			Sw_buffer_cookie[20:19] = 2'b10: 80 MHz TX only
			
			Sw_buffer_cookie[20:19] = 2'b11: 160 MHz TX only
			
			
			
			In case of punctured transmission:
			
			Sw_buffer_cookie[20:18] = 3'b000: pattern 0 only
			
			Sw_buffer_cookie[20:18] = 3'b001: pattern 1 only
			
			Sw_buffer_cookie[20:18] = 3'b010: pattern 2 only
			
			Sw_buffer_cookie[20:18] = 3'b011: pattern 3 only
			
			Sw_buffer_cookie[20:18] = 3'b100: pattern 4 only
			
			Sw_buffer_cookie[20:18] = 3'b101: pattern 5 only
			
			Sw_buffer_cookie[20:18] = 3'b110: pattern 6 only
			
			Sw_buffer_cookie[20:18] = 3'b111: pattern 7 only
			
			
			
			Note: a punctured transmission is indicated by the
			presence of TLV TX_PUNCTURE_SETUP embedded in the scheduler
			TLV
			
			
			
			<legal all>
*/
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE_OFFSET         0x00000004
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE_LSB            11
#define TCL_DATA_CMD_1_BUF_ADDR_INFO_SW_BUFFER_COOKIE_MASK           0xfffff800

/* Description		TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE
			
			<enum 0 MSDU_buffer> The address points to an MSDU
			buffer. 
			
			<enum 1 extension_descriptor> The address points to an
			MSDU link extension descriptor
			
			< legal all>
*/
#define TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET                   0x00000008
#define TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB                      0
#define TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK                     0x00000001

/* Description		TCL_DATA_CMD_2_EPD
			
			When this bit is set then input packet is an EPD type
			
			<legal all>
*/
#define TCL_DATA_CMD_2_EPD_OFFSET                                    0x00000008
#define TCL_DATA_CMD_2_EPD_LSB                                       1
#define TCL_DATA_CMD_2_EPD_MASK                                      0x00000002

/* Description		TCL_DATA_CMD_2_ENCAP_TYPE
			
			Indicates the encapsulation that HW will perform:
			
			<enum 0 RAW> No encapsulation
			
			<enum 1 Native_WiFi>
			
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses
			SNAP/LLC)
			
			<enum 3 802_3> DO NOT USE. Indicate Ethernet
			
			
			
			Used by the OLE during encapsulation.
			
			<legal all>
*/
#define TCL_DATA_CMD_2_ENCAP_TYPE_OFFSET                             0x00000008
#define TCL_DATA_CMD_2_ENCAP_TYPE_LSB                                2
#define TCL_DATA_CMD_2_ENCAP_TYPE_MASK                               0x0000000c

/* Description		TCL_DATA_CMD_2_ENCRYPT_TYPE
			
			Field only valid for encap_type: RAW 
			
			
			
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
#define TCL_DATA_CMD_2_ENCRYPT_TYPE_OFFSET                           0x00000008
#define TCL_DATA_CMD_2_ENCRYPT_TYPE_LSB                              4
#define TCL_DATA_CMD_2_ENCRYPT_TYPE_MASK                             0x000000f0

/* Description		TCL_DATA_CMD_2_SRC_BUFFER_SWAP
			
			Treats source memory (packet buffer) organization as
			big-endian. The packets are read and byte swapped.
			
			1'b0: Source memory is little endian
			
			1'b1: Source memory is big endian
			
			<legal all>
*/
#define TCL_DATA_CMD_2_SRC_BUFFER_SWAP_OFFSET                        0x00000008
#define TCL_DATA_CMD_2_SRC_BUFFER_SWAP_LSB                           8
#define TCL_DATA_CMD_2_SRC_BUFFER_SWAP_MASK                          0x00000100

/* Description		TCL_DATA_CMD_2_LINK_META_SWAP
			
			Treats link descriptor and Metadata as big-endian. The
			link descriptor/Metadata is read and byte swapped.
			
			1'b0: Memory is little endian
			
			1'b1: Memory is big endian
			
			<legal all>
*/
#define TCL_DATA_CMD_2_LINK_META_SWAP_OFFSET                         0x00000008
#define TCL_DATA_CMD_2_LINK_META_SWAP_LSB                            9
#define TCL_DATA_CMD_2_LINK_META_SWAP_MASK                           0x00000200

/* Description		TCL_DATA_CMD_2_TQM_NO_DROP
			
			This bit is used to stop TQM from dropping MSDUs while
			adding them to MSDU flows1'b1: Do not drop MSDU when any of
			the threshold value is met while adding MSDU in a flow1'b1:
			Drop MSDU when any of the threshold value is met while
			adding MSDU in a flow
			
			<legal all>
*/
#define TCL_DATA_CMD_2_TQM_NO_DROP_OFFSET                            0x00000008
#define TCL_DATA_CMD_2_TQM_NO_DROP_LSB                               10
#define TCL_DATA_CMD_2_TQM_NO_DROP_MASK                              0x00000400

/* Description		TCL_DATA_CMD_2_RESERVED_2A
			
			<legal 0>
*/
#define TCL_DATA_CMD_2_RESERVED_2A_OFFSET                            0x00000008
#define TCL_DATA_CMD_2_RESERVED_2A_LSB                               11
#define TCL_DATA_CMD_2_RESERVED_2A_MASK                              0x00000800

/* Description		TCL_DATA_CMD_2_SEARCH_TYPE
			
			Search type select 
			
			
			
			<enum 0 Normal_search> Address and flow search will use
			packet contents
			
			<enum 1 Index_based_address_search> Address search will
			
			<enum 2 Index_based_flow_search> Flow search will use
			'search_index', and address search will use packet contents
			
			<legal 0-2>
*/
#define TCL_DATA_CMD_2_SEARCH_TYPE_OFFSET                            0x00000008
#define TCL_DATA_CMD_2_SEARCH_TYPE_LSB                               12
#define TCL_DATA_CMD_2_SEARCH_TYPE_MASK                              0x00003000

/* Description		TCL_DATA_CMD_2_ADDRX_EN
			
			Address X search enable in ASE
			
			1'b0: Search disable
			
			1'b1: Search Enable
			
			<legal all>
*/
#define TCL_DATA_CMD_2_ADDRX_EN_OFFSET                               0x00000008
#define TCL_DATA_CMD_2_ADDRX_EN_LSB                                  14
#define TCL_DATA_CMD_2_ADDRX_EN_MASK                                 0x00004000

/* Description		TCL_DATA_CMD_2_ADDRY_EN
			
			Address Y search enable in ASE
			
			1'b0: Search disable
			
			1'b1: Search Enable
			
			<legal all>
*/
#define TCL_DATA_CMD_2_ADDRY_EN_OFFSET                               0x00000008
#define TCL_DATA_CMD_2_ADDRY_EN_LSB                                  15
#define TCL_DATA_CMD_2_ADDRY_EN_MASK                                 0x00008000

/* Description		TCL_DATA_CMD_2_TCL_CMD_NUMBER
			
			This number can be used by SW to track, identify and
			link the created commands with the command statuses
			
			
			
			Is set to the value 'TCL_CMD_Number' of the related
			TCL_DATA command
			
			<legal all> 
*/
#define TCL_DATA_CMD_2_TCL_CMD_NUMBER_OFFSET                         0x00000008
#define TCL_DATA_CMD_2_TCL_CMD_NUMBER_LSB                            16
#define TCL_DATA_CMD_2_TCL_CMD_NUMBER_MASK                           0xffff0000

/* Description		TCL_DATA_CMD_3_DATA_LENGTH
			
			Valid Data length in bytes. 
			
			
			
			MSDU length in case of direct descriptor.
			
			Length of link extension descriptor in case of Link
			extension descriptor. This is used to know the size of
			Metadata.
			
			<legal all>
*/
#define TCL_DATA_CMD_3_DATA_LENGTH_OFFSET                            0x0000000c
#define TCL_DATA_CMD_3_DATA_LENGTH_LSB                               0
#define TCL_DATA_CMD_3_DATA_LENGTH_MASK                              0x0000ffff

/* Description		TCL_DATA_CMD_3_IPV4_CHECKSUM_EN
			
			OLE related control
			
			Enable IPv4 checksum replacement
*/
#define TCL_DATA_CMD_3_IPV4_CHECKSUM_EN_OFFSET                       0x0000000c
#define TCL_DATA_CMD_3_IPV4_CHECKSUM_EN_LSB                          16
#define TCL_DATA_CMD_3_IPV4_CHECKSUM_EN_MASK                         0x00010000

/* Description		TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN
			
			OLE related control
			
			Enable UDP over IPv4 checksum replacement.  UDP checksum
			over IPv4 is optional for TCP/IP stacks.
*/
#define TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN_LSB                 17
#define TCL_DATA_CMD_3_UDP_OVER_IPV4_CHECKSUM_EN_MASK                0x00020000

/* Description		TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN
			
			OLE related control
			
			Enable UDP over IPv6 checksum replacement.  UDP checksum
			over IPv6 is mandatory for TCP/IP stacks.
*/
#define TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN_LSB                 18
#define TCL_DATA_CMD_3_UDP_OVER_IPV6_CHECKSUM_EN_MASK                0x00040000

/* Description		TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN
			
			OLE related control
			
			Enable TCP checksum over IPv4 replacement
*/
#define TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN_LSB                 19
#define TCL_DATA_CMD_3_TCP_OVER_IPV4_CHECKSUM_EN_MASK                0x00080000

/* Description		TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN
			
			OLE related control
			
			Enable TCP checksum over IPv6 replacement
*/
#define TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN_OFFSET              0x0000000c
#define TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN_LSB                 20
#define TCL_DATA_CMD_3_TCP_OVER_IPV6_CHECKSUM_EN_MASK                0x00100000

/* Description		TCL_DATA_CMD_3_TO_FW
			
			Forward packet to FW along with classification result.
			The packet will not be forward to TQM when this bit is set
			
			
			
			1'b0: Use classification result to forward the packet.
			
			1'b1: Override classification result and forward packet
			only to FW.
			
			<legal all>
*/
#define TCL_DATA_CMD_3_TO_FW_OFFSET                                  0x0000000c
#define TCL_DATA_CMD_3_TO_FW_LSB                                     21
#define TCL_DATA_CMD_3_TO_FW_MASK                                    0x00200000

/* Description		TCL_DATA_CMD_3_RESERVED_3A
			
			<legal 0>
*/
#define TCL_DATA_CMD_3_RESERVED_3A_OFFSET                            0x0000000c
#define TCL_DATA_CMD_3_RESERVED_3A_LSB                               22
#define TCL_DATA_CMD_3_RESERVED_3A_MASK                              0x00400000

/* Description		TCL_DATA_CMD_3_PACKET_OFFSET
			
			Packet offset from Metadata in case of direct buffer
			descriptor. This field is valid when Buf_or_ext_desc_type is
			reset(= 0).
			
			<legal all>
*/
#define TCL_DATA_CMD_3_PACKET_OFFSET_OFFSET                          0x0000000c
#define TCL_DATA_CMD_3_PACKET_OFFSET_LSB                             23
#define TCL_DATA_CMD_3_PACKET_OFFSET_MASK                            0xff800000

/* Description		TCL_DATA_CMD_4_BUFFER_TIMESTAMP
			
			Field only valid when 'Buffer_timestamp_valid ' is set.
			
			
			
			Frame system entrance timestamp. The timestamp is
			related to the global system timer
			
			
			
			Generally the first module (SW, TCL or TQM). that sees
			this frame and this timestamp field is not valid, shall fill
			in this field.
			
			
			
			Timestamp in units of 1024 us
*/
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_OFFSET                       0x00000010
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_LSB                          0
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_MASK                         0x0007ffff

/* Description		TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID
			
			When set, the Buffer_timestamp field contains valid
			info.
*/
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID_OFFSET                 0x00000010
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID_LSB                    19
#define TCL_DATA_CMD_4_BUFFER_TIMESTAMP_VALID_MASK                   0x00080000

/* Description		TCL_DATA_CMD_4_RESERVED_4A
			
			<legal 0>
*/
#define TCL_DATA_CMD_4_RESERVED_4A_OFFSET                            0x00000010
#define TCL_DATA_CMD_4_RESERVED_4A_LSB                               20
#define TCL_DATA_CMD_4_RESERVED_4A_MASK                              0x00100000

/* Description		TCL_DATA_CMD_4_HLOS_TID_OVERWRITE
			
			When set, TCL shall ignore the IP DSCP and VLAN PCP
			fields and use HLOS_TID as the final TID. Otherwise TCL
			shall consider the DSCP and PCP fields as well as HLOS_TID
			and choose a final TID based on the configured priority 
			
			<legal all>
*/
#define TCL_DATA_CMD_4_HLOS_TID_OVERWRITE_OFFSET                     0x00000010
#define TCL_DATA_CMD_4_HLOS_TID_OVERWRITE_LSB                        21
#define TCL_DATA_CMD_4_HLOS_TID_OVERWRITE_MASK                       0x00200000

/* Description		TCL_DATA_CMD_4_HLOS_TID
			
			HLOS MSDU priority
			
			
			
			Field is used when HLOS_TID_overwrite is set.
			
			
			
			Field is also used when HLOS_TID_overwrite is not set
			and DSCP/PCP is not available in the packet
			
			<legal all>
*/
#define TCL_DATA_CMD_4_HLOS_TID_OFFSET                               0x00000010
#define TCL_DATA_CMD_4_HLOS_TID_LSB                                  22
#define TCL_DATA_CMD_4_HLOS_TID_MASK                                 0x03c00000

/* Description		TCL_DATA_CMD_4_LMAC_ID
			
			TCL uses this LMAC_ID in address search, i.e, while
			finding matching entry for the packet in AST corresponding
			to given LMAC_ID
			
			If LMAC ID is all 1s (=> value 3), it indicates wildcard
			match for any MAC
			
			<legal 0-3>
*/
#define TCL_DATA_CMD_4_LMAC_ID_OFFSET                                0x00000010
#define TCL_DATA_CMD_4_LMAC_ID_LSB                                   26
#define TCL_DATA_CMD_4_LMAC_ID_MASK                                  0x0c000000

/* Description		TCL_DATA_CMD_4_RESERVED_4B
			
			<legal 0>
*/
#define TCL_DATA_CMD_4_RESERVED_4B_OFFSET                            0x00000010
#define TCL_DATA_CMD_4_RESERVED_4B_LSB                               28
#define TCL_DATA_CMD_4_RESERVED_4B_MASK                              0xf0000000

/* Description		TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM
			
			DSCP to TID mapping table number that need to be used
			for the MSDU, should be specified using this field
			
			<legal all>
*/
#define TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM_OFFSET                     0x00000014
#define TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM_LSB                        0
#define TCL_DATA_CMD_5_DSCP_TID_TABLE_NUM_MASK                       0x0000003f

/* Description		TCL_DATA_CMD_5_SEARCH_INDEX
			
			The index that will be used for index based address or
			flow search. The field is valid when 'search_type' is  1 or
			2. 
			
			<legal all>
*/
#define TCL_DATA_CMD_5_SEARCH_INDEX_OFFSET                           0x00000014
#define TCL_DATA_CMD_5_SEARCH_INDEX_LSB                              6
#define TCL_DATA_CMD_5_SEARCH_INDEX_MASK                             0x03ffffc0

/* Description		TCL_DATA_CMD_5_CACHE_SET_NUM
			
			Cache set number that should be used to cache the index
			based search results, for address and flow search. This
			value should be equal to LSB four bits of the hash value of
			match data, in case of search index points to an entry which
			may be used in content based search also. The value can be
			anything when the entry pointed by search index will not be
			used for content based search. 
			
			<legal all>
*/
#define TCL_DATA_CMD_5_CACHE_SET_NUM_OFFSET                          0x00000014
#define TCL_DATA_CMD_5_CACHE_SET_NUM_LSB                             26
#define TCL_DATA_CMD_5_CACHE_SET_NUM_MASK                            0x3c000000

/* Description		TCL_DATA_CMD_5_MESH_ENABLE
			
			If set to a non-zero value:
			
			* For raw WiFi frames, this indicates transmission to a
			mesh STA, enabling the interpretation of the 'Mesh Control
			Present' bit (bit 8) of QoS Control (otherwise this bit is
			ignored). The interpretation of the A-MSDU 'Length' field is
			decided by the e-numerations below.
			
			* For native WiFi frames, this indicates that a 'Mesh
			Control' field is present between the header and the LLC.
			The three non-zero values are interchangeable.
			
			
			
			<enum 0 MESH_DISABLE>
			
			<enum 1 MESH_Q2Q> A-MSDU 'Length' is big endian and
			includes the length of Mesh Control.
			
			<enum 2 MESH_11S_BE> A-MSDU 'Length' is big endian and
			excludes the length of Mesh Control.
			
			<enum 3 MESH_11S_LE> A-MSDU 'Length' is little endian
			and excludes the length of Mesh Control. This is
			802.11s-compliant.
*/
#define TCL_DATA_CMD_5_MESH_ENABLE_OFFSET                            0x00000014
#define TCL_DATA_CMD_5_MESH_ENABLE_LSB                               30
#define TCL_DATA_CMD_5_MESH_ENABLE_MASK                              0xc0000000

/* Description		TCL_DATA_CMD_6_RESERVED_6A
			
			<legal 0>
*/
#define TCL_DATA_CMD_6_RESERVED_6A_OFFSET                            0x00000018
#define TCL_DATA_CMD_6_RESERVED_6A_LSB                               0
#define TCL_DATA_CMD_6_RESERVED_6A_MASK                              0x000fffff

/* Description		TCL_DATA_CMD_6_RING_ID
			
			The buffer pointer ring ID.
			
			0 refers to the IDLE ring
			
			1 - N refers to other rings
			
			
			
			Helps with debugging when dumping ring contents.
			
			<legal all>
*/
#define TCL_DATA_CMD_6_RING_ID_OFFSET                                0x00000018
#define TCL_DATA_CMD_6_RING_ID_LSB                                   20
#define TCL_DATA_CMD_6_RING_ID_MASK                                  0x0ff00000

/* Description		TCL_DATA_CMD_6_LOOPING_COUNT
			
			A count value that indicates the number of times the
			producer of entries into the Ring has looped around the
			ring.
			
			At initialization time, this value is set to 0. On the
			first loop, this value is set to 1. After the max value is
			reached allowed by the number of bits for this field, the
			count value continues with 0 again.
			
			
			
			In case SW is the consumer of the ring entries, it can
			use this field to figure out up to where the producer of
			entries has created new entries. This eliminates the need to
			check where the head pointer' of the ring is located once
			the SW starts processing an interrupt indicating that new
			entries have been put into this ring...
			
			
			
			Also note that SW if it wants only needs to look at the
			LSB bit of this count value.
			
			<legal all>
*/
#define TCL_DATA_CMD_6_LOOPING_COUNT_OFFSET                          0x00000018
#define TCL_DATA_CMD_6_LOOPING_COUNT_LSB                             28
#define TCL_DATA_CMD_6_LOOPING_COUNT_MASK                            0xf0000000


#endif // _TCL_DATA_CMD_H_

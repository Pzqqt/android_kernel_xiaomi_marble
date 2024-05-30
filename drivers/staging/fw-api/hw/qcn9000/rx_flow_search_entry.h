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

#ifndef _RX_FLOW_SEARCH_ENTRY_H_
#define _RX_FLOW_SEARCH_ENTRY_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	src_ip_127_96[31:0]
//	1	src_ip_95_64[31:0]
//	2	src_ip_63_32[31:0]
//	3	src_ip_31_0[31:0]
//	4	dest_ip_127_96[31:0]
//	5	dest_ip_95_64[31:0]
//	6	dest_ip_63_32[31:0]
//	7	dest_ip_31_0[31:0]
//	8	src_port[15:0], dest_port[31:16]
//	9	l4_protocol[7:0], valid[8], reserved_9[23:9], reo_destination_indication[28:24], msdu_drop[29], reo_destination_handler[31:30]
//	10	metadata[31:0]
//	11	aggregation_count[6:0], lro_eligible[7], msdu_count[31:8]
//	12	msdu_byte_count[31:0]
//	13	timestamp[31:0]
//	14	cumulative_l4_checksum[15:0], cumulative_ip_length[31:16]
//	15	tcp_sequence_number[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_FLOW_SEARCH_ENTRY 16

struct rx_flow_search_entry {
             uint32_t src_ip_127_96                   : 32; //[31:0]
             uint32_t src_ip_95_64                    : 32; //[31:0]
             uint32_t src_ip_63_32                    : 32; //[31:0]
             uint32_t src_ip_31_0                     : 32; //[31:0]
             uint32_t dest_ip_127_96                  : 32; //[31:0]
             uint32_t dest_ip_95_64                   : 32; //[31:0]
             uint32_t dest_ip_63_32                   : 32; //[31:0]
             uint32_t dest_ip_31_0                    : 32; //[31:0]
             uint32_t src_port                        : 16, //[15:0]
                      dest_port                       : 16; //[31:16]
             uint32_t l4_protocol                     :  8, //[7:0]
                      valid                           :  1, //[8]
                      reserved_9                      : 15, //[23:9]
                      reo_destination_indication      :  5, //[28:24]
                      msdu_drop                       :  1, //[29]
                      reo_destination_handler         :  2; //[31:30]
             uint32_t metadata                        : 32; //[31:0]
             uint32_t aggregation_count               :  7, //[6:0]
                      lro_eligible                    :  1, //[7]
                      msdu_count                      : 24; //[31:8]
             uint32_t msdu_byte_count                 : 32; //[31:0]
             uint32_t timestamp                       : 32; //[31:0]
             uint32_t cumulative_l4_checksum          : 16, //[15:0]
                      cumulative_ip_length            : 16; //[31:16]
             uint32_t tcp_sequence_number             : 32; //[31:0]
};

/*

src_ip_127_96
			
			Uppermost 32 bits of source IPv6 address or prefix as
			per Common Parser register field IP_DA_SA_PREFIX (with the
			first byte in the MSB and the last byte in the LSB, i.e.
			requiring a byte-swap for little-endian SW w.r.t. the byte
			order in an IPv6 packet)
			
			<legal all>

src_ip_95_64
			
			Next 32 bits of source IPv6 address or prefix (requiring
			a byte-swap for little-endian SW) <legal all>

src_ip_63_32
			
			Next 32 bits of source IPv6 address or lowest 32 bits of
			prefix (requiring a byte-swap for little-endian SW)
			
			<legal all>

src_ip_31_0
			
			Lowest 32 bits of source IPv6 address, or source IPv4
			address (requiring a byte-swap for little-endian SW w.r.t.
			the byte order in an IPv6 or IPv4 packet)
			
			<legal all>

dest_ip_127_96
			
			Uppermost 32 bits of destination IPv6 address or prefix
			as per Common Parser register field IP_DA_SA_PREFIX (with
			the first byte in the MSB and the last byte in the LSB, i.e.
			requiring a byte-swap for little-endian SW w.r.t. the byte
			order as in an IPv6 packet)
			
			<legal all>

dest_ip_95_64
			
			Next 32 bits of destination IPv6 address or prefix
			(requiring a byte-swap for little-endian SW)
			
			<legal all>

dest_ip_63_32
			
			Next 32 bits of destination IPv6 address or lowest 32
			bits of prefix (requiring a byte-swap for little-endian SW)
			
			<legal all>

dest_ip_31_0
			
			Lowest 32 bits of destination IPv6 address, or
			destination IPv4 address (requiring a byte-swap for
			little-endian SW w.r.t. the byte order in an IPv6 or IPv4
			packet)
			
			<legal all>

src_port
			
			LSB of SPI in case of ESP/AH
			
			else source port in case of TCP/UDP without IPsec,
			
			else zeros in case of ICMP (with the first/third byte in
			the MSB and the second/fourth byte in the LSB, i.e.
			requiring a byte-swap for little-endian SW w.r.t. the byte
			order as in an IPv6 or IPv4 packet)  <legal all>

dest_port
			
			MSB of SPI in case of ESP/AH
			
			else destination port in case of TCP/UDP without IPsec,
			
			else zeros in case of ICMP (with the first byte in the
			MSB and the second byte in the LSB, i.e. requiring a
			byte-swap for little-endian SW w.r.t. the byte order as in
			an IPv6 or IPv4 packet)
			
			<legal all>

l4_protocol
			
			IPsec or L4 protocol
			
			
			
			<enum 1 ICMPV4>
			
			<enum 6 TCP>
			
			<enum 17 UDP>
			
			<enum 50 ESP>
			
			<enum 51 AH>
			
			<enum 58 ICMPV6>
			
			<legal 1, 6, 17, 50, 51, 58>

valid
			
			Indicates validity of entry
			
			<legal all>

reserved_9
			
			<legal 0>

reo_destination_indication
			
			The ID of the REO exit ring where the MSDU frame shall
			push after (MPDU level) reordering has finished.
			
			
			
			<enum 0 reo_destination_tcl> Reo will push the frame
			into the REO2TCL ring
			
			<enum 1 reo_destination_sw1> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 2 reo_destination_sw2> Reo will push the frame
			into the REO2SW2 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW3 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW4 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_sw5> Reo will push the frame
			into the REO2SW5 ring (REO remaps this in chips without
			REO2SW5 ring, e.g. Pine)
			
			<enum 8 reo_destination_sw6> Reo will push the frame
			into the REO2SW6 ring (REO remaps this in chips without
			REO2SW6 ring, e.g. Pine) 
			
			<enum 9 reo_destination_9> REO remaps this <enum 10
			reo_destination_10> REO remaps this 
			
			<enum 11 reo_destination_11> REO remaps this 
			
			<enum 12 reo_destination_12> REO remaps this <enum 13
			reo_destination_13> REO remaps this 
			
			<enum 14 reo_destination_14> REO remaps this 
			
			<enum 15 reo_destination_15> REO remaps this 
			
			<enum 16 reo_destination_16> REO remaps this 
			
			<enum 17 reo_destination_17> REO remaps this 
			
			<enum 18 reo_destination_18> REO remaps this 
			
			<enum 19 reo_destination_19> REO remaps this 
			
			<enum 20 reo_destination_20> REO remaps this 
			
			<enum 21 reo_destination_21> REO remaps this 
			
			<enum 22 reo_destination_22> REO remaps this 
			
			<enum 23 reo_destination_23> REO remaps this 
			
			<enum 24 reo_destination_24> REO remaps this 
			
			<enum 25 reo_destination_25> REO remaps this 
			
			<enum 26 reo_destination_26> REO remaps this 
			
			<enum 27 reo_destination_27> REO remaps this 
			
			<enum 28 reo_destination_28> REO remaps this 
			
			<enum 29 reo_destination_29> REO remaps this 
			
			<enum 30 reo_destination_30> REO remaps this 
			
			<enum 31 reo_destination_31> REO remaps this 
			
			
			
			<legal all>

msdu_drop
			
			Overriding indication to REO to forward to REO release
			ring
			
			<legal all>

reo_destination_handler
			
			Indicates how to decide the REO destination indication
			
			<enum 0 RXFT_USE_FT> Follow this entry
			
			<enum 1 RXFT_USE_ASPT> Use address search+peer table
			entry
			
			<enum 2 RXFT_USE_FT2> Follow this entry
			
			<enum 3 RXFT_USE_CCE> Use CCE super-rule
			
			<legal all>

metadata
			
			Value to be passed to SW if this flow search entry
			matches
			
			<legal all>

aggregation_count
			
			FISA: Number'of MSDU's aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

lro_eligible
			
			FISA: To indicate whether the previous MSDU for this
			flow is eligible for LRO/FISA
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

msdu_count
			
			Number of Rx MSDUs matching this flow
			
			<legal all>

msdu_byte_count
			
			Number of bytes in Rx MSDUs matching this flow
			
			<legal all>

timestamp
			
			Time of last reception (as measured at Rx OLE) matching
			this flow
			
			<legal all>

cumulative_l4_checksum
			
			FISA: checksum 'or MSDU's that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

cumulative_ip_length
			
			FISA: Total MSDU length that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

tcp_sequence_number
			
			FISA: TCP Sequence number of the last packet in this
			flow to detect sequence number jump
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/


/* Description		RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96
			
			Uppermost 32 bits of source IPv6 address or prefix as
			per Common Parser register field IP_DA_SA_PREFIX (with the
			first byte in the MSB and the last byte in the LSB, i.e.
			requiring a byte-swap for little-endian SW w.r.t. the byte
			order in an IPv6 packet)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96_OFFSET                  0x00000000
#define RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96_LSB                     0
#define RX_FLOW_SEARCH_ENTRY_0_SRC_IP_127_96_MASK                    0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64
			
			Next 32 bits of source IPv6 address or prefix (requiring
			a byte-swap for little-endian SW) <legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64_OFFSET                   0x00000004
#define RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64_LSB                      0
#define RX_FLOW_SEARCH_ENTRY_1_SRC_IP_95_64_MASK                     0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32
			
			Next 32 bits of source IPv6 address or lowest 32 bits of
			prefix (requiring a byte-swap for little-endian SW)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32_OFFSET                   0x00000008
#define RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32_LSB                      0
#define RX_FLOW_SEARCH_ENTRY_2_SRC_IP_63_32_MASK                     0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0
			
			Lowest 32 bits of source IPv6 address, or source IPv4
			address (requiring a byte-swap for little-endian SW w.r.t.
			the byte order in an IPv6 or IPv4 packet)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0_OFFSET                    0x0000000c
#define RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0_LSB                       0
#define RX_FLOW_SEARCH_ENTRY_3_SRC_IP_31_0_MASK                      0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96
			
			Uppermost 32 bits of destination IPv6 address or prefix
			as per Common Parser register field IP_DA_SA_PREFIX (with
			the first byte in the MSB and the last byte in the LSB, i.e.
			requiring a byte-swap for little-endian SW w.r.t. the byte
			order as in an IPv6 packet)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96_OFFSET                 0x00000010
#define RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96_LSB                    0
#define RX_FLOW_SEARCH_ENTRY_4_DEST_IP_127_96_MASK                   0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64
			
			Next 32 bits of destination IPv6 address or prefix
			(requiring a byte-swap for little-endian SW)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64_OFFSET                  0x00000014
#define RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64_LSB                     0
#define RX_FLOW_SEARCH_ENTRY_5_DEST_IP_95_64_MASK                    0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32
			
			Next 32 bits of destination IPv6 address or lowest 32
			bits of prefix (requiring a byte-swap for little-endian SW)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32_OFFSET                  0x00000018
#define RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32_LSB                     0
#define RX_FLOW_SEARCH_ENTRY_6_DEST_IP_63_32_MASK                    0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0
			
			Lowest 32 bits of destination IPv6 address, or
			destination IPv4 address (requiring a byte-swap for
			little-endian SW w.r.t. the byte order in an IPv6 or IPv4
			packet)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0_OFFSET                   0x0000001c
#define RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0_LSB                      0
#define RX_FLOW_SEARCH_ENTRY_7_DEST_IP_31_0_MASK                     0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_8_SRC_PORT
			
			LSB of SPI in case of ESP/AH
			
			else source port in case of TCP/UDP without IPsec,
			
			else zeros in case of ICMP (with the first/third byte in
			the MSB and the second/fourth byte in the LSB, i.e.
			requiring a byte-swap for little-endian SW w.r.t. the byte
			order as in an IPv6 or IPv4 packet)  <legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_8_SRC_PORT_OFFSET                       0x00000020
#define RX_FLOW_SEARCH_ENTRY_8_SRC_PORT_LSB                          0
#define RX_FLOW_SEARCH_ENTRY_8_SRC_PORT_MASK                         0x0000ffff

/* Description		RX_FLOW_SEARCH_ENTRY_8_DEST_PORT
			
			MSB of SPI in case of ESP/AH
			
			else destination port in case of TCP/UDP without IPsec,
			
			else zeros in case of ICMP (with the first byte in the
			MSB and the second byte in the LSB, i.e. requiring a
			byte-swap for little-endian SW w.r.t. the byte order as in
			an IPv6 or IPv4 packet)
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_8_DEST_PORT_OFFSET                      0x00000020
#define RX_FLOW_SEARCH_ENTRY_8_DEST_PORT_LSB                         16
#define RX_FLOW_SEARCH_ENTRY_8_DEST_PORT_MASK                        0xffff0000

/* Description		RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL
			
			IPsec or L4 protocol
			
			
			
			<enum 1 ICMPV4>
			
			<enum 6 TCP>
			
			<enum 17 UDP>
			
			<enum 50 ESP>
			
			<enum 51 AH>
			
			<enum 58 ICMPV6>
			
			<legal 1, 6, 17, 50, 51, 58>
*/
#define RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL_OFFSET                    0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL_LSB                       0
#define RX_FLOW_SEARCH_ENTRY_9_L4_PROTOCOL_MASK                      0x000000ff

/* Description		RX_FLOW_SEARCH_ENTRY_9_VALID
			
			Indicates validity of entry
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_9_VALID_OFFSET                          0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_VALID_LSB                             8
#define RX_FLOW_SEARCH_ENTRY_9_VALID_MASK                            0x00000100

/* Description		RX_FLOW_SEARCH_ENTRY_9_RESERVED_9
			
			<legal 0>
*/
#define RX_FLOW_SEARCH_ENTRY_9_RESERVED_9_OFFSET                     0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_RESERVED_9_LSB                        9
#define RX_FLOW_SEARCH_ENTRY_9_RESERVED_9_MASK                       0x00fffe00

/* Description		RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION
			
			The ID of the REO exit ring where the MSDU frame shall
			push after (MPDU level) reordering has finished.
			
			
			
			<enum 0 reo_destination_tcl> Reo will push the frame
			into the REO2TCL ring
			
			<enum 1 reo_destination_sw1> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 2 reo_destination_sw2> Reo will push the frame
			into the REO2SW2 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW3 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW4 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_sw5> Reo will push the frame
			into the REO2SW5 ring (REO remaps this in chips without
			REO2SW5 ring, e.g. Pine)
			
			<enum 8 reo_destination_sw6> Reo will push the frame
			into the REO2SW6 ring (REO remaps this in chips without
			REO2SW6 ring, e.g. Pine) 
			
			<enum 9 reo_destination_9> REO remaps this <enum 10
			reo_destination_10> REO remaps this 
			
			<enum 11 reo_destination_11> REO remaps this 
			
			<enum 12 reo_destination_12> REO remaps this <enum 13
			reo_destination_13> REO remaps this 
			
			<enum 14 reo_destination_14> REO remaps this 
			
			<enum 15 reo_destination_15> REO remaps this 
			
			<enum 16 reo_destination_16> REO remaps this 
			
			<enum 17 reo_destination_17> REO remaps this 
			
			<enum 18 reo_destination_18> REO remaps this 
			
			<enum 19 reo_destination_19> REO remaps this 
			
			<enum 20 reo_destination_20> REO remaps this 
			
			<enum 21 reo_destination_21> REO remaps this 
			
			<enum 22 reo_destination_22> REO remaps this 
			
			<enum 23 reo_destination_23> REO remaps this 
			
			<enum 24 reo_destination_24> REO remaps this 
			
			<enum 25 reo_destination_25> REO remaps this 
			
			<enum 26 reo_destination_26> REO remaps this 
			
			<enum 27 reo_destination_27> REO remaps this 
			
			<enum 28 reo_destination_28> REO remaps this 
			
			<enum 29 reo_destination_29> REO remaps this 
			
			<enum 30 reo_destination_30> REO remaps this 
			
			<enum 31 reo_destination_31> REO remaps this 
			
			
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION_OFFSET     0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION_LSB        24
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_INDICATION_MASK       0x1f000000

/* Description		RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP
			
			Overriding indication to REO to forward to REO release
			ring
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP_OFFSET                      0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP_LSB                         29
#define RX_FLOW_SEARCH_ENTRY_9_MSDU_DROP_MASK                        0x20000000

/* Description		RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER
			
			Indicates how to decide the REO destination indication
			
			<enum 0 RXFT_USE_FT> Follow this entry
			
			<enum 1 RXFT_USE_ASPT> Use address search+peer table
			entry
			
			<enum 2 RXFT_USE_FT2> Follow this entry
			
			<enum 3 RXFT_USE_CCE> Use CCE super-rule
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER_OFFSET        0x00000024
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER_LSB           30
#define RX_FLOW_SEARCH_ENTRY_9_REO_DESTINATION_HANDLER_MASK          0xc0000000

/* Description		RX_FLOW_SEARCH_ENTRY_10_METADATA
			
			Value to be passed to SW if this flow search entry
			matches
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_10_METADATA_OFFSET                      0x00000028
#define RX_FLOW_SEARCH_ENTRY_10_METADATA_LSB                         0
#define RX_FLOW_SEARCH_ENTRY_10_METADATA_MASK                        0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT
			
			FISA: Number'of MSDU's aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT_OFFSET             0x0000002c
#define RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT_LSB                0
#define RX_FLOW_SEARCH_ENTRY_11_AGGREGATION_COUNT_MASK               0x0000007f

/* Description		RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE
			
			FISA: To indicate whether the previous MSDU for this
			flow is eligible for LRO/FISA
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE_OFFSET                  0x0000002c
#define RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE_LSB                     7
#define RX_FLOW_SEARCH_ENTRY_11_LRO_ELIGIBLE_MASK                    0x00000080

/* Description		RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT
			
			Number of Rx MSDUs matching this flow
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT_OFFSET                    0x0000002c
#define RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT_LSB                       8
#define RX_FLOW_SEARCH_ENTRY_11_MSDU_COUNT_MASK                      0xffffff00

/* Description		RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT
			
			Number of bytes in Rx MSDUs matching this flow
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT_OFFSET               0x00000030
#define RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT_LSB                  0
#define RX_FLOW_SEARCH_ENTRY_12_MSDU_BYTE_COUNT_MASK                 0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP
			
			Time of last reception (as measured at Rx OLE) matching
			this flow
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP_OFFSET                     0x00000034
#define RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP_LSB                        0
#define RX_FLOW_SEARCH_ENTRY_13_TIMESTAMP_MASK                       0xffffffff

/* Description		RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM
			
			FISA: checksum 'or MSDU's that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM_OFFSET        0x00000038
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM_LSB           0
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_L4_CHECKSUM_MASK          0x0000ffff

/* Description		RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH
			
			FISA: Total MSDU length that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH_OFFSET          0x00000038
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH_LSB             16
#define RX_FLOW_SEARCH_ENTRY_14_CUMULATIVE_IP_LENGTH_MASK            0xffff0000

/* Description		RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER
			
			FISA: TCP Sequence number of the last packet in this
			flow to detect sequence number jump
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER_OFFSET           0x0000003c
#define RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER_LSB              0
#define RX_FLOW_SEARCH_ENTRY_15_TCP_SEQUENCE_NUMBER_MASK             0xffffffff


#endif // _RX_FLOW_SEARCH_ENTRY_H_

/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _RX_MSDU_END_H_
#define _RX_MSDU_END_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	rxpcu_mpdu_filter_in_category[1:0], sw_frame_group_id[8:2], reserved_0[15:9], phy_ppdu_id[31:16]
//	1	ip_hdr_chksum[15:0], reported_mpdu_length[29:16], reserved_1a[31:30]
//	2	key_id_octet[7:0], cce_super_rule[13:8], cce_classify_not_done_truncate[14], cce_classify_not_done_cce_dis[15], cumulative_l3_checksum[31:16]
//	3	rule_indication_31_0[31:0]
//	4	rule_indication_63_32[31:0]
//	5	da_offset[5:0], sa_offset[11:6], da_offset_valid[12], sa_offset_valid[13], reserved_5a[15:14], l3_type[31:16]
//	6	ipv6_options_crc[31:0]
//	7	tcp_seq_number[31:0]
//	8	tcp_ack_number[31:0]
//	9	tcp_flag[8:0], lro_eligible[9], reserved_9a[15:10], window_size[31:16]
//	10	tcp_udp_chksum[15:0], sa_idx_timeout[16], da_idx_timeout[17], msdu_limit_error[18], flow_idx_timeout[19], flow_idx_invalid[20], wifi_parser_error[21], amsdu_parser_error[22], sa_is_valid[23], da_is_valid[24], da_is_mcbc[25], l3_header_padding[27:26], first_msdu[28], last_msdu[29], tcp_udp_chksum_fail[30], ip_chksum_fail[31]
//	11	sa_idx[15:0], da_idx_or_sw_peer_id[31:16]
//	12	msdu_drop[0], reo_destination_indication[5:1], flow_idx[25:6], reserved_12a[31:26]
//	13	fse_metadata[31:0]
//	14	cce_metadata[15:0], sa_sw_peer_id[31:16]
//	15	aggregation_count[7:0], flow_aggregation_continuation[8], fisa_timeout[9], reserved_15a[31:10]
//	16	cumulative_l4_checksum[15:0], cumulative_ip_length[31:16]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_MSDU_END 17

struct rx_msdu_end {
             uint32_t rxpcu_mpdu_filter_in_category   :  2, //[1:0]
                      sw_frame_group_id               :  7, //[8:2]
                      reserved_0                      :  7, //[15:9]
                      phy_ppdu_id                     : 16; //[31:16]
             uint32_t ip_hdr_chksum                   : 16, //[15:0]
                      reported_mpdu_length            : 14, //[29:16]
                      reserved_1a                     :  2; //[31:30]
             uint32_t key_id_octet                    :  8, //[7:0]
                      cce_super_rule                  :  6, //[13:8]
                      cce_classify_not_done_truncate  :  1, //[14]
                      cce_classify_not_done_cce_dis   :  1, //[15]
                      cumulative_l3_checksum          : 16; //[31:16]
             uint32_t rule_indication_31_0            : 32; //[31:0]
             uint32_t rule_indication_63_32           : 32; //[31:0]
             uint32_t da_offset                       :  6, //[5:0]
                      sa_offset                       :  6, //[11:6]
                      da_offset_valid                 :  1, //[12]
                      sa_offset_valid                 :  1, //[13]
                      reserved_5a                     :  2, //[15:14]
                      l3_type                         : 16; //[31:16]
             uint32_t ipv6_options_crc                : 32; //[31:0]
             uint32_t tcp_seq_number                  : 32; //[31:0]
             uint32_t tcp_ack_number                  : 32; //[31:0]
             uint32_t tcp_flag                        :  9, //[8:0]
                      lro_eligible                    :  1, //[9]
                      reserved_9a                     :  6, //[15:10]
                      window_size                     : 16; //[31:16]
             uint32_t tcp_udp_chksum                  : 16, //[15:0]
                      sa_idx_timeout                  :  1, //[16]
                      da_idx_timeout                  :  1, //[17]
                      msdu_limit_error                :  1, //[18]
                      flow_idx_timeout                :  1, //[19]
                      flow_idx_invalid                :  1, //[20]
                      wifi_parser_error               :  1, //[21]
                      amsdu_parser_error              :  1, //[22]
                      sa_is_valid                     :  1, //[23]
                      da_is_valid                     :  1, //[24]
                      da_is_mcbc                      :  1, //[25]
                      l3_header_padding               :  2, //[27:26]
                      first_msdu                      :  1, //[28]
                      last_msdu                       :  1, //[29]
                      tcp_udp_chksum_fail             :  1, //[30]
                      ip_chksum_fail                  :  1; //[31]
             uint32_t sa_idx                          : 16, //[15:0]
                      da_idx_or_sw_peer_id            : 16; //[31:16]
             uint32_t msdu_drop                       :  1, //[0]
                      reo_destination_indication      :  5, //[5:1]
                      flow_idx                        : 20, //[25:6]
                      reserved_12a                    :  6; //[31:26]
             uint32_t fse_metadata                    : 32; //[31:0]
             uint32_t cce_metadata                    : 16, //[15:0]
                      sa_sw_peer_id                   : 16; //[31:16]
             uint32_t aggregation_count               :  8, //[7:0]
                      flow_aggregation_continuation   :  1, //[8]
                      fisa_timeout                    :  1, //[9]
                      reserved_15a                    : 22; //[31:10]
             uint32_t cumulative_l4_checksum          : 16, //[15:0]
                      cumulative_ip_length            : 16; //[31:16]
};

/*

rxpcu_mpdu_filter_in_category
			
			Field indicates what the reason was that this MPDU frame
			was allowed to come into the receive path by RXPCU
			
			<enum 0 rxpcu_filter_pass> This MPDU passed the normal
			frame filter programming of rxpcu
			
			<enum 1 rxpcu_monitor_client> This MPDU did NOT pass the
			regular frame filter and would have been dropped, were it
			not for the frame fitting into the 'monitor_client'
			category.
			
			<enum 2 rxpcu_monitor_other> This MPDU did NOT pass the
			regular frame filter and also did not pass the
			rxpcu_monitor_client filter. It would have been dropped
			accept that it did pass the 'monitor_other' category.
			
			<legal 0-2>

sw_frame_group_id
			
			SW processes frames based on certain classifications.
			This field indicates to what sw classification this MPDU is
			mapped.
			
			The classification is given in priority order
			
			
			
			<enum 0 sw_frame_group_NDP_frame> 
			
			
			
			<enum 1 sw_frame_group_Multicast_data> 
			
			<enum 2 sw_frame_group_Unicast_data> 
			
			<enum 3 sw_frame_group_Null_data > This includes mpdus
			of type Data Null as well as QoS Data Null
			
			
			
			<enum 4 sw_frame_group_mgmt_0000 > 
			
			<enum 5 sw_frame_group_mgmt_0001 > 
			
			<enum 6 sw_frame_group_mgmt_0010 > 
			
			<enum 7 sw_frame_group_mgmt_0011 > 
			
			<enum 8 sw_frame_group_mgmt_0100 > 
			
			<enum 9 sw_frame_group_mgmt_0101 > 
			
			<enum 10 sw_frame_group_mgmt_0110 > 
			
			<enum 11 sw_frame_group_mgmt_0111 > 
			
			<enum 12 sw_frame_group_mgmt_1000 > 
			
			<enum 13 sw_frame_group_mgmt_1001 > 
			
			<enum 14 sw_frame_group_mgmt_1010 > 
			
			<enum 15 sw_frame_group_mgmt_1011 > 
			
			<enum 16 sw_frame_group_mgmt_1100 > 
			
			<enum 17 sw_frame_group_mgmt_1101 > 
			
			<enum 18 sw_frame_group_mgmt_1110 > 
			
			<enum 19 sw_frame_group_mgmt_1111 > 
			
			
			
			<enum 20 sw_frame_group_ctrl_0000 > 
			
			<enum 21 sw_frame_group_ctrl_0001 > 
			
			<enum 22 sw_frame_group_ctrl_0010 > 
			
			<enum 23 sw_frame_group_ctrl_0011 > 
			
			<enum 24 sw_frame_group_ctrl_0100 > 
			
			<enum 25 sw_frame_group_ctrl_0101 > 
			
			<enum 26 sw_frame_group_ctrl_0110 > 
			
			<enum 27 sw_frame_group_ctrl_0111 > 
			
			<enum 28 sw_frame_group_ctrl_1000 > 
			
			<enum 29 sw_frame_group_ctrl_1001 > 
			
			<enum 30 sw_frame_group_ctrl_1010 > 
			
			<enum 31 sw_frame_group_ctrl_1011 > 
			
			<enum 32 sw_frame_group_ctrl_1100 > 
			
			<enum 33 sw_frame_group_ctrl_1101 > 
			
			<enum 34 sw_frame_group_ctrl_1110 > 
			
			<enum 35 sw_frame_group_ctrl_1111 >
			
			 
			
			<enum 36 sw_frame_group_unsupported> This covers type 3
			and protocol version != 0
			
			
			
			
			
			
			<legal 0-37>

reserved_0
			
			<legal 0>

phy_ppdu_id
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>

ip_hdr_chksum
			
			This can include the IP header checksum or the pseudo
			header checksum used by TCP/UDP checksum. 
			
			(with the first byte in the MSB and the second byte in
			the LSB, i.e. requiring a byte-swap for little-endian FW/SW
			w.r.t. the byte order in a packet)

reported_mpdu_length
			
			MPDU length before decapsulation.  Only valid when
			first_msdu is set.  This field is taken directly from the
			length field of the A-MPDU delimiter or the preamble length
			field for non-A-MPDU frames.

reserved_1a
			
			<legal 0>

key_id_octet
			
			The key ID octet from the IV.  Only valid when
			first_msdu is set.

cce_super_rule
			
			Indicates the super filter rule 

cce_classify_not_done_truncate
			
			Classification failed due to truncated frame

cce_classify_not_done_cce_dis
			
			Classification failed due to CCE global disable

cumulative_l3_checksum
			
			FISA: IP header checksum including the total MSDU length
			that is part of this flow aggregated so far, reported if
			'RXOLE_R0_FISA_CTRL. CHKSUM_CUM_IP_LEN_EN' is set
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

rule_indication_31_0
			
			Bitmap indicating which of rules 31-0 have matched

rule_indication_63_32
			
			Bitmap indicating which of rules 63-32 have matched

da_offset
			
			Offset into MSDU buffer for DA

sa_offset
			
			Offset into MSDU buffer for SA

da_offset_valid
			
			da_offset field is valid. This will be set to 0 in case
			of a dynamic A-MSDU when DA is compressed

sa_offset_valid
			
			sa_offset field is valid. This will be set to 0 in case
			of a dynamic A-MSDU when SA is compressed

reserved_5a
			
			<legal 0>

l3_type
			
			The 16-bit type value indicating the type of L3 later
			extracted from LLC/SNAP, set to zero if SNAP is not
			available

ipv6_options_crc
			
			32 bit CRC computed out of  IP v6 extension headers

tcp_seq_number
			
			TCP sequence number (as a number assembled from a TCP
			packet in big-endian order, i.e. requiring a byte-swap for
			little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			
			In Pine, if 'RXOLE_R0_MISC_CONFIG.
			OVERRIDE_MSDU_END_FIELDS' is set, toeplitz_hash_2_or_4 from
			'RX_MSDU_START' will be reported here:
			
			Controlled by multiple RxOLE registers for TCP/UDP over
			IPv4/IPv6 - Either Toeplitz hash computed over 2-tuple IPv4
			or IPv6 src/dest addresses is reported; or, Toeplitz hash
			computed over 4-tuple IPv4 or IPv6 src/dest addresses and
			src/dest ports is reported. The Flow_id_toeplitz hash can
			also be reported here. Usually the hash reported here is the
			one used for hash-based REO routing (see
			use_flow_id_toeplitz_clfy in 'RXPT_CLASSIFY_INFO').
			Optionally the 3-tuple Toeplitz hash over IPv4 or IPv6
			src/dest addresses and L4 protocol can be reported here.
			
			(Unsupported in HastingsPrime)

tcp_ack_number
			
			TCP acknowledge number (as a number assembled from a TCP
			packet in big-endian order, i.e. requiring a byte-swap for
			little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			
			In Pine, if 'RXOLE_R0_MISC_CONFIG.
			OVERRIDE_MSDU_END_FIELDS' is set, flow_id_toeplitz from
			'RX_MSDU_START' will be reported here:
			
			Toeplitz hash of 5-tuple {IP source address, IP
			destination address, IP source port, IP destination port, L4
			protocol}  in case of non-IPSec. In case of IPSec - Toeplitz
			hash of 4-tuple {IP source address, IP destination address,
			SPI, L4 protocol}. Optionally the 3-tuple Toeplitz hash over
			IPv4 or IPv6 src/dest addresses and L4 protocol can be
			reported here. 
			
			The relevant Toeplitz key registers are provided in
			RxOLE's instance of common parser module. These registers
			are separate from the Toeplitz keys used by ASE/FSE modules
			inside RxOLE. The actual value will be passed on from common
			parser module to RxOLE in one of the WHO_* TLVs.
			
			(Unsupported in HastingsPrime)

tcp_flag
			
			TCP flags
			
			{NS,CWR,ECE,URG,ACK,PSH, RST ,SYN,FIN}(with the NS bit
			in bit 8 and the FIN bit in bit 0, i.e. in big-endian order,
			i.e. requiring a byte-swap for little-endian FW/SW w.r.t.
			the byte order in a packet)

lro_eligible
			
			Computed out of TCP and IP fields to indicate that this
			MSDU is eligible for  LRO

reserved_9a
			
			NOTE: DO not assign a field... Internally used in
			RXOLE..
			
			<legal 0>

window_size
			
			TCP receive window size (as a number assembled from a
			TCP packet in big-endian order, i.e. requiring a byte-swap
			for little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			
			In Pine, if 'RXOLE_R0_MISC_CONFIG.
			OVERRIDE_MSDU_END_FIELDS' is set, msdu_length from
			'RX_MSDU_START' will be reported in the 14 LSBs here:
			
			MSDU length in bytes after decapsulation. This field is
			still valid for MPDU frames without A-MSDU.  It still
			represents MSDU length after decapsulation.
			
			(Unsupported in HastingsPrime)

tcp_udp_chksum
			
			The value of the computed TCP/UDP checksum.  A mode bit
			selects whether this checksum is the full checksum or the
			partial checksum which does not include the pseudo header.
			(with the first byte in the MSB and the second byte in the
			LSB, i.e. requiring a byte-swap for little-endian FW/SW
			w.r.t. the byte order in a packet)

sa_idx_timeout
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer.

da_idx_timeout
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer.

msdu_limit_error
			
			Indicates that the MSDU threshold was exceeded and thus
			all the rest of the MSDUs will not be scattered and will not
			be decapsulated but will be DMA'ed in RAW format as a single
			MSDU buffer

flow_idx_timeout
			
			Indicates an unsuccessful flow search due to the
			expiring of the search timer.
			
			<legal all>

flow_idx_invalid
			
			flow id is not valid
			
			<legal all>

wifi_parser_error
			
			Indicates that the WiFi frame has one of the following
			errors
			
			o has less than minimum allowed bytes as per standard
			
			o has incomplete VLAN LLC/SNAP (only for non A-MSDUs)
			
			<legal all>

amsdu_parser_error
			
			A-MSDU could not be properly de-agregated.
			
			<legal all>

sa_is_valid
			
			Indicates that OLE found a valid SA entry

da_is_valid
			
			Indicates that OLE found a valid DA entry

da_is_mcbc
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address.

l3_header_padding
			
			Number of bytes padded  to make sure that the L3 header
			will always start of a Dword   boundary

first_msdu
			
			Indicates the first MSDU of A-MSDU.  If both first_msdu
			and last_msdu are set in the MSDU then this is a
			non-aggregated MSDU frame: normal MPDU.  Interior MSDU in an
			A-MSDU shall have both first_mpdu and last_mpdu bits set to
			0.

last_msdu
			
			Indicates the last MSDU of the A-MSDU.  MPDU end status
			is only valid when last_msdu is set.

tcp_udp_chksum_fail
			
			if 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS' is
			set, tcp_udp_chksum_fail from 'RX_ATTENTION' will be
			reported here:
			
			Indicates that the computed checksum (tcp_udp_chksum)
			did not match the checksum in the TCP/UDP header.
			
			(unsupported in HastingsPrime)

ip_chksum_fail
			
			If 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS' is
			set, ip_chksum_fail from 'RX_MSDU_START' will be reported in
			the MSB here:
			
			Indicates that the computed checksum (ip_hdr_chksum) did
			not match the checksum in the IP header.
			
			(unsupported in HastingsPrime)

sa_idx
			
			The offset in the address table which matches the MAC
			source address.

da_idx_or_sw_peer_id
			
			Based on a register configuration in RXOLE, this field
			will contain:
			
			The offset in the address table which matches the MAC
			destination address
			
			OR:
			
			sw_peer_id from the address search entry corresponding
			to the destination address of the MSDU

msdu_drop
			
			When set, REO shall drop this MSDU and not forward it to
			any other ring...
			
			<legal all>

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

flow_idx
			
			Flow table index
			
			<legal all>

reserved_12a
			
			<legal 0>

fse_metadata
			
			FSE related meta data:
			
			<legal all>

cce_metadata
			
			CCE related meta data:
			
			<legal all>

sa_sw_peer_id
			
			sw_peer_id from the address search entry corresponding
			to the source address of the MSDU
			
			<legal all>

aggregation_count
			
			FISA: Number of MSDU's aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

flow_aggregation_continuation
			
			FISA: To indicate that this MSDU can be aggregated with
			the previous packet with the same flow id
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

fisa_timeout
			
			FISA: To indicate that the aggregation has restarted for
			this flow due to timeout
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

reserved_15a
			
			<legal 0>

cumulative_l4_checksum
			
			FISA: checksum for MSDU's that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>

cumulative_ip_length
			
			FISA: Total MSDU length that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/


/* Description		RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY
			
			Field indicates what the reason was that this MPDU frame
			was allowed to come into the receive path by RXPCU
			
			<enum 0 rxpcu_filter_pass> This MPDU passed the normal
			frame filter programming of rxpcu
			
			<enum 1 rxpcu_monitor_client> This MPDU did NOT pass the
			regular frame filter and would have been dropped, were it
			not for the frame fitting into the 'monitor_client'
			category.
			
			<enum 2 rxpcu_monitor_other> This MPDU did NOT pass the
			regular frame filter and also did not pass the
			rxpcu_monitor_client filter. It would have been dropped
			accept that it did pass the 'monitor_other' category.
			
			<legal 0-2>
*/
#define RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET           0x00000000
#define RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB              0
#define RX_MSDU_END_0_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK             0x00000003

/* Description		RX_MSDU_END_0_SW_FRAME_GROUP_ID
			
			SW processes frames based on certain classifications.
			This field indicates to what sw classification this MPDU is
			mapped.
			
			The classification is given in priority order
			
			
			
			<enum 0 sw_frame_group_NDP_frame> 
			
			
			
			<enum 1 sw_frame_group_Multicast_data> 
			
			<enum 2 sw_frame_group_Unicast_data> 
			
			<enum 3 sw_frame_group_Null_data > This includes mpdus
			of type Data Null as well as QoS Data Null
			
			
			
			<enum 4 sw_frame_group_mgmt_0000 > 
			
			<enum 5 sw_frame_group_mgmt_0001 > 
			
			<enum 6 sw_frame_group_mgmt_0010 > 
			
			<enum 7 sw_frame_group_mgmt_0011 > 
			
			<enum 8 sw_frame_group_mgmt_0100 > 
			
			<enum 9 sw_frame_group_mgmt_0101 > 
			
			<enum 10 sw_frame_group_mgmt_0110 > 
			
			<enum 11 sw_frame_group_mgmt_0111 > 
			
			<enum 12 sw_frame_group_mgmt_1000 > 
			
			<enum 13 sw_frame_group_mgmt_1001 > 
			
			<enum 14 sw_frame_group_mgmt_1010 > 
			
			<enum 15 sw_frame_group_mgmt_1011 > 
			
			<enum 16 sw_frame_group_mgmt_1100 > 
			
			<enum 17 sw_frame_group_mgmt_1101 > 
			
			<enum 18 sw_frame_group_mgmt_1110 > 
			
			<enum 19 sw_frame_group_mgmt_1111 > 
			
			
			
			<enum 20 sw_frame_group_ctrl_0000 > 
			
			<enum 21 sw_frame_group_ctrl_0001 > 
			
			<enum 22 sw_frame_group_ctrl_0010 > 
			
			<enum 23 sw_frame_group_ctrl_0011 > 
			
			<enum 24 sw_frame_group_ctrl_0100 > 
			
			<enum 25 sw_frame_group_ctrl_0101 > 
			
			<enum 26 sw_frame_group_ctrl_0110 > 
			
			<enum 27 sw_frame_group_ctrl_0111 > 
			
			<enum 28 sw_frame_group_ctrl_1000 > 
			
			<enum 29 sw_frame_group_ctrl_1001 > 
			
			<enum 30 sw_frame_group_ctrl_1010 > 
			
			<enum 31 sw_frame_group_ctrl_1011 > 
			
			<enum 32 sw_frame_group_ctrl_1100 > 
			
			<enum 33 sw_frame_group_ctrl_1101 > 
			
			<enum 34 sw_frame_group_ctrl_1110 > 
			
			<enum 35 sw_frame_group_ctrl_1111 >
			
			 
			
			<enum 36 sw_frame_group_unsupported> This covers type 3
			and protocol version != 0
			
			
			
			
			
			
			<legal 0-37>
*/
#define RX_MSDU_END_0_SW_FRAME_GROUP_ID_OFFSET                       0x00000000
#define RX_MSDU_END_0_SW_FRAME_GROUP_ID_LSB                          2
#define RX_MSDU_END_0_SW_FRAME_GROUP_ID_MASK                         0x000001fc

/* Description		RX_MSDU_END_0_RESERVED_0
			
			<legal 0>
*/
#define RX_MSDU_END_0_RESERVED_0_OFFSET                              0x00000000
#define RX_MSDU_END_0_RESERVED_0_LSB                                 9
#define RX_MSDU_END_0_RESERVED_0_MASK                                0x0000fe00

/* Description		RX_MSDU_END_0_PHY_PPDU_ID
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/
#define RX_MSDU_END_0_PHY_PPDU_ID_OFFSET                             0x00000000
#define RX_MSDU_END_0_PHY_PPDU_ID_LSB                                16
#define RX_MSDU_END_0_PHY_PPDU_ID_MASK                               0xffff0000

/* Description		RX_MSDU_END_1_IP_HDR_CHKSUM
			
			This can include the IP header checksum or the pseudo
			header checksum used by TCP/UDP checksum. 
			
			(with the first byte in the MSB and the second byte in
			the LSB, i.e. requiring a byte-swap for little-endian FW/SW
			w.r.t. the byte order in a packet)
*/
#define RX_MSDU_END_1_IP_HDR_CHKSUM_OFFSET                           0x00000004
#define RX_MSDU_END_1_IP_HDR_CHKSUM_LSB                              0
#define RX_MSDU_END_1_IP_HDR_CHKSUM_MASK                             0x0000ffff

/* Description		RX_MSDU_END_1_REPORTED_MPDU_LENGTH
			
			MPDU length before decapsulation.  Only valid when
			first_msdu is set.  This field is taken directly from the
			length field of the A-MPDU delimiter or the preamble length
			field for non-A-MPDU frames.
*/
#define RX_MSDU_END_1_REPORTED_MPDU_LENGTH_OFFSET                    0x00000004
#define RX_MSDU_END_1_REPORTED_MPDU_LENGTH_LSB                       16
#define RX_MSDU_END_1_REPORTED_MPDU_LENGTH_MASK                      0x3fff0000

/* Description		RX_MSDU_END_1_RESERVED_1A
			
			<legal 0>
*/
#define RX_MSDU_END_1_RESERVED_1A_OFFSET                             0x00000004
#define RX_MSDU_END_1_RESERVED_1A_LSB                                30
#define RX_MSDU_END_1_RESERVED_1A_MASK                               0xc0000000

/* Description		RX_MSDU_END_2_KEY_ID_OCTET
			
			The key ID octet from the IV.  Only valid when
			first_msdu is set.
*/
#define RX_MSDU_END_2_KEY_ID_OCTET_OFFSET                            0x00000008
#define RX_MSDU_END_2_KEY_ID_OCTET_LSB                               0
#define RX_MSDU_END_2_KEY_ID_OCTET_MASK                              0x000000ff

/* Description		RX_MSDU_END_2_CCE_SUPER_RULE
			
			Indicates the super filter rule 
*/
#define RX_MSDU_END_2_CCE_SUPER_RULE_OFFSET                          0x00000008
#define RX_MSDU_END_2_CCE_SUPER_RULE_LSB                             8
#define RX_MSDU_END_2_CCE_SUPER_RULE_MASK                            0x00003f00

/* Description		RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE
			
			Classification failed due to truncated frame
*/
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE_OFFSET          0x00000008
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE_LSB             14
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_TRUNCATE_MASK            0x00004000

/* Description		RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS
			
			Classification failed due to CCE global disable
*/
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS_OFFSET           0x00000008
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS_LSB              15
#define RX_MSDU_END_2_CCE_CLASSIFY_NOT_DONE_CCE_DIS_MASK             0x00008000

/* Description		RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM
			
			FISA: IP header checksum including the total MSDU length
			that is part of this flow aggregated so far, reported if
			'RXOLE_R0_FISA_CTRL. CHKSUM_CUM_IP_LEN_EN' is set
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM_OFFSET                  0x00000008
#define RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM_LSB                     16
#define RX_MSDU_END_2_CUMULATIVE_L3_CHECKSUM_MASK                    0xffff0000

/* Description		RX_MSDU_END_3_RULE_INDICATION_31_0
			
			Bitmap indicating which of rules 31-0 have matched
*/
#define RX_MSDU_END_3_RULE_INDICATION_31_0_OFFSET                    0x0000000c
#define RX_MSDU_END_3_RULE_INDICATION_31_0_LSB                       0
#define RX_MSDU_END_3_RULE_INDICATION_31_0_MASK                      0xffffffff

/* Description		RX_MSDU_END_4_RULE_INDICATION_63_32
			
			Bitmap indicating which of rules 63-32 have matched
*/
#define RX_MSDU_END_4_RULE_INDICATION_63_32_OFFSET                   0x00000010
#define RX_MSDU_END_4_RULE_INDICATION_63_32_LSB                      0
#define RX_MSDU_END_4_RULE_INDICATION_63_32_MASK                     0xffffffff

/* Description		RX_MSDU_END_5_DA_OFFSET
			
			Offset into MSDU buffer for DA
*/
#define RX_MSDU_END_5_DA_OFFSET_OFFSET                               0x00000014
#define RX_MSDU_END_5_DA_OFFSET_LSB                                  0
#define RX_MSDU_END_5_DA_OFFSET_MASK                                 0x0000003f

/* Description		RX_MSDU_END_5_SA_OFFSET
			
			Offset into MSDU buffer for SA
*/
#define RX_MSDU_END_5_SA_OFFSET_OFFSET                               0x00000014
#define RX_MSDU_END_5_SA_OFFSET_LSB                                  6
#define RX_MSDU_END_5_SA_OFFSET_MASK                                 0x00000fc0

/* Description		RX_MSDU_END_5_DA_OFFSET_VALID
			
			da_offset field is valid. This will be set to 0 in case
			of a dynamic A-MSDU when DA is compressed
*/
#define RX_MSDU_END_5_DA_OFFSET_VALID_OFFSET                         0x00000014
#define RX_MSDU_END_5_DA_OFFSET_VALID_LSB                            12
#define RX_MSDU_END_5_DA_OFFSET_VALID_MASK                           0x00001000

/* Description		RX_MSDU_END_5_SA_OFFSET_VALID
			
			sa_offset field is valid. This will be set to 0 in case
			of a dynamic A-MSDU when SA is compressed
*/
#define RX_MSDU_END_5_SA_OFFSET_VALID_OFFSET                         0x00000014
#define RX_MSDU_END_5_SA_OFFSET_VALID_LSB                            13
#define RX_MSDU_END_5_SA_OFFSET_VALID_MASK                           0x00002000

/* Description		RX_MSDU_END_5_RESERVED_5A
			
			<legal 0>
*/
#define RX_MSDU_END_5_RESERVED_5A_OFFSET                             0x00000014
#define RX_MSDU_END_5_RESERVED_5A_LSB                                14
#define RX_MSDU_END_5_RESERVED_5A_MASK                               0x0000c000

/* Description		RX_MSDU_END_5_L3_TYPE
			
			The 16-bit type value indicating the type of L3 later
			extracted from LLC/SNAP, set to zero if SNAP is not
			available
*/
#define RX_MSDU_END_5_L3_TYPE_OFFSET                                 0x00000014
#define RX_MSDU_END_5_L3_TYPE_LSB                                    16
#define RX_MSDU_END_5_L3_TYPE_MASK                                   0xffff0000

/* Description		RX_MSDU_END_6_IPV6_OPTIONS_CRC
			
			32 bit CRC computed out of  IP v6 extension headers
*/
#define RX_MSDU_END_6_IPV6_OPTIONS_CRC_OFFSET                        0x00000018
#define RX_MSDU_END_6_IPV6_OPTIONS_CRC_LSB                           0
#define RX_MSDU_END_6_IPV6_OPTIONS_CRC_MASK                          0xffffffff

/* Description		RX_MSDU_END_7_TCP_SEQ_NUMBER
			
			TCP sequence number (as a number assembled from a TCP
			packet in big-endian order, i.e. requiring a byte-swap for
			little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			
			In Pine, if 'RXOLE_R0_MISC_CONFIG.
			OVERRIDE_MSDU_END_FIELDS' is set, toeplitz_hash_2_or_4 from
			'RX_MSDU_START' will be reported here:
			
			Controlled by multiple RxOLE registers for TCP/UDP over
			IPv4/IPv6 - Either Toeplitz hash computed over 2-tuple IPv4
			or IPv6 src/dest addresses is reported; or, Toeplitz hash
			computed over 4-tuple IPv4 or IPv6 src/dest addresses and
			src/dest ports is reported. The Flow_id_toeplitz hash can
			also be reported here. Usually the hash reported here is the
			one used for hash-based REO routing (see
			use_flow_id_toeplitz_clfy in 'RXPT_CLASSIFY_INFO').
			Optionally the 3-tuple Toeplitz hash over IPv4 or IPv6
			src/dest addresses and L4 protocol can be reported here.
			
			(Unsupported in HastingsPrime)
*/
#define RX_MSDU_END_7_TCP_SEQ_NUMBER_OFFSET                          0x0000001c
#define RX_MSDU_END_7_TCP_SEQ_NUMBER_LSB                             0
#define RX_MSDU_END_7_TCP_SEQ_NUMBER_MASK                            0xffffffff

/* Description		RX_MSDU_END_8_TCP_ACK_NUMBER
			
			TCP acknowledge number (as a number assembled from a TCP
			packet in big-endian order, i.e. requiring a byte-swap for
			little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			
			In Pine, if 'RXOLE_R0_MISC_CONFIG.
			OVERRIDE_MSDU_END_FIELDS' is set, flow_id_toeplitz from
			'RX_MSDU_START' will be reported here:
			
			Toeplitz hash of 5-tuple {IP source address, IP
			destination address, IP source port, IP destination port, L4
			protocol}  in case of non-IPSec. In case of IPSec - Toeplitz
			hash of 4-tuple {IP source address, IP destination address,
			SPI, L4 protocol}. Optionally the 3-tuple Toeplitz hash over
			IPv4 or IPv6 src/dest addresses and L4 protocol can be
			reported here. 
			
			The relevant Toeplitz key registers are provided in
			RxOLE's instance of common parser module. These registers
			are separate from the Toeplitz keys used by ASE/FSE modules
			inside RxOLE. The actual value will be passed on from common
			parser module to RxOLE in one of the WHO_* TLVs.
			
			(Unsupported in HastingsPrime)
*/
#define RX_MSDU_END_8_TCP_ACK_NUMBER_OFFSET                          0x00000020
#define RX_MSDU_END_8_TCP_ACK_NUMBER_LSB                             0
#define RX_MSDU_END_8_TCP_ACK_NUMBER_MASK                            0xffffffff

/* Description		RX_MSDU_END_9_TCP_FLAG
			
			TCP flags
			
			{NS,CWR,ECE,URG,ACK,PSH, RST ,SYN,FIN}(with the NS bit
			in bit 8 and the FIN bit in bit 0, i.e. in big-endian order,
			i.e. requiring a byte-swap for little-endian FW/SW w.r.t.
			the byte order in a packet)
*/
#define RX_MSDU_END_9_TCP_FLAG_OFFSET                                0x00000024
#define RX_MSDU_END_9_TCP_FLAG_LSB                                   0
#define RX_MSDU_END_9_TCP_FLAG_MASK                                  0x000001ff

/* Description		RX_MSDU_END_9_LRO_ELIGIBLE
			
			Computed out of TCP and IP fields to indicate that this
			MSDU is eligible for  LRO
*/
#define RX_MSDU_END_9_LRO_ELIGIBLE_OFFSET                            0x00000024
#define RX_MSDU_END_9_LRO_ELIGIBLE_LSB                               9
#define RX_MSDU_END_9_LRO_ELIGIBLE_MASK                              0x00000200

/* Description		RX_MSDU_END_9_RESERVED_9A
			
			NOTE: DO not assign a field... Internally used in
			RXOLE..
			
			<legal 0>
*/
#define RX_MSDU_END_9_RESERVED_9A_OFFSET                             0x00000024
#define RX_MSDU_END_9_RESERVED_9A_LSB                                10
#define RX_MSDU_END_9_RESERVED_9A_MASK                               0x0000fc00

/* Description		RX_MSDU_END_9_WINDOW_SIZE
			
			TCP receive window size (as a number assembled from a
			TCP packet in big-endian order, i.e. requiring a byte-swap
			for little-endian FW/SW w.r.t. the byte order in a packet)
			
			
			
			In Pine, if 'RXOLE_R0_MISC_CONFIG.
			OVERRIDE_MSDU_END_FIELDS' is set, msdu_length from
			'RX_MSDU_START' will be reported in the 14 LSBs here:
			
			MSDU length in bytes after decapsulation. This field is
			still valid for MPDU frames without A-MSDU.  It still
			represents MSDU length after decapsulation.
			
			(Unsupported in HastingsPrime)
*/
#define RX_MSDU_END_9_WINDOW_SIZE_OFFSET                             0x00000024
#define RX_MSDU_END_9_WINDOW_SIZE_LSB                                16
#define RX_MSDU_END_9_WINDOW_SIZE_MASK                               0xffff0000

/* Description		RX_MSDU_END_10_TCP_UDP_CHKSUM
			
			The value of the computed TCP/UDP checksum.  A mode bit
			selects whether this checksum is the full checksum or the
			partial checksum which does not include the pseudo header.
			(with the first byte in the MSB and the second byte in the
			LSB, i.e. requiring a byte-swap for little-endian FW/SW
			w.r.t. the byte order in a packet)
*/
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_OFFSET                         0x00000028
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_LSB                            0
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_MASK                           0x0000ffff

/* Description		RX_MSDU_END_10_SA_IDX_TIMEOUT
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer.
*/
#define RX_MSDU_END_10_SA_IDX_TIMEOUT_OFFSET                         0x00000028
#define RX_MSDU_END_10_SA_IDX_TIMEOUT_LSB                            16
#define RX_MSDU_END_10_SA_IDX_TIMEOUT_MASK                           0x00010000

/* Description		RX_MSDU_END_10_DA_IDX_TIMEOUT
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer.
*/
#define RX_MSDU_END_10_DA_IDX_TIMEOUT_OFFSET                         0x00000028
#define RX_MSDU_END_10_DA_IDX_TIMEOUT_LSB                            17
#define RX_MSDU_END_10_DA_IDX_TIMEOUT_MASK                           0x00020000

/* Description		RX_MSDU_END_10_MSDU_LIMIT_ERROR
			
			Indicates that the MSDU threshold was exceeded and thus
			all the rest of the MSDUs will not be scattered and will not
			be decapsulated but will be DMA'ed in RAW format as a single
			MSDU buffer
*/
#define RX_MSDU_END_10_MSDU_LIMIT_ERROR_OFFSET                       0x00000028
#define RX_MSDU_END_10_MSDU_LIMIT_ERROR_LSB                          18
#define RX_MSDU_END_10_MSDU_LIMIT_ERROR_MASK                         0x00040000

/* Description		RX_MSDU_END_10_FLOW_IDX_TIMEOUT
			
			Indicates an unsuccessful flow search due to the
			expiring of the search timer.
			
			<legal all>
*/
#define RX_MSDU_END_10_FLOW_IDX_TIMEOUT_OFFSET                       0x00000028
#define RX_MSDU_END_10_FLOW_IDX_TIMEOUT_LSB                          19
#define RX_MSDU_END_10_FLOW_IDX_TIMEOUT_MASK                         0x00080000

/* Description		RX_MSDU_END_10_FLOW_IDX_INVALID
			
			flow id is not valid
			
			<legal all>
*/
#define RX_MSDU_END_10_FLOW_IDX_INVALID_OFFSET                       0x00000028
#define RX_MSDU_END_10_FLOW_IDX_INVALID_LSB                          20
#define RX_MSDU_END_10_FLOW_IDX_INVALID_MASK                         0x00100000

/* Description		RX_MSDU_END_10_WIFI_PARSER_ERROR
			
			Indicates that the WiFi frame has one of the following
			errors
			
			o has less than minimum allowed bytes as per standard
			
			o has incomplete VLAN LLC/SNAP (only for non A-MSDUs)
			
			<legal all>
*/
#define RX_MSDU_END_10_WIFI_PARSER_ERROR_OFFSET                      0x00000028
#define RX_MSDU_END_10_WIFI_PARSER_ERROR_LSB                         21
#define RX_MSDU_END_10_WIFI_PARSER_ERROR_MASK                        0x00200000

/* Description		RX_MSDU_END_10_AMSDU_PARSER_ERROR
			
			A-MSDU could not be properly de-agregated.
			
			<legal all>
*/
#define RX_MSDU_END_10_AMSDU_PARSER_ERROR_OFFSET                     0x00000028
#define RX_MSDU_END_10_AMSDU_PARSER_ERROR_LSB                        22
#define RX_MSDU_END_10_AMSDU_PARSER_ERROR_MASK                       0x00400000

/* Description		RX_MSDU_END_10_SA_IS_VALID
			
			Indicates that OLE found a valid SA entry
*/
#define RX_MSDU_END_10_SA_IS_VALID_OFFSET                            0x00000028
#define RX_MSDU_END_10_SA_IS_VALID_LSB                               23
#define RX_MSDU_END_10_SA_IS_VALID_MASK                              0x00800000

/* Description		RX_MSDU_END_10_DA_IS_VALID
			
			Indicates that OLE found a valid DA entry
*/
#define RX_MSDU_END_10_DA_IS_VALID_OFFSET                            0x00000028
#define RX_MSDU_END_10_DA_IS_VALID_LSB                               24
#define RX_MSDU_END_10_DA_IS_VALID_MASK                              0x01000000

/* Description		RX_MSDU_END_10_DA_IS_MCBC
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address.
*/
#define RX_MSDU_END_10_DA_IS_MCBC_OFFSET                             0x00000028
#define RX_MSDU_END_10_DA_IS_MCBC_LSB                                25
#define RX_MSDU_END_10_DA_IS_MCBC_MASK                               0x02000000

/* Description		RX_MSDU_END_10_L3_HEADER_PADDING
			
			Number of bytes padded  to make sure that the L3 header
			will always start of a Dword   boundary
*/
#define RX_MSDU_END_10_L3_HEADER_PADDING_OFFSET                      0x00000028
#define RX_MSDU_END_10_L3_HEADER_PADDING_LSB                         26
#define RX_MSDU_END_10_L3_HEADER_PADDING_MASK                        0x0c000000

/* Description		RX_MSDU_END_10_FIRST_MSDU
			
			Indicates the first MSDU of A-MSDU.  If both first_msdu
			and last_msdu are set in the MSDU then this is a
			non-aggregated MSDU frame: normal MPDU.  Interior MSDU in an
			A-MSDU shall have both first_mpdu and last_mpdu bits set to
			0.
*/
#define RX_MSDU_END_10_FIRST_MSDU_OFFSET                             0x00000028
#define RX_MSDU_END_10_FIRST_MSDU_LSB                                28
#define RX_MSDU_END_10_FIRST_MSDU_MASK                               0x10000000

/* Description		RX_MSDU_END_10_LAST_MSDU
			
			Indicates the last MSDU of the A-MSDU.  MPDU end status
			is only valid when last_msdu is set.
*/
#define RX_MSDU_END_10_LAST_MSDU_OFFSET                              0x00000028
#define RX_MSDU_END_10_LAST_MSDU_LSB                                 29
#define RX_MSDU_END_10_LAST_MSDU_MASK                                0x20000000

/* Description		RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL
			
			if 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS' is
			set, tcp_udp_chksum_fail from 'RX_ATTENTION' will be
			reported here:
			
			Indicates that the computed checksum (tcp_udp_chksum)
			did not match the checksum in the TCP/UDP header.
			
			(unsupported in HastingsPrime)
*/
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL_OFFSET                    0x00000028
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL_LSB                       30
#define RX_MSDU_END_10_TCP_UDP_CHKSUM_FAIL_MASK                      0x40000000

/* Description		RX_MSDU_END_10_IP_CHKSUM_FAIL
			
			If 'RXOLE_R0_MISC_CONFIG. OVERRIDE_MSDU_END_FIELDS' is
			set, ip_chksum_fail from 'RX_MSDU_START' will be reported in
			the MSB here:
			
			Indicates that the computed checksum (ip_hdr_chksum) did
			not match the checksum in the IP header.
			
			(unsupported in HastingsPrime)
*/
#define RX_MSDU_END_10_IP_CHKSUM_FAIL_OFFSET                         0x00000028
#define RX_MSDU_END_10_IP_CHKSUM_FAIL_LSB                            31
#define RX_MSDU_END_10_IP_CHKSUM_FAIL_MASK                           0x80000000

/* Description		RX_MSDU_END_11_SA_IDX
			
			The offset in the address table which matches the MAC
			source address.
*/
#define RX_MSDU_END_11_SA_IDX_OFFSET                                 0x0000002c
#define RX_MSDU_END_11_SA_IDX_LSB                                    0
#define RX_MSDU_END_11_SA_IDX_MASK                                   0x0000ffff

/* Description		RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID
			
			Based on a register configuration in RXOLE, this field
			will contain:
			
			The offset in the address table which matches the MAC
			destination address
			
			OR:
			
			sw_peer_id from the address search entry corresponding
			to the destination address of the MSDU
*/
#define RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_OFFSET                   0x0000002c
#define RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_LSB                      16
#define RX_MSDU_END_11_DA_IDX_OR_SW_PEER_ID_MASK                     0xffff0000

/* Description		RX_MSDU_END_12_MSDU_DROP
			
			When set, REO shall drop this MSDU and not forward it to
			any other ring...
			
			<legal all>
*/
#define RX_MSDU_END_12_MSDU_DROP_OFFSET                              0x00000030
#define RX_MSDU_END_12_MSDU_DROP_LSB                                 0
#define RX_MSDU_END_12_MSDU_DROP_MASK                                0x00000001

/* Description		RX_MSDU_END_12_REO_DESTINATION_INDICATION
			
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
#define RX_MSDU_END_12_REO_DESTINATION_INDICATION_OFFSET             0x00000030
#define RX_MSDU_END_12_REO_DESTINATION_INDICATION_LSB                1
#define RX_MSDU_END_12_REO_DESTINATION_INDICATION_MASK               0x0000003e

/* Description		RX_MSDU_END_12_FLOW_IDX
			
			Flow table index
			
			<legal all>
*/
#define RX_MSDU_END_12_FLOW_IDX_OFFSET                               0x00000030
#define RX_MSDU_END_12_FLOW_IDX_LSB                                  6
#define RX_MSDU_END_12_FLOW_IDX_MASK                                 0x03ffffc0

/* Description		RX_MSDU_END_12_RESERVED_12A
			
			<legal 0>
*/
#define RX_MSDU_END_12_RESERVED_12A_OFFSET                           0x00000030
#define RX_MSDU_END_12_RESERVED_12A_LSB                              26
#define RX_MSDU_END_12_RESERVED_12A_MASK                             0xfc000000

/* Description		RX_MSDU_END_13_FSE_METADATA
			
			FSE related meta data:
			
			<legal all>
*/
#define RX_MSDU_END_13_FSE_METADATA_OFFSET                           0x00000034
#define RX_MSDU_END_13_FSE_METADATA_LSB                              0
#define RX_MSDU_END_13_FSE_METADATA_MASK                             0xffffffff

/* Description		RX_MSDU_END_14_CCE_METADATA
			
			CCE related meta data:
			
			<legal all>
*/
#define RX_MSDU_END_14_CCE_METADATA_OFFSET                           0x00000038
#define RX_MSDU_END_14_CCE_METADATA_LSB                              0
#define RX_MSDU_END_14_CCE_METADATA_MASK                             0x0000ffff

/* Description		RX_MSDU_END_14_SA_SW_PEER_ID
			
			sw_peer_id from the address search entry corresponding
			to the source address of the MSDU
			
			<legal all>
*/
#define RX_MSDU_END_14_SA_SW_PEER_ID_OFFSET                          0x00000038
#define RX_MSDU_END_14_SA_SW_PEER_ID_LSB                             16
#define RX_MSDU_END_14_SA_SW_PEER_ID_MASK                            0xffff0000

/* Description		RX_MSDU_END_15_AGGREGATION_COUNT
			
			FISA: Number of MSDU's aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_MSDU_END_15_AGGREGATION_COUNT_OFFSET                      0x0000003c
#define RX_MSDU_END_15_AGGREGATION_COUNT_LSB                         0
#define RX_MSDU_END_15_AGGREGATION_COUNT_MASK                        0x000000ff

/* Description		RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION
			
			FISA: To indicate that this MSDU can be aggregated with
			the previous packet with the same flow id
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION_OFFSET          0x0000003c
#define RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION_LSB             8
#define RX_MSDU_END_15_FLOW_AGGREGATION_CONTINUATION_MASK            0x00000100

/* Description		RX_MSDU_END_15_FISA_TIMEOUT
			
			FISA: To indicate that the aggregation has restarted for
			this flow due to timeout
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_MSDU_END_15_FISA_TIMEOUT_OFFSET                           0x0000003c
#define RX_MSDU_END_15_FISA_TIMEOUT_LSB                              9
#define RX_MSDU_END_15_FISA_TIMEOUT_MASK                             0x00000200

/* Description		RX_MSDU_END_15_RESERVED_15A
			
			<legal 0>
*/
#define RX_MSDU_END_15_RESERVED_15A_OFFSET                           0x0000003c
#define RX_MSDU_END_15_RESERVED_15A_LSB                              10
#define RX_MSDU_END_15_RESERVED_15A_MASK                             0xfffffc00

/* Description		RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM
			
			FISA: checksum for MSDU's that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM_OFFSET                 0x00000040
#define RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM_LSB                    0
#define RX_MSDU_END_16_CUMULATIVE_L4_CHECKSUM_MASK                   0x0000ffff

/* Description		RX_MSDU_END_16_CUMULATIVE_IP_LENGTH
			
			FISA: Total MSDU length that is part of this flow
			aggregated so far
			
			
			
			Set to zero in chips not supporting FISA, e.g. Pine
			
			<legal all>
*/
#define RX_MSDU_END_16_CUMULATIVE_IP_LENGTH_OFFSET                   0x00000040
#define RX_MSDU_END_16_CUMULATIVE_IP_LENGTH_LSB                      16
#define RX_MSDU_END_16_CUMULATIVE_IP_LENGTH_MASK                     0xffff0000


#endif // _RX_MSDU_END_H_

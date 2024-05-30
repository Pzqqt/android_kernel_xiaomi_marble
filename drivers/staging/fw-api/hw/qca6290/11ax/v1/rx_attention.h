/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _RX_ATTENTION_H_
#define _RX_ATTENTION_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	rxpcu_mpdu_filter_in_category[1:0], sw_frame_group_id[8:2], reserved_0[15:9], phy_ppdu_id[31:16]
//	1	first_mpdu[0], reserved_1a[1], mcast_bcast[2], ast_index_not_found[3], ast_index_timeout[4], power_mgmt[5], non_qos[6], null_data[7], mgmt_type[8], ctrl_type[9], more_data[10], eosp[11], a_msdu_error[12], fragment_flag[13], order[14], cce_match[15], overflow_err[16], msdu_length_err[17], tcp_udp_chksum_fail[18], ip_chksum_fail[19], sa_idx_invalid[20], da_idx_invalid[21], reserved_1b[22], rx_in_tx_decrypt_byp[23], encrypt_required[24], directed[25], buffer_fragment[26], mpdu_length_err[27], tkip_mic_err[28], decrypt_err[29], unencrypted_frame_err[30], fcs_err[31]
//	2	flow_idx_timeout[0], flow_idx_invalid[1], wifi_parser_error[2], amsdu_parser_error[3], sa_idx_timeout[4], da_idx_timeout[5], msdu_limit_error[6], da_is_valid[7], da_is_mcbc[8], sa_is_valid[9], decrypt_status_code[12:10], rx_bitmap_not_updated[13], reserved_2[30:14], msdu_done[31]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_ATTENTION 3

struct rx_attention {
             uint32_t rxpcu_mpdu_filter_in_category   :  2, //[1:0]
                      sw_frame_group_id               :  7, //[8:2]
                      reserved_0                      :  7, //[15:9]
                      phy_ppdu_id                     : 16; //[31:16]
             uint32_t first_mpdu                      :  1, //[0]
                      reserved_1a                     :  1, //[1]
                      mcast_bcast                     :  1, //[2]
                      ast_index_not_found             :  1, //[3]
                      ast_index_timeout               :  1, //[4]
                      power_mgmt                      :  1, //[5]
                      non_qos                         :  1, //[6]
                      null_data                       :  1, //[7]
                      mgmt_type                       :  1, //[8]
                      ctrl_type                       :  1, //[9]
                      more_data                       :  1, //[10]
                      eosp                            :  1, //[11]
                      a_msdu_error                    :  1, //[12]
                      fragment_flag                   :  1, //[13]
                      order                           :  1, //[14]
                      cce_match                       :  1, //[15]
                      overflow_err                    :  1, //[16]
                      msdu_length_err                 :  1, //[17]
                      tcp_udp_chksum_fail             :  1, //[18]
                      ip_chksum_fail                  :  1, //[19]
                      sa_idx_invalid                  :  1, //[20]
                      da_idx_invalid                  :  1, //[21]
                      reserved_1b                     :  1, //[22]
                      rx_in_tx_decrypt_byp            :  1, //[23]
                      encrypt_required                :  1, //[24]
                      directed                        :  1, //[25]
                      buffer_fragment                 :  1, //[26]
                      mpdu_length_err                 :  1, //[27]
                      tkip_mic_err                    :  1, //[28]
                      decrypt_err                     :  1, //[29]
                      unencrypted_frame_err           :  1, //[30]
                      fcs_err                         :  1; //[31]
             uint32_t flow_idx_timeout                :  1, //[0]
                      flow_idx_invalid                :  1, //[1]
                      wifi_parser_error               :  1, //[2]
                      amsdu_parser_error              :  1, //[3]
                      sa_idx_timeout                  :  1, //[4]
                      da_idx_timeout                  :  1, //[5]
                      msdu_limit_error                :  1, //[6]
                      da_is_valid                     :  1, //[7]
                      da_is_mcbc                      :  1, //[8]
                      sa_is_valid                     :  1, //[9]
                      decrypt_status_code             :  3, //[12:10]
                      rx_bitmap_not_updated           :  1, //[13]
                      reserved_2                      : 17, //[30:14]
                      msdu_done                       :  1; //[31]
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

first_mpdu
			
			Indicates the first MSDU of the PPDU.  If both
			first_mpdu and last_mpdu are set in the MSDU then this is a
			not an A-MPDU frame but a stand alone MPDU.  Interior MPDU
			in an A-MPDU shall have both first_mpdu and last_mpdu bits
			set to 0.  The PPDU start status will only be valid when
			this bit is set.

reserved_1a
			
			<legal 0>

mcast_bcast
			
			Multicast / broadcast indicator.  Only set when the MAC
			address 1 bit 0 is set indicating mcast/bcast and the BSSID
			matches one of the 4 BSSID registers. Only set when
			first_msdu is set.

ast_index_not_found
			
			Only valid when first_msdu is set.
			
			
			
			Indicates no AST matching entries within the the max
			search count.  

ast_index_timeout
			
			Only valid when first_msdu is set.
			
			
			
			Indicates an unsuccessful search in the address seach
			table due to timeout.  

power_mgmt
			
			Power management bit set in the 802.11 header.  Only set
			when first_msdu is set.

non_qos
			
			Set if packet is not a non-QoS data frame.  Only set
			when first_msdu is set.

null_data
			
			Set if frame type indicates either null data or QoS null
			data format.  Only set when first_msdu is set.

mgmt_type
			
			Set if packet is a management packet.  Only set when
			first_msdu is set.

ctrl_type
			
			Set if packet is a control packet.  Only set when
			first_msdu is set.

more_data
			
			Set if more bit in frame control is set.  Only set when
			first_msdu is set.

eosp
			
			Set if the EOSP (end of service period) bit in the QoS
			control field is set.  Only set when first_msdu is set.

a_msdu_error
			
			Set if number of MSDUs in A-MSDU is above a threshold or
			if the size of the MSDU is invalid.  This receive buffer
			will contain all of the remainder of the MSDUs in this MPDU
			without decapsulation.

fragment_flag
			
			Indicates that this is an 802.11 fragment frame.  This
			is set when either the more_frag bit is set in the frame
			control or the fragment number is not zero.  Only set when
			first_msdu is set.

order
			
			Set if the order bit in the frame control is set.  Only
			set when first_msdu is set.

cce_match
			
			Indicates that this status has a corresponding MSDU that
			requires FW processing.  The OLE will have classification
			ring mask registers which will indicate the ring(s) for
			packets and descriptors which need FW attention.

overflow_err
			
			RXPCU Receive FIFO ran out of space to receive the full
			MPDU. Therefor this MPDU is terminated early and is thus
			corrupted.  
			
			
			
			This MPDU will not be ACKed.
			
			RXPCU might still be able to correctly receive the
			following MPDUs in the PPDU if enough fifo space became
			available in time

msdu_length_err
			
			Indicates that the MSDU length from the 802.3
			encapsulated length field extends beyond the MPDU boundary
			or if the length is less than 14 bytes.
			
			Merged with original other_msdu_err: Indicates that the
			MSDU threshold was exceeded and thus all the rest of the
			MSDUs will not be scattered and will not be decasulated but
			will be DMA'ed in RAW format as a single MSDU buffer

tcp_udp_chksum_fail
			
			Indicates that the computed checksum (tcp_udp_chksum)
			did not match the checksum in the TCP/UDP header.

ip_chksum_fail
			
			Indicates that the computed checksum did not match the
			checksum in the IP header.

sa_idx_invalid
			
			Indicates no matching entry was found in the address
			search table for the source MAC address.

da_idx_invalid
			
			Indicates no matching entry was found in the address
			search table for the destination MAC address.

reserved_1b
			

rx_in_tx_decrypt_byp
			
			Indicates that RX packet is not decrypted as Crypto is
			busy with TX packet processing.

encrypt_required
			
			Indicates that this data type frame is not encrypted
			even if the policy for this MPDU requires encryption as
			indicated in the peer entry key type.

directed
			
			MPDU is a directed packet which means that the RA
			matched our STA addresses.  In proxySTA it means that the TA
			matched an entry in our address search table with the
			corresponding no_ack bit is the address search entry
			cleared.

buffer_fragment
			
			Indicates that at least one of the rx buffers has been
			fragmented.  If set the FW should look at the rx_frag_info
			descriptor described below.

mpdu_length_err
			
			Indicates that the MPDU was pre-maturely terminated
			resulting in a truncated MPDU.  Don't trust the MPDU length
			field.

tkip_mic_err
			
			Indicates that the MPDU Michael integrity check failed

decrypt_err
			
			Indicates that the MPDU decrypt integrity check failed
			or CRYPTO received an encrypted frame, but did not get a
			valid corresponding key id in the peer entry.

unencrypted_frame_err
			
			Copied here by RX OLE from the RX_MPDU_END TLV

fcs_err
			
			Indicates that the MPDU FCS check failed

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

sa_idx_timeout
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer.

da_idx_timeout
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer.

msdu_limit_error
			
			Indicates that the MSDU threshold was exceeded and thus
			all the rest of the MSDUs will not be scattered and will not
			be decasulated but will be DMA'ed in RAW format as a single
			MSDU buffer

da_is_valid
			
			Indicates that OLE found a valid DA entry

da_is_mcbc
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address.

sa_is_valid
			
			Indicates that OLE found a valid SA entry

decrypt_status_code
			
			Field provides insight into the decryption performed
			
			
			
			<enum 0 decrypt_ok> Frame had protection enabled and
			decrypted properly 
			
			<enum 1 decrypt_unprotected_frame > Frame is unprotected
			and hence bypassed 
			
			<enum 2 decrypt_data_err > Frame has protection enabled
			and could not be properly decrypted due to MIC/ICV mismatch
			etc. 
			
			<enum 3 decrypt_key_invalid > Frame has protection
			enabled but the key that was required to decrypt this frame
			was not valid 
			
			<enum 4 decrypt_peer_entry_invalid > Frame has
			protection enabled but the key that was required to decrypt
			this frame was not valid
			
			<enum 5 decrypt_other > Reserved for other indications
			
			
			
			<legal 0 - 5>

rx_bitmap_not_updated
			
			Frame is received, but RXPCU could not update the
			receive bitmap due to (temporary) fifo contraints.
			
			<legal all>

reserved_2
			
			<legal 0>

msdu_done
			
			If set indicates that the RX packet data, RX header
			data, RX PPDU start descriptor, RX MPDU start/end
			descriptor, RX MSDU start/end descriptors and RX Attention
			descriptor are all valid.  This bit must be in the last
			octet of the descriptor.
*/


/* Description		RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY
			
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
#define RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET          0x00000000
#define RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB             0
#define RX_ATTENTION_0_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK            0x00000003

/* Description		RX_ATTENTION_0_SW_FRAME_GROUP_ID
			
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
#define RX_ATTENTION_0_SW_FRAME_GROUP_ID_OFFSET                      0x00000000
#define RX_ATTENTION_0_SW_FRAME_GROUP_ID_LSB                         2
#define RX_ATTENTION_0_SW_FRAME_GROUP_ID_MASK                        0x000001fc

/* Description		RX_ATTENTION_0_RESERVED_0
			
			<legal 0>
*/
#define RX_ATTENTION_0_RESERVED_0_OFFSET                             0x00000000
#define RX_ATTENTION_0_RESERVED_0_LSB                                9
#define RX_ATTENTION_0_RESERVED_0_MASK                               0x0000fe00

/* Description		RX_ATTENTION_0_PHY_PPDU_ID
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/
#define RX_ATTENTION_0_PHY_PPDU_ID_OFFSET                            0x00000000
#define RX_ATTENTION_0_PHY_PPDU_ID_LSB                               16
#define RX_ATTENTION_0_PHY_PPDU_ID_MASK                              0xffff0000

/* Description		RX_ATTENTION_1_FIRST_MPDU
			
			Indicates the first MSDU of the PPDU.  If both
			first_mpdu and last_mpdu are set in the MSDU then this is a
			not an A-MPDU frame but a stand alone MPDU.  Interior MPDU
			in an A-MPDU shall have both first_mpdu and last_mpdu bits
			set to 0.  The PPDU start status will only be valid when
			this bit is set.
*/
#define RX_ATTENTION_1_FIRST_MPDU_OFFSET                             0x00000004
#define RX_ATTENTION_1_FIRST_MPDU_LSB                                0
#define RX_ATTENTION_1_FIRST_MPDU_MASK                               0x00000001

/* Description		RX_ATTENTION_1_RESERVED_1A
			
			<legal 0>
*/
#define RX_ATTENTION_1_RESERVED_1A_OFFSET                            0x00000004
#define RX_ATTENTION_1_RESERVED_1A_LSB                               1
#define RX_ATTENTION_1_RESERVED_1A_MASK                              0x00000002

/* Description		RX_ATTENTION_1_MCAST_BCAST
			
			Multicast / broadcast indicator.  Only set when the MAC
			address 1 bit 0 is set indicating mcast/bcast and the BSSID
			matches one of the 4 BSSID registers. Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_1_MCAST_BCAST_OFFSET                            0x00000004
#define RX_ATTENTION_1_MCAST_BCAST_LSB                               2
#define RX_ATTENTION_1_MCAST_BCAST_MASK                              0x00000004

/* Description		RX_ATTENTION_1_AST_INDEX_NOT_FOUND
			
			Only valid when first_msdu is set.
			
			
			
			Indicates no AST matching entries within the the max
			search count.  
*/
#define RX_ATTENTION_1_AST_INDEX_NOT_FOUND_OFFSET                    0x00000004
#define RX_ATTENTION_1_AST_INDEX_NOT_FOUND_LSB                       3
#define RX_ATTENTION_1_AST_INDEX_NOT_FOUND_MASK                      0x00000008

/* Description		RX_ATTENTION_1_AST_INDEX_TIMEOUT
			
			Only valid when first_msdu is set.
			
			
			
			Indicates an unsuccessful search in the address seach
			table due to timeout.  
*/
#define RX_ATTENTION_1_AST_INDEX_TIMEOUT_OFFSET                      0x00000004
#define RX_ATTENTION_1_AST_INDEX_TIMEOUT_LSB                         4
#define RX_ATTENTION_1_AST_INDEX_TIMEOUT_MASK                        0x00000010

/* Description		RX_ATTENTION_1_POWER_MGMT
			
			Power management bit set in the 802.11 header.  Only set
			when first_msdu is set.
*/
#define RX_ATTENTION_1_POWER_MGMT_OFFSET                             0x00000004
#define RX_ATTENTION_1_POWER_MGMT_LSB                                5
#define RX_ATTENTION_1_POWER_MGMT_MASK                               0x00000020

/* Description		RX_ATTENTION_1_NON_QOS
			
			Set if packet is not a non-QoS data frame.  Only set
			when first_msdu is set.
*/
#define RX_ATTENTION_1_NON_QOS_OFFSET                                0x00000004
#define RX_ATTENTION_1_NON_QOS_LSB                                   6
#define RX_ATTENTION_1_NON_QOS_MASK                                  0x00000040

/* Description		RX_ATTENTION_1_NULL_DATA
			
			Set if frame type indicates either null data or QoS null
			data format.  Only set when first_msdu is set.
*/
#define RX_ATTENTION_1_NULL_DATA_OFFSET                              0x00000004
#define RX_ATTENTION_1_NULL_DATA_LSB                                 7
#define RX_ATTENTION_1_NULL_DATA_MASK                                0x00000080

/* Description		RX_ATTENTION_1_MGMT_TYPE
			
			Set if packet is a management packet.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_1_MGMT_TYPE_OFFSET                              0x00000004
#define RX_ATTENTION_1_MGMT_TYPE_LSB                                 8
#define RX_ATTENTION_1_MGMT_TYPE_MASK                                0x00000100

/* Description		RX_ATTENTION_1_CTRL_TYPE
			
			Set if packet is a control packet.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_1_CTRL_TYPE_OFFSET                              0x00000004
#define RX_ATTENTION_1_CTRL_TYPE_LSB                                 9
#define RX_ATTENTION_1_CTRL_TYPE_MASK                                0x00000200

/* Description		RX_ATTENTION_1_MORE_DATA
			
			Set if more bit in frame control is set.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_1_MORE_DATA_OFFSET                              0x00000004
#define RX_ATTENTION_1_MORE_DATA_LSB                                 10
#define RX_ATTENTION_1_MORE_DATA_MASK                                0x00000400

/* Description		RX_ATTENTION_1_EOSP
			
			Set if the EOSP (end of service period) bit in the QoS
			control field is set.  Only set when first_msdu is set.
*/
#define RX_ATTENTION_1_EOSP_OFFSET                                   0x00000004
#define RX_ATTENTION_1_EOSP_LSB                                      11
#define RX_ATTENTION_1_EOSP_MASK                                     0x00000800

/* Description		RX_ATTENTION_1_A_MSDU_ERROR
			
			Set if number of MSDUs in A-MSDU is above a threshold or
			if the size of the MSDU is invalid.  This receive buffer
			will contain all of the remainder of the MSDUs in this MPDU
			without decapsulation.
*/
#define RX_ATTENTION_1_A_MSDU_ERROR_OFFSET                           0x00000004
#define RX_ATTENTION_1_A_MSDU_ERROR_LSB                              12
#define RX_ATTENTION_1_A_MSDU_ERROR_MASK                             0x00001000

/* Description		RX_ATTENTION_1_FRAGMENT_FLAG
			
			Indicates that this is an 802.11 fragment frame.  This
			is set when either the more_frag bit is set in the frame
			control or the fragment number is not zero.  Only set when
			first_msdu is set.
*/
#define RX_ATTENTION_1_FRAGMENT_FLAG_OFFSET                          0x00000004
#define RX_ATTENTION_1_FRAGMENT_FLAG_LSB                             13
#define RX_ATTENTION_1_FRAGMENT_FLAG_MASK                            0x00002000

/* Description		RX_ATTENTION_1_ORDER
			
			Set if the order bit in the frame control is set.  Only
			set when first_msdu is set.
*/
#define RX_ATTENTION_1_ORDER_OFFSET                                  0x00000004
#define RX_ATTENTION_1_ORDER_LSB                                     14
#define RX_ATTENTION_1_ORDER_MASK                                    0x00004000

/* Description		RX_ATTENTION_1_CCE_MATCH
			
			Indicates that this status has a corresponding MSDU that
			requires FW processing.  The OLE will have classification
			ring mask registers which will indicate the ring(s) for
			packets and descriptors which need FW attention.
*/
#define RX_ATTENTION_1_CCE_MATCH_OFFSET                              0x00000004
#define RX_ATTENTION_1_CCE_MATCH_LSB                                 15
#define RX_ATTENTION_1_CCE_MATCH_MASK                                0x00008000

/* Description		RX_ATTENTION_1_OVERFLOW_ERR
			
			RXPCU Receive FIFO ran out of space to receive the full
			MPDU. Therefor this MPDU is terminated early and is thus
			corrupted.  
			
			
			
			This MPDU will not be ACKed.
			
			RXPCU might still be able to correctly receive the
			following MPDUs in the PPDU if enough fifo space became
			available in time
*/
#define RX_ATTENTION_1_OVERFLOW_ERR_OFFSET                           0x00000004
#define RX_ATTENTION_1_OVERFLOW_ERR_LSB                              16
#define RX_ATTENTION_1_OVERFLOW_ERR_MASK                             0x00010000

/* Description		RX_ATTENTION_1_MSDU_LENGTH_ERR
			
			Indicates that the MSDU length from the 802.3
			encapsulated length field extends beyond the MPDU boundary
			or if the length is less than 14 bytes.
			
			Merged with original other_msdu_err: Indicates that the
			MSDU threshold was exceeded and thus all the rest of the
			MSDUs will not be scattered and will not be decasulated but
			will be DMA'ed in RAW format as a single MSDU buffer
*/
#define RX_ATTENTION_1_MSDU_LENGTH_ERR_OFFSET                        0x00000004
#define RX_ATTENTION_1_MSDU_LENGTH_ERR_LSB                           17
#define RX_ATTENTION_1_MSDU_LENGTH_ERR_MASK                          0x00020000

/* Description		RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL
			
			Indicates that the computed checksum (tcp_udp_chksum)
			did not match the checksum in the TCP/UDP header.
*/
#define RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_OFFSET                    0x00000004
#define RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_LSB                       18
#define RX_ATTENTION_1_TCP_UDP_CHKSUM_FAIL_MASK                      0x00040000

/* Description		RX_ATTENTION_1_IP_CHKSUM_FAIL
			
			Indicates that the computed checksum did not match the
			checksum in the IP header.
*/
#define RX_ATTENTION_1_IP_CHKSUM_FAIL_OFFSET                         0x00000004
#define RX_ATTENTION_1_IP_CHKSUM_FAIL_LSB                            19
#define RX_ATTENTION_1_IP_CHKSUM_FAIL_MASK                           0x00080000

/* Description		RX_ATTENTION_1_SA_IDX_INVALID
			
			Indicates no matching entry was found in the address
			search table for the source MAC address.
*/
#define RX_ATTENTION_1_SA_IDX_INVALID_OFFSET                         0x00000004
#define RX_ATTENTION_1_SA_IDX_INVALID_LSB                            20
#define RX_ATTENTION_1_SA_IDX_INVALID_MASK                           0x00100000

/* Description		RX_ATTENTION_1_DA_IDX_INVALID
			
			Indicates no matching entry was found in the address
			search table for the destination MAC address.
*/
#define RX_ATTENTION_1_DA_IDX_INVALID_OFFSET                         0x00000004
#define RX_ATTENTION_1_DA_IDX_INVALID_LSB                            21
#define RX_ATTENTION_1_DA_IDX_INVALID_MASK                           0x00200000

/* Description		RX_ATTENTION_1_RESERVED_1B
			
*/
#define RX_ATTENTION_1_RESERVED_1B_OFFSET                            0x00000004
#define RX_ATTENTION_1_RESERVED_1B_LSB                               22
#define RX_ATTENTION_1_RESERVED_1B_MASK                              0x00400000

/* Description		RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP
			
			Indicates that RX packet is not decrypted as Crypto is
			busy with TX packet processing.
*/
#define RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP_OFFSET                   0x00000004
#define RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP_LSB                      23
#define RX_ATTENTION_1_RX_IN_TX_DECRYPT_BYP_MASK                     0x00800000

/* Description		RX_ATTENTION_1_ENCRYPT_REQUIRED
			
			Indicates that this data type frame is not encrypted
			even if the policy for this MPDU requires encryption as
			indicated in the peer entry key type.
*/
#define RX_ATTENTION_1_ENCRYPT_REQUIRED_OFFSET                       0x00000004
#define RX_ATTENTION_1_ENCRYPT_REQUIRED_LSB                          24
#define RX_ATTENTION_1_ENCRYPT_REQUIRED_MASK                         0x01000000

/* Description		RX_ATTENTION_1_DIRECTED
			
			MPDU is a directed packet which means that the RA
			matched our STA addresses.  In proxySTA it means that the TA
			matched an entry in our address search table with the
			corresponding no_ack bit is the address search entry
			cleared.
*/
#define RX_ATTENTION_1_DIRECTED_OFFSET                               0x00000004
#define RX_ATTENTION_1_DIRECTED_LSB                                  25
#define RX_ATTENTION_1_DIRECTED_MASK                                 0x02000000

/* Description		RX_ATTENTION_1_BUFFER_FRAGMENT
			
			Indicates that at least one of the rx buffers has been
			fragmented.  If set the FW should look at the rx_frag_info
			descriptor described below.
*/
#define RX_ATTENTION_1_BUFFER_FRAGMENT_OFFSET                        0x00000004
#define RX_ATTENTION_1_BUFFER_FRAGMENT_LSB                           26
#define RX_ATTENTION_1_BUFFER_FRAGMENT_MASK                          0x04000000

/* Description		RX_ATTENTION_1_MPDU_LENGTH_ERR
			
			Indicates that the MPDU was pre-maturely terminated
			resulting in a truncated MPDU.  Don't trust the MPDU length
			field.
*/
#define RX_ATTENTION_1_MPDU_LENGTH_ERR_OFFSET                        0x00000004
#define RX_ATTENTION_1_MPDU_LENGTH_ERR_LSB                           27
#define RX_ATTENTION_1_MPDU_LENGTH_ERR_MASK                          0x08000000

/* Description		RX_ATTENTION_1_TKIP_MIC_ERR
			
			Indicates that the MPDU Michael integrity check failed
*/
#define RX_ATTENTION_1_TKIP_MIC_ERR_OFFSET                           0x00000004
#define RX_ATTENTION_1_TKIP_MIC_ERR_LSB                              28
#define RX_ATTENTION_1_TKIP_MIC_ERR_MASK                             0x10000000

/* Description		RX_ATTENTION_1_DECRYPT_ERR
			
			Indicates that the MPDU decrypt integrity check failed
			or CRYPTO received an encrypted frame, but did not get a
			valid corresponding key id in the peer entry.
*/
#define RX_ATTENTION_1_DECRYPT_ERR_OFFSET                            0x00000004
#define RX_ATTENTION_1_DECRYPT_ERR_LSB                               29
#define RX_ATTENTION_1_DECRYPT_ERR_MASK                              0x20000000

/* Description		RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR
			
			Copied here by RX OLE from the RX_MPDU_END TLV
*/
#define RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR_OFFSET                  0x00000004
#define RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR_LSB                     30
#define RX_ATTENTION_1_UNENCRYPTED_FRAME_ERR_MASK                    0x40000000

/* Description		RX_ATTENTION_1_FCS_ERR
			
			Indicates that the MPDU FCS check failed
*/
#define RX_ATTENTION_1_FCS_ERR_OFFSET                                0x00000004
#define RX_ATTENTION_1_FCS_ERR_LSB                                   31
#define RX_ATTENTION_1_FCS_ERR_MASK                                  0x80000000

/* Description		RX_ATTENTION_2_FLOW_IDX_TIMEOUT
			
			Indicates an unsuccessful flow search due to the
			expiring of the search timer.
			
			<legal all>
*/
#define RX_ATTENTION_2_FLOW_IDX_TIMEOUT_OFFSET                       0x00000008
#define RX_ATTENTION_2_FLOW_IDX_TIMEOUT_LSB                          0
#define RX_ATTENTION_2_FLOW_IDX_TIMEOUT_MASK                         0x00000001

/* Description		RX_ATTENTION_2_FLOW_IDX_INVALID
			
			flow id is not valid
			
			<legal all>
*/
#define RX_ATTENTION_2_FLOW_IDX_INVALID_OFFSET                       0x00000008
#define RX_ATTENTION_2_FLOW_IDX_INVALID_LSB                          1
#define RX_ATTENTION_2_FLOW_IDX_INVALID_MASK                         0x00000002

/* Description		RX_ATTENTION_2_WIFI_PARSER_ERROR
			
			Indicates that the WiFi frame has one of the following
			errors
			
			o has less than minimum allowed bytes as per standard
			
			o has incomplete VLAN LLC/SNAP (only for non A-MSDUs)
			
			<legal all>
*/
#define RX_ATTENTION_2_WIFI_PARSER_ERROR_OFFSET                      0x00000008
#define RX_ATTENTION_2_WIFI_PARSER_ERROR_LSB                         2
#define RX_ATTENTION_2_WIFI_PARSER_ERROR_MASK                        0x00000004

/* Description		RX_ATTENTION_2_AMSDU_PARSER_ERROR
			
			A-MSDU could not be properly de-agregated.
			
			<legal all>
*/
#define RX_ATTENTION_2_AMSDU_PARSER_ERROR_OFFSET                     0x00000008
#define RX_ATTENTION_2_AMSDU_PARSER_ERROR_LSB                        3
#define RX_ATTENTION_2_AMSDU_PARSER_ERROR_MASK                       0x00000008

/* Description		RX_ATTENTION_2_SA_IDX_TIMEOUT
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer.
*/
#define RX_ATTENTION_2_SA_IDX_TIMEOUT_OFFSET                         0x00000008
#define RX_ATTENTION_2_SA_IDX_TIMEOUT_LSB                            4
#define RX_ATTENTION_2_SA_IDX_TIMEOUT_MASK                           0x00000010

/* Description		RX_ATTENTION_2_DA_IDX_TIMEOUT
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer.
*/
#define RX_ATTENTION_2_DA_IDX_TIMEOUT_OFFSET                         0x00000008
#define RX_ATTENTION_2_DA_IDX_TIMEOUT_LSB                            5
#define RX_ATTENTION_2_DA_IDX_TIMEOUT_MASK                           0x00000020

/* Description		RX_ATTENTION_2_MSDU_LIMIT_ERROR
			
			Indicates that the MSDU threshold was exceeded and thus
			all the rest of the MSDUs will not be scattered and will not
			be decasulated but will be DMA'ed in RAW format as a single
			MSDU buffer
*/
#define RX_ATTENTION_2_MSDU_LIMIT_ERROR_OFFSET                       0x00000008
#define RX_ATTENTION_2_MSDU_LIMIT_ERROR_LSB                          6
#define RX_ATTENTION_2_MSDU_LIMIT_ERROR_MASK                         0x00000040

/* Description		RX_ATTENTION_2_DA_IS_VALID
			
			Indicates that OLE found a valid DA entry
*/
#define RX_ATTENTION_2_DA_IS_VALID_OFFSET                            0x00000008
#define RX_ATTENTION_2_DA_IS_VALID_LSB                               7
#define RX_ATTENTION_2_DA_IS_VALID_MASK                              0x00000080

/* Description		RX_ATTENTION_2_DA_IS_MCBC
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address.
*/
#define RX_ATTENTION_2_DA_IS_MCBC_OFFSET                             0x00000008
#define RX_ATTENTION_2_DA_IS_MCBC_LSB                                8
#define RX_ATTENTION_2_DA_IS_MCBC_MASK                               0x00000100

/* Description		RX_ATTENTION_2_SA_IS_VALID
			
			Indicates that OLE found a valid SA entry
*/
#define RX_ATTENTION_2_SA_IS_VALID_OFFSET                            0x00000008
#define RX_ATTENTION_2_SA_IS_VALID_LSB                               9
#define RX_ATTENTION_2_SA_IS_VALID_MASK                              0x00000200

/* Description		RX_ATTENTION_2_DECRYPT_STATUS_CODE
			
			Field provides insight into the decryption performed
			
			
			
			<enum 0 decrypt_ok> Frame had protection enabled and
			decrypted properly 
			
			<enum 1 decrypt_unprotected_frame > Frame is unprotected
			and hence bypassed 
			
			<enum 2 decrypt_data_err > Frame has protection enabled
			and could not be properly decrypted due to MIC/ICV mismatch
			etc. 
			
			<enum 3 decrypt_key_invalid > Frame has protection
			enabled but the key that was required to decrypt this frame
			was not valid 
			
			<enum 4 decrypt_peer_entry_invalid > Frame has
			protection enabled but the key that was required to decrypt
			this frame was not valid
			
			<enum 5 decrypt_other > Reserved for other indications
			
			
			
			<legal 0 - 5>
*/
#define RX_ATTENTION_2_DECRYPT_STATUS_CODE_OFFSET                    0x00000008
#define RX_ATTENTION_2_DECRYPT_STATUS_CODE_LSB                       10
#define RX_ATTENTION_2_DECRYPT_STATUS_CODE_MASK                      0x00001c00

/* Description		RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED
			
			Frame is received, but RXPCU could not update the
			receive bitmap due to (temporary) fifo contraints.
			
			<legal all>
*/
#define RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED_OFFSET                  0x00000008
#define RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED_LSB                     13
#define RX_ATTENTION_2_RX_BITMAP_NOT_UPDATED_MASK                    0x00002000

/* Description		RX_ATTENTION_2_RESERVED_2
			
			<legal 0>
*/
#define RX_ATTENTION_2_RESERVED_2_OFFSET                             0x00000008
#define RX_ATTENTION_2_RESERVED_2_LSB                                14
#define RX_ATTENTION_2_RESERVED_2_MASK                               0x7fffc000

/* Description		RX_ATTENTION_2_MSDU_DONE
			
			If set indicates that the RX packet data, RX header
			data, RX PPDU start descriptor, RX MPDU start/end
			descriptor, RX MSDU start/end descriptors and RX Attention
			descriptor are all valid.  This bit must be in the last
			octet of the descriptor.
*/
#define RX_ATTENTION_2_MSDU_DONE_OFFSET                              0x00000008
#define RX_ATTENTION_2_MSDU_DONE_LSB                                 31
#define RX_ATTENTION_2_MSDU_DONE_MASK                                0x80000000


#endif // _RX_ATTENTION_H_

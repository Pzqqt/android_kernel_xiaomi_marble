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

#ifndef _RX_MSDU_DESC_INFO_H_
#define _RX_MSDU_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	first_msdu_in_mpdu_flag[0], last_msdu_in_mpdu_flag[1], msdu_continuation[2], msdu_length[16:3], reo_destination_indication[21:17], msdu_drop[22], sa_is_valid[23], sa_idx_timeout[24], da_is_valid[25], da_is_mcbc[26], da_idx_timeout[27], l3_header_padding_msb[28], tcp_udp_chksum_fail[29], ip_chksum_fail[30], raw_mpdu[31]
//	1	sa_idx_or_sw_peer_id_14_0[14:0], mpdu_ast_idx_or_sw_peer_id_14_0[29:15], fr_ds[30], to_ds[31]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_MSDU_DESC_INFO 2

struct rx_msdu_desc_info {
             uint32_t first_msdu_in_mpdu_flag         :  1, //[0]
                      last_msdu_in_mpdu_flag          :  1, //[1]
                      msdu_continuation               :  1, //[2]
                      msdu_length                     : 14, //[16:3]
                      reo_destination_indication      :  5, //[21:17]
                      msdu_drop                       :  1, //[22]
                      sa_is_valid                     :  1, //[23]
                      sa_idx_timeout                  :  1, //[24]
                      da_is_valid                     :  1, //[25]
                      da_is_mcbc                      :  1, //[26]
                      da_idx_timeout                  :  1, //[27]
                      l3_header_padding_msb           :  1, //[28]
                      tcp_udp_chksum_fail             :  1, //[29]
                      ip_chksum_fail                  :  1, //[30]
                      raw_mpdu                        :  1; //[31]
             uint32_t sa_idx_or_sw_peer_id_14_0       : 15, //[14:0]
                      mpdu_ast_idx_or_sw_peer_id_14_0 : 15, //[29:15]
                      fr_ds                           :  1, //[30]
                      to_ds                           :  1; //[31]
};

/*

first_msdu_in_mpdu_flag
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU 
			
			
			
			<enum 0 Not_first_msdu> This is not the first MSDU in
			the MPDU. 
			
			<enum 1 first_msdu> This MSDU is the first one in the
			MPDU.
			
			
			
			<legal all>

last_msdu_in_mpdu_flag
			
			Consumer: WBM/REO/SW/FW
			
			Producer: RXDMA
			
			
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to
			this MSDU that belongs to this MPDU 
			
			<enum 1 Last_msdu> this MSDU is the last one in the
			MPDU. This setting is only allowed in combination with
			'Msdu_continuation' set to 0. This implies that when an msdu
			is spread out over multiple buffers and thus
			msdu_continuation is set, only for the very last buffer of
			the msdu, can the 'last_msdu_in_mpdu_flag' be set.
			
			
			
			When both first_msdu_in_mpdu_flag and
			last_msdu_in_mpdu_flag are set, the MPDU that this MSDU
			belongs to only contains a single MSDU.
			
			
			
			
			
			<legal all>

msdu_continuation
			
			When set, this MSDU buffer was not able to hold the
			entire MSDU. The next buffer will therefor contain
			additional information related to this MSDU.
			
			
			
			<legal all>

msdu_length
			
			Parsed from RX_MSDU_START TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the First
			buffer used by MSDU.
			
			 
			
			Full MSDU length in bytes after decapsulation. 
			
			
			
			This field is still valid for MPDU frames without
			A-MSDU.  It still represents MSDU length after decapsulation
			
			
			
			Or in case of RAW MPDUs, it indicates the length of the
			entire MPDU (without FCS field)
			
			<legal all>

reo_destination_indication
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
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
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			When set, REO shall drop this MSDU and not forward it to
			any other ring...
			
			<legal all>

sa_is_valid
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates that OLE found a valid SA entry for this MSDU
			
			<legal all>

sa_idx_timeout
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer for this MSDU
			
			<legal all>

da_is_valid
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates that OLE found a valid DA entry for this MSDU
			
			<legal all>

da_is_mcbc
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address for this MSDU
			
			<legal all>

da_idx_timeout
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer for this MSDU
			
			<legal all>

l3_header_padding_msb
			
			Passed on from 'RX_MSDU_END' TLV (only the MSB is
			reported as the LSB is always zero)
			
			Number of bytes padded to make sure that the L3 header
			will always start of a Dword boundary
			
			<legal all>

tcp_udp_chksum_fail
			
			Passed on from 'RX_ATTENTION' TLV
			
			Indicates that the computed checksum did not match the
			checksum in the TCP/UDP header.
			
			<legal all>

ip_chksum_fail
			
			Passed on from 'RX_ATTENTION' TLV
			
			Indicates that the computed checksum did not match the
			checksum in the IP header.
			
			<legal all>

raw_mpdu
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV
			
			Set to 1 by RXOLE when it has not performed any 802.11
			to Ethernet/Natvie WiFi header conversion on this MPDU.
			
			<legal all>

sa_idx_or_sw_peer_id_14_0
			
			Passed on from 'RX_MSDU_END' TLV (one MSB is omitted)
			
			Based on a register configuration in RXDMA, this field
			will contain: 
			
			The offset in the address search table which matches the
			MAC source address
			
			OR
			
			
			
			'sw_peer_id' from the address search entry corresponding
			to the source address of the MSDU
			
			<legal all>

mpdu_ast_idx_or_sw_peer_id_14_0
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV (one MSB is omitted)
			
			 
			
			Based on a register configuration in RXDMA, this field
			will contain: 
			
			The index of the address search entry corresponding to
			this MPDU (a value of 0xFFFF indicates an invalid AST index,
			meaning that no AST entry was found or no AST search was
			performed)
			
			
			
			OR:
			
			
			
			'sw_peer_id' from the address search entry corresponding
			to this MPDU (in case of ndp or phy_err or
			AST_based_lookup_valid == 0, this field will be set to 0)
			
			<legal all>

fr_ds
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV
			
			Set if the 'from DS' bit is set in the frame control.
			
			<legal all>

to_ds
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV
			
			Set if the 'to DS' bit is set in the frame control.
			
			<legal all>
*/


/* Description		RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU 
			
			
			
			<enum 0 Not_first_msdu> This is not the first MSDU in
			the MPDU. 
			
			<enum 1 first_msdu> This MSDU is the first one in the
			MPDU.
			
			
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_OFFSET           0x00000000
#define RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_LSB              0
#define RX_MSDU_DESC_INFO_0_FIRST_MSDU_IN_MPDU_FLAG_MASK             0x00000001

/* Description		RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG
			
			Consumer: WBM/REO/SW/FW
			
			Producer: RXDMA
			
			
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to
			this MSDU that belongs to this MPDU 
			
			<enum 1 Last_msdu> this MSDU is the last one in the
			MPDU. This setting is only allowed in combination with
			'Msdu_continuation' set to 0. This implies that when an msdu
			is spread out over multiple buffers and thus
			msdu_continuation is set, only for the very last buffer of
			the msdu, can the 'last_msdu_in_mpdu_flag' be set.
			
			
			
			When both first_msdu_in_mpdu_flag and
			last_msdu_in_mpdu_flag are set, the MPDU that this MSDU
			belongs to only contains a single MSDU.
			
			
			
			
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_OFFSET            0x00000000
#define RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_LSB               1
#define RX_MSDU_DESC_INFO_0_LAST_MSDU_IN_MPDU_FLAG_MASK              0x00000002

/* Description		RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION
			
			When set, this MSDU buffer was not able to hold the
			entire MSDU. The next buffer will therefor contain
			additional information related to this MSDU.
			
			
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_OFFSET                 0x00000000
#define RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_LSB                    2
#define RX_MSDU_DESC_INFO_0_MSDU_CONTINUATION_MASK                   0x00000004

/* Description		RX_MSDU_DESC_INFO_0_MSDU_LENGTH
			
			Parsed from RX_MSDU_START TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the First
			buffer used by MSDU.
			
			 
			
			Full MSDU length in bytes after decapsulation. 
			
			
			
			This field is still valid for MPDU frames without
			A-MSDU.  It still represents MSDU length after decapsulation
			
			
			
			Or in case of RAW MPDUs, it indicates the length of the
			entire MPDU (without FCS field)
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_MSDU_LENGTH_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_MSDU_LENGTH_LSB                          3
#define RX_MSDU_DESC_INFO_0_MSDU_LENGTH_MASK                         0x0001fff8

/* Description		RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
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
#define RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_OFFSET        0x00000000
#define RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_LSB           17
#define RX_MSDU_DESC_INFO_0_REO_DESTINATION_INDICATION_MASK          0x003e0000

/* Description		RX_MSDU_DESC_INFO_0_MSDU_DROP
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			When set, REO shall drop this MSDU and not forward it to
			any other ring...
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_MSDU_DROP_OFFSET                         0x00000000
#define RX_MSDU_DESC_INFO_0_MSDU_DROP_LSB                            22
#define RX_MSDU_DESC_INFO_0_MSDU_DROP_MASK                           0x00400000

/* Description		RX_MSDU_DESC_INFO_0_SA_IS_VALID
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates that OLE found a valid SA entry for this MSDU
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_SA_IS_VALID_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_SA_IS_VALID_LSB                          23
#define RX_MSDU_DESC_INFO_0_SA_IS_VALID_MASK                         0x00800000

/* Description		RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates an unsuccessful MAC source address search due
			to the expiring of the search timer for this MSDU
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_OFFSET                    0x00000000
#define RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_LSB                       24
#define RX_MSDU_DESC_INFO_0_SA_IDX_TIMEOUT_MASK                      0x01000000

/* Description		RX_MSDU_DESC_INFO_0_DA_IS_VALID
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates that OLE found a valid DA entry for this MSDU
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_DA_IS_VALID_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_DA_IS_VALID_LSB                          25
#define RX_MSDU_DESC_INFO_0_DA_IS_VALID_MASK                         0x02000000

/* Description		RX_MSDU_DESC_INFO_0_DA_IS_MCBC
			
			Field Only valid if da_is_valid is set
			
			
			
			Indicates the DA address was a Multicast of Broadcast
			address for this MSDU
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_DA_IS_MCBC_OFFSET                        0x00000000
#define RX_MSDU_DESC_INFO_0_DA_IS_MCBC_LSB                           26
#define RX_MSDU_DESC_INFO_0_DA_IS_MCBC_MASK                          0x04000000

/* Description		RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans
			over multiple buffers, this field will be valid in the Last
			buffer used by the MSDU
			
			 
			
			Indicates an unsuccessful MAC destination address search
			due to the expiring of the search timer for this MSDU
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_OFFSET                    0x00000000
#define RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_LSB                       27
#define RX_MSDU_DESC_INFO_0_DA_IDX_TIMEOUT_MASK                      0x08000000

/* Description		RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB
			
			Passed on from 'RX_MSDU_END' TLV (only the MSB is
			reported as the LSB is always zero)
			
			Number of bytes padded to make sure that the L3 header
			will always start of a Dword boundary
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB_OFFSET             0x00000000
#define RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB_LSB                28
#define RX_MSDU_DESC_INFO_0_L3_HEADER_PADDING_MSB_MASK               0x10000000

/* Description		RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL
			
			Passed on from 'RX_ATTENTION' TLV
			
			Indicates that the computed checksum did not match the
			checksum in the TCP/UDP header.
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL_OFFSET               0x00000000
#define RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL_LSB                  29
#define RX_MSDU_DESC_INFO_0_TCP_UDP_CHKSUM_FAIL_MASK                 0x20000000

/* Description		RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL
			
			Passed on from 'RX_ATTENTION' TLV
			
			Indicates that the computed checksum did not match the
			checksum in the IP header.
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL_OFFSET                    0x00000000
#define RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL_LSB                       30
#define RX_MSDU_DESC_INFO_0_IP_CHKSUM_FAIL_MASK                      0x40000000

/* Description		RX_MSDU_DESC_INFO_0_RAW_MPDU
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV
			
			Set to 1 by RXOLE when it has not performed any 802.11
			to Ethernet/Natvie WiFi header conversion on this MPDU.
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_0_RAW_MPDU_OFFSET                          0x00000000
#define RX_MSDU_DESC_INFO_0_RAW_MPDU_LSB                             31
#define RX_MSDU_DESC_INFO_0_RAW_MPDU_MASK                            0x80000000

/* Description		RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0
			
			Passed on from 'RX_MSDU_END' TLV (one MSB is omitted)
			
			Based on a register configuration in RXDMA, this field
			will contain: 
			
			The offset in the address search table which matches the
			MAC source address
			
			OR
			
			
			
			'sw_peer_id' from the address search entry corresponding
			to the source address of the MSDU
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0_OFFSET         0x00000004
#define RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0_LSB            0
#define RX_MSDU_DESC_INFO_1_SA_IDX_OR_SW_PEER_ID_14_0_MASK           0x00007fff

/* Description		RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV (one MSB is omitted)
			
			 
			
			Based on a register configuration in RXDMA, this field
			will contain: 
			
			The index of the address search entry corresponding to
			this MPDU (a value of 0xFFFF indicates an invalid AST index,
			meaning that no AST entry was found or no AST search was
			performed)
			
			
			
			OR:
			
			
			
			'sw_peer_id' from the address search entry corresponding
			to this MPDU (in case of ndp or phy_err or
			AST_based_lookup_valid == 0, this field will be set to 0)
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0_OFFSET   0x00000004
#define RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0_LSB      15
#define RX_MSDU_DESC_INFO_1_MPDU_AST_IDX_OR_SW_PEER_ID_14_0_MASK     0x3fff8000

/* Description		RX_MSDU_DESC_INFO_1_FR_DS
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV
			
			Set if the 'from DS' bit is set in the frame control.
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_1_FR_DS_OFFSET                             0x00000004
#define RX_MSDU_DESC_INFO_1_FR_DS_LSB                                30
#define RX_MSDU_DESC_INFO_1_FR_DS_MASK                               0x40000000

/* Description		RX_MSDU_DESC_INFO_1_TO_DS
			
			Passed on from 'RX_MPDU_INFO' structure in
			'RX_MPDU_START' TLV
			
			Set if the 'to DS' bit is set in the frame control.
			
			<legal all>
*/
#define RX_MSDU_DESC_INFO_1_TO_DS_OFFSET                             0x00000004
#define RX_MSDU_DESC_INFO_1_TO_DS_LSB                                31
#define RX_MSDU_DESC_INFO_1_TO_DS_MASK                               0x80000000


#endif // _RX_MSDU_DESC_INFO_H_

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

#ifndef _RX_MSDU_DESC_INFO_H_
#define _RX_MSDU_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	first_msdu_in_mpdu_flag[0], last_msdu_in_mpdu_flag[1], msdu_continuation[2], msdu_length[16:3], reo_destination_indication[21:17], msdu_drop[22], sa_is_valid[23], sa_idx_timeout[24], da_is_valid[25], da_is_mcbc[26], da_idx_timeout[27], reserved_0a[31:28]
//	1	reserved_1a[31:0]
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
                      reserved_0a                     :  4; //[31:28]
             uint32_t reserved_1a                     : 32; //[31:0]
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
			into the REO2SW1 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_7> REO remaps this
			
			<enum 8 reo_destination_8> REO remaps this <enum 9
			reo_destination_9> REO remaps this <enum 10
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

reserved_0a
			
			<legal 0>

reserved_1a
			
			<legal 0>
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
			into the REO2SW1 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_7> REO remaps this
			
			<enum 8 reo_destination_8> REO remaps this <enum 9
			reo_destination_9> REO remaps this <enum 10
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

/* Description		RX_MSDU_DESC_INFO_0_RESERVED_0A
			
			<legal 0>
*/
#define RX_MSDU_DESC_INFO_0_RESERVED_0A_OFFSET                       0x00000000
#define RX_MSDU_DESC_INFO_0_RESERVED_0A_LSB                          28
#define RX_MSDU_DESC_INFO_0_RESERVED_0A_MASK                         0xf0000000

/* Description		RX_MSDU_DESC_INFO_1_RESERVED_1A
			
			<legal 0>
*/
#define RX_MSDU_DESC_INFO_1_RESERVED_1A_OFFSET                       0x00000004
#define RX_MSDU_DESC_INFO_1_RESERVED_1A_LSB                          0
#define RX_MSDU_DESC_INFO_1_RESERVED_1A_MASK                         0xffffffff


#endif // _RX_MSDU_DESC_INFO_H_

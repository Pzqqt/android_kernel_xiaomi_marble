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

#ifndef _RECEIVED_TRIGGER_INFO_H_
#define _RECEIVED_TRIGGER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#include "received_trigger_info_details.h"
#define NUM_OF_DWORDS_RECEIVED_TRIGGER_INFO 6

#define NUM_OF_QWORDS_RECEIVED_TRIGGER_INFO 3


struct received_trigger_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   received_trigger_info_details                             received_trigger_details;
             uint32_t tlv64_padding                                           : 32; // [31:0]
#else
             struct   received_trigger_info_details                             received_trigger_details;
             uint32_t tlv64_padding                                           : 32; // [31:0]
#endif
};


/* Description		RECEIVED_TRIGGER_DETAILS

			Info related to the type of trigger (that potentially requires
			 SIFS response) that was received
*/


/* Description		TRIGGER_TYPE

			This field indicates for what type of trigger has been received
			
			
			<enum 0 SCH_Qboost_trigger> 
			<enum 1 SCH_PSPOLL_trigger>
			<enum 2 SCH_UAPSD_trigger>
			<enum 3 SCH_11ax_trigger> 
			Field "AX_trigger_type" indicates the ID of the received
			 trigger
			<enum 7 SCH_EHT_trigger> 
			Field "AX_trigger_type" indicates the ID of the received
			 trigger
			<enum 4 SCH_11ax_wildcard_trigger> 
			Field "AX_trigger_type" indicates the ID of the received
			 trigger
			<enum 5 SCH_11ax_unassoc_wildcard_trigger> 
			Field "AX_trigger_type" indicates the ID of the received
			 trigger
			<enum 6 SCH_11az_ranging_trigger> Field "AX_trigger_type" 
			indicates the subtype of the received trigger
			
			<legal 0-7>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_TYPE_OFFSET          0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_TYPE_LSB             0
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_TYPE_MSB             3
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_TYPE_MASK            0x000000000000000f


/* Description		AX_TRIGGER_SOURCE

			Field Only valid when Trigger_type  is an 11ax related trigger
			 
			
			<enum 0 11ax_trigger_frame>
			<enum 1 he_control_based_trigger>
			
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_OFFSET     0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_LSB        4
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_MSB        4
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_SOURCE_MASK       0x0000000000000010


/* Description		AX_TRIGGER_TYPE

			Field Only valid when Trigger_type  is an 11ax related trigger
			
			
			The 11AX trigger type/ trigger number:
			It identifies which trigger was received.
			<enum 0 ax_trigger_basic>
			<enum 1 ax_trigger_brpoll>
			<enum 2 ax_trigger_mu_bar>
			<enum 3 ax_trigger_mu_rts>
			<enum 4 ax_trigger_buffer_size>
			<enum 5 ax_trigger_gcr_mu_bar>
			<enum 6 ax_trigger_BQRP> 
			<enum 7 ax_trigger_NDP_fb_report_poll> 
			<enum 8 ax_tb_ranging_trigger> Indicates the reception of
			 Ranging Trigger Frame of subvariant indicated by Ranging_Trigger_Subtype
			
			<enum 9 ax_trigger_reserved_9>
			<enum 10 ax_trigger_reserved_10>
			<enum 11 ax_trigger_reserved_11>
			<enum 12 ax_trigger_reserved_12>
			<enum 13 ax_trigger_reserved_13>
			<enum 14 ax_trigger_reserved_14>
			<enum 15 ax_trigger_reserved_15>
			
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_TYPE_OFFSET       0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_TYPE_LSB          5
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_TYPE_MSB          8
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_AX_TRIGGER_TYPE_MASK         0x00000000000001e0


/* Description		TRIGGER_SOURCE_STA_FULL_AID

			The sta_full_aid of the sta/ap that generated the trigger.
			
			Comes from the address_search_entry
			
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_OFFSET 0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_LSB 9
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_MSB 21
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_TRIGGER_SOURCE_STA_FULL_AID_MASK 0x00000000003ffe00


/* Description		FRAME_CONTROL_VALID

			When set, the 'frame_control' field contains valid info
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_VALID_OFFSET   0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_VALID_LSB      22
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_VALID_MSB      22
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_VALID_MASK     0x0000000000400000


/* Description		QOS_CONTROL_VALID

			When set, the 'QoS_control' field contains valid info
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_VALID_OFFSET     0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_VALID_LSB        23
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_VALID_MSB        23
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_VALID_MASK       0x0000000000800000


/* Description		HE_CONTROL_INFO_VALID

			When set, the 'HE control' field contains valid info
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_INFO_VALID_OFFSET 0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_INFO_VALID_LSB    24
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_INFO_VALID_MSB    24
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_INFO_VALID_MASK   0x0000000001000000


/* Description		RANGING_TRIGGER_SUBTYPE

			Field only valid if  AX_Trigger_type = ax_tb_ranging_trigger
			
			
			Indicates the Trigger subtype for the current ranging TF
			
			
			<enum 0 TF_Poll>
			<enum 1 TF_Sound>
			<enum 2 TF_Secure_Sound>
			<enum 3 TF_Report>
			
			<legal 0-3>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_OFFSET 0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_LSB  25
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_MSB  28
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RANGING_TRIGGER_SUBTYPE_MASK 0x000000001e000000


/* Description		RESERVED_0B

			<legal 0>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_0B_OFFSET           0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_0B_LSB              29
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_0B_MSB              31
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_0B_MASK             0x00000000e0000000


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_PHY_PPDU_ID_OFFSET           0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_PHY_PPDU_ID_LSB              32
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_PHY_PPDU_ID_MSB              47
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_PHY_PPDU_ID_MASK             0x0000ffff00000000


/* Description		LSIG_RESPONSE_LENGTH

			Field only valid in case of OFDMA trigger
			
			Indicates the value of the L-SIG Length field of the HE 
			trigger-based PPDU that is the response to the Trigger frame
			 
			
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_OFFSET  0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_LSB     48
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_MSB     59
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_LSIG_RESPONSE_LENGTH_MASK    0x0fff000000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_1A_OFFSET           0x0000000000000000
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_1A_LSB              60
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_1A_MSB              63
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_1A_MASK             0xf000000000000000


/* Description		FRAME_CONTROL

			frame control field of the received frame
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_OFFSET         0x0000000000000008
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_LSB            0
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_MSB            15
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_FRAME_CONTROL_MASK           0x000000000000ffff


/* Description		QOS_CONTROL

			frame control field of the received frame (if present)
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_OFFSET           0x0000000000000008
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_LSB              16
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_MSB              31
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_QOS_CONTROL_MASK             0x00000000ffff0000


/* Description		SW_PEER_ID

			A unique identifier for this STA. Extracted from the Address_Search_Entry
			
			
			Used by the SCH to find linkage between this trigger and
			 potentially pre-programmed responses.
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_SW_PEER_ID_OFFSET            0x0000000000000008
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_SW_PEER_ID_LSB               32
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_SW_PEER_ID_MSB               47
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_SW_PEER_ID_MASK              0x0000ffff00000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_3A_OFFSET           0x0000000000000008
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_3A_LSB              48
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_3A_MSB              63
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_RESERVED_3A_MASK             0xffff000000000000


/* Description		HE_CONTROL

			Field only valid when HE_control_info_valid is set
			
			This is the 'RAW HE_CONTROL field' that was present in the
			 frame.
			<legal all>
*/

#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_OFFSET            0x0000000000000010
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_LSB               0
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_MSB               31
#define RECEIVED_TRIGGER_INFO_RECEIVED_TRIGGER_DETAILS_HE_CONTROL_MASK              0x00000000ffffffff


/* Description		TLV64_PADDING

			Automatic DWORD padding inserted while converting TLV32 
			to TLV64 for 64 bit ARCH
			<legal 0>
*/

#define RECEIVED_TRIGGER_INFO_TLV64_PADDING_OFFSET                                  0x0000000000000010
#define RECEIVED_TRIGGER_INFO_TLV64_PADDING_LSB                                     32
#define RECEIVED_TRIGGER_INFO_TLV64_PADDING_MSB                                     63
#define RECEIVED_TRIGGER_INFO_TLV64_PADDING_MASK                                    0xffffffff00000000



#endif   // RECEIVED_TRIGGER_INFO

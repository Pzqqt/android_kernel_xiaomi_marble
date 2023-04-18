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

#ifndef _RX_PPDU_NO_ACK_REPORT_H_
#define _RX_PPDU_NO_ACK_REPORT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "no_ack_report.h"
#define NUM_OF_DWORDS_RX_PPDU_NO_ACK_REPORT 4

#define NUM_OF_QWORDS_RX_PPDU_NO_ACK_REPORT 2


struct rx_ppdu_no_ack_report {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   no_ack_report                                             no_ack_report_details;
#else
             struct   no_ack_report                                             no_ack_report_details;
#endif
};


/* Description		NO_ACK_REPORT_DETAILS

			Info indicating why frame did not require a response transmission
			 in SIFS time.
*/


/* Description		NO_ACK_TRANSMIT_REASON

			Field that indicates why the received frame is not needing
			 any transmit response in SIFS time. 
			
			The possible responses are listed in order.
			
			<enum 0     NO_ACK_FCS_errors > All received frames have
			 FCS errors.
			<enum 1     Unicast_no_ack_frame_received > All received
			 frames did not require a response.
			<enum 2     NO_ACK_Broadcast> Broadcast frame received
			<enum 3     NO_ACK_Multicast> Multicast frame received
			<enum 4     Not_directed> Frames received are not directed
			 to this device (based on addr1)
			<enum 5     AST_no_ack> The AST entry indicated that NO 
			ACK shall be send
			<enum 6     PHY_GID_mismatch> PHY dropped the incoming frame
			 dur to GID mismatch
			<enum 7     PHY_AID_mismatch> PHY dropped the incoming frame
			 dur to AID mismatch
			<enum 8     NO_ACK_PHY_error> PHY reported an error during
			 reception. For details, see the 'phy_error...' fields
			<enum 9     RTS_bw_not_available> The requested BW for the
			 CTS response frame is not available
			<enum 10     NDPA_Frame> An NDPA frame got received
			<enum 11     NDP_Frame> An NDP frame got received
			<enum 12     Trigger_NAV_blocked> a trigger frame was received, 
			but due to NAV setting, no response could be generated
			<enum 13     Trigger_no_AID> A trigger frame was received, 
			but this device's AID was not in the list
			<enum 14     NO_ACK_MAC_ABORT_REQ > No ACK is needed as 
			SW asked RXPCU to send a abort_request to the PHYRX
			<enum 15     no_response_other> placeholder in case non 
			of the above properly cover the reasons
			
			Also see the field SR_PPDU_during_OBSS.
			<legal 0-15>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_NO_ACK_TRANSMIT_REASON_OFFSET   0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_NO_ACK_TRANSMIT_REASON_LSB      0
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_NO_ACK_TRANSMIT_REASON_MSB      3
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_NO_ACK_TRANSMIT_REASON_MASK     0x000000000000000f


/* Description		MACRX_ABORT_REASON

			Field only valid when No_ack_transmit_reason is set to NO_ACK_MAC_ABORT_REQ
			
			
			Error field received from MACRX_ABORT_REQUEST.Macrx_abort_reason[2:0]
			
			 <Legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_MACRX_ABORT_REASON_OFFSET       0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_MACRX_ABORT_REASON_LSB          4
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_MACRX_ABORT_REASON_MSB          7
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_MACRX_ABORT_REASON_MASK         0x00000000000000f0


/* Description		PHYRX_ABORT_REASON

			Field only valid when No_ack_transmit_reason is set to NO_ACK_PHY_error
			
			
			Error field received from PHYRX_ABORT_REQUEST.Phyrx_abort_reason
			
			
			<Legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PHYRX_ABORT_REASON_OFFSET       0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PHYRX_ABORT_REASON_LSB          8
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PHYRX_ABORT_REASON_MSB          15
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PHYRX_ABORT_REASON_MASK         0x000000000000ff00


/* Description		FRAME_CONTROL

			frame control field of the received (first properly received) 
			frame
			
			<Legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FRAME_CONTROL_OFFSET            0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FRAME_CONTROL_LSB               16
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FRAME_CONTROL_MSB               31
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FRAME_CONTROL_MASK              0x00000000ffff0000


/* Description		RX_PPDU_DURATION

			The length of this PPDU reception in us 
			<Legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RX_PPDU_DURATION_OFFSET         0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RX_PPDU_DURATION_LSB            32
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RX_PPDU_DURATION_MSB            55
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RX_PPDU_DURATION_MASK           0x00ffffff00000000


/* Description		SR_PPDU_DURING_OBSS

			Field only valid with SRP Responder support
			
			Indicates that the received frame was sent using SRP as 
			indicated by the 'SR PPDU' bit in the 'CAS Control' in the
			 'HE A-Control' in one of the MPDUs received, and that the
			 response could not be generated due to OBSS traffic setting
			 the NAV
			<legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SR_PPDU_DURING_OBSS_OFFSET      0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SR_PPDU_DURING_OBSS_LSB         56
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SR_PPDU_DURING_OBSS_MSB         56
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SR_PPDU_DURING_OBSS_MASK        0x0100000000000000


/* Description		SELFGEN_RESPONSE_REASON_TO_SR_PPDU

			Field only valid with SRP Responder support
			
			This field indicates why the received SR PPDU needs a response
			 in SIFS time. The e-num used is the same as in the field
			 selfgen_response_reason in 'ACK_REPORT' structure although
			 some of these will be unused in case of an SR PPDU.
			
			<enum 0     CTS_frame>
			<enum 1     ACK_frame>
			<enum 2     BA_frame >
			<enum 3     Qboost_trigger> Qboost trigger received
			<enum 4     PSPOLL_trigger> PSPOLL trigger received
			<enum 5     UAPSD_trigger > Unscheduled APSD  trigger received
			
			<enum 6     CBF_frame> the CBF frame needs to be send as
			 a result of NDP or BRPOLL
			<enum 7     ax_su_trigger> 11ax trigger received for this
			 device
			<enum 8     ax_wildcard_trigger> 11ax wildcardtrigger has
			 been received
			<enum 9     ax_unassoc_wildcard_trigger> 11ax wildcard trigger
			 for unassociated STAs has been received
			<enum 12     eht_su_trigger> EHT R1 trigger received for
			 this device
			<enum 10     MU_UL_response_to_response>
			
			<enum 11     Ranging_NDP_LMR_frames> Ranging NDP + LMR need
			 to be sent in response to ranging NDPA + NDP
			
			<legal 0-12>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_OFFSET 0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_LSB 57
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_MSB 60
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_TO_SR_PPDU_MASK 0x1e00000000000000


/* Description		RESERVED_1

			<legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_1_OFFSET               0x0000000000000000
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_1_LSB                  61
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_1_MSB                  63
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_1_MASK                 0xe000000000000000


/* Description		PRE_BT_BROADCAST_STATUS_DETAILS

			Same contents as field "bt_broadcast_status_details" for
			 the first received COEX_STATUS_BROADCAST tlv during this
			 PPDU reception.
			After power up, this field is all initialized to 0
			
			Bits: [31:28]: always 0
			
			<legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PRE_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x0000000000000008
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PRE_BT_BROADCAST_STATUS_DETAILS_LSB 0
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PRE_BT_BROADCAST_STATUS_DETAILS_MSB 11
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_PRE_BT_BROADCAST_STATUS_DETAILS_MASK 0x0000000000000fff


/* Description		FIRST_BT_BROADCAST_STATUS_DETAILS

			Same contents as field "bt_broadcast_status_details" for
			 the first received COEX_STATUS_BROADCAST tlv during this
			 PPDU reception.
			
			If no COEX_STATUS_BROADCAST tlv is received during this 
			PPDU reception, this field will be set to 0
			<legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FIRST_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x0000000000000008
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FIRST_BT_BROADCAST_STATUS_DETAILS_LSB 12
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FIRST_BT_BROADCAST_STATUS_DETAILS_MSB 23
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_FIRST_BT_BROADCAST_STATUS_DETAILS_MASK 0x0000000000fff000


/* Description		RESERVED_2

			<legal 0>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_2_OFFSET               0x0000000000000008
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_2_LSB                  24
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_2_MSB                  31
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_2_MASK                 0x00000000ff000000


/* Description		SECOND_BT_BROADCAST_STATUS_DETAILS

			Same contents as field "bt_broadcast_status_details" for
			 the second received COEX_STATUS_BROADCAST tlv during this
			 PPDU reception.
			
			If no second COEX_STATUS_BROADCAST tlv is received during
			 this PPDU reception, this field will be set to 0
			<legal all>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SECOND_BT_BROADCAST_STATUS_DETAILS_OFFSET 0x0000000000000008
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SECOND_BT_BROADCAST_STATUS_DETAILS_LSB 32
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SECOND_BT_BROADCAST_STATUS_DETAILS_MSB 43
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_SECOND_BT_BROADCAST_STATUS_DETAILS_MASK 0x00000fff00000000


/* Description		RESERVED_3

			<legal 0>
*/

#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_3_OFFSET               0x0000000000000008
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_3_LSB                  44
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_3_MSB                  63
#define RX_PPDU_NO_ACK_REPORT_NO_ACK_REPORT_DETAILS_RESERVED_3_MASK                 0xfffff00000000000



#endif   // RX_PPDU_NO_ACK_REPORT

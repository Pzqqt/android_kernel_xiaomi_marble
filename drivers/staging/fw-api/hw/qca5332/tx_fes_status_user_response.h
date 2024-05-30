
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _TX_FES_STATUS_USER_RESPONSE_H_
#define _TX_FES_STATUS_USER_RESPONSE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_TX_FES_STATUS_USER_RESPONSE 2

#define NUM_OF_QWORDS_TX_FES_STATUS_USER_RESPONSE 1


struct tx_fes_status_user_response {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t fes_transmit_result                                     :  4, // [3:0]
                      reserved_0                                              : 28; // [31:4]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t reserved_after_struct16                                 : 16; // [31:16]
#else
             uint32_t reserved_0                                              : 28, // [31:4]
                      fes_transmit_result                                     :  4; // [3:0]
             uint32_t reserved_after_struct16                                 : 16; // [31:16]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
#endif
};


/* Description		FES_TRANSMIT_RESULT

			Transmit result:
			
			<enum 0 tx_ok> Successful transmission of entire Frame exchange
			 sequence
			<enum 1 prot_resp_rx_timeout> 
			No Protection response frame received so timeout is triggered. 
			
			<enum 2 ppdu_resp_rx_timeout> No PPDU response frame received
			 so timeout is triggered. 
			<enum 3 resp_frame_crc_err> Response frame was received 
			with an invalid FCS.
			<enum 4 SU_Response_type_mismatch> Response frame is received
			 without CRC error but it's not matched with expected SU_Response_type. 
			
			<enum 5 cbf_mimo_ctrl_mismatch> Set if CBF is received without
			 any error but the Nr, Nc, BW, type or token in VHT MIMO
			 control field is not matched with expected values which
			 are specified by TX_FES_SETUP.cbf_* fields. 
			 <enum 7 MU_Response_type_mismatch> Response frame is received
			 without CRC error but it's not matched with expected SU_Response_type. 
			
			<enum 8 MU_Response_mpdu_not_valid>  For this user, no MPDU
			 was received at all, or all received MPDUs had an FCS error.
			
			
			<enum 9 MU_UL_not_enough_user_response> An MU UL response
			 reception was expected. That response came but the threshold
			 for number of successful user receptions was not met.
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			<enum 10 Transmit_data_null_ratio_not_met> transmission 
			was successful and proper responses have been received. 
			But the required ratio between useful MPDU data and null
			 delimiters was not met as specified by field : Fes_continuation_ratio_threshold. 
			The FES (and potentially the SIFS burst) shall be terminated
			 by the SCHeduler
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			
			<enum 6 TB_ranging_resp_timeout> A TB ranging response was
			 expected for a sounding TF, but the response did not arrive
			 and timeout is triggered.
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			<enum 11 tb_ranging_resp_mismatch> A TB ranging response
			 was expected for a sounding TF, but the reception did not
			 match the expected response.
			NOTE: This e-num will only be used in the TX_FES_STATUS_END
			 TLV...
			
			<legal 0-11>
*/

#define TX_FES_STATUS_USER_RESPONSE_FES_TRANSMIT_RESULT_OFFSET                      0x0000000000000000
#define TX_FES_STATUS_USER_RESPONSE_FES_TRANSMIT_RESULT_LSB                         0
#define TX_FES_STATUS_USER_RESPONSE_FES_TRANSMIT_RESULT_MSB                         3
#define TX_FES_STATUS_USER_RESPONSE_FES_TRANSMIT_RESULT_MASK                        0x000000000000000f


/* Description		RESERVED_0

			Bits [15:4]: BAR_start_sequence_number:
			
			Starting sequence number to be overwritten by TXPCU for 
			BAR or MU-BAR Trigger, to be copied from 'MPDU_QUEUE_OVERVIEW' 
			by TXPCU
			
			Bit [16]: BAR_SSN_overwrite_enable:
			
			Enable for TXPCU overwrite of the starting sequence number
			 for BAR or MU-BAR Trigger, to be copied from 'TX_QUEUE_EXTENSION' 
			by TXPCU
			
			Not supported in Hamilton v1
			<legal 0-8191>
*/

#define TX_FES_STATUS_USER_RESPONSE_RESERVED_0_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_USER_RESPONSE_RESERVED_0_LSB                                  4
#define TX_FES_STATUS_USER_RESPONSE_RESERVED_0_MSB                                  31
#define TX_FES_STATUS_USER_RESPONSE_RESERVED_0_MASK                                 0x00000000fffffff0


/* Description		PHYTX_ABORT_REQUEST_INFO_DETAILS

			The reason why PHYTX is requesting an abort
*/


/* Description		PHYTX_ABORT_REASON

			Reason for early termination of TX packet by the PHY 
			
			<enum_type PHYTX_ABORT_ENUM>
*/

#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x0000000000000000
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB 32
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB 39
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK 0x000000ff00000000


/* Description		USER_NUMBER

			For some errors, the user for which this error was detected
			 can be indicated in this field.
			<legal 0-36>
*/

#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET 0x0000000000000000
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB 40
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB 45
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK 0x00003f0000000000


/* Description		RESERVED

			<legal 0>
*/

#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET 0x0000000000000000
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB   46
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB   47
#define TX_FES_STATUS_USER_RESPONSE_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK  0x0000c00000000000


/* Description		RESERVED_AFTER_STRUCT16

			<legal 0>
*/

#define TX_FES_STATUS_USER_RESPONSE_RESERVED_AFTER_STRUCT16_OFFSET                  0x0000000000000000
#define TX_FES_STATUS_USER_RESPONSE_RESERVED_AFTER_STRUCT16_LSB                     48
#define TX_FES_STATUS_USER_RESPONSE_RESERVED_AFTER_STRUCT16_MSB                     63
#define TX_FES_STATUS_USER_RESPONSE_RESERVED_AFTER_STRUCT16_MASK                    0xffff000000000000



#endif   // TX_FES_STATUS_USER_RESPONSE

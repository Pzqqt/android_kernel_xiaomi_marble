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

#ifndef _TX_FES_STATUS_PROT_H_
#define _TX_FES_STATUS_PROT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "phytx_abort_request_info.h"
#define NUM_OF_DWORDS_TX_FES_STATUS_PROT 14

#define NUM_OF_QWORDS_TX_FES_STATUS_PROT 7


struct tx_fes_status_prot {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t success                                                 :  1, // [0:0]
                      phytx_pkt_end_info_valid                                :  1, // [1:1]
                      phytx_abort_request_info_valid                          :  1, // [2:2]
                      reserved_0                                              : 20, // [22:3]
                      pkt_type                                                :  4, // [26:23]
                      dot11ax_su_extended                                     :  1, // [27:27]
                      rate_mcs                                                :  4; // [31:28]
             uint32_t frame_type                                              :  2, // [1:0]
                      frame_subtype                                           :  4, // [5:2]
                      rx_pwr_mgmt                                             :  1, // [6:6]
                      status                                                  :  1, // [7:7]
                      duration_field                                          : 16, // [23:8]
                      reserved_1a                                             :  2, // [25:24]
                      agc_cbw                                                 :  3, // [28:26]
                      service_cbw                                             :  3; // [31:29]
             uint32_t start_of_frame_timestamp_15_0                           : 16, // [15:0]
                      start_of_frame_timestamp_31_16                          : 16; // [31:16]
             uint32_t end_of_frame_timestamp_15_0                             : 16, // [15:0]
                      end_of_frame_timestamp_31_16                            : 16; // [31:16]
             uint32_t tx_group_delay                                          : 12, // [11:0]
                      timing_status                                           :  2, // [13:12]
                      dpdtrain_done                                           :  1, // [14:14]
                      reserved_4                                              :  1, // [15:15]
                      transmit_delay                                          : 16; // [31:16]
             uint32_t tpc_dbg_info_cmn_15_0                                   : 16, // [15:0]
                      tpc_dbg_info_cmn_31_16                                  : 16; // [31:16]
             uint32_t tpc_dbg_info_cmn_47_32                                  : 16, // [15:0]
                      tpc_dbg_info_chn1_15_0                                  : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_31_16                                 : 16, // [15:0]
                      tpc_dbg_info_chn1_47_32                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn1_63_48                                 : 16, // [15:0]
                      tpc_dbg_info_chn1_79_64                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_15_0                                  : 16, // [15:0]
                      tpc_dbg_info_chn2_31_16                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_47_32                                 : 16, // [15:0]
                      tpc_dbg_info_chn2_63_48                                 : 16; // [31:16]
             uint32_t tpc_dbg_info_chn2_79_64                                 : 16; // [15:0]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint32_t phytx_tx_end_sw_info_15_0                               : 16, // [15:0]
                      phytx_tx_end_sw_info_31_16                              : 16; // [31:16]
             uint32_t phytx_tx_end_sw_info_47_32                              : 16, // [15:0]
                      phytx_tx_end_sw_info_63_48                              : 16; // [31:16]
#else
             uint32_t rate_mcs                                                :  4, // [31:28]
                      dot11ax_su_extended                                     :  1, // [27:27]
                      pkt_type                                                :  4, // [26:23]
                      reserved_0                                              : 20, // [22:3]
                      phytx_abort_request_info_valid                          :  1, // [2:2]
                      phytx_pkt_end_info_valid                                :  1, // [1:1]
                      success                                                 :  1; // [0:0]
             uint32_t service_cbw                                             :  3, // [31:29]
                      agc_cbw                                                 :  3, // [28:26]
                      reserved_1a                                             :  2, // [25:24]
                      duration_field                                          : 16, // [23:8]
                      status                                                  :  1, // [7:7]
                      rx_pwr_mgmt                                             :  1, // [6:6]
                      frame_subtype                                           :  4, // [5:2]
                      frame_type                                              :  2; // [1:0]
             uint32_t start_of_frame_timestamp_31_16                          : 16, // [31:16]
                      start_of_frame_timestamp_15_0                           : 16; // [15:0]
             uint32_t end_of_frame_timestamp_31_16                            : 16, // [31:16]
                      end_of_frame_timestamp_15_0                             : 16; // [15:0]
             uint32_t transmit_delay                                          : 16, // [31:16]
                      reserved_4                                              :  1, // [15:15]
                      dpdtrain_done                                           :  1, // [14:14]
                      timing_status                                           :  2, // [13:12]
                      tx_group_delay                                          : 12; // [11:0]
             uint32_t tpc_dbg_info_cmn_31_16                                  : 16, // [31:16]
                      tpc_dbg_info_cmn_15_0                                   : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_15_0                                  : 16, // [31:16]
                      tpc_dbg_info_cmn_47_32                                  : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_47_32                                 : 16, // [31:16]
                      tpc_dbg_info_chn1_31_16                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn1_79_64                                 : 16, // [31:16]
                      tpc_dbg_info_chn1_63_48                                 : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_31_16                                 : 16, // [31:16]
                      tpc_dbg_info_chn2_15_0                                  : 16; // [15:0]
             uint32_t tpc_dbg_info_chn2_63_48                                 : 16, // [31:16]
                      tpc_dbg_info_chn2_47_32                                 : 16; // [15:0]
             struct   phytx_abort_request_info                                  phytx_abort_request_info_details;
             uint16_t tpc_dbg_info_chn2_79_64                                 : 16; // [15:0]
             uint32_t phytx_tx_end_sw_info_31_16                              : 16, // [31:16]
                      phytx_tx_end_sw_info_15_0                               : 16; // [15:0]
             uint32_t phytx_tx_end_sw_info_63_48                              : 16, // [31:16]
                      phytx_tx_end_sw_info_47_32                              : 16; // [15:0]
#endif
};


/* Description		SUCCESS

			When set, protection response has been received
*/

#define TX_FES_STATUS_PROT_SUCCESS_OFFSET                                           0x0000000000000000
#define TX_FES_STATUS_PROT_SUCCESS_LSB                                              0
#define TX_FES_STATUS_PROT_SUCCESS_MSB                                              0
#define TX_FES_STATUS_PROT_SUCCESS_MASK                                             0x0000000000000001


/* Description		PHYTX_PKT_END_INFO_VALID

			All the fields originating from PHYTX_PKT_END TLV contain
			 valid info
*/

#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_OFFSET                          0x0000000000000000
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_LSB                             1
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_MSB                             1
#define TX_FES_STATUS_PROT_PHYTX_PKT_END_INFO_VALID_MASK                            0x0000000000000002


/* Description		PHYTX_ABORT_REQUEST_INFO_VALID

			Field Phytx_abort_request_info_details contains valid info
			
*/

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_LSB                       2
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_MSB                       2
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_VALID_MASK                      0x0000000000000004


/* Description		RESERVED_0

			<legal 0>
*/

#define TX_FES_STATUS_PROT_RESERVED_0_OFFSET                                        0x0000000000000000
#define TX_FES_STATUS_PROT_RESERVED_0_LSB                                           3
#define TX_FES_STATUS_PROT_RESERVED_0_MSB                                           22
#define TX_FES_STATUS_PROT_RESERVED_0_MASK                                          0x00000000007ffff8


/* Description		PKT_TYPE

			Field only valid when success is set
			Source of the info here is the 'RECEIVED_RESPONSE_INFO' 
			TLV.
			
			Packet type:
			<enum 0 dot11a>802.11a PPDU type
			<enum 1 dot11b>802.11b PPDU type
			<enum 2 dot11n_mm>802.11n Mixed Mode PPDU type
			<enum 3 dot11ac>802.11ac PPDU type
			<enum 4 dot11ax>802.11ax PPDU type
			<enum 5 dot11ba>802.11ba (WUR) PPDU type
			<enum 6 dot11be>802.11be PPDU type
			<enum 7 dot11az>802.11az (ranging) PPDU type
			<enum 8 dot11n_gf>802.11n Green Field PPDU type (unsupported
			 & aborted)
*/

#define TX_FES_STATUS_PROT_PKT_TYPE_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_PROT_PKT_TYPE_LSB                                             23
#define TX_FES_STATUS_PROT_PKT_TYPE_MSB                                             26
#define TX_FES_STATUS_PROT_PKT_TYPE_MASK                                            0x0000000007800000


/* Description		DOT11AX_SU_EXTENDED

			Field only valid when success is set and pkt_type == 11ax
			 OR pkt_type == 11be
			Source of the info here is the 'RECEIVED_RESPONSE_INFO' 
			TLV.
			
			When set, the 11ax or 11be reception was an extended range
			 SU 
*/

#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_OFFSET                               0x0000000000000000
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_LSB                                  27
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_MSB                                  27
#define TX_FES_STATUS_PROT_DOT11AX_SU_EXTENDED_MASK                                 0x0000000008000000


/* Description		RATE_MCS

			Field only valid when success is set
			Source of the info here is the 'RECEIVED_RESPONSE_INFO' 
			TLV.
			
			For details, refer to  MCS_TYPE description
			Note: This is "rate" in case of 11a/11b
			
			<legal all>
*/

#define TX_FES_STATUS_PROT_RATE_MCS_OFFSET                                          0x0000000000000000
#define TX_FES_STATUS_PROT_RATE_MCS_LSB                                             28
#define TX_FES_STATUS_PROT_RATE_MCS_MSB                                             31
#define TX_FES_STATUS_PROT_RATE_MCS_MASK                                            0x00000000f0000000


/* Description		FRAME_TYPE

			Field only valid when 'success' is set.
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			802.11 frame type field
			This field applies for 11ah as well.
*/

#define TX_FES_STATUS_PROT_FRAME_TYPE_OFFSET                                        0x0000000000000000
#define TX_FES_STATUS_PROT_FRAME_TYPE_LSB                                           32
#define TX_FES_STATUS_PROT_FRAME_TYPE_MSB                                           33
#define TX_FES_STATUS_PROT_FRAME_TYPE_MASK                                          0x0000000300000000


/* Description		FRAME_SUBTYPE

			Field only valid when 'success' is set.
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			802.11 frame subtype field
			This field applies for 11ah as well.
*/

#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_OFFSET                                     0x0000000000000000
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_LSB                                        34
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_MSB                                        37
#define TX_FES_STATUS_PROT_FRAME_SUBTYPE_MASK                                       0x0000003c00000000


/* Description		RX_PWR_MGMT

			Field only valid when 'success' is set.
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			Power Management bit extracted from the header of the received
			 frame.
*/

#define TX_FES_STATUS_PROT_RX_PWR_MGMT_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_LSB                                          38
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_MSB                                          38
#define TX_FES_STATUS_PROT_RX_PWR_MGMT_MASK                                         0x0000004000000000


/* Description		STATUS

			Field only valid when 'success' is set.
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			If set indicates that receive packet passed FCS check.
*/

#define TX_FES_STATUS_PROT_STATUS_OFFSET                                            0x0000000000000000
#define TX_FES_STATUS_PROT_STATUS_LSB                                               39
#define TX_FES_STATUS_PROT_STATUS_MSB                                               39
#define TX_FES_STATUS_PROT_STATUS_MASK                                              0x0000008000000000


/* Description		DURATION_FIELD

			Field only valid when 'success' is set.
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			The contents of the duration field of the received frame.
			
			<legal all>
*/

#define TX_FES_STATUS_PROT_DURATION_FIELD_OFFSET                                    0x0000000000000000
#define TX_FES_STATUS_PROT_DURATION_FIELD_LSB                                       40
#define TX_FES_STATUS_PROT_DURATION_FIELD_MSB                                       55
#define TX_FES_STATUS_PROT_DURATION_FIELD_MASK                                      0x00ffff0000000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define TX_FES_STATUS_PROT_RESERVED_1A_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_PROT_RESERVED_1A_LSB                                          56
#define TX_FES_STATUS_PROT_RESERVED_1A_MSB                                          57
#define TX_FES_STATUS_PROT_RESERVED_1A_MASK                                         0x0300000000000000


/* Description		AGC_CBW

			Field only valid when 'success' is set.
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			BW as detected by the AGC 
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_STATUS_PROT_AGC_CBW_OFFSET                                           0x0000000000000000
#define TX_FES_STATUS_PROT_AGC_CBW_LSB                                              58
#define TX_FES_STATUS_PROT_AGC_CBW_MSB                                              60
#define TX_FES_STATUS_PROT_AGC_CBW_MASK                                             0x1c00000000000000


/* Description		SERVICE_CBW

			Field only valid when 'success' is set.
			
			Source of the info here is the RECEIVED_RESPONSE_INFO TLV
			
			
			This field reflects the BW extracted from the Serivce Field
			 for 11ac mode of operation .
			
			This field is used in the context of Dynamic/Static BW evaluation
			 purposes in TxPCU
			CBW field extracted from Service field
			
			<enum 0 20_mhz>20 Mhz BW
			<enum 1 40_mhz>40 Mhz BW
			<enum 2 80_mhz>80 Mhz BW
			<enum 3 160_mhz>160 Mhz BW
			<enum 4 320_mhz>320 Mhz BW
			<enum 5 240_mhz>240 Mhz BW
*/

#define TX_FES_STATUS_PROT_SERVICE_CBW_OFFSET                                       0x0000000000000000
#define TX_FES_STATUS_PROT_SERVICE_CBW_LSB                                          61
#define TX_FES_STATUS_PROT_SERVICE_CBW_MSB                                          63
#define TX_FES_STATUS_PROT_SERVICE_CBW_MASK                                         0xe000000000000000


/* Description		START_OF_FRAME_TIMESTAMP_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 15:0 of a 64 bit time stamp 
			Start of frame in the medium @960 MHz
			<legal all> 
*/

#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_OFFSET                     0x0000000000000008
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_LSB                        0
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_MSB                        15
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_15_0_MASK                       0x000000000000ffff


/* Description		START_OF_FRAME_TIMESTAMP_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 31:16 of a 64 bit time stamp 
			Start of frame in the medium @960 MHz
			<legal all>
*/

#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_OFFSET                    0x0000000000000008
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_LSB                       16
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_MSB                       31
#define TX_FES_STATUS_PROT_START_OF_FRAME_TIMESTAMP_31_16_MASK                      0x00000000ffff0000


/* Description		END_OF_FRAME_TIMESTAMP_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 15:0 of a 64 bit time stamp 
			End of frame in the medium @960 MHz
			<legal all> 
*/

#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_OFFSET                       0x0000000000000008
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_LSB                          32
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_MSB                          47
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_15_0_MASK                         0x0000ffff00000000


/* Description		END_OF_FRAME_TIMESTAMP_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			bits 31:16 of a 64 bit time stamp 
			End of frame in the medium @960 MHz
			<legal all>
*/

#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_OFFSET                      0x0000000000000008
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_LSB                         48
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_MSB                         63
#define TX_FES_STATUS_PROT_END_OF_FRAME_TIMESTAMP_31_16_MASK                        0xffff000000000000


/* Description		TX_GROUP_DELAY

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Group delay on TxTD+PHYRF path for this PPDU (packet BW 
			dependent), useful for RTT
			
			Unit is 960MHz cycles.
			<legal all>
*/

#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_OFFSET                                    0x0000000000000010
#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_LSB                                       0
#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_MSB                                       11
#define TX_FES_STATUS_PROT_TX_GROUP_DELAY_MASK                                      0x0000000000000fff


/* Description		TIMING_STATUS

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			<enum 0 No_tx_timing_request> The MAC did not request for
			 the transmission to start at a particular time
			<enum 1 successful_tx_timing > MAC did request for transmission
			 to start at a particular time and PHY was able to do so.
			
			<enum 2 tx_timing_not_honoured> PHY was not able to honour
			 the requested transmit time by the MAC. The transmission
			 started later, and field transmit_delay indicates how much
			 later.
			<legal 0-2>
*/

#define TX_FES_STATUS_PROT_TIMING_STATUS_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_PROT_TIMING_STATUS_LSB                                        12
#define TX_FES_STATUS_PROT_TIMING_STATUS_MSB                                        13
#define TX_FES_STATUS_PROT_TIMING_STATUS_MASK                                       0x0000000000003000


/* Description		DPDTRAIN_DONE

			Field only valid when PHYTX_PKT_END_info_valid is set
			
			For DPD Training packets, this bit is set to indicate that
			 DPD Training was successfully run to completion.  Also 
			reused by Implicit BF Calibration Packets. This bit is intended
			 for debug purposes.
			<legal all>
*/

#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_OFFSET                                     0x0000000000000010
#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_LSB                                        14
#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_MSB                                        14
#define TX_FES_STATUS_PROT_DPDTRAIN_DONE_MASK                                       0x0000000000004000


/* Description		RESERVED_4

			PHYTX_PKT_END info
			
			<legal 0>
*/

#define TX_FES_STATUS_PROT_RESERVED_4_OFFSET                                        0x0000000000000010
#define TX_FES_STATUS_PROT_RESERVED_4_LSB                                           15
#define TX_FES_STATUS_PROT_RESERVED_4_MSB                                           15
#define TX_FES_STATUS_PROT_RESERVED_4_MASK                                          0x0000000000008000


/* Description		TRANSMIT_DELAY

			PHYTX_PKT_END info
			
			The number of 480 MHz clock cycles that the transmission
			 started after the actual requested transmit start time.
			
			Value saturates at 0xFFFF
			<legal all>
*/

#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_OFFSET                                    0x0000000000000010
#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_LSB                                       16
#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_MSB                                       31
#define TX_FES_STATUS_PROT_TRANSMIT_DELAY_MASK                                      0x00000000ffff0000


/* Description		TPC_DBG_INFO_CMN_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_OFFSET                             0x0000000000000010
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_LSB                                32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_MSB                                47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_15_0_MASK                               0x0000ffff00000000


/* Description		TPC_DBG_INFO_CMN_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_OFFSET                            0x0000000000000010
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_LSB                               48
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_MSB                               63
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_31_16_MASK                              0xffff000000000000


/* Description		TPC_DBG_INFO_CMN_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some TPC debug info that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_LSB                               0
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_MSB                               15
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CMN_47_32_MASK                              0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN1_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_OFFSET                            0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_LSB                               16
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_MSB                               31
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_15_0_MASK                              0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN1_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_OFFSET                           0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_LSB                              32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_MSB                              47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_31_16_MASK                             0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN1_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_OFFSET                           0x0000000000000018
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_LSB                              48
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_MSB                              63
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_47_32_MASK                             0xffff000000000000


/* Description		TPC_DBG_INFO_CHN1_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_OFFSET                           0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_LSB                              0
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_MSB                              15
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_63_48_MASK                             0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN1_79_64

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the first selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_OFFSET                           0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_LSB                              16
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_MSB                              31
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN1_79_64_MASK                             0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN2_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_OFFSET                            0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_LSB                               32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_MSB                               47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_15_0_MASK                              0x0000ffff00000000


/* Description		TPC_DBG_INFO_CHN2_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_OFFSET                           0x0000000000000020
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_LSB                              48
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_MSB                              63
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_31_16_MASK                             0xffff000000000000


/* Description		TPC_DBG_INFO_CHN2_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_OFFSET                           0x0000000000000028
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_LSB                              0
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_MSB                              15
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_47_32_MASK                             0x000000000000ffff


/* Description		TPC_DBG_INFO_CHN2_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_OFFSET                           0x0000000000000028
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_LSB                              16
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_MSB                              31
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_63_48_MASK                             0x00000000ffff0000


/* Description		TPC_DBG_INFO_CHN2_79_64

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some per-chain TPC debug info for the second selected chain
			 that PHY can pass back to MAC FW
			<legal all>
*/

#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_OFFSET                           0x0000000000000028
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_LSB                              32
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_MSB                              47
#define TX_FES_STATUS_PROT_TPC_DBG_INFO_CHN2_79_64_MASK                             0x0000ffff00000000


/* Description		PHYTX_ABORT_REQUEST_INFO_DETAILS

			Field only valid when PHYTX_ABORT_REQUEST_info_valid is 
			set
			
			The reason why PHYTX is requested an abort
*/


/* Description		PHYTX_ABORT_REASON

			Reason for early termination of TX packet by the PHY 
			
			<enum_type PHYTX_ABORT_ENUM>
*/

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_OFFSET 0x0000000000000028
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_LSB  48
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MSB  55
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_PHYTX_ABORT_REASON_MASK 0x00ff000000000000


/* Description		USER_NUMBER

			For some errors, the user for which this error was detected
			 can be indicated in this field.
			<legal 0-36>
*/

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_OFFSET      0x0000000000000028
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_LSB         56
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MSB         61
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_USER_NUMBER_MASK        0x3f00000000000000


/* Description		RESERVED

			<legal 0>
*/

#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_OFFSET         0x0000000000000028
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_LSB            62
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MSB            63
#define TX_FES_STATUS_PROT_PHYTX_ABORT_REQUEST_INFO_DETAILS_RESERVED_MASK           0xc000000000000000


/* Description		PHYTX_TX_END_SW_INFO_15_0

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_OFFSET                         0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_LSB                            0
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_MSB                            15
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_15_0_MASK                           0x000000000000ffff


/* Description		PHYTX_TX_END_SW_INFO_31_16

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_OFFSET                        0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_LSB                           16
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_MSB                           31
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_31_16_MASK                          0x00000000ffff0000


/* Description		PHYTX_TX_END_SW_INFO_47_32

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_OFFSET                        0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_LSB                           32
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_MSB                           47
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_47_32_MASK                          0x0000ffff00000000


/* Description		PHYTX_TX_END_SW_INFO_63_48

			PHYTX_PKT_END info
			
			Field only valid when PHYTX_PKT_END_info_valid is set
			
			Some PHY status data that PHY microcode can pass back to
			 MAC FW, for any future requests, e.g. any DMA download 
			time
			<legal all>
*/

#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_OFFSET                        0x0000000000000030
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_LSB                           48
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_MSB                           63
#define TX_FES_STATUS_PROT_PHYTX_TX_END_SW_INFO_63_48_MASK                          0xffff000000000000



#endif   // TX_FES_STATUS_PROT

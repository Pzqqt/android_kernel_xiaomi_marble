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

#ifndef _TX_FES_STATUS_USER_PPDU_H_
#define _TX_FES_STATUS_USER_PPDU_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_FES_STATUS_USER_PPDU 6

#define NUM_OF_QWORDS_TX_FES_STATUS_USER_PPDU 3


struct tx_fes_status_user_ppdu {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t underflow_mpdu_count                                    :  9, // [8:0]
                      data_underflow_warning                                  :  2, // [10:9]
                      bw_drop_underflow_warning                               :  1, // [11:11]
                      qc_eosp_setting                                         :  1, // [12:12]
                      fc_more_data_setting                                    :  1, // [13:13]
                      fc_pwr_mgt_setting                                      :  1, // [14:14]
                      mpdu_tx_count                                           :  9, // [23:15]
                      user_blocked                                            :  1, // [24:24]
                      pre_trig_response_delim_count                           :  7; // [31:25]
             uint32_t underflow_byte_count                                    : 16, // [15:0]
                      coex_abort_mpdu_count_valid                             :  1, // [16:16]
                      coex_abort_mpdu_count                                   :  9, // [25:17]
                      transmitted_tid                                         :  4, // [29:26]
                      txdma_dropped_mpdu_warning                              :  1, // [30:30]
                      reserved_1                                              :  1; // [31:31]
             uint32_t duration                                                : 16, // [15:0]
                      num_eof_delim_added                                     : 16; // [31:16]
             uint32_t psdu_octet                                              : 24, // [23:0]
                      qos_buf_state                                           :  8; // [31:24]
             uint32_t num_null_delim_added                                    : 22, // [21:0]
                      reserved_4a                                             :  2, // [23:22]
                      cv_corr_user_valid_in_phy                               :  1, // [24:24]
                      nss                                                     :  3, // [27:25]
                      mcs                                                     :  4; // [31:28]
             uint32_t ht_control                                              : 32; // [31:0]
#else
             uint32_t pre_trig_response_delim_count                           :  7, // [31:25]
                      user_blocked                                            :  1, // [24:24]
                      mpdu_tx_count                                           :  9, // [23:15]
                      fc_pwr_mgt_setting                                      :  1, // [14:14]
                      fc_more_data_setting                                    :  1, // [13:13]
                      qc_eosp_setting                                         :  1, // [12:12]
                      bw_drop_underflow_warning                               :  1, // [11:11]
                      data_underflow_warning                                  :  2, // [10:9]
                      underflow_mpdu_count                                    :  9; // [8:0]
             uint32_t reserved_1                                              :  1, // [31:31]
                      txdma_dropped_mpdu_warning                              :  1, // [30:30]
                      transmitted_tid                                         :  4, // [29:26]
                      coex_abort_mpdu_count                                   :  9, // [25:17]
                      coex_abort_mpdu_count_valid                             :  1, // [16:16]
                      underflow_byte_count                                    : 16; // [15:0]
             uint32_t num_eof_delim_added                                     : 16, // [31:16]
                      duration                                                : 16; // [15:0]
             uint32_t qos_buf_state                                           :  8, // [31:24]
                      psdu_octet                                              : 24; // [23:0]
             uint32_t mcs                                                     :  4, // [31:28]
                      nss                                                     :  3, // [27:25]
                      cv_corr_user_valid_in_phy                               :  1, // [24:24]
                      reserved_4a                                             :  2, // [23:22]
                      num_null_delim_added                                    : 22; // [21:0]
             uint32_t ht_control                                              : 32; // [31:0]
#endif
};


/* Description		UNDERFLOW_MPDU_COUNT

			The MPDU count correctly received from TX DMA when the first
			 underrun condition was detected
			<legal 0-256>
*/

#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_LSB                            0
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_MSB                            8
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_MPDU_COUNT_MASK                           0x00000000000001ff


/* Description		DATA_UNDERFLOW_WARNING

			Mac data underflow warning for this user
			
			<enum 0 no_data_underrun> No data underflow
			<enum 1 data_underrun_between_mpdu> PCU experienced data
			 underflow in between MPDUs
			<enum 2 data_underrun_within_mpdu> PCU experienced data 
			underflow within an MPDU
			<legal 0-2>
*/

#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_OFFSET                       0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_LSB                          9
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_MSB                          10
#define TX_FES_STATUS_USER_PPDU_DATA_UNDERFLOW_WARNING_MASK                         0x0000000000000600


/* Description		BW_DROP_UNDERFLOW_WARNING

			When set, data underflow happened while TXPCU was busy with
			 dropping a frame that is only allowed to go out at certain
			 BW, which is not the BW of the current transmission
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_OFFSET                    0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_LSB                       11
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_MSB                       11
#define TX_FES_STATUS_USER_PPDU_BW_DROP_UNDERFLOW_WARNING_MASK                      0x0000000000000800


/* Description		QC_EOSP_SETTING

			This field indicates if TX PCU set the eosp bit in the QoS
			 control field for this user (indicated in field User_Id.)
			
			0: No action
			1: eosp bit is set in all the transmitted frames. This is
			 done upon request of the PDG.
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_OFFSET                              0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_LSB                                 12
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_MSB                                 12
#define TX_FES_STATUS_USER_PPDU_QC_EOSP_SETTING_MASK                                0x0000000000001000


/* Description		FC_MORE_DATA_SETTING

			This field indicates what the setting was of the More data
			 bit in the Frame control field for this user (indicated
			 in field User_Id.) 
			
			Note that TXPCU, depending on programming, might overwrite
			 this bit in the Frame control field or just passes on what
			 SW and/or OLE has already programmed. Either way, TXPCU
			 just blindly copies the final setting of this "more Data" 
			bit in the frame control field into this field in the TLV.
			
			
			0: more_data bit NOT set
			1: more_data bit is set
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_LSB                            13
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_MSB                            13
#define TX_FES_STATUS_USER_PPDU_FC_MORE_DATA_SETTING_MASK                           0x0000000000002000


/* Description		FC_PWR_MGT_SETTING

			This field indicates what the setting was of the pwr bit
			 in the Frame control field for this user (indicated in 
			field User_Id.)
			
			Note that TXPCU never manipulates the pwr bit in the FC 
			field. Generating the correct setting is all managed by 
			TX OLE.
			TXPCU just reports here what the pwr setting was of the (last) 
			transmitted MPDU.
			
			0: pwr_mgt bit NOT set
			1: pwr_mgt bit is set
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_OFFSET                           0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_LSB                              14
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_MSB                              14
#define TX_FES_STATUS_USER_PPDU_FC_PWR_MGT_SETTING_MASK                             0x0000000000004000


/* Description		MPDU_TX_COUNT

			Number of MPDU frames transmitted
			
			Note: MPDUs that had an underrun during transmission will
			 still be listed here. The assumption is that underrun is
			 a very rare occasion, and any miscounting can therefor 
			be accepted. If underrun happens too often, SW should change
			 the density settings.
			<legal 0-256>
*/

#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_OFFSET                                0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_LSB                                   15
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_MSB                                   23
#define TX_FES_STATUS_USER_PPDU_MPDU_TX_COUNT_MASK                                  0x0000000000ff8000


/* Description		USER_BLOCKED

			When set, TXPCU received the TX_PEER_ENTRY TLV with bit 'Block_this_user' 
			set. As a result TXDMA did not push in any MPDUs for this
			 user and non were expected to be transmitted. TXPCU will
			 therefor NOT report any underrun conditions for this user
			
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_OFFSET                                 0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_LSB                                    24
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_MSB                                    24
#define TX_FES_STATUS_USER_PPDU_USER_BLOCKED_MASK                                   0x0000000001000000


/* Description		PRE_TRIG_RESPONSE_DELIM_COUNT

			This field is only valid when this TX_FES_STATUS_USER_PPDU
			 is generated in the context of sending a response to a 
			received trigger frame....(=> TX_FES_STATUS start indicated
			 for field Transmit_start_reason ==  Trigger_based_transmit_start)
			
			
			The number of NULL delimiters the TXPCU passed on to the
			 PHY before any real MPDU (response) data is given to the
			 PHY that originated from the SCHeduler command.
			
			NOTE that this should not be flagged as an underrun condition.
			
			
			In units of 32 delimiters.
			
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_OFFSET                0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_LSB                   25
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_MSB                   31
#define TX_FES_STATUS_USER_PPDU_PRE_TRIG_RESPONSE_DELIM_COUNT_MASK                  0x00000000fe000000


/* Description		UNDERFLOW_BYTE_COUNT

			The number of bytes correctly received for this MPDU when
			 the first underrun condition was detected
			
			In case of self-gen + SCH related data, self-gen will not
			 be part of the underflow byte count. For example, in case
			 of BA/CBF, if underrun is hit immediately after BA/CBF 
			is sent, the underflow byte count will be 0.the BA/CBF bytes
			 will not be part of the underflow byte count.
*/

#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_OFFSET                         0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_LSB                            32
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_MSB                            47
#define TX_FES_STATUS_USER_PPDU_UNDERFLOW_BYTE_COUNT_MASK                           0x0000ffff00000000


/* Description		COEX_ABORT_MPDU_COUNT_VALID

			When set to 1, the (A-MPDU) transmission was silently aborted
			 in the middle of transmission. The PHY faked the remaining
			 transmission on the medium, so that TX PCU is still waiting
			 for the BA frame to be received.  
*/

#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_OFFSET                  0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_LSB                     48
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_MSB                     48
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_VALID_MASK                    0x0001000000000000


/* Description		COEX_ABORT_MPDU_COUNT

			Field only valid when 'Coex_abort_mpdu_count_valid' is set.
			
			The number of MPDU frames that were properly sent bdoefore
			 the coex transmit abort request was received
			<legal 0-256>
*/

#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_OFFSET                        0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_LSB                           49
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_MSB                           57
#define TX_FES_STATUS_USER_PPDU_COEX_ABORT_MPDU_COUNT_MASK                          0x03fe000000000000


/* Description		TRANSMITTED_TID

			TID field blindy copied over from the TX_QUEUE_EXTENSION
			 TLV, field qos_ctrl[3:0]
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_OFFSET                              0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_LSB                                 58
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_MSB                                 61
#define TX_FES_STATUS_USER_PPDU_TRANSMITTED_TID_MASK                                0x3c00000000000000


/* Description		TXDMA_DROPPED_MPDU_WARNING

			Indication to FW a warning that Tx DMA has dropped MPDUs
			 due to SFM FIFO full condition
			TXPCU fills this from OR of all TXDMA_dropped_mpdu_warning
			 in 'TX_MPDU_STARTs' for this PPDU.
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_OFFSET                   0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_LSB                      62
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_MSB                      62
#define TX_FES_STATUS_USER_PPDU_TXDMA_DROPPED_MPDU_WARNING_MASK                     0x4000000000000000


/* Description		RESERVED_1

			Reserved and not used by HW
			<legal 0>
*/

#define TX_FES_STATUS_USER_PPDU_RESERVED_1_OFFSET                                   0x0000000000000000
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_LSB                                      63
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_MSB                                      63
#define TX_FES_STATUS_USER_PPDU_RESERVED_1_MASK                                     0x8000000000000000


/* Description		DURATION

			The value of the duration field that TXPCU inserted in transmitted
			 frames, for Tx Monitor to report
			
			For frames of encap type Ethernet or 802_3 TXPCU will always
			 insert this value
			
			
			For frames of encap type: RAW and Native WiFi, TXPCU will
			 check the 'Duration_field_mask' setting in TX_RAW_OR_NATIVE_FRAME_SETUP
			 TLV to find out if overwrite is enabled. (This is per user)
			
			
			In case of multi TID transmission of Multi STA transmission, 
			TXPCU will look at the 'TX_RAW_OR_NATIVE_FRAME_SETUP' of
			 the 'first user'
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_DURATION_OFFSET                                     0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_DURATION_LSB                                        0
#define TX_FES_STATUS_USER_PPDU_DURATION_MSB                                        15
#define TX_FES_STATUS_USER_PPDU_DURATION_MASK                                       0x000000000000ffff


/* Description		NUM_EOF_DELIM_ADDED

			The total number of EOF pad delimiters added by TXPCU to
			 the current PPDU for the MD/multi-TID group this user belongs
			 to
			
			Set to 0xFFFF if the number exceeds the field width
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_OFFSET                          0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_LSB                             16
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_MSB                             31
#define TX_FES_STATUS_USER_PPDU_NUM_EOF_DELIM_ADDED_MASK                            0x00000000ffff0000


/* Description		PSDU_OCTET

			Field only valid in case in 'TX_FES_STATUS_START' the field
			 Transmit_start_reason != Trigger_based_transmit_start
			
			PSDU length in octets which includes all useful data in 
			a packet which includes EOF padding and final padding (including
			 the last 0 - 3 bytes).
			
			This is copied by TXPCU from 'PCU_PPDU_SETUP_USER.'
			
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_OFFSET                                   0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_LSB                                      32
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_MSB                                      55
#define TX_FES_STATUS_USER_PPDU_PSDU_OCTET_MASK                                     0x00ffffff00000000


/* Description		QOS_BUF_STATE

			The value of the buffer state field in the QoS control that
			 TXPCU inserted in transmitted frames, for Tx Monitor to
			 report
			
			TXPCU checks the '*Buf_state*' settings in 'TX_QUEUE_EXTENSION' 
			TLV to determine the value to insert.
			
			If TXPCU did not overwrite the buffer state field, this 
			shall be set to 0x0.
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_OFFSET                                0x0000000000000008
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_LSB                                   56
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_MSB                                   63
#define TX_FES_STATUS_USER_PPDU_QOS_BUF_STATE_MASK                                  0xff00000000000000


/* Description		NUM_NULL_DELIM_ADDED

			The total number of non-EOF pad/null delimiters added by
			 TXPCU to the current PPDU for this user
			
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_OFFSET                         0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_LSB                            0
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_MSB                            21
#define TX_FES_STATUS_USER_PPDU_NUM_NULL_DELIM_ADDED_MASK                           0x00000000003fffff


/* Description		RESERVED_4A

			<legal 0>
*/

#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_OFFSET                                  0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_LSB                                     22
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_MSB                                     23
#define TX_FES_STATUS_USER_PPDU_RESERVED_4A_MASK                                    0x0000000000c00000


/* Description		CV_CORR_USER_VALID_IN_PHY

			PDG sets this as 1 for up to 8 users enabled in 'PHYTX_CV_CORR_STATUS' 
			after CV correlation, to be copied from 'PCU_PPDU_SETUP_USER.'
			
			
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_OFFSET                    0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_LSB                       24
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_MSB                       24
#define TX_FES_STATUS_USER_PPDU_CV_CORR_USER_VALID_IN_PHY_MASK                      0x0000000001000000


/* Description		NSS

			Number of Spatial Streams occupied by the User, to be copied
			 from 'PCU_PPDU_SETUP_USER' by TXPCU
			
			<enum 0 1_spatial_stream>Single spatial stream
			<enum 1 2_spatial_streams>2 spatial streams
			<enum 2 3_spatial_streams>3 spatial streams
			<enum 3 4_spatial_streams>4 spatial streams
			<enum 4 5_spatial_streams>5 spatial streams
			<enum 5 6_spatial_streams>6 spatial streams
			<enum 6 7_spatial_streams>7 spatial streams
			<enum 7 8_spatial_streams>8 spatial streams
*/

#define TX_FES_STATUS_USER_PPDU_NSS_OFFSET                                          0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_NSS_LSB                                             25
#define TX_FES_STATUS_USER_PPDU_NSS_MSB                                             27
#define TX_FES_STATUS_USER_PPDU_NSS_MASK                                            0x000000000e000000


/* Description		MCS

			Modulation Coding Scheme for the User, to be copied from
			 'PCU_PPDU_SETUP_USER' by TXPCU
			
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_MCS_OFFSET                                          0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_MCS_LSB                                             28
#define TX_FES_STATUS_USER_PPDU_MCS_MSB                                             31
#define TX_FES_STATUS_USER_PPDU_MCS_MASK                                            0x00000000f0000000


/* Description		HT_CONTROL

			The value of the HT control field that TXPCU inserted in
			 transmitted frames, for Tx Monitor to report
			
			TXPCU checks the various HT-control-related settings in 'TX_QUEUE_EXTENSION' 
			TLV to determine the value to insert.
			
			If TXPCU did not overwrite the HT control field, this shall
			 be set to 0x0.
			<legal all>
*/

#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_OFFSET                                   0x0000000000000010
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_LSB                                      32
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_MSB                                      63
#define TX_FES_STATUS_USER_PPDU_HT_CONTROL_MASK                                     0xffffffff00000000



#endif   // TX_FES_STATUS_USER_PPDU

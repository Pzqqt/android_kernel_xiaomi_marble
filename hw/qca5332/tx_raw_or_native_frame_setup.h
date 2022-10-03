
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

 
 
 
 
 
 
 


#ifndef _TX_RAW_OR_NATIVE_FRAME_SETUP_H_
#define _TX_RAW_OR_NATIVE_FRAME_SETUP_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_RAW_OR_NATIVE_FRAME_SETUP 2

#define NUM_OF_QWORDS_TX_RAW_OR_NATIVE_FRAME_SETUP 1


struct tx_raw_or_native_frame_setup {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t fc_to_ds_mask                                           :  1, // [0:0]
                      fc_from_ds_mask                                         :  1, // [1:1]
                      fc_more_frag_mask                                       :  1, // [2:2]
                      fc_retry_mask                                           :  1, // [3:3]
                      fc_pwr_mgt_mask                                         :  1, // [4:4]
                      fc_more_data_mask                                       :  1, // [5:5]
                      fc_prot_frame_mask                                      :  1, // [6:6]
                      fc_order_mask                                           :  1, // [7:7]
                      duration_field_mask                                     :  1, // [8:8]
                      sequence_control_mask                                   :  1, // [9:9]
                      qc_tid_mask                                             :  1, // [10:10]
                      qc_eosp_mask                                            :  1, // [11:11]
                      qc_ack_policy_mask                                      :  1, // [12:12]
                      qc_amsdu_mask                                           :  1, // [13:13]
                      reserved_0a                                             :  1, // [14:14]
                      qc_15to8_mask                                           :  1, // [15:15]
                      iv_mask                                                 :  1, // [16:16]
                      fc_to_ds_setting                                        :  1, // [17:17]
                      fc_from_ds_setting                                      :  1, // [18:18]
                      fc_more_frag_setting                                    :  1, // [19:19]
                      fc_retry_setting                                        :  2, // [21:20]
                      fc_pwr_mgt_setting                                      :  1, // [22:22]
                      fc_more_data_setting                                    :  2, // [24:23]
                      fc_prot_frame_setting                                   :  2, // [26:25]
                      fc_order_setting                                        :  1, // [27:27]
                      qc_tid_setting                                          :  4; // [31:28]
             uint32_t qc_eosp_setting                                         :  2, // [1:0]
                      qc_ack_policy_setting                                   :  2, // [3:2]
                      qc_amsdu_setting                                        :  1, // [4:4]
                      qc_15to8_setting                                        :  8, // [12:5]
                      mlo_addr_override                                       :  1, // [13:13]
                      mlo_ignore_addr3_override                               :  1, // [14:14]
                      sequence_control_source                                 :  1, // [15:15]
                      fragment_number                                         :  4, // [19:16]
                      sequence_number                                         : 12; // [31:20]
#else
             uint32_t qc_tid_setting                                          :  4, // [31:28]
                      fc_order_setting                                        :  1, // [27:27]
                      fc_prot_frame_setting                                   :  2, // [26:25]
                      fc_more_data_setting                                    :  2, // [24:23]
                      fc_pwr_mgt_setting                                      :  1, // [22:22]
                      fc_retry_setting                                        :  2, // [21:20]
                      fc_more_frag_setting                                    :  1, // [19:19]
                      fc_from_ds_setting                                      :  1, // [18:18]
                      fc_to_ds_setting                                        :  1, // [17:17]
                      iv_mask                                                 :  1, // [16:16]
                      qc_15to8_mask                                           :  1, // [15:15]
                      reserved_0a                                             :  1, // [14:14]
                      qc_amsdu_mask                                           :  1, // [13:13]
                      qc_ack_policy_mask                                      :  1, // [12:12]
                      qc_eosp_mask                                            :  1, // [11:11]
                      qc_tid_mask                                             :  1, // [10:10]
                      sequence_control_mask                                   :  1, // [9:9]
                      duration_field_mask                                     :  1, // [8:8]
                      fc_order_mask                                           :  1, // [7:7]
                      fc_prot_frame_mask                                      :  1, // [6:6]
                      fc_more_data_mask                                       :  1, // [5:5]
                      fc_pwr_mgt_mask                                         :  1, // [4:4]
                      fc_retry_mask                                           :  1, // [3:3]
                      fc_more_frag_mask                                       :  1, // [2:2]
                      fc_from_ds_mask                                         :  1, // [1:1]
                      fc_to_ds_mask                                           :  1; // [0:0]
             uint32_t sequence_number                                         : 12, // [31:20]
                      fragment_number                                         :  4, // [19:16]
                      sequence_control_source                                 :  1, // [15:15]
                      mlo_ignore_addr3_override                               :  1, // [14:14]
                      mlo_addr_override                                       :  1, // [13:13]
                      qc_15to8_setting                                        :  8, // [12:5]
                      qc_amsdu_setting                                        :  1, // [4:4]
                      qc_ack_policy_setting                                   :  2, // [3:2]
                      qc_eosp_setting                                         :  2; // [1:0]
#endif
};


/* Description		FC_TO_DS_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			Note: Enc_type is NOT allowed b
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will get the setting
			 from the frame_ctl field in the MPDU_queue extension data
			 structure.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_to_ds_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
			
			In 11ah mode of Operation, same description as above applies
			 if this field is a part of FC field of the MPDU. This field
			 does not apply to Short MAC header (PV=1) and is ignored
			 by HW
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_LSB                              0
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_MSB                              0
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_MASK_MASK                             0x0000000000000001


/* Description		FC_FROM_DS_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will get the setting
			 from the frame_ctl field in the MPDU_queue extension data
			 structure.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_from_ds_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
			
			In 11ah mode of Operation, same description as above applies
			 if this field is a part of FC field of the MPDU.
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_LSB                            1
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_MSB                            1
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_MASK_MASK                           0x0000000000000002


/* Description		FC_MORE_FRAG_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will get the setting
			 from the frame_ctl field in the MPDU_queue extension data
			 structure.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_more_frag_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_OFFSET                       0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_LSB                          2
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_MSB                          2
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_MASK_MASK                         0x0000000000000004


/* Description		FC_RETRY_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the setting
			 for this field on the retry_bitmap_31_0 and retry_bitmap_63_32
			 fields in the MPDU_queue_extension descriptor
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_retry_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_LSB                              3
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_MSB                              3
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_MASK_MASK                             0x0000000000000008


/* Description		FC_PWR_MGT_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will get the setting
			 from the frame_ctl field in the MPDU_queue extension data
			 structure.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_pwr_mgt_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_LSB                            4
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_MSB                            4
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_MASK_MASK                           0x0000000000000010


/* Description		FC_MORE_DATA_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will get the setting
			 from the frame_ctl field in the MPDU_queue extension data
			 structure.
			
			TX_PCU has the abilty of overwrite the More data field, 
			based on the Set_fc_more_data field in the PPDU_SS_... TLVs
			 given by PDG.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_more_data_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_OFFSET                       0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_LSB                          5
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_MSB                          5
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_MASK_MASK                         0x0000000000000020


/* Description		FC_PROT_FRAME_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the setting
			 for the Protected frame bit on the key_type setting  in
			 the peer entry. When NO encryption is needed, the bit will
			 be set to 0, When the any encryption is needed, the bit
			 will be set to 0.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_prot_frame_setting. When fc_prot_frame_setting is set, 
			OLE will encrypt the frame, based on the encryption type
			 indicate with the key_type setting  in the peer entry
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_LSB                         6
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_MSB                         6
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_MASK_MASK                        0x0000000000000040


/* Description		FC_ORDER_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will get the setting
			 from the frame_ctl field in the MPDU_queue extension data
			 structure.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			fc_order_setting.
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
			
			In 11ah mode of Operation, same description as above applies
			 if this field is a part of FC field of the MPDU.
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_LSB                              7
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_MSB                              7
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_MASK_MASK                             0x0000000000000080


/* Description		DURATION_FIELD_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, TX PCU will get the
			 value for this field from the Duration fields in the PPDU_SS_... 
			TLVs from PDG.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (TX_PCU) will insert is coming from the
			 Duration fields in the PPDU_SS_... TLVs from PDG (similar
			 as with NON RAW/Native WiFi frames).
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
			
			In 11ah mode of Operation, same description as above applies
			 if this field is a part of FC field of the MPDU.
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_OFFSET                     0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_LSB                        8
#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_MSB                        8
#define TX_RAW_OR_NATIVE_FRAME_SETUP_DURATION_FIELD_MASK_MASK                       0x0000000000000100


/* Description		SEQUENCE_CONTROL_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the value
			 for  this field on sequence number field in the TX_MPDU_START
			 descriptor
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is dependent on the 
			setting in the 'sequence_control_source' field
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_OFFSET                   0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_LSB                      9
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_MSB                      9
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_MASK_MASK                     0x0000000000000200


/* Description		QC_TID_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the value
			 for  this field on the qos_ctl field from the MPDU_queue_ext
			 data structure.
			
			<enum 0 mask_disable>: HW is allowed to update this field. 
			The value that HW (OLE) will insert is the given in field: 
			qc_tid_setting.
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_OFFSET                             0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_LSB                                10
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_MSB                                10
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_MASK_MASK                               0x0000000000000400


/* Description		QC_EOSP_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the value
			 for  this field on the qos_ctl field from the MPDU_queue_ext
			 data structure.
			
			TX_PCU has the abilty of overwrite the QoS eosp field, based
			 on the Set_fc_more_data field in the PPDU_SS_... TLVs given
			 by PDG.
			
			<enum 0 mask_disable>: HW is allowed to update the QoS eosp
			 field. The value that HW (OLE) will insert is the given
			 in field: qc_eosp_setting.
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
			
			In 11ah mode of Operation, same description as above applies
			 if this field is a part of FC field of the MPDU.
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_OFFSET                            0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_LSB                               11
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_MSB                               11
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_MASK_MASK                              0x0000000000000800


/* Description		QC_ACK_POLICY_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the value
			 for  this field on the qos_ctl field from the MPDU_queue_ext
			 data structure.
			
			<enum 0 mask_disable>: HW is allowed to update the QoS ack
			 policy field. The value that HW (OLE) will insert is determined
			 by field: qc_ack_policy_setting.
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_LSB                         12
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_MSB                         12
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_MASK_MASK                        0x0000000000001000


/* Description		QC_AMSDU_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the value
			 for  this field on the qos_ctl field from the MPDU_queue_ext
			 data structure.
			
			<enum 0 mask_disable>: HW is allowed to update the QoS amsdu
			 field. The value that HW (OLE) will insert is determined
			 by field: qc_amsdu_setting.
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_LSB                              13
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_MSB                              13
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_MASK_MASK                             0x0000000000002000


/* Description		RESERVED_0A

			<legal 0> 
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_OFFSET                             0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_LSB                                14
#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_MSB                                14
#define TX_RAW_OR_NATIVE_FRAME_SETUP_RESERVED_0A_MASK                               0x0000000000004000


/* Description		QC_15TO8_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the value
			 for  this field on the qos_ctl field from the MPDU_queue_ext
			 data structure.
			
			<enum 0 mask_disable>: HW is allowed to update the QoS control
			 field, bits 15-8. The value that HW (OLE) will insert is
			 determined by field: qc_15to8_setting.
			
			<enum 1 mask_enable>: HW is not allowed to update the contents
			 of this field.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_OFFSET                           0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_LSB                              15
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_MSB                              15
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_MASK_MASK                             0x0000000000008000


/* Description		IV_MASK

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MPDU frames with MSDU enc_type == RAW
			 or Native WiFi
			
			Note: 
			When enc_type != RAW or Native WiFi, OLE will base the IV
			 field insertion/value on the on the encryption type indicate
			 with the key_type setting  in the peer entry
			
			<enum 0 mask_disable>: OLE is allowed to overwrite the IV
			 field, in case key_type setting  in the peer entry indicates
			 some encryption.
			
			<enum 1 mask_enable>: OLE  is not allowed to overwrite any
			 of the IV field contents.
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_OFFSET                                 0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_LSB                                    16
#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_MSB                                    16
#define TX_RAW_OR_NATIVE_FRAME_SETUP_IV_MASK_MASK                                   0x0000000000010000


/* Description		FC_TO_DS_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_to_ds_mask is not set.
			
			<enum 0 clear>: OLE will set the frame control field, to
			 ds bit to 0
			<enum 1 set>: OLE will set the frame control field, to ds
			 bit to 1
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_LSB                           17
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_MSB                           17
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_TO_DS_SETTING_MASK                          0x0000000000020000


/* Description		FC_FROM_DS_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_from_ds_mask is not set.
			
			<enum 0 clear>: OLE will set the frame control field, from
			 ds bit to 0
			<enum 1 set>: OLE will set the frame control field, from
			 ds bit to 1
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_LSB                         18
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_MSB                         18
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_FROM_DS_SETTING_MASK                        0x0000000000040000


/* Description		FC_MORE_FRAG_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_more_frag_mask is not set.
			
			
			<enum 0 clear>: OLE will set the frame control field, more
			 frag bit to 0
			<enum 1 set>: OLE will set the frame control field, more
			 frag bit to 1
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_OFFSET                    0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_LSB                       19
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_MSB                       19
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_FRAG_SETTING_MASK                      0x0000000000080000


/* Description		FC_RETRY_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_retry_mask is not set.
			
			<enum 0 fc_retry_clear>: OLE will set the frame control 
			field, retry bit to 0
			<enum 1 fc_retry_set>: OLE will set the frame control field, 
			retry bit to 1
			<enum 2 fc_retry_bimap_based>: OLE will base the setting
			 for this field on the retry_bitmap_31_0 and retry_bitmap_63_32
			 fields in the MPDU_queue_extension descriptor
			
			<legal 0-2>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_LSB                           20
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_MSB                           21
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_RETRY_SETTING_MASK                          0x0000000000300000


/* Description		FC_PWR_MGT_SETTING

			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_pwr_mgt_mask is not set.
			
			<enum 0 clear>: OLE will set the frame control field, pwr_mgt
			 bit to 0
			<enum 1 set>: OLE will set the frame control field, pwr_mgt
			 bit to 1
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_OFFSET                      0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_LSB                         22
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_MSB                         22
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PWR_MGT_SETTING_MASK                        0x0000000000400000


/* Description		FC_MORE_DATA_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_more_Data_mask is not set.
			
			
			<enum 0 fc_more_data_clear>: OLE will set the frame control
			 field, More data bit to 0
			<enum 1 fc_more_data_set>: OLE will set the frame control
			 field, More data bit to 1
			
			<enum 2 fc_more_data_pdg_based>: OLE will set the Frame 
			control, More data bit to 0, but TX_PCU has the abilty to
			 overwrite this based on the Set_fc_more_data field in the
			 PPDU_SS_... TLVs given by PDG.
			
			<legal 0-2>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_OFFSET                    0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_LSB                       23
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_MSB                       24
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_MORE_DATA_SETTING_MASK                      0x0000000001800000


/* Description		FC_PROT_FRAME_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_prot_frame_mask is not set.
			
			
			<enum 0 fc_prot_frame_clear>: OLE will set the frame control
			 field , "Protected Frame" bit to 0
			<enum 1 fc_prot_frame_set>: OLE will set the frame control
			 field , "Protected Frame" bit to 1
			<enum 2 fc_prot_frame_encap_type_based>: OLE configures 
			the Frame Control field, Prot frame bit according to the
			 following rule: 
			When the encryption type indicated with the key_type setting
			  in the peer entry is set to no crypto, the Frame control
			 "Protected Frame" bit is set to 0. 
			When the encryption type indicated with the key_type setting
			 in the peer entry is set to some encryption type, the OLE
			 will set the frame control "Protected Frame" bit to 1.
			OLE changes only the value of the prot_frame bit. It won't
			 push IV in the frame according to this bit.
			
			<legal 0-2>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_OFFSET                   0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_LSB                      25
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_MSB                      26
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_PROT_FRAME_SETTING_MASK                     0x0000000006000000


/* Description		FC_ORDER_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Fc_order_mask is not set.
			
			<enum 0 clear>: OLE will set the frame control field , order
			 bit to 0
			<enum 1 set>: OLE will set the frame control field , order
			 bit to 1
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_LSB                           27
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_MSB                           27
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FC_ORDER_SETTING_MASK                          0x0000000008000000


/* Description		QC_TID_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Qc_tid_mask is not set.
			
			OLE sets the TID field in the QoS control field to this 
			value.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_OFFSET                          0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_LSB                             28
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_MSB                             31
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_TID_SETTING_MASK                            0x00000000f0000000


/* Description		QC_EOSP_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Qc_eosp_mask is not set.
			
			<enum 0 qc_eosp_clear>: OLE will set the QoS control bit
			 to 0
			<enum 1 qc_eosp_set>: OLE will set the QoS control bit to
			 1
			<enum 2 qc_eosp_pdg_based>: OLE will set the QoS control
			 bit to 0, but TX_PCU has the abilty of overwrite the QoS
			 eosp field, based on the Set_fc_more_data field in the 
			PPDU_SS_... TLVs given by PDG.
			
			<legal 0-2>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_LSB                            32
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_MSB                            33
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_EOSP_SETTING_MASK                           0x0000000300000000


/* Description		QC_ACK_POLICY_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Qc_ack_policy_mask is not set.
			
			
			This is is QoS ACK policy value that RXOLE shall put in 
			the ACK policy field in the QoS control field
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_OFFSET                   0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_LSB                      34
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_MSB                      35
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_ACK_POLICY_SETTING_MASK                     0x0000000c00000000


/* Description		QC_AMSDU_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Qc_amsdu_mask is not set.
			
			<enum 0 clear>: OLE will set the QoS control field amsdu
			 bit to 0
			<enum 1 set>: OLE will set the QoS control field amsdu bit
			 to 1
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_LSB                           36
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_MSB                           36
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_AMSDU_SETTING_MASK                          0x0000001000000000


/* Description		QC_15TO8_SETTING

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			Field only valid when field Qc_15to8_mask is not set.
			
			OLE sets bit 8 to 16 in the QoS control field to this value.
			
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_OFFSET                        0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_LSB                           37
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_MSB                           44
#define TX_RAW_OR_NATIVE_FRAME_SETUP_QC_15TO8_SETTING_MASK                          0x00001fe000000000


/* Description		MLO_ADDR_OVERRIDE

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			
			Enables address translation for raw Wi-Fi frames to multi-link
			 peers, esp. management frames
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_OFFSET                       0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_LSB                          45
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_MSB                          45
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_ADDR_OVERRIDE_MASK                         0x0000200000000000


/* Description		MLO_IGNORE_ADDR3_OVERRIDE

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi when Mlo_addr_override is set.
			
			Preserves Address3 (BSSID) for raw Wi-Fi management frames
			 to multi-link peers.
			
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_OFFSET               0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_LSB                  46
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_MSB                  46
#define TX_RAW_OR_NATIVE_FRAME_SETUP_MLO_IGNORE_ADDR3_OVERRIDE_MASK                 0x0000400000000000


/* Description		SEQUENCE_CONTROL_SOURCE

			Field only valid when field Sequence_control_mask is set
			 to 'mask_disable'.
			
			<enum 0 seq_ctrl_source_mpdu_start>: OLE will set the sequence
			 control field based on what is indicated in the TX_MPDU_START
			 TLV. 
			
			<enum 1 seq_ctrl_source_this_tlv>: OLE will set the sequence
			 control field based on what is indicated in this TLV, fields
			 Fragment_number and Sequence_number
			Note that this setting assumes that there is only a single
			 RAW or Native Wifi MPDU for this user in the transmit path. 
			This works well for level 1 fragmentation. Reason that there
			 should only be a single RAW or Native WiFi frames is that
			 with this feature they would all get the same sequence + 
			fragment  number
			
			<legal 0-1>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_OFFSET                 0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_LSB                    47
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_MSB                    47
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_CONTROL_SOURCE_MASK                   0x0000800000000000


/* Description		FRAGMENT_NUMBER

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			
			Field only valid when field Sequence_control_mask =  mask_disable
			 AND sequence_control_source is set to seq_ctrl_source_this_tlv
			
			
			The Fragment number to be filled in 
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_LSB                            48
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_MSB                            51
#define TX_RAW_OR_NATIVE_FRAME_SETUP_FRAGMENT_NUMBER_MASK                           0x000f000000000000


/* Description		SEQUENCE_NUMBER

			Consumer: TXOLE
			Producer: SW
			
			Field only valid for MSDU frames with enc_type == RAW or
			 Native WiFi.
			
			Field only valid when field Sequence_control_mask =  mask_disable
			 AND sequence_control_source is set to seq_ctrl_source_this_tlv
			
			
			The Sequence number to be filled in 
			<legal all>
*/

#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_OFFSET                         0x0000000000000000
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_LSB                            52
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_MSB                            63
#define TX_RAW_OR_NATIVE_FRAME_SETUP_SEQUENCE_NUMBER_MASK                           0xfff0000000000000



#endif   // TX_RAW_OR_NATIVE_FRAME_SETUP

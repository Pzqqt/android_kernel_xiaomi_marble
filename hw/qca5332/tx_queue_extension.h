
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

 
 
 
 
 
 
 


#ifndef _TX_QUEUE_EXTENSION_H_
#define _TX_QUEUE_EXTENSION_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_QUEUE_EXTENSION 14

#define NUM_OF_QWORDS_TX_QUEUE_EXTENSION 7


struct tx_queue_extension {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t frame_ctl                                               : 16, // [15:0]
                      qos_ctl                                                 : 16; // [31:16]
             uint32_t ampdu_flag                                              :  1, // [0:0]
                      tx_notify_no_htc_override                               :  1, // [1:1]
                      reserved_1a                                             :  7, // [8:2]
                      checksum_tso_disable_for_frag                           :  1, // [9:9]
                      key_id                                                  :  8, // [17:10]
                      qos_buf_state_overwrite                                 :  1, // [18:18]
                      buf_state_sta_id                                        :  1, // [19:19]
                      buf_state_source                                        :  1, // [20:20]
                      ht_control_overwrite_enable                             :  1, // [21:21]
                      ht_control_overwrite_source                             :  4, // [25:22]
                      reserved_1b                                             :  6; // [31:26]
             uint32_t ul_headroom_insertion_enable                            :  1, // [0:0]
                      ul_headroom_offset                                      :  5, // [5:1]
                      bqrp_insertion_enable                                   :  1, // [6:6]
                      bqrp_offset                                             :  5, // [11:7]
                      ul_headroom_rsvd_7_6                                    :  2, // [13:12]
                      bqr_rsvd_9_8                                            :  2, // [15:14]
                      base_pn_63_48                                           : 16; // [31:16]
             uint32_t base_pn_95_64                                           : 32; // [31:0]
             uint32_t base_pn_127_96                                          : 32; // [31:0]
             uint32_t ht_control_field_bw20                                   : 32; // [31:0]
             uint32_t ht_control_field_bw40                                   : 32; // [31:0]
             uint32_t ht_control_field_bw80                                   : 32; // [31:0]
             uint32_t ht_control_field_bw160                                  : 32; // [31:0]
             uint32_t ht_control_overwrite_mask                               : 32; // [31:0]
             uint32_t cas_control_info                                        :  8, // [7:0]
                      cas_offset                                              :  5, // [12:8]
                      cas_insertion_enable                                    :  1, // [13:13]
                      reserved_10a                                            :  2, // [15:14]
                      ht_control_overwrite_source_for_srp                     :  4, // [19:16]
                      ht_control_overwrite_source_for_bsrp                    :  4, // [23:20]
                      reserved_10b                                            :  6, // [29:24]
                      mpdu_hdr_len_override_en                                :  1, // [30:30]
                      bar_ssn_overwrite_enable                                :  1; // [31:31]
             uint32_t bar_ssn_offset                                          : 12, // [11:0]
                      mpdu_hdr_len_override_val                               :  9, // [20:12]
                      reserved_11a                                            : 11; // [31:21]
             uint32_t ht_control_field_bw320                                  : 32; // [31:0]
             uint32_t fw2sw_info                                              : 32; // [31:0]
#else
             uint32_t qos_ctl                                                 : 16, // [31:16]
                      frame_ctl                                               : 16; // [15:0]
             uint32_t reserved_1b                                             :  6, // [31:26]
                      ht_control_overwrite_source                             :  4, // [25:22]
                      ht_control_overwrite_enable                             :  1, // [21:21]
                      buf_state_source                                        :  1, // [20:20]
                      buf_state_sta_id                                        :  1, // [19:19]
                      qos_buf_state_overwrite                                 :  1, // [18:18]
                      key_id                                                  :  8, // [17:10]
                      checksum_tso_disable_for_frag                           :  1, // [9:9]
                      reserved_1a                                             :  7, // [8:2]
                      tx_notify_no_htc_override                               :  1, // [1:1]
                      ampdu_flag                                              :  1; // [0:0]
             uint32_t base_pn_63_48                                           : 16, // [31:16]
                      bqr_rsvd_9_8                                            :  2, // [15:14]
                      ul_headroom_rsvd_7_6                                    :  2, // [13:12]
                      bqrp_offset                                             :  5, // [11:7]
                      bqrp_insertion_enable                                   :  1, // [6:6]
                      ul_headroom_offset                                      :  5, // [5:1]
                      ul_headroom_insertion_enable                            :  1; // [0:0]
             uint32_t base_pn_95_64                                           : 32; // [31:0]
             uint32_t base_pn_127_96                                          : 32; // [31:0]
             uint32_t ht_control_field_bw20                                   : 32; // [31:0]
             uint32_t ht_control_field_bw40                                   : 32; // [31:0]
             uint32_t ht_control_field_bw80                                   : 32; // [31:0]
             uint32_t ht_control_field_bw160                                  : 32; // [31:0]
             uint32_t ht_control_overwrite_mask                               : 32; // [31:0]
             uint32_t bar_ssn_overwrite_enable                                :  1, // [31:31]
                      mpdu_hdr_len_override_en                                :  1, // [30:30]
                      reserved_10b                                            :  6, // [29:24]
                      ht_control_overwrite_source_for_bsrp                    :  4, // [23:20]
                      ht_control_overwrite_source_for_srp                     :  4, // [19:16]
                      reserved_10a                                            :  2, // [15:14]
                      cas_insertion_enable                                    :  1, // [13:13]
                      cas_offset                                              :  5, // [12:8]
                      cas_control_info                                        :  8; // [7:0]
             uint32_t reserved_11a                                            : 11, // [31:21]
                      mpdu_hdr_len_override_val                               :  9, // [20:12]
                      bar_ssn_offset                                          : 12; // [11:0]
             uint32_t ht_control_field_bw320                                  : 32; // [31:0]
             uint32_t fw2sw_info                                              : 32; // [31:0]
#endif
};


/* Description		FRAME_CTL

			Consumer: TXOLE
			Producer: SW 
			
			
			802.11 Frame control field: 
			fc [1:0]: Protocol Version
			fc[7:2]: type/subtypeFor non-11ah  fc[3:2] = Type  fc[7:4] = 
			Subtype For 11ah  fc[4:2] = Typefc[7:5] = PTID/SubType
			fc [8]: To DS ( for Non-11ah)  From DS ( for 11ah )
			fc [9]: From DS ( for Non-11ah )
			  More Frag ( for 11ah )
			fc [10]: More Frag ( for Non-11ah )
			  Power Management ( for 11ah)
			fc [11]: Retry ( for Non-11ah )
			  More Data ( for 11ah )
			fc [12]: Pwr Mgt ( for Non-11ah )
			  Protected Frame ( for 11ah )
			fc [13]: More Data( for Non-11ah )
			  EOSP ( for 11ah )
			fc [14]: Protected Frame ( for Non-11ah)
			  Relayed Frame ( for 11ah )
			fc [15]: Order ( for Non-11ah )
			Ack Policy ( for 11ah )
			Used by OLE during the encapsulation process for Native 
			WiFi, Ethernet II, and 802.3.
			When the Order field is set, TXOLE shall insert 4 placeholder
			 bytes for the HE-control field in the frame. TXPCU will
			 overwrite them with the final actual value...
*/

#define TX_QUEUE_EXTENSION_FRAME_CTL_OFFSET                                         0x0000000000000000
#define TX_QUEUE_EXTENSION_FRAME_CTL_LSB                                            0
#define TX_QUEUE_EXTENSION_FRAME_CTL_MSB                                            15
#define TX_QUEUE_EXTENSION_FRAME_CTL_MASK                                           0x000000000000ffff


/* Description		QOS_CTL

			Consumer: TXOLE
			Producer: SW 
			
			QoS control field is valid if the type field is data and
			 the upper bit of the subtype field is set.  The field decode
			 is as below:
			qos_ctl[3:0]: TID
			qos_ctl[4]: EOSP (with some exceptions)
			qos_ctl[6:5]: Ack Policy
			0x0: Normal Ack or Implicit BAR
			0x1: No Ack
			0x2: No explicit Ack or PSMP Ack (not supported)
			0x3: Block Ack (Not supported)
			Qos_ctl[7]: A-MSDU Present (with some exceptions)
			Qos_ctl[15:8]: TXOP limit, AP PS buffer state, TXOP duration
			 requested or queue size 
			This field is inserted into the 802.11 header during the
			 encapsulation process
			<legal all>
*/

#define TX_QUEUE_EXTENSION_QOS_CTL_OFFSET                                           0x0000000000000000
#define TX_QUEUE_EXTENSION_QOS_CTL_LSB                                              16
#define TX_QUEUE_EXTENSION_QOS_CTL_MSB                                              31
#define TX_QUEUE_EXTENSION_QOS_CTL_MASK                                             0x00000000ffff0000


/* Description		AMPDU_FLAG

			Consumer: PDG/TXPCU
			Producer: SW 
			
			Note: 
			For legacy rate transmissions (11 b and 11a, an 11g), this
			 bit shall always be set to zero.
			
			0:
			For legacy and .11n rates:
			MPDUs are only allowed to be sent out 1 at a time in NON
			 A-MPDU format.
			For .11ac and .11ax rates:
			MPDUs are sent out in S-MPDU format (TXPCU sets the 'EOF' 
			bit in the MPDU delimiter).
			1: All MPDUs should be sent out using the A-MPDU format, 
			even if there is only one MPDU.
			
			Note that this bit should be set to 0 in order to construct
			 an S-MPDU frame. VHT and HE frames are all A-MPDU format
			 but if this bit is clear, EOF bit is set to 1 for the MPDU
			 delimiter in A-MPDU, which is the indicator of S-MPDU and
			 solicits ACK rather than BA as response frame. 
			
			This bit shall be set to 1 for any MD (Multi Destination) 
			transmission.
*/

#define TX_QUEUE_EXTENSION_AMPDU_FLAG_OFFSET                                        0x0000000000000000
#define TX_QUEUE_EXTENSION_AMPDU_FLAG_LSB                                           32
#define TX_QUEUE_EXTENSION_AMPDU_FLAG_MSB                                           32
#define TX_QUEUE_EXTENSION_AMPDU_FLAG_MASK                                          0x0000000100000000


/* Description		TX_NOTIFY_NO_HTC_OVERRIDE

			When set, and a 'TX_MPDU_START' TLV has Tx_notify_frame 
			set to TX_HARD_NOTIFY or TX_SOFT_NOTIFY or TX_SEMI_HARD_NOTIFY, 
			then PDG would have updated the rate fields for a legacy
			 PPDU which may not support HT Control.
			
			In this case TXOLE shall not:
			set the Order/+HTC bit in the 'Frame Control,'
			include 4 bytes for TXPCU to fill the HT Control, or
			set vht_control_present in 'TX_MPDU_START,'
			even if requested, and instead shall subtract '4' from the
			 mpdu_length in 'TX_MPDU_START' and overwrite it.
			
			Hamilton v1 used bits [29:26], [8:1] along with word 11 
			bits [31:12] for 'HT_control_field_bw320.'
			<legal all>
*/

#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_OFFSET                         0x0000000000000000
#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_LSB                            33
#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_MSB                            33
#define TX_QUEUE_EXTENSION_TX_NOTIFY_NO_HTC_OVERRIDE_MASK                           0x0000000200000000


/* Description		RESERVED_1A

			Hamilton v1 used bits [29:26], [8:1] along with word 11 
			bits [31:12] for 'HT_control_field_bw320.'
			<legal 0>
*/

#define TX_QUEUE_EXTENSION_RESERVED_1A_OFFSET                                       0x0000000000000000
#define TX_QUEUE_EXTENSION_RESERVED_1A_LSB                                          34
#define TX_QUEUE_EXTENSION_RESERVED_1A_MSB                                          40
#define TX_QUEUE_EXTENSION_RESERVED_1A_MASK                                         0x000001fc00000000


/* Description		CHECKSUM_TSO_DISABLE_FOR_FRAG

			Field only valid in case of level-1 fragmentation, identified
			 by TXOLE getting the 'TX_FRAG_STATE' TLV
			
			If set, TXOLE disables all checksum and TSO overwrites for
			 the fragment(s) being transmitted.
			
			This is useful if it is known that the checksum and TSO 
			overwrites affect only the first fragment (or first few 
			fragments) and for the rest these can be safely disabled.
			
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_OFFSET                     0x0000000000000000
#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_LSB                        41
#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_MSB                        41
#define TX_QUEUE_EXTENSION_CHECKSUM_TSO_DISABLE_FOR_FRAG_MASK                       0x0000020000000000


/* Description		KEY_ID

			Field only valid in case of encryption, and TXOLE being 
			instructured to insert the IV.
			
			TXOLE blindly copies this field into the key ID octet (which
			 is part of the IV) of the encrypted frame.
			
			For AES/TKIP the encoding is:
			key_id_octet[7:6]: key ID
			key_id_octet[5]: extended IV: 
			key_id_octet[4:0]: Reserved bits
			
			For WEP the encoding is:
			key_id_octet[7:6]: key ID
			key_id_octet[5]: extended IV: 
			key_id_octet[4:0]: Reserved bits
			
			For WAPI the encoding is:
			key_id_octet[7:2]: Reserved bits
			key_id_octet[1:0]: key ID
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_KEY_ID_OFFSET                                            0x0000000000000000
#define TX_QUEUE_EXTENSION_KEY_ID_LSB                                               42
#define TX_QUEUE_EXTENSION_KEY_ID_MSB                                               49
#define TX_QUEUE_EXTENSION_KEY_ID_MASK                                              0x0003fc0000000000


/* Description		QOS_BUF_STATE_OVERWRITE

			When clear, TXPCU shall not overwrite buffer state field
			 in the QoS frame control field.
			
			When set, TXPCU shall overwrite the buffer state field in
			 the QoS frame control field, with info that SW has programmed
			 in TXPCU registers. Note that TXPCU shall pick up the values
			 related to this TID.
			<legal all>
*/

#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_OFFSET                           0x0000000000000000
#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_LSB                              50
#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_MSB                              50
#define TX_QUEUE_EXTENSION_QOS_BUF_STATE_OVERWRITE_MASK                             0x0004000000000000


/* Description		BUF_STATE_STA_ID

			Field only valid when QoS_Buf_state_overwrite is set.
			
			This field indicates what the STA ID register source is 
			of the buffer status.
			
			1'b0: TXPCU registers: STA0_buf_status_... 
			1'b1: TXPCU registers: STA1_buf_status_... 
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_OFFSET                                  0x0000000000000000
#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_LSB                                     51
#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_MSB                                     51
#define TX_QUEUE_EXTENSION_BUF_STATE_STA_ID_MASK                                    0x0008000000000000


/* Description		BUF_STATE_SOURCE

			Field only valid when QoS_Buf_state_overwrite is set.
			
			This field indicates what the source is of the actual value
			 TXPCU will insert
			
			<enum 0 BUF_STATE_TID_BASED> TXPCU looks at the TID field
			 in the QoS control frame and based on this TID, selects
			 the buffer source value from the corresponding TID register.
			
			<enum 1 BUF_STATE_SUM_BASED> TXPCU inserts the value from
			 the buffer_state_sum register
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_OFFSET                                  0x0000000000000000
#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_LSB                                     52
#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_MSB                                     52
#define TX_QUEUE_EXTENSION_BUF_STATE_SOURCE_MASK                                    0x0010000000000000


/* Description		HT_CONTROL_OVERWRITE_ENABLE

			When set, TXPCU shall overwrite some (or all) of the HT_CONTROL
			 field with values that are programmed in TXPCU registers: 
			HT_CONTROL_OVERWRITE_IX???
			
			See HT/HE control overwrite order NOTE after this table
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_OFFSET                       0x0000000000000000
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_LSB                          53
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_MSB                          53
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_ENABLE_MASK                         0x0020000000000000


/* Description		HT_CONTROL_OVERWRITE_SOURCE

			Field only valid when HT_control_overwrite_enable  is set.
			
			
			This field indicates the index of the TXPCU register HT_CONTROL_OVERWRITE_IX??? 
			That is the source of the overwrite data.
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_OFFSET                       0x0000000000000000
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_LSB                          54
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_MSB                          57
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_MASK                         0x03c0000000000000


/* Description		RESERVED_1B

			Hamilton v1 used bits [29:26], [8:1] along with word 11 
			bits [31:12] for 'HT_control_field_bw320.'
			<legal 0>
*/

#define TX_QUEUE_EXTENSION_RESERVED_1B_OFFSET                                       0x0000000000000000
#define TX_QUEUE_EXTENSION_RESERVED_1B_LSB                                          58
#define TX_QUEUE_EXTENSION_RESERVED_1B_MSB                                          63
#define TX_QUEUE_EXTENSION_RESERVED_1B_MASK                                         0xfc00000000000000


/* Description		UL_HEADROOM_INSERTION_ENABLE

			When set, and this transmission services a trigger response
			 transmission, TXPCU shall create and insert the UL headroom
			 info in the HE control field, starting at offset indicated
			 by field: UL_headroom_offset
			
			See HT/HE control overwrite order NOTE after this table
			<legal all>
*/

#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_OFFSET                      0x0000000000000008
#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_LSB                         0
#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_MSB                         0
#define TX_QUEUE_EXTENSION_UL_HEADROOM_INSERTION_ENABLE_MASK                        0x0000000000000001


/* Description		UL_HEADROOM_OFFSET

			Field only valid when UL_headroom_insertion_enable is set.
			
			
			The bit location in HE_CONTROL Field where TXPCU will start
			 writing the the 4 bit Control ID field that needs to be
			 inserted, followed by the lower 6 bits of the 8 bit bit
			 UL_headroom info (UPH Control). 
			
			NOTE: currently on 6 bits are defined in the UPH control
			 field. The upper two bits are provided by SW in UL_headroom_rsvd_7_6.
			
			
			<legal 2-20>
*/

#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_OFFSET                                0x0000000000000008
#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_LSB                                   1
#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_MSB                                   5
#define TX_QUEUE_EXTENSION_UL_HEADROOM_OFFSET_MASK                                  0x000000000000003e


/* Description		BQRP_INSERTION_ENABLE

			When set, and this transmission services a BQRP trigger 
			response transmission, TXPCU shall create and insert the
			 BQR control field into the HE control field, as well as
			 the 4 bit preceding Control ID field.
			
			See HT/HE control overwrite order NOTE after this table
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_OFFSET                             0x0000000000000008
#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_LSB                                6
#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_MSB                                6
#define TX_QUEUE_EXTENSION_BQRP_INSERTION_ENABLE_MASK                               0x0000000000000040


/* Description		BQRP_OFFSET

			Field only valid when BQRP_insertion_enable is set.
			
			The bit location in HE_CONTROL Field where TXPCU will start
			 writing the 4 bit Control ID field that needs to be inserted, 
			followed by the lower 8 bits of the 10 bit BQR control field.
			
			
			NOTE: currently only 8 bits are defined in the 10 bit BQR
			 control field. The upper two bits are provided by SW in
			 BQR_rsvd_9_8.
			
			<legal 2-20>
*/

#define TX_QUEUE_EXTENSION_BQRP_OFFSET_OFFSET                                       0x0000000000000008
#define TX_QUEUE_EXTENSION_BQRP_OFFSET_LSB                                          7
#define TX_QUEUE_EXTENSION_BQRP_OFFSET_MSB                                          11
#define TX_QUEUE_EXTENSION_BQRP_OFFSET_MASK                                         0x0000000000000f80


/* Description		UL_HEADROOM_RSVD_7_6

			These will be used by TXPCU to fill the upper two bits of
			 the UPH control field.
			<legal all>
*/

#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_OFFSET                              0x0000000000000008
#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_LSB                                 12
#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_MSB                                 13
#define TX_QUEUE_EXTENSION_UL_HEADROOM_RSVD_7_6_MASK                                0x0000000000003000


/* Description		BQR_RSVD_9_8

			These will be used by TXPCU to fill the upper two bits of
			 the BQR control field.
			NOTE: When overwriting CAS control (8-bit) at the same offset
			 as BQR control (10-bit), TXPCU will ignore the BQR overwrite, 
			including these upper two bits.
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_OFFSET                                      0x0000000000000008
#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_LSB                                         14
#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_MSB                                         15
#define TX_QUEUE_EXTENSION_BQR_RSVD_9_8_MASK                                        0x000000000000c000


/* Description		BASE_PN_63_48

			Upper bits PN number, in case a larger then 48 bit PN number
			 needs to be inserted in the transmit frame.
			
			63-48 bits of the 128-bit packet number
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BASE_PN_63_48_OFFSET                                     0x0000000000000008
#define TX_QUEUE_EXTENSION_BASE_PN_63_48_LSB                                        16
#define TX_QUEUE_EXTENSION_BASE_PN_63_48_MSB                                        31
#define TX_QUEUE_EXTENSION_BASE_PN_63_48_MASK                                       0x00000000ffff0000


/* Description		BASE_PN_95_64

			Upper bits PN number, in case a larger then 48 bit PN number
			 needs to be inserted in the transmit frame.
			
			95-64 bits of the 128-bit packet number
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BASE_PN_95_64_OFFSET                                     0x0000000000000008
#define TX_QUEUE_EXTENSION_BASE_PN_95_64_LSB                                        32
#define TX_QUEUE_EXTENSION_BASE_PN_95_64_MSB                                        63
#define TX_QUEUE_EXTENSION_BASE_PN_95_64_MASK                                       0xffffffff00000000


/* Description		BASE_PN_127_96

			Upper bits PN number, in case a larger then 48 bit PN number
			 needs to be inserted in the transmit frame.
			
			127-96 bits of the 128-bit packet number
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BASE_PN_127_96_OFFSET                                    0x0000000000000010
#define TX_QUEUE_EXTENSION_BASE_PN_127_96_LSB                                       0
#define TX_QUEUE_EXTENSION_BASE_PN_127_96_MSB                                       31
#define TX_QUEUE_EXTENSION_BASE_PN_127_96_MASK                                      0x00000000ffffffff


/* Description		HT_CONTROL_FIELD_BW20

			Field used by TXPCU when in TX_MPDU_START TLV field vht_control_present
			  is set.
			
			Note that TXPCU might overwrite some fields. This is controlled
			 with field  HT_control_overwrite_enable
			
			See HT/HE control overwrite order NOTE after this table
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_OFFSET                             0x0000000000000010
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_LSB                                32
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_MSB                                63
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW20_MASK                               0xffffffff00000000


/* Description		HT_CONTROL_FIELD_BW40

			Field used by TXPCU when in TX_MPDU_START TLV field vht_control_present
			  is set.
			
			Note that TXPCU might overwrite some fields. This is controlled
			 with field  HT_control_overwrite_enable
			
			See HT/HE control overwrite order NOTE after this table
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_OFFSET                             0x0000000000000018
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_LSB                                0
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_MSB                                31
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW40_MASK                               0x00000000ffffffff


/* Description		HT_CONTROL_FIELD_BW80

			Field used by TXPCU when in TX_MPDU_START TLV field vht_control_present
			  is set.
			
			Note that TXPCU might overwrite some fields. This is controlled
			 with field  HT_control_overwrite_enable
			
			See HT/HE control overwrite order NOTE after this table
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_OFFSET                             0x0000000000000018
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_LSB                                32
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_MSB                                63
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW80_MASK                               0xffffffff00000000


/* Description		HT_CONTROL_FIELD_BW160

			Field used by TXPCU when in TX_MPDU_START TLV field vht_control_present
			  is set.
			
			Note that TXPCU might overwrite some fields. This is controlled
			 with field  HT_control_overwrite_enable
			
			See HT/HE control overwrite order NOTE after this table
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_OFFSET                            0x0000000000000020
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_LSB                               0
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_MSB                               31
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW160_MASK                              0x00000000ffffffff


/* Description		HT_CONTROL_OVERWRITE_MASK

			Field only valid when HT_control_overwrite_enable  is set.
			
			
			This field indicates which bits of the HT_CONTROL_FIELD 
			shall be overwritten with bits from TXPCU register HT_CONTROL_OVERWRITE_IX??? 
			
			Every bit that needs to be overwritten is set to 1 in this
			 register.
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_OFFSET                         0x0000000000000020
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_LSB                            32
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_MSB                            63
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_MASK_MASK                           0xffffffff00000000


/* Description		CAS_CONTROL_INFO

			This contains 8-bit CAS control field to be used for transmission
			 during SRP window
			<legal all>
*/

#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_OFFSET                                  0x0000000000000028
#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_LSB                                     0
#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_MSB                                     7
#define TX_QUEUE_EXTENSION_CAS_CONTROL_INFO_MASK                                    0x00000000000000ff


/* Description		CAS_OFFSET

			5 bit offset for CAS insertion
			<legal 2-20>
*/

#define TX_QUEUE_EXTENSION_CAS_OFFSET_OFFSET                                        0x0000000000000028
#define TX_QUEUE_EXTENSION_CAS_OFFSET_LSB                                           8
#define TX_QUEUE_EXTENSION_CAS_OFFSET_MSB                                           12
#define TX_QUEUE_EXTENSION_CAS_OFFSET_MASK                                          0x0000000000001f00


/* Description		CAS_INSERTION_ENABLE

			single bit used as ENABLE for CAS control insertion for 
			transmission during SRP window
			<legal all>
*/

#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_OFFSET                              0x0000000000000028
#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_LSB                                 13
#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_MSB                                 13
#define TX_QUEUE_EXTENSION_CAS_INSERTION_ENABLE_MASK                                0x0000000000002000


/* Description		RESERVED_10A

			<legal 0>
*/

#define TX_QUEUE_EXTENSION_RESERVED_10A_OFFSET                                      0x0000000000000028
#define TX_QUEUE_EXTENSION_RESERVED_10A_LSB                                         14
#define TX_QUEUE_EXTENSION_RESERVED_10A_MSB                                         15
#define TX_QUEUE_EXTENSION_RESERVED_10A_MASK                                        0x000000000000c000


/* Description		HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP

			4-bit index similar to HT_control_overwrite_source field
			 to be used for transmission during SRP window
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_OFFSET               0x0000000000000028
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_LSB                  16
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_MSB                  19
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_SRP_MASK                 0x00000000000f0000


/* Description		HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP

			4-bit index similar to HT_control_overwrite_source field
			 to be used for response to BSRP triggers (even during SRP
			 window)
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_OFFSET              0x0000000000000028
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_LSB                 20
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_MSB                 23
#define TX_QUEUE_EXTENSION_HT_CONTROL_OVERWRITE_SOURCE_FOR_BSRP_MASK                0x0000000000f00000


/* Description		RESERVED_10B

			<legal 0>
*/

#define TX_QUEUE_EXTENSION_RESERVED_10B_OFFSET                                      0x0000000000000028
#define TX_QUEUE_EXTENSION_RESERVED_10B_LSB                                         24
#define TX_QUEUE_EXTENSION_RESERVED_10B_MSB                                         29
#define TX_QUEUE_EXTENSION_RESERVED_10B_MASK                                        0x000000003f000000


/* Description		MPDU_HDR_LEN_OVERRIDE_EN

			This is for the FW override of MPDU overhead length programmed
			 in the TQM queue.
			
			If enabled, PDG will update the length of each MPDU by subtracting
			 the value of field Mpdu_header_length  in 'MPDU_QUEUE_OVERVIEW' 
			and adding Mpdu_hdr_len_override_val (in this TLV).
			<legal all>
*/

#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_OFFSET                          0x0000000000000028
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_LSB                             30
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_MSB                             30
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_EN_MASK                            0x0000000040000000


/* Description		BAR_SSN_OVERWRITE_ENABLE

			If enabled, TXPCU will overwrite the starting sequence number
			 in case of Tx BAR or MU-BAR Trigger from with the sequence
			 number from 'MPDU_QUEUE_OVERVIEW'
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_OFFSET                          0x0000000000000028
#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_LSB                             31
#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_MSB                             31
#define TX_QUEUE_EXTENSION_BAR_SSN_OVERWRITE_ENABLE_MASK                            0x0000000080000000


/* Description		BAR_SSN_OFFSET

			Offset to the starting sequence number in case of Tx BAR
			 or MU-BAR Trigger that TXPCU can overwrite with the sequence
			 number from 'MPDU_QUEUE_OVERVIEW'
			<legal all>
*/

#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_OFFSET                                    0x0000000000000028
#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_LSB                                       32
#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_MSB                                       43
#define TX_QUEUE_EXTENSION_BAR_SSN_OFFSET_MASK                                      0x00000fff00000000


/* Description		MPDU_HDR_LEN_OVERRIDE_VAL

			This is for the FW override of MPDU overhead length programmed
			 in the TQM queue.
			
			See field Mpdu_hdr_len_override_en.
			
			Hamilton v1 used bits [31:12] along with word 1 bits [29:26], [8:1] 
			for 'HT_control_field_bw320.'
			<legal all>
*/

#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_OFFSET                         0x0000000000000028
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_LSB                            44
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_MSB                            52
#define TX_QUEUE_EXTENSION_MPDU_HDR_LEN_OVERRIDE_VAL_MASK                           0x001ff00000000000


/* Description		RESERVED_11A

			Hamilton v1 used bits [31:12] along with word 1 bits [29:26], [8:1] 
			for 'HT_control_field_bw320.'
			<legal 0>
*/

#define TX_QUEUE_EXTENSION_RESERVED_11A_OFFSET                                      0x0000000000000028
#define TX_QUEUE_EXTENSION_RESERVED_11A_LSB                                         53
#define TX_QUEUE_EXTENSION_RESERVED_11A_MSB                                         63
#define TX_QUEUE_EXTENSION_RESERVED_11A_MASK                                        0xffe0000000000000


/* Description		HT_CONTROL_FIELD_BW320

			Field used by TXPCU when in TX_MPDU_START TLV field vht_control_present
			  is set.
			
			Note that TXPCU might overwrite some fields. This is controlled
			 with field  HT_control_overwrite_enable
			
			See HT/HE control overwrite order NOTE after this table
			
			Hamilton v1 did not include this (and any subsequent) word.
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_OFFSET                            0x0000000000000030
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_LSB                               0
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_MSB                               31
#define TX_QUEUE_EXTENSION_HT_CONTROL_FIELD_BW320_MASK                              0x00000000ffffffff


/* Description		FW2SW_INFO

			This field is provided by FW, to be logged via TXMON to 
			host SW. It is transparent to HW.
			
			<legal all>
*/

#define TX_QUEUE_EXTENSION_FW2SW_INFO_OFFSET                                        0x0000000000000030
#define TX_QUEUE_EXTENSION_FW2SW_INFO_LSB                                           32
#define TX_QUEUE_EXTENSION_FW2SW_INFO_MSB                                           63
#define TX_QUEUE_EXTENSION_FW2SW_INFO_MASK                                          0xffffffff00000000



#endif   // TX_QUEUE_EXTENSION

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

#ifndef _RX_MPDU_END_H_
#define _RX_MPDU_END_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MPDU_END 4

#define NUM_OF_QWORDS_RX_MPDU_END 2


struct rx_mpdu_end {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t rxpcu_mpdu_filter_in_category                           :  2, // [1:0]
                      sw_frame_group_id                                       :  7, // [8:2]
                      reserved_0                                              :  7, // [15:9]
                      phy_ppdu_id                                             : 16; // [31:16]
             uint32_t reserved_1a                                             : 11, // [10:0]
                      unsup_ktype_short_frame                                 :  1, // [11:11]
                      rx_in_tx_decrypt_byp                                    :  1, // [12:12]
                      overflow_err                                            :  1, // [13:13]
                      mpdu_length_err                                         :  1, // [14:14]
                      tkip_mic_err                                            :  1, // [15:15]
                      decrypt_err                                             :  1, // [16:16]
                      unencrypted_frame_err                                   :  1, // [17:17]
                      pn_fields_contain_valid_info                            :  1, // [18:18]
                      fcs_err                                                 :  1, // [19:19]
                      msdu_length_err                                         :  1, // [20:20]
                      rxdma0_destination_ring                                 :  3, // [23:21]
                      rxdma1_destination_ring                                 :  3, // [26:24]
                      decrypt_status_code                                     :  3, // [29:27]
                      rx_bitmap_not_updated                                   :  1, // [30:30]
                      reserved_1b                                             :  1; // [31:31]
             uint32_t reserved_2a                                             : 15, // [14:0]
                      rxpcu_mgmt_sequence_nr_valid                            :  1, // [15:15]
                      rxpcu_mgmt_sequence_nr                                  : 16; // [31:16]
             uint32_t rxframe_assert_mlo_timestamp                            : 32; // [31:0]
#else
             uint32_t phy_ppdu_id                                             : 16, // [31:16]
                      reserved_0                                              :  7, // [15:9]
                      sw_frame_group_id                                       :  7, // [8:2]
                      rxpcu_mpdu_filter_in_category                           :  2; // [1:0]
             uint32_t reserved_1b                                             :  1, // [31:31]
                      rx_bitmap_not_updated                                   :  1, // [30:30]
                      decrypt_status_code                                     :  3, // [29:27]
                      rxdma1_destination_ring                                 :  3, // [26:24]
                      rxdma0_destination_ring                                 :  3, // [23:21]
                      msdu_length_err                                         :  1, // [20:20]
                      fcs_err                                                 :  1, // [19:19]
                      pn_fields_contain_valid_info                            :  1, // [18:18]
                      unencrypted_frame_err                                   :  1, // [17:17]
                      decrypt_err                                             :  1, // [16:16]
                      tkip_mic_err                                            :  1, // [15:15]
                      mpdu_length_err                                         :  1, // [14:14]
                      overflow_err                                            :  1, // [13:13]
                      rx_in_tx_decrypt_byp                                    :  1, // [12:12]
                      unsup_ktype_short_frame                                 :  1, // [11:11]
                      reserved_1a                                             : 11; // [10:0]
             uint32_t rxpcu_mgmt_sequence_nr                                  : 16, // [31:16]
                      rxpcu_mgmt_sequence_nr_valid                            :  1, // [15:15]
                      reserved_2a                                             : 15; // [14:0]
             uint32_t rxframe_assert_mlo_timestamp                            : 32; // [31:0]
#endif
};


/* Description		RXPCU_MPDU_FILTER_IN_CATEGORY

			Field indicates what the reason was that this MPDU frame
			 was allowed to come into the receive path by RXPCU
			<enum 0 rxpcu_filter_pass> This MPDU passed the normal frame
			 filter programming of rxpcu
			<enum 1 rxpcu_monitor_client> This MPDU did NOT pass the
			 regular frame filter and would have been dropped, were 
			it not for the frame fitting into the 'monitor_client' category.
			
			<enum 2 rxpcu_monitor_other> This MPDU did NOT pass the 
			regular frame filter and also did not pass the rxpcu_monitor_client
			 filter. It would have been dropped accept that it did pass
			 the 'monitor_other' category.
			<enum 3 rxpcu_filter_pass_monitor_ovrd> This MPDU passed
			 the normal frame filter programming of RXPCU but additionally
			 fit into the 'monitor_override_client' category.
			<legal 0-3>
*/

#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_OFFSET                            0x0000000000000000
#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_LSB                               0
#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MSB                               1
#define RX_MPDU_END_RXPCU_MPDU_FILTER_IN_CATEGORY_MASK                              0x0000000000000003


/* Description		SW_FRAME_GROUP_ID

			SW processes frames based on certain classifications. This
			 field indicates to what sw classification this MPDU is 
			mapped.
			The classification is given in priority order
			
			<enum 0 sw_frame_group_NDP_frame> 
			
			<enum 1 sw_frame_group_Multicast_data> 
			<enum 2 sw_frame_group_Unicast_data> 
			<enum 3 sw_frame_group_Null_data > This includes mpdus of
			 type Data Null.
			<enum 38 sw_frame_group_QoS_Null_data> This includes QoS
			 Null frames except in UL MU or TB PPDUs.
			<enum 39 sw_frame_group_QoS_Null_data_TB> This includes 
			QoS Null frames in UL MU or TB PPDUs.
			
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
			
			<enum 37 sw_frame_group_phy_error> PHY reported an error
			
			
			<legal 0-39>
*/

#define RX_MPDU_END_SW_FRAME_GROUP_ID_OFFSET                                        0x0000000000000000
#define RX_MPDU_END_SW_FRAME_GROUP_ID_LSB                                           2
#define RX_MPDU_END_SW_FRAME_GROUP_ID_MSB                                           8
#define RX_MPDU_END_SW_FRAME_GROUP_ID_MASK                                          0x00000000000001fc


/* Description		RESERVED_0

			<legal 0>
*/

#define RX_MPDU_END_RESERVED_0_OFFSET                                               0x0000000000000000
#define RX_MPDU_END_RESERVED_0_LSB                                                  9
#define RX_MPDU_END_RESERVED_0_MSB                                                  15
#define RX_MPDU_END_RESERVED_0_MASK                                                 0x000000000000fe00


/* Description		PHY_PPDU_ID

			A ppdu counter value that PHY increments for every PPDU 
			received. The counter value wraps around  
			<legal all>
*/

#define RX_MPDU_END_PHY_PPDU_ID_OFFSET                                              0x0000000000000000
#define RX_MPDU_END_PHY_PPDU_ID_LSB                                                 16
#define RX_MPDU_END_PHY_PPDU_ID_MSB                                                 31
#define RX_MPDU_END_PHY_PPDU_ID_MASK                                                0x00000000ffff0000


/* Description		RESERVED_1A

			<legal 0>
*/

#define RX_MPDU_END_RESERVED_1A_OFFSET                                              0x0000000000000000
#define RX_MPDU_END_RESERVED_1A_LSB                                                 32
#define RX_MPDU_END_RESERVED_1A_MSB                                                 42
#define RX_MPDU_END_RESERVED_1A_MASK                                                0x000007ff00000000


/* Description		UNSUP_KTYPE_SHORT_FRAME

			This bit will be '1' when WEP or TKIP or WAPI key type is
			 received for 11ah short frame.  Crypto will bypass the 
			received packet without decryption to RxOLE after setting
			 this bit.
*/

#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_OFFSET                                  0x0000000000000000
#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_LSB                                     43
#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_MSB                                     43
#define RX_MPDU_END_UNSUP_KTYPE_SHORT_FRAME_MASK                                    0x0000080000000000


/* Description		RX_IN_TX_DECRYPT_BYP

			Indicates that RX packet is not decrypted as Crypto is busy
			 with TX packet processing.
*/

#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_OFFSET                                     0x0000000000000000
#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_LSB                                        44
#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_MSB                                        44
#define RX_MPDU_END_RX_IN_TX_DECRYPT_BYP_MASK                                       0x0000100000000000


/* Description		OVERFLOW_ERR

			RXPCU Receive FIFO ran out of space to receive the full 
			MPDU. Therefor this MPDU is terminated early and is thus
			 corrupted.  
			
			This MPDU will not be ACKed.
			RXPCU might still be able to correctly receive the following
			 MPDUs in the PPDU if enough fifo space became available
			 in time
*/

#define RX_MPDU_END_OVERFLOW_ERR_OFFSET                                             0x0000000000000000
#define RX_MPDU_END_OVERFLOW_ERR_LSB                                                45
#define RX_MPDU_END_OVERFLOW_ERR_MSB                                                45
#define RX_MPDU_END_OVERFLOW_ERR_MASK                                               0x0000200000000000


/* Description		MPDU_LENGTH_ERR

			Set by RXPCU if the expected MPDU length does not correspond
			 with the actually received number of bytes in the MPDU.
			
*/

#define RX_MPDU_END_MPDU_LENGTH_ERR_OFFSET                                          0x0000000000000000
#define RX_MPDU_END_MPDU_LENGTH_ERR_LSB                                             46
#define RX_MPDU_END_MPDU_LENGTH_ERR_MSB                                             46
#define RX_MPDU_END_MPDU_LENGTH_ERR_MASK                                            0x0000400000000000


/* Description		TKIP_MIC_ERR

			Set by RX CRYPTO when CRYPTO detected a TKIP MIC error for
			 this MPDU
*/

#define RX_MPDU_END_TKIP_MIC_ERR_OFFSET                                             0x0000000000000000
#define RX_MPDU_END_TKIP_MIC_ERR_LSB                                                47
#define RX_MPDU_END_TKIP_MIC_ERR_MSB                                                47
#define RX_MPDU_END_TKIP_MIC_ERR_MASK                                               0x0000800000000000


/* Description		DECRYPT_ERR

			Set by RX CRYPTO when CRYPTO detected a decrypt error for
			 this MPDU or CRYPTO received an encrypted frame, but did
			 not get a valid corresponding key id in the peer entry.
			
*/

#define RX_MPDU_END_DECRYPT_ERR_OFFSET                                              0x0000000000000000
#define RX_MPDU_END_DECRYPT_ERR_LSB                                                 48
#define RX_MPDU_END_DECRYPT_ERR_MSB                                                 48
#define RX_MPDU_END_DECRYPT_ERR_MASK                                                0x0001000000000000


/* Description		UNENCRYPTED_FRAME_ERR

			Set by RX CRYPTO when CRYPTO detected an unencrypted frame
			 while in the peer entry field 'All_frames_shall_be_encrypted' 
			is set.
*/

#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_OFFSET                                    0x0000000000000000
#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_LSB                                       49
#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_MSB                                       49
#define RX_MPDU_END_UNENCRYPTED_FRAME_ERR_MASK                                      0x0002000000000000


/* Description		PN_FIELDS_CONTAIN_VALID_INFO

			Set by RX CRYPTO to indicate that there is a valid PN field
			 present in this MPDU
*/

#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET                             0x0000000000000000
#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_LSB                                50
#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_MSB                                50
#define RX_MPDU_END_PN_FIELDS_CONTAIN_VALID_INFO_MASK                               0x0004000000000000


/* Description		FCS_ERR

			Set by RXPCU when there is an FCS error detected for this
			 MPDU
			NOTE that when this field is set, all other (error) field
			 settings should be ignored as modules could have made wrong
			 decisions based on the corrupted data.
*/

#define RX_MPDU_END_FCS_ERR_OFFSET                                                  0x0000000000000000
#define RX_MPDU_END_FCS_ERR_LSB                                                     51
#define RX_MPDU_END_FCS_ERR_MSB                                                     51
#define RX_MPDU_END_FCS_ERR_MASK                                                    0x0008000000000000


/* Description		MSDU_LENGTH_ERR

			Set by RXOLE when there is an msdu length error detected
			 in at least 1 of the MSDUs embedded within the MPDU
*/

#define RX_MPDU_END_MSDU_LENGTH_ERR_OFFSET                                          0x0000000000000000
#define RX_MPDU_END_MSDU_LENGTH_ERR_LSB                                             52
#define RX_MPDU_END_MSDU_LENGTH_ERR_MSB                                             52
#define RX_MPDU_END_MSDU_LENGTH_ERR_MASK                                            0x0010000000000000


/* Description		RXDMA0_DESTINATION_RING

			The ring to which RXDMA0 shall push the frame, assuming 
			no MPDU level errors are detected. In case of MPDU level
			 errors, RXDMA0 might change the RXDMA0 destination
			
			<enum 0  rxdma_release_ring >  RXDMA0 shall push the frame
			 to the Release ring. Effectively this means the frame needs
			 to be dropped.
			
			<enum 1  rxdma2fw_pmac0_ring >  RXDMA0 shall push the frame
			 to the FW ring for PMAC0.
			
			<enum 2  rxdma2sw_ring >  RXDMA0 shall push the frame to
			 the SW ring
			
			<enum 3  rxdma2reo_ring >  RXDMA0 shall push the frame to
			 the REO entrance ring
			
			<enum 4  rxdma2fw_pmac1_ring> RXDMA0 shall push the frame
			 to the FW ring for PMAC1.
			
			<enum 5 rxdma2reo_remote0_ring> RXDMA0 shall push the frame
			 to the first MLO REO entrance ring.
			
			<enum 6 rxdma2reo_remote1_ring> RXDMA0 shall push the frame
			 to the second MLO REO entrance ring.
			
			<legal 0 - 6>
*/

#define RX_MPDU_END_RXDMA0_DESTINATION_RING_OFFSET                                  0x0000000000000000
#define RX_MPDU_END_RXDMA0_DESTINATION_RING_LSB                                     53
#define RX_MPDU_END_RXDMA0_DESTINATION_RING_MSB                                     55
#define RX_MPDU_END_RXDMA0_DESTINATION_RING_MASK                                    0x00e0000000000000


/* Description		RXDMA1_DESTINATION_RING

			The ring to which RXDMA1 shall push the frame, assuming 
			no MPDU level errors are detected. In case of MPDU level
			 errors, RXDMA1 might change the RXDMA destination
			
			<enum 0  rxdma_release_ring >  DO NOT USE.
			
			<enum 1  rxdma2fw_pmac0_ring >  DO NOT USE. 
			
			<enum 2  rxdma2sw_ring >  RXDMA1 shall push the frame to
			 the SW ring 
			
			<enum 3  rxdma2reo_ring >  DO NOT USE.
			
			<enum 4  rxdma2fw_pmac1_ring> DO NOT USE.
			
			<enum 5 rxdma2reo_remote0_ring> DO NOT USE.
			
			<enum 6 rxdma2reo_remote1_ring> DO NOT USE.
			
			<legal 0 - 6>
*/

#define RX_MPDU_END_RXDMA1_DESTINATION_RING_OFFSET                                  0x0000000000000000
#define RX_MPDU_END_RXDMA1_DESTINATION_RING_LSB                                     56
#define RX_MPDU_END_RXDMA1_DESTINATION_RING_MSB                                     58
#define RX_MPDU_END_RXDMA1_DESTINATION_RING_MASK                                    0x0700000000000000


/* Description		DECRYPT_STATUS_CODE

			Field provides insight into the decryption performed
			
			<enum 0 decrypt_ok> Frame had protection enabled and decrypted
			 properly 
			<enum 1 decrypt_unprotected_frame > Frame is unprotected
			 and hence bypassed 
			<enum 2 decrypt_data_err > Frame has protection enabled 
			and could not be properly decrypted due to MIC/ICV mismatch
			 etc. 
			<enum 3 decrypt_key_invalid > Frame has protection enabled
			 but the key that was required to decrypt this frame was
			 not valid 
			<enum 4 decrypt_peer_entry_invalid > Frame has protection
			 enabled but the key that was required to decrypt this frame
			 was not valid
			<enum 5 decrypt_other > Reserved for other indications
			
			<legal 0 - 5>
*/

#define RX_MPDU_END_DECRYPT_STATUS_CODE_OFFSET                                      0x0000000000000000
#define RX_MPDU_END_DECRYPT_STATUS_CODE_LSB                                         59
#define RX_MPDU_END_DECRYPT_STATUS_CODE_MSB                                         61
#define RX_MPDU_END_DECRYPT_STATUS_CODE_MASK                                        0x3800000000000000


/* Description		RX_BITMAP_NOT_UPDATED

			Frame is received, but RXPCU could not update the receive
			 bitmap due to (temporary) fifo contraints.
			<legal all>
*/

#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_OFFSET                                    0x0000000000000000
#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_LSB                                       62
#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_MSB                                       62
#define RX_MPDU_END_RX_BITMAP_NOT_UPDATED_MASK                                      0x4000000000000000


/* Description		RESERVED_1B

			<legal 0>
*/

#define RX_MPDU_END_RESERVED_1B_OFFSET                                              0x0000000000000000
#define RX_MPDU_END_RESERVED_1B_LSB                                                 63
#define RX_MPDU_END_RESERVED_1B_MSB                                                 63
#define RX_MPDU_END_RESERVED_1B_MASK                                                0x8000000000000000


/* Description		RESERVED_2A

			<legal 0>
*/

#define RX_MPDU_END_RESERVED_2A_OFFSET                                              0x0000000000000008
#define RX_MPDU_END_RESERVED_2A_LSB                                                 0
#define RX_MPDU_END_RESERVED_2A_MSB                                                 14
#define RX_MPDU_END_RESERVED_2A_MASK                                                0x0000000000007fff


/* Description		RXPCU_MGMT_SEQUENCE_NR_VALID

			This field gets set by RXPCU when the received management
			 frame is destined to this device, passes FCS and is categorized
			 as one for which RXPCU should assign a rxpcu_mgmt_sequence_number. 
			After assigning a number, the RXPCU will increment the sequence
			 number for the next management frame that meets these criteria.
			
			 
			<legal all>
*/

#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_OFFSET                             0x0000000000000008
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_LSB                                15
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_MSB                                15
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_VALID_MASK                               0x0000000000008000


/* Description		RXPCU_MGMT_SEQUENCE_NR

			Field only valid when rxpcu_mgmt_sequence_nr_valid is set
			
			
			This RXPCU generated sequence number is assigned to this
			 management frame. It is used by FW and host SW for management
			 frame reordering across multiple bands/links.
			
			<legal all>
*/

#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_OFFSET                                   0x0000000000000008
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_LSB                                      16
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_MSB                                      31
#define RX_MPDU_END_RXPCU_MGMT_SEQUENCE_NR_MASK                                     0x00000000ffff0000


/* Description		RXFRAME_ASSERT_MLO_TIMESTAMP

			'mlo_global_timestamp' that indicates when for the PPDU 
			that contained this MPDU, the 'rx_frame' signal got asserted.
			
			 
			This field is always valid, irrespective of the frame being
			 related to MLO reception or not. It is used by FW and host
			 SW for management frame reordering purposes.
			
			<legal all>
*/

#define RX_MPDU_END_RXFRAME_ASSERT_MLO_TIMESTAMP_OFFSET                             0x0000000000000008
#define RX_MPDU_END_RXFRAME_ASSERT_MLO_TIMESTAMP_LSB                                32
#define RX_MPDU_END_RXFRAME_ASSERT_MLO_TIMESTAMP_MSB                                63
#define RX_MPDU_END_RXFRAME_ASSERT_MLO_TIMESTAMP_MASK                               0xffffffff00000000



#endif   // RX_MPDU_END

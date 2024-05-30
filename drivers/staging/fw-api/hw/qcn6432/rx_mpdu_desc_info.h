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

#ifndef _RX_MPDU_DESC_INFO_H_
#define _RX_MPDU_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MPDU_DESC_INFO 2


struct rx_mpdu_desc_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t msdu_count                                              :  8, // [7:0]
                      fragment_flag                                           :  1, // [8:8]
                      mpdu_retry_bit                                          :  1, // [9:9]
                      ampdu_flag                                              :  1, // [10:10]
                      bar_frame                                               :  1, // [11:11]
                      pn_fields_contain_valid_info                            :  1, // [12:12]
                      raw_mpdu                                                :  1, // [13:13]
                      more_fragment_flag                                      :  1, // [14:14]
                      src_info                                                : 12, // [26:15]
                      mpdu_qos_control_valid                                  :  1, // [27:27]
                      tid                                                     :  4; // [31:28]
             uint32_t peer_meta_data                                          : 32; // [31:0]
#else
             uint32_t tid                                                     :  4, // [31:28]
                      mpdu_qos_control_valid                                  :  1, // [27:27]
                      src_info                                                : 12, // [26:15]
                      more_fragment_flag                                      :  1, // [14:14]
                      raw_mpdu                                                :  1, // [13:13]
                      pn_fields_contain_valid_info                            :  1, // [12:12]
                      bar_frame                                               :  1, // [11:11]
                      ampdu_flag                                              :  1, // [10:10]
                      mpdu_retry_bit                                          :  1, // [9:9]
                      fragment_flag                                           :  1, // [8:8]
                      msdu_count                                              :  8; // [7:0]
             uint32_t peer_meta_data                                          : 32; // [31:0]
#endif
};


/* Description		MSDU_COUNT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The number of MSDUs within the MPDU 
			<legal all>
*/

#define RX_MPDU_DESC_INFO_MSDU_COUNT_OFFSET                                         0x00000000
#define RX_MPDU_DESC_INFO_MSDU_COUNT_LSB                                            0
#define RX_MPDU_DESC_INFO_MSDU_COUNT_MSB                                            7
#define RX_MPDU_DESC_INFO_MSDU_COUNT_MASK                                           0x000000ff


/* Description		FRAGMENT_FLAG

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, this MPDU is a fragment and REO should forward
			 this fragment MPDU to the REO destination ring without 
			any reorder checks, pn checks or bitmap update. This implies
			 that REO is forwarding the pointer to the MSDU link descriptor. 
			The destination ring is coming from a programmable register
			 setting in REO
			
			<legal all>
*/

#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_OFFSET                                      0x00000000
#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_LSB                                         8
#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_MSB                                         8
#define RX_MPDU_DESC_INFO_FRAGMENT_FLAG_MASK                                        0x00000100


/* Description		MPDU_RETRY_BIT

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			The retry bit setting from the MPDU header of the received
			 frame
			<legal all>
*/

#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_OFFSET                                     0x00000000
#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_LSB                                        9
#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_MSB                                        9
#define RX_MPDU_DESC_INFO_MPDU_RETRY_BIT_MASK                                       0x00000200


/* Description		AMPDU_FLAG

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the MPDU was received as part of an A-MPDU.
			<legal all>
*/

#define RX_MPDU_DESC_INFO_AMPDU_FLAG_OFFSET                                         0x00000000
#define RX_MPDU_DESC_INFO_AMPDU_FLAG_LSB                                            10
#define RX_MPDU_DESC_INFO_AMPDU_FLAG_MSB                                            10
#define RX_MPDU_DESC_INFO_AMPDU_FLAG_MASK                                           0x00000400


/* Description		BAR_FRAME

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			When set, the received frame is a BAR frame. After processing, 
			this frame shall be pushed to SW or deleted.
			<legal all>
*/

#define RX_MPDU_DESC_INFO_BAR_FRAME_OFFSET                                          0x00000000
#define RX_MPDU_DESC_INFO_BAR_FRAME_LSB                                             11
#define RX_MPDU_DESC_INFO_BAR_FRAME_MSB                                             11
#define RX_MPDU_DESC_INFO_BAR_FRAME_MASK                                            0x00000800


/* Description		PN_FIELDS_CONTAIN_VALID_INFO

			Consumer: REO/SW/FW
			Producer: RXDMA
			
			Copied here by RXDMA from RX_MPDU_END
			When not set, REO will Not perform a PN sequence number 
			check
*/

#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_OFFSET                       0x00000000
#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_LSB                          12
#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_MSB                          12
#define RX_MPDU_DESC_INFO_PN_FIELDS_CONTAIN_VALID_INFO_MASK                         0x00001000


/* Description		RAW_MPDU

			Field only valid when first_msdu_in_mpdu_flag is set.
			
			When set, the contents in the MSDU buffer contains a 'RAW' 
			MPDU. This 'RAW' MPDU might be spread out over multiple 
			MSDU buffers.
			<legal all>
*/

#define RX_MPDU_DESC_INFO_RAW_MPDU_OFFSET                                           0x00000000
#define RX_MPDU_DESC_INFO_RAW_MPDU_LSB                                              13
#define RX_MPDU_DESC_INFO_RAW_MPDU_MSB                                              13
#define RX_MPDU_DESC_INFO_RAW_MPDU_MASK                                             0x00002000


/* Description		MORE_FRAGMENT_FLAG

			The More Fragment bit setting from the MPDU header of the
			 received frame
			
			<legal all>
*/

#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_OFFSET                                 0x00000000
#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_LSB                                    14
#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_MSB                                    14
#define RX_MPDU_DESC_INFO_MORE_FRAGMENT_FLAG_MASK                                   0x00004000


/* Description		SRC_INFO

			Source (virtual) device/interface info. associated with 
			this peer
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MPDU_DESC_INFO_SRC_INFO_OFFSET                                           0x00000000
#define RX_MPDU_DESC_INFO_SRC_INFO_LSB                                              15
#define RX_MPDU_DESC_INFO_SRC_INFO_MSB                                              26
#define RX_MPDU_DESC_INFO_SRC_INFO_MASK                                             0x07ff8000


/* Description		MPDU_QOS_CONTROL_VALID

			When set, the MPDU has a QoS control field.
			
			In case of ndp or phy_err, this field will never be set.
			
			<legal all>
*/

#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_OFFSET                             0x00000000
#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_LSB                                27
#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_MSB                                27
#define RX_MPDU_DESC_INFO_MPDU_QOS_CONTROL_VALID_MASK                               0x08000000


/* Description		TID

			Field only valid when mpdu_qos_control_valid is set
			
			The TID field in the QoS control field
			<legal all>
*/

#define RX_MPDU_DESC_INFO_TID_OFFSET                                                0x00000000
#define RX_MPDU_DESC_INFO_TID_LSB                                                   28
#define RX_MPDU_DESC_INFO_TID_MSB                                                   31
#define RX_MPDU_DESC_INFO_TID_MASK                                                  0xf0000000


/* Description		PEER_META_DATA

			Meta data that SW has programmed in the Peer table entry
			 of the transmitting STA.
			<legal all>
*/

#define RX_MPDU_DESC_INFO_PEER_META_DATA_OFFSET                                     0x00000004
#define RX_MPDU_DESC_INFO_PEER_META_DATA_LSB                                        0
#define RX_MPDU_DESC_INFO_PEER_META_DATA_MSB                                        31
#define RX_MPDU_DESC_INFO_PEER_META_DATA_MASK                                       0xffffffff



#endif   // RX_MPDU_DESC_INFO

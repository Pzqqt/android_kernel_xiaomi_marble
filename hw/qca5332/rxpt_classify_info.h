
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

 
 
 
 
 
 
 


#ifndef _RXPT_CLASSIFY_INFO_H_
#define _RXPT_CLASSIFY_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RXPT_CLASSIFY_INFO 1


struct rxpt_classify_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reo_destination_indication                              :  5, // [4:0]
                      lmac_peer_id_msb                                        :  2, // [6:5]
                      use_flow_id_toeplitz_clfy                               :  1, // [7:7]
                      pkt_selection_fp_ucast_data                             :  1, // [8:8]
                      pkt_selection_fp_mcast_data                             :  1, // [9:9]
                      pkt_selection_fp_1000                                   :  1, // [10:10]
                      rxdma0_source_ring_selection                            :  3, // [13:11]
                      rxdma0_destination_ring_selection                       :  3, // [16:14]
                      mcast_echo_drop_enable                                  :  1, // [17:17]
                      wds_learning_detect_en                                  :  1, // [18:18]
                      intrabss_check_en                                       :  1, // [19:19]
                      use_ppe                                                 :  1, // [20:20]
                      ppe_routing_enable                                      :  1, // [21:21]
                      reserved_0b                                             : 10; // [31:22]
#else
             uint32_t reserved_0b                                             : 10, // [31:22]
                      ppe_routing_enable                                      :  1, // [21:21]
                      use_ppe                                                 :  1, // [20:20]
                      intrabss_check_en                                       :  1, // [19:19]
                      wds_learning_detect_en                                  :  1, // [18:18]
                      mcast_echo_drop_enable                                  :  1, // [17:17]
                      rxdma0_destination_ring_selection                       :  3, // [16:14]
                      rxdma0_source_ring_selection                            :  3, // [13:11]
                      pkt_selection_fp_1000                                   :  1, // [10:10]
                      pkt_selection_fp_mcast_data                             :  1, // [9:9]
                      pkt_selection_fp_ucast_data                             :  1, // [8:8]
                      use_flow_id_toeplitz_clfy                               :  1, // [7:7]
                      lmac_peer_id_msb                                        :  2, // [6:5]
                      reo_destination_indication                              :  5; // [4:0]
#endif
};


/* Description		REO_DESTINATION_INDICATION

			The ID of the REO exit ring where the MSDU frame shall push
			 after (MPDU level) reordering has finished.
			
			<enum 0 reo_destination_sw0> Reo will push the frame into
			 the REO2SW0 ring
			<enum 1 reo_destination_sw1> Reo will push the frame into
			 the REO2SW1 ring
			<enum 2 reo_destination_sw2> Reo will push the frame into
			 the REO2SW2 ring
			<enum 3 reo_destination_sw3> Reo will push the frame into
			 the REO2SW3 ring
			<enum 4 reo_destination_sw4> Reo will push the frame into
			 the REO2SW4 ring
			<enum 5 reo_destination_release> Reo will push the frame
			 into the REO_release ring
			<enum 6 reo_destination_fw> Reo will push the frame into
			 the REO2FW ring
			<enum 7 reo_destination_sw5> Reo will push the frame into
			 the REO2SW5 ring (REO remaps this in chips without REO2SW5
			 ring, e.g. Pine)
			<enum 8 reo_destination_sw6> Reo will push the frame into
			 the REO2SW6 ring (REO remaps this in chips without REO2SW6
			 ring, e.g. Pine)
			<enum 9 reo_destination_sw7> Reo will push the frame into
			 the REO2SW7 ring (REO remaps this in chips without REO2SW7
			 ring)
			<enum 10 reo_destination_sw8> Reo will push the frame into
			 the REO2SW8 ring (REO remaps this in chips without REO2SW8
			 ring)
			<enum 11 reo_destination_11> REO remaps this 
			<enum 12 reo_destination_12> REO remaps this <enum 13 reo_destination_13> 
			REO remaps this 
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

#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_OFFSET                        0x00000000
#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_LSB                           0
#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_MSB                           4
#define RXPT_CLASSIFY_INFO_REO_DESTINATION_INDICATION_MASK                          0x0000001f


/* Description		LMAC_PEER_ID_MSB

			If use_flow_id_toeplitz_clfy is set and lmac_peer_id_'sb
			 is 2'b00, Rx OLE uses a REO desination indicati'n of {1'b1, 
			hash[3:0]} using the chosen Toeplitz hash from Common Parser
			 if flow search fails.
			If use_flow_id_toeplitz_clfy is set and lmac_peer_id_msb
			 's not 2'b00, Rx OLE uses a REO desination indication of
			 {lmac_peer_id_msb, hash[2:0]} using the chosen Toeplitz
			 hash from Common Parser if flow search fails.
			This LMAC/peer-based routing is not supported in Hastings80
			 and HastingsPrime.
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_OFFSET                                  0x00000000
#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_LSB                                     5
#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_MSB                                     6
#define RXPT_CLASSIFY_INFO_LMAC_PEER_ID_MSB_MASK                                    0x00000060


/* Description		USE_FLOW_ID_TOEPLITZ_CLFY

			Indication to Rx OLE to enable REO destination routing based
			 on the chosen Toeplitz hash from Common Parser, in case
			 flow search fails
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET                         0x00000000
#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_LSB                            7
#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_MSB                            7
#define RXPT_CLASSIFY_INFO_USE_FLOW_ID_TOEPLITZ_CLFY_MASK                           0x00000080


/* Description		PKT_SELECTION_FP_UCAST_DATA

			Filter pass Unicast data frame (matching rxpcu_filter_pass
			 and sw_frame_group_Unicast_data) routing selection
			TODO: What about 'rxpcu_filter_pass_monior_ovrd'?
			
			1'b0: source and destination rings are selected from the
			 RxOLE register settings for the packet type
			
			1'b1: source ring and destination ring is selected from 
			the rxdma0_source_ring_selection and rxdma0_destination_ring_selection
			 fields in this STRUCT
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_OFFSET                       0x00000000
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_LSB                          8
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_MSB                          8
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_UCAST_DATA_MASK                         0x00000100


/* Description		PKT_SELECTION_FP_MCAST_DATA

			Filter pass Multicast data frame (matching rxpcu_filter_pass
			 and sw_frame_group_Multicast_data) routing selection
			TODO: What about 'rxpcu_filter_pass_monior_ovrd'?
			
			1'b0: source and destination rings are selected from the
			 RxOLE register settings for the packet type
			
			1'b1: source ring and destination ring is selected from 
			the rxdma0_source_ring_selection and rxdma0_destination_ring_selection
			 fields in this STRUCT
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_OFFSET                       0x00000000
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_LSB                          9
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_MSB                          9
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_MCAST_DATA_MASK                         0x00000200


/* Description		PKT_SELECTION_FP_1000

			Filter pass BAR frame (matching rxpcu_filter_pass and sw_frame_group_ctrl_1000) 
			routing selection
			TODO: What about 'rxpcu_filter_pass_monior_ovrd'?
			
			1'b0: source and destination rings are selected from the
			 RxOLE register settings for the packet type
			
			1'b1: source ring and destination ring is selected from 
			the rxdma0_source_ring_selection and rxdma0_destination_ring_selection
			 fields in this STRUCT
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_OFFSET                             0x00000000
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_LSB                                10
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_MSB                                10
#define RXPT_CLASSIFY_INFO_PKT_SELECTION_FP_1000_MASK                               0x00000400


/* Description		RXDMA0_SOURCE_RING_SELECTION

			Field only valid when for the received frame type the corresponding
			 pkt_selection_fp_... bit is set
			
			<enum 0 sw2rxdma0_0_buf_source_ring> The data buffer for
			 this frame shall be sourced by sw2rxdma0 buffer source 
			ring.
			<enum 1 fw2rxdma0_pmac0_buf_source_ring> The data buffer
			 for this frame shall be sourced by fw2rxdma buffer source
			 ring for PMAC0.
			<enum 2 sw2rxdma0_1_buf_source_ring> The data buffer for
			 this frame shall be sourced by sw2rxdma1 buffer source 
			ring.
			<enum 3 no_buffer_rxdma0_ring> The frame shall not be written
			 to any data buffer.
			<enum 4 sw2rxdma0_exception_buf_source_ring> The data buffer
			 for this frame shall be sourced by sw2rxdma_exception buffer
			 source ring.
			<enum 5 fw2rxdma0_pmac1_buf_source_ring> The data buffer
			 for this frame shall be sourced by fw2rxdma buffer source
			 ring for PMAC1.
			
			<legal 0-5>
*/

#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_OFFSET                      0x00000000
#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_LSB                         11
#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_MSB                         13
#define RXPT_CLASSIFY_INFO_RXDMA0_SOURCE_RING_SELECTION_MASK                        0x00003800


/* Description		RXDMA0_DESTINATION_RING_SELECTION

			Field only valid when for the received frame type the corresponding
			 pkt_selection_fp_... bit is set
			
			<enum 0  rxdma_release_ring> RXDMA0 shall push the frame
			 to the Release ring. Effectively this means the frame needs
			 to be dropped.
			<enum 1  rxdma2fw_pmac0_ring> RXDMA0 shall push the frame
			 to the FW ring for PMAC0.
			<enum 2  rxdma2sw_ring> RXDMA0 shall push the frame to the
			 SW ring.
			<enum 3  rxdma2reo_ring> RXDMA0 shall push the frame to 
			the REO entrance ring.
			<enum 4  rxdma2fw_pmac1_ring> RXDMA0 shall push the frame
			 to the FW ring for PMAC1.
			<enum 5 rxdma2reo_remote0_ring> RXDMA0 shall push the frame
			 to the first MLO REO entrance ring.
			<enum 6 rxdma2reo_remote1_ring> RXDMA0 shall push the frame
			 to the second MLO REO entrance ring.
			
			<legal 0-6>
*/

#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_OFFSET                 0x00000000
#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_LSB                    14
#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_MSB                    16
#define RXPT_CLASSIFY_INFO_RXDMA0_DESTINATION_RING_SELECTION_MASK                   0x0001c000


/* Description		MCAST_ECHO_DROP_ENABLE

			If set, for multicast packets, multicast echo check (i.e. 
			SA search with mcast_echo_check = 1) shall be performed 
			by RXOLE, and any multicast echo packets should be indicated
			 to RXDMA for release to WBM
			
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_OFFSET                            0x00000000
#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_LSB                               17
#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_MSB                               17
#define RXPT_CLASSIFY_INFO_MCAST_ECHO_DROP_ENABLE_MASK                              0x00020000


/* Description		WDS_LEARNING_DETECT_EN

			If set, WDS learning detection based on SA search and notification
			 to FW (using RXDMA0 status ring) is enabled and the "timestamp" 
			field in address search failure cache-only entry should 
			be used to avoid multiple WDS learning notifications.
			
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_OFFSET                            0x00000000
#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_LSB                               18
#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_MSB                               18
#define RXPT_CLASSIFY_INFO_WDS_LEARNING_DETECT_EN_MASK                              0x00040000


/* Description		INTRABSS_CHECK_EN

			If set, intra-BSS routing detection is enabled
			
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_OFFSET                                 0x00000000
#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_LSB                                    19
#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_MSB                                    19
#define RXPT_CLASSIFY_INFO_INTRABSS_CHECK_EN_MASK                                   0x00080000


/* Description		USE_PPE

			Indicates to RXDMA to ignore the REO_destination_indication
			 and use a programmed value corresponding to the REO2PPE
			 ring
			
			This override to REO2PPE for packets requiring multiple 
			buffers shall be disabled based on an RXDMA configuration, 
			as PPE may not support such packets.
			
			Supported only in full AP chips like Waikiki, not in client/soft
			 AP chips like Hamilton
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_USE_PPE_OFFSET                                           0x00000000
#define RXPT_CLASSIFY_INFO_USE_PPE_LSB                                              20
#define RXPT_CLASSIFY_INFO_USE_PPE_MSB                                              20
#define RXPT_CLASSIFY_INFO_USE_PPE_MASK                                             0x00100000


/* Description		PPE_ROUTING_ENABLE

			Global enable/disable bit for routing to PPE, used to disable
			 PPE routing even if RXOLE CCE or flow search indicate 'Use_PPE'
			
			
			This is set by SW for peers which are being handled by a
			 host SW/accelerator subsystem that also handles packet 
			buffer management for WiFi-to-PPE routing.
			
			This is cleared by SW for peers which are being handled 
			by a different subsystem, completely disabling WiFi-to-PPE
			 routing for such peers.
			
			<legal all>
*/

#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_OFFSET                                0x00000000
#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_LSB                                   21
#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_MSB                                   21
#define RXPT_CLASSIFY_INFO_PPE_ROUTING_ENABLE_MASK                                  0x00200000


/* Description		RESERVED_0B

			<legal 0>
*/

#define RXPT_CLASSIFY_INFO_RESERVED_0B_OFFSET                                       0x00000000
#define RXPT_CLASSIFY_INFO_RESERVED_0B_LSB                                          22
#define RXPT_CLASSIFY_INFO_RESERVED_0B_MSB                                          31
#define RXPT_CLASSIFY_INFO_RESERVED_0B_MASK                                         0xffc00000



#endif   // RXPT_CLASSIFY_INFO

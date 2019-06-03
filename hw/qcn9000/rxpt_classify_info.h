/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	reo_destination_indication[4:0], reserved_0a[6:5], use_flow_id_toeplitz_clfy[7], pkt_selection_fp_ucast_data[8], pkt_selection_fp_mcast_data[9], pkt_selection_fp_1000[10], rxdma0_source_ring_selection[12:11], rxdma0_destination_ring_selection[14:13], reserved_0b[31:15]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RXPT_CLASSIFY_INFO 1

struct rxpt_classify_info {
             uint32_t reo_destination_indication      :  5, //[4:0]
                      reserved_0a                     :  2, //[6:5]
                      use_flow_id_toeplitz_clfy       :  1, //[7]
                      pkt_selection_fp_ucast_data     :  1, //[8]
                      pkt_selection_fp_mcast_data     :  1, //[9]
                      pkt_selection_fp_1000           :  1, //[10]
                      rxdma0_source_ring_selection    :  2, //[12:11]
                      rxdma0_destination_ring_selection:  2, //[14:13]
                      reserved_0b                     : 17; //[31:15]
};

/*

reo_destination_indication
			
			The ID of the REO exit ring where the MSDU frame shall
			push after (MPDU level) reordering has finished.
			
			
			
			<enum 0 reo_destination_tcl> Reo will push the frame
			into the REO2TCL ring
			
			<enum 1 reo_destination_sw1> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 2 reo_destination_sw2> Reo will push the frame
			into the REO2SW2 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW3 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW4 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_sw5> Reo will push the frame
			into the REO2SW5 ring 
			
			<enum 8 reo_destination_sw6> Reo will push the frame
			into the REO2SW6 ring 
			
			<enum 9 reo_destination_9> REO remaps this <enum 10
			reo_destination_10> REO remaps this 
			
			<enum 11 reo_destination_11> REO remaps this 
			
			<enum 12 reo_destination_12> REO remaps this <enum 13
			reo_destination_13> REO remaps this 
			
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

reserved_0a
			
			<legal 0>

use_flow_id_toeplitz_clfy
			
			Indication to Rx OLE to enable classification based on
			the chosen Toeplitz hash from Common Parser, in case flow
			search fails
			
			<legal all>

pkt_selection_fp_ucast_data
			
			Filter pass Unicast data frame (matching
			rxpcu_filter_pass and sw_frame_group_Unicast_data) routing
			selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>

pkt_selection_fp_mcast_data
			
			Filter pass Multicast data frame (matching
			rxpcu_filter_pass and sw_frame_group_Multicast_data) routing
			selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>

pkt_selection_fp_1000
			
			Filter pass BAR frame (matching rxpcu_filter_pass and
			sw_frame_group_ctrl_1000) routing selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>

rxdma0_source_ring_selection
			
			Field only valid when for the received frame type the
			corresponding pkt_selection_fp_... bit is set
			
			
			
			<enum 0 wbm2rxdma_buf_source_ring> The data buffer for
			
			<enum 1 fw2rxdma_buf_source_ring> The data buffer for
			this frame shall be sourced by fw2rxdma buffer source ring.
			
			<enum 2 sw2rxdma_buf_source_ring> The data buffer for
			this frame shall be sourced by sw2rxdma buffer source ring.
			
			<enum 3 no_buffer_ring> The frame shall not be written
			to any data buffer.
			
			
			
			<legal all>

rxdma0_destination_ring_selection
			
			Field only valid when for the received frame type the
			corresponding pkt_selection_fp_... bit is set
			
			
			
			<enum 0  rxdma_release_ring> RXDMA0 shall push the frame
			to the Release ring. Effectively this means the frame needs
			to be dropped.
			
			<enum 1  rxdma2fw_ring> RXDMA0 shall push the frame to
			the FW ring.
			
			<enum 2  rxdma2sw_ring> RXDMA0 shall push the frame to
			the SW ring.
			
			<enum 3  rxdma2reo_ring> RXDMA0 shall push the frame to
			the REO entrance ring.
			
			
			
			<legal all>

reserved_0b
			
			<legal 0>
*/


/* Description		RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION
			
			The ID of the REO exit ring where the MSDU frame shall
			push after (MPDU level) reordering has finished.
			
			
			
			<enum 0 reo_destination_tcl> Reo will push the frame
			into the REO2TCL ring
			
			<enum 1 reo_destination_sw1> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 2 reo_destination_sw2> Reo will push the frame
			into the REO2SW2 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW3 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW4 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_sw5> Reo will push the frame
			into the REO2SW5 ring 
			
			<enum 8 reo_destination_sw6> Reo will push the frame
			into the REO2SW6 ring 
			
			<enum 9 reo_destination_9> REO remaps this <enum 10
			reo_destination_10> REO remaps this 
			
			<enum 11 reo_destination_11> REO remaps this 
			
			<enum 12 reo_destination_12> REO remaps this <enum 13
			reo_destination_13> REO remaps this 
			
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
#define RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION_OFFSET       0x00000000
#define RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION_LSB          0
#define RXPT_CLASSIFY_INFO_0_REO_DESTINATION_INDICATION_MASK         0x0000001f

/* Description		RXPT_CLASSIFY_INFO_0_RESERVED_0A
			
			<legal 0>
*/
#define RXPT_CLASSIFY_INFO_0_RESERVED_0A_OFFSET                      0x00000000
#define RXPT_CLASSIFY_INFO_0_RESERVED_0A_LSB                         5
#define RXPT_CLASSIFY_INFO_0_RESERVED_0A_MASK                        0x00000060

/* Description		RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY
			
			Indication to Rx OLE to enable classification based on
			the chosen Toeplitz hash from Common Parser, in case flow
			search fails
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET        0x00000000
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_LSB           7
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_MASK          0x00000080

/* Description		RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA
			
			Filter pass Unicast data frame (matching
			rxpcu_filter_pass and sw_frame_group_Unicast_data) routing
			selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA_OFFSET      0x00000000
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA_LSB         8
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_UCAST_DATA_MASK        0x00000100

/* Description		RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA
			
			Filter pass Multicast data frame (matching
			rxpcu_filter_pass and sw_frame_group_Multicast_data) routing
			selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA_OFFSET      0x00000000
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA_LSB         9
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_MCAST_DATA_MASK        0x00000200

/* Description		RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000
			
			Filter pass BAR frame (matching rxpcu_filter_pass and
			sw_frame_group_ctrl_1000) routing selection
			
			
			
			1'b0: source and destination rings are selected from the
			RxOLE register settings for the packet type
			
			
			
			1'b1: source ring and destination ring is selected from
			the rxdma0_source_ring_selection and
			rxdma0_destination_ring_selection fields in this STRUCT
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000_OFFSET            0x00000000
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000_LSB               10
#define RXPT_CLASSIFY_INFO_0_PKT_SELECTION_FP_1000_MASK              0x00000400

/* Description		RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION
			
			Field only valid when for the received frame type the
			corresponding pkt_selection_fp_... bit is set
			
			
			
			<enum 0 wbm2rxdma_buf_source_ring> The data buffer for
			
			<enum 1 fw2rxdma_buf_source_ring> The data buffer for
			this frame shall be sourced by fw2rxdma buffer source ring.
			
			<enum 2 sw2rxdma_buf_source_ring> The data buffer for
			this frame shall be sourced by sw2rxdma buffer source ring.
			
			<enum 3 no_buffer_ring> The frame shall not be written
			to any data buffer.
			
			
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION_OFFSET     0x00000000
#define RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION_LSB        11
#define RXPT_CLASSIFY_INFO_0_RXDMA0_SOURCE_RING_SELECTION_MASK       0x00001800

/* Description		RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION
			
			Field only valid when for the received frame type the
			corresponding pkt_selection_fp_... bit is set
			
			
			
			<enum 0  rxdma_release_ring> RXDMA0 shall push the frame
			to the Release ring. Effectively this means the frame needs
			to be dropped.
			
			<enum 1  rxdma2fw_ring> RXDMA0 shall push the frame to
			the FW ring.
			
			<enum 2  rxdma2sw_ring> RXDMA0 shall push the frame to
			the SW ring.
			
			<enum 3  rxdma2reo_ring> RXDMA0 shall push the frame to
			the REO entrance ring.
			
			
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION_OFFSET 0x00000000
#define RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION_LSB   13
#define RXPT_CLASSIFY_INFO_0_RXDMA0_DESTINATION_RING_SELECTION_MASK  0x00006000

/* Description		RXPT_CLASSIFY_INFO_0_RESERVED_0B
			
			<legal 0>
*/
#define RXPT_CLASSIFY_INFO_0_RESERVED_0B_OFFSET                      0x00000000
#define RXPT_CLASSIFY_INFO_0_RESERVED_0B_LSB                         15
#define RXPT_CLASSIFY_INFO_0_RESERVED_0B_MASK                        0xffff8000


#endif // _RXPT_CLASSIFY_INFO_H_

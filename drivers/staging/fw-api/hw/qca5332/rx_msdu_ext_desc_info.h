
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

 
 
 
 
 
 
 


#ifndef _RX_MSDU_EXT_DESC_INFO_H_
#define _RX_MSDU_EXT_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_EXT_DESC_INFO 1


struct rx_msdu_ext_desc_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t reo_destination_indication                              :  5, // [4:0]
                      service_code                                            :  9, // [13:5]
                      priority_valid                                          :  1, // [14:14]
                      data_offset                                             : 12, // [26:15]
                      src_link_id                                             :  3, // [29:27]
                      reserved_0a                                             :  2; // [31:30]
#else
             uint32_t reserved_0a                                             :  2, // [31:30]
                      src_link_id                                             :  3, // [29:27]
                      data_offset                                             : 12, // [26:15]
                      priority_valid                                          :  1, // [14:14]
                      service_code                                            :  9, // [13:5]
                      reo_destination_indication                              :  5; // [4:0]
#endif
};


/* Description		REO_DESTINATION_INDICATION

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
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

#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_OFFSET                     0x00000000
#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_LSB                        0
#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_MSB                        4
#define RX_MSDU_EXT_DESC_INFO_REO_DESTINATION_INDICATION_MASK                       0x0000001f


/* Description		SERVICE_CODE

			Opaque service code between PPE and Wi-Fi
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_OFFSET                                   0x00000000
#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_LSB                                      5
#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_MSB                                      13
#define RX_MSDU_EXT_DESC_INFO_SERVICE_CODE_MASK                                     0x00003fe0


/* Description		PRIORITY_VALID

			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_OFFSET                                 0x00000000
#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_LSB                                    14
#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_MSB                                    14
#define RX_MSDU_EXT_DESC_INFO_PRIORITY_VALID_MASK                                   0x00004000


/* Description		DATA_OFFSET

			The offset to Rx packet data within the buffer (including
			 Rx DMA offset programming and L3 header padding inserted
			 by Rx OLE).
			
			This field gets passed on by REO to PPE in the EDMA descriptor
			 ('REO_TO_PPE_RING').
			
			<legal all>
*/

#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_OFFSET                                    0x00000000
#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_LSB                                       15
#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_MSB                                       26
#define RX_MSDU_EXT_DESC_INFO_DATA_OFFSET_MASK                                      0x07ff8000


/* Description		SRC_LINK_ID

			Consumer: SW
			Producer: RXDMA
			
			Set to the link ID of the PMAC that received the frame
			<legal all>
*/

#define RX_MSDU_EXT_DESC_INFO_SRC_LINK_ID_OFFSET                                    0x00000000
#define RX_MSDU_EXT_DESC_INFO_SRC_LINK_ID_LSB                                       27
#define RX_MSDU_EXT_DESC_INFO_SRC_LINK_ID_MSB                                       29
#define RX_MSDU_EXT_DESC_INFO_SRC_LINK_ID_MASK                                      0x38000000


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_OFFSET                                    0x00000000
#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_LSB                                       30
#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_MSB                                       31
#define RX_MSDU_EXT_DESC_INFO_RESERVED_0A_MASK                                      0xc0000000



#endif   // RX_MSDU_EXT_DESC_INFO

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

#ifndef _TCL_ENTRANCE_FROM_PPE_RING_H_
#define _TCL_ENTRANCE_FROM_PPE_RING_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TCL_ENTRANCE_FROM_PPE_RING 8


struct tcl_entrance_from_ppe_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t buffer_addr_lo                                          : 32; // [31:0]
             uint32_t buffer_addr_hi                                          :  8, // [7:0]
                      drop_prec                                               :  2, // [9:8]
                      fake_mac_header                                         :  1, // [10:10]
                      known_ind                                               :  1, // [11:11]
                      cpu_code_valid                                          :  1, // [12:12]
                      tunnel_term_ind                                         :  1, // [13:13]
                      tunnel_type                                             :  1, // [14:14]
                      wifi_qos_flag                                           :  1, // [15:15]
                      service_code                                            :  9, // [24:16]
                      reserved_1b                                             :  1, // [25:25]
                      int_pri                                                 :  4, // [29:26]
                      more                                                    :  1, // [30:30]
                      reserved_1a                                             :  1; // [31:31]
             uint32_t opaque_lo                                               : 32; // [31:0]
             uint32_t opaque_hi                                               : 32; // [31:0]
             uint32_t src_info                                                : 16, // [15:0]
                      dst_info                                                : 16; // [31:16]
             uint32_t data_length                                             : 18, // [17:0]
                      pool_id                                                 :  6, // [23:18]
                      wifi_qos                                                :  8; // [31:24]
             uint32_t data_offset                                             : 12, // [11:0]
                      l4_csum_status                                          :  1, // [12:12]
                      l3_csum_status                                          :  1, // [13:13]
                      hash_flag                                               :  2, // [15:14]
                      hash_value                                              : 16; // [31:16]
             uint32_t dscp                                                    :  8, // [7:0]
                      valid_toggle                                            :  1, // [8:8]
                      pppoe_flag                                              :  1, // [9:9]
                      svlan_flag                                              :  1, // [10:10]
                      cvlan_flag                                              :  1, // [11:11]
                      pid                                                     :  4, // [15:12]
                      l3_offset                                               :  8, // [23:16]
                      l4_offset                                               :  8; // [31:24]
#else
             uint32_t buffer_addr_lo                                          : 32; // [31:0]
             uint32_t reserved_1a                                             :  1, // [31:31]
                      more                                                    :  1, // [30:30]
                      int_pri                                                 :  4, // [29:26]
                      reserved_1b                                             :  1, // [25:25]
                      service_code                                            :  9, // [24:16]
                      wifi_qos_flag                                           :  1, // [15:15]
                      tunnel_type                                             :  1, // [14:14]
                      tunnel_term_ind                                         :  1, // [13:13]
                      cpu_code_valid                                          :  1, // [12:12]
                      known_ind                                               :  1, // [11:11]
                      fake_mac_header                                         :  1, // [10:10]
                      drop_prec                                               :  2, // [9:8]
                      buffer_addr_hi                                          :  8; // [7:0]
             uint32_t opaque_lo                                               : 32; // [31:0]
             uint32_t opaque_hi                                               : 32; // [31:0]
             uint32_t dst_info                                                : 16, // [31:16]
                      src_info                                                : 16; // [15:0]
             uint32_t wifi_qos                                                :  8, // [31:24]
                      pool_id                                                 :  6, // [23:18]
                      data_length                                             : 18; // [17:0]
             uint32_t hash_value                                              : 16, // [31:16]
                      hash_flag                                               :  2, // [15:14]
                      l3_csum_status                                          :  1, // [13:13]
                      l4_csum_status                                          :  1, // [12:12]
                      data_offset                                             : 12; // [11:0]
             uint32_t l4_offset                                               :  8, // [31:24]
                      l3_offset                                               :  8, // [23:16]
                      pid                                                     :  4, // [15:12]
                      cvlan_flag                                              :  1, // [11:11]
                      svlan_flag                                              :  1, // [10:10]
                      pppoe_flag                                              :  1, // [9:9]
                      valid_toggle                                            :  1, // [8:8]
                      dscp                                                    :  8; // [7:0]
#endif
};


/* Description		BUFFER_ADDR_LO

			Consumer: TCL
			Producer: PPE DMA/SW
			
			Lower 32 bits of the buffer address buffer_addr_31_0.
			
			This is the address of the starting point of the buffer 
			directly from the PPE Rx Fill descriptor. TCL needs to calculate
			 the packet data address based on DATA_OFFSET. 
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_OFFSET                            0x00000000
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_LSB                               0
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_MSB                               31
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_LO_MASK                              0xffffffff


/* Description		BUFFER_ADDR_HI

			Consumer: TCL/TXDMA
			Producer: PPE DMA/SW
			
			Higher 8 bits of the buffer address buffer_addr_39_32 (Not
			 supported PPE but could be supported by PPE in
			 future). Also see BUFFER_ADDR_LO.
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_OFFSET                            0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_LSB                               0
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_MSB                               7
#define TCL_ENTRANCE_FROM_PPE_RING_BUFFER_ADDR_HI_MASK                              0x000000ff


/* Description		DROP_PREC

			Consumer: TCL/TQM
			Producer: Switch Core
			
			Packet drop precedence
			
			TCL maps DROP_PREC to field msdu_color in structure
			 'TX_MSDU_DETAILS' in  'TQM_ENTRANCE_RING' if the internal
			 parameter 'DROP_PREC_ENABLE' is set (see field DST_INFO) 
			and DROP_PREC is set to a legal value. Otherwise msdu_color
			 is set to MSDU_COLORLESS.
			
			<enum 0 PPE_drop_prec_green>
			<enum 1 PPE_drop_prec_yellow>
			<enum 2 PPE_drop_prec_red>
			<legal 0-2>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_OFFSET                                 0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_LSB                                    8
#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_MSB                                    9
#define TCL_ENTRANCE_FROM_PPE_RING_DROP_PREC_MASK                                   0x00000300


/* Description		FAKE_MAC_HEADER

			Consumer: SW
			Producer: Switch Core
			
			Indicates the MAC header is fake (Not supported for direct
			 switch connect)
			0:  No fake MAC header
			1:  Fake MAC header
			<legal 0>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_OFFSET                           0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_LSB                              10
#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_MSB                              10
#define TCL_ENTRANCE_FROM_PPE_RING_FAKE_MAC_HEADER_MASK                             0x00000400


/* Description		KNOWN_IND

			Consumer: TCL
			Producer: Switch Core
			
			Known packet indication
			0: packet is unknown flooding.
			1: packet is forwarded by any known entry.
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_OFFSET                                 0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_LSB                                    11
#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_MSB                                    11
#define TCL_ENTRANCE_FROM_PPE_RING_KNOWN_IND_MASK                                   0x00000800


/* Description		CPU_CODE_VALID

			Consumer: SW
			Producer: Switch Core
			
			Indicates validity of 'CPU_CODE' (used to indicate the reason
			 the packet is sent to the CPU) (Not supported for direct
			 switch connect)
			0: Invalid
			1: Valid
			<legal 0>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_OFFSET                            0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_LSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_MSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_CPU_CODE_VALID_MASK                              0x00001000


/* Description		TUNNEL_TERM_IND

			Consumer: TCL
			Producer: Switch Core
			
			Tunnel termination indication
			0: packet is not decapsulated
			1: packet is decapsulated
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_OFFSET                           0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_LSB                              13
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_MSB                              13
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TERM_IND_MASK                             0x00002000


/* Description		TUNNEL_TYPE

			Consumer: TCL
			Producer: Switch Core
			
			Tunnel Type
			0: Layer 2 tunnel
			1: Layer 3 tunnel
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_OFFSET                               0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_LSB                                  14
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_MSB                                  14
#define TCL_ENTRANCE_FROM_PPE_RING_TUNNEL_TYPE_MASK                                 0x00004000


/* Description		WIFI_QOS_FLAG

			Consumer: TCL
			Producer: Switch Core
			
			Wi-Fi QoS Flag
			0: If WIFI_QOS[7] is set, WIFI_QOS[3:1] provides a 3-bit
			 HLOS_TID value and HLOS_TID_overwrite is enabled, else 
			there is no overwrite.
			1: WIFI_QOS[5:0] provides a 6-bit "flow pointer override" 
			value by using:
			who_classify_info_sel = WIFI_QOS[5:4],
			HLOS_TID = WIFI_QOS[3:1],
			flow_override = WIFI_QOS[0],
			and HLOS_TID_overwrite and flow_override_enable are set.
			
			
			Also see field INT_PRI for another way to enable HLOS_TID_overwrite.
			
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_OFFSET                             0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_LSB                                15
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_MSB                                15
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_FLAG_MASK                               0x00008000


/* Description		SERVICE_CODE

			Consumer: TCL
			Producer: Switch Core
			
			Opaque service code between engines
			0: Indicates the end of service path
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_OFFSET                              0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_LSB                                 16
#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_MSB                                 24
#define TCL_ENTRANCE_FROM_PPE_RING_SERVICE_CODE_MASK                                0x01ff0000


/* Description		RESERVED_1B

			<legal 0, 1>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_OFFSET                               0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_LSB                                  25
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_MSB                                  25
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1B_MASK                                 0x02000000


/* Description		INT_PRI

			Consumer: TCL
			Producer: Switch Core
			
			Internal/User Priority
			
			TCL maps INT_PRI to HLOS_TID using an internal mapping
			 table if the internal parameter 'USE_PPE_INT_PRI_FOR_TID' 
			is set (see field DST_INFO) and WIFI_QOS_FLAG is unset and
			 WIFI_QOS[7] is unset.
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_OFFSET                                   0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_LSB                                      26
#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_MSB                                      29
#define TCL_ENTRANCE_FROM_PPE_RING_INT_PRI_MASK                                     0x3c000000


/* Description		MORE

			Consumer: TCL
			Producer: PPE DMA
			
			0: The last segment of packet
			1: More segments to follow, indicating scatter/gather
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_MORE_OFFSET                                      0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_MORE_LSB                                         30
#define TCL_ENTRANCE_FROM_PPE_RING_MORE_MSB                                         30
#define TCL_ENTRANCE_FROM_PPE_RING_MORE_MASK                                        0x40000000


/* Description		RESERVED_1A

			<legal 0>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_OFFSET                               0x00000004
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_LSB                                  31
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_MSB                                  31
#define TCL_ENTRANCE_FROM_PPE_RING_RESERVED_1A_MASK                                 0x80000000


/* Description		OPAQUE_LO

			Consumer: TCL/WBM/SW
			Producer: PPE DMA/SW
			
			Lower 32 bits of opaque SW value
			
			OPAQUE_LO[19:0] are used for Sw_buffer_cookie with OPAQUE_LO[31:20] 
			ignored, for direct switch connect.
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_OFFSET                                 0x00000008
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_LSB                                    0
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_MSB                                    31
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_LO_MASK                                   0xffffffff


/* Description		OPAQUE_HI

			Consumer: SW
			Producer: PPE DMA/SW
			
			Higher 32 bits of opaque SW value, ignored completely for
			 direct switch connect
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_OFFSET                                 0x0000000c
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_LSB                                    0
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_MSB                                    31
#define TCL_ENTRANCE_FROM_PPE_RING_OPAQUE_HI_MASK                                   0xffffffff


/* Description		SRC_INFO

			Consumer: TCL
			Producer: Switch Core
			
			Source port: SRC_INFO[15:12] = 'b0010, SRC_INFO[11:0] is
			 the PORT_ID.
			See DST_INFO for PORT_ID values.
			<legal 8192-8447>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_OFFSET                                  0x00000010
#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_LSB                                     0
#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_MSB                                     15
#define TCL_ENTRANCE_FROM_PPE_RING_SRC_INFO_MASK                                    0x0000ffff


/* Description		DST_INFO

			Consumer: TCL
			Producer: Switch Core
			
			Destination port or next hop information
			
			DST_INFO[15:12] = 'b0000 indicates invalid information.
			If DST_INFO[15:12] = 'b0001, DST_INFO[11:0] is the next 
			hop index (Not supported for direct switch connect).
			If DST_INFO[15:12] = 'b0010, DST_INFO[11:0] is the PORT_ID, 
			which TCL can process.
			If DST_INFO[15:12] = 'b0011, DST_INFO[11:0] is the destination
			 port bitmap (Not supported for direct switch connect).
			
			PORT_ID:
			0-31 indicates a physical Ethernet port.
			32-63 indicates a link aggregation group (LAG) of ports (Not
			 supported for direct switch connect).
			64-255 indicates a virtual port, which TCL maps
			to Bank_id, PMAC_ID, vdev_id, To_FW and Search_index.
			 TCL also maps this to internal parameters 'USE_PPE_INT_PRI_FOR_TID' 
			and 'DROP_PREC_ENABLE' (see fields INT_PRI and DROP_PREC).
			
			Other values are reserved.
			<legal 0-8447,12288-16383>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_OFFSET                                  0x00000010
#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_LSB                                     16
#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_MSB                                     31
#define TCL_ENTRANCE_FROM_PPE_RING_DST_INFO_MASK                                    0xffff0000


/* Description		DATA_LENGTH

			Consumer: TCL/TXDMA
			Producer: PPE DMA
			
			Length of valid packet data in the current buffer in bytes
			 (Bits [17:16] not supported PPE and bits [17:14] 
			not supported)
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_OFFSET                               0x00000014
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_LSB                                  0
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_MSB                                  17
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_LENGTH_MASK                                 0x0003ffff


/* Description		POOL_ID

			Consumer: TCL/SW
			Producer: PPE DMA/SW
			
			To be used for hardware buffer management
			
			SW must ensure 1:1 mapping between PPE Rx Fill and PPE Rx
			 completion descriptors.
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_OFFSET                                   0x00000014
#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_LSB                                      18
#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_MSB                                      23
#define TCL_ENTRANCE_FROM_PPE_RING_POOL_ID_MASK                                     0x00fc0000


/* Description		WIFI_QOS

			Consumer: TCL
			Producer: Switch Core
			
			Wi-Fi QoS Value
			
			TCL maps as follows:
			who_classify_info_sel = WIFI_QOS[5:4] if WIFI_QOS_FLAG set
			
			HLOS_TID = WIFI_QOS[3:1] if HLOS_TID_overwrite enabled
			flow_override = WIFI_QOS [0] if WIFI_QOS_FLAG set
			flow_override_enable = WIFI_QOS_FLAG
			HLOS_TID_overwrite = WIFI_QOS_FLAG || WIFI_QOS[7]
			
			WIFI_QOS[6] is ignored by TCL.
			
			Also see field INT_PRI for another way to enable HLOS_TID_overwrite.
			
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_OFFSET                                  0x00000014
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_LSB                                     24
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_MSB                                     31
#define TCL_ENTRANCE_FROM_PPE_RING_WIFI_QOS_MASK                                    0xff000000


/* Description		DATA_OFFSET

			Consumer: TCL
			Producer: PPE DMA
			
			Offset to the packet data from the buffer address
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_OFFSET                               0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_LSB                                  0
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_MSB                                  11
#define TCL_ENTRANCE_FROM_PPE_RING_DATA_OFFSET_MASK                                 0x00000fff


/* Description		L4_CSUM_STATUS

			Consumer: TCL
			Producer: PPE DMA/Switch Core
			
			Layer 4 checksum verification result
			0: Unknown or invalid
			1: Valid
			The default value is 0. Only when PPE DMA performs the checksum
			 calculation and the result is correct, is this bit set.
			
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_OFFSET                            0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_LSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_MSB                               12
#define TCL_ENTRANCE_FROM_PPE_RING_L4_CSUM_STATUS_MASK                              0x00001000


/* Description		L3_CSUM_STATUS

			Consumer: TCL
			Producer: PPE DMA/Switch Core
			
			Layer 3 checksum verification result
			0: Unknown or invalid
			1: Valid
			The default value is 0. Only when PPE DMA performs the checksum
			 calculation and the result is correct, is this bit set.
			
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_OFFSET                            0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_LSB                               13
#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_MSB                               13
#define TCL_ENTRANCE_FROM_PPE_RING_L3_CSUM_STATUS_MASK                              0x00002000


/* Description		HASH_FLAG

			Consumer: SW
			Producer: Switch Core
			
			Hash type
			00: Hash invalid
			01: 5-tuple hash
			10: 3-tuple hash
			11: Reserved
			<legal 0-2>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_OFFSET                                 0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_LSB                                    14
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_MSB                                    15
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_FLAG_MASK                                   0x0000c000


/* Description		HASH_VALUE

			Consumer: SW
			Producer: Switch Core
			
			Hash value
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_OFFSET                                0x00000018
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_LSB                                   16
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_MSB                                   31
#define TCL_ENTRANCE_FROM_PPE_RING_HASH_VALUE_MASK                                  0xffff0000


/* Description		DSCP

			Consumer: TCL
			Producer: PPE DMA/Switch Core
			
			Differential Services Code Point value
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_OFFSET                                      0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_LSB                                         0
#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_MSB                                         7
#define TCL_ENTRANCE_FROM_PPE_RING_DSCP_MASK                                        0x000000ff


/* Description		VALID_TOGGLE

			Consumer: TCL
			Producer: PPE DMA
			
			Toggle bit to indicate the validity of the descriptor
			The value is toggled when the producer pointer wraps around.
			
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_OFFSET                              0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_LSB                                 8
#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_MSB                                 8
#define TCL_ENTRANCE_FROM_PPE_RING_VALID_TOGGLE_MASK                                0x00000100


/* Description		PPPOE_FLAG

			Consumer: TCL
			Producer: Switch Core
			
			Indicates a PPPoE packet
			0: No PPPoE header
			1: PPPoE header exists
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_OFFSET                                0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_LSB                                   9
#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_MSB                                   9
#define TCL_ENTRANCE_FROM_PPE_RING_PPPOE_FLAG_MASK                                  0x00000200


/* Description		SVLAN_FLAG

			Consumer: TCL
			Producer: PPE DMA/Switch Core
			
			Indicates the existence of S-VLAN tag
			0: No S-VLAN
			1: S-VLAN exists, including priority
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_OFFSET                                0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_LSB                                   10
#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_MSB                                   10
#define TCL_ENTRANCE_FROM_PPE_RING_SVLAN_FLAG_MASK                                  0x00000400


/* Description		CVLAN_FLAG

			Consumer: TCL
			Producer: PPE DMA/Switch Core
			
			Indicates the existence of C-VLAN tag
			0: No C-VLAN
			1: C-VLAN exists, including priority
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_OFFSET                                0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_LSB                                   11
#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_MSB                                   11
#define TCL_ENTRANCE_FROM_PPE_RING_CVLAN_FLAG_MASK                                  0x00000800


/* Description		PID

			Consumer: TCL
			Producer: Switch Core
			
			Protocol ID, indicating the protocol type of the packet
			0: IPv4 (no supported L4)
			1: TCP over IPv4
			2: UDP over IPv4
			3: UDP-Lite over IPv4
			4: IPv6 (no supported L4)
			5: TCP over IPv6
			6: UDP over IPv6
			7: UDP-Lite over IPv6
			8: Non-IP
			Other values are reserved
			<legal 0-8>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_PID_OFFSET                                       0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_PID_LSB                                          12
#define TCL_ENTRANCE_FROM_PPE_RING_PID_MSB                                          15
#define TCL_ENTRANCE_FROM_PPE_RING_PID_MASK                                         0x0000f000


/* Description		L3_OFFSET

			Consumer: TCL
			Producer: PPE DMA
			
			Layer 3 header offset from DATA_OFFSET
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_OFFSET                                 0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_LSB                                    16
#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_MSB                                    23
#define TCL_ENTRANCE_FROM_PPE_RING_L3_OFFSET_MASK                                   0x00ff0000


/* Description		L4_OFFSET

			Consumer: TCL
			Producer: PPE DMA
			
			Layer 4 header offset from DATA_OFFSET
			<legal all>
*/

#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_OFFSET                                 0x0000001c
#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_LSB                                    24
#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_MSB                                    31
#define TCL_ENTRANCE_FROM_PPE_RING_L4_OFFSET_MASK                                   0xff000000



#endif   // TCL_ENTRANCE_FROM_PPE_RING

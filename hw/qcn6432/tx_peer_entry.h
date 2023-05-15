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

#ifndef _TX_PEER_ENTRY_H_
#define _TX_PEER_ENTRY_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_TX_PEER_ENTRY 18

#define NUM_OF_QWORDS_TX_PEER_ENTRY 9


struct tx_peer_entry {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t mac_addr_a_31_0                                         : 32; // [31:0]
             uint32_t mac_addr_a_47_32                                        : 16, // [15:0]
                      mac_addr_b_15_0                                         : 16; // [31:16]
             uint32_t mac_addr_b_47_16                                        : 32; // [31:0]
             uint32_t use_ad_b                                                :  1, // [0:0]
                      strip_insert_vlan_inner                                 :  1, // [1:1]
                      strip_insert_vlan_outer                                 :  1, // [2:2]
                      vlan_llc_mode                                           :  1, // [3:3]
                      key_type                                                :  4, // [7:4]
                      a_msdu_wds_ad3_ad4                                      :  3, // [10:8]
                      ignore_hard_filters                                     :  1, // [11:11]
                      ignore_soft_filters                                     :  1, // [12:12]
                      epd_output                                              :  1, // [13:13]
                      wds                                                     :  1, // [14:14]
                      insert_or_strip                                         :  1, // [15:15]
                      sw_filter_id                                            : 16; // [31:16]
             uint32_t temporal_key_31_0                                       : 32; // [31:0]
             uint32_t temporal_key_63_32                                      : 32; // [31:0]
             uint32_t temporal_key_95_64                                      : 32; // [31:0]
             uint32_t temporal_key_127_96                                     : 32; // [31:0]
             uint32_t temporal_key_159_128                                    : 32; // [31:0]
             uint32_t temporal_key_191_160                                    : 32; // [31:0]
             uint32_t temporal_key_223_192                                    : 32; // [31:0]
             uint32_t temporal_key_255_224                                    : 32; // [31:0]
             uint32_t sta_partial_aid                                         : 11, // [10:0]
                      transmit_vif                                            :  4, // [14:11]
                      block_this_user                                         :  1, // [15:15]
                      mesh_amsdu_mode                                         :  2, // [17:16]
                      use_qos_alt_mute_mask                                   :  1, // [18:18]
                      dl_ul_direction                                         :  1, // [19:19]
                      reserved_12                                             : 12; // [31:20]
             uint32_t insert_vlan_outer_tci                                   : 16, // [15:0]
                      insert_vlan_inner_tci                                   : 16; // [31:16]
             uint32_t multi_link_addr_ad1_31_0                                : 32; // [31:0]
             uint32_t multi_link_addr_ad1_47_32                               : 16, // [15:0]
                      multi_link_addr_ad2_15_0                                : 16; // [31:16]
             uint32_t multi_link_addr_ad2_47_16                               : 32; // [31:0]
             uint32_t multi_link_addr_crypto_enable                           :  1, // [0:0]
                      reserved_17a                                            : 15, // [15:1]
                      sw_peer_id                                              : 16; // [31:16]
#else
             uint32_t mac_addr_a_31_0                                         : 32; // [31:0]
             uint32_t mac_addr_b_15_0                                         : 16, // [31:16]
                      mac_addr_a_47_32                                        : 16; // [15:0]
             uint32_t mac_addr_b_47_16                                        : 32; // [31:0]
             uint32_t sw_filter_id                                            : 16, // [31:16]
                      insert_or_strip                                         :  1, // [15:15]
                      wds                                                     :  1, // [14:14]
                      epd_output                                              :  1, // [13:13]
                      ignore_soft_filters                                     :  1, // [12:12]
                      ignore_hard_filters                                     :  1, // [11:11]
                      a_msdu_wds_ad3_ad4                                      :  3, // [10:8]
                      key_type                                                :  4, // [7:4]
                      vlan_llc_mode                                           :  1, // [3:3]
                      strip_insert_vlan_outer                                 :  1, // [2:2]
                      strip_insert_vlan_inner                                 :  1, // [1:1]
                      use_ad_b                                                :  1; // [0:0]
             uint32_t temporal_key_31_0                                       : 32; // [31:0]
             uint32_t temporal_key_63_32                                      : 32; // [31:0]
             uint32_t temporal_key_95_64                                      : 32; // [31:0]
             uint32_t temporal_key_127_96                                     : 32; // [31:0]
             uint32_t temporal_key_159_128                                    : 32; // [31:0]
             uint32_t temporal_key_191_160                                    : 32; // [31:0]
             uint32_t temporal_key_223_192                                    : 32; // [31:0]
             uint32_t temporal_key_255_224                                    : 32; // [31:0]
             uint32_t reserved_12                                             : 12, // [31:20]
                      dl_ul_direction                                         :  1, // [19:19]
                      use_qos_alt_mute_mask                                   :  1, // [18:18]
                      mesh_amsdu_mode                                         :  2, // [17:16]
                      block_this_user                                         :  1, // [15:15]
                      transmit_vif                                            :  4, // [14:11]
                      sta_partial_aid                                         : 11; // [10:0]
             uint32_t insert_vlan_inner_tci                                   : 16, // [31:16]
                      insert_vlan_outer_tci                                   : 16; // [15:0]
             uint32_t multi_link_addr_ad1_31_0                                : 32; // [31:0]
             uint32_t multi_link_addr_ad2_15_0                                : 16, // [31:16]
                      multi_link_addr_ad1_47_32                               : 16; // [15:0]
             uint32_t multi_link_addr_ad2_47_16                               : 32; // [31:0]
             uint32_t sw_peer_id                                              : 16, // [31:16]
                      reserved_17a                                            : 15, // [15:1]
                      multi_link_addr_crypto_enable                           :  1; // [0:0]
#endif
};


/* Description		MAC_ADDR_A_31_0

			Consumer: TX OLE
			Producer: SW
			
			Lower 32 bits of the MAC address A used by HW for encapsulating
			 802.11
			<legal all>
*/

#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_OFFSET                                        0x0000000000000000
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_LSB                                           0
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_MSB                                           31
#define TX_PEER_ENTRY_MAC_ADDR_A_31_0_MASK                                          0x00000000ffffffff


/* Description		MAC_ADDR_A_47_32

			Consumer: TX OLE
			Producer: SW
			
			Upper 16 bits of the MAC address A used by HW for encapsulating
			 802.11
			<legal all>
*/

#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_OFFSET                                       0x0000000000000000
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_LSB                                          32
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_MSB                                          47
#define TX_PEER_ENTRY_MAC_ADDR_A_47_32_MASK                                         0x0000ffff00000000


/* Description		MAC_ADDR_B_15_0

			Consumer: TX OLE
			Producer: SW
			
			Lower 16 bits of the MAC address B used by HW for encapsulating
			 802.11
			<legal all>
*/

#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_OFFSET                                        0x0000000000000000
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_LSB                                           48
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_MSB                                           63
#define TX_PEER_ENTRY_MAC_ADDR_B_15_0_MASK                                          0xffff000000000000


/* Description		MAC_ADDR_B_47_16

			Consumer: TX OLE
			Producer: SW
			
			Upper 32 bits of the MAC address B used by HW for encapsulating
			 802.11
			<legal all>
*/

#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_OFFSET                                       0x0000000000000008
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_LSB                                          0
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_MSB                                          31
#define TX_PEER_ENTRY_MAC_ADDR_B_47_16_MASK                                         0x00000000ffffffff


/* Description		USE_AD_B

			Consumer: TX OLE
			Producer: SW
			
			The bit is only evaluated when this MSDU is the first MSDU
			 in an MPDU. For other MSDUs this bit setting is ignored.
			
			It is part of the sw_msdu_param coming from the QM ADD frame
			 command.
			
			Normally in AP mode the DA address is used as the RA.  This
			 is normally fine but the use_ad_b bit should be set when
			 DA is a multicast/broadcast address but we want to send
			 this packet using the destination STA address which will
			 be held in the mac_addr_b field of the peer descriptor.
			
			<legal all>
*/

#define TX_PEER_ENTRY_USE_AD_B_OFFSET                                               0x0000000000000008
#define TX_PEER_ENTRY_USE_AD_B_LSB                                                  32
#define TX_PEER_ENTRY_USE_AD_B_MSB                                                  32
#define TX_PEER_ENTRY_USE_AD_B_MASK                                                 0x0000000100000000


/* Description		STRIP_INSERT_VLAN_INNER

			Consumer: TX OLE
			Producer: SW
			
			Strip or insert C-VLAN during encapsulation.
			Insert_or_strip determines whether C-VLAN is to be stripped
			 or inserted.
			<legal all>
*/

#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_OFFSET                                0x0000000000000008
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_LSB                                   33
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_MSB                                   33
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_INNER_MASK                                  0x0000000200000000


/* Description		STRIP_INSERT_VLAN_OUTER

			Consumer: TX OLE
			Producer: SW
			
			Strip or insert S-VLAN during encapsulation.
			Insert or strip determines whether S-VLAN is to be stripped
			 or inserted.
			<legal all>
*/

#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_OFFSET                                0x0000000000000008
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_LSB                                   34
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_MSB                                   34
#define TX_PEER_ENTRY_STRIP_INSERT_VLAN_OUTER_MASK                                  0x0000000400000000


/* Description		VLAN_LLC_MODE

			Consumer: TX OLE
			Producer: SW
			
			If set encapsulate/decapsulate using the Scorpion compatible
			 VLAN LLC format
*/

#define TX_PEER_ENTRY_VLAN_LLC_MODE_OFFSET                                          0x0000000000000008
#define TX_PEER_ENTRY_VLAN_LLC_MODE_LSB                                             35
#define TX_PEER_ENTRY_VLAN_LLC_MODE_MSB                                             35
#define TX_PEER_ENTRY_VLAN_LLC_MODE_MASK                                            0x0000000800000000


/* Description		KEY_TYPE

			Consumer: TX OLE, TX CRYPTO
			Producer: SW
			
			The key_type indicates the cipher suite corresponding to
			 this peer entry:
			<enum 0 wep_40> WEP 40-bit
			<enum 1 wep_104> WEP 104-bit
			<enum 2 tkip_no_mic> TKIP without MIC
			<enum 3 wep_128> WEP 128-bit
			<enum 4 tkip_with_mic> TKIP with MIC
			<enum 5 wapi> WAPI
			<enum 6 aes_ccmp_128> AES CCMP 128
			<enum 7 no_cipher> No crypto
			<enum 8 aes_ccmp_256> AES CCMP 256
			<enum 9 aes_gcmp_128> AES GCMP 128
			<enum 10 aes_gcmp_256> AES GCMP 256
			<enum 11 wapi_gcm_sm4> WAPI GCM SM4
			
			<enum 12 wep_varied_width> DO NOT USE. This Key type ONLY
			 to be used for RX side
			
			<legal 0-12>
*/

#define TX_PEER_ENTRY_KEY_TYPE_OFFSET                                               0x0000000000000008
#define TX_PEER_ENTRY_KEY_TYPE_LSB                                                  36
#define TX_PEER_ENTRY_KEY_TYPE_MSB                                                  39
#define TX_PEER_ENTRY_KEY_TYPE_MASK                                                 0x000000f000000000


/* Description		A_MSDU_WDS_AD3_AD4

			Consumer: TX OLE
			Producer: SW
			
			Determines the selection of AD3 and AD4 for A-MSDU 4 address
			 frames (WDS):
			<enum 0 ad3_a__ad4_a> AD3 = AD_A, AD4 = AD_A
			<enum 1 ad3_a__ad4_b> AD3 = AD_A, AD4 = AD_B
			<enum 2 ad3_b__ad4_a> AD3 = AD_B, AD4 = AD_A
			<enum 3 ad3_b__ad4_b> AD3 = AD_B, AD4 = AD_B
			<enum 4 ad3_da__ad4_sa> AD3 = DA, AD4 = SA
			<legal 0-4>
*/

#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_OFFSET                                     0x0000000000000008
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_LSB                                        40
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_MSB                                        42
#define TX_PEER_ENTRY_A_MSDU_WDS_AD3_AD4_MASK                                       0x0000070000000000


/* Description		IGNORE_HARD_FILTERS

			SW can program this bit to 0x1 to ignore HARD filter conditions
			 and HWSCH will proceed with transmission, even if the HARD
			 filter bit is set in Filter LUT.
			Note that SOFT filter conditions will filter the command, 
			even if this bit is set and ignore_soft_filters is not set
			
			For filtering all frames marked in the Filter LUT, both 
			ignore_soft_filters and ignore_hard_filters should be set
			
			<legal all>
*/

#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_OFFSET                                    0x0000000000000008
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_LSB                                       43
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_MSB                                       43
#define TX_PEER_ENTRY_IGNORE_HARD_FILTERS_MASK                                      0x0000080000000000


/* Description		IGNORE_SOFT_FILTERS

			SW can program this bit to 0x1 to ignore SOFT filter conditions
			 and HWSCH will proceed with transmission, even if the SOFT
			 filter bit is set in Filter LUT.
			Note that HARD filter conditions will filter the command, 
			even if this bit is set and ignore_hard_filters is not set
			
			For filtering all frames marked in the Filter LUT, both 
			ignore_soft_filters and ignore_hard_filters should be set
			
			
			<legal all>
*/

#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_OFFSET                                    0x0000000000000008
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_LSB                                       44
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_MSB                                       44
#define TX_PEER_ENTRY_IGNORE_SOFT_FILTERS_MASK                                      0x0000100000000000


/* Description		EPD_OUTPUT

			Consumer: TX OLE
			Producer: SW
			
			If set use EPD instead of LPD
*/

#define TX_PEER_ENTRY_EPD_OUTPUT_OFFSET                                             0x0000000000000008
#define TX_PEER_ENTRY_EPD_OUTPUT_LSB                                                45
#define TX_PEER_ENTRY_EPD_OUTPUT_MSB                                                45
#define TX_PEER_ENTRY_EPD_OUTPUT_MASK                                               0x0000200000000000


/* Description		WDS

			If set all the frames in this transmission (for this user) 
			are 4-address frame.  
			
			If not all frames need to use 4 address format, SW has per
			 frame 'wds' control, by using the 'wds' flag in the MSDU_EXTENSION
			 descriptor
			
			Used by the OLE during encapsulation.  
			<legal all>
*/

#define TX_PEER_ENTRY_WDS_OFFSET                                                    0x0000000000000008
#define TX_PEER_ENTRY_WDS_LSB                                                       46
#define TX_PEER_ENTRY_WDS_MSB                                                       46
#define TX_PEER_ENTRY_WDS_MASK                                                      0x0000400000000000


/* Description		INSERT_OR_STRIP

			<enum 0 TXOLE_STRIP_VLAN> TXOLE will strip inner or outer
			 VLAN (if present in the frame) based on Strip_insert_vlan_{inner, 
			outer}
			<enum 1 TXOLE_INSERT_VLAN> TXOLE will insert inner or outer
			 VLAN (only if absent in the frame) based on Strip_insert_vlan_{inner, 
			outer} with the TCI(s) given by Insert_vlan_{inner, outer}_tci
			
			NOTE: Strip VLAN is not supported by TCL.
			<legal all>
*/

#define TX_PEER_ENTRY_INSERT_OR_STRIP_OFFSET                                        0x0000000000000008
#define TX_PEER_ENTRY_INSERT_OR_STRIP_LSB                                           47
#define TX_PEER_ENTRY_INSERT_OR_STRIP_MSB                                           47
#define TX_PEER_ENTRY_INSERT_OR_STRIP_MASK                                          0x0000800000000000


/* Description		SW_FILTER_ID

			Consumer: SCH
			Producer: SW
			
			The full STA AID.
			Use by SCH to determine if transmission for this STA should
			 be filtered as it just went into power save state.
			In case of MU transmission, it means only this STA needs
			 to be removed from the transmission...
			
			<legal all>
*/

#define TX_PEER_ENTRY_SW_FILTER_ID_OFFSET                                           0x0000000000000008
#define TX_PEER_ENTRY_SW_FILTER_ID_LSB                                              48
#define TX_PEER_ENTRY_SW_FILTER_ID_MSB                                              63
#define TX_PEER_ENTRY_SW_FILTER_ID_MASK                                             0xffff000000000000


/* Description		TEMPORAL_KEY_31_0

			Consumer: TX CRYPTO
			Producer: SW
			
			First 32 bits of the temporal key material.  The temporal
			 key for WEP 40-bit uses the first 40 bits, WEP 104-bit 
			uses the first 104 bits, WEP 128-bit uses all 128 bits, 
			TKIP with/without MIC uses 128 bits, WAPI uses all 128 bits, 
			and AES-CCM uses all 128 bits.
			
			Note that for TKIP, the 64 MIC bits are located in fields
			 'temporal_key[255:192]
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_OFFSET                                      0x0000000000000010
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_LSB                                         0
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_MSB                                         31
#define TX_PEER_ENTRY_TEMPORAL_KEY_31_0_MASK                                        0x00000000ffffffff


/* Description		TEMPORAL_KEY_63_32

			Consumer: TX CRYPTO
			Producer: SW
			
			Second 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_OFFSET                                     0x0000000000000010
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_LSB                                        32
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_MSB                                        63
#define TX_PEER_ENTRY_TEMPORAL_KEY_63_32_MASK                                       0xffffffff00000000


/* Description		TEMPORAL_KEY_95_64

			Consumer: TX CRYPTO
			Producer: SW
			
			Third 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_OFFSET                                     0x0000000000000018
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_LSB                                        0
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_MSB                                        31
#define TX_PEER_ENTRY_TEMPORAL_KEY_95_64_MASK                                       0x00000000ffffffff


/* Description		TEMPORAL_KEY_127_96

			Consumer: TX CRYPTO
			Producer: SW
			
			Fourth 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_OFFSET                                    0x0000000000000018
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_LSB                                       32
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_MSB                                       63
#define TX_PEER_ENTRY_TEMPORAL_KEY_127_96_MASK                                      0xffffffff00000000


/* Description		TEMPORAL_KEY_159_128

			Consumer: TX CRYPTO
			Producer: SW
			
			Fifth 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.  
			
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_OFFSET                                   0x0000000000000020
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_159_128_MASK                                     0x00000000ffffffff


/* Description		TEMPORAL_KEY_191_160

			Consumer: TX CRYPTO
			Producer: SW
			
			Final 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.  
			
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_OFFSET                                   0x0000000000000020
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_LSB                                      32
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_MSB                                      63
#define TX_PEER_ENTRY_TEMPORAL_KEY_191_160_MASK                                     0xffffffff00000000


/* Description		TEMPORAL_KEY_223_192

			Consumer: TX CRYPTO
			Producer: SW
			
			Final 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.  
			
			For TKIP this is the TX MIC key[31:0].
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_OFFSET                                   0x0000000000000028
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_LSB                                      0
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_MSB                                      31
#define TX_PEER_ENTRY_TEMPORAL_KEY_223_192_MASK                                     0x00000000ffffffff


/* Description		TEMPORAL_KEY_255_224

			Consumer: TX CRYPTO
			Producer: SW
			
			Final 32 bits of the temporal key material.  See the description
			 of temporal_key_31_0.  
			
			For TKIP this is the TX MIC key[63:32].
			<legal all>
*/

#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_OFFSET                                   0x0000000000000028
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_LSB                                      32
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_MSB                                      63
#define TX_PEER_ENTRY_TEMPORAL_KEY_255_224_MASK                                     0xffffffff00000000


/* Description		STA_PARTIAL_AID

			This field in only used by the PDG. All other modules should
			 ignore this field.
			
			This field is only valid in case of a transmission at VHT
			 rates or HE rates.
			
			For VHT:
			This field is the Partial AID to be filled in to the VHT
			 preamble.
			
			For HE:
			This field is the sta_aid to be filled into the SIG B field.
			
			
			In 11ah mode of operation, this field is provided by SW 
			to populate the the ID value of the SIG preamble of the 
			PPDU
*/

#define TX_PEER_ENTRY_STA_PARTIAL_AID_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_STA_PARTIAL_AID_LSB                                           0
#define TX_PEER_ENTRY_STA_PARTIAL_AID_MSB                                           10
#define TX_PEER_ENTRY_STA_PARTIAL_AID_MASK                                          0x00000000000007ff


/* Description		TRANSMIT_VIF

			Consumer: TXOLE
			Producer: SW
			
			The VIF for this transmission. Used in MCC mode to control/overwrite
			 the PM bit settings.
			<legal all>
*/

#define TX_PEER_ENTRY_TRANSMIT_VIF_OFFSET                                           0x0000000000000030
#define TX_PEER_ENTRY_TRANSMIT_VIF_LSB                                              11
#define TX_PEER_ENTRY_TRANSMIT_VIF_MSB                                              14
#define TX_PEER_ENTRY_TRANSMIT_VIF_MASK                                             0x0000000000007800


/* Description		BLOCK_THIS_USER

			Consumer: PDG
			Producer: SCH
			
			Set by SCH when a MU transmission is started and this STA
			 has (just) entered or is in power save mode. 
			Due to the MU transmission SCH shall not terminate this 
			MU transmission (as is done with SU transmission), but continue
			 with the transmissions for all other STAs. 
			
			As a result of this bit being set, PDG will at certain moment
			 generate the MPDU limit TLV with field Num_mpdu_user set
			 to 0 
			
			PDG shall treat this user as a user without any data. All
			 rules related to terminating MU transmissions when too 
			many users do not have any data shall include this user 
			as a user having zero data. 
			
			When clear, PDG can ignore this bit
			<legal all>
*/

#define TX_PEER_ENTRY_BLOCK_THIS_USER_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_BLOCK_THIS_USER_LSB                                           15
#define TX_PEER_ENTRY_BLOCK_THIS_USER_MSB                                           15
#define TX_PEER_ENTRY_BLOCK_THIS_USER_MASK                                          0x0000000000008000


/* Description		MESH_AMSDU_MODE

			Consumer: TX OLE
			Producer: SW
			
			This field is used only when the first MSDU of any MPDU 
			that TX OLE encounters is in Native WiFi format and includes
			 a 'Mesh Control' field between the header and the LLC.
			
			The creation of the A-MSDU 'Length' field in the MPDU (if
			 aggregating multiple MSDUs) is decided by the value of 
			this field.
			
			<enum 0 MESH_MODE_0> DO NOT USE
			<enum 1 MESH_MODE_Q2Q> A-MSDU 'Length' is big endian and
			 includes the length of Mesh Control.
			<enum 2 MESH_MODE_11S_BE> A-MSDU 'Length' is big endian 
			and excludes the length of Mesh Control.
			<enum 3 MESH_MODE_11S_LE> A-MSDU 'Length' is little endian
			 and excludes the length of Mesh Control. This is 802.11s-compliant.
			
			
			NOTE 1: For compatibility TXOLE treats MESH_MODE_0 identically
			 to MESH_MODE_Q2Q.
			
			NOTE 2: This e-numeration is different from other fields
			 named Mesh_sta or mesh_enable where the value zero disables
			 mesh processing.
			<legal 0-3>
*/

#define TX_PEER_ENTRY_MESH_AMSDU_MODE_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_LSB                                           16
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_MSB                                           17
#define TX_PEER_ENTRY_MESH_AMSDU_MODE_MASK                                          0x0000000000030000



#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_OFFSET                                  0x0000000000000030
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_LSB                                     18
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_MSB                                     18
#define TX_PEER_ENTRY_USE_QOS_ALT_MUTE_MASK_MASK                                    0x0000000000040000


/* Description		DL_UL_DIRECTION

			'Direction' to be inferred for raw WiFi esp. management 
			frames sent to a multi-link peer, for translating RA and/or
			 TA.
			
			<enum 0 DL_UL_FLAG_IS_DL_OR_TDLS>
			<enum 1 DL_UL_FLAG_IS_UL>
			<legal all>
*/

#define TX_PEER_ENTRY_DL_UL_DIRECTION_OFFSET                                        0x0000000000000030
#define TX_PEER_ENTRY_DL_UL_DIRECTION_LSB                                           19
#define TX_PEER_ENTRY_DL_UL_DIRECTION_MSB                                           19
#define TX_PEER_ENTRY_DL_UL_DIRECTION_MASK                                          0x0000000000080000


/* Description		RESERVED_12

			<legal 0>
*/

#define TX_PEER_ENTRY_RESERVED_12_OFFSET                                            0x0000000000000030
#define TX_PEER_ENTRY_RESERVED_12_LSB                                               20
#define TX_PEER_ENTRY_RESERVED_12_MSB                                               31
#define TX_PEER_ENTRY_RESERVED_12_MASK                                              0x00000000fff00000


/* Description		INSERT_VLAN_OUTER_TCI

			The tag control info to use when TXOLE inserts outer VLAN
			 if enabled by Strip_insert_vlan_outer and Insert_or_strip
			
*/

#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_OFFSET                                  0x0000000000000030
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_LSB                                     32
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_MSB                                     47
#define TX_PEER_ENTRY_INSERT_VLAN_OUTER_TCI_MASK                                    0x0000ffff00000000


/* Description		INSERT_VLAN_INNER_TCI

			The tag control info to use when TXOLE inserts inner VLAN
			 if enabled by Strip_insert_vlan_inner and Insert_or_strip
			
*/

#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_OFFSET                                  0x0000000000000030
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_LSB                                     48
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_MSB                                     63
#define TX_PEER_ENTRY_INSERT_VLAN_INNER_TCI_MASK                                    0xffff000000000000


/* Description		MULTI_LINK_ADDR_AD1_31_0

			Consumer: TX CRYPTO
			Producer: FW
			
			Field only valid if Multi_link_addr_crypto_enable is set
			
			
			Multi-link receiver address (address1) for transmissions
			 matching this peer entry, bits [31:0]
*/

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_OFFSET                               0x0000000000000038
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_LSB                                  0
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_MSB                                  31
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_31_0_MASK                                 0x00000000ffffffff


/* Description		MULTI_LINK_ADDR_AD1_47_32

			Consumer: TX CRYPTO
			Producer: FW
			
			Field only valid if Multi_link_addr_crypto_enable is set
			
			
			Multi-link receiver address (address1) for transmissions
			 matching this peer entry, bits [47:32]
*/

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_OFFSET                              0x0000000000000038
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_LSB                                 32
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_MSB                                 47
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD1_47_32_MASK                                0x0000ffff00000000


/* Description		MULTI_LINK_ADDR_AD2_15_0

			Consumer: TX CRYPTO
			Producer: FW
			
			Field only valid if Multi_link_addr_crypto_enable is set
			
			
			Multi-link transmitter address (address2) for transmissions
			 matching this peer entry, bits [15:0]
*/

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_OFFSET                               0x0000000000000038
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_LSB                                  48
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_MSB                                  63
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_15_0_MASK                                 0xffff000000000000


/* Description		MULTI_LINK_ADDR_AD2_47_16

			Consumer: TX CRYPTO
			Producer: FW
			
			Field only valid if Multi_link_addr_crypto_enable is set
			
			
			Multi-link transmitter address (address2) for transmissions
			 matching this peer entry, bits [47:16]
*/

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_OFFSET                              0x0000000000000040
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_LSB                                 0
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_MSB                                 31
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_AD2_47_16_MASK                                0x00000000ffffffff


/* Description		MULTI_LINK_ADDR_CRYPTO_ENABLE

			Consumer: TX CRYPTO
			Producer: FW
			
			If set, TX CRYPTO shall convert Address1, Address2 and BSSID
			 of received data frames to multi-link addresses for the
			 AAD and Nonce during encryption.
			<legal all>
*/

#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_OFFSET                          0x0000000000000040
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_LSB                             32
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_MSB                             32
#define TX_PEER_ENTRY_MULTI_LINK_ADDR_CRYPTO_ENABLE_MASK                            0x0000000100000000


/* Description		RESERVED_17A

			<legal 0>
*/

#define TX_PEER_ENTRY_RESERVED_17A_OFFSET                                           0x0000000000000040
#define TX_PEER_ENTRY_RESERVED_17A_LSB                                              33
#define TX_PEER_ENTRY_RESERVED_17A_MSB                                              47
#define TX_PEER_ENTRY_RESERVED_17A_MASK                                             0x0000fffe00000000


/* Description		SW_PEER_ID

			This field indicates a unique peer identifier provided by
			 FW, to be logged via TXMON to host SW.
			
			<legal all>
*/

#define TX_PEER_ENTRY_SW_PEER_ID_OFFSET                                             0x0000000000000040
#define TX_PEER_ENTRY_SW_PEER_ID_LSB                                                48
#define TX_PEER_ENTRY_SW_PEER_ID_MSB                                                63
#define TX_PEER_ENTRY_SW_PEER_ID_MASK                                               0xffff000000000000



#endif   // TX_PEER_ENTRY

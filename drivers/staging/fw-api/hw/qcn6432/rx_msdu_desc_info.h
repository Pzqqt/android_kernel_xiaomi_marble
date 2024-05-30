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

#ifndef _RX_MSDU_DESC_INFO_H_
#define _RX_MSDU_DESC_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RX_MSDU_DESC_INFO 1


struct rx_msdu_desc_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t first_msdu_in_mpdu_flag                                 :  1, // [0:0]
                      last_msdu_in_mpdu_flag                                  :  1, // [1:1]
                      msdu_continuation                                       :  1, // [2:2]
                      msdu_length                                             : 14, // [16:3]
                      msdu_drop                                               :  1, // [17:17]
                      sa_is_valid                                             :  1, // [18:18]
                      da_is_valid                                             :  1, // [19:19]
                      da_is_mcbc                                              :  1, // [20:20]
                      l3_header_padding_msb                                   :  1, // [21:21]
                      tcp_udp_chksum_fail                                     :  1, // [22:22]
                      ip_chksum_fail                                          :  1, // [23:23]
                      fr_ds                                                   :  1, // [24:24]
                      to_ds                                                   :  1, // [25:25]
                      intra_bss                                               :  1, // [26:26]
                      dest_chip_id                                            :  2, // [28:27]
                      decap_format                                            :  2, // [30:29]
                      dest_chip_pmac_id                                       :  1; // [31:31]
#else
             uint32_t dest_chip_pmac_id                                       :  1, // [31:31]
                      decap_format                                            :  2, // [30:29]
                      dest_chip_id                                            :  2, // [28:27]
                      intra_bss                                               :  1, // [26:26]
                      to_ds                                                   :  1, // [25:25]
                      fr_ds                                                   :  1, // [24:24]
                      ip_chksum_fail                                          :  1, // [23:23]
                      tcp_udp_chksum_fail                                     :  1, // [22:22]
                      l3_header_padding_msb                                   :  1, // [21:21]
                      da_is_mcbc                                              :  1, // [20:20]
                      da_is_valid                                             :  1, // [19:19]
                      sa_is_valid                                             :  1, // [18:18]
                      msdu_drop                                               :  1, // [17:17]
                      msdu_length                                             : 14, // [16:3]
                      msdu_continuation                                       :  1, // [2:2]
                      last_msdu_in_mpdu_flag                                  :  1, // [1:1]
                      first_msdu_in_mpdu_flag                                 :  1; // [0:0]
#endif
};


/* Description		FIRST_MSDU_IN_MPDU_FLAG

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU 
			
			<enum 0 Not_first_msdu> This is not the first MSDU in the
			 MPDU. 
			<enum 1 first_msdu> This MSDU is the first one in the MPDU.
			
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_OFFSET                            0x00000000
#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_LSB                               0
#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MSB                               0
#define RX_MSDU_DESC_INFO_FIRST_MSDU_IN_MPDU_FLAG_MASK                              0x00000001


/* Description		LAST_MSDU_IN_MPDU_FLAG

			Consumer: WBM/REO/SW/FW
			Producer: RXDMA
			
			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			
			<enum 0 Not_last_msdu> There are more MSDUs linked to this
			 MSDU that belongs to this MPDU 
			<enum 1 Last_msdu> this MSDU is the last one in the MPDU. 
			This setting is only allowed in combination with 'Msdu_continuation' 
			set to 0. This implies that when an msdu is spread out over
			 multiple buffers and thus msdu_continuation is set, only
			 for the very last buffer of the msdu, can the 'last_msdu_in_mpdu_flag' 
			be set.
			
			When both first_msdu_in_mpdu_flag and last_msdu_in_mpdu_flag
			 are set, the MPDU that this MSDU belongs to only contains
			 a single MSDU.
			
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_OFFSET                             0x00000000
#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_LSB                                1
#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MSB                                1
#define RX_MSDU_DESC_INFO_LAST_MSDU_IN_MPDU_FLAG_MASK                               0x00000002


/* Description		MSDU_CONTINUATION

			When set, this MSDU buffer was not able to hold the entire
			 MSDU. The next buffer will therefor contain additional 
			information related to this MSDU.
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_OFFSET                                  0x00000000
#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_LSB                                     2
#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_MSB                                     2
#define RX_MSDU_DESC_INFO_MSDU_CONTINUATION_MASK                                    0x00000004


/* Description		MSDU_LENGTH

			Parsed from RX_MSDU_START TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the First
			 buffer used by MSDU.
			 
			Full MSDU length in bytes after decapsulation. 
			
			This field is still valid for MPDU frames without A-MSDU. 
			 It still represents MSDU length after decapsulation 
			
			Or in case of RAW MPDUs, it indicates the length of the 
			entire MPDU (without FCS field)
			<legal all>
*/

#define RX_MSDU_DESC_INFO_MSDU_LENGTH_OFFSET                                        0x00000000
#define RX_MSDU_DESC_INFO_MSDU_LENGTH_LSB                                           3
#define RX_MSDU_DESC_INFO_MSDU_LENGTH_MSB                                           16
#define RX_MSDU_DESC_INFO_MSDU_LENGTH_MASK                                          0x0001fff8


/* Description		MSDU_DROP

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			When set, REO shall drop this MSDU and not forward it to
			 any other ring...
			<legal all>
*/

#define RX_MSDU_DESC_INFO_MSDU_DROP_OFFSET                                          0x00000000
#define RX_MSDU_DESC_INFO_MSDU_DROP_LSB                                             17
#define RX_MSDU_DESC_INFO_MSDU_DROP_MSB                                             17
#define RX_MSDU_DESC_INFO_MSDU_DROP_MASK                                            0x00020000


/* Description		SA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid SA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_DESC_INFO_SA_IS_VALID_OFFSET                                        0x00000000
#define RX_MSDU_DESC_INFO_SA_IS_VALID_LSB                                           18
#define RX_MSDU_DESC_INFO_SA_IS_VALID_MSB                                           18
#define RX_MSDU_DESC_INFO_SA_IS_VALID_MASK                                          0x00040000


/* Description		DA_IS_VALID

			Parsed from RX_MSDU_END TLV . In the case MSDU spans over
			 multiple buffers, this field will be valid in the Last 
			buffer used by the MSDU
			 
			Indicates that OLE found a valid DA entry for this MSDU
			<legal all>
*/

#define RX_MSDU_DESC_INFO_DA_IS_VALID_OFFSET                                        0x00000000
#define RX_MSDU_DESC_INFO_DA_IS_VALID_LSB                                           19
#define RX_MSDU_DESC_INFO_DA_IS_VALID_MSB                                           19
#define RX_MSDU_DESC_INFO_DA_IS_VALID_MASK                                          0x00080000


/* Description		DA_IS_MCBC

			Field Only valid if "da_is_valid" is set
			
			Indicates the DA address was a Multicast of Broadcast address
			 for this MSDU
			<legal all>
*/

#define RX_MSDU_DESC_INFO_DA_IS_MCBC_OFFSET                                         0x00000000
#define RX_MSDU_DESC_INFO_DA_IS_MCBC_LSB                                            20
#define RX_MSDU_DESC_INFO_DA_IS_MCBC_MSB                                            20
#define RX_MSDU_DESC_INFO_DA_IS_MCBC_MASK                                           0x00100000


/* Description		L3_HEADER_PADDING_MSB

			Passed on from 'RX_MSDU_END' TLV (only the MSB is reported
			 as the LSB is always zero)
			Number of bytes padded to make sure that the L3 header will
			 always start of a Dword boundary
			<legal all>
*/

#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_OFFSET                              0x00000000
#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_LSB                                 21
#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_MSB                                 21
#define RX_MSDU_DESC_INFO_L3_HEADER_PADDING_MSB_MASK                                0x00200000


/* Description		TCP_UDP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the TCP/UDP header.
			<legal all>
*/

#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_OFFSET                                0x00000000
#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_LSB                                   22
#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_MSB                                   22
#define RX_MSDU_DESC_INFO_TCP_UDP_CHKSUM_FAIL_MASK                                  0x00400000


/* Description		IP_CHKSUM_FAIL

			Passed on from 'RX_ATTENTION' TLV
			Indicates that the computed checksum did not match the checksum
			 in the IP header.
			<legal all>
*/

#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_OFFSET                                     0x00000000
#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_LSB                                        23
#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_MSB                                        23
#define RX_MSDU_DESC_INFO_IP_CHKSUM_FAIL_MASK                                       0x00800000


/* Description		FR_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'from DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_DESC_INFO_FR_DS_OFFSET                                              0x00000000
#define RX_MSDU_DESC_INFO_FR_DS_LSB                                                 24
#define RX_MSDU_DESC_INFO_FR_DS_MSB                                                 24
#define RX_MSDU_DESC_INFO_FR_DS_MASK                                                0x01000000


/* Description		TO_DS

			Passed on from 'RX_MPDU_INFO' structure in 'RX_MPDU_START' 
			TLV
			Set if the 'to DS' bit is set in the frame control.
			<legal all>
*/

#define RX_MSDU_DESC_INFO_TO_DS_OFFSET                                              0x00000000
#define RX_MSDU_DESC_INFO_TO_DS_LSB                                                 25
#define RX_MSDU_DESC_INFO_TO_DS_MSB                                                 25
#define RX_MSDU_DESC_INFO_TO_DS_MASK                                                0x02000000


/* Description		INTRA_BSS

			This packet needs intra-BSS routing by SW as the 'vdev_id' 
			for the destination is the same as the 'vdev_id' (from 'RX_MPDU_PCU_START') 
			that this MSDU was got in.
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_INTRA_BSS_OFFSET                                          0x00000000
#define RX_MSDU_DESC_INFO_INTRA_BSS_LSB                                             26
#define RX_MSDU_DESC_INFO_INTRA_BSS_MSB                                             26
#define RX_MSDU_DESC_INFO_INTRA_BSS_MASK                                            0x04000000


/* Description		DEST_CHIP_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which chip's TCL the packet should be
			 queued.
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_OFFSET                                       0x00000000
#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_LSB                                          27
#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_MSB                                          28
#define RX_MSDU_DESC_INFO_DEST_CHIP_ID_MASK                                         0x18000000


/* Description		DECAP_FORMAT

			Indicates the format after decapsulation:
			
			<enum 0 RAW> No encapsulation
			<enum 1 Native_WiFi>
			<enum 2 Ethernet> Ethernet 2 (DIX)  or 802.3 (uses SNAP/LLC)
			
			<enum 3 802_3> Indicate Ethernet
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_DECAP_FORMAT_OFFSET                                       0x00000000
#define RX_MSDU_DESC_INFO_DECAP_FORMAT_LSB                                          29
#define RX_MSDU_DESC_INFO_DECAP_FORMAT_MSB                                          30
#define RX_MSDU_DESC_INFO_DECAP_FORMAT_MASK                                         0x60000000


/* Description		DEST_CHIP_PMAC_ID

			If intra_bss is set, copied by RXOLE/RXDMA from 'ADDR_SEARCH_ENTRY' 
			to support intra-BSS routing with multi-chip multi-link 
			operation.
			
			This indicates into which link/'vdev' the packet should 
			be queued in TCL.
			
			<legal all>
*/

#define RX_MSDU_DESC_INFO_DEST_CHIP_PMAC_ID_OFFSET                                  0x00000000
#define RX_MSDU_DESC_INFO_DEST_CHIP_PMAC_ID_LSB                                     31
#define RX_MSDU_DESC_INFO_DEST_CHIP_PMAC_ID_MSB                                     31
#define RX_MSDU_DESC_INFO_DEST_CHIP_PMAC_ID_MASK                                    0x80000000



#endif   // RX_MSDU_DESC_INFO

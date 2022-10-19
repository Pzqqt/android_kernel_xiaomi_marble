
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


#ifndef _RX_REO_QUEUE_H_
#define _RX_REO_QUEUE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_descriptor_header.h"
#define NUM_OF_DWORDS_RX_REO_QUEUE 32


struct rx_reo_queue {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   uniform_descriptor_header                                 descriptor_header;
             uint32_t receive_queue_number                                    : 16, // [15:0]
                      reserved_1b                                             : 16; // [31:16]
             uint32_t vld                                                     :  1, // [0:0]
                      associated_link_descriptor_counter                      :  2, // [2:1]
                      disable_duplicate_detection                             :  1, // [3:3]
                      soft_reorder_enable                                     :  1, // [4:4]
                      ac                                                      :  2, // [6:5]
                      bar                                                     :  1, // [7:7]
                      rty                                                     :  1, // [8:8]
                      chk_2k_mode                                             :  1, // [9:9]
                      oor_mode                                                :  1, // [10:10]
                      ba_window_size                                          : 10, // [20:11]
                      pn_check_needed                                         :  1, // [21:21]
                      pn_shall_be_even                                        :  1, // [22:22]
                      pn_shall_be_uneven                                      :  1, // [23:23]
                      pn_handling_enable                                      :  1, // [24:24]
                      pn_size                                                 :  2, // [26:25]
                      ignore_ampdu_flag                                       :  1, // [27:27]
                      reserved_2b                                             :  4; // [31:28]
             uint32_t svld                                                    :  1, // [0:0]
                      ssn                                                     : 12, // [12:1]
                      current_index                                           : 10, // [22:13]
                      seq_2k_error_detected_flag                              :  1, // [23:23]
                      pn_error_detected_flag                                  :  1, // [24:24]
                      reserved_3a                                             :  6, // [30:25]
                      pn_valid                                                :  1; // [31:31]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t last_rx_enqueue_timestamp                               : 32; // [31:0]
             uint32_t last_rx_dequeue_timestamp                               : 32; // [31:0]
             uint32_t ptr_to_next_aging_queue_31_0                            : 32; // [31:0]
             uint32_t ptr_to_next_aging_queue_39_32                           :  8, // [7:0]
                      reserved_11a                                            : 24; // [31:8]
             uint32_t ptr_to_previous_aging_queue_31_0                        : 32; // [31:0]
             uint32_t ptr_to_previous_aging_queue_39_32                       :  8, // [7:0]
                      statistics_counter_index                                :  6, // [13:8]
                      reserved_13a                                            : 18; // [31:14]
             uint32_t rx_bitmap_31_0                                          : 32; // [31:0]
             uint32_t rx_bitmap_63_32                                         : 32; // [31:0]
             uint32_t rx_bitmap_95_64                                         : 32; // [31:0]
             uint32_t rx_bitmap_127_96                                        : 32; // [31:0]
             uint32_t rx_bitmap_159_128                                       : 32; // [31:0]
             uint32_t rx_bitmap_191_160                                       : 32; // [31:0]
             uint32_t rx_bitmap_223_192                                       : 32; // [31:0]
             uint32_t rx_bitmap_255_224                                       : 32; // [31:0]
             uint32_t rx_bitmap_287_256                                       : 32; // [31:0]
             uint32_t current_mpdu_count                                      :  7, // [6:0]
                      current_msdu_count                                      : 25; // [31:7]
             uint32_t last_sn_reg_index                                       :  4, // [3:0]
                      timeout_count                                           :  6, // [9:4]
                      forward_due_to_bar_count                                :  6, // [15:10]
                      duplicate_count                                         : 16; // [31:16]
             uint32_t frames_in_order_count                                   : 24, // [23:0]
                      bar_received_count                                      :  8; // [31:24]
             uint32_t mpdu_frames_processed_count                             : 32; // [31:0]
             uint32_t msdu_frames_processed_count                             : 32; // [31:0]
             uint32_t total_processed_byte_count                              : 32; // [31:0]
             uint32_t late_receive_mpdu_count                                 : 12, // [11:0]
                      window_jump_2k                                          :  4, // [15:12]
                      hole_count                                              : 16; // [31:16]
             uint32_t aging_drop_mpdu_count                                   : 16, // [15:0]
                      aging_drop_interval                                     :  8, // [23:16]
                      reserved_30                                             :  8; // [31:24]
             uint32_t reserved_31                                             : 32; // [31:0]
#else
             struct   uniform_descriptor_header                                 descriptor_header;
             uint32_t reserved_1b                                             : 16, // [31:16]
                      receive_queue_number                                    : 16; // [15:0]
             uint32_t reserved_2b                                             :  4, // [31:28]
                      ignore_ampdu_flag                                       :  1, // [27:27]
                      pn_size                                                 :  2, // [26:25]
                      pn_handling_enable                                      :  1, // [24:24]
                      pn_shall_be_uneven                                      :  1, // [23:23]
                      pn_shall_be_even                                        :  1, // [22:22]
                      pn_check_needed                                         :  1, // [21:21]
                      ba_window_size                                          : 10, // [20:11]
                      oor_mode                                                :  1, // [10:10]
                      chk_2k_mode                                             :  1, // [9:9]
                      rty                                                     :  1, // [8:8]
                      bar                                                     :  1, // [7:7]
                      ac                                                      :  2, // [6:5]
                      soft_reorder_enable                                     :  1, // [4:4]
                      disable_duplicate_detection                             :  1, // [3:3]
                      associated_link_descriptor_counter                      :  2, // [2:1]
                      vld                                                     :  1; // [0:0]
             uint32_t pn_valid                                                :  1, // [31:31]
                      reserved_3a                                             :  6, // [30:25]
                      pn_error_detected_flag                                  :  1, // [24:24]
                      seq_2k_error_detected_flag                              :  1, // [23:23]
                      current_index                                           : 10, // [22:13]
                      ssn                                                     : 12, // [12:1]
                      svld                                                    :  1; // [0:0]
             uint32_t pn_31_0                                                 : 32; // [31:0]
             uint32_t pn_63_32                                                : 32; // [31:0]
             uint32_t pn_95_64                                                : 32; // [31:0]
             uint32_t pn_127_96                                               : 32; // [31:0]
             uint32_t last_rx_enqueue_timestamp                               : 32; // [31:0]
             uint32_t last_rx_dequeue_timestamp                               : 32; // [31:0]
             uint32_t ptr_to_next_aging_queue_31_0                            : 32; // [31:0]
             uint32_t reserved_11a                                            : 24, // [31:8]
                      ptr_to_next_aging_queue_39_32                           :  8; // [7:0]
             uint32_t ptr_to_previous_aging_queue_31_0                        : 32; // [31:0]
             uint32_t reserved_13a                                            : 18, // [31:14]
                      statistics_counter_index                                :  6, // [13:8]
                      ptr_to_previous_aging_queue_39_32                       :  8; // [7:0]
             uint32_t rx_bitmap_31_0                                          : 32; // [31:0]
             uint32_t rx_bitmap_63_32                                         : 32; // [31:0]
             uint32_t rx_bitmap_95_64                                         : 32; // [31:0]
             uint32_t rx_bitmap_127_96                                        : 32; // [31:0]
             uint32_t rx_bitmap_159_128                                       : 32; // [31:0]
             uint32_t rx_bitmap_191_160                                       : 32; // [31:0]
             uint32_t rx_bitmap_223_192                                       : 32; // [31:0]
             uint32_t rx_bitmap_255_224                                       : 32; // [31:0]
             uint32_t rx_bitmap_287_256                                       : 32; // [31:0]
             uint32_t current_msdu_count                                      : 25, // [31:7]
                      current_mpdu_count                                      :  7; // [6:0]
             uint32_t duplicate_count                                         : 16, // [31:16]
                      forward_due_to_bar_count                                :  6, // [15:10]
                      timeout_count                                           :  6, // [9:4]
                      last_sn_reg_index                                       :  4; // [3:0]
             uint32_t bar_received_count                                      :  8, // [31:24]
                      frames_in_order_count                                   : 24; // [23:0]
             uint32_t mpdu_frames_processed_count                             : 32; // [31:0]
             uint32_t msdu_frames_processed_count                             : 32; // [31:0]
             uint32_t total_processed_byte_count                              : 32; // [31:0]
             uint32_t hole_count                                              : 16, // [31:16]
                      window_jump_2k                                          :  4, // [15:12]
                      late_receive_mpdu_count                                 : 12; // [11:0]
             uint32_t reserved_30                                             :  8, // [31:24]
                      aging_drop_interval                                     :  8, // [23:16]
                      aging_drop_mpdu_count                                   : 16; // [15:0]
             uint32_t reserved_31                                             : 32; // [31:0]
#endif
};


/* Description		DESCRIPTOR_HEADER

			Details about which module owns this struct.
			Note that sub field "Buffer_type" shall be set to "Receive_REO_queue_descriptor"
			
*/


/* Description		OWNER

			Consumer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO
			Producer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO 
			
			The owner of this data structure:
			<enum 0 WBM_owned> Buffer Manager currently owns this data
			 structure.
			<enum 1 SW_OR_FW_owned> Software of FW currently owns this
			 data structure.
			<enum 2 TQM_owned> Transmit Queue Manager currently owns
			 this data structure.
			<enum 3 RXDMA_owned> Receive DMA currently owns this data
			 structure.
			<enum 4 REO_owned> Reorder currently owns this data structure.
			
			<enum 5 SWITCH_owned> SWITCH currently owns this data structure.
			
			
			<legal 0-5> 
*/

#define RX_REO_QUEUE_DESCRIPTOR_HEADER_OWNER_OFFSET                                 0x00000000
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_OWNER_LSB                                    0
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_OWNER_MSB                                    3
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_OWNER_MASK                                   0x0000000f


/* Description		BUFFER_TYPE

			Consumer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO
			Producer: In DEBUG mode: WBM, TQM, TXDMA, RXDMA, REO 
			
			Field describing what contents format is of this descriptor
			
			
			<enum 0 Transmit_MSDU_Link_descriptor> 
			<enum 1 Transmit_MPDU_Link_descriptor> 
			<enum 2 Transmit_MPDU_Queue_head_descriptor>
			<enum 3 Transmit_MPDU_Queue_ext_descriptor>
			<enum 4 Transmit_flow_descriptor>
			<enum 5 Transmit_buffer> NOT TO BE USED: 
			
			<enum 6 Receive_MSDU_Link_descriptor>
			<enum 7 Receive_MPDU_Link_descriptor>
			<enum 8 Receive_REO_queue_descriptor>
			<enum 9 Receive_REO_queue_1k_descriptor>
			<enum 10 Receive_REO_queue_ext_descriptor>
			
			<enum 11 Receive_buffer>
			
			<enum 12 Idle_link_list_entry>
			
			<legal 0-12> 
*/

#define RX_REO_QUEUE_DESCRIPTOR_HEADER_BUFFER_TYPE_OFFSET                           0x00000000
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_BUFFER_TYPE_LSB                              4
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_BUFFER_TYPE_MSB                              7
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_BUFFER_TYPE_MASK                             0x000000f0


/* Description		TX_MPDU_QUEUE_NUMBER

			Consumer: TQM/Debug
			Producer: SW (in 'TX_MPDU_QUEUE_HEAD')/TQM (elsewhere)
			
			Field only valid if Buffer_type is any of Transmit_MPDU_*_descriptor
			
			
			Indicates the MPDU queue ID to which this MPDU descriptor
			 belongs
			Used for tracking and debugging
			
			Hamilton and Waikiki used bits [19:0] of word 1 of 'TX_MPDU_LINK,' 
			word 16 of 'TX_MPDU_QUEUE_HEAD' and word 1 of 'TX_MPDU_QUEUE_EXT' 
			for this.
			 <legal all>
*/

#define RX_REO_QUEUE_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_OFFSET                  0x00000000
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_LSB                     8
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_MSB                     27
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_TX_MPDU_QUEUE_NUMBER_MASK                    0x0fffff00


/* Description		RESERVED_0A

			<legal 0>
*/

#define RX_REO_QUEUE_DESCRIPTOR_HEADER_RESERVED_0A_OFFSET                           0x00000000
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_RESERVED_0A_LSB                              28
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_RESERVED_0A_MSB                              31
#define RX_REO_QUEUE_DESCRIPTOR_HEADER_RESERVED_0A_MASK                             0xf0000000


/* Description		RECEIVE_QUEUE_NUMBER

			Indicates the MPDU queue ID to which this MPDU link descriptor
			 belongs
			Used for tracking and debugging
			<legal all>
*/

#define RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_OFFSET                                    0x00000004
#define RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_LSB                                       0
#define RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_MSB                                       15
#define RX_REO_QUEUE_RECEIVE_QUEUE_NUMBER_MASK                                      0x0000ffff


/* Description		RESERVED_1B

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_1B_OFFSET                                             0x00000004
#define RX_REO_QUEUE_RESERVED_1B_LSB                                                16
#define RX_REO_QUEUE_RESERVED_1B_MSB                                                31
#define RX_REO_QUEUE_RESERVED_1B_MASK                                               0xffff0000


/* Description		VLD

			Valid bit indicating a session is established and the queue
			 descriptor is valid(Filled by SW)
			<legal all>
*/

#define RX_REO_QUEUE_VLD_OFFSET                                                     0x00000008
#define RX_REO_QUEUE_VLD_LSB                                                        0
#define RX_REO_QUEUE_VLD_MSB                                                        0
#define RX_REO_QUEUE_VLD_MASK                                                       0x00000001


/* Description		ASSOCIATED_LINK_DESCRIPTOR_COUNTER

			Indicates which of the 3 link descriptor counters shall 
			be incremented or decremented when link descriptors are 
			added or removed from this flow queue.
			MSDU link descriptors related with MPDUs stored in the re-order
			 buffer shall also be included in this count.
			
			<legal 0-2>
*/

#define RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_OFFSET                      0x00000008
#define RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_LSB                         1
#define RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MSB                         2
#define RX_REO_QUEUE_ASSOCIATED_LINK_DESCRIPTOR_COUNTER_MASK                        0x00000006


/* Description		DISABLE_DUPLICATE_DETECTION

			When set, do not perform any duplicate detection.
			
			<legal all>
*/

#define RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_OFFSET                             0x00000008
#define RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_LSB                                3
#define RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_MSB                                3
#define RX_REO_QUEUE_DISABLE_DUPLICATE_DETECTION_MASK                               0x00000008


/* Description		SOFT_REORDER_ENABLE

			When set, REO has been instructed to not perform the actual
			 re-ordering of frames for this queue, but just to insert
			 the reorder opcodes.
			
			Note that this implies that REO is also not going to perform
			 any MSDU level operations, and the entire MPDU (and thus
			 pointer to the MSDU link descriptor) will be pushed to 
			a destination ring that SW has programmed in a SW programmable
			 configuration register in REO
			
			<legal all>
*/

#define RX_REO_QUEUE_SOFT_REORDER_ENABLE_OFFSET                                     0x00000008
#define RX_REO_QUEUE_SOFT_REORDER_ENABLE_LSB                                        4
#define RX_REO_QUEUE_SOFT_REORDER_ENABLE_MSB                                        4
#define RX_REO_QUEUE_SOFT_REORDER_ENABLE_MASK                                       0x00000010


/* Description		AC

			Indicates which access category the queue descriptor belongs
			 to(filled by SW)
			<legal all>
*/

#define RX_REO_QUEUE_AC_OFFSET                                                      0x00000008
#define RX_REO_QUEUE_AC_LSB                                                         5
#define RX_REO_QUEUE_AC_MSB                                                         6
#define RX_REO_QUEUE_AC_MASK                                                        0x00000060


/* Description		BAR

			Indicates if  BAR has been received (mostly used for debug
			 purpose and this is filled by REO)
			<legal all>
*/

#define RX_REO_QUEUE_BAR_OFFSET                                                     0x00000008
#define RX_REO_QUEUE_BAR_LSB                                                        7
#define RX_REO_QUEUE_BAR_MSB                                                        7
#define RX_REO_QUEUE_BAR_MASK                                                       0x00000080


/* Description		RTY

			Retry bit is checked if this bit is set.  
			<legal all>
*/

#define RX_REO_QUEUE_RTY_OFFSET                                                     0x00000008
#define RX_REO_QUEUE_RTY_LSB                                                        8
#define RX_REO_QUEUE_RTY_MSB                                                        8
#define RX_REO_QUEUE_RTY_MASK                                                       0x00000100


/* Description		CHK_2K_MODE

			Indicates what type of operation is expected from Reo when
			 the received frame SN falls within the 2K window
			
			See REO MLD document for programming details.
			<legal all>
*/

#define RX_REO_QUEUE_CHK_2K_MODE_OFFSET                                             0x00000008
#define RX_REO_QUEUE_CHK_2K_MODE_LSB                                                9
#define RX_REO_QUEUE_CHK_2K_MODE_MSB                                                9
#define RX_REO_QUEUE_CHK_2K_MODE_MASK                                               0x00000200


/* Description		OOR_MODE

			Out of Order mode:
			Indicates what type of operation is expected when the received
			 frame falls within the OOR window.
			
			See REO MLD document for programming details.
			<legal all>
*/

#define RX_REO_QUEUE_OOR_MODE_OFFSET                                                0x00000008
#define RX_REO_QUEUE_OOR_MODE_LSB                                                   10
#define RX_REO_QUEUE_OOR_MODE_MSB                                                   10
#define RX_REO_QUEUE_OOR_MODE_MASK                                                  0x00000400


/* Description		BA_WINDOW_SIZE

			Indicates the negotiated (window size + 1). 
			It can go up to Max of 256bits.
			
			A value 255 means 256 bitmap, 63 means 64 bitmap, 0 (means
			 non-BA session, with window size of 0). The 3 values here
			 are the main values validated, but other values should 
			work as well.
			
			A value 1023 means 1024 bitmap, 511 means 512 bitmap. The
			 2 values here are the main values validated for 1k-bitmap
			 support, but other values should work as well.
			
			A BA window size of 0 (=> one frame entry bitmap), means
			 that there is NO RX_REO_QUEUE_EXT descriptor following 
			this RX_REO_QUEUE STRUCT in memory
			
			A BA window size of 1 - 105 means that there is 1 RX_REO_QUEUE_EXT
			 descriptor directly following this RX_REO_QUEUE STRUCT 
			in memory.
			
			A BA window size of 106 - 210 means that there are 2 RX_REO_QUEUE_EXT
			 descriptors directly following this RX_REO_QUEUE STRUCT
			 in memory
			
			A BA window size of 211 - 256 means that there are 3 RX_REO_QUEUE_EXT
			 descriptors directly following this RX_REO_QUEUE STRUCT
			 in memory
			
			A BA window size of 257 - 315 means that there is one RX_REO_QUEUE_1K
			 descriptor followed by 3 RX_REO_QUEUE_EXT descriptors directly
			 following this RX_REO_QUEUE STRUCT in memory
			
			A BA window size of 316 - 420 means that there is one RX_REO_QUEUE_1K
			 descriptor followed by 4 RX_REO_QUEUE_EXT descriptors directly
			 following this RX_REO_QUEUE STRUCT in memory
			...
			A BA window size of 946 - 1024 means that there is one RX_REO_QUEUE_1K
			 descriptor followed by 10 RX_REO_QUEUE_EXT descriptors 
			directly following this RX_REO_QUEUE STRUCT in memory
			
			TODO: Should the above text use '255' and '1023' instead
			 of '256' and '1024'?
			<legal 0 - 1023>
*/

#define RX_REO_QUEUE_BA_WINDOW_SIZE_OFFSET                                          0x00000008
#define RX_REO_QUEUE_BA_WINDOW_SIZE_LSB                                             11
#define RX_REO_QUEUE_BA_WINDOW_SIZE_MSB                                             20
#define RX_REO_QUEUE_BA_WINDOW_SIZE_MASK                                            0x001ff800


/* Description		PN_CHECK_NEEDED

			When set, REO shall perform the PN increment check
			<legal all>
*/

#define RX_REO_QUEUE_PN_CHECK_NEEDED_OFFSET                                         0x00000008
#define RX_REO_QUEUE_PN_CHECK_NEEDED_LSB                                            21
#define RX_REO_QUEUE_PN_CHECK_NEEDED_MSB                                            21
#define RX_REO_QUEUE_PN_CHECK_NEEDED_MASK                                           0x00200000


/* Description		PN_SHALL_BE_EVEN

			Field only valid when 'pn_check_needed' is set.
			
			When set, REO shall confirm that the received PN number 
			is not only incremented, but also always an even number
			<legal all>
*/

#define RX_REO_QUEUE_PN_SHALL_BE_EVEN_OFFSET                                        0x00000008
#define RX_REO_QUEUE_PN_SHALL_BE_EVEN_LSB                                           22
#define RX_REO_QUEUE_PN_SHALL_BE_EVEN_MSB                                           22
#define RX_REO_QUEUE_PN_SHALL_BE_EVEN_MASK                                          0x00400000


/* Description		PN_SHALL_BE_UNEVEN

			Field only valid when 'pn_check_needed' is set.
			
			When set, REO shall confirm that the received PN number 
			is not only incremented, but also always an uneven number
			
			<legal all>
*/

#define RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_OFFSET                                      0x00000008
#define RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_LSB                                         23
#define RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_MSB                                         23
#define RX_REO_QUEUE_PN_SHALL_BE_UNEVEN_MASK                                        0x00800000


/* Description		PN_HANDLING_ENABLE

			Field only valid when 'pn_check_needed' is set.
			
			When set, and REO detected a PN error, HW shall set the 'pn_error_detected_flag'.
			
			<legal all>
*/

#define RX_REO_QUEUE_PN_HANDLING_ENABLE_OFFSET                                      0x00000008
#define RX_REO_QUEUE_PN_HANDLING_ENABLE_LSB                                         24
#define RX_REO_QUEUE_PN_HANDLING_ENABLE_MSB                                         24
#define RX_REO_QUEUE_PN_HANDLING_ENABLE_MASK                                        0x01000000


/* Description		PN_SIZE

			Size of the PN field check.
			Needed for wrap around handling...
			
			<enum 0     pn_size_24>
			<enum 1     pn_size_48>
			<enum 2     pn_size_128>
			
			<legal 0-2>
*/

#define RX_REO_QUEUE_PN_SIZE_OFFSET                                                 0x00000008
#define RX_REO_QUEUE_PN_SIZE_LSB                                                    25
#define RX_REO_QUEUE_PN_SIZE_MSB                                                    26
#define RX_REO_QUEUE_PN_SIZE_MASK                                                   0x06000000


/* Description		IGNORE_AMPDU_FLAG

			When set, REO shall ignore the ampdu_flag on the entrance
			 descriptor for this queue.
			<legal all>
*/

#define RX_REO_QUEUE_IGNORE_AMPDU_FLAG_OFFSET                                       0x00000008
#define RX_REO_QUEUE_IGNORE_AMPDU_FLAG_LSB                                          27
#define RX_REO_QUEUE_IGNORE_AMPDU_FLAG_MSB                                          27
#define RX_REO_QUEUE_IGNORE_AMPDU_FLAG_MASK                                         0x08000000


/* Description		RESERVED_2B

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_2B_OFFSET                                             0x00000008
#define RX_REO_QUEUE_RESERVED_2B_LSB                                                28
#define RX_REO_QUEUE_RESERVED_2B_MSB                                                31
#define RX_REO_QUEUE_RESERVED_2B_MASK                                               0xf0000000


/* Description		SVLD

			Sequence number in next field is valid one. It can be filled
			 by SW if the want to fill in the any negotiated SSN, otherwise
			 REO will fill the sequence number of first received packet
			 and set this bit to 1.
			<legal all>
*/

#define RX_REO_QUEUE_SVLD_OFFSET                                                    0x0000000c
#define RX_REO_QUEUE_SVLD_LSB                                                       0
#define RX_REO_QUEUE_SVLD_MSB                                                       0
#define RX_REO_QUEUE_SVLD_MASK                                                      0x00000001


/* Description		SSN

			Starting Sequence number of the session, this changes whenever
			 window moves. (can be filled by SW then maintained by REO)
			
			<legal all>
*/

#define RX_REO_QUEUE_SSN_OFFSET                                                     0x0000000c
#define RX_REO_QUEUE_SSN_LSB                                                        1
#define RX_REO_QUEUE_SSN_MSB                                                        12
#define RX_REO_QUEUE_SSN_MASK                                                       0x00001ffe


/* Description		CURRENT_INDEX

			Points to last forwarded packet
			<legal all>
*/

#define RX_REO_QUEUE_CURRENT_INDEX_OFFSET                                           0x0000000c
#define RX_REO_QUEUE_CURRENT_INDEX_LSB                                              13
#define RX_REO_QUEUE_CURRENT_INDEX_MSB                                              22
#define RX_REO_QUEUE_CURRENT_INDEX_MASK                                             0x007fe000


/* Description		SEQ_2K_ERROR_DETECTED_FLAG

			Set by REO, can only be cleared by SW
			
			When set, REO has detected a 2k error jump in the sequence
			 number and from that moment forward, all new frames are
			 forwarded directly to FW, without duplicate detect, reordering, 
			etc.
			<legal all>
*/

#define RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_OFFSET                              0x0000000c
#define RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_LSB                                 23
#define RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_MSB                                 23
#define RX_REO_QUEUE_SEQ_2K_ERROR_DETECTED_FLAG_MASK                                0x00800000


/* Description		PN_ERROR_DETECTED_FLAG

			Set by REO, can only be cleared by SW
			
			When set, REO has detected a PN error and from that moment
			 forward, all new frames are forwarded directly to FW, without
			 duplicate detect, reordering, etc.
			<legal all>
*/

#define RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_OFFSET                                  0x0000000c
#define RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_LSB                                     24
#define RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_MSB                                     24
#define RX_REO_QUEUE_PN_ERROR_DETECTED_FLAG_MASK                                    0x01000000


/* Description		RESERVED_3A

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_3A_OFFSET                                             0x0000000c
#define RX_REO_QUEUE_RESERVED_3A_LSB                                                25
#define RX_REO_QUEUE_RESERVED_3A_MSB                                                30
#define RX_REO_QUEUE_RESERVED_3A_MASK                                               0x7e000000


/* Description		PN_VALID

			PN number in next fields are valid. It can be filled by 
			SW if it wants to fill in the any negotiated SSN, otherwise
			 REO will fill the pn based on the first received packet
			 and set this bit to 1.
			<legal all>
*/

#define RX_REO_QUEUE_PN_VALID_OFFSET                                                0x0000000c
#define RX_REO_QUEUE_PN_VALID_LSB                                                   31
#define RX_REO_QUEUE_PN_VALID_MSB                                                   31
#define RX_REO_QUEUE_PN_VALID_MASK                                                  0x80000000


/* Description		PN_31_0

			Bits [31:0] of the PN number extracted from the IV field
			
			<legal all>
*/

#define RX_REO_QUEUE_PN_31_0_OFFSET                                                 0x00000010
#define RX_REO_QUEUE_PN_31_0_LSB                                                    0
#define RX_REO_QUEUE_PN_31_0_MSB                                                    31
#define RX_REO_QUEUE_PN_31_0_MASK                                                   0xffffffff


/* Description		PN_63_32

			Bits [63:32] of the PN number.  
			<legal all> 
*/

#define RX_REO_QUEUE_PN_63_32_OFFSET                                                0x00000014
#define RX_REO_QUEUE_PN_63_32_LSB                                                   0
#define RX_REO_QUEUE_PN_63_32_MSB                                                   31
#define RX_REO_QUEUE_PN_63_32_MASK                                                  0xffffffff


/* Description		PN_95_64

			Bits [95:64] of the PN number.  
			<legal all>
*/

#define RX_REO_QUEUE_PN_95_64_OFFSET                                                0x00000018
#define RX_REO_QUEUE_PN_95_64_LSB                                                   0
#define RX_REO_QUEUE_PN_95_64_MSB                                                   31
#define RX_REO_QUEUE_PN_95_64_MASK                                                  0xffffffff


/* Description		PN_127_96

			Bits [127:96] of the PN number.  
			<legal all>
*/

#define RX_REO_QUEUE_PN_127_96_OFFSET                                               0x0000001c
#define RX_REO_QUEUE_PN_127_96_LSB                                                  0
#define RX_REO_QUEUE_PN_127_96_MSB                                                  31
#define RX_REO_QUEUE_PN_127_96_MASK                                                 0xffffffff


/* Description		LAST_RX_ENQUEUE_TIMESTAMP

			This timestamp is updated when an MPDU is received and accesses
			 this Queue Descriptor. It does not include the access due
			 to Command TLVs or Aging (which will be updated in Last_rx_dequeue_timestamp).
			
			<legal all>
*/

#define RX_REO_QUEUE_LAST_RX_ENQUEUE_TIMESTAMP_OFFSET                               0x00000020
#define RX_REO_QUEUE_LAST_RX_ENQUEUE_TIMESTAMP_LSB                                  0
#define RX_REO_QUEUE_LAST_RX_ENQUEUE_TIMESTAMP_MSB                                  31
#define RX_REO_QUEUE_LAST_RX_ENQUEUE_TIMESTAMP_MASK                                 0xffffffff


/* Description		LAST_RX_DEQUEUE_TIMESTAMP

			This timestamp is used for Aging. When an MPDU or multiple
			 MPDUs are forwarded, either due to window movement, bar, 
			aging or command flush, this timestamp is updated. Also 
			when the bitmap is all zero and the first time an MPDU is
			 queued (opcode=QCUR), this timestamp is updated for aging.
			
			<legal all>
*/

#define RX_REO_QUEUE_LAST_RX_DEQUEUE_TIMESTAMP_OFFSET                               0x00000024
#define RX_REO_QUEUE_LAST_RX_DEQUEUE_TIMESTAMP_LSB                                  0
#define RX_REO_QUEUE_LAST_RX_DEQUEUE_TIMESTAMP_MSB                                  31
#define RX_REO_QUEUE_LAST_RX_DEQUEUE_TIMESTAMP_MASK                                 0xffffffff


/* Description		PTR_TO_NEXT_AGING_QUEUE_31_0

			Address  (address bits 31-0)of next RX_REO_QUEUE descriptor
			 in the 'receive timestamp' ordered list.
			From it the Position of this queue descriptor in the per
			 AC aging waitlist  can be derived.
			Value 0x0 indicates the 'NULL' pointer which implies that
			 this is the last entry in the list.
			<legal all>
*/

#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_31_0_OFFSET                            0x00000028
#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_31_0_LSB                               0
#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_31_0_MSB                               31
#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_31_0_MASK                              0xffffffff


/* Description		PTR_TO_NEXT_AGING_QUEUE_39_32

			Address  (address bits 39-32)of next RX_REO_QUEUE descriptor
			 in the 'receive timestamp' ordered list.
			From it the Position of this queue descriptor in the per
			 AC aging waitlist  can be derived.
			Value 0x0 indicates the 'NULL' pointer which implies that
			 this is the last entry in the list.
			<legal all>
*/

#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_39_32_OFFSET                           0x0000002c
#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_39_32_LSB                              0
#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_39_32_MSB                              7
#define RX_REO_QUEUE_PTR_TO_NEXT_AGING_QUEUE_39_32_MASK                             0x000000ff


/* Description		RESERVED_11A

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_11A_OFFSET                                            0x0000002c
#define RX_REO_QUEUE_RESERVED_11A_LSB                                               8
#define RX_REO_QUEUE_RESERVED_11A_MSB                                               31
#define RX_REO_QUEUE_RESERVED_11A_MASK                                              0xffffff00


/* Description		PTR_TO_PREVIOUS_AGING_QUEUE_31_0

			Address  (address bits 31-0)of next RX_REO_QUEUE descriptor
			 in the 'receive timestamp' ordered list.
			From it the Position of this queue descriptor in the per
			 AC aging waitlist  can be derived.
			Value 0x0 indicates the 'NULL' pointer which implies that
			 this is the first entry in the list.
			<legal all>
*/

#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_OFFSET                        0x00000030
#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_LSB                           0
#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_MSB                           31
#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_31_0_MASK                          0xffffffff


/* Description		PTR_TO_PREVIOUS_AGING_QUEUE_39_32

			Address  (address bits 39-32)of next RX_REO_QUEUE descriptor
			 in the 'receive timestamp' ordered list.
			From it the Position of this queue descriptor in the per
			 AC aging waitlist  can be derived.
			Value 0x0 indicates the 'NULL' pointer which implies that
			 this is the first entry in the list.
			<legal all>
*/

#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_OFFSET                       0x00000034
#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_LSB                          0
#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_MSB                          7
#define RX_REO_QUEUE_PTR_TO_PREVIOUS_AGING_QUEUE_39_32_MASK                         0x000000ff


/* Description		STATISTICS_COUNTER_INDEX

			This is used to select one of the REO register sets for 
			tracking statistics—MSDU count and MSDU byte count in 
			Waikiki (Not supported in Hamilton).
			
			Usually all the queues pertaining to one virtual device 
			use one statistics register set, and each virtual device
			 maps to a different set in case of not too many virtual
			 devices.
			<legal 0-47>
*/

#define RX_REO_QUEUE_STATISTICS_COUNTER_INDEX_OFFSET                                0x00000034
#define RX_REO_QUEUE_STATISTICS_COUNTER_INDEX_LSB                                   8
#define RX_REO_QUEUE_STATISTICS_COUNTER_INDEX_MSB                                   13
#define RX_REO_QUEUE_STATISTICS_COUNTER_INDEX_MASK                                  0x00003f00


/* Description		RESERVED_13A

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_13A_OFFSET                                            0x00000034
#define RX_REO_QUEUE_RESERVED_13A_LSB                                               14
#define RX_REO_QUEUE_RESERVED_13A_MSB                                               31
#define RX_REO_QUEUE_RESERVED_13A_MASK                                              0xffffc000


/* Description		RX_BITMAP_31_0

			When a bit is set, the corresponding frame is currently 
			held in the re-order queue.
			The bitmap  is Fully managed by HW. 
			SW shall init this to 0, and then never ever change it
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_31_0_OFFSET                                          0x00000038
#define RX_REO_QUEUE_RX_BITMAP_31_0_LSB                                             0
#define RX_REO_QUEUE_RX_BITMAP_31_0_MSB                                             31
#define RX_REO_QUEUE_RX_BITMAP_31_0_MASK                                            0xffffffff


/* Description		RX_BITMAP_63_32

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_63_32_OFFSET                                         0x0000003c
#define RX_REO_QUEUE_RX_BITMAP_63_32_LSB                                            0
#define RX_REO_QUEUE_RX_BITMAP_63_32_MSB                                            31
#define RX_REO_QUEUE_RX_BITMAP_63_32_MASK                                           0xffffffff


/* Description		RX_BITMAP_95_64

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_95_64_OFFSET                                         0x00000040
#define RX_REO_QUEUE_RX_BITMAP_95_64_LSB                                            0
#define RX_REO_QUEUE_RX_BITMAP_95_64_MSB                                            31
#define RX_REO_QUEUE_RX_BITMAP_95_64_MASK                                           0xffffffff


/* Description		RX_BITMAP_127_96

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_127_96_OFFSET                                        0x00000044
#define RX_REO_QUEUE_RX_BITMAP_127_96_LSB                                           0
#define RX_REO_QUEUE_RX_BITMAP_127_96_MSB                                           31
#define RX_REO_QUEUE_RX_BITMAP_127_96_MASK                                          0xffffffff


/* Description		RX_BITMAP_159_128

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_159_128_OFFSET                                       0x00000048
#define RX_REO_QUEUE_RX_BITMAP_159_128_LSB                                          0
#define RX_REO_QUEUE_RX_BITMAP_159_128_MSB                                          31
#define RX_REO_QUEUE_RX_BITMAP_159_128_MASK                                         0xffffffff


/* Description		RX_BITMAP_191_160

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_191_160_OFFSET                                       0x0000004c
#define RX_REO_QUEUE_RX_BITMAP_191_160_LSB                                          0
#define RX_REO_QUEUE_RX_BITMAP_191_160_MSB                                          31
#define RX_REO_QUEUE_RX_BITMAP_191_160_MASK                                         0xffffffff


/* Description		RX_BITMAP_223_192

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_223_192_OFFSET                                       0x00000050
#define RX_REO_QUEUE_RX_BITMAP_223_192_LSB                                          0
#define RX_REO_QUEUE_RX_BITMAP_223_192_MSB                                          31
#define RX_REO_QUEUE_RX_BITMAP_223_192_MASK                                         0xffffffff


/* Description		RX_BITMAP_255_224

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_255_224_OFFSET                                       0x00000054
#define RX_REO_QUEUE_RX_BITMAP_255_224_LSB                                          0
#define RX_REO_QUEUE_RX_BITMAP_255_224_MSB                                          31
#define RX_REO_QUEUE_RX_BITMAP_255_224_MASK                                         0xffffffff


/* Description		RX_BITMAP_287_256

			See Rx_bitmap_31_0 description
			<legal all>
*/

#define RX_REO_QUEUE_RX_BITMAP_287_256_OFFSET                                       0x00000058
#define RX_REO_QUEUE_RX_BITMAP_287_256_LSB                                          0
#define RX_REO_QUEUE_RX_BITMAP_287_256_MSB                                          31
#define RX_REO_QUEUE_RX_BITMAP_287_256_MASK                                         0xffffffff


/* Description		CURRENT_MPDU_COUNT

			The number of MPDUs in the queue.
			
			<legal all>
*/

#define RX_REO_QUEUE_CURRENT_MPDU_COUNT_OFFSET                                      0x0000005c
#define RX_REO_QUEUE_CURRENT_MPDU_COUNT_LSB                                         0
#define RX_REO_QUEUE_CURRENT_MPDU_COUNT_MSB                                         6
#define RX_REO_QUEUE_CURRENT_MPDU_COUNT_MASK                                        0x0000007f


/* Description		CURRENT_MSDU_COUNT

			The number of MSDUs in the queue.
			<legal all>
*/

#define RX_REO_QUEUE_CURRENT_MSDU_COUNT_OFFSET                                      0x0000005c
#define RX_REO_QUEUE_CURRENT_MSDU_COUNT_LSB                                         7
#define RX_REO_QUEUE_CURRENT_MSDU_COUNT_MSB                                         31
#define RX_REO_QUEUE_CURRENT_MSDU_COUNT_MASK                                        0xffffff80


/* Description		LAST_SN_REG_INDEX

			REO has registers to save the last SN seen in up to 9 REO
			 queues, to support "leaky APs."
			
			This field gives the register number to use for saving the
			 last SN of this REO queue.
			<legal 0-8>
*/

#define RX_REO_QUEUE_LAST_SN_REG_INDEX_OFFSET                                       0x00000060
#define RX_REO_QUEUE_LAST_SN_REG_INDEX_LSB                                          0
#define RX_REO_QUEUE_LAST_SN_REG_INDEX_MSB                                          3
#define RX_REO_QUEUE_LAST_SN_REG_INDEX_MASK                                         0x0000000f


/* Description		TIMEOUT_COUNT

			The number of times that REO started forwarding frames even
			 though there is a hole in the bitmap. Forwarding reason
			 is Timeout
			
			The counter saturates and freezes at 0x3F
			
			<legal all>
*/

#define RX_REO_QUEUE_TIMEOUT_COUNT_OFFSET                                           0x00000060
#define RX_REO_QUEUE_TIMEOUT_COUNT_LSB                                              4
#define RX_REO_QUEUE_TIMEOUT_COUNT_MSB                                              9
#define RX_REO_QUEUE_TIMEOUT_COUNT_MASK                                             0x000003f0


/* Description		FORWARD_DUE_TO_BAR_COUNT

			The number of times that REO started forwarding frames even
			 though there is a hole in the bitmap. Forwarding reason
			 is reception of BAR frame.
			
			The counter saturates and freezes at 0x3F
			
			<legal all>
*/

#define RX_REO_QUEUE_FORWARD_DUE_TO_BAR_COUNT_OFFSET                                0x00000060
#define RX_REO_QUEUE_FORWARD_DUE_TO_BAR_COUNT_LSB                                   10
#define RX_REO_QUEUE_FORWARD_DUE_TO_BAR_COUNT_MSB                                   15
#define RX_REO_QUEUE_FORWARD_DUE_TO_BAR_COUNT_MASK                                  0x0000fc00


/* Description		DUPLICATE_COUNT

			The number of duplicate frames that have been detected
			<legal all>
*/

#define RX_REO_QUEUE_DUPLICATE_COUNT_OFFSET                                         0x00000060
#define RX_REO_QUEUE_DUPLICATE_COUNT_LSB                                            16
#define RX_REO_QUEUE_DUPLICATE_COUNT_MSB                                            31
#define RX_REO_QUEUE_DUPLICATE_COUNT_MASK                                           0xffff0000


/* Description		FRAMES_IN_ORDER_COUNT

			The number of frames that have been received in order (without
			 a hole that prevented them from being forwarded immediately)
			
			
			This corresponds to the Reorder opcodes:
			'FWDCUR' and 'FWD BUF'
			
			<legal all>
*/

#define RX_REO_QUEUE_FRAMES_IN_ORDER_COUNT_OFFSET                                   0x00000064
#define RX_REO_QUEUE_FRAMES_IN_ORDER_COUNT_LSB                                      0
#define RX_REO_QUEUE_FRAMES_IN_ORDER_COUNT_MSB                                      23
#define RX_REO_QUEUE_FRAMES_IN_ORDER_COUNT_MASK                                     0x00ffffff


/* Description		BAR_RECEIVED_COUNT

			The number of times a BAR frame is received.
			
			This corresponds to the Reorder opcodes with 'DROP'
			
			The counter saturates and freezes at 0xFF
			<legal all>
*/

#define RX_REO_QUEUE_BAR_RECEIVED_COUNT_OFFSET                                      0x00000064
#define RX_REO_QUEUE_BAR_RECEIVED_COUNT_LSB                                         24
#define RX_REO_QUEUE_BAR_RECEIVED_COUNT_MSB                                         31
#define RX_REO_QUEUE_BAR_RECEIVED_COUNT_MASK                                        0xff000000


/* Description		MPDU_FRAMES_PROCESSED_COUNT

			The total number of MPDU frames that have been processed
			 by REO. 'Processing' here means that REO has received them
			 out of the entrance ring, and retrieved the corresponding
			 RX_REO_QUEUE Descriptor. 
			
			Note that this count includes duplicates, frames that later
			 had errors, etc.
			
			Note that field 'Duplicate_count' indicates how many of 
			these MPDUs were duplicates.
			
			<legal all>
*/

#define RX_REO_QUEUE_MPDU_FRAMES_PROCESSED_COUNT_OFFSET                             0x00000068
#define RX_REO_QUEUE_MPDU_FRAMES_PROCESSED_COUNT_LSB                                0
#define RX_REO_QUEUE_MPDU_FRAMES_PROCESSED_COUNT_MSB                                31
#define RX_REO_QUEUE_MPDU_FRAMES_PROCESSED_COUNT_MASK                               0xffffffff


/* Description		MSDU_FRAMES_PROCESSED_COUNT

			The total number of MSDU frames that have been processed
			 by REO. 'Processing' here means that REO has received them
			 out of the entrance ring, and retrieved the corresponding
			 RX_REO_QUEUE Descriptor. 
			
			Note that this count includes duplicates, frames that later
			 had errors, etc.
			
			<legal all>
*/

#define RX_REO_QUEUE_MSDU_FRAMES_PROCESSED_COUNT_OFFSET                             0x0000006c
#define RX_REO_QUEUE_MSDU_FRAMES_PROCESSED_COUNT_LSB                                0
#define RX_REO_QUEUE_MSDU_FRAMES_PROCESSED_COUNT_MSB                                31
#define RX_REO_QUEUE_MSDU_FRAMES_PROCESSED_COUNT_MASK                               0xffffffff


/* Description		TOTAL_PROCESSED_BYTE_COUNT

			An approximation of the number of bytes processed for this
			 queue. 
			'Processing' here means that REO has received them out of
			 the entrance ring, and retrieved the corresponding RX_REO_QUEUE
			 Descriptor. 
			
			Note that this count includes duplicates, frames that later
			 had errors, etc.
			
			In 64 byte units
			<legal all>
*/

#define RX_REO_QUEUE_TOTAL_PROCESSED_BYTE_COUNT_OFFSET                              0x00000070
#define RX_REO_QUEUE_TOTAL_PROCESSED_BYTE_COUNT_LSB                                 0
#define RX_REO_QUEUE_TOTAL_PROCESSED_BYTE_COUNT_MSB                                 31
#define RX_REO_QUEUE_TOTAL_PROCESSED_BYTE_COUNT_MASK                                0xffffffff


/* Description		LATE_RECEIVE_MPDU_COUNT

			The number of MPDUs received after the window had already
			 moved on. The 'late' sequence window is defined as (Window
			 SSN - 256) - (Window SSN - 1)
			
			This corresponds with Out of order detection in duplicate
			 detect FSM
			
			The counter saturates and freezes at 0xFFF
			
			<legal all>
*/

#define RX_REO_QUEUE_LATE_RECEIVE_MPDU_COUNT_OFFSET                                 0x00000074
#define RX_REO_QUEUE_LATE_RECEIVE_MPDU_COUNT_LSB                                    0
#define RX_REO_QUEUE_LATE_RECEIVE_MPDU_COUNT_MSB                                    11
#define RX_REO_QUEUE_LATE_RECEIVE_MPDU_COUNT_MASK                                   0x00000fff


/* Description		WINDOW_JUMP_2K

			The number of times the window moved more then 2K
			
			The counter saturates and freezes at 0xF
			
			(Note: field name can not start with number: previous 2k_window_jump)
			
			
			<legal all>
*/

#define RX_REO_QUEUE_WINDOW_JUMP_2K_OFFSET                                          0x00000074
#define RX_REO_QUEUE_WINDOW_JUMP_2K_LSB                                             12
#define RX_REO_QUEUE_WINDOW_JUMP_2K_MSB                                             15
#define RX_REO_QUEUE_WINDOW_JUMP_2K_MASK                                            0x0000f000


/* Description		HOLE_COUNT

			The number of times a hole was created in the receive bitmap.
			
			
			This corresponds to the Reorder opcodes with 'QCUR'
			
			<legal all>
*/

#define RX_REO_QUEUE_HOLE_COUNT_OFFSET                                              0x00000074
#define RX_REO_QUEUE_HOLE_COUNT_LSB                                                 16
#define RX_REO_QUEUE_HOLE_COUNT_MSB                                                 31
#define RX_REO_QUEUE_HOLE_COUNT_MASK                                                0xffff0000


/* Description		AGING_DROP_MPDU_COUNT

			The number of holes in the bitmap that moved due to aging
			 counter expiry
			<legal all>
*/

#define RX_REO_QUEUE_AGING_DROP_MPDU_COUNT_OFFSET                                   0x00000078
#define RX_REO_QUEUE_AGING_DROP_MPDU_COUNT_LSB                                      0
#define RX_REO_QUEUE_AGING_DROP_MPDU_COUNT_MSB                                      15
#define RX_REO_QUEUE_AGING_DROP_MPDU_COUNT_MASK                                     0x0000ffff


/* Description		AGING_DROP_INTERVAL

			The number of times holes got removed from the bitmap due
			 to aging counter expiry
			<legal all>
*/

#define RX_REO_QUEUE_AGING_DROP_INTERVAL_OFFSET                                     0x00000078
#define RX_REO_QUEUE_AGING_DROP_INTERVAL_LSB                                        16
#define RX_REO_QUEUE_AGING_DROP_INTERVAL_MSB                                        23
#define RX_REO_QUEUE_AGING_DROP_INTERVAL_MASK                                       0x00ff0000


/* Description		RESERVED_30

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_30_OFFSET                                             0x00000078
#define RX_REO_QUEUE_RESERVED_30_LSB                                                24
#define RX_REO_QUEUE_RESERVED_30_MSB                                                31
#define RX_REO_QUEUE_RESERVED_30_MASK                                               0xff000000


/* Description		RESERVED_31

			<legal 0>
*/

#define RX_REO_QUEUE_RESERVED_31_OFFSET                                             0x0000007c
#define RX_REO_QUEUE_RESERVED_31_LSB                                                0
#define RX_REO_QUEUE_RESERVED_31_MSB                                                31
#define RX_REO_QUEUE_RESERVED_31_MASK                                               0xffffffff



#endif   // RX_REO_QUEUE

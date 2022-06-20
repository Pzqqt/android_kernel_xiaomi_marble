
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

 
 
 
 
 
 
 


#ifndef _RU_ALLOCATION_160_INFO_H_
#define _RU_ALLOCATION_160_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_RU_ALLOCATION_160_INFO 4


struct ru_allocation_160_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t ru_allocation_band0_0                                   :  9, // [8:0]
                      ru_allocation_band0_1                                   :  9, // [17:9]
                      reserved_0a                                             :  6, // [23:18]
                      ru_allocations_01_subband80_mask                        :  4, // [27:24]
                      ru_allocations_23_subband80_mask                        :  4; // [31:28]
             uint32_t ru_allocation_band0_2                                   :  9, // [8:0]
                      ru_allocation_band0_3                                   :  9, // [17:9]
                      reserved_1a                                             : 14; // [31:18]
             uint32_t ru_allocation_band1_0                                   :  9, // [8:0]
                      ru_allocation_band1_1                                   :  9, // [17:9]
                      reserved_2a                                             : 14; // [31:18]
             uint32_t ru_allocation_band1_2                                   :  9, // [8:0]
                      ru_allocation_band1_3                                   :  9, // [17:9]
                      reserved_3a                                             : 14; // [31:18]
#else
             uint32_t ru_allocations_23_subband80_mask                        :  4, // [31:28]
                      ru_allocations_01_subband80_mask                        :  4, // [27:24]
                      reserved_0a                                             :  6, // [23:18]
                      ru_allocation_band0_1                                   :  9, // [17:9]
                      ru_allocation_band0_0                                   :  9; // [8:0]
             uint32_t reserved_1a                                             : 14, // [31:18]
                      ru_allocation_band0_3                                   :  9, // [17:9]
                      ru_allocation_band0_2                                   :  9; // [8:0]
             uint32_t reserved_2a                                             : 14, // [31:18]
                      ru_allocation_band1_1                                   :  9, // [17:9]
                      ru_allocation_band1_0                                   :  9; // [8:0]
             uint32_t reserved_3a                                             : 14, // [31:18]
                      ru_allocation_band1_3                                   :  9, // [17:9]
                      ru_allocation_band1_2                                   :  9; // [8:0]
#endif
};


/* Description		RU_ALLOCATION_BAND0_0

			Field not used for MIMO
			
			Indicates RU arrangement in frequency domain. RU allocated
			 for MU-MIMO, and number of users in the MU-MIMO.
			0 - valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320
			1 - valid for HE_80/HE_160 (80+80)/ EHT_80/EHT_160/EHT_240/EHT_320
			
			2 - valid for HE_160 (80+80)/ EHT_160/EHT_240/EHT_320
			3 - valid for HE_160 (80+80)/ EHT_160/EHT_240/EHT_320
			
			The four bands are for HE_SIGB0 & B1 respectively or for
			 EHT_SIG0, EHT_SIG1, EHT_SIG2 & EHT_SIG3 respectively.
			
			valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320 packets and denotes RU-map of the first 
			20MHz band of HE_SIGB0 or EHT_SIG0
			<legal all>
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_OFFSET                         0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_0_MASK                           0x000001ff


/* Description		RU_ALLOCATION_BAND0_1

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_80/HE_160/ EHT_80/EHT_160/EHT_240/EHT_320 packets
			 and denotes RU-map of the second 20MHz band of HE_SIGB0
			 or EHT_SIG0
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_OFFSET                         0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_1_MASK                           0x0003fe00


/* Description		RESERVED_0A

			<legal 0>
*/

#define RU_ALLOCATION_160_INFO_RESERVED_0A_OFFSET                                   0x00000000
#define RU_ALLOCATION_160_INFO_RESERVED_0A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_0A_MSB                                      23
#define RU_ALLOCATION_160_INFO_RESERVED_0A_MASK                                     0x00fc0000


/* Description		RU_ALLOCATIONS_01_SUBBAND80_MASK

			Field not used for HE
			
			Indicates what 80 MHz subbands 'ru_allocation_band{0, 1}_{0, 
			1}' are valid for
			Bit 0: lowest 80 MHz
			Bit 1: 2nd lowest 80 MHz
			Bit 2: 2nd highest 80 MHz
			Bit 3: highest 80 MHz
			
			In other 80 MHz subbands PHY microcode should override these
			 with 'zero-user RU996.'
			<legal all>
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_OFFSET              0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_LSB                 24
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_MSB                 27
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_01_SUBBAND80_MASK_MASK                0x0f000000


/* Description		RU_ALLOCATIONS_23_SUBBAND80_MASK

			Field not used for HE
			
			Indicates what 80 MHz subbands 'ru_allocation_band{0, 1}_{2, 
			3}' are valid for
			Bit 0: lowest 80 MHz
			Bit 1: 2nd lowest 80 MHz
			Bit 2: 2nd highest 80 MHz
			Bit 3: highest 80 MHz
			
			In other 80 MHz subbands PHY microcode should override these
			 with 'zero-user RU996.'
			<legal all>
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_OFFSET              0x00000000
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_LSB                 28
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_MSB                 31
#define RU_ALLOCATION_160_INFO_RU_ALLOCATIONS_23_SUBBAND80_MASK_MASK                0xf0000000


/* Description		RU_ALLOCATION_BAND0_2

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the third 20MHz band of HE_SIGB0 or EHT_SIG0
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_OFFSET                         0x00000004
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_2_MASK                           0x000001ff


/* Description		RU_ALLOCATION_BAND0_3

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the fourth 20MHz band of HE_SIGB0 or EHT_SIG0
			
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_OFFSET                         0x00000004
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND0_3_MASK                           0x0003fe00


/* Description		RESERVED_1A

			<legal 0>
*/

#define RU_ALLOCATION_160_INFO_RESERVED_1A_OFFSET                                   0x00000004
#define RU_ALLOCATION_160_INFO_RESERVED_1A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_1A_MSB                                      31
#define RU_ALLOCATION_160_INFO_RESERVED_1A_MASK                                     0xfffc0000


/* Description		RU_ALLOCATION_BAND1_0

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_40/HE_80/HE_160/ EHT_40/EHT_80/EHT_160/ EHT_240/EHT_320
			 packets and denotes RU-map of the first 20MHz band of HE_SIGB1
			 or EHT_SIG1
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_OFFSET                         0x00000008
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_0_MASK                           0x000001ff


/* Description		RU_ALLOCATION_BAND1_1

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_80/HE_160/ EHT_80/EHT_160/EHT_240/EHT_320 packets
			 and denotes RU-map of the second 20MHz band of HE_SIGB1
			 or EHT_SIG1
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_OFFSET                         0x00000008
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_1_MASK                           0x0003fe00


/* Description		RESERVED_2A

			<legal 0>
*/

#define RU_ALLOCATION_160_INFO_RESERVED_2A_OFFSET                                   0x00000008
#define RU_ALLOCATION_160_INFO_RESERVED_2A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_2A_MSB                                      31
#define RU_ALLOCATION_160_INFO_RESERVED_2A_MASK                                     0xfffc0000


/* Description		RU_ALLOCATION_BAND1_2

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the third 20MHz band of HE_SIGB1 or EHT_SIG1
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_OFFSET                         0x0000000c
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_LSB                            0
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_MSB                            8
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_2_MASK                           0x000001ff


/* Description		RU_ALLOCATION_BAND1_3

			Field not used for MIMO
			
			See description of ru_allocation_band0_0
			
			valid for HE_160/ EHT_160/EHT_240/EHT_320 packets and denotes
			 RU-map of the fourth 20MHz band of HE_SIGB1 or EHT_SIG1
			
*/

#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_OFFSET                         0x0000000c
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_LSB                            9
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_MSB                            17
#define RU_ALLOCATION_160_INFO_RU_ALLOCATION_BAND1_3_MASK                           0x0003fe00


/* Description		RESERVED_3A

			<legal 0>
*/

#define RU_ALLOCATION_160_INFO_RESERVED_3A_OFFSET                                   0x0000000c
#define RU_ALLOCATION_160_INFO_RESERVED_3A_LSB                                      18
#define RU_ALLOCATION_160_INFO_RESERVED_3A_MSB                                      31
#define RU_ALLOCATION_160_INFO_RESERVED_3A_MASK                                     0xfffc0000



#endif   // RU_ALLOCATION_160_INFO

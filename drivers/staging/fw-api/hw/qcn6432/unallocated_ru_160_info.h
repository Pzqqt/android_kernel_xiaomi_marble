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

#ifndef _UNALLOCATED_RU_160_INFO_H_
#define _UNALLOCATED_RU_160_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_UNALLOCATED_RU_160_INFO 1


struct unallocated_ru_160_info {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t subband80_0_cc0                                         :  8, // [7:0]
                      subband80_0_cc1                                         :  8, // [15:8]
                      subband80_1_cc0                                         :  8, // [23:16]
                      subband80_1_cc1                                         :  8; // [31:24]
#else
             uint32_t subband80_1_cc1                                         :  8, // [31:24]
                      subband80_1_cc0                                         :  8, // [23:16]
                      subband80_0_cc1                                         :  8, // [15:8]
                      subband80_0_cc0                                         :  8; // [7:0]
#endif
};


/* Description		SUBBAND80_0_CC0

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 0 (HE_SIGB0 or EHT_SIG0) 
			for the lower 80 MHz
			
			Valid for HE_20/HE_40/HE_80/HE_160/ EHT_20/EHT_40/EHT_80/EHT_160/ 
			EHT_240/EHT_320
			<legal all>
*/

#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC0_OFFSET                              0x00000000
#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC0_LSB                                 0
#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC0_MSB                                 7
#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC0_MASK                                0x000000ff


/* Description		SUBBAND80_0_CC1

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 1 (HE_SIGB1 or EHT_SIG1) 
			for the lower 80 MHz
			
			Valid for HE_40/HE_80/HE_160 (80+80)/ EHT_40/EHT_80/EHT_160/EHT_240/EHT_320
			
			<legal all>
*/

#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC1_OFFSET                              0x00000000
#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC1_LSB                                 8
#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC1_MSB                                 15
#define UNALLOCATED_RU_160_INFO_SUBBAND80_0_CC1_MASK                                0x0000ff00


/* Description		SUBBAND80_1_CC0

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 0 (HE_SIGB0 or EHT_SIG0) 
			for the higher 80 MHz
			
			Valid for EHT_160/EHT_240/EHT_320
			All 80 MHz subbands are identical for HE_160 (80+80).
			<legal all>
*/

#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC0_OFFSET                              0x00000000
#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC0_LSB                                 16
#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC0_MSB                                 23
#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC0_MASK                                0x00ff0000


/* Description		SUBBAND80_1_CC1

			Number of STA_ID=2046 HE-SIG-B/EHT-SIG user fields microcode
			 should generate for content channel 1 (HE_SIGB1 or EHT_SIG1) 
			for the higher 80 MHz
			
			Valid for EHT_160/EHT_240/EHT_320
			All 80 MHz subbands are identical for HE_160 (80+80).
			<legal all>
*/

#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC1_OFFSET                              0x00000000
#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC1_LSB                                 24
#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC1_MSB                                 31
#define UNALLOCATED_RU_160_INFO_SUBBAND80_1_CC1_MASK                                0xff000000



#endif   // UNALLOCATED_RU_160_INFO

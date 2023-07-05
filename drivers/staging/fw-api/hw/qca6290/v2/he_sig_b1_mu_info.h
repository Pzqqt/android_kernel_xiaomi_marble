/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HE_SIG_B1_MU_INFO_H_
#define _HE_SIG_B1_MU_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	ru_allocation[7:0], reserved_0[31:8]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_HE_SIG_B1_MU_INFO 1

struct he_sig_b1_mu_info {
             uint32_t ru_allocation                   :  8, //[7:0]
                      reserved_0                      : 24; //[31:8]
};

/*

ru_allocation
			
			RU allocation for the user(s) following this common
			portion of the SIG
			
			
			
			For details, refer to  RU_TYPE description
			
			<legal all>

reserved_0
			
			<legal 0>
*/


/* Description		HE_SIG_B1_MU_INFO_0_RU_ALLOCATION
			
			RU allocation for the user(s) following this common
			portion of the SIG
			
			
			
			For details, refer to  RU_TYPE description
			
			<legal all>
*/
#define HE_SIG_B1_MU_INFO_0_RU_ALLOCATION_OFFSET                     0x00000000
#define HE_SIG_B1_MU_INFO_0_RU_ALLOCATION_LSB                        0
#define HE_SIG_B1_MU_INFO_0_RU_ALLOCATION_MASK                       0x000000ff

/* Description		HE_SIG_B1_MU_INFO_0_RESERVED_0
			
			<legal 0>
*/
#define HE_SIG_B1_MU_INFO_0_RESERVED_0_OFFSET                        0x00000000
#define HE_SIG_B1_MU_INFO_0_RESERVED_0_LSB                           8
#define HE_SIG_B1_MU_INFO_0_RESERVED_0_MASK                          0xffffff00


#endif // _HE_SIG_B1_MU_INFO_H_

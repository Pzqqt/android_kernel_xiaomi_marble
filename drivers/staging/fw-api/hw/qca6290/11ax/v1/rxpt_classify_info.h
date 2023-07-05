/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef _RXPT_CLASSIFY_INFO_H_
#define _RXPT_CLASSIFY_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	reo_destination_indication[4:0], use_toeplitz_hash_2_clfy[5], use_toeplitz_hash_4_clfy[6], use_flow_id_toeplitz_clfy[7], reserved_0a[31:8]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RXPT_CLASSIFY_INFO 1

struct rxpt_classify_info {
             uint32_t reo_destination_indication      :  5, //[4:0]
                      use_toeplitz_hash_2_clfy        :  1, //[5]
                      use_toeplitz_hash_4_clfy        :  1, //[6]
                      use_flow_id_toeplitz_clfy       :  1, //[7]
                      reserved_0a                     : 24; //[31:8]
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
			into the REO2SW1 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_7> REO remaps this
			
			<enum 8 reo_destination_8> REO remaps this <enum 9
			reo_destination_9> REO remaps this <enum 10
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

use_toeplitz_hash_2_clfy
			
			indication to Rx OLE to enable classification based on
			'hash_2_toeplitz' .
			
			<legal all>

use_toeplitz_hash_4_clfy
			
			indication to Rx OLE to enable classification based on
			'hash_4_toeplitz' .
			
			<legal all>

use_flow_id_toeplitz_clfy
			
			indication to Rx OLE to enable classification based on
			'flow_id_toeplitz' from Common Parser, in case flow search
			fails
			
			<legal all>

reserved_0a
			
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
			into the REO2SW1 ring
			
			<enum 3 reo_destination_sw3> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 4 reo_destination_sw4> Reo will push the frame
			into the REO2SW1 ring
			
			<enum 5 reo_destination_release> Reo will push the frame
			into the REO_release ring
			
			<enum 6 reo_destination_fw> Reo will push the frame into
			the REO2FW ring
			
			<enum 7 reo_destination_7> REO remaps this
			
			<enum 8 reo_destination_8> REO remaps this <enum 9
			reo_destination_9> REO remaps this <enum 10
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

/* Description		RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_2_CLFY
			
			indication to Rx OLE to enable classification based on
			'hash_2_toeplitz' .
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_2_CLFY_OFFSET         0x00000000
#define RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_2_CLFY_LSB            5
#define RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_2_CLFY_MASK           0x00000020

/* Description		RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_4_CLFY
			
			indication to Rx OLE to enable classification based on
			'hash_4_toeplitz' .
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_4_CLFY_OFFSET         0x00000000
#define RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_4_CLFY_LSB            6
#define RXPT_CLASSIFY_INFO_0_USE_TOEPLITZ_HASH_4_CLFY_MASK           0x00000040

/* Description		RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY
			
			indication to Rx OLE to enable classification based on
			'flow_id_toeplitz' from Common Parser, in case flow search
			fails
			
			<legal all>
*/
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_OFFSET        0x00000000
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_LSB           7
#define RXPT_CLASSIFY_INFO_0_USE_FLOW_ID_TOEPLITZ_CLFY_MASK          0x00000080

/* Description		RXPT_CLASSIFY_INFO_0_RESERVED_0A
			
			<legal 0>
*/
#define RXPT_CLASSIFY_INFO_0_RESERVED_0A_OFFSET                      0x00000000
#define RXPT_CLASSIFY_INFO_0_RESERVED_0A_LSB                         8
#define RXPT_CLASSIFY_INFO_0_RESERVED_0A_MASK                        0xffffff00


#endif // _RXPT_CLASSIFY_INFO_H_

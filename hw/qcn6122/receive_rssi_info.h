/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _RECEIVE_RSSI_INFO_H_
#define _RECEIVE_RSSI_INFO_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	rssi_pri20_chain0[7:0], rssi_ext20_chain0[15:8], rssi_ext40_low20_chain0[23:16], rssi_ext40_high20_chain0[31:24]
//	1	rssi_ext80_low20_chain0[7:0], rssi_ext80_low_high20_chain0[15:8], rssi_ext80_high_low20_chain0[23:16], rssi_ext80_high20_chain0[31:24]
//	2	rssi_pri20_chain1[7:0], rssi_ext20_chain1[15:8], rssi_ext40_low20_chain1[23:16], rssi_ext40_high20_chain1[31:24]
//	3	rssi_ext80_low20_chain1[7:0], rssi_ext80_low_high20_chain1[15:8], rssi_ext80_high_low20_chain1[23:16], rssi_ext80_high20_chain1[31:24]
//	4	rssi_pri20_chain2[7:0], rssi_ext20_chain2[15:8], rssi_ext40_low20_chain2[23:16], rssi_ext40_high20_chain2[31:24]
//	5	rssi_ext80_low20_chain2[7:0], rssi_ext80_low_high20_chain2[15:8], rssi_ext80_high_low20_chain2[23:16], rssi_ext80_high20_chain2[31:24]
//	6	rssi_pri20_chain3[7:0], rssi_ext20_chain3[15:8], rssi_ext40_low20_chain3[23:16], rssi_ext40_high20_chain3[31:24]
//	7	rssi_ext80_low20_chain3[7:0], rssi_ext80_low_high20_chain3[15:8], rssi_ext80_high_low20_chain3[23:16], rssi_ext80_high20_chain3[31:24]
//	8	rssi_pri20_chain4[7:0], rssi_ext20_chain4[15:8], rssi_ext40_low20_chain4[23:16], rssi_ext40_high20_chain4[31:24]
//	9	rssi_ext80_low20_chain4[7:0], rssi_ext80_low_high20_chain4[15:8], rssi_ext80_high_low20_chain4[23:16], rssi_ext80_high20_chain4[31:24]
//	10	rssi_pri20_chain5[7:0], rssi_ext20_chain5[15:8], rssi_ext40_low20_chain5[23:16], rssi_ext40_high20_chain5[31:24]
//	11	rssi_ext80_low20_chain5[7:0], rssi_ext80_low_high20_chain5[15:8], rssi_ext80_high_low20_chain5[23:16], rssi_ext80_high20_chain5[31:24]
//	12	rssi_pri20_chain6[7:0], rssi_ext20_chain6[15:8], rssi_ext40_low20_chain6[23:16], rssi_ext40_high20_chain6[31:24]
//	13	rssi_ext80_low20_chain6[7:0], rssi_ext80_low_high20_chain6[15:8], rssi_ext80_high_low20_chain6[23:16], rssi_ext80_high20_chain6[31:24]
//	14	rssi_pri20_chain7[7:0], rssi_ext20_chain7[15:8], rssi_ext40_low20_chain7[23:16], rssi_ext40_high20_chain7[31:24]
//	15	rssi_ext80_low20_chain7[7:0], rssi_ext80_low_high20_chain7[15:8], rssi_ext80_high_low20_chain7[23:16], rssi_ext80_high20_chain7[31:24]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RECEIVE_RSSI_INFO 16

struct receive_rssi_info {
             uint32_t rssi_pri20_chain0               :  8, //[7:0]
                      rssi_ext20_chain0               :  8, //[15:8]
                      rssi_ext40_low20_chain0         :  8, //[23:16]
                      rssi_ext40_high20_chain0        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain0         :  8, //[7:0]
                      rssi_ext80_low_high20_chain0    :  8, //[15:8]
                      rssi_ext80_high_low20_chain0    :  8, //[23:16]
                      rssi_ext80_high20_chain0        :  8; //[31:24]
             uint32_t rssi_pri20_chain1               :  8, //[7:0]
                      rssi_ext20_chain1               :  8, //[15:8]
                      rssi_ext40_low20_chain1         :  8, //[23:16]
                      rssi_ext40_high20_chain1        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain1         :  8, //[7:0]
                      rssi_ext80_low_high20_chain1    :  8, //[15:8]
                      rssi_ext80_high_low20_chain1    :  8, //[23:16]
                      rssi_ext80_high20_chain1        :  8; //[31:24]
             uint32_t rssi_pri20_chain2               :  8, //[7:0]
                      rssi_ext20_chain2               :  8, //[15:8]
                      rssi_ext40_low20_chain2         :  8, //[23:16]
                      rssi_ext40_high20_chain2        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain2         :  8, //[7:0]
                      rssi_ext80_low_high20_chain2    :  8, //[15:8]
                      rssi_ext80_high_low20_chain2    :  8, //[23:16]
                      rssi_ext80_high20_chain2        :  8; //[31:24]
             uint32_t rssi_pri20_chain3               :  8, //[7:0]
                      rssi_ext20_chain3               :  8, //[15:8]
                      rssi_ext40_low20_chain3         :  8, //[23:16]
                      rssi_ext40_high20_chain3        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain3         :  8, //[7:0]
                      rssi_ext80_low_high20_chain3    :  8, //[15:8]
                      rssi_ext80_high_low20_chain3    :  8, //[23:16]
                      rssi_ext80_high20_chain3        :  8; //[31:24]
             uint32_t rssi_pri20_chain4               :  8, //[7:0]
                      rssi_ext20_chain4               :  8, //[15:8]
                      rssi_ext40_low20_chain4         :  8, //[23:16]
                      rssi_ext40_high20_chain4        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain4         :  8, //[7:0]
                      rssi_ext80_low_high20_chain4    :  8, //[15:8]
                      rssi_ext80_high_low20_chain4    :  8, //[23:16]
                      rssi_ext80_high20_chain4        :  8; //[31:24]
             uint32_t rssi_pri20_chain5               :  8, //[7:0]
                      rssi_ext20_chain5               :  8, //[15:8]
                      rssi_ext40_low20_chain5         :  8, //[23:16]
                      rssi_ext40_high20_chain5        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain5         :  8, //[7:0]
                      rssi_ext80_low_high20_chain5    :  8, //[15:8]
                      rssi_ext80_high_low20_chain5    :  8, //[23:16]
                      rssi_ext80_high20_chain5        :  8; //[31:24]
             uint32_t rssi_pri20_chain6               :  8, //[7:0]
                      rssi_ext20_chain6               :  8, //[15:8]
                      rssi_ext40_low20_chain6         :  8, //[23:16]
                      rssi_ext40_high20_chain6        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain6         :  8, //[7:0]
                      rssi_ext80_low_high20_chain6    :  8, //[15:8]
                      rssi_ext80_high_low20_chain6    :  8, //[23:16]
                      rssi_ext80_high20_chain6        :  8; //[31:24]
             uint32_t rssi_pri20_chain7               :  8, //[7:0]
                      rssi_ext20_chain7               :  8, //[15:8]
                      rssi_ext40_low20_chain7         :  8, //[23:16]
                      rssi_ext40_high20_chain7        :  8; //[31:24]
             uint32_t rssi_ext80_low20_chain7         :  8, //[7:0]
                      rssi_ext80_low_high20_chain7    :  8, //[15:8]
                      rssi_ext80_high_low20_chain7    :  8, //[23:16]
                      rssi_ext80_high20_chain7        :  8; //[31:24]
};

/*

rssi_pri20_chain0
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain0
			
			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain1
			
			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain1
			
			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain2
			
			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain2
			
			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain3
			
			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain3
			
			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain4
			
			RSSI of RX PPDU on chain 4 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain4
			
			RSSI of RX PPDU on chain 4 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain5
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain5
			
			RSSI of RX PPDU on chain 5 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain6
			
			RSSI of RX PPDU on chain 6 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain6
			
			RSSI of RX PPDU on chain 6 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_pri20_chain7
			
			RSSI of RX PPDU on chain 7 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.

rssi_ext40_low20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext40_high20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_low_high20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high_low20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.

rssi_ext80_high20_chain7
			
			RSSI of RX PPDU on chain 7 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/


/* Description		RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0_OFFSET                 0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0_LSB                    0
#define RECEIVE_RSSI_INFO_0_RSSI_PRI20_CHAIN0_MASK                   0x000000ff

/* Description		RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0_OFFSET                 0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0_LSB                    8
#define RECEIVE_RSSI_INFO_0_RSSI_EXT20_CHAIN0_MASK                   0x0000ff00

/* Description		RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0_OFFSET           0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0_LSB              16
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_LOW20_CHAIN0_MASK             0x00ff0000

/* Description		RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0_OFFSET          0x00000000
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0_LSB             24
#define RECEIVE_RSSI_INFO_0_RSSI_EXT40_HIGH20_CHAIN0_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0_OFFSET           0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0_LSB              0
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW20_CHAIN0_MASK             0x000000ff

/* Description		RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0_OFFSET      0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0_LSB         8
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_LOW_HIGH20_CHAIN0_MASK        0x0000ff00

/* Description		RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0_OFFSET      0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0_LSB         16
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH_LOW20_CHAIN0_MASK        0x00ff0000

/* Description		RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0
			
			RSSI of RX PPDU on chain 0 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0_OFFSET          0x00000004
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0_LSB             24
#define RECEIVE_RSSI_INFO_1_RSSI_EXT80_HIGH20_CHAIN0_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1_OFFSET                 0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1_LSB                    0
#define RECEIVE_RSSI_INFO_2_RSSI_PRI20_CHAIN1_MASK                   0x000000ff

/* Description		RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1_OFFSET                 0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1_LSB                    8
#define RECEIVE_RSSI_INFO_2_RSSI_EXT20_CHAIN1_MASK                   0x0000ff00

/* Description		RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1_OFFSET           0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1_LSB              16
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_LOW20_CHAIN1_MASK             0x00ff0000

/* Description		RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1_OFFSET          0x00000008
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1_LSB             24
#define RECEIVE_RSSI_INFO_2_RSSI_EXT40_HIGH20_CHAIN1_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1_OFFSET           0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1_LSB              0
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW20_CHAIN1_MASK             0x000000ff

/* Description		RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1_OFFSET      0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1_LSB         8
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_LOW_HIGH20_CHAIN1_MASK        0x0000ff00

/* Description		RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1_OFFSET      0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1_LSB         16
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH_LOW20_CHAIN1_MASK        0x00ff0000

/* Description		RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1
			
			RSSI of RX PPDU on chain 1 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1_OFFSET          0x0000000c
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1_LSB             24
#define RECEIVE_RSSI_INFO_3_RSSI_EXT80_HIGH20_CHAIN1_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2_OFFSET                 0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2_LSB                    0
#define RECEIVE_RSSI_INFO_4_RSSI_PRI20_CHAIN2_MASK                   0x000000ff

/* Description		RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2_OFFSET                 0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2_LSB                    8
#define RECEIVE_RSSI_INFO_4_RSSI_EXT20_CHAIN2_MASK                   0x0000ff00

/* Description		RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2_OFFSET           0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2_LSB              16
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_LOW20_CHAIN2_MASK             0x00ff0000

/* Description		RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2_OFFSET          0x00000010
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2_LSB             24
#define RECEIVE_RSSI_INFO_4_RSSI_EXT40_HIGH20_CHAIN2_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2_OFFSET           0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2_LSB              0
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW20_CHAIN2_MASK             0x000000ff

/* Description		RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2_OFFSET      0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2_LSB         8
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_LOW_HIGH20_CHAIN2_MASK        0x0000ff00

/* Description		RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2_OFFSET      0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2_LSB         16
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH_LOW20_CHAIN2_MASK        0x00ff0000

/* Description		RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2
			
			RSSI of RX PPDU on chain 2 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2_OFFSET          0x00000014
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2_LSB             24
#define RECEIVE_RSSI_INFO_5_RSSI_EXT80_HIGH20_CHAIN2_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3_OFFSET                 0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3_LSB                    0
#define RECEIVE_RSSI_INFO_6_RSSI_PRI20_CHAIN3_MASK                   0x000000ff

/* Description		RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3_OFFSET                 0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3_LSB                    8
#define RECEIVE_RSSI_INFO_6_RSSI_EXT20_CHAIN3_MASK                   0x0000ff00

/* Description		RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3_OFFSET           0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3_LSB              16
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_LOW20_CHAIN3_MASK             0x00ff0000

/* Description		RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3_OFFSET          0x00000018
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3_LSB             24
#define RECEIVE_RSSI_INFO_6_RSSI_EXT40_HIGH20_CHAIN3_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3_OFFSET           0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3_LSB              0
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW20_CHAIN3_MASK             0x000000ff

/* Description		RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3_OFFSET      0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3_LSB         8
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_LOW_HIGH20_CHAIN3_MASK        0x0000ff00

/* Description		RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3_OFFSET      0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3_LSB         16
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH_LOW20_CHAIN3_MASK        0x00ff0000

/* Description		RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3
			
			RSSI of RX PPDU on chain 3 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3_OFFSET          0x0000001c
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3_LSB             24
#define RECEIVE_RSSI_INFO_7_RSSI_EXT80_HIGH20_CHAIN3_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4_OFFSET                 0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4_LSB                    0
#define RECEIVE_RSSI_INFO_8_RSSI_PRI20_CHAIN4_MASK                   0x000000ff

/* Description		RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4_OFFSET                 0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4_LSB                    8
#define RECEIVE_RSSI_INFO_8_RSSI_EXT20_CHAIN4_MASK                   0x0000ff00

/* Description		RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4_OFFSET           0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4_LSB              16
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_LOW20_CHAIN4_MASK             0x00ff0000

/* Description		RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4_OFFSET          0x00000020
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4_LSB             24
#define RECEIVE_RSSI_INFO_8_RSSI_EXT40_HIGH20_CHAIN4_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4_OFFSET           0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4_LSB              0
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW20_CHAIN4_MASK             0x000000ff

/* Description		RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4_OFFSET      0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4_LSB         8
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_LOW_HIGH20_CHAIN4_MASK        0x0000ff00

/* Description		RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4_OFFSET      0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4_LSB         16
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH_LOW20_CHAIN4_MASK        0x00ff0000

/* Description		RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4
			
			RSSI of RX PPDU on chain 4 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4_OFFSET          0x00000024
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4_LSB             24
#define RECEIVE_RSSI_INFO_9_RSSI_EXT80_HIGH20_CHAIN4_MASK            0xff000000

/* Description		RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5
			
			RSSI of RX PPDU on chain 0 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5_OFFSET                0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5_LSB                   0
#define RECEIVE_RSSI_INFO_10_RSSI_PRI20_CHAIN5_MASK                  0x000000ff

/* Description		RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5_OFFSET                0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5_LSB                   8
#define RECEIVE_RSSI_INFO_10_RSSI_EXT20_CHAIN5_MASK                  0x0000ff00

/* Description		RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5_OFFSET          0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5_LSB             16
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_LOW20_CHAIN5_MASK            0x00ff0000

/* Description		RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5_OFFSET         0x00000028
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5_LSB            24
#define RECEIVE_RSSI_INFO_10_RSSI_EXT40_HIGH20_CHAIN5_MASK           0xff000000

/* Description		RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5_OFFSET          0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5_LSB             0
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW20_CHAIN5_MASK            0x000000ff

/* Description		RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5_OFFSET     0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5_LSB        8
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_LOW_HIGH20_CHAIN5_MASK       0x0000ff00

/* Description		RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5_OFFSET     0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5_LSB        16
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH_LOW20_CHAIN5_MASK       0x00ff0000

/* Description		RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5
			
			RSSI of RX PPDU on chain 5 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5_OFFSET         0x0000002c
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5_LSB            24
#define RECEIVE_RSSI_INFO_11_RSSI_EXT80_HIGH20_CHAIN5_MASK           0xff000000

/* Description		RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6_OFFSET                0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6_LSB                   0
#define RECEIVE_RSSI_INFO_12_RSSI_PRI20_CHAIN6_MASK                  0x000000ff

/* Description		RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6_OFFSET                0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6_LSB                   8
#define RECEIVE_RSSI_INFO_12_RSSI_EXT20_CHAIN6_MASK                  0x0000ff00

/* Description		RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6_OFFSET          0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6_LSB             16
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_LOW20_CHAIN6_MASK            0x00ff0000

/* Description		RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6_OFFSET         0x00000030
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6_LSB            24
#define RECEIVE_RSSI_INFO_12_RSSI_EXT40_HIGH20_CHAIN6_MASK           0xff000000

/* Description		RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6_OFFSET          0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6_LSB             0
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW20_CHAIN6_MASK            0x000000ff

/* Description		RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6_OFFSET     0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6_LSB        8
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_LOW_HIGH20_CHAIN6_MASK       0x0000ff00

/* Description		RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6_OFFSET     0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6_LSB        16
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH_LOW20_CHAIN6_MASK       0x00ff0000

/* Description		RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6
			
			RSSI of RX PPDU on chain 6 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6_OFFSET         0x00000034
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6_LSB            24
#define RECEIVE_RSSI_INFO_13_RSSI_EXT80_HIGH20_CHAIN6_MASK           0xff000000

/* Description		RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of primary 20 MHz bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7_OFFSET                0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7_LSB                   0
#define RECEIVE_RSSI_INFO_14_RSSI_PRI20_CHAIN7_MASK                  0x000000ff

/* Description		RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 20 MHz
			bandwidth. 
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7_OFFSET                0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7_LSB                   8
#define RECEIVE_RSSI_INFO_14_RSSI_EXT20_CHAIN7_MASK                  0x0000ff00

/* Description		RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7_OFFSET          0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7_LSB             16
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_LOW20_CHAIN7_MASK            0x00ff0000

/* Description		RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 40, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7_OFFSET         0x00000038
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7_LSB            24
#define RECEIVE_RSSI_INFO_14_RSSI_EXT40_HIGH20_CHAIN7_MASK           0xff000000

/* Description		RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7_OFFSET          0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7_LSB             0
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW20_CHAIN7_MASK            0x000000ff

/* Description		RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, low-high 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7_OFFSET     0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7_LSB        8
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_LOW_HIGH20_CHAIN7_MASK       0x0000ff00

/* Description		RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high-low 20
			MHz bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7_OFFSET     0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7_LSB        16
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH_LOW20_CHAIN7_MASK       0x00ff0000

/* Description		RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7
			
			RSSI of RX PPDU on chain 7 of extension 80, high 20 MHz
			bandwidth.  
			
			Value of 0x80 indicates invalid.
*/
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7_OFFSET         0x0000003c
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7_LSB            24
#define RECEIVE_RSSI_INFO_15_RSSI_EXT80_HIGH20_CHAIN7_MASK           0xff000000


#endif // _RECEIVE_RSSI_INFO_H_

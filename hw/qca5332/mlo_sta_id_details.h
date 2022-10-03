
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

 
 
 
 
 
 
 


#ifndef _MLO_STA_ID_DETAILS_H_
#define _MLO_STA_ID_DETAILS_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_WORDS_MLO_STA_ID_DETAILS 1


struct mlo_sta_id_details {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint16_t nstr_mlo_sta_id                                         : 10, // [9:0]
                      block_self_ml_sync                                      :  1, // [10:10]
                      block_partner_ml_sync                                   :  1, // [11:11]
                      nstr_mlo_sta_id_valid                                   :  1, // [12:12]
                      reserved_0a                                             :  3; // [15:13]
#else
             uint16_t reserved_0a                                             :  3, // [15:13]
                      nstr_mlo_sta_id_valid                                   :  1, // [12:12]
                      block_partner_ml_sync                                   :  1, // [11:11]
                      block_self_ml_sync                                      :  1, // [10:10]
                      nstr_mlo_sta_id                                         : 10; // [9:0]
#endif
};


/* Description		NSTR_MLO_STA_ID

			ID of peer participating in non-STR MLO
*/

#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_OFFSET                                   0x00000000
#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_LSB                                      0
#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_MSB                                      9
#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_MASK                                     0x000003ff


/* Description		BLOCK_SELF_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for self-link.
*/

#define MLO_STA_ID_DETAILS_BLOCK_SELF_ML_SYNC_OFFSET                                0x00000000
#define MLO_STA_ID_DETAILS_BLOCK_SELF_ML_SYNC_LSB                                   10
#define MLO_STA_ID_DETAILS_BLOCK_SELF_ML_SYNC_MSB                                   10
#define MLO_STA_ID_DETAILS_BLOCK_SELF_ML_SYNC_MASK                                  0x00000400


/* Description		BLOCK_PARTNER_ML_SYNC

			Only valid for TX
			
			When set, this provides an indication to block the peer 
			for partner links.
*/

#define MLO_STA_ID_DETAILS_BLOCK_PARTNER_ML_SYNC_OFFSET                             0x00000000
#define MLO_STA_ID_DETAILS_BLOCK_PARTNER_ML_SYNC_LSB                                11
#define MLO_STA_ID_DETAILS_BLOCK_PARTNER_ML_SYNC_MSB                                11
#define MLO_STA_ID_DETAILS_BLOCK_PARTNER_ML_SYNC_MASK                               0x00000800


/* Description		NSTR_MLO_STA_ID_VALID

			All the fields in this TLV are valid only if this bit is
			 set.
*/

#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_VALID_OFFSET                             0x00000000
#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_VALID_LSB                                12
#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_VALID_MSB                                12
#define MLO_STA_ID_DETAILS_NSTR_MLO_STA_ID_VALID_MASK                               0x00001000


/* Description		RESERVED_0A

			<legal 0>
*/

#define MLO_STA_ID_DETAILS_RESERVED_0A_OFFSET                                       0x00000000
#define MLO_STA_ID_DETAILS_RESERVED_0A_LSB                                          13
#define MLO_STA_ID_DETAILS_RESERVED_0A_MSB                                          15
#define MLO_STA_ID_DETAILS_RESERVED_0A_MASK                                         0x0000e000



#endif   // MLO_STA_ID_DETAILS

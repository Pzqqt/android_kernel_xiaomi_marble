/*
 * Copyright (c) 2011-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*
 *
 * This file dph_hash_table.h contains the definition of the scheduler class.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __DPH_HASH_TABLE_H__
#define __DPH_HASH_TABLE_H__

#include "ani_global.h"
/* Compare MAC addresses, return true if same */
static inline uint8_t dph_compare_mac_addr(uint8_t addr1[], uint8_t addr2[])
{
	return (addr1[0] == addr2[0]) &&
		(addr1[1] == addr2[1]) &&
		(addr1[2] == addr2[2]) &&
		(addr1[3] == addr2[3]) &&
		(addr1[4] == addr2[4]) && (addr1[5] == addr2[5]);
}

/* Hash table class */
typedef struct {

	/* The hash table itself */
	tpDphHashNode *pHashTable;

	/* The state array */
	tDphHashNode *pDphNodeArray;
	uint16_t size;
} dphHashTableClass;

/* The hash table object */
extern dphHashTableClass dphHashTable;

/* Print MAC addresse */
extern void dph_print_mac_addr(struct sAniSirGlobal *pMac, uint8_t addr[],
			       uint32_t);

tpDphHashNode dph_lookup_hash_entry(tpAniSirGlobal pMac, uint8_t staAddr[],
				    uint16_t *pStaId,
				    dphHashTableClass *pDphHashTable);
tpDphHashNode dph_lookup_assoc_id(tpAniSirGlobal pMac, uint16_t staIdx,
				  uint16_t *assocId,
				  dphHashTableClass *pDphHashTable);

/* Get a pointer to the hash node */
extern tpDphHashNode dph_get_hash_entry(tpAniSirGlobal pMac, uint16_t staId,
					dphHashTableClass *pDphHashTable);

/* Add an entry to the hash table */
extern tpDphHashNode dph_add_hash_entry(tpAniSirGlobal pMac,
					tSirMacAddr staAddr,
					uint16_t staId,
					dphHashTableClass *pDphHashTable);

/* Delete an entry from the hash table */
extern tSirRetStatus dph_delete_hash_entry(tpAniSirGlobal pMac,
					   tSirMacAddr staAddr, uint16_t staId,
					   dphHashTableClass *pDphHashTable);

void dph_hash_table_class_init(tpAniSirGlobal pMac,
			       dphHashTableClass *pDphHashTable);
/* Initialize STA state */
extern tpDphHashNode dph_init_sta_state(tpAniSirGlobal pMac,
					tSirMacAddr staAddr,
					uint16_t staId, uint8_t validStaIdx,
					dphHashTableClass *pDphHashTable);

#endif

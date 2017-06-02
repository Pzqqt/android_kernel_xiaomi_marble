/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * This file dph_hash_table.cc implements the member functions of
 * DPH hash table class.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#include "cds_api.h"
#include "cfg_api.h"
#include "sch_api.h"
#include "dph_global.h"
#include "lim_api.h"
#include "wma_if.h"

/* --------------------------------------------------------------------- */
/**
 * dphHashTableClass()
 *
 * FUNCTION:
 * Constructor function
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

void dph_hash_table_class_init(tpAniSirGlobal pMac,
			       dphHashTableClass *pDphHashTable)
{
	uint16_t i;

	for (i = 0; i < pDphHashTable->size; i++) {
		pDphHashTable->pHashTable[i] = 0;
	}

	for (i = 0; i < pDphHashTable->size; i++) {
		pDphHashTable->pDphNodeArray[i].valid = 0;
		pDphHashTable->pDphNodeArray[i].added = 0;
		pDphHashTable->pDphNodeArray[i].assocId = i;
	}

}

/* --------------------------------------------------------------------- */
/**
 * hash_function
 *
 * FUNCTION:
 * Hashing function
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param staAddr MAC address of the station
 * @return None
 */

static uint16_t hash_function(tpAniSirGlobal pMac, uint8_t staAddr[],
			      uint16_t numSta)
{
	int i;
	uint16_t sum = 0;

	for (i = 0; i < 6; i++)
		sum += staAddr[i];

	return sum % numSta;
}

/* --------------------------------------------------------------------- */
/**
 * dph_lookup_hash_entry
 *
 * FUNCTION:
 * Look up an entry in hash table
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param staAddr MAC address of the station
 * @param pStaId pointer to the Station ID assigned to the station
 * @return pointer to STA hash entry if lookup was a success \n
 *         NULL if lookup was a failure
 */

tpDphHashNode dph_lookup_hash_entry(tpAniSirGlobal pMac, uint8_t staAddr[],
				    uint16_t *pAssocId,
				    dphHashTableClass *pDphHashTable)
{
	tpDphHashNode ptr = NULL;
	uint16_t index = hash_function(pMac, staAddr, pDphHashTable->size);

	if (!pDphHashTable->pHashTable) {
		pe_err("pHashTable is NULL");
		return ptr;
	}

	for (ptr = pDphHashTable->pHashTable[index]; ptr; ptr = ptr->next) {
		if (dph_compare_mac_addr(staAddr, ptr->staAddr)) {
			*pAssocId = ptr->assocId;
			break;
		}
	}
	return ptr;
}

/* --------------------------------------------------------------------- */
/**
 * dph_get_hash_entry
 *
 * FUNCTION:
 * Get a pointer to the hash node
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param staId Station ID
 * @return pointer to STA hash entry if lookup was a success \n
 *         NULL if lookup was a failure
 */

tpDphHashNode dph_get_hash_entry(tpAniSirGlobal pMac, uint16_t peerIdx,
				 dphHashTableClass *pDphHashTable)
{
	if (peerIdx < pDphHashTable->size) {
		if (pDphHashTable->pDphNodeArray[peerIdx].added)
			return &pDphHashTable->pDphNodeArray[peerIdx];
		else
			return NULL;
	} else
		return NULL;

}

static inline tpDphHashNode get_node(tpAniSirGlobal pMac, uint8_t assocId,
				     dphHashTableClass *pDphHashTable)
{
	return &pDphHashTable->pDphNodeArray[assocId];
}

/* --------------------------------------------------------------------- */
/**
 * dph_lookup_assoc_id
 *
 * FUNCTION:
 * This function looks up assocID given the station Id. It traverses the complete table to do this.
 * Need to find an efficient way to do this.
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pMac pointer to global Mac structure.
 * @param staIdx station ID
 * @param *assocId pointer to associd to be returned by this function.
 * @return pointer to the dph node.
 */
tpDphHashNode dph_lookup_assoc_id(tpAniSirGlobal pMac, uint16_t staIdx,
				  uint16_t *assocId,
				  dphHashTableClass *pDphHashTable)
{
	uint8_t i;

	for (i = 0; i < pDphHashTable->size; i++) {
		if ((pDphHashTable->pDphNodeArray[i].added) &&
		    (pDphHashTable->pDphNodeArray[i].staIndex == staIdx)) {
			*assocId = i;
			break;
		}

	}
	if (i == pDphHashTable->size)
		return NULL;
	return &pDphHashTable->pDphNodeArray[i];

}

/** -------------------------------------------------------------
   \fn dph_init_sta_state
   \brief Initialize STA state. this function saves the staId from the current entry in the DPH table with given assocId
 \ if validStaIdx flag is set. Otherwise it sets the staId to invalid.
   \param  tpAniSirGlobal    pMac
   \param  tSirMacAddr staAddr
   \param  uint16_t assocId
   \param  uint8_t validStaIdx -   true ==> the staId in the DPH entry with given assocId is valid and restore it back.
 \                                              false ==> set the staId to invalid.
   \return tpDphHashNode - DPH hash node if found.
   -------------------------------------------------------------*/

tpDphHashNode dph_init_sta_state(tpAniSirGlobal pMac, tSirMacAddr staAddr,
				 uint16_t assocId, uint8_t validStaIdx,
				 dphHashTableClass *pDphHashTable)
{
	uint32_t val;

	tpDphHashNode pStaDs, pnext;
	uint16_t staIdx = STA_INVALID_IDX;

	if (assocId >= pDphHashTable->size) {
		pe_err("Invalid Assoc Id %d", assocId);
		return NULL;
	}

	pStaDs = get_node(pMac, (uint8_t) assocId, pDphHashTable);
	staIdx = pStaDs->staIndex;
	pnext = pStaDs->next;

	/* Clear the STA node except for the next pointer */
	qdf_mem_set((uint8_t *)pStaDs, sizeof(tDphHashNode), 0);
	pStaDs->next = pnext;

	/* Initialize the assocId */
	pStaDs->assocId = assocId;
	if (true == validStaIdx)
		pStaDs->staIndex = staIdx;
	else
		pStaDs->staIndex = STA_INVALID_IDX;

	/* Initialize STA mac address */
	qdf_mem_copy(pStaDs->staAddr, staAddr, sizeof(tSirMacAddr));

	/* Initialize fragmentation threshold */
	if (wlan_cfg_get_int(pMac, WNI_CFG_FRAGMENTATION_THRESHOLD, &val) !=
	    eSIR_SUCCESS)
		pe_warn("could not retrieve fragmentation threshold");
	else
		pStaDs->fragSize = (uint16_t) val;

	pStaDs->added = 1;
	pStaDs->encPolicy = ENC_POLICY_NULL;
	pStaDs->is_disassoc_deauth_in_progress = 0;
#ifdef WLAN_FEATURE_11W
	pStaDs->last_assoc_received_time = 0;
#endif
	pStaDs->sta_deletion_in_progress = false;
	pStaDs->valid = 1;
	return pStaDs;
}

/* --------------------------------------------------------------------- */
/**
 * dph_add_hash_entry
 *
 * FUNCTION:
 * Add entry to hash table
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param staAddr MAC address of the station
 * @param staId Station ID assigned to the station
 * @return Pointer to STA hash entry
 */

tpDphHashNode dph_add_hash_entry(tpAniSirGlobal pMac, tSirMacAddr staAddr,
				 uint16_t assocId,
				 dphHashTableClass *pDphHashTable)
{
	tpDphHashNode ptr, node;
	uint16_t index = hash_function(pMac, staAddr, pDphHashTable->size);

	pe_debug("assocId %d index %d STA addr",
		       assocId, index);
	pe_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(staAddr));

	if (assocId >= pDphHashTable->size) {
		pe_err("invalid STA id %d", assocId);
		return NULL;
	}

	if (pDphHashTable->pDphNodeArray[assocId].added) {
		pe_err("already added STA %d", assocId);
		return NULL;
	}

	for (ptr = pDphHashTable->pHashTable[index]; ptr; ptr = ptr->next) {
		if (ptr == ptr->next) {
			pe_err("Infinite Loop");
			return NULL;
		}

		if (dph_compare_mac_addr(staAddr, ptr->staAddr)
		    || ptr->assocId == assocId)
			break;
	}

	if (ptr) {
		/* Duplicate entry */
		pe_err("assocId %d hashIndex %d entry exists",
			assocId, index);
		return NULL;
	} else {
		if (dph_init_sta_state
			    (pMac, staAddr, assocId, false, pDphHashTable) == NULL) {
			pe_err("could not Init STA id: %d", assocId);
			return NULL;
		}
		/* Add the node to the link list */
		pDphHashTable->pDphNodeArray[assocId].next =
			pDphHashTable->pHashTable[index];
		pDphHashTable->pHashTable[index] =
			&pDphHashTable->pDphNodeArray[assocId];

		node = pDphHashTable->pHashTable[index];
		return node;
	}
}

/* --------------------------------------------------------------------- */
/**
 * dph_delete_hash_entry
 *
 * FUNCTION:
 * Delete entry from hash table
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param staAddr MAC address of the station
 * @param staId Station ID assigned to the station
 * @return eSIR_SUCCESS if successful,\n
 *         eSIR_FAILURE otherwise
 */

tSirRetStatus dph_delete_hash_entry(tpAniSirGlobal pMac, tSirMacAddr staAddr,
				    uint16_t assocId,
				    dphHashTableClass *pDphHashTable)
{
	tpDphHashNode ptr, prev;
	uint16_t index = hash_function(pMac, staAddr, pDphHashTable->size);

	pe_debug("assocId %d index %d STA addr", assocId, index);
	pe_debug(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(staAddr));

	if (assocId >= pDphHashTable->size) {
		pe_err("invalid STA id %d", assocId);
		return eSIR_FAILURE;
	}

	if (pDphHashTable->pDphNodeArray[assocId].added == 0) {
		pe_err("STA %d never added", assocId);
		return eSIR_FAILURE;
	}

	for (prev = 0, ptr = pDphHashTable->pHashTable[index];
	     ptr; prev = ptr, ptr = ptr->next) {
		if (dph_compare_mac_addr(staAddr, ptr->staAddr))
			break;
		if (prev == ptr) {
			pe_err("Infinite Loop");
			return eSIR_FAILURE;
		}
	}

	if (ptr) {
		/* / Delete the entry after invalidating it */
		ptr->valid = 0;
		memset(ptr->staAddr, 0, sizeof(ptr->staAddr));
		if (prev == 0)
			pDphHashTable->pHashTable[index] = ptr->next;
		else
			prev->next = ptr->next;
		ptr->added = 0;
		ptr->is_disassoc_deauth_in_progress = 0;
#ifdef WLAN_FEATURE_11W
		ptr->last_assoc_received_time = 0;
#endif
		ptr->sta_deletion_in_progress = false;
		ptr->next = 0;
	} else {
		pe_err("Entry not present STA addr");
		pe_err(MAC_ADDRESS_STR, MAC_ADDR_ARRAY(staAddr));
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}



/*
 * Copyright (c) 2011-2016, 2018 The Linux Foundation. All rights reserved.
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
 *
 * This file lim_aid_mgmt.c contains the functions related to
 * AID pool management like initialization, assignment etc.
 * Author:        Chandra Modumudi
 * Date:          03/20/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */

#include "cds_api.h"
#include "wni_cfg.h"
#include "ani_global.h"
#include "cfg_api.h"
#include "sir_params.h"
#include "lim_utils.h"
#include "lim_timer_utils.h"
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "lim_session_utils.h"

#define LIM_START_PEER_IDX   1

/**
 * lim_init_peer_idxpool() -- initializes peer index pool
 * @pMac: mac context
 * @pSessionEntry: session entry
 *
 * This function is called while starting a BSS at AP
 * to initialize AID pool. This may also be called while
 * starting/joining an IBSS if 'Association' is allowed
 * in IBSS.
 *
 * Return: None
 */

void lim_init_peer_idxpool(tpAniSirGlobal pMac, tpPESession pSessionEntry)
{
	uint8_t i;
	uint8_t maxAssocSta = pMac->lim.maxStation;

	pSessionEntry->gpLimPeerIdxpool[0] = 0;

#ifdef FEATURE_WLAN_TDLS
	/*
	* In station role, DPH_STA_HASH_INDEX_PEER (index 1) is reserved
	* for peer station index corresponding to AP. Avoid choosing that index
	* and get index starting from (DPH_STA_HASH_INDEX_PEER + 1)
	* (index 2) for TDLS stations;
	*/
	if (LIM_IS_STA_ROLE(pSessionEntry)) {
		pSessionEntry->freePeerIdxHead = DPH_STA_HASH_INDEX_PEER + 1;
	} else
#endif
#ifdef QCA_IBSS_SUPPORT
	if (LIM_IS_IBSS_ROLE(pSessionEntry)) {
		pSessionEntry->freePeerIdxHead = LIM_START_PEER_IDX;
	} else
#endif
	{
		pSessionEntry->freePeerIdxHead = LIM_START_PEER_IDX;
	}

	for (i = pSessionEntry->freePeerIdxHead; i < maxAssocSta; i++) {
		pSessionEntry->gpLimPeerIdxpool[i] = i + 1;
	}
	pSessionEntry->gpLimPeerIdxpool[i] = 0;

	pSessionEntry->freePeerIdxTail = i;

}

/**
 * lim_assign_peer_idx()
 *
 ***FUNCTION:
 * This function is called to get a peer station index. This index is
 * used during Association/Reassociation
 * frame handling to assign association ID (aid) to a STA.
 * In case of TDLS, this is used to assign a index into the Dph hash entry.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @return peerIdx  - assigned peer Station IDx for STA
 */

uint16_t lim_assign_peer_idx(tpAniSirGlobal pMac, tpPESession pSessionEntry)
{
	uint16_t peerId;

	/* make sure we haven't exceeded the configurable limit on associations */
	/* This count is global to ensure that it doesn't exceed the hardware limits. */
	if (pe_get_current_stas_count(pMac) >= pMac->lim.gLimAssocStaLimit) {
		/* too many associations already active */
		return 0;
	}

	/* return head of free list */

	if (pSessionEntry->freePeerIdxHead) {
		peerId = pSessionEntry->freePeerIdxHead;
		pSessionEntry->freePeerIdxHead =
			pSessionEntry->gpLimPeerIdxpool[pSessionEntry->
							freePeerIdxHead];
		if (pSessionEntry->freePeerIdxHead == 0)
			pSessionEntry->freePeerIdxTail = 0;
		pSessionEntry->gLimNumOfCurrentSTAs++;
		return peerId;
	}

	return 0;               /* no more free peer index */
}

/**
 * lim_release_peer_idx()
 *
 ***FUNCTION:
 * This function is called when a STA context is removed
 * at AP (or at a STA in IBSS mode or TDLS) to return peer Index
 * to free pool.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  pMac - Pointer to Global MAC structure
 * @param  peerIdx - peer station index that need to return to free pool
 *
 * @return None
 */

void
lim_release_peer_idx(tpAniSirGlobal pMac, uint16_t peerIdx,
		     tpPESession pSessionEntry)
{
	pSessionEntry->gLimNumOfCurrentSTAs--;

	/* insert at tail of free list */
	if (pSessionEntry->freePeerIdxTail) {
		pSessionEntry->gpLimPeerIdxpool[pSessionEntry->
						freePeerIdxTail] =
			(uint8_t) peerIdx;
		pSessionEntry->freePeerIdxTail = (uint8_t) peerIdx;
	} else {
		pSessionEntry->freePeerIdxTail =
			pSessionEntry->freePeerIdxHead = (uint8_t) peerIdx;
	}
	pSessionEntry->gpLimPeerIdxpool[(uint8_t) peerIdx] = 0;
}

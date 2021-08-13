/*
 * Copyright (c) 2011-2016, 2018-2021 The Linux Foundation. All rights reserved.
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
#include "sir_params.h"
#include "lim_utils.h"
#include "lim_timer_utils.h"
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "lim_session_utils.h"
#include <wlan_mlo_mgr_ap.h>
#include <wlan_mlo_mgr_peer.h>

#define LIM_START_PEER_IDX   1

/**
 * lim_init_peer_idxpool_legacy() - init aid pool for non MLO SAP
 * @mac: Pointer to Global MAC structure
 * @pe_session: pe session
 *
 * Return: Void
 */
static void lim_init_peer_idxpool_legacy(struct mac_context *mac,
					 struct pe_session *pe_session)
{
	uint8_t i;
	uint8_t max_assoc_sta = mac->lim.max_sta_of_pe_session;

	pe_session->gpLimPeerIdxpool[0] = 0;

#ifdef FEATURE_WLAN_TDLS
	/*
	* In station role, DPH_STA_HASH_INDEX_PEER (index 1) is reserved
	* for peer station index corresponding to AP. Avoid choosing that index
	* and get index starting from (DPH_STA_HASH_INDEX_PEER + 1)
	* (index 2) for TDLS stations;
	*/
	if (LIM_IS_STA_ROLE(pe_session)) {
		pe_session->freePeerIdxHead = DPH_STA_HASH_INDEX_PEER + 1;
	} else
#endif
	{
		pe_session->freePeerIdxHead = LIM_START_PEER_IDX;
	}

	for (i = pe_session->freePeerIdxHead; i < max_assoc_sta; i++)
		pe_session->gpLimPeerIdxpool[i] = i + 1;

	pe_session->gpLimPeerIdxpool[i] = 0;

	pe_session->freePeerIdxTail = i;
}

void lim_init_peer_idxpool(struct mac_context *mac,
			   struct pe_session *pe_session)
{
	if (!wlan_vdev_mlme_is_mlo_ap(pe_session->vdev))
		lim_init_peer_idxpool_legacy(mac, pe_session);
}

/**
 * lim_create_peer_idxpool_legacy() - AID pool creation for non-MLO AP
 * @pe_session: pe session
 * @idx_pool_size: aid pool size
 *
 * Return: true if pool is created successfully
 */
static bool lim_create_peer_idxpool_legacy(struct pe_session *pe_session,
					   uint8_t idx_pool_size)
{
	pe_session->gpLimPeerIdxpool = qdf_mem_malloc(
		sizeof(*pe_session->gpLimPeerIdxpool) * idx_pool_size);
	if (!pe_session->gpLimPeerIdxpool)
		return false;

	pe_session->freePeerIdxHead = 0;
	pe_session->freePeerIdxTail = 0;

	return true;
}

bool lim_create_peer_idxpool(struct pe_session *pe_session,
			     uint8_t idx_pool_size)
{
	pe_session->gLimNumOfCurrentSTAs = 0;
	if (!wlan_vdev_mlme_is_mlo_ap(pe_session->vdev))
		return lim_create_peer_idxpool_legacy(pe_session,
						      idx_pool_size);

	return true;
}

/**
 * lim_free_peer_idxpool_legacy() - Free the non-MLO AP aid pool
 * @pe_session: pe session
 *
 * Return: Void
 */
static void lim_free_peer_idxpool_legacy(struct pe_session *pe_session)
{
	if (pe_session->gpLimPeerIdxpool) {
		qdf_mem_free(pe_session->gpLimPeerIdxpool);
		pe_session->gpLimPeerIdxpool = NULL;
	}
}

void lim_free_peer_idxpool(struct pe_session *pe_session)
{
	if (!wlan_vdev_mlme_is_mlo_ap(pe_session->vdev))
		lim_free_peer_idxpool_legacy(pe_session);
}

/**
 * lim_assign_peer_idx_mlo() - trigger mlo api to allocate an AID for a STA
 * @pe_session: pe session
 *
 * Return: peer_idx - assigned AID for STA
 */
#ifdef WLAN_FEATURE_11BE_MLO
static uint16_t lim_assign_peer_idx_mlo(struct pe_session *pe_session)
{
	return mlme_get_aid(pe_session->vdev);
}
#else
static uint16_t lim_assign_peer_idx_mlo(struct pe_session *pe_session)
{
	return 0;               /* no more free peer index */
}
#endif

/**
 * lim_assign_peer_idx_legacy() - non-MLO AP allocates an AID for a STA
 * @mac: Pointer to Global MAC structure
 * @pe_session: pe session
 *
 * Return: peer_idx - assigned AID for STA
 */
static uint16_t lim_assign_peer_idx_legacy(struct mac_context *mac,
					   struct pe_session *pe_session)
{
	uint16_t peer_id;

	/* return head of free list */

	if (pe_session->freePeerIdxHead) {
		peer_id = pe_session->freePeerIdxHead;
		pe_session->freePeerIdxHead =
		    pe_session->gpLimPeerIdxpool[pe_session->freePeerIdxHead];
		if (pe_session->freePeerIdxHead == 0)
			pe_session->freePeerIdxTail = 0;
		return peer_id;
	}

	return 0;               /* no more free peer index */
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
 * @param  mac - Pointer to Global MAC structure
 * @return peerIdx  - assigned peer Station IDx for STA
 */

uint16_t lim_assign_peer_idx(struct mac_context *mac,
			     struct pe_session *pe_session)
{
	uint16_t peer_id;

	/* make sure we haven't exceeded the configurable limit on associations */
	/* This count is global to ensure that it doesn't exceed the hardware limits. */
	if (pe_get_current_stas_count(mac) >=
	    mac->mlme_cfg->sap_cfg.assoc_sta_limit) {
		/* too many associations already active */
		return 0;
	}

	if (wlan_vdev_mlme_is_mlo_ap(pe_session->vdev))
		peer_id = lim_assign_peer_idx_mlo(pe_session);
	else
		peer_id = lim_assign_peer_idx_legacy(mac, pe_session);

	if (peer_id)
		pe_session->gLimNumOfCurrentSTAs++;

	return peer_id;
}

#ifdef WLAN_FEATURE_11BE_MLO
uint16_t lim_assign_mlo_conn_idx(struct mac_context *mac,
				 struct pe_session *pe_session,
				 uint16_t partner_peer_idx)
{
	uint16_t peer_id;

	if (pe_get_current_stas_count(mac) >=
	    mac->mlme_cfg->sap_cfg.assoc_sta_limit) {
		/* too many associations already active */
		return 0;
	}

	if (partner_peer_idx)
		peer_id = partner_peer_idx;
	else
		peer_id = mlo_get_aid(pe_session->vdev);

	if (peer_id)
		pe_session->gLimNumOfCurrentSTAs++;

	return peer_id;
}

/**
 * lim_release_peer_idx_mlo() - trigger mlo api to release aid
 * @peer_idx: aid to free
 * @pe_session: pe session
 *
 * Return: Void
 */
static void lim_release_peer_idx_mlo(uint16_t peer_idx,
				     struct pe_session *pe_session)
{
	return mlme_free_aid(pe_session->vdev, peer_idx);
}
#else
static void lim_release_peer_idx_mlo(uint16_t peer_idx,
				     struct pe_session *pe_session)
{
}
#endif

/**
 * lim_release_peer_idx_legacy() - non-MLO AP releases an AID
 * @mac: Pointer to Global MAC structure
 * @peer_idx: aid to free
 * @pe_session: pe session
 *
 * Return: Void
 */
static void lim_release_peer_idx_legacy(struct mac_context *mac,
					uint16_t peer_idx,
					struct pe_session *pe_session)
{
	/* insert at tail of free list */
	if (pe_session->freePeerIdxTail) {
		pe_session->gpLimPeerIdxpool[pe_session->freePeerIdxTail] =
			(uint8_t)peer_idx;
		pe_session->freePeerIdxTail = (uint8_t)peer_idx;
	} else {
		pe_session->freePeerIdxTail =
			pe_session->freePeerIdxHead = (uint8_t)peer_idx;
	}
	pe_session->gpLimPeerIdxpool[(uint8_t)peer_idx] = 0;
}

/**
 * lim_release_peer_idx()
 *
 ***FUNCTION:
 * This function is called when a STA context is removed
 * at AP (or TDLS) to return peer Index
 * to free pool.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @param  peerIdx - peer station index that need to return to free pool
 *
 * @return None
 */

void
lim_release_peer_idx(struct mac_context *mac, uint16_t peer_idx,
		     struct pe_session *pe_session)
{
	pe_session->gLimNumOfCurrentSTAs--;

	if (wlan_vdev_mlme_is_mlo_ap(pe_session->vdev))
		lim_release_peer_idx_mlo(peer_idx, pe_session);
	else
		lim_release_peer_idx_legacy(mac, peer_idx, pe_session);
}

#ifdef WLAN_FEATURE_11BE_MLO
void
lim_release_mlo_conn_idx(struct mac_context *mac, uint16_t peer_idx,
			 struct pe_session *session, bool free_aid)
{
	session->gLimNumOfCurrentSTAs--;
	if (free_aid &&
	    wlan_mlo_get_mlpeer_by_aid(session->vdev->mlo_dev_ctx, peer_idx))
		mlo_free_aid(session->vdev, peer_idx);
}
#endif

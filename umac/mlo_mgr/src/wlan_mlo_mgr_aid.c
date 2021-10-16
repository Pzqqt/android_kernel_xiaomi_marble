/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include "wlan_mlo_mgr_main.h"
#include "qdf_types.h"
#include "wlan_cmn.h"
#include <include/wlan_vdev_mlme.h>
#include "wlan_mlo_mgr_ap.h"
#include "wlan_mlo_mgr_cmn.h"

static void mlo_peer_set_aid_bit(struct wlan_ml_vdev_aid_mgr *ml_aid_mgr,
				 uint16_t assoc_id_ix)
{
	uint16_t ix;
	struct wlan_vdev_aid_mgr *vdev_aid_mgr;

	/* Mark this bit as AID assigned */
	for (ix = 0; ix < WLAN_UMAC_MLO_MAX_VDEVS; ix++) {
		vdev_aid_mgr = ml_aid_mgr->aid_mgr[ix];
		if (vdev_aid_mgr)
			qdf_set_bit(assoc_id_ix, vdev_aid_mgr->aid_bitmap);
	}
}

static uint16_t wlan_mlo_peer_alloc_aid(
		struct wlan_ml_vdev_aid_mgr *ml_aid_mgr,
		bool is_mlo_peer,
		uint8_t link_ix)
{
	uint16_t assoc_id = (uint16_t)-1;
	uint16_t i, j;
	struct wlan_vdev_aid_mgr *vdev_aid_mgr;
	uint16_t first_aid = 0;
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_mgr_ctx)
		return assoc_id;

	if (!is_mlo_peer && link_ix == MLO_INVALID_LINK_IDX)
		return assoc_id;
	/* TODO check locking strategy */
	ml_aid_lock_acquire(mlo_mgr_ctx);

	for (i = 0; i < ml_aid_mgr->max_aid; i++) {
		if (qdf_test_bit(i, ml_aid_mgr->aid_bitmap))
			continue;

		if (is_mlo_peer) {
			for (j = 0; j < WLAN_UMAC_MLO_MAX_VDEVS; j++) {
				vdev_aid_mgr = ml_aid_mgr->aid_mgr[j];
				if (vdev_aid_mgr &&
				    qdf_test_bit(i, vdev_aid_mgr->aid_bitmap))
					break;
				/* AID is free */
				if (j == WLAN_UMAC_MLO_MAX_VDEVS - 1) {
					assoc_id = i + 1;
					mlo_peer_set_aid_bit(ml_aid_mgr, i);
				}
			}

			if (assoc_id == i + 1) {
				qdf_set_bit(i, ml_aid_mgr->aid_bitmap);
				break;
			}
		} else {
			vdev_aid_mgr = ml_aid_mgr->aid_mgr[link_ix];
			if (vdev_aid_mgr)
				break;

			if (qdf_test_bit(i, vdev_aid_mgr->aid_bitmap))
				continue;

			if (!first_aid)
				first_aid = i + 1;

			for (j = 0; j < WLAN_UMAC_MLO_MAX_VDEVS; j++) {
				if (j == link_ix)
					continue;

				vdev_aid_mgr = ml_aid_mgr->aid_mgr[j];
				if (vdev_aid_mgr &&
				    qdf_test_bit(i, vdev_aid_mgr->aid_bitmap)) {
					assoc_id = i + 1;
					break;
				}
			}
			if (assoc_id == i + 1) {
				vdev_aid_mgr = ml_aid_mgr->aid_mgr[link_ix];
				qdf_set_bit(i, vdev_aid_mgr->aid_bitmap);
				first_aid = 0;
				break;
			}
		}
	}

	if ((!is_mlo_peer) && first_aid) {
		vdev_aid_mgr = ml_aid_mgr->aid_mgr[link_ix];
		qdf_set_bit(first_aid, vdev_aid_mgr->aid_bitmap);
		assoc_id = first_aid;
	}

	ml_aid_lock_release(mlo_mgr_ctx);

	return assoc_id;
}

static uint16_t wlan_mlme_peer_alloc_aid(
		struct wlan_vdev_aid_mgr *vdev_aid_mgr,
		bool no_lock)
{
	uint16_t assoc_id = (uint16_t)-1;
	uint16_t i;
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_mgr_ctx)
		return assoc_id;

	if (!no_lock)
		ml_aid_lock_acquire(mlo_mgr_ctx);

	for (i = 0; i < vdev_aid_mgr->max_aid; i++) {
		if (qdf_test_bit(i, vdev_aid_mgr->aid_bitmap))
			continue;

		assoc_id = i + 1;
		qdf_set_bit(i, vdev_aid_mgr->aid_bitmap);
	}

	if (!no_lock)
		ml_aid_lock_release(mlo_mgr_ctx);

	return assoc_id;
}

QDF_STATUS wlan_mlo_peer_free_aid(
		struct wlan_ml_vdev_aid_mgr *ml_aid_mgr,
		uint8_t link_ix,
		uint16_t assoc_id)
{
	uint16_t  j;
	struct wlan_vdev_aid_mgr *vdev_aid_mgr;
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();
	uint16_t assoc_id_ix;

	if (!mlo_mgr_ctx)
		return QDF_STATUS_E_FAILURE;

	/* TODO check locking strategy */
	ml_aid_lock_acquire(mlo_mgr_ctx);
	assoc_id_ix = assoc_id - 1;
	if (qdf_test_bit(assoc_id_ix, ml_aid_mgr->aid_bitmap)) {
		qdf_clear_bit(assoc_id_ix, ml_aid_mgr->aid_bitmap);
		for (j = 0; j < WLAN_UMAC_MLO_MAX_VDEVS; j++) {
			vdev_aid_mgr = ml_aid_mgr->aid_mgr[j];
			if (vdev_aid_mgr &&
			    qdf_test_bit(assoc_id_ix,
					 vdev_aid_mgr->aid_bitmap)) {
				qdf_clear_bit(assoc_id_ix,
					      vdev_aid_mgr->aid_bitmap);
			}
		}
	} else {
		vdev_aid_mgr = ml_aid_mgr->aid_mgr[link_ix];
		if (vdev_aid_mgr)
			qdf_clear_bit(assoc_id_ix, vdev_aid_mgr->aid_bitmap);
	}

	ml_aid_lock_release(mlo_mgr_ctx);

	return QDF_STATUS_SUCCESS;
}

void wlan_mlme_peer_free_aid(
		struct wlan_vdev_aid_mgr *vdev_aid_mgr,
		bool no_lock, uint16_t assoc_id)
{
	struct mlo_mgr_context *mlo_mgr_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_mgr_ctx)
		return;

	if (!no_lock)
		ml_aid_lock_acquire(mlo_mgr_ctx);

	qdf_clear_bit(assoc_id - 1, vdev_aid_mgr->aid_bitmap);

	if (!no_lock)
		ml_aid_lock_release(mlo_mgr_ctx);
}

QDF_STATUS mlo_peer_allocate_aid(
		struct wlan_mlo_dev_context *ml_dev,
		struct wlan_mlo_peer_context *ml_peer)
{
	uint16_t assoc_id = (uint16_t)-1;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return QDF_STATUS_E_INVAL;

	assoc_id = wlan_mlo_peer_alloc_aid(ml_aid_mgr, true, 0xff);
	if (assoc_id == (uint16_t)-1)
		return QDF_STATUS_E_NOENT;

	ml_peer->assoc_id = assoc_id;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_peer_free_aid(struct wlan_mlo_dev_context *ml_dev,
			     struct wlan_mlo_peer_context *ml_peer)
{
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!ml_dev->ap_ctx) {
		mlo_err("ml_dev->ap_ctx is null");
		return QDF_STATUS_E_INVAL;
	}

	if (!ml_peer) {
		mlo_err("ml_peer is null");
		return QDF_STATUS_E_INVAL;
	}

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return QDF_STATUS_E_INVAL;

	wlan_mlo_peer_free_aid(ml_aid_mgr, 0xff, ml_peer->assoc_id);

	return status;
}

uint16_t mlo_get_aid(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *ml_dev;
	uint16_t assoc_id = (uint16_t)-1;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;

	ml_dev = vdev->mlo_dev_ctx;

	if (!ml_dev)
		return assoc_id;

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return assoc_id;

	return wlan_mlo_peer_alloc_aid(ml_aid_mgr, true, 0xff);
}

QDF_STATUS mlo_free_aid(struct wlan_objmgr_vdev *vdev, uint16_t assoc_id)
{
	struct wlan_mlo_dev_context *ml_dev;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;

	ml_dev = vdev->mlo_dev_ctx;

	if (!ml_dev)
		return QDF_STATUS_E_INVAL;

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return QDF_STATUS_E_INVAL;

	return wlan_mlo_peer_free_aid(ml_aid_mgr, true, assoc_id);
}

uint16_t mlme_get_aid(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *ml_dev;
	uint16_t assoc_id = (uint16_t)-1;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;
	struct wlan_vdev_aid_mgr *vdev_aid_mgr;
	bool no_lock = true;
	uint8_t link_id;

	ml_dev = vdev->mlo_dev_ctx;

	if (!ml_dev) {
		vdev_aid_mgr = wlan_vdev_mlme_get_aid_mgr(vdev);
		if (vdev_aid_mgr) {
			if (qdf_atomic_read(&vdev_aid_mgr->ref_cnt) > 1)
				no_lock = false;
			return wlan_mlme_peer_alloc_aid(vdev_aid_mgr, no_lock);
		}
		return assoc_id;
	}

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return assoc_id;

	link_id = mlo_get_link_vdev_ix(ml_dev, vdev);

	assoc_id = wlan_mlo_peer_alloc_aid(ml_aid_mgr, false, link_id);

	return assoc_id;
}

void mlme_free_aid(struct wlan_objmgr_vdev *vdev, uint16_t assoc_id)
{
	struct wlan_mlo_dev_context *ml_dev;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;
	struct wlan_vdev_aid_mgr *vdev_aid_mgr;
	bool no_lock = true;
	uint8_t link_id;

	ml_dev = vdev->mlo_dev_ctx;

	if (!ml_dev) {
		vdev_aid_mgr = wlan_vdev_mlme_get_aid_mgr(vdev);
		if (vdev_aid_mgr) {
			if (qdf_atomic_read(&vdev_aid_mgr->ref_cnt) > 1)
				no_lock = false;

			wlan_mlme_peer_free_aid(vdev_aid_mgr, no_lock,
						assoc_id);
		}
		return;
	}

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return;

	link_id = mlo_get_link_vdev_ix(ml_dev, vdev);

	wlan_mlo_peer_free_aid(ml_aid_mgr, link_id, assoc_id);
}

struct wlan_vdev_aid_mgr *wlan_vdev_aid_mgr_init(uint16_t max_aid)
{
	struct wlan_vdev_aid_mgr *aid_mgr;

	aid_mgr = qdf_mem_malloc(sizeof(struct wlan_vdev_aid_mgr));
	if (!aid_mgr)
		return NULL;

	aid_mgr->max_aid = max_aid;
	qdf_atomic_init(&aid_mgr->ref_cnt);
	/* Take reference before returning */
	qdf_atomic_inc(&aid_mgr->ref_cnt);

	return aid_mgr;
}

void wlan_vdev_aid_mgr_free(struct wlan_vdev_aid_mgr *aid_mgr)
{
	if (!aid_mgr)
		return;

	if (!qdf_atomic_dec_and_test(&aid_mgr->ref_cnt))
		return;

	aid_mgr->max_aid = 0;
	qdf_mem_free(aid_mgr);
}

QDF_STATUS wlan_mlo_vdev_alloc_aid_mgr(struct wlan_mlo_dev_context *ml_dev,
				       struct wlan_objmgr_vdev *vdev)
{
	uint8_t i;
	uint8_t is_mbss_enabled = 0;
	struct wlan_objmgr_vdev *vdev_iter;
	struct wlan_objmgr_vdev *tx_vdev = NULL;
	struct wlan_vdev_aid_mgr *aid_mgr;
	struct wlan_ml_vdev_aid_mgr *ml_aidmgr;
	uint16_t max_aid = WLAN_UMAC_MAX_AID;

	if (!ml_dev->ap_ctx) {
		mlo_err(" ML AP context is not initialized");
		QDF_BUG(0);
		return QDF_STATUS_E_NOMEM;
	}
	ml_aidmgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aidmgr) {
		mlo_err(" ML AID mgr allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		vdev_iter = ml_dev->wlan_vdev_list[i];
		if (!vdev_iter)
			continue;

		if (vdev != vdev_iter)
			continue;

		/* TODO */
		/* Get Tx VDEV, if VDEV is MBSSID */
		if (is_mbss_enabled) {
			aid_mgr = wlan_vdev_mlme_get_aid_mgr(tx_vdev);
			if (!aid_mgr) {
				mlo_err("AID bitmap allocation failed for Tx VDEV%d",
					wlan_vdev_get_id(tx_vdev));
				return QDF_STATUS_E_NOMEM;
			}
			qdf_atomic_inc(&aid_mgr->ref_cnt);
			ml_aidmgr->aid_mgr[i] = aid_mgr;
			wlan_vdev_mlme_set_aid_mgr(vdev,
						   ml_aidmgr->aid_mgr[i]);
			break;
		} else {
			ml_aidmgr->aid_mgr[i] = wlan_vdev_aid_mgr_init(max_aid);
			if (!ml_aidmgr->aid_mgr[i]) {
				mlo_err("AID bitmap allocation failed for VDEV%d",
					wlan_vdev_get_id(vdev));
				return QDF_STATUS_E_NOMEM;
			}
			wlan_vdev_mlme_set_aid_mgr(vdev, ml_aidmgr->aid_mgr[i]);
			break;
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlo_vdev_free_aid_mgr(struct wlan_mlo_dev_context *ml_dev,
				      struct wlan_objmgr_vdev *vdev)
{
	uint8_t i;
	struct wlan_objmgr_vdev *vdev_iter;
	struct wlan_ml_vdev_aid_mgr *ml_aidmgr;

	if (!ml_dev->ap_ctx) {
		mlo_err(" ML AP context is not initialized");
		QDF_BUG(0);
		return QDF_STATUS_E_NOMEM;
	}
	ml_aidmgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aidmgr) {
		mlo_err(" ML AID mgr allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		vdev_iter = ml_dev->wlan_vdev_list[i];
		if (!vdev_iter)
			continue;

		if (vdev != vdev_iter)
			continue;

		wlan_vdev_aid_mgr_free(ml_aidmgr->aid_mgr[i]);
		ml_aidmgr->aid_mgr[i] = NULL;
		break;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_mlo_vdev_aid_mgr_init(struct wlan_mlo_dev_context *ml_dev)
{
	uint8_t i;
	uint8_t is_mbss_enabled = 0;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_vdev *tx_vdev = NULL;
	struct wlan_vdev_aid_mgr *aid_mgr;
	struct wlan_ml_vdev_aid_mgr *ml_aidmgr;
	uint16_t max_aid = WLAN_UMAC_MAX_AID;

	ml_aidmgr = qdf_mem_malloc(sizeof(struct wlan_ml_vdev_aid_mgr));
	if (!ml_aidmgr) {
		ml_dev->ap_ctx->ml_aid_mgr = NULL;
		mlo_err(" ML AID mgr allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	ml_aidmgr->max_aid = max_aid;
	ml_dev->ap_ctx->ml_aid_mgr = ml_aidmgr;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		vdev = ml_dev->wlan_vdev_list[i];
		if (!vdev)
			continue;
		/* TODO */
		/* Get Tx VDEV, if VDEV is MBSSID */
		if (is_mbss_enabled) {
			aid_mgr = wlan_vdev_mlme_get_aid_mgr(tx_vdev);
			if (!aid_mgr) {
				mlo_err("AID bitmap allocation failed for Tx VDEV%d",
					wlan_vdev_get_id(tx_vdev));
				goto free_ml_aid_mgr;
			}

			qdf_atomic_inc(&aid_mgr->ref_cnt);
			ml_aidmgr->aid_mgr[i] = aid_mgr;
		} else {
			ml_aidmgr->aid_mgr[i] = wlan_vdev_aid_mgr_init(max_aid);
			if (!ml_aidmgr->aid_mgr[i]) {
				mlo_err("AID bitmap allocation failed for VDEV%d",
					wlan_vdev_get_id(vdev));
				goto free_ml_aid_mgr;
			}
			wlan_vdev_mlme_set_aid_mgr(vdev,
						   ml_aidmgr->aid_mgr[i]);
		}
	}

	return QDF_STATUS_SUCCESS;

free_ml_aid_mgr:
	wlan_mlo_vdev_aid_mgr_deinit(ml_dev);

	return QDF_STATUS_E_NOMEM;
}

void wlan_mlo_vdev_aid_mgr_deinit(struct wlan_mlo_dev_context *ml_dev)
{
	uint8_t i;
	struct wlan_ml_vdev_aid_mgr *ml_aid_mgr;
	int32_t n;

	ml_aid_mgr = ml_dev->ap_ctx->ml_aid_mgr;
	if (!ml_aid_mgr)
		return;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {

		if (ml_aid_mgr->aid_mgr[i]) {
			n = qdf_atomic_read(&ml_aid_mgr->aid_mgr[i]->ref_cnt);
			mlo_info("AID mgr ref cnt %d", n);
		} else {
			mlo_err("ID %d, doesn't have associated AID mgr", i);
			continue;
		}
		wlan_vdev_aid_mgr_free(ml_aid_mgr->aid_mgr[i]);
		ml_aid_mgr->aid_mgr[i] = NULL;
	}

	qdf_mem_free(ml_aid_mgr);
	ml_dev->ap_ctx->ml_aid_mgr = NULL;
}

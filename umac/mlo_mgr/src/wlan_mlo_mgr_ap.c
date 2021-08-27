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

/*
 * DOC: contains MLO manager ap related functionality
 */
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_mlo_mgr_ap.h"
#include <wlan_mlo_mgr_cmn.h>
#include <wlan_mlo_mgr_main.h>
#include <wlan_utility.h>

bool mlo_ap_vdev_attach(struct wlan_objmgr_vdev *vdev,
			uint8_t link_id,
			uint16_t vdev_count)
{
	struct wlan_mlo_dev_context *dev_ctx;

	if (!vdev || !vdev->mlo_dev_ctx || !vdev->mlo_dev_ctx->ap_ctx) {
		mlo_err("Invalid input");
		return false;
	}

	dev_ctx = vdev->mlo_dev_ctx;
	wlan_vdev_set_link_id(vdev, link_id);
	wlan_vdev_mlme_feat_ext2_cap_set(vdev, WLAN_VDEV_FEXT2_MLO);

	/**
	 * every link will trigger mlo_ap_vdev_attach,
	 * and they should provide the same vdev_count.
	 */
	mlo_dev_lock_acquire(dev_ctx);
	dev_ctx->ap_ctx->num_ml_vdevs = vdev_count;
	mlo_dev_lock_release(dev_ctx);

	return true;
}

void mlo_ap_get_vdev_list(struct wlan_objmgr_vdev *vdev,
			  uint16_t *vdev_count,
			  struct wlan_objmgr_vdev **wlan_vdev_list)
{
	struct wlan_mlo_dev_context *dev_ctx;
	int i;
	QDF_STATUS status;

	*vdev_count = 0;

	if (!vdev || !vdev->mlo_dev_ctx) {
		mlo_err("Invalid input");
		return;
	}

	dev_ctx = vdev->mlo_dev_ctx;

	mlo_dev_lock_acquire(dev_ctx);
	*vdev_count = 0;
	for (i = 0; i < QDF_ARRAY_SIZE(dev_ctx->wlan_vdev_list); i++) {
		if (dev_ctx->wlan_vdev_list[i] &&
		    wlan_vdev_mlme_is_mlo_ap(dev_ctx->wlan_vdev_list[i])) {
			status = wlan_objmgr_vdev_try_get_ref(
						dev_ctx->wlan_vdev_list[i],
						WLAN_MLO_MGR_ID);
			if (QDF_IS_STATUS_ERROR(status))
				break;
			wlan_vdev_list[*vdev_count] =
				dev_ctx->wlan_vdev_list[i];
			(*vdev_count) += 1;
		}
	}
	mlo_dev_lock_release(dev_ctx);
}

/**
 * mlo_is_ap_vdev_up_allowed() - Is mlo ap allowed to come up
 * @vdev: vdev pointer
 *
 * Return: true if given ap is allowed to up, false otherwise.
 */
static bool mlo_is_ap_vdev_up_allowed(struct wlan_objmgr_vdev *vdev)
{
	uint16_t vdev_count = 0;
	struct wlan_mlo_dev_context *dev_ctx;
	int i;
	bool up_allowed = false;

	if (!vdev || !vdev->mlo_dev_ctx || !vdev->mlo_dev_ctx->ap_ctx) {
		mlo_err("Invalid input");
		return up_allowed;
	}

	dev_ctx = vdev->mlo_dev_ctx;

	mlo_dev_lock_acquire(dev_ctx);
	for (i = 0; i < QDF_ARRAY_SIZE(dev_ctx->wlan_vdev_list); i++) {
		if (dev_ctx->wlan_vdev_list[i] &&
		    wlan_vdev_mlme_is_mlo_ap(dev_ctx->wlan_vdev_list[i]) &&
		    QDF_IS_STATUS_SUCCESS(
		    wlan_vdev_chan_config_valid(dev_ctx->wlan_vdev_list[i])))
			vdev_count++;
	}

	if (vdev_count == dev_ctx->ap_ctx->num_ml_vdevs)
		up_allowed = true;
	mlo_dev_lock_release(dev_ctx);

	return up_allowed;
}

/**
 * mlo_pre_link_up() - Carry out preparation before bringing up the link
 * @vdev: vdev pointer
 *
 * Return: true if preparation is done successfully
 */
static bool mlo_pre_link_up(struct wlan_objmgr_vdev *vdev)
{
	if (!vdev) {
		mlo_err("vdev is NULL");
		return false;
	}

	if ((wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_UP) &&
	    (wlan_vdev_mlme_get_substate(vdev) ==
	     WLAN_VDEV_SS_MLO_SYNC_WAIT))
		return true;

	return false;
}

/**
 * mlo_handle_link_ready() - Check if mlo ap is allowed to up or not.
 *                           If it is allowed, for every link in the
 *                           WLAN_VDEV_SS_MLO_SYNC_WAIT state, deliver
 *                           event WLAN_VDEV_SM_EV_MLO_SYNC_COMPLETE.
 *
 * This function is triggered once a link gets start response or enters
 * LAN_VDEV_SS_MLO_SYNC_WAIT state
 *
 * @vdev: vdev pointer
 *
 * Return: None
 */
static void mlo_handle_link_ready(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_vdev *vdev_list[WLAN_UMAC_MLO_MAX_VDEVS] = {NULL};
	uint16_t num_links = 0;
	uint8_t i;

	if (!vdev || !vdev->mlo_dev_ctx) {
		mlo_err("Invalid input");
		return;
	}

	if (!mlo_is_ap_vdev_up_allowed(vdev))
		return;

	mlo_ap_get_vdev_list(vdev, &num_links, vdev_list);
	if (!num_links || (num_links > QDF_ARRAY_SIZE(vdev_list))) {
		mlo_err("Invalid number of VDEVs under AP-MLD");
		return;
	}

	for (i = 0; i < num_links; i++) {
		if (mlo_pre_link_up(vdev_list[i]))
			wlan_vdev_mlme_sm_deliver_evt_sync(
					vdev_list[i],
					WLAN_VDEV_SM_EV_MLO_SYNC_COMPLETE,
					0, NULL);
		/* Release ref taken as part of mlo_ap_get_vdev_list */
		mlo_release_vdev_ref(vdev_list[i]);
	}
}

void mlo_ap_link_sync_wait_notify(struct wlan_objmgr_vdev *vdev)
{
	mlo_handle_link_ready(vdev);
}

void mlo_ap_link_start_rsp_notify(struct wlan_objmgr_vdev *vdev)
{
	mlo_handle_link_ready(vdev);
}

void mlo_ap_vdev_detach(struct wlan_objmgr_vdev *vdev)
{
	if (!vdev || !vdev->mlo_dev_ctx) {
		mlo_err("Invalid input");
		return;
	}
	wlan_vdev_mlme_feat_ext2_cap_clear(vdev, WLAN_VDEV_FEXT2_MLO);
}

void mlo_ap_link_down_cmpl_notify(struct wlan_objmgr_vdev *vdev)
{
	mlo_ap_vdev_detach(vdev);
}

uint16_t mlo_ap_ml_peerid_alloc(void)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	uint16_t i;

	ml_peerid_lock_acquire(mlo_ctx);
	for (i = 0; i < mlo_ctx->max_mlo_peer_id; i++) {
		if (qdf_test_bit(i, mlo_ctx->mlo_peer_id_bmap))
			continue;

		qdf_set_bit(i, mlo_ctx->mlo_peer_id_bmap);
		break;
	}
	ml_peerid_lock_release(mlo_ctx);

	if (i == mlo_ctx->max_mlo_peer_id)
		return MLO_INVALID_PEER_ID;

	return i + 1;
}

void mlo_ap_ml_peerid_free(uint16_t mlo_peer_id)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (mlo_peer_id == MLO_INVALID_PEER_ID)
		return;

	ml_peerid_lock_acquire(mlo_ctx);
	if (qdf_test_bit(mlo_peer_id - 1, mlo_ctx->mlo_peer_id_bmap))
		qdf_clear_bit(mlo_peer_id - 1, mlo_ctx->mlo_peer_id_bmap);

	ml_peerid_lock_release(mlo_ctx);
}

void mlo_ap_vdev_quiet_set(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mld_ctx = vdev->mlo_dev_ctx;
	uint8_t idx;

	if (!mld_ctx || !wlan_vdev_mlme_is_mlo_ap(vdev))
		return;

	idx = mlo_get_link_vdev_ix(mld_ctx, vdev);
	if (idx == MLO_INVALID_LINK_IDX)
		return;

	mlo_debug("Quiet set for PSOC:%d vdev:%d",
		  wlan_psoc_get_id(wlan_vdev_get_psoc(vdev)),
		  wlan_vdev_get_id(vdev));

	wlan_util_change_map_index(mld_ctx->ap_ctx->mlo_vdev_quiet_bmap,
				   idx, 1);
}

void mlo_ap_vdev_quiet_clear(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mld_ctx = vdev->mlo_dev_ctx;
	uint8_t idx;

	if (!mld_ctx || !wlan_vdev_mlme_is_mlo_ap(vdev))
		return;

	idx = mlo_get_link_vdev_ix(mld_ctx, vdev);
	if (idx == MLO_INVALID_LINK_IDX)
		return;

	mlo_debug("Quiet clear for PSOC:%d vdev:%d",
		  wlan_psoc_get_id(wlan_vdev_get_psoc(vdev)),
		  wlan_vdev_get_id(vdev));

	wlan_util_change_map_index(mld_ctx->ap_ctx->mlo_vdev_quiet_bmap,
				   idx, 0);
}

bool mlo_ap_vdev_quiet_is_any_idx_set(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mld_ctx = vdev->mlo_dev_ctx;

	if (!mld_ctx || !wlan_vdev_mlme_is_mlo_ap(vdev))
		return false;

	return wlan_util_map_is_any_index_set(
			mld_ctx->ap_ctx->mlo_vdev_quiet_bmap,
			sizeof(mld_ctx->ap_ctx->mlo_vdev_quiet_bmap));
}

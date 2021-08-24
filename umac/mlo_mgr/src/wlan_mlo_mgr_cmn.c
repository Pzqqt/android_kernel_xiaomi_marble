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
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_main.h"

void mlo_get_link_information(struct qdf_mac_addr *mld_addr,
			      struct mlo_link_info *info)
{
/* Pass the partner link information*/
}

void is_mlo_all_links_up(struct wlan_mlo_dev_context *mldev)
{
/* Loop through all the vdev's part of the ML device*/
/* STA: Loop through all the associated vdev status. */
}

struct wlan_objmgr_vdev *mlo_get_vdev_by_link_id(
			struct wlan_objmgr_vdev *vdev,
			uint8_t link_id)
{
	struct wlan_mlo_dev_context *dev_ctx;
	int i;
	struct wlan_objmgr_vdev *partner_vdev = NULL;

	if (!vdev || !vdev->mlo_dev_ctx) {
		mlo_err("Invalid input");
		return partner_vdev;
	}

	dev_ctx = vdev->mlo_dev_ctx;

	mlo_dev_lock_acquire(dev_ctx);
	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (dev_ctx->wlan_vdev_list[i] &&
		    wlan_vdev_mlme_is_mlo_vdev(dev_ctx->wlan_vdev_list[i]) &&
		    dev_ctx->wlan_vdev_list[i]->vdev_mlme.mlo_link_id ==
		    link_id) {
			if (wlan_objmgr_vdev_try_get_ref(
						dev_ctx->wlan_vdev_list[i],
						WLAN_MLO_MGR_ID) ==
							QDF_STATUS_SUCCESS)
				partner_vdev = dev_ctx->wlan_vdev_list[i];

			break;
		}
	}
	mlo_dev_lock_release(dev_ctx);

	return partner_vdev;
}

void mlo_release_vdev_ref(struct wlan_objmgr_vdev *vdev)
{
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLO_MGR_ID);
}

QDF_STATUS mlo_reg_mlme_ext_cb(struct mlo_mgr_context *ctx,
			       struct mlo_mlme_ext_ops *ops)
{
	if (!ctx)
		return QDF_STATUS_E_FAILURE;

	ctx->mlme_ops = ops;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_unreg_mlme_ext_cb(struct mlo_mgr_context *ctx)
{
	if (!ctx)
		return QDF_STATUS_E_FAILURE;

	ctx->mlme_ops = NULL;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlo_mlme_validate_conn_req(struct wlan_objmgr_vdev *vdev,
				      void *ext_data)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_validate_conn_req)
		return QDF_STATUS_E_FAILURE;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return QDF_STATUS_E_FAILURE;

	status =
		mlo_ctx->mlme_ops->mlo_mlme_ext_validate_conn_req(vdev_mlme,
								  ext_data);
	return status;
}

QDF_STATUS mlo_mlme_create_link_vdev(struct wlan_objmgr_vdev *vdev,
				     void *ext_data)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_create_link_vdev)
		return QDF_STATUS_E_FAILURE;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return QDF_STATUS_E_FAILURE;

	status =
		mlo_ctx->mlme_ops->mlo_mlme_ext_create_link_vdev(vdev_mlme,
								 ext_data);
	return status;
}

void mlo_mlme_peer_create(struct wlan_objmgr_vdev *vdev,
			  struct wlan_mlo_peer_context *ml_peer,
			  struct qdf_mac_addr *addr,
			  qdf_nbuf_t frm_buf)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_create)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_create(vdev, ml_peer,
						    addr, frm_buf);
}

void mlo_mlme_peer_assoc(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc(peer);
}

void mlo_mlme_peer_assoc_fail(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc_fail)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_assoc_fail(peer);
}

void mlo_mlme_peer_delete(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_peer_delete)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_peer_delete(peer);
}

void mlo_mlme_peer_assoc_resp(struct wlan_objmgr_peer *peer)
{
	struct mlo_mgr_context *mlo_ctx = wlan_objmgr_get_mlo_ctx();

	if (!mlo_ctx || !mlo_ctx->mlme_ops ||
	    !mlo_ctx->mlme_ops->mlo_mlme_ext_assoc_resp)
		return;

	mlo_ctx->mlme_ops->mlo_mlme_ext_assoc_resp(peer);
}

uint8_t mlo_get_link_vdev_ix(struct wlan_mlo_dev_context *ml_dev,
			     struct wlan_objmgr_vdev *vdev)
{
	uint8_t i;

	mlo_dev_lock_acquire(ml_dev);
	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (vdev == ml_dev->wlan_vdev_list[i]) {
			mlo_dev_lock_release(ml_dev);
			return i;
		}
	}
	mlo_dev_lock_release(ml_dev);

	return (uint8_t)-1;
}
